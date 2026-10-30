#ifndef COMMON_V1_H
#define COMMON_V1_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <string.h>

typedef struct {
    sem_t queue_free;
    sem_t queue_fill;
    sem_t mutex;
} shared_semaphores_t;

#define MAX_STUDENTS 5
#define MAX_QUEUE 3

typedef struct {
    int student_id;
    int ticket;
    int grade;
    int is_ready;
} exam_data_t;

typedef struct {
    exam_data_t queue[MAX_QUEUE];
    int head, tail, count;
    int total_students;
    int running;
} shared_memory_t;

#endif