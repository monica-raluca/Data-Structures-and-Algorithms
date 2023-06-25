#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vma.h"

/* Returns a pointer to the linked list,
while also allocating the list and initializing
its fiels. */
list_t *create_list(unsigned int data_size)
{
	list_t *list = (list_t *)malloc(sizeof(list_t));
	if (!list) {
		perror("Error allocating memory");
		return NULL;
	}
	list->head = NULL;
	list->data_size = data_size;
	list->size = 0;
	return list;
}

/* Returns a pointer to the node,
while also allocating memory for it and
initializing the other fields. */
node *create_node(list_t *list, const void *new_data)
{
	node *new_node = (node *)malloc(sizeof(node));
	if (!new_node) {
		perror("Error allocating memory");
		return NULL;
	}
	new_node->data = malloc(list->data_size);
	if (!new_node->data) {
		perror("Error allocating memory");
		return NULL;
	}
	memcpy(new_node->data, new_data, list->data_size);
	new_node->prev = NULL;
	new_node->next = NULL;
	return new_node;
}

/* Adds a node in the n-th position. */
void push(list_t *list, unsigned int n, const void *new_data)
{
	unsigned int pos = 0;
	node *new_node = create_node(list, new_data);
	if (!new_node) {
		perror("Error allocating memory");
		return;
	}
	if (n >= list->size)
		n = list->size;
	if (list->size == 0) {
		new_node->prev = NULL;
		new_node->next = NULL;
		list->head = new_node;
	} else if (n == 0) {
		new_node->next = list->head;
		new_node->prev = NULL;
		if (list->head)
			list->head->prev = new_node;
		list->head = new_node;
	} else {
		node *temp_node = list->head;
		while (pos < n - 1) {
			temp_node = temp_node->next;
			pos++;
		}
		new_node->next = temp_node->next;
		new_node->prev = temp_node;
		if (n != list->size)
			temp_node->next->prev = new_node;
		temp_node->next = new_node;
	}
	list->size++;
}

/* Deletes a node from the n-th position. */
void pop(list_t *list, unsigned int n)
{
	unsigned int pos = 0;
	if (!list)
		return;
	if (n >= list->size)
		n = list->size - 1;
	node *removed_node = list->head;
	node *prev_node = NULL;
	if (list->size == 1) {
		list->head = NULL;
	} else if (n == 0) {
		if (list->head) {
			list->head = list->head->next;
			if (list->head)
				list->head->prev = NULL;
		}
	} else {
		while (pos < n) {
			prev_node = removed_node;
			removed_node = removed_node->next;
			pos++;
		}
		prev_node->next = removed_node->next;
		if (removed_node->next)
			removed_node->next->prev = prev_node;
	}
	free(removed_node->data);
	free(removed_node);
	list->size--;
}

/* Frees the list by always deleting the first node */
void free_list(list_t *list)
{
	if (!list) {
		perror("List empty");
		return;
	}
	for (unsigned int i = 0; i < list->size; i++)
		pop(list, 0);
	free(list);
}

arena_t *alloc_arena(const uint64_t size)
{
	arena_t *arena = malloc(sizeof(arena_t));
	if (!arena) {
		perror("Error allocating memory");
		return NULL;
	}
	arena->alloc_list = create_list(sizeof(block_t));
	arena->arena_size = size;
	arena->free_memory = size;
	arena->block_number = 0;
	arena->miniblock_number = 0;
	return arena;
}

void dealloc_arena(arena_t *arena)
{
	node *temp = arena->alloc_list->head;
	while (temp) {
		int address = ((block_t *)temp->data)->start_address;
		miniblock_t *miniblock = ((block_t *)temp->data)->minblk_list;
		temp = temp->next;
		while (miniblock) {
			int miniblock_size = miniblock->size;
			miniblock = miniblock->next;
			free_block(arena, address);
			address += miniblock_size;
		}
	}
	arena->alloc_list->size = arena->block_number;
	free_list(arena->alloc_list);
	free(arena);
}

/* Initializes a block */
block_t *init_block(arena_t *arena, const uint64_t address,
					const uint64_t size, miniblock_t *new_miniblock)
{
	block_t *block = malloc(sizeof(block_t));
	block->size = size;
	block->start_address = address;
	block->minblk_list = new_miniblock;
	block->tail = new_miniblock;
	arena->alloc_list->size = arena->block_number;
	return block;
}

