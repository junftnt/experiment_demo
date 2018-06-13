/*链栈*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE    1
#define FALSE   -1

/*结点*/
typedef struct {
    char data[10];
    struct Node *next;
} Node, *StacksNode;

/*链表栈*/
typedef struct {
    StacksNode top;
    int count;
} Stacks;

static int init(Stacks *s)
{
    s->top = NULL;
    s->count = 0;
    return TRUE;
}

static int empty(Stacks *s)
{
    return s->count == 0 ? TRUE : FALSE;
}

static int top(Stacks *s, char *e)
{
    if (s->count == 0) {
        return FALSE;
    }
    strcpy(e, s->top->data);
    return TRUE;
}

static int push(Stacks *s, char *e)
{
    Node *n = calloc(1, sizeof(Node));
    strcpy(n->data, e);
    n->next = (struct Node *)s->top;
    s->top = n;
    s->count ++;
    return TRUE;
}

static int pop(Stacks *s, char *e)
{
    if (s->count == 0) {
        return FALSE;
    }
    strcpy(e, s->top->data);
    Node *tmp = s->top;
    s->top = (Node *)tmp->next;
    free(tmp);
    s->count --;
    return TRUE;
}

static int length(Stacks *s)
{
    return s->count;
}

int main ()
{
    Stacks *s = calloc(1, sizeof(Stacks));

    init(s);
    empty(s);
    char e[10];
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
