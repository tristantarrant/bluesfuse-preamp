/*
 * BluesFuse Preamp - Xotic BB Preamp (1.5 / AT / Bass / Comp / MB) emulation
 * DPF plugin wrapper around the faust-generated DSP class.
 *
 * Copyright (C) 2026 Dataforte
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DistrhoPlugin.hpp"

#include <cstring>
#include <vector>

// Faust/dsp2cc generated DSP (global namespace, binds controls by PortIndex)
#include "../../src/bf_glue.h"
#include "../../src/bluesfuse.h"
#include "../../src/bluesfuse.cc"

// Preset table and parameter defaults, generated from presets/bluesfuse.json
#include "Presets.hpp"

START_NAMESPACE_DISTRHO

// --------------------------------------------------------------------
// Presets come from presets/bluesfuse.json via Presets.hpp (make presets).
// DPF exports them to LV2 manifest/presets.ttl and to VST3/CLAP.
static_assert(preset_data::kParamCount == static_cast<unsigned>(kParamBypass),
              "presets/bluesfuse.json parameter list does not match DistrhoPluginInfo.h");

// --------------------------------------------------------------------

class PluginBluesFuse : public Plugin
{
public:
    PluginBluesFuse()
        : Plugin(kParamCount, preset_data::kCount, 0),
          fDsp(bluesfuse::plugin())
    {
        for (uint32_t i = 0; i < kParamBypass; ++i)
            fParams[i] = preset_data::kDefaults.value[i];
        fParams[kParamBypass] = 0.0f;

        for (uint32_t i = 0; i < kParamBypass; ++i)
            fDsp->connect_ports(i, &fParams[i], fDsp);

        fDsp->set_samplerate(static_cast<uint32_t>(getSampleRate()), fDsp);
        fDry.resize(getBufferSize());
        fFadeStep = 1.0f / (0.010f * static_cast<float>(getSampleRate()));
    }

    ~PluginBluesFuse() override
    {
        if (fDsp->activate_plugin != nullptr)
            fDsp->activate_plugin(false, fDsp);
        fDsp->delete_instance(fDsp);
    }

protected:
    const char* getLabel() const override { return "bluesfuse"; }
    const char* getDescription() const override
    {
        return "Emulation of the Xotic BB Preamp overdrive and its variants (1.5, AT, Bass, "
               "Custom Shop Comp and MB), after the Aion FX trace schematics: TS-style diode "
               "clipper, dual-gang volume and 2-band Baxandall EQ.";
    }
    const char* getMaker() const override { return "Dataforte"; }
    const char* getHomePage() const override { return "https://github.com/tristantarrant/bluesfuse-preamp"; }
    const char* getLicense() const override { return "GPL-2.0-or-later"; }
    uint32_t getVersion() const override { return d_version(1, 1, 0); }
    int64_t getUniqueId() const override { return d_cconst('D', 'b', 'F', 's'); }

    void initAudioPort(const bool input, const uint32_t index, AudioPort& port) override
    {
        port.groupId = kPortGroupMono;
        Plugin::initAudioPort(input, index, port);
        // keep the symbols of the pre-DPF plugin: pedalboards store them
        port.symbol = input ? "in" : "out";
        port.name   = input ? "In" : "Out";
    }

    void initParameter(const uint32_t index, Parameter& p) override
    {
        p.hints = kParameterIsAutomatable;
        p.ranges.min = 0.0f;
        p.ranges.max = 10.0f;
        p.ranges.def = 5.0f;

        switch (index)
        {
        case kParamGain:   p.name = "GAIN";   p.symbol = "gain";   break;
        case kParamVolume: p.name = "VOLUME"; p.symbol = "volume"; break;
        case kParamTreble: p.name = "TREBLE"; p.symbol = "treble"; break;
        case kParamBass:   p.name = "BASS";   p.symbol = "bass";   break;
        case kParamMid:
            p.name = "MID"; p.symbol = "mid";
            p.ranges.def = 2.0f;
            break;
        case kParamComp:
            p.hints |= kParameterIsInteger;
            p.name = "COMP"; p.symbol = "comp";
            p.ranges.max = 2.0f;
            p.ranges.def = 2.0f;
            setEnum(p, { "AT (1 diode)", "Off (lift)", "1.5 (2 diodes)" });
            break;
        case kParamModel:
            p.hints |= kParameterIsInteger;
            p.name = "MODEL"; p.symbol = "model";
            p.ranges.max = 1.0f;
            p.ranges.def = 0.0f;
            setEnum(p, { "Guitar", "Bass" });
            break;
        case kParamBypass:
            p.initDesignation(kParameterDesignationBypass);
            break;
        }

        if (index < kParamBypass)
            p.ranges.def = preset_data::kDefaults.value[index];
    }

    void initProgramName(const uint32_t index, String& name) override
    {
        if (index < preset_data::kCount)
            name = preset_data::kEntries[index].name;
    }

    void loadProgram(const uint32_t index) override
    {
        if (index >= preset_data::kCount)
            return;
        for (uint32_t i = 0; i < kParamBypass; ++i)
            fParams[i] = preset_data::kEntries[index].value[i];
    }

    float getParameterValue(const uint32_t index) const override
    {
        return index < kParamCount ? fParams[index] : 0.0f;
    }

    void setParameterValue(const uint32_t index, const float value) override
    {
        if (index < kParamCount)
            fParams[index] = value;
    }

    void activate() override
    {
        if (fDsp->activate_plugin != nullptr)
            fDsp->activate_plugin(true, fDsp);
        fWet = fParams[kParamBypass] > 0.5f ? 0.0f : 1.0f;
    }

    void deactivate() override
    {
        if (fDsp->activate_plugin != nullptr)
            fDsp->activate_plugin(false, fDsp);
    }

    void bufferSizeChanged(const uint32_t newBufferSize) override
    {
        fDry.resize(newBufferSize);
    }

    void sampleRateChanged(const double newSampleRate) override
    {
        fDsp->set_samplerate(static_cast<uint32_t>(newSampleRate), fDsp);
        fFadeStep = 1.0f / (0.010f * static_cast<float>(newSampleRate));
    }

    void run(const float** inputs, float** outputs, const uint32_t frames) override
    {
        AVOIDDENORMALS();

        const float* const in = inputs[0];
        float* const out = outputs[0];
        if (frames == 0)
            return;

        const float target = fParams[kParamBypass] > 0.5f ? 0.0f : 1.0f;
        const bool fading = fWet != target;

        // keep the DSP running while bypassed so that re-engaging is seamless
        if (fading || target == 0.0f)
            std::memcpy(fDry.data(), in, sizeof(float) * frames);

        fDsp->mono_audio(static_cast<int>(frames), const_cast<float*>(in), out, fDsp);

        if (!fading && target == 1.0f)
            return;
        if (!fading)
        {
            std::memcpy(out, fDry.data(), sizeof(float) * frames);
            return;
        }
        for (uint32_t i = 0; i < frames; ++i)
        {
            if (fWet < target)
                fWet = std::min(target, fWet + fFadeStep);
            else if (fWet > target)
                fWet = std::max(target, fWet - fFadeStep);
            out[i] = fDry[i] + fWet * (out[i] - fDry[i]);
        }
    }

private:
    PluginLV2* const fDsp;
    float fParams[kParamCount];
    std::vector<float> fDry;
    float fWet = 1.0f;
    float fFadeStep = 0.002f;

    static void setEnum(Parameter& p, std::initializer_list<const char*> labels)
    {
        const uint32_t n = static_cast<uint32_t>(labels.size());
        ParameterEnumerationValue* const values = new ParameterEnumerationValue[n];
        uint32_t i = 0;
        for (const char* label : labels)
        {
            values[i].label = label;
            values[i].value = static_cast<float>(i);
            ++i;
        }
        p.enumValues.count = n;
        p.enumValues.restrictedMode = true;
        p.enumValues.values = values;
    }

    DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginBluesFuse)
};

Plugin* createPlugin()
{
    return new PluginBluesFuse();
}

END_NAMESPACE_DISTRHO