/* Initializes a miniblock*/
miniblock_t *init_miniblock(arena_t *arena, const uint64_t address,
							const uint64_t size)
{
	miniblock_t *new_miniblock = (miniblock_t *)malloc(sizeof(miniblock_t));
	new_miniblock->next = NULL;
	new_miniblock->prev = NULL;
	new_miniblock->size = size;
	new_miniblock->perm = 6;
	new_miniblock->start_address = address;
	new_miniblock->rw_buffer = malloc(size);
	memset(new_miniblock->rw_buffer, 0, size);
	arena->free_memory -= new_miniblock->size;
	arena->miniblock_number++;
	return new_miniblock;
}

/* Check if the address is valid */
int out_of_bounds(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (address >= arena->arena_size) {
		printf("The allocated address is outside the size of arena\n");
		return 0;
	} else if (address + size > arena->arena_size) {
		printf("The end address is past the size of the arena\n");
		return 0;
	}
	node *temp = arena->alloc_list->head;
	while (temp) {
		if ((address > ((block_t *)temp->data)->start_address &&
			 address < ((block_t *)temp->data)->start_address +
			((block_t *)temp->data)->size) ||
			(address + size < ((block_t *)temp->data)->start_address +
			((block_t *)temp->data)->size && address +
			 size > ((block_t *)temp->data)->start_address) ||
			(address <= ((block_t *)temp->data)->start_address &&
			 address + size >= ((block_t *)temp->data)->start_address +
			((block_t *)temp->data)->size)) {
			printf("This zone was already allocated.\n");
			return 0;
		}
		temp = temp->next;
	}
	return 1;
}

void alloc_block(arena_t *arena, const uint64_t address, const uint64_t size)
{
	if (!out_of_bounds(arena, address, size))
		return;
	miniblock_t *new_miniblock = init_miniblock(arena, address, size);
	node *temp = arena->alloc_list->head;
	int index = 0;
	while (temp) {
		index++;
		if (address + size == ((block_t *)temp->data)->start_address) {
			((block_t *)temp->data)->minblk_list->prev = new_miniblock;
			new_miniblock->next = ((block_t *)temp->data)->minblk_list;
			((block_t *)temp->data)->minblk_list = new_miniblock;
			((block_t *)temp->data)->start_address = address;
			((block_t *)temp->data)->size += size;
			if (temp->prev)
				if (((block_t *)temp->data)->start_address ==
				   ((block_t *)temp->prev->data)->start_address +
				   ((block_t *)temp->prev->data)->size) {
					((block_t *)temp->prev->data)->tail->next =
					((block_t *)temp->data)->minblk_list;
					((block_t *)temp->data)->minblk_list->prev =
					((block_t *)temp->prev->data)->tail;
					((block_t *)temp->prev->data)->tail =
					((block_t *)temp->data)->tail;
					((block_t *)temp->prev->data)->size +=
					((block_t *)temp->data)->size;
					arena->block_number--;
					pop(arena->alloc_list, index);
				}
			return;
		}
		if (address == ((block_t *)temp->data)->start_address +
		   ((block_t *)temp->data)->size) {
			((block_t *)temp->data)->tail->next = new_miniblock;
			new_miniblock->prev = ((block_t *)temp->data)->tail;
			((block_t *)temp->data)->tail = new_miniblock;
			((block_t *)temp->data)->size += size;
			if (temp->next)
				if (((block_t *)temp->data)->start_address +
				   ((block_t *)temp->data)->size ==
				   ((block_t *)temp->next->data)->start_address) {
					((block_t *)temp->next->data)->minblk_list->prev =
					((block_t *)temp->data)->tail;
					((block_t *)temp->data)->tail->next =
					((block_t *)temp->next->data)->minblk_list;
					((block_t *)temp->data)->tail =
					((block_t *)temp->next->data)->tail;
					((block_t *)temp->data)->size +=
					((block_t *)temp->next->data)->size;
					arena->block_number--;
					pop(arena->alloc_list, index);
				}
			return;
		}
		if (address < ((block_t *)temp->data)->start_address) {
			block_t *block = init_block(arena, address, size, new_miniblock);
			push(arena->alloc_list, index - 1, block);
			arena->block_number++;
			free(block);
			return;
		}
		if (address > ((block_t *)temp->data)->start_address +
		   ((block_t *)temp->data)->size && !temp->next) {
			block_t *block = init_block(arena, address, size, new_miniblock);
			push(arena->alloc_list, index, block);
			arena->block_number++;
			free(block);
			return;
		}
		temp = temp->next;
	}
	if (index == 0) {
		block_t *block = init_block(arena, address, size, new_miniblock);
		push(arena->alloc_list, index, block);
		arena->block_number++;
		free(block);
		return;
	}
}

