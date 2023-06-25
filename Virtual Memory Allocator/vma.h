#pragma once
#include <inttypes.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef struct node {
	void *data;
	struct node *next, *prev;
} node;

typedef struct linked_list {
	node *head;
	unsigned int data_size;
	unsigned int size;
} list_t;

typedef struct miniblock_t {
	uint64_t start_address;
	size_t size;
	uint8_t perm;
	void *rw_buffer;
	struct miniblock_t *next, *prev;
	int permissions_changed;
} miniblock_t;

typedef struct {
	uint64_t start_address;
	size_t size;
	miniblock_t *minblk_list, *tail;
} block_t;

typedef struct {
	uint64_t arena_size;
	uint64_t free_memory;
	uint64_t block_number;
	uint64_t miniblock_number;
	list_t *alloc_list;
} arena_t;

list_t *create_list(unsigned int data_size);
node *create_node(list_t *list, const void *new_data);
void push(list_t *list, unsigned int n, const void *new_data);
void pop(list_t *list, unsigned int n);
void free_list(list_t *list);
void print_list(list_t *list);

arena_t *alloc_arena(const uint64_t size);
void dealloc_arena(arena_t *arena);

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size);
block_t *init_block(arena_t *arena, const uint64_t address,
					const uint64_t size, miniblock_t *new_miniblock);
miniblock_t *init_miniblock(arena_t *arena, const uint64_t address,
							const uint64_t size);
int out_of_bounds(arena_t *arena, const uint64_t address, const uint64_t size);

void free_block(arena_t *arena, const uint64_t address);
void free_first(arena_t *arena, node *temp, miniblock_t *tmp_minblk);
void free_middle(arena_t *arena, node *temp, miniblock_t *tmp_minblk, int idx);
void free_last(arena_t *arena, node *temp, miniblock_t *tmp_minblk);

void read(arena_t *arena, uint64_t address, uint64_t size);
void write(arena_t *arena, const uint64_t address,  const uint64_t size,
		   int8_t *data);

void pmap(const arena_t *arena);
void mprotect(arena_t *arena, uint64_t address, int8_t *permission);
void change_permissions(arena_t *arena, uint64_t address, int changed);

int min(int a, int b);
