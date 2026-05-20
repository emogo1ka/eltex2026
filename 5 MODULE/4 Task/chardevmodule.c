#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "emogotka"
#define CLASS_NAME "emogotka_chardev"
#define BUF_SIZE 1024

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Task 4 here we go");
MODULE_AUTHOR("emogo1ka");

static int major;
static struct cdev my_cdev;
static struct class *my_class = NULL;
static struct device *my_device = NULL;

static char kernel_buffer[BUF_SIZE];
static int data_size = 0;

static int dev_open(struct inode *inode, struct file *file) {
    return 0;
}

static int dev_release(struct inode *inode, struct file *file) {
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buffer, size_t len, loff_t *offset) {
    if (*offset >= data_size) return 0;
    if (len > data_size - *offset) len = data_size - *offset;

    if (copy_to_user(buffer, kernel_buffer + *offset, len)) {
        return -EFAULT;
    }

    *offset += len;
    return len;
}

static ssize_t dev_write(struct file *file, const char __user *buffer, size_t len, loff_t *offset) {
    if (len > BUF_SIZE - 1) len = BUF_SIZE - 1;

    memset(kernel_buffer, 0, BUF_SIZE);
    if (copy_from_user(kernel_buffer, buffer, len)) {
        return -EFAULT;
    }

    data_size = len;
    kernel_buffer[data_size] = '\0';
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
};

static int __init chardev_init(void) {
    dev_t dev;
    
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        return -1;
    }
    major = MAJOR(dev);

    cdev_init(&my_cdev, &fops);
    my_cdev.owner = THIS_MODULE;

    if (cdev_add(&my_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        return -1;
    }

    my_class = class_create(CLASS_NAME);
    if (IS_ERR(my_class)) {
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(my_class);
    }

    my_device = device_create(my_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(my_device)) {
        class_destroy(my_class);
        cdev_del(&my_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(my_device);
    }

    return 0;
}

static void __exit chardev_exit(void) {
    device_destroy(my_class, MKDEV(major, 0));
    class_destroy(my_class);
    cdev_del(&my_cdev);
    unregister_chrdev_region(MKDEV(major, 0), 1);
}

module_init(chardev_init);
module_exit(chardev_exit);
