#ifndef REAPER_AI_A_STAR_H
#define REAPER_AI_A_STAR_H

#include <memory>
#include <unordered_map>
#include <vector>
#include "ai/navigation_graph.h"


namespace reaper{
namespace ai{
namespace astar{

	enum SearchState {
		SEARCH_STATE_NOT_INITIALISED,
		SEARCH_STATE_SEARCHING,
		SEARCH_STATE_SUCCEEDED,
		SEARCH_STATE_FAILED
	};

	/// A node in the pathfinding graph, not to be confused with 
	/// a node in the navigation graph. Contains astar specific data.
	class Node
	{
	public:
		navigation_graph::NodePtr navidata = nullptr;
		bool visited = false;

		Node* parent = nullptr;
		Node* child = nullptr;
					
		float g = 0; // Cost of this node + its predecessors
		float h = 0; // Heuristic estimate of cost to goal
		float f = 0; // Sum of g and h
		
		Node() = default;
		explicit Node(navigation_graph::NodePtr data) : navidata(data) { }

		// For debugging purposes
		void draw();
	};

	class NodeCmp
	{
	public:
		bool operator()(const Node *a, const Node *b) const
		{
			return (a->f > b->f);
		}
	};

	typedef std::vector<Node*>::iterator NodeListIterator;


	/// The astar search algorithm
	class AStar
	{
		navigation_graph::Graph &navigraph; // Reference to the navigation graph

		std::vector<Node*> open;   // The Open list (vector but used as a heap)
		std::vector<Node*> closed; // The Closed list
		
		std::vector<Node*> successors;
		std::unordered_map<
			navigation_graph::NodePtr,
			std::unique_ptr<Node>> nodes;
		
		Node* start = nullptr;
		Node* goal = nullptr;

		Node* current = nullptr;
		
		SearchState search_state = SEARCH_STATE_NOT_INITIALISED;
		int steps = 0;
		
		// Iterators for debugging the open and closed lists
		NodeListIterator dbg_open_it;
		NodeListIterator dbg_closed_it;

		void get_successors(Node* node);
		Node* get_node(navigation_graph::NodePtr node);
		float estimate_cost_to_goal(Node* node);
		float get_cost(Node* node1, Node* node2);

	public:

		explicit AStar(navigation_graph::Graph& ng);
		~AStar() = default;

		AStar(const AStar&) = delete;
		AStar& operator=(const AStar&) = delete;

		// Initialize the search
		void new_search(const Point& sp, const Point& gp);

		// Take one step forward in the search
		// and return the resulting state
		int search_step();

		// Methods for traversing a solution when a search has ended 
		Node* get_solution_start();
		Node* get_solution_end();
		Node* get_solution_next();
		Node* get_solution_prev();

		// Methods to view the open list during a search
		Node* get_open_start();
		Node* get_open_start(float& f, float& g, float& h);
		Node* get_open_next();
		Node* get_open_next(float& f, float& g, float& h);
		
		// Methods to view the closed list during a search
		Node* get_closed_start();
		Node* get_closed_start(float& f, float& g, float& h);
		Node* get_closed_next();
		Node* get_closed_next(float& f, float& g, float& h);

		// Return the number of steps taken in the search
		int get_step_count() const;

		// Function for reseting the search and clearing all lists
		void reset();
	};

}
}
}

#endif

/*
 * $Author: niklas $
 * $Date: 2002/02/01 15:40:30 $
 * $Log: astar.h,v $
 * Revision 1.4  2002/02/01 15:40:30  niklas
 * no message
 *
 * Revision 1.3  2002/01/27 16:40:39  niklas
 * *** empty log message ***
 *
 * Revision 1.2  2002/01/24 20:24:45  niklas
 * *** empty log message ***
 *
 * Revision 1.1  2002/01/23 17:24:32  niklas
 * *** empty log message ***
 *
*/
