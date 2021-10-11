#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fddata.h"
#include "circ_buffer.h"
#include "CRC32.h"

extern circ_buffer uart_read;

int main(int argc, char* argv[]){

    if( argc != 2 ){
        printf("Incorrect number of arguments\n");
        return -1;
    }

    int fd = open(argv[1], O_RDONLY);
    if( fd == -1 ){
        perror("Open error");
        return -1;
    }

    while(1){
        char rdata;
        char prefix[3] = {0x3B, 0x3D, 0};
        read(fd, &rdata, 1);
        circ_write(rdata, &uart_read);


        /*
        for(int i = 0; i < uart_read.cur_elem; i++){
            printf("%x", uart_read.buf[i]);
        }
        printf("\n");
        */

        if( circmp(prefix, &uart_read) == 0 ){ //none of the while(...) further will work when buffer loops. To be fixed

            uart_read.cur_read = (uart_read.cur_read + 2) % CIRC_BUF_SIZE;//skip parcel prefix

            while( uart_read.cur_elem - uart_read.cur_read < 3 ){ //wait for data size and type in buffer
                read(fd, &rdata, 1);
                circ_write(rdata, &uart_read);
            }
            

            uint16_t data_size = * (uint16_t*) (uart_read.buf + uart_read.cur_read);
            uart_read.cur_read = (uart_read.cur_read + 2) % CIRC_BUF_SIZE;
            char data_type = uart_read.buf[uart_read.cur_read];
            char* data = (char*) malloc(data_size);
            uart_read.cur_read = (uart_read.cur_read + 1) % CIRC_BUF_SIZE;

            while( uart_read.cur_elem - uart_read.cur_read < data_size ){ //wait for all data in buffer
                read(fd, &rdata, 1);
                circ_write(rdata, &uart_read);
            }

            memcpy(data, uart_read.buf + uart_read.cur_read, data_size);
            uart_read.cur_read = (uart_read.cur_read + data_size) % CIRC_BUF_SIZE;

            while( uart_read.cur_elem - uart_read.cur_read < 4 ){ //wait for crc in buffer
                read(fd, &rdata, 1);
                circ_write(rdata, &uart_read);
            }

            uint32_t crc32; 
            memcpy(&crc32, uart_read.buf + uart_read.cur_read, 4);


            char crc_flag;
            uint32_t crc32_data = 0;
            crc32_data = doCRC32(data, data_size);
            if( crc32 != crc32_data )
                crc_flag = 1;
            else
                crc_flag = 0;

            printf("RX(found prefix): ");
            printf(" size: %d, type %d\n", data_size, data_type);
            printf("Data: %s\n", data);
            if( crc_flag ){
                printf("\e[31mCRC ERROR\e[0m\n");
                printf("Received %x; Expected %x\n", crc32, crc32_data);
            }
            else{
                printf("\e[32mCRC OK\e[0m\n");
            }

            printf("\n");

        }

    }
    return 0;
}
