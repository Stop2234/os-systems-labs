#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h> 

#define MAX_STUDENTS 50
#define MAX_FILENAME 256
#define DEFAULT_CONFIG_FILE "exam_config.txt"
#define DEFAULT_OUTPUT_FILE "exam_results.txt"

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
FILE* output_stream = NULL;
int use_file_output = 0;
int verbose_mode = 1;

void log_message(const char* format, ...) {
    va_list args1, args2;

    if (verbose_mode) {
        va_start(args1, format);
        vprintf(format, args1);
        va_end(args1);
    }

    if (use_file_output && output_stream) {
        va_start(args2, format);
        vfprintf(output_stream, format, args2);
        fflush(output_stream);
        va_end(args2);
    }
}

void handle_signal(int sig) {
    printf("\nПолучен сигнал прерывания. Завершение работы...\n");
    if (output_stream) {
        fprintf(output_stream, "\nПолучен сигнал прерывания. Завершение работы...\n");
    }
    running = 0;
}

void* teacher_thread(void* arg) {
    log_message("Преподаватель начинает проверку работ.\n");
    while (running) {
        sem_wait(&new_work);
        if (!running) break;

        pthread_mutex_lock(&queue_mutex);
        if (work_count > 0) {
            Student* s = works[--work_count];
            pthread_mutex_unlock(&queue_mutex);

            log_message("Преподаватель проверяет работу студента %d (билет %d)\n", s->id, s->ticket);
            int check_time = rand() % 3 + 1;
            sleep(check_time);
            
            s->grade = rand() % 4 + 2;
            log_message("Преподаватель выставил оценку %d студенту %d\n", s->grade, s->id);
            
            sem_post(&(s->student_done));
        } else {
            pthread_mutex_unlock(&queue_mutex);
        }
    }
    log_message("Преподаватель завершил работу.\n");
    return NULL;
}

void* student_thread(void* arg) {
    Student* s = (Student*)arg;
    log_message("Студент %d получает билет №%d\n", s->id, s->ticket);

    int prep_time = rand() % 5 + 1;
    sleep(prep_time);
    log_message("Студент %d подготовил ответ за %d сек\n", s->id, prep_time);

    pthread_mutex_lock(&queue_mutex);
    works[work_count++] = s;
    pthread_mutex_unlock(&queue_mutex);

    sem_post(&new_work);
    
    sem_wait(&(s->student_done));
    
    log_message("Студент %d получил оценку %d\n", s->id, s->grade);
    
    sem_destroy(&(s->student_done));
    free(s);
    return NULL;
}

int load_config_from_file(const char* filename) {
    FILE* config_file = fopen(filename, "r");
    if (!config_file) {
        printf("Ошибка: не удалось открыть конфигурационный файл '%s'\n", filename);
        return -1;
    }
    
    int students;
    if (fscanf(config_file, "%d", &students) != 1) {
        printf("Ошибка: некорректный формат конфигурационного файла\n");
        fclose(config_file);
        return -1;
    }
    
    fclose(config_file);
    return students;
}

void print_usage(const char* prog_name) {
    printf("Использование: %s [ПАРАМЕТРЫ]\n", prog_name);
    printf("Параметры:\n");
    printf("  -n <число>    Количество студентов (1-%d)\n", MAX_STUDENTS);
    printf("  -c <файл>     Конфигурационный файл (вместо -n)\n");
    printf("  -o <файл>     Файл для вывода результатов\n");
    printf("  -q            Тихий режим (только вывод в файл)\n");
    printf("  -v            Подробный режим (по умолчанию)\n");
    printf("  -h            Вывод этой справки\n");
    printf("\nПримеры:\n");
    printf("  %s -n 5 -o results.txt\n", prog_name);
    printf("  %s -c config.txt -o results.txt -q\n", prog_name);
    printf("  %s -n 10                        (только консольный вывод)\n", prog_name);
    printf("\nФормат конфигурационного файла:\n");
    printf("  <количество_студентов>\n");
    printf("  (одно число от 1 до %d)\n", MAX_STUDENTS);
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    signal(SIGINT, handle_signal);

    total_students = 0;
    char* config_file = NULL;
    char* output_file = NULL;
    verbose_mode = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 && i + 1 < argc) {
            total_students = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) {
            config_file = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            output_file = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-q") == 0) {
            verbose_mode = 0;
        } else if (strcmp(argv[i], "-v") == 0) {
            verbose_mode = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            print_usage(argv[0]);
            return 0;
        } else {
            printf("Ошибка: неизвестный параметр '%s'\n\n", argv[i]);
            print_usage(argv[0]);
            return 1;
        }
    }

    if (config_file) {
        int students_from_file = load_config_from_file(config_file);
        if (students_from_file > 0) {
            total_students = students_from_file;
            printf("Загружено из файла '%s': %d студентов\n", config_file, total_students);
        } else {
            return 1;
        }
    }
 
    if (total_students <= 0 || total_students > MAX_STUDENTS) {
        printf("Ошибка: количество студентов должно быть от 1 до %d\n", MAX_STUDENTS);
        printf("Текущее значение: %d\n\n", total_students);
        print_usage(argv[0]);
        return 1;
    }
 
    if (output_file) {
        output_stream = fopen(output_file, "w");
        if (!output_stream) {
            printf("Ошибка: не удалось открыть файл для вывода '%s'\n", output_file);
            return 1;
        }
        use_file_output = 1;
        printf("Вывод результатов будет сохранен в файл: %s\n", output_file);
    }

    log_message("=== МОДЕЛИРОВАНИЕ ЭКЗАМЕНА ===\n");
    log_message("Количество студентов: %d\n", total_students);
    if (config_file) {
        log_message("Конфигурационный файл: %s\n", config_file);
    }
    time_t now = time(NULL);
    log_message("Время запуска: %s", ctime(&now));
    log_message("\n");

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
        usleep((rand() % 2000) * 1000); 
    }

    for (int i = 0; i < total_students; i++) {
        pthread_join(students[i], NULL);
    }

    running = 0;
    sem_post(&new_work);
    pthread_join(teacher, NULL);

    sem_destroy(&new_work);
    pthread_mutex_destroy(&queue_mutex);

    log_message("\n=== СТАТИСТИКА ЭКЗАМЕНА ===\n");
    log_message("Всего студентов: %d\n", total_students);
    now = time(NULL);
    log_message("Время завершения: %s", ctime(&now));
    log_message("Экзамен успешно завершен.\n");
    
    if (output_stream) {
        fclose(output_stream);
        if (verbose_mode) {
            printf("\nРезультаты сохранены в файл: %s\n", output_file);
        }
    }

    return 0;
}