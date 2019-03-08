#include<stdio.h>

int main() {
	char phone[11];
	int num;
	int has_error = 0;

	scanf("%s", phone);
	do {
		scanf("%i", &num);
		if (num == -1) {
			printf("%s \n", phone);
		} else if (num >= 0 && num <= 9) {
			printf("%c \n", phone[num]);
		} else {
			printf("ERROR \n");
			has_error = 1;
		}
	} while (1);
	return has_error;
}
