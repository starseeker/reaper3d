
/* $Id: exceptions.h,v 1.19 2002/03/11 10:50:49 pstrand Exp $ */

#ifndef REAPER_MAIN_EXCEPTIONS_H
#define REAPER_MAIN_EXCEPTIONS_H

#include <exception>
#include <string>

namespace reaper
{

/// All reaper-exceptions should be derived from this.
class error_base
{
public:
	virtual const char* what() const = 0;
	virtual ~error_base() { }
};


/** Fatal exceptions, i.e. non-recoverable,
 *  but which may still be handled to enable
 * a controlled shutdown..
 */
class fatal_error_base : public error_base { };



/// Exception template to ease implementation of own exceptions.

template<class T>
class error_tmpl : public T
{
	std::string msg;
public:
	error_tmpl(const std::string& m)
	 : msg(m)
	{ }
	const char* what() const {
		return msg.c_str();
	}
};


#define define_reaper_exception(name) \
	class name##_t : public error_base { }; \
	typedef error_tmpl<name##_t> name;

define_reaper_exception(reaper_error);

}
    
#endif


