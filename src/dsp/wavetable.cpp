#include "wavetable.h"
#include <cmath>
#include <algorithm>
#include <iostream>
#include "../Utils/AudioFile.h"

static float clamp(float val, float lo, float hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

Wavetable::Wavetable() {
    clear();
}

void Wavetable::clear() {
    for (auto& frame : frames) {
        frame.fill(0.f);
    }
    for (auto& frame : framesWav) {
        frame.fill(0.f);
    }
}

void Wavetable::generateBasic(int preset) {
    switch(preset) {
        case 0:
            generate1();
            break;
        case 1:
            generate2();
            break;
        default:
            generate1();
    }
    normalizeAndWrap();
}

void Wavetable::generate1() {
    auto clamp01 = [](float x) {
        return (x < 0.f) ? 0.f : (x > 1.f ? 1.f : x);
    };
    auto smoothstep01 = [&](float x) {
        x = clamp01(x);
        return x * x * (3.f - 2.f * x);
    };
    auto lerp = [](float start, float end, float alpha) {
        return start + (end - start) * alpha;
    };

    for (int frameIndex = 0; frameIndex < NUM_FRAMES; ++frameIndex) {
        float morphPos = static_cast<float>(frameIndex) / static_cast<float>(NUM_FRAMES - 1);

        for (int sampleIndex = 0; sampleIndex < TABLE_SIZE; ++sampleIndex) {
            float phaseRadians = 2.f * static_cast<float>(M_PI) * sampleIndex / TABLE_SIZE;
            float sineFundamental = std::sin(phaseRadians);

            float k1Organ = 0.90f * std::sin(phaseRadians) + 0.22f * std::sin(2.f * phaseRadians) + 0.10f * std::sin(3.f * phaseRadians);
            float k2Triangle = (2.f / static_cast<float>(M_PI)) * std::asin(std::sin(phaseRadians));
            float k3SaturatedSaw = std::tanh(1.8f * std::sin(phaseRadians) + 0.8f * std::sin(2.f * phaseRadians) + 0.2f * std::sin(3.f * phaseRadians));
            float pulseCenter = 0.60f - 0.35f * morphPos;
            float k4Pulse = std::tanh(7.f * (sineFundamental - pulseCenter));
            float k5Fm = std::sin(phaseRadians + (0.5f + 4.0f * morphPos) * std::sin(2.01f * phaseRadians));

            float morphSegmentPos = morphPos * 4.f;
            int segmentIndex = static_cast<int>(morphSegmentPos);
            if (segmentIndex > 3) segmentIndex = 3;
            float segmentAlpha = smoothstep01(morphSegmentPos - static_cast<float>(segmentIndex));

            float segmentStart, segmentEnd;
            if (segmentIndex == 0) { segmentStart = k1Organ; segmentEnd = k2Triangle; }
            else if (segmentIndex == 1) { segmentStart = k2Triangle; segmentEnd = k3SaturatedSaw; }
            else if (segmentIndex == 2) { segmentStart = k3SaturatedSaw; segmentEnd = k4Pulse; }
            else { segmentStart = k4Pulse; segmentEnd = k5Fm; }

            float sampleValue = lerp(segmentStart, segmentEnd, segmentAlpha);
            sampleValue = std::tanh((1.1f + 0.8f * morphPos) * sampleValue);

            frames[frameIndex][sampleIndex] = sampleValue;  // ← In frames (Additive)
        }
    }
}

void Wavetable::generate2() {
    for (int f = 0; f < NUM_FRAMES; ++f) {
        float morphPos = f / 255.f;
        int numHarmonics = 4 + (int)(morphPos * 12);

        for (int i = 0; i < TABLE_SIZE; ++i) {
            float phase = 2.f * M_PI * i / TABLE_SIZE;
            float saw = 0.f;

            for (int h = 1; h <= numHarmonics; h++) {
                saw += std::sin(h * phase) / h;
            }

            frames[f][i] = saw;
        }
    }
}

float Wavetable::getSample(float phase, float morph) const {
    morph = clamp(morph, 0.f, 1.f);
    phase -= std::floor(phase);

    const auto& activeFrames = (currentMode == 0) ? frames : framesWav;

    float framePos = morph * (NUM_FRAMES - 1);
    int frameA = static_cast<int>(framePos);
    int frameB = std::min(frameA + 1, NUM_FRAMES - 1);
    float frameFrac = framePos - frameA;

    float tablePos = phase * TABLE_SIZE;
    int idx = static_cast<int>(tablePos);
    float frac = tablePos - idx;

    float a0 = activeFrames[frameA][idx];
    float a1 = activeFrames[frameA][idx + 1];
    float b0 = activeFrames[frameB][idx];
    float b1 = activeFrames[frameB][idx + 1];

    float sampleA = a0 + (a1 - a0) * frac;
    float sampleB = b0 + (b1 - b0) * frac;

    return sampleA + (sampleB - sampleA) * frameFrac;
}

void Wavetable::normalizeAndWrap() {
    float globalMax = 0.f;

    for (int f = 0; f < NUM_FRAMES; ++f) {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            globalMax = std::max(globalMax, std::fabs(frames[f][i]));
        }
    }

    if (globalMax <= 0.f) return;

    for (int f = 0; f < NUM_FRAMES; ++f) {
        for (int i = 0; i < TABLE_SIZE; ++i) {
            frames[f][i] /= globalMax;
        }
        frames[f][TABLE_SIZE] = frames[f][0];
    }
}

void Wavetable::loadFromWav(const char* path) {
    std::cout << "=== NEBULA: loadFromWav ===" << std::endl;

    AudioFile<float> audioFile;

    if (!audioFile.load(path)) {
        std::cout << "ERROR: Failed to load!" << std::endl;
        return;
    }

    if (!audioFile.isMono()) {
        std::cout << "ERROR: Not mono!" << std::endl;
        return;
    }

    int totalSamples = audioFile.getNumSamplesPerChannel();

    if (totalSamples % TABLE_SIZE != 0) {
        std::cout << "ERROR: Samples not divisible by " << TABLE_SIZE << std::endl;
        return;
    }

    int sourceFrames = totalSamples / TABLE_SIZE;
    std::cout << "Source frames: " << sourceFrames << ", Target frames: " << NUM_FRAMES << std::endl;

    // Resampling: in framesWav schreiben
    for (int targetFrame = 0; targetFrame < NUM_FRAMES; targetFrame++) {
        float sourcePos = (float)targetFrame / (NUM_FRAMES - 1) * (sourceFrames - 1);
        int sourceFrameA = (int)sourcePos;
        int sourceFrameB = std::min(sourceFrameA + 1, sourceFrames - 1);
        float frac = sourcePos - sourceFrameA;

        for (int i = 0; i < TABLE_SIZE; i++) {
            int idxA = sourceFrameA * TABLE_SIZE + i;
            int idxB = sourceFrameB * TABLE_SIZE + i;

            float sampleA = audioFile.samples[0][idxA];
            float sampleB = audioFile.samples[0][idxB];

            framesWav[targetFrame][i] = sampleA + (sampleB - sampleA) * frac;  // ← In framesWav!
        }

        framesWav[targetFrame][TABLE_SIZE] = framesWav[targetFrame][0];
    }

    std::cout << "SUCCESS: Resampled " << sourceFrames << " → " << NUM_FRAMES << " frames" << std::endl;
}

void Wavetable::generateMipsFromFullBandwidth() {}

int Wavetable::calculateMipLevel(float freq) { return 0; }