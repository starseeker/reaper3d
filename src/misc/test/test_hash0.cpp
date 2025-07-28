#include "hw/compat.h"
#include "misc/test_main.h"
#include "misc/sequence.h"

#include "misc/hash.h"
#include "hw/profile.h"

//#include <ext/hash_map>
#include <map>
#include <iostream>
#include <string>

#include <stdio.h>

using namespace reaper::misc;
using namespace std;

void insert(map<int,int>& m, int k, int v)
{
	m.insert(make_pair(k, v));
}

/*
void insert(std::hash_map<int,int>& m, int k, int v)
{
	m.insert(make_pair(k, v));
}
*/

void insert(misc::hash_map<int,int>& m, int k, int v)
{
	m.insert(k, v);
}

template<class T>
int test()
{
	int N = 1000;
	int r = 0;
	T map;
	for (int n = 0; n < N; ++n) {
		if ((n % 10) == 0) {
			for (int i = 0; i < 100; i+=1) {
				for (int j = 0; j < 100; j+=3) {
					insert(map, i*j, i);
				}
			}
		}
		for (int i = 0; i < 100; i+=1) {
			for (int j = 0; j < 100; j+=3) {
				r += map[i*j];
			}
		}
	}
	return r;
}


int test_main()
{
	misc::hash_map<int,int> hm;
	hm.insert(1, 1);
	hm.insert(2, 2);
	hm.insert(3, 3);
	hm.insert(4, 4);
	hm.insert(5, 5);
	hm[6] = 6;
	hm[7] = 7;
	printf("%d %d %d %d | %d %d %d %d\n", hm[0], hm[1], hm[2], hm[3],
			                      hm[4], hm[5], hm[6], hm[7]);

	for (int i = 0; i < 100000; ++i) {
		hm[i] = i*i;
	}

	
	misc::hash_map<string, string> hmap;
	hmap.insert("hello", "world");
	hmap.insert("hello2", "world2");
	hmap.insert("foo", "bar");
	hmap.insert("Foo", "baz");
	hmap.erase("Foo");
	hmap.elem("Foo");
	hmap["bar"] = "frob";
	derr << hmap.size() << ' '
	     << hmap["hello"] << ' '
	     << hmap["Hello"] << ' '
	     << hmap.elem("foo") << ' '
	     << hmap.elem("Foo") << '\n';

	misc::hash_map<string,string>::iterator c, e = hmap.end();
	for (c = hmap.begin(); c != e; ++c) {
		derr << c->first << ": " << c->second << '\n';
	}
/*
	{ hw::time::Profile p_map("map     ");
		printf("res: %d\n", test<map<int,int> >());;
	}
	{ hw::time::Profile p_map("hash_map");
		printf("res: %d\n", test<std::hash_map<int,int> >());
	}
	{ hw::time::Profile p_map("hash    ");
		printf("res: %d\n", test<misc::hash_map<int,int> >());
	}
*/
	return 0;
}

