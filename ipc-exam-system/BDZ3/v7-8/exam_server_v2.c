#include "common_v2.h"

shared_memory_v2_t *shm = NULL;
sem_t *sem_free = NULL;
sem_t *sem_fill = NULL;
sem_t *sem_mutex = NULL;

void cleanup_server() {
    printf("\n[Сервер] Завершение...\n");
    
    if (shm) shm->running = 0;
    
    if (sem_fill) {
        for (int i = 0; i < MAX_QUEUE; i++) sem_post(sem_fill);
    }
    
    sleep(1);
    
    if (sem_free) sem_close(sem_free);
    if (sem_fill) sem_close(sem_fill);
    if (sem_mutex) sem_close(sem_mutex);
    
    sem_unlink(SEM_QUEUE_FILL_V2);
    sem_unlink(SEM_QUEUE_FREE_V2);
    sem_unlink(SEM_MUTEX_V2);
    
    if (shm) {
        munmap(shm, sizeof(shared_memory_v2_t));
        shm_unlink(SHM_NAME_V2);
    }
    
    exit(0);
}

void signal_handler(int sig) {
    printf("\n[Сервер] Сигнал %d\n", sig);
    cleanup_server();
}

int main(int argc, char *argv[]) {
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    srand(time(NULL));
    
    printf("=== ЭКЗАМЕН v2 (7-8 баллов) ===\n");
    
    int expected_students = 0;
    if (argc > 1) {
        expected_students = atoi(argv[1]);
        printf("[Сервер] Ожидаю %d студентов\n", expected_students);
    } else {
        printf("[Сервер] Ожидание студентов...\n");
    }

    sem_unlink(SEM_QUEUE_FILL_V2);
    sem_unlink(SEM_QUEUE_FREE_V2);
    sem_unlink(SEM_MUTEX_V2);
    shm_unlink(SHM_NAME_V2);

    int shm_fd = shm_open(SHM_NAME_V2, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_memory_v2_t));
    shm = mmap(NULL, sizeof(shared_memory_v2_t), 
               PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    
    memset(shm, 0, sizeof(shared_memory_v2_t));
    shm->running = 1;
    shm->total_students = expected_students;

    sem_free = sem_open(SEM_QUEUE_FREE_V2, O_CREAT, 0666, MAX_QUEUE);
    sem_fill = sem_open(SEM_QUEUE_FILL_V2, O_CREAT, 0666, 0);
    sem_mutex = sem_open(SEM_MUTEX_V2, O_CREAT, 0666, 1);
    
    printf("[Сервер] Готов. Запустите клиентов: ./exam_client_v2 <ID>\n");
    printf("----------------------------------------\n");
    
    int checked = 0;
    
    while (shm->running) {
        sem_wait(sem_fill);
        
        sem_wait(sem_mutex);
        
        exam_data_t work = shm->queue[shm->head];
        shm->head = (shm->head + 1) % MAX_QUEUE;
        shm->count--;

        int new_student = 1;
        for (int i = 0; i < MAX_STUDENTS; i++) {
            if (shm->students_pids[i] == 0) {
                shm->students_pids[i] = work.student_id;
                shm->students_active++;
                break;
            }
        }
        
        sem_post(sem_mutex);
        sem_post(sem_free);
        
        if (!new_student) continue;

        printf("[Преподаватель] Проверяю студента %d (билет %d)\n", 
               work.student_id, work.ticket);
        
        int check_time = random_ms(1000, 3000);
        usleep(check_time * 1000);
        
        int grade = 2 + rand() % 4;

        sem_wait(sem_mutex);
        for (int i = 0; i < MAX_QUEUE; i++) {
            int idx = (shm->head + i) % MAX_QUEUE;
            if (shm->queue[idx].student_id == work.student_id) {
                shm->queue[idx].grade = grade;
                break;
            }
        }
        sem_post(sem_mutex);
        
        printf("[Преподаватель] Студент %d -> оценка %d\n", work.student_id, grade);
        checked++;
        
        if (expected_students > 0 && checked >= expected_students) {
            printf("\n[Сервер] Все студенты сдали!\n");
            sleep(2);
            break;
        }
    }
    
    cleanup_server();
    return 0;
}
