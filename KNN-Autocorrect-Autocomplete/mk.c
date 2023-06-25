#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ALPHABET_SIZE 26
#define ALPHABET "abcdefghijklmnopqrstuvwxyz"
#define LENGTH 1024

typedef struct trie_node_t trie_node_t;
struct trie_node_t {
	/* Value associated with key (set if end_of_word = 1) */
	void *value;

	/* end of word is 0 if we do not have that word
	otherwise, it becomes a counter for the appearances of that word */
	int end_of_word;

	trie_node_t **children;
	int n_children;
};

typedef struct trie_t trie_t;
struct trie_t {
	trie_node_t *root;

	/* Number of keys */
	int size;

	/* Generic Data Structure */
	int data_size;

	/* Trie-Specific, alphabet properties */
	int alphabet_size;
	char *alphabet;

	/* Number of nodes, useful to test correctness */
	int nodes_counter;
};

/* Creates the node. Allocates the node and initializes the values */
trie_node_t *trie_create_node(trie_t *trie)
{
	trie_node_t *node = calloc(1, sizeof(trie_node_t));
	node->value = NULL;
	node->end_of_word = 0;
	node->n_children = 0;
	node->children = calloc(trie->alphabet_size + 1, sizeof(trie_node_t *));
	return node;
}

/* Creates the trie. Allocates the trie and initializes all its fields*/
trie_t *trie_create(int data_size, int alphabet_size, char *alphabet)
{
	trie_t *trie = calloc(1, sizeof(trie_t));

	trie->size = 0;
	trie->data_size = data_size;

	trie->alphabet_size = alphabet_size;
	trie->alphabet = calloc(alphabet_size + 1, sizeof(char));
	strcpy(trie->alphabet, alphabet);

	trie->nodes_counter = 1;

	trie->root = trie_create_node(trie);
	trie->root->value = calloc(1, trie->data_size);
	*(int *)trie->root->value = -1;
	return trie;
}

/* Inserts a node into the trie recursively - Helper function */
void insert(trie_t *trie, trie_node_t *new_node, char *key, void *value)
{
	trie_node_t *next_node;
	if (strlen(key) == 0) {
		if (!new_node->value)
			new_node->value = calloc(1, trie->data_size);
		memcpy(new_node->value, value, strlen(value));
		if (new_node->end_of_word > 0)
			new_node->end_of_word++;
		else
			new_node->end_of_word = 1;
		return;
	}

	int index = key[0] - 'a';
	next_node = new_node->children[index];

	if (!next_node) {
		next_node = trie_create_node(trie);
		new_node->children[index] = next_node;
		new_node->n_children++;
		trie->nodes_counter++;
	}

	insert(trie, next_node, key + 1, value);
}

/* Calls the recursive function for insert */
void trie_insert(trie_t *trie, char *key, void *value)
{
	insert(trie, trie->root, key, value);
}

/* Recursive function for removing a certain node */
int remove_rec(trie_t *trie, trie_node_t *node, char *key)
{
	trie_node_t *next_node;
	if (strlen(key) == 0) {
		if (node->end_of_word) {
			node->end_of_word = 0;
			free(node->value);
			node->value = NULL;
			return node->n_children == 0;
		}
		return 0;
	}

	int index = key[0] - 'a';
	next_node = node->children[index];

	if (next_node && remove_rec(trie, next_node, key + 1) == 1) {
		free(next_node->children);
		free(next_node);
		node->children[index] = NULL;
		node->n_children--;
		trie->nodes_counter--;
		if (node->n_children == 0 && node->end_of_word == 0)
			return 1;
	}
	return 0;
}

/* Calls the recursive remove function */
void trie_remove(trie_t *trie, char *key)
{
	remove_rec(trie, trie->root, key);
}

/* Performs dfs to free the nodes*/
void dfs(trie_node_t *node)
{
	for (int i = 0; i < ALPHABET_SIZE; i++)
		if (node->children[i])
			dfs(node->children[i]);
	if (node->value) {
		free(node->value);
		node->value = NULL;
	}
	free(node->children);
	free(node);
}

/* Frees everything */
void trie_free(trie_t **ptrie)
{
	dfs((*ptrie)->root);
	free((*ptrie)->alphabet);
	free((*ptrie));
}

/* Loads data into the trie from the 'path' file*/
void load_fun(trie_t *trie, char *path)
{
	char word[LENGTH];
	FILE *input = fopen(path, "r");
	while (!feof(input)) {
		fgets(word, LENGTH, input);
		char *p = strtok(word, " \t\n");
		while (p) {
			trie_insert(trie, p, p);
			p = strtok(NULL, " \t\n");
		}
	}
	fclose(input);
}

/* Searches for a certain prefix recursively. Returns a pointer
to the node if found, otherwise returns NULL */
trie_node_t *base_search(trie_node_t *node, char *word, int length)
{
	if (length == 1 && node->children[word[0] - 'a'])
		return node->children[word[0] - 'a'];

	/* Keeps searching until we finish the prefix we search for.
	If we do not find a certain letter from the prefix, we stop */
	while (length != 1) {
		if (node->children[word[0] - 'a'])
			return base_search(node->children[word[0] - 'a'],
							word + 1, length - 1);
		else
			return NULL;
	}

	return NULL;
}

/* Starts from the prefix and keeps searching
for the shortest word with that prefix with dfs
until we reach end of word (aka a valid word).
If found, we set the found value to 1. */
void dfs_1(trie_node_t *node, int *found)
{
	if (node->end_of_word && *found == 0) {
		printf("%s\n", (char *)node->value);
		*found = 1;
		return;
	}
	for (int i = 0; i < ALPHABET_SIZE; i++)
		if (node->children[i])
			dfs_1(node->children[i], found);
}

