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
        auto smooth = [&](float x) {
            x = clamp01(x);
            return x * x * (3.f - 2.f * x); // smoothstep
        };
        auto lerp = [](float a, float b, float t) {
            return a + (b - a) * t;
        };

        for (int f = 0; f < Wavetable::NUM_FRAMES; ++f) {
            float t = static_cast<float>(f) / static_cast<float>(Wavetable::NUM_FRAMES - 1);

            for (int i = 0; i < Wavetable::TABLE_SIZE; ++i) {
                float p = 2.f * static_cast<float>(M_PI) * i / Wavetable::TABLE_SIZE;
                float s = std::sin(p);

                // 5 different wave "keyframes"
                float k0 = 0.90f * std::sin(p) + 0.22f * std::sin(2.f * p) + 0.10f * std::sin(3.f * p); // warm organ

                float k1 = (2.f / static_cast<float>(M_PI)) * std::asin(std::sin(p));                    // triangle

                float k2 = std::tanh(1.8f * std::sin(p) + 0.8f * std::sin(2.f * p) + 0.2f * std::sin(3.f * p)); // warm saw-ish

                float pwm = 0.60f - 0.35f * t; // moving pulse width
                float k3 = std::tanh(7.f * (s - pwm));   // pulse

                float k4 = std::sin(p + (0.5f + 4.0f * t) * std::sin(2.01f * p));                        // warm FM

                // Morph across keyframes: k0->k1->k2->k3->k4
                float m = t * 4.f;
                int seg = static_cast<int>(m);
                if (seg > 3) seg = 3;
                float u = smooth(m - static_cast<float>(seg));

                float a = 0.f, b = 0.f;
                if (seg == 0) { a = k0; b = k1; }
                else if (seg == 1) { a = k1; b = k2; }
                else if (seg == 2) { a = k2; b = k3; }
                else { a = k3; b = k4; }

                float value = lerp(a, b, u);

                // final glue
                value = std::tanh((1.1f + 0.8f * t) * value);

                wt.frames[f][i] = value;
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
