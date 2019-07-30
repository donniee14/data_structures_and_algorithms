/* Donald Elmore
 * Bugs: None known
 */

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "datatypes.h"   /* defines data_t */
#include "list.h"        /* defines public functions for list ADT */

/* definitions for private constants used in list.c only */
#define LIST_SORTED_ASCENDING   -1234567
#define LIST_SORTED_DESCENDING  -7654321
#define LIST_UNSORTED           -4444444
int VALIDATE_STATUS = 0;        //turn validate on or off

/* prototypes for private functions used in list.c only */
void list_debug_validate(list_t *L);
void list_insertion_sort(list_t** L, int sort_order);
void list_recursive_selection_sort(list_t** L, int sort_order);
void list_selection_sort(list_t** L, int sort_order);
void list_merge_sort(list_t** L, int sort_order);

/* Find the max node of a given list.
 * 
 * list_ptr: pointer to list of interest
 * head: pointer to head of list of interest
 * tail: pointer to tail of list of interest
 *
 * returns: a pointer to the max node in the list
 */
list_node_t* findMax(list_t* list_ptr, list_node_t *head, list_node_t *tail)
{
    list_node_t *i = head, *max = head;

    do {
        i = i->next;
        if (list_ptr->comp_proc(i->data_ptr, max->data_ptr) < 0)
            max = i;
    } while (i != tail);

    return max;
}

/* Find the min node of a given list.
 * 
 * list_ptr: pointer to list of interest
 * head: pointer to head of list of interest
 * tail: pointer to tail of list of interest
 *
 * returns: a pointer to the min node in the list
 */
list_node_t* findMin(list_t* list_ptr, list_node_t *head, list_node_t *tail)
{   
    list_node_t *i = head, *min = head;

    do {
        i = i->next;
        if (list_ptr->comp_proc(i->data_ptr, min->data_ptr) > 0)
            min = i;
    } while (i != tail);

    return min;
}

/* (sorting helper functions) */

/* Sorts a list used a specific sorting algorithm and direction.
 * 
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from list_construct.
 *
 * sort_type: sorting algorithm used during sorting: 1) Insertion Sort;
              2) Recursive Selection Sort; 3) Iterative Selection Sort;
              and 4) Merge Sort
 *
 * sort_order: if 1 then the items in the list are sorted in ascending order.
 *             if 2 then the items in the list are sorted in decending order.
 *
 * return value: None.
 */
void list_sort(list_t **list_ptr, int sort_type, int sort_order)
{
    int typeSort = sort_order;   
    switch (sort_type) {
        case 1 :
            list_insertion_sort(list_ptr, typeSort);
            break;
        case 2 :
            list_recursive_selection_sort(list_ptr, typeSort);
            break;
        case 3 :
            list_selection_sort(list_ptr, typeSort);
            break;
        case 4 :
            list_merge_sort(list_ptr, typeSort);
            break;
        default :
            break;
    }
    
    if (typeSort == 1) {
        (*list_ptr)->list_sorted_state = LIST_SORTED_ASCENDING;
    } else if (typeSort == 2) {
        (*list_ptr)->list_sorted_state = LIST_SORTED_DESCENDING;
    }
    list_debug_validate(*list_ptr);
}

/* Sorting the list via the insertion sort algorithm
 * 
 * L: pointer to list-of-interest.  
 *
 * sort_order: 1 sort list in ascending order 2 sort in descending order 
 */
void list_insertion_sort(list_t** L, int sort_order)
{
    int iterator;
    if (L != NULL) {
        //create new sorted list
        list_t* newList = list_construct((*L)->comp_proc, (*L)->data_clean);
        (newList)->list_sorted_state = sort_order == 1 ? LIST_SORTED_ASCENDING
            : LIST_SORTED_DESCENDING;
            
        //iterate through list
        for (iterator = 0; (*L)->current_list_size > 0; iterator++) {
            list_insert_sorted(newList, list_remove(*L, LISTPOS_HEAD));
        }
        
        //destruct original list
        list_destruct(*L);
        
        //make L point to new list created
        *L = newList;
    }
}

/* Recursive Selection Sort Helper Function
 * 
 * L: pointer to list-of-interest.
 * head: points to the head of the list of interest
 * tail: points to the tail of the list of interest
 *
 * sort_order: 1 sort list in ascending order 2 sort in descending order 
 */
