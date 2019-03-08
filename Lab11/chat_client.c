#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef PORT
  #define PORT 30000
#endif
#define BUF_SIZE 128

int main(void) {
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("client: socket");
        exit(1);
    }

    // Set the IP and port of the server to connect to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server.sin_addr) < 1) {
        perror("client: inet_pton");
        close(sock_fd);
        exit(1);
    }

    // Connect to the server.
    if (connect(sock_fd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        perror("client: connect");
        close(sock_fd);
        exit(1);
    }

    printf("%s\n", "Please enter username:");
    char get_user_name[100];
    int check_read = read(STDIN_FILENO, get_user_name, 100);
    get_user_name[check_read - 1] = '\0';
    int num_written= write(sock_fd, get_user_name, check_read);
    if (num_written != check_read) {
        perror("username not written");
        close(sock_fd);
        exit(1);
    }

    // init range of all file descriptors and sets for file descriptors
    int maxfd;
    fd_set fds, listen_fds;

    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    FD_SET(sock_fd, &fds);

    // STDIN_FILENO == 0 /* Standard input. */
    // STDOUT_FILENO == 1 /* Standard output. */
    // STDERR_FILENO == 2 /* Standard error output. */
    if (STDIN_FILENO > sock_fd) {
        maxfd = STDIN_FILENO;
    } else {
        maxfd = sock_fd;
    }

    // Read input from the user, send it to the server, and then accept the
    // echo that returns. Exit when stdin is closed.
    char buf[BUF_SIZE + 1];
    while (1) {
        listen_fds = fds;

        // nready observe listen_fds's change
        int nready = select(maxfd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }

        // init number of read
        int num_read;

        // check if fd is ready
        if (FD_ISSET(STDIN_FILENO, &listen_fds)) {
            num_read = read(STDIN_FILENO, buf, BUF_SIZE);
            if (num_read == 0) {
                break;
            }
            buf[num_read] = '\0';         // Just because I'm paranoid

            int num_written = write(sock_fd, buf, num_read);
            if (num_written != num_read) {
                perror("client: write");
                close(sock_fd);
                exit(1);
            }
        }
        if (FD_ISSET(sock_fd, &listen_fds)) {
            num_read = read(sock_fd, buf, BUF_SIZE);
            buf[num_read] = '\0';
            printf("Received from server: %s", buf);
        }
    }

    close(sock_fd);
    return 0;
}
