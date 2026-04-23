#ifndef NEBULA_WAVETABLE_HPP
#define NEBULA_WAVETABLE_HPP

#include <array>

struct Wavetable
{
    static constexpr int TABLE_SIZE = 2048;
    static constexpr int NUM_FRAMES = 256;

    std::array<std::array<float, TABLE_SIZE + 1>, NUM_FRAMES> frames{};     // Additive
    std::array<std::array<float, TABLE_SIZE + 1>, NUM_FRAMES> framesWav{};  // WAV

    int currentMode = 0;   // 0 = Additive, 1 = WAV

    Wavetable();

    void clear();
    void loadFromWav(const char* path);

    void generateBasic(int preset);

    float getSample(float phase, float morph) const;

private:
    void normalizeAndWrap();
    void generateMipsFromFullBandwidth();
    int calculateMipLevel(float freq);

    void generate1();
    void generate2();
};

#endif