#include "Nebula.h"
#include <iostream>

Nebula::Nebula() {
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    configParam(PITCH_A_PARAM, -3.f, 3.f, 0.f, "Pitch A", " Oct");
    configParam(FINE_A_PARAM, -100.f, 100.f, 0.f, "Fine A", " cents");
    configParam(MORPH_A_PARAM, 0.f, 1.f, 0.f, "Morph A");
    configParam(VOLUME_A_PARAM, 0.f, 1.f, 0.8f, "Volume A");
    configParam(SUB_OCTAVE_A_PARAM, 0.f, 1.f, 0.f, "Sub Octave A");
    configParam(PRESET_A_PARAM, 0.f, 1.f, 0.f, "Preset A");
    configParam(SUB_LEVEL_A_PARAM, 0.f, 1.f, 0.f, "Sub Level A");

    configParam(PITCH_B_PARAM, -3.f, 3.f, 1.f, "Pitch B", " Oct");
    configParam(FINE_B_PARAM, -100.f, 100.f, 0.f, "Fine B", " cents");
    configParam(MORPH_B_PARAM, 0.f, 1.f, 0.f, "Morph B");
    configParam(VOLUME_B_PARAM, 0.f, 1.f, 0.8f, "Volume B");
    configParam(SUB_OCTAVE_B_PARAM, 0.f, 1.f, 0.f, "Sub Octave B");
    configParam(PRESET_B_PARAM, 0.f, 1.f, 0.f, "Preset B");
    configParam(SUB_LEVEL_B_PARAM, 0.f, 1.f, 0.f, "Sub Level B");

    configParam(CUTOFF_A_B_PARAM, 3.f, 9.9f, 9.9f, "Cutoff", " Hz", M_E);
    configParam(RES_A_B_PARAM, 0.f, 1.f, 0.707f, "Resonance");
    configParam(PM_AMOUNT_PARAM, 0.f, 1.f, 0.f, "PM Amount", " %");
    configParam(PM_DIRECTION_PARAM, 0.f, 2.f, 0.f, "PM Direction", " %");

    //Phaser BAnk A und B
    configParam(PHASER_MOD_A_PARAM, 0.f, 1.f, 0.5f, "Phaser Mod A", " %", 0, 100);
    auto* pqStagesA = configParam(PHASER_STAGES_A_PARAM, 1.f, 8.f, 4.f, "Phaser Stages A");
    pqStagesA->snapEnabled = true;

    configParam(PHASER_SKEW_A_PARAM, -1.f, 1.f, 0.f, "Phaser Skew A");

    configParam(PHASER_MOD_B_PARAM, 0.f, 1.f, 0.5f, "Phaser Mod B", " %", 0, 100);
    configParam(PHASER_MOD_A_PARAM, 0.f, 1.f, 0.5f, "Phaser Mod A", " %", 0, 100);
    auto* pqStagesB = configParam(PHASER_STAGES_A_PARAM, 1.f, 8.f, 4.f, "Phaser Stages B");
    configParam(PHASER_SKEW_B_PARAM, -1.f, 1.f, 0.f, "Phaser Skew B");

    configInput(PHASER_CV_A_INPUT, "Phaser CV A");
    configInput(PHASER_CV_B_INPUT, "Phaser CV B");


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

}

