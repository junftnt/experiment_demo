/*链栈 实现四目运算表达式*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE        1
#define FALSE       -1
#define EXPRESS_LEN 256
#define CHAR_LEN    1

/*结点*/
typedef struct {
    char data[CHAR_LEN];
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

static void clear(Stacks *s)
{
    char e[CHAR_LEN];
    while (!empty(s)) {
        pop(s, e);
    } 
}

static int length(Stacks *s)
{
    return s->count;
}

/*优先级*/
static int priority(char e)
{
    switch (e) {
        case '+':
        case '-':
            return 1;
            break;
        case '*':
        case '/':
            return 2;
            break;
        case '(':
            return 0;
            break;
        case ')':
            return 3;
            break;
        default:
            return -1;
    } 
}

int main ()
{
    Stacks *n_s = calloc(1, sizeof(Stacks));
    init(n_s);

    char o_express[EXPRESS_LEN], n_express[EXPRESS_LEN], k[CHAR_LEN];
    memset(o_express, 0, EXPRESS_LEN);
    memset(n_express, 0, EXPRESS_LEN);
    memset(k, 0, CHAR_LEN);

    printf("please input your express: ");
    /*fgets(o_express, EXPRESS_LEN, stdin);*/
    strcpy(o_express, " 9 + 2 * ( 3 - 1) ");
    
    printf("\n\ninput: %s \n", o_express);
    int i, n = 0, len = strlen(o_express);
    for (i = 0; i < len; i ++) {
        if (o_express[i] != ' ' && o_express[i] != '\0') {
            int pri = priority(o_express[i]);
            printf("express %d: %c \n", pri, o_express[i]);
        }
    }

    clear(n_s);
    init(n_s);

    return 0;
}
