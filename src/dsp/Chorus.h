//
// Chorus.h
//

#pragma once
#include <cmath>
#include <array>
#include <math.hpp>


struct StereoPair { float l, r; };

class Chorus {
public:
    void setSampleRate(float sr) {
        sampleRate_ = sr;
        reset();
    }

    void setRate(float hz)     { lfoRate_ = hz; }
    void setDepth(float d)     { depth_ = rack::math::clamp(d, 0.f, 1.f); }
    void setMix(float m)       { mix_ = rack::math::clamp(m, 0.f, 1.f); }
    void setFeedback(float f)  { feedback_ = 0.f; }

    void reset() {
        delayBuffer_.fill(0.f);
        writeIndex_ = 0;
        slowPhase_ = 0.f;
        fastPhase_ = 0.f;
    }

    StereoPair process(float input) {
        // Input limiting
        float x = std::tanh(input * 0.2f);
        // Dual LFO update
        slowPhase_ += lfoRate_ / sampleRate_;
        if (slowPhase_ >= 1.f) slowPhase_ -= 1.f;

        fastPhase_ += (lfoRate_ * 8.f) / sampleRate_;
        if (fastPhase_ >= 1.f) fastPhase_ -= 1.f;

        struct Voice {
            float baseDelay;
            float phaseOffset;
            float panPos;
        };

        constexpr Voice voices[8] = {
            {10.0f, 0.00f, -0.85f},
            {13.5f, 0.25f, -0.55f},
            {17.0f, 0.50f, -0.25f},
            {20.5f, 0.75f, -0.05f},
            {11.5f, 0.13f, 0.05f},
            {15.0f, 0.38f, 0.25f},
            {18.5f, 0.63f, 0.55f},
            {22.0f, 0.88f, 0.85f}
        };

        float sumL = 0.f;
        float sumR = 0.f;

        for (int v = 0; v < 8; v++) {
            const auto& voice = voices[v];

            float slowP = slowPhase_ + voice.phaseOffset;
            if (slowP >= 1.f) slowP -= 1.f;
            float slowLfo = std::sin(2.f * M_PI * slowP);

            float fastP = fastPhase_ + voice.phaseOffset;
            if (fastP >= 1.f) fastP -= 1.f;
            float fastLfo = std::sin(2.f * M_PI * fastP);

            
            float slowDepth = 3.5f * 0.001f * sampleRate_ * depth_;
            float fastDepth = 1.2f * 0.001f * sampleRate_ * depth_;

            float delayMod = slowDepth * (1.f + 0.95f * slowLfo);
            delayMod += fastDepth * (1.f + 0.95f * fastLfo);

            float totalDelay = voice.baseDelay * 0.001f * sampleRate_ + delayMod;
            totalDelay = rack::math::clamp(totalDelay, 1.f, (float)(BUFFER_SIZE - 4));

            float readPos = (float)writeIndex_ - totalDelay;
            while (readPos < 0.f) readPos += BUFFER_SIZE;
            while (readPos >= BUFFER_SIZE) readPos -= BUFFER_SIZE;

            int i0 = (int)readPos;
            int i1 = (i0 + 1) % BUFFER_SIZE;
            float frac = readPos - (float)i0;

            float delayed = delayBuffer_[i0] * (1.f - frac) + delayBuffer_[i1] * frac;

            float panAngle = (voice.panPos + 1.f) * 0.25f * M_PI;
            float gainL = std::cos(panAngle);
            float gainR = std::sin(panAngle);


            constexpr float VOICE_GAIN = 0.25;

            sumL += delayed * gainL * VOICE_GAIN;
            sumR += delayed * gainR * VOICE_GAIN;
        }

        delayBuffer_[writeIndex_] = x;
        writeIndex_ = (writeIndex_ + 1) % BUFFER_SIZE;

        sumL *= 7.0f;
        sumR *= 7.0f;

        // Mix
        StereoPair out;
        out.l = input * (1.f - mix_) + sumL * mix_;
        out.r = input * (1.f - mix_) + sumR * mix_;

        return out;
    }

private:
    static constexpr int BUFFER_SIZE = 8192;

    float sampleRate_ = 48000.f;
    float lfoRate_ = 0.0f;
    float depth_ = 0.f;
    float mix_ = 0.5f;
    float feedback_ = 0.f;
    float slowPhase_ = 0.f;
    float fastPhase_ = 0.f;
    int writeIndex_ = 0;

    std::array<float, BUFFER_SIZE> delayBuffer_{};
};