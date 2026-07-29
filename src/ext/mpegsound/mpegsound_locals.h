/* MPEG Sound library

   (C) 1997 by Jung woo-jae */

// Mpegsound_locals.h
// It is used for compiling library

#ifndef _L__SOUND_LOCALS__
#define _L__SOUND_LOCALS__

namespace mpegsound {

// Inline functions
inline int Mpegtoraw::getbyte(void)
{
  return getbits(8);
};

inline int Mpegtoraw::getbits9(int bits)
{
  return getbits(bits);
};

inline int Mpegtoraw::getbits8(void)
{
  return getbits(8);
};

inline int Mpegtoraw::getbit(void)
{
  return getbits(1);
};

}
#endif
