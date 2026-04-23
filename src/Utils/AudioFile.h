//=======================================================================
/** @file AudioFile.h
 *  @author Adam Stark
 *  @copyright Copyright (C) 2017  Adam Stark
 *
 * This file is part of the 'AudioFile' library
 *
 * MIT License
 *
 * Copyright (c) 2017 Adam Stark
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * DISCLAIMER:
 *
 *Für den Nebula Drone Synth wird nur eine vereinfachte Version benötigt, ausschließlich für den Upload von .wav Files.
 *
 */

//=======================================================================

#ifndef _AS_AudioFile_h
#define _AS_AudioFile_h

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <fstream>
#include <cstdint>

//=============================================================
template <class T>
class AudioFile
{
public:
    typedef std::vector<std::vector<T>> AudioBuffer;

    AudioBuffer samples;

    AudioFile() : sampleRate(44100), bitDepth(16) {
        samples.resize(1);
        samples[0].resize(0);
    }

    bool load(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);

        if (!file.good())
            return false;

        std::vector<uint8_t> fileData;
        file.unsetf(std::ios::skipws);
        file.seekg(0, std::ios::end);
        size_t length = file.tellg();
        file.seekg(0, std::ios::beg);
        fileData.resize(length);
        file.read(reinterpret_cast<char*>(fileData.data()), length);
        file.close();

        if (file.gcount() != length || fileData.size() < 12)
            return false;

        return decodeWaveFile(fileData);
    }

    uint32_t getSampleRate() const { return sampleRate; }
    int getNumChannels() const { return static_cast<int>(samples.size()); }
    int getNumSamplesPerChannel() const {
        return samples.size() > 0 ? static_cast<int>(samples[0].size()) : 0;
    }
    bool isMono() const { return getNumChannels() == 1; }

private:
    uint32_t sampleRate;
    int bitDepth;

    bool decodeWaveFile(const std::vector<uint8_t>& fileData) {
        // Check RIFF header
        std::string headerChunkID(fileData.begin(), fileData.begin() + 4);
        std::string format(fileData.begin() + 8, fileData.begin() + 12);

        if (headerChunkID != "RIFF" || format != "WAVE")
            return false;

        // Find fmt and data chunks
        int indexOfFormatChunk = getIndexOfChunk(fileData, "fmt ", 12);
        int indexOfDataChunk = getIndexOfChunk(fileData, "data", 12);

        if (indexOfDataChunk == -1 || indexOfFormatChunk == -1)
            return false;

        // Parse format chunk
        int f = indexOfFormatChunk;
        uint16_t numChannels = twoBytesToInt(fileData, f + 10);
        sampleRate = static_cast<uint32_t>(fourBytesToInt(fileData, f + 12));
        bitDepth = static_cast<int>(twoBytesToInt(fileData, f + 22));

        if (bitDepth != 8 && bitDepth != 16 && bitDepth != 24 && bitDepth != 32)
            return false;

        // Parse data chunk
        int d = indexOfDataChunk;
        int32_t dataChunkSize = fourBytesToInt(fileData, d + 4);
        int numSamples = dataChunkSize / (numChannels * bitDepth / 8);
        int samplesStartIndex = indexOfDataChunk + 8;

        uint16_t numBytesPerSample = static_cast<uint16_t>(bitDepth) / 8;
        uint16_t numBytesPerBlock = numChannels * numBytesPerSample;

        // Clear and resize
        samples.clear();
        samples.resize(numChannels);

        // Read samples
        for (int i = 0; i < numSamples; i++) {
            for (int channel = 0; channel < numChannels; channel++) {
                size_t sampleIndex = samplesStartIndex + (numBytesPerBlock * i) + channel * numBytesPerSample;

                if ((sampleIndex + (bitDepth / 8) - 1) >= fileData.size())
                    return false;

                T sample = 0;

                if (bitDepth == 16) {
                    int16_t sampleAsInt = twoBytesToInt(fileData, sampleIndex);
                    sample = static_cast<T>(sampleAsInt) / static_cast<T>(32767.0);
                }
                else if (bitDepth == 24) {
                    int32_t sampleAsInt = (fileData[sampleIndex + 2] << 16) |
                                         (fileData[sampleIndex + 1] << 8) |
                                          fileData[sampleIndex];

                    if (sampleAsInt & 0x800000)
                        sampleAsInt = sampleAsInt | ~0xFFFFFF;

                    sample = static_cast<T>(sampleAsInt) / static_cast<T>(8388607.0);
                }
                else if (bitDepth == 32) {
                    int32_t sampleAsInt = fourBytesToInt(fileData, sampleIndex);
                    float floatValue;
                    memcpy(&floatValue, &sampleAsInt, sizeof(int32_t));
                    sample = static_cast<T>(floatValue);
                }

                samples[channel].push_back(sample);
            }
        }

        return true;
    }

    static int32_t fourBytesToInt(const std::vector<uint8_t>& source, int startIndex) {
        return (source[startIndex + 3] << 24) | (source[startIndex + 2] << 16) |
               (source[startIndex + 1] << 8) | source[startIndex];
    }

    static int16_t twoBytesToInt(const std::vector<uint8_t>& source, int startIndex) {
        return (source[startIndex + 1] << 8) | source[startIndex];
    }

    static int getIndexOfChunk(const std::vector<uint8_t>& source, const std::string& chunkHeaderID, int startIndex) {
        int i = startIndex;
        while (i < source.size() - 4) {
            if (memcmp(&source[i], chunkHeaderID.data(), 4) == 0)
                return i;

            i += 4;
            if ((i + 4) >= source.size())
                return -1;

            int32_t chunkSize = fourBytesToInt(source, i);
            if (chunkSize > (source.size() - i - 4) || chunkSize < 0)
                return -1;

            i += (4 + chunkSize);
        }
        return -1;
    }
};

#endif