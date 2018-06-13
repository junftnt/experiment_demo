/*共享线性栈 增删改查*/
/*一般出现在 互斥的两个栈里，一个减少，另外一个减少或者增多，总之不会两个同时增多的情况，不会造成 栈满的情况*/

/*线性表的两头分别作为两个栈的底端*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE    1
#define FALSE   -1
#define MAXLEN  20

typedef struct {
    char data[MAXLEN];
    int top1;
    int top2;
} Stacks;

static int init(Stacks *s)
{
    memset(s->data, '\0', MAXLEN);
    s->top1 = -1;
    s->top2 = MAXLEN;
    return TRUE;
}

static int empty(Stacks *s, int who)
{
    if (who == 1) {
        return s->top1 >= 0 ? TRUE : FALSE;
    } else {
        return s->top2 < MAXLEN ? TRUE : FALSE;
    }
}

static int top(Stacks *s, char *e, int who)
{
    if (who == 1) {
        *e = s->data[s->top1];
    } else {
        *e = s->data[s->top2];
    }
    return TRUE;
}

static int push(Stacks *s, char *e, int who)
{
    /*(max-top2) + (top1+1) >= max*/
    if (s->top1 - s->top2 + 1 >= 0) {
        return FALSE;
    }
    if (who == 1) {
        s->top1 ++;
        s->data[s->top1] = *e;
    } else {
        s->top2 --;
        s->data[s->top2] = *e;
    }
    return TRUE;
}

static int pop(Stacks *s, char *e, int who)
{
    if (who == 1) {
        if (s->top1 < 0) {
            return FALSE;
        }
        s->data[s->top1] = '\0';
        s->top1 --;
    } else {
        if (s->top2 >= MAXLEN) {
            return FALSE;
        }
        s->data[s->top2] = '\0';
        s->top2 ++;
    }
    return TRUE;
}

static int length(Stacks *s, int who)
{
    return who == 1 ? s->top1 + 1 : MAXLEN - s->top2;
}

int main ()
{
    Stacks *s = calloc(1, sizeof(Stacks));

    init(s);
    empty(s, 1);
    char e[1];
    push(s, "d", 1);
    top(s, e, 1);
    push(s, "c", 1);
    top(s, e, 1);
    push(s, "b", 2);
    top(s, e, 2);
    push(s, "a", 2);
    top(s, e, 2);
    length(s, 1);
    pop(s, e, 1);
    top(s, e, 2);

    return 0;
}
