# BluesFuse Preamp

**BluesFuse Preamp** is a Faust-based emulation of the **Xotic BB Preamp** overdrive (LV2 with a
Stom-Pi/MOD modgui, VST3, CLAP and a standalone JACK app, with a scalable GUI that recalls
the original pedal), covering
the five variants traced by Aion FX in 2021
([Tracing Journal: Xotic BB Preamp (five variants)](https://aionfx.com/news/tracing-journal-xotic-bb-preamp-five-variants/)):

| Variant | How to get it |
|---|---|
| BB Preamp 1.5 (current, post-2007) | defaults (COMP = 1.5, MODEL = Guitar, MID = 2) |
| BB-AT (Andy Timmons / pre-2007) | COMP = AT |
| Custom Shop Comp | the COMP switch: AT / Off (lift) / 1.5 |
| Custom Shop MB (mid boost) | the MID knob (2 = stock voicing) |
| Bass BB Preamp | MODEL = Bass |

All five are also shipped as LV2 presets, along with three published settings (see [Presets](#presets)).

## Circuit

The BB Preamp is a Tube Screamer with two extra op-amp stages spliced in
before the output buffer (Aion's analysis). Component values come from the
trace schematics:

```
in -> Q1 buffer -> IC1A TS clipper -> IC1B TS tone -> IC2A gain stage
   -> IC2B Baxandall (BASS/TREBLE) -> VOLUME A -> Q2 buffer -> out
                                        ^ VOLUME B sets the IC2A gain
```

- **IC1A clipper**: non-inverting stage with the diodes in the feedback
  loop. The ground leg (R6 + C4: 4k7/47n, so a 720 Hz high-pass on guitar;
  10k/10 µF, full range, on Bass) sets a current through Rf = GAIN (1M) + R5,
  shunted by C3 and the 1N914 strings. The diode equation
  `v + Rf·2·Is·sinh(v / (N·n·Vt)) = u` is solved per sample with Newton
  iterations (< 10 µV error), and the output is the classic TS `Vin + v`.
  N = 1 (AT), 2 (1.5), or lifted (Comp "Off", where only the op-amp rails
  clip).
- **IC1B TS tone**: the Tube Screamer tone stage, hard-wired by R9/R10
  (1k/18k) near 9 o'clock. The MB restores the 20k W-taper pot, which MID
  models (2 = 9 o'clock = stock). The Bass BB keeps its fixed resistors, so
  MID has no effect in the Bass model (see below).
- **IC2A gain stage**: an inverting stage whose input arm is the "clumsy"
  series chain C7, R13, (R14 ‖ VOLUME B), C8, R15. Turning VOLUME up raises
  this stage's gain from +0.8 to +12.5 dB, together with a ~150 Hz high-pass.
- **IC2B Baxandall**: flat at noon, ±19 dB bass and ±17 dB treble.
- **Op-amp rails**: every JRC4558 output saturates smoothly at ±3.6 V
  around its bias.

### How the DSP is built

The linear stages are **exact solutions of the circuit**: `tools/gen_tf.py`
runs symbolic nodal analysis (sympy) on the netlists of both component sets
and writes `src/bluesfuse_tf.lib`. This contains the analog H(s) coefficients
as polynomials in the pot positions, plus their bilinear discretization.
`src/bluesfuse.dsp` adds the clipper, rails and buffers, and the guitarix
`dsp2cc` tool turns it into `src/bluesfuse.cc`.

IC1B is modelled with the JRC4558's finite gain-bandwidth (3 MHz). The Bass
BB's extra C7 (1n from IC1B's + input to its output) is a positive-feedback
path whose loop gain exceeds 1 at MHz frequencies with an ideal op-amp. The
real pedal is stable only because of the op-amp's roll-off. With a MID pot
turned up, that loop has enough gain to oscillate in hardware too, which is
why MID applies to the guitar model only.

Measured against the analytic circuit, the plugin matches within 0.03 dB
from 40 Hz to 2 kHz and within 1.5 dB up to 8 kHz at 48 kHz. Above that,
bilinear warping makes it roll off a little early; at 88.2/96 kHz this
mostly disappears.

## Controls

| Port | Symbol   | Range   | Default | Meaning |
|------|----------|---------|---------|---------|
| 0    | `gain`   | 0–10    | 5       | drive (1MB pot in the clipper feedback) |
| 1    | `volume` | 0–10    | 5       | dual gang: output level + IC2A gain |
| 2    | `treble` | 0–10    | 5       | Baxandall treble (5 = flat) |
| 3    | `bass`   | 0–10    | 5       | Baxandall bass (5 = flat) |
| 4    | `mid`    | 0–10    | 2       | MB tone pot, W taper (2 = stock BB) |
| 5    | `comp`   | 0/1/2   | 2       | AT (1 diode) / Off (lift) / 1.5 (2 diodes) |
| 6    | `model`  | 0/1     | 0       | Guitar / Bass component set |

Levels: 1.0 full scale = 1 V at the pedal input and output, so the pedal's
own gain structure is kept (unity is around VOLUME 5–6). At full VOLUME the
pedal is a +12 dB booster and can exceed 0 dBFS, just as the hardware is
meant to push an amp.

## Presets

Variant presets (knobs at noon): **1.5 (current circuit)**, **AT (pre-2007,
one diode)**, **Bass model**, **Comp: no compression**, **MB: mids up**.

Published settings, converted from clock positions (7:00 = 0, noon = 5,
5:00 = 10):

| Preset | Source | GAIN | VOLUME | TREBLE | BASS | COMP | MODEL |
|---|---|---|---|---|---|---|---|
| Andy Timmons (AT manual setting) | the setting pictured in Timmons' BB-AT owner's manual | 5.0 | 2.3 | 4.0 | 7.0 | AT | Guitar |
| Bass: SVT Overdrive | Xotic Bass v1.5 manual, factory "Sample Settings" | 5.0 | 5.0 | 6.0 | 8.5 | 1.5 | Bass |
| Bass: Flip-Top Clean | Xotic Bass v1.5 manual, factory "Sample Settings" | 2.0 | 6.0 | 5.0 | 6.5 | 1.5 | Bass |

Timmons' setting sits around unity gain (+3 dB for soft playing,
compressing to −11 dB when hit hard), which fits his use of the pedal as
an always-on drive. The two bass settings are named after Ampeg's SVT
and B-15 "Flip-Top" amps. They come from the manual of the current Bass
v1.5, which may differ from the traced Bass circuit modelled here. On the
traced circuit (linear 1M gain pot) "Flip-Top Clean" is clean only for
soft playing (2.6% THD at 50 mV) and lightly driven at normal bass levels
(~13% at 200 mV). The values are kept as Xotic published them.

The pedal is credited to Keith Urban (Custom Shop MB + standard), Ben
Gibbard, Mike Sullivan, Greg Howe and Danielle Haim, but no knob settings
of theirs have been published, so they are not shipped as presets.

## GUI

Both GUIs draw the same pedal, which recalls the BB Preamp v1.5: the
tall vermilion box with a dark inset line, four white knobs in a 2×2
grid (Gain / Treble over Volume / Bass), the red LED between the top
pair, a white italic logo, and "out" / "in" printed along the sides. The
extra controls borrow the Custom Shop layouts: a small MID knob in the
centre (as on the MB), a mini COMP bat-lever toggle between Volume and
Bass (as on the Comp; up = AT, centre = Off, down = 1.5), and a GUITAR /
BASS slide switch under the logo. The footswitch toggles bypass. The
names and branding are our own.

- **Desktop** (LV2, VST3, CLAP, standalone): vector-drawn with cairo, so it
  scales cleanly to any size. Resize the window freely; the aspect ratio
  is kept and the host's HiDPI scale factor is honoured. Drag knobs up or
  down (Shift for fine control), use the mouse wheel, double-click to
  reset. Click the upper or lower half of the COMP lever to move it, and
  click the MODEL switch and the footswitch to toggle them.
- **Stom-Pi / MOD** (modgui): `modgui/` is rendered from the same artwork
  (`make modgui`), with film-strip knobs and switches at 2x resolution.

Linux GUIs are X11 (`ui:X11UI`). LV2 defines no Wayland UI type, so on
Wayland desktops plugin UIs run through XWayland, as in every LV2/VST3
host today.

## Building

Built with [DPF](https://github.com/DISTRHO/DPF) (a git submodule): one
codebase gives an LV2 bundle (DSP and UI in separate binaries, plus the
modgui), a VST3, a CLAP and a standalone JACK app. It needs a C++
compiler; the UI also needs the cairo and X11 development headers, and is
skipped automatically if they are missing.

```
git submodule update --init --recursive
make                # all formats into bin/
make NOUI=true      # DSP only (headless build hosts such as Stom-Pi)
make install        # LV2 bundle into /usr/lib/lv2 (INSTALL_DIR=...)
make install-all    # + VST3, CLAP and the JACK app (VST3_DIR, CLAP_DIR, BIN_DIR)
make test-all       # DSP, CLAP, UI (Xvfb) and modgui (headless Chrome) tests
make tf             # regenerate src/bluesfuse_tf.lib from the netlists (sympy)
make dsp            # regenerate src/bluesfuse.cc from the .dsp
make modgui         # re-render modgui/ from the artwork
```

The DPF plugin class binds the generated DSP's controls by the `PortIndex`
enum. Presets are DPF programs, exported to LV2 `presets.ttl` and to
VST3/CLAP from the same list. The LV2 URI and port symbols (including the
audio ports `in`/`out`) are unchanged from the earlier hand-written LV2
version, so existing pedalboards keep working.

CPU: about 0.6% of one desktop core (Ryzen 9900X) at 48 kHz. Expect a
5–10% share of one core on a Raspberry Pi 4.

## Layout

```
tools/gen_tf.py             netlists + nodal analysis -> Faust transfer functions
src/bluesfuse_tf.lib        generated linear stages ('make tf')
src/bluesfuse.dsp           Faust DSP: clipper, rails, buffers, controls
src/bluesfuse.cc            generated DSP class ('make dsp')
src/bluesfuse.h, bf_glue.h  PortIndex enum, glue for the generated class
plugins/BluesFuse/          DPF plugin: info, plugin class, UI, artwork
  Artwork.hpp               the pedal, drawn with cairo (shared by both GUIs)
tools/render_modgui.cpp     renders modgui/ (images, HTML, CSS) from the artwork
modgui/                     generated modgui (checked in)
tests/bf_test.c             LV2 DSP tests (circuit behaviour, bypass)
tests/ui_test.c             X11/XTest UI test host (run under Xvfb)
tests/modgui_check.py       modgui vs artwork in headless Chrome
tests/clap_check.c          CLAP smoke test
dpf/                        DPF submodule
```

## License

GPL-2.0-or-later (see LICENSE). "Xotic" and "BB Preamp" are trademarks of
their owner, used here only to identify the emulated circuit.
