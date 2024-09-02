#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT "5555"
#define BACKLOG_SIZE 10

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
    if ((status = getaddrinfo(NULL, PORT, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        exit(EXIT_FAILURE);
    }
    return res;
}

int get_listener_socket(struct addrinfo *list) {
    int listener_socket;
    for (struct addrinfo *p = list; p != NULL; p = p->ai_next) {
        listener_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener_socket == -1) {
            handle_warning("socket");
            continue;
        }

        socklen_t addrlen = sizeof *p->ai_addr;
        if (bind(listener_socket, p->ai_addr, addrlen) != 0) {
            handle_warning("bind");
            continue;
        }

        if (listen(listener_socket, BACKLOG_SIZE) != 0) {
            handle_warning("listen");
            continue;
        }

        break;
    }

    if (listener_socket == -1) {
        fprintf(stderr, "Couldn't find appropriate address info from getaddrinfo "
                        "result list\n");
        exit(EXIT_FAILURE);
    }
    return listener_socket;
}

int main(void) {
    struct addrinfo *res = get_addrinfo_list();
    int listener_socket = get_listener_socket(res);

    // listen first working socket
    struct sockaddr_storage sockaddr = {0};
    socklen_t sockaddr_size = sizeof sockaddr;
    char message[] = "Hello, world!\n";
    while (1) {
        int connection_sockfd = accept(listener_socket, (struct sockaddr *) &sockaddr, &sockaddr_size);
        if (connection_sockfd == -1) {
            handle_warning("accept");
            continue;
        }

        ssize_t sent = send(connection_sockfd, message, sizeof message, 0);
        if (sent == -1) {
            handle_warning("send");
        }

        if (close(connection_sockfd) == -1) {
            handle_warning("close");
        }
    }

    return 0;
}
