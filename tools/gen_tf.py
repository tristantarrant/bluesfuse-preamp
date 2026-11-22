#!/usr/bin/env python3
"""
Generate src/bluesfuse_tf.lib: Faust transfer functions of the linear stages
of the Xotic BB Preamp, derived by symbolic nodal analysis (ideal op-amps)
of the Aion FX trace schematics (10/22/2021):
https://aionfx.com/news/tracing-journal-xotic-bb-preamp-five-variants/

Every stage is solved for both component sets (guitar BB 1.5/AT/Comp/MB and
Bass BB). The analog H(s) coefficients are polynomials in the pot positions;
they are discretized at run time in Faust with the bilinear transform.

Pot positions (0..1, clockwise rotation, already tapered):
  pg  GAIN   1MB    rheostat pin1-wiper in the IC1A feedback
  pw  MID    20kW   MB tone pot, fraction from the + input side (R9/R10)
  pv  VOLUME 100kB  gang B: rheostat wiper-pin3 in front of IC2A
  pb  BASS   50kB   Baxandall, CW = wiper towards the input side
  pt  TREBLE 50kB   Baxandall, CW = wiper towards the input side

Usage: tools/gen_tf.py > src/bluesfuse_tf.lib
"""

import sys
import sympy as sp

s = sp.Symbol("s")
pg, pw, pv, pb, pt = sp.symbols("pg pw pv pb pt")

RMIN = 1  # ohm, residual wiper/track resistance (keeps rheostats finite)

k = 1e3
n = 1e-9
u = 1e-6
p = 1e-12

# Component sets. Designators follow the 1.5 schematic; the bass schematic
# renumbers some parts (noted in comments).
GUITAR = dict(
    R5=10 * k, R6=4.7 * k, C3=47 * p, C4=47 * n,
    R7=1 * k, C5=220 * n, R8=10 * k, C6=220 * n, R11=220, R12=1 * k,
    R9=1 * k, R10=18 * k,       # stock tone resistors (MB: 20kW pot)
    tone_fixed=False,           # guitar: MID pot (= stock at 9 o'clock)
    C7tone=0.0,                 # (bass only: 1n across IC1B +in -> out)
    Cc1=1 * u, R13=1 * k, R14=47 * k, Cc2=100 * n, R15=10 * k,
    R16=47 * k, C9=150 * p,
    R17=4.7 * k, R18=4.7 * k, C10=33 * n, C11=33 * n, R19=33 * k,
    R20=10 * k, C12=4.7 * n, C13=4.7 * n,
)
BASS = dict(GUITAR)
BASS.update(
    R5=4.7 * k,                 # gain pot series resistor
    R6=10 * k, C4=10 * u,       # clipper ground leg: full-range gain
    C5=0.0,                     # C5 omitted
    C6=1 * u, R11=0.01,         # R11 jumper
    tone_fixed=True,            # fixed R9 1k / R10 18k, no MID
    C7tone=1 * n,               # C7 1n, IC1B pin 5 -> pin 7
    R14=22 * k,                 # R14 22k
    Cc1=1 * u, Cc2=10 * u,      # C8 1uF, C9 10uF
    C12=10 * n, C13=10 * n,     # C13/C14 10n
)

GBW = 3e6  # Hz, JRC4558 gain-bandwidth product

PGAIN = 1e6
PMID = 20e3
PVOL = 100e3
PEQ = 50e3


def solve(elements, opamps, unknown, out):
    """Nodal analysis. elements: (kind, n1, n2, value); 'in' is the unit
    source, 'gnd' is ground. opamps: (plus, minus, out[, gbw_hz]) -- ideal
    op-amps, or single-pole op-amps A(s) = 2*pi*gbw/s when gbw is given."""
    V = {nd: sp.Symbol("V_" + nd) for nd in unknown}
    V["gnd"] = 0
    V["in"] = 1
    opouts = {op[2] for op in opamps}
    eqs = []
    for nd in unknown:
        if nd in opouts:
            continue
        e = 0
        for kind, a, b, val in elements:
            if kind == "C" and val == 0:
                continue
            Y = (1 / sp.nsimplify(val)) if kind == "R" else s * sp.nsimplify(val)
            if a == nd:
                e += Y * (V[a] - V[b])
            elif b == nd:
                e += Y * (V[b] - V[a])
        eqs.append(e)
    for op in opamps:
        pl, mi, o = op[:3]
        if len(op) > 3:
            wt = 2 * sp.pi * sp.nsimplify(op[3])
            eqs.append(s * V[o] - wt * (V[pl] - V[mi]))
        else:
            eqs.append(V[pl] - V[mi])
    sol = sp.solve(eqs, [V[nd] for nd in unknown], dict=True)[0]
    H = sp.cancel(sp.together(sol[V[out]]))
    num, den = sp.fraction(H)
    num = sp.Poly(sp.expand(num), s)
    den = sp.Poly(sp.expand(den), s)
    # ascending powers of s
    B = list(reversed(num.all_coeffs()))
    A = list(reversed(den.all_coeffs()))
    return B, A


