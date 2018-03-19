/*守护进程实现*/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

int daemonize ()
{
    int fd;

    switch (fork()) {
        case -1:
            return -1;
        case 0:
            break;
        default:
            exit(0);
    }

    if (setsid() == -1) {
        return -1;
    }

    if (chdir("/tmp") != 0) {
        perror("chdir error");
        return -1;
    }
    
    if ((fd = open("/dev/null", O_RDWR, 0)) != -1) {
        if (dup2(fd, STDIN_FILENO) < 0) {
            perror("dup2 stdin");
            return -1;
        } 

        if (dup2(fd, STDOUT_FILENO) < 0) {
            perror("dup2 stdout");
            return -1;
        } 

        if (dup2(fd, STDERR_FILENO) < 0) {
            perror("dup2 stderr");
            return -1;
        } 

        if (fd > STDERR_FILENO) {
            if (close(fd)) {
                perror("close error");
                return -1;
            }
        }
    }

    return 0;
}

int main ()
{
    int fp;
    char *datetime;
    time_t t;

    if (daemonize() < 0) {
        printf("daemonize faild \n");
        exit(0);
    }

    while (1) {
        if ((fp = open("/tmp/daemonize.log", O_CREAT | O_WRONLY | O_APPEND, 0600)) >= 0) {
            t = time(0);
            datetime = asctime(localtime(&t));
            write(fp, datetime, strlen(datetime));
            close(fp);
        }
        sleep(3);
    }
    
    return 0;
}
