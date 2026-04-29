//
// Created by Emiel Kästner on 26.04.26.
//

#pragma once
#include <cmath>                   // für std::sin, std::abs
#include <cstdlib>                 // für std::rand, RAND_MAX

class LFO {
public:

    enum class Shape {
        SINE,
        TRIANGLE,
        SMOOTH_RANDOM
    };

    void setRate(float hz)   { rate = hz; }
    void setShape(Shape s)   { shape = s; }

    void reset() {
        phase = 0.f;
        randPrev = 0.f;
        randTarget = uniform();
    }

    // Pro Audio-Sample aufgerufen. Gibt LFO-Wert zwischen -1 und +1 zurück.
    float process(float sampleTime) {

        // 1. Phasor weiterzählen
        //    rate ist Hz, sampleTime ist 1/sampleRate
        //    -> pro Sekunde wandert phase genau "rate" mal von 0 nach 1
        phase += rate * sampleTime;


        while (phase >= 1.f) {
            phase -= 1.f;

            randPrev = randTarget;
            randTarget = uniform();
        }


        switch (shape) {
            case Shape::SINE:
                // phase 0..1 wird zu Winkel 0..2π
                return std::sin(2.f * (float)M_PI * phase);

            case Shape::TRIANGLE:

                return 4.f * std::abs(phase - 0.5f) - 1.f;

            case Shape::SMOOTH_RANDOM:

                return randPrev + (randTarget - randPrev) * smoothstep(phase);
        }

        return 0.f;
    }

private:

    float phase      = 0.f;
    float rate       = 1.f;
    Shape shape      = Shape::SINE;
    float randPrev   = 0.f;
    float randTarget = 0.f;
    

    static float uniform() {
        return ((float)std::rand() / (float)RAND_MAX) * 2.f - 1.f;
    }

    static float smoothstep(float x) {
        return x * x * (3.f - 2.f * x);
    }
};