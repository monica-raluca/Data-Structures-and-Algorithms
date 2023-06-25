/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "utils.h"

server_memory *init_server_memory()
{
	server_memory *server = calloc(1, sizeof(struct server_memory));
	DIE(!server, "Error allocating memory");
	server->hashtable = ht_create(HMAX, hash_function_string,
						compare_function_strings, key_val_free_function);
	server->number_entries = 0;
	server->label = 0;
	server->server_id = 0;
	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	ht_put(server->hashtable, key, strlen(key) + 1, value, strlen(value) + 1);
	server->number_entries++;
}

char *server_retrieve(server_memory *server, char *key) {
	return ht_get(server->hashtable, key);
}

void server_remove(server_memory *server, char *key) {
	ht_remove_entry(server->hashtable, key);
	server->number_entries--;
}

void free_server_memory(server_memory *server) {
	ht_free(server->hashtable);
	free(server);
}
