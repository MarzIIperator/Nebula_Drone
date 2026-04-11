#include "plugin.hpp"

struct Nebula : Module {
    enum ParamId { PARAMS_LEN };
    enum InputId { INPUTS_LEN };
    enum OutputId { OUTPUTS_LEN };
    enum LightId { LIGHTS_LEN };

    Nebula() {
        config(PARAMS_LEN, INPUTS_LEN, OUTPUTS_LEN, LIGHTS_LEN);
    }

    void process(const ProcessArgs& args) override {
    }
};

struct NebulaWidget : ModuleWidget {
    NebulaWidget(Nebula* module) {
        setModule(module);
        setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));
    }
};

Model* modelNebula = createModel<Nebula, NebulaWidget>("Nebula");
