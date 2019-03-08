#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main (int argc, char *argv[])
{
    char line[1024];
    int i = 0;
    char name[31];
    int id;
    

    if (argc == 1) {
	int freq[1024];
	int idlist[1024];
	char *names[1024];
	names[i] = malloc(256 * sizeof *names[i]);
	int lines = 0;
        while (fscanf(stdin, "%s %d", names[i], &idlist[i]) != EOF) {
	    fgets(line, 1024, stdin);
	    freq[i] = -1;
	    lines++;
	    i++;
	    names[i] = malloc (256 * sizeof *names[i]);
	}

	for (int i=0; i < lines; i++) {
    	    int count = 1;
	    for(int j=i+1; j< lines; j++) {
		if (strcmp(names[i], names[j]) == 0) {
		    count++;
		}
	    }
	    freq[i] = count;
	}
	int most = 0;
	int index = 0;
	for (i=0; i<1024;i++) {
	    if (freq[i] > most) {
		index = i;
		most = freq[i];
	    }
	}
	printf("%s %d\n", names[index], most);

    } else if (argc > 2) {
	printf("USAGE: most_processes [ppid]\n");
	return 1;

    } else {
	while (fscanf(stdin, "%s %d", name, &id) != EOF) {
	    fgets(line, 1024, stdin);
	    if(strtol(argv[1], NULL, 10) == id) {
		printf("%s %d\n", name, id);
		return 0;
	    }
	}
    }
    return 0;
}


