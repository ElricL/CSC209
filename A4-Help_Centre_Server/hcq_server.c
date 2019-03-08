#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "hcq.h"

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 3
#define DELIM " \n"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


#ifndef PORT
  #define PORT 57644
#endif
#define MAX_BACKLOG 5
#define MAX_CONNECTIONS 24
#define BUF_SIZE 128

// Contains client information
struct sockname {
    int sock_fd;
    char *type; // If client is TA or Student
    char *name;
    int registered; //If client is registered as TA or in Queue as student
};

// Use global variables so we can have exactly one TA list and one student list
    Ta *ta_list = NULL;
    Student *stu_list = NULL;

    Course *courses;
    int num_courses = 3;

int accept_connection(int fd, struct sockname *clients) {
    struct sockaddr_in client_addr;
    unsigned int client_len = sizeof(struct sockaddr_in);
    client_addr.sin_family = AF_INET;

    int index = 0;
    while (index < MAX_CONNECTIONS && clients[index].sock_fd != -1) {
        index++;
    }

    int client_fd = accept(fd, (struct sockaddr *)&client_addr,
                           &client_len);
    if (client_fd < 0) {
        perror("server: accept");
        close(fd);
        exit(1);
    }
    if (index == MAX_CONNECTIONS) {
        fprintf(stderr, "server: max concurrent connections\n");
        close(client_fd);
        return -1;
    }
    dprintf(client_fd, "Welcome to the Help Centre, what is your name?\n");
    clients[index].sock_fd = client_fd;

    return client_fd;

}

/*Search the first inbuf characters of buf for a network newline ("\r\n").
  Return the location of the '\r' if the network newline is found,
  or -1 otherwise.
  Definitely do not use strchr or any other string function in here. (Why not?)
*/

int find_network_newline(const char *buf, int inbuf) {
  // Step 1: write this function
  int i = 0;
  while (i < inbuf) {
    if (buf[i] == '\r' && buf[i+1] == '\n') {
      return i;
    }
    i++;
  }
  return -1; // return the location of '\r' if found
}

/* Read a message from client_index and echo it back to them.
 * Return the fd if it has been closed or 0 otherwise.
 */
int read_from(int client_index, struct sockname *clients, fd_set *fs, int max_fd) {
    int client_fd = clients[client_index].sock_fd;
    // Buf for messages to be sent to client
    char buf[BUF_SIZE + 1];
    buf[BUF_SIZE] = '\0';

    // temp for command inputs from client
    char temp[BUF_SIZE + 1] = {'\0'};
    int inbuf = 0;          // buffer is empty; has no bytes
    int room = sizeof(temp); // room == capacity of the whole buffer
    char *after = temp;        // start writing at beginning of buf

    int num_read;
    //Make sure all clients inputs are read
    if ((num_read = read(client_fd, after, room)) > 0) {
        inbuf += num_read;
		int where = find_network_newline(temp, inbuf);
        if (where >= 0) {
          temp[where] = '\n';
          temp[where + 1] = '\0';
          inbuf = inbuf - where - 2;
          memmove(temp, temp + where + 2, inbuf);

        }
        room = sizeof(temp) - inbuf;
        after = temp + inbuf;
    }

    if (num_read > 0) {
        // check if there is a new client
        if (clients[client_index].name == NULL && client_fd > 0) {
            clients[client_index].name = malloc(31);
            strcpy(clients[client_index].name, temp);
            clients[client_index].name[num_read] = '\0';
            if (clients[client_index].name[num_read - 1] == '\n'){
                clients[client_index].name[num_read - 1] = '\0';
            }
            strcpy(buf, "Are you a TA or a Student (enter T or S)?\n");
        // Receive Type from client
        } else if (clients[client_index].registered == 0 && clients[client_index].type == NULL ) {
            if (strcmp(temp, "T") == 0) {
                clients[client_index].registered = 1;
                strcpy(buf, "Valid commands for TA: stats, next, (or use Ctrl-C to leave) \n");
                clients[client_index].type = malloc(2);
                strcpy(clients[client_index].type, "T");
                clients[client_index].type[1] = '\0';
                add_ta(&ta_list, clients[client_index].name);
            } else if (strcmp(temp, "S") == 0) {
                strcpy(buf, "Valid courses: CSC108, CSC148, CSC209. ");
                strcat(buf, "Which course are you asking about?\n");
                clients[client_index].type = malloc(2);
                strcpy(clients[client_index].type, "S");
                clients[client_index].type[1] = '\0';
            } else if (strcmp(temp, "T") != 0 && strcmp(temp, "S") != 0) {
                strcpy(buf, "Invalid role (enter T or S)\n");
            }
        // Student still needs to add Course to be registered into queue
        } else if (clients[client_index].registered == 0 && strcmp(clients[client_index].type, "S") == 0) {
            int result = add_student(&stu_list, clients[client_index].name, temp, courses, 3);
            if (result == 1) {
                strcpy(buf, "You are already in the queue and cannot ");
                strcat(buf, "be added again for any course. Good-bye.\n");
                if (FD_ISSET(client_fd, fs)) {
                    if (num_read == 0 || write(client_fd, buf, strlen(buf)) != strlen(buf)) {
                        clients[client_index].sock_fd = -1;
                        return client_fd;
                    }
                }
                return client_fd;
            } else if (result == 2) {
                strcpy(buf, "This is not a valid course. Good-bye.\n");
                if (FD_ISSET(client_fd, fs)) {
                    if (num_read == 0 || write(client_fd, buf, strlen(buf)) != strlen(buf)) {
                        clients[client_index].sock_fd = -1;
                        return client_fd;
                    }
                }
                return client_fd;
            }
            strcpy(buf, "You have been entered into the queue. While you wait, ");
            strcat(buf,  "you can use the command stats to see which TAs are ");
            strcat(buf, "currently serving students.\n");
            clients[client_index].registered = 1;
        // Commands for student in queue
        } else if (strcmp(clients[client_index].type, "S") == 0
                   && clients[client_index].registered == 1) {
            if (strcmp(temp, "stats") == 0) {
                strcpy(buf, print_currently_serving(ta_list));
            } else {
                strcpy(buf, "Incorrect syntax");
            }
        // Commands for TA
        } else if (strcmp(clients[client_index].type, "T") == 0) {
            if (strcmp(temp, "stats") == 0) {
                strcpy(buf, print_full_queue(stu_list));
            } else if (strcmp(temp, "next") == 0) {
                Ta *client = find_ta(ta_list, clients[client_index].name);
                if (client->current_student != NULL) {
                    next_overall(clients[client_index].name, &ta_list, &stu_list);
                    int stu_index;
                    for (stu_index = 0; stu_index  < MAX_CONNECTIONS; stu_index++) {
                        if (strcmp(clients[stu_index].name, client->current_student->name) == 0
                            && strcmp(clients[client_index].type, "S") == 0) {
                                return clients[stu_index].sock_fd;
                            }
                    }
                }
            } else {
                strcpy(buf, "Incorrect syntax\n");
            }
        }
    }

    if (FD_ISSET(client_fd, fs)) {
            if (num_read == 0 || write(client_fd, buf, strlen(buf)) != strlen(buf)) {
                clients[client_index].sock_fd = -1;
                return client_fd;
            }
    }


    return 0;
}

