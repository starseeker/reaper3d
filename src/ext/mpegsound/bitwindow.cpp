/* MPEG Sound library

   (C) 1997 by Jung woo-jae */

// Bitwindow.cc
// It's bit reservior for MPEG layer 3

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mpegsound.h"

#include <algorithm>
#include <cstdint>

namespace mpegsound {

int Mpegbitwindow::getbits(int bits)
{
  std::uint32_t value = 0;
  while (bits > 0)
  {
    const int bit_offset = bitindex & 7;
    const int count = std::min(bits, 8 - bit_offset);
    const auto byte = static_cast<std::uint8_t>(buffer[bitindex >> 3]);
    const auto mask = (1U << count) - 1U;
    value = (value << count) |
            ((byte >> (8 - bit_offset - count)) & mask);
    bitindex += count;
    bits -= count;
  }
  return static_cast<int>(value);
}

}
