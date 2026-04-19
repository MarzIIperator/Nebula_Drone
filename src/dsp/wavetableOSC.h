
//
// Created by Emiel Kästner on 11.04.26.
//
#ifndef NEBULA_WAVETABLE_OSC_H
#define NEBULA_WAVETABLE_OSC_H
#include <cmath>
#include "wavetable.h"

struct WavetableOsc {
    float phase = 0.f;

    void reset(float newPhase = 0.f) {
        phase = newPhase - std::floor(newPhase);
    }

    void advance(float freq, float sampleTime) {
        phase += freq * sampleTime;
        phase -= std::floor(phase);
    }

    float process(float freq, float sampleTime, const Wavetable& wavetable, float morph) {
        advance(freq, sampleTime);
        return wavetable.getSample(phase, morph);
    }
};

#endif




