//
// Created by Ziv on 06/05/2022.
//

#undef __KERNEL__
#define __KERNEL__
#undef MODULE
#define MODULE

#include <linux/slab.h> // check
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

MODULE_LICENSE("GPL");

#include "message_slot.h"
#define ALLOCATION_ERROR {printk("Failing allocation memory"); return -1;}

struct channel_node{
    long id;
    char message[BUF_LEN];
    int message_len;
    struct channel_node *next;
} channel_node;

struct channels_list {
    struct channel_node *head;
} channels_list ;

struct file_info{
    int minor;
    long id;
    struct channel_node *channel;
} file_info;

static struct channels_list channels_lists_array[MAX_MESSAGE_SLOTS_NUM];

// ----------- DEVICE FUNCTIONS -------------

// check - if the file already exist?
static int device_open(struct inode* inode, struct file* file) {
    struct file_info *file_info_p =  kmalloc(sizeof(file_info), GFP_KERNEL);
    if (file_info_p == NULL) ALLOCATION_ERROR

    file_info_p -> minor = iminor(inode);
    file_info_p -> id = 0;
    file -> private_data = (void*) file_info_p;

    return 0;
}

static int device_release( struct inode* inode, struct file*  file) {
    kfree(file -> private_data);
        return 0;
}


static ssize_t device_read(struct file* file, char __user* buffer, size_t length, loff_t* offset){
    char *message;
    int message_len, num;
    struct file_info *info = (struct file_info*) (file->private_data);

    if(info->id == 0)
        return -EINVAL;

    message = info -> channel -> message;
    message_len = info -> channel -> message_len;

    if(message == NULL || message_len==0)
        return -EWOULDBLOCK;

    if(length < message_len)
        return -ENOSPC;

    for (num = 0; num<message_len; num++)
        if (put_user(message[num], buffer + num) != 0)
            return -EINVAL;

    return num;
}


static ssize_t device_write(struct file* file, const char __user* buffer, size_t length, loff_t* offset){
    char message [BUF_LEN];
    int num;
    struct file_info *info = (struct file_info*) (file->private_data);

    if(info->id == 0)
        return -EINVAL;

    if(length > BUF_LEN ||  length <= 0)
        return -EMSGSIZE;

    for (num = 0; num<length; num++)
        if (get_user(message[num], buffer + num) != 0)
            return -EINVAL;

    info -> channel -> message_len = length;
    for(num = 0; num<length; num++)
        info -> channel -> message[num] = message[num];

    return num;

}


static long device_ioctl(struct file* file, unsigned int ioctl_command_id, unsigned long ioctl_param){

    struct file_info *file_info_p;
    struct channel_node *new_node, *current_node;

    if(MSG_SLOT_CHANNEL != ioctl_command_id || ioctl_param <= 0)
        return -EINVAL;

    file_info_p = (struct file_info*) file -> private_data;
    file_info_p -> id = ioctl_param;

    current_node = channels_lists_array[file_info_p->minor].head;

    if (current_node != NULL) {
        while (current_node -> next != NULL) {
            if (current_node->id == ioctl_param){
                file_info_p->channel = current_node;
                return 0;
            }
            current_node = current_node->next;
        }
        if(current_node->id == ioctl_param) {
            file_info_p->channel = current_node;
            return 0;
        }
    }

    new_node = kmalloc(sizeof(channel_node), GFP_KERNEL);
    if(new_node == NULL) ALLOCATION_ERROR
    new_node -> id = ioctl_param;
    new_node -> message_len = 0;
    new_node -> next = NULL;
    if(current_node == NULL)
        channels_lists_array[file_info_p->minor].head = new_node;
    else
        current_node -> next = new_node;
    file_info_p -> channel = new_node;

    return 0;
}

//==================== DEVICE SETUP =============================

struct file_operations Fops = {
        .owner = THIS_MODULE,
        .read = device_read,
        .write = device_write,
        .open = device_open,
        .unlocked_ioctl = device_ioctl,
        .release = device_release,
};


static int __init simple_init(void) {
    int i, rc = register_chrdev( MAJOR_NUM, DEVICE_RANGE_NAME, &Fops );
    if(rc < 0)
        return rc;

    for(i=0; i<MAX_MESSAGE_SLOTS_NUM; i++)
        channels_lists_array[i].head = NULL;

    return 0;
}


static void __exit simple_cleanup(void)
{
    struct channel_node *temp, *current_node;
    int i;

    for(i=0; i<MAX_MESSAGE_SLOTS_NUM; i++){
        current_node = channels_lists_array[i].head;
        while(current_node!= NULL){
            temp = current_node;
            current_node = temp -> next;
            kfree(temp);
        }
    }

    unregister_chrdev(MAJOR_NUM, DEVICE_RANGE_NAME);
}


module_init(simple_init);
module_exit(simple_cleanup);

