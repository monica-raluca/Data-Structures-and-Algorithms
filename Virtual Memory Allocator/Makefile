#Birladeanu Raluca-Monica 315CA

CC=gcc
CFLAGS=-Wall -Wextra -std=c99

DEPS= vma.h
OBJ= main.c vma.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

build: vma

vma: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

run_vma: build
	./vma

clean:
	rm -f vma *.o
