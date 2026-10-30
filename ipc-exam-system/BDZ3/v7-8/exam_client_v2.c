#include "common_v2.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <ID_студента>\n", argv[0]);
        exit(1);
    }
    
    int student_id = atoi(argv[1]);
    srand(time(NULL) ^ getpid() ^ student_id);
    
    printf("\n=== СТУДЕНТ %d (PID: %d) ===\n", student_id, getpid());

    int shm_fd = shm_open(SHM_NAME_V2, O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("[Ошибка] Сервер не запущен");
        exit(1);
    }
    
    shared_memory_v2_t *shm = mmap(NULL, sizeof(shared_memory_v2_t),
                                   PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);

    sem_t *sem_free = sem_open(SEM_QUEUE_FREE_V2, 0);
    sem_t *sem_fill = sem_open(SEM_QUEUE_FILL_V2, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX_V2, 0);
    
    if (sem_free == SEM_FAILED || sem_fill == SEM_FAILED || sem_mutex == SEM_FAILED) {
        printf("[Ошибка] Не могу подключиться к семафорам\n");
        exit(1);
    }
 
    int ticket = 1 + rand() % 20;
    printf("[Студент %d] Билет №%d\n", student_id, ticket);

    int prep_time = random_ms(2000, 5000);
    printf("[Студент %d] Готовлю ответ (%d мс)...\n", student_id, prep_time);
    usleep(prep_time * 1000);

    sem_wait(sem_free);
    sem_wait(sem_mutex);
    
    int pos = shm->tail;
    shm->queue[pos].student_id = student_id;
    shm->queue[pos].ticket = ticket;
    shm->queue[pos].grade = 0;
    shm->queue[pos].is_ready = 1;
    shm->queue[pos].pid = getpid();
    
    shm->tail = (shm->tail + 1) % MAX_QUEUE;
    shm->count++;
    
    sem_post(sem_mutex);
    sem_post(sem_fill);
    
    printf("[Студент %d] Работа сдана\n", student_id);

    int attempts = 0;
    int grade = 0;
    
    while (attempts < 50 && grade == 0) {
        usleep(100000);
        
        sem_wait(sem_mutex);
        for (int i = 0; i < MAX_QUEUE; i++) {
            int idx = (shm->head + i) % MAX_QUEUE;
            if (shm->queue[idx].student_id == student_id && shm->queue[idx].grade > 0) {
                grade = shm->queue[idx].grade;
                break;
            }
        }
        sem_post(sem_mutex);
        attempts++;
    }

    if (grade > 0) {
        printf("[Студент %d] Оценка: %d\n", student_id, grade);
        if (grade >= 4) printf("[Студент %d] Ура! 🎉\n", student_id);
    } else {
        printf("[Студент %d] Оценка не получена\n", student_id);
    }
    
    printf("[Студент %d] Завершаю работу\n", student_id);

    sem_close(sem_free);
    sem_close(sem_fill);
    sem_close(sem_mutex);
    munmap(shm, sizeof(shared_memory_v2_t));
    
    return 0;
}
