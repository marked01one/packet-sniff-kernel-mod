#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>


#define DEVICE_NAME "adder"
#define CLASS_NAME "calc"

// Structure for pasing two numbers
struct numbers {
    char* expression;
    int exp_length;
    int result;
};

int ret;

static int precedence(char c) {
    switch (c) {
        case '^': return 3;
        case '/': return 2;
        case '*': return 2;
        case '+': return 1;
        case '-': return 1;
        default: return -1;
    }
}


static int infixToPostfix(char* exp, int len) {
    char result[len];
    char stack[len];
    int j = 0;
    int top = -1;

    for (int i = 0; i < len; i++) {
        char c = exp[i];

        // If the scanned character is 
        // an operand, add it to the output string.
        if (isalnum(c))
            result[j++] = c;

        // If the scanned character is
        // an ‘(‘, push it to the stack.
        else if (c == '(')
            stack[++top] = '(';

        // If the scanned character is an ‘)’,
        // pop and add to the output string from the stack 
        // until an ‘(‘ is encountered.
        else if (c == ')') {
            while (top != -1 && stack[top] != '(') {
                result[j++] = stack[top--];
            }
            top--; 
        }

        // If an operator is scanned
        else {
            while (top != -1 && (prec(c) < prec(stack[top]) ||
                                 prec(c) == prec(stack[top]))) {
                result[j++] = stack[top--];
            }
            stack[++top] = c;
        }
    }

    // Pop all the remaining elements from the stack
    while (top != -1) {
        result[j++] = stack[top--];
    }

    result[j] = '\0';
    printf("%s\n", result);
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

    nums.result = nums.num1 + nums.num2;

    ret = copy_to_user((struct numbers*) arg, &nums, sizeof(struct numbers));
    if (ret) return -EFAULT;
    
    printk(KERN_INFO "Addition performed: %d + %d = %d\n", nums.num1, nums.num2, nums.result);

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