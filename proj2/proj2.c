#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define HIGH 1
#define LOW 0
#define DEV_MAJOR_NUMBER 223
#define DEV_NAME "device_proj2" 

int sw[4] = {4,17,27,22};
int led[4] = {23, 24, 25, 1};

irqreturn_t irq_handler(int irq, void *dev_id){
    printk(KERN_INFO "Debug %d\n", irq);
    switch (irq)
    {

    case 60:
        printk(KERN_INFO "sw1 interrupt ocurred!\n");
        timer_led_module_init();
        break;
    case 61:
        del_timer(&timer);
        printk(KERN_INFO "sw2 interrupt ocurred!\n");
        break;
    case 62:
        del_timer(&timer);
        printk(KERN_INFO "sw3 interrupt ocurred!\n");
        break;
    case 63:
        del_timer(&timer);
        printk(KERN_INFO "sw4 interrupt ocurred!\n");
        break;
    default:
        break;
    }
    return 0; 
}

static void timer_cb(struct timer_list *timer){
    int ret, i;
    printk(KERN_INFO "timer callback function !\n");
    if (flag == 0)
    {
        for ( i = 0; i < 4; i++)
        {
            ret = gpio_direction_output(led[i],HIGH);
        }
        flag =1;
        
    }
    else
    {
        for ( i = 0; i < 4; i++)
        {
            ret = gpio_direction_output(led[i], LOW);
        }
        flag = 0;
        
    }
    timer->expires = (jiffies + HZ * 1)*2;
    add_timer(timer);
    
}


static int timer_led_module_init(void){
    int i, ret;
printk(KERN_INFO"led_module_init!\n");
    for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (ret < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
    }
timer_setup(&timer, timer_cb,0);
timer.expires = (jiffies + HZ * 1)*2;
add_timer(&timer);
    return 0;
    

}

static int chr_write(struct file *file, const char * buf, size_t length, loff_t * ofs){
    int ret, i;
    unsigned char cbuf[4];
    printk(KERN_INFO"led_driver_write!\n");
    ret = copy_from_user(cbuf, buf, length);
    for ( i = 0; i < 4; i++)
    {
        gpio_direction_output(led[i],cbuf[i]);
    }
    
    return 0;
}

static int chr_read(struct file *file, char * buf, size_t length, loff_t * ofs){
    printk(KERN_INFO "driver_exam_read!\n");
    return 0;
}


static int chr_open(struct inode *inode, struct file *filep){
    int i, ret;
printk(KERN_INFO"led_module_init!\n");
    for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (ret < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
    }
         for ( i = 0; i < 4; i++)
     {
        res = gpio_request(sw[i], "sw");
        res = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler, IRQF_TRIGGER_RISING,"IRQ",(void *)(irq_handler));
        if(res<0)
            printk(KERN_INFO "request_irq failed!\n");
     }
    return 0;
}


static int chr_release(struct inode *inode, struct file *filep){
    int i;
    printk(KERN_INFO "led_driver_module_exit!\n");
    for ( i = 0; i < 4; i++)
    {
        gpio_free(led[i]);
    }
    return 0;
}

static struct file_operations chr_fops =
{
    .owner = THIS_MODULE,
    .write = chr_write,
    .read = chr_read,
    .open = chr_open, 
    .release = chr_release, // NOT USED
};

static int chr_init(void)
{
    printk(KERN_INFO "drvier_exam_init!\n");
    register_chrdev(DEV_MAJOR_NUMBER, DEV_NAME,&chr_fops);
    return 0;   
}

static void chr_exit(void)
{
    printk(KERN_INFO "drvier_exam_exit!\n");
    unregister_chrdev(DEV_MAJOR_NUMBER, DEV_NAME);
    
}

module_init(chr_init);
module_exit(chr_exit);

MODULE_LICENSE("GPL");