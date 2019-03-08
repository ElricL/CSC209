#include <stdio.h>

int check_group(int **elements, int n) {
int i, j, x, y;
for (i=0;i < n;i++) {
    for (j=0;j<n;j++) {
        if (elements[i][j] < 1 || elements[i][j] > n*n) {
            return 1;
        }
        for (x=i;x<n;x++) {
            for (y=0;y<n;y++) {
                if ((x==i && y!=j) || x != i) {
                    if (elements[i][j] == elements[x][y]) {
                        return 1;
                    }
                }
            }
        }
    }
}
return 0;
}

int check_regular_sudoku(int **puzzle) {
    int i, j;
    for (i=0;i<9;i++) {
        int row1[3] = {puzzle[i][0],puzzle[i][1], puzzle[i][2]};
        int row2[3] = {puzzle[i][3],puzzle[i][4], puzzle[i][5]};
        int row3[3] = {puzzle[i][6],puzzle[i][7], puzzle[i][5]};
        int *elements[] = {row1, row2, row3};
        if(check_group(elements, 3)) {
            return 1;
        }
    }   

for (i=0;i<9;i++) {
    int col1[3] = {puzzle[0][i], puzzle[1][i], puzzle[2][i]};
    int col2[3] = {puzzle[3][i], puzzle[4][i], puzzle[5][i]};
    int col3[3] = {puzzle[6][i], puzzle[7][i], puzzle[8][i]};
    int *elements[] = {col1, col2, col3};
    if (check_group(elements, 3)) {
        return 1;
    }
}

for (i=0;i<9;i+= 3) {
    for(j=0;j<9;j+=3) {
        int row1[3] = {puzzle[i][j], puzzle[i][j+1], puzzle[i][j+2]};
        int row2[3] = {puzzle[i+1][j], puzzle[i][j+1], puzzle[i][j+2]};
        int row3[3] = {puzzle[i+2][j], puzzle[i][j+1], puzzle[i][j+2]};
        int *elements[] = {row1, row2, row3};
    if (check_group(elements, 3)) {
	return 1;
    }
    }
}
return 0;
}
