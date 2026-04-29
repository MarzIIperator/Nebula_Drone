#include "plugin.hpp"
#include "dsp/wavetable.h"
#include "dsp/wavetableOSC.h"
#include <osdialog.h>
#include <iostream>
#include "dsp/LFO.h"
#include "dsp/LadderFilter.h"


struct Nebula : Module {
    enum ParamId {
        PITCH_A_PARAM, FINE_A_PARAM, MORPH_A_PARAM, VOLUME_A_PARAM,
        PAN_A_PARAM, SUB_LEVEL_A_PARAM, SUB_OCTAVE_A_PARAM, PRESET_A_PARAM,
        CUTOFF_A_B_PARAM, RES_A_B_PARAM, FILTER_MODE_A_B_PARAM,
        PITCH_B_PARAM, FINE_B_PARAM, MORPH_B_PARAM, VOLUME_B_PARAM,
        PAN_B_PARAM, SUB_LEVEL_B_PARAM, SUB_OCTAVE_B_PARAM, PRESET_B_PARAM,
        LFO_RATE_A_PARAM, LFO_DEPTH_A_PARAM, LFO_SHAPE_A_PARAM,
        LFO_RATE_B_PARAM, LFO_DEPTH_B_PARAM, LFO_SHAPE_B_PARAM,
        PARAMS_LEN
    };
    enum InputId { INPUTS_LEN };
    enum OutputId { AUDIO_LEFT_OUTPUT, AUDIO_RIGHT_OUTPUT, OUTPUTS_LEN };
    enum LightId { ADDITIVE_A_LIGHT, WAV_A_LIGHT, ADDITIVE_B_LIGHT, WAV_B_LIGHT, LIGHTS_LEN };

    Wavetable wavetableA, wavetableB;
    WavetableOsc mainOscA, subOscA, mainOscB, subOscB;
    LFO lfoA, lfoB;

    // BiquadFilter von der VCV Rack SDK
    LadderFilter filterA, filterB;

    float morphASmoothed = 0.f, morphBSmoothed = 0.f;
    float freqASmoothed = 110.f, freqBSmoothed = 110.f;

    int lastPresetA = -1, lastPresetB = -1;
    bool wavLoadedA = false, wavLoadedB = false;

    Nebula() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);

        configParam(PITCH_A_PARAM, -3.f, 3.f, 0.f, "Pitch A", " Oct");
        configParam(MORPH_A_PARAM, 0.f, 1.f, 0.f, "Morph A");
        configParam(VOLUME_A_PARAM, 0.f, 1.f, 0.8f, "Volume A");
        configParam(SUB_OCTAVE_A_PARAM, 0.f, 1.f, 0.f, "Sub Octave A");
        configParam(PRESET_A_PARAM, 0.f, 1.f, 0.f, "Preset A");
        configParam(FINE_A_PARAM, -100.f, 100.f, 0.f, "Fine A");
        configParam(PAN_A_PARAM, -1.f, 1.f, 0.f, "Pan A", " %");
        configParam(LFO_RATE_A_PARAM, 0.01f, 10.f, 1.f, "LFO Rate A", " Hz");
        configParam(LFO_DEPTH_A_PARAM, 0.f, 1.f, 0.f, "LFO Depth A", " %");
        configSwitch(LFO_SHAPE_A_PARAM, 0.f, 2.f, 0.f, "LFO Shape A", {"Sine", "Triangle", "Random"});

        configParam(PITCH_B_PARAM, -3.f, 3.f, 0.f, "Pitch B", " Oct");
        configParam(MORPH_B_PARAM, 0.f, 1.f, 0.f, "Morph B");
        configParam(VOLUME_B_PARAM, 0.f, 1.f, 0.8f, "Volume B");
        configParam(SUB_OCTAVE_B_PARAM, 0.f, 1.f, 0.f, "Sub Octave B");
        configParam(PRESET_B_PARAM, 0.f, 1.f, 0.f, "Preset B");
        configParam(FINE_B_PARAM, -100.f, 100.f, 0.f, "Fine B");
        configParam(PAN_B_PARAM, -1.f, 1.f, 0.f, "Pan B", " %");
        configParam(LFO_RATE_B_PARAM, 0.01f, 10.f, 1.f, "LFO Rate A", " Hz");
        configParam(LFO_DEPTH_B_PARAM, 0.f, 1.f, 0.f, "LFO Depth A", " %");
        configSwitch(LFO_SHAPE_B_PARAM, 0.f, 2.f, 0.f, "LFO Shape A", {"Sine", "Triangle", "Random"});


       // Globaler Filter
        configParam(CUTOFF_A_B_PARAM, 3.f, 9.9f, 9.9f, "Cutoff", " Hz", M_E);
        configParam(RES_A_B_PARAM, 0.f, 1.f, 0.707f, "Resonance A");


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

        //LFO
        lfoA.setRate(params[LFO_RATE_A_PARAM].getValue());
        lfoA.setShape( (LFO::Shape)(int)params[LFO_SHAPE_A_PARAM].getValue() );
        float lfoOutA = lfoA.process(args.sampleTime);

        float depthA = params[LFO_DEPTH_A_PARAM].getValue();
        morphATarget = params[MORPH_A_PARAM].getValue() + lfoOutA * depthA;
        morphATarget = clamp(morphATarget, 0.f, 1.f);


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

        // ========== LADDER FILTER ==========
        // Param-Werte holen
        float cutoffParam = params[CUTOFF_A_B_PARAM].getValue();
        float resParam    = params[RES_A_B_PARAM].getValue();
        float cutoffHz    = std::exp(cutoffParam);


        // An beide Filter geben (Setter rufen intern update() auf)
        filterA.setSampleRate(args.sampleRate);
        filterA.setCutoff(cutoffHz);
        filterA.setResonanz(resParam);

        filterB.setSampleRate(args.sampleRate);
        filterB.setCutoff(cutoffHz);
        filterB.setResonanz(resParam);

        // Filter pro Bank anwenden (mono, vor dem Pannen)
        float filteredA = filterA.processLp(sampleA);
        float filteredB = filterB.processLp(sampleB);

        // ========== PANNING ==========

        // Equal-Power Panning Bank A
       // float panA = clamp(params[PAN_A_PARAM].getValue(), -1.f, 1.f);
       // float angleA = (panA + 1.f) * (float)M_PI_4;   // -1..+1  ->  0..π/2
       // float gainAL = std::cos(angleA);
       // float gainAR = std::sin(angleA);

        // Equal-Power Panning Bank B
       // float panB = clamp(params[PAN_B_PARAM].getValue(), -1.f, 1.f);
       // float angleB = (panB + 1.f) * (float)M_PI_4;
       // float gainBL = std::cos(angleB);
      //  float gainBR = std::sin(angleB);

        // Stereo-Summe (gefilterte Signale!)
        float left  = filteredA  ;
        float right = filteredB ;

        // 5. Main Stereo Out
        outputs[AUDIO_LEFT_OUTPUT].setVoltage(left * 5.f);
        outputs[AUDIO_RIGHT_OUTPUT].setVoltage(right * 5.f);
    }
};

