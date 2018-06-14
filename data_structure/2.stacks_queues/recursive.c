/*递归*/
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>

/*斐波那契数列-兔子问题*/
/*后面的数是前两个数之和*/
/*F(n) = F(n-1) + F(n-2)*/

/*循环*/
static int robot_cycle(int nums)
{
    int i;
    int robots[nums];
    robots[0] = 0;
    robots[1] = 1;

    for (i = 2; i < nums; i ++) {
        robots[i] = robots[i-1] + robots[i-2]; 
        printf("当前天数: %d, 当前兔子对数: %d \n", i, robots[i]);
    }
}
/*递归*/
static int robot_recursive_fun(int i)
{
    if (i < 2) {
        return i == 0 ? 0 : 1;
    } 
    return robot_recursive_fun(i - 1) + robot_recursive_fun(i - 2);
}
static int robot_recursive(int nums)
{
    int i;
    for (i = 0; i < nums; i ++) {
        printf("当前天数: %d, 当前兔子对数: %d \n", i, robot_recursive_fun(i));
    }
    
}


int main ()
{
    /*循环占用空间多一些，但是 递归时间长一些，不过递归是可以优化的，比如将每次计算的结果存储起来，而不是 每次都循环调用 前面所有的数据*/
    /*robot_cycle(40);*/
    robot_recursive(40);
    return 0;
}
