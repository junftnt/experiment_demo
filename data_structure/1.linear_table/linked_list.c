#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

typedef struct {
    char data[10];
    /*头结点里面 len 存储的整个链表长度，data为空*/
    /*其余结点 len 存储的是 data 长度*/
    int len;
    struct Node *next;
} Node;

#define TRUE                1
#define FALSE               -1

static int listEmpty(Node *list)
{
    if (list->len == 0) {
        return TRUE;
    }

    return FALSE;
}

static int getElem(Node *list, int pos, char *e)
{
    if (pos <= 0 || pos > list->len) {
        return FALSE;
    }

    Node *tmp = list;
    while (tmp) {
        pos --;
        if (pos == 0) {
            strcpy(e, tmp->data);
            break;
        }
        tmp = (Node *)tmp->next;
    }

    return TRUE;
}

static int locateElem(Node *list, char *e)
{
    Node *tmp = list;
    int i = 1;
    while (tmp) {
        if (strcmp(e, tmp->data) == 0) {
            return i;
        }
        i ++;
        tmp = (Node *)tmp->next;
    }

    return FALSE;
}

static int listInsert(Node *list, int pos, char *e)
{
    if (pos <= 0 || pos - 1 > list->len) {
        return FALSE;
    }

    list->len ++;

    Node *l = calloc(1, sizeof(Node));
    strcpy(l->data, e);
    l->len = sizeof(e);

    if (list->len == 1) {
        list->next = (struct Node *) l;
        return TRUE;
    }

    Node *tmp = list;
    while (tmp) {
        pos --;
        if (pos == 0) {
            l->next = tmp->next;
            tmp->next = (struct Node *) l;
            break;
        }
        tmp = (Node *)tmp->next;
    }

    return TRUE;
}

static int listDelete(Node *list, int pos)
{
    if (pos <= 0 || pos - 1 > list->len) {
        return FALSE;
    }

    Node *tmp = list;
    while (tmp) {
        pos --;
        if (pos == 1) {
            Node *d = (Node *)((Node *)tmp->next)->next;
            free(tmp->next);
            tmp->next = (struct Node *)d;
            break;
        }
        tmp = (Node *)tmp->next;
    }

    return TRUE;
}

static void listShow(Node *list)
{
    Node *tmp = list;
    while (tmp) {
        printf("data: %s, len: %d, next: %p \n", tmp->data, tmp->len, (void *)tmp->next);
        tmp = (Node *)tmp->next;
    }
    printf("\n");
}

static int listLength(Node *list)
{
    return list->len;
}

int main (void)
{
    Node *l = calloc(1, sizeof(Node));
    listEmpty(l);
    listShow(l);
    listInsert(l, 1, "d0");
    listShow(l);
    listInsert(l, 1, "c0");
    listShow(l);
    listInsert(l, 1, "b0");
    listShow(l);
    listInsert(l, 2, "a0");
    listShow(l);
    listLength(l);
    char str[10];
    getElem(l, 2, str);
    locateElem(l, "c0");
    listDelete(l, 2);
    listShow(l);

    return TRUE;
}
