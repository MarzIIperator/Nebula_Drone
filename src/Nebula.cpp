#include "plugin.hpp"
#include "dsp/wavetable.h"
#include "dsp/wavetableOSC.h"
#include <osdialog.h>
#include <iostream>

struct Nebula : Module {
    enum ParamId {
        PITCH_A_PARAM, FINE_A_PARAM, MORPH_A_PARAM, VOLUME_A_PARAM,
        PAN_A_PARAM, SUB_LEVEL_A_PARAM, SUB_OCTAVE_A_PARAM, PRESET_A_PARAM,
        PITCH_B_PARAM, FINE_B_PARAM, MORPH_B_PARAM, VOLUME_B_PARAM,
        PAN_B_PARAM, SUB_LEVEL_B_PARAM, SUB_OCTAVE_B_PARAM, PRESET_B_PARAM,
        PARAMS_LEN
    };
    enum InputId { INPUTS_LEN };
    enum OutputId { AUDIO_LEFT_OUTPUT, AUDIO_RIGHT_OUTPUT, OUTPUTS_LEN };
    enum LightId { ADDITIVE_A_LIGHT, WAV_A_LIGHT, ADDITIVE_B_LIGHT, WAV_B_LIGHT, LIGHTS_LEN };

    Wavetable wavetableA, wavetableB;
    WavetableOsc mainOscA, subOscA, mainOscB, subOscB;

    float morphASmoothed = 0.f, morphBSmoothed = 0.f;
    float freqASmoothed = 110.f, freqBSmoothed = 110.f;

    int lastPresetA = -1, lastPresetB = -1;
    bool wavLoadedA = false, wavLoadedB = false;

    Nebula() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(PITCH_A_PARAM, -3.f, 3.f, 0.f, "Pitch A", " Oct");
        configParam(MORPH_A_PARAM, 0.f, 1.f, 0.f, "Morph A");
        configParam(VOLUME_A_PARAM, 0.f, 1.f, 0.8f, "Volume A");
        configParam(SUB_LEVEL_A_PARAM, 0.f, 1.f, 0.f, "Sub Level A");
        configParam(SUB_OCTAVE_A_PARAM, 0.f, 1.f, 0.f, "Sub Octave A");
        configParam(PRESET_A_PARAM, 0.f, 1.f, 0.f, "Preset A");
        configParam(FINE_A_PARAM, -100.f, 100.f, 0.f, "Fine A");

        configParam(PITCH_B_PARAM, -3.f, 3.f, 0.f, "Pitch B", " Oct");
        configParam(MORPH_B_PARAM, 0.f, 1.f, 0.f, "Morph B");
        configParam(VOLUME_B_PARAM, 0.f, 1.f, 0.8f, "Volume B");
        configParam(SUB_LEVEL_B_PARAM, 0.f, 1.f, 0.f, "Sub Level B");
        configParam(SUB_OCTAVE_B_PARAM, 0.f, 1.f, 0.f, "Sub Octave B");
        configParam(PRESET_B_PARAM, 0.f, 1.f, 0.f, "Preset B");
        configParam(FINE_B_PARAM, -100.f, 100.f, 0.f, "Fine B");

        configOutput(AUDIO_LEFT_OUTPUT, "Left");
        configOutput(AUDIO_RIGHT_OUTPUT, "Right");

        std::cout << "=== NEBULA: Init ===" << std::endl;
        wavetableA.generateBasic(0);
        wavetableB.generateBasic(1);
        std::cout << "Bank A: Generate 1 loaded" << std::endl;
        std::cout << "Bank B: Generate 2 loaded" << std::endl;
    }

    void process(const ProcessArgs& args) override {
        // ========== BANK A ==========
        int presetA = (int)params[PRESET_A_PARAM].getValue();

        lights[ADDITIVE_A_LIGHT].setBrightness(presetA == 0 ? 1.f : 0.f);
        lights[WAV_A_LIGHT].setBrightness(presetA == 1 ? 1.f : 0.f);

        if (presetA != lastPresetA) {
            if (presetA == 0) {
                std::cout << "Bank A: Switching to Additive (Generate 1)" << std::endl;
                wavetableA.generateBasic(0);
                wavetableA.currentMode = 0;
                wavLoadedA = false;
            } else {
                std::cout << "Bank A: Switching to WAV mode" << std::endl;
                wavetableA.currentMode = 1;
            }
            lastPresetA = presetA;
        }

        float octaveA = params[PITCH_A_PARAM].getValue();
        float fineA = params[FINE_A_PARAM].getValue();
        octaveA += fineA / 1200.f;

        float pitchA = 110.f * std::pow(2.f, octaveA);
        float morphATarget = params[MORPH_A_PARAM].getValue();

        float alpha = 1.f - std::exp(-args.sampleTime / 0.005f);
        morphASmoothed += (morphATarget - morphASmoothed) * alpha;
        freqASmoothed += (pitchA - freqASmoothed) * alpha;

        float sampleA = mainOscA.process(freqASmoothed, args.sampleTime, wavetableA, morphASmoothed);

        // ===== Sub OSC ======

        float subLevelA = params[SUB_LEVEL_A_PARAM].getValue();

        if (subLevelA > 0.f)
        {
            // 1. Oktave auslesen (0 oder 1)
            int subOctaveA = (int)params[SUB_OCTAVE_A_PARAM].getValue();;

            // 2. Sub-Frequenz berechnen
            float freqSubA = freqASmoothed / (subOctaveA == 0 ? 2.f : 4.f);

            // 3. Sub-Oszillator aufrufen
            float subSampleA = subOscA.process(freqSubA, args.sampleTime, wavetableA, morphASmoothed);;

            // 4. Sub zu Main addieren
            sampleA += subSampleA * subLevelA;;
        }

        float volumeA = params[VOLUME_A_PARAM].getValue();
        sampleA *= volumeA;


        // ========== BANK B ==========
        int presetB = (int)params[PRESET_B_PARAM].getValue();

        lights[ADDITIVE_B_LIGHT].setBrightness(presetB == 0 ? 1.f : 0.f);
        lights[WAV_B_LIGHT].setBrightness(presetB == 1 ? 1.f : 0.f);

        if (presetB != lastPresetB) {
            if (presetB == 0) {
                std::cout << "Bank B: Switching to Additive (Generate 2)" << std::endl;
                wavetableB.generateBasic(1);
                wavetableB.currentMode = 0;
                wavLoadedB = false;
            } else {
                std::cout << "Bank B: Switching to WAV mode" << std::endl;
                wavetableB.currentMode = 1;
            }
            lastPresetB = presetB;
        }

        float octaveB = params[PITCH_B_PARAM].getValue();
        float fineB = params[FINE_B_PARAM].getValue();
        octaveB += fineB / 1200.f;

        float pitchB = 110.f * std::pow(2.f, octaveB);
        float morphBTarget = params[MORPH_B_PARAM].getValue();

        morphBSmoothed += (morphBTarget - morphBSmoothed) * alpha;
        freqBSmoothed += (pitchB - freqBSmoothed) * alpha;

        float sampleB = mainOscB.process(freqBSmoothed, args.sampleTime, wavetableB, morphBSmoothed);

        // ===== Sub OSC ======

        float subLevelB = params[SUB_LEVEL_B_PARAM].getValue();

        if (subLevelB > 0.f)
        {
            int subOctaveB = (int)params[SUB_OCTAVE_B_PARAM].getValue();;

            float freqSubB = freqBSmoothed / (subOctaveB == 0 ? 2.f : 4.f);

            float subSampleB = subOscB.process(freqSubB, args.sampleTime, wavetableB, morphBSmoothed);;

            sampleB += subSampleB * subLevelB;;
        }

        float volumeB = params[VOLUME_B_PARAM].getValue();
        sampleB *= volumeB;

        outputs[AUDIO_LEFT_OUTPUT].setVoltage(sampleA * 5.f);
        outputs[AUDIO_RIGHT_OUTPUT].setVoltage(sampleB * 5.f);
    }
};

