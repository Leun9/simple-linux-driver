#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/mutex.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("U201814846@hust.edu.cn");
MODULE_DESCRIPTION("kfifo_buf in a blocking or a nonblocking mode.");

#define DEVICE_NAME "kfifo_buf"
#define BUFFER_SIZE 32
#define TIMEOUT_MS 1000

static char __dev_name[] = DEVICE_NAME;

static int __major_num;
static struct kfifo __kfifo_buf;
static struct mutex __mutex_read;
static struct mutex __mutex_write;
static wait_queue_head_t __read_queue;
static wait_queue_head_t __write_queue;

#define PRT_CALLED()                                                            \
do {                                                                            \
    printk(KERN_INFO "%s: %s: start. count: %ld.\n",                            \
        __dev_name, __func__, count);                                           \
} while (0);

#define PRT_RET(retcode)                                                        \
do {                                                                            \
    printk(KERN_INFO "%s: %s: return. retcode = %d. buf size: %u.\n",           \
        __dev_name, __func__, retcode, kfifo_len(&__kfifo_buf));                \
    return retcode;                                                             \
} while (0);


static ssize_t
driver_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    int ret;
    int actual_readed;

    PRT_CALLED();
    if (file->f_flags &O_NONBLOCK) {
        if (0 == mutex_trylock(&__mutex_read))
            PRT_RET(-EBUSY);
    }
    else 
        mutex_lock(&__mutex_read);

    if (kfifo_is_empty(&__kfifo_buf))
    {
        if (file->f_flags &O_NONBLOCK) {
            mutex_unlock(&__mutex_read);
            PRT_RET(-EAGAIN);
        }

        ret = wait_event_interruptible_timeout(__read_queue,
            !kfifo_is_empty(&__kfifo_buf), msecs_to_jiffies(TIMEOUT_MS));
        if (ret == 0) {
            mutex_unlock(&__mutex_read);
            PRT_RET(-ETIMEDOUT);
        }
    }
    ret = kfifo_to_user(&__kfifo_buf, buf, count, &actual_readed);
    
    if (!kfifo_is_full(&__kfifo_buf))
        wake_up_interruptible(&__write_queue);

    mutex_unlock(&__mutex_read);

    PRT_RET(actual_readed);
}

static ssize_t
driver_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    int ret;
    int actual_write;

    PRT_CALLED();
    if (file->f_flags &O_NONBLOCK) {
        if (0 == mutex_trylock(&__mutex_write))
            PRT_RET(-EBUSY);
    }
    else 
        mutex_lock(&__mutex_write);

    if (kfifo_is_full(&__kfifo_buf))
    {
        if (file->f_flags &O_NONBLOCK) {
            mutex_unlock(&__mutex_write);
            PRT_RET(-EAGAIN);
        }

        ret = wait_event_interruptible_timeout(__write_queue,
            !kfifo_is_full(&__kfifo_buf), msecs_to_jiffies(TIMEOUT_MS));
        if (ret == 0) {
            mutex_unlock(&__mutex_write);
            PRT_RET(-ETIMEDOUT);
        }
    }

    ret = kfifo_from_user(&__kfifo_buf, buf, count, &actual_write);
    
    if (!kfifo_is_empty(&__kfifo_buf))
        wake_up_interruptible(&__read_queue);

    mutex_unlock(&__mutex_write);

    PRT_RET(actual_write);
}

static int
driver_open(struct inode *inode, struct file *file) {
    printk(KERN_INFO "%s: %s: nonblock = %d.\n", __dev_name, __func__,
        (bool)(file->f_flags & O_NONBLOCK));
	return 0;
}


static struct file_operations 
driver_fops = {
    .owner      =   THIS_MODULE,
    .open       =   driver_open,
    .read       =   driver_read,    
    .write      =   driver_write,
};

static int
__init driver_init(void) {
    int ret = 0;
    
    ret = kfifo_alloc(&__kfifo_buf, BUFFER_SIZE, GFP_KERNEL);
    if (ret != 0) {
        printk(KERN_ERR "KFIFO malloc failed, errcode = %d.\n", ret);
        return ret;
    }
    
    mutex_init(&__mutex_read);
    mutex_init(&__mutex_write);
    init_waitqueue_head(&__read_queue);
    init_waitqueue_head(&__write_queue);

    ret = register_chrdev(0, __dev_name, &driver_fops);
    if (ret < 0) {
        printk(KERN_ERR "Register character device error, errorcode = %d.\n", ret);
        return ret;
    }
    __major_num = ret;
    printk(KERN_NOTICE "Register character device succeed, major number = %d.\n",
        __major_num);

    return 0;
}

static void
__exit driver_exit(void) {
    if (__major_num != 0) {
        unregister_chrdev(__major_num, __dev_name);
    }
	kfifo_free(&__kfifo_buf);
    printk(KERN_INFO "Unregister character device succeed.\n");
}

module_init(driver_init);
module_exit(driver_exit);