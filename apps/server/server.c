#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define SOCKET_PATH "/sys/testsock"

int main()
{
    printf("S: Server starting...\n");

    int fd;
    struct sockaddr_un addr;

    fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == -1)
    {
        printf("S: socket\n");
        exit(1);
    }
    printf("S: Socket created.\n");

    addr.sun_family = AF_UNIX;
    snprintf(addr.sun_path, sizeof(addr.sun_path), "%s", SOCKET_PATH);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        printf("S: bind\n");
        exit(1);
    }
    printf("S: Socket bound to %s.\n", SOCKET_PATH);

    if (listen(fd, 1) == -1)
    {
        printf("S: listen\n");
        exit(1);
    }
    printf("S: Listening for connections...\n");

    int cfd = accept(fd, NULL, NULL);
    if (cfd == -1)
    {
        printf("S: accept\n");
        exit(1);
    }
    printf("S: Client connected!\n");

    return 0;
}
