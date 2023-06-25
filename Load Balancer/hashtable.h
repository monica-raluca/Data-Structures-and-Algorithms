/* Copyright 2023 Birladeanu Raluca-Monica 315CA */
#ifndef HASHTABLE_H_
#define HASHTABLE_H_

#define MAX_STRING_SIZE	256
#define HMAX 100
#include "linked_list.h"

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	linked_list_t **buckets;
	unsigned int hmax;
	unsigned int (*hash_function)(void*);
	int (*compare_function)(void*, void*);
	void (*key_val_free_function)(void*);
};

/** compare_function_strings() - Compares two strings using strcmp
 *
 * @arg1: first string
 * @arg2: second string
 * 
 * Return: the result of the comparison with strcmp
 */
int compare_function_strings(void *a, void *b);

/** hash_function_string() - Hashes a given string
 *
 * @arg1: string to be hashed
 * 
 * Return: hashed value.
 */
unsigned int hash_function_string(void *a);

/** key_val_free_function() - Call this function to free the memory
 * 							  ocupied by the key and the value of a pair
 *
 * @arg1: data we have to free
 */
void key_val_free_function(void *data);

/** ht_create() - Initializes the memory for a struct hashtable
 * 				  While also initializing all of its values
 *
 * @arg1: max number of buckets
 * @arg2: hash function
 * @arg3: compare function
 * @arg4: key and value free function
 * 
 * Return: pointer to the allocated hashtable_t struct.
 */
hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*));

/** ht_has_key() - Returns whether or not the key is in the hashtable
 *
 * Return: 1, if the key has a previously associated value in the hashtable
 * 		   0, otherwise
 */
int ht_has_key(hashtable_t *ht, void *key);

/** ht_get() - Retrieves the value of a specific key
 *
 * Return: the value, in case it was found
 * 		   NULL, otherwise
 */
void *ht_get(hashtable_t *ht, void *key);

/** ht_put() - Places a key-value pair into the hashtable
 *
 * @arg1: Hashtable in which we want to insert.
 * @arg2: Key represented as a string.
 * @arg3: Key size.
 * @arg4: Value represented as a string.
 * @arg5: Value size.
 */
void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size);

/** ht_remove_entry() - Removes an entry from the hashtable
 *
 * @arg1: Hashtable from which we want to remove the entry.
 * @arg2: Key represented as a string.
 */
void ht_remove_entry(hashtable_t *ht, void *key);

/** ht_free() - Frees all the entries from the hashtable.
 * 				Afterwards frees the hashtable itself.
 *
 * @arg1: Hashtable we want to free.
 */
void ht_free(hashtable_t *ht);

#endif /* HASHTABLE_H_ */
