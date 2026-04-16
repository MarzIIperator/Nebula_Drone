//
// Created by Emiel Kästner on 14.04.26.
// Hier wird ein Wavetable nach additiver art implemnentiert

#ifndef NEBULA_WAVETABLE_H
#define NEBULA_WAVETABLE_H
#pragma once
#include <cmath>
#include <array>

struct Wavetable
{
    static const int TABLE_SIZE = 256;
    static const int NUM_FRAMES = 32;

    std::array<std::array<float, 256>, 32> frames;

    // Konstruktor, alles auf 0 setzen

    Wavetable()
    {
        for (int f = 0; f < NUM_FRAMES; f++){
            for(int i = 0; i < TABLE_SIZE; i++){
                frames[f][i] = 0.f;
            }
        }
    }

    void generate() {

        // 1. Äußere Schleife: jeden Frame durchgehen
        for (int f = 0; f < NUM_FRAMES; f++) {

            // 2. Wie viele Obertöne hat dieser Frame?
            int maxHarmonics = 1 + (f * 15) / (NUM_FRAMES - 1);

            // 3. Mittlere Schleife: jedes Sample im Frame
            for (int i = 0; i < TABLE_SIZE; i++) {

                float value = 0.f;

                // 4. Innere Schleife: jeden Oberton addieren
                for (int h = 1; h <= maxHarmonics; h++) {

                    // 5. Sinus berechnen und addieren
                    value += std::sin(2.f * M_PI * h * i / TABLE_SIZE) / h;
                }

                frames[f][i] = value;
            }

            // 6. Normalisieren: maximalen Wert finden
            float maxVal = 0.f;
            for (int i = 0; i < TABLE_SIZE; i++) {
                if (std::fabs(frames[f][i]) > maxVal) {
                    maxVal = std::fabs(frames[f][i]);
                }
            }

            // Durch den Maximalwert teilen
            if (maxVal > 0.f) {
                for (int i = 0; i < TABLE_SIZE; i++) {
                    frames[f][i] /= maxVal;
                }
            }
        }
    }
};


#endif //NEBULA_WAVETABLE_H
