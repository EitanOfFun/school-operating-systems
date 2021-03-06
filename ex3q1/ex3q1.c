#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>

#define PATH "/tmp"
#define C 'b'

typedef struct {
    key_t key;
    int shm_id;
    void* shm_ptr;
} Shm;

typedef struct ParallelSortableArray {
    size_t size;
    int parallelLevel;
    int *nums;
} ParallelSortableArray;

Shm mallocShm(const char* path, char c, int permissions, size_t size);
void deleteShm(Shm s);
void clearShm(Shm s);
void myParMergeSort(int *nums, size_t size, int splitLevel);
void myParRecMerge(int *nums, size_t size, int splitLevel);
void myMerge(int *nums, size_t size);
ParallelSortableArray getPArrFromFile(const char *fileName);

int main(int argc, char **argv) {
    ParallelSortableArray pArr = getPArrFromFile(argv[1]);

    Shm shm = mallocShm(PATH, C, IPC_CREAT | IPC_EXCL | 0600, sizeof(int)*(pArr.size + 1));
    int *shm_ptr = (int*) shm.shm_ptr;

    for (int i = 0; i < pArr.size; ++i)
        shm_ptr[i] = pArr.nums[i];

    printf("Amount of numbers that sort: %zu\n", pArr.size);
    printf("Degree of parallelism: %d\n", pArr.parallelLevel);
    printf("Before sort: %d", shm_ptr[0]);
    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", shm_ptr[i]);
    fflush(stdout);

    myParMergeSort(shm_ptr, pArr.size, pArr.parallelLevel);

    for (int i = 0; i < pArr.parallelLevel; ++i)
        wait(NULL);

    myParRecMerge(shm_ptr, pArr.size, pArr.parallelLevel);

    printf("\nAfter  sort: %d", shm_ptr[0]);

    for (int i = 1; i < pArr.size; ++i)
        printf(",%d", shm_ptr[i]);

    printf("\n");

    deleteShm(shm);
    clearShm(shm);
    free(pArr.nums);
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
 * Parallel Merge sort algorithm using fork() for every portion of array
 * @param nums array to sort
 * @param size size of array to sort
 * @param splitLevel amount of process and portions of array to split array into
 */
void myParMergeSort(int *nums, size_t size, int splitLevel) {
    if (splitLevel == 1) {
        pid_t pid = fork();
        if (pid == -1)
            perror("fork() failed");
        else if (pid == 0) {
            printf("\nCreate a process: %d", getpid());
            myRecSort(nums, size);
            exit(EXIT_SUCCESS);
        }
    } else {
        myParMergeSort(nums, size / 2, splitLevel / 2);
        myParMergeSort(nums + (size / 2), size / 2, splitLevel / 2);
    }
}
/**
 * Helper function (like a ctor) for generating Shared Memory struct with all data needed
 * @param path used for generating ftok()
 * @param c used for generating ftok()
 * @param permissions of shared memory array
 * @param size of shared memory array
 * @return Shm struct
 */
Shm mallocShm(const char* path, char c, int permissions, size_t size) {
    key_t key ;
    if ((key = ftok(path, c)) == -1) {
        perror("ftok() failed");
        exit(EXIT_FAILURE);
    }
    int shm_id = shmget(key, size, permissions) ;
    if (shm_id == -1) {
        perror("shmget() failed");
        exit(EXIT_FAILURE);
    }
    char *shm_ptr = (char *) shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror("shmat() failed");
        exit(EXIT_FAILURE);
    }
    Shm shm = {key, shm_id, shm_ptr};
    return shm;
}
/**
 * Helper function to delete the Shm
 * @param s
 */
void deleteShm(Shm s) {
    shmdt(s.shm_ptr);
}
/**
 * Helper function to clear the Shm
 * @param s
 */
void clearShm(Shm s) {
    if (shmctl(s.shm_id,IPC_RMID, NULL) == -1) {
        perror("shmctl() failed");
        exit(EXIT_FAILURE);
    }
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
