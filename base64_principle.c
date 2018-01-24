/*base64 编码原理*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

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
    for ( i = 0, j = 0 ; i < binlength ; i += 3 ) {
        current = (bindata[i] >> 2) ;
        current &= (uint8_t)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (uint8_t)(bindata[i] << 4 ) ) & ( (uint8_t)0x30 ) ;
        if ( i + 1 >= binlength ) {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }

        current |= ( (uint8_t)(bindata[i+1] >> 4) ) & ( (uint8_t) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (uint8_t)(bindata[i+1] << 2) ) & ( (uint8_t)0x3C ) ;
        if ( i + 2 >= binlength ) {
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

int main (int argc, char **argv)
{
    if (argc < 2) {
        printf("input error \n");
        exit(0);
    }
    printf("input: %s \n", argv[1]);

    char *base64_str = calloc(1, 1024);
    base64_encode(argv[1], base64_str, strlen(argv[1]));
    printf("input base64: %s \n", base64_str);

    free(base64_str);
    return 0;   
}