def stage_clipfb(c):
    """IC1A feedback drive: the linear voltage across the feedback network,
    u = Vin * Yg(s) * Zf(s), Yg = ground leg (R6 + C4), Zf = (Rgain+R5)||C3.
    The diodes clip this voltage (see bluesfuse.dsp)."""
    Rf = pg * PGAIN + RMIN + sp.nsimplify(c["R5"])
    R6, C4, C3 = (sp.nsimplify(c[x]) for x in ("R6", "C4", "C3"))
    H = sp.cancel(Rf * s * C4 / ((1 + s * C4 * R6) * (1 + s * Rf * C3)))
    num, den = sp.fraction(H)
    B = list(reversed(sp.Poly(sp.expand(num), s).all_coeffs()))
    A = list(reversed(sp.Poly(sp.expand(den), s).all_coeffs()))
    return B, A


def stage_tone(c):
    """IC1B: Tube Screamer tone stage. Stock BB: R9 1k / R10 18k fixed
    (tone hard-wired near 9:00); MB: 20kW pot. Modelled as the pot.
    The Bass BB's C7 (1n, + input -> output) is a positive feedback path
    whose loop gain exceeds 1 at MHz frequencies with an ideal op-amp;
    the real circuit is stable thanks to the op-amp roll-off, so IC1B is
    modelled with the JRC4558's finite gain-bandwidth. The Bass BB keeps
    its fixed R9/R10: with a MID pot turned up, that C7 loop has enough
    gain to oscillate (in hardware too), so MID does not apply to it."""
    if c["tone_fixed"]:
        Rl, Rr = c["R9"], c["R10"]
    else:
        Rl = pw * PMID + RMIN
        Rr = (1 - pw) * PMID + RMIN
    el = [
        ("R", "in", "P", c["R7"]),
        ("C", "P", "gnd", c["C5"]),
        ("R", "P", "gnd", c["R8"]),          # bias to VB (AC ground)
        ("R", "P", "J", Rl),                  # pot, + input side
        ("R", "J", "M", Rr),                  # pot, - input side
        ("C", "J", "K", c["C6"]),
        ("R", "K", "gnd", c["R11"]),
        ("R", "M", "O", c["R12"]),
        ("C", "P", "O", c["C7tone"]),
    ]
    return solve(el, [("P", "M", "O", GBW)], ["P", "J", "K", "M", "O"], "O")


def stage_gain(c):
    """IC2A: inverting stage whose input arm is C7, R13, (R14 || VOLUME B),
    C8, R15 in series; feedback R16 || C9."""
    Rvb = (1 - pv) * PVOL + RMIN
    el = [
        ("C", "in", "a", c["Cc1"]),
        ("R", "a", "b", c["R13"]),
        ("R", "b", "c", c["R14"]),
        ("R", "b", "c", Rvb),
        ("C", "c", "d", c["Cc2"]),
        ("R", "d", "M", c["R15"]),
        ("R", "M", "O", c["R16"]),
        ("C", "M", "O", c["C9"]),
    ]
    return solve(el, [("gnd", "M", "O")], ["a", "b", "c", "d", "M", "O"], "O")


def stage_bax(c):
    """IC2B: two-band Baxandall (BASS / TREBLE)."""
    el = [
        ("R", "in", "N1", c["R17"]),
        ("R", "N1", "W", (1 - pb) * PEQ + RMIN),
        ("R", "W", "N2", pb * PEQ + RMIN),
        ("R", "N2", "O", c["R18"]),
        ("C", "N1", "W", c["C10"]),
        ("C", "N2", "W", c["C11"]),
        ("R", "W", "M", c["R19"]),
        ("R", "M", "TW", c["R20"]),
        ("C", "in", "T3", c["C12"]),
        ("R", "T3", "TW", (1 - pt) * PEQ + RMIN),
        ("R", "TW", "T1", pt * PEQ + RMIN),
        ("C", "T1", "O", c["C13"]),
    ]
    return solve(el, [("gnd", "M", "O")],
                 ["N1", "W", "N2", "M", "TW", "T3", "T1", "O"], "O")


