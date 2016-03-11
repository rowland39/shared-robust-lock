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
    pthread_mutexattr_t psharedm;
    int lock_status;

    if ((fd = open(SHARED_KEY_FILE, O_CREAT|O_RDWR|O_TRUNC, 0600)) == -1) {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    close(fd);

    if (pthread_mutexattr_init(&psharedm) != 0) {
        fprintf(stderr, "pthread_mutexattr_init error\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setpshared(&psharedm, PTHREAD_PROCESS_SHARED) != 0) {
        fprintf(stderr, "pthread_mutexattr_setpshared error\n");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutexattr_setrobust(&psharedm, PTHREAD_MUTEX_ROBUST) != 0) {
        fprintf(stderr, "pthread_mutexattr_setrobust error\n");
        exit(EXIT_FAILURE);
    }

    if ((shmkey = ftok(SHARED_KEY_FILE, SHARED_KEY_ID)) == -1) {
        perror("ftok error");
        exit(EXIT_FAILURE);
    }

    if ((shmid = shmget(shmkey, sizeof(pthread_mutex_t), IPC_CREAT|0600)) ==
        -1) {
        perror("shmget error");
        exit(EXIT_FAILURE);
    }

    if ((lock = (pthread_mutex_t *)shmat(shmid, 0, 0)) == (void *)-1) {
        perror("shmat error");
        exit(EXIT_FAILURE);
    }

    if (pthread_mutex_init(lock, &psharedm) != 0) {
        fprintf(stderr, "pthread_mutex_init error\n");
        exit(EXIT_FAILURE);
    }

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

    sleep(10);
    
    if (pthread_mutex_unlock(lock) != 0) {
        fprintf(stderr, "pthread_mutex_unlock error\n");
        exit(EXIT_FAILURE);
    }

    printf("Unlocked.\n");
    sleep(10);

    if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
