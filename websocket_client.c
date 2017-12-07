#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

#include <fcntl.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

typedef int bool;
#define true 1
#define false 0

typedef struct socket_info {
    struct sockaddr_in addr;
    char     *host;
    char     *port;
} socket_info;

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

//websocket根据data[0]判别数据包类型    比如0x81 = 0x80 | 0x1 为一个txt类型数据包
typedef enum {
    // 0x0：标识一个中间数据包
    WCT_MINDATA = -20,
    // 0x1：标识一个txt类型数据包
    WCT_TXTDATA = -19,
    // 0x2：标识一个bin类型数据包
    WCT_BINDATA = -18,
    // 0x8：标识一个断开连接类型数据包
    WCT_DISCONN = -17,
    // 0x8：标识一个断开连接类型数据包
    WCT_PING = -16,
    // 0xA：表示一个pong类型数据包
    WCT_PONG = -15,
    WCT_ERR = -1,
    WCT_NULL = 0
} w_com_type;


static void get_random_string(uint8_t *buf, uint32_t len)
{
    uint32_t i;
    uint8_t temp;
    srand((int)time(0));
    for (i = 0; i < len; i ++) {
        temp = (uint8_t)(rand()%256);
        if (temp == 0) {
            temp = 128;
        }
        buf[i] = temp;
    }
}

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

