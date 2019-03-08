#include <stdio.h>
#include <stdlib.h>

/* Return a pointer to an array of two dynamically allocated arrays of ints.
   The first array contains the elements of the input array s that are
   at even indices.  The second array contains the elements of the input
   array s that are at odd indices.

   Do not allocate any more memory than necessary.
*/

int **split_array(const int *s, int length) {
    int *odd_arr = malloc(length * sizeof(int));
    int *even_arr = malloc(length * sizeof(int));
    int x = 0;
    int j = 0;
    for (int i=0; i < length - 1;i++) {
        if  (i % 2 == 0) {
            even_arr[x] = s[i];
            x++;
        } else {
            odd_arr[j] = s[i];
            j++;
        }
    }
   int **arr = malloc(2*length*sizeof(int));
   arr[1] = odd_arr;
   arr[0] = even_arr;
   return arr;
}

/* Return a pointer to an array of ints with size elements.
   - strs is an array of strings where each element is the string
     representation of an integer.
   - size is the size of the array
 */

int *build_array(char **strs, int size) {
    int *arr_ptr = malloc(size * sizeof(int));
    for (int i=1;i<size;i++) {
	arr_ptr[i-1] = atoi(strs[i]);
   }
    return arr_ptr;
}


int main(int argc, char **argv) {
    /* Replace the comments in the next two lines with the appropriate
       arguments.  Do not add any additional lines of code to the main
       function or make other changes.
     */
    int *full_array = build_array(argv, argc);
    int **result = split_array(full_array, argc);

    printf("Original array:\n");
    for (int i = 0; i < argc - 1; i++) {
        printf("%d ", full_array[i]);
    }
    printf("\n");

    printf("result[0]:\n");
    for (int i = 0; i < argc/2; i++) {
        printf("%d ", result[0][i]);
    }
    printf("\n");

    printf("result[1]:\n");
    for (int i = 0; i < (argc - 1)/2; i++) {
        printf("%d ", result[1][i]);
    }
    printf("\n");
    free(full_array);
    free(result[0]);
    free(result[1]);
    free(result);
    return 0;
}
