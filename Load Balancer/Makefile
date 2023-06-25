CC=gcc
CFLAGS=-Wall -Wextra -std=c99
DEPS= hashtable.h linked_list.h load_balancer.h server.h utils.h
OBJ= hashtable.o linked_list.o load_balancer.o server.o main.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: tema2

tema2: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run_load_balancer: build
	./tema2

.PHONY: clean

clean:
	rm -f *.o tema2
