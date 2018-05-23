#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct ArrAndSize {
    int* nums;
    int size;
} ArrAndSize;

typedef struct ParallelSortableArray {
    int size;
    int parallelLevel;
    int *nums;
} ParallelSortableArray;

int comparator(const void* a, const void* b);
void myMergeSort(pthread_t *threads, int* nums, int size, int splitLevel, int startIndex);
void myRecMerge(int *nums, int size, int splitLevel, int startIndex);
void myMerge(int *nums, int size);
ParallelSortableArray getPArrFromFile(const char *fileName);

int main(int argc, char **argv) {
    ParallelSortableArray pArr = getPArrFromFile(argv[1]);

    printf("Amount of numbers that sort: %d\n", pArr.size);
    printf("Degree of parallelism: %d\n", pArr.parallelLevel);
    printf("Before sort: %d", pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);
    fflush(stdout);

    pthread_t threads[pArr.parallelLevel];

    myMergeSort(threads, pArr.nums, pArr.size, pArr.parallelLevel, 0);

    for (int i = 0; i < pArr.parallelLevel; ++i)
        pthread_join(threads[i], NULL);

    myRecMerge(pArr.nums, pArr.size, pArr.parallelLevel, 0);

    printf("\nAfter sort: %d", pArr.nums[0]);

    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);
}

void myMerge(int *nums, int size) {
    int mergedNums[size];
    int i = 0, j = size / 2, k;
    for (k = 0; i < size / 2 && j < size; k++) {
        if (nums[i] < nums[j]) {
            mergedNums[k] = nums[i];
            i++;
        }
        else {
            mergedNums[k] = nums[j];
            j++;
        }
    }
    while (i < size / 2) {
        mergedNums[k] = nums[i];
        i++;
        k++;
    }
    while (j < size) {
        mergedNums[k] = nums[j];
        j++;
        k++;
    }
    for (i = 0; i < size; ++i)
        nums[i] = mergedNums[i];
}
int comparator(const void* a, const void* b) {
    int num1 = *(const int *)a;
    int num2 = *(const int *)b;
    if (num1 > num2)
        return 1;
    if (num1 == num2)
        return 0;
    return -1;
}
void myRecMerge(int *nums, int size, int splitLevel, int startIndex) {
    if (splitLevel == 1)
        return;
    else {
        myRecMerge(nums, size / 2, splitLevel / 2, startIndex);
        myRecMerge(nums, size / 2, splitLevel / 2, startIndex + (size / 2));
        myMerge(nums + startIndex, size);
    }
}

void* sort(void *p) {
    ArrAndSize* arrAndSize = (ArrAndSize*)p;
    mergesort(arrAndSize->nums, (size_t) arrAndSize->size, sizeof(int), comparator);
    free(arrAndSize);
    return NULL;
}
void myMergeSort(pthread_t *threads, int* nums, int size, int splitLevel, int startIndex) {
    if (splitLevel == 1) {
        ArrAndSize* arrAndSize = malloc(sizeof(ArrAndSize));
        arrAndSize->nums = nums + startIndex;
        arrAndSize->size = size;
        if (pthread_create(&threads[startIndex / size], NULL, sort, (void*)arrAndSize) != 0) {
            perror("pthread create failed");
            exit(EXIT_FAILURE);
        } else
            printf("\nCreate a thread: %d", startIndex / size + 1);
    } else {
        myMergeSort(threads, nums, size / 2, splitLevel / 2, startIndex);
        myMergeSort(threads, nums, size / 2, splitLevel / 2, startIndex + (size / 2));
    }
}
ParallelSortableArray getPArrFromFile(const char *fileName) {
    ParallelSortableArray pArr;
    FILE* fp = fopen(fileName, "r");
    fscanf(fp, "%d", &pArr.size);
    fscanf(fp, "%d", &pArr.parallelLevel);

    pArr.nums = malloc(pArr.size * sizeof(int));
    fscanf(fp, "%d", &pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        fscanf(fp, ",%d", &pArr.nums[i]);

    fclose(fp);
    return pArr;
}