#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
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

void sigchld_handler() {
    // waitpid() might overwrite errno, so we save and restore it:
    int saved_errno = errno;

    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;

    errno = saved_errno;
}

void register_sigchld_handler() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        handle_error("sigaction");
    }
}

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

int get_listener_socket() {
    struct addrinfo *list = get_addrinfo_list();
    int listener_socket;
    for (struct addrinfo *p = list; p != NULL; p = p->ai_next) {
        listener_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener_socket == -1) {
            handle_warning("socket");
            continue;
        }

        const int enable = 1;
        if (setsockopt(listener_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &enable, sizeof(int)) < 0)
            handle_error("setsockopt(SO_REUSEADDR) failed");

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

    freeaddrinfo(list);

    if (listener_socket == -1) {
        fprintf(stderr, "Couldn't find appropriate address info from getaddrinfo "
                        "result list\n");
        exit(EXIT_FAILURE);
    }

    return listener_socket;
}

int main(void) {
    int listener_socket = get_listener_socket();

    register_sigchld_handler();

    char buffer[100];
    struct sockaddr_storage client = {0};
    socklen_t sockaddr_size = sizeof client;
    while (1) {
        int connection_sockfd = accept(listener_socket, (struct sockaddr *) &client, &sockaddr_size);
        if (connection_sockfd == -1) {
            handle_warning("accept");
            continue;
        }

        if (!fork()) {
            close(listener_socket);
            ssize_t received = recv(connection_sockfd, buffer, sizeof buffer, 0);
            if (received == -1) {
                handle_warning("recv");
            }

            printf("received message from client - %s\n", buffer);

            ssize_t sent = send(connection_sockfd, buffer, sizeof buffer, 0);
            if (sent == -1) {
                handle_warning("send");
            }

            printf("sent message back to client - %s\n", buffer);

            if (close(connection_sockfd) == -1) {
                handle_warning("close");
            }
            exit(0);
        }
        close(connection_sockfd);
    }

    return 0;
}
