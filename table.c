/*	Donald Elmore
 *	Purpose: The goal of this machine problem is to design and implement a table ADT 
 	using hashing. There were three collision resolution policies: open 
 	addressing with linear probing and double hashing, and separate chaining. 
 	The performance evaluation will consider with additions and deletions in equilibrium. 
	The equilibrium driver will demonstrate that very poor performance is possible for open
	addressing when there are a large number of deletions, and that rehashing the table
	is required to restore the table and achieve the expected performance.
 * 	Known bugs: the -e and -b tests do not run properly, but both of my test drivers
 		show all of the functionality of the operations on the hash table.
 */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "table.h"

#define EmptyKey NULL 
#define DeleteKey 1
#define PRIME 5

int equal_key(char *k1, char *k2);
/* probing_type is one of LINEAR, DOUBLE, CHAIN */

unsigned int hash(hashkey_t key)
{
    unsigned h = 0;
    /* TODO */
    char *p = key;
    int i;

    for (i = 0; i < strlen(key); i++) {
        h = 33 * h ^ p[i];
    }

    return h;
}

unsigned int probe(int addr)
{
    unsigned h = 0;    
    
    h = PRIME - (addr % PRIME);
    
    return h;
}

/* Creates space for the table and constucts an "blank table in 'oa' or 'sc'
 * depending on tree type
 *
 * table_size - maximum size the table 
 * probing_type - probing method to use for the table
 *
 * RETURNS - newly created table
 */
table_t *table_construct (int table_size, int probing_type)
{
	/* TODO: allocate memory for table and populate each entry */
    int i;
    table_t *T = (table_t *)malloc(sizeof(table_t));
    if (T == NULL) {
    	return NULL;
    }
    
	T->table_size = table_size;
	T->probing_type = probing_type;
	T->num_stored_keys = 0;
	T->num_probes_for_most_recent_call = 0;
	
	if (probing_type != CHAIN) {
		T->oa = (table_entry_t *)malloc(sizeof(table_entry_t) * T->table_size);
		
		for (i = 0; i < T->table_size; i++) {
			T->oa[i].key = EmptyKey;
			T->oa[i].data_ptr = NULL;
			T->oa[i].deleted = 0;
		}
	}
	else if (probing_type == CHAIN) {
		T->sc = (sep_chain_t **)malloc(sizeof(sep_chain_t) * (T->table_size));

		for (i = 0; i < T->table_size; i++) {
			//T->sc[i] = (sep_chain_t *)malloc(sizeof(sep_chain_t));
			T->sc[i] = NULL;
		}
	}

    return T;
}

/* Rehashes (copies) table T into an new table of size 'new_table_size'.
 *
 * T - table to rehash
 * new_table_size - size of new table to construct
 *
 * RETURN - newly constructed table
 */
table_t *table_rehash (table_t * T, int new_table_size)  
{
    if (T->probing_type == CHAIN) {
        printf("\nDid not implement rehash\n");
        return T;
    }
    /* TODO: build new table to rehash. destroy the old table */
    int i;// check;
    table_t *new_table = table_construct(new_table_size, T->probing_type);
    printf("\n");
    for (i = 0; i < T->table_size; i++) {
    	if (T->oa[i].key != EmptyKey) {
    		//printf("Key: %s \t Data: %p\n", T->oa[i].key, T->oa[i].data_ptr);
    		table_insert(new_table, strdup(T->oa[i].key), T->oa[i].data_ptr);
    	}
    	//printf("check = %d\n", check);
    	//assert(check == 0);
    }
    new_table->num_stored_keys = T->num_stored_keys;
    table_destruct(T);
    
    return new_table;    
}

/* returns number of entries in the table */ 
int table_entries (table_t *T)
{
    /* TODO */
    return T->num_stored_keys;
}

/* returns 1 if table is full and 0 if not full. */
int table_full(table_t *T)
{
    /* TODO: remember chaining is different */
    if (T->probing_type != CHAIN) {
		if (T->num_stored_keys < (T->table_size - 1)) {
			return 0;
		}
		else if (T->num_stored_keys == (T->table_size - 1)) {
			return 1;
		}
	}	
	//List can not be full with chaining?
	return 0;
}

/* returns the number of keys in the table that are marked 'deleted' */
int table_deletekeys(table_t *T)
{
    /* TODO: count the keys that are 'deleted' remember chaining is different */
    int i, count = 0;
    if (T->probing_type != CHAIN) {
    	for (i = 0; i < T->table_size; i++) {
    		if (T->oa[i].deleted == DeleteKey) {
    			count++;
    		}
    	}
    	return count;
    }
    
    /*//I assume we only count as deleted if the head node in list is marked?
    //Because to delete a node in a list you just fill the gap.
    else if (T->probing_type == CHAIN) {
    	for (i = 0; i < T->table_size; i++) {
    		if (*T->sc[i]->key == DeleteKey) {
    			count++;
    		}
    	}
    } */
    
    return 0;
}
   