void rssHelper(list_t **L, list_node_t *head, list_node_t *tail, int sort_order)
{
    data_t *dataPtr;
    list_node_t *min, *max;
    if (head->next != NULL) {
        if (sort_order == 2) {
            max = findMax(*L, head, tail);
            dataPtr = head->data_ptr;
            head->data_ptr = max->data_ptr;
            max->data_ptr = dataPtr;
        }
        else if (sort_order == 1) {
            min = findMin(*L, head, tail);
            dataPtr = head->data_ptr;
            head->data_ptr = min->data_ptr;
            min->data_ptr = dataPtr;
        }
        rssHelper(L, head->next, tail, sort_order);
    }   
}

void list_recursive_selection_sort(list_t** L, int sort_order)
{
    rssHelper(L, (*L)->head, (*L)->tail, sort_order);
}

/* Sorting the list via the selection sort algorithm
 * 
 * L: pointer to list-of-interest.  
 *
 * sort_order: 1 sort list in ascending order 2 sort in descending order 
 */
void list_selection_sort(list_t** L, int sort_order)
{
    data_t *dataPtr;
    list_node_t *head = (*L)->head, *tail = (*L)->tail;  
    list_node_t *min, *max;
    while (head->next != NULL) {
        if (sort_order == 2) {
            max = findMax(*L, head, tail);
            dataPtr = head->data_ptr;
            head->data_ptr = max->data_ptr;
            max->data_ptr = dataPtr;
        }
        else if (sort_order == 1) {
            min = findMin(*L, head, tail);
            dataPtr = head->data_ptr;
            head->data_ptr = min->data_ptr;
            min->data_ptr = dataPtr;
        }
        head = head->next;
    }   
}

/* Splits a given list into two lists of half size
 * 
 * list_ptr1: pointer to list 1 (the original list)
 * list_ptr2: pointer to the 2nd list that will be the second half
 *
 * sort_order: 1 sort list in ascending order 2 sort in descending order 
 */
void split(list_t *list_ptr1, list_t *list_ptr2)
{
    int newSize = list_ptr1->current_list_size / 2;
    list_ptr1->current_list_size = list_ptr1->current_list_size - newSize;
    list_ptr2->current_list_size = newSize;
 
    list_node_t *dummy = list_ptr1->head;
    int i = 0;

    while (i < list_ptr1->current_list_size - 1) {
        dummy = dummy->next;
        i++;
    }

    list_ptr2->head = dummy->next;
    list_ptr2->tail = list_ptr1->tail;
    list_ptr1->tail = dummy;
    list_ptr1->tail->next = NULL;
    list_ptr2->head->prev = NULL;
}

/* Merge two given lists into one list_t
 * 
 * list_ptr1: pointer to list 1
 * list_ptr2: pointer to list 2
 * sort_order: 1 sort list in ascending order 2 sort in descending order
 *
 * returns: new list merged from list_ptr1 and list_ptr2
 */
list_t *listMerge(list_t *list_ptr1, list_t *list_ptr2, int sort_order)
{
    int testerProc;
    list_t *list3 = list_construct(list_ptr1->comp_proc, list_ptr1->data_clean);

    while ((list_ptr1->current_list_size > 0) && (list_ptr2->current_list_size > 0)) {
        data_t *list1Head = list_access(list_ptr1, LISTPOS_HEAD);
        data_t *list2Head = list_access(list_ptr2, LISTPOS_HEAD);

        testerProc = (sort_order == 1&&(list3->comp_proc)(list1Head, list2Head) > 0) ||
            (sort_order == 2&&(list3->comp_proc)(list1Head, list2Head) < 0);
            
        //list1 comes first, insert to it
        if (testerProc) {
            list_insert(list3, list_remove(list_ptr1, LISTPOS_HEAD), LISTPOS_TAIL);
        } 
        //list2 comes first, insert to it
        else {
            list_insert(list3, list_remove(list_ptr2, LISTPOS_HEAD), LISTPOS_TAIL);
        }
    }
    //remove nodes from the list that is not empty
    while (list_ptr1->current_list_size > 0) {
        list_insert(list3, list_remove(list_ptr1, LISTPOS_HEAD), LISTPOS_TAIL);
    }
    while (list_ptr2->current_list_size > 0) {
        list_insert(list3, list_remove(list_ptr2, LISTPOS_HEAD), LISTPOS_TAIL);
    }

    list_destruct(list_ptr1);
    list_destruct(list_ptr2);
    return list3;
}

