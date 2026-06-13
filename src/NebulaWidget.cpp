#include "Nebula.h"
#include "plugin.hpp"
#include <osdialog.h>

// =====================================================================
// Einfaches zentriertes Text-Label
// =====================================================================
struct TextLabel : Widget
{
    std::string text;
    float fontSize = 8.5f;
    NVGcolor color = nvgRGB(40, 40, 40);

    void draw(const DrawArgs& args) override
    {
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
                     const std::string& text, float fs = 8.5f)
{
    auto* lbl = new TextLabel();
    lbl->text = text;
    lbl->fontSize = fs;
    lbl->box.pos = mm2px(Vec(xMM, yMM));
    lbl->box.size = Vec(0, 0);
    w->addChild(lbl);
}

NebulaWidget::NebulaWidget(Nebula* module)
{
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

    // =====================================================================
    //  VERTIKALES RASTER
    // =====================================================================
    const float Y_PITCH = 18.f;
    const float Y_MORPH = 34.f;
    const float Y_VOL = 50.f;
    const float Y_SUB = 66.f;
    const float Y_PHASER = 85.f; // +1mm mehr Luft!
    const float Y_CHORUS = 104.f; // +2mm mehr Luft!
    const float Y_BOTTOM = 118.f;
    const float LBL_OFF = -7.f;
    const float LBL_SUB_OFF = -5.2f;

    // =====================================================================
    // ============== BANK A (Links, X-Zentrum: 22) ==============
    // =====================================================================

    // ----- PITCH A -----
    addLabel(this, 22, Y_PITCH + LBL_OFF, "PITCH");
    addLabel(this, 11, Y_PITCH + LBL_SUB_OFF, "CV", 7.5f);
    addLabel(this, 33, Y_PITCH + LBL_SUB_OFF, "FINE", 7.5f);

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, Y_PITCH)), module, Nebula::PITCH_A_CV_INPUT));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22, Y_PITCH)), module, Nebula::PITCH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(33, Y_PITCH)), module, Nebula::FINE_A_PARAM));

    // ----- MORPH A -----
    addLabel(this, 22, Y_MORPH + LBL_OFF, "MORPH");
    addLabel(this, 11, Y_MORPH + LBL_SUB_OFF, "CV", 7.5f);

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, Y_MORPH)), module, Nebula::MORPH_A_CV_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22, Y_MORPH)), module, Nebula::MORPH_A_PARAM));

    // ----- VOLUME A -----
    addLabel(this, 22, Y_VOL + LBL_OFF, "VOLUME");
    addLabel(this, 11, Y_VOL + LBL_SUB_OFF, "CV", 7.5f);

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(11, Y_VOL)), module, Nebula::VOLUME_A_CV_INPUT));
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22, Y_VOL)), module, Nebula::VOLUME_A_PARAM));

    // ----- SUB A -----
    addLabel(this, 22, Y_SUB + LBL_OFF, "SUB");
    addLabel(this, 22, Y_SUB + LBL_SUB_OFF, "LVL", 7.5f);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22, Y_SUB)), module, Nebula::SUB_LEVEL_A_PARAM));

    // ----- PHASER A -----
    addLabel(this, 22, Y_PHASER + LBL_OFF, "PHASER");
    addLabel(this, 8, Y_PHASER + LBL_SUB_OFF, "RATE", 7.5f);
    addLabel(this, 17, Y_PHASER + LBL_SUB_OFF, "DPTH", 7.5f);
    addLabel(this, 26, Y_PHASER + LBL_SUB_OFF, "FB", 7.5f);
    addLabel(this, 35, Y_PHASER + LBL_SUB_OFF, "CV", 7.5f);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(8, Y_PHASER)), module, Nebula::PHASER_RATE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(17, Y_PHASER)), module, Nebula::PHASER_DEPTH_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, Y_PHASER)), module, Nebula::PHASER_FB_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(35, Y_PHASER)), module, Nebula::PHASER_LFO_A_INPUT));

    // ----- CHORUS A -----
    addLabel(this, 22, Y_CHORUS + LBL_OFF, "CHORUS");
    addLabel(this, 17, Y_CHORUS + LBL_SUB_OFF, "RATE", 7.5f);
    addLabel(this, 26, Y_CHORUS + LBL_SUB_OFF, "DPTH", 7.5f);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(17, Y_CHORUS)), module, Nebula::CHORUS_RATE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(26, Y_CHORUS)), module, Nebula::CHORUS_DEPTH_A_PARAM));

    // ----- OUTPUT A -----
    addLabel(this, 11, Y_BOTTOM - 6.f, "OUT", 7.5f);
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(11, Y_BOTTOM)), module, Nebula::AUDIO_LEFT_OUTPUT));

    addLabel(this, 20, Y_BOTTOM - 3.f, "ADD", 6.f);
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(20, Y_BOTTOM)), module, Nebula::ADDITIVE_A_LIGHT));

    addLabel(this, 25, Y_BOTTOM - 6.f, "MODE", 6.f);
    addParam(createParamCentered<CKSS>(mm2px(Vec(25, Y_BOTTOM)), module, Nebula::PRESET_A_PARAM));

    addLabel(this, 30, Y_BOTTOM - 3.f, "WAV", 6.f);
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(30, Y_BOTTOM)), module, Nebula::WAV_A_LIGHT));

    // =====================================================================
    // ============== MIDDLE / GLOBAL ==============
    // =====================================================================

    const float Y_MID_FILTER = 20.f;
    const float Y_MID_PM = 40.f;
    const float Y_MID_PAN = 55.f;
    const float Y_MID_MIX = 82.f;
    const float Y_MID_VOICES = 110.f;


    // ===== FILTER =====
    addLabel(this, 63.5, Y_MID_FILTER + LBL_OFF, "FILTER", 10.f);
    addLabel(this, 51, Y_MID_FILTER + LBL_SUB_OFF, "CV", 7.5f);
    addLabel(this, 77, Y_MID_FILTER + LBL_SUB_OFF, "RES", 7.5f);

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(51, Y_MID_FILTER)), module, Nebula::CUTOFF_CV_INPUT));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(63.5, Y_MID_FILTER)), module, Nebula::CUTOFF_A_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(77, Y_MID_FILTER)), module, Nebula::RES_A_B_PARAM));

    // ===== PHASE MOD =====
    addLabel(this, 63.5, Y_MID_PM + LBL_OFF, "PHASE MOD", 9.f);
    addLabel(this, 54, Y_MID_PM + LBL_SUB_OFF, "AMT", 7.5f);
    addLabel(this, 63.5, Y_MID_PM + LBL_SUB_OFF, "DIR", 7.5f);
    addLabel(this, 73, Y_MID_PM + LBL_SUB_OFF, "SCALE", 7.5f);

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(54, Y_MID_PM)), module, Nebula::PM_AMOUNT_PARAM));
    addParam(createParamCentered<CKSSThree>(mm2px(Vec(63.5, Y_MID_PM)), module, Nebula::PM_DIRECTION_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(73, Y_MID_PM)), module, Nebula::PM_SCALE_PARAM));

    // ===== CROSS PAN =====
    addLabel(this, 63.5, Y_MID_PAN + LBL_OFF, "CROSS PAN", 8.5f);
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(63.5, Y_MID_PAN)), module, Nebula::CROSS_SPILL_PARAM));

    // ===== MIX - Abkürzungen PH/CH! =====
    addLabel(this, 63.5, Y_MID_MIX - 14.f, "MIX", 11.f);

    // Bank A
    addLabel(this, 55.5, Y_MID_MIX - 7.f, "BANK A", 8.5f);
    addLabel(this, 51, Y_MID_MIX - 2.f, "PH", 8.f); // Abkürzung!
    addLabel(this, 60, Y_MID_MIX - 2.f, "CH", 8.f); // Abkürzung!

    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(51, Y_MID_MIX + 12.f)), module,
                                                         Nebula::PHASER_MIX_PARAM_A));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(60, Y_MID_MIX + 12.f)), module,
                                                         Nebula::CHORUS_MIX_PARAM_A));

    // Bank B
    addLabel(this, 71.5, Y_MID_MIX - 7.f, "BANK B", 8.5f);
    addLabel(this, 67, Y_MID_MIX - 2.f, "PH", 8.f); // Abkürzung!
    addLabel(this, 76, Y_MID_MIX - 2.f, "CH", 8.f); // Abkürzung!

    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(67, Y_MID_MIX + 12.f)), module,
                                                         Nebula::PHASER_MIX_PARAM_B));
    addParam(createParamCentered<LightSlider<VCVSlider>>(mm2px(Vec(76, Y_MID_MIX + 12.f)), module,
                                                         Nebula::CHORUS_MIX_PARAM_B));

    // ===== VOICES (Mitte, unter Mix) =====
    addLabel(this, 63.5, Y_MID_VOICES + 4, "VOICES", 8.5f);
    addLabel(this, 63.5, Y_MID_VOICES + 1, " 3 / 1", 7.f);
    addParam(createParamCentered<CKSS>(mm2px(Vec(63.5, Y_MID_VOICES + 8)), module, Nebula::VOICES_A_B_PARAM));

    // =====================================================================
    // ============== BANK B (Rechts, X-Zentrum: 105) ==============
    // =====================================================================

    // ----- PITCH B -----
    addLabel(this, 105, Y_PITCH + LBL_OFF, "PITCH");
    addLabel(this, 94, Y_PITCH + LBL_SUB_OFF, "FINE", 7.5f);
    addLabel(this, 116, Y_PITCH + LBL_SUB_OFF, "CV", 7.5f);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(94, Y_PITCH)), module, Nebula::FINE_B_PARAM));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(105, Y_PITCH)), module, Nebula::PITCH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, Y_PITCH)), module, Nebula::PITCH_B_CV_INPUT));

    // ----- MORPH B -----
    addLabel(this, 105, Y_MORPH + LBL_OFF, "MORPH");
    addLabel(this, 116, Y_MORPH + LBL_SUB_OFF, "CV", 7.5f);

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105, Y_MORPH)), module, Nebula::MORPH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, Y_MORPH)), module, Nebula::MORPH_B_CV_INPUT));

    // ----- VOLUME B -----
    addLabel(this, 105, Y_VOL + LBL_OFF, "VOLUME");
    addLabel(this, 116, Y_VOL + LBL_SUB_OFF, "CV", 7.5f);

    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105, Y_VOL)), module, Nebula::VOLUME_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(116, Y_VOL)), module, Nebula::VOLUME_B_CV_INPUT));

    // ----- SUB B -----
    addLabel(this, 105, Y_SUB + LBL_OFF, "SUB");
    addLabel(this, 105, Y_SUB + LBL_SUB_OFF, "LVL", 7.5f);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(105, Y_SUB)), module, Nebula::SUB_LEVEL_B_PARAM));

    // ----- PHASER B -----
    addLabel(this, 105, Y_PHASER + LBL_OFF, "PHASER");
    addLabel(this, 92, Y_PHASER + LBL_SUB_OFF, "CV", 7.5f);
    addLabel(this, 101, Y_PHASER + LBL_SUB_OFF, "RATE", 7.5f);
    addLabel(this, 110, Y_PHASER + LBL_SUB_OFF, "DPTH", 7.5f);
    addLabel(this, 119, Y_PHASER + LBL_SUB_OFF, "FB", 7.5f);

    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(92, Y_PHASER)), module, Nebula::PHASER_LFO_B_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(101, Y_PHASER)), module, Nebula::PHASER_RATE_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(110, Y_PHASER)), module, Nebula::PHASER_DEPTH_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(119, Y_PHASER)), module, Nebula::PHASER_FB_B_PARAM));

    // ----- CHORUS B -----
    addLabel(this, 105, Y_CHORUS + LBL_OFF, "CHORUS");
    addLabel(this, 101, Y_CHORUS + LBL_SUB_OFF, "RATE", 7.5f);
    addLabel(this, 110, Y_CHORUS + LBL_SUB_OFF, "DPTH", 7.5f);

    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(101, Y_CHORUS)), module, Nebula::CHORUS_RATE_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(110, Y_CHORUS)), module, Nebula::CHORUS_DEPTH_B_PARAM));

    // ----- OUTPUT B -----
    addLabel(this, 97, Y_BOTTOM - 3.f, "ADD", 6.f);
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(97, Y_BOTTOM)), module, Nebula::ADDITIVE_B_LIGHT));

    addLabel(this, 102, Y_BOTTOM - 6.f, "MODE", 6.f);
    addParam(createParamCentered<CKSS>(mm2px(Vec(102, Y_BOTTOM)), module, Nebula::PRESET_B_PARAM));

    addLabel(this, 107, Y_BOTTOM - 3.f, "WAV", 6.f);
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(107, Y_BOTTOM)), module, Nebula::WAV_B_LIGHT));

    addLabel(this, 116, Y_BOTTOM - 6.f, "OUT", 7.5f);
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(116, Y_BOTTOM)), module, Nebula::AUDIO_RIGHT_OUTPUT));
}

void NebulaWidget::appendContextMenu(Menu* menu)
{
    Nebula* module = dynamic_cast<Nebula*>(this->module);
    if (!module) return;

    menu->addChild(new MenuSeparator);
    menu->addChild(createMenuItem("Load WAV for Bank A", "", [=]()
    {
        char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, osdialog_filters_parse("WAV:wav"));
        if (path)
        {
            module->wavetableA.loadFromWav(path);
            module->wavetableA.currentMode = 1;
            module->params[Nebula::PRESET_A_PARAM].setValue(1.f);
            free(path);
        }
    }));
    menu->addChild(createMenuItem("Load WAV for Bank B", "", [=]()
    {
        char* path = osdialog_file(OSDIALOG_OPEN, NULL, NULL, osdialog_filters_parse("WAV:wav"));
        if (path)
        {
            module->wavetableB.loadFromWav(path);
            module->wavetableB.currentMode = 1;
            module->params[Nebula::PRESET_B_PARAM].setValue(1.f);
            free(path);
        }
    }));
}

Model* modelNebula = createModel<Nebula, NebulaWidget>("Nebula");
