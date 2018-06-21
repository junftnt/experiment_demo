/*线性队列 增删改查*/
/*队列：先进先出, 一端入队列，另外一端出队列*/

/*单纯的 线性队列存在一个问题，即每次出队列都需要将所有数据往前移动，操作 O(n), 所以 线性队列一般都是采用 循环队列, 出队列和入队列都是 O(1)*/

/*线性列表 循环队列, 一般采用 数组来存储数据, 另外设置 头部、尾部 游标, 来标记 头尾结点位置, 方便操作*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE    1
#define FALSE   -9999
#define MAXLEN  10

typedef struct {
    int data[MAXLEN];
    int top;
    int bottom;
    int count;
} Queues;

static int init(Queues *q)
{
    memset(q->data, 0, MAXLEN);
    q->top = 0;
    q->bottom = 0;
    q->count = 0;
    return TRUE;
}

static int empty(Queues *q)
{
    return q->count >= 0 ? TRUE : FALSE;
}

static int top(Queues *q)
{
    if (q->count == 0) {
        return FALSE;
    }
    return q->data[q->top];
}

static int push(Queues *q, int e)
{
    if (q->count >= MAXLEN) {
        return FALSE;
    }
    q->count ++;
    q->data[q->bottom] = e;
    if (q->top + q->count >= MAXLEN) {
        q->bottom = q->top + q->count - MAXLEN;
    } else {
        q->bottom = q->top + q->count;
    }
    return TRUE;
}

static int pop(Queues *q)
{
    if (q->count == 0) {
        return FALSE;
    }
    int ret = q->data[q->top];
    q->count --;
    q->top ++;
    if (q->top == MAXLEN) {
        q->top = 0;
    }

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
