#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>

#define SHM_NAME "/my_shm_segment"

struct shmdata {
    int value;
    int running;
};

struct shmdata *shared;
int fd;

void cleanup() {
    if (shared != NULL)
        munmap(shared, sizeof(struct shmdata));
    if (fd >= 0)
        close(fd);
}

void handle_sigint(int sig) {
    printf("\n[CLIENT] Завершение...\n");
    shared->running = 0;  
    cleanup();
    exit(0);
}

int main() {
    signal(SIGINT, handle_sigint);

    fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd < 0) { perror("shm_open"); exit(1); }

    shared = mmap(NULL, sizeof(struct shmdata),
                  PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    srand(time(NULL));

    printf("[CLIENT] Генерирую данные...\n");

    while (shared->running) {
        shared->value = rand() % 100;
        usleep(200000);
    }

    printf("[CLIENT] Сервер завершил работу\n");
    cleanup();
    return 0;
}
