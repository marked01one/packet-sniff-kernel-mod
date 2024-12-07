#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>


#define DEVICE_NAME "adder"
#define CLASS_NAME "calc"
#define STACK_SIZE 256

// Structure for pasing two numbers
struct numbers {
    int num1;
    int num2;
    char expr;
    int result;
};
int ret;

static int calculate(struct numbers* nums) {
    switch (nums->expr) {
        case '+': return nums->num1 + nums->num2;
        case '-': return nums->num1 - nums->num2;
        case '*': return nums->num1 * nums->num2;
        case '/': return nums->num1 / nums->num2;
        default: return 0;
    }
}

// IOCTL command
#define MAGIC_NUMBER 'k'
#define IOCTL_ADD _IOWR(MAGIC_NUMBER, 1, struct numbers)

static int major_number;
static struct class* adder_class = NULL;
static struct device* adder_device = NULL;

static int device_open(struct inode*, struct file*);
static int device_release(struct inode*, struct file*);
static long device_ioctl(struct file*, unsigned int, unsigned long);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = device_open,
    .release = device_release,
    .unlocked_ioctl = device_ioctl
};


static int device_open(struct inode* inodep, struct file* filep) {
    printk(KERN_INFO "Adder device opened\n");
    return 0;
}

static int device_release(struct inode* inodep, struct file* filep) {
    printk(KERN_INFO "Adder device closed\n");
    return 0;
}


static long device_ioctl(struct file* filep, unsigned int cmd, unsigned long arg) {
    struct numbers nums;

    if (cmd != IOCTL_ADD) return -EINVAL;

    ret = copy_from_user(&nums, (struct numbers*) arg, sizeof(struct numbers));
    if (ret) return -EFAULT;

    printk(KERN_INFO "Calculator operation detected\n");

    int result = calculate(&nums);
    nums.result = result;

    ret = copy_to_user((struct numbers*) arg, &nums, sizeof(struct numbers));
    if (ret) return -EFAULT;
    
    printk(KERN_INFO "Calculation: %d %c %d = %d\n", nums.num1, nums.expr, nums.num2, nums.result);

    return 0;         
}


static int __init adder_init(void) {
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_INFO "Failed to register major number\n");
        return major_number;
    }

    adder_class = class_create(CLASS_NAME);

    if (IS_ERR(adder_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(adder_class);
    }

    adder_device = device_create(adder_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(adder_device)) {
        class_destroy(adder_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        return PTR_ERR(adder_device);
    }

    printk(KERN_INFO "Adder module loaded\n");
    return 0;
}


static void __exit adder_exit(void) {
    device_destroy(adder_class, MKDEV(major_number, 0));
    class_destroy(adder_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "Adder module unloaded\n"); 
}



module_init(adder_init);
module_exit(adder_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("marked01one");