#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arya Rozhgar Abdullah");
MODULE_DESCRIPTION("A custom kernel-space bridge for application optimization.");
MODULE_VERSION("1.0");

#define DEVICE_NAME "app_bridge"
#define CLASS_NAME "app_class"
#define BUFFER_SIZE 1024
static int major_number;
static char kernel_buffer[BUFFER_SIZE];
static short size_of_message;
static struct class *app_bridge_class;
static struct device *app_bridge_device;
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);
static const struct file_operations fops = {
	.open = dev_open,
	.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};
static int __init app_bridge_init(void)
{
	pr_info("AppBridge: Initializing the AppBridge Kernel Module\n");
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_number < 0) {
		pr_alert("AppBridge failed to register a major number\n");
		return major_number;
	}
	app_bridge_class = class_create(CLASS_NAME);
	if (IS_ERR(app_bridge_class)) {
		unregister_chrdev(major_number, DEVICE_NAME);
		pr_alert("Failed to register device class\n");
		return PTR_ERR(app_bridge_class);
	}
	app_bridge_device = device_create(app_bridge_class, NULL,
					  MKDEV(major_number, 0), NULL,
					  DEVICE_NAME);
	if (IS_ERR(app_bridge_device)) {
		class_destroy(app_bridge_class);
		unregister_chrdev(major_number, DEVICE_NAME);
		pr_alert("Failed to create the device\n");
		return PTR_ERR(app_bridge_device);
	}

	pr_info("AppBridge: Device class interface successfully created\n");
	return 0;
}
static void __exit app_bridge_exit(void)
{
	device_destroy(app_bridge_class, MKDEV(major_number, 0));
	class_unregister(app_bridge_class);
	class_destroy(app_bridge_class);
	unregister_chrdev(major_number, DEVICE_NAME);
	pr_info("AppBridge: Subsystem cleanly unloaded from kernel core\n");
}
static int dev_open(struct inode *inodep, struct file *filep)
{
	pr_info("AppBridge: Application has connected to the kernel bridge.\n");
	return 0;
}
static ssize_t dev_read(struct file *file_ptr, char *user_buffer, size_t len, loff_t *offset)
{
	int error_count;

	if (*offset > 0 || size_of_message == 0)
		return 0;
	error_count = copy_to_user(user_buffer, kernel_buffer, size_of_message);

	if (error_count == 0) {
		pr_info("AppBridge: Sent %d characters to the application\n", size_of_message);
		*offset += size_of_message;
		return size_of_message;
	}

	pr_info("AppBridge: Failed to safely stream characters to application layer\n");
	return -EFAULT;
}
static ssize_t dev_write(struct file *file_ptr, const char *user_buffer, size_t len, loff_t *offset)
{
	size_t copy_len = (len > BUFFER_SIZE - 1) ? BUFFER_SIZE - 1 : len;
	if (copy_from_user(kernel_buffer, user_buffer, copy_len))
		return -EFAULT;

	kernel_buffer[copy_len] = '\0';
	size_of_message = copy_len;

	pr_info("AppBridge: Received %zu characters: %s\n", copy_len, kernel_buffer);
	return copy_len;
}
static int dev_release(struct inode *inodep, struct file *filep)
{
	pr_info("AppBridge: Application closed connection.\n");
	return 0;
}
module_init(app_bridge_init);
module_exit(app_bridge_exit);