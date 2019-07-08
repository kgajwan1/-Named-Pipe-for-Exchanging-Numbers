/*
Course: Operating Systems
Assignment 3: Character Device
*/
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/semaphore.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/rtc.h>
#include <linux/sched.h>

/*defining author, license and description of this module*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("GAJJU");
MODULE_DESCRIPTION("NUMPIPE");

/*defining sempaphore for buffer*/
static struct semaphore full;
static struct semaphore empty;
static struct semaphore read_op_mutex;
static struct semaphore write_op_mutex;
/*misc device struct*/
static struct miscdevice my_device;
/*number of open devices*/
static int open_count;
/*buffer size to store the characters*/
static int buffer_size;

static int string_char_count = 10000;//string size too
static int read_index = 0, write_index = 0;
/*need at least 1 empty slot to READ/write*/
static int buffer_empty_slots;

/*getting buffer size from command line*/
module_param(buffer_size, int, 0000);

/*buffer to store strings*/
char** buffer;

/*declaring fops functions*/
static ssize_t my_write(struct file*, const char*, size_t, loff_t*);
static ssize_t my_read(struct file*, char*, size_t, loff_t*);
static int my_release(struct inode*, struct file*);
static int my_open(struct inode*, struct file*);

static struct file_operations my_device_fops = {
	.open = &my_open,
	.read = &my_read,
	.write = &my_write,
	.release = &my_release
};

int init_module(){
	/*initializing parameters*/
	
	my_device.minor = MISC_DYNAMIC_MINOR;
	my_device.fops = &my_device_fops;
	
	/*registering the device*/
	int register_return_value;
	if((register_return_value = misc_register(&my_device))){
		printk(KERN_ERR "Unable to register the device\n");
		return register_return_value;
	}
        else{
	printk(KERN_INFO "Device Registered!\n");
	printk(KERN_INFO "Details\n--------------\n");
	printk(KERN_INFO "Name: numpipe\n");
	printk(KERN_INFO "Queue size: %d\n", buffer_size);
	}
	/*allocating memory for the buffer. 2d array*/
	int _allocated = 0;
	buffer = (char**)kmalloc(buffer_size*sizeof(char*), GFP_KERNEL);
	while(_allocated < buffer_size){
		buffer[_allocated] = (char*)kmalloc((string_char_count+1)*sizeof(char), GFP_KERNEL);
		buffer[string_char_count] = '\0';
		++_allocated;
	}
	sema_init(&full, 0);
	sema_init(&empty, buffer_size);
	sema_init(&read_op_mutex, 1);
	sema_init(&write_op_mutex, 1);
	buffer_empty_slots = buffer_size;
	open_count = 0;
	return 0;
}

/*cleanup the module*/
void cleanup_module(){
	/*freeing memory*/
	int _iter;
	for(_iter = 0; _iter < buffer_size; _iter++){
		kfree(buffer[_iter]);
	}
	misc_deregister(&my_device);
	printk(KERN_INFO "Numpipe Unregistered!\n");
}

static int my_open(struct inode* _inode, struct file* _file){
	
	++open_count;
	return 0;
}

static int my_release(struct inode* _inode, struct file* _file){
	--open_count;
	return 0;
}

static ssize_t my_read(struct file* _file, char* user_buffer, size_t chars_to_be_read, loff_t* offset){
	int user_queue_index = 0;
	down_interruptible(&read_op_mutex);
	down_interruptible(&full);
	read_index %= buffer_size;
	for(user_queue_index = 0; user_queue_index < chars_to_be_read; user_queue_index++){
		if(buffer_empty_slots >= buffer_size){
			break;
		}
		copy_to_user(&user_buffer[user_queue_index], &buffer[read_index][user_queue_index], 1);
	}
	++read_index;
	++buffer_empty_slots;
	up(&empty);
	up(&read_op_mutex);
	return user_queue_index;
}
static ssize_t my_write(struct file* _file, const char* user_buffer, size_t number_of_chars_to_write, loff_t* offset){
	int user_queue_index = 0;
	int i = 0;
	
	down_interruptible(&write_op_mutex);
	down_interruptible(&empty);
	write_index %= buffer_size;
	for(user_queue_index = 0; user_queue_index < number_of_chars_to_write; user_queue_index++){
		if(buffer_empty_slots <= 0){
			break;
		}
		copy_from_user(&buffer[write_index][user_queue_index], &user_buffer[user_queue_index], 1);
	}
	++write_index;
	--buffer_empty_slots;
	up(&full);
	up(&write_op_mutex);
	return user_queue_index;
}
