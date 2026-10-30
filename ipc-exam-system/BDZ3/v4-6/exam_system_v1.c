#include "common_v1.h"

shared_memory_t *shm;
shared_semaphores_t *sems;

void student_process(int id) {
    srand(time(NULL) ^ getpid() ^ id);
    
    printf("[Студент %d] Начал подготовку\n", id);
    

    int ticket = 1 + rand() % 20;
    printf("[Студент %d] Билет №%d\n", id, ticket);
    

    int prep_time = 1000 + rand() % 2000;
    usleep(prep_time * 1000);
    

    sem_wait(&sems->queue_free);
    sem_wait(&sems->mutex);
    
 
    int pos = shm->tail;
    shm->queue[pos].student_id = id;
    shm->queue[pos].ticket = ticket;
    shm->queue[pos].grade = 0;
    shm->queue[pos].is_ready = 1;
    
    shm->tail = (shm->tail + 1) % MAX_QUEUE;
    shm->count++;
    
    sem_post(&sems->mutex);
    sem_post(&sems->queue_fill);
    
    printf("[Студент %d] Сдал работу. Жду оценку...\n", id);

    while (shm->running) {
        sem_wait(&sems->mutex);
        
        int got_grade = 0;
        for (int i = 0; i < MAX_QUEUE; i++) {
            if (shm->queue[i].student_id == id && shm->queue[i].grade > 0) {
                printf("[Студент %d] Получил оценку: %d\n", id, shm->queue[i].grade);
                got_grade = 1;
                break;
            }
        }
        
        sem_post(&sems->mutex);
        
        if (got_grade) break;
        usleep(100000); 
    }
    
    exit(0);
}

void teacher_process() {
    printf("[Преподаватель] Начало экзамена\n");
    
    int graded = 0;
    
    while (graded < shm->total_students && shm->running) {

        sem_wait(&sems->queue_fill);
        
        if (!shm->running) break;
        
        sem_wait(&sems->mutex);

        exam_data_t work = shm->queue[shm->head];
        shm->head = (shm->head + 1) % MAX_QUEUE;
        shm->count--;
        
        sem_post(&sems->mutex);
        sem_post(&sems->queue_free);

        printf("[Преподаватель] Проверяю студента %d\n", work.student_id);
        int check_time = 500 + rand() % 1500;
        usleep(check_time * 1000);

        int grade = 2 + rand() % 4;

        sem_wait(&sems->mutex);
        for (int i = 0; i < MAX_QUEUE; i++) {
            if (shm->queue[i].student_id == work.student_id) {
                shm->queue[i].grade = grade;
                break;
            }
        }
        sem_post(&sems->mutex);
        
        printf("[Преподаватель] Студент %d - оценка %d\n", work.student_id, grade);
        graded++;
    }
    
    printf("[Преподаватель] Все работы проверены\n");
}

void cleanup() {
    printf("\nЗавершение...\n");
    shm->running = 0;
    

    for (int i = 0; i < MAX_QUEUE; i++) {
        sem_post(&sems->queue_fill);
        sem_post(&sems->queue_free);
    }
    
    sleep(1);

    sem_destroy(&sems->queue_free);
    sem_destroy(&sems->queue_fill);
    sem_destroy(&sems->mutex);
    

    munmap(shm, sizeof(shared_memory_t) + sizeof(shared_semaphores_t));
    
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, cleanup);
    signal(SIGTERM, cleanup);
    
    srand(time(NULL));
    

    void *shared_block = mmap(NULL, 
        sizeof(shared_memory_t) + sizeof(shared_semaphores_t),
        PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    
    if (shared_block == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }
    
    shm = (shared_memory_t*)shared_block;
    sems = (shared_semaphores_t*)((char*)shared_block + sizeof(shared_memory_t));

    memset(shm, 0, sizeof(shared_memory_t));
    shm->total_students = (argc > 1) ? atoi(argv[1]) : 3;
    shm->running = 1;
    
    printf("=== СИСТЕМА ЭКЗАМЕНА (Версия 1) ===\n");
    printf("Количество студентов: %d\n", shm->total_students);
    printf("-----------------------------------\n");

    sem_init(&sems->queue_free, 1, MAX_QUEUE); 
    sem_init(&sems->queue_fill, 1, 0);     
    sem_init(&sems->mutex, 1, 1);          
    

    for (int i = 1; i <= shm->total_students; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            student_process(i);
        } else if (pid < 0) {
            perror("fork");
            cleanup();
        }
    }

    teacher_process();

    sleep(2);
    cleanup();
    
    return 0;
}