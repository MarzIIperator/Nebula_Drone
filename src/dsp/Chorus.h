//
// Chorus.h — 6-stimmiger Stereo-Ensemble-Chorus
//
// Architektur:
//   Ein einzelner Ringpuffer (8192 Samples) mit 6 Leseköpfen.
//   Jede Stimme hat eine feste Basis-Verzögerung (18–52 ms), die von
//   einem Dreieck-LFO moduliert wird. Jede Stimme hat einen eigenen
//   LFO-Phasenversatz, sodass sie nie synchron laufen — das erzeugt
//   eine organische Ensemble-Bewegung.
//
//   Die Modulationstiefe ist bipolar (±4 ms bei max. Depth), das Delay
//   schwingt symmetrisch um seinen Basis-Wert.
//
//   Constant-Power-Stereo-Panning (cos/sin) verteilt die 6 Stimmen im
//   Stereofeld. Pan_compensation korrigiert den Pegel-Verlust, der durch
//   das Verteilen der Stimmen von der Mitte aus entsteht.
//
//   Wet/Dry-Mix: out = input × (1−mix) + sum × mix
//   0% = nur trocken, 100% = nur nass (6 modulierte Stimmen).
//

#pragma once
#include <cmath>
#include <array>
#include <math.hpp>


struct StereoPair { float l, r; };

class Chorus
{
public:
    void setSampleRate(float sr)      { sampleRate_ = sr; reset(); }
    void setRate(float hz)            { lfoRate_ = hz; }
    void setDepth(float d)            { depth_ = rack::math::clamp(d, 0.f, 1.f); }
    void setMix(float m)              { mix_ = rack::math::clamp(m, 0.f, 1.f); }

    void reset()
    {
        delayBuffer_.fill(0.f);
        writeIndex_ = 0;
        lfoPhase_ = 0.f;
    }

    StereoPair process(float input)
    {
        float x = input;

        // Stimmen-Deskriptoren: Basis-Delay (ms), LFO-Phasenversatz, Stereo-Position.
        struct Voice { float baseDelay; float phaseOffset; float panPos; };

        constexpr Voice voices[6] = {
            {18.0f, 0.00f, -0.85f},   // Stimme 0: ganz links
            {29.5f, 0.33f,  0.85f},   // Stimme 1: ganz rechts
            {35.0f, 0.66f, -0.45f},   // Stimme 2: halblinks
            {41.5f, 0.15f,  0.45f},   // Stimme 3: halbrechts
            {46.0f, 0.45f, -0.15f},   // Stimme 4: leicht links
            {52.5f, 0.75f,  0.15f}    // Stimme 5: leicht rechts
        };

        float sumL = 0.f;
        float sumR = 0.f;

        // Phasor: rückt jeden Sample um lfoRate/sampleRate vor (0 → 1 wrap).
        lfoPhase_ += lfoRate_ / sampleRate_;
        if (lfoPhase_ >= 1.f) lfoPhase_ -= 1.f;

        for (int v = 0; v < 6; v++)
        {
            const auto& voice = voices[v];

            // LFO-Phase pro Stimme = globale Phase + fester Offset → Stimmen desynchronisiert.
            float slowP = lfoPhase_ + voice.phaseOffset;
            if (slowP >= 1.f) slowP -= 1.f;

            // Dreieck-LFO: lineares Auf und Ab von +1 → −1 → +1 (kein "Seasick" wie Sinus).
            float slowLfo = 2 * std::abs(2.f * slowP -1) -1;

            // Modulationstiefe: 4 ms (Spitze) × Depth-Regler, umgerechnet von ms → Samples.
            float slowDepth = 4.f * 0.001f * sampleRate_ * depth_;

            // Bipolare Delay-Modulation (±slowDepth um die Basis-Verzögerung).
            float delayMod = slowDepth * slowLfo;

            // Gesamt-Delay (Basis + Modulation), geclampt auf gültigen Puffer-Bereich.
            float totalDelay = voice.baseDelay * 0.001f * sampleRate_ + delayMod;
            totalDelay = rack::math::clamp(totalDelay, 1.f, (float)(BUFFER_SIZE - 4));

            // Ringpuffer-Lesen mit linearer Interpolation.
            float readPos = (float)writeIndex_ - totalDelay;
            while (readPos < 0.f) readPos += BUFFER_SIZE;
            while (readPos >= BUFFER_SIZE) readPos -= BUFFER_SIZE;

            int i0 = (int)readPos;
            int i1 = (i0 + 1) % BUFFER_SIZE;
            float frac = readPos - (float)i0;

            float delayed = delayBuffer_[i0] * (1.f - frac) + delayBuffer_[i1] * frac;

            // Constant-Power-Stereo-Panning:  cos²(θ) + sin²(θ) = 1.
            float panAngle = (voice.panPos + 1.f) * 0.25f * M_PI;   // −1…+1 → 0…π/2
            float gainL = std::cos(panAngle);
            float gainR = std::sin(panAngle);

            constexpr float VOICE_GAIN = 0.167f;   // 1/6 → Einheitspegel bei 6 gleichen Stimmen

            sumL += delayed * gainL * VOICE_GAIN;
            sumR += delayed * gainR * VOICE_GAIN;
        }

        // Pegel-Kompensation für die Stereoverteilung.
        constexpr float Pan_compensation = 1.f / 0.64f;
        sumL *= Pan_compensation;
        sumR *= Pan_compensation;

        // Aktuelles Sample in den Ringpuffer schreiben (NACH allen Lesevorgängen).
        delayBuffer_[writeIndex_] = x;
        writeIndex_ = (writeIndex_ + 1) % BUFFER_SIZE;

        // Wet/Dry-Mix.
        StereoPair out;
        out.l = input * (1.f - mix_) + sumL * mix_;
        out.r = input * (1.f - mix_) + sumR * mix_;

        return out;
    }

private:
    static constexpr int BUFFER_SIZE = 8192;

    float sampleRate_ = 48000.f;
    float lfoRate_   = 0.0f;      // LFO-Rate in Hz
    float depth_     = 0.f;        // Modulationstiefe (0–1)
    float mix_       = 0.5f;       // Wet/Dry-Mix (0–1)
    float lfoPhase_  = 0.f;        // Phasor (0–1)
    int   writeIndex_ = 0;

    std::array<float, BUFFER_SIZE> delayBuffer_{};
};
