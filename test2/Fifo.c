#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/semaphore.h>
#define BUFF_SIZE 50
MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;

DECLARE_WAIT_QUEUE_HEAD(readQ);
DECLARE_WAIT_QUEUE_HEAD(writeQ);
struct semaphore sem;

uint fifo[16];
int rpos = 0; //Pokazivac na prvi upisani el.
int wpos = 0; //Pokazivac na sledece slobodno mesto
int num_of_el = 0; //ukupan broj podataka u baferu
int endRead = 0;  //broj read procesa spremnih za terminiranje
int times_read = 0; //broj pozivanja read funkcije
int read_mode = 0; //0 za hex, 1 za dec
int read_num = 1; //broj clanova za citanje

//rpos == wpos => fifo prazan
//(rpos+1)%10 == wpos => fifo pun
//ideja za popravku, dodaj promenljivu koja oznacava slobodna mesta!

int fifo_open(struct inode *pinode, struct file *pfile);
int fifo_close(struct inode *pinode, struct file *pfile);
ssize_t fifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops = {
  .owner = THIS_MODULE,
  .open = fifo_open,
  .read = fifo_read,
  .write = fifo_write,
  .release = fifo_close,
};

int fifo_open(struct inode *pinode, struct file *pfile) {
  printk(KERN_INFO "Succesfully opened fifo\n");
  return 0;
}

int fifo_close(struct inode *pinode, struct file *pfile) {
  printk(KERN_INFO "Succesfully closed fifo\n");
  return 0;
}

ssize_t fifo_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
  int ret;
  char buff[BUFF_SIZE];
  long int len = 0;

  if(endRead > 0) {
    //Zauzmi semafor jer pristupas deljenom resursu
    if(down_interruptible(&sem))
      return -ERESTARTSYS;

    endRead--;
    up(&sem);

    return 0;
  }

  //Kreni da citas read_num puta, zauzmi semafor
  if(down_interruptible(&sem))
    return -ERESTARTSYS;

  //Ako je prazan bafer, predji u listu cekanja
  while(num_of_el == 0) {
    up(&sem);

    if(wait_event_interruptible(readQ, (num_of_el > 0)))
      return -ERESTARTSYS;

    if(down_interruptible(&sem))
      return -ERESTARTSYS;
  }

  if(num_of_el > 0) {
    num_of_el--;

    if(read_mode) {
      //Decimalno citanje ako je read_mode = 1
      len = scnprintf(buff, BUFF_SIZE, "%d\n", fifo[rpos]);
    } else {
      //Heksadecimalno citanje ako je read_mode = 0
      len = scnprintf(buff, BUFF_SIZE, "0x%x\n", fifo[rpos]);
    }

    printk(KERN_INFO "Succesfully read value 0x%x\n", fifo[rpos]);

    ret = copy_to_user(buffer, buff, len);
    if(ret)
      return -EFAULT;

    rpos = (rpos + 1) % 16; //Povecaj pokazivac za citanje
    times_read++; //Proslo citanje
  }

  //Ako procitas read_num puta, pripremi se za terminiranje
  if(times_read == read_num) {
    times_read = 0;
    endRead++; //Povecaj broj procesa koji cekaju na terminiranje
  }

  //Oslobodi semafor i pomeri listu cekanja za upis
  printk(KERN_WARNING "OSLOBODICU SEMAFOR!\n");
  up(&sem);
  wake_up_interruptible(&writeQ);

  return len;
}


ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
  int ret;
  char buff[BUFF_SIZE];
  char *token;
  char *parse_buff = buff;
  uint value[BUFF_SIZE];
  int num_of_values = 0;
  int i = 0;

  ret = copy_from_user(buff, buffer, length);
  if (ret)
    return -EFAULT;
  buff[length - 1] = '\0';

  if (strncmp(buff, "hex", 3) == 0) {
    if (down_interruptible(&sem))
      return -ERESTARTSYS;

    printk(KERN_INFO "Read mode set to hexadecimal\n");
    read_mode = 0;
    up(&sem);
  } else if (strncmp(buff, "dec", 3) == 0) {
    if (down_interruptible(&sem))
      return -ERESTARTSYS;

    read_mode = 1;
    printk(KERN_INFO "Read mode set to decimal\n");
    up(&sem);
  } else if (strncmp(buff, "num=", 4) == 0) {
    if (down_interruptible(&sem))
      return -ERESTARTSYS;

    sscanf(buff, "num=%d", &read_num);
    printk(KERN_INFO "Reading %d elements from FIFO buffer\n", read_num);
    up(&sem);
  } else {
    //upis brojeva
    token = strsep(&parse_buff, ",");
    while(token != NULL) {
      kstrtouint(token, 16, &value[num_of_values]);
      printk(KERN_INFO "Parsovani string je : %s\n", token);
      printk(KERN_INFO "Konvertovana vrednost je : %u\n", value[num_of_values]);

      if (value[num_of_values] > 255) {
        printk(KERN_WARNING "Value above 255\n");
      } else {
        num_of_values++;
      }

      token = strsep(&parse_buff, ",");
    }

    //Gotovo parsiranje stringa, sledi upis
    if(down_interruptible(&sem))
      return -ERESTARTSYS;

    while (num_of_values > 0) {
      while(num_of_el == 16) {
        up(&sem);
        if(wait_event_interruptible(writeQ, (num_of_el < 16)))
          return -ERESTARTSYS;
        if(down_interruptible(&sem))
          return -ERESTARTSYS;
      }

      if(num_of_el < 16) {
        fifo[wpos] = value[i];
        printk(KERN_INFO "Succesfully wrote %u\n", value[i]);
        i++;
        wpos = (wpos + 1) % 16;
        num_of_el++;
        num_of_values--;
      }

      //Probudi proces za citanje iz liste cekanja
      wake_up_interruptible(&readQ);
    }

    //Oslobodi semafor tek kad upises sve elemente!
    printk(KERN_WARNING "Broj elemenata u baferu je : %d\n", num_of_el);
    up(&sem);
  }

  return length;
}

