#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>

#define PATH "/tmp"
#define C 'y'
#define NUMS 30

typedef struct {
    key_t key;
    int shm_id;
    void* shm_ptr;
} Shm;

Shm mallocShm(const char* path, char c, int permissions, size_t size);
void deleteShm(Shm s);
void clearShm(Shm s);

int main() {
    Shm shm = mallocShm(PATH, C, 0600, 0);

    pid_t pid1 = fork();

    if (pid1 == 0) {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            pid_t pid3 = fork();
            if (pid3 == 0) {
                int sum2 = 0;
                for (int i = 10; i < 20; ++i)
                    sum2 += ((int*)shm.shm_ptr)[i];
                ((int*)shm.shm_ptr)[NUMS] += sum2;
            } else {
                wait(NULL);
                int sum3 = 0;
                for (int i = 20; i < 30; ++i) {
                    sum3 += ((int*)shm.shm_ptr)[i];
                }
                ((int*)shm.shm_ptr)[NUMS] += sum3;
            }
        } else {
            wait(NULL);
            int sum1 = 0;
            for (int i = 0; i < 10; ++i)
                sum1 += ((int*)shm.shm_ptr)[i];
            ((int*)shm.shm_ptr)[NUMS] += sum1;
        }
        deleteShm(shm);
    } else {
        wait(NULL);
        printf("%d\n", ((int*)shm.shm_ptr)[NUMS]);
        deleteShm(shm);
        clearShm(shm);
    }
    return 0;
}

Shm mallocShm(const char* path, char c, int permissions, size_t size) {
    key_t key ;
    if ((key = ftok(path, c)) == -1) {
        perror("ftok failed") ;
        exit(EXIT_FAILURE) ;
    }
    int shm_id = shmget(key, size, permissions) ;
    if (shm_id == -1) {
        perror("shmget failed") ;
        exit(EXIT_FAILURE) ;
    }
    char *shm_ptr = (char *) shmat(shm_id, NULL, 0);
    if (shm_ptr == (char *) -1) {
        perror( "shmat failed" ) ;
        exit( EXIT_FAILURE ) ;
    }
    Shm shm = {key, shm_id, shm_ptr};
    return shm;
}
void deleteShm(Shm s) {
    shmdt(s.shm_ptr);
}
void clearShm(Shm s) {
    if (shmctl(s.shm_id,IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(EXIT_FAILURE);
    }
}