/* Insert a new table entry (K, I) into the table provided the table is not
 * already full.  
 * Return:
 *      0 if (K, I) is inserted, 
 *      1 if an older (K, I) is already in the table (update with the new I), or 
 *     -1 if the * (K, I) pair cannot be inserted.
 */
int table_insert (table_t *T, hashkey_t key, data_t D)
{
    int addr, first_addr, prob_dec;
    sep_chain_t *new;
    T->num_probes_for_most_recent_call = 0;
    addr = hash(key) % T->table_size;
    first_addr = addr;
    int M = T->table_size;
    
    if (table_full(T)) {
    	return -1;
    }
    T->num_stored_keys++;

  	if (T->probing_type != CHAIN) {
        // TODO: calc prob_dec
        if (T->probing_type == LINEAR) {
        	prob_dec = 1;
        } else
        	prob_dec = probe(addr);
		do {
			T->num_probes_for_most_recent_call++;
			if ((T->oa[addr].key == EmptyKey) || (T->oa[addr].deleted == DeleteKey)) {
				T->oa[addr].key = key;
				T->oa[addr].data_ptr = D;
				T->oa[addr].deleted = 0;
				return 0;
			} 
			else if (equal_key(T->oa[addr].key, key)) {
				T->num_probes_for_most_recent_call++;
				T->oa[addr].data_ptr = D;
				return 1;
			}
			T->num_probes_for_most_recent_call++;
			addr = (addr + prob_dec) % M;
			
		} while (addr != first_addr);
    }

    else { // CHAIN
    	new = (sep_chain_t *)malloc(sizeof(sep_chain_t));
    	if (new == NULL) {
    		free(new);
    		return -1;
    	}
    	new->key = key;
    	new->data_ptr = D;
    	new->next = NULL;
    	
		//chain is empty
        if (T->sc[addr] == NULL) {
        	T->sc[addr] = new;        	
        } else { // chain is not empty
            sep_chain_t *current = T->sc[addr];
            while (current->next != NULL) {
            	if (equal_key(current->key, new->key)) {
            		current = new;
            		return 1;
            	}
            	T->num_probes_for_most_recent_call++;
            	current = current->next;
            }
            current->next = new;
        }
    }

    return 0;
}

/* Delete the table entry (K, I) from the table.  
 * Return:
 *     pointer to I, or
 *     null if (K, I) is not found in the table.  
 *
 * See the note on page 490 in Standish¿s book about marking table entries for
 * deletions when using open addressing.
 */
data_t table_delete (table_t *T, hashkey_t key) 
{
    T->num_probes_for_most_recent_call = 1;	//changed to 1 from 0
    int addr = hash(key) % T->table_size;
    int prob_dec;
    int M = T->table_size;
    int first_addr = addr;
    data_t *returnData;
    sep_chain_t *prev = NULL;;

    if (T->probing_type == CHAIN) {
    	//no chain/list
    	if (T->sc[addr] == NULL)
    		return NULL;
    	sep_chain_t *current = T->sc[addr];
    	//if only one node
    	if (current->next == NULL) {
			returnData = current->data_ptr;
			free(current->key);
			free(current);
			T->sc[addr] = NULL;
			T->num_stored_keys--;
			return returnData;
		}
    	while (current->next != NULL) {
    		if (equal_key(current->key, key) == 1) {
    			//if at beginning of chain
    			if (prev == NULL && current->next != NULL) {
    				returnData = current->data_ptr;
    				T->sc[addr] = current->next;
    				free(current->key);
					free(current);
					T->num_stored_keys--;
					return returnData;
				}
				//if in middle of chain
				else if (prev != NULL && current->next != NULL) {
					returnData = current->data_ptr;
					prev->next = current->next;
					free(current->key);
					free(current);
					T->num_stored_keys--;
					return returnData;
				}
				//if at end of list
				else if (prev != NULL && current->next == NULL) {
					returnData = current->data_ptr;
					prev->next = NULL;
					free(current->key);
					free(current);
					T->num_stored_keys--;
					return returnData;
				}
			}
			prev = current;
			current = current->next;
		}
    }
    else {
        // TODO set prob_dec based on type
        if (T->probing_type == LINEAR) {
        	prob_dec = 1;
        } else {
        	prob_dec = probe(addr);
        }
        // TODO look to find correct entry
		do {
			//key found, delete key
			T->num_probes_for_most_recent_call++;
			if (equal_key(T->oa[addr].key, key)) {
				T->oa[addr].deleted = 1;
				returnData = T->oa[addr].data_ptr;
				free(T->oa[addr].key);
				T->num_stored_keys--;		//update num_stored
				return returnData;
			}
			addr = (addr + prob_dec) % M;
		} while (addr != first_addr);
    }

    return NULL;
}

/* Given a key, K, retrieve the pointer to the information, I, from the table,
 * but do not remove (K, I) from the table.  Return NULL if the key is not
 * found.
 */