/* We free the first miniblock */
void free_first(arena_t *arena, node *temp, miniblock_t *tmp_minblk)
{
	arena->free_memory += tmp_minblk->size;
	((block_t *)temp->data)->minblk_list = tmp_minblk->next;
	if (tmp_minblk->next)
		tmp_minblk->next->prev = NULL;

	/* The start address of the block will be
	the start address of the next miniblock */
	((block_t *)temp->data)->start_address =
	((block_t *)temp->data)->minblk_list->start_address;

	((block_t *)temp->data)->size -= tmp_minblk->size;

	free(tmp_minblk->rw_buffer);
	free(tmp_minblk);
	arena->miniblock_number--;
}

/* We free the middle miniblock*/
void free_middle(arena_t *arena, node *temp, miniblock_t *tmp_minblk, int idx)
{
	arena->free_memory += tmp_minblk->size;
	block_t *block = malloc(sizeof(block_t));
	block->start_address = tmp_minblk->next->start_address;

	block->size = (((block_t *)temp->data)->start_address +
	((block_t *)temp->data)->size) -
	(tmp_minblk->start_address + tmp_minblk->size);

	block->minblk_list = tmp_minblk->next;
	block->tail = ((block_t *)temp->data)->tail;
	((block_t *)temp->data)->tail = tmp_minblk->prev;

	((block_t *)temp->data)->size =
	((block_t *)temp->data)->size -
	tmp_minblk->size - block->size;

	arena->alloc_list->size = arena->block_number;
	push(arena->alloc_list, idx, block);
	free(block);

	tmp_minblk->prev->next = NULL;
	tmp_minblk->next->prev = NULL;
	free(tmp_minblk->rw_buffer);
	free(tmp_minblk);
	arena->miniblock_number--;
	arena->block_number++;
}

/* We free the last miniblock */
void free_last(arena_t *arena, node *temp, miniblock_t *tmp_minblk)
{
	arena->free_memory += tmp_minblk->size;
	((block_t *)temp->data)->tail = tmp_minblk->prev;
	tmp_minblk->prev->next = NULL;
	((block_t *)temp->data)->size -= tmp_minblk->size;
	free(tmp_minblk->rw_buffer);
	free(tmp_minblk);
	arena->miniblock_number--;
}

void free_block(arena_t *arena, const uint64_t address)
{
	node *temp = arena->alloc_list->head;
	int index = 0;
	// Go through blocks
	while (temp) {
		index++;
		// If we are in a valid block
		if (address >= ((block_t *)temp->data)->start_address &&
			address < ((block_t *)temp->data)->start_address +
			((block_t *)temp->data)->size) {
			miniblock_t *tmp_minblk = ((block_t *)temp->data)->minblk_list;
			// If there is only one miniblock
			if (!tmp_minblk->next &&
				address == tmp_minblk->start_address) {
				arena->free_memory += tmp_minblk->size;
				tmp_minblk->size = 0;
				((block_t *)temp->data)->tail = NULL;
				// If it is the first block
				if (!(block_t *)temp->prev) {
					arena->alloc_list->head = arena->alloc_list->head->next;
					if (arena->alloc_list->head)
						arena->alloc_list->head->prev = NULL;
				} else {
					temp->prev->next = temp->next;
					if (temp->next)
						temp->next->prev = temp->prev;
				}
				free(tmp_minblk->rw_buffer);
				free(tmp_minblk);
				free((block_t *)temp->data);
				free(temp);
				arena->miniblock_number--;
				arena->block_number--;
				return;
			}
			// Go through miniblocks
			while (tmp_minblk) {
				// Current miniblock
				if (address == tmp_minblk->start_address) {
					if (tmp_minblk->next && tmp_minblk->prev) {
						free_middle(arena, temp, tmp_minblk, index);
						return;
					} else if (!tmp_minblk->next) {
						free_last(arena, temp, tmp_minblk);
						return;
					} else if (!tmp_minblk->prev) {
						free_first(arena, temp, tmp_minblk);
						return;
					}
				}
				tmp_minblk = tmp_minblk->next;
			}
		}
		temp = temp->next;
	}
	printf("Invalid address for free.\n");
}

int min(int a, int b)
{
	if (a < b)
		return a;
	return b;
}

