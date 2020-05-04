
#ifndef __bleserial_H
#define __bleserial_H

struct bleserial_dev {
    struct mutex mutex;       /* Mutual exclusion semaphore. */
    struct cdev cdev;     /* Char device structure. */
};

void bleserial_setup_cdev(struct bleserial_dev *dev, int index);

#endif /* __bleserial_H */