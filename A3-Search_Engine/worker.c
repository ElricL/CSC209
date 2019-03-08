#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

/* get_word will return an array of FreqRecord elements for the word
*/
FreqRecord *get_word(char *word, Node *head, char **file_names) {
    // Find word in linkedlist head
    Node *cur = head;
    while (cur != NULL && strcmp(cur->word, word) != 0) {
        cur = cur->next;
    }

    FreqRecord *wordRec = malloc(sizeof(FreqRecord)*(MAXFILES+1));
    if (wordRec == 0) {
        perror("malloc");
        exit(1);
    }
    if (cur == NULL) { // If word not found set first freq to 0 (sentinel)
        wordRec[0].freq = 0;
    } else { //copy word's frequencies & filenames to FreqRecord
        int i = 0;
        int j = 0;
        while (i < MAXFILES && file_names[i] != NULL) {
            if (cur->freq[i] > 0) {
                strncpy(wordRec[j].filename, file_names[i], PATHLENGTH);
                wordRec[j].freq = cur->freq[i];
                j++;
            }
            i++;
        }
        wordRec[j].freq = 0;
    }
    return wordRec;
}

/* Print to standard output the frequency records for a word.
* Use this for your own testing and also for query.c
*/
void print_freq_records(FreqRecord *frp) {
    int i = 0;

    while (frp != NULL && frp[i].freq != 0) {
        printf("%d    %s\n", frp[i].freq, frp[i].filename);
        i++;
    }
}

/* run_worker first load the index into a data structure.
* Then read one word at a time from the file descriptor in
* and find word in index list until file descriptor is closed.
* Write frequency records of each words to the file descriptor "out"
*/
void run_worker(char *dirname, int in, int out) {
    char *listfile;
    char *namefile;
    Node *head;
    char other[MAXWORD];
    // Create an array to hold filenames and initialize it to all NULL
    char **filenames = init_filenames();

    //allocate listfile and namefile
    listfile = malloc(sizeof(char)*PATHLENGTH);
    namefile = malloc(sizeof(char)*PATHLENGTH);
    if (listfile == 0) {
        perror("malloc");
        exit(1);
    }
    if (namefile == 0) {
        perror("malloc");
        exit(1);
    }

    strncpy(listfile, dirname, PATHLENGTH);
    strncat(listfile, "/index", PATHLENGTH);
    strncpy(namefile, dirname, PATHLENGTH);
    strncat(namefile, "/filenames", PATHLENGTH);

    //load index with the two files
    read_list(listfile, namefile, &head, filenames);

    //variable bytes stores how many bytes have been read
    int bytes = read(in, other, MAXWORD);
    if (bytes < 0) {
        perror("read");
        exit(1);
    }

    while(bytes != 0) {
        other[bytes-1] = '\0';

        FreqRecord *rec = get_word(other, head, filenames);
        //write each FreqRecord of the word to out
        int i;
        for (i = 0; rec[i].freq != 0; i++) {
			if (write(out, &rec[i], sizeof(FreqRecord)) < 0) {
				perror("write");
				exit(1);
			}
		}
		//write to out to empty the sentinel value of FreqRecords
		if (write(out, &rec[i], sizeof(FreqRecord)) < 0) {
			perror("write");
			exit(1);
		}

        bytes = read(in, other, MAXWORD);
        if (bytes < 0) {
            perror("read");
            exit(1);
        }
    }
    close(in);
    close(out);
}
