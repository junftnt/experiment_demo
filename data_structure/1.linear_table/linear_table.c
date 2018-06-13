/*线性表 增删改查*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define TRUE                1
#define FALSE               -1

#define MAX_LINEAR_TABLE    20


typedef struct {
    char data[MAX_LINEAR_TABLE]; 
    int len;
} Linear;

static int initList(Linear *list)
{
    memset(list->data, '\0', MAX_LINEAR_TABLE);
    list->len = 0;

    return TRUE;
}

static int listEmpty(Linear *list)
{
    if (list->len == 0) {
        return TRUE;
    }

    return FALSE;
}

static int getElem(Linear *list, int pos, char *e)
{
    if (pos <= 0 || pos > list->len) {
        return FALSE;
    }
    *e = list->data[pos - 1];
}

static int locateElem(Linear *list, char e)
{
    int i;
    for (i = 0; i < list->len; i++) {
        char tmp = list->data[i];
        if (tmp == e)  {
            return i + 1;
        }
    }

    return FALSE;
}

static int listInsert(Linear *list, int pos, char e)
{
    if (pos <= 0 || pos - 1 > list->len || list->len >= MAX_LINEAR_TABLE) {
        return FALSE;
    }

    int i;
    char tmp;
    list->len++;

    for (i = pos - 1; i < list->len; i++) {
        tmp = list->data[i];
        list->data[i] = e;
        e = tmp;
    }
    
    return TRUE;
}

static int listDelete(Linear *list, int pos)
{
    if (pos <= 0 || pos - 1 > list->len) {
        return FALSE;
    }

    int i;
    char *tmp;

    for (i = pos - 1; i < list->len; i++) {
        list->data[i] = list->data[i+1];
    }
    list->data[list->len] = '\0';
    list->len --;
    
    return TRUE;
}

static int listLength(Linear *list)
{
    return list->len;
}

int main (void)
{
    Linear *l = calloc(1, sizeof(Linear));
    initList(l);
    listEmpty(l);
    listInsert(l, 1, 'd');
    listInsert(l, 1, 'c');
    listInsert(l, 1, 'b');
    listInsert(l, 1, 'a');
    listLength(l);
    char str;
    getElem(l, 2, &str);
    locateElem(l, 'c');
    listDelete(l, 2);

    return TRUE;
}