void Nebula::process(const ProcessArgs& args) {

    phaserA.setSampleRate(APP->engine->getSampleRate());
    phaserB.setSampleRate(APP->engine->getSampleRate());

    int pmDirection = (int)params[PM_DIRECTION_PARAM].getValue();
    float pmAmount = params[PM_AMOUNT_PARAM].getValue();
    float pmOffsetA = 0.f, pmOffsetB = 0.f;
    float alpha = 1.f - std::exp(-args.sampleTime / 0.005f);
    const float pmScale = 10.f;

    int presetA = (int)params[PRESET_A_PARAM].getValue();
    lights[ADDITIVE_A_LIGHT].setBrightness(presetA == 0 ? 1.f : 0.f);
    lights[WAV_A_LIGHT].setBrightness(presetA == 1 ? 1.f : 0.f);

    if (presetA != lastPresetA) {
        if (presetA == 0) {
            std::cout << "Bank A: Switching to Additive (Generate 1)" << std::endl;
            wavetableA.generateBasic(0);
            wavetableA.currentMode = 0;
            wavLoadedA = false;
        } else {
            std::cout << "Bank A: Switching to WAV mode" << std::endl;
            wavetableA.currentMode = 1;
        }
        lastPresetA = presetA;
    }

    // Pitch = Octave (-3..+3) + Fine (-100..+100 cents)
    float octaveA = params[PITCH_A_PARAM].getValue();
    float fineA   = params[FINE_A_PARAM].getValue();
    float pitchA  = 110.f * std::pow(2.f, octaveA + fineA / 1200.f);


    float morphATarget = params[MORPH_A_PARAM].getValue();
    morphATarget = clamp(morphATarget, 0.f, 1.f);
    morphASmoothed += (morphATarget - morphASmoothed) * alpha;
    freqASmoothed += (pitchA - freqASmoothed) * alpha;

    int presetB = (int)params[PRESET_B_PARAM].getValue();
    lights[ADDITIVE_B_LIGHT].setBrightness(presetB == 0 ? 1.f : 0.f);
    lights[WAV_B_LIGHT].setBrightness(presetB == 1 ? 1.f : 0.f);

    if (presetB != lastPresetB) {
        if (presetB == 0) {
            std::cout << "Bank B: Switching to Additive (Generate 2)" << std::endl;
            wavetableB.generateBasic(1);
            wavetableB.currentMode = 0;
            wavLoadedB = false;
        } else {
            std::cout << "Bank B: Switching to WAV mode" << std::endl;
            wavetableB.currentMode = 1;
        }
        lastPresetB = presetB;
    }

    float octaveB = params[PITCH_B_PARAM].getValue();
    float fineB   = params[FINE_B_PARAM].getValue();
    float pitchB  = 110.f * std::pow(2.f, octaveB + fineB / 1200.f);

    float cvBMorph = params[MORPH_B_CV_INPUT].getValue();

    float morphBTarget = params[MORPH_B_PARAM].getValue();
    morphBTarget = clamp(morphBTarget, 0.f, 1.f);
    morphBSmoothed += (morphBTarget - morphBSmoothed) * alpha;
    freqBSmoothed += (pitchB - freqBSmoothed) * alpha;


    float sampleA;
    float sampleB;

    if (pmDirection == 0) {
        sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed, 0.f);
        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
        if (subLevelA > 0.f) {
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);
            float subSampleA = subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed, 0.f);
            sampleA += subSampleA * subLevelA;
        }
        sampleA *= params[VOLUME_A_PARAM].getValue();
        float pmSourceA = sampleA;
        pmOffsetB = pmSourceA * pmAmount * pmScale;
        sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB);
        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
        if (subLevelB > 0.f) {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();
            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);
            float subSampleB = subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB);
            sampleB += subSampleB * subLevelB;
        }
        sampleB *= params[VOLUME_B_PARAM].getValue();
    } else if (pmDirection == 1) {
        sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed, 0.f);
        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
        if (subLevelB > 0.f) {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();
            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);
            float subSampleB = subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed, 0.f);
            sampleB += subSampleB * subLevelB;
        }
        sampleB *= params[VOLUME_B_PARAM].getValue();
        float pmSourceB = sampleB;
        pmOffsetA = pmSourceB * pmAmount * pmScale;
        sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA);
        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
        if (subLevelA > 0.f) {
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);
            float subSampleA = subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA);
            sampleA += subSampleA * subLevelA;
        }
        sampleA *= params[VOLUME_A_PARAM].getValue();
    } else {
        pmOffsetA = lastSampleA * pmAmount * pmScale;
        pmOffsetB = lastSampleB * pmAmount * pmScale;
        sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA);
        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
        if (subLevelA > 0.f) {
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);
            float subSampleA = subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed, pmOffsetA);
            sampleA += subSampleA * subLevelA;
        }
        sampleA *= params[VOLUME_A_PARAM].getValue();
        sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB);
        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
        if (subLevelB > 0.f) {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();
            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);
            float subSampleB = subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed, pmOffsetB);
            sampleB += subSampleB * subLevelB;
        }
        sampleB *= params[VOLUME_B_PARAM].getValue();
    }

    float cutoffParam = params[CUTOFF_A_B_PARAM].getValue();
    float resParam = params[RES_A_B_PARAM].getValue();
    float cutoffHz = std::exp(cutoffParam);
    filterA.setCutoff(cutoffHz);
    filterA.setResonanz(resParam);
    filterB.setCutoff(cutoffHz);
    filterB.setResonanz(resParam);

    float filteredA = filterA.processLp(sampleA);
    float filteredB = filterB.processLp(sampleB);

    // === PHASER A ===
    phaserA.setModulation(params[PHASER_MOD_A_PARAM].getValue());
    phaserA.setStages(params[PHASER_STAGES_A_PARAM].getValue());
    phaserA.setSkew(params[PHASER_SKEW_A_PARAM].getValue());

    float cvA = inputs[PHASER_CV_A_INPUT].isConnected() ?
             inputs[PHASER_CV_A_INPUT].getVoltage() / 5.f : 0.f;
    // Clamp zu -1..+1 falls LFO out-of-range Spannung liefert
    if (cvA < -1.f) cvA = -1.f;
    if (cvA > 1.f) cvA = 1.f;

    float phaserOutA;
    phaserA.process(filteredA, cvA, phaserOutA);

    // === PHASER B ===
    phaserB.setModulation(params[PHASER_MOD_B_PARAM].getValue());
    phaserA.setStages(params[PHASER_STAGES_B_PARAM].getValue());
    phaserB.setSkew(params[PHASER_SKEW_B_PARAM].getValue());

    float cvBphaser = inputs[PHASER_CV_B_INPUT].isConnected() ?
            inputs[PHASER_CV_B_INPUT].getVoltage() / 5.f : 0.f;
    // Clamp zu -1..+1 falls LFO out-of-range Spannung liefert
    if (cvBphaser < -1.f) cvA = -1.f;
    if (cvBphaser > 1.f) cvA = 1.f;

    float phaserOutB;

    phaserB.process(filteredB, cvBphaser, phaserOutB);

    // === Output ===
    outputs[AUDIO_LEFT_OUTPUT].setVoltage(phaserOutA * 5.f);
    outputs[AUDIO_RIGHT_OUTPUT].setVoltage(phaserOutB * 5.f);

    lastSampleA = sampleA;
    lastSampleB = sampleB;
}
