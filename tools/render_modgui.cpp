/*
 * Renders the modgui for BluesFuse Preamp from the same cairo artwork as
 * the plugin UI (plugins/BluesFuse/Artwork.hpp).
 *
 *   render_modgui <outdir>            modgui images, icon.html, stylesheet.css
 *                                     and modgui.ttl (positions come from the
 *                                     artwork's control layout)
 *   render_modgui --preview <file> [scale [v0 v1 ...]]
 *                                     full pedal; optional parameter values
 *                                     in parameter order (default: defaults)
 *
 * Copyright (C) 2026 Dataforte
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "../plugins/BluesFuse/Artwork.hpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <string>

using namespace bf_art;

// modgui CSS size of the pedal = design size * CSS_SCALE; images are
// rendered at IMG_SCALE times that for high-DPI screens.
static constexpr double CSS_SCALE = 0.8;
static constexpr double IMG_SCALE = 2.0;
static constexpr int KNOB_FRAMES = 61;

// Film-strip boxes in design units, chosen so frames are whole pixels at
// both CSS scale (x0.8) and image scale (x1.6): mod-ui derives the frame
// count from the strip width.
static constexpr double KNOB_BOX = 90.0;
static constexpr double MID_BOX = 45.0;
static constexpr double COMP_BOX = 40.0;
static constexpr double MODEL_BOX = 35.0;
static constexpr double LED_BOX = 30.0;
static constexpr double FOOT_BOX = 55.0;

static bool g_fractional = false;

static std::string px(double design)
{
    const double v = design * CSS_SCALE;
    if (std::fabs(v - std::round(v)) > 1e-6) {
        std::fprintf(stderr, "error: CSS value %g px is not a whole pixel\n", v);
        g_fractional = true;
    }
    char b[32];
    std::snprintf(b, sizeof b, "%gpx", v);
    return b;
}

// absolutely positioned box of `box` design units centred on (x, y)
static std::string place(double x, double y, double box)
{
    return "left: " + px(x - box / 2) + "; top: " + px(y - box / 2) +
           "; width: " + px(box) + "; height: " + px(box) + ";";
}

static std::string film(const char* img, int frames, double box)
{
    return std::string("background-image: url(/resources/") + img +
           "{{{ns}}}); background-repeat: no-repeat; background-size: " +
           px(box * frames) + " " + px(box) + ";";
}

static bool save(cairo_surface_t* s, const std::string& path)
{
    if (cairo_surface_write_to_png(s, path.c_str()) != CAIRO_STATUS_SUCCESS) {
        std::fprintf(stderr, "cannot write %s\n", path.c_str());
        return false;
    }
    std::printf("wrote %s (%dx%d)\n", path.c_str(),
                cairo_image_surface_get_width(s), cairo_image_surface_get_height(s));
    return true;
}

static bool write_text(const std::string& path, const std::string& body)
{
    std::ofstream f(path);
    f << body;
    std::printf("wrote %s\n", path.c_str());
    return bool(f);
}

static cairo_surface_t* surface(double w, double h, double scale, cairo_t** cr)
{
    cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                                                    (int)std::lround(w * scale),
                                                    (int)std::lround(h * scale));
    *cr = cairo_create(s);
    cairo_scale(*cr, scale, scale);
    return s;
}

static bool pedal(const std::string& path, unsigned layers, double scale, const State& st = State())
{
    cairo_t* cr;
    cairo_surface_t* s = surface(W, H, scale, &cr);
    draw(cr, st, layers);
    cairo_destroy(cr);
    const bool ok = save(s, path);
    cairo_surface_destroy(s);
    return ok;
}

// Horizontal film strip: frame i draws `fn(cr, i)` centred in a box.
template <typename F>
static bool strip(const std::string& path, int frames, double box, double scale, F fn)
{
    cairo_t* cr;
    cairo_surface_t* s = surface(box * frames, box, scale, &cr);
    for (int i = 0; i < frames; ++i) {
        cairo_save(cr);
        cairo_translate(cr, i * box + box / 2, box / 2);
        fn(cr, i);
        cairo_restore(cr);
    }
    cairo_destroy(cr);
    const bool ok = save(s, path);
    cairo_surface_destroy(s);
    return ok;
}

static const char* kJacks = R"(    <div class="mod-pedal-input">
        {{#effect.ports.audio.input}}
        <div class="mod-input mod-input-disconnected" title="{{name}}" mod-role="input-audio-port" mod-port-symbol="{{symbol}}">
            <div class="mod-pedal-input-image"></div>
        </div>
        {{/effect.ports.audio.input}}
    </div>
    <div class="mod-pedal-output">
        {{#effect.ports.audio.output}}
        <div class="mod-output mod-output-disconnected" title="{{name}}" mod-role="output-audio-port" mod-port-symbol="{{symbol}}">
            <div class="mod-pedal-output-image"></div>
        </div>
        {{/effect.ports.audio.output}}
    </div>
)";

static bool write_modgui_text(const std::string& dir)
{
    struct Knob { const char* sym; double x, y; };
    const Knob knobs[4] = {
        { "gain", KXL, KYT }, { "treble", KXR, KYT }, { "volume", KXL, KYB }, { "bass", KXR, KYB },
    };

    std::string html =
        "<div class=\"mod-pedal bf-pedal\">\n"
        "    <div mod-role=\"drag-handle\" class=\"mod-drag-handle\"></div>\n";
    for (const Knob& k : knobs)
        html += std::string("    <div class=\"bf-knob bf-") + k.sym +
                "\" mod-role=\"input-control-port\" mod-port-symbol=\"" + k.sym +
                "\" mod-widget=\"film\"></div>\n";
    html +=
        "    <div class=\"bf-mid\" mod-role=\"input-control-port\" mod-port-symbol=\"mid\" mod-widget=\"film\"></div>\n"
        "    <div class=\"bf-comp\" mod-role=\"input-control-port\" mod-port-symbol=\"comp\" mod-widget=\"film\"></div>\n"
        "    <div class=\"bf-model\" mod-role=\"input-control-port\" mod-port-symbol=\"model\" mod-widget=\"film\"></div>\n"
        "    <div class=\"bf-led mod-light\" mod-role=\"bypass-light\"></div>\n"
        "    <div class=\"bf-foot mod-footswitch\" mod-role=\"bypass\"></div>\n";
    html += kJacks;
    html += "</div>\n";

    std::string css = "/* generated by tools/render_modgui.cpp from Artwork.hpp -- do not edit */\n";
    css += ".bf-pedal { position: relative; width: " + px(W) + "; height: " + px(H) +
           "; background-image: url(/resources/pedal.png{{{ns}}}); background-size: " +
           px(W) + " " + px(H) + "; background-repeat: no-repeat; }\n";
    css += ".bf-pedal .bf-knob, .bf-pedal .bf-mid, .bf-pedal .bf-comp, .bf-pedal .bf-model,"
           " .bf-pedal .bf-led, .bf-pedal .bf-foot { position: absolute; cursor: pointer; }\n";
    for (const Knob& k : knobs)
        css += std::string(".bf-pedal .bf-") + k.sym + " { " + place(k.x, k.y, KNOB_BOX) + " " +
               film("knob.png", KNOB_FRAMES, KNOB_BOX) + " }\n";
    css += ".bf-pedal .bf-mid { " + place(MID_X, MID_Y, MID_BOX) + " " +
           film("midknob.png", KNOB_FRAMES, MID_BOX) + " }\n";
    css += ".bf-pedal .bf-comp { " + place(COMP_X, COMP_Y, COMP_BOX) + " " +
           film("comp.png", 3, COMP_BOX) + " }\n";
    css += ".bf-pedal .bf-model { " + place(MODEL_X, MODEL_Y, MODEL_BOX) + " " +
           film("switch.png", 2, MODEL_BOX) + " }\n";
    css += ".bf-pedal .bf-led { " + place(LED_X, LED_Y, LED_BOX) + " " +
           film("led.png", 2, LED_BOX) + " }\n";
    css += ".bf-pedal .bf-led.on { background-position: -" + px(LED_BOX) + " 0px; }\n";
    css += ".bf-pedal .bf-foot { " + place(FOOT_X, FOOT_Y, FOOT_BOX) + " border-radius: 50%; }\n";
    css += ".bf-pedal .mod-pedal-input, .bf-pedal .mod-pedal-output { top: " + px(215) + "; }\n";

    const std::string ttl =
        "@prefix modgui: <http://moddevices.com/ns/modgui#> .\n"
        "@prefix lv2:    <http://lv2plug.in/ns/lv2core#> .\n\n"
        "<http://dataforte.net/plugins/bluesfuse>\n"
        "    modgui:gui [\n"
        "        modgui:resourcesDirectory <modgui> ;\n"
        "        modgui:iconTemplate <modgui/icon.html> ;\n"
        "        modgui:stylesheet <modgui/stylesheet.css> ;\n"
        "        modgui:screenshot <modgui/screenshot.png> ;\n"
        "        modgui:thumbnail <modgui/thumbnail.png> ;\n"
        "        modgui:brand \"Dataforte\" ;\n"
        "        modgui:label \"BluesFuse Preamp\" ;\n"
        "    ] .\n";

    return write_text(dir + "/icon.html", html) &&
           write_text(dir + "/stylesheet.css", css) &&
           write_text(dir + "/modgui.ttl", ttl);
}

