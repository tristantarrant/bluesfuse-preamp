declare id "bluesfuse";
declare name "BluesFuse Preamp";
declare category "Distortion";
declare author "Dataforte";
declare license "GPL-2.0-or-later";
declare description "Emulation of the Xotic BB Preamp overdrive and its variants (1.5, AT, Bass, Custom Shop Comp and MB), after the Aion FX trace schematics (2021): BJT input buffer, Tube Screamer style diode-feedback clipper, TS tone stage, variable-gain stage (dual-gang VOLUME), two-band Baxandall EQ and BJT output buffer.";

import("stdfaust.lib");
tf = library("bluesfuse_tf.lib");

// ====================================================================
//  Circuit constants
// ====================================================================

// Signal level mapping: 1.0 full scale <-> 1 V at the pedal input and
// output (unity in/out, so the pedal's own gain structure is kept).
VIN  = 1.0;

// JRC4558 output swing around the 4.5 V bias on a 9 V supply.
VR   = 3.6;

// 1N914 clipping diodes.
IS   = 2.52e-9;               // saturation current (A)
NVT  = 1.752*0.02585;         // emission coefficient * thermal voltage (V)

// ====================================================================
//  Controls
// ====================================================================

gain   = hslider("gain[name:GAIN]",     5.0, 0.0, 10.0, 0.1) : si.smooth(0.999);
volume = hslider("volume[name:VOLUME]", 5.0, 0.0, 10.0, 0.1) : si.smooth(0.999);
treble = hslider("treble[name:TREBLE]", 5.0, 0.0, 10.0, 0.1) : si.smooth(0.999);
bass   = hslider("bass[name:BASS]",     5.0, 0.0, 10.0, 0.1) : si.smooth(0.999);

// Custom Shop MB: restores the Tube Screamer tone pot (20k W taper).
// 2.0 (9 o'clock) = the 1k/18k the stock BB hard-wires in its place.
mid    = hslider("mid[name:MID]",       2.0, 0.0, 10.0, 0.1) : si.smooth(0.999);

// Custom Shop Comp: clipping diode switch.
//   0 = one diode per direction (BB-AT / pre-2007, "more compression")
//   1 = diodes lifted ("no compression", op-amp rail clipping only)
//   2 = two diodes per direction (BB 1.5, the current circuit)
comp   = nentry("comp[name:COMP]", 2, 0, 2, 1);

// Component set: 0 = guitar BB (1.5/AT/Comp/MB), 1 = Bass BB.
model  = nentry("model[name:MODEL]", 0, 0, 1, 1);

// Pot tapers. B = linear. W = symmetric S-curve: 20% rotation gives
// 5% resistance (Aion: 1k/18k on a W pot sits at 9 o'clock).
K_W = log(0.1)/log(0.4);
wtaper(r) = select2(r > 0.5, 0.5*pow(2*r, K_W), 1 - 0.5*pow(2*(1-r), K_W));

pg = gain/10;
pv = volume/10;
pt = treble/10;
pb = bass/10;
pw = mid/10 : wtaper;

// ====================================================================
//  Op-amp rail saturation
// ====================================================================
// Smooth limiter towards +-VR; transparent at normal levels (-0.06 dB
// at half swing), hard-ish knee at the rails.
rail(x) = x / pow(1 + pow(abs(x)/VR, 4), 0.25);

// ====================================================================
//  Input buffer (Q1 emitter follower)
// ====================================================================
// C1 22n into R1 + R2 (470k bias), C2 into R4 10k. The Bass BB uses
// a 100n C2, which tightens the low end going into the clipper.
inbuf = fi.highpass(1, 1/(2*ma.PI*471e3*22e-9))
      : fi.highpass(1, 1/(2*ma.PI*10e3*select2(model, 1e-6, 100e-9)));

// ====================================================================
//  IC1A: Tube Screamer clipper (diodes in the feedback loop)
// ====================================================================
// Non-inverting stage: the ground leg (R6 + C4) sets a current
// Vin*Yg(s) that flows through the feedback network Rf || C3 || diodes,
// with Rf = GAIN + R5. tf.clipfb gives the linear feedback voltage u;
// the diode strings (nd per direction) clip it by solving
//     v + Rf * 2*Is*sinh(v/(nd*nVt)) = u
// with Newton iterations, started from an upper bound so that they
// converge monotonically (4 iterations: < 10 uV error). The output is
// Vin + v, the classic TS "clean + clipped" sum.
clipper(x) = rail(x + vf) with {
    rf   = pg*1e6 + 1 + select2(model, 10e3, 4.7e3);
    u    = x : tf.clipfb(model, pg);
    nd   = select2(comp == 0, 2, 1);
    vd   = nd*NVT;
    v0   = ma.signum(u) * min(abs(u), vd*ma.asinh(abs(u)/(2*IS*rf)));
    step(v) = v - (v + rf*IS*(e - 1/e) - u)/(1 + rf*IS*(e + 1/e)/vd)
        with { e = exp(v/vd); };
    vf   = select2(comp == 1, v0 : seq(i, 4, step), u);
};

// ====================================================================
//  Output buffer (Q2 emitter follower)
// ====================================================================
// VOLUME gang A (100kB divider), C15 100n into the 470k bias.
outbuf = *(pv) : fi.highpass(1, 1/(2*ma.PI*470e3*100e-9));

// ====================================================================
//  Process
// ====================================================================
//  in -> Q1 -> IC1A clipper -> IC1B TS tone -> IC2A gain (VOLUME B)
//     -> IC2B Baxandall (BASS/TREBLE) -> VOLUME A -> Q2 -> out
process = *(VIN) : inbuf : clipper
        : tf.tone(model, pw)        : rail
        : tf.gainstage(model, pv)   : rail
        : tf.baxandall(model, pb, pt) : rail
        : outbuf : /(VIN);
