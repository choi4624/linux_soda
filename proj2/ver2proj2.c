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
#include <linux/kthread.h>


#define HIGH 1
#define LOW 0

int sw[4] = {4,17,27,22};
int led[4] = {23, 24, 25, 1}, flag = 0;
static struct timer_list timer;
struct task_struct *thread_id = NULL;
int ledflag[3] = {0};
irqreturn_t irq_handler2(int irq, void *dev_id);
irqreturn_t irq_handler(int irq, void *dev_id);

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

static void timer_led(void){
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

}


static int led_cycle_timer(void *arg){
    char value =1, temp = 0;
    int ret, i;
    printk(KERN_INFO "kthread function !\n");
    while (!kthread_should_stop())
    {
        temp = value; 
        for ( i = 0; i < 4; i++)
        {
            if (temp&0x01)  
            {
                ret = gpio_direction_output(led[i], HIGH);
            }
            else
            {
                ret = gpio_direction_output(led[i],LOW);
            }
            temp = temp >> 1;
            
            
        }
        value = value << 1;
        if (value == 0x10)  
        {
            value = 0x01;
        }
        ssleep(1);
        
    }
    printk(KERN_INFO "kthread_should_stop call !\n");
    return 0;
}

static void switch_led(int oper){
    int i, ret, val;
        printk(KERN_INFO"led change!\n");
           if(ledflag[oper] == 1) 
           {
            ledflag[oper] = 0;
           }
           else{
            ledflag[oper] = 1;
           }
        
        for (i = 0; i < 3; i++)
        {
        if (ledflag[i] == 1)
        {
            ret = gpio_direction_output(led[oper], HIGH);
        }
        else{
             ret = gpio_direction_output(led[oper], LOW);
        }
        }
    }
    
    

static void kthread_timer_start(void){
int i, ret;
printk(KERN_INFO"led_kthread_init!\n");
    for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (ret < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
    }
if (thread_id == NULL)
{
    thread_id = (struct task_struct *)
        kthread_run(led_cycle_timer,NULL,"led_th read");
}
    
}

static void kthread_timer_stop(void){
    int i, ret;

    printk(KERN_INFO "kthread timer stop ocurred!\n");
    if (thread_id)  
    {
        kthread_stop(thread_id);
        thread_id = NULL;
        printk(KERN_INFO "kthread timer stop success!\n");
    }
}

static void switch_interrupt_exit(void){
    int res, i;
     printk(KERN_INFO "switch_interrupt_exit!\n"); 
     for ( i = 0; i < 4; i++)
     {
        free_irq(gpio_to_irq(sw[i]),(void *)(irq_handler2));
        gpio_free(sw[i]);
        res = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler, IRQF_TRIGGER_RISING,"IRQ",(void *)(irq_handler));
     }
       
}

static void switch_interrupt(void){
    int res, i;
     printk(KERN_INFO "switch_interrupt_init!\n"); 
     for ( i = 0; i < 4; i++)
     {

        res = gpio_request(led[i], "LED");
        if (res < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
        free_irq(gpio_to_irq(sw[i]),(void *)(irq_handler));
        res = gpio_request(sw[i], "sw");
        res = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler2, IRQF_TRIGGER_RISING,"IRQ",(void *)(irq_handler2));
        if(res<0)
            printk(KERN_INFO "request_irq failed!\n");
     }
     return 0;
       
}

irqreturn_t irq_handler2(int irq, void *dev_id){
    int i, ret, val;
    int condtion;
    printk(KERN_INFO "Debug %d\n", irq);
    switch (irq)
    {

    case 60:
        printk(KERN_INFO "sw1 interrupt ocurred!\n");
        condtion = 0;
        break;
    case 61:
        printk(KERN_INFO "sw2 interrupt ocurred!\n");
        condtion = 1;
        break;
    case 62:
        printk(KERN_INFO "sw3 interrupt ocurred!\n");
        condtion = 2;
        break;
    case 63:
        printk(KERN_INFO "sw4 interrupt ocurred!\n");
        switch_interrupt_exit();
        break;
    default:
        break;
    }
        printk(KERN_INFO"led change!\n");
        if(ledflag[condtion] == 1) 
        {
            ledflag[condtion] = 0;
        }
           else{
            ledflag[condtion] = 1;
           }
        
        for (i = 0; i < 3; i++)
        {
            if (i == condition)
            {
                gpio_direction_output(led[i], ledflag[condtion]);
            }
        }
    return 0; 
}

irqreturn_t irq_handler(int irq, void *dev_id){
    printk(KERN_INFO "Debug %d\n", irq);
    

    switch (irq)
    {

    case 60:
        printk(KERN_INFO "sw1 interrupt ocurred!\n");
        printk(KERN_INFO "timer delete!\n");    
        if(&timer){
        del_timer(&timer);
        printk(KERN_INFO "timer stopped\n");    
        }
        else{
        printk(KERN_INFO "no timer\n");    
        }    
        timer_led();
        break;
    case 61:
        printk(KERN_INFO "sw2 interrupt ocurred!\n");
        printk(KERN_INFO "timer delete!\n");    
        if(&timer){
        del_timer(&timer);
        printk(KERN_INFO "timer stopped\n");    
        }
        else{
        printk(KERN_INFO "no timer\n");    
        }    
        kthread_timer_start();
        break;
    case 62:
        printk(KERN_INFO "sw3 interrupt ocurred!\n");   
        switch_interrupt();
        break;
    case 63:
        printk(KERN_INFO "sw4 interrupt ocurred!\n");
        if(&timer){
        del_timer(&timer);
        printk(KERN_INFO "timer stopped\n");    
        }
        else{
        printk(KERN_INFO "no timer\n");    
        }    
        
        break;
    default:
        break;
    }
    return 0; 
}







static int switch_module_init(void){
    int val;

    int res, i;
     printk(KERN_INFO "switch_interrupt_init!\n"); 
     for ( i = 0; i < 4; i++)
     {
        res = gpio_request(sw[i], "sw");
        res = request_irq(gpio_to_irq(sw[i]), (irq_handler_t)irq_handler, IRQF_TRIGGER_RISING,"IRQ",(void *)(irq_handler));
        if(res<0)
            printk(KERN_INFO "request_irq failed!\n");
     }
     
    return 0;
}

static void switch_module_exit(void){
    int i, ret;

    for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (gpio_get_value(sw[i])    == 0)
        {
            ret = gpio_direction_output(led[i], LOW);
        }
    }
    printk(KERN_INFO"switch_module_exit!\n");
    
    for ( i = 0; i < 4 ; i++)
    {
        free_irq(gpio_to_irq(sw[i]),(void *)(irq_handler));
        gpio_free(sw[i]);
    }
    del_timer(&timer);
    
}

module_init(switch_module_init);
module_exit(switch_module_exit);
MODULE_LICENSE("GPL");