#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>

#define MAX_STUDENTS 50

typedef struct {
    int id;
    int ticket;
    int grade;
    sem_t student_done; 
} Student;

pthread_mutex_t queue_mutex;
sem_t new_work;
Student* works[MAX_STUDENTS];
int work_count = 0;
int total_students;
int running = 1;

void handle_signal(int sig) {
    printf("\nПолучен сигнал прерывания. Завершение работы...\n");
    running = 0;
}

void* teacher_thread(void* arg) {
    printf("Преподаватель начинает проверку работ.\n");
    while (running) {
        sem_wait(&new_work); 
        if (!running) break;

        pthread_mutex_lock(&queue_mutex);
        if (work_count > 0) {
            Student* s = works[--work_count];
            pthread_mutex_unlock(&queue_mutex);

            printf("Преподаватель проверяет работу студента %d (билет %d)\n", s->id, s->ticket);
            int check_time = rand() % 3 + 1;
            sleep(check_time);
            
            s->grade = rand() % 4 + 2;
            printf("Преподаватель выставил оценку %d студенту %d\n", s->grade, s->id);

            sem_post(&(s->student_done));
        } else {
            pthread_mutex_unlock(&queue_mutex);
        }
    }
    printf("Преподаватель завершил работу.\n");
    return NULL;
}

void* student_thread(void* arg) {
    Student* s = (Student*)arg;
    printf("Студент %d получает билет №%d\n", s->id, s->ticket);

    int prep_time = rand() % 5 + 1;
    sleep(prep_time);
    printf("Студент %d подготовил ответ за %d сек\n", s->id, prep_time);

    pthread_mutex_lock(&queue_mutex);
    works[work_count++] = s;
    pthread_mutex_unlock(&queue_mutex);

    sem_post(&new_work); 

    sem_wait(&(s->student_done));
    
    printf("Студент %d получил оценку %d\n", s->id, s->grade);
 
    sem_destroy(&(s->student_done));
    free(s);
    return NULL;
}

void print_usage(const char* prog_name) {
    printf("Использование: %s [ПАРАМЕТРЫ]\n", prog_name);
    printf("Параметры:\n");
    printf("  -n <число>    Количество студентов (1-%d, обязательно)\n", MAX_STUDENTS);
    printf("  -h            Вывод этой справки\n");
    printf("\nПримеры:\n");
    printf("  %s -n 5           # Экзамен для 5 студентов\n", prog_name);
    printf("  %s -n 10          # Экзамен для 10 студентов\n", prog_name);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    signal(SIGINT, handle_signal);

    total_students = 0;
    
    if (argc < 2) {
        printf("Ошибка: не указаны параметры!\n\n");
        print_usage(argv[0]);
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            total_students = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        }
    }
    
    if (total_students <= 0 || total_students > MAX_STUDENTS) {
        printf("Ошибка: количество студентов должно быть от 1 до %d\n", MAX_STUDENTS);
        printf("Вы указали: %d\n\n", total_students);
        print_usage(argv[0]);
        return 1;
    }
    
    printf("=== МОДЕЛИРОВАНИЕ ЭКЗАМЕНА ===\n");
    printf("Количество студентов: %d\n\n", total_students);

    pthread_mutex_init(&queue_mutex, NULL);
    sem_init(&new_work, 0, 0);

    pthread_t teacher;
    pthread_t students[total_students];

    pthread_create(&teacher, NULL, teacher_thread, NULL);

    for (int i = 0; i < total_students; i++) {
        Student* s = malloc(sizeof(Student));
        s->id = i + 1;
        s->ticket = rand() % 100 + 1;
        sem_init(&(s->student_done), 0, 0); 
        pthread_create(&students[i], NULL, student_thread, s);
        sleep(rand() % 2);
    }

    for (int i = 0; i < total_students; i++) {
        pthread_join(students[i], NULL);
    }

    running = 0;
    sem_post(&new_work);
    pthread_join(teacher, NULL);

    sem_destroy(&new_work);
    pthread_mutex_destroy(&queue_mutex);

    printf("\n=== ЭКЗАМЕН ЗАВЕРШЁН ===\n");
    printf("Все %d студентов сдали экзамен.\n", total_students);
    
    return 0;
}