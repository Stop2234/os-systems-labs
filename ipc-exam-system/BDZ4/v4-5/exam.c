#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define MAX_STUDENTS 50

typedef struct {
    int id;
    int ticket;
    int grade;
} Student;

pthread_mutex_t queue_mutex;
sem_t new_work;
sem_t check_done;
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

            sem_post(&check_done); 
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
    sem_wait(&check_done); 

    printf("Студент %d получил оценку %d\n", s->id, s->grade);
    free(s);
    return NULL;
}

int main() {
    srand(time(NULL));
    signal(SIGINT, handle_signal);

    pthread_mutex_init(&queue_mutex, NULL);
    sem_init(&new_work, 0, 0);
    sem_init(&check_done, 0, 0);

    printf("Введите количество студентов: ");
    scanf("%d", &total_students);
    if (total_students > MAX_STUDENTS) total_students = MAX_STUDENTS;

    pthread_t teacher;
    pthread_t students[total_students];

    pthread_create(&teacher, NULL, teacher_thread, NULL);

    for (int i = 0; i < total_students; i++) {
        Student* s = malloc(sizeof(Student));
        s->id = i + 1;
        s->ticket = rand() % 100 + 1; 
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
    sem_destroy(&check_done);
    pthread_mutex_destroy(&queue_mutex);

    printf("Экзамен завершён.\n");
    return 0;
}