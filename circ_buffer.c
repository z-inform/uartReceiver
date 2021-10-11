#include "circ_buffer.h"
#include "string.h"
#include <stdint.h>

circ_buffer uart_read = {.buf = {0}, .cur_elem = 0, .cur_read = 0};

int circ_write(char val, circ_buffer* buffer){
    buffer->buf[buffer->cur_elem] = val;
    buffer->cur_elem = (buffer->cur_elem + 1) % CIRC_BUF_SIZE;
    return 0;
}

int circmp(char *str, circ_buffer* buffer){
    unsigned int offset = 0;
    char* status;
    char unwound_buf[CIRC_BUF_SIZE + 1];
    unwound_buf[CIRC_BUF_SIZE] = 0;

    //locate a character that is not in target substring
    for( ; offset < CIRC_BUF_SIZE; offset++ ){
        if( (buffer->buf[offset] == 0) || (strchr(str, buffer->buf[offset]) == NULL) ) break;
    }
    if( offset == CIRC_BUF_SIZE )
        offset = 0;

    //unwind a circular buffer into C string, splitting buffer on located element
    //translates all '\0' in buffer to '\n' in unwound buffer to make a singular string
    int unwound_i = 0;
    for(int i = offset; i < CIRC_BUF_SIZE; i++){
        if( buffer->buf[i] == 0 )
            unwound_buf[unwound_i] = '\n';
        else
            unwound_buf[unwound_i] = buffer->buf[i];
        unwound_i++;
    }
    for(int i = 0; i < offset; i++){
        if( buffer->buf[i] == 0 )
            unwound_buf[unwound_i] = '\n';
        else
            unwound_buf[unwound_i] = buffer->buf[i];
        unwound_i++;
    }

    status = strstr(unwound_buf, str);
    if( status == NULL )
        return 1;
    else{
        //corrupt the string in buffer to avoid finding at again
        offset = (offset + (unsigned int) status - (unsigned int) unwound_buf) % CIRC_BUF_SIZE;
        buffer->buf[offset] = '\n';
        buffer->cur_read = offset;
        return 0;
    }

}

int circ_unread(circ_buffer* buffer){
    int delta;
    if (buffer->cur_read > buffer->cur_elem)
        delta = CIRC_BUF_SIZE - buffer->cur_read + buffer->cur_elem;
    else    
        delta = buffer->cur_elem - buffer->cur_read;

    return delta;
}

int circ_read(void* dest, uint32_t size, circ_buffer* buffer){
    if (buffer->cur_read + size > CIRC_BUF_SIZE) {
        int delta = CIRC_BUF_SIZE - buffer->cur_read;
        memcpy(dest, buffer->buf + buffer->cur_read, delta);
        memcpy((char*) dest + delta, buffer->buf, size - delta);
    } else
        memcpy(dest, buffer->buf + buffer->cur_read, size);
    
    return 0;
}

