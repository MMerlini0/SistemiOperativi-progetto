CC=gcc
CCOPTS=--std=gnu99 -Wall -D_LIST_DEBUG_ 
AR=ar

OBJS=bit_map.o \
    buddy_allocator.o

HEADERS=bit_map.h buddy_allocator.h

LIBS=libbuddy.a

BIN=buddy_allocator_test.exe

.PHONY: all clean

all: $(BIN)

%.o: %.c $(HEADERS)
	$(CC) $(CCOPTS) -c -o $@ $<

$(LIBS): $(OBJS)
	$(AR) -rcs $@ $^

$(BIN): buddy_allocator_test.o $(LIBS)
	$(CC) $(CCOPTS) -o $@ $^ -lm

clean:
	-del /Q *.o *.a *.exe 2>NUL
