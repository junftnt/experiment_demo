#ifndef THREAD_H
#define THREAD_H

#ifndef HAVE_STDIO_H
#define HAVE_STDIO_H
#include <stdio.h>
#endif

#include "config.h"

void *thread_main(void *argv);

#endif  /*THREAD_H*/