def bilinear_matrix(N):
    """M[j][k]: coefficient of z^-j in (1 - z^-1)^k (1 + z^-1)^(N-k)."""
    z = sp.Symbol("z")
    M = [[0] * (N + 1) for _ in range(N + 1)]
    for kk in range(N + 1):
        poly = sp.Poly(sp.expand((1 - z) ** kk * (1 + z) ** (N - kk)), z)
        for j in range(N + 1):
            M[j][kk] = int(poly.coeff_monomial(z ** j))
    return M


def fexpr(e):
    e = sp.horner(sp.nsimplify(sp.expand(e), rational=False)) if e.free_symbols else e
    return sp.ccode(sp.N(e, 17))


def emit_stage(name, args, doc, fn):
    Bg, Ag = fn(GUITAR)
    Bb, Ab = fn(BASS)
    N = max(len(Bg), len(Ag), len(Bb), len(Ab)) - 1
    pad = lambda L: L + [sp.Integer(0)] * (N + 1 - len(L))
    Bg, Ag, Bb, Ab = pad(Bg), pad(Ag), pad(Bb), pad(Ab)
    if sp.simplify(Ag[N]) == 0 or sp.simplify(Ab[N]) == 0:
        sys.exit("%s: model orders differ, cannot share one filter" % name)
    M = bilinear_matrix(N)
    out = []
    out.append("// %s" % doc)
    out.append("%s(m, %s) = fi.iir((%s), (%s)) with {" % (
        name, ", ".join(args),
        ", ".join("b%d/a0" % j for j in range(N + 1)),
        ", ".join("a%d/a0" % j for j in range(1, N + 1))))
    out.append("    c = 2.0*ma.SR;")
    for kk in range(N + 1):
        out.append("    B%d = select2(m, %s, %s);" % (kk, fexpr(Bg[kk]), fexpr(Bb[kk])))
    for kk in range(N + 1):
        out.append("    A%d = select2(m, %s, %s);" % (kk, fexpr(Ag[kk]), fexpr(Ab[kk])))
    for kk in range(1, N + 1):
        out.append("    c%d = %s;" % (kk, "*".join(["c"] * kk)))
    for j in range(N + 1):
        tb = []
        ta = []
        for kk in range(N + 1):
            mjk = M[j][kk]
            if mjk == 0:
                continue
            ck = "" if kk == 0 else "*c%d" % kk
            tb.append("(%d)*B%d%s" % (mjk, kk, ck))
            ta.append("(%d)*A%d%s" % (mjk, kk, ck))
        out.append("    b%d = %s;" % (j, " + ".join(tb)))
        out.append("    a%d = %s;" % (j, " + ".join(ta)))
    out.append("};")
    out.append("")
    return "\n".join(out)


def main():
    hdr = """// GENERATED by tools/gen_tf.py -- do not edit by hand.
//
// Linear stages of the Xotic BB Preamp (Aion FX trace schematics,
// 10/22/2021), solved by nodal analysis with ideal op-amps. Each function
// takes the model selector m (0 = guitar BB, 1 = Bass BB) and the tapered
// pot positions (0..1) and discretizes H(s) with the bilinear transform.

fi = library("filters.lib");
ma = library("maths.lib");
"""
    parts = [hdr]
    parts.append(emit_stage("clipfb", ["pg"],
        "IC1A feedback drive (linear voltage across Rf||C3 for input Vin)",
        stage_clipfb))
    parts.append(emit_stage("tone", ["pw"],
        "IC1B Tube Screamer tone stage (R9/R10 or the MB 20kW pot)",
        stage_tone))
    parts.append(emit_stage("gainstage", ["pv"],
        "IC2A inverting gain stage, gain set by VOLUME gang B",
        stage_gain))
    parts.append(emit_stage("baxandall", ["pb", "pt"],
        "IC2B two-band Baxandall tone stack (inverting)",
        stage_bax))
    sys.stdout.write("\n".join(parts))


if __name__ == "__main__":
    main()