/* Sorting the list via the merge sort algorithm
 * 
 * L: pointer to list-of-interest.  
 *
 * sort_order: 1 sort list in ascending order 2 sort in descending order 
 */
void list_merge_sort(list_t** L, int sort_order)
{
    if ((*L)->current_list_size > 1) {
        //malloc the pointer to the pointer
        list_t *list;
        list = list_construct((*L)->comp_proc, (*L)->data_clean);

        //call split function that splits list into two
        split(*L, list);

        //merge sort each of the lists
        list_merge_sort(L, sort_order);
        list_merge_sort(&list, sort_order);

        *L = listMerge(*L, list, sort_order);
    }
}

/* ----- below are the functions  ----- */

/* Obtains a pointer to an element stored in the specified list, at the
 * specified list position
 * 
 * list_ptr: pointer to list-of-interest.  A pointer to an empty list is
 *           obtained from list_construct.
 *
 * pos_index: position of the element to be accessed.  Index starts at 0 at
 *            head of the list, and incremented by one until the tail is
 *            reached.  Can also specify LISTPOS_HEAD and LISTPOS_TAIL
 *
 * return value: pointer to element accessed within the specified list.  A
 * value NULL is returned if the pos_index does not correspond to an element in
 * the list.  For example, if the list is empty, NULL is returned.
 */
data_t * list_access(list_t *list_ptr, int pos_index)
{
    int count;
    list_node_t *L;
 
    assert(list_ptr != NULL);

    /* debugging function to verify that the structure of the list is valid */
    list_debug_validate(list_ptr);

    /* handle three special cases.
     *   1.  The list is empty
     *   2.  Asking for the head 
     *   3.  Asking for the tail
     */
    if (list_ptr->current_list_size == 0) {
        return NULL;  /* list is empty */
    }
    else if (pos_index == LISTPOS_HEAD || pos_index == 0) {
        return list_ptr->head->data_ptr;
    }
    else if (pos_index == LISTPOS_TAIL || 
             pos_index == list_ptr->current_list_size - 1) {
        return list_ptr->tail->data_ptr;
    }
    else if (pos_index < 0 || pos_index >= list_ptr->current_list_size)
        return NULL;   /* does not correspond to position in list */

    /* we now know pos_index is for an interal element */
    /* loop through the list until find correct position index */
    L = list_ptr->head;
    for (count = 0; count < pos_index; count++) {
        L = L->next;
    }
    assert(L != NULL);
    return L->data_ptr;
}

/* Allocates a new, empty list 
 *
 * By convention, the list is initially assumed to be sorted.  The field sorted
 * can only take values LIST_SORTED_ASCENDING LIST_SORTED_DESCENDING or 
 * LIST_UNSORTED
 *
 * Use list_free to remove and deallocate all elements on a list (retaining the
 * list itself).
 *
 * comp_proc = pointer to comparison function
 *
 * Use the standard function free() to deallocate a list which is no longer
 * useful (after freeing its elements).
 */
list_t * list_construct(int (*fcomp)(const data_t *, const data_t *),
        void (*dataclean)(data_t *))
{
    list_t *L;
    L = (list_t *) malloc(sizeof(list_t));
    L->head = NULL;
    L->tail = NULL;
    L->current_list_size = 0;
    L->list_sorted_state = LIST_SORTED_ASCENDING;
    L->comp_proc = fcomp;
    L->data_clean = dataclean;

    /* the last line of this function must call validate */
    list_debug_validate(L);
    return L;
}

/* Finds an element in a list and returns a pointer to it.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: element against which other elements in the list are compared.
 *           Note: use the comp_proc function pointer found in the list_t 
 *           header block. 
 *
 * The function returns a pointer to the matching element with lowest index if
 * a match if found.  If a match is not found the return value is NULL.
 *
 * The function also returns the integer position of matching element with the
 *           lowest index.  If a matching element is not found, the position
 *           index that is returned should be -1. 
 *
 * pos_index: used as a return value for the position index of matching element
 *
 *
 */
 
 //pasted in