struct NebulaWidget : ModuleWidget {
    NebulaWidget(Nebula* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

    // ========== BANK A ==========
    // Pitch + Fine
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22.f, 34.f)),    module, Nebula::PITCH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(35.f, 36.f)),    module, Nebula::FINE_A_PARAM));

    // Wavetable
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.f, 55.f)),         module, Nebula::MORPH_A_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.f, 71.f)),         module, Nebula::VOLUME_A_PARAM));

    // Sub Octave
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.f, 84.5f)),                  module, Nebula::SUB_OCTAVE_A_PARAM));

    // LFO
    addParam(createParamCentered<Trimpot>(mm2px(Vec(13.f, 103.5f)),              module, Nebula::LFO_RATE_A_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(22.f, 103.5f)),              module, Nebula::LFO_DEPTH_A_PARAM));
    addParam(createParamCentered<CKSSThree>(mm2px(Vec(31.f, 103.75f)),           module, Nebula::LFO_SHAPE_A_PARAM));

    // Mode Switch + LEDs
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.f, 114.5f)),                 module, Nebula::PRESET_A_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(15.f, 114.f)), module, Nebula::ADDITIVE_A_LIGHT));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(29.f, 114.f)),   module, Nebula::WAV_A_LIGHT));

    // Output L
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.f, 120.5f)),         module, Nebula::AUDIO_LEFT_OUTPUT));


    // ========== GLOBAL (Center) ==========
    // Filter
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(63.5f, 38.f)),   module, Nebula::CUTOFF_A_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 54.f)),    module, Nebula::RES_A_B_PARAM));


    // ========== BANK B ==========
    // Fine + Pitch (Fine LINKS, Pitch zentriert — gespiegelt zu Bank A)
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(92.f, 36.f)),    module, Nebula::FINE_B_PARAM));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(105.f, 34.f)),   module, Nebula::PITCH_B_PARAM));

    // Wavetable
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105.f, 55.f)),        module, Nebula::MORPH_B_PARAM));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105.f, 71.f)),        module, Nebula::VOLUME_B_PARAM));

    // Pan / Octave / Sub  (gespiegelt: Pan links, Sub rechts)
    addParam(createParamCentered<CKSS>(mm2px(Vec(105.f, 84.5f)),                 module, Nebula::SUB_OCTAVE_B_PARAM));

    // LFO (gespiegelt: Shape links, Rate rechts)
    addParam(createParamCentered<CKSSThree>(mm2px(Vec(96.f, 103.75f)),           module, Nebula::LFO_SHAPE_B_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(105.f, 103.5f)),             module, Nebula::LFO_DEPTH_B_PARAM));
    addParam(createParamCentered<Trimpot>(mm2px(Vec(114.f, 103.5f)),             module, Nebula::LFO_RATE_B_PARAM));

    // Mode Switch + LEDs
    addParam(createParamCentered<CKSS>(mm2px(Vec(105.f, 114.5f)),                module, Nebula::PRESET_B_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(98.f, 114.f)), module, Nebula::ADDITIVE_B_LIGHT));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(112.f, 114.f)),  module, Nebula::WAV_B_LIGHT));

    // Output R
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(105.f, 120.5f)),        module, Nebula::AUDIO_RIGHT_OUTPUT));
}
    //Dry - Wet Slider Global

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