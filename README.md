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

## 3. Wavetable-Synthese: Theorie und Umsetzung

### 3.1 Grundprinzip

Eine Wavetable ist eine Sammlung von vorberechneten Wellenformen (Frames). Jeder Frame repräsentiert einen Zyklus einer Wellenform, gespeichert als Array von Samples (z.B. 256 Werte zwischen -1 und +1).

Beim Abspielen läuft ein Phase-Akkumulator von 0 bis 1 durch die Tabelle. Die Geschwindigkeit bestimmt die Tonhöhe: bei 440 Hz durchläuft die Phase 440 mal pro Sekunde einen kompletten Zyklus.

**Verschiedene Amplitudenverteilungen erzeugen verschiedene Klangfarben:**
- Nur `amplitude[1] = 1`, Rest 0 → reiner Sinus
- `amplitude[h] = 1/h` für alle h → Sägezahn-ähnlich
- `amplitude[h] = 1/h` nur für ungerade h → Rechteck-ähnlich
- Zufällige Amplituden → einzigartige, organische Klangfarben (Random-Button)

### 3.2 Morphing zwischen Frames

Die 32 Frames sind aufsteigend nach Komplexität sortiert (Frame 0 = wenig Obertöne, Frame 31 = viele Obertöne). Der Morph-Knob bestimmt die Position:

```
morph = 0.0 ... 1.0
position = morph × 31
frameA = floor(position)      // Linker Nachbar-Frame
frameB = frameA + 1           // Rechter Nachbar-Frame
frac = position - frameA      // Bruchteil zwischen beiden

sample = frames[frameA][i] × (1 - frac) + frames[frameB][i] × frac
```

Dies ist lineare Interpolation zwischen zwei benachbarten Frames — ergibt stufenlose Übergänge.

### 3.3 Phase-Akkumulator und Tabellen-Lookup

```
// Pro Sample (48000x pro Sekunde):
phase += frequency / sampleRate;
if (phase >= 1.0) phase -= 1.0;

// Position in der Tabelle
pos = phase × TABLE_SIZE;
idx = floor(pos);
frac = pos - idx;
next = (idx + 1) % TABLE_SIZE;

// Lineare Interpolation zwischen Nachbar-Samples
output = table[idx] × (1 - frac) + table[next] × frac;
```

---

## 4. Phasenmodulation: Theorie

### 4.1 Prinzip

Bei der Phasenmodulation wird die Phase eines Oszillators durch das Ausgangssignal eines anderen Oszillators verschoben:

```
output_B = wavetable_lookup(phase_B + output_A × pmAmount)
```

Yamaha's DX7 — der berühmteste "FM-Synth" — verwendet tatsächlich PM, nicht FM. PM ist mathematisch stabiler: die Grundtonhöhe bleibt erhalten, nur die Obertonstruktur ändert sich.

### 4.2 Musikalische Auswirkung

- **Ganzzahlige Frequenzverhältnisse** (1:1, 1:2, 2:3) → harmonische, musikalische Obertöne
- **Nicht-ganzzahlige Verhältnisse** → inharmonische, glocken- oder metallartige Klänge
- **Hoher PM Amount** → zunehmend chaotische, noise-artige Texturen

### 4.3 Feedback (A ↔ B)

Im gegenseitigen Modus moduliert Bank A Bank B und gleichzeitig Bank B Bank A. Dies erzeugt eine Feedback-Schleife mit unvorhersehbaren, sich selbst entwickelnden Klängen — ideal für evolvierende Drone-Texturen.

---

## 5. Dateistruktur

```
Nebula/
├── src/
│   ├── plugin.cpp              // Plugin-Registrierung
│   ├── plugin.hpp              // Globale Deklarationen
│   ├── Nebula.cpp              // Hauptmodul (Params, I/O, process())
│   └── dsp/
│       ├── Wavetable.h         // Wavetable-Klasse (Frames, Morph, Lookup)
│       ├
│       ├── Oscillator.h        // Phase-Akkumulator, Tabellen-Lookup
│       ├── LFO.h               // Low Frequency Oscillator
│       ├── Filter.h            
│       ├── Reverb.h            
│       ├── PhaseMod.h          // Phasenmodulations-Logik
│       ├── Drift.h             // Random-Frequenzschwankung
│       └── Utils.h             // Panning, Clipping, Interpolation
├── res/
│   └── Nebula.svg              // Panel-Design (Inkscape)
├── plugin.json                 // Modul-Manifest
├── Makefile                    // Build-Konfiguration
└── README.md                   // Diese Datei
```

---

## 6. Panel-Layout (Konzept)

```
┌─────────────────────────────────────────┐
│              N E B U L A                │
│         drone texture synthesizer       │
│                                         │
│   ┌─ Bank A ──┐     ┌─ Bank B ──┐      │
│   │ Pitch Fine│     │ Pitch Fine│      │
│   │ Vol   Pan │     │ Vol   Pan │      │
│   │ SubLv Sub8│     │ SubLv Sub8│      │
│   │ Morph     │     │ Morph     │      │
│   │ LFO R/D/S │     │ LFO R/D/S │      │
│   └───────────┘     └───────────┘      │
│                                         │
│          ┌─ PM ──────────┐              │
│          │ Amount  Dir   │              │
│          └───────────────┘              │
│                                         │
│   ┌─ Filter ─┐  ┌ Reverb ┐  ┌Drift┐   │
│   │ Cut Res  │  │ Sz Mix │  │ Amt │   │
│   │ LP/HP    │  └────────┘  └─────┘   │
│   └──────────┘              [Random]   │
│                              Master    │
│                                         │
│  CV: MrA MrB PM  Cut Drf    Out: L R  │
└─────────────────────────────────────────┘
```

---

## 7. Implementierungsplan

### Phase 1 — Grundstruktur (Woche 1–3, ~40h)

- [ ] VCV Rack SDK Setup und leeres Modul kompiliert
- [ ] `Wavetable.h`: Klasse mit 32 Frames, TABLE_SIZE = 256
- [ ] `WavetableGenerator.h`: Frames per additiver Synthese vorberechnen
- [ ] `Oscillator.h`: Phase-Akkumulator, Tabellen-Lookup mit Interpolation
- [ ] 1 Oszillator mit Morph-Knob bringt Sound in VCV Rack
- [ ] Auf 2 VCOs erweitern (1 Bank mit Haupt + Sub)
- [ ] Git Repository aufsetzen, saubere Commits

**Milestone:** Ein Oszillator spielt hörbar in VCV Rack, Morph funktioniert.

### Phase 2 — 2-Bank-System (Woche 4–5, ~24h)

- [ ] Auf 2 Bänke erweitern (4 VCOs total)
- [ ] Pitch, Fine, Volume, Pan pro Bank
- [ ] Sub-Level und Sub-Octave-Switch pro Bank
- [ ] Jede Bank hat eigene Wavetable
- [ ] Stereo-Panning (Constant Power)

**Milestone:** Zwei unabhängige Drone-Bänke mit Stereo-Output.

### Phase 3 — Phasenmodulation (Woche 6–7, ~20h)

- [ ] `PhaseMod.h`: PM-Logik implementieren
- [ ] PM-Amount Knob
- [ ] PM-Direction Switch (A→B / B→A / A↔B)
- [ ] Testen und Feintuning des PM-Bereichs

**Milestone:** Bänke modulieren sich gegenseitig, PM-Amount steuert Klangkomplexität.

### Phase 4 — Modulation (Woche 8–9, ~20h)

- [ ] `LFO.h`: Sine, Triangle, Smooth Random
- [ ] 1 LFO pro Bank, standardmäßig auf Pitch normalisiert
- [ ] `Drift.h`: Smooth-Random pro VCO
- [ ] Drift Amount Knob
- [ ] Random-Button: Wavetables neu generieren per Knopfdruck

**Milestone:** Klang bewegt sich organisch, Random-Button erzeugt neue Paletten.

### Phase 5 — Effekte (Woche 10–11, ~20h)

- [ ] `Filter.h`: Biquad-Filter mit LP/HP, Cutoff, Resonance
- [ ] `Reverb.h`: FreeVerb oder Schroeder Reverb
- [ ] Filter und Reverb in Signalkette integrieren

**Milestone:** Filter-Sweeps und räumliche Tiefe hörbar.

### Phase 6 — CV und Panel (Woche 12–13, ~16h)

- [ ] CV-Inputs implementieren (Morph A/B, PM, Cutoff, Drift)
- [ ] Panel-Design in Inkscape (SVG)
- [ ] Knob-Positionen im Code abgleichen

**Milestone:** Modul ist von außen steuerbar, Panel sieht professionell aus.

### Phase 7 — Testing und Dokumentation (Woche 14–15, ~10h)

- [ ] Ausgiebiges Testing in verschiedenen Patches
- [ ] CPU-Performance messen und optimieren
- [ ] Code-Dokumentation (Kommentare, README finalisieren)
- [ ] Projektbericht schreiben (5–10 Seiten)
- [ ] Distribution bauen (`make dist`)
- [ ] Präsentation vorbereiten

**Milestone:** Fertiges Plugin, Dokumentation, Abgabe.

---

## 8. Technologie-Stack

| Komponente | Technologie |
|---|---|
| Sprache | C++ (C++11) |
| Plattform | VCV Rack 2 |
| SDK | VCV Rack SDK (Makefile-basiert) |
| IDE | CLion |
| Build | Make, Clang (macOS ARM64) |
| Panel-Design | Inkscape (SVG) |
| Prototyping | Max/MSP |
| Versionskontrolle | Git / GitHub |
| Betriebssystem | macOS (Apple Silicon) |

---

## 9. Glossar

| Begriff | Erklärung |
|---|---|
| **Wavetable** | Sammlung von vorberechneten Wellenformen (Frames), durch die per Morph-Knob gescannt wird |
| **Frame** | Ein einzelner Wellenzyklus innerhalb einer Wavetable (z.B. 256 Samples) |
| **Morph** | Stufenloses Überblenden zwischen benachbarten Frames per Interpolation |
| **Additive Synthese** | Klangerzeugung durch Übereinanderstapeln von Sinuswellen (Grundton + Obertöne) |
| **Phasenmodulation (PM)** | Ein Oszillator verschiebt die Phase eines anderen → neue Obertöne entstehen |
| **Phase-Akkumulator** | Zähler der pro Sample um `freq/sampleRate` hochzählt und bei 1.0 zurückspringt |
| **Interpolation** | Berechnung von Zwischenwerten zwischen zwei diskreten Tabellenpunkten |
| **Nyquist-Frequenz** | Halbe Samplerate (24 kHz bei 48 kHz) — Obertöne darüber erzeugen Aliasing |
| **Band-Limiting** | Entfernen von Obertönen über der Nyquist-Frequenz zur Vermeidung von Aliasing |
| **Mip-Mapping** | Mehrere Versionen einer Wavetable mit unterschiedlich vielen Obertönen |
| **LFO** | Low Frequency Oscillator — langsamer Oszillator zur Modulation von Parametern |
| **Drift** | Zufällige, langsame Frequenzschwankung zur Simulation analoger Instabilität |
| **CV** | Control Voltage — Steuerspannung im Eurorack/VCV-Standard (±5V oder 0–10V) |
| **Biquad** | Digitaler Filter 2. Ordnung, kann LP/HP/BP/Notch realisieren |
| **FreeVerb** | Public-Domain Reverb-Algorithmus (8 Comb-Filter + 4 Allpass-Filter) |
| **Constant Power Panning** | Panning-Methode bei der die wahrgenommene Lautstärke konstant bleibt |

---

## 11. Quellen und Ressourcen

### VCV Rack
- [VCV Rack Manual](https://vcvrack.com/manual)
- [VCV Rack Plugin Tutorial](https://vcvrack.com/manual/PluginDevelopmentTutorial)
- [VCV Rack API Reference](https://vcvrack.com/docs-v2)

### Wavetable-Synthese
- [Futur3soundz: Wavetable Synthesis](https://www.futur3soundz.com/wavetable-synthesis/)
- [Keith McMillen: Wavetable Synthesis 101](https://www.keithmcmillen.com/blog/simple-synthesis-wavetable-synthesis/)

### DSP / Additive Synthese
- [Wikipedia: Additive Synthesis](https://en.wikipedia.org/wiki/Additive_synthesis)
- [CCRMA Stanford: Additive and Wavetable Synthesis](https://ccrma.stanford.edu/CCRMA/Courses/SoundSynthesis/Lectures/3/Slides.html)
- [Moogulator: Synthesizer Grundlagen](https://www.moogulator.com/synthesizer/)

### Referenzmodule
- [Moffenzeef Stargazer Manual](https://moffenzeefmodular.com/stargazer)
- [Befaco Oneiroi](https://www.befaco.org/oneiroi/)
- [Neuzeit Instruments Warp](https://www.neuzeit-instruments.com/warp)
- [Forge TME VHIKK X](https://forge-tme.com/vhikk-x/)

### Reverb-Algorithmen
- FreeVerb (Jezar) — Public Domain C++ Implementation
- [Julius O. Smith: Physical Audio Signal Processing](https://ccrma.stanford.edu/~jos/pasp/)

---

## Build

```bash
export RACK_DIR=/path/to/Rack-SDK
make install
```

## License

GPL-3.0-or-later