data_t * list_elem_find(list_t *list_ptr, data_t *elem_ptr, int *pos_index)
{
    list_debug_validate(list_ptr);

    int i, comp = -99;
    //loop through list
    for (i = 0; i < list_ptr->current_list_size; i++) {
        comp = (list_ptr->comp_proc)(list_access(list_ptr, i), elem_ptr);
        if (comp == 1) {
            *pos_index = i;
            return list_access(list_ptr, *pos_index);
        }
    }
            
    if (comp == -99) {
        *pos_index = -1;
        return NULL;
    }
    return NULL;
}

/* Deallocates the contents of the specified list, releasing associated memory
 * resources for other purposes.
 *
 * Free all elements in the list, and the header block.  Use the data_clean
 * function point to free the data_t items in the list.
 */
void list_destruct(list_t *list_ptr)
{
    /* the first line must validate the list */
    list_debug_validate(list_ptr);
    list_node_t* current = list_ptr->head;
    
    //iterate through list and free nodes
    while (current != NULL) {             
        list_ptr->data_clean(current->data_ptr);        
        list_node_t* temp = current->next;
        free(current);
        current = temp;
    }

    free(current);
    free(list_ptr);
}

/* Inserts the specified data element into the specified list at the specified
 * position.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * pos_index: numeric position index of the element to be inserted into the 
 *            list.  Index starts at 0 at head of the list, and incremented by 
 *            one until the tail is reached.  The index can also be equal
 *            to LISTPOS_HEAD or LISTPOS_TAIL (these are special negative 
 *            values use to provide a short cut for adding to the head
 *            or tail of the list).
 *
 * If pos_index is greater than the number of elements currently in the list, 
 * the element is simply appended to the end of the list (no additional elements
 * are inserted).
 *
 * Note that use of this function results in the list to be marked as unsorted,
 * even if the element has been inserted in the correct position.  That is, on
 * completion of this subroutine the list_ptr->list_sorted_state must be equal 
 * to LIST_UNSORTED.
 */
void list_insert(list_t *list_ptr, data_t *elem_ptr, int pos_index)
{
    assert(list_ptr != NULL);
    int i;
    
    list_node_t *newNode = (list_node_t *)malloc(sizeof(list_node_t));
    newNode->data_ptr = elem_ptr;
    newNode->prev = NULL;
    newNode->next = NULL;
    
    //list is empty
    if (list_ptr->current_list_size == 0) {
        list_ptr->head = newNode;
        list_ptr->tail = newNode;
        newNode->prev = NULL;
        newNode->next = NULL;
    }
    //insert at head of list
    else if (pos_index == LISTPOS_HEAD || pos_index == 0) {
        newNode->next = list_ptr->head;
        list_ptr->head->prev = newNode;
        list_ptr->head = newNode;
    }
    //insert at tail of list
    else if (pos_index == LISTPOS_TAIL || (pos_index >= (list_ptr->current_list_size))) {   
        newNode->prev = list_ptr->tail;
        list_ptr->tail->next = newNode;  
        list_ptr->tail = newNode;
    }
    //insert to list
    else {
        list_node_t *current = list_ptr->head;
        for (i = 0; i < pos_index; i++) {
            current = current->next;
        }
        newNode->next = current;
        newNode->prev = current->prev;
        newNode->prev->next = newNode;
        newNode->next->prev = newNode;
    }
    list_ptr->current_list_size++;
          
    /* the last two lines of this function must be the following */
    if (list_ptr->list_sorted_state != LIST_UNSORTED)
        list_ptr->list_sorted_state = LIST_UNSORTED;
    list_debug_validate(list_ptr);
}

/* Inserts the element into the specified sorted list at the proper position,
 * as defined by the comp_proc function pointer found in the header block.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * elem_ptr: pointer to the element to be inserted into list.
 *
 * If you use list_insert_sorted, the list preserves its sorted nature.
 *
 * If you use list_insert, the list will be considered to be unsorted, even
 * if the element has been inserted in the correct position.
 *
 * If the list is not sorted and you call list_insert_sorted, this subroutine
 * should generate a system error and the program should immediately stop.
 *
 * The comparison procedure must accept two arguments (A and B) which are both
 * pointers to elements of type data_t.  The comparison procedure returns an
 * integer code which indicates the precedence relationship between the two
 * elements.  The integer code takes on the following values:
 *    1: A should be closer to the list head than B
 *   -1: B should be closer to the list head than A
 *    0: A and B are equal in rank
 * This definition results in the list being in ascending order.  To insert
 * in decending order, change the sign of the value that is returned.
 *
 * Note: if the element to be inserted is equal in rank to an element already
 * in the list, the newly inserted element will be placed after all the
 * elements of equal rank that are already in the list.
 */
