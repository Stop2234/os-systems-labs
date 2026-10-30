
CC = gcc
CFLAGS = -Wall -Wextra -pthread -lrt
SERVER = exam_server_v3
CLIENT = exam_client_v3
MONITOR = exam_monitor_v3

all: $(SERVER) $(CLIENT) $(MONITOR)

$(SERVER): exam_server_v3.c common_v3.h
	$(CC) $(CFLAGS) exam_server_v3.c -o $(SERVER)

$(CLIENT): exam_client_v3.c common_v3.h
	$(CC) $(CFLAGS) exam_client_v3.c -o $(CLIENT)

$(MONITOR): exam_monitor_v3.c common_v3.h
	$(CC) $(CFLAGS) exam_monitor_v3.c -o $(MONITOR)

clean:
	rm -f $(SERVER) $(CLIENT) $(MONITOR)
	-rm -f /dev/shm/*exam* 2>/dev/null
	-rm -f /tmp/exam_fifo_v3 2>/dev/null

clean_all: clean
	-sem_unlink /exam_qfree_v3 2>/dev/null || true
	-sem_unlink /exam_qfill_v3 2>/dev/null || true
	-sem_unlink /exam_mutex_v3 2>/dev/null || true

demo:
	@echo "=== Демонстрация версии 3 ==="
	@make clean_all
	@make
	@echo "1. Запуск монитора..."
	@./$(MONITOR) > monitor.log 2>&1 &
	@sleep 1
	@echo "2. Запуск сервера..."
	@./$(SERVER) 3 > server.log 2>&1 &
	@sleep 2
	@echo "3. Запуск студентов..."
	@for i in 1 2 3; do \
		echo "   Студент $$i..."; \
		./$(CLIENT) $$i > client$$i.log 2>&1 & \
		sleep 1; \
	done
	@sleep 10
	@echo "4. Демонстрация завершена"
	@echo "   Логи: monitor.log server.log client*.log"
	@pkill -f $(SERVER) 2>/dev/null || true

.PHONY: all clean clean_all demo