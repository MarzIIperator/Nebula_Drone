
//
// Created by Emiel Kästner on 11.04.26.
//
#ifndef NEBULA_VCO_H
#define NEBULA_VCO_H

#include <cmath>
enum WaveformTypes
{
    Sinus,
    Dreieck,
    Pulse,
    Noise
};
struct Oscillator
{
    float phase = 0.f;

    float process(float freq, float sampleTime, Waveform waveform) {
        phase += freq * sampleTime;
        if (phase >= 1.f) phase -= 1.f;

        switch (waveform) {
        case SINE:
            return std::sin(2.f * M_PI * phase);
        case SAW:
            return 2.f * phase - 1.f;
        case SQUARE:
            return phase < 0.5f ? 1.f : -1.f;
        case NOISE:
            return (float)rand() / RAND_MAX * 2.f - 1.f;
        default:
            return 0.f;
        }
    }
};

#endif
