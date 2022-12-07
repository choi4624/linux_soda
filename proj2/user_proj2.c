#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){
    int dev, i;
    char buf[100];
    printf("driver open!\n");
    dev= open("/dev/device_proj2", O_RDWR);
    if (dev<0)
    {
        printf("driver open failed!\n");
        return -1;

    }
    return 0;
    
    
    
}