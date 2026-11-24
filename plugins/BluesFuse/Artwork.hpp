/*
 * BluesFuse Preamp - pedal artwork, drawn with cairo.
 *
 * Shared by the scalable plugin UI (UIBluesFuse.cpp) and by the modgui
 * renderer (tools/render_modgui.cpp), so the desktop GUI and the modgui
 * look the same. Everything is drawn in a fixed design space of
 * W x H units; callers scale the cairo context.
 *
 * The layout recalls the BB Preamp v1.5: tall vermilion box with a dark
 * inset line, four white knobs in a 2x2 grid (Gain / Treble over Volume /
 * Bass), red LED between the top pair, white italic logo, "out" / "in"
 * printed along the sides. The extra controls borrow the Custom Shop
 * layouts: a small MID knob in the centre (MB) and a mini COMP toggle
 * (Comp), plus a GUITAR / BASS selector. Names and branding are our own.
 *
 * Copyright (C) 2026 Dataforte
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#pragma once

#include <cairo.h>
#include <cmath>

#include "BrandPath.hpp"

namespace bf_art {

constexpr double W = 240.0;
constexpr double H = 450.0;

// ---- palette ---------------------------------------------------------
struct RGB { double r, g, b; };
constexpr RGB kBodyTop    { 0.875, 0.275, 0.130 };
constexpr RGB kBodyBottom { 0.775, 0.200, 0.090 };
constexpr RGB kLine       { 0.380, 0.075, 0.030 };
constexpr RGB kWhite      { 0.98, 0.97, 0.95 };
constexpr RGB kInk        { 0.10, 0.04, 0.02 };

// ---- control layout --------------------------------------------------
enum Kind { kKnob, kToggle, kFootswitch };

struct Control {
    int    param;      // DPF parameter index
    Kind   kind;
    double x, y;       // centre
    double r;          // hit radius (knobs, footswitch)
    double w, h;       // hit box (toggles), centred on x, y
    double min, max;   // toggles step through min..max (integers)
};

// parameter indices (mirrors DistrhoPluginInfo.h)
enum { P_GAIN, P_VOLUME, P_TREBLE, P_BASS, P_MID, P_COMP, P_MODEL, P_BYPASS, P_COUNT };

// Positions are snapped so that every modgui element (centre minus half
// its film box) lands on a whole CSS pixel (1.25 design units): browsers
// place backgrounds at fractional offsets with sub-pixel blur otherwise.
// tools/render_modgui.cpp refuses to generate fractional positions.
constexpr double KNOB_R = 33.0;
constexpr double KXL = 68.75, KXR = 171.25;     // knob columns
constexpr double KYT = 78.75, KYB = 182.5;      // knob rows
constexpr double MID_X = 120.0, MID_Y = 130.0, MID_R = 15.0;
constexpr double COMP_X = 120.0, COMP_Y = 206.25;
constexpr double LED_X = 120.0, LED_Y = 41.25;
constexpr double MODEL_X = 120.0, MODEL_Y = 322.5;
constexpr double FOOT_X = 120.0, FOOT_Y = 371.25;

constexpr Control kControls[] = {
    { P_GAIN,   kKnob,       KXL,     KYT,     36.0, 0, 0,   0, 10 },
    { P_TREBLE, kKnob,       KXR,     KYT,     36.0, 0, 0,   0, 10 },
    { P_VOLUME, kKnob,       KXL,     KYB,     36.0, 0, 0,   0, 10 },
    { P_BASS,   kKnob,       KXR,     KYB,     36.0, 0, 0,   0, 10 },
    { P_MID,    kKnob,       MID_X,   MID_Y,   19.0, 0, 0,   0, 10 },
    { P_COMP,   kToggle,     COMP_X,  COMP_Y,  0,   30, 58,  0, 2 },
    { P_MODEL,  kToggle,     MODEL_X, MODEL_Y, 0,  150, 20,  0, 1 },
    { P_BYPASS, kFootswitch, FOOT_X,  FOOT_Y,  26.0, 0, 0,   0, 1 },
};

struct State {
    float value[P_COUNT] = { 5.0f, 5.0f, 5.0f, 5.0f, 2.0f, 2.0f, 0.0f, 0.0f };
};

// ---- helpers ---------------------------------------------------------
inline void set(cairo_t* cr, const RGB& c, double a = 1.0)
{
    cairo_set_source_rgba(cr, c.r, c.g, c.b, a);
}

inline void rounded_rect(cairo_t* cr, double x, double y, double w, double h, double r)
{
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r, r, -M_PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, M_PI / 2);
    cairo_arc(cr, x + r, y + h - r, r, M_PI / 2, M_PI);
    cairo_arc(cr, x + r, y + r, r, M_PI, 3 * M_PI / 2);
    cairo_close_path(cr);
}

enum Align { kLeft, kCenter, kRight };

// Text with its baseline at y; squeezed horizontally if wider than maxw.
inline void text(cairo_t* cr, const char* s, double x, double y, double size,
                 const RGB& c, Align align = kCenter, bool bold = true,
                 bool italic = false, const char* family = "DejaVu Sans",
                 double maxw = 0.0)
{
    cairo_save(cr);
    cairo_select_font_face(cr, family,
                           italic ? CAIRO_FONT_SLANT_OBLIQUE : CAIRO_FONT_SLANT_NORMAL,
                           bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    cairo_text_extents_t e;
    cairo_text_extents(cr, s, &e);
    const double sx = (maxw > 0.0 && e.x_advance > maxw) ? maxw / e.x_advance : 1.0;
    const double w = e.x_advance * sx;
    double x0 = x;
    if (align == kCenter) x0 = x - w / 2;
    else if (align == kRight) x0 = x - w;
    cairo_translate(cr, x0, y);
    cairo_scale(cr, sx, 1.0);
    cairo_move_to(cr, 0, 0);
    set(cr, c);
    cairo_show_text(cr, s);
    cairo_restore(cr);
    cairo_new_path(cr);
}

// 0..1 -> angle (radians, cairo convention), 7 o'clock .. 5 o'clock
inline double knob_angle(double norm)
{
    const double deg = -150.0 + 300.0 * norm;      // 0 = 12 o'clock
    return (deg - 90.0) * M_PI / 180.0;
}

inline cairo_pattern_t* body_gradient()
{
    cairo_pattern_t* g = cairo_pattern_create_linear(0, 0, 0, H);
    cairo_pattern_add_color_stop_rgb(g, 0, kBodyTop.r, kBodyTop.g, kBodyTop.b);
    cairo_pattern_add_color_stop_rgb(g, 1, kBodyBottom.r, kBodyBottom.g, kBodyBottom.b);
    return g;
}

// ---- parts -----------------------------------------------------------

// White lobed knob with a black pointer line, lit from top left.
inline void white_knob(cairo_t* cr, double cx, double cy, double r, double norm)
{
    const double a = knob_angle(norm);
    cairo_new_path(cr);

    // shadow
    cairo_pattern_t* sh = cairo_pattern_create_radial(cx + r * 0.1, cy + r * 0.16, r * 0.75,
                                                      cx + r * 0.1, cy + r * 0.16, r * 1.28);
    cairo_pattern_add_color_stop_rgba(sh, 0, 0, 0, 0, 0.45);
    cairo_pattern_add_color_stop_rgba(sh, 1, 0, 0, 0, 0.0);
    cairo_arc(cr, cx + r * 0.1, cy + r * 0.16, r * 1.28, 0, 2 * M_PI);
    cairo_set_source(cr, sh);
    cairo_fill(cr);
    cairo_pattern_destroy(sh);

    // lobed skirt (lobes rotate with the knob)
    const int lobes = 8, steps = 160;
    for (int i = 0; i <= steps; ++i) {
        const double t = i * 2 * M_PI / steps;
        const double rr = r * (0.935 + 0.065 * cos(lobes * (t - a)));
        if (i == 0) cairo_move_to(cr, cx + rr * cos(t), cy + rr * sin(t));
        else        cairo_line_to(cr, cx + rr * cos(t), cy + rr * sin(t));
    }
    cairo_close_path(cr);
    cairo_pattern_t* sk = cairo_pattern_create_linear(cx - r, cy - r, cx + r, cy + r);
    cairo_pattern_add_color_stop_rgb(sk, 0, 1.00, 1.00, 0.99);
    cairo_pattern_add_color_stop_rgb(sk, 0.6, 0.86, 0.86, 0.84);
    cairo_pattern_add_color_stop_rgb(sk, 1, 0.66, 0.66, 0.64);
    cairo_set_source(cr, sk);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(sk);
    cairo_set_source_rgba(cr, 0.25, 0.22, 0.20, 0.9);
    cairo_set_line_width(cr, r * 0.035);
    cairo_stroke(cr);

    // top face
    cairo_pattern_t* f = cairo_pattern_create_radial(cx - r * 0.25, cy - r * 0.3, r * 0.05,
                                                     cx, cy, r * 0.75);
    cairo_pattern_add_color_stop_rgb(f, 0, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb(f, 1, 0.88, 0.88, 0.86);
    cairo_arc(cr, cx, cy, r * 0.72, 0, 2 * M_PI);
    cairo_set_source(cr, f);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(f);
    cairo_set_source_rgba(cr, 0.55, 0.53, 0.50, 0.6);
    cairo_set_line_width(cr, r * 0.025);
    cairo_stroke(cr);

    // pointer line, centre to edge
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, r * 0.085);
    set(cr, kInk);
    cairo_move_to(cr, cx + r * 0.12 * cos(a), cy + r * 0.12 * sin(a));
    cairo_line_to(cr, cx + r * 0.86 * cos(a), cy + r * 0.86 * sin(a));
    cairo_stroke(cr);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
}

// Miniature bat-lever toggle seen from above: 0 = lever up, 1 = centre
// (end-on), 2 = lever down.
inline void bat_toggle(cairo_t* cr, double cx, double cy, int pos)
{
    cairo_new_path(cr);
    // hex nut
    const double rn = 8.0;
    for (int i = 0; i < 6; ++i) {
        const double t = i * M_PI / 3;
        if (i == 0) cairo_move_to(cr, cx + rn * cos(t), cy + rn * sin(t));
        else        cairo_line_to(cr, cx + rn * cos(t), cy + rn * sin(t));
    }
    cairo_close_path(cr);
    cairo_pattern_t* g = cairo_pattern_create_linear(cx - rn, cy - rn, cx + rn, cy + rn);
    cairo_pattern_add_color_stop_rgb(g, 0, 0.95, 0.95, 0.93);
    cairo_pattern_add_color_stop_rgb(g, 1, 0.42, 0.42, 0.41);
    cairo_set_source(cr, g);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(g);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
    cairo_set_line_width(cr, 0.7);
    cairo_stroke(cr);
    cairo_arc(cr, cx, cy, 4.6, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.30, 0.30, 0.30);
    cairo_fill(cr);

    double tx = cx, ty = cy;
    if (pos != 1) {
        const double dir = pos == 0 ? -1.0 : 1.0;
        ty = cy + dir * 16.0;
        // lever shadow, then tapered chrome lever
        cairo_move_to(cr, cx - 2.6 + 1.5, cy + 1.5);
        cairo_line_to(cr, cx + 2.6 + 1.5, cy + 1.5);
        cairo_line_to(cr, tx + 1.7 + 1.5, ty + 1.5);
        cairo_line_to(cr, tx - 1.7 + 1.5, ty + 1.5);
        cairo_close_path(cr);
        cairo_set_source_rgba(cr, 0, 0, 0, 0.35);
        cairo_fill(cr);
        cairo_move_to(cr, cx - 2.6, cy);
        cairo_line_to(cr, cx + 2.6, cy);
        cairo_line_to(cr, tx + 1.7, ty);
        cairo_line_to(cr, tx - 1.7, ty);
        cairo_close_path(cr);
        cairo_pattern_t* l = cairo_pattern_create_linear(cx - 3, 0, cx + 3, 0);
        cairo_pattern_add_color_stop_rgb(l, 0, 0.55, 0.55, 0.54);
        cairo_pattern_add_color_stop_rgb(l, 0.4, 1.0, 1.0, 1.0);
        cairo_pattern_add_color_stop_rgb(l, 1, 0.50, 0.50, 0.49);
        cairo_set_source(cr, l);
        cairo_fill(cr);
        cairo_pattern_destroy(l);
    }
    // ball tip (end-on in the centre position)
    const double rb = pos == 1 ? 3.8 : 3.0;
    cairo_pattern_t* b = cairo_pattern_create_radial(tx - 1, ty - 1, 0.3, tx, ty, rb);
    cairo_pattern_add_color_stop_rgb(b, 0, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb(b, 1, 0.45, 0.45, 0.44);
    cairo_arc(cr, tx, ty, rb, 0, 2 * M_PI);
    cairo_set_source(cr, b);
    cairo_fill(cr);
    cairo_pattern_destroy(b);
}

// Miniature slide switch, thumb left (0) or right (1).
inline void slide_switch(cairo_t* cr, double cx, double cy, int pos)
{
    const double w = 28, h = 11;
    cairo_new_path(cr);
    rounded_rect(cr, cx - w / 2, cy - h / 2, w, h, 3);
    cairo_set_source_rgb(cr, 0.14, 0.05, 0.03);
    cairo_fill_preserve(cr);
    cairo_set_source_rgba(cr, 1, 1, 1, 0.35);
    cairo_set_line_width(cr, 0.8);
    cairo_stroke(cr);
    const double tw = 12, th = 9;
    const double tx = pos ? cx + w / 2 - tw - 1 : cx - w / 2 + 1;
    cairo_pattern_t* g = cairo_pattern_create_linear(tx, cy - th / 2, tx, cy + th / 2);
    cairo_pattern_add_color_stop_rgb(g, 0, 0.97, 0.97, 0.95);
    cairo_pattern_add_color_stop_rgb(g, 1, 0.58, 0.58, 0.56);
    rounded_rect(cr, tx, cy - th / 2, tw, th, 2);
    cairo_set_source(cr, g);
    cairo_fill(cr);
    cairo_pattern_destroy(g);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.45);
    cairo_set_line_width(cr, 0.8);
    for (int i = 0; i < 3; ++i) {
        const double lx = tx + 3.5 + i * 2.5;
        cairo_move_to(cr, lx, cy - th / 2 + 2);
        cairo_line_to(cr, lx, cy + th / 2 - 2);
    }
    cairo_stroke(cr);
}

inline void footswitch(cairo_t* cr, double cx, double cy)
{
    cairo_new_path(cr);
    const double rn = 19;
    for (int i = 0; i < 6; ++i) {
        const double t = M_PI / 6 + i * M_PI / 3;
        if (i == 0) cairo_move_to(cr, cx + rn * cos(t), cy + rn * sin(t));
        else        cairo_line_to(cr, cx + rn * cos(t), cy + rn * sin(t));
    }
    cairo_close_path(cr);
    cairo_pattern_t* g = cairo_pattern_create_linear(cx - rn, cy - rn, cx + rn, cy + rn);
    cairo_pattern_add_color_stop_rgb(g, 0, 0.94, 0.94, 0.92);
    cairo_pattern_add_color_stop_rgb(g, 1, 0.40, 0.40, 0.39);
    cairo_set_source(cr, g);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(g);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
    cairo_set_line_width(cr, 0.8);
    cairo_stroke(cr);
    cairo_arc(cr, cx, cy, 13.5, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.56, 0.56, 0.55);
    cairo_fill(cr);
    cairo_pattern_t* p = cairo_pattern_create_radial(cx - 3, cy - 4, 1, cx, cy, 10);
    cairo_pattern_add_color_stop_rgb(p, 0, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgb(p, 0.5, 0.82, 0.82, 0.81);
    cairo_pattern_add_color_stop_rgb(p, 1, 0.46, 0.46, 0.45);
    cairo_arc(cr, cx, cy, 9.5, 0, 2 * M_PI);
    cairo_set_source(cr, p);
    cairo_fill_preserve(cr);
    cairo_pattern_destroy(p);
    cairo_set_source_rgba(cr, 0, 0, 0, 0.5);
    cairo_stroke(cr);
}

inline void led(cairo_t* cr, double cx, double cy, bool lit)
{
    cairo_new_path(cr);
    cairo_arc(cr, cx, cy, 5.5, 0, 2 * M_PI);
    cairo_set_source_rgb(cr, 0.30, 0.08, 0.04);
    cairo_fill(cr);
    if (lit) {
        cairo_pattern_t* glow = cairo_pattern_create_radial(cx, cy, 2, cx, cy, 13);
        cairo_pattern_add_color_stop_rgba(glow, 0, 1.0, 0.25, 0.15, 0.6);
        cairo_pattern_add_color_stop_rgba(glow, 1, 1.0, 0.25, 0.15, 0.0);
        cairo_arc(cr, cx, cy, 13, 0, 2 * M_PI);
        cairo_set_source(cr, glow);
        cairo_fill(cr);
    }
    cairo_pattern_t* g = cairo_pattern_create_radial(cx - 1.2, cy - 1.4, 0.5, cx, cy, 4.2);
    if (lit) {
        cairo_pattern_add_color_stop_rgb(g, 0, 1.0, 0.90, 0.85);
        cairo_pattern_add_color_stop_rgb(g, 0.4, 1.0, 0.15, 0.08);
        cairo_pattern_add_color_stop_rgb(g, 1, 0.70, 0.02, 0.02);
    } else {
        cairo_pattern_add_color_stop_rgb(g, 0, 0.60, 0.30, 0.26);
        cairo_pattern_add_color_stop_rgb(g, 1, 0.30, 0.04, 0.03);
    }
    cairo_arc(cr, cx, cy, 4.2, 0, 2 * M_PI);
    cairo_set_source(cr, g);
    cairo_fill(cr);
    cairo_pattern_destroy(g);
}

// White italic logo with a dark outline: "BluesFuse" over "-preamp".
inline void logo(cairo_t* cr)
{
    struct Line { const char* s; double size, y, x; Align a; };
    const Line lines[2] = {
        { "BluesFuse", 40.0, 280.0, W / 2, kCenter },
        { "-preamp",   20.0, 302.0, W - 26, kRight },
    };
    for (const Line& l : lines) {
        cairo_save(cr);
        cairo_select_font_face(cr, "DejaVu Sans", CAIRO_FONT_SLANT_OBLIQUE, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, l.size);
        cairo_text_extents_t e;
        cairo_text_extents(cr, l.s, &e);
        const double maxw = W - 44;
        const double sx = e.x_advance > maxw ? maxw / e.x_advance : 1.0;
        const double w = e.x_advance * sx;
        const double x0 = l.a == kCenter ? l.x - w / 2 : l.x - w;
        cairo_translate(cr, x0, l.y);
        cairo_scale(cr, sx, 1.0);
        cairo_move_to(cr, 0, 0);
        cairo_text_path(cr, l.s);
        cairo_restore(cr);
        set(cr, kInk, 0.85);
        cairo_set_line_width(cr, 3.0);
        cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
        cairo_stroke_preserve(cr);
        set(cr, kWhite);
        cairo_fill(cr);
    }
}

inline void vertical_text(cairo_t* cr, const char* s, double x, double y, double size, double angle)
{
    cairo_save(cr);
    cairo_translate(cr, x, y);
    cairo_rotate(cr, angle);
    text(cr, s, 0, 0, size, kWhite, kCenter, true);
    cairo_restore(cr);
}

// ---- layers ----------------------------------------------------------
enum Layer {
    kLayerBody     = 1 << 0,   // box, print, footswitch
    kLayerKnobs    = 1 << 1,   // GAIN / TREBLE / VOLUME / BASS
    kLayerMid      = 1 << 2,
    kLayerComp     = 1 << 3,
    kLayerModel    = 1 << 4,
    kLayerLed      = 1 << 5,
    kLayerAll      = 0xff
};

inline double norm(const State& s, int p)
{
    for (const Control& c : kControls)
        if (c.param == p)
            return (s.value[p] - c.min) / (c.max - c.min);
    return 0.0;
}

inline void draw(cairo_t* cr, const State& s, unsigned layers = kLayerAll)
{
    if (layers & kLayerBody) {
        cairo_new_path(cr);
        cairo_pattern_t* g = body_gradient();
        rounded_rect(cr, 1, 1, W - 2, H - 2, 16);
        cairo_set_source(cr, g);
        cairo_fill_preserve(cr);
        cairo_pattern_destroy(g);
        cairo_set_source_rgba(cr, 0, 0, 0, 0.6);
        cairo_set_line_width(cr, 1.5);
        cairo_stroke(cr);
        // soft top highlight
        cairo_pattern_t* hl = cairo_pattern_create_linear(0, 0, 0, 60);
        cairo_pattern_add_color_stop_rgba(hl, 0, 1, 1, 1, 0.18);
        cairo_pattern_add_color_stop_rgba(hl, 1, 1, 1, 1, 0.0);
        rounded_rect(cr, 2, 2, W - 4, 60, 15);
        cairo_set_source(cr, hl);
        cairo_fill(cr);
        cairo_pattern_destroy(hl);
        // dark inset line
        rounded_rect(cr, 9, 9, W - 18, H - 18, 11);
        set(cr, kLine);
        cairo_set_line_width(cr, 2.2);
        cairo_stroke(cr);

        // control legends
        text(cr, "Gain",   KXL, KYT + KNOB_R + 17, 13, kWhite);
        text(cr, "Treble", KXR, KYT + KNOB_R + 17, 13, kWhite);
        text(cr, "Volume", KXL, KYB + KNOB_R + 17, 13, kWhite);
        text(cr, "Bass",   KXR, KYB + KNOB_R + 17, 13, kWhite);
        text(cr, "Mid",    MID_X, MID_Y + MID_R + 12, 9.5, kWhite);
        text(cr, "AT",     COMP_X, COMP_Y - 21, 8.0, kWhite);
        text(cr, "1.5",    COMP_X, COMP_Y + 29, 8.0, kWhite);
        text(cr, "COMP",   COMP_X, COMP_Y + 39, 7.0, kWhite);

        logo(cr);

        text(cr, "GUITAR", MODEL_X - 20, MODEL_Y + 3.5, 9.0, kWhite, kRight);
        text(cr, "BASS",   MODEL_X + 20, MODEL_Y + 3.5, 9.0, kWhite, kLeft);

        vertical_text(cr, "out", 18, 236, 13, -M_PI / 2);
        vertical_text(cr, "in", W - 18, 236, 13, M_PI / 2);

        footswitch(cr, FOOT_X, FOOT_Y);
        text(cr, "Bypass", FOOT_X, FOOT_Y + 35, 11, kWhite, kCenter, true, true);
        // maker wordmark, baked from Bitcount Grid Double (tools/gen_brand.py),
        // so the UI and the modgui match on machines without the font
        set(cr, kWhite);
        bf_brand::draw(cr, W / 2, H - 20, 15.0);
    }

    if (layers & kLayerKnobs) {
        white_knob(cr, KXL, KYT, KNOB_R, norm(s, P_GAIN));
        white_knob(cr, KXR, KYT, KNOB_R, norm(s, P_TREBLE));
        white_knob(cr, KXL, KYB, KNOB_R, norm(s, P_VOLUME));
        white_knob(cr, KXR, KYB, KNOB_R, norm(s, P_BASS));
    }
    if (layers & kLayerMid)
        white_knob(cr, MID_X, MID_Y, MID_R, norm(s, P_MID));
    if (layers & kLayerComp)
        bat_toggle(cr, COMP_X, COMP_Y, static_cast<int>(std::lround(s.value[P_COMP])));
    if (layers & kLayerModel)
        slide_switch(cr, MODEL_X, MODEL_Y, s.value[P_MODEL] > 0.5f ? 1 : 0);
    if (layers & kLayerLed)
        led(cr, LED_X, LED_Y, s.value[P_BYPASS] < 0.5f);
}

} // namespace bf_art
