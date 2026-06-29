# Nebula — Drone-Synthesizer

**VCV Rack 2 Plugin-Modul**  
*Emiel Kästner — Sommersemester 2026*

---

## Übersicht

Nebula ist ein zweibankiger Wavetable-Drone/Texture-Synthesizer für VCV Rack 2.
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
- **11 CV-Eingänge** für externe Modulation (Pitch, Morph, Volume, Cutoff, Phaser-LFO)

---

## Architektur

### Pro Bank

| Komponente | 
|-----------|
| **Wavetable** | 256 Frames × 2048 Samples, `generate1()` (5 Wellenformen) oder `generate2()` (12 Harmonische additiv) |
| **Unisono** | 3 Stimmen mit asymmetrischer Detune (−15 / 0 / +15 ct), pro Stimme eigene Lautstärke, zufällige Startphasen |
| **Sub-OSC** | Rechteckwelle, fest eine Oktave unter dem Haupt-Pitch |
| **Morph** | Weicher Übergang über alle 256 Frames, CV-steuerbar |

### Global / Gemeinsam

| Komponente |
|-----------|
| **Ladder Filter** | 
| **Cutoff** | 
| **Resonanz** | 
| **Cross-Spill** | 

### Effekte (pro Bank)

| Effekt | Implementierung |
|--------|----------------|
| **Phaser A / B** | 6 × Allpass 1. Ordnung, Bilinear-Transformation, exponentieller LFO-Sweep (10–4000 Hz), tanh-Feedback |
| **Chorus A / B** | 6 Delay-Stimmen (18–52 ms), Dreieck-LFO, Constant-Power-Stereo-Panning, Ringpuffer (8192 Samples) |

---

## Build-Anleitung

### Voraussetzungen

- **VCV Rack 2** (die Anwendung) — [vcvrack.com](https://vcvrack.com/)
- **VCV Rack 2 SDK** Installieren:
  ```
  Unter dem Kapitel: **Building Rack plugins** 
  die Rack-SDK herunterladen und entpacken. 
  
  https://vcvrack.com/manual/Building
  
  ```


### Build mit Make 

```bash
# Im Nebula-Projektverzeichnis:
export RACK_DIR=/pfad/zum/Rack-SDK
make install
```

`make install` kompiliert das Plugin und kopiert es automatisch in den VCV-Rack-Plugin-Ordner.

### Ausführen

1. VCV Rack 2 starten
2. Rechtsklick im Modul-Browser → „Nebula" suchen
3. Modul zum Patch hinzufügen

---

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
