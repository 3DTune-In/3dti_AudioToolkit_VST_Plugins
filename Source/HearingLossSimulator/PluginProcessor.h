/**
 * \class HLSPluginAudioProcessor
 *
 * \brief Declaration of HLSPluginAudioProcessor interface.
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

#include <JuceHeader.h>
#include <ff_buffers/ff_buffers_AudioBufferFIFO.h>
#include <HAHLSimulation/3DTI_HAHLSimulator.h>
#include "FrequencySmearingProcessor.h"

static const std::vector<int> jitterBandLimits = {
    200,
    400,
    800,
    1600,
    3200,
    6400,
    12800
};

namespace HAHLSimulation
{
    class CButterworthMultibandExpander;
    class CGammatoneMultibandExpander;
};

using namespace HAHLSimulation;

//==============================================================================
/**
 */
class HLSPluginAudioProcessor  : public AudioProcessor, private AudioProcessorValueTreeState::Listener, private Timer
{
public:
    //==============================================================================
    HLSPluginAudioProcessor();
    ~HLSPluginAudioProcessor();
    
    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif
    
    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;
    
    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    
    //==============================================================================
    const String getName() const override;
    
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
    
    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //==============================================================================
    juce::Array<int> getBandFrequencies()
    {
        juce::Array<int> bands;
        for (int i = 0; i < simulator.GetNumberOfBands(); i++)
            bands.add (simulator.GetBandFrequency (i));
        return bands;
    }
    
    //==============================================================================
    CHearingLossSim simulator;
    
    //==============================================================================
    enum MultibandExpanderType
    {
        Butterworth,
        Gammatone
    };
    
    AudioParameterBool* hearingLossLink;
    AudioParameterFloat** hearingLoss[2];
    
    // AudioParameterBool*   channelsLink;
    AudioParameterBool*   enableSimulation[2];
    // Non-linear Attenuation settings
    AudioParameterBool*   nonLinearAttenuationLink;
    AudioParameterBool*   nonLinearAttenuationEnabled[2];
    AudioParameterInt*    nonLinearAttenuationFilterType[2];
    // Temporal Distortion settings
    AudioParameterBool*   temporalDistortionLink;
    AudioParameterBool*   temporalDistortionEnabled[2];
    AudioParameterInt*    jitterBandLimit[2];
    AudioParameterFloat*  jitterNoisePower[2];
    AudioParameterInt*    jitterNoiseAutocorrelationCutoff[2];
    AudioParameterFloat*  jitterLeftRightSynchronicity;
    // Frequency Smear settings
    FrequencySmearingProcessor frequencySmearProcessor;
private:
    void processBlockInternal (AudioBuffer<float>&);
    
    //==============================================================================
    Common::CEarPair<CMonoBuffer<float>>  bIn;
    Common::CEarPair<CMonoBuffer<float>>  bOut;
    
    AudioBuffer<float>     scratchBuffer;
    AudioBufferFIFO<float> inFifo  {2, 512},
                           outFifo {2, 512};
    
    juce::Array<shared_ptr<CButterworthMultibandExpander>> butterWorthExpanders;
    juce::Array<shared_ptr<CGammatoneMultibandExpander>>   gammatoneExpanders;
    
    //==============================================================================
    void setMultiBandExpander (int channel, MultibandExpanderType type);
    
    //==============================================================================
    void timerCallback() override;
    
    void updateFrequencySmearingParametersIfNeeded();
    
    void parameterChanged(const String& parameterID, float newValue) override;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HLSPluginAudioProcessor)
};
