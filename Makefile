CC=gcc
CFLAGS=-Wall -Wextra -Werror -g -std=c99 
LDFLAGS=

TARGET=ping_pong
SRC=main.c
OBJ=$(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
