#include <linux/module.h> 
#include <linux/kernel.h> 
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>     /* kmalloc() */
#include <linux/fs.h>       /* everything... */
#include "bleserial.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Eug");
MODULE_DESCRIPTION("A BLE module");

int bleserial_major =   0;
int bleserial_minor =   0;
int bleserial_nr_devs = 4;
struct bleserial_dev *bleserial_devices;    /* allocated in init_module */

#include "dev.c"

/*
 * Empty out the bleserial device; must be called with the device
 * mutex held.
 */
static int bleserial_trim(struct bleserial_dev *dev)
{
    //kfree

    return 0;
}

static void bleserial_cleanup(void)
{
    int i;
    dev_t devno = MKDEV(bleserial_major, bleserial_minor);
    
    /* Get rid of our char dev entries. */
    if (bleserial_devices) {
        for (i = 0; i < bleserial_nr_devs; i++) {
            bleserial_trim(bleserial_devices + i);
            cdev_del(&bleserial_devices[i].cdev);
        }
        kfree(bleserial_devices);
    }
    /* cleanup_module is never called if registering failed. */
    unregister_chrdev_region(devno, bleserial_nr_devs);
    printk(KERN_INFO "bleserial: cleanup success\n");
}

static int __init bleserial_init(void)
{
    int result, i;
    dev_t dev = 0;

    /*
     * Get a range of minor numbers to work with, asking for a dynamic major
     * unless directed otherwise at load time.
     */
    if (bleserial_major) {
        dev = MKDEV(bleserial_major, bleserial_minor);
        result = register_chrdev_region(dev, bleserial_nr_devs, "bleserial");
    } else {
        result = alloc_chrdev_region(&dev, bleserial_minor, 
            bleserial_nr_devs, "bleserial");
        bleserial_major = MAJOR(dev);
    }

    if (result < 0) {
        printk(KERN_WARNING "bleserial: can't get major %d\n", bleserial_major);
        return result;
    } else {
        printk(KERN_INFO "bleserial: get major %d success\n", bleserial_major);
    }

    /*
     * Allocate the devices. This must be dynamic as the device number can
     * be specified at load time.
     */
    bleserial_devices = kmalloc(bleserial_nr_devs * 
                sizeof(struct bleserial_dev), GFP_KERNEL);
    if (!bleserial_devices) {
        result = -ENOMEM;
        goto fail;
    }
    memset(bleserial_devices, 0, bleserial_nr_devs *
                sizeof(struct bleserial_dev));

    /* Initialize each device. */
    for (i = 0; i < bleserial_nr_devs; i++) {
        mutex_init(&bleserial_devices[i].mutex);
        bleserial_setup_cdev(&bleserial_devices[i], i);
    }
    return 0; /* succeed */

fail:
    bleserial_cleanup();
    return result;
}

module_init(bleserial_init);
module_exit(bleserial_cleanup);
