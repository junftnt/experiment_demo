/*链表队列*/
/*队列：先进先出, 一端入队列，另外一端出队列*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE    1
#define FALSE   -9999

/*结点*/
typedef struct {
    int data;
    struct Node *next;
} Node, *QueuesNode;

/*链表队列*/
typedef struct {
    QueuesNode top, bottom;
    int count;
} Queues;

static int init(Queues *q)
{
    q->top = NULL;
    q->bottom = NULL;
    q->count = 0;
    return TRUE;
}

static int empty(Queues *q)
{
    return q->count == 0 ? TRUE : FALSE;
}

static int top(Queues *q)
{
    if (q->count == 0) {
        return FALSE;
    }
    return q->top->data;
}

static int push(Queues *q, int e)
{
    Node *n = calloc(1, sizeof(Node));
    n->data = e;
    q->count ++;
    if (q->count == 1) {
        q->top = n;
        q->bottom = n;
        return TRUE;
    }
    q->bottom->next = (struct Node *)n;
    q->bottom = n;
    return TRUE;
}

static int pop(Queues *q)
{
    if (q->count == 0) {
        return FALSE;
    } else if (q->count == 1) {
        int ret = q->top->data;
        free(q->top);
        init(q);
        return ret;
    }
    int ret = q->top->data;
    Node *tmp = q->top;
    q->top = (Node *)tmp->next;
    free(tmp);
    q->count --;
    return ret;
}

static int length(Queues *q)
{
    return q->count;
}

int main ()
{
    Queues *q = calloc(1, sizeof(Queues));

    init(q);
    empty(q);
    push(q, 1);
    printf("top: %d \n", top(q));
    printf("length: %d \n\n", length(q));
    push(q, 2);
    printf("top: %d \n", top(q));
    printf("length: %d \n\n", length(q));
    push(q, 3);
    printf("top: %d \n", top(q));
    printf("length: %d \n\n", length(q));
    push(q, 4);
    push(q, 5);
    push(q, 6);
    push(q, 7);
    push(q, 8);
    push(q, 9);
    push(q, 10);
    printf("top: %d \n", top(q));
    printf("length: %d \n\n", length(q));
    printf("pop: %d \n", pop(q));
    printf("top: %d \n", top(q));
    printf("length: %d \n\n", length(q));

    return 0;
}
