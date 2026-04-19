#include "plugin.hpp"
#include "dsp/wavetable.h"
#include "dsp/wavetableOSC.h"
#include "dsp/wavetablePresets.h"

struct Nebula : Module {
    enum ParamId {
        PITCH_PARAM,
        MORPH_PARAM,
        PARAMS_LEN
    };
    enum InputId {
        INPUTS_LEN
    };
    enum OutputId {
        AUDIO_OUTPUT,
        OUTPUTS_LEN
    };
    enum LightId {
        LIGHTS_LEN
    };

    Wavetable wavetable;
    WavetableOsc osc;

    float morphSmoothed = 0.f;
    float freqSmoothed = 110.f;

    Nebula() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
        configParam(PITCH_PARAM, 20.f, 500.f, 110.f, "Pitch", " Hz");
        configParam(MORPH_PARAM, 0.f, 1.f, 0.f, "Morph");
        configOutput(AUDIO_OUTPUT, "Audio");

        WavetablePresets::generate(wavetable, WavetablePresetId::Basic);
    }

    void process(const ProcessArgs& args) override {
        float freqTarget = params[PITCH_PARAM].getValue();
        float morphTarget = params[MORPH_PARAM].getValue();

        float morphAlpha = 1.f - std::exp(-args.sampleTime / 0.005f);
        float freqAlpha  = 1.f - std::exp(-args.sampleTime / 0.002f);

        morphSmoothed += (morphTarget - morphSmoothed) * morphAlpha;
        freqSmoothed  += (freqTarget  - freqSmoothed)  * freqAlpha;

        float sample = osc.process(freqSmoothed, args.sampleTime, wavetable, morphSmoothed);

        outputs[AUDIO_OUTPUT].setChannels(1);
        outputs[AUDIO_OUTPUT].setVoltage(5.f * sample);
    }
};
// UI
struct NebulaWidget : ModuleWidget {
    NebulaWidget(Nebula* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

        addParam(createParamCentered<RoundLargeBlackKnob>(
            Vec(25.4, 60), module, Nebula::PITCH_PARAM));
        addParam(createParamCentered<RoundBlackKnob>(
            Vec(25.4, 100), module, Nebula::MORPH_PARAM));
        addOutput(createOutputCentered<PJ301MPort>(
            Vec(25.4, 140), module, Nebula::AUDIO_OUTPUT));
    }
};

Model* modelNebula = createModel<Nebula, NebulaWidget>("Nebula");