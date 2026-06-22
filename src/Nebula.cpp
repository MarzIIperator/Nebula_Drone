#include "Nebula.h"
#include <iostream>
#include <cmath>

Nebula::Nebula()
{
    config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

    //Bank A
    configParam(PITCH_A_PARAM, -3.f, 3.f, 0.f, "Pitch A", " Oct");
    configParam(FINE_A_PARAM, -100.f, 100.f, 0.f, "Fine A", " cents");
    configParam(MORPH_A_PARAM, 0.f, 1.f, 0.25f, "Morph A");
    configParam(VOLUME_A_PARAM, 0.f, 1.f, 0.8f, "Volume A");
    configParam(SUB_OCTAVE_A_PARAM, 0.f, 1.f, 0.f, "Sub Octave A");
    configParam(PRESET_A_PARAM, 0.f, 1.f, 0.f, "Preset A");
    configParam(SUB_LEVEL_A_PARAM, 0.f, 1.f, 0.5f, "Sub Level A");

    //Bank B
    configParam(PITCH_B_PARAM, -3.f, 3.f, 1.f, "Pitch B", " Oct");
    configParam(FINE_B_PARAM, -100.f, 100.f, 0.f, "Fine B", " cents");
    configParam(MORPH_B_PARAM, 0.f, 1.f, 0.25f, "Morph B");
    configParam(VOLUME_B_PARAM, 0.f, 1.f, 0.8f, "Volume B");
    configParam(SUB_OCTAVE_B_PARAM, 0.f, 1.f, 0.f, "Sub Octave B");
    configParam(PRESET_B_PARAM, 0.f, 1.f, 0.f, "Preset B");
    configParam(SUB_LEVEL_B_PARAM, 0.f, 1.f, 0.5f, "Sub Level B");

    //Filter A und B
    configParam(CUTOFF_A_B_PARAM, 3.f, 9.9f, 4.6f, "Cutoff", " Hz", M_E);
    configParam(RES_A_B_PARAM, 0.f, 1.f, 0.2f, "Resonance");

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

    // Chorus Bank A und B
    configParam(CHORUS_RATE_A_PARAM, 0.01f, 5.f, 1.f, "Chorus Rate A", " Hz");
    configParam(CHORUS_DEPTH_A_PARAM, 0.f, 1.f, 0.2f, "Chorus Depth A");
    configParam(CHORUS_MIX_PARAM_A, 0.f, 1.f, 0.f, "Chorus Mix A", " %", 0, 100);

    configParam(CHORUS_RATE_B_PARAM, 0.01f, 5.f, 1.f, "Chorus Rate B", " Hz");
    configParam(CHORUS_DEPTH_B_PARAM, 0.f, 1.f, 0.2f, "Chorus Depth B");
    configParam(CHORUS_MIX_PARAM_B, 0.f, 1.f, 0.f, "Chorus Mix B", " %", 0, 100);

    configParam(PM_SCALE_PARAM, 0.f, 5.f, 0.f, "PM Scale", " %");

    configParam(SYNC_A_B_PARAM, 0.f, 1.f, 0.f, "Sync A/B");

    configParam(VOICES_A_B_PARAM, 0.f, 1.f, 0.5f, "Voices A/B");

    configParam(CROSS_SPILL_PARAM, 0.f, 1.f, 0.5f, " CROSS SPILL", " %", 0, 100);

    configOutput(AUDIO_LEFT_OUTPUT, "Left");
    configOutput(AUDIO_RIGHT_OUTPUT, "Right");

    std::cout << "=== NEBULA: Init ===" << std::endl;
    wavetableA.generateBasic(0);
    wavetableB.generateBasic(1);
    std::cout << "Bank A: Generate 1 loaded" << std::endl;
    std::cout << "Bank B: Generate 2 loaded" << std::endl;

    for (int i = 0; i < MAIN_VOICES; i++)
    {
        mainVoicesA[i].reset(rack::random::uniform());
        mainVoicesB[i].reset(rack::random::uniform());
    }
}
void Nebula::onSampleRateChange()
{
    filterA.setSampleRate(APP->engine->getSampleRate());
    filterB.setSampleRate(APP->engine->getSampleRate());

    phaserA.setSampleRate(APP->engine->getSampleRate());
    phaserB.setSampleRate(APP->engine->getSampleRate());

    chorusA.setSampleRate(APP->engine->getSampleRate());
    chorusB.setSampleRate(APP->engine->getSampleRate());
}

// ---------------------------------------------------------------------------
// processVoices() — Mehrstimmiges Unisono + Detune + Drift
//
// Rendert eine Bank (A oder B) über bis zu 3 verstimmte Wavetable-Oszillatoren.
//
// Stimmen-Aufteilung (asymmetrisch, simuliert analoge Oszillator-Ungenauigkeiten):
//   Voice 0:  −15 ct Detune, 0,65× Gain, 0,00 Phasen-Offset, −0,12 Morph-Delta
//   Voice 1:    0 ct Detune, 0,50× Gain, 0,33 Phasen-Offset,  0,00 Morph-Delta
//   Voice 2:  +15 ct Detune, 0,65× Gain, 0,66 Phasen-Offset, +0,15 Morph-Delta
// ---------------------------------------------------------------------------
float Nebula::processVoices(float freq, float morph, Wavetable& wt, std::array<WavetableOsc, 3>& voices, float pmOffset, float sampleTime, float* drift)
{
    // CKSS-Schalter: Position 0 → 3 Stimmen (Unisono), Position 1 → 1 Stimme
    int voiceCount = (params[VOICES_A_B_PARAM].getValue() < 0.5f) ? 3 : 1;

    if (voiceCount == 1)
        return voices[0].process(freq, sampleTime, wt, morph, pmOffset);

    // Pro-Stimme Voreinstellungen: Detune (Cent), Gain, Phasen-Offset, Morph-Delta.
    const float detCents[3] = { -15.f, 0.f, +15.f };
    const float gain[3]     = { 0.65f, 0.5f, 0.65f };
    const float phOff[3]    = { 0.00f, 0.33f, 0.66f };
    const float mDelta[3]   = { -0.12f, 0.f, +0.15f };

    float sample = 0.f;
    for (int v = 0; v < voiceCount; v++) {
        // Static detune + drift (random walk, ±5 ct).
        float totalDetune = detCents[v] + drift[v];
        // Cent → frequency:  f × 2^(cents/1200)
        float freqV   = freq * std::pow(2.f, totalDetune / 1200.f);

        // Timbre variation: each voice reads a slightly different wavetable frame.
        float morphV  = rack::clamp(morph + mDelta[v], 0.f, 1.f);

        // Phase offset: fixed per-voice + global PM offset from cross-modulation.
        float phaseOff = phOff[v] + pmOffset;

        sample += voices[v].process(freqV, sampleTime, wt, morphV, phaseOff) * gain[v];
    }

    // RMS normalisation: sum of squared gains = 0.65² + 0.5² + 0.65² = 1.095
    //   √1.095 ≈ 1.046.  We use √1.8 as an empirical loudness match to single-voice mode.
    return sample / std::sqrt(1.8f);
}

void Nebula::process(const ProcessArgs& args)
{
    // ======================================================================
    // Signalfluss pro Sample:
    //   Parameter lesen → Bank A + B (Pitch, Morph, Voices, Sub)
    //   → PM Routing (A↔B) → Ladder Filter → Phaser A/B → Chorus A/B
    //   → Volume A/B (nach Effekten!) → Stereo-Mix → Output L/R
    // ======================================================================

    int pmDirection = (int)params[PM_DIRECTION_PARAM].getValue();
    float pmAmount = params[PM_AMOUNT_PARAM].getValue();
    float pmOffsetA = 0.f, pmOffsetB = 0.f;
    float pmScale = params[PM_SCALE_PARAM].getValue();

    float alpha = 1.f - std::exp(-args.sampleTime / 0.005f);

    // ===== BANK A =====
    int presetA = (int)params[PRESET_A_PARAM].getValue();
    lights[ADDITIVE_A_LIGHT].setBrightness(presetA == 0 ? 1.f : 0.f);
    lights[WAV_A_LIGHT].setBrightness(presetA == 1 ? 1.f : 0.f);

    if (presetA != lastPresetA)
    {
        if (presetA == 0)
        {
            wavetableA.generateBasic(0);
            wavetableA.currentMode = 0;
            wavLoadedA = false;
        }
        else
        {
            wavetableA.currentMode = 1;
        }
        lastPresetA = presetA;
    }

    // Pitch A
    float octaveA = params[PITCH_A_PARAM].getValue();
    float fineA = params[FINE_A_PARAM].getValue();

    if (inputs[PITCH_A_CV_INPUT].isConnected())
    {
        octaveA += inputs[PITCH_A_CV_INPUT].getVoltage() * 1.f;
    }

    float pitchA = 110.f * std::pow(2.f, octaveA + fineA / 1200.f);
    freqASmoothed += (pitchA - freqASmoothed) * alpha;

    // Morph A (Regler + CV Input)
    float morphParamA = params[MORPH_A_PARAM].getValue();
    if (inputs[MORPH_A_CV_INPUT].isConnected())
    {
        morphParamA += inputs[MORPH_A_CV_INPUT].getVoltage() * 0.2f;
    }
    float morphATarget = rack::clamp(morphParamA, 0.f, 1.f);
    morphASmoothed += (morphATarget - morphASmoothed) * alpha;


    // ===== BANK B =====
    int presetB = (int)params[PRESET_B_PARAM].getValue();
    lights[ADDITIVE_B_LIGHT].setBrightness(presetB == 0 ? 1.f : 0.f);
    lights[WAV_B_LIGHT].setBrightness(presetB == 1 ? 1.f : 0.f);

    if (presetB != lastPresetB)
    {
        if (presetB == 0)
        {
            wavetableB.generateBasic(1);
            wavetableB.currentMode = 0;
            wavLoadedB = false;
        }
        else
        {
            wavetableB.currentMode = 1;
        }
        lastPresetB = presetB;
    }

    // Pitch B
    float octaveB = params[PITCH_B_PARAM].getValue();
    float fineB = params[FINE_B_PARAM].getValue();

    if (inputs[PITCH_B_CV_INPUT].isConnected())
    {
        octaveB += inputs[PITCH_B_CV_INPUT].getVoltage() * 1.f;
    }

    float pitchB = 110.f * std::pow(2.f, octaveB + fineB / 1200.f);

    freqBSmoothed += (pitchB - freqBSmoothed) * alpha;

    // Morph B (Regler + CV Input)
    float morphParamB = params[MORPH_B_PARAM].getValue();
    if (inputs[MORPH_B_CV_INPUT].isConnected())
    {
        morphParamB += inputs[MORPH_B_CV_INPUT].getVoltage() * 0.2f;
    }
    float morphBTarget = rack::clamp(morphParamB, 0.f, 1.f);
    morphBSmoothed += (morphBTarget - morphBSmoothed) * alpha;

    //=== SYNC A <-> B ===
    bool syncAktive = params[SYNC_A_B_PARAM].getValue() > 0.5f;
    float syncPhaseOffset = 0.f;

    if (syncAktive)
    {
        freqBSmoothed += (freqASmoothed - freqBSmoothed) * alpha;
        float pitchCoarse = (octaveB + 3.f) / 6.f;
        float phaseFine = fineB / 600.f;
        syncPhaseOffset = pitchCoarse + phaseFine;
        syncPhaseOffset -= std::floor(syncPhaseOffset);
        syncPhaseSmoothed += (syncPhaseOffset - syncPhaseSmoothed) * alpha * 0.05f;
        syncPhaseSmoothed -= std::floor(syncPhaseSmoothed);
    }

    // ===== Volume prep =====
    float volA = params[VOLUME_A_PARAM].getValue();
    float volB = params[VOLUME_B_PARAM].getValue();

    if (inputs[VOLUME_A_CV_INPUT].isConnected())
    {
        volA += inputs[VOLUME_A_CV_INPUT].getVoltage() * 0.1f;
        volA = rack::clamp(volA, 0.f, 1.f);
    }
    if (inputs[VOLUME_B_CV_INPUT].isConnected())
    {
        volB += inputs[VOLUME_B_CV_INPUT].getVoltage() * 0.1f;
        volB = rack::clamp(volB, 0.f, 1.f);
    }



    // Voice Drift: zufällige, langsame Frequenz-Wanderung pro Stimme.
    // Simuliert analoge Oszillator-Instabilität (Temperaturdrift, Bauteil-Toleranzen).
    // Jede Stimme akkumuliert winzige Zufallswerte (±0,002 ct/Sample) und wird
    // auf ±5 ct begrenzt. Bricht periodische Schwebungen zwischen den Stimmen auf.
    for (int v = 0; v < MAIN_VOICES; v++)
    {
        float noise = (rack::random::uniform() ) * 0.002f;
        voiceDriftA[v] += noise;
        voiceDriftB[v] += noise;

        voiceDriftA[v] = rack::clamp(voiceDriftA[v], -5.f, 5.f);
        voiceDriftB[v] = rack::clamp(voiceDriftB[v], -5.f, 5.f);
    }

    float sampleA, sampleB;

    if (pmDirection == 0)
    {
        sampleA = processVoices(freqASmoothed, morphASmoothed, wavetableA,
                                mainVoicesA, 0.f, args.sampleTime, voiceDriftA);

        pmOffsetB = sampleA * pmAmount * pmScale ;
        float phaseB = syncAktive ? (syncPhaseSmoothed + pmOffsetB) : pmOffsetB;
        sampleB = processVoices(freqBSmoothed, morphBSmoothed, wavetableB,
                                mainVoicesB, phaseB, args.sampleTime, voiceDriftB);

    }
    else if (pmDirection == 1)
    {
        sampleB = processVoices(freqBSmoothed, morphBSmoothed, wavetableB,
                                mainVoicesB, 0.f, args.sampleTime, voiceDriftB);


        pmOffsetA = sampleB * pmAmount * pmScale ;
        sampleA = processVoices(freqASmoothed, morphASmoothed, wavetableA,
                                mainVoicesA, pmOffsetA, args.sampleTime, voiceDriftA);
    }
    else
    {
        pmOffsetA = lastSampleB * pmAmount * pmScale ;
        pmOffsetB = lastSampleA * pmAmount * pmScale ;

        sampleA = processVoices(freqASmoothed, morphASmoothed, wavetableA,
                                mainVoicesA, pmOffsetA, args.sampleTime, voiceDriftA);

        float phaseB = syncAktive ? (syncPhaseSmoothed + pmOffsetB) : pmOffsetB;
        sampleB = processVoices(freqBSmoothed, morphBSmoothed, wavetableB,
                                mainVoicesB, phaseB, args.sampleTime, voiceDriftB);

    }

    //Sub OSC
     float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();
     float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();
     float freqSubA = freqASmoothed / 2;
     float freqSubB = freqBSmoothed / 2;

    if (subLevelA > 0.f)
    {
        sampleA += subOscA.processRechteck(freqSubA, args.sampleTime) * subLevelA;
    }
    if (subLevelB > 0.f)
    {
        sampleB += subOscB.processRechteck(freqSubB, args.sampleTime) * subLevelB;
    }

    // ===== Filter =====
    // Cutoff: exp(knob) × pow(2, cv)  →  1V/Oct Eurorack-Standard
    float totalCutoffVoltage = params[CUTOFF_A_B_PARAM].getValue();
    float cutoffHz = std::exp(totalCutoffVoltage);

    if (inputs[CUTOFF_CV_INPUT].isConnected())
    {
        float cv = inputs[CUTOFF_CV_INPUT].getVoltage();
        cutoffHz *= std::pow(2.f, cv);
    }

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
    if (inputs[PHASER_LFO_A_INPUT].isConnected())
    {
        float cv = inputs[PHASER_LFO_A_INPUT].getVoltage();
        float lfoVal = rack::clamp(cv / 10.f + 0.5f, 0.f, 1.f);
        phaserOutA = phaserA.processExternalLFO(filteredA, lfoVal);
    }
    else
    {
        phaserOutA = phaserA.process(filteredA);
    }

    float phaserOutB;
    if (inputs[PHASER_LFO_B_INPUT].isConnected())
    {
        float cv = inputs[PHASER_LFO_B_INPUT].getVoltage();
        float lfoVal = rack::clamp(cv / 10.f + 0.5f, 0.f, 1.f);
        phaserOutB = phaserB.processExternalLFO(filteredB, lfoVal);
    }
    else
    {
        phaserOutB = phaserB.process(filteredB);
    }

    // ===== Chorus pro Bank =====
    float rateKnobA = params[CHORUS_RATE_A_PARAM].getValue();
    float depthKnobA = params[CHORUS_DEPTH_A_PARAM].getValue();

    float rateKnobB = params[CHORUS_RATE_B_PARAM].getValue();
    float depthKnobB = params[CHORUS_DEPTH_B_PARAM].getValue();


    chorusA.setRate( rateKnobA);
    chorusA.setDepth(depthKnobA);
    chorusA.setMix(params[CHORUS_MIX_PARAM_A].getValue());

    chorusB.setRate( rateKnobB);
    chorusB.setDepth(depthKnobB);
    chorusB.setMix(params[CHORUS_MIX_PARAM_B].getValue());

    auto chorusOutA = chorusA.process(phaserOutA);
    auto chorusOutB = chorusB.process(phaserOutB);

    // ===== Volume  =====
    chorusOutA.l *= volA ;
    chorusOutA.r *= volA ;
    chorusOutB.l *= volB ;
    chorusOutB.r *= volB ;

    // ===== Stereo Mix =====
    // Cross-Spill: kein Pan, sondern Cross-Fader zwischen den Bänken.
    // 0 = Bank A → L, Bank B → R (getrennt).  1 = beide auf beide Kanäle (50/50).
    // tanh() Soft-Clip begrenzt auf ±1.0, ×5.0 → VCV Audio-Standard (±5V).
    float crossSpill = params[CROSS_SPILL_PARAM].getValue();

    float leftDirect = chorusOutA.l * (1.f - crossSpill * 0.5f);
    float leftCross = chorusOutB.l * (crossSpill * 0.5f);

    float rightDirect = chorusOutB.r * (1.f - crossSpill * 0.5f);
    float rightCross = chorusOutA.r * (crossSpill * 0.5f);

    float finalLeft = leftDirect + leftCross;
    float finalRight = rightDirect + rightCross;

    float outputGain = 5.0f;

    auto softClip = [](float x)
    {
        return std::tanh(x);
    };

    finalLeft = softClip(finalLeft);
    finalRight = softClip(finalRight);

    outputs[AUDIO_LEFT_OUTPUT].setVoltage(finalLeft * outputGain);
    outputs[AUDIO_RIGHT_OUTPUT].setVoltage(finalRight * outputGain);

    lastSampleA = sampleA;
    lastSampleB = sampleB;
}