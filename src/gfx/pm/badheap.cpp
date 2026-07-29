#include "pm.h"
#include "treap.h"
#include "badheap.h"
#include "hw/debug.h"

namespace reaper 
{
namespace gfx
{
namespace pm
{

	Pair_node::Pair_node(Pairdat data)
	{
		pd = data;
		next = prev = nullptr;
	}

	Pair_node_ptr Slowheap::insert(Pairdat pd)
	{
		debug::DebugOutput dout("gfx::pm::sheap::insert");

		Pair_node_ptr cur = first;
		Pair_node_ptr neue;

		// Very First Element
		if(first == nullptr){
			neue = new Pair_node(pd);
			neue->next = nullptr;
			neue->prev = nullptr;
			first = last = neue;
			return neue;
		}

		while((pd.cost < cur->pd.cost) && (cur != last))cur = cur->next;
		neue = new Pair_node(pd);
		if(cur->pd.cost == pd.cost)return nullptr; // Element already stored
		if(cur == first){ // First Element
			first->prev = neue;
			neue->next = first;
			first = neue;
		} else if(pd.cost < cur->pd.cost){ // Last Element
			cur->next = neue;
			neue->next = nullptr;
			neue->prev = cur;
		} else {
			cur->prev->next = neue;
			neue->prev = cur->prev;
			neue->next = cur;
			cur->prev = neue;
		}

		return neue;
	}

	bool Slowheap::remove(Pair_node_ptr node)
	{
		if(node == nullptr) return true;

		if((node == last) && (node == first)){
			last = first = nullptr;
		} else if(node == last){
			last->prev->next = nullptr;
			last = last->prev;
		} else if(node == first){
			first->next->prev = nullptr;
			first = first->next;
		} else {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		}

		delete(node);



		return false;
	}

	bool Slowheap::update(Pair_node_ptr node)
	{
		Pair_node_ptr tmp;

		// update right
		while((node->next != last) && (node->pd.cost < node->next->pd.cost)){
			tmp = node->next;

			node->prev->next = node->next;
			node->next->prev = node->prev;

			tmp->next->prev = node;
			node->prev = tmp;
			node->next = tmp->next;
			tmp->next = tmp;
		}

		// Last element
		if(node->pd.cost < node->next->pd.cost){

			node->prev->next = node->next;
			node->next->prev = node->prev;

			last->next = node;
			node->prev = last;
			last = node;
			node->next = nullptr;
		}


		// update left
		while((node->prev != first) && (node->pd.cost > node->prev->pd.cost)){
			tmp = node->prev;

			node->prev->next = node->next;
			node->next->prev = node->prev;

			tmp->prev->next = node;
			node->prev = tmp->prev;
			node->next = tmp;
			tmp->prev = node;
		}

		// First element
		if(node->pd.cost > node->prev->pd.cost){

			node->prev->next = node->next;
			node->next->prev = node->prev;

			first->prev = node;			
			node->next = first;
			first = node;
			node->prev = nullptr;
		}

		return false;

	}


	Pair_node_ptr Slowheap::pop(void)
	{
		return last;
	}




}
}
}

