# Compilatore e flag
CC = gcc
CFLAGS = -Wall -std=gnu99

# Sorgenti e oggetti
SRC = bit_map.c buddy_allocator.c disastrOS_malloc.c
OBJ = $(SRC:.c=.o)
BIN = disastrOS_malloc.exe

.PHONY: all clean

# Regola principale
all: $(BIN)

# Linking finale
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compilazione dei .c in .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	del /Q *.o *.exe 2> NUL
