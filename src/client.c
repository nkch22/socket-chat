#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int get_connection_socket() {
    struct addrinfo *list = get_addrinfo_list();

    int connection_socket = -1;
    for (struct addrinfo* p = list; p != NULL; p = p->ai_next) {
        if ((connection_socket = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            handle_warning("socket");
            continue;
        }

        if (connect(connection_socket, p->ai_addr, p->ai_addrlen) == -1) {
            handle_warning("connect");
            continue;
        }

        break;
    }

    if (connection_socket == -1) {
        fprintf(stderr, "Could not connect to the server\n");
        exit(EXIT_FAILURE);
    }

    return connection_socket;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        fprintf(stderr, "USAGE: %s [message-text]\n", argv[0]);
        return EXIT_FAILURE;
    }

    char buffer[100];
    int connection = get_connection_socket();

    memcpy(buffer, argv[1], strlen(argv[1]) + 1);

    ssize_t sent = send(connection, buffer, sizeof buffer, 0);
    if (sent == -1) {
        handle_error("send");
    }

    printf("Sent to the server message - %s\n", buffer);

    ssize_t bytes = recv(connection, buffer, sizeof buffer, 0);
    if (bytes == -1) {
        handle_error("recv");
    }
    printf("Received message from the server - %s\n", buffer);

    if (close(connection) == -1) {
        handle_error("close");
    }

    return 0;
}
