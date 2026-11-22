/*
 * BluesFuse Preamp - LV2 test harness
 *
 * Loads the plugin with the raw LV2 C API, exercises the DSP with a few
 * control/audio scenarios and checks the results against the expected
 * behaviour of the circuit.
 *
 * Usage: ./bf_test /path/to/bluesfuse.lv2/bluesfuse.so
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <dlfcn.h>

typedef struct _LV2_Descriptor LV2_Descriptor;
typedef void *LV2_Handle;

struct _LV2_Descriptor {
  const char *plugin_uri;
  LV2_Handle (*instantiate)(const LV2_Descriptor *, double, const char *,
                            const void *const *);
  void (*connect_port)(LV2_Handle, unsigned int, void *);
  void (*activate)(LV2_Handle);
  void (*run)(LV2_Handle, unsigned int);
  void (*deactivate)(LV2_Handle);
  void (*cleanup)(LV2_Handle);
  void *reserved;
};

enum { GAIN, VOLUME, TREBLE, BASS, MID, COMP, MODEL, NCTRL };
#define P_OUT 7
#define P_IN  8

#define BLOCK 1024

static int failures = 0;
static const LV2_Descriptor *desc;
static float ctrl[NCTRL];
static float in[BLOCK], out[BLOCK];

static void
check(int ok, const char *msg)
{
  printf("%s: %s\n", ok ? "PASS" : "FAIL", msg);
  if (!ok)
    ++failures;
}

static void
defaults(void)
{
  ctrl[GAIN] = 5; ctrl[VOLUME] = 5; ctrl[TREBLE] = 5; ctrl[BASS] = 5;
  ctrl[MID] = 2; ctrl[COMP] = 2; ctrl[MODEL] = 0;
}

static LV2_Handle
open_plugin(double sr)
{
  LV2_Handle h = desc->instantiate(desc, sr, NULL, NULL);
  if (!h)
    return NULL;
  for (int i = 0; i < NCTRL; ++i)
    desc->connect_port(h, i, &ctrl[i]);
  desc->connect_port(h, P_OUT, out);
  desc->connect_port(h, P_IN, in);
  desc->activate(h);
  return h;
}

static void
close_plugin(LV2_Handle h)
{
  desc->deactivate(h);
  desc->cleanup(h);
}

/* Run a sine of the given frequency and peak amplitude for ~1.5 s
   (control smoothing and filters settle), return the output rms over
   the last block; *finite reports NaN/Inf anywhere. */
static double
sine_rms(LV2_Handle h, double sr, double freq, double amp, int *finite)
{
  static long phase = 0;
  int nblocks = (int)(1.5 * sr / BLOCK) + 1;
  double acc = 0;
  *finite = 1;
  for (int b = 0; b < nblocks; ++b) {
    for (int i = 0; i < BLOCK; ++i, ++phase)
      in[i] = (float)(amp * sin(2.0 * M_PI * freq * phase / sr));
    desc->run(h, BLOCK);
    for (int i = 0; i < BLOCK; ++i)
      if (!isfinite(out[i]))
        *finite = 0;
  }
  for (int i = 0; i < BLOCK; ++i)
    acc += (double)out[i] * out[i];
  return sqrt(acc / BLOCK);
}

static double
db(double x)
{
  return 20.0 * log10(x + 1e-30);
}

