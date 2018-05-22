#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>

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
    Shm shm = mallocShm(PATH, C, IPC_CREAT  | IPC_EXCL  | 0600, sizeof(int)*(NUMS + 1));
    int x = 9;
    for (int i = 0; i < NUMS; ++i)
        scanf("%d", &(((int*)shm.shm_ptr)[i]));
    deleteShm(shm);
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