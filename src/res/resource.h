
#ifndef REAPER_RES_RESOURCE_H
#define REAPER_RES_RESOURCE_H

#include <string>
#include <map>
#include <typeinfo>
#include <memory>
#include <deque>
#include "main/exceptions.h"
#include "misc/smartptr.h"
#include "misc/base.h"
#include "misc/free.h"
#include "misc/sequence.h"
#include "res/res.h"
#include <stdio.h>

namespace reaper {
namespace res {

typedef std::string Ident;
typedef const Ident& IdentRef;


class Managed
{
public:
	Managed(IdentRef);
	virtual ~Managed();

	virtual int flush() = 0;
	virtual void fetch(IdentRef) { }
	virtual void save(IdentRef) { }
};

class Mgr
{
	std::map<Ident, Managed*> nodes;
public:
	void flush();
	void add(IdentRef, Managed*);
	void del(Managed*);
	void clear();

	void prefetch(const Ident&);
};

Mgr& mgr();


struct res_error_t : public error_base { };
typedef error_tmpl<res_error_t> res_error;

template<class T> class NodeConfig;

template<class T>
struct tp {
	typedef misc::SmartPtr<T> ptr;
	typedef NodeConfig<T>* cfg;
	typedef T& ref;
};



template<class T>
class NodeConfig
{
	typename tp<T>::cfg par;
protected:
	typename tp<T>::cfg parent() { return par; }
public:
	NodeConfig() : par(0) { }
	virtual typename tp<T>::cfg set_parent(typename tp<T>::cfg p)
	{
		typename tp<T>::cfg old = par;
		par = p;
		return old;
	}
	typedef typename tp<T>::ptr Ptr;

	virtual Ptr create(IdentRef) = 0;
	virtual void save(IdentRef, const T&) { }

	virtual ~NodeConfig() {
		delete par;
	}
};



template<class T>
void create_error(Ident id)
{
	throw res_error(std::string("Don't know how to create ") +
	                            id + " :: " + typeid(T).name());
}

class Res { };


template<class T>
class ResNode : misc::NoCopy, public Managed
{
	typename tp<T>::cfg config;
	typedef std::map<Ident, typename tp<T>::ptr> Ents;
	typedef typename Ents::iterator Ents_i;
	Ents ents;

public:
	ResNode(IdentRef id) : Managed(id), config(0) { }

	~ResNode() {
		flush();
		delete config;
	}

	typename tp<T>::ptr lookup(IdentRef id) {
		Ents_i i = ents.find(id);
		return (i != ents.end()) ? i->second : 0;
	}

	void save(IdentRef id) {
		typename tp<T>::ptr p = lookup(id);
		if (p != 0)
			config->save(id, *p);
	}

	typename tp<T>::ptr get(IdentRef id) {
		typename tp<T>::ptr p = config->create(id);
		if (p == 0)
			create_error<T>(id + " (create failed)");
		return p;
	}

	typename tp<T>::ptr find_ptr(IdentRef id) {
		typename tp<T>::ptr p = lookup(id);
		if (p != 0)
			return p;
		p = get(id);
		ents[id] = p;
		return p;
	}

	int flush() {
		ents.clear();
		return 42;
	}

	void push_config(typename tp<T>::cfg p) {
		p->set_parent(config);
		config = p;
	}

	void pop_config() {
		typename tp<T>::cfg p = config;
		config = config->set_parent(0);
		delete p;
	}
};

Managed* get_res_ptr(const std::string& id);
void reinit_ptr(const std::string& id, Managed* p);

template<class T>
ResNode<T>* inst()
{
	return dynamic_cast<ResNode<T>*>(get_res_ptr(typeid(T).name()));
}

template<class T>
typename tp<T>::ref resource(const Ident& ident)
{
	return *inst<T>()->find_ptr(ident);
}

template<class T>
typename tp<T>::ptr resource_ptr(const Ident& a)
{
	return inst<T>()->find_ptr(a);
}


template<class T>
void push_config(NodeConfig<T>* cr, IdentRef nm = "")
{
	ResNode<T>* n = inst<T>();
	if (!nm.empty()) {
		reinit_ptr(typeid(T).name(), new ResNode<T>(nm));
	}
	inst<T>()->push_config(cr);
}

template<class T>
void pop_config()
{
	inst<T>()->pop_config();
}


template<class T>
void flush(const Ident& dir)
{
	inst<T>()->flush(dir);
}

template<class T>
void save(const Ident& path)
{
	inst<T>()->save(path);
}


}
}

#endif

