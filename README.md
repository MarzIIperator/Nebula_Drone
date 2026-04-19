# Nebula — Drone/Texture Synthesizer

**VCV Rack 2 Plugin Module**
*Emiel Kästner — Sommersemester 2026*

---

## Projektbeschreibung

Nebula ist ein Drone/Texture-Synthesizer-Modul für die Open-Source Modular-Synthesizer-Plattform VCV Rack 2. Das Modul erzeugt dichte, sich langsam entwickelnde Klangtexturen durch die Kombination von Wavetable-Synthese, Phasenmodulation und subtraktiver Filterung.

## 1. Designphilosophie

- **Drone-fokussiert:** Keine ADSR-Hüllkurven, keine Note-On/Off-Logik. Der Klang läuft dauerhaft und entwickelt sich langsam über die Zeit.
- **2-Bank-Architektur:** Zwei unabhängige Klangbänke (je 1 Haupt-VCO + 1 Sub-VCO) die sich gegenseitig per Phasenmodulation beeinflussen.
- **Wavetable-basiert:** Jede Bank hat ihre eigene Wavetable mit 32 vorberechneten Frames. Morph-Knobs scannen stufenlos durch die Frames.


---

## 2. Architektur

### 2.1 Signalfluss (Übersicht)

```
Bank A                          Bank B
┌─────────────────┐            ┌─────────────────┐
│ VCO 1 (Haupt)   │            │ VCO 2 (Haupt)   │
│ + Sub 1 (-1oct) │            │ + Sub 2 (-1oct) │
│ Wavetable A     │            │ Wavetable B     │
│ Morph A         │            │ Morph B         │
│ LFO A           │            │ LFO B           │
└────────┬────────┘            └────────┬────────┘
         │                            │
         ▼                            ▼
      Mix A ──── PM Amount ────▶     Mix B
         │       (A/B moduliert       │
         │         Phase von B/A)     │
         │                            │
         └──────────┬─────────────────┘
                    ▼
             Stereo Mix (Pan)
                    │
                    ▼
              Filter (global)
              LP/HP, Cutoff, Reso
                    │
                    ▼
              Reverb (global)
              Size, Mix
                    │
                    ▼
              Master Volume
                    │
                    ▼
              Stereo Out (L/R)
```

### 2.2 Pro Bank (×2)

Jede Bank besteht aus:

| Komponente | Beschreibung |
|---|---|
| **Haupt-VCO** | Wavetable-Oszillator, liest aus eigener Wavetable |
| **Sub-VCO** | Identisch zum Haupt-VCO, läuft fest 1 oder 2 Oktaven tiefer, liest denselben Frame |
| **Wavetable** | 32 Frames, vorberechnet, evtl ein Preset an Wavetables pro Bank. Mit einem Random knopf veränderbar |
| **Morph-Knob** | Scannt stufenlos durch die 32 Frames der Bank-eigenen Wavetable |
| **LFO** | Rate, Depth, Shape (Sine/Tri/Random) |

#### Parameter pro Bank

- **Pitch** — Grundtonhöhe (gerastert in Halbtönen/Oktaven)
- **Fine** — Mikroverstimmung ±50  (Detune zwischen Haupt und Sub)
- **Volume** — Lautstärke der Bank
- **Pan** — Stereoposition (L/R)
- **Sub Level** — Lautstärke des Sub-VCO relativ zum Haupt-VCO
- **Sub Octave** — Switch: -1 oder -2 Oktaven unter dem Haupt-VCO
- **Morph** — Position in der Wavetable 
- **LFO Rate** — Geschwindigkeit des LFO (0.01 – 10 Hz)
- **LFO Depth** — Modulationstiefe
- **LFO Shape** — Wellenform: Sine, Triangle, Random (Smooth Random)

### 2.3 Phasenmodulation (PM) — zwischen den Bänken

Die Phasenmodulation ist das zentrale klangformende Feature von Nebula. Der gemischte Output von Bank A moduliert die Phase der Oszillatoren in Bank B oder umgekehrt (Wechselbar durch einen Switch).

```
output_B = wavetable[phase_B + output_A * pmAmount]
```

**Parameter:**
- **PM Amount** — Wie stark Bank A/B die Phase von Bank B/A beeinflusst (0 = kein Effekt, voll = chaotisch)
- **PM Direction** — Switch mit 3 Positionen:
  - A → B (Bank A moduliert Bank B, Standard)
  - B → A (Bank B moduliert Bank A)
  - A ↔ B (gegenseitig, Feedback-Schleife, chaotischste Ergebnisse)

### 2.4 Globale Effekte

#### Filter
- **Typ:** Biquad-Filter (12 dB/Oktave) / Moog Ladder Filter (24 dB) ?
- **Cutoff** — 
- **Resonance** — 
- **LP/HP Switch** — Tiefpass oder Hochpass

#### Reverb
- **Algorithmus:** 
- **Size** — "Raumgröße"
- **Mix** — Dry/Wet-Verhältnis

#### Drift
- **Funktion:** Zufällige, langsame Frequenzschwankungen auf alle VCOs
- **Umsetzung:** Unabhängiger Smooth-Random-LFO pro VCO (0.01–0.1 Hz) oder für alle VCO´s 
- **Amount** — Intensität der Schwankung 

#### Master
- **Volume** — Gesamtlautstärke vor dem Ausgang

### 2.6 CV-Eingänge

| CV Input | Ziel | Beschreibung |
|---|---|---|
| **Morph A** | Bank A Morph | Wavetable-Position von außen steuern |
| **Morph B** | Bank B Morph | Wavetable-Position von außen steuern |
| **PM Amount** | PM Intensität | Phasenmodulation von außen modulieren |
| **Cutoff** | Filter Cutoff | Filter-Sweeps per externem LFO |
| **Drift** | Drift Amount | Instabilität von außen steuerbar |

Alle CV-Inputs: ±5V Bereich. Knob-Wert + CV-Wert = finaler Parameter.

### 2.7 Outputs

- **Out L** — Linker Stereokanal
- **Out R** — Rechter Stereokanal

---

## 3. Panel-Layout (Konzept)

```

```

---

## 4. Build

```bash
export RACK_DIR=/path/to/Rack-SDK
make install
```