struct NebulaWidget : ModuleWidget {
    NebulaWidget(Nebula* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

    // KORREKTE Umrechnung: viewBox → Pixel
    // 25 HP = 375px, viewBox = 127 → Faktor = 2.953

    float bankAX = 94;    // 31.75mm im SVG
    float bankBX = 281;   // 95.25mm im SVG

    float pitchY = 103;   // 35mm
    float morphY = 157;   // 53mm
    float volumeY = 207;  // 70mm
    float subY = 257;     // 87mm
    float switchY = 301;  // 102mm
    float outputY = 348;  // 118mm

    // BANK A
    addParam(createParamCentered<RoundLargeBlackKnob>(Vec(bankAX, pitchY), module, Nebula::PITCH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(Vec(45, 110), module, Nebula::FINE_A_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(bankAX, morphY), module, Nebula::MORPH_A_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(bankAX, volumeY), module, Nebula::VOLUME_A_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(bankAX, subY), module, Nebula::SUB_LEVEL_A_PARAM));
    addParam(createParamCentered<CKSS>(Vec(bankAX, switchY), module, Nebula::PRESET_A_PARAM));
    addChild(createLightCentered<MediumLight<GreenLight>>(Vec(bankAX - 17, switchY), module, Nebula::ADDITIVE_A_LIGHT));
    addChild(createLightCentered<MediumLight<RedLight>>(Vec(bankAX + 17, switchY), module, Nebula::WAV_A_LIGHT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(bankAX, outputY), module, Nebula::AUDIO_LEFT_OUTPUT));

    // BANK B
    addParam(createParamCentered<RoundLargeBlackKnob>(Vec(bankBX, pitchY), module, Nebula::PITCH_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(Vec(335, 110), module, Nebula::FINE_B_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(bankBX, morphY), module, Nebula::MORPH_B_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(bankBX, volumeY), module, Nebula::VOLUME_B_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(Vec(bankBX, subY), module, Nebula::SUB_LEVEL_B_PARAM));
    addParam(createParamCentered<CKSS>(Vec(bankBX, switchY), module, Nebula::PRESET_B_PARAM));
    addChild(createLightCentered<MediumLight<GreenLight>>(Vec(bankBX - 17, switchY), module, Nebula::ADDITIVE_B_LIGHT));
    addChild(createLightCentered<MediumLight<RedLight>>(Vec(bankBX + 17, switchY), module, Nebula::WAV_B_LIGHT));
    addOutput(createOutputCentered<PJ301MPort>(Vec(bankBX, outputY), module, Nebula::AUDIO_RIGHT_OUTPUT));
}

    void appendContextMenu(Menu* menu) override {
        Nebula* module = dynamic_cast<Nebula*>(this->module);
        if (!module) return;

        menu->addChild(new MenuSeparator);

        menu->addChild(createMenuItem("Load WAV for Bank A", "", [module]() {
            char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL,
                                       osdialog_filters_parse("WAV files:wav"));
            if (path) {
                std::cout << "=== Loading WAV for Bank A ===" << std::endl;
                module->wavetableA.loadFromWav(path);
                module->wavetableA.currentMode = 1;
                module->wavLoadedA = true;
                module->params[Nebula::PRESET_A_PARAM].setValue(1.f);
                std::cout << "Bank A currentMode set to: " << module->wavetableA.currentMode << std::endl;
                free(path);
            }
        }));

        menu->addChild(createMenuItem("Load WAV for Bank B", "", [module]() {
            char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL,
                                       osdialog_filters_parse("WAV files:wav"));
            if (path) {
                std::cout << "=== Loading WAV for Bank B ===" << std::endl;
                module->wavetableB.loadFromWav(path);
                module->wavetableB.currentMode = 1;
                module->wavLoadedB = true;
                module->params[Nebula::PRESET_B_PARAM].setValue(1.f);
                std::cout << "Bank B currentMode set to: " << module->wavetableB.currentMode << std::endl;
                free(path);
            }
        }));
    }
};

Model* modelNebula = createModel<Nebula, NebulaWidget>("Nebula");