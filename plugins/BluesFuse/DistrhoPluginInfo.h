/*
 * BluesFuse Preamp - Xotic BB Preamp (1.5 / AT / Bass / Comp / MB) emulation
 * Copyright (C) 2026 Dataforte
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND   "Dataforte"
#define DISTRHO_PLUGIN_NAME    "BluesFuse Preamp"
#define DISTRHO_PLUGIN_URI     "http://dataforte.stom-pi/plugins/bluesfuse"
#define DISTRHO_PLUGIN_CLAP_ID "net.dataforte.bluesfuse"

#define DISTRHO_PLUGIN_BRAND_ID  Dfte
#define DISTRHO_PLUGIN_UNIQUE_ID bFus

#define DISTRHO_PLUGIN_HAS_UI          1
#define DISTRHO_PLUGIN_IS_RT_SAFE      1
#define DISTRHO_PLUGIN_NUM_INPUTS      1
#define DISTRHO_PLUGIN_NUM_OUTPUTS     1
#define DISTRHO_PLUGIN_WANT_PROGRAMS   1

// static modgui shipped in modgui/ (rendered from Artwork.hpp), not DPF's
// generated one; this only adds the manifest reference to modgui.ttl
#define DISTRHO_PLUGIN_USES_MODGUI        1
#define DISTRHO_PLUGIN_USES_CUSTOM_MODGUI 1

#define DISTRHO_PLUGIN_LV2_CATEGORY    "lv2:DistortionPlugin"
#define DISTRHO_PLUGIN_VST3_CATEGORIES "Fx|Distortion|Mono"
#define DISTRHO_PLUGIN_CLAP_FEATURES   "audio-effect", "distortion", "mono"

#define DISTRHO_UI_USE_CAIRO       1
#define DISTRHO_UI_USER_RESIZABLE  1
#define DISTRHO_UI_DEFAULT_WIDTH   240
#define DISTRHO_UI_DEFAULT_HEIGHT  450

// Parameter order matches PortIndex in src/bluesfuse.h (the faust-
// generated DSP binds its controls by that enum); bypass is ours.
enum Parameters {
    kParamGain,
    kParamVolume,
    kParamTreble,
    kParamBass,
    kParamMid,
    kParamComp,
    kParamModel,
    kParamBypass,
    kParamCount
};

#endif // DISTRHO_PLUGIN_INFO_H_INCLUDED