/* Starts from the prefix and uses dfs to determine
the shortest valid word. If it finds a shortest word, it
updates the current_word.
Since the current word is null at first, it updates the current
word at first without checking the length condition.*/
void dfs_2(trie_node_t *node, char *current_word)
{
	if (node->end_of_word) {
		if (current_word[0] == '\0')
			strcpy(current_word, (char *)node->value);
		else if (strlen(node->value) < strlen(current_word))
			strcpy(current_word, (char *)node->value);
	} else {
		for (int i = 0; i < ALPHABET_SIZE; i++)
			if (node->children[i])
				dfs_2(node->children[i], current_word);
	}
}

/* Starts from the prefix and uses dfs to determine
the word that is used most frequently. If it find a word with
more appearances, it updates the frequency and the current_word.
Since the current word is null at first, it updates the current
word at first without checking the frequency condition. */
void dfs_3(trie_node_t *node, trie_node_t *starting_node,
		   char *current_word, int *frequency)
{
	if (node->end_of_word) {
		if (current_word[0] == '\0') {
			strcpy(current_word, (char *)node->value);
			*frequency = node->end_of_word;
			for (int i = 0; i < ALPHABET_SIZE; i++)
				if (node->children[i])
					dfs_3(node->children[i], starting_node,
						  current_word, frequency);
		} else {
			if (node->end_of_word > *frequency) {
				strcpy(current_word, (char *)node->value);
				*frequency = node->end_of_word;
			}
			for (int i = 0; i < ALPHABET_SIZE; i++)
				if (node->children[i])
					dfs_3(node->children[i], starting_node,
						  current_word, frequency);
		}
	} else {
		for (int i = 0; i < ALPHABET_SIZE; i++)
			if (node->children[i])
				dfs_3(node->children[i], starting_node,
					  current_word, frequency);
	}
}

/* Implements all autocomplete tasks.
If k is 0, then it calls the function 3 times,
with all the possible values for k. At first, the
function uses base_search to get to prefix we are searching for,
since every we return needs to contain that prefix. */
void autocomplete(trie_t *trie, char *word, int k)
{
	trie_node_t *starting_node;
	starting_node = base_search(trie->root, word, strlen(word));
	if (!starting_node) {
		if (k == 0)
			for (int i = 0; i < 3; i++)
				printf("No words found\n");
		else
			printf("No words found\n");
		return;
	}
	if (k == 0) {
		for (int i = 1; i < 4; i++)
			autocomplete(trie, word, i);
	} else if (k == 1) {
		int found = 0;
		dfs_1(starting_node, &found);
	} else if (k == 2) {
		char current_word[LENGTH];
		current_word[0] = '\0';
		dfs_2(starting_node, current_word);
		printf("%s\n", current_word);
	} else if (k == 3) {
		char current_word[LENGTH];
		current_word[0] = '\0';
		int freq = 0;
		dfs_3(starting_node, starting_node, current_word, &freq);
		printf("%s\n", current_word);
	}
}

/* Uses dfs to implement the autocorrect function.

word - is the one we autocorrect
k - the maximum number of character changes allowed
length - the length of the original word

Each time we encounter a different letter, we call the function
recursively with our k decreased, otherwise we keep the k intact.
Length decreases each time, since we want our words to have the same
length. If we did maximum k changes, have the same length and an
actual word (end of word is not null), and no words have been printed,
we print the word we found. */
int dfs_correct(trie_node_t *node, char *word, int k, int length)
{
	/* Printed is initially 0.
	The function uses bitwise operator to combine the results
	of multiple recursive calls of the function, making
	printed 1 when a word has been printed (when the function
	returns 1 too)*/
	int printed = 0;
	for (int i = 0; i < ALPHABET_SIZE && word[0] != '\0'; i++) {
		if (node->children[i]) {
			if ('a' + i != word[0])
				printed |= dfs_correct(node->children[i],
						   word + 1, k - 1, length - 1);
			else
				printed |= dfs_correct(node->children[i],
						   word + 1, k, length - 1);
		}
	}
	if (k >= 0 && !length && node->end_of_word && !printed) {
		printf("%s\n", (char *)node->value);
		printed = 1;
	}
	return printed;
}

/* Calls the dfs_correct function. If our variable printed is
still 0, we print that no words were found. */
void autocorrect(trie_t *trie, char *word, int k)
{
	int printed = dfs_correct(trie->root, word, k, strlen(word));
	if (!printed)
		printf("No words found\n");
}

int main(void)
{
	char command[20], word[LENGTH];
	trie_t *trie = trie_create(LENGTH, ALPHABET_SIZE, ALPHABET);
	int k;

	while (1) {
		scanf("%s", command);
		if (strncmp(command, "INSERT", 6) == 0) {
			scanf("%s", word);
			trie_insert(trie, word, word);
		} else if (strncmp(command, "LOAD", 4) == 0) {
			char path[100];
			scanf("%s", path);
			load_fun(trie, path);
		} else if (strncmp(command, "REMOVE", 6) == 0) {
			scanf("%s", word);
			trie_remove(trie, word);
		} else if (strncmp(command, "AUTOCOMPLETE", 12) == 0) {
			scanf("%s%d", word, &k);
			autocomplete(trie, word, k);
		} else if (strncmp(command, "AUTOCORRECT", 11) == 0) {
			scanf("%s%d", word, &k);
			autocorrect(trie, word, k);
		} else if (strncmp(command, "EXIT", 4) == 0) {
			trie_free(&trie);
			return 0;
		}
	}
	return 0;
}
