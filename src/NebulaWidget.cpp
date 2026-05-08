#include "Nebula.h"
#include <osdialog.h>

// =====================================================================
// Einfaches zentriertes Text-Label
// =====================================================================
struct TextLabel : Widget {
    std::string text;
    float fontSize = 8.5f;
    NVGcolor color = nvgRGB(40, 40, 40);

    void draw(const DrawArgs& args) override {
        std::shared_ptr<Font> font = APP->window->loadFont(
            asset::system("res/fonts/ShareTechMono-Regular.ttf"));
        if (!font) return;
        nvgFontFaceId(args.vg, font->handle);
        nvgFontSize(args.vg, fontSize);
        nvgFillColor(args.vg, color);
        nvgTextAlign(args.vg, NVG_ALIGN_CENTER | NVG_ALIGN_MIDDLE);
        nvgText(args.vg, 0, 0, text.c_str(), NULL);
    }
};

static void addLabel(NebulaWidget* w, float xMM, float yMM,
                     const std::string& text, float fs = 8.5f) {
    auto* lbl = new TextLabel();
    lbl->text = text;
    lbl->fontSize = fs;
    lbl->box.pos = mm2px(Vec(xMM, yMM));
    lbl->box.size = Vec(0, 0);
    w->addChild(lbl);
}

