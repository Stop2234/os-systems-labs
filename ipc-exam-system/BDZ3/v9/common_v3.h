#ifndef COMMON_V3_H
#define COMMON_V3_H

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
#include <sys/stat.h>

#define SHM_NAME_V3 "/exam_shm_v3"
#define SEM_QUEUE_FREE_V3 "/exam_qfree_v3"
#define SEM_QUEUE_FILL_V3 "/exam_qfill_v3"
#define SEM_MUTEX_V3 "/exam_mutex_v3"

#define FIFO_NAME "/tmp/exam_fifo_v3"

#define MAX_STUDENTS 10
#define MAX_QUEUE 5

typedef enum {
    MSG_STUDENT_START = 1,
    MSG_STUDENT_TICKET,
    MSG_STUDENT_PREPARE,
    MSG_STUDENT_SUBMIT,
    MSG_TEACHER_CHECK,
    MSG_GRADE_GIVEN,
    MSG_PROCESS_EXIT
} msg_type_t;

typedef struct {
    msg_type_t type;
    int process_id;  
    int student_id;  
    int data;        
    char timestamp[20];
    char message[100];
} monitor_message_t;

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
} shared_memory_v3_t;

static inline int random_ms(int min, int max) {
    return min + rand() % (max - min + 1);
}

static inline void send_to_monitor(msg_type_t type, int student_id, int data, const char* msg) {
    int fifo_fd = open(FIFO_NAME, O_WRONLY | O_NONBLOCK);
    if (fifo_fd > 0) {
        monitor_message_t message;
        message.type = type;
        message.process_id = getpid();
        message.student_id = student_id;
        message.data = data;

        time_t now = time(NULL);
        struct tm *t = localtime(&now);
        strftime(message.timestamp, 20, "%H:%M:%S", t);
        
        if (msg) {
            strncpy(message.message, msg, 99);
            message.message[99] = '\0';
        } else {
            message.message[0] = '\0';
        }
        
        write(fifo_fd, &message, sizeof(monitor_message_t));
        close(fifo_fd);
    }
}

#endif