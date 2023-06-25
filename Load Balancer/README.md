## Birladeanu Raluca-Monica 315CA

--------------------------------------
# Structures used

## linked_list
        Uses the lab implementation for linked lists.

## hashtable
        The hashtable will have the buckets, an array of simply linked lists, as well as the number of buckets hmax and the hash, compare and key and value free functions.

## server_memory
        Each server will contain the hashtable that will store the entries, the number of entries in the hashtable, the label - which is basically the hash - and the server_id (the replicas will have the same server_id).

## load_balancer
        The loadbalancer has an array of pointers to the servers, as well as the number of servers stored on it and the maximum size, which represents the maximum capacity that the loadbalancer can store at a certain point, it getting resized if necessary. 
-------------------------------------
# Server functions

For the server functions, the program uses the hashtable functionalities. 

## init_server_memory
        Initializes the server memory struct and all of its fields. Creates the hashtable and sets the number of entries, label and server_id to 0. 

## server_store
        Adds the entry in the hashtable and increases the number of entries. 

## server_retrieve 
        Removes the entry with a certain key from the hashtable and decreases the number of entries. 

## free_server_memory
        First frees the hashtable, then frees the server itself. 

--------------------------------------------
# Load_balancer functions

## init_load_balancer
        Initializes the load balancer struct and all of its fields. Sets the servers array to NULL and the server number and max size to 0.

## free_load_balancer
        First frees every server, than the pointer to the servers array itself, lastly also freeing the load balancer.

## loader_store
        Stores the key-value pair. First determines the hash of the key and then uses binary search to find the index of the server the key-value pair should be stored on. If the index is equal to the number of servers, the index is set to 0 since the array works somehow like a circular array. After the server is found, the server_store function is called to store the pair on it.
## loader_retrieve
        This function retrieves the given key. First determines the hash of the key and then uses binary search to find the index of the server the key-value pair is stored on. After the index is determined, it calls the server_retrieve function to get the desired value. 
## loader_add_server
        This function treats the first server added individually, since there are no objects to distribute, nor do we need to resize the load balancer. We start my allocating memory for the server and its replicas, and setting the max size to 3. From there, we find the position of the server to be added with binary search and shift the servers to make space for the new one, copying the new server and all of its information into the load balancer.
        If we already have servers into the load balancer, we first check if the load balancer needs to be resized. To be more efficient, we double the size everytime we resize so that we don't always realloc with every server added. Like before, we find the index with binary search and shift the elements accordingly. This time, however, we want to see if there is any object that needs to be redistributed on our new server. We go through each bucket and check whether the object needs to go on our new server. If the condition checks, we use server_store to move the object on the desired server.
## loader_remove_server
        This function removes the server with the given server_id, among with its replicas. We start with a remove_index, who will represent the server we have to remove. We go through each server and if the server id of the server is equal to the one we are looking for we modify the remove_index and start redistributing the objects on the other servers. Every time, we place all the objects that are on our server to be removed on the server right after it, we then free the server, decrement the server number and shift all the servers, making sure to decrement our counter for the for loop to make sure there is no server skipped. We keep going through the load balancer until we reach the last server we have.
        After we finish removing the server and the replicas, if the remove index was modified, this means that we removed some servers so we have to see if resizing the servers array is necessary. If we dropped below the resize capacity, we allocate a new array of servers and put all the elements in it, and then we attribute the new_servers to our array of server pointers from the load balancer.  

## Thanks for reading, I hope you have a great day! <3

⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢴⠒⢢⡀⢀⡤⢤⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠷⣄⡓⡋⣀⣴⠗⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣠⣤⣀⣤⣴⠶⠶⠛⠛⠛⠻⢿⣥⣽⣷⣤⣼⢃⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣾⠋⠁⠀⣿⠀⠀⠁⠀⣤⡶⠶⢶⣄⠀⠀⠀⠈⢻⡏⠙⢷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣼⠿⠶⠶⠾⠋⠀⠀⠀⠘⢿⣤⣤⣤⠿⠃⠀⠀⠀⠈⠛⠷⠦⠿⢷⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡶⢿⡅⠀⣠⡴⠶⠛⠛⠳⠶⠶⠶⠶⠶⠶⠾⠟⠛⠛⠳⠶⣦⣄⠀⠀⢀⣿⠷⣦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢾⡅⣸⡿⣾⠏⢀⠀⠀⠐⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠆⠀⠀⠀⠙⢿⣦⠸⣧⣠⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⣟⡛⢹⡏⠀⠈⠑⢠⣶⠟⠳⣆⣀⣀⣀⣀⢀⣴⠾⠳⣦⡀⠐⠀⠀⠹⣧⠈⢹⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣷⠃⠸⣷⡉⠀⢠⡿⠁⣶⠀⠙⣉⠉⣉⡉⠙⠃⣶⡆⢸⡇⠀⠀⠀⢀⡿⠰⣿⣽⠇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣆⡀⠙⢷⣤⡟⠀⠀⠉⠀⠀⠙⠛⠛⠁⠀⠀⠉⠀⠈⢻⡄⢀⣠⡾⠃⢀⣼⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠛⠷⣦⣬⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⣿⣭⣩⣤⣴⠿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣹⡷⣀⠀⠀⣀⡀⠀⠀⠀⠀⠀⠀⠀⢀⡜⣷⠀⠉⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⢀⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⣿⣄⠉⠛⠛⢩⡿⠀⠀⠀⣾⠛⠛⠛⠛⠁⣼⡿⢻⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣤⠤⢤⠸⠃⡼⠃⠀⠀⠀⠀
⠀⠀⢰⣋⣙⠢⢂⣠⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⡇⠀⠻⣆⠛⠛⠋⠁⠀⠀⠀⠙⠻⠶⠾⠃⣼⠏⠀⠀⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⠦⣄⠄⠠⡄⠙⢒⡄⠀⠀
⠀⠀⠀⠈⠉⠁⣷⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠀⠀⠀⠸⡇⢶⡄⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠋⢰⡆⢸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠯⠄⡔⠈⠉⠉⠀⠀⠀
⠀⠀⠀⠀⠀⢰⡇⠀⠀⢾⣋⢉⠂⢘⣈⡷⠀⠀⢀⣀⣠⣿⡀⢻⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣼⠃⣻⣛⣛⣿⡦⠀⠀⢀⣀⣀⠀⡠⠤⢄⠀⢸⡇⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠈⠁⠀⠀⠀⠈⠉⢳⡉⠁⠀⠀⠀⠿⠽⣶⡶⣰⡄⣿⣤⣄⣀⡀⠀⠀⠀⠀⠀⣀⣀⣀⣤⣿⣸⣾⣽⣷⡄⠀⠀⠀⠳⠥⠤⣅⠥⠴⠛⠀⣾⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠛⠛⠙⠷⠟⠀⠈⠉⠉⠛⠛⠛⠋⠉⠉⠉⠉⠀⠈⠙⠿⣿⠀⠀⠀⠀⠀⠀⠀⠀⢹⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