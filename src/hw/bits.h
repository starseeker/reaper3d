#ifndef REAPER_HW_BITS_H
#define REAPER_HW_BITS_H

#include <cstdint>

namespace reaper
{
namespace hw
{
namespace lowlevel
{

using uint32 = std::uint32_t;
using uint16 = std::uint16_t;
using uint8 = std::uint8_t;

using int32 = std::int32_t;
using int16 = std::int16_t;
using int8 = std::int8_t;


// convert to little endian

#if defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

inline uint32 to_le(uint32& i)
{
	i = ((i & 0xff) << 24) | ((i & 0xff00) << 8)
	  | ((i & 0xff0000) >> 8) | ((i & 0xff000000) >> 24);
	return i;
}

inline uint16 to_le(uint16& i)
{
	i = ((i & 0xff) << 8) | (i >> 8);
	return i;
}

inline int32 to_le(int32& i)
{
	i = ((i & 0xff) << 24) | ((i & 0xff00) << 8)
	  | ((i & 0xff0000) >> 8) | ((i & 0xff000000) >> 24);
	return i;
}

inline int16 to_le(int16& i)
{
	i = ((i & 0xff) << 8) | (i >> 8);
	return i;
}

#else

inline uint32 to_le(uint32 i) { return i; }
inline uint16 to_le(uint16 i) { return i; }
inline int32 to_le(int32 i) { return i; }
inline int16 to_le(int16 i) { return i; }

#endif


}
}
}

#endif

/*
 * $Author: peter $
 * $Date: 2001/08/06 12:16:18 $
 * $Log: bits.h,v $
 * Revision 1.4  2001/08/06 12:16:18  peter
 * MegaMerge (se strandy_test-grenen för diffar...)
 *
 * Revision 1.3.4.1  2001/08/03 13:43:56  peter
 * pragma once obsolete...
 *
 * Revision 1.3  2001/07/06 01:47:17  macke
 * Refptrfix/headerfilsstäd/objekt-skapande/mm
 *
 * Revision 1.2  2001/03/21 10:27:52  peter
 * *** empty log message ***
 *
 * Revision 1.1  2001/03/15 00:31:31  peter
 * endianness...
 *
 */

