#include "Nebula.h"
#include <osdialog.h>

NebulaWidget::NebulaWidget(Nebula* module) {
    setModule(module);
    setPanel(createPanel(asset::plugin(pluginInstance, "res/Nebula.svg")));

    // Bank A
    //Pitch
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(22.f, 34.f)), module, Nebula::PITCH_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(13.f, 34.f)), module, Nebula::PITCH_A_CV_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(35.f, 36.f)), module, Nebula::FINE_A_PARAM));
    //Morphing
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.f, 55.f)), module, Nebula::MORPH_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(22.f, 55.f)), module, Nebula::MORPH_A_CV_INPUT));
    //Volume
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(22.f, 71.f)), module, Nebula::VOLUME_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(105.f, 71.f)), module, Nebula::VOLUME_A_CV_INPUT));
    //SubOSC
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.f, 84.5f)), module, Nebula::SUB_OCTAVE_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(35.f, 86.f)), module, Nebula::SUB_LEVEL_A_PARAM));
    // .wav file load
    addParam(createParamCentered<CKSS>(mm2px(Vec(22.f, 114.5f)), module, Nebula::PRESET_A_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(15.f, 114.f)), module, Nebula::ADDITIVE_A_LIGHT));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(29.f, 114.f)), module, Nebula::WAV_A_LIGHT));
    //Audio out
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(22.f, 120.5f)), module, Nebula::AUDIO_LEFT_OUTPUT));


    // Bank B
    //Pitch
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(92.f, 36.f)), module, Nebula::FINE_B_PARAM));
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(105.f, 34.f)), module, Nebula::PITCH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(96.f, 34.f)), module, Nebula::PITCH_B_CV_INPUT));
    //Morphing
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105.f, 55.f)), module, Nebula::MORPH_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(105.f, 55.f)), module, Nebula::MORPH_B_CV_INPUT));
    //Volume
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(105.f, 71.f)), module, Nebula::VOLUME_B_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(105.f, 71.f)), module, Nebula::VOLUME_B_CV_INPUT));
    //SubOSC
    addParam(createParamCentered<CKSS>(mm2px(Vec(105.f, 84.5f)), module, Nebula::SUB_OCTAVE_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(118.f, 86.f)), module, Nebula::SUB_LEVEL_B_PARAM));
    // .wav file load
    addParam(createParamCentered<CKSS>(mm2px(Vec(105.f, 114.5f)), module, Nebula::PRESET_B_PARAM));
    addChild(createLightCentered<SmallLight<GreenLight>>(mm2px(Vec(98.f, 114.f)), module, Nebula::ADDITIVE_B_LIGHT));
    addChild(createLightCentered<SmallLight<RedLight>>(mm2px(Vec(112.f, 114.f)), module, Nebula::WAV_B_LIGHT));
    //audio out
    addOutput(createOutputCentered<PJ301MPort>(mm2px(Vec(105.f, 120.5f)), module, Nebula::AUDIO_RIGHT_OUTPUT));


    //Phaser Bank A und B
    // Phaser A
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(13.f, 103.5f)), module, Nebula::PHASER_MOD_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(22.f, 103.5f)), module, Nebula::PHASER_STAGES_A_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(31.f, 103.5f)), module, Nebula::PHASER_SKEW_A_PARAM));
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(41.f, 103.5f)), module, Nebula::PHASER_CV_A_INPUT));

    // Phaser B
    addInput(createInputCentered<PJ301MPort>(mm2px(Vec(96.f, 103.75f)), module, Nebula::PHASER_CV_B_INPUT));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(105.f, 103.75f)), module, Nebula::PHASER_SKEW_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(114.f, 103.75f)), module, Nebula::PHASER_STAGES_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(123.f, 103.75f)), module, Nebula::PHASER_MOD_B_PARAM));


    // Global
    //Filter
    addParam(createParamCentered<RoundLargeBlackKnob>(mm2px(Vec(63.5f, 38.f)), module, Nebula::CUTOFF_A_B_PARAM));
    addParam(createParamCentered<RoundSmallBlackKnob>(mm2px(Vec(55.f, 54.f)), module, Nebula::RES_A_B_PARAM));

    //PM
    addParam(createParamCentered<RoundBlackKnob>(mm2px(Vec(55.f, 72.5f)), module, Nebula::PM_AMOUNT_PARAM));
    addParam(createParamCentered<CKSS>(mm2px(Vec(55.f, 86.f)), module, Nebula::PM_SCALE_PARAM));
    addParam(createParamCentered<CKSSThree>(mm2px(Vec(72.f, 72.5f)), module, Nebula::PM_DIRECTION_PARAM));


    // Dry - Wet Regler für Effekte
    //Bank A
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::PHASER_MIX_PARAM_A));
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::PM_MIX_PARAM_A));
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::FILTER_MIX_PARAM_A));
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::CHORUS_MIX_PARAM_A));

    //Bank B
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::PHASER_MIX_PARAM_B));
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::PM_MIX_PARAM_B));
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::CHORUS_MIX_PARAM_B));
    addParam(createParamCentered<VCVSlider>(mm2px(Vec(75.f, 100.f)), module, Nebula::FILTER_MIX_PARAM_B));


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
