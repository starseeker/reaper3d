
/* $Id: exceptions.h,v 1.9 2001/11/26 02:20:08 peter Exp $ */


#ifndef REAPER_HW_EXCEPTIONS_H
#define REAPER_HW_EXCEPTIONS_H

#include "main/exceptions.h"

namespace reaper
{
namespace hw
{   

class hw_fatal_error_t : public fatal_error_base { };
typedef error_tmpl<hw_fatal_error_t> hw_fatal_error;

class hw_error_t : public error_base { };
typedef error_tmpl<hw_error_t> hw_error;


}
}

#endif

