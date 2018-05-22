#include <stdio.h>
#include <string.h>
#include <stdlib.h>


int getWords(const char* string) {
    char copy[512];
    strcpy(copy, string);
    int words = 0;
    char *ptr = strtok(copy, " \n");
    while (ptr != NULL) {
        ptr = strtok(NULL, " \n");
        words++;
    }
    return words;
}
int main() {

    char input[512];
    char copy[512];


    fgets(input, 512, stdin);
    strcpy(copy, input);
    int words = getWords(input);

    char *args[words + 1];
    args[words] = NULL;

    char *ptr = strtok(copy, " \n");
    for (int i = 0; ptr != NULL; i++) {

        args[i] = (char*)malloc((strlen(ptr) + 1)* sizeof(char));
        strcpy(args[i], ptr);
        ptr = strtok(NULL, " \n");
    }

    for (int j = 0; j < words; ++j) {
        printf("%s\n", args[j]);
        free(args[j]);
    }

    return 0;
}
