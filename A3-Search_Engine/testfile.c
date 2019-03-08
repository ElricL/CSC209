#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#include "freq_list.h"
#include "worker.h"

int main(int argc, char *argv[]) {
    Node *head = malloc(sizeof(Node)*4);;
    char *filenames[] = {"menu1","menu2"};
    add_word(head, filenames, "pepper", "menu2");
    for(int i = 0; i < 11;i++) {
        add_word(head, filenames, "potato", "menu1");
    }
    for(int i = 0; i < 3;i++) {
        add_word(head, filenames, "potato", "menu2");
    }
    for(int i = 0; i < 4;i++) {
        add_word(head, filenames, "salad", "menu1");
    }
    for(int i = 0; i < 2;i++) {
        add_word(head, filenames, "spinach", "menu1");
    }
    for(int i = 0; i < 6;i++) {
        add_word(head, filenames, "spinach", "menu2");
    }
    FreqRecord *word = get_word(argv[1], head, filenames);
    if (word[0].freq == 0) {
        printf("Invalid word\n");
    } else {
        print_freq_records(word);
    }
    return 0;
}
