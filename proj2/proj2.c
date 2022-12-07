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
int led[4] = {23, 24, 25, 1}, flag = 0;
static struct timer_list timer;

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
    timer->expires = jiffies + HZ * 2;
    printk(KERN_INFO"add_timer form timer_cb\n");
    add_timer(timer);
    
}

static int timer_led_module_init(void){
    int i, ret;
    printk(KERN_INFO"led_module_init -timer!\n");
    for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (ret < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
    }
timer_setup(&timer, timer_cb,0);
timer.expires = jiffies + HZ * 2;
printk(KERN_INFO"add_timer from module_init\n");
add_timer(&timer);

printk(KERN_INFO"led_module_init function_end!\n");
    return 0;
    

}

static void gpio_setting(){

    printk(KERN_INFO "irq_handler2_IRQ!\n");
    switch (irq)
    {

    case 60:
        printk(KERN_INFO "sw1 interrupt ocurred!\n");
        if (gpio_get_value(led[0])    == 1)
        {
            gpio_direction_output(led[0], HIGH); //low
        }
        else if (gpio_get_value(led[0])    == 0)
        {
            gpio_direction_output(led[0], HIGH);
        }
        break;
    case 61:
        printk(KERN_INFO "sw2 interrupt ocurred!\n");
        if (gpio_get_value(led[1])    == 1)
        {
           gpio_direction_output(led[1], HIGH);
        }
        else if (gpio_get_value(led[1])    == 0)
        {
            gpio_direction_output(led[1], HIGH);
        }
        break;
    case 62:
        printk(KERN_INFO "sw3 interrupt ocurred!\n");
        if (gpio_get_value(led[2])    == 1)
        {
            gpio_direction_output(led[2], HIGH);
        }
        else if (gpio_get_value(led[2])    == 0)
        {
            gpio_direction_output(led[2], HIGH);
        }
        break;
    case 63:
        printk(KERN_INFO "sw4 interrupt ocurred!\n");
        break;
    default:
        break;
    }
    return 0; 
}

static int switch_interrupt_init(void){
    int res, i;
    printk(KERN_INFO "FREE with before IRQ and request gpio!\n"); 
    for ( i = 0; i < 4; i++)
     {
        free_irq(gpio_to_irq(sw[i]),(void *)(irq_handler));
        gpio_free(sw[i]);
     }
     printk(KERN_INFO "switch_interrupt_init!\n"); 
     for ( i = 0; i < 4; i++)
     {
        res = gpio_request(sw[i], "sw");
        
        if(res<0)
            printk(KERN_INFO "SW3 request_irq failed!\n");
     }
     return 0;
       
}

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
        switch_interrupt_init();
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


static void freegpio(void){
     int i;
     printk(KERN_INFO "free_gpio!\n");
    for ( i = 0; i < 4; i++)
    {
        gpio_free(led[i]);
    }
}



static int chr_write(struct file *file, const char * buf, size_t length, loff_t * ofs){
    int ret, i;
    unsigned char cbuf[4];
    printk(KERN_INFO"led_driver_write!\n");
    // ret = copy_to_user(cbuf, buf, length);
    // for ( i = 0; i < 4; i++)
    // {
    //     gpio_direction_output(led[i],cbuf[i]);
    // }
    
    return 0;
}

static int chr_read(struct file *file, char * buf, size_t length, loff_t * ofs){
    printk(KERN_INFO "driver_exam_read!\n");
    return 0;
}


static int chr_open(struct inode *inode, struct file *filep){
    int i, ret;
printk(KERN_INFO"device_open!\n");
    
         for ( i = 0; i < 4; i++)
     {
        ret = gpio_request(sw[i], "sw");
        ret = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler, IRQF_TRIGGER_RISING,"IRQ",(void *)(irq_handler));
        if(ret<0)
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
    int i;
    printk(KERN_INFO "drvier_exam_exit!\n");
    
    del_timer(&timer);
    for ( i = 0; i < 4; i++)
     {
        free_irq(gpio_to_irq(sw[i]),(void *)(irq_handler));
        free_irq(gpio_to_irq(sw[i]),(void *)(irq_handler2));
        gpio_free(sw[i]);
        gpio_free(led[i]);
     }
    unregister_chrdev(DEV_MAJOR_NUMBER, DEV_NAME);
}

module_init(chr_init);
module_exit(chr_exit);

MODULE_LICENSE("GPL");