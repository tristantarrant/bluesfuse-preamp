#pragma once

#ifndef BF_BLUESFUSE_H
#define BF_BLUESFUSE_H


#define BFPLUGIN_URI "http://dataforte.net/plugins/bluesfuse"

typedef enum
{
  GAIN,
  VOLUME,
  TREBLE,
  BASS,
  MID,
  COMP,
  MODEL,
  EFFECTS_OUTPUT,
  EFFECTS_INPUT,
} PortIndex;

#endif // BF_BLUESFUSE_H
