
#pragma once

#include <vector>
#include "PluginProcessor.h"

// =============================================================================
static std::vector<std::vector<float>> hearingLossPresetValues = {
    std::vector<float> (Constants::NUM_BANDS, 0),
    {10, 10, 10, 20, 30, 30, 40, 40, 40},
    {20, 20, 20, 30, 40, 40, 50, 50, 50},
    {40, 40, 40, 50, 60, 70, 80, 80, 80}
};

// =============================================================================
struct Presets
{
    int audiometryPreset[2];
    int temporalDistortionPresets[2];
    int frequencySmearPresets[2][2] = {0};
};

inline Presets& selectedPresets()
{
    static Presets p;
    return p;
}
