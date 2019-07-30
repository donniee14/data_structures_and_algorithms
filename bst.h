/* bst.h 
 * Interface and tree definition for basic binary tree
 */

enum balanceoptions {BST, AVL};

#define TRUE 1
#define FALSE 0

typedef void *data_t;
typedef int bst_key_t;

typedef struct bst_node_tag {
    data_t data_ptr;
    bst_key_t key;
    int height;			    //added node height to struct
    struct bst_node_tag *left;
    struct bst_node_tag *right;
} bst_node_t;

typedef struct bst_tag {
    bst_node_t *root;
    int size;			    // number of keys in tree
    int num_recent_rotations;       // number of rotations in last operation
    int policy;			    // must be BST or AVL
    int num_recent_key_comparisons; // number of comparisons in last operation
} bst_t;


/* prototype definitions for functions in bst.c */
data_t bst_access(bst_t *bst_ptr, bst_key_t key);
bst_t *bst_construct(int);
void bst_destruct(bst_t *bst_ptr);
int bst_insert(bst_t *bst_ptr, bst_key_t key, data_t elem_ptr);
int bst_avl_insert(bst_t *bst_ptr, bst_key_t key, data_t elem_ptr);
data_t bst_remove(bst_t *bst_ptr, bst_key_t key);

int bst_size(bst_t *bst_ptr);
int bst_key_comps(bst_t *bst_ptr);
int bst_int_path_len(bst_t *);
int bst_rotations(bst_t *bst_ptr);
void bst_debug_print_tree(bst_t *bst_ptr);
void bst_debug_validate(bst_t *T);


/* vi:set ts=8 sts=4 sw=4: */
