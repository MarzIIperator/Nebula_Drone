//
// Phaser.h — 6-stufiger Allpass-Phaser
//

#pragma once
#include <cmath>

class Phaser
{
public:
    static constexpr int NUM_STAGES = 6;

    void setSampleRate(float sr)
    {
        sampleRate_ = sr;
        for (int i = 0; i < NUM_STAGES; i++) state_[i] = 0.f;
        lfoPhase_ = 0.f;
        lastOut_ = 0.f;
    }

    void setRate(float hz)     { lfoRate_ = hz; }
    void setDepth(float d)     { depth_ = d; }
    void setFeedback(float f)  { feedback_ = f; }
    void setMix(float m)       { mix_ = m; }

    float process(float input)
    {
        // Phasor: rückt jeden Sample um lfoRate/sampleRate vor (0 → 1 wrap).
        lfoPhase_ += lfoRate_ / sampleRate_;
        if (lfoPhase_ >= 1.f) lfoPhase_ -= 1.f;

        // Sinus-LFO: Phase (0…1) → Sinus (−1…+1) → skaliert auf 0…1.
        float lfoVal = 0.5f * (std::sin(2.f * (float)M_PI * lfoPhase_) + 1.f);

        // Exponentielle Frequenz-Abbildung: 10 Hz – 4000 Hz.
        const float minFreq = 10.f;
        const float maxFreq = 4000.f;
        float fc = minFreq * std::pow(maxFreq / minFreq, lfoVal * depth_);

        // Bilinear-Transformation: tan() verzerrt die Frequenz-Achse so vor,
        // dass die digitale Cutoff-Frequenz exakt der analogen entspricht.
        float t  = std::tan((float)M_PI * fc / sampleRate_);
        float a1 = (t - 1.f) / (t + 1.f);

        // Nichtlineares Feedback
        float fbSignal = std::tanh(lastOut_ * feedback_ * 1.5f);
        float x = input + fbSignal;

        // 6 kaskadierte Allpass-Stufen
        for (int i = 0; i < NUM_STAGES; i++)
        {
            float y = a1 * x + state_[i];
            state_[i] = x - a1 * y;
            x = y;
        }

        lastOut_ = x;   // Für Feedback des nächsten Samples speichern

        // Wet/Dry-Mix.
        return input * (1.f - mix_) + x * mix_;
    }

    float processExternalLFO(float input, float lfoVal01)
    {
        // Gleiche Verarbeitung wie oben, verwendet aber einen extern
        // bereitgestellten LFO-Wert (0…1, aus CV-Eingang gemappt).

        const float minFreq = 10.f;
        const float maxFreq = 4000.f;
        float fc = minFreq * std::pow(maxFreq / minFreq, lfoVal01 * depth_);

        float t  = std::tan((float)M_PI * fc / sampleRate_);
        float a1 = (t - 1.f) / (t + 1.f);

        float fbSignal = std::tanh(lastOut_ * feedback_ * 1.5f);
        float x = input + fbSignal;

        for (int i = 0; i < NUM_STAGES; i++)
        {
            float y = a1 * x + state_[i];
            state_[i] = x - a1 * y;
            x = y;
        }

        lastOut_ = x;
        return input * (1.f - mix_) + x * mix_;
    }

private:
    float sampleRate_ = 48000.f;
    float lfoRate_    = 0.5f;      // LFO-Rate in Hz
    float lfoPhase_   = 0.f;        // Phasor (0–1)
    float depth_      = 0.7f;       // Modulationstiefe
    float feedback_   = 0.7f;       // Feedback-Stärke
    float mix_        = 0.5f;       // Wet/Dry-Mix
    float state_[NUM_STAGES] = {};  // Allpass-Zustandsspeicher
    float lastOut_    = 0.f;        // Vorheriger Ausgang (für Feedback)
};
