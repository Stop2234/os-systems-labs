CC = gcc
CFLAGS = -Wall -Wextra -pthread -lrt
TARGET = exam_v1
SOURCES = exam_system_v1.c
HEADERS = common_v1.h

all: $(TARGET)

$(TARGET): $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f *.o

run: $(TARGET)
	./$(TARGET) 3

run5: $(TARGET)
	./$(TARGET) 5

debug: $(SOURCES) $(HEADERS)
	$(CC) $(CFLAGS) -g $(SOURCES) -o $(TARGET)_debug
	gdb ./$(TARGET)_debug

.PHONY: all clean run run5 debug