void read(arena_t *arena, uint64_t address, uint64_t size)
{
	int my_address = address, left_bytes = size;
	if (address > arena->arena_size)
		printf("Invalid address for read.\n");
	/* Going through blocks to determine if all
	miniblocks have write permissions */
	node *temp = arena->alloc_list->head;
	while (temp) {
		// If we are in a valid block
		if (address >= ((block_t *)temp->data)->start_address &&
			address <= ((block_t *)temp->data)->start_address +
		   ((block_t *)temp->data)->size) {
			// Go through its miniblocks
			miniblock_t *miniblock = ((block_t *)temp->data)->minblk_list;
			while (miniblock) {
				if (miniblock->perm / 4) {
					miniblock = miniblock->next;
				} else {
					printf("Invalid permissions for read.\n");
					return;
				}
			}
		}
		temp = temp->next;
	}
	temp = arena->alloc_list->head;
	char *text = malloc(size + 1);
	memset(text, '\0', size + 1);
	int read_bytes = 0;
	while (temp) {
		// If we are in a valid block
		if (address >= ((block_t *)temp->data)->start_address &&
			address <= ((block_t *)temp->data)->start_address
			+ ((block_t *)temp->data)->size) {
			// Go through its miniblocks
			miniblock_t *miniblock = ((block_t *)temp->data)->minblk_list;
			while (miniblock) {
				/* Read either the whole miniblock, or the left_bytes */
				int bytes_to_read = min(miniblock->size +
					miniblock->start_address - my_address, left_bytes);
				/* Form the text by concatenating the buffer, starting
				from a specifies offset in case we don't want to
				start reading at the beginning of the buffer. */
				strncat(text, miniblock->rw_buffer + my_address -
						miniblock->start_address, bytes_to_read);
				read_bytes += bytes_to_read;
				text[read_bytes] = '\0';
				left_bytes -= bytes_to_read;
				if (miniblock->next)
					my_address = miniblock->next->start_address;
				else if (!miniblock->next && left_bytes)
					printf("Warning: size was bigger than the block size. "
					"Reading %u characters.\n", read_bytes);
				miniblock = miniblock->next;
			}
		}
		temp = temp->next;
	}
	if (read_bytes == 0)
		printf("Invalid address for read.\n");
	else
		puts(text);
	free(text);
}

void write(arena_t *arena, const uint64_t address, const uint64_t size,
		   int8_t *data)
{
	int my_address = address, left_bytes = size;
	if (address >= arena->arena_size)
		printf("Invalid address for write.\n");
	/* Going through blocks to determine if all
	miniblocks have write permissions */
	node *temp = arena->alloc_list->head;
	while (temp) {
		// If we are in a valid block
		if (address >= ((block_t *)temp->data)->start_address &&
			address <= ((block_t *)temp->data)->start_address +
					   ((block_t *)temp->data)->size) {
			// Go through its miniblocks
			miniblock_t *miniblock = ((block_t *)temp->data)->minblk_list;
			while (miniblock) {
				int permissions = miniblock->perm;
				if (permissions / 4)
					permissions -= 4;
				if (permissions / 2) {
					miniblock = miniblock->next;
				} else {
					printf("Invalid permissions for write.\n");
					return;
				}
			}
		}
		temp = temp->next;
	}
	temp = arena->alloc_list->head;
	int written_bytes = 0;
	while (temp) {
		// If we are in a valid block
		if (address >= ((block_t *)temp->data)->start_address &&
			address <= ((block_t *)temp->data)->start_address +
		   ((block_t *)temp->data)->size) {
			// Go through its miniblocks
			miniblock_t *miniblock = ((block_t *)temp->data)->minblk_list;
			while (miniblock) {
				int offset = my_address - miniblock->start_address;
				/* Write either the whole miniblock, or the left_bytes */
				int bytes_to_write = min(miniblock->size - offset, left_bytes);
				/* Copy data into the buffer starting from the given address,
				making sure we keep writing from the message given from
				where we stopped after writing in the previous miniblock */

				memcpy(miniblock->rw_buffer + offset,
					   data + written_bytes, bytes_to_write);
				written_bytes += bytes_to_write;
				left_bytes -= bytes_to_write;
				if (miniblock->next)
					my_address = miniblock->next->start_address;
				/* If there is no miniblock left to write in,
				and we still have characters to write, print a message. */
				else if (!miniblock->next && written_bytes && left_bytes)
					printf("Warning: size was bigger than the block size. "
					"Writing %u characters.\n", written_bytes);
				miniblock = miniblock->next;
			}
		}
		temp = temp->next;
	}
	if (written_bytes == 0)
		printf("Invalid address for write.\n");
}

