#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define SERVER_PORT "5555"

#define handle_error(msg)                                                                                              \
    do {                                                                                                               \
        perror(msg);                                                                                                   \
        exit(EXIT_SUCCESS);                                                                                            \
    } while (0)

#define handle_warning(msg)                                                                                            \
    do {                                                                                                               \
        perror(msg);                                                                                                   \
    } while (0)

struct addrinfo *get_addrinfo_list() {
    struct addrinfo hints = {0};

    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    struct addrinfo *res;

    if ((status = getaddrinfo(NULL, SERVER_PORT, &hints, &res)) != 0) {
        handle_error("getaddrinfo");
    }
    return res;
}

int main(void) { return 0; }
