#ifndef COMMON_V2_H
#define COMMON_V2_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <errno.h>

#define SHM_NAME_V2 "/exam_shm_v2"
#define SEM_QUEUE_FREE_V2 "/exam_qfree_v2"
#define SEM_QUEUE_FILL_V2 "/exam_qfill_v2"
#define SEM_MUTEX_V2 "/exam_mutex_v2"

#define MAX_STUDENTS 10
#define MAX_QUEUE 5

typedef struct {
    int student_id;
    int ticket;
    int grade;
    int is_ready;
    pid_t pid;
    char answer[100];
} exam_data_t;

typedef struct {
    exam_data_t queue[MAX_QUEUE];
    int head, tail, count;
    int total_students;
    int running;
    int students_active;
    pid_t students_pids[MAX_STUDENTS];
} shared_memory_v2_t;

static inline int random_ms(int min, int max) {
    return min + rand() % (max - min + 1);
}

#endif
