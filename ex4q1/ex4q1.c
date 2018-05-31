#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

typedef struct ArrAndSize {
    int* nums;
    size_t size;
} ArrAndSize;

typedef struct ParallelSortableArray {
    size_t size;
    int parallelLevel;
    int *nums;
} ParallelSortableArray;

void myParMergeSort(ParallelSortableArray pArr, pthread_t *threads);
void myParRecMerge(int *nums, size_t size, int splitLevel);
void myRecSort(int *nums, size_t size);
void myMerge(int *nums, size_t size);
ParallelSortableArray getPArrFromFile(const char *fileName);
void* sort(void *p);


int main(int argc, char **argv) {
    ParallelSortableArray pArr = getPArrFromFile(argv[1]);

    printf("Amount of numbers that sort: %zu\n", pArr.size);
    printf("Degree of parallelism: %d\n", pArr.parallelLevel);
    printf("Before sort: %d", pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);
    fflush(stdout);

    pthread_t threads[pArr.parallelLevel];

    myParMergeSort(pArr, threads);



    printf("\nAfter sort: %d", pArr.nums[0]);

    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", pArr.nums[i]);

    printf("\n");
    return EXIT_SUCCESS;
}
/**
 * Simple merge algorithm (special case of arrays who are of size = x^2)
 * Merges to halves of an array (which are already sorted)
 * @param nums array to merge
 * @param size
 */
void myMerge(int *nums, size_t size) {
    int mergedNums[size];
    size_t i = 0, j = size / 2, k;
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
/**
 *
 * @param nums
 * @param size
 * @param splitLevel
 */
void myParRecMerge(int *nums, size_t size, int splitLevel) {
    if (splitLevel == 1)
        return;
    else {
        myParRecMerge(nums, size / 2, splitLevel / 2);
        myParRecMerge(nums + (size / 2), size / 2, splitLevel / 2);
        myMerge(nums, size);
    }
}
/**
 * Simple merge sort algorithm to sort each portion of array (not in parallel)
 * @param nums array
 * @param size of array
 */
void myRecSort(int *nums, size_t size) {
    if (size == 1)
        return;
    else {
        myRecSort(nums, size / 2);
        myRecSort(nums + (size / 2), size / 2);
        myMerge(nums, size);
    }
}
/**
 * Sorting function to send to each thread
 * @param p  ArrAndSize
 * @return
 */
void* sort(void *p) {
//    int pthreadid = pthread_self();
//    printf("\npid: %d\n", pthreadid);
    ArrAndSize* arrAndSize = (ArrAndSize*)p;
    myRecSort(arrAndSize->nums, arrAndSize->size);
    free(arrAndSize);
    return NULL;
}
/**
 * Parallel Merge sort algorithm using threads for every portion of array
 * @param nums array to sort
 * @param size size of array to sort
 * @param splitLevel amount of threads and portions of array to split array into
 */
void myParMergeSort(ParallelSortableArray pArr, pthread_t *threads) {
    size_t len = pArr.size / pArr.parallelLevel;
    for (int i = 0; i < pArr.parallelLevel; ++i) {
        ArrAndSize *arrAndSize = malloc(sizeof(ArrAndSize));
        arrAndSize->nums = pArr.nums + (i * len);
        arrAndSize->size = len;
        if (pthread_create(&threads[i], NULL, sort, (void *) arrAndSize) != 0) {
            perror("pthread create failed");
            exit(EXIT_FAILURE);
        } else
            printf("\nCreate a thread: %lu", threads[i]);

    }

    for (int i = 0; i < pArr.parallelLevel; ++i)
        pthread_join(threads[i], NULL);

    myParRecMerge(pArr.nums, pArr.size, pArr.parallelLevel);
}
/**
 * Helper function to create the ParallelSortableArray struct from a file in the format of
 * <array size>
 * <parallelLevel>
 * <nums> (Array of ints)
 * @param fileName
 * @return ParallelSortableArray
 */
ParallelSortableArray getPArrFromFile(const char *fileName) {
    ParallelSortableArray pArr;
    FILE* fp = fopen(fileName, "r");
    fscanf(fp, "%zu", &pArr.size);
    fscanf(fp, "%d", &pArr.parallelLevel);

    pArr.nums = malloc(pArr.size * sizeof(int));
    fscanf(fp, "%d", &pArr.nums[0]);
    for (int i = 1; i < pArr.size; ++i)
        fscanf(fp, ",%d", &pArr.nums[i]);

    fclose(fp);
    return pArr;
}
