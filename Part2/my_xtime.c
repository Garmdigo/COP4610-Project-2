#include <linux/init.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <stdbool.h>
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Getting kernel's time");

#define ENTRY_NAME "timed"
#define ENTRY_SIZE 400
#define PERMS 0644
#define PARENT NULL
static struct file_operations fops;
static char *message;
static int read_p;
struct timespec time_spec1;
struct timespec time_spec2;
static bool counter = false;
struct timespec elapse;

int hello_proc_open(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_INFO "proc called open\n");
	 read_p = 1;
	message = kmalloc(sizeof(char) * ENTRY_SIZE, __GFP_RECLAIM | __GFP_IO | __GFP_FS);
	if (message == NULL) {
		printk(KERN_WARNING "timed_proc_open");
		return -ENOMEM;
	}

	time_spec1 = current_kernel_time();
	if (counter == false)
	{
		sprintf(message, "current time: %ld.%ld\n", time_spec1.tv_sec, time_spec1.tv_nsec);
		counter=true;
    }
	else
	{
		elapse.tv_sec = time_spec1.tv_sec - time_spec2.tv_sec;
		elapse.tv_nsec = time_spec1.tv_nsec - time_spec2.tv_nsec;
		if(elapse.tv_nsec < 0)
		{
			elapse.tv_sec = elapse.tv_sec - 1;
			elapse.tv_nsec = elapse.tv_nsec*-1;
        }
		sprintf(message, "current time: %ld.%ld\nelapsed time: %ld.%ld\n", time_spec1.tv_sec, time_spec1.tv_nsec, elapse.tv_sec, elapse.tv_nsec);
	}
	time_spec2 = time_spec1;
	return 0;
}

ssize_t hello_proc_read(struct file *sp_file, char __user *buf, size_t size, loff_t *offset) {
	int len = strlen(message);
	read_p = !read_p;
	if (read_p)
		return 0;
	printk(KERN_INFO "proc called read\n");
	copy_to_user(buf, message, len);
	return len;
}

int hello_proc_release(struct inode *sp_inode, struct file *sp_file) {
	printk(KERN_NOTICE "proc called release\n");
	kfree(message);
	return 0;
}

static int hello_init(void) {
	printk(KERN_NOTICE "/proc/%s create\n",ENTRY_NAME);
	fops.open = hello_proc_open;
	fops.read = hello_proc_read;
	fops.release = hello_proc_release;

	if (!proc_create(ENTRY_NAME, PERMS, NULL, &fops)) {
		printk(KERN_WARNING "proc create\n");
		remove_proc_entry(ENTRY_NAME, NULL);
		return -ENOMEM;
	}

	return 0;
}
module_init(hello_init);

static void hello_exit(void)
{
	remove_proc_entry(ENTRY_NAME, NULL);
	printk(KERN_NOTICE "Removing /proc/%s\n", ENTRY_NAME);
}
module_exit(hello_exit);
