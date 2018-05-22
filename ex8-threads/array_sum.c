#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define N 9
#define THREADS 3

typedef struct ArrAndSumResult {
    int* arr;
    int sum;
} ArrAndSumResult;

void* sumArrayPartition(void* arr) {
    int s = 0;
    for (int i = 0; i < N / THREADS; ++i)
        s += ((ArrAndSumResult*)arr)->arr[i];
    ((ArrAndSumResult*)arr)->sum = s;
    return NULL;
}

int main() {
    pthread_t thread_data[THREADS];
    ArrAndSumResult sumResults[THREADS];
    int arr[N];

    printf("Enter %d numbers: ", N);
    for (int i = 0; i < N; ++i)
        scanf("%d", arr + i);

    for (int i = 0; i < THREADS; ++i) {
        ArrAndSumResult sumResult = {arr + (i * THREADS), 0};
        sumResults[i] = sumResult;
        if (pthread_create(&thread_data[i], NULL, sumArrayPartition, (void*)&sumResults[i]) != 0) {
            perror("pthread create failed");
            exit(EXIT_FAILURE);
        }
    }

    int sum = 0;
    for (int i = 0; i < THREADS; ++i) {
        pthread_join(thread_data[i], NULL);
        sum += sumResults[i].sum;
    }
    printf("Sum is: %d", sum);
    return 0;
}