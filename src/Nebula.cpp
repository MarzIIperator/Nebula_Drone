#include "Nebula.h"
#include <iostream>
#include <cmath>

Nebula::Nebula() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    //Bank A
    configParam(PITCH_A_PARAM, -3.f, 3.f, 0.f, "Pitch A", " Oct");
    configParam(FINE_A_PARAM, -100.f, 100.f, 0.f, "Fine A", " cents");
    configParam(MORPH_A_PARAM, 0.f, 1.f, 0.f, "Morph A");
    configParam(VOLUME_A_PARAM, 0.f, 1.f, 0.8f, "Volume A");
    configParam(SUB_OCTAVE_A_PARAM, 0.f, 1.f, 0.f, "Sub Octave A");
    configParam(PRESET_A_PARAM, 0.f, 1.f, 0.f, "Preset A");
    configParam(SUB_LEVEL_A_PARAM, 0.f, 1.f, 0.f, "Sub Level A");

    //Bank B
    configParam(PITCH_B_PARAM, -3.f, 3.f, 1.f, "Pitch B", " Oct");
    configParam(FINE_B_PARAM, -100.f, 100.f, 0.f, "Fine B", " cents");
    configParam(MORPH_B_PARAM, 0.f, 1.f, 0.f, "Morph B");
    configParam(VOLUME_B_PARAM, 0.f, 1.f, 0.8f, "Volume B");
    configParam(SUB_OCTAVE_B_PARAM, 0.f, 1.f, 0.f, "Sub Octave B");
    configParam(PRESET_B_PARAM, 0.f, 1.f, 0.f, "Preset B");
    configParam(SUB_LEVEL_B_PARAM, 0.f, 1.f, 0.f, "Sub Level B");

    //Filter A und B
    configParam(CUTOFF_A_B_PARAM, 3.f, 9.9f, 9.9f, "Cutoff", " Hz", M_E);
    configParam(RES_A_B_PARAM, 0.f, 1.f, 0.707f, "Resonance");

    //Pm Bank A und B
    configParam(PM_AMOUNT_PARAM, 0.f, 1.f, 0.f, "PM Amount", " %");
    configParam(PM_DIRECTION_PARAM, 0.f, 2.f, 0.f, "PM Direction", " %");

    //Phaser Bank A und B
    configParam(PHASER_DEPTH_A_PARAM, 0.f, 1.f, 0.5f, "Phaser Depth A", " %", 0, 100);
    configParam(PHASER_RATE_A_PARAM, 0.02f, 1.f, 0.02f, "Phaser Rate A");
    configParam(PHASER_MIX_PARAM_A, 0.f, 1.f, 0.5f, "Phaser Mix A", " %", 0, 100);
    configParam(PHASER_FB_A_PARAM, 0.f, 0.95f, 0.3f, "Phaser Feedback A");
    configInput(PHASER_LFO_A_INPUT, "Phaser LFO A");

    configParam(PHASER_DEPTH_B_PARAM, 0.f, 1.f, 0.5f, "Phaser Depth B", " %", 0, 100);
    configParam(PHASER_RATE_B_PARAM, 0.02f, 1.f, 0.02f, "Phaser Rate B", " Hz");
    configParam(PHASER_MIX_PARAM_B, 0.f, 1.f, 0.5f, "Phaser Mix B", " %", 0, 100);
    configParam(PHASER_FB_B_PARAM, 0.f, 0.95f, 0.3f, "Phaser Feedback B");
    configInput(PHASER_LFO_B_INPUT, "Phaser LFO B");


    configOutput(AUDIO_LEFT_OUTPUT, "Left");
    configOutput(AUDIO_RIGHT_OUTPUT, "Right");

    std::cout << "=== NEBULA: Init ===" << std::endl;
    wavetableA.generateBasic(0);
    wavetableB.generateBasic(1);
    std::cout << "Bank A: Generate 1 loaded" << std::endl;
    std::cout << "Bank B: Generate 2 loaded" << std::endl;

}

void Nebula::onSampleRateChange() {
        filterA.setSampleRate(APP->engine->getSampleRate());
        filterB.setSampleRate(APP->engine->getSampleRate());

        phaserA.setSampleRate(APP->engine->getSampleRate());
        phaserB.setSampleRate(APP->engine->getSampleRate());

        chorusA.setSampleRate(APP->engine->getSampleRate());
        chorusB.setSampleRate(APP->engine->getSampleRate());

}

