#include "circ_buffer.h"
#include "string.h"

circ_buffer uart_read = {.buf = {0}, .cur_elem = 0, .cur_read = 0};

int circ_write(char val, circ_buffer* buffer){
    buffer->buf[buffer->cur_elem] = val;
    buffer->cur_elem = (buffer->cur_elem + 1) % CIRC_BUF_SIZE;
    return 0;
}

int circmp(char *str, circ_buffer* buffer){
    unsigned char offset = 0;
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
        offset = (offset + (unsigned char) status - (unsigned char) unwound_buf) % CIRC_BUF_SIZE;
        buffer->buf[offset] = '\n';
        buffer->cur_read = offset;
        return 0;
    }

}
