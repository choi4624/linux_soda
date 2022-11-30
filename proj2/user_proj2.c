#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int main(void){
    int dev, i;
    char buf[100];
    dev= open("/dev/device_proj2", O_RDWR);
    if (dev<0)
    {
        printf("driver open failed!\n");
        return -1;

    }
    for ( i = 0; i < 1000; i++)
    {
        read(dev,&buf,sizeof(buf));
        printf("%s\n", buf);
        sleep(1);
    }
    close(dev);
    return 0;
    
    
    
}