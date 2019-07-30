/* Donald Elmore
 * Bugs: None known
 */
 
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <limits.h>

#include "bst.h"

#define MYMAX(a, b) (a > b ? a : b)
//counters for statistics
int CompCalls = 0;
int NumRotations = 0;

//global for checking data_ptr was replaced
int replaced;

int Debug_flag = FALSE;

//definitions for use in bst.c only
void ugly_print(bst_node_t *N, int level, int policy);
int bst_debug_validate_rec(bst_node_t *N, int min, int max, int *count);
int rec_height(bst_node_t *N);
int children(bst_node_t *N);
void pretty_print(bst_t *T);

bst_node_t *newNode(int key, data_t elem_ptr);

/* searchViaNode helper function that finds the element with the matching key given
 * the root and returns the node in which it exists.
 *
 * root - the root of the tree of interest
 * key - the key to be found
 *
 * RETURNS pointer to the node in which the match occurs
 */
bst_node_t* searchViaNode(bst_t *T, bst_node_t *root, bst_key_t key) {   
    CompCalls++;
    //Base case: root is null or key is at root
    if (root == NULL || root->key == key) {
        return root;
    }
    //Key is larger than root's key
    CompCalls++;
    if (root->key < key) {
        return searchViaNode(T, root->right, key);
    }
    //Key is smaller than root's key
    return searchViaNode(T, root->left, key);
}

/* Finds the tree element with the matching key and returns the data that is
 * stored in this node in the tree.
 *
 * T - tree to search in key - key to find inside T
 *
 * RETURNS pointer to the data stored in the found node or NULL if no match is
 * found
 */
data_t bst_access(bst_t *T, bst_key_t key)
{
    CompCalls = 0;
    if (T == NULL || T->root == NULL) {
        printf("Error! Tree or root is NULL!\n");
        return NULL;
    }
    
    bst_node_t* tempNode = searchViaNode(T, T->root, key);
    if (tempNode == NULL)
        return NULL;
    
    T->num_recent_key_comparisons = CompCalls;
    return tempNode->data_ptr;
}

/* Creates the header block for the tree with the provided management policy,
 * and initializes it with default 'blank' data.
 *
 * tree_policy - tree management policy to use either AVL or BST.
 *
 * RETURNS pointer to the newly created tree
 */
bst_t *bst_construct(int tree_policy)
{
    bst_t *newTree = (bst_t *)malloc(sizeof(bst_t));   
    newTree->root = NULL;
    newTree->size = 0;
    newTree->num_recent_rotations = 0;
    newTree->policy = tree_policy;
    newTree->num_recent_key_comparisons = 0;
    return newTree;
}

/* bst_destruct helper function that recurses through tree freeing as it goes
 *
 * node -  root node of tree to be destructed
 */
void deleteTree(bst_node_t *node) {
    if (node == NULL) return;
 
    //delete both subtrees
    deleteTree(node->left);
    deleteTree(node->right);
    free(node->data_ptr);
   
    //then delete node
    //printf("\n Deleting node: %d", node->key);
    free(node);
}

/* Free all items stored in the tree including the memory block with the data
 * and the bst_node_t structure.  Also frees the header block.  
 *
 * T - tree to destroy
 */
void bst_destruct(bst_t *T)
{
    deleteTree(T->root);
    free(T);
}

/* Creates/mallocs a new node that can be inserted to a binary tree,
 * initilzes the data, key and height.
 *
 * key - key to be assigned to new node
 * elem_ptr - data to be assigned to new node
 *
 * RETURNS a pointer to the new node
 */
bst_node_t *newNode(int key, data_t elem_ptr) {
    bst_node_t *temp = (bst_node_t *)malloc(sizeof(bst_node_t));
    temp->data_ptr = elem_ptr;
    temp->key = key;
    temp->height = 1;
    temp->left = temp->right = NULL;
    return temp;
}

/* Insert data_t into the tree with the associated key. Insertion MUST
 * follow the tree's property (i.e., AVL or BST)
 *
 * T - tree to insert into
 * key - search key to determine if key is in the tree
 * elem_ptr - data to be stored at tree node indicated by key
 *
 * RETURNS 0 if key is found and element is replaced, and 1 if key is not found
 * and element is inserted
 */
int bst_insert(bst_t *T, bst_key_t key, data_t elem_ptr)
{
    CompCalls = 0;
    NumRotations = 0;
    
    if (T->policy == AVL) {
        return bst_avl_insert(T, key, elem_ptr);
    }
    
    T->size++;
    bst_node_t *current = T->root;
    bst_node_t *parent = NULL;
    
    if (T->root == NULL) {
        T->root = newNode(key, elem_ptr);
        return 1;
    }
    
    //traverse tree and find parent of node key
    while (current != NULL) {
        parent = current;
        CompCalls++;
        if (key == current->key) {
            free(current->data_ptr);
            current->data_ptr = elem_ptr;
            T->size--;      //because dupe
            return 0;
        }
        CompCalls++;
        if (key < current->key)
            current = current->left;
        else
            current = current->right;
    }
    
    //create new node and assign to parent pointer
    if (key < parent->key)
        parent->left = newNode(key, elem_ptr);
    else
        parent->right = newNode(key, elem_ptr);
        
    return 1;
}

/* find max function, returns max of two ints
 *
 * a, b - two ints of interest
 *
 * RETURNS the max value of a and b
 */
int max(int a, int b) {
    return (a > b)? a : b;
}

/* function that returns height of given node
 *
 * node - node of interested height
 *
 * RETURNS height of node of interest
 */
int height(bst_node_t *node) {
    if (node == NULL)
        return 0;
    return node->height;
}

/* A utility function to right rotate subtree rooted with y
 *
 * y - node to be rotated to the right
 *
 * RETURNS corrected node
 */
bst_node_t *rightRotate(bst_node_t *y) {
    bst_node_t *x = y->left;
    bst_node_t *T2 = x->right;
 
    // Perform rotation
    x->right = y;
    y->left = T2;
 
    // Update heights
    y->height = max(height(y->left), height(y->right))+1;
    x->height = max(height(x->left), height(x->right))+1;
 
    // Return new root
    NumRotations++;
    return x;
}
 
/* A utility function to right rotate subtree rooted with x
 *
 * y - node to be rotated to the left
 *
 * RETURNS corrected node
 */
bst_node_t *leftRotate(bst_node_t *x) {
    bst_node_t *y = x->right;
    bst_node_t *T2 = y->left;
 
    // Perform rotation
    y->left = x;
    x->right = T2;
 
    //  Update heights
    x->height = max(height(x->left), height(x->right))+1;
    y->height = max(height(y->left), height(y->right))+1;
 
    // Return new root
    NumRotations++;
    return y;
}

//Get Balance factor of the node and return it
int getBalance(bst_node_t *node) {
    if (node == NULL)
        return 0;
    return (height(node->left) - height(node->right));
}

/* Recursive AVL Insertion Helper Function
 *
 * T - tree of interest
 * node - node to be inserted
 * key - key of interest
 * data - data to be mapped to node
 * 
 * RETURNS unchanged node ptr
 */
bst_node_t *avlInsert(bst_t *T, bst_node_t *node, bst_key_t key, data_t elem_ptr) {
    if (node == NULL)
        return newNode(key, elem_ptr);
    CompCalls++;
    if (key < node->key)
        node->left = avlInsert(T, node->left, key, elem_ptr);
    else if (key > node->key)
        node->right = avlInsert(T, node->right, key, elem_ptr);
    else if (key == node->key) {
        replaced = 0;
        free(node->data_ptr);
        node->data_ptr = elem_ptr;
        T->size--;
        return node;
     }
     CompCalls++;
     //update height
     node->height = 1 + max(height(node->left), height(node->right));
     
     int balance = getBalance(node);
     
     //if this node becomes unbalanced there are 4 cases:
     //left left
     if (balance > 1 && key < node->left->key)
        return rightRotate(node);
     //right right
     if (balance < -1 && key > node->right->key)
        return leftRotate(node);
     //left right
     if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
     }
     //right left
     if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
     }
     
     //return unchanged node ptr
     T->num_recent_rotations = NumRotations;
     T->num_recent_key_comparisons = CompCalls;
     return node;    
}

/* Insert data_t into the tree with the associated key. Insertion MUST
 * follow the tree's property AVL. This function should be called from
 * bst_insert for AVL tree's inserts.
 *
 * T - tree to insert into
 * key - search key to determine if key is in the tree
 * elem_ptr - data to be stored at tree node indicated by key
 *
 * RETURNS 0 if key is found and element is replaced, and 1 if key is not found
 * and element is inserted
 */
int bst_avl_insert(bst_t *T, bst_key_t key, data_t elem_ptr)
{
    NumRotations = 0;
    CompCalls = 0;
    T->size++;
    T->root = avlInsert(T, T->root, key, elem_ptr);
    if (replaced == 0)
        return 0;
        
    return 1;
}

/* DO NOT NEED TO IMPLEMENT FOR REGULAR ASSIGNMENT. ONLY FOR EXTRA ASSIGNMENT.
 *
 * Removes the item in the tree with the matching key.
 * 
 * T - pointer to tree
 * key - search key for particular node in the tree 'T'
 *
 * RETURNS the pointer to the data memory block and free the bst_node_t memory
 * block.  If the key is not found in the tree, return NULL.  If the tree's 
 * policy is AVL, then ensure all nodes have the AVL property.
 *
 */
data_t bst_remove(bst_t *T, bst_key_t key)
{
    data_t dp = NULL;
    CompCalls = 0;
    NumRotations = 0;
    if (T->policy == AVL)
	    dp = NULL; /*TODO: AVL remove */
    else
	    dp = NULL; /*TODO: BST remove */

    /*TODO: update tree stats*/

    if (Debug_flag)
        bst_debug_validate(T);
    return dp;
}

/* RETURNS the number of keys in the tree */
int bst_size(bst_t *T)
{
    return T->size;
}

/* RETURNS the number of key comparisons  */
int bst_key_comps(bst_t *T)
{
    return T->num_recent_key_comparisons;
}

