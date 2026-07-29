/*
 * $Author: pstrand $
 * $Date: 2002/04/16 19:58:41 $
 * $Revision: 1.12 $
 */


#ifndef REAPER_HW_SND_MP3_H
#define REAPER_HW_SND_MP3_H

#include <iosfwd>

#include "hw/snd_types.h"

namespace reaper
{
namespace hw
{
namespace snd
{

class Mp3Source;

class Mp3Decoder : public AudioDecoder
{
	std::unique_ptr<res::res_stream> rs;
	std::unique_ptr<Mp3Source> src;
public:
	Mp3Decoder();
	~Mp3Decoder();
	bool init(std::unique_ptr<res::res_stream>);
	AudioSourcePtr get();
};


}
}
}

#endif
