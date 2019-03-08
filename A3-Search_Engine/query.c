#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* A program that calls multiple run_workers on each MAXWORKER amount of
* subdirectories and prints out frequency records of word given by user.
* User can enter as many amount of words until CTRL+D is pressed.
*/

/*compares two Freqrecords and determines which one is higher
* based on the frequencies or filename if frequencies are equal.
* - Returns negative if first Freqrecord is greater
* - Returns positive if first Freqrecord is smaller
*/
int compare(const void *s1, const void *s2) {
    FreqRecord *e1 = (FreqRecord *)s1;
    FreqRecord *e2 = (FreqRecord *)s2;
    int freqcomp = e1->freq - e2->freq;
    if (freqcomp == 0)  /* same frequency so sort by id */
        return strcmp(e1->filename, e2->filename);
    else
        return -freqcomp;
}

int main(int argc, char **argv) {
    char ch;
    char path[PATHLENGTH];
    char *startdir = ".";

    /* this models using getopt to process command-line flags and arguments */
    while ((ch = getopt(argc, argv, "d:")) != -1) {
        switch (ch) {
        case 'd':
            startdir = optarg;
            break;
        default:
            fprintf(stderr, "Usage: queryone [-d DIRECTORY_NAME]\n");
            exit(1);
        }
    }

    // Open the directory provided by the user (or current working directory)
    DIR *dirp;
    if ((dirp = opendir(startdir)) == NULL) {
        perror("opendir");
        exit(1);
    }

    /* For each entry in the directory, eliminate . and .., and check
     * to make sure that the entry is a directory, then call run_worker
     * to process the index file contained in the directory.
     * Note that this implementation of the query engine iterates
     * sequentially through the directories, and will expect to read
     * a word from standard input for each index it checks.
     */
    struct dirent *dp;

    int num_sub; // Number of subdirectories to be accumulated
    char dir_paths[MAXWORKERS][PATHLENGTH]; // array of subdirectory paths

    while ((dp = readdir(dirp)) != NULL) {
        if (strcmp(dp->d_name, ".") == 0 ||
            strcmp(dp->d_name, "..") == 0 ||
            strcmp(dp->d_name, ".svn") == 0 ||
            strcmp(dp->d_name, ".git") == 0) {
                continue;
        }

        strncpy(path, startdir, PATHLENGTH);
        strncat(path, "/", PATHLENGTH - strlen(path));
        strncat(path, dp->d_name, PATHLENGTH - strlen(path));
        path[PATHLENGTH - 1] = '\0';

        struct stat sbuf;
        if (stat(path, &sbuf) == -1) {
            // This should only fail if we got the path wrong
            // or we don't have permissions on this entry.
            perror("stat");
            exit(1);
        }

        // if directory add directory path to dir_paths
        if (S_ISDIR(sbuf.st_mode) && num_sub < MAXWORKERS) {
            strncpy(dir_paths[num_sub], path, PATHLENGTH);
            num_sub = num_sub + 1;
        }
    }

    int fd1[num_sub][2];    //pipe to write to worker processes
    int fd2[num_sub][2];    //pipe to read from worker processes

    // create pipes for all processes
    for (int i = 0; i < num_sub; i++) {
        if (pipe(fd1[i]) == -1) {
            perror("pipe");
            exit(1);
        }
        if (pipe(fd2[i]) == -1) {
            perror("pipe");
            exit(1);
        }
    }
    for (int i = 0; i < num_sub; i++) {
        int result = fork();

        switch (result) {
            case -1:
                perror("fork");
                exit(1);
                //child process
            case 0:
                //close writing for fd1
                if (close(fd1[i][1]) != 0) {
                    perror("close");
                    exit(1);
                }
                //close reading for fd2
                if (close(fd2[i][0]) != 0) {
                    perror("close");
                    exit(1);
                }
		for (int j = 0; j < i; j++) {
		    if (close(fd1[j][1]) != 0) {   //close writing for fd1
                perror("close");
                exit(1);
		    }
		    if (close(fd2[j][0]) != 0) {   //close reading for fd2
                perror("close");
                exit(1);
		    }
		}

		//run_worker for each subdirectories
		//fd1's reading as input and fd2's writing as output
		run_worker(dir_paths[i], fd1[i][0], fd2[i][1]);
		exit(0);

		//parent process
		default:
		    if (close(fd1[i][0]) != 0) {	//close reading for fd1
                perror("close");
                exit(1);
            }
		    if (close(fd2[i][1]) != 0) {	//close writing for fd2
                perror("close");
                exit(1);
		    }
		    break;
        }
    }

    char other[MAXWORD]; //word from stdin
    //array of frequency records from workers
    FreqRecord records[MAXRECORDS+1];
    FreqRecord *rec = malloc(sizeof(FreqRecord)); //record from a worker
    if (rec == 0) {
        perror("malloc");
        exit(1);
    }

    while (1) {
        int bytes = 0;
        int n = 0;
        if ((bytes = read(STDIN_FILENO, other, MAXWORD)) == 0) {
            break;
        } else {
            other[bytes-1] = '\0';
            for (int i = 0; i < MAXRECORDS+1; i++) {
                records[i].freq = 0;
                strncpy(records[i].filename, "", PATHLENGTH);
            }
            for (int i = 0; i < num_sub; i++) {
                //write the word to worker
                if (write(fd1[i][1], other, MAXWORD) < 0) {
                    perror("write");
                    exit(1);
                }
            }

            //fill up records
            for (int i = 0; i < num_sub; i++) {
                //read one FreqRecord from each worker
                if (read(fd2[i][0], rec, sizeof(FreqRecord)) < 0) {
                    perror("read");
                    exit(1);
                }

                if(rec->freq == 0) continue;

                //while until data from worker has frequency 0 (sentinel value)
                while (rec->freq != 0) {
                    if (n < MAXRECORDS) {
                        records[n].freq = rec->freq;
                        strncpy(records[n].filename, rec->filename, PATHLENGTH);
                        if (read(fd2[i][0], rec, sizeof(FreqRecord)) < 0) {
                            perror("read");
                            exit(1);
                        }
                        n++;
                    } else { //if number of data exceeds number of MAXRECORDS
                        //replace least frequent if recent data is greater
                        if (records[MAXRECORDS-1].freq < rec->freq) {
                            records[MAXRECORDS-1].freq = rec->freq;
                            strncpy(records[MAXRECORDS-1].filename,
                                rec->filename, PATHLENGTH);
                        }
                        if (read(fd2[i][0], rec, sizeof(FreqRecord)) < 0) {
                            perror("read");
                            exit(1);
                        }
                    }
                    //sort highest to lowest
                    qsort(records, n, sizeof(FreqRecord), compare);
                }
            }
            print_freq_records(records);
    	}
    }

    //close all pipes before exiting
    for (int i = 0; i < num_sub; i++) {
        if (close(fd1[i][1]) != 0)
            exit(1);
        if (close(fd2[i][0]) != 0)
            exit(1);
        if (close(fd2[i][1]) != 0)
            exit(1);
        if (close(fd1[i][0]) != 0)
            exit(1);
    }

    for (int i = 0; i < num_sub; i++) {
        if (wait(NULL) == -1) {
            perror("wait");
            exit(1);
        }
    }
    free(rec);
    return 0;
}
