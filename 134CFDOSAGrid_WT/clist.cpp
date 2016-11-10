#include "clist.h"
#include <list>
using namespace std;

typedef list<void*> ll;
typedef ll::iterator it;
typedef ll::const_iterator cit;

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

extern "C"
{
	// returns a new list object
	scl* clist_alloc(){
		scl* nl = new scl;
		ll* ol = new ll;
		nl->list = reinterpret_cast<void*>(ol);
		nl->data = 0;
		it* i = new it(ol->begin());
		nl->node = reinterpret_cast<void*>(i);
		return (nl);
	}

	// Returns a new list object, copied from existing list, call list free when done
	scl* clist_copy(scl *l)
	{
		scl* nl = NULL;
		if(l){
			nl = new scl;
			ll* ol = new ll(*reinterpret_cast<ll*>(l->list));
			nl->list = reinterpret_cast<void*>(ol);
			it* i = new it(ol->begin());
			nl->node = reinterpret_cast<void*>(i);
			nl->data = *(*i);
		}else{
			nl = clist_alloc();
		}
		return (nl);
	}
	
	// destroys an unused list
	void  clist_free(scl* l){
		if(l){
			if(l->node){
				it* i = reinterpret_cast<it*>(l->node);
				delete i;
			}
			if(l->list){
				ll* ol = reinterpret_cast<ll*>(l->list);
				ol->clear( );
				delete ol;			
			}
		}
	}
	// Returns TRUE if list empty, FALSE otherwise
	int clist_is_empty(scl* l){
		int nResult = TRUE;
		if(l){
			ll* ol = reinterpret_cast<ll*>(l->list); 
			if(ol)
				nResult = ol->empty( );
		}
		return nResult;
	}
	// Returns count of list elements
	int clist_count(scl* l)
	{
		int nCount = 0;
		if(l){
			ll* ol = reinterpret_cast<ll*>(l->list); 
			if(ol)
				nCount = ol->size( );
		}
		return nCount;
	}
	// returns the head node
	int clist_head(scl* l){
		int nResult = FALSE;
		if(l){
			ll* ol = reinterpret_cast<ll*>(l->list); 
			if(ol && !ol->empty()){			
				it* i = reinterpret_cast<it*>(l->node);
				(*i) = ol->begin();
				l->data = *(*i);
				nResult = TRUE;
			}
		}
		return nResult;
	}
	// returns the tail node
	int clist_tail(scl* l){
		int nResult = FALSE;
		if(l){
			ll* ol = reinterpret_cast<ll*>(l->list); 
			if(ol && !ol->empty()){			
				it* i = reinterpret_cast<it*>(l->node);
				(*i) = ol->end();
				(*i)--;
				l->data = *(*i);
				nResult = TRUE;
			}
		}
		return nResult;
	}
	// returns the next node in the list
	int clist_next(scl* l){
		int nResult = FALSE;
		if(l){
			ll* ol = reinterpret_cast<ll*>(l->list); 
			if(ol && !ol->empty()){
				it* i = reinterpret_cast<it*>(l->node);
				(*i)++;
				if((*i) != ol->end()){
					l->data = *(*i);
					nResult = TRUE;
				}
			}
		}
		return nResult;
	}
	// returns the prev node in the list
	int clist_prev(scl* l){
		int nResult = FALSE;
		if(l){
			ll* ol = reinterpret_cast<ll*>(l->list); 
			if(ol && !ol->empty()){			
				it* i = reinterpret_cast<it*>(l->node);
				if((*i) != ol->begin()){
					(*i)--;
					l->data = *(*i);
					nResult = TRUE;
				}
			}
		}
		return nResult;
	}
	// inserts item before anode, returns node
	void clist_insert(scl* l, void* i){
		ll* ol = reinterpret_cast<ll*>(l->list);
		it* p = reinterpret_cast<it*>(l->node);
		if(ol && p){
			(*p) = ol->insert((*p), i);
			l->data = i;
		}
	}
	// appends item to end of list
	void clist_add(scl* l, void* i){
		ll* ol = reinterpret_cast<ll*>(l->list); 
		if(ol)
			ol->push_back(i);
	}
	// Removes current item from list, owner responsible to free, true if list not empty
	int clist_del(scl* l){
		int nResult = FALSE;
		ll* ol = reinterpret_cast<ll*>(l->list);
		if(ol){
			if(!ol->empty()){
				it* i = reinterpret_cast<it*>(l->node);
				(*i) = ol->erase((*i));
				// erase moves to element next or end
				if((*i) != ol->end()){
					// set data to element next
					l->data = *(*i);
					nResult = TRUE;
				}else if(!ol->empty()){
					// list is not empty, set data to element prev
					(*i)--;
					l->data = *(*i);
					nResult = TRUE;
				}else{
					// list is empty, no data, iterator is equal to end()
					l->data = 0;
				}
			}
		}
		return nResult;
	}	
}
