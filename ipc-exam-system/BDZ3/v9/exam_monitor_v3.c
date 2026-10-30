#include "common_v3.h"

void print_message(monitor_message_t *msg) {
    printf("[Монитор %s] ", msg->timestamp);
    
    switch (msg->type) {
        case MSG_STUDENT_START:
            printf("СТУДЕНТ %d запущен (PID: %d)\n", 
                   msg->student_id, msg->process_id);
            break;
            
        case MSG_STUDENT_TICKET:
            printf("СТУДЕНТ %d получил БИЛЕТ №%d\n",
                   msg->student_id, msg->data);
            break;
            
        case MSG_STUDENT_PREPARE:
            printf("СТУДЕНТ %d готовит ОТВЕТ (%d мс)\n",
                   msg->student_id, msg->data);
            break;
            
        case MSG_STUDENT_SUBMIT:
            printf("СТУДЕНТ %d СДАЛ работу\n", msg->student_id);
            break;
            
        case MSG_TEACHER_CHECK:
            printf("ПРЕПОДАВАТЬ проверяет студента %d (билет %d)\n",
                   msg->student_id, msg->data);
            break;
            
        case MSG_GRADE_GIVEN:
            printf("СТУДЕНТ %d получает ОЦЕНКУ: %d\n",
                   msg->student_id, msg->data);
            break;
            
        case MSG_PROCESS_EXIT:
            if (msg->student_id == 0) {
                printf("СЕРВЕР завершает работу. Всего проверено: %d\n", msg->data);
            } else {
                printf("СТУДЕНТ %d завершил работу. Оценка: %d\n", 
                       msg->student_id, msg->data);
            }
            break;
            
        default:
            printf("Неизвестное сообщение: %d\n", msg->type);
    }
    
    if (strlen(msg->message) > 0) {
        printf("                 -> %s\n", msg->message);
    }
}

void cleanup_monitor() {
    printf("\n[Монитор] Завершение работы...\n");
    unlink(FIFO_NAME);
    exit(0);
}

int main() {
    signal(SIGINT, cleanup_monitor);
    signal(SIGTERM, cleanup_monitor);
    
    printf("========================================\n");
    printf("     МОНИТОР ЭКЗАМЕНА v3\n");
    printf("     (9 баллов: процесс-наблюдатель)\n");
    printf("========================================\n");
    printf("[Монитор] PID: %d\n", getpid());
    printf("[Монитор] Ожидание сообщений...\n");
    printf("----------------------------------------\n");

    unlink(FIFO_NAME);

    if (mkfifo(FIFO_NAME, 0666) == -1) {
        perror("mkfifo");
        exit(1);
    }
    
    printf("[Монитор] FIFO создан: %s\n", FIFO_NAME);
    printf("[Монитор] Готов к приему сообщений\n");
    printf("----------------------------------------\n");

    int fifo_fd = open(FIFO_NAME, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open FIFO");
        exit(1);
    }
    
    int message_count = 0;
    monitor_message_t msg;
    
    while (1) {
        ssize_t n = read(fifo_fd, &msg, sizeof(monitor_message_t));
        
        if (n > 0) {
            message_count++;
            print_message(&msg);

            if (message_count % 5 == 0) {
                printf("[Монитор] Получено сообщений: %d\n", message_count);
                printf("----------------------------------------\n");
            }
        } else if (n == 0) {

            printf("[Монитор] Нет активных процессов\n");
            sleep(1);
        } else {
            if (errno != EAGAIN) {
                perror("read");
                break;
            }
        }
        
        usleep(100000); 
    }
    
    close(fifo_fd);
    cleanup_monitor();
    return 0;
}