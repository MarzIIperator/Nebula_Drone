#ifndef NEBULA_WAVETABLE_HPP
#define NEBULA_WAVETABLE_HPP

#include <array>

struct Wavetable
{
    static constexpr int TABLE_SIZE = 2048;
    static constexpr int NUM_FRAMES = 64;

    std::array<std::array<float, TABLE_SIZE + 1>, NUM_FRAMES> frames{};

    Wavetable();

    void clear();
    void generateBasic(int preset);
    void loadFromWav(const char* path);

    float getSample(float phase, float morph) const;

private:
    void normalizeAndWrap();
    void generateMipsFromFullBandwidth();
    int calculateMipLevel(float freq);

    // Die 4 Preset-Implementierungen
    void generate1();
    void generate2();
    void generate3();
    void generate4();
};

#endif