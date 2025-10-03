#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/timekeeping.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Ayush Sharma");
MODULE_DESCRIPTION("Simple kernel module with timing functionality");

//Global variables ot store the intial values
static unsigned long init_jiffies;
static struct timespec64 init_time;

//Module initialization function
static int __init hello_init(void){
	struct timespec64 current_time;
	unsigned long tick_time_ms;
	int hours, minutes, seconds;

	//Calculating tick time and printing kernel info message
	tick_time_ms = 1000/HZ;
	printk(KERN_INFO "Hello, tick time is %lu milliseconds\n", tick_time_ms);

	//Get current time
	ktime_get_real_ts64(&current_time);

	//Convert to hours, minutes and seconds
	long total_seconds = current_time.tv_sec%86400;
	hours = total_seconds/3600;
	minutes = (total_seconds%3600)/60;
	seconds = total_seconds%60;

	//Print current time in hh:mm:ss
	printk(KERN_INFO "Current time is: %02d:%02d:%02d\n", hours, minutes, seconds);

	//Saving the intial jiffies and time for later
	init_jiffies = jiffies;
	init_time = current_time;

	return 0;	
}

//Module cleanup function
static void hello_exit(void){
	struct timespec64 current_time;
	unsigned long elapsed_jiffies;
	unsigned long elapsed_ms;
	int hours, minutes, seconds;

	//Get current time
	ktime_get_real_ts64(&current_time);

	//Calculate elapsed time using jiffies
	elapsed_jiffies = jiffies - init_jiffies;
	elapsed_ms = jiffies_to_msecs(elapsed_jiffies);

	//Print kernel info message with elapsed time
	printk(KERN_INFO "Goodbye, module was loaded for %lu milliseconds\n", elapsed_ms);

	//Convert to hours, minutes and seconds
	long total_seconds = current_time.tv_sec%86400;
	hours = total_seconds/3600;
	minutes = (total_seconds%3600)/60;
	seconds = total_seconds%60;

	printk(KERN_INFO "Current time is: %02d:%02d:%02d\n", hours, minutes, seconds);
}

//Call init and exit functions
module_init(hello_init);
module_exit(hello_exit);
