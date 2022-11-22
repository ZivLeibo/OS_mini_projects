//
// Created by Ziv on 06/05/2022.
//

#ifndef MESAGE_SLOT_H
#define MESAGE_SLOT_H


#include <linux/ioctl.h>

#define MAJOR_NUM 235
#define MAX_MESSAGE_SLOTS_NUM 256
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long)

#define DEVICE_RANGE_NAME "char_dev"
#define BUF_LEN 128
#define DEVICE_FILE_NAME "message_slot"

#endif