data_t table_retrieve (table_t *T, hashkey_t key) 
{
    /* TODO: */
    int addr, prob_dec;
    T->num_probes_for_most_recent_call = 0;
    addr = hash(key) % T->table_size;
    int M = T->table_size;
    int first_addr = addr;
    data_t *returnData;

    if (T->probing_type == CHAIN) {
	    //no chain
	    T->num_probes_for_most_recent_call++;
    	if (T->sc[addr] == NULL) {
    		return NULL;
    	}
    	sep_chain_t *current = T->sc[addr];
    	
        while (equal_key(current->key, key) != 1) {
        	T->num_probes_for_most_recent_call++;
        	if (current->next == NULL) {
        		return NULL;
        	}
        	current = current->next;
        }
        returnData = current->data_ptr;	      
    	return returnData;
    }
    else {
        // TODO set prob_dec based on type
        if (T->probing_type == LINEAR) {
        	prob_dec = 1;
        } else {
        	prob_dec = probe(addr);
        	//printf("prob_dec = %d\n", prob_dec);
        }
        // TODO look to find correct entry
		do {
			T->num_probes_for_most_recent_call++;
			if (T->oa[addr].key != NULL) {
				T->num_probes_for_most_recent_call++;
				if (equal_key(T->oa[addr].key, key)) {
					//T->num_probes_for_most_recent_call++;
					returnData = T->oa[addr].data_ptr;
					return returnData;
				}
			}
			//T->num_probes_for_most_recent_call++;
			addr = (addr + prob_dec) % M;
		} while (addr != first_addr);
    }
    return NULL;
}

/* Free all information in the table, the table itself, and any additional
 * headers or other supporting data structures.  
 */
void table_destruct (table_t *T)
{
    /*TODO free all the memory*/
    int i;
    if (T->probing_type != CHAIN) {
    	for (i = 0; i < T->table_size; i++) {
    		if (T->oa[i].deleted != DeleteKey && T->oa[i].key != EmptyKey) {
    			//free(T->oa[i].data_ptr);  
    		}	
    		free(T->oa[i].key);
    	}
    	free(T->oa);
    }
    else if (T->probing_type == CHAIN) {
    	sep_chain_t *temp;
    	for (i = 0; i < T->table_size; i++) {
			sep_chain_t *current = T->sc[i];
			
			if (current != NULL) {
				while (current->next != NULL) {       //iterate through list and free nodes
					temp = current->next;
					//free(current->data_ptr);
					free(current->key);
					free(current);
					current = temp;
				}
				free(current->key);				
			}
			free(current);
		}
		free(T->sc);
	}
	free(T);
}

/* The number of probes for the most recent call to table_retrieve,
 * table_insert, or table_delete 
 */
int table_stats (table_t *T)
{
    /*TODO */
    return T->num_probes_for_most_recent_call;
}

/* Print the table position and keys in a easily readable and compact format.
 * Only useful when the table is small.
 */
void table_debug_print(table_t *T) {
    int i;
    int count = 0; 
    printf("keys in table %d\n", T->num_stored_keys);
    if (T->probing_type == CHAIN) {
        sep_chain_t *rover;
        for (i = 0; i < T->table_size; i++)
        {
            printf("%d: ", i);
            rover = T->sc[i];
            while (rover != NULL)
            {
                printf("%s ", rover->key);
                rover = rover->next;
                count++;
            }
            printf("\n");
        }
    } else {
        for (i = 0; i < T->table_size; i++)
        {
            if (T->oa[i].deleted == DeleteKey) {
                printf("%d: del\n", i);
            }
            else if (T->oa[i].key == EmptyKey) {
                printf("%d: em\n", i);
            }
            else {
                printf("%d: %s,\tdata: %p\n", i, T->oa[i].key, T->oa[i].data_ptr);
                count++;
            }
        }
    }
    //printf("count is = %d; num_stored is %d\n", count, T->num_stored_keys);
    assert(count == T->num_stored_keys);
}

/* This function is for testing purposes only.  Given an index position into
 * the hash table return the value of the key stored there or a 0 if the
 * index position does not contain a key.  For separate chaining, return the
 * first key found at this index position.  Make the first line of this
 * function
 *     assert(0 <= index && index < table_size);
 */
hashkey_t table_peek(table_t *T, int index, int position)
{
    assert(0 <= index && index < T->table_size);
    assert(position >= 0);
    int count = 0;
    
    if (T->probing_type == CHAIN) {
	    if (T->sc[index] == NULL)
	        return 0;
        sep_chain_t *rover = T->sc[index]->next;
        /*TODO walk down the chain */
        for (count = 0; count < position; count++) {
        	if (rover->next == NULL) {
        		return 0;
        	}
        	rover = rover->next;
       }
       if (T->sc[index]->key == NULL) {
    		return 0;
       }
       return rover->key;
    }
    else {
    	if ((T->oa[index].key == EmptyKey) || (T->oa[index].deleted) == DeleteKey) {
    		return 0;
    	}
    	return (T->oa[index].key);
    }  
}


int equal_key(char *k1, char *k2)
{
    return (strcmp(k1, k2) == 0);
}