int
main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "usage: %s plugin.so\n", argv[0]);
    return 1;
  }
  void *lib = dlopen(argv[1], RTLD_NOW);
  if (!lib) {
    fprintf(stderr, "cannot dlopen %s: %s\n", argv[1], dlerror());
    return 1;
  }
  const LV2_Descriptor *(*desc_fn)(unsigned int) =
    (const LV2_Descriptor * (*)(unsigned int)) dlsym(lib, "lv2_descriptor");
  desc = desc_fn ? desc_fn(0) : NULL;
  check(desc != NULL && desc->plugin_uri != NULL, "lv2_descriptor(0)");
  if (!desc)
    return 1;
  check(desc_fn(1) == NULL, "single plugin in the library");

  const double SR = 48000;
  int fin;
  double r, r1, r2;

  defaults();
  LV2_Handle h = open_plugin(SR);
  check(h != NULL, "instantiate @48k");
  if (!h)
    return 1;

  /* --- defaults: sane overdrive level -------------------------------- */
  r = sine_rms(h, SR, 440, 0.3, &fin);
  check(fin, "defaults: output finite");
  check(r > 0.05 && r < 3.0, "defaults: sensible output level");
  printf("      rms(defaults, 0.3 V 440 Hz) = %.3f\n", r);

  /* --- silence in -> silence out ------------------------------------- */
  r = sine_rms(h, SR, 440, 0.0, &fin);
  check(fin && r < 1e-6, "silent input gives silent output");

  /* --- VOLUME 0 mutes (gang A goes to ground) ------------------------ */
  ctrl[VOLUME] = 0;
  r = sine_rms(h, SR, 440, 0.3, &fin);
  check(fin && r < 1e-4, "VOLUME 0 mutes");
  defaults();

  /* --- Baxandall: BASS / TREBLE boost and cut (small signal, lift) --- */
  ctrl[GAIN] = 0; ctrl[COMP] = 1;
  double flat100 = sine_rms(h, SR, 100, 0.01, &fin);
  double flat8k = sine_rms(h, SR, 8000, 0.01, &fin);
  ctrl[BASS] = 10;
  r1 = sine_rms(h, SR, 100, 0.01, &fin);
  ctrl[BASS] = 0;
  r2 = sine_rms(h, SR, 100, 0.01, &fin);
  printf("      100 Hz: bass 10 %+.1f dB, bass 0 %+.1f dB\n",
         db(r1 / flat100), db(r2 / flat100));
  check(db(r1 / flat100) > 10 && db(r2 / flat100) < -10,
        "BASS boosts/cuts 100 Hz by > 10 dB");
  ctrl[BASS] = 5;
  ctrl[TREBLE] = 10;
  r1 = sine_rms(h, SR, 8000, 0.01, &fin);
  ctrl[TREBLE] = 0;
  r2 = sine_rms(h, SR, 8000, 0.01, &fin);
  printf("      8 kHz: treble 10 %+.1f dB, treble 0 %+.1f dB\n",
         db(r1 / flat8k), db(r2 / flat8k));
  check(db(r1 / flat8k) > 10 && db(r2 / flat8k) < -10,
        "TREBLE boosts/cuts 8 kHz by > 10 dB");
  defaults();

  /* --- COMP switch: clipping threshold order -------------------------- */
  /* High gain, low volume (keeps the later stages out of the rails): the
     clipped level must rank AT (1 diode) < 1.5 (2 diodes) < lift. */
  ctrl[GAIN] = 10; ctrl[VOLUME] = 2;
  double lvl[3];
  for (int c = 0; c < 3; ++c) {
    ctrl[COMP] = c;
    lvl[c] = sine_rms(h, SR, 440, 0.3, &fin);
    check(fin, c == 0 ? "COMP AT: finite" : c == 1 ? "COMP off: finite"
                                                   : "COMP 1.5: finite");
  }
  printf("      rms AT %.3f  1.5 %.3f  lift %.3f\n", lvl[0], lvl[2], lvl[1]);
  check(lvl[0] < lvl[2] && lvl[2] < lvl[1],
        "clip level: AT < 1.5 < lift (diodes lifted)");
  check(lvl[2] / lvl[0] > 1.4 && lvl[2] / lvl[0] < 2.6,
        "two diodes clip ~2x higher than one");
  defaults();

  /* --- op-amp rails bound the output --------------------------------- */
  ctrl[GAIN] = 10; ctrl[VOLUME] = 10; ctrl[COMP] = 1;
  ctrl[TREBLE] = 10; ctrl[BASS] = 10;
  r = sine_rms(h, SR, 440, 1.0, &fin);
  check(fin && r < 3.6, "worst case (all max, lift, 1 V): bounded by rails");
  printf("      rms(worst case) = %.3f\n", r);
  defaults();

  /* --- MID knob changes the voicing ---------------------------------- */
  ctrl[GAIN] = 0; ctrl[COMP] = 1;
  r1 = sine_rms(h, SR, 2000, 0.01, &fin);
  ctrl[MID] = 10;
  r2 = sine_rms(h, SR, 2000, 0.01, &fin);
  printf("      2 kHz: MID 10 vs stock %+.1f dB\n", db(r2 / r1));
  check(db(r2 / r1) > 3, "MID (MB pot) towards bright lifts 2 kHz");
  defaults();

  /* --- MODEL: Bass BB is full range into the clipper ----------------- */
  /* the guitar clipper has a 720 Hz high-pass in its ground leg (47n +
     4k7), the bass one 10 uF + 10k: small-signal, the bass model passes
     ~10 dB more at 60 Hz (analytic circuit: +10.1 dB). */
  ctrl[COMP] = 1;
  r1 = sine_rms(h, SR, 60, 0.001, &fin);
  ctrl[MODEL] = 1;
  r2 = sine_rms(h, SR, 60, 0.001, &fin);
  printf("      60 Hz: bass vs guitar model %+.1f dB\n", db(r2 / r1));
  check(fin && db(r2 / r1) > 8 && db(r2 / r1) < 12,
        "MODEL bass: full-range clipper (+8..12 dB at 60 Hz)");
  defaults();

  /* --- long run with random control changes -------------------------- */
  int stable = 1;
  srand(1);
  for (int b = 0; b < 4096 && stable; ++b) {
    if (b % 16 == 0) {
      ctrl[GAIN] = rand() % 101 / 10.0f;
      ctrl[VOLUME] = rand() % 101 / 10.0f;
      ctrl[TREBLE] = rand() % 101 / 10.0f;
      ctrl[BASS] = rand() % 101 / 10.0f;
      ctrl[MID] = rand() % 101 / 10.0f;
      ctrl[COMP] = rand() % 3;
      ctrl[MODEL] = rand() % 2;
    }
    for (int i = 0; i < BLOCK; ++i)
      in[i] = (rand() % 2001 / 1000.0f - 1.0f) * 0.8f;
    desc->run(h, BLOCK);
    for (int i = 0; i < BLOCK; ++i)
      if (!isfinite(out[i]) || fabsf(out[i]) > 4.0f)
        stable = 0;
  }
  check(stable, "4096 blocks noise + random controls: finite, bounded");
  close_plugin(h);

  /* --- other sample rates -------------------------------------------- */
  const double rates[] = { 44100, 88200, 96000, 192000 };
  for (int i = 0; i < 4; ++i) {
    char msg[96];
    defaults();
    LV2_Handle h2 = open_plugin(rates[i]);
    r = h2 ? sine_rms(h2, rates[i], 440, 0.3, &fin) : -1;
    snprintf(msg, sizeof msg, "@%.0f Hz: finite, sensible level (rms %.3f)",
             rates[i], r);
    check(h2 && fin && r > 0.05 && r < 3.0, msg);
    if (h2)
      close_plugin(h2);
  }

  dlclose(lib);
  printf("\n%s (%d failure%s)\n", failures ? "TESTS FAILED" : "ALL TESTS PASSED",
         failures, failures == 1 ? "" : "s");
  return failures ? 1 : 0;
}
