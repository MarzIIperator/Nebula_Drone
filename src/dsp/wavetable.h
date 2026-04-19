#ifndef NEBULA_WAVETABLE_HPP
#define NEBULA_WAVETABLE_HPP

#include <array>
#include <algorithm>
#include <cmath>

struct Wavetable {
    static constexpr int TABLE_SIZE = 2048;
    static constexpr int NUM_FRAMES = 64;

    std::array<std::array<float, TABLE_SIZE + 1>, NUM_FRAMES> frames{};

    void clear() {
        for (auto& frame : frames) {
            frame.fill(0.f);
        }
    }

    float getSample(float phase, float morph) const {
        morph = clamp(morph, 0.f, 1.f);
        phase -= std::floor(phase);

        float framePos = morph * (NUM_FRAMES - 1);
        int frameA = static_cast<int>(framePos);
        int frameB = std::min(frameA + 1, NUM_FRAMES - 1);
        float frameFrac = framePos - frameA;

        float tablePos = phase * TABLE_SIZE;
        int idx = static_cast<int>(tablePos);
        float frac = tablePos - idx;

        float a0 = frames[frameA][idx];
        float a1 = frames[frameA][idx + 1];
        float b0 = frames[frameB][idx];
        float b1 = frames[frameB][idx + 1];

        float sampleA = a0 + (a1 - a0) * frac;
        float sampleB = b0 + (b1 - b0) * frac;

        return sampleA + (sampleB - sampleA) * frameFrac;
    }
};

#endif