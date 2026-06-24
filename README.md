# Nebula — Drone-Synthesizer

**VCV Rack 2 Plugin-Modul**  
*Emiel Kästner — Sommersemester 2026*

---

## Übersicht

Nebula ist ein zweibank Wavetable-Drone/Texture-Synthesizer für VCV Rack 2.
Das Modul erzeugt dichte, sich langsam entwickelnde Klangflächen durch
Wavetable-Synthese, Phasenmodulation, einen analog modellierten Ladder-Filter,
Phaser und Ensemble-Chorus.

Keine Hüllkurven, kein Keyboard — der Klang läuft dauerhaft und entwickelt sich
über interne Drift, CV-Modulation und Cross-Bank-Interaktion kontinuierlich weiter.

<img width="738" height="750" alt="Screenshot 2026-06-24 at 18 36 06" src="https://github.com/user-attachments/assets/22963e76-8fa7-4092-b2d9-26261ea8dbdc" />


### Kernfunktionen

- **Zwei unabhängige Klangbänke** (A + B), je mit 3 verstimmten Unisono-Stimmen + Sub-Oszillator
- **Wavetable-Synthese**: 256 morphbare Frames pro Bank, eingebaute Presets + ladbare WAV-Dateien
- **Phasenmodulation**: Bank A moduliert Bank B (oder umgekehrt, oder gekreuzt)
- **Moog-Ladder-Filter**: 4-stufiges nichtlineares tanh-Modell nach Huovilainen (2004), gemeinsamer Cutoff + Resonanz
- **Phaser**: 6-stufige Allpass-Kaskade mit internem/externem LFO und nichtlinearem Feedback
- **Ensemble-Chorus**: 6 Delay-Stimmen mit Dreieck-LFO, Constant-Power-Stereo-Panning
- **13 CV-Eingänge** für externe Modulation (Pitch, Morph, Volume, Cutoff, Phaser-LFO)

---

## Architektur

### Pro Bank

| Komponente | 
|-----------|
| **Wavetable** | 256 Frames × 2048 Samples, `generate1()` (5 Wellenformen) oder `generate2()` (12 Harmonische additiv) |
| **Unisono** | 3 Stimmen mit asymmetrischer Detune (−15 / 0 / +15 ct), pro Stimme eigene Lautstärke, zufällige Startphasen |
| **Sub-OSC** | Rechteckwelle, fest eine Oktave unter dem Haupt-Pitch |
| **Morph** | Weicher Übergang über alle 256 Frames, CV-steuerbar |
| **Voice Drift** | Random Walk ±5 ct pro Stimme (simuliert analoge Instabilität) |

### Global / Gemeinsam

| Komponente | 
|-----------|
| **Ladder Filter** | 
| **Cutoff** | 
| **Resonanz** | 
| **Cross-Spill** |

### Effekte (pro Bank)

| Effekt | 
|--------|
| **Phaser A / B** |
| **Chorus A / B** | 

---

## Build-Anleitung

### Voraussetzungen

- [VCV Rack 2 SDK](https://github.com/VCVRack/Rack) (nach z.B. `~/Desktop/Rack-SDK` klonen)
- C++17-Compiler (Apple Clang auf macOS, GCC auf Linux, MSVC auf Windows)
- CMake ≥ 3.20

### Build mit Make

```bash
export RACK_DIR=/path/to/Rack-SDK
make install
```

Das kompilierte Plugin (`.dylib` / `.so` / `.dll`) wird im Rack-Plugin-Ordner abgelegt.

### Build mit CMake

```bash
mkdir build && cd build
cmake .. -DRACK_SDK_DIR=/path/to/Rack-SDK
make -j$(nproc)
```

### Installieren & Ausführen

1. Die kompilierte `Nebula.dylib` (bzw. `.so`/`.dll`) in den VCV Rack `plugins-v2`-Ordner kopieren
2. VCV Rack 2 starten
3. Rechtsklick im Modul-Browser → „Nebula" suchen
4. Modul zum Patch hinzufügen


## Projektstruktur

```
Nebula/
├── CMakeLists.txt          # CMake-Build-Konfiguration
├── Makefile                # GNU-Make-Build
├── plugin.json             # VCV Rack Plugin-Metadaten
├── README.md               # Diese Datei
├── res/
│   └── Nebula.svg          # Panel-Grafik
├── src/
│   ├── plugin.cpp          # Plugin-Einstiegspunkt
│   ├── plugin.hpp
│   ├── Nebula.h            # Modul-Deklaration (Parameter, Eingänge, Ausgänge)
│   ├── Nebula.cpp          # DSP-Engine: process(), processVoices()
│   ├── NebulaWidget.cpp    # Panel-Layout und Widget-Erstellung
│   └── dsp/
│       ├── wavetable.h     # Wavetable-Datenstruktur (256 Frames × 2048)
│       ├── wavetable.cpp   # Wellenform-Generierung (generate1/2), Normalisierung, WAV-Lader
│       ├── wavetableOSC.h  # Wavetable-Oszillator (Phasor + Lookup)
│       ├── LadderFilter.h  # 4-stufiger nichtlinearer Moog-Ladder-Filter
│       ├── Phaser.h        # 6-stufiger Allpass-Phaser mit LFO
│       ├── Chorus.h        # 6-stimmiger Ensemble-Chorus mit Stereo-Panning
│       └── Utils/
│           └── AudioFile.h # WAV-Datei-Ladefunktion (externe Bibliothek)
```

## Lizenz

MIT 