static int tolower(int c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + 'a' - 'A';
    } else {
        return c;
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
static void delayms(uint32_t ms)
{
    struct timeval t;
    t.tv_sec    = ms / 1000;
    t.tv_usec   = (ms%1000)*1000;
    select(0, NULL, NULL, NULL, &t);
}

static int build_shake_key(uint8_t *key)
{
    uint8_t temp_key[16] = {"\0"};
    get_random_string(temp_key, 16);
    return base64_encode((const uint8_t *)temp_key, (char *)key, 16);
}

static int build_respon_shake_key(uint8_t *accept_key, uint32_t accept_key_len, uint8_t *respond_key)
{
    char *client_key;
    char *sha1_data_temp;
    char *sha1_data;
    int i, n;
    const char guid[] = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    uint32_t guidlen;

    if(accept_key == NULL)
        return 0;

    guidlen = sizeof(guid);
    client_key = (char *)calloc(1, sizeof(char)*(accept_key_len + guidlen + 10));
    memset(client_key, 0, (accept_key_len + guidlen + 10));

    memcpy(client_key, accept_key, accept_key_len);
    memcpy(&client_key[accept_key_len], guid, guidlen);
    client_key[accept_key_len + guidlen] = '\0';

    sha1_data_temp = sha1_hash(client_key);
    n = strlen(sha1_data_temp);
    sha1_data = (char *)calloc(1, n / 2 + 1);
    memset(sha1_data, 0, n / 2 + 1);

    for(i = 0; i < n; i += 2)
        sha1_data[ i / 2 ] = htoi(sha1_data_temp, i, 2);

    n = base64_encode((const uint8_t *)sha1_data, (char *)respond_key, (n / 2));

    free(sha1_data_temp);
    free(sha1_data);
    free(client_key);
    return n;
}

static int match_shake_key(uint8_t *my_key, uint32_t my_key_len, uint8_t *accept_key, 
        uint32_t accept_key_len)
{
    int ret_len;
    uint8_t temp_key[256] = {"\0"};

    ret_len = build_respon_shake_key(my_key, my_key_len, temp_key);

    if(ret_len != accept_key_len) {
        return -1;
    } else if(strcmp((const char *)temp_key, (const char *)accept_key) != 0) {
        return -1;
    }

    return 0;
}

static void build_header(char *ip, char *port, uint8_t *shake_key, char *package)
{
    const char header[] = "GET /null HTTP/1.1\r\n"
        "Connection: Upgrade\r\n"
        "Host: %s:%s\r\n"
        "Sec-WebSocket-Key: %s\r\n"
        "Sec-WebSocket-Version: 13\r\n"
        "Upgrade: websocket\r\n\r\n";
    sprintf(package, header, ip, port, shake_key);
}

static void build_respon_header(uint8_t *accept_key, uint32_t accept_key_len, char *package)
{
    const char header[] = "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Server: Microsoft-HTTPAPI/2.0\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n"
        "%s\r\n\r\n";

    time_t now;
    struct tm *tm_now;
    char time_str[256] = {0};
    uint8_t respond_shake_key[256] = {0};
    /*构建回应的握手key*/
    build_respon_shake_key(accept_key, accept_key_len, respond_shake_key);   
    /*构建回应时间字符串*/
    time(&now);
    tm_now = localtime(&now);
    /*时间打包待续  格式如 "Date: Tue, 20 Jun 2017 08:50:41 CST\r\n"*/
    strftime(time_str, sizeof(time_str), "Date: %a, %d %b %Y %T %Z", tm_now);
    /*组成回复信息*/
    sprintf(package, header, respond_shake_key, time_str);
}

/**
 * websocket数据收发阶段的数据打包, 通常client发server的数据都要isMask(掩码)处理, 
 * 反之server到client却不用
 *
 * @param uint8_t       *data               准备发出的数据
 * @param uint32_t      data_len            长度
 * @param uint8_t       *package            打包后存储地址
 * @param uint32_t      package_max_len     存储地址可用长度
 * @param bool          is_mask             是否使用掩码     1要   0 不要
 * @param w_com_type    type                数据类型, 由打包后第一个字节决定, 这里默认是数据传输, 即0x81
 *
 * @return uint32_t     打包后的长度(会比原数据长2~16个字节不等)      <=0 打包失败
 *
 */
static int w_enpackage(uint8_t *data, uint32_t data_len, uint8_t *package, uint32_t package_max_len, 
        bool is_mask, w_com_type type)
{
    /*掩码*/
    uint8_t mask_key[4] = {0};
    uint8_t temp1, temp2;
    int count;
    uint32_t i, len = 0;

    if(package_max_len < 2)
        return -1;

    if(type == WCT_MINDATA)
        *package++ = 0x00;
    else if(type == WCT_TXTDATA)
        *package++ = 0x81;
    else if(type == WCT_BINDATA)
        *package++ = 0x82;
    else if(type == WCT_DISCONN)
        *package++ = 0x88;
    else if(type == WCT_PING)
        *package++ = 0x89;
    else if(type == WCT_PONG)
        *package++ = 0x8A;
    else
        return -1;

    if(is_mask)
        *package = 0x80;
    len += 1;

    if(data_len < 126) {
        *package++ |= (data_len&0x7F);
        len += 1;
    } else if(data_len < 65536) {
        if(package_max_len < 4)
            return -1;
        *package++ |= 0x7E;
        *package++ = (char)((data_len >> 8) & 0xFF);
        *package++ = (uint8_t)((data_len >> 0) & 0xFF);
        len += 3;
    } else if(data_len < 0xFFFFFFFF) {
        if(package_max_len < 10)
            return -1;
        *package++ |= 0x7F;
        /*(char)((data_len >> 56) & 0xFF);   数据长度变量是 uint32_t data_len, 暂时没有那么多数据*/
        *package++ = 0; 
        /*(char)((data_len >> 48) & 0xFF);*/
        *package++ = 0;
        /*(char)((data_len >> 40) & 0xFF);*/
        *package++ = 0;
        /*(char)((data_len >> 32) & 0xFF);*/
        *package++ = 0;
        /*到这里就够传4GB数据了*/
        *package++ = (char)((data_len >> 24) & 0xFF);
        *package++ = (char)((data_len >> 16) & 0xFF);
        *package++ = (char)((data_len >> 8) & 0xFF);
        *package++ = (char)((data_len >> 0) & 0xFF);
        len += 9;
    }
    /*数据使用掩码时, 使用异或解码, maskKey[4]依次和数据异或运算, 逻辑如下*/
    if(is_mask) {
        if(package_max_len < len + data_len + 4)
            return -1;
        
        /*随机生成掩码*/
        get_random_string(mask_key, sizeof(mask_key));
        *package++ = mask_key[0];
        *package++ = mask_key[1];
        *package++ = mask_key[2];
        *package++ = mask_key[3];
        len += 4;
        for(i = 0, count = 0; i < data_len; i++) {
            temp1 = mask_key[count];
            temp2 = data[i];
            /*异或运算后得到数据*/
            *package++ = (char)(((~temp1)&temp2) | (temp1&(~temp2)));
            count += 1;
            /*maskKey[4]循环使用*/
            if(count >= sizeof(mask_key))
                count = 0;
        }
        len += i;
        *package = '\0';
    } else {
        /*数据没使用掩码, 直接复制数据段*/
        if(package_max_len < len + data_len)
            return -1;
        memcpy(package, data, data_len);
        package[data_len] = '\0';
        len += data_len;
    }
    
    return len;
}

/**
 * websocket数据收发阶段的数据解包, 通常client发server的数据都要isMask(掩码)处理, 
 * 反之server到client却不用
 *
 * @param uint8_t   *data           解包的数据
 * @param uint32_t  data_len        长度
 * @param uint8_t   *package        解包后存储地址
 * @param uint32_t  package_max_len 存储地址可用长度
 * @param uint32_t  *package_len    解包所得长度
 *
 * @return int      解包识别的数据类型 如 : txt数据, bin数据, ping, pong等
 *
 */
static int w_depackage(uint8_t *data, uint32_t data_len, uint8_t *package, uint32_t package_max_len, 
        uint32_t *package_len)
{
    /*掩码*/
    uint8_t mask_key[4] = {0};
    uint8_t temp1, temp2;
    char mask = 0, type;
    int count, ret;
    uint32_t i, len = 0, data_start = 2;
    if(data_len < 2)
        return -1;

    type = data[0]&0x0F;

    if((data[0]&0x80) == 0x80) {
        if(type == 0x01) 
            ret = WCT_TXTDATA;
        else if(type == 0x02) 
            ret = WCT_BINDATA;
        else if(type == 0x08) 
            ret = WCT_DISCONN;
        else if(type == 0x09) 
            ret = WCT_PING;
        else if(type == 0x0A) 
            ret = WCT_PONG;
        else 
            return WCT_ERR;
    } else if(type == 0x00) {
        ret = WCT_MINDATA;
    } else {
        return WCT_ERR;
    }

    if((data[1] & 0x80) == 0x80) {
        mask = 1;
        count = 4;
    } else {
        mask = 0;
        count = 0;
    }

    len = data[1] & 0x7F;

    if(len == 126) {
        if(data_len < 4)
            return WCT_ERR;
        len = data[2];
        len = (len << 8) + data[3];
        if(data_len < len + 4 + count)
            return WCT_ERR;
        if(mask) {
            mask_key[0] = data[4];
            mask_key[1] = data[5];
            mask_key[2] = data[6];
            mask_key[3] = data[7];
            data_start = 8;
        } else {
            data_start = 4;
        }
    } else if(len == 127) {
        if(data_len < 10)
            return WCT_ERR;

        /*使用8个字节存储长度时, 前4位必须为0, 装不下那么多数据...*/
        if(data[2] != 0 || data[3] != 0 || data[4] != 0 || data[5] != 0)
            return WCT_ERR;
        len = data[6];
        len = (len << 8) + data[7];
        len = (len << 8) + data[8];
        len = (len << 8) + data[9];
        if(data_len < len + 10 + count)
            return WCT_ERR;

        if(mask) {
            mask_key[0] = data[10];
            mask_key[1] = data[11];
            mask_key[2] = data[12];
            mask_key[3] = data[13];
            data_start = 14;
        } else {
            data_start = 10;
        }
    } else {
        if(data_len < len + 2 + count)
            return WCT_ERR;

        if(mask) {
            mask_key[0] = data[2];
            mask_key[1] = data[3];
            mask_key[2] = data[4];
            mask_key[3] = data[5];
            data_start = 6;
        } else {
            data_start = 2;
        }
    }

    if(data_len < len + data_start)
        return WCT_ERR;

    if(package_max_len < len + 1)
        return WCT_ERR;

    /*解包数据使用掩码时, 使用异或解码, maskKey[4]依次和数据异或运算, 逻辑如下*/
    if(mask) {
        for(i = 0, count = 0; i < len; i++) {
            temp1 = mask_key[count];
            temp2 = data[i + data_start];
            /*异或运算后得到数据*/
            *package++ =  (char)(((~temp1)&temp2) | (temp1&(~temp2)));
            count += 1;
            /*mask_key[4]循环使用*/
            if(count >= sizeof(mask_key))
                count = 0;
        }
        *package = '\0';
    } else {
        /*解包数据没使用掩码, 直接复制数据段*/
        memcpy(package, &data[data_start], len);
        package[len] = '\0';
    }
    *package_len = len;
    return ret;
}

/**
 * 服务器回复客户端的连接请求, 以建立websocket连接
 *
 * @param int       fd          连接句柄
 * @param char      *recv_buf   接收到来自客户端的数据(内含http连接请求)
 * @param uint32_t  buf_len : 
 *
 * @return int      =0 建立websocket连接成功        <0 建立websocket连接失败
 *
 */
static int w_server_to_client(int fd, char *recv_buf, uint32_t buf_len)
{
    char *p;
    int ret;
    char recv_shake_key[512], respond_package[1024];
    
    if((p = strstr(recv_buf, "Sec-WebSocket-Key: ")) == NULL)
        return -1;

    p += strlen("Sec-WebSocket-Key: ");

    memset(recv_shake_key, 0, sizeof(recv_shake_key));
    /*取得握手key*/
    sscanf(p, "%s", recv_shake_key);
    ret = strlen(recv_shake_key);
    if(ret < 1) {
        return -1;
    }
    memset(respond_package, 0, sizeof(respond_package));
    build_respon_header(recv_shake_key, ret, respond_package);

    return send(fd, respond_package, strlen(respond_package), MSG_NOSIGNAL);
}

/*客户端连接服务端握手处理*/
static int w_client_to_server(socket_info *socket_infos)
{
    struct sockaddr_in addr = socket_infos->addr;
    int fd, ret, timeout;
    char buf[512] = {"\0"}, *p;
    /*协议内容*/
    uint8_t shake_buf[512] = {"\0"}, shake_key[128] = {"\0"};

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        fprintf(stderr, "socket create faild: %s \n", strerror(errno));
        return fd;
    }

    /*非阻塞*/
    ret = fcntl(fd, F_SETFL, 0);
    fcntl(fd, F_SETFL, ret | O_NONBLOCK);

    timeout = 0;
    while ((ret = connect(fd, (struct sockaddr *)&addr, sizeof(addr))) == -1) {
        if (++timeout > 100) {
            fprintf(stderr, "connect timeout: %s \n", strerror(errno));
            close(fd);
            return -1;
        }
        delayms(1);
    }

    /*握手key*/
    memset(shake_key, 0, sizeof(shake_key));
    build_shake_key(shake_key);
    /*协议包*/
    memset(shake_buf, 0, sizeof(shake_buf));
    build_header(socket_infos->host, socket_infos->port, shake_key, (char *)shake_buf);
    printf("Request header:-------------- \n%s\n", shake_buf);
    /*发送协议包*/
    ret = send(fd, shake_buf, strlen((const char *)shake_buf), MSG_NOSIGNAL);
    if (ret < 0) {
        fprintf(stderr, "Send to shake info faild: %s \n", strerror(errno));
        return ret;
    }

    /*握手*/
    timeout = 0;
    while(true) {
        memset(buf, 0, sizeof(buf));
        ret = recv(fd, buf, sizeof(buf), MSG_NOSIGNAL);
        printf("Response header:-------------- \n%s\n", buf);
        if (ret > 0) {
            /*判断是不是HTTP协议头*/
            if (strncmp((const char *)buf, (const char *)"HTTP", strlen((const char *)"HTTP")) == 0) {
                /*检验握手信号*/
                if ((p = (uint8_t *)strstr((const char*)buf, (const char *)"Sec-WebSocket-Accept: ")) != NULL) {
                    p += strlen((const char *)"Sec-WebSocket-Accept: "); 
                    sscanf((const char *)p, "%s\r\n", p);
                    printf("accept key: %s\n", p);
                    /*握手成功*/
                    if (match_shake_key(shake_key, strlen((const char*)shake_key), p, 
                                strlen((const char *)p)) == 0) {
                        return fd;
                    } else {
                        /*握手不对，重新发送协议包*/
                        ret = send(fd, shake_buf, strlen((const char *)shake_buf), MSG_NOSIGNAL);
                    }
                } else {
                    ret = send(fd, shake_buf, strlen((const char *)shake_buf), MSG_NOSIGNAL);
                }
            }
        }
        if (++timeout > 100) {
            fprintf(stderr, "shake to server timeout: %s \n", strerror(errno));
            close(fd);
            return -1;
        }
        delayms(1);
    }
    return -1;
}

