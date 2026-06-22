//
// LadderFilter.h — Nichtlinearer Moog-Ladder-Filter
//
// Basiert auf: Huovilainen (2004), "Nonlinear Digital Implementation of the Moog Ladder Filter"
//              DAFx-04, Naples, Italy.

#ifndef NEBULA_LADDERFILTER_H
#define NEBULA_LADDERFILTER_H
#include <cmath>

class LadderFilter {

public:

    void setSampleRate(float sr) {
        sampleRate = sr;
        update();
    }

    void setResonanz(float r) {
        if (r < 0.f) r = 0.f;
        if (r > 1.f) r = 1.f;
        resonanz = r;
        update();
    }

    void setCutoff(float hz) {
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
        // Half-Sample-Delay: Mittelwert zweier vorheriger Ausgangswerte.
        float yd_averaged = 0.5f * (Yd_1 + Yd_2);

        // Resonanz-Feedback (invertiert)
        float feedback = 4.f * resonanz * yd_averaged;

        // Eingangs-Differenzpaar (Treiber Q1/Q2).
        // tanh() modelliert die Transistor-Sättigungskurve aus der Ebers-Moll-Gleichung.
        float tanh_in = tanh((input - feedback) / vt2);

        // Vorheriger Ausgang der 4. Stufe — benötigt für den Integrator der letzten Stufe.
        float tanh_yd_old = tanh(Yd / vt2);

        // === Stufe 1 (Kondensator C1) ===
        Ya = Ya + g * vt2 * (tanh_in - Wa);
        float Wa_new = tanh(Ya / vt2);   // Ausgangs-Strom für Stufe 2

        // === Stufe 2 (Kondensator C2) ===
        Yb = Yb + g * vt2 * (Wa_new - Wb);
        float Wb_new = tanh(Yb / vt2);

        // === Stufe 3 (Kondensator C3) ===
        Yc = Yc + g * vt2 * (Wb_new - Wc);
        float Wc_new = tanh(Yc / vt2);

        // === Stufe 4 (Kondensator C4) ===
        Yd = Yd + g * vt2 * (Wc_new - tanh_yd_old);

        // tanh()-Ergebnisse fürs nächste Sample sichern (Euler explizit).
        Wa = Wa_new;
        Wb = Wb_new;
        Wc = Wc_new;

        // Half-Sample-Delay State-Update:
        Yd_2 = Yd_1;
        Yd_1 = Yd;

        return Yd;
    }

private:

    float sampleRate = 48000.f;   // VCV Rack Abtastrate
    float cutoffFreq = 1000.f;    // Aktuelle Grenzfrequenz (Hz)
    float resonanz = 0.f;         // Resonanz (0,0 – 1,0)

    // vt2 = empirisch ermittelte Sättigungsschwelle (0,7 V).
    float vt2 = 0.7f;

    // Kondensator-Spannungen der 4 Leiter-Stufen.
    float Ya = 0.f, Yb = 0.f, Yc = 0.f, Yd = 0.f;

    // Gespeicherte tanh()-Ausgaben vom vorherigen Sample (Euler explizit).
    float Wa = 0.f, Wb = 0.f, Wc = 0.f;

    float Yd_1 = 0.f, Yd_2 = 0.f;

    // Digitaler Integrator-Koeffizient — ersetzt den analogen Steuerstrom I_bias.
    float g = 0.f;

    // Berechnet g neu, wenn sich Grenzfrequenz oder Abtastrate ändern.
    void update()
    {
        g = 1 - exp(-2*M_PI * cutoffFreq/sampleRate);
    }

};
#endif // NEBULA_LADDERFILTER_H
