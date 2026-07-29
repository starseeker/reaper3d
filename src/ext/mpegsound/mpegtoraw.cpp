/* MPEG Sound library

   (C) 1997 by Jung woo-jae */

// Mpegtoraw.cc
// Server which get mpeg format and put raw format.

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <algorithm>
#include <math.h>
#include <cstdint>
#include <stdlib.h>
//#include <unistd.h>

#include "mpegsound.h"
#include "mpegsound_locals.h"

namespace mpegsound {

Mpegtoraw::Mpegtoraw(Soundinputstream *loader,Soundplayer *player)
{
  __errorcode=SOUND_ERROR_OK;

  forcetomonoflag=false;
  downfrequency=0;

  this->loader=loader;
  this->player=player;
}

Mpegtoraw::~Mpegtoraw() = default;

int Mpegtoraw::getbits( int bits )
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

void Mpegtoraw::setforcetomono(bool flag)
{
  forcetomonoflag=flag;
}

void Mpegtoraw::setdownfrequency(int value)
{
  downfrequency=0;
  if(value)downfrequency=1;
}

bool Mpegtoraw::getforcetomono(void)
{
  return forcetomonoflag;
}

int Mpegtoraw::getdownfrequency(void)
{
  return downfrequency;
}

int  Mpegtoraw::getpcmperframe(void)
{
  int s;

  s=32;
  if(layer==3)
  {
    s*=18;
    if(version==0)s*=2;
  }
  else
  {
    s*=SCALEBLOCK;
    if(layer==2)s*=3;
  }

  return s;
}

inline void Mpegtoraw::flushrawdata(void)
{
  player->putblock((char *)rawdata,rawdataoffset<<1);
  currentframe++;
  rawdataoffset=0;
};

// Convert mpeg to raw
// Mpeg headder class
void Mpegtoraw::initialize()
{
  int i;

  scalefactor=SCALE;
  calcbufferoffset=15;
  currentcalcbuffer=0;

  for(i=CALCBUFFERSIZE-1;i>=0;i--)
    calcbufferL[0][i]=calcbufferL[1][i]=
    calcbufferR[0][i]=calcbufferR[1][i]=0.0;

  layer3initialize();

  currentframe=decodeframe=0;
  if(loadheader())
  {
    totalframe=(loader->getsize()+framesize-1)/framesize;
    loader->setposition(0);
  }
  else totalframe=0;


  frameoffsets.assign(std::max(totalframe, 0), 0);
};

void Mpegtoraw::setframe(int framenumber)
{
  int pos=0;

  if(frameoffsets.empty())return;
  if(framenumber==0)pos=frameoffsets[0];
  else
  {
    if(framenumber>=totalframe)framenumber=totalframe-1;
    pos=frameoffsets[framenumber];
    if(pos==0)
    {
      int i;

      for(i=framenumber-1;i>0;i--)
	if(frameoffsets[i]!=0)break;

      loader->setposition(frameoffsets[i]);

      while(i<framenumber)
      {
	loadheader();
	i++;
	frameoffsets[i]=loader->getposition();
      }
      pos=frameoffsets[framenumber];
    }
  }

  clearbuffer();
  loader->setposition(pos);
  decodeframe=currentframe=framenumber;
}

void Mpegtoraw::clearbuffer(void)
{
  player->abort();
  player->resetsoundtype();
}

bool Mpegtoraw::loadheader(void)
{
  int c;
  bool flag;

  sync();

// Synchronize
  flag=false;
  do
  {

    if((c=loader->getbytedirect())<0)break;

    if(c==0xff) {
      while(!flag)
      {
	if((c=loader->getbytedirect())<0)
	{
	  flag=true;
	  break;
	}
	if((c&0xf0)==0xf0)
	{
	  flag=true;
	  break;
	}
	else if(c!=0xff)break;
      }
    }
    else if (c=='I') { // possible ID3v2 tag
        char buf[10];
        int c2,c3;
        int length;

        if((c2=loader->getbytedirect())<0) break;
        if (c2=='D') {
            if((c3=loader->getbytedirect())<0) break;
            if(c3=='3') {
                  // OK, found ID3v2 tag.
                if (!loader->_readbuffer(buf,7)) break;
                
                  // Compute number of bytes to skip
                length=((buf[3]&0x7f)<<21) + ((buf[4]&0x7f)<<14) +
                    ((buf[5]&0x7f)<<7) + (buf[6]&0x7f);
                
       //printf("Found ID3v2 tag; skipping %d bytes\n",length);
                while (length-->0) {
                    if(loader->getbytedirect()<0) {
                        break;
                    }
                }
            }
        }
    }
    //else {
    //    printf("garbage character: %02x\n",c);
    //}
  }while(!flag);

  if(c<0)return seterrorcode(SOUND_ERROR_FINISH);



// Analyzing
  c&=0xf;
  protection=c&1;
  layer=4-((c>>1)&3);
  version=(_mpegversion)((c>>3)^1);
  if(layer<1 || layer>3)
    return seterrorcode(SOUND_ERROR_BAD);

  c=loader->getbytedirect();
  if(c<0)
    return seterrorcode(SOUND_ERROR_FINISH);
  c>>=1;
  padding=(c&1);             c>>=1;
  const int frequency_index=c&3;
  if(frequency_index==3)
    return seterrorcode(SOUND_ERROR_BAD);
  frequency=(_frequency)frequency_index; c>>=2;
  bitrateindex=(int)c;
  if(bitrateindex==0 || bitrateindex==15)
    return seterrorcode(SOUND_ERROR_BAD);

  c=loader->getbytedirect();
  if(c<0)
    return seterrorcode(SOUND_ERROR_FINISH);
  c=((unsigned int)c)>>4;
  extendedmode=c&3;
  mode=(_mode)(c>>2);


// Making information
  inputstereo= (mode==single)?0:1;
  if(forcetomonoflag)outputstereo=0; else outputstereo=inputstereo;

  /*  if(layer==2)
    if((bitrateindex>=1 && bitrateindex<=3) || (bitrateindex==5)) {
      if(inputstereo)return seterrorcode(SOUND_ERROR_BAD); }
    else if(bitrateindex==11 && mode==single)
    return seterrorcode(SOUND_ERROR_BAD); */

  channelbitrate=bitrateindex;
  if(inputstereo)
    if(channelbitrate==4)channelbitrate=1;
    else channelbitrate-=4;

  if(channelbitrate==1 || channelbitrate==2)tableindex=0; else tableindex=1;

  if(layer==1)subbandnumber=MAXSUBBAND;
  else
  {
    if(!tableindex)
      if(frequency==frequency32000)subbandnumber=12; else subbandnumber=8;
    else if(frequency==frequency48000||
	    (channelbitrate>=3 && channelbitrate<=5))
      subbandnumber=27;
    else subbandnumber=30;
  }

  if(mode==single)stereobound=0;
  else if(mode==joint)stereobound=(extendedmode+1)<<2;
  else stereobound=subbandnumber;

  if(stereobound>subbandnumber)stereobound=subbandnumber;

  // framesize & slots
  if(layer==1)
  {
    framesize=(12000*bitrate[version][0][bitrateindex])/
              frequencies[version][frequency];
    if(frequency==frequency44100 && padding)framesize++;
    framesize<<=2;
  }
  else
  {
    framesize=(144000*bitrate[version][layer-1][bitrateindex])/
      (frequencies[version][frequency]<<version);
    if(padding)framesize++;
    if(layer==3)
    {
      if(version)
	layer3slots=framesize-((mode==single)?9:17)
	                     -(protection?0:2)
	                     -4;
      else
	layer3slots=framesize-((mode==single)?17:32)
	                     -(protection?0:2)
	                     -4;
    }
  }

  if(framesize<4 || framesize>static_cast<int>(sizeof(buffer))+4)
    return seterrorcode(SOUND_ERROR_BAD);
  if(!fillbuffer(framesize-4))
    return seterrorcode(SOUND_ERROR_FILEREADFAIL);

  if(!protection)
  {
    getbyte();                      // CRC, Not check!!
    getbyte();
  }


  return true;
}

// Convert mpeg to raw
bool Mpegtoraw::run(int frames)
{
  clearrawdata();
  if(frames<0)lastfrequency=0;

  for(;frames;frames--)
  {
    if(totalframe>0)
    {
      if(decodeframe<totalframe)
	frameoffsets[decodeframe]=loader->getposition();
    }

    if(loader->eof())
    {
      seterrorcode(SOUND_ERROR_FINISH);
      break;
    }
    if(loadheader()==false)break;

    if(frequency!=lastfrequency)
    {
      if(lastfrequency>0)seterrorcode(SOUND_ERROR_BAD);
      lastfrequency=frequency;
    }
    if(frames<0)
    {
      frames=-frames;
      player->setsoundtype(outputstereo,16,
			   frequencies[version][frequency]>>downfrequency);
    }

    decodeframe++;

    if     (layer==3)extractlayer3();
    else if(layer==2)extractlayer2();
    else if(layer==1)extractlayer1();

    flushrawdata();
    if(player->geterrorcode())seterrorcode(geterrorcode());
  }

  return (geterrorcode()==SOUND_ERROR_OK);
}

}