int main(int argc, char** argv)
{
    if (argc >= 3 && std::string(argv[1]) == "--preview") {
        const double scale = argc >= 4 ? std::atof(argv[3]) : 1.0;
        State st;
        for (int i = 4; i < argc && i - 4 < P_COUNT; ++i)
            st.value[i - 4] = static_cast<float>(std::atof(argv[i]));
        return pedal(argv[2], kLayerAll, scale, st) ? 0 : 1;
    }
    if (argc != 2) {
        std::fprintf(stderr, "usage: %s <outdir> | --preview <file.png> [scale [values...]]\n", argv[0]);
        return 1;
    }
    const std::string dir = argv[1];
    const double sc = CSS_SCALE * IMG_SCALE;   // design units -> image pixels
    bool ok = true;

    ok &= pedal(dir + "/pedal.png", kLayerBody, sc);
    ok &= strip(dir + "/knob.png", KNOB_FRAMES, KNOB_BOX, sc, [](cairo_t* cr, int i) {
        white_knob(cr, 0, 0, KNOB_R, i / double(KNOB_FRAMES - 1));
    });
    ok &= strip(dir + "/midknob.png", KNOB_FRAMES, MID_BOX, sc, [](cairo_t* cr, int i) {
        white_knob(cr, 0, 0, MID_R, i / double(KNOB_FRAMES - 1));
    });
    ok &= strip(dir + "/comp.png", 3, COMP_BOX, sc, [](cairo_t* cr, int i) {
        bat_toggle(cr, 0, 0, i);
    });
    ok &= strip(dir + "/switch.png", 2, MODEL_BOX, sc, [](cairo_t* cr, int i) {
        slide_switch(cr, 0, 0, i);
    });
    ok &= strip(dir + "/led.png", 2, LED_BOX, sc, [](cairo_t* cr, int i) {
        led(cr, 0, 0, i == 1);
    });
    ok &= pedal(dir + "/screenshot.png", kLayerAll, CSS_SCALE);
    ok &= pedal(dir + "/thumbnail.png", kLayerAll, CSS_SCALE * 0.3);

    ok &= write_modgui_text(dir);
    if (g_fractional) {
        std::fprintf(stderr, "error: fix the layout in Artwork.hpp (positions must snap to "
                             "1.25 design units)\n");
        return 1;
    }
    return ok ? 0 : 1;
}
