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
#define BUFF_SIZE 20
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
int endRead = 0;

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

  if (endRead) {
    endRead = 0;
    return 0;
  }

  if(down_interruptible(&sem)) {
    return -ERESTARTSYS;
  }

  while (num_of_el == 0) {
    up(&sem);
    if (wait_event_interruptible(readQ, (num_of_el > 0))) {
      return -ERESTARTSYS;
    }
    if (down_interruptible(&sem)) {
      return -ERESTARTSYS;
    }
  }

  //Prodje proces koji dobije semafor, ako bafer nije prazan
  if (num_of_el > 0) {
    len = scnprintf(buff, BUFF_SIZE, "%d", fifo[rpos]); //cita sa rpos mesta
    ret = copy_to_user(buffer, buff, len);
    rpos = (rpos + 1) % 16;
    num_of_el--;
    if (ret)
      return -EFAULT;
    printk(KERN_INFO "Successfully read\n");
  } else {
    printk(KERN_INFO "Fifo is empty\n");
  }

  up(&sem);
  wake_up_interruptible(&writeQ);

  endRead = 1;
  return len;
}

//Dobro parsuje, doraditi kod ukoliko neko unese hex sa vise/manje od 2 bajta
//Problem, upisuje vrednosti od nazad
//echo 1,2 > /dev/fifo prvo upise 2 pa 1
ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
  int ret;
  int i = 0;
  int last_comma = 0;
  int num_of_value = 0;
  char buff[BUFF_SIZE];
  uint value[BUFF_SIZE];

  ret = copy_from_user(buff, buffer, length);
  if (ret)
    return -EFAULT;
  buff[length - 1] = '\0';

  if (strchr(buff, ',') != NULL) {
    buff[length - 1] = ',';
    buff[length] = '\0';
  } //umesto '\0' stavi ',' da bi radio algoritam dole

  down_interruptible(&sem);
  for (i = 0; i < length; i++) {
    char parse_buff[BUFF_SIZE];
    if (buff[i] == ',') {
      strncpy(parse_buff, buff + last_comma, 2);
      parse_buff[2] = '\0';
      kstrtouint(parse_buff, 16, &value[num_of_value]);
      printk(KERN_INFO "Parsovani string je : %s", parse_buff);
      printk(KERN_INFO "Konvertovana vrednost je : %u", value[num_of_value]);
      if (value[num_of_value] > 255) {
        printk(KERN_WARNING "Value above 255\n");
      } else {
        num_of_value++;
      }
      last_comma = i + 1;
    }
  }
  up(&sem);

  //slucaj ako nema zareza
  if (last_comma == 0) {
    kstrtouint(buff, 16, &value[num_of_value]);
    if (value[num_of_value] > 255) {
      printk(KERN_WARNING "Value above 255\n");
    } else {
      num_of_value++;
    }
  }

  while (num_of_value > 0) {
    printk(KERN_INFO "Number of value is: %d", num_of_value-1);
    printk(KERN_INFO "value[%d] = %u", num_of_value-1, value[num_of_value-1]);
    //Zauzima semafor svaki put kad prodje petlju za upis elementa
    if (down_interruptible(&sem))
      return -ERESTARTSYS;

    while (num_of_el == 16) {
      //Oslobadja semafor i ceka budjenje dok se ne oslobodi mesto u baferu
      up(&sem);
      if(wait_event_interruptible(writeQ, (num_of_el < 16)))
        return -ERESTARTSYS;
      if(down_interruptible(&sem))
        return -ERESTARTSYS;
    }

    if(num_of_el < 16) {
      fifo[wpos] = value[num_of_value-1];
      printk(KERN_INFO "Succesfully wrote %u\n", value[num_of_value-1]);
      wpos = (wpos + 1) % 16;
      num_of_el++;
      num_of_value--;

      //probudi uspavani proces za citanje
      up(&sem);
      wake_up_interruptible(&readQ);
    }
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
