// server.c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#define SHM_NAME "/my_shm_segment"

struct shmdata {
    int value;
    int running;
};

struct shmdata *shared;

int fd;

void cleanup() {
    shared->running = 0;   
    munmap(shared, sizeof(struct shmdata));
    close(fd);
    shm_unlink(SHM_NAME);  
}

void handle_sigint(int sig) {
    printf("\n[SERVER] Завершение...\n");
    cleanup();
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);

    fd = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0666);
    if (fd < 0) { perror("shm_open"); exit(1); }

    ftruncate(fd, sizeof(struct shmdata));

    shared = mmap(NULL, sizeof(struct shmdata),
                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    shared->running = 1;

    printf("[SERVER] Ожидаю данные...\n");

    while (shared->running) {
        printf("[SERVER] value = %d\n", shared->value);
        usleep(300000);
    }

    printf("[SERVER] Клиент завершил работу\n");
    cleanup();
    return 0;
}
