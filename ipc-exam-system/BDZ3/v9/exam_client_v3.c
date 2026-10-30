#include "common_v3.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Использование: %s <ID_студента>\n", argv[0]);
        exit(1);
    }
    
    int student_id = atoi(argv[1]);
    srand(time(NULL) ^ getpid() ^ student_id);

    char msg[100];
    snprintf(msg, 100, "Студент %d запущен", student_id);
    send_to_monitor(MSG_STUDENT_START, student_id, getpid(), msg);
    
    printf("\n=== СТУДЕНТ %d (PID: %d) ===\n", student_id, getpid());

    int shm_fd = shm_open(SHM_NAME_V3, O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("[Студент %d] Сервер не запущен\n", student_id);
        exit(1);
    }
    
    shared_memory_v3_t *shm = mmap(NULL, sizeof(shared_memory_v3_t),
                                   PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    
    sem_t *sem_free = sem_open(SEM_QUEUE_FREE_V3, 0);
    sem_t *sem_fill = sem_open(SEM_QUEUE_FILL_V3, 0);
    sem_t *sem_mutex = sem_open(SEM_MUTEX_V3, 0);

    int ticket = 1 + rand() % 20;
    snprintf(msg, 100, "Студент %d получил билет %d", student_id, ticket);
    send_to_monitor(MSG_STUDENT_TICKET, student_id, ticket, msg);
    
    printf("[Студент %d] Билет №%d\n", student_id, ticket);

    int prep_time = random_ms(2000, 5000);
    snprintf(msg, 100, "Студент %d готовит ответ %d мс", student_id, prep_time);
    send_to_monitor(MSG_STUDENT_PREPARE, student_id, prep_time, msg);
    
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
    
    snprintf(msg, 100, "Студент %d сдал работу", student_id);
    send_to_monitor(MSG_STUDENT_SUBMIT, student_id, 0, msg);
    
    printf("[Студент %d] Работа сдана\n", student_id);

    int attempts = 0;
    int grade = 0;
    
    while (attempts < 50 && grade == 0) {
        usleep(100000);
        
        sem_wait(sem_mutex);
        for (int i = 0; i < MAX_QUEUE; i++) {
            int idx = (shm->head + i) % MAX_QUEUE;
            if (shm->queue[idx].student_id == student_id && 
                shm->queue[idx].grade > 0) {
                grade = shm->queue[idx].grade;
                break;
            }
        }
        sem_post(sem_mutex);
        attempts++;
    }

    if (grade > 0) {
        printf("[Студент %d] Оценка: %d\n", student_id, grade);
    } else {
        printf("[Студент %d] Оценка не получена\n", student_id);
    }
    
    snprintf(msg, 100, "Студент %d завершил работу с оценкой %d", student_id, grade);
    send_to_monitor(MSG_PROCESS_EXIT, student_id, grade, msg);
    
    printf("[Студент %d] Завершаю работу\n", student_id);

    sem_close(sem_free);
    sem_close(sem_fill);
    sem_close(sem_mutex);
    munmap(shm, sizeof(shared_memory_v3_t));
    
    return 0;
}