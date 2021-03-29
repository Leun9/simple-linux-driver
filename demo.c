#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/kfifo.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("U201814846@hust.edu.cn");
MODULE_DESCRIPTION("block_dev");

#define DEVICE_NAME "demo"
#define BUFFER_SIZE 32
#define TIMEOUT 1000

struct BlockDev {
    int major_num;
    struct kfifo kfifo_buf;
    wait_queue_head_t read_queue;
    wait_queue_head_t write_queue;
};

static struct BlockDev block_dev;
static char dev_name[] = DEVICE_NAME;

#define PRT_CALLED()                                                            \
do {                                                                            \
    printk(KERN_INFO "%s: %s: start. count: %ld.\n",                            \
        dev_name, __func__, count);                                             \
} while (0);

#define PRT_RET(retcode)                                                        \
do {                                                                            \
    printk(KERN_INFO "%s: %s: return. retcode = %d. buf size: %u.\n",           \
        dev_name, __func__, retcode, kfifo_len(&block_dev.kfifo_buf));          \
    return retcode;                                                             \
} while (0);



static ssize_t
driver_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    int ret;
    int actual_readed;

    PRT_CALLED();
    if (kfifo_is_empty(&block_dev.kfifo_buf)) {
        if (file->f_flags &O_NONBLOCK)
            PRT_RET(-EAGAIN);

        ret = wait_event_interruptible_timeout(block_dev.read_queue,
            !kfifo_is_empty(&block_dev.kfifo_buf), msecs_to_jiffies(TIMEOUT));
        if (ret == 0) 
            PRT_RET(-ETIMEDOUT);
    }

    ret = kfifo_to_user(&block_dev.kfifo_buf, buf, count, &actual_readed);
    
    if (!kfifo_is_full(&block_dev.kfifo_buf))
        wake_up_interruptible(&block_dev.write_queue);
    
    PRT_RET(actual_readed);
}

static ssize_t
driver_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    int ret;
    int actual_write;

    PRT_CALLED();
    if (kfifo_is_full(&block_dev.kfifo_buf))
    {
        if (file->f_flags &O_NONBLOCK)
            PRT_RET(-EAGAIN);

        ret = wait_event_interruptible_timeout(block_dev.write_queue,
            !kfifo_is_full(&block_dev.kfifo_buf), msecs_to_jiffies(TIMEOUT));
        if (ret == 0)
            PRT_RET(-ETIMEDOUT);
    }

    ret = kfifo_from_user(&block_dev.kfifo_buf, buf, count, &actual_write);
    
    if (!kfifo_is_empty(&block_dev.kfifo_buf))
        wake_up_interruptible(&block_dev.read_queue);

    PRT_RET(actual_write);
}

static struct file_operations 
driver_fops = {
    .owner      =   THIS_MODULE,
    .read       =   driver_read,    
    .write      =   driver_write,
};

static int
__init driver_init(void) {
    int ret = 0;
    
    ret = kfifo_alloc(&block_dev.kfifo_buf, BUFFER_SIZE, GFP_KERNEL);
    if (ret != 0) {
        printk(KERN_ERR "KFIFO malloc failed, errcode = %d.\n", ret);
        return ret;
    }
    
    init_waitqueue_head(&block_dev.read_queue);
    init_waitqueue_head(&block_dev.write_queue);

    ret = register_chrdev(0, dev_name, &driver_fops);
    if (ret < 0) {
        printk(KERN_ERR "Register character device error, errorcode = %d.\n", ret);
        return ret;
    }
    block_dev.major_num = ret;
    printk(KERN_NOTICE "Register character device succeed, major number = %d.\n",
        block_dev.major_num);

    return 0;
}

static void
__exit driver_exit(void) {
    if (block_dev.major_num != 0) {
        unregister_chrdev(block_dev.major_num, dev_name);
    }
	kfifo_free(&block_dev.kfifo_buf);
    printk(KERN_INFO "Unregister character device succeed.\n");
}

module_init(driver_init);
module_exit(driver_exit);