#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#include <linux/spinlock.h>
//#include <stdio.h>
//#include <sys/types.h>
//#include <sqlite3.h>


#define PROC_NAME "user_input"
#define BUF_SIZE 256


typedef struct __circular_buffer {
    char buffer[BUF_SIZE];
    int write_pos;
    int num_entries;
    spinlock_t lock;
} circle_buffer;

static struct proc_dir_entry *proc_file;
static struct timer_list *timer;

static ssize_t proc_write(struct file*, const char*, size_t, loff_t*);
static int proc_show(struct seq_file*, void*);
static int proc_open(struct inode*, struct file*);

// File operations structure
static struct proc_ops proc_file_ops = {
    .proc_write = proc_write,
    .proc_open = proc_open,
    .proc_read = seq_read,
    .proc_release = single_release
};

static ssize_t proc_write(struct file* file, const char *buffer, size_t count, loff_t *pos)
{
    char *tmp = kzalloc((count + 1), GFP_KERNEL);
    if (!tmp) return -ENOMEM;

    int ret = copy_from_user(tmp, buffer, count);
    if (ret != 0) {
        kfree(tmp);
        return -EFAULT;
    };




    kfree(tmp);
    return count;
}


static int proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, proc_show, NULL);  
}


static int proc_show(struct seq_file *seq_f, void *v)
{
    seq_printf(seq_f, "Hello from Kernel!\n");
    return 0;
}


static void cron_loop(struct timer_list *t)
{

}


static int __init user_input_init(void) 
{
    proc_file = proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);

    timer_setup(timer, cron_loop, 0);
    mod_timer(timer, jiffies + msecs_to_jiffies(1000));

    printk(KERN_INFO "/proc/%s is created. Write to it to see responses.\n", PROC_NAME);
    return 0;
}


static void __exit user_input_end(void)
{
    del_timer(timer);
    remove_proc_entry(PROC_NAME, NULL);
    
    printk(KERN_INFO "/proc/%s is removed.\n", PROC_NAME);
    return;
}




module_init(user_input_init);
module_exit(user_input_end);


MODULE_LICENSE("GPL");