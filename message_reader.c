//
// Created by Ziv on 06/05/2022.
//

#include "message_slot.h"
#include <errno.h>
#include <string.h>
#include <fcntl.h>      /* open */
#include <unistd.h>     /* exit */
#include <sys/ioctl.h>  /* ioctl */
#include <stdio.h>
#include <stdlib.h>


#ifndef ERROR_HANDLER
#define ERROR_HANDLER {perror("ERROR! "); exit(1);}
#endif

int main(int argc, char** argv) {
    int file_desc, id, ret_val=-1;
    char buffer[BUF_LEN];

    if(argc != 3) {perror("ERROR!, wrong argc number\n"); exit(1);}

    file_desc = open( argv[1], O_RDWR );
    if(file_desc < 0) {perror ("ERROR! ,Can't open device file:\n"); exit(1);}

    id = atoi(argv[2]);
    if(id == 0){ perror ("ERROR!, wrong channel id number\n"); exit(1);}

    if(0!= ioctl(file_desc, MSG_SLOT_CHANNEL , id)) {perror("ERROR! ioctl failed \n"); exit(1);}
    ret_val = read( file_desc, buffer, BUF_LEN);
    if(ret_val < 0) {perror("ERROR! read failed"); exit(1);}
    if (write(1, buffer, ret_val) != ret_val)  {perror("ERROR! write failed"); exit(1);}
    close(file_desc);

    exit(0);
}
