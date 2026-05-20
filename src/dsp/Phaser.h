//
// ChowPhaser.h — Lite Port of Chow Phaser (Mod section)
// Based on Jatin Chowdhury's open source code (chowdsp)
// https://github.com/jatinchowdhury18/ChowPhaser
//

#pragma once
#include <cmath>

class Phaser {
public:
    static constexpr int NUM_STAGES = 6;

    void setSampleRate(float sr) {
        sampleRate_ = sr;
        for (int i = 0; i < NUM_STAGES; i++) state_[i] = 0.f;
        lfoPhase_ = 0.f;
        lastOut_ = 0.f;
    }

    void setRate(float hz)     { lfoRate_ = hz; }
    void setDepth(float d)     { depth_ = d; }
    void setFeedback(float f)  { feedback_ = f; }
    void setMix(float m)       { mix_ = m; }

    float process(float input) {

        lfoPhase_ += lfoRate_ / sampleRate_;
        if (lfoPhase_ >= 1.f) lfoPhase_ -= 1.f;
        float lfoVal = 0.5f * (std::sin(2.f * (float)M_PI * lfoPhase_) + 1.f);

        const float minFreq = 10.f;
        const float maxFreq = 4000.f;
        float fc = minFreq * std::pow(maxFreq / minFreq, lfoVal * depth_);

        float t = std::tan((float)M_PI * fc / sampleRate_);
        float a1 = (t - 1.f) / (t + 1.f);

        // Nichtlineares Feedback
        float fbSignal = std::tanh(lastOut_ * feedback_ * 1.5f);
        float x = input + fbSignal;

        // kaskadierte Allpass-Stages
        for (int i = 0; i < NUM_STAGES; i++) {
            float y = a1 * x + state_[i];
            state_[i] = x - a1 * y;
            x = y;
        }

        lastOut_ = x;
        return input * (1.f - mix_) + x * mix_;
    }

    float processExternalLFO(float input, float lfoVal01) {

        lfoPhase_ += lfoRate_ / sampleRate_;
        if (lfoPhase_ >= 1.f) lfoPhase_ -= 1.f;

        const float minFreq = 100.f;
        const float maxFreq = 4000.f;
        float fc = minFreq * std::pow(maxFreq / minFreq, lfoVal01 * depth_);

        float t = std::tan((float)M_PI * fc / sampleRate_);
        float a1 = (t - 1.f) / (t + 1.f);

        float fbSignal = std::tanh(lastOut_ * feedback_);
        float x = input + fbSignal;

        for (int i = 0; i < NUM_STAGES; i++) {
            float y = a1 * x + state_[i];
            state_[i] = x - a1 * y;
            x = y;
        }

        lastOut_ = x;
        return input * (1.f - mix_) + x * mix_;
    }

private:
    float sampleRate_ = 48000.f;
    float lfoRate_ = 0.5f;
    float lfoPhase_ = 0.f;
    float depth_ = 0.7f;
    float feedback_ = 0.7f;
    float mix_ = 0.5f;
    float state_[NUM_STAGES] = {};
    float lastOut_ = 0.f;
};