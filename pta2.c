#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "pta_ioctl.h"

#define NAME "pta2"
#define INITIAL_SIZE 256

static DEFINE_MUTEX(lock);
static char *buffer;
static size_t capacity = INITIAL_SIZE;
static size_t used;

static ssize_t device_read(struct file *file, char __user *dst, size_t count, loff_t *pos)
{
	ssize_t result;

	mutex_lock(&lock);
	result = simple_read_from_buffer(dst, count, pos, buffer, used);
	mutex_unlock(&lock);
	return result;
}

static ssize_t device_write(struct file *file, const char __user *src, size_t count, loff_t *pos)
{
	ssize_t result;

	mutex_lock(&lock);
	result = simple_write_to_buffer(buffer, capacity, pos, src, count);
	if (result > 0)
		used = *pos;
	else if (!result && count)
		result = -ENOSPC;
	mutex_unlock(&lock);
	return result;
}

static long device_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	__u32 size;
	char *resized;

	if (cmd != PTA_RESIZE)
		return -ENOTTY;
	if (copy_from_user(&size, (void __user *)arg, sizeof(size)))
		return -EFAULT;
	if (!size || size > PTA_MAX_SIZE)
		return -EINVAL;

	mutex_lock(&lock);
	resized = krealloc(buffer, size, GFP_KERNEL);
	if (!resized) {
		mutex_unlock(&lock);
		return -ENOMEM;
	}
	if (size > capacity)
		memset(resized + capacity, 0, size - capacity);
	buffer = resized;
	capacity = size;
	used = min(used, capacity);
	mutex_unlock(&lock);
	return 0;
}

static const struct file_operations ops = {
	.owner = THIS_MODULE,
	.open = nonseekable_open,
	.read = device_read,
	.write = device_write,
	.unlocked_ioctl = device_ioctl,
};

static struct miscdevice dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = NAME,
	.fops = &ops,
};

static int __init device_init(void)
{
	int err;

	buffer = kzalloc(capacity, GFP_KERNEL);
	if (!buffer)
		return -ENOMEM;

	err = misc_register(&dev);
	if (err) {
		kfree(buffer);
		return err;
	}
	pr_info(NAME ": registered with 256-byte buffer\n");
	return 0;
}

static void __exit device_exit(void)
{
	misc_deregister(&dev);
	kfree(buffer);
	pr_info(NAME ": removed\n");
}

module_init(device_init);
module_exit(device_exit);
MODULE_LICENSE("GPL");
