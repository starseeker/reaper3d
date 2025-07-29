

#include <vector>
#include <string>
#include <map>

namespace reaper {
namespace misc {

/* A very na�ve and simplistic hashtable, improve! */


/* Default hasher, just xor the representation,
 * if you know your type, you can and should do better..
 */

template<class T>
struct Hash
{
	size_t operator()(const T& t) const {
		const char* p = reinterpret_cast<const char*>(&t);
		size_t r = 0;
		for (size_t i = 0; i < sizeof(T); ++i)
			r = (r<<1)^*p++;
		return r;
	}
};



template<>
struct Hash<std::string>
{
	size_t operator()(const std::string& s) const {
		size_t n = 0;
		for (size_t i = 0; i < s.size(); ++i) 
			n = (n<<1) ^ s[i];
		return n;
	}
};

template<>
struct Hash<int>
{
	size_t operator()(int i) const {
		return i;
	}
};

/* Note that the element type needs a default constructor,
 * it can be avoided by a slight redesign, if necessary..
 * Really big elements should not be stored, since they
 * are copied upon resize.
 *
 */

template<class K, class T, class H = Hash<K>, class EQ = std::equal_to<K> >
class hash_map
{
private:
	struct Node
	{
		Node* next;
		std::pair<K,T> data;
		bool used;

		Node() : next(0), used(false) { }
		Node(const std::pair<K,T>& p, Node* n = 0)
		 : next(n), data(p.first, p.second), used(true)
		{ }
		Node(const Node& n) : next(n.next), data(n.data.first, n.data.second), used(n.used) { }
		Node& operator=(const Node& n) {
			next = n.next;
			data.first = n.data.first;
			data.second = n.data.second;
			used = n.used;
			return *this;
		}
	};
	typedef std::vector<Node> Tbl;
	typedef Node* Iter;
	typedef const Node* CIter;
	Tbl tbl;
	H hash;
	EQ eq;
	size_t count;

public:
	class iterator : public std::iterator<std::forward_iterator_tag,
					      std::pair<K,T> > {
		Tbl* tbl;
		size_t idx;
		Iter iter;
		
		void find_next() {
			do {
				iter = iter->next;
				if (!iter) {
					++idx;
					if (idx == tbl->size()) {
						tbl = 0;
						idx = 0;
						iter = 0;
						return;
					}
					iter = &(*tbl)[idx];
				}
			} while (!iter->used);
		}
	public:
		typedef std::pair<K,T> value_type;

		iterator(Tbl* t) : tbl(t), idx(0), iter(&(*tbl)[0]) {
			if (!iter->used)
				find_next();
		}
		iterator() : tbl(0), idx(0), iter(0) { }

		iterator(const iterator& i)
		 : tbl(i.tbl), idx(i.idx), iter(i.iter)
		{ }

		iterator& operator=(const iterator& i) {
			tbl = i.tbl;
			idx = i.idx;
			iter = i.iter;
			return *this;
		}

		iterator& operator++() {
			find_next();
			return *this;
		}

		iterator operator++(int) {
			iterator i(*this);
			++(*this);
			return i;
		}
		
		value_type& operator*() { return iter->data; }
		const value_type& operator*() const { return iter->data; }

		value_type* operator->() { return &iter->data; }
		
		bool operator==(const iterator& i) const {
			return tbl == i.tbl && idx == i.idx && iter == i.iter;
		}

		bool operator!=(const iterator& i) const {
			return !(*this == i);
		}
	};
private:

	Iter find(const K& k) {
		return &tbl[hash(k) % tbl.size()];
	}
	CIter find(const K& k) const {
		return &tbl[hash(k) % tbl.size()];
	}

	void rehash() {
		Tbl old;
		old.swap(tbl);
		count = 0;
		tbl.resize(static_cast<int>(3.14*old.size()));
		for (size_t i = 0; i < old.size(); ++i) {
			Iter n = &old[i];
			if (n->used)
				insert(n->data);
			n = n->next;
			while (n) {
				if (n->used)
					insert(n->data);
				Iter p = n;
				n = n->next;
				delete p;
			}
		}
	}
public:
	hash_map(int init_size = 173)
	 : tbl(init_size), count(0)
	{ }
	~hash_map() {
		clear();
	}
	void insert(const K& k, const T& t) {
		insert(std::make_pair(k,t));
	}

	void insert(const std::pair<K,T>& p) {
		if (count > tbl.size()) {
			rehash();
		}
		Iter n = find(p.first);
		if (n->used)
			n->next = new Node(p, n->next);
		else
			*n = Node(p, n->next);
		++count;
	}
	T& operator[](const K& k) {
		Iter p = find(k);
		while (p && !eq(p->data.first, k))
			p = p->next;
		if (p && p->used) {
			return p->data.second;
		} else {
			insert(k, T());
			return (*this)[k];
		}
	}
	void erase(const K& k) {
		--count;
		Iter n = find(k);
		Iter p = 0;
		if (eq(n->data.first, k)) {
			if (n->next)
				*n = *n->next;
			else {
				n->data.second = T();
				n->used = false;
			}
			return;
		}
		while (n->next) {
			p = n;
			n = n->next;
			if (eq(n->data.first, k)) {
				p->next = n->next;
				delete n;
				return;
			}
		}
		++count;
	}
	size_t size() const {
		return count;
	}
	bool elem(const K& k) const {
		CIter n = find(k);
		while (n) {
			if (eq(n->data.first, k) && n->used)
				return true;
			n = n->next;
		}
		return false;
	}
	void clear() {
		size_t init_size = tbl.size();
		Tbl empty;
		tbl.swap(empty);
		tbl.resize(init_size);
		
		for (size_t i = 0; i < empty.size(); ++i) {
			Iter n = empty[i].next;
			while (n) {
				Iter t = n;
				n = n->next;
				delete t;
			}
		}

	}
	
	iterator begin() {
		return iterator(&tbl);
	}
	iterator end() {
		return iterator();
	}
};



}
}

