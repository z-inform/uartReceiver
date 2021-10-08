#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "fddata.h"
#include "circ_buffer.h"

extern circ_buffer uart_read;

uint32_t crc_calc(char* src, uint16_t size);

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

            while( uart_read.cur_elem - uart_read.cur_read < data_size ){ //wait for all data in buffer
                read(fd, &rdata, 1);
                circ_write(rdata, &uart_read);
            }

            memcpy(data, uart_read.buf + uart_read.cur_read, data_size);
            uart_read.cur_read = (uart_read.cur_read + data_size) % CIRC_BUF_SIZE;

            uint32_t crc32 = * (uint32_t*) (uart_read.buf + uart_read.cur_read);

            if( crc32 != crc_calc(uart_read.buf, data_size) )
                crc32 = 1;
            else
                crc32 = 0;

            printf("RX(found prefix):\t");
            printf(" size: %d, type %d\n", data_size, data_type);
            if( crc32 ){
                printf("\e[31mCRC ERROR\e[0m\n");
            }
            else{
                printf("\e[30mCRC OK\e[0m\n");
                printf("%s\n", data);
            }


        }

    }
    return 0;
}


uint32_t crc_calc(char* src, uint16_t size){

    const unsigned char *buffer = (const unsigned char*) src;
    uint32_t crc = -1;

    while( size-- )
    {
        crc = crc ^ *buffer++;
        for( int bit = 0; bit < 8; bit++ )
        {
            if( crc & 1 ) crc = (crc >> 1) ^ 0x4C11DB7;
            else          crc = (crc >> 1);
        }
    }

    return ~crc;
}

