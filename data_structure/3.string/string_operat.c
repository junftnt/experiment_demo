/*字符串基本操作*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <assert.h>

#define TRUE    1
#define FALSE   -9999

/*字符串翻转*/
static void flip(char *from, char *to)
{
    assert(from != NULL && to != NULL);
    char *ptr = from;;
    int len = strlen(from);
    while (*ptr != '\0') {
        to[len - 1] = *ptr;
        len --;
        *ptr ++;
    }
    to[strlen(from)] = '\0';
}

/*第一个只出现K次的字符*/
static int first_count(char *from, int k)
{
    int count[255] = {0};
    assert(from != NULL);
    char *ptr = from;;
    int i = 0, j = 0;
    while (*ptr != '\0') {
        count[from[i]] ++;
        i ++;
        *ptr ++;
    }
    for (j; j < i; j ++) {
        if (count[from[j]] == k) {
            return j;
        }
    }
    return FALSE;
}

/*大写变成小写，小写变成大写*/
static void case_conversion(char *from)
{
    assert(from != NULL);
    char *ptr = from;;
    while (*ptr != '\0') {
        if (*ptr <= 'Z' && *ptr >= 'A') {
            *ptr = *ptr + 'a' - 'A';
        } else {
            *ptr = *ptr + 'A' - 'a';
        }
        *ptr ++;
    }
}

/*打印字符串string1中有多少个string2*/
static int find_str_nums(char *from, char *find)
{
    assert(from != NULL && find != NULL);
    int i, j, ok, oknum = 0, frlen = strlen(from), filen = strlen(find);
    for (i = 0; i < frlen; i ++) {
        ok = TRUE;
        for (j = 0; j < filen; j ++) {
            if (from[i+j] != find[j]) {
                ok = FALSE;
                break;
            }
        }
        if (ok == TRUE) {
            oknum ++;
        }
    }

    return oknum; 
}

int main ()
{
    char from[] = "aBcDEFGaBaBc";
    char *to;
    flip(from, to);
    printf("%s\n", to);

    int n = first_count(from, 3);
    printf("%c\n", from[n]);

    case_conversion(from);

    char find[] = "bA";
    printf("%d\n", find_str_nums(from, find));

    return 0;
}
