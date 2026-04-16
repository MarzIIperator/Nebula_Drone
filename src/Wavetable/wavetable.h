//
// Created by Emiel Kästner on 14.04.26.
// Hier wird ein Wavetable nach additiver art implemnentiert

#ifndef NEBULA_WAVETABLE_H
#define NEBULA_WAVETABLE_H
#pragma once
#include <JuceHeader.h>

class AdditiveWavetable
{

    public:

        AdditiveWavetable();
        ~AdditiveWavetable();

        void prepareToPlay(float sampleRate);

    private:

        juce::AudioBuffer<float> wavetable;
        double sampleRate = -1;
        float phaseIncrement = 0.f;
        float currentIndex = 0.f;
        float currentSample = 0.f;
        float gain = 0.5f;


};









#endif //NEBULA_WAVETABLE_H