/* Uses memset to set initial permissions to
'---', and updates them based on the number. */
void permissions(int number)
{
	char *permissions = malloc(4 * sizeof(char));
	memset(permissions, '-', 3);
	permissions[3] = '\0';
	if (number / 4) {
		permissions[0] = 'R';
		number -= 4;
	}
	if (number / 2) {
		permissions[1] = 'W';
		number -= 2;
	}
	if (number) {
		permissions[2] = 'X';
		number--;
	}
	printf("%s\n", permissions);
	free(permissions);
}

/* Prints information about the arena using the
fields in arena, and then goes through each block
and miniblock to print the required info. */
void pmap(const arena_t *arena)
{
	printf("Total memory: 0x%lX bytes\n", arena->arena_size);
	printf("Free memory: 0x%lX bytes\n", arena->free_memory);
	printf("Number of allocated blocks: %lu\n", arena->block_number);
	printf("Number of allocated miniblocks: %lu\n", arena->miniblock_number);
	int block_index = 1, miniblock_index;
	node *temp = arena->alloc_list->head;
	while (temp) {
		printf("\n");
		printf("Block %d begin\n", block_index);
		printf("Zone: 0x%lX - 0x%lX\n",
			   ((block_t *)temp->data)->start_address,
			   ((block_t *)temp->data)->start_address +
			   ((block_t *)temp->data)->size);
		miniblock_index = 1;
		miniblock_t *tmp_minblk = ((block_t *)temp->data)->minblk_list;
		while (tmp_minblk) {
			printf("Miniblock %d:\t\t0x%lX\t\t-\t\t0x%lX\t\t| ",
				   miniblock_index, tmp_minblk->start_address,
				   tmp_minblk->start_address + tmp_minblk->size);
			permissions(tmp_minblk->perm);
			miniblock_index++;
			tmp_minblk = tmp_minblk->next;
		}
		printf("Block %d end\n", block_index);
		block_index++;
		temp = temp->next;
	}
}

/* Transforms the permissions into digits. */
uint8_t m_permissions(int8_t *permission)
{
	if (strcmp((char *)permission, "PROT_NONE") == 0)
		return 0;
	if (strcmp((char *)permission, "PROT_READ") == 0)
		return 4;
	if (strcmp((char *)permission, "PROT_WRITE") == 0)
		return 2;
	if (strcmp((char *)permission, "PROT_EXEC") == 0)
		return 1;
	return -1;
}

/* When the function is called, it changes the
permissions_changed parameter to symbolize that
mprotect was called with more parameters.
After the last mprotect call for that miniblock, the
permissions_changed variable is reset. */
void change_permissions(arena_t *arena, uint64_t address, int changed)
{
	node *temp = arena->alloc_list->head;
	while (temp) {
		if (address >= ((block_t *)temp->data)->start_address &&
			address < ((block_t *)temp->data)->start_address +
			((block_t *)temp->data)->size) {
			miniblock_t *tmp_minblk = ((block_t *)temp->data)->minblk_list;
			while (tmp_minblk) {
				if (address == tmp_minblk->start_address) {
					tmp_minblk->permissions_changed = changed;
					return;
				}
				tmp_minblk = tmp_minblk->next;
			}
		}
		temp = temp->next;
	}
}

/* Goes through each miniblock and changes permissions.
If the function was called with only one parameter,
overwrites the permission. Otherwise, adds it up. */
void mprotect(arena_t *arena, uint64_t address, int8_t *permission)
{
	node *temp = arena->alloc_list->head;
	while (temp) {
		if (address >= ((block_t *)temp->data)->start_address &&
			address < ((block_t *)temp->data)->start_address +
			((block_t *)temp->data)->size) {
			miniblock_t *tmp_minblk = ((block_t *)temp->data)->minblk_list;
			while (tmp_minblk) {
				if (address == tmp_minblk->start_address) {
					uint8_t perm = m_permissions(permission);
					if (tmp_minblk->permissions_changed == 0)
						tmp_minblk->perm = perm;
					else
						tmp_minblk->perm += perm;
					return;
				}
				tmp_minblk = tmp_minblk->next;
			}
		}
		temp = temp->next;
	}
	printf("Invalid address for mprotect.\n");
}
