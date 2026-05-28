#pragma once
#include "dsp/Phaser.h"
#include "dsp/wavetable.h"
#include "dsp/wavetableOSC.h"
#include "dsp/LadderFilter.h"
#include "dsp/Chorus.h"
#include "rack.hpp"

struct Nebula : rack::Module
{
    enum ParamId
    {
        PITCH_A_PARAM, FINE_A_PARAM,

        MORPH_A_PARAM, VOLUME_A_PARAM,
        SUB_LEVEL_A_PARAM, SUB_OCTAVE_A_PARAM,

        PRESET_A_PARAM, CUTOFF_A_B_PARAM, RES_A_B_PARAM,

        PM_AMOUNT_PARAM,
        PM_SCALE_PARAM,
        PM_DIRECTION_PARAM,

        PITCH_B_PARAM, FINE_B_PARAM,
        MORPH_B_PARAM, VOLUME_B_PARAM, SUB_LEVEL_B_PARAM,
        SUB_OCTAVE_B_PARAM, PRESET_B_PARAM,

        CHORUS_MIX_PARAM,
        CHORUS_RATE_A_PARAM,
        CHORUS_DEPTH_A_PARAM,

        CHORUS_RATE_B_PARAM,
        CHORUS_DEPTH_B_PARAM,

        //Phaser Bank A und B
        PHASER_RATE_A_PARAM,
        PHASER_DEPTH_A_PARAM,
        PHASER_FB_A_PARAM,

        PHASER_RATE_B_PARAM,
        PHASER_DEPTH_B_PARAM,
        PHASER_FB_B_PARAM,

        //Dry Wet Enums:
        PHASER_MIX_PARAM_A,
        CHORUS_MIX_PARAM_A,

        PHASER_MIX_PARAM_B,
        CHORUS_MIX_PARAM_B,

        SYNC_A_B_PARAM,

        CROSS_SPILL_PARAM,

        UNISON_A_PARAM,
        UNISON_B_PARAM,
        SPREAD_A_PARAM,
        SPREAD_B_PARAM,

        PARAMS_LEN
    };

    enum InputId
    {
        MORPH_A_CV_INPUT,
        MORPH_B_CV_INPUT,
        CUTOFF_CV_INPUT,
        PM_AMOUNT_CV_INPUT,
        CHORUS_MIX_CV_INPUT,
        PHASER_MIX_CV_INPUT,
        MOD_CV_INPUT,
        PITCH_A_CV_INPUT,
        PITCH_B_CV_INPUT,
        VOLUME_A_CV_INPUT,
        VOLUME_B_CV_INPUT,
        PHASER_LFO_A_INPUT,
        PHASER_LFO_B_INPUT,
        INPUTS_LEN
    };

    enum OutputId { AUDIO_LEFT_OUTPUT, AUDIO_RIGHT_OUTPUT, OUTPUTS_LEN };

    enum LightId { ADDITIVE_A_LIGHT, WAV_A_LIGHT, ADDITIVE_B_LIGHT, WAV_B_LIGHT, LIGHTS_LEN };

    Wavetable wavetableA, wavetableB;
    static constexpr int MAX_UNISON = 4;
    std::array<WavetableOsc, MAX_UNISON> mainVoicesA, subVoicesA;
    std::array<WavetableOsc, MAX_UNISON> mainVoicesB, subVoicesB;

    LadderFilter filterA, filterB;
    Phaser phaserA, phaserB;
    Chorus chorusA, chorusB;

    float lastSampleA = 0.f, lastSampleB = 0.f;
    float morphASmoothed = 0.f, morphBSmoothed = 0.f;
    float syncPhaseSmoothed = 0.f;
    float freqASmoothed = 110.f, freqBSmoothed = 110.f;
    int lastPresetA = -1, lastPresetB = -1;
    bool wavLoadedA = false, wavLoadedB = false;


    Nebula();
    void onSampleRateChange() override;
    void process(const ProcessArgs& args) override;

private:
    float processVoices(float freq, float morph, Wavetable& wt,
                        std::array<WavetableOsc, 4>& voices, int numVoices, float spread,
                        WavetableOsc& subOsc, float subLevel, float freqSub,
                        float pmOffset, float volume, float sampleTime);
};

struct NebulaWidget : rack::ModuleWidget
{
    NebulaWidget(Nebula* module);
    void appendContextMenu(rack::Menu* menu) override;
};
