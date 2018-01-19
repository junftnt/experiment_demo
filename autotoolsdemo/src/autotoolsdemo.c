#include "autotoolsdemo.h"

int main ()
{
    int i;

    pthread_t threads[THREAD_NUMS];

    for (i = 0; i < THREAD_NUMS; i ++) {
        if (pthread_create(&threads[i], NULL, &thread_main, NULL) != 0) {
            fprintf(stderr, "create pthread error: %s \n", strerror(errno));
            exit(1);
        }
    }

    for (i = 0; i < THREAD_NUMS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
