#include"list.h"
#include<stdlib.h>
#include<stdio.h>


/* define here a debug macro */
#ifdef DEBUG
#define DEBUG_PRINTF(...) \
	do {					  						\
		printf("In %s - function %s at line %d: ", __FILE__, __func__, __LINE__);		\
		printf(__VA_ARGS__);								\
	  } while(0)
#else 
	#define DEBUG_PRINTF(...)
#endif

typedef void element_copy_func(element_ptr_t *dest_element, element_ptr_t src_element);
typedef void element_free_func(element_ptr_t *element);
typedef int  element_compare_func(element_ptr_t x, element_ptr_t y);
typedef void element_print_func(element_ptr_t element);


struct list{
  list_node_ptr_t head;
  element_copy_func     *element_copy;
  element_free_func     *element_free;
  element_compare_func  *element_compare;
  element_print_func    *element_print;
  int size;
};

struct list_node{
  element_ptr_t     ele_ptr;
  list_node_ptr_t   next_node,pre_node;
};


list_ptr_t list_create 	( // callback functions
/*			  void (*element_copy)(element_ptr_t *dest_element, element_ptr_t src_element),
			  void (*element_free)(element_ptr_t *element),
			  int (*element_compare)(element_ptr_t x, element_ptr_t y),
			  void (*element_print)(element_ptr_t element)
*/			
		element_copy_func element_copy,
		element_free_func element_free,
		element_compare_func element_compare,
		element_print_func element_print
        )
{

	DEBUG_PRINTF("\n list_create called\n\n");
	list_errno = LIST_NO_ERROR;
	list_ptr_t lptr= (list_ptr_t) malloc(sizeof(list_t));
	if(lptr == NULL){
	list_errno = LIST_MEMORY_ERROR;
	return NULL;
	}else{
	lptr-> head = NULL;
	lptr->element_copy = element_copy;
	lptr->element_free = element_free;
	lptr->element_compare = element_compare; 
	lptr->element_print = element_print;
	lptr->size = 0;
	return lptr;
	}
}
// Returns a pointer to a newly-allocated list.
// Returns NULL if memory allocation failed and list_errno is set to LIST_MEMORY_ERROR 

void list_free( list_ptr_t* list ){

    DEBUG_PRINTF("\n list_free called\n\n");
	list_errno=LIST_NO_ERROR;
	
	if(list == NULL){
		printf("Invalid list\n");
		list_errno = LIST_MEMORY_ERROR;
	}else{
		list_node_ptr_t next_node_ptr = (*list)->head;
		(*list)->head = NULL;
		list_node_ptr_t this_node = next_node_ptr;
		
		while(next_node_ptr != NULL){  //free elements and node, mind the sequence 
			//free element and set ptr to NULL
		    ((*list)->element_free)(&this_node->ele_ptr);
			this_node->ele_ptr = NULL; 
			//move the next_node_a to the next node and set the next_node ptr in this node to NULL.
			next_node_ptr = this_node->next_node;
			this_node->next_node = NULL;
			//free this node and set this node ptr to NULL
		    free(this_node);
			this_node = next_node_ptr;			
		}

		free(*list);  // free list and set to NULL
		*list = NULL; 
	}
}
// Every list node and node element of the list needs to be deleted (free memory)
// The list itself also needs to be deleted (free all memory) and set to NULL


int list_size( list_ptr_t list ){
	DEBUG_PRINTF("\n list_size called\n\n");
	list_errno=LIST_NO_ERROR;
	if(list == NULL){
      	list_errno= LIST_INVALID_ERROR;
 		return 0;
    } 
	else   
    	return list->size;
}
// Returns the number of elements in 'list'.

list_ptr_t list_insert_at_index( list_ptr_t list, element_ptr_t element, int index){
	
	DEBUG_PRINTF("\n list insert at index called\n\n");
	list_errno=LIST_NO_ERROR;

    if(list == NULL){
      list_errno= LIST_INVALID_ERROR;
      printf("list is null\n");
      return NULL;
    }    

    list_node_ptr_t temp = (list_node_ptr_t)malloc(sizeof(list_node_t));

    if(temp == NULL){
      list_errno=LIST_MEMORY_ERROR;
      return NULL;
    }else{
	    if(index <= 0){ //insert in the beginning	 
			list_node_ptr_t temp_next = list_get_reference_at_index(list, 0);
			// list->head should be the same as temp_next;
			if(temp_next != NULL)
				temp_next->pre_node = temp;
			
		   	list->element_copy(&(temp->ele_ptr),element);  //element_copy 
			temp->pre_node= NULL;
			temp->next_node= temp_next;
			list->head = temp; 
			list->size++;
			return list;
		}else if(index>= list->size){ // insert in the end
   			list_node_ptr_t temp_pre = list_get_reference_at_index(list, list->size-1 );
			if(temp_pre != NULL)
 				temp_pre->next_node = temp;
			else
				list->head = temp; 
			
		 	list->element_copy(&temp->ele_ptr,element);  //element_copy
		 	temp->next_node= NULL;
		 	temp->pre_node= temp_pre; 	
		 	list->size++;
			
		 	return list;
		}else{ // insert in between
			 list_node_ptr_t temp_pre,temp_next;
			 temp_pre= list_get_reference_at_index(list, index-1 );
			 temp_next= list_get_reference_at_index(list, index );

			 temp_pre->next_node = temp;
			 temp_next->pre_node = temp;
			 list->element_copy(&temp->ele_ptr,element);  //element_copy
			 temp->pre_node= temp_pre;
			 temp->next_node = temp_next;
			 
			 list->size++;
		     return list;
		}
	}
}
// Inserts a new list node containing 'element' in 'list' at position 'index'  and returns a pointer to the new list.
// Remark: the first list node has index 0.
// If 'index' is 0 or negative, the list node is inserted at the start of 'list'. 
// If 'index' is bigger than the number of elements in 'list', the list node is inserted at the end of 'list'.
// Returns NULL if memory allocation failed and list_errno is set to LIST_MEMORY_ERROR 

list_ptr_t list_remove_at_index( list_ptr_t list, int index){

    DEBUG_PRINTF("\n list remove at index called\n\n");
	list_errno=LIST_NO_ERROR;
    if(list == NULL){
        list_errno=  LIST_INVALID_ERROR;
		return list;
 	}else{
		list_node_ptr_t temp_pre,temp_next,temp_this;
		temp_this= list_get_reference_at_index(list, index);

        if(temp_this == NULL){
            list_errno=LIST_EMPTY_ERROR;
            return list;
        }

		temp_pre = temp_this->pre_node;
		temp_next = temp_this-> next_node;
		
		if(temp_pre == NULL ){
			if(temp_next == NULL){		//only one node in list
				list->head =NULL;		 
			}else{     							// more than one node in list, remove first node
				temp_next->pre_node = temp_pre;
				list->head = temp_next;	
				temp_this->next_node = NULL;
			}
		}else{
			if(temp_next == NULL){ 	//more than one node in list, remove last node
				temp_pre->next_node = temp_next;
				temp_this->pre_node = NULL;
			}else{    							// remove in between
				temp_pre->next_node = temp_next; 
				temp_this->next_node= NULL;
				temp_next->pre_node = temp_pre;
				temp_this->pre_node = NULL;
			}
		}
        
		free(temp_this);  //free this node.
		list->size--;
		return list;
	}
}

// Removes the list node at index 'index' from 'list'. NO free() is called on the element pointer of the list node. 
// If 'index' is 0 or negative, the first list node is removed. 
// If 'index' is bigger than the number of elements in 'list', the last list node is removed.
// If the list is empty, return list and list_errno is set to LIST_EMPTY_ERROR (to see the difference with removing the last element from a list)

list_ptr_t list_free_at_index( list_ptr_t list, int index){

    DEBUG_PRINTF("\n lsit free at index called\n\n");
	list_errno=LIST_NO_ERROR;

    if(list == NULL){
        list_errno=LIST_INVALID_ERROR;
		return list;
 	}else{
		list_node_ptr_t temp_pre,temp_next,temp_this;
		temp_this= list_get_reference_at_index(list, index);

        if(temp_this == NULL){
            list_errno=LIST_EMPTY_ERROR;
            return list;
        }

		temp_pre = temp_this->pre_node;
		temp_next = temp_this-> next_node;
		
		if(temp_pre == NULL ){
			if(temp_next == NULL){		//only one node in list
				list->head =NULL;		 
			}else{     							// more than one node in list, remove first node
				temp_next->pre_node = temp_pre;
				list->head = temp_next;	
				temp_this->next_node = NULL;
			}
		}else{
			if(temp_next == NULL){ 	//more than one node in list, remove last node
				temp_pre->next_node = temp_next;
				temp_this->pre_node = NULL;
			}else{    							// remove in between
				temp_pre->next_node = temp_next; 
				temp_this->next_node= NULL;
				temp_next->pre_node = temp_pre;
				temp_this->pre_node = NULL;
			}
		}

		list->element_free(&temp_this-> ele_ptr);   //free element.  
		
		free(temp_this);  //free this node.
		temp_this = NULL;
		list->size--;
		return list;
	}
}
// Deletes the list node at index 'index' in 'list'. 
// A free() is called on the element pointer of the list node to free any dynamic memory allocated to the element pointer. 
// If 'index' is 0 or negative, the first list node is deleted. 
// If 'index' is bigger than the number of elements in 'list', the last list node is deleted.
// If the list is empty, return list and list_errno is set to LIST_EMPTY_ERROR (to see the difference with freeing the last element from a list)

list_node_ptr_t list_get_reference_at_index( list_ptr_t list, int index )
{
	DEBUG_PRINTF("\n list get reference at index called\n\n");
	list_errno=LIST_NO_ERROR;
	
	if(list == NULL){
		list_errno=LIST_INVALID_ERROR;
		return NULL;
	}

	int i;
	list_node_ptr_t temp = list->head;

	if(list->size ==0){
		return NULL;
	}else{
		if(index<=0){                 //return first node
			return list->head; 
		}else if(index >= list->size-1){  //return last node
			for(i=0;i<list->size-1; i++)
				temp = temp->next_node;
			return temp;
		}else{                        //return node at index 
			for(i=0;i<index; i++)
				temp = temp->next_node;
			return temp;
		}
	}
}
// Returns a reference to the list node with index 'index' in 'list'. 
// If 'index' is 0 or negative, a reference to the first list node is returned. 
// If 'index' is bigger than the number of list nodes in 'list', a reference to the last list node is returned. 
// If the list is empty, NULL is returned.

element_ptr_t list_get_element_at_index( list_ptr_t list, int index )
{
    DEBUG_PRINTF("\n get element at index called\n\n");
	if(list == NULL)  
		return NULL;
	else if(list->size ==0)
		return NULL;
	else
	{
		list_node_ptr_t temp = list_get_reference_at_index(list,index);
		if(temp == NULL){
			// printf(" node is NULL");
			//nnode ptr is NULL;
			return NULL;
		}else{
			// printf(" node is not NULL");
			//node ptr is not NULL\n");
			return temp->ele_ptr;
		}
	}
}
// Returns the list element contained in the list node with index 'index' in 'list'. 
// If 'index' is 0 or negative, the element of the first list node is returned. 
// If 'index' is bigger than the number of elements in 'list', the element of the last list node is returned.
// If the list is empty, NULL is returned.

int list_get_index_of_element( list_ptr_t list, element_ptr_t element )
{
	list_node_ptr_t temp = list->head;
	int i=0;
	while(temp != NULL)
	{
		if(list->element_compare(temp->ele_ptr, element) == 1)
		{
			return i;
		}
		temp=temp->next_node;
		i++;
	}
	return -1;
}
// Returns an index to the first list node in 'list' containing 'element'.  
// If 'element' is not found in 'list', -1 is returned.

void list_print( list_ptr_t list )
{
	DEBUG_PRINTF("\n print called\n\n");
	list_node_ptr_t temp = list->head;
	while(temp != NULL)
	{
		list->element_print(temp->ele_ptr);
		temp=temp->next_node;
	}
}
// for testing purposes: print the entire list on screen
