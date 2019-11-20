#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/kdev_t.h>
#include <linux/uaccess.h>
#include <linux/errno.h>

MODULE_LICENSE("Dual BSD/GPL");

dev_t my_dev_id;
static struct class *my_class;
static struct device *my_device;
static struct cdev *my_cdev;
#define num_of_minors 10
int storage[num_of_minors][10]; //niz od 10 nizova
int pos = 0;
int endRead = 0;

int storage_open(struct inode *pinode, struct file *pfile);
int storage_close(struct inode *pinode, struct file *pfile);
ssize_t storage_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset);
ssize_t storage_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset);

struct file_operations my_fops =
  {
    .owner = THIS_MODULE,
    .open = storage_open,
    .read = storage_read, 
    .write = storage_write,
    .release = storage_close,
  };


int storage_open(struct inode *pinode, struct file *pfile) 
{
  printk(KERN_INFO "Succesfully opened file\n");
  return 0;
}

int storage_close(struct inode *pinode, struct file *pfile) 
{
  printk(KERN_INFO "Succesfully closed file\n");
  return 0;
}

ssize_t storage_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) 
{
  int ret;
  char buff[20];
  long int len;
  int minor = MINOR(pfile->f_inode->i_rdev);
  if (endRead){
    endRead = 0;
    pos = 0;
    printk(KERN_INFO "Succesfully read from file\n");
    return 0;
  }
  len = scnprintf(buff, strlen(buff), "%d ", storage[minor][pos]);
  ret = copy_to_user(buffer, buff, len);
  if(ret)
    return -EFAULT;
  pos ++;
  if (pos == 10) {
    endRead = 1;
  }
  return len;
}

ssize_t storage_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) 
{
  char buff[20];
  int value, position;
  int ret;
  int minor = MINOR(pfile->f_inode->i_rdev);

  ret = copy_from_user(buff, buffer, length);
  if(ret)
    return -EFAULT;
  buff[length-1] = '\0';

  ret = sscanf(buff,"%d,%d",&value,&position);

  if(ret==2)//two parameters parsed in sscanf
    {
      if(position >=0 && position <=9)
	{
	  storage[minor][position] = value; 
	    printk(KERN_INFO "Succesfully wrote value %d at position %d in minor storage%d\n", value, position, minor);
	}
      else
	{
	  printk(KERN_WARNING "Position should be between 0 and 9\n"); 
	}
    }
  else
    {
      printk(KERN_WARNING "Wrong command format\nexpected: n,m\n\tn-position\n\tm-value\n");
    }


  return length;
}

static int __init storage_init(void)
{
  int ret = 0;
  int i=0;
  int j=0;
  char buff[10];
 
  for (i = 0; i < num_of_minors; i++) {
    for (j = 0; j < 10; j++) {
      storage[i][j] = 0;
    }
  }
  ret = alloc_chrdev_region(&my_dev_id, 0, num_of_minors, "storage");
  if (ret){
    printk(KERN_INFO "failed to register char device\n");
    return ret;
  }
  printk(KERN_INFO "char device region allocated\n");

  my_class = class_create(THIS_MODULE, "storage_class");
  if (my_class == NULL){
    printk(KERN_ERR "failed to create class\n");
    goto fail_0;
  }
  printk(KERN_INFO "class created\n");

  printk(KERN_INFO "creating device\n");
  for (i = 0; i < num_of_minors; i++)
    {
      printk(KERN_INFO "created nod %d\n", i);
      scnprintf(buff, 10, "storage%d", i);
      my_device = device_create(my_class, NULL, MKDEV(MAJOR(my_dev_id), i), NULL, buff);
      if (my_device == NULL){
	printk(KERN_ERR "failed to create device\n");
	goto fail_1;
      }

    }
  printk(KERN_INFO "device created\n");

  my_cdev = cdev_alloc();	
  my_cdev->ops = &my_fops;
  my_cdev->owner = THIS_MODULE;
  ret = cdev_add(my_cdev, my_dev_id, num_of_minors);
  if (ret)
    {
      printk(KERN_ERR "failed to add cdev\n");
      goto fail_2;
    }
  printk(KERN_INFO "cdev added\n");
  printk(KERN_INFO "Hello world\n");

  return 0;

 fail_2:
  for (j = 0; j < i; j++)
    device_destroy(my_class, MKDEV(MAJOR(my_dev_id), j));
 fail_1:
  class_destroy(my_class);
 fail_0:
  unregister_chrdev_region(my_dev_id, 1);
  return -1;
}

static void __exit storage_exit(void)
{
  int i = 0;
  cdev_del(my_cdev);
  for (i = 0; i < num_of_minors; i++) {
    device_destroy(my_class, MKDEV(MAJOR(my_dev_id), i));
  }
  class_destroy(my_class);
  unregister_chrdev_region(my_dev_id,1);
  printk(KERN_INFO "Goodbye, cruel world\n");
}


module_init(storage_init);
module_exit(storage_exit);
