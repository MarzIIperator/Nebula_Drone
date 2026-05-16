#include "Nebula.h"
#include "plugin.hpp"
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

NebulaWidget::NebulaWidget(Nebula* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

    // =====================================================================
    //  VERTIKALES RASTER (in mm)
    //  VCV Rack Panels sind exakt 128.5 mm hoch.
    // =====================================================================
    const float Y_PITCH  = 18.f;
    const float Y_MORPH  = 34.f;
    const float Y_VOL    = 50.f;
    const float Y_SUB    = 66.f;
    const float Y_PHASER = 82.f;

    // --- HIER IST DEIN NEUER PLATZ! ---
    const float Y_CHORUS = 100.f;

    const float Y_BOTTOM = 118.f; // Ganz unten für Jacks & Switches
    const float LBL_OFF  = -7.f;  // Labels sitzen immer 7mm über den Reglern

    // =====================================================================
    // ============== BANK A (Links, X-Zentrum: 22) ==============
    // =====================================================================

    addLabel(this, 22, Y_PITCH + LBL_OFF, "PITCH");
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, Y_PITCH)), module, Nebula::PITCH_A_CV_INPUT));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22, Y_PITCH)), module, Nebula::PITCH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(33, Y_PITCH)), module, Nebula::FINE_A_PARAM));

    addLabel(this, 22, Y_MORPH + LBL_OFF, "MORPH");
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, Y_MORPH)), module, Nebula::MORPH_A_CV_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22, Y_MORPH)), module, Nebula::MORPH_A_PARAM));

    addLabel(this, 22, Y_VOL + LBL_OFF, "VOLUME");
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, Y_VOL)), module, Nebula::VOLUME_A_CV_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22, Y_VOL)), module, Nebula::VOLUME_A_PARAM));

    addLabel(this, 22, Y_SUB + LBL_OFF, "SUB");
    addParam(createParamCentered<CKSS>(mm2px(Vec(15, Y_SUB)), module, Nebula::SUB_OCTAVE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(28, Y_SUB)), module, Nebula::SUB_LEVEL_A_PARAM));

    addLabel(this, 22, Y_PHASER + LBL_OFF, "PHASER");
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8, Y_PHASER)), module, Nebula::PHASER_RATE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(17, Y_PHASER)), module, Nebula::PHASER_DEPTH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, Y_PHASER)), module, Nebula::PHASER_FB_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35, Y_PHASER)), module, Nebula::PHASER_LFO_A_INPUT));

    // --- CHORUS BANK A (Platzhalter für deine neuen Regler) ---
    addLabel(this, 22, Y_CHORUS + LBL_OFF, "CHORUS");
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(17, Y_CHORUS)), module, Nebula::CHORUS_RATE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, Y_CHORUS)), module, Nebula::CHORUS_DEPTH_A_PARAM));

    // AUDIO OUT & PRESET WÄHLER
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11, Y_BOTTOM)), module, Nebula::AUDIO_LEFT_OUTPUT));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(20, Y_BOTTOM)), module, Nebula::ADDITIVE_A_LIGHT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(25, Y_BOTTOM)), module, Nebula::PRESET_A_PARAM));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(30, Y_BOTTOM)), module, Nebula::WAV_A_LIGHT));


    // =====================================================================
    // ============== MIDDLE / GLOBAL (Zentrum: 63.5) ==============
    // =====================================================================

    const float Y_MID_FILTER = 22.f;
    const float Y_MID_PM     = 42.f;
    const float Y_MID_MIXA   = 72.f;
    const float Y_MID_MIXB   = 110.f;

    addLabel(this, 63.5, Y_MID_FILTER + LBL_OFF - 2, "FILTER", 9.5f);
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(63.5, Y_MID_FILTER)), module, Nebula::CUTOFF_A_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(50, Y_MID_FILTER + 8)), module, Nebula::RES_A_B_PARAM));

    addLabel(this, 63.5, Y_MID_PM + LBL_OFF, "PHASE MOD", 9.5f);
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(55, Y_MID_PM + 2)), module, Nebula::PM_AMOUNT_PARAM));
    addParam(createParamCentered<CKSSThree>(mm2px(Vec(72, Y_MID_PM)), module, Nebula::PM_DIRECTION_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(85, Y_MID_PM + 2)), module, Nebula::PM_SCALE_PARAM));

    // --- MIX A Sektion ---
    addLabel(this, 63.5, Y_MID_MIXA - 20.0f, "MIX A", 9.5f);

    const float LABEL_OFFSET_Y = -14.5f;

    addLabel(this, 50, Y_MID_MIXA + LABEL_OFFSET_Y, "PH",  7.0f);
    addLabel(this, 58, Y_MID_MIXA + LABEL_OFFSET_Y, "PM",  7.0f);
    addLabel(this, 66, Y_MID_MIXA + LABEL_OFFSET_Y, "FLT", 7.0f);
    addLabel(this, 74, Y_MID_MIXA + LABEL_OFFSET_Y, "CH",  7.0f);

    // Slider Mix A
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(50, Y_MID_MIXA)), module, Nebula::PHASER_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(58, Y_MID_MIXA)), module, Nebula::PM_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(66, Y_MID_MIXA)), module, Nebula::FILTER_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(74, Y_MID_MIXA)), module, Nebula::CHORUS_MIX_PARAM_A));

    // --- MIX B Sektion ---
    // Hier ist wichtig, dass MIX B nicht mit dem unteren Ende von A kollidiert
    addLabel(this, 63.5, Y_MID_MIXB - 20.0f, "MIX B", 9.5f);

    addLabel(this, 50, Y_MID_MIXB + LABEL_OFFSET_Y, "PH",  7.0f);
    addLabel(this, 58, Y_MID_MIXB + LABEL_OFFSET_Y, "PM",  7.0f);
    addLabel(this, 66, Y_MID_MIXB + LABEL_OFFSET_Y, "FLT", 7.0f);
    addLabel(this, 74, Y_MID_MIXB + LABEL_OFFSET_Y, "CH",  7.0f);

    // Slider Mix B
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(50, Y_MID_MIXB)), module, Nebula::PHASER_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(58, Y_MID_MIXB)), module, Nebula::PM_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(66, Y_MID_MIXB)), module, Nebula::FILTER_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(74, Y_MID_MIXB)), module, Nebula::CHORUS_MIX_PARAM_B));;;
    // =====================================================================
    // ============== BANK B (Rechts, X-Zentrum: 105) ==============
    // =====================================================================

    addLabel(this, 105, Y_PITCH + LBL_OFF, "PITCH");
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(94, Y_PITCH)), module, Nebula::FINE_B_PARAM));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(105, Y_PITCH)), module, Nebula::PITCH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, Y_PITCH)), module, Nebula::PITCH_B_CV_INPUT));

    addLabel(this, 105, Y_MORPH + LBL_OFF, "MORPH");
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105, Y_MORPH)), module, Nebula::MORPH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, Y_MORPH)), module, Nebula::MORPH_B_CV_INPUT));

    addLabel(this, 105, Y_VOL + LBL_OFF, "VOLUME");
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105, Y_VOL)), module, Nebula::VOLUME_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, Y_VOL)), module, Nebula::VOLUME_B_CV_INPUT));

    addLabel(this, 105, Y_SUB + LBL_OFF, "SUB");
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(99, Y_SUB)), module, Nebula::SUB_LEVEL_B_PARAM));
    addParam(createParamCentered<CKSS>(mm2px(Vec(112, Y_SUB)), module, Nebula::SUB_OCTAVE_B_PARAM));

    addLabel(this, 105, Y_PHASER + LBL_OFF, "PHASER");
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92, Y_PHASER)), module, Nebula::PHASER_LFO_B_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(101, Y_PHASER)), module, Nebula::PHASER_RATE_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(110, Y_PHASER)), module, Nebula::PHASER_DEPTH_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(119, Y_PHASER)), module, Nebula::PHASER_FB_B_PARAM));

    // --- CHORUS BANK B (Platzhalter für deine neuen Regler) ---
    addLabel(this, 105, Y_CHORUS + LBL_OFF, "CHORUS");
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(101, Y_CHORUS)), module, Nebula::CHORUS_RATE_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(110, Y_CHORUS)), module, Nebula::CHORUS_DEPTH_B_PARAM));

    // AUDIO OUT & PRESET WÄHLER (Spiegelverkehrt zu A)
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(97, Y_BOTTOM)), module, Nebula::ADDITIVE_B_LIGHT));
    addParam(createParamCentered<CKSS>(mm2px(Vec(102, Y_BOTTOM)), module, Nebula::PRESET_B_PARAM));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(107, Y_BOTTOM)), module, Nebula::WAV_B_LIGHT));
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(116, Y_BOTTOM)), module, Nebula::AUDIO_RIGHT_OUTPUT));
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