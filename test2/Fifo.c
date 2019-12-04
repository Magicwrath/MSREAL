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

int fifo[10];
int rpos = 0; //Pokazivac na prvi upisani el.
int wpos = 0; //Pokazivac na sledece slobodno mesto
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

  while (rpos == wpos) {
    up(&sem);
    if (wait_event_interruptible(readQ, (rpos != wpos))) {
      return -ERESTARTSYS;
    }
    if (down_interruptible(&sem)) {
      return -ERESTARTSYS;
    }
  }

  //Prodje proces koji dobije semafor, ako bafer nije prazan
  if (rpos != wpos) {
    len = scnprintf(buff, BUFF_SIZE, "%d", fifo[rpos]); //cita sa rpos mesta
    ret = copy_to_user(buffer, buff, len);
    rpos = (rpos + 1) % 10;
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

ssize_t fifo_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
  int ret = 0;
  int value;
  char buff[BUFF_SIZE];

  ret = copy_from_user(buff, buffer, length);
  if (ret)
    return -EFAULT;

  buff[length-1] = '\0';
  if (down_interruptible(&sem))
    return -ERESTARTSYS;

  while (((rpos + 1)%10) == wpos) {
    up(&sem);
    if (wait_event_interruptible(writeQ, (((rpos + 1)%10) == wpos)))
      return -ERESTARTSYS;
    if (down_interruptible(&sem))
      return -ERESTARTSYS;
  }

  if (((rpos + 1)%10) != wpos) {
    ret = sscanf(buff, "%d", &value);
    if (ret == 1) {
      printk(KERN_INFO "Succesfully wrote value %d", value);
      fifo[wpos] = value;
      wpos = (wpos + 1) % 10;
    } else {
      printk(KERN_WARNING "Wrong command format\n");
    }
  } else {
    printk(KERN_WARNING "Fifo is full\n");
  }

  up(&sem);
  wake_up_interruptible(&readQ);
  return length;
}

static int __init fifo_init(void) {
  int ret = 0;
  int i = 0;

  sema_init(&sem, 1);

  //Inicijalizacija niza
  for (i = 0; i < 10; i++)
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
