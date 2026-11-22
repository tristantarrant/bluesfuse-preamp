#pragma once

#ifndef BF_BLUESFUSE_H
#define BF_BLUESFUSE_H

#include <lv2.h>

#define BFPLUGIN_URI "http://dataforte.stom-pi/plugins/bluesfuse"

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
