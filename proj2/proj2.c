#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>

#define HIGH 1
#define LOW 0
#define DEV_MAJOR_NUMBER 223
#define DEV_NAME "device_proj2" 

int sw[4] = {4,17,27,22};
int led[4] = {23, 24, 25, 1};