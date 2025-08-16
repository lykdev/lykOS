#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SOCKET_PATH "/sys/testsock"

int main()
{
    printf("C: Client starting...\n");

    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
    {
        printf("C: socket\n");
        exit(1);
    }
    printf("C: Socket created.\n");

    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);
    printf("C: Socket address set to %s.\n", SOCKET_PATH);

    while (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
        ;

    printf("C: Connected to server!\n");

    return 0;
}