/**
 * websocket数据基本打包和发送
 *
 * @param int           fd          连接句柄
 * @param uint8_t       *data       数据
 * @param uint32_t      data_len    长度
 * @param bool          mod         数据是否使用掩码, 客户端到服务器必须使用掩码模式
 * @param w_com_type    type        数据要要以什么识别头类型发送(txt, bin, ping, pong ...)
 *
 * @return uint32_t     调用send的返回
 *
 */
static int w_send(int fd, uint8_t *data, uint32_t data_len, bool mod, w_com_type type)
{
    uint8_t *websocket_package;
    uint32_t ret_len, ret;

    /*websocket数据打包*/
    websocket_package = (uint8_t *)calloc(1, sizeof(char)*(data_len + 128));
    memset(websocket_package, 0, (data_len + 128));
    ret_len = w_enpackage(data, data_len, websocket_package, (data_len + 128), mod, type);
    ret = send(fd, websocket_package, ret_len, MSG_NOSIGNAL);
    free(websocket_package);
    return ret;
}

/**
 * websocket数据接收和基本解包
 *
 * @param int       fd              连接句柄
 * @param uint8_t   *data           数据接收地址
 * @param uint32_t  data_max_len    接收区可用最大长度
 *
 * @return int      <= 0 没有收到有效数据   > 0 成功接收并解包数据
 *
 */
static int w_recv(int fd, uint8_t *data, uint32_t data_max_len)
{
    uint8_t *websocket_package, *recv_buf;
    int ret, ret2 = 0;
    uint32_t ret_len = 0;

    recv_buf = (uint8_t *)calloc(1, sizeof(char)*data_max_len);
    memset(recv_buf, 0, data_max_len);
    ret = recv(fd, recv_buf, data_max_len, MSG_NOSIGNAL);
    if(ret > 0) {
        if(strncmp(recv_buf, "GET", 3) == 0) {
            ret2 = w_server_to_client(fd, recv_buf, ret);
            free(recv_buf);
            if(ret2 < 0) {
                memset(data, 0, data_max_len);
                fprintf(stderr, "connect false: %s \n", strerror(errno));
                return -1;
            }
            memset(data, 0, data_max_len);
            printf("retry connect ...\r\n");
            return -1;
        }

        /*websocket数据打包*/
        websocket_package = (uint8_t *)calloc(1, sizeof(char)*(ret + 128));
        memset(websocket_package, 0, (ret + 128));
        ret2 = w_depackage(recv_buf, ret, websocket_package, (ret + 128), &ret_len);
        /*解析为ping包, 自动回pong*/
        if(ret2 == WCT_PING && ret_len > 0) {
            w_send(fd, websocket_package, ret_len, true, WCT_PONG);
            /*显示数据*/
            printf("webSocket_recv : PING %d\r\n%s\r\n" , ret_len, websocket_package); 
            free(recv_buf);
            free(websocket_package);
            return WCT_NULL;
        } else if(ret_len > 0 && (ret2 == WCT_TXTDATA || ret2 == WCT_BINDATA || ret2 == WCT_MINDATA)) {
            /*解析为数据包*/
            /*把解析得到的数据复制出去*/
            memcpy(data, websocket_package, ret_len);
            strncpy(data, recv_buf, strlen(data));
            free(recv_buf);
            free(websocket_package);
            return ret_len;
        }
        free(recv_buf);
        free(websocket_package);
        return -ret;
    } else {
        free(recv_buf);
        return ret;
    }
}


int main ()
{
    char *host = "127.0.0.1", *port = "2346";
    /*char *host = "127.0.0.1", *port = "8001";*/

    /*配置地址*/
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    int fd, ret, timeout;
    char buf[512] = {"\0"}, send_text[512] = "\0", *p;
    while (strlen(send_text) < 256) {
        strcat(send_text, "x");
    }

    socket_info *socket_infos = calloc(1, sizeof(socket_info));
    socket_infos->host  = host;
    socket_infos->port  = port;
    socket_infos->addr  = server_addr;
    fd = w_client_to_server(socket_infos);
    if (fd < 0) {
        fprintf(stderr, "Shake with %s:%s faild: %s \n", host, port, strerror(errno));
        exit(0);
    } else {
        printf("Shake with %s:%s succeeded...\n\n", host, port);
    }

    /*send(fd, send_text, strlen(send_text) + 1, 0);*/
    /*recv(fd, buf, 512, 0);*/
    ret = w_send(fd, send_text, strlen(send_text), true, WCT_TXTDATA);
    timeout = 0;
    while (true) {
        ret = w_recv(fd, buf, sizeof(buf));
        if (ret < 0) {
            if (++timeout > 100) {
                fprintf(stderr, "recv message timeout: %s \n", strerror(errno));
                break;
            }
        } else {
            printf("recv message: %s \n", buf);
            break;
        }
        delayms(1);
    }

    close(fd);

    return 0;
}
