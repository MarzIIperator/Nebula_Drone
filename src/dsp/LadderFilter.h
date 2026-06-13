//
// Created by Emiel Kästner on 28.04.26.
//

#ifndef NEBULA_LADDERFILTER_H
#define NEBULA_LADDERFILTER_H
#include <cmath>
#include <algorithm>

class LadderFilter {

public:

    void setSampleRate(float sr) {
        sampleRate = sr;
        update();
    }

    void setResonanz(float r) {
        // Clampen damit r immer in [0, 1] bleibt
        if (r < 0.f) r = 0.f;
        if (r > 1.f) r = 1.f;
        resonanz = r;
        update();
    }

    void setCutoff(float hz) {
        // Clampen auf sicheren Bereich: nicht unter 20 Hz, nicht über Nyquist
        float maxHz = sampleRate * 0.5f - 100.f;
        if (hz < 20.f) hz = 20.f;
        if (hz > maxHz) hz = maxHz;
        cutoffFreq = hz;
        update();
    }
    void reset()
    {
         Ya = 0.f, Yb = 0.f, Yc = 0.f, Yd = 0.f;
         Wa = 0.f, Wb = 0.f, Wc = 0.f;
         Yd_1 = 0.f, Yd_2 = 0.f;
    }
    float processLp(float input)
    {

        float yd_averaged = 0.5f * (Yd_1 + Yd_2);
        float feedback   = 4.f * resonanz  * yd_averaged;

        // ===== Vorberechnungen =====
        float tanh_in = tanh((input - feedback) / vt2);
        float tanh_yd_old = tanh(Yd / vt2);

        // ===== 4 Stages mit Caches =====
        Ya = Ya + g * vt2 * (tanh_in - Wa);
        float Wa_new = tanh(Ya / vt2);

        Yb = Yb + g * vt2 * (Wa_new - Wb);
        float Wb_new = tanh(Yb / vt2);

        Yc = Yc + g * vt2 * (Wb_new - Wc);
        float Wc_new = tanh(Yc / vt2);

        Yd = Yd + g * vt2 * (Wc_new - tanh_yd_old);

        // ===== State für nächstes Sample sichern =====
        Wa = Wa_new;
        Wb = Wb_new;
        Wc = Wc_new;

        // Half-Sample-Delay
        Yd_2 = Yd_1;
        Yd_1 = Yd;

        return Yd;
    }

    float processHp(float input)
    {
        return input - processLp(input);
    }

private:

    float sampleRate = 48000.f;
    float cutoffFreq = 1000.f;
    float resonanz = 0.f;

    float vt2 = 0.7f;
    float Ya = 0.f, Yb = 0.f, Yc = 0.f, Yd = 0.f; // 4 Stufen Output
    float Wa = 0.f, Wb = 0.f, Wc = 0.f;
    float Yd_1 = 0.f, Yd_2 = 0.f;// Für Half-Sample-Delay
    float g = 0.f;

    void update()
    {

        g = 1 - exp(-2*M_PI * cutoffFreq/sampleRate);
    }


};
#endif //NEBULA_LADDERFILTER_H