int main(void) {
    struct sockname clients[MAX_CONNECTIONS];
    for (int index = 0; index < MAX_CONNECTIONS; index++) {
        clients[index].sock_fd = -1;
        clients[index].name = NULL;
        clients[index].type = NULL;
        clients[index].registered = 0;
    }

    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("server: socket");
        exit(1);
    }

    // Set information about the port (and IP) we want to be connected to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    memset(&server.sin_zero, 0, 8);
    server.sin_addr.s_addr = INADDR_ANY;

    // recycle port right away
    int on = 1;
    int status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &on, sizeof(on));
    if (status == -1) {
            perror("setsockopt -- REUSEADDR");
    }

    // Bind the selected port to the socket.
    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("server: bind");
        close(sock_fd);
        exit(1);
    }

    // Announce willingness to accept connections on this socket.
    if (listen(sock_fd, MAX_BACKLOG) < 0) {
        perror("server: listen");
        close(sock_fd);
        exit(1);
    }

    // Setup Help Centre Courses
    if ((courses = malloc(sizeof(Course) * 3)) == NULL) {
        perror("malloc for course list\n");
        exit(1);
    }
    strcpy(courses[0].code, "CSC108");
    strcpy(courses[1].code, "CSC148");
    strcpy(courses[2].code, "CSC209");

    // Prepare to listen to multiple
    // file descriptors by initializing a set of file descriptors.
    int max_fd = sock_fd;
    fd_set all_fds, listen_fds;
    FD_ZERO(&all_fds);
    FD_SET(sock_fd, &all_fds);

    while (1) {
        // select updates the fd_set it receives
        listen_fds = all_fds;
        int nready = select(max_fd + 1, &listen_fds, NULL, NULL, NULL);
        if (nready == -1) {
            perror("server: select");
            exit(1);
        }

        //  Accept new connections
        if (FD_ISSET(sock_fd, &listen_fds)) {
            int client_fd = accept_connection(sock_fd, clients);
            if (client_fd > max_fd) {
                max_fd = client_fd;
            }
            FD_SET(client_fd, &all_fds);
            printf("Accepted connection\n");
        }

        // Next, check the clients inputs.
        for (int index = 0; index < MAX_CONNECTIONS; index++) {
            if (clients[index].sock_fd > -1 && FD_ISSET(clients[index].sock_fd, &listen_fds)) {
                int client_closed = read_from(index, clients, &all_fds, max_fd);
                if (client_closed > 0) {
                    free(clients[index].name);
                    free(clients[index].type);
                    clients[index].type = NULL;
                    clients[index].name = NULL;
                    FD_CLR(client_closed, &all_fds);
                    clients[index].sock_fd = -1;
                    printf("Client %d disconnected\n", client_closed);
                } else {
                    printf("Received command from client %d\n", clients[index].sock_fd);
                }
            }
        }
    }
    return 1;
}
