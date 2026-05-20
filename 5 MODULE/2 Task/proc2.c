#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DEVICE_NAME "emogotka_proc"
#define BUF_SIZE 20

static char *kernel_buffer;
static unsigned long data_len = 0;

static ssize_t lab_read(struct file *filp, char __user *buf, size_t count, loff_t *offp)
{
    if (*offp > 0 || data_len == 0) 
        return 0;

    if (copy_to_user(buf, kernel_buffer, data_len)) {
        return -EFAULT;
    }

    *offp = data_len;
    return data_len;
}

static ssize_t lab_write(struct file *filp, const char __user *buf, size_t count, loff_t *offp)
{
    data_len = (count > BUF_SIZE) ? BUF_SIZE : count;
S
    if (copy_from_user(kernel_buffer, buf, data_len)) {
        return -EFAULT;
    }

    printk(KERN_INFO "Received %lu bytes\n", data_len);
    return count;
}

static const struct proc_ops lab_proc_ops = {
    .proc_read = lab_read,
    .proc_write = lab_write,
};

static int __init lab_proc_init(void)
{
    kernel_buffer = kmalloc(BUF_SIZE, GFP_KERNEL);
    if (!kernel_buffer) {
        return -ENOMEM;
    }

    if (!proc_create(DEVICE_NAME, 0666, NULL, &lab_proc_ops)) {
        kfree(kernel_buffer);
        return -ENOMEM;
    }

    printk(KERN_INFO "Module loaded /proc/%s created\n", DEVICE_NAME);
    return 0;
}

static void __exit lab_proc_exit(void)
{
    remove_proc_entry(DEVICE_NAME, NULL);
    kfree(kernel_buffer);
    printk(KERN_INFO "Module unloaded\n");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("emogotka");
MODULE_DESCRIPTION("Task 2");

module_init(lab_proc_init);
module_exit(lab_proc_exit);
