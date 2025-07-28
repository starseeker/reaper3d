#ifndef REAPER_MISC_CREATOR_H
#define REAPER_MISC_CREATOR_H

namespace reaper
{
namespace misc
{

template<class Base>
class CreateBase {
public:
	virtual Base* create() = 0;
};

template<class Base, class Derived>
class Creator : public CreateBase<Base> {
public:
	virtual Base* create() { return new Derived(); }
};

}
}

#endif

/*
 * $Author: peter $
 * $Date: 2001/08/06 12:16:30 $
 * $Log: creator.h,v $
 * Revision 1.3  2001/08/06 12:16:30  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.2.4.1  2001/08/03 13:44:05  peter
 * pragma once obsolete...
 *
 * Revision 1.2  2001/07/06 01:47:25  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.1  2001/03/22 02:09:12  macke
 * no message
 *
 */

/*
class B {
public:
	virtual void moj() = 0;
};

class C1 : public B {
public:
	virtual void moj() { std::cout << "C1\n"; }
};

class C2 : public B {
public:
	virtual void moj() { std::cout << "C2\n"; }
};

int main() {
	std::map<std::string, CreateBase<B>*> foo;
	
	foo["bar"] = new Creator<B,C1>();
	foo["baz"] = new Creator<B,C2>();

	B* c1 = foo["bar"]->create();
	B* c2 = foo["baz"]->create();

	c1->moj();
	c2->moj();
	
}
*/

