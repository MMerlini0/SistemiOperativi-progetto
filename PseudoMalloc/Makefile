# Compilatore e flag
CC = gcc
CFLAGS = -Wall -std=gnu99

# Sorgenti e oggetti
SRC = bit_map.c buddy_allocator.c disastrOS_malloc.c
OBJ = $(SRC:.c=.o)
BIN = disastrOS_malloc

.PHONY: all clean

# Regola principale
all: $(BIN)

# Linking finale
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Compilazione dei .c in .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	rm -f *.o $(BIN)
