
#include <linux/module.h>
#include <linux/fs.h>       /* everything... */
#include <linux/errno.h>    /* error codes */
#include <linux/types.h>    /* size_t */
#include <linux/kernel.h>   /* printk() */
#include <linux/fcntl.h>    /* O_ACCMODE */
#include <linux/cdev.h>
#include <asm/uaccess.h>    /* copy_*_user */
#include "bleserial.h"

/*
 * Data management: read and write.
 */
ssize_t bleserial_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    ssize_t retval = 0;
    struct bleserial_dev *dev = filp->private_data;

    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;

//out:
    mutex_unlock(&dev->mutex);
    return retval;
}

ssize_t bleserial_write(struct file *filp, const char __user *buf,
                size_t count, loff_t *f_pos)
{
    struct bleserial_dev *dev = filp->private_data;
    ssize_t retval = -ENOMEM; /* Value used in "goto out" statements. */

    if (mutex_lock_interruptible(&dev->mutex))
        return -ERESTARTSYS;

//out:
    mutex_unlock(&dev->mutex);
    return retval;
}

int bleserial_open(struct inode *inode, struct file *filp)
{
    struct bleserial_dev *dev; /* device information */

    dev = container_of(inode->i_cdev, struct bleserial_dev, cdev);
    filp->private_data = dev; /* for other methods */

    printk(KERN_DEBUG "process %i (%s) success open minor(%u) file\n",
        current->pid, current->comm, iminor(inode));
    return 0;
}

int bleserial_release(struct inode *inode, struct file *filp)
{
    printk(KERN_DEBUG "process %i (%s) success release minor(%u) file\n", 
        current->pid, current->comm, iminor(inode));
    return 0;
}

loff_t bleserial_llseek(struct file *filp, loff_t off, int whence)
{
    return -EINVAL;
}

struct file_operations bleserial_fops = {
    .owner =    THIS_MODULE,
    .llseek =   bleserial_llseek,
    .read =     bleserial_read,
    .write =    bleserial_write,
    .open =     bleserial_open,
    .release =  bleserial_release,
};

/*
 * Set up the char_dev structure for this device.
 */
extern int bleserial_major;
extern int bleserial_minor;
void bleserial_setup_cdev(struct bleserial_dev *dev, int index)
{
    int err, devno = MKDEV(bleserial_major, bleserial_minor + index);
    cdev_init(&dev->cdev, &bleserial_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &bleserial_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    /* Fail gracefully if need be. */
    if (err)
        printk(KERN_NOTICE "Error %d adding bleserial%d", err, index);
    else
        printk(KERN_INFO "bleserial: %d add success\n", index);
}