static int __init fifo_init(void) {
  int ret = 0;
  int i = 0;

  sema_init(&sem, 1);

  //Inicijalizacija niza
  for (i = 0; i < 16; i++)
    fifo[i] = 0;

  ret = alloc_chrdev_region(&my_dev_id, 0, 1, "fifo");
  if (ret) {
    printk(KERN_ERR "Failed to register char device\n");
    return ret;
  }
  printk(KERN_INFO "Char device region allocated\n");

  my_class = class_create(THIS_MODULE, "fifo_class");
  if (my_class == NULL) {
    printk(KERN_ERR "Failed to create class\n");
    goto fail_0;
  }
  printk(KERN_INFO "Class created\n");

  my_device = device_create(my_class, NULL, my_dev_id, NULL, "fifo");
  if (my_device == NULL) {
    printk(KERN_ERR "Failed to create device\n");
    goto fail_1;
  }
  printk(KERN_INFO "Device created\n");

  my_cdev = cdev_alloc();
  my_cdev -> ops = &my_fops;
  my_cdev -> owner = THIS_MODULE;
  ret = cdev_add(my_cdev, my_dev_id, 1);
  if (ret) {
    printk(KERN_ERR "Failed to add cdev\n");
    goto fail_2;
  }
  printk(KERN_INFO "cdev added\n");
  printk(KERN_INFO "Driver initialized\n");
  return 0;

 fail_2:
  device_destroy(my_class, my_dev_id);
 fail_1:
  class_destroy(my_class);
 fail_0:
  unregister_chrdev_region(my_dev_id, 1);

  return -1;
}

static void __exit fifo_exit(void) {
  cdev_del(my_cdev);
  device_destroy(my_class, my_dev_id);
  class_destroy(my_class);
  unregister_chrdev_region(my_dev_id, 1);
  printk(KERN_INFO "Driver removed, goodbye\n");
}

module_init(fifo_init);
module_exit(fifo_exit);
