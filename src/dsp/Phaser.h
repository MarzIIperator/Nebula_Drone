//
// ChowPhaser.h — Lite Port of Chow Phaser (Mod section)
// Based on Jatin Chowdhury's open source code (chowdsp)
// https://github.com/jatinchowdhury18/ChowPhaser
//

#pragma once
#include <cmath>

class Phaser {
public:
    static constexpr int MAX_STAGES = 8;

    void setSampleRate(float sr) {
        sampleRate_ = sr;
        reset();
    }

    void setModulation(float v) { mod_ = v; }
    void setStages(float n) {
        if (n < 1.f) n = 1.f;
        if (n > (float)MAX_STAGES) n = (float)MAX_STAGES;
        numStages_ = n;
    }
    void setSkew(float v) { skew_ = v; }

    void reset() {
        for (int i = 0; i <= MAX_STAGES; i++) z_[i] = 0.f;
    }

    // input: Audio (typisch ±1 für unsere Pipeline)
    // lfo:   bipolarer Modulator -1..+1 (extern oder 0 wenn nicht gepatcht)
    void process(float input, float lfo, float& output) {
        // 1. LDR-Widerstand aus LFO + Skew berechnen
        const float R = getLDRResistance(lfo, skew_);

        // 2. Analoge → Digitale Allpass-Koeffizienten
        calcCoefs(R, sampleRate_);

        // 3. Cascade durch numStages Stages
        const float x = input;
        float y = x;
        int intStages = (int)numStages_;
        for (int stage = 0; stage < intStages; ++stage) {
            y = processStage(y, stage);
        }

        // 4. Fractional stage (smoothes Wechsel zwischen Stage-Anzahlen)
        float stageFrac = numStages_ - (float)intStages;
        if (stageFrac > 0.f && intStages < MAX_STAGES) {
            float yFrac = processStage(y, intStages);
            y = stageFrac * yFrac + (1.f - stageFrac) * y;
        }

        // 5. Mix Dry/Wet
        output = mod_ * y + (1.f - mod_) * x;
    }

private:
    // LDR-Modell — formt LFO zu Widerstandswert
    static float lightShape(float x, float skewPow) {
        if (x < -1.f) x = -1.f;
        if (x > 1.f) x = 1.f;
        return (std::pow((x + 1.f) * 0.5f, skewPow) * 2.f) - 1.f;
    }

    static float getLDRResistance(float lfo, float skew) {
        constexpr float maxDepth = 20.0f;
        const float skewVal = std::pow(2.0f, skew);
        const float lfoVal = lightShape(lfo, skewVal);
        const float lightVal = (maxDepth + 0.1f) - (lfoVal * maxDepth);
        return 100000.0f * std::pow(lightVal / 0.1f, -0.75f);
    }

    // Analog → Digital Filter-Koeffizienten (Bilinear-Transform)
    void calcCoefs(float R, float fs) {
        constexpr float C = 25e-9f;     // 25 nF Kondensator (CHOW Original)
        const float RC = R * C;

        // Analoge Koeffizienten (Schaltungs-Topologie)
        const float b0s = RC;
        const float b1s = -1.0f;
        const float a0s = b0s;
        const float a1s = 1.0f;

        // Bilinear-Transform
        const float K = 2.0f * fs;
        const float a0 = a0s * K + a1s;
        b_[0] = ( b0s * K + b1s) / a0;
        b_[1] = (-b0s * K + b1s) / a0;
        a_[1] = (-a0s * K + a1s) / a0;
    }

    // Direct Form II Transposed Allpass-Stage
    float processStage(float x, int stage) {
        float y = z_[stage] + x * b_[0];
        z_[stage] = x * b_[1] - y * a_[1];
        return y;
    }

    float sampleRate_ = 48000.f;
    float mod_ = 0.5f;
    float numStages_ = 4.f;
    float skew_ = 0.f;

    float a_[2] = {1.f, 0.f};
    float b_[2] = {1.f, 0.f};
    float z_[MAX_STAGES + 1] = {0.f};
};