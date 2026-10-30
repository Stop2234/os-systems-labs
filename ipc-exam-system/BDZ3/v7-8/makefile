CC = gcc
CFLAGS = -Wall -Wextra -pthread -lrt
SERVER = exam_server_v2
CLIENT = exam_client_v2

all: $(SERVER) $(CLIENT)

$(SERVER): exam_server_v2.c common_v2.h
	$(CC) $(CFLAGS) exam_server_v2.c -o $(SERVER)

$(CLIENT): exam_client_v2.c common_v2.h
	$(CC) $(CFLAGS) exam_client_v2.c -o $(CLIENT)

clean:
	rm -f $(SERVER) $(CLIENT)
	-rm -f /dev/shm/*exam* 2>/dev/null

run_server: $(SERVER)
	./$(SERVER) 3

run_client: $(CLIENT)
	./$(CLIENT) 1

test: all
	@echo "Запуск теста..."
	@./$(SERVER) 3 > server.log 2>&1 &
	@sleep 2
	@for i in 1 2 3; do \
		./$(CLIENT) $$i > client$$i.log 2>&1 & \
		sleep 1; \
	done
	@sleep 10
	@echo "Тест завершен"
	@echo "Логи: server.log client1.log client2.log client3.log"

.PHONY: all clean run_server run_client test