int GetInternalPathLength (bst_node_t *root, int curr) {
    if (root != NULL) {
        if (root->left == NULL && root->right == NULL)
            return curr;

        return curr + GetInternalPathLength (root->left, curr + 1) +
                GetInternalPathLength (root->right, curr + 1);
    }
    return 0;
}

/* RETURNS the computed internal path length of the tree T */
int bst_int_path_len(bst_t *T)
{
    int curr = 0;
    return GetInternalPathLength(T->root, curr);
}

/* RETURNS the number of rotations from the last remove*/
int bst_rotations(bst_t *T)
{
    return T->num_recent_rotations;
}

/* prints the tree T */
void bst_debug_print_tree(bst_t *T)
{
    ugly_print(T->root, 0, T->policy);
    printf("\n");
    if (T->size < 64)
	pretty_print(T);
}

/* basic print function for a binary tree */
void ugly_print(bst_node_t *N, int level, int policy)
{
    int i;
    if (N == NULL) return;
    ugly_print(N->right, level+1, policy);
    if (policy == AVL) {
	    for (i = 0; i<level; i++) printf("       ");
	        printf("%5d\n", N->key); // TODO: extend for AVL
    } else {
	    for (i = 0; i<level; i++) printf("     ");
	        printf("%5d\n", N->key);
    }
    ugly_print(N->left, level+1, policy);
}


/* Basic validation function for tree T */
void bst_debug_validate(bst_t *T)
{
    int size = 0;
    assert(bst_debug_validate_rec(T->root, INT_MIN, INT_MAX, &size) == TRUE);
    assert(size == T->size);
    if (T->policy == AVL)
	    rec_height(T->root);
}

/* A tree validation function
 */
int bst_debug_validate_rec(bst_node_t *N, int min, int max, int *count)
{
    if (N == NULL)
        return TRUE;
    if (N->key <= min || N->key >= max)
        return FALSE;
    assert(N->data_ptr != NULL);
    *count += 1;
    return bst_debug_validate_rec(N->left, min, N->key, count) &&
        bst_debug_validate_rec(N->right, N->key, max, count);
}

/* Verifies AVL tree properties */

int rec_height(bst_node_t *N)
{
    if (N == NULL)
	    return 0;
    int lh = rec_height(N->left);
    int rh = rec_height(N->right);
    int lean = lh - rh;
    assert(lean == 0 || lean == 1 || lean == -1);
    return 1 + MYMAX(lh, rh); 

}

/* Recursive function to count children */
int children(bst_node_t *N)
{
    if (N == NULL) return 0;
    return 1 + children(N->left) + children(N->right);
}

/* Prints the tree to the terminal in ASCII art*/
void pretty_print(bst_t *T)
{
    typedef struct queue_tag {
	    bst_node_t *N;
	    int level;
	    int list_sum;
    } queue_t;

    queue_t Q[T->size];
    int q_head = 0;
    int q_tail = 0;
    int i, j;
    int current_level = 0;
    int col_cnt = 0;
    bst_node_t *N;

    Q[q_tail].N = T->root;
    Q[q_tail].level = 0;
    Q[q_tail].list_sum = 0;
    q_tail++;
    for (i = 0; i < T->size; i++)
    {
	assert(q_head < T->size);
	N = Q[q_head].N;
	if (Q[q_head].level > current_level) {
	    printf("\n");
	    current_level++;
	    col_cnt = 0;
	}
	int left_ch = children(N->left);
	int my_pos = 1 + left_ch + Q[q_head].list_sum;
	int left_child_pos = my_pos;
	if (N->left != NULL)
	    left_child_pos = 1 + Q[q_head].list_sum + children(N->left->left);
	int right_child_pos = my_pos;
	if (N->right != NULL)
	    right_child_pos = my_pos + 1 + children(N->right->left);
	for (j = col_cnt + 1; j <= right_child_pos; j++)
	{
	    if (j == my_pos)
		if (left_child_pos < my_pos)
		    if (N->key < 10)
			printf("--%d", N->key);
		    else if (N->key < 100)
			printf("-%d", N->key);
		    else
			printf("%d", N->key);
		else
		    printf("%3d", N->key);
	    else if (j == left_child_pos)
		//printf("  |");
		printf("  /");
	    else if (j > left_child_pos && j < my_pos)
		printf("---");
	    else if (j > my_pos && j < right_child_pos)
		printf("---");
	    else if (j == right_child_pos)
		//printf("--|");
		printf("-\\ ");
	    else
		printf("   ");
	}
	col_cnt = right_child_pos;

	if (N->left != NULL) {
	    Q[q_tail].N = N->left;
	    Q[q_tail].level = Q[q_head].level + 1;
	    Q[q_tail].list_sum = Q[q_head].list_sum;
	    q_tail++;
	}
	if (N->right != NULL) {
	    Q[q_tail].N = N->right;
	    Q[q_tail].level = Q[q_head].level + 1;
	    Q[q_tail].list_sum = Q[q_head].list_sum + left_ch + 1;
	    q_tail++;
	}
	q_head++;
    }
    printf("\n");
}

/* vi:set ts=8 sts=4 sw=4 et: */
