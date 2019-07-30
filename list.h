/* list.h 
 *
 * Public functions for two-way linked list
 *
 * You should not need to change any of the code this file.  If you do, you
 * must get permission from the instructor.
 */

/* public constants used as parameters by most functions */

#define LISTPOS_HEAD -10922
#define LISTPOS_TAIL -43690

typedef struct list_node_tag {
    // Private members for list.c only
    data_t *data_ptr;
    struct list_node_tag *prev;
    struct list_node_tag *next;
} list_node_t;

typedef struct list_tag {
    // Private members for list.c only
    list_node_t *head;
    list_node_t *tail;
    int current_list_size;
    int list_sorted_state;
    // Private method for list.c only
    int (*comp_proc)(const data_t *, const data_t *);
    void (*data_clean)(data_t *);
} list_t;

/* public prototypes defintions for MP3 */
void list_sort(list_t **list_ptr, int sort_type, int sort_order);

/* public prototype definitions for list.c */
data_t * list_access(list_t *list_ptr, int pos_index);
list_t * list_construct(int (*fcomp)(const data_t *, const data_t *),
                        void (*dataclean)(data_t *));
data_t * list_elem_find(list_t *list_ptr, data_t *elem_ptr, int *pos_index);
void     list_destruct(list_t *list_ptr);
void     list_insert(list_t *list_ptr, data_t *elem_ptr, int pos_index);
void     list_insert_sorted(list_t *list_ptr, data_t *elem_ptr);
data_t * list_remove(list_t *list_ptr, int pos_index);
void     list_reverse(list_t *list_ptr);
int      list_size(list_t *list_ptr);
int      list_order(list_t *list_ptr);

/* commands for vim. ts: tabstop, sts: soft tabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */
