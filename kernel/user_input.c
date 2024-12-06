#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
#include <linux/ioctl.h>
#include <linux/debugfs.h>


#define PROC_NAME "user_input"
#define BUF_SIZE 8192
#define MAX_ENTRIES 100
#define LINE_LENGTH 128
#define TIMER_DELAY 1000

#define BASE 'k'
#define IOCTL_READ _IOR(BASE, 1, char*)
#define IOCTL_WRITE _IOW(BASE, 2, int)
#define IOCTL_CLEAR _IO(BASE, 3) 


static int driver_ioctl(struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg);


static int driver_ioctl(struct inode *inode, struct file *fp, unsigned int cmd, unsigned long arg)
{
    if (_IOC_TYPE(cmd) != BASE) return -EINVAL;
}


module_init(driver_ioctl);
module_exit(user_input_end);


MODULE_LICENSE("GPL");