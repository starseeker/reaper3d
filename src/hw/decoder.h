/*
 * $Author: pstrand $
 * $Date: 2002/04/16 19:58:41 $
 * $Revision: 1.7 $
 */


#ifndef REAPER_HW_DECODER_H
#define REAPER_HW_DECODER_H

#include "res/res.h"

namespace reaper {
namespace hw {


template<class Source>
class Decoder
{
public:
	virtual bool init(res::res_stream*) = 0;
	virtual Source* get() = 0;
	virtual ~Decoder() { }
};


}
}


#endif

