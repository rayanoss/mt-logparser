CC = gcc
CFLAGS = -Wall -Wextra -Werror -g -std=c11 -Iinclude
LDFLAGS = -lm -pthread

# Source files
SRCS = src/main.c src/parser.c src/stats.c src/sort.c src/hash_table.c
OBJS = $(SRCS:.c=.o)

# Headers
HEADERS = include/parser.h include/stats.h include/sort.h include/hash_table.h

# Target executable
TARGET = parser

.PHONY: all clean run

# Default target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

# Compile with header dependencies
%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Run the parser
run: $(TARGET)
	./$(TARGET)

# Clean build artifacts
clean:
	rm -f $(OBJS) $(TARGET)