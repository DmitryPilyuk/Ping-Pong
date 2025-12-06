CC=gcc
CFLAGS=-Wall -Wextra -Werror -g 
LDFLAGS=

TARGET=ping_pong
SRC=main.c sync.c
OBJ=$(SRC:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJ)
