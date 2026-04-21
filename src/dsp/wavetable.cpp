#include "wavetable.h"
#include <cmath>
#include <algorithm>

// Hilfsfunktion
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
}

void Wavetable::generateBasic(int preset) {
    switch(preset) {
        case 0:
            generate1();
            break;
        case 1:
            generate2();
            break;
        case 2:
            generate3();
            break;
        case 3:
            generate4();
            break;
        default:
            generate1();
    }

    normalizeAndWrap();
}

// KEIN "private:" hier! Einfach direkt die Funktionen:

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

            frames[frameIndex][sampleIndex] = sampleValue;
        }
    }
    // KEIN normalizeAndWrap() hier - wird oben in generateBasic() gemacht
}

void Wavetable::generate2() {
    // Platzhalter: Sägezahn-Familie
    for (int f = 0; f < NUM_FRAMES; f++) {
        int maxHarmonics = 1 + (f * 15) / (NUM_FRAMES - 1);

        for (int i = 0; i < TABLE_SIZE; i++) {
            float phase = 2.f * M_PI * i / TABLE_SIZE;
            float value = 0.f;

            for (int h = 1; h <= maxHarmonics; h++) {
                value += std::sin(phase * h) / h;
            }

            frames[f][i] = value;
        }
    }
}

void Wavetable::generate3() {
    // Platzhalter: Square-Familie (nur ungerade Obertöne)
    for (int f = 0; f < NUM_FRAMES; f++) {
        int maxHarmonics = 1 + (f * 15) / (NUM_FRAMES - 1);

        for (int i = 0; i < TABLE_SIZE; i++) {
            float phase = 2.f * M_PI * i / TABLE_SIZE;
            float value = 0.f;

            for (int h = 1; h <= maxHarmonics; h += 2) {  // nur ungerade
                value += std::sin(phase * h) / h;
            }

            frames[f][i] = value;
        }
    }
}

void Wavetable::generate4() {
    // Platzhalter: Warm (1/h²)
    for (int f = 0; f < NUM_FRAMES; f++) {
        int maxHarmonics = 1 + (f * 15) / (NUM_FRAMES - 1);

        for (int i = 0; i < TABLE_SIZE; i++) {
            float phase = 2.f * M_PI * i / TABLE_SIZE;
            float value = 0.f;

            for (int h = 1; h <= maxHarmonics; h++) {
                value += std::sin(phase * h) / (h * h);
            }

            frames[f][i] = value;
        }
    }
}

float Wavetable::getSample(float phase, float morph) const {
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
    //
}

void Wavetable::generateMipsFromFullBandwidth() {
    //
}

int Wavetable::calculateMipLevel(float freq) {
    return 0;
}