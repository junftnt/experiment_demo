/*逆波兰法 - 链栈 实现个位数四目运算表达式*/

/*一、表达式转换成 逆波兰法表达式(中缀表达式), 动画效果如下: http://www.web-lovers.com/assets/bimg/reverse_poland_expression.gif*/
    /*从中缀式的左端开始逐个读取字符x, 逐序进行如下步骤:*/
    /*1.若x是操作数, 则分析出完整的运算数 (在这里为方便, 用字母代替数字), 将x直接压入栈s2*/
    /*2.若x是运算符, 则分情况讨论:*/
        /*若x是'(', 则直接压入栈s1*/
        /*若x是')', 则将距离栈s1栈顶的最近的'('之间的运算符, 逐个出栈, 依次压入栈s2, 此时抛弃'('*/
        /*若x是除'('和')'外的运算符, 则再分如下情况讨论*/
        /*若当前栈s1的栈顶元素为'(', 则将x直接压入栈s1*/
        /*若当前栈s1的栈顶元素不为'(', 则将x与栈s1的栈顶元素比较, 若x的优先级大于栈s1栈顶运算符优先级, 则将x直接压入栈s1.否者, 将栈s1的栈顶运算符弹出, 压入栈s2中, 直到栈s1的栈顶运算符优先级别低于(不包括等于) x的优先级, 或栈s2的栈顶运算符为'(', 此时再则将x压入栈s1*/
    /*3.在进行完(2)后, 检查栈s1是否为空, 若不为空, 则将栈中元素依次弹出并压入栈s2中(不包括'#')*/
    /*4.完成上述步骤后, 栈s2便为逆波兰式输出结果. 但是栈s2应做一下逆序处理, 因为此时表达式的首字符位于栈底*/

/*二、逆波兰表达式计算, 动画效果如下: http://www.web-lovers.com/assets/bimg/reverse_poland_calc.gif*/
    /*1.遍历逆波兰表达式的元素, 如果是数字依次压入栈中, 如果是操作符号( + - * /), 则进行下面操作*/
    /*2.从栈中弹出两个操作数 num1 和 num2, 注意后弹出的 num2 作为操作数, num1 作为被操作数, 比如 num2 - num1 */
    /*3.将刚才 num1 和 num2 的计算结果 num3 压入栈中*/
    /*4.然后重复进行1 的操作, 继续遍历, 如果是数字继续压入栈中, 如果是操作符号则进入2*/

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>


#define TRUE        1
#define FALSE       -9999
#define EXPRESS_LEN 50

/*结点*/
typedef struct {
    int data;
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

static int top(Stacks *s)
{
    if (s->count == 0) {
        return FALSE;
    }
    return s->top->data;
}

static int push(Stacks *s, int e)
{
    Node *n = calloc(1, sizeof(Node));
    n->data = e;
    n->next = (struct Node *)s->top;
    s->top = n;
    s->count ++;
    return TRUE;
}

static int pop(Stacks *s)
{
    if (s->count == 0) {
        return FALSE;
    }
    int e = s->top->data;
    Node *tmp = s->top;
    s->top = (Node *)tmp->next;
    free(tmp);
    s->count --;
    return e;
}

static int length(Stacks *s)
{
    return s->count;
}

/*优先级*/
static int priority(int e)
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

static void showLinked(Stacks *s)
{
    Node *tmp = s->top;
    while (tmp) {
        /*printf("data: %c , next: %p \n", tmp->data, (void *)tmp->next);*/
        if (priority(tmp->data + '0') == -1) {
            printf("%d ", tmp->data);
        } else {
            printf("%c ", tmp->data + '0');
        }
        tmp = (Node *)tmp->next;
    }
}

/*组装表达式*/
static int initExpress(Stacks *o_s, Stacks *n_s, char *o_express)
{
    printf("\n\ninput: %s \n\n", o_express);
    char *ptr = o_express;
    int k = 0;
    while (*ptr != '\0') {
        if (*ptr != ' ') {
            int pri = priority(*ptr);
            if (pri == -1) {
            /*数字*/
                push(n_s, *ptr - '0');
            } else if (pri == 0){
            /*( 左括号*/
                push(o_s, *ptr - '0');
            } else if (pri == 3) {
            /*) 右括号*/
                k = top(o_s);
                while (k != FALSE && priority(k + '0') != 0) {
                    k = pop(o_s); 
                    if (priority(k + '0') != 0) {
                        push(n_s, k);
                    }
                }
            } else {
            /*+ - 加减符号*/
                k = top(o_s);
                while (k != FALSE && pri < priority(k + '0')) {
                    k = pop(o_s); 
                    push(n_s, k);
                }
                push(o_s, *ptr - '0');
            }
        }
        ptr ++;
    }
    if (o_s->count != 0) {
        Node *tmp = o_s->top;
        while (tmp) {
            push(n_s, tmp->data);
            tmp = (Node *)tmp->next;
        }
    }
    printf("n_s: ");
    showLinked(n_s);
    printf("\n");
}

/*翻转表达式*/
static void flipExpress(Stacks *f_s, Stacks *n_s)
{
    int k = 0;
    while (n_s->count > 0) {
        k = pop(n_s);
        push(f_s, k);
    }
    printf("flip express: ");
    showLinked(f_s);
    printf("\n");
}

static int calcu(Stacks *f_s, Stacks *j_s)
{
    int k = 0, b = 0;
    while (f_s->count > 0) {
        b = pop(f_s);
        int pri = priority(b + '0');
        if (pri == -1) {
            push(j_s, b);
        } else {
            k = pop(j_s);
            int x = k;
            k = pop(j_s);
            int y = k;
            int z;
            switch (b + '0') {
                case '+':
                    z = y + x;
                    break;
                case '-':
                    z = y - x;
                    break;
                case '*':
                    z = y * x;
                    break;
                case '/':
                    z = y / x;
                    break;
            } 
            push(j_s, z);
        }
        printf("\n");
        printf("j_s: ");
        showLinked(j_s);
        printf("\n");
        printf("f_s: ");
        showLinked(f_s);
        printf("\n");
    }

    k = top(j_s);
    return k;
}

int main ()
{
    Stacks *o_s = calloc(1, sizeof(Stacks));
    Stacks *n_s = calloc(1, sizeof(Stacks));
    init(o_s);
    init(n_s);

    char o_express[EXPRESS_LEN];
    memset(o_express, 0, EXPRESS_LEN);

    printf("please input your express: ");
    /*fgets(o_express, EXPRESS_LEN, stdin);*/
    strcpy(o_express, " 9 + ( 4 -1 ) * 3 + 8 / 2 ");
    
    /*组装表达式*/
    initExpress(o_s, n_s, o_express);
    free(o_s);

    Stacks *f_s = calloc(1, sizeof(Stacks));
    init(f_s);
    /*翻转表达式*/
    flipExpress(f_s, n_s);
    free(n_s);

    Stacks *j_s = calloc(1, sizeof(Stacks));
    init(j_s);
    int result = calcu(f_s, j_s);

    printf("\n=============================\n\nresult: %d \n\n", result);
    return 0;
}
