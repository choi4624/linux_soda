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

static void cycle_led(void){
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
    timer_setup(&timer, led_cycle_timer,0);    
    timer.expires = jiffies + HZ * 2;
    printk(KERN_INFO"add_timer from module_init\n");
    add_timer(&timer);

        printk(KERN_INFO"led_module_init function_end!\n");
}

static void led_cycle_timer(struct timer_list *timer){
    char value =1, temp = 0;
    int ret, i;
    printk(KERN_INFO "kthread function !\n");
    for ( i = 0; i < 4; i++)
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
        ssleep(2);
    }
    
        
        
    timer->expires = jiffies + HZ * 2;
    printk(KERN_INFO"add_timer form timer_cb\n");
    add_timer(timer);
}

static void switch_led(void){
    int i, ret, val;
printk(KERN_INFO"switch_module_init!\n");
    
    while (1)
    {
        if(gpio_request(sw[3],"sw") == 1 )
        {  break;
        }

            for ( i = 0; i < 3; i++)
    {
        ret = gpio_request(sw[i], "sw");
        if (ret < 0)
        {
            printk(KERN_INFO "switch_module gpio_request failed!\n");
        }
        else
            ret = gpio_direction_input(sw[i]);
        
    }

        for (i = 0; i < 3; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (gpio_get_value(sw[i])    == 0)
        {
            ret = gpio_direction_output(led[i], LOW);
        }
    }


    for (i = 0; i < 3; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (ret < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
    }
    for (i = 0; i < 3; i++)
    {
        if (gpio_get_value(sw[i])    == 1)
        {
            ret = gpio_direction_output(led[i], HIGH);
        }
    }

    for ( i = 0; i < 3; i++)
    {
        val = gpio_get_value(sw[i]);
        printk(KERN_ALERT "sw %d value = %d\n", i, val);
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
        for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (gpio_get_value(sw[i])    == 0)
        {
            ret = gpio_direction_output(led[i], LOW);
            if (ret < 0)
        {
            printk(KERN_INFO "kthread timer stop gpio_Request failed\n");
        }
        }
    }
    printk(KERN_INFO"led free!\n");
    
    for ( i = 0; i < 4 ; i++)
    {
        gpio_free(sw[i]);
    }
}


irqreturn_t irq_handler(int irq, void *dev_id){
    printk(KERN_INFO "Debug %d\n", irq);
    
    switch (irq)
    {

    case 60:
        printk(KERN_INFO "sw1 interrupt ocurred!\n");    
        del_timer(&timer);
        timer_led();
        break;
    case 61:
        printk(KERN_INFO "sw2 interrupt ocurred!\n");
        del_timer(&timer);
        cycle_led();
        break;
    case 62:
        printk(KERN_INFO "sw3 interrupt ocurred!\n");      
        del_timer(&timer);
        break;
    case 63:
        printk(KERN_INFO "sw4 interrupt ocurred!\n");
        del_timer(&timer);
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
        gpio_free(sw[i]);
    }
    
}

module_init(switch_module_init);
module_exit(switch_module_exit);
MODULE_LICENSE("GPL");