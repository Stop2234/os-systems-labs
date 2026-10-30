#include "common_v3.h"

shared_memory_v3_t *shm = NULL;
sem_t *sem_free = NULL;
sem_t *sem_fill = NULL;
sem_t *sem_mutex = NULL;

void cleanup_server() {
    printf("\n[Сервер] Завершение...\n");
    send_to_monitor(MSG_PROCESS_EXIT, 0, 0, "Сервер завершает работу");
    
    if (shm) shm->running = 0;
    
    sleep(1);
    
    if (sem_free) sem_close(sem_free);
    if (sem_fill) sem_close(sem_fill);
    if (sem_mutex) sem_close(sem_mutex);
    
    sem_unlink(SEM_QUEUE_FILL_V3);
    sem_unlink(SEM_QUEUE_FREE_V3);
    sem_unlink(SEM_MUTEX_V3);
    
    if (shm) {
        munmap(shm, sizeof(shared_memory_v3_t));
        shm_unlink(SHM_NAME_V3);
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
    
    printf("========================================\n");
    printf("     ЭКЗАМЕН v3 (9 баллов)\n");
    printf("     С процессом-наблюдателем\n");
    printf("========================================\n");
    
    send_to_monitor(MSG_PROCESS_EXIT, 0, 0, "Сервер запущен");
    
    int expected_students = (argc > 1) ? atoi(argv[1]) : 3;
    printf("[Сервер] Ожидаю %d студентов\n", expected_students);

    sem_unlink(SEM_QUEUE_FILL_V3);
    sem_unlink(SEM_QUEUE_FREE_V3);
    sem_unlink(SEM_MUTEX_V3);
    shm_unlink(SHM_NAME_V3);

    mkfifo(FIFO_NAME, 0666);

    int shm_fd = shm_open(SHM_NAME_V3, O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(shared_memory_v3_t));
    shm = mmap(NULL, sizeof(shared_memory_v3_t), 
               PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    close(shm_fd);
    
    memset(shm, 0, sizeof(shared_memory_v3_t));
    shm->running = 1;
    shm->total_students = expected_students;

    sem_free = sem_open(SEM_QUEUE_FREE_V3, O_CREAT, 0666, MAX_QUEUE);
    sem_fill = sem_open(SEM_QUEUE_FILL_V3, O_CREAT, 0666, 0);
    sem_mutex = sem_open(SEM_MUTEX_V3, O_CREAT, 0666, 1);
    
    printf("[Сервер] Система инициализирована\n");
    printf("[Сервер] Запустите наблюдателя: ./exam_monitor_v3\n");
    printf("[Сервер] Запустите студентов: ./exam_client_v3 <ID>\n");
    printf("----------------------------------------\n");
    
    int checked = 0;
    
    while (shm->running && checked < expected_students) {
        sem_wait(sem_fill);
        
        sem_wait(sem_mutex);
        
        exam_data_t work = shm->queue[shm->head];
        shm->head = (shm->head + 1) % MAX_QUEUE;
        shm->count--;

        for (int i = 0; i < MAX_STUDENTS; i++) {
            if (shm->students_pids[i] == 0) {
                shm->students_pids[i] = work.student_id;
                shm->students_active++;
                break;
            }
        }
        
        sem_post(sem_mutex);
        sem_post(sem_free);
 
        char msg[100];
        snprintf(msg, 100, "Студент %d сдал работу", work.student_id);
        send_to_monitor(MSG_TEACHER_CHECK, work.student_id, work.ticket, msg);
        
        printf("[Преподаватель] Проверяю студента %d\n", work.student_id);

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

        snprintf(msg, 100, "Студент %d получил %d", work.student_id, grade);
        send_to_monitor(MSG_GRADE_GIVEN, work.student_id, grade, msg);
        
        printf("[Преподаватель] Студент %d -> %d\n", work.student_id, grade);
        checked++;
        
        printf("[Преподаватель] Проверено: %d/%d\n", checked, expected_students);
    }
    
    printf("\n[Сервер] Экзамен завершен!\n");
    send_to_monitor(MSG_PROCESS_EXIT, 0, checked, "Экзамен завершен");
    
    sleep(2);
    cleanup_server();
    return 0;
}