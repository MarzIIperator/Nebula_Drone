#ifndef NEBULA_WAVETABLE_PRESETS_HPP
#define NEBULA_WAVETABLE_PRESETS_HPP

#include <algorithm>
#include <cmath>
#include "wavetable.h"

enum class WavetablePresetId {
    Basic,
    eins,
    zwei,
    drei
};

struct WavetablePresets {
    // Rebuild wavetable data for the selected preset.
    // Clear first so switching presets never leaves stale frame/sample values behind.
    static void generate(Wavetable& wt, WavetablePresetId preset) {
        wt.clear();

        switch (preset) {
            case WavetablePresetId::Basic:
                generateBasic(wt);
                break;
            case WavetablePresetId::eins:
                generateeins(wt);
                break;
            case WavetablePresetId::zwei:
                generatezwei(wt);
                break;
            case WavetablePresetId::drei:
                generatedrei(wt);
                break;
        }
    }

private:
    static void normalizeAndWrap(Wavetable& wt) {
        float globalMax = 0.f;

        for (int f = 0; f < Wavetable::NUM_FRAMES; ++f) {
            for (int i = 0; i < Wavetable::TABLE_SIZE; ++i) {
                globalMax = std::max(globalMax, std::fabs(wt.frames[f][i]));
            }
        }

        if (globalMax <= 0.f) {
            return;
        }

        for (int f = 0; f < Wavetable::NUM_FRAMES; ++f) {
            for (int i = 0; i < Wavetable::TABLE_SIZE; ++i) {
                wt.frames[f][i] /= globalMax;
            }
            wt.frames[f][Wavetable::TABLE_SIZE] = wt.frames[f][0];
        }
    }

    static void generateBasic(Wavetable& wt) {

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

        for (int frameIndex = 0; frameIndex < Wavetable::NUM_FRAMES; ++frameIndex) {
            float morphPos = static_cast<float>(frameIndex) / static_cast<float>(Wavetable::NUM_FRAMES - 1);

            for (int sampleIndex = 0; sampleIndex < Wavetable::TABLE_SIZE; ++sampleIndex) {
                float phaseRadians = 2.f * static_cast<float>(M_PI) * sampleIndex / Wavetable::TABLE_SIZE;
                float sineFundamental = std::sin(phaseRadians);

                // 5 different wave "keyframes"
                // 0.90 keeps the fundamental dominant while 0.22/0.10 add gentle color (musical but not buzzy).
                float k1Organ = 0.90f * std::sin(phaseRadians) + 0.22f * std::sin(2.f * phaseRadians) + 0.10f * std::sin(3.f * phaseRadians); // warm organ

                float k2Triangle = (2.f / static_cast<float>(M_PI)) * std::asin(std::sin(phaseRadians));                    // triangle

                float k3SaturatedSaw = std::tanh(1.8f * std::sin(phaseRadians) + 0.8f * std::sin(2.f * phaseRadians) + 0.2f * std::sin(3.f * phaseRadians)); // warm saw-ish

                // Start duty near 60% (0.60) and narrow with morph to increase edge/brightness over the scan.
                float pulseCenter = 0.60f - 0.35f * morphPos;
                float k4Pulse = std::tanh(7.f * (sineFundamental - pulseCenter));   // pulse

                float k5Fm = std::sin(phaseRadians + (0.5f + 4.0f * morphPos) * std::sin(2.01f * phaseRadians));                        // warm FM

                // Morph across keyframes: organ -> triangle -> saturated saw -> pulse -> FM
                float morphSegmentPos = morphPos * 4.f;
                int segmentIndex = static_cast<int>(morphSegmentPos);
                if (segmentIndex > 3) segmentIndex = 3;
                float segmentAlpha = smoothstep01(morphSegmentPos - static_cast<float>(segmentIndex));

                float segmentStart = 0.f;
                float segmentEnd = 0.f;
                if (segmentIndex == 0) { segmentStart = k1Organ; segmentEnd = k2Triangle; }
                else if (segmentIndex == 1) { segmentStart = k2Triangle; segmentEnd = k3SaturatedSaw; }
                else if (segmentIndex == 2) { segmentStart = k3SaturatedSaw; segmentEnd = k4Pulse; }
                else { segmentStart = k4Pulse; segmentEnd = k5Fm; }

                float sampleValue = lerp(segmentStart, segmentEnd, segmentAlpha);

                sampleValue = std::tanh((1.1f + 0.8f * morphPos) * sampleValue);

                wt.frames[frameIndex][sampleIndex] = sampleValue;
            }
        }

        normalizeAndWrap(wt);
    }

    static void generateeins(Wavetable& wt) {


        float k1;
        float k2;
        float k3;
        float k4;
        float k5;


    }

    static void generatezwei(Wavetable& wt) {


        float k1;
        float k2;
        float k3;
        float k4;
        float k5;

    }

    static void generatedrei(Wavetable& wt) {


        float k1;
        float k2;
        float k3;
        float k4;
        float k5;

    }
};

#endif
