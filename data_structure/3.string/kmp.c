/**

KMP算法
参考: http://www.ruanyifeng.com/blog/2013/05/Knuth%E2%80%93Morris%E2%80%93Pratt_algorithm.html

核心为获取搜索词子串的 部分匹配词, 形成一张 部分匹配表, 用来决定搜索词的移动位置
--------------------------------------
搜索词     | A   B   C   D   A   B   D
部分匹配词 | 0   0   0   0   1   2   0
--------------------------------------

搜索词产生逻辑:
首先, 要了解两个概念: "前缀"和"后缀". "前缀"指除了最后一个字符以外, 一个字符串的全部头部组合; "后缀"指除了第一个字符以外, 一个字符串的全部尾部组合

"部分匹配值"就是"前缀"和"后缀"的最长的共有元素的长度. 以"ABCDABD"为例:
- "A"的前缀和后缀都为空集, 共有元素的长度为0
- "AB"的前缀为[A], 后缀为[B], 共有元素的长度为0
- "ABC"的前缀为[A, AB], 后缀为[BC, C], 共有元素的长度0
- "ABCD"的前缀为[A, AB, ABC], 后缀为[BCD, CD, D], 共有元素的长度为0
- "ABCDA"的前缀为[A, AB, ABC, ABCD], 后缀为[BCDA, CDA, DA, A], 共有元素为"A", 长度为1
- "ABCDAB"的前缀为[A, AB, ABC, ABCD, ABCDA], 后缀为[BCDAB, CDAB, DAB, AB, B], 共有元素为"AB", 长度为2
- "ABCDABD"的前缀为[A, AB, ABC, ABCD, ABCDA, ABCDAB], 后缀为[BCDABD, CDABD, DABD, ABD, BD, D], 共有元素的长度为0
*/


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

static int *next_val(char *p, int len)
{
    int *next = (int *)calloc(1, len);
    
    int i = 0, j = -1;
    next[0] = j;
    while (i < len) {
        if (j == -1 || p[i] == p[j]) {
            ++i;
            ++j;
            next[i] = j;
        } else {
            j = next[j];
        }
    }

    return next;
}

int main ()
{
    /*char str[] = "BBC ABCDAB ABCDABCDABDE";*/
    /*char search[] = "ABCDABD";*/
    char str[] = "ababababca";
    char search[] = "abababca";
    /*char str[] = "decdefghigkl";*/
    /*char search[] = "defgh";*/
    int slen = strlen(str);
    int len = strlen(search);

    printf("str:    %s \n", str);
    printf("search: %s \n", search);
    printf("       ");

    int *next = next_val(search, len);
    int i, j;
    for (i = 0; i < len; i++) {
        printf("%d", next[i]);
    }

    i = 0;
    j = 0;

    while (i < slen && j < len) {
        if (j == -1 || str[i] == search[j]) {
            i++;
            j++;
        } else {
            j = next[j];
        }
    }
    if (j == len) {
        printf("\n\npos: %d \n", i - j);
    } else {
        printf("\n\nno pos: %d \n", -1);
    }

    return 0;
}
