#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define SHARED_LOCK_FILE "/tmp/shared-lock.txt"

int
main(void)
{
    int fd;
    pthread_mutex_t *lock;
    pthread_mutexattr_t psharedm;
    int lock_status;

    if ((fd = open(SHARED_LOCK_FILE, O_CREAT|O_RDWR, 0600)) == -1) {
        perror("open error");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(pthread_mutex_t)) == -1) {
        perror("ftruncate error");
        exit(EXIT_FAILURE);
    }

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

    if ((lock = (pthread_mutex_t *)mmap(NULL, sizeof(pthread_mutex_t),
        PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED) {
        perror("mmap error");
        exit(EXIT_FAILURE);
    }

    close(fd);

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

    if (munmap(lock, sizeof(pthread_mutex_t)) == -1) {
        perror("munmap error");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}
