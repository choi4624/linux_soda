#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){
    int dev, i;
    char buf[100];
    printf("driver close!\n");
    dev= open("/dev/device_proj2", O_RDWR);
    if (dev<0)
    {
        printf("driver close failed!\n");
        return -1;

    }

    close(dev);
    return 0;
    
    
    
}