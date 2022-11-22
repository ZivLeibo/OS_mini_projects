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

int main(int argc, char** argv) {
    int file_desc, id, message_len;
    if(argc != 4) {perror("ERROR!, wrong argc number\n"); exit(1);}

    file_desc = open( argv[1], O_RDWR );
    if(file_desc < 0) {perror ("ERROR! ,Can't open device file:\n"); exit(1);}

    id = atoi(argv[2]);
    if(id == 0){ perror ("ERROR!, wrong channel id number\n"); exit(1);}

    message_len = strlen(argv[3]);
    if(argv[3] == NULL || message_len > BUF_LEN){ perror ("ERROR! ,wrong message:\n"); exit(1);}

    if(0!= ioctl(file_desc, MSG_SLOT_CHANNEL , id)) {perror("ERROR! ioctl failed"); exit(1);}
    if(message_len != write( file_desc, argv[3], message_len)) {perror("ERROR! write failed"); exit(1);}

    close(file_desc);

    exit(0);
}
