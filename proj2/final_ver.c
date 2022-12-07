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
int ledflag[3] = {0,0,0};
int condition =0;
irqreturn_t irq_handler(int irq, void *dev_id);
int base;

/* 코드 설명 
    irq 안에 동작하는 조건을 2개로 분기를 만들어서 진행 
    3번째 (sw2)를 누르면 condition이 바뀌면서 동작이 변화 
    변화된 동작에서는 led 스위치 조작 버튼이 되고 
    4번째 sw3을 누르면 다시 condition이 변경되도록 구조 제작 
    kthread 오류는 버그인가? 
*/
 
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
    for (i = 0; i < 4; i++)
    {
        ret = gpio_request(led[i], "LED");
        if (ret < 0)
        {
            printk(KERN_INFO "led_module gpio_Request failed\n");
        }
    }
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

static int kthread_timer_start(void){
int ret;
printk(KERN_INFO"led_kthread_init!\n");
        if (thread_id == NULL)
        {
            thread_id = (struct task_struct *)
                kthread_run(led_cycle_timer,NULL,"led_th read");
        }
    return 0;
}

static void kthread_timer_stop(void){
    int i;


    printk(KERN_INFO "kthread timer stop ocurred!\n");
    if (thread_id)  
    {
        kthread_stop(thread_id);
        thread_id = NULL;
        printk(KERN_INFO "kthread timer stop success!\n");
    }
        for ( i = 0; i < 4; i++)
    {

        gpio_free(led[i]);

    }
}

static void setled(int i){
    if (ledflag[i] == 0)
    {
            gpio_direction_output(led[i],HIGH);
            ledflag[i] = 1;
    }
    else
    {
            gpio_direction_output(led[i],LOW);
            ledflag[i] = 0;
    }
}

// irq 동작 정의
irqreturn_t irq_handler(int irq, void *dev_id){
    printk(KERN_INFO "Debug %d\n", irq);
    
    int i;

    switch (irq)
    {

    case 60:
        printk(KERN_INFO "sw1 interrupt ocurred!\n");
        
        if (condition>0)
        {
            setled(0);
            printk(KERN_INFO "%d condition!\n", condition); 
        }
        else if(condition)
        {
            printk(KERN_INFO "%d condition passed!\n", condition); 
            for ( i = 0; i < 4; i++)
            {
                gpio_free(led[i]);
            }
            printk(KERN_INFO "timer delete!\n");    
            del_timer(&timer);
            printk(KERN_INFO "timer stopped\n");  
            timer_led();
        }
        
        break;
    case 61:
        
        if (condition>0)
        {
            setled(1);
            printk(KERN_INFO "%d condition!\n", condition); 
        }
        else{
            printk(KERN_INFO "%d condition passed!\n", condition); 
                for ( i = 0; i < 4; i++)
            {
                gpio_free(led[i]);
            }
            printk(KERN_INFO "timer delete!\n");    
            del_timer(&timer);
            kthread_timer_start();
        }
        break;
    case 62:
        printk(KERN_INFO "sw3 interrupt ocurred!\n");   
        

        if (condition>0)
        {
            setled(2);
            printk(KERN_INFO "%d condition!\n", condition); 
        }
        else{
            printk(KERN_INFO "%d condition passed!\n", condition); 
            for ( i = 0; i < 4; i++)
                {
                    gpio_free(led[i]);
                }
            printk(KERN_INFO "timer delete!\n");    
            del_timer(&timer);
                for (i = 0; i < 4; i++)
                {
                    gpio_request(led[i], "LED");
                }
            condition = 1;

            printk(KERN_INFO "%d condition!\n", condition);    
        }
        break;
    case 63:
        printk(KERN_INFO "sw4 interrupt ocurred!\n");
        for ( i = 0; i < 4; i++)
        {
            gpio_free(led[i]);
        }
        del_timer(&timer);
        printk(KERN_INFO "timer stopped\n");       
        kthread_timer_stop();
        condition = 0;
        printk(KERN_INFO "%d condition!\n", condition); 
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
    kthread_timer_stop();
}

module_init(switch_module_init);
module_exit(switch_module_exit);
MODULE_LICENSE("GPL");