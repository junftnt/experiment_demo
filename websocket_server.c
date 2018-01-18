/*1、握手*/
/*2、即时响应一条*/
/*3、定期反馈消息*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define MAXRECVBUF_LEN  512
#define MAXSENDBUF_LEN  512
#define MAXSHAKE_LEN    30
#define MAXLISTEN_NUM   20

typedef struct sha1_context{  
    unsigned message_digest[5];        
    unsigned length_low;               
    unsigned length_high;              
    uint8_t message_block[64];   
    int message_block_index;           
    int computed;                      
    int corrupted;                     
} sha1_context;  

#define sha1_circular_shift(bits,word) ((((word) << (bits)) & 0xFFFFFFFF) | ((word) >> (32-(bits))))  

/*base64编/解码用的基础字符集*/
const char base64char[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/**
 * ascii编码为base64格式
 *
 * @param const uint8_t bindata     ascii字符串输入
 * @param char          base64      base64字符串输出
 * @param int           binlength   bindata的长度
 *
 * @return int          base64字符串长度
 *
 */
static int base64_encode( const uint8_t *bindata, char *base64, int binlength)
{
    int i, j;
    uint8_t current;
    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (uint8_t)0x3F;
        base64[j++] = base64char[(int)current];
        current = ( (uint8_t)(bindata[i] << 4 ) ) & ( (uint8_t)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (uint8_t)(bindata[i+1] >> 4) ) & ( (uint8_t) 0x0F );
        base64[j++] = base64char[(int)current];
        current = ( (uint8_t)(bindata[i+1] << 2) ) & ( (uint8_t)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (uint8_t)(bindata[i+2] >> 6) ) & ( (uint8_t) 0x03 );
        base64[j++] = base64char[(int)current];
        current = ( (uint8_t)bindata[i+2] ) & ( (uint8_t)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return j;
}

/**
 * base64格式解码为ascii
 *
 * @param const char    *base64     base64字符串输入
 * @param uint8_t       *bindata    ascii字符串输出
 * 
 * @return int          解码出来的ascii字符串长度
 *
 */
static int base64_decode( const char *base64, uint8_t *bindata)
{
    int i, j;
    uint8_t k;
    uint8_t temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }
        bindata[j++] = ((uint8_t)(((uint8_t)(temp[0] << 2))&0xFC)) | \
                       ((uint8_t)((uint8_t)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;
        bindata[j++] = ((uint8_t)(((uint8_t)(temp[1] << 4))&0xF0)) | \
                       ((uint8_t)((uint8_t)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;
        bindata[j++] = ((uint8_t)(((uint8_t)(temp[2] << 6))&0xF0)) | \
                       ((uint8_t)(temp[3]&0x3F));
    }
    return j;
}

static void sha1_process_message_block(sha1_context *context)
{
    const unsigned k[] = {0x5A827999, 0x6ED9EBA1, 0x8F1BBCDC, 0xCA62C1D6 };
    int         t;
    unsigned    temp;
    unsigned    w[80];
    unsigned    a, b, c, d, e;

    for(t = 0; t < 16; t++) {
        w[t] = ((unsigned) context->message_block[t * 4]) << 24;
        w[t] |= ((unsigned) context->message_block[t * 4 + 1]) << 16;
        w[t] |= ((unsigned) context->message_block[t * 4 + 2]) << 8;
        w[t] |= ((unsigned) context->message_block[t * 4 + 3]);
    }

    for(t = 16; t < 80; t++)
        w[t] = sha1_circular_shift(1, w[t-3] ^ w[t-8] ^ w[t-14] ^ w[t-16]);

    a = context->message_digest[0];
    b = context->message_digest[1];
    c = context->message_digest[2];
    d = context->message_digest[3];
    e = context->message_digest[4];
    
    for(t = 0; t < 20; t++) {
        temp =  sha1_circular_shift(5, a) + ((b & c) | ((~b) & d)) + e + w[t] + k[0];
        temp &= 0xFFFFFFFF;
        e = d;
        d = c;
        c = sha1_circular_shift(30, b);
        b = a;
        a = temp;  
    }
    
    for(t = 20; t < 40; t++) {
        temp = sha1_circular_shift(5, a) + (b ^ c ^ d) + e + w[t] + k[1];
        temp &= 0xFFFFFFFF;
        e = d;
        d = c;
        c = sha1_circular_shift(30, b);
        b = a;
        a = temp;
    }
    
    for(t = 40; t < 60; t++) {
        temp = sha1_circular_shift(5, a) + ((b & c) | (b & d) | (c & d)) + e + w[t] + k[2];
        temp &= 0xFFFFFFFF;
        e = d;
        d = c;
        c = sha1_circular_shift(30, b);
        b = a;
        a = temp;
    }
    
    for(t = 60; t < 80; t++) {  
        temp = sha1_circular_shift(5, a) + (b ^ c ^ d) + e + w[t] + k[3];
        temp &= 0xFFFFFFFF;
        e = d;
        d = c;
        c = sha1_circular_shift(30, b);
        b = a;
        a = temp;
    }
    context->message_digest[0] = (context->message_digest[0] + a) & 0xFFFFFFFF;
    context->message_digest[1] = (context->message_digest[1] + b) & 0xFFFFFFFF;
    context->message_digest[2] = (context->message_digest[2] + c) & 0xFFFFFFFF;
    context->message_digest[3] = (context->message_digest[3] + d) & 0xFFFFFFFF;
    context->message_digest[4] = (context->message_digest[4] + e) & 0xFFFFFFFF;
    context->message_block_index = 0;
}

static void sha1_reset(sha1_context *context)
{
    context->length_low             = 0;
    context->length_high            = 0;
    context->message_block_index    = 0;

    context->message_digest[0]      = 0x67452301;
    context->message_digest[1]      = 0xEFCDAB89;
    context->message_digest[2]      = 0x98BADCFE;
    context->message_digest[3]      = 0x10325476;
    context->message_digest[4]      = 0xC3D2E1F0;

    context->computed   = 0;
    context->corrupted  = 0;
}  

static void sha1_pad_message(sha1_context *context)
{
    if (context->message_block_index > 55) {
        context->message_block[context->message_block_index++] = 0x80;
        while(context->message_block_index < 64)  context->message_block[context->message_block_index++] = 0;
        sha1_process_message_block(context);
        while(context->message_block_index < 56) context->message_block[context->message_block_index++] = 0;
    } else {
        context->message_block[context->message_block_index++] = 0x80;
        while(context->message_block_index < 56) context->message_block[context->message_block_index++] = 0;
    }
    context->message_block[56] = (context->length_high >> 24 ) & 0xFF;
    context->message_block[57] = (context->length_high >> 16 ) & 0xFF;
    context->message_block[58] = (context->length_high >> 8 ) & 0xFF;
    context->message_block[59] = (context->length_high) & 0xFF;
    context->message_block[60] = (context->length_low >> 24 ) & 0xFF;
    context->message_block[61] = (context->length_low >> 16 ) & 0xFF;
    context->message_block[62] = (context->length_low >> 8 ) & 0xFF;
    context->message_block[63] = (context->length_low) & 0xFF;

    sha1_process_message_block(context);
} 

static int sha1_result(sha1_context *context)
{
    if (context->corrupted) {
        return 0;
    }
    
    if (!context->computed) {
        sha1_pad_message(context);
        context->computed = 1;
    }
    return 1;
}

static void sha1_input(sha1_context *context, const char *message_array, unsigned length)
{
    if (!length)
        return;
    
    if (context->computed || context->corrupted) {
        context->corrupted = 1;
        return;
    }
    
    while(length-- && !context->corrupted) {
        context->message_block[context->message_block_index++] = (*message_array & 0xFF);

        context->length_low += 8;
        context->length_low &= 0xFFFFFFFF;

        if (context->length_low == 0) {
            context->length_high++;
            context->length_high &= 0xFFFFFFFF;
            if (context->length_high == 0) context->corrupted = 1;
        }
        
        if (context->message_block_index == 64) {
            sha1_process_message_block(context);
        }
        message_array++;
    }
}

static char * sha1_hash(const char *source)
{
    sha1_context sha;
    char *buf;
    
    sha1_reset(&sha);
    sha1_input(&sha, source, strlen(source));
    
    if (!sha1_result(&sha)) {
        printf("SHA1 ERROR: Could not compute message digest");
        return NULL;
    } else {
        buf = (char *)malloc(128);
        memset(buf, 0, 128);
        sprintf(buf, "%08X%08X%08X%08X%08X", sha.message_digest[0], sha.message_digest[1],
                sha.message_digest[2],sha.message_digest[3], sha.message_digest[4]);
        return buf;
    }
}
static int htoi(const char s[], int start, int len)
{
    int i, j;
    int n = 0;

    /*判断是否有前导0x或者0X*/
    if (s[0] == '0' && (s[1]=='x' || s[1]=='X')) {
        i = 2;
    } else {
        i = 0;
    }
    
    i += start;
    j = 0;
    for (; (s[i] >= '0' && s[i] <= '9') 
            || (s[i] >= 'a' && s[i] <= 'f') || (s[i] >='A' && s[i] <= 'F');++i) {
        if(j>=len) {
            break;
        }
        
        if (tolower(s[i]) > '9') {
            n = 16 * n + (10 + tolower(s[i]) - 'a');
        } else {
            n = 16 * n + (tolower(s[i]) - '0');
        }
        j++;
    }
    return n;
}
static int build_respone_header(char *shake_key, char *send_buf)
{
    const char respone_header[] = "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Connection: Upgrade\r\n"
        "Server: websocket_server\r\n"
        "Sec-WebSocket-Accept: %s \r\n\r\n";
    const char guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"; 
    strcat(shake_key, guid);

    char *sha1_data_temp;
    char *sha1_data;
    int n, i;
    sha1_data_temp = sha1_hash(shake_key);
    n = strlen(sha1_data_temp);
    sha1_data = (char *)calloc(1, n / 2 + 1);
    memset(sha1_data, 0, n / 2 + 1);

    for(i = 0; i < n; i += 2) {
        sha1_data[ i / 2 ] = htoi(sha1_data_temp, i, 2);
    }

    n = base64_encode((const uint8_t *)sha1_data, (char *)shake_key, (n / 2));

    sprintf(send_buf, respone_header, shake_key);

    return n;
}

static int denpackage(char *recv_buf, char *recv_text)
{
    int recv_buf_len = strlen(recv_buf);
    if (recv_buf_len < 2) {
        return -1;
    }
    /*第一位fin*/
    int fin = (recv_buf[0] & 0x80) == 0x80;
    if (!fin) {
        return -1;
    }
    /*是否包含掩码*/
    int mask = (recv_buf[1] & 0x80) == 0x80;
    char mask_key[4];
    memset(mask_key, 0, 4);
    if (!mask) {
        return -1;
    }
    /*数据长度*/
    unsigned long n, payload_len = recv_buf[1] & 0x7F;
    char *payload_data;
    char temp[8] = {"\0"};
    int i;
    if (payload_len == 126) {

        memcpy(mask_key, recv_buf + 4, 4);
        payload_len = (recv_buf[2] & 0xFF) << 8 | (recv_buf[3] & 0xFF);
        payload_data = (char *)malloc(payload_len);
        memset(payload_data, 0, payload_len);
        memcpy(payload_data, recv_buf + 8, payload_len);

    } else if (payload_len == 127) {
        
        memcpy(mask_key, recv_buf + 10, 4);
        for (i = 0; i < 8; i ++) {
            temp[i] = recv_buf[9 - i];
        }
        memcpy(&n, temp, 8);
        payload_data = (char *)malloc(n);
        memset(payload_data, 0, n);
        memcpy(payload_data, recv_buf + 14, n);
        payload_len = n;
    
    } else {
    
        memcpy(mask_key, recv_buf + 2, 4);
        payload_data = (char *)malloc(payload_len);
        memset(payload_data, 0, payload_len);
        memcpy(payload_data, recv_buf + 6, payload_len);
    }

    for (i = 0; i < payload_len; i ++) {
        recv_text[i] = (char) (payload_data[i] ^ mask_key[i % 4]);
    }
    printf("recv_text: %s \n\n", recv_text);

    return 0;
}

static int enpackage(char *recv_text, char *send_buf)
{
    int len = strlen(recv_text);

    /*我们这里以text为例子*/
    *send_buf++ = 0x81;

    if (len < 126) {

        *send_buf++ = len;
        memcpy(send_buf, recv_text, len);

    } else if (len < 0xFFFF) {
        
        *send_buf++ = 126;
        *send_buf++ = (len >> 8 & 0xFF);
        *send_buf++ = (len & 0xFF);
        memcpy(send_buf, recv_text, len);

    } else {
    
        return -1;
    
    }

    return 0;

}

static int build_send_buf(char *recv_buf, char *send_buf)
{
    char *p;
    if ((p = strstr((char *)recv_buf, (const char *)"Sec-WebSocket-Key: ")) != NULL) {
        /*握手*/
        char *shake_key = calloc(1, MAXSHAKE_LEN);
        sscanf(p, "Sec-WebSocket-Key: %s\r\n", shake_key);
        if (build_respone_header((char *)shake_key, (char *)send_buf) <= 0) {
            return -1;
        }
        free(shake_key);
    } else {
        char *recv_text = calloc(1, MAXRECVBUF_LEN);
        /*解包*/
        if (denpackage((char *)recv_buf, (char *)recv_text) == -1) {
            return -1;
        }
        int i;
        for (i = 0; i < strlen(recv_text); i ++) {
            recv_text[i] = toupper(recv_text[i]);
        }
        if (enpackage((char *)recv_text, (char *)send_buf) == -1) {
            return -1; 
        }
        free(recv_text);
    }

    return 0;
}

int main ()
{
    int listenport = 2346;

    struct sockaddr_in saddr, caddr;

    int listenfd, connectfd, addr_len;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);

    memset(&saddr, 0, sizeof(saddr));
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(listenport);
    saddr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(listenfd, (struct sockaddr *)&saddr, sizeof(saddr));

    /*设置最大连接数*/
    listen(listenfd, MAXLISTEN_NUM);

    printf("Accepting connections.... \n\n");

    char *recv_buf = calloc(1, MAXRECVBUF_LEN);
    char *send_buf = calloc(1, MAXSENDBUF_LEN);
    char *send_text = calloc(1, MAXSENDBUF_LEN);

    int ret, i = 0;

    int is_shake = 0;

    while (1) {

        if (is_shake == 0) {
            addr_len = sizeof(caddr);
            connectfd = accept(listenfd, (struct sockaddr *)&caddr, &addr_len); 
            /*if (fcntl(connectfd, F_SETFL, fcntl(connectfd, F_GETFD, 0) | O_NONBLOCK) == -1) {*/
                /*fprintf(stderr, "set nonblock error: %s \n", strerror(errno));*/
            /*}*/
            printf("connectfd: %d \n\n", connectfd);

            char str[100];
            memset(str, 0, 100);
            printf("Recive from %s:%d \n\n", inet_ntop(AF_INET, &caddr.sin_addr, str, sizeof(str)), ntohs(caddr.sin_port));

            if ((ret = recv(connectfd, recv_buf, MAXRECVBUF_LEN, 0)) <= 0 ) {
                fprintf(stderr, "shake recv buf error: %s \n", strerror(errno));
                break;
            }

            printf("shake ret: %d \n\n", ret);
            printf("shake recv_buf: %s \n\n", recv_buf);

            build_send_buf((char *)recv_buf, (char *)send_buf);
            printf("shake send_buf: %s \n\n", send_buf);

            if (send(connectfd, send_buf, strlen(send_buf), MSG_NOSIGNAL) < 0) {
                fprintf(stderr, "shake send buf error: %s \n", strerror(errno));
                break;
            }
            is_shake = 1;

        } else {

            memset(recv_buf, 0, MAXRECVBUF_LEN);
            memset(send_buf, 0, MAXSENDBUF_LEN);
            if ((ret = recv(connectfd, recv_buf, MAXRECVBUF_LEN, 0)) <= 0) {
                fprintf(stderr, "recv buf error: %s \n", strerror(errno));
                break;
            }
            printf("recv_buf: %s \n", recv_buf);

            build_send_buf((char *)recv_buf, (char *)send_buf);
            printf("send_buf: %s \n\n", send_buf);

            if (send(connectfd, send_buf, strlen(send_buf), MSG_NOSIGNAL) < 0) {
                fprintf(stderr, "shake send buf error: %s \n", strerror(errno));
                break;
            }


            memset(recv_buf, 0, MAXRECVBUF_LEN);
            memset(send_buf, 0, MAXSENDBUF_LEN);
            memset(send_text, 0, MAXSENDBUF_LEN);

            sprintf(send_text, "num: %d", i++);

            if (enpackage(send_text, (char *)send_buf) == -1) {
                break;
            }
            if (send(connectfd, send_buf, strlen(send_buf), MSG_NOSIGNAL) < 0) {
                fprintf(stderr, "send buf error: %s \n", strerror(errno));
                break;
            }
            printf("send buf: %s \n", send_buf);
        }
        sleep(2);
    }
    
    free(recv_buf);
    free(send_buf);
    free(send_text);

    close(connectfd);

    return 0;
}