// =====================================================================
// Widget Constructor
// =====================================================================
//
//  LAYOUT-RASTER (Banks kompakt oben, Phaser/Out unten klar getrennt)
//  ------------------------------------------------------------------
//  Bank A center : x = 22       (CV inputs links bei x = 11)
//  Middle center : x = 63.5
//  Bank B center : x = 105      (CV inputs rechts bei x = 116)
//
//  Vertikale Zonen je Bank (~13mm Zonen):
//    y =  20  PITCH      (label)
//    y =  28  PITCH      (knobs + CV)
//    y =  41  MORPH
//    y =  49  MORPH      (knob + CV)
//    y =  61  VOLUME
//    y =  69  VOLUME     (knob + CV)
//    y =  82  SUB
//    y =  90  SUB        (switch + knob)
//    y = 102  PHASER
//    y = 110  PHASER     (3 knobs + CV)
//    y = 121  BOTTOM     (output + preset switch + lights)
// =====================================================================
NebulaWidget::NebulaWidget(Nebula* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

    // ============== BANK A ==============
    addLabel(this, 22, 20, "PITCH", 8.5f);
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 28)), module, Nebula::PITCH_A_CV_INPUT));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22, 28)), module, Nebula::PITCH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(33, 28)), module, Nebula::FINE_A_PARAM));

    addLabel(this, 22, 41, "MORPH", 8.5f);
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 49)), module, Nebula::MORPH_A_CV_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22, 49)), module, Nebula::MORPH_A_PARAM));

    addLabel(this, 22, 61, "VOLUME", 8.5f);
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, 69)), module, Nebula::VOLUME_A_CV_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22, 69)), module, Nebula::VOLUME_A_PARAM));

    addLabel(this, 22, 82, "SUB", 8.5f);
    addParam(createParamCentered<CKSS>(mm2px(Vec(15, 90)), module, Nebula::SUB_OCTAVE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28, 90)), module, Nebula::SUB_LEVEL_A_PARAM));

    addLabel(this, 22, 102, "PHASER", 8.5f);
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8, 110)), module, Nebula::PHASER_RATE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(17, 110)), module, Nebula::PHASER_DEPTH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, 110)), module, Nebula::PHASER_FB_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35, 110)), module, Nebula::PHASER_LFO_A_INPUT));

    // Bottom row: audio out (left) + lights+switch (right)
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(8, 121)), module, Nebula::AUDIO_LEFT_OUTPUT));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(17, 121)), module, Nebula::ADDITIVE_A_LIGHT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(22, 121)), module, Nebula::PRESET_A_PARAM));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(27, 121)), module, Nebula::WAV_A_LIGHT));


    // ============== MIDDLE / GLOBAL ==============
    addLabel(this, 63.5, 20, "FILTER", 9.5f);
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(63.5, 30)), module, Nebula::CUTOFF_A_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(50, 38)), module, Nebula::RES_A_B_PARAM));

    addLabel(this, 63.5, 38, "PHASE MOD", 9.5f);
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(55, 48)), module, Nebula::PM_AMOUNT_PARAM));
    addParam(createParamCentered<CKSSThree>(mm2px(Vec(72, 46)), module, Nebula::PM_DIRECTION_PARAM));

    // MIX A — Slider auf y=75
    addLabel(this, 63.5, 58, "MIX A", 9.f);
    addLabel(this, 50, 60, "PH",  6.5f);
    addLabel(this, 58, 60, "PM",  6.5f);
    addLabel(this, 66, 60, "FLT", 6.5f);
    addLabel(this, 74, 60, "CH",  6.5f);
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(50, 75)), module, Nebula::PHASER_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(58, 75)), module, Nebula::PM_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(66, 75)), module, Nebula::FILTER_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(74, 75)), module, Nebula::CHORUS_MIX_PARAM_A));

    // MIX B — Slider auf y=106
    addLabel(this, 63.5, 92, "MIX B", 9.f);
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(50, 106)), module, Nebula::PHASER_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(58, 106)), module, Nebula::PM_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(66, 106)), module, Nebula::FILTER_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(74, 106)), module, Nebula::CHORUS_MIX_PARAM_B));


    // ============== BANK B (gespiegelt) ==============
    addLabel(this, 105, 20, "PITCH", 8.5f);
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(94, 28)), module, Nebula::FINE_B_PARAM));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(105, 28)), module, Nebula::PITCH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, 28)), module, Nebula::PITCH_B_CV_INPUT));

    addLabel(this, 105, 41, "MORPH", 8.5f);
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105, 49)), module, Nebula::MORPH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, 49)), module, Nebula::MORPH_B_CV_INPUT));

    addLabel(this, 105, 61, "VOLUME", 8.5f);
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105, 69)), module, Nebula::VOLUME_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, 69)), module, Nebula::VOLUME_B_CV_INPUT));

    addLabel(this, 105, 82, "SUB", 8.5f);
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(99, 90)), module, Nebula::SUB_LEVEL_B_PARAM));
    addParam(createParamCentered<CKSS>(mm2px(Vec(112, 90)), module, Nebula::SUB_OCTAVE_B_PARAM));

    addLabel(this, 105, 102, "PHASER", 8.5f);
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92, 110)), module, Nebula::PHASER_LFO_B_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(101, 110)), module, Nebula::PHASER_RATE_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(110, 110)), module, Nebula::PHASER_DEPTH_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(119, 110)), module, Nebula::PHASER_FB_B_PARAM));

    // Bottom row: lights+switch (left) + audio out (right) — gespiegelt
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(100, 121)), module, Nebula::ADDITIVE_B_LIGHT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(105, 121)), module, Nebula::PRESET_B_PARAM));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(110, 121)), module, Nebula::WAV_B_LIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(119, 121)), module, Nebula::AUDIO_RIGHT_OUTPUT));
}

void NebulaWidget::appendContextMenu(Menu* menu) {
    Nebula* module = dynamic_cast<Nebula*>(this->module);
    if (!module) return;

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Load WAV for Bank A", "", [=]() {
        char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, osdialog_filters_parse("WAV:wav"));
        if (path) {
            module->wavetableA.loadFromWav(path);
            module->wavetableA.currentMode = 1;
            module->params[Nebula::PRESET_A_PARAM].setValue(1.f);
            free(path);
        }
    }));
    menu->addChild(createMenuItem("Load WAV for Bank B", "", [=]() {
        char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, osdialog_filters_parse("WAV:wav"));
        if (path) {
            module->wavetableB.loadFromWav(path);
            module->wavetableB.currentMode = 1;
            module->params[Nebula::PRESET_B_PARAM].setValue(1.f);
            free(path);
        }
    }));
}

Model* modelNebula = createModel<Nebula, NebulaWidget>("Nebula");
