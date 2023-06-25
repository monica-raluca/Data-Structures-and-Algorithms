#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "vma.h"

#define MAX_LENGTH 256
int main(void)
{
	arena_t *arena;
	uint64_t address, size, arena_size;

	while (1) {
		char command[16];
		scanf("%s", command);
		if (strncmp(command, "FREE_BLOCK", 10) == 0) {
			scanf("%ld", &address);
			free_block(arena, address);
		} else if (strncmp(command, "ALLOC_ARENA", 11) == 0) {
			scanf("%lu", &arena_size);
			arena = alloc_arena(arena_size);
		} else if (strncmp(command, "ALLOC_BLOCK", 11) == 0) {
			scanf("%lu%lu", &address, &size);
			alloc_block(arena, address, size);
		} else if (strncmp(command, "PMAP", 4) == 0) {
			pmap(arena);
		} else if (strncmp(command, "WRITE", 5) == 0) {
			int error_count = 0, character;
			scanf("%lu%lu", &address, &size);
			int8_t *input = malloc(size + 1);
			int remaining_characters = size;
			size_t index = 0;
			character = getchar();
			while (remaining_characters) {
				character = getchar();
				input[index++] = (char)character;
				remaining_characters--;
				if (!remaining_characters)
					break;
			}
			input[index] = '\0';
			if (character != '\n')
				while ((character = getchar()) != ' ' && character != '\n')
					index++;
			if (character != '\n' || (index > (strlen((char *)input) + 1))) {
				error_count += 4;
				while (error_count) {
					printf("Invalid command. Please try again.\n");
					error_count--;
				}
				free(input);
			} else {
				write(arena, address, size, input);
				free(input);
			}
		} else if (strncmp(command, "READ", 4) == 0) {
			scanf("%lu%lu", &address, &size);
			read(arena, address, size);
		} else if (strncmp(command, "MPROTECT", 8) == 0) {
			char *input = malloc(MAX_LENGTH + 1);
			fgets(input, MAX_LENGTH, stdin);
			char *p = strtok(input, " |\n");
			if (sscanf(p, "%ld", &address) != 1) {
				printf("Values werent't read properly");
				return NULL;
			}
			p = strtok(NULL, " |\n");
			change_permissions(arena, address, 0);
			while (p) {
				mprotect(arena, address, (int8_t *)p);
				change_permissions(arena, address, 1);
				p = strtok(NULL, " |\n");
			}
			change_permissions(arena, address, 0);
			free(input);
		} else if (strncmp(command, "DEALLOC_ARENA", 13) == 0) {
			dealloc_arena(arena);
			exit(0);
		} else {
			printf("Invalid command. Please try again.\n");
		}
	}
	return 0;
}
