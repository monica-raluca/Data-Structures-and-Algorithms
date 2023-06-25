/* Copyright 2023 Birladeanu Raluca-Monica 315CA */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "server.h"
#include "load_balancer.h"
#include "utils.h"

struct load_balancer {
    int server_number, max_size;
    server_memory **servers;
};

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

int binary_search(load_balancer *main, unsigned int label)
{
    int left = 0, right = main->server_number - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (main->servers[mid]->label <= label)
            left = mid + 1;
        else
            right = mid - 1;
    }
    return left;
}

load_balancer *init_load_balancer() {
    load_balancer *loadbalancer = calloc(1, sizeof(load_balancer));
    DIE(!loadbalancer, "Error allocating load balancer memory");
    loadbalancer->servers = NULL;
    loadbalancer->server_number = 0;
    loadbalancer->max_size = 0;
    return loadbalancer;
}

void loader_add_server(load_balancer *main, int server_id) {
    int replica_id = 0;
    /* If this is the first server to be added */
    if (!main->server_number) {
        main->servers = calloc(3, sizeof(server_memory *));
        DIE(!main->servers, "Error allocating servers");
        main->max_size = 3;
        for (int i = 0; i < 3; i++) {
            int label = replica_id * 100000 + server_id;
            /* Initialize a new temporary server */
            server_memory *new_server = init_server_memory();
            new_server->label = hash_function_servers(&label);
            new_server->server_id = server_id;
            replica_id++;
            /* Find the index on the hash ring */
            int index = binary_search(main, new_server->label);
            /* Shift the servers */
            memmove(&main->servers[index + 1], &main->servers[index],
                    sizeof(server_memory *) * (main->server_number - index));
            main->server_number++;
            main->servers[index] = new_server;
            /* Add the server to the hash ring */
            memcpy(main->servers[index], new_server, sizeof(server_memory));
        }
    } else {
        /* If the servers array needs to be resized, double its size */
        if (main->server_number + 3 > main->max_size) {
            main->servers = realloc(main->servers,
                            (main->max_size * 2) * sizeof(server_memory *));
            main->max_size *= 2;
        }
        for (int i = 0; i < 3; i++) {
            int label = replica_id * 100000 + server_id;
            server_memory *new_server = init_server_memory();
            new_server->label = hash_function_servers(&label);
            new_server->server_id = server_id;
            replica_id++;
            int index = binary_search(main, new_server->label);
            memmove(&main->servers[index + 1], &main->servers[index],
                    sizeof(server_memory *) * (main->server_number - index));
            main->servers[index] = new_server;
            memcpy(main->servers[index], new_server, sizeof(server_memory));
            main->server_number++;
            server_memory *temp_server =
                            main->servers[(index + 1) % main->server_number];
            for (unsigned int j = 0; j < temp_server->hashtable->hmax; j++)
                if (temp_server->hashtable->buckets[j]) {
                    ll_node_t *temp_node;
                    temp_node = temp_server->hashtable->buckets[j]->head;
                    while (temp_node) {
                        void *key = ((info *)temp_node->data)->key;
                        if (hash_function_key(key) <= new_server->label ||
                            hash_function_key(key) > temp_server->label)
                            /* Redistribute the objects */
                            server_store(main->servers[index],
                            ((info*)temp_node->data)->key,
                            ((info *)temp_node->data)->value);
                        temp_node = temp_node->next;
                    }
                }
        }
    }
}

void loader_remove_server(load_balancer *main, int server_id) {
    int remove_index = -1;
    for (int i = 0; i < main->server_number; i++) {
        server_memory *server = main->servers[i];
        if (server->server_id == server_id) {
            remove_index = i;
            // Redistribute objects on other servers
            for (unsigned int j = 0; j < server->hashtable->hmax; j++) {
                if (server->hashtable->buckets[j]) {
                    ll_node_t *temp_node = server->hashtable->buckets[j]->head;
                    while (temp_node) {
                        int tmp_idx = (remove_index + 1) % main->server_number;
                        server_store(main->servers[tmp_idx],
                        ((info*)temp_node->data)->key,
                        ((info *)temp_node->data)->value);
                        temp_node = temp_node->next;
                    }
                }
            }
            free_server_memory(server);
            main->server_number--;
            memmove(&main->servers[i], &main->servers[i + 1],
                    sizeof(server_memory *)*(main->server_number - i));
            i--;
        }
    }
    /* If a server was removed */
    if (remove_index >= 0) {
        // Resize servers array if necessary
        if (main->server_number < main->max_size / 2) {
            main->max_size /= 2;
            /* Make a new resized array */
            server_memory **new_servers =
                            calloc(main->max_size, sizeof(server_memory*));
            DIE(!new_servers, "Error allocating memory for the new servers");
            for (int i = 0; i < main->server_number; i++) {
                memcpy(main->servers[i], new_servers[i], sizeof(server_memory));
            }
            free(main->servers);
            main->servers = new_servers;
        }
    }
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id) {
    int hashed_key = hash_function_key(key);
    int index = binary_search(main, hashed_key);
    if (index == main->server_number)
        index = 0;
    *server_id = main->servers[index]->server_id;
    server_memory *server = main->servers[index];
    server_store(server, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id) {
    int hashed_key = hash_function_key(key);
    int index = binary_search(main, hashed_key);
    if (index == main->server_number)
        index = 0;
    *server_id = main->servers[index]->server_id;
    server_memory *server = main->servers[index];
    if (server)
        return server_retrieve(main->servers[index], key);
    return NULL;
}

void free_load_balancer(load_balancer *main) {
    for (int i = 0; i < main->server_number; i++) {
        server_memory *server = main->servers[i];
        free_server_memory(server);
    }
    free(main->servers);
    free(main);
}

