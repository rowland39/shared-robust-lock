#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#define SHARED_KEY_FILE "/tmp/shared-key.txt"
#define SHARED_KEY_ID 2377

int
main(void)
{
    int fd;
    key_t shmkey;
    int shmid;
    pthread_mutex_t *lock;
    int lock_status;

    if ((shmkey = ftok(SHARED_KEY_FILE, SHARED_KEY_ID)) == -1) {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    if ((shmid = shmget(shmkey, sizeof(pthread_mutex_t), 0600)) == -1) {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    if ((lock = (pthread_mutex_t *)shmat(shmid, 0, 0)) == (void *)-1) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }

    printf("Trying to get lock.\n");
    lock_status = pthread_mutex_lock(lock);

    if (lock_status == EOWNERDEAD) {
        printf("Locked: fixing consistency.\n");

        if (pthread_mutex_consistent(lock) != 0) {
            fprintf(stderr, "pthread_mutex_consistent error\n");
            exit(EXIT_FAILURE);
        }
    } else if (lock_status != 0) {
        fprintf(stderr, "pthread_mutex_lock error\n");
        exit(EXIT_FAILURE);
    } else {
        printf("Locked.\n");
    }

    if (pthread_mutex_unlock(lock) != 0) {
        fprintf(stderr, "pthread_mutex_unlock error\n");
        exit(EXIT_FAILURE);
    }

    printf("Unlocked.\n");

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
