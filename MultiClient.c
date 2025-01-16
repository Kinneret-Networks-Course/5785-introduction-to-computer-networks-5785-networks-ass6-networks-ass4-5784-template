/**
Assignment 6 Multiclient code
Introduction to Computer Networks
Semester 1 5785

Michael J. May
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

// Parses the address list into an array of strings
// input - the input line comma delimited (e.g. 12.12.12.12,56.56.56.56,1.1.1.1
// addresses - the output address list
// count - the number of addresses found
// No return value
void parse_addresses(const char *input, char ***addresses, int *count) {
    char *token;
    char *input_copy = strdup(input);
    char *rest = input_copy;

    *count = 0;
    *addresses = NULL;

    while ((token = strtok_r(rest, ",", &rest))) {
        *addresses = realloc(*addresses, sizeof(char *) * (*count + 1));
        (*addresses)[*count] = strdup(token);
        (*count)++;
    }
    free(input_copy);
}

// Frees the list of addresses allocated using new
// addresses - the address list
// count - the number of addresses in the list
void free_addresses(char **addresses, int count) {
    for (int i = 0; i < count; i++) {
        free(addresses[i]);
    }
    free(addresses);
}

// Reads up to the maximum length from the network socket until it hits a new line
// sock - the socket to read from
// buffer - where to put the bytes read
// maxlen - the maximum number of bytes to read (the buffer's length)
// return value - the number of bytes read
ssize_t read_line(int sock, char *buffer, size_t maxlen) {
    ssize_t n, rc;
    char c;

    for (n = 0; n < maxlen - 1; n++) {
        if ((rc = read(sock, &c, 1)) == 1) {
            buffer[n] = c;
            if (c == '\n') {
                n++;
                break;
            }
        } else if (rc == 0) {
            if (n == 0)
                return 0; // EOF, no data read
            else
                break; // EOF, some data was read
        } else {
            return -1; // error
        }
    }

    buffer[n] = '\0';
    return n;
}

// Runs the program.  Opens three sockets and sends the addresses from each list on each socket, in turns.  First word on first socket, first word on second socket, first word on third socket.  Second word on first socket, secord word on second socket, third word on third socket.  Etc.
// Outputs the response from the sockets to STDOUT
int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Usage: %s <IP> <Port> <List1> <List2> <List3>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *ip = argv[1];
    int port = atoi(argv[2]);
    char **addresses1, **addresses2, **addresses3;
    int count1, count2, count3;

    parse_addresses(argv[3], &addresses1, &count1);
    parse_addresses(argv[4], &addresses2, &count2);
    parse_addresses(argv[5], &addresses3, &count3);

    int max_count = count1 > count2 ? (count1 > count3 ? count1 : count3) : (count2 > count3 ? count2 : count3);

    int sockets[3];
    struct sockaddr_in server_addr;

    for (int i = 0; i < 3; i++) {
        if ((sockets[i] = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation error");
            exit(EXIT_FAILURE);
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
            perror("Invalid address/Address not supported");
            exit(EXIT_FAILURE);
        }

        if (connect(sockets[i], (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
            perror("Connection failed");
            exit(EXIT_FAILURE);
        }
    }

    char buffer[BUFFER_SIZE];

    for (int i = 0; i < max_count; i++) {
        if (i < count1) {
	    char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "%s\n", addresses1[i]);
            if (send(sockets[0], msg, strlen(msg), 0) < 0) {
                perror("Send error on socket 1");
            }
            ssize_t valread = read_line(sockets[0], buffer, BUFFER_SIZE);
            if (valread < 0) {
                perror("Read error on socket 1");
            } else {
                buffer[valread] = '\0';
                printf("Socket 1 Response: %s -> %s", addresses1[i], buffer);
            }
        }
        if (i < count2) {
            char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "%s\n", addresses2[i]);
            if (send(sockets[1], msg, strlen(msg), 0) < 0) {
                perror("Send error on socket 2");
            }
            ssize_t valread = read_line(sockets[1], buffer, BUFFER_SIZE);
            if (valread < 0) {
                perror("Read error on socket 2");
            } else {
                buffer[valread] = '\0';
                printf("Socket 2 Response: %s -> %s", addresses2[i], buffer);
            }
        }
        if (i < count3) {
			char msg[BUFFER_SIZE];
            snprintf(msg, sizeof(msg), "%s\n", addresses3[i]);
            if (send(sockets[2], msg, strlen(msg), 0) < 0) {
                perror("Send error on socket 3");
            }
            ssize_t valread = read_line(sockets[2], buffer, BUFFER_SIZE);
            if (valread < 0) {
                perror("Read error on socket 3");
            } else {
                buffer[valread] = '\0';
                printf("Socket 3 Response: %s -> %s", addresses3[i], buffer);
            }
        }
    }

    for (int i = 0; i < 3; i++) {
        close(sockets[i]);
    }

    free_addresses(addresses1, count1);
    free_addresses(addresses2, count2);
    free_addresses(addresses3, count3);

    return 0;
}
