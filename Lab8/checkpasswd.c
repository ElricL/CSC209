#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 256
#define MAX_PASSWORD 10

#define SUCCESS "Password verified\n"
#define INVALID "Invalid password\n"
#define NO_USER "No such user\n"

int main(void) {
    char userid[MAXLINE];
    char password[MAXLINE];

    /* Read a user id and password from stdin */
    printf("User id:\n");
    if((fgets(userid, MAXLINE, stdin)) == NULL) {
        fprintf(stderr, "Could not read from stdin\n");
        exit(1);
    }
    printf("Password:\n");
    if((fgets(password, MAXLINE, stdin)) == NULL) {
        fprintf(stderr, "Could not read from stdin\n");
        exit(1);
    }
    char *ptr;
    if((ptr = strchr(userid, '\n')) == NULL) {
        userid[MAX_PASSWORD - 1] = '\0';
    } else {
        *ptr = '\0';
    }

    int pfd[2];

    if(pipe(pfd) == -1) {
        perror("pipe");
        exit(1);
    }

    switch (fork()) {
    case -1:
        perror("fork");
        exit(1);

    case 0:   /* Child process for executing ./validate. */
        if (close(pfd[1]) == -1) {
            perror("close pfd_1 child");
            exit(1);
        }
        if (pfd[0] !=  fileno(stdin)) {
            if (dup2(pfd[0],  fileno(stdin)) == -1) {
                perror("dup2");
                exit(1);
            }
            if (close(pfd[0]) == -1) {
                perror("close pfd_0 child");
                exit(1);
            }
        }
        /* Read from pipe */
        execlp("./validate", "validate", NULL);
        perror("execl ./validate");
        exit(1);

     default:    /* Parent process */
         if (close(pfd[0]) == -1) {
             perror("close pfd_0 parent");
             exit(1);
         }
         write(pfd[1], userid, MAX_PASSWORD);
         write(pfd[1], password, MAX_PASSWORD);

         if (close(pfd[1]) == -1) {
             perror("close pdf_1 parent");
             exit(1);
         }

        int status, exit_status;
        if(wait(&status) != -1) {
          if(WIFEXITED(status)) {
              exit_status = WEXITSTATUS(status);
              switch(exit_status) {
                  case 1:
                      printf("Error occured!!\n");
                      break;
                  case 2:
                      printf("Invalid password\n");
                      break;
                  case 3:
                      printf("No such user\n");
                      break;
                  default:
                      printf("Password verified\n");
                      break;
              }
          } else {
              printf("[%d] Child exited abnormally\n", getpid());
          }
        }
    }

    return 0;
}
