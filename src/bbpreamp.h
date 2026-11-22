#pragma once

#ifndef BB_BBPREAMP_H
#define BB_BBPREAMP_H

#include <lv2.h>

#define BBPLUGIN_URI "http://dataforte.stom-pi/plugins/bbpreamp"

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

#endif // BB_BBPREAMP_H
