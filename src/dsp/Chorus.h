//
// Chorus.h — True Stereo Modulated Delay (Phase C)
//

#pragma once
#include <cmath>
#include <array>


struct StereoPair {
    float l;
    float r;
};

class Chorus {
public:
    void setSampleRate(float sr) {
        sampleRate_ = sr;
        reset();
    }

    void setRate(float hz)     { lfoRate_ = hz; }
    void setDepth(float d)     { depth_ = d; }
    void setMix(float m)       { mix_ = m; }
    void setFeedback(float f)  { feedback_ = f; }

    void reset() {
        for (auto& s : delayBuffer_) s = 0.f;
        writeIndex_ = 0;
        lfoPhase_ = 0.f;
    }


    StereoPair process(float input) {
        // LFO-Basis updaten
        lfoPhase_ += lfoRate_ / sampleRate_;
        if (lfoPhase_ >= 1.f) lfoPhase_ -= 1.f;

        //  8 Stimmen  (4 pro Seite)
        constexpr float baseDelays[NUM_VOICES]    =
            {7.0f, 9.5f, 13.5f, 17.2f, 21.5f, 24.5f, 11.0f, 19.0f}; // ms

        constexpr float phaseOffsets[NUM_VOICES]  =
            {0.f, 1.f/4.f, 2.f/4.f, 3.f/4.f, 0.f, 1.f/4.f, 2.f/4.f, 3.f/4.f};

        constexpr float rateMultipliers[NUM_VOICES] = {
            1.00f, 0.82f, 0.76f, 0.92f, 0.64f, 0.56f, 1.33f, 1.15f
        };

        float sumL = 0.f;
        float sumR = 0.f;

        for (int v = 0; v < NUM_VOICES; v++) {
            // Eigene Phase, eigene Rate pro Voice
            float phaseV = lfoPhase_ * rateMultipliers[v] + phaseOffsets[v];
            phaseV -= std::floor(phaseV);  // wrap auf 0..1
            float lfoValV = std::sin(2.f * (float)M_PI * phaseV);

            // Eigene Base-Delay + Modulation
            float modDelayMs = baseDelays[v] + lfoValV * MOD_DEPTH_MS * depth_;
            float delaySamples = (modDelayMs / 1000.f) * sampleRate_;

            float readPos = (float)writeIndex_ - delaySamples;
            while (readPos < 0.f) readPos += (float)BUFFER_SIZE;
            while (readPos >= (float)BUFFER_SIZE) readPos -= (float)BUFFER_SIZE;

            //Lineare Interpolation
            int idx0 = (int)readPos;
            int idx1 = (idx0 + 1) % BUFFER_SIZE;
            float frac = readPos - (float)idx0;
            float delayed = delayBuffer_[idx0] + frac * (delayBuffer_[idx1] - delayBuffer_[idx0]);

            // STEREO-PANNING
            if (v % 2 == 0) {
                sumL += delayed;
            } else {
                sumR += delayed;
            }
        }


        float halfVoices = NUM_VOICES / 2.0f;
        sumL /= halfVoices;
        sumR /= halfVoices;

        delayBuffer_[writeIndex_] = input + (sumL + sumR) * 0.5f * feedback_;
        writeIndex_ = (writeIndex_ + 1) % BUFFER_SIZE;


        StereoPair output;
        output.l = input * (1.f - mix_) + sumL * mix_;
        output.r = input * (1.f - mix_) + sumR * mix_;

        return output;
    }

    StereoPair processExternalLFO(float input, float lfoVal01) {
        return process(input);
    }

private:
    static constexpr int NUM_VOICES = 8;
    static constexpr int BUFFER_SIZE = 4096;
    static constexpr float MOD_DEPTH_MS = 4.f;

    float sampleRate_ = 48000.f;
    float lfoRate_    = 0.0f;
    float depth_      = 0.0f;
    float mix_        = 0.5f;
    float feedback_   = 0.4f;
    float lfoPhase_   = 0.f;
    int   writeIndex_ = 0;
    std::array<float, BUFFER_SIZE> delayBuffer_ = {};
};