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
};


#endif //NEBULA_WAVETABLE_H
