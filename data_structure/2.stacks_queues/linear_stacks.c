/*线性栈 增删改查*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE    1
#define FALSE   -1
#define MAXLEN  100

typedef struct {
    char data[MAXLEN];
    int top;
} Stacks;

static int init(Stacks *s)
{
    memset(s->data, '\0', MAXLEN);
    s->top = -1;
    return TRUE;
}

static int empty(Stacks *s)
{
    return s->top >= 0 ? TRUE : FALSE;
}

static int top(Stacks *s, char *e)
{
    /*strcpy(e, (char *)&(s->data[s->top]));*/
    *e = s->data[s->top];
    return TRUE;
}

static int push(Stacks *s, char *e)
{
    if (s->top >= MAXLEN - 1) {
        return FALSE;
    }
    s->top ++;
    s->data[s->top] = *e;
    return TRUE;
}

static int pop(Stacks *s, char *e)
{
    if (s->top == -1) {
        return FALSE;
    }
    s->data[s->top] = '\0';
    s->top--;
    return TRUE;
}

static int length(Stacks *s)
{
    return s->top + 1;
}

int main ()
{
    Stacks *s = calloc(1, sizeof(Stacks));

    init(s);
    empty(s);
    char e[1];
    push(s, "d");
    top(s, e);
    push(s, "c");
    top(s, e);
    push(s, "b");
    top(s, e);
    push(s, "a");
    top(s, e);
    length(s);
    pop(s, e);
    top(s, e);

    return 0;
}
