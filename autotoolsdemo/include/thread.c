#include "thread.h"

int num = 0;

void *thread_main(void *argv)
{
    printf("this pthread num is: %d \n", num ++); 
}
