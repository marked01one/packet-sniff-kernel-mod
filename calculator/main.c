#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define STACK_SIZE 256

struct numbers {
    int num1;
    int num2;
    char expr;
    int result;
};

#define MAGIC_NUM 'k'
#define IOCTL_ADD _IOWR(MAGIC_NUM, 1, struct numbers)


int main(int argc, char* argv[]) {
    int fd;
    struct numbers nums;
    

    if (argc != 4) {
        printf("Usage: %s [num1] [expression] [num2]\n", argv[0]);
        return 1;
    }

    fd = open("/dev/adder", O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 2;
    }

    nums.num1 = atoi(argv[1]);
    nums.expr = argv[2][0];
    nums.num2 = atoi(argv[3]);


    if (ioctl(fd, IOCTL_ADD, &nums) < 0) {
        perror("IOCTL failed");
        close(fd);
        return 3;
    }

    printf("Result: %d %c %d = %d\n", nums.num1, nums.expr, nums.num2, nums.result);
    return 0;
}