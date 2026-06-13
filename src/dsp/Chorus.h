//
// Chorus.h
//

#pragma once
#include <cmath>
#include <array>
#include <math.hpp>


struct StereoPair
{
    float l, r;
};

class Chorus
{
public:
    void setSampleRate(float sr)
    {
        sampleRate_ = sr;
        reset();
    }

    void setRate(float hz) { lfoRate_ = hz; }
    void setDepth(float d) { depth_ = rack::math::clamp(d, 0.f, 1.f); }
    void setMix(float m) { mix_ = rack::math::clamp(m, 0.f, 1.f); }

    void reset()
    {
        delayBuffer_.fill(0.f);
        writeIndex_ = 0;
        lfoPhase_ = 0.f;
    }

    StereoPair process(float input)
    {
        float x = input;

        struct Voice
        {
            float baseDelay;
            float phaseOffset;
            float panPos;
        };

        constexpr Voice voices[6] = {
            {18.0f, 0.00f, -0.85f},
            {29.5f, 0.33f,  0.85f},
            {35.0f, 0.66f, -0.45f},
            {41.5f, 0.15f,  0.45f},
            {46.0f, 0.45f, -0.15f},
            {52.5f, 0.75f,  0.15f}
        };

        float sumL = 0.f;
        float sumR = 0.f;

        lfoPhase_ += lfoRate_ / sampleRate_;
        if (lfoPhase_ >= 1.f) lfoPhase_ -= 1.f;

        for (int v = 0; v < 6; v++)
        {
            const auto& voice = voices[v];

            float slowP = lfoPhase_ + voice.phaseOffset;
            if (slowP >= 1.f) slowP -= 1.f;

            float slowLfo = 2 * std::abs(2.f  * slowP -1 ) -1;

            float slowDepth = 4.f * 0.001f * sampleRate_ * depth_;

            float delayMod = slowDepth * slowLfo;

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

            constexpr float VOICE_GAIN = 0.167f;

            sumL += delayed * gainL * VOICE_GAIN;
            sumR += delayed * gainR * VOICE_GAIN;
        }

        constexpr float Pan_compensation = 1.f / 0.64f;

        sumL *= Pan_compensation;
        sumR *= Pan_compensation;

        delayBuffer_[writeIndex_] = x;
        writeIndex_ = (writeIndex_ + 1) % BUFFER_SIZE;

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
    float lfoPhase_ = 0.f;
    int writeIndex_ = 0;

    std::array<float, BUFFER_SIZE> delayBuffer_{};
};
