#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#define INF 2000000
#define MAX_VALID_POINTS 10000

/* kd-tree node, has left and right,
as well as an array of dimensions, point*/
typedef struct kd_node {
	struct kd_node *left, *right;
	int point[3];
} kd_node;

/* Creates a node, allocates space for it and initializes
and reads its dimensions from the input file */
kd_node *create_node(FILE *input, int k)
{
	kd_node *new_node = calloc(1, sizeof(kd_node));
	new_node->left = NULL;
	new_node->right = NULL;
	for (int i = 0; i < k; i++)
		fscanf(input, "%d", &new_node->point[i]);
	return new_node;
}

/* Inserts a node into the tree on the appropriate level
based on the splitting coordinate that we are checking
at any particular time. The current level is depth % k,
where k is the number of dimensions. */
void insert_node(kd_node *root, kd_node *new_node, int k)
{
	if (!root) {
		root = new_node;
		return;
	}
	int depth = 0;
	kd_node *temp_node = root, *prev = NULL;
	int curr_level;
	while (temp_node) {
		curr_level = depth % k;
		prev = temp_node;
		if (new_node->point[curr_level] >= temp_node->point[curr_level])
			temp_node = temp_node->right;
		else
			temp_node = temp_node->left;
		depth++;
	}
	if (new_node->point[curr_level] >= prev->point[curr_level])
		prev->right = new_node;
	else
		prev->left = new_node;
}

/* Calculates the distance based on the provided formula */
double distance(int coordinates[], int *node2, int k)
{
	double dist = 0;
	for (int i = 0; i < k; i++)
		dist += (node2[i] - coordinates[i]) * (node2[i] - coordinates[i]);
	dist = sqrt(dist);
	return dist;
}

/* Implements the nearest neighbour algorithm.
Everytime it find a point that has a better distance, it updates
the best distance, resets the number of points and adds the coordinate
to the array. If there are more 'best points', it adds all of them.
First of all the algorithms goes to the position the target would
be at. When it reaches a leaf, it starts going back in recursion.
If the difference between the target splitting coordinate and
the node's we are on splitting coordinate is smaller than our
best distance, we go into the other splitting plane since there
might be a better point there. */
void nn(kd_node *node, int *target, int k, int current_level,
		int **coordinates, int *number_of_points, double *best_dist)
{
	if (!node)
		return;

	if (target[current_level] < node->point[current_level])
		nn(node->left, target, k, (current_level + 1) % k,
		   coordinates, number_of_points, best_dist);
	else
		nn(node->right, target, k, (current_level + 1) % k,
		   coordinates, number_of_points, best_dist);

	double current_dist = distance(node->point, target, k);

	if (current_dist < (*best_dist)) {
		(*best_dist) = current_dist;
		(*number_of_points) = 0;
		for (int i = 0; i < k; i++)
			coordinates[(*number_of_points)][i] = node->point[i];
		(*number_of_points)++;
	} else if (current_dist == (*best_dist)) {
		for (int i = 0; i < k; i++)
			coordinates[(*number_of_points)][i] = node->point[i];
		(*number_of_points)++;
	}

	if (abs(target[current_level] - node->point[current_level])
	    < (*best_dist)) {
		if (target[current_level] < node->point[current_level])
			nn(node->right, target, k, (current_level + 1) % k,
			   coordinates, number_of_points, best_dist);
		else
			nn(node->left, target, k, (current_level + 1) % k,
			   coordinates, number_of_points, best_dist);
	}
}

/* Performs recursive range search, checking if each coordinate
of the point is in the appropriate bounds. If it isn't, it sets in_range to 0.
Makes sure to not go left or right anymore if it already finds an
invalid point. */
void range_search(kd_node *node, int k, int *start, int *end,
				  int **valid_points, int *number_of_points, int split_coord)
{
	if (!node)
		return;
	int in_range = 1;

	if (in_range)
		for (int i = 0; i < k; i++)
			if (node->point[i] < start[i] || node->point[i] > end[i]) {
				in_range = 0;
				break;
			}

	if (in_range) {
		for (int i = 0; i < k; i++)
			valid_points[*number_of_points][i] = node->point[i];
		(*number_of_points)++;
	}

	if (node->point[split_coord] < start[split_coord]) {
		// do not go left
		range_search(node->right, k, start, end,
					 valid_points, number_of_points, (split_coord + 1) % k);
	} else if (node->point[split_coord] > end[split_coord]) {
		// do not go right
		range_search(node->left, k, start, end,
					 valid_points, number_of_points, (split_coord + 1) % k);
	} else {
		range_search(node->left, k, start, end,
					 valid_points, number_of_points, (split_coord + 1) % k);
		range_search(node->right, k, start, end,
					 valid_points, number_of_points, (split_coord + 1) % k);
	}
}

/* Performs a swap using bitwise operator xor. */
void swap(int *a, int *b)
{
	*a ^= *b;
	*b ^= *a;
	*a ^= *b;
}

/* Sorts the valid points obtained, using memcmp to compare
blocks of memory. If valid_points[j] > valid_points[j + 1], it
swaps the points in points in the valid points matrix
(which contains one point per line)*/
void sort_range(int k, int number_of_points, int **valid_points)
{
	for (int i = 0; i < number_of_points - 1; i++)
		for (int j = 0; j < number_of_points - i - 1; j++) {
			int cmp_result = memcmp(valid_points[j],
									valid_points[j + 1], k * sizeof(int));
			if (cmp_result > 0)
				for (int c = 0; c < k; c++)
					swap(&valid_points[j][c], &valid_points[j + 1][c]);
		}
}

/* Frees each node recursively */
void free_tree(kd_node *root)
{
	if (!root)
		return;
	free_tree(root->left);
	free_tree(root->right);
	free(root);
}

int main(void)
{
	char command[20];
	int n, k, start[4], end[4], **valid_points;
	kd_node *new_node, *root;
	scanf("%s", command);

	/* Since the load is only done once, it does it outside of
	the while loop. First reads the number of points and dimensions,
	it creates the root separately and then the other nodes. */
	if (strncmp(command, "LOAD", 4) == 0) {
		char path[100];
		scanf("%s", path);
		FILE *input = fopen(path, "r");
		fscanf(input, "%d%d", &n, &k);
		root = create_node(input, k);
		for (int i = 1; i < n; i++) {
			new_node = create_node(input, k);
			insert_node(root, new_node, k);
		}
		fclose(input);
	}
	/* Matrix that will contain the valid points*/
	valid_points = (int **)calloc(MAX_VALID_POINTS, sizeof(int *));
	for (int i = 0; i < MAX_VALID_POINTS; i++)
		valid_points[i] = (int *)calloc(k, sizeof(int));
	while (1) {
		scanf("%s", command);
		if (strncmp(command, "NN", 3) == 0) {
			int number_of_points = 0;
			kd_node *target = calloc(1, sizeof(kd_node));
			for (int i = 0; i < k; i++)
				scanf("%d", &target->point[i]);
			double best_dist = INF;
			nn(root, target->point, k, 0,
			   valid_points, &number_of_points, &best_dist);
			for (int i = 0; i < number_of_points; i++) {
				for (int j = 0; j < k; j++)
					printf("%d ", valid_points[i][j]);
				printf("\n");
			}
			free(target);
		} else if (strncmp(command, "RS", 2) == 0) {
			for (int i = 0; i < k; i++)
				scanf("%d%d", &start[i], &end[i]);
			int number_of_points = 0;
			range_search(root, k, start, end,
						 valid_points, &number_of_points, 0);
			sort_range(k, number_of_points, valid_points);
			for (int i = 0; i < number_of_points; i++) {
				for (int j = 0; j < k; j++)
					printf("%d ", valid_points[i][j]);
				printf("\n");
			}
		} else if (strncmp(command, "EXIT", 4) == 0) {
			for (int i = 0; i < MAX_VALID_POINTS; i++)
				free(valid_points[i]);
			free(valid_points);
			free_tree(root);
			exit(0);
		}
	}
	return 0;
}
