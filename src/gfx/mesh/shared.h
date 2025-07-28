#ifndef REAPER_MESH_SHARED_H
#define REAPER_MESH_SHARED_H

#include <algorithm>
#include <vector>
#include <set>
#include <list>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define MESH_HASH_SIZE 1000

namespace reaper 
{
namespace gfx
{
namespace mesh
{


class SetTooSmall{};
class MalFormedInput{};
class ContractionOutOfDate{};

class MeshException
{
public:
	char* error;

	MeshException(char* e):error(e){}
};

enum ContStatus { OK, SUBDUE, INVALID };

class Timer {
public:

	Timer(void):ellapsed(0){};

	int start_time, ellapsed;

	void reset(void){ellapsed = 0;};
	void start(void){start_time = clock();};
	void stop(void){ellapsed +=  ( clock() - start_time );};

	float time(void){return ((float) ellapsed / (float) CLOCKS_PER_SEC);};
};

/**********************************************
				CLASS: Pair
**********************************************/



class Pair {
public:
	int a, b;

	Pair(int x, int y)
	{
		if(x>y)std::swap(x,y);

		a = x;
		b = y;
	}

	bool operator<(const Pair& p0) const
	{
		return ( (a < p0.a) || ( (a == p0.a) && (b < p0.b) ) );
	}


	bool operator==(const Pair p0) const
	{
		return (a == p0.a) && (b == p0.b);
	}

};


class HashTab {

private:
	int tab[MESH_HASH_SIZE][2];

public:
	void clear(void){for(int i = 0; i < MESH_HASH_SIZE; i++)tab[i][0] = tab[i][1] = -1;};

	void insert(int ei, int val){
		tab[ei % MESH_HASH_SIZE][0] = ei;
		tab[ei % MESH_HASH_SIZE][1] = val;
	}

	int lookup(int ei){
		if(tab[ei % MESH_HASH_SIZE][0] == ei)return tab[ei % MESH_HASH_SIZE][1];
		else return -1;
	}
};


typedef std::vector<int> vint;
typedef std::list<int> lint;

template<class T>
T pop(std::set<T>& s)
{
	std::set<T>::reverse_iterator ci = s.rbegin();
	T t = *ci;
	s.erase(t);
	return t;
}
	
template<class T>
T pop(std::vector<T>& s)
{
	T t = s.back();
	s.pop_back();
	return t;
}


}
}
}


#endif

