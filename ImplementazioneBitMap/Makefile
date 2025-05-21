CC=gcc
CFLAGS=-Wall -std=gnu99

SRC=bit_map.c buddy_allocator.c buddy_allocator_test.c disastrOS_malloc.c
OBJ=$(SRC:.c=.o)
BIN=disastrOS_malloc.exe

.PHONY: all clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	del /Q *.o *.exe 2> NUL
