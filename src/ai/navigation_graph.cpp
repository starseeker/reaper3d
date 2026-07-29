
#include <iostream>
#include <string>
#include "hw/debug.h"
#include "res/res.h"
#include "res/config.h"
#include "misc/parse.h"
#include "main/types_io.h"
#include "ai/navigation_graph.h"
// #include "world/world.h"

namespace reaper{
namespace ai{
namespace navigation_graph{

namespace { debug::DebugOutput dout("ai::navigation_graph"); }


Graph::Graph()
// : wr(world::World::get_ref()) 
{
	id_table.resize(1000, nullptr);
}

std::unique_ptr<Node> Graph::load_node(std::istream& is)
{
	res::ConfigEnv env(is, true);

	if(env.defined("node") && env.defined("point") && env.defined("link1")){
		auto node = std::make_unique<Node>();

		node->id = read<unsigned int>(env["node"]);	
		node->pos = read<Point>(env["point"]);
		
		int i = 1;
		while(env.defined("link" + misc::ltos(i))){
			node->links.push_back(read<Link>(env["link" + misc::ltos(i)]));
			++i;
		}
		dout << "node loaded\n";
		return node;
	} else {
		return {};
	}
}

void Graph::build_graph(const std::string& filename)
{
	clear_graph();

	try {
		res::res_stream is(res::AI, filename, res::throw_on_error);

		res::ConfigEnv env(is, true);
		unsigned int nr_of_nodes = read<unsigned int>(env["nr_of_nodes"]);
		id_table.resize(nr_of_nodes);

		do {
			auto node = load_node(is);
			if(!node) {
				break;
			}

			if (node->id >= id_table.size())
				id_table.resize(node->id + 1, nullptr);

			Node* observer = node.get();
			id_table[node->id] = observer;
			graph.push_back(std::move(node));
		} while(true);

	} catch(const res::resource_not_found& e) {
		dout << e.what() << '\n';
		dout << "Building graph from terrain data...";
		dout.flush();


		// TODO : Implement graph builder


		dout << "done!\n";
	}
}

void Graph::clear_graph()
{
	graph.clear();
	id_table.clear();
}

NodePtr Graph::find_node(unsigned int id) const
{
	return id < id_table.size() ? id_table[id] : nullptr;
}

void Graph::find_onoff_ramps(
	const Point&,
	const Point&,
	NodePtr& on,
	NodePtr& off)
{
	if(!graph.empty()){
		
		// TODO: implement algorithm
		
		on = graph.front().get();
		off = graph.back().get();
	} else {
		on = off = nullptr;
	}
}


}
}
}

/*
 * $Author: pstrand $
 * $Date: 2002/04/11 01:02:04 $
 * $Log: navigation_graph.cpp,v $
 * Revision 1.11  2002/04/11 01:02:04  pstrand
 * explicit res_stream exception...
 *
 * Revision 1.10  2002/02/01 15:40:30  niklas
 * no message
 *
 * Revision 1.9  2002/01/27 18:26:06  niklas
 * *** empty log message ***
 *
 * Revision 1.8  2002/01/27 16:40:39  niklas
 * *** empty log message ***
 *
 * Revision 1.7  2002/01/26 23:27:11  macke
 * Matrox fix, plus some misc stuff..
 *
 * Revision 1.6  2002/01/24 21:33:16  peter
 * använd hellre misc::ltos&stol istf atoi&itoa...
 *
 * Revision 1.5  2002/01/24 20:24:45  niklas
 * *** empty log message ***
 *
 * Revision 1.4  2002/01/24 17:49:03  niklas
 * Inläsning av grafdata från fil
 *
 * Revision 1.3  2002/01/23 17:24:32  niklas
 * *** empty log message ***
 *
 * Revision 1.2  2002/01/22 23:55:04  niklas
 * no message
 *
 * Revision 1.1  2002/01/22 23:53:48  niklas
 * no message
 *
*/