void list_insert_sorted(list_t *list_ptr, data_t *elem_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->list_sorted_state != LIST_UNSORTED);
    
    //malloc new node
    list_node_t *newNode = (list_node_t *)malloc(sizeof(list_node_t));
    newNode->data_ptr = elem_ptr;
    newNode->next = NULL;
    newNode->prev = NULL;
    
    int sortedState;
    sortedState = list_ptr->list_sorted_state == LIST_SORTED_ASCENDING ? 1 : -1;
    
    //list is empty
    if (list_ptr->current_list_size == 0) {
        list_ptr->head = newNode;
        list_ptr->tail = newNode;
        list_ptr->current_list_size++;
        return;
    }
    //new head
    else if ((list_ptr->comp_proc(list_ptr->head->data_ptr, elem_ptr) * sortedState) < 0) {
        newNode->next = list_ptr->head;
        list_ptr->head->prev = newNode;
        list_ptr->head = newNode;
    }
    //new tail
    else if ((list_ptr->comp_proc(list_ptr->tail->data_ptr, elem_ptr) * sortedState) > 0) {
        newNode->prev = list_ptr->tail;
        list_ptr->tail->next = newNode;
        list_ptr->tail = newNode;
    }
    //standard insert
    else {
        list_node_t * prev = list_ptr->head;
        while((list_ptr->comp_proc(prev->next->data_ptr, elem_ptr) * sortedState) >= 0) {
            prev = prev->next;
        }
        newNode->next = prev->next;
        newNode->prev = prev;
        newNode->prev->next = newNode;
        newNode->next->prev = newNode;
    }

    list_ptr->current_list_size++;   

    /* the last line of this function must be the following */
    list_debug_validate(list_ptr);
}

/* Removes an element from the specified list, at the specified list position,
 * and returns a pointer to the element.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * pos_index: position of the element to be removed.  Index starts at 0 at
 *            head of the list, and incremented by one until the tail is
 *            reached.  Can also specify LISTPOS_HEAD and LISTPOS_TAIL
 *
 * Attempting to remove an element at a position index that is not contained in
 * the list will result in no element being removed, and a NULL pointer will be
 * returned.
 */
data_t * list_remove(list_t *list_ptr, int pos_index)
{   
    //set pos_index if needed
    if (pos_index == LISTPOS_HEAD) {
        pos_index = 0;
    } else if (pos_index == LISTPOS_TAIL) {
        pos_index = list_ptr->current_list_size - 1;
    }
    
    if (list_ptr->current_list_size == 0) {
        return NULL;
    }
    if (pos_index < 0 || pos_index >= list_ptr->current_list_size) {
        return NULL;
    }
    
    int i;
    list_node_t *current, *current2;
    current = list_ptr->head;
    
    for (i = 0; i < pos_index; i++) {
            current = current->next;
    }
    current2 = current; //hold onto atom to remove
    
    //if only one element
    if (list_ptr->current_list_size == 1) {
        current->prev = NULL;
        current->next = NULL;
        list_ptr->head = NULL;
        list_ptr->tail = NULL;
        list_ptr->current_list_size--;
        list_debug_validate(list_ptr);
        return current2->data_ptr;
    }
    //is head
    if (pos_index == 0) {
        current = current->next;
        current->prev = NULL;
        list_ptr->head = current;
        list_ptr->current_list_size--;
        list_debug_validate(list_ptr);
        return current2->data_ptr;
    }
    //is tail
    if (pos_index == (list_ptr->current_list_size - 1)) {
        current = current->prev;
        current->next = NULL;
        list_ptr->tail = current;
        list_ptr->current_list_size--;
        list_debug_validate(list_ptr);
        return current2->data_ptr;
    }  
    
    //set to node after current
    current = current->next;
    current->prev = current2->prev;
    current2->prev->next = current;
    list_ptr->current_list_size--;
        
    /* fix the return value */
    list_debug_validate(list_ptr);
    return current2->data_ptr;
}

/* Reverse the order of the elements in the list.  Also change the 
 * list_sorted_state flag.  This function can only be called on a list
 * that is sorted.
 *
 * list_ptr: pointer to list-of-interest.  
 */
void list_reverse(list_t *list_ptr)
{
    assert(list_order(list_ptr) != 0);
    list_node_t *current, *temp, *temp2;
    current = list_ptr->head;
    temp2 = current;
    
    while (current != NULL) {
        temp = current->prev;
        current->prev = current->next;
        current->next = temp;
        current = current->prev;
    }
    if (temp != NULL) {
        list_ptr->head = temp->prev;
    }
    list_ptr->tail = temp2;
    //change sorted state
    if (list_ptr->list_sorted_state == LIST_SORTED_ASCENDING) {
        list_ptr->list_sorted_state = LIST_SORTED_DESCENDING;
    } else if (list_ptr->list_sorted_state == LIST_SORTED_DESCENDING) {
        list_ptr->list_sorted_state = LIST_SORTED_ASCENDING;
    }
    
    // after the list is reversed verify it is valid.
    list_debug_validate(list_ptr);
}

/* Obtains the length of the specified list, that is, the number of elements
 * that the list contains.
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * Returns an integer equal to the number of elements stored in the list.  An
 * empty list has a size of zero.
 */
int list_size(list_t *list_ptr)
{
    assert(list_ptr != NULL);
    assert(list_ptr->current_list_size >= 0);
    return list_ptr->current_list_size;
}

/* Obtains the sort status and order of the specified list. 
 *
 * list_ptr: pointer to list-of-interest.  
 *
 * Returns 
 *    1: the list is sorted in ascending order
 *   -1: descending order
 *    0: the list is not sorted but a queue
 */
int list_order(list_t *list_ptr)
{
    assert(list_ptr != NULL);
    if (list_ptr->list_sorted_state == LIST_SORTED_ASCENDING)
        return 1;
    else if (list_ptr->list_sorted_state == LIST_SORTED_DESCENDING)
        return -1;
    else if (list_ptr->list_sorted_state == LIST_UNSORTED)
        return 0;
    else 
        exit(5);  // this should not happen
}


/* This function verifies that the pointers for the two-way linked list are
 * valid, and that the list size matches the number of items in the list.
 *
 * If the linked list is sorted it also checks that the elements in the list
 * appear in the proper order.
 *
 * The function produces no output if the two-way linked list is correct.  It
 * causes the program to terminate and print a line beginning with "Assertion
 * failed:" if an error is detected.
 *
 * The checks are not exhaustive, so an error may still exist in the
 * list even if these checks pass.
 *
 * YOU MUST NOT CHANGE THIS FUNCTION.  WE USE IT DURING GRADING TO VERIFY THAT
 * YOUR LIST IS CONSISTENT.
 */
void list_debug_validate(list_t *L)
{
    list_node_t *N;
    int count = 0;
    assert(L != NULL);
    if (VALIDATE_STATUS != 1)
        return;

    if (L->head == NULL)
        assert(L->tail == NULL && L->current_list_size == 0);
    if (L->tail == NULL)
        assert(L->head == NULL && L->current_list_size == 0);
    if (L->current_list_size == 0)
        assert(L->head == NULL && L->tail == NULL);
    assert(L->list_sorted_state == LIST_SORTED_ASCENDING 
            || L->list_sorted_state == LIST_SORTED_DESCENDING
            || L->list_sorted_state == LIST_UNSORTED);

    if (L->current_list_size == 1) {
        assert(L->head == L->tail && L->head != NULL);
        assert(L->head->next == NULL && L->head->prev == NULL);
        assert(L->head->data_ptr != NULL);
    }
    if (L->head == L->tail && L->head != NULL)
        assert(L->current_list_size == 1);
    if (L->current_list_size > 1) {
        assert(L->head != L->tail && L->head != NULL && L->tail != NULL);
        N = L->head;
        assert(N->prev == NULL);
        while (N != NULL) {
            assert(N->data_ptr != NULL);
            if (N->next != NULL)
                assert(N->next->prev == N);
            else
                assert(N == L->tail);
            count++;
            N = N->next;
        }
        assert(count == L->current_list_size);
    }
    if (L->list_sorted_state != LIST_UNSORTED && L->head != NULL) {
        N = L->head;
        int comp_val = -1 * list_order(L);
        while (N->next != NULL) {
            assert((L->comp_proc)(N->data_ptr, N->next->data_ptr) != comp_val);
            N = N->next;
        }
    }
}
/* commands for vim. ts: tabstop, sts: softtabstop sw: shiftwidth */
/* vi:set ts=8 sts=4 sw=4 et: */

