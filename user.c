#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define handle_error(msg)   \
    do {                    \
        perror(msg);        \
        exit(EXIT_FAILURE); \
    } while (0)

#define VPOLL_IOC_MAGIC '^'
#define VPOLL_IO_ADDEVENTS _IO(VPOLL_IOC_MAGIC, 1)
#define VPOLL_IO_DELEVENTS _IO(VPOLL_IOC_MAGIC, 2)

int main(int argc, char *argv[])
{
    struct epoll_event ev = {
        .events =
            EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLOUT | EPOLLHUP | EPOLLPRI,
        .data.u64 = 0,
    };

    int efd = open("/dev/vpoll", O_RDWR | O_CLOEXEC);
    if (efd == -1)
        handle_error("/dev/vpoll");
    int epollfd = epoll_create1(EPOLL_CLOEXEC);
    if (efd == -1)
        handle_error("epoll_create1");
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, efd, &ev) == -1)
        handle_error("epoll_ctl");

    switch (fork()) {
    case 0: // child process
    #if 0
        sleep(1);
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLIN); // 0x01
        sleep(1);
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLIN); // 0x01
        sleep(1);
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLIN | EPOLLPRI); // 0x03
        sleep(1);
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLPRI); // 0x02
        sleep(1);
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLOUT); // 0x04
        sleep(1);
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLHUP); //0x10
    #else
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLIN); // 0x01
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLIN); // 0x01
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLIN | EPOLLPRI); // 0x03
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLPRI); // 0x02
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLOUT); // 0x04
        ioctl(efd, VPOLL_IO_ADDEVENTS, EPOLLHUP); //0x10
    #endif
        exit(EXIT_SUCCESS);
    default:
        while (1) {
            int nfds = epoll_wait(epollfd, &ev, 1, 1000);
            if (nfds < 0)
                handle_error("epoll_wait");
            else if (nfds == 0)
                printf("timeout...\n");// 0x03
            else {
                printf("(%d)GOT event %x\n",nfds , ev.events);
                // ioctl(efd, VPOLL_IO_DELEVENTS, ev.events);
                if (ev.events & EPOLLHUP)
                    break;
            }
        }
        break;
    case -1: /* should not happen */
        handle_error("fork");
    }

    close(epollfd);
    close(efd);
    return 0;
}