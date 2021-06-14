/**
 * \class FrequencySmearingProcessor
 *
 * \brief Declaration of FrequencySmearingProcessor interface.
 * \date  November 2020
 *
 * \authors Reactify Music LLP: R. Hrafnkelsson ||
 * Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
 * \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
 *
 * \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
 * \b Website: http://3d-tune-in.eu/
 *
 * \b Copyright: University of Malaga and Imperial College London - 2020
 *
 * \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
 *
 * \b Acknowledgement: This project HLS received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#pragma once

#include <HAHLSimulation/HearingLossSim.h>
#include <JuceHeader.h>

using namespace HAHLSimulation;

//==============================================================================
class FrequencySmearingProcessor
{
public:
    FrequencySmearingProcessor (CHearingLossSim& sim);
    
    ~FrequencySmearingProcessor() {}

    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock)
    {
        for (auto const& channel : channels)
            channel->prepareToPlay (sampleRate, samplesPerBlock);
    }

    //==============================================================================
    enum FrequencySmearType
    {
        BaerMoore,
        Graf3dti
    };
    
    //==============================================================================
    AudioParameterBool*   freqSmearLink;
    AudioParameterBool*   freqSmearEnabled[2];
    AudioParameterBool*   freqSmearType[2];
    AudioParameterFloat*  freqSmearSpectralBroadFactorUp[2];
    AudioParameterFloat*  freqSmearSpectralBroadFactorDown[2];
    AudioParameterFloat*  freqSmearSpectralFrequencyDown[2];
    AudioParameterFloat*  freqSmearSpectralFrequencyUp[2];
    AudioParameterFloat*  freqSmearSpectralBufferSizeDown[2];
    AudioParameterFloat*  freqSmearSpectralBufferSizeUp[2];
    
    //==============================================================================
    struct Channel
    {
        Channel();
        
        void prepareToPlay (double sampleRate, int samplesPerBlock);
        
        std::shared_ptr<CBaerMooreFrequencySmearing> baerMooreSmearing;
        std::shared_ptr<CGraf3DTIFrequencySmearing>  graf3dtiSmearing;
    };
    
    const OwnedArray<Channel>& getChannels() { return channels; }
    
    //==============================================================================
    void updateSettingsIfNeeded();
    
private:
    
    CHearingLossSim& simulator;
    
    OwnedArray<Channel> channels;
    
    float previousSpectralBroadFactorUp[2]   = {1.01f, 1.01f};
    float previousSpectralBroadFactorDown[2] = {1.01f, 1.01f};
    
    float previousSpectralFrequencyDown[2];
    float previousSpectralFrequencyUp[2];
    float previousSpectralBufferSizeDown[2];
    float previousSpectralBufferSizeUp[2];
};
