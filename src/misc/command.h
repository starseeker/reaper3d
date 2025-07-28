
#ifndef REAPER_MISC_COMMAND_H
#define REAPER_MISC_COMMAND_H

#include <deque>
#include "misc/sequence.h"
#include "misc/stlhelper.h"
#include "misc/smartptr.h"

namespace reaper {
namespace misc {


#ifdef MSVC

struct Null { };

template<class R, class Arg = Null>
class uck
{
public:
	template<int arg>
	class CommandImpl {
	public:
	};

	template<>
	class CommandImpl<0> {
	public:
		virtual R operator()() = 0;
		virtual ~CommandImpl<0>() { }
	};
	template<>
	class CommandImpl<1>
	{
	public:
		virtual R operator()(Arg) = 0;
		virtual ~CommandImpl<1>() { }
	};
};



template<int arg, class Arg = Null>
struct ick
{
public:
	template<class R>
	class VCommand
	{
		uck<R, Arg>::CommandImpl<arg>* impl;
	public:
		VCommand() : impl(0) { }
		VCommand(uck<R, Arg>::CommandImpl<arg>* i) : impl(i) { }
		R operator()() { return (*impl)(); }
		R operator()(Arg a) { return (*impl)(a); }
		void del() { delete impl; }
	};
	template<>
	class VCommand<void>
	{
		uck<void, Arg>::CommandImpl<arg>* impl;
	public:
		VCommand<void>() : impl(0) { }
		VCommand<void>(uck<void, Arg>::CommandImpl<arg>* i) : impl(i) { }
		void operator()() { (*impl)(); }
		void operator()(Arg a) { (*impl)(a); }
		void del() { delete impl; }
	};
};


template<class R>
struct Command : public ick<0>::VCommand<R>
{
	Command() : ick<0>::VCommand<R>() { }
	Command(uck<R>::CommandImpl<0>* i)
	 : ick<0>::VCommand<R>(i)
	{ }
};


template<class S, class T, class R>
class MemVoidArgCaller : public uck<R>::CommandImpl<0>
{
	T* o;
	R (T::*mem)();
public:
	MemVoidArgCaller(S* t, R (T::*m)()) : o(t), mem(m) { }
	R operator()() { (o->*mem)(); }
};

template<class R>
struct foo1 { typedef ick<0>::VCommand<R> bar; };

template<class S, class T, class R>
Command<R> mk_cmd(S* t, R (T::*m)())
{
	return Command<R>(new MemVoidArgCaller<S, T, R>(t, m));
}


template<class S, class T, class A, class R>
class MemArgCaller : public uck<R>::CommandImpl<0>
{
	T* o;
	R (T::*mem)(A);
	A arg;
public:
	MemArgCaller(S* t, R (T::*m)(A), A a) : o(t), mem(m), arg(a) { }
	R operator()() { return (o->*mem)(arg); }
};

template<class S, class T, class A, class R>
foo1<R>::bar mk_cmd(S* t, R (T::*m)( A),  A a)
{
	return ick<0>::VCommand<R>(new MemArgCaller<S, T, A, R>(t, m, a));
}



template<class S, class T, class A>
class ack {
public:
	template<class R>
	class MemCaller : public uck<R, A>::CommandImpl<1>
	{
		T* o;
		R (T::*mem)(A);
	public:
		MemCaller(S* t, R (T::*m)(A)) : o(t), mem(m) { }
		R operator()(A arg) { return (o->*mem)(arg); }
	};
	template<>
	class MemCaller<void> : public uck<void, A>::CommandImpl<1> {
		T* o;
		void (T::*mem)(A);
	public:
		MemCaller<void>(S* t, void (T::*m)(A)) : o(t), mem(m) { }
		void operator()(A arg) { (o->*mem)(arg); }
	};

};

template<class A, class R>
struct foo2 { typedef ick<1, A>::VCommand<R> bar; };

template<class S, class T, class A, class R>
foo2<A, R>::bar mk_cmd(S* t, R (T::*m)(A a))
{
	return ick<1, A>::VCommand<R>(new ack<S, T, A>::MemCaller<R>(t, m));
}

template<class O, class R>
class ObjCaller : public uck<R>::CommandImpl<0>
{
	O o;
public:
	ObjCaller(O oo) : o(oo) { }
	void operator()() { o(); }
};

template<class R, class O>
Command<R> mk_cmd(O o)
{
	return Command<R>(new ObjCaller<O, R>(o));
}

#else


struct Null { };

template<int arg, class R, class Arg = Null>
class CommandImpl
{
public:
	virtual ~CommandImpl() { }
};

template<class R>
class CommandImpl<0, R>
{
public:
	virtual R operator()() = 0;
	virtual ~CommandImpl() { }
};

template<class R, class Arg>
class CommandImpl<1, R, Arg>
{
public:
	virtual R operator()(Arg) = 0;
	virtual ~CommandImpl() { }
};

template<class R, int arg = 0, class Arg = Null>
class Command
{
	misc::SmartPtr<CommandImpl<arg, R, Arg> > impl;
public:
	Command() : impl(0) { }
	Command(CommandImpl<arg, R, Arg>* i) : impl(i) { }
	Command(const Command& cmd) : impl(cmd.impl) { }
	R operator()() { return (*impl)(); }
	R operator()(Arg a) { return (*impl)(a); }
	void del() { delete impl; }
};

template<class O, class R>
class ObjCaller : public CommandImpl<0, R>
{
	O o;
public:
	ObjCaller(O oo) : o(oo) { }
	ObjCaller(const ObjCaller& oc) : o(oc.o) { }
	ObjCaller& operator=(const ObjCaller& oc)
	{
		o = oc.o;
		return *this;
	}
	R operator()() { return o(); }
};

template<class R, class O>
Command<R, 0> mk_cmd(const O& o)
{
	return Command<R, 0>(new ObjCaller<O, R>(o));
}


template<class S, class T, class R>
class MemVoidArgCaller : public CommandImpl<0, R>
{
	T* o;
	R (T::*mem)();
public:
	MemVoidArgCaller(S* t, R (T::*m)()) : o(t), mem(m) { }
	R operator()() { return (o->*mem)(); }
};

template<class S, class T, class R>
Command<R, 0> mk_cmd(S* t, R (T::*m)())
{
	return Command<R, 0>(new MemVoidArgCaller<S, T, R>(t, m));
}


template<class S, class T, class A, class R>
class MemArgCaller : public CommandImpl<0, R>
{
	T* o;
	R (T::*mem)(A);
	A arg;
public:
	MemArgCaller(S* t, R (T::*m)(A), A a) : o(t), mem(m), arg(a) { }
	R operator()() { return (o->*mem)(arg); }
};

template<class S, class T, class A, class R>
Command<R, 0> mk_cmd(S* t, R (T::*m)(A), A a)
{
	return Command<R, 0>(new MemArgCaller<S, T, A, R>(t, m, a));
}

template<class S, class T, class A, class R>
Command<R, 0> mk_cmd(S* t, R (T::*m)(const A&), A a)
{
	return Command<R, 0>(new MemArgCaller<S, T, const A&, R>(t, m, a));
}

template<class S, class T, class A, class R>
class MemCaller : public CommandImpl<1, R, A>
{
	T* o;
	R (T::*mem)(A);
public:
	MemCaller(S* t, R (T::*m)(A)) : o(t), mem(m) { }
	R operator()(A arg) { return (o->*mem)(arg); }
};

template<class S, class T, class A, class R>
Command<R, 1, A> mk_cmd(S* t, R (T::*m)(A a))
{
	return Command<R, 1, A>(new MemCaller<S, T, A, R>(t, m));
}


#endif


}
}

#endif