void Nebula::process(const ProcessArgs& args)
{
    int pmDirection = (int)params[PM_DIRECTION_PARAM].getValue();
    float pmAmount = params[PM_AMOUNT_PARAM].getValue();
    float pmOffsetA = 0.f, pmOffsetB = 0.f;
    float pmScale = params[PM_SCALE_PARAM].getValue();

    float alpha = 1.f - std::exp(-args.sampleTime / 0.005f);

// ===== BANK A =====
    int presetA = (int)params[PRESET_A_PARAM].getValue();
    lights[ADDITIVE_A_LIGHT].setBrightness(presetA == 0 ? 1.f : 0.f);
    lights[WAV_A_LIGHT].setBrightness(presetA == 1 ? 1.f : 0.f);

    if (presetA != lastPresetA) {
        if (presetA == 0) {
            wavetableA.generateBasic(0);
            wavetableA.currentMode = 0;
            wavLoadedA = false;
        } else {
            wavetableA.currentMode = 1;
        }
        lastPresetA = presetA;
    }

    // Pitch A
    float octaveA = params[PITCH_A_PARAM].getValue();
    float fineA   = params[FINE_A_PARAM].getValue();
    float pitchA  = 110.f * std::pow(2.f, octaveA + fineA / 1200.f);
    freqASmoothed  += (pitchA - freqASmoothed) * alpha;

    // Morph A (Regler + CV Input)
    float morphParamA = params[MORPH_A_PARAM].getValue();
    if (inputs[MORPH_A_CV_INPUT].isConnected()) {
        
        morphParamA += inputs[MORPH_A_CV_INPUT].getVoltage() * 0.1f;
    }
    float morphATarget = rack::clamp(morphParamA, 0.f, 1.f);
    morphASmoothed += (morphATarget - morphASmoothed) * alpha;


    // ===== BANK B =====
    int presetB = (int)params[PRESET_B_PARAM].getValue();
    lights[ADDITIVE_B_LIGHT].setBrightness(presetB == 0 ? 1.f : 0.f);
    lights[WAV_B_LIGHT].setBrightness(presetB == 1 ? 1.f : 0.f);

    if (presetB != lastPresetB) {
        if (presetB == 0) {
            wavetableB.generateBasic(1);
            wavetableB.currentMode = 0;
            wavLoadedB = false;
        } else {
            wavetableB.currentMode = 1;
        }
        lastPresetB = presetB;
    }

    // Pitch B
    float octaveB = params[PITCH_B_PARAM].getValue();
    float fineB   = params[FINE_B_PARAM].getValue();
    float pitchB  = 110.f * std::pow(2.f, octaveB + fineB / 1200.f);
    freqBSmoothed  += (pitchB - freqBSmoothed) * alpha;

    // Morph B (Regler + CV Input)
    float morphParamB = params[MORPH_B_PARAM].getValue();
    if (inputs[MORPH_B_CV_INPUT].isConnected()) {

        morphParamB += inputs[MORPH_B_CV_INPUT].getVoltage() * 0.1f;
    }
    float morphBTarget = rack::clamp(morphParamB, 0.f, 1.f);
    morphBSmoothed += (morphBTarget - morphBSmoothed) * alpha;

    // ===== Phasen-Modulation Routing =====
    float sampleA, sampleB;
    if (pmDirection == 0) {
        // A → B
        sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed, 0.f);
        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
        if (subLevelA > 0.f) {
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);
            sampleA += subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed, 0.f) * subLevelA;
        }
        sampleA *= params[VOLUME_A_PARAM].getValue();

        pmOffsetB = sampleA * pmAmount * pmScale;

        sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB);
        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
        if (subLevelB > 0.f) {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();
            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);
            sampleB += subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB) * subLevelB;
        }
        sampleB *= params[VOLUME_B_PARAM].getValue();

    } else if (pmDirection == 1) {
        // B → A
        sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed, 0.f);
        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
        if (subLevelB > 0.f) {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();
            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);
            sampleB += subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed, 0.f) * subLevelB;
        }
        sampleB *= params[VOLUME_B_PARAM].getValue();

        pmOffsetA = sampleB * pmAmount * pmScale;

        sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA);
        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
        if (subLevelA > 0.f) {
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);
            sampleA += subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA) * subLevelA;
        }
        sampleA *= params[VOLUME_A_PARAM].getValue();

    } else {
        // A ↔ B mit 1-Sample Delay
        pmOffsetA = lastSampleA * pmAmount * pmScale;
        pmOffsetB = lastSampleB * pmAmount * pmScale;

        sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA);
        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
        if (subLevelA > 0.f) {
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);
            sampleA += subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA) * subLevelA;
        }
        sampleA *= params[VOLUME_A_PARAM].getValue();

        sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB);
        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
        if (subLevelB > 0.f) {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();
            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);
            sampleB += subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB) * subLevelB;
        }
        sampleB *= params[VOLUME_B_PARAM].getValue();
    }

    // ===== Filter =====
    float cutoffHz = std::exp(params[CUTOFF_A_B_PARAM].getValue());
    float resParam = params[RES_A_B_PARAM].getValue();
    filterA.setCutoff(cutoffHz);
    filterA.setResonanz(resParam);
    filterB.setCutoff(cutoffHz);
    filterB.setResonanz(resParam);

    float filteredA = filterA.processLp(sampleA);
    float filteredB = filterB.processLp(sampleB);

    // ===== Phaser pro Bank =====
    phaserA.setFeedback(params[PHASER_FB_A_PARAM].getValue());
    phaserA.setRate(params[PHASER_RATE_A_PARAM].getValue());
    phaserA.setDepth(params[PHASER_DEPTH_A_PARAM].getValue());
    phaserA.setMix(params[PHASER_MIX_PARAM_A].getValue());

    phaserB.setFeedback(params[PHASER_FB_B_PARAM].getValue());
    phaserB.setRate(params[PHASER_RATE_B_PARAM].getValue());
    phaserB.setDepth(params[PHASER_DEPTH_B_PARAM].getValue());
    phaserB.setMix(params[PHASER_MIX_PARAM_B].getValue());

    float phaserOutA;
    if (inputs[PHASER_LFO_A_INPUT].isConnected()) {
        float cv = inputs[PHASER_LFO_A_INPUT].getVoltage();
        float lfoVal = rack::clamp(cv / 10.f + 0.5f, 0.f, 1.f);
        phaserOutA = phaserA.processExternalLFO(filteredA, lfoVal);
    } else {
        phaserOutA = phaserA.process(filteredA);
    }

    float phaserOutB;
    if (inputs[PHASER_LFO_B_INPUT].isConnected()) {
        float cv = inputs[PHASER_LFO_B_INPUT].getVoltage();
        float lfoVal = rack::clamp(cv / 10.f + 0.5f, 0.f, 1.f);
        phaserOutB = phaserB.processExternalLFO(filteredB, lfoVal);
    } else {
        phaserOutB = phaserB.process(filteredB);
    }

    // ===== Chorus pro Bank =====

    chorusA.setRate(params[CHORUS_RATE_A_PARAM].getValue());
    chorusA.setDepth(params[CHORUS_DEPTH_A_PARAM].getValue());
    chorusA.setMix(params[CHORUS_MIX_PARAM_A].getValue());

    chorusB.setRate(params[CHORUS_RATE_B_PARAM].getValue());
    chorusB.setDepth(params[CHORUS_DEPTH_B_PARAM].getValue());
    chorusB.setMix(params[CHORUS_MIX_PARAM_B].getValue());


    auto chorusOutA = chorusA.process(phaserOutA);
    auto chorusOutB = chorusB.process(phaserOutB);

    // ===== Output Mischen ( Stereo Diffusion) =====

    float finalLeft  = chorusOutA.l + chorusOutB.l;
    float finalRight = chorusOutA.r + chorusOutB.r;


    float outputGain = 5.f * 0.5f;

    outputs[AUDIO_LEFT_OUTPUT].setVoltage(finalLeft * outputGain);
    outputs[AUDIO_RIGHT_OUTPUT].setVoltage(finalRight * outputGain);

    lastSampleA = sampleA;
    lastSampleB = sampleB;
}