#ifndef _LIST_H_
#define _LIST_H_

// C Wrapper for C++ STL list class (double-linked list)
// Note: Not all methods implemented
#ifdef __cplusplus
extern "C"
{	
#endif
	typedef struct clist_struct{
		void* list;	// C++ list object
		void* node; // C++ current iterator object
		void* data; // C data (when valid, yours!)
	} scl;

	// Returns a new list object, call list free when done
	scl* clist_alloc();
	// Returns a new list object, copied from existing list, call list free when done
	scl* clist_copy(scl * l);
	// Destroys an unused list, user responsible for freeing data
	void  clist_free(scl* l);
	// Returns TRUE if list empty, FALSE otherwise
	int clist_is_empty(scl* l);
	// Returns count of list elements
	int clist_count(scl* l);
	// Sets data to head of list, returns TRUE for success, otherwise FALSE (e.g. empty list)
	int clist_head(scl* l);
	// Sets data to tail of list, returns TRUE for success, otherwise FALSE (e.g. empty list)
	int clist_tail(scl* l);
	// Sets data to next in list, returns TRUE for success, otherwise FALSE (e.g. empty list)
	int clist_next(scl* l);				
	// Sets data to previous in list, returns TRUE for success, otherwise FALSE (e.g. empty list)
	int clist_prev(scl* l);

	// Appends item i to end of list
	void clist_add(scl* l, void* i);
	// Removes current item from list, owner responsible to free, true if list not empty
	int clist_del(scl* l);
	// Inserts item i before current element in list
	void clist_insert(scl* l, void* i);	
#ifdef __cplusplus
}
#endif
#endif//_LIST_H_