/* Copyright 2023 Birladeanu Raluca-Monica 315CA */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable.h"
#include "utils.h"

int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

unsigned int hash_function_string(void *a)
{
	/*
	 * Credits: http://www.cse.yorku.ca/~oz/hash.html
	 */
	unsigned char *puchar_a = (unsigned char*) a;
	unsigned long hash = 5381;
	int c;

	while ((c = *puchar_a++))
		hash = ((hash << 5u) + hash) + c; /* hash * 33 + c */

	return hash;
}

void key_val_free_function(void *data)
{
	free(((info*)data)->key);
	free(((info*)data)->value);
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t *ht = calloc(1, sizeof(hashtable_t));
	DIE(!ht, "Error allocating hashtable memory");
	ht->hmax = hmax;
	ht->buckets = calloc(ht->hmax, sizeof(linked_list_t*));
	DIE(!ht->buckets, "Error allocating buckets memory");
	for (unsigned int i = 0; i < ht->hmax; i++)
		ht->buckets[i] = ll_create(sizeof(info));
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;
	return ht;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax;
	linked_list_t* bucket = ht->buckets[index];
	ll_node_t* node = bucket->head;
	while (node) {
		info* current_info = (info*) node->data;
		if (ht->compare_function(key, current_info->key) == 0)
			return 1;
		node = node->next;
	}
	return 0;
}

void *ht_get(hashtable_t *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax;
	linked_list_t* bucket = ht->buckets[index];
	ll_node_t* node = bucket->head;
	while (node) {
		info* current_info = (info*) node->data;
		if (ht->compare_function(key, current_info->key) == 0)
			return current_info->value;
		node = node->next;
	}
	return NULL;
}

void ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	linked_list_t* bucket = ht->buckets[index];
	ll_node_t* node = bucket->head;
	info* new_info = malloc(sizeof(info));

	new_info->key = malloc(key_size);
	memcpy(new_info->key, key, key_size);
	new_info->value = malloc(value_size);
	memcpy(new_info->value, value, value_size);

	if (ht_has_key(ht, key) == 0) {
		ll_add_nth_node(bucket, 0, new_info);
		free(new_info);
	} else {
		while (node) {
			info* current_info = (info*) node->data;
			if (ht->compare_function(key, current_info->key) == 0) {
				free(current_info->value);
				current_info->value = malloc(value_size);
				memcpy(current_info->value, new_info->value, value_size);
				free(new_info->key);
				free(new_info->value);
				free(new_info);
			}
			node = node->next;
		}
	}
}

void ht_remove_entry(hashtable_t *ht, void *key)
{
	int index = ht->hash_function(key) % ht->hmax;
	linked_list_t* bucket = ht->buckets[index];
	ll_node_t* node = bucket->head;
	int count = 0;
	while (node) {
		info* current_info = (info*) node->data;
		if (ht->compare_function(key, current_info->key) == 0) {
			key_val_free_function(current_info);
			ll_remove_nth_node(ht->buckets[index], count);
			return;
		}
		count++;
		node = node->next;
	}
}

void ht_free(hashtable_t *ht)
{
	for (unsigned int i = 0; i < ht->hmax; i++) {
		linked_list_t* list = ht->buckets[i];
		ll_free(&list);
	}
	free(ht->buckets);
	free(ht);
}
