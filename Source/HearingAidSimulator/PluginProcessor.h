/**
 * \class HASPluginAudioProcessor
 *
 * \brief Declaration of HASPluginAudioProcessor interface.
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
 * \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#pragma once

#include <HAHLSimulation/3DTI_HAHLSimulator.h>
#include <JuceHeader.h>

static constexpr int BANDS_NUMBER = 7;
#define Q_HIGH_PASS_FILTER  0.707
#define Q_LOW_PASS_FILTER   0.707

static Common::T_ear ears[2] = {Common::T_ear::LEFT, Common::T_ear::RIGHT};

//==============================================================================
/**
 */
class HASPluginAudioProcessor  : public AudioProcessor, private AudioProcessorValueTreeState::Listener, private Timer
{
public:
    //==============================================================================
    HASPluginAudioProcessor();
    ~HASPluginAudioProcessor();
    
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
    
    AudioParameterBool* hearingLossLink;
    AudioParameterFloat** hearingLoss[2];
    
    AudioParameterBool*   channelLink;
    AudioParameterBool*   channelEnabled[2];
    AudioParameterFloat*  channelGain[2];
    AudioParameterBool*   enableQuantisationPre;
    AudioParameterBool*   enableQuantisationPost;
    AudioParameterInt*    quantisationNumBits;
    
    AudioParameterBool*   dynamicEQLink;
    AudioParameterBool*   dynamicEQLowPassEnabled;
    AudioParameterBool*   dynamicEQHiPassEnabled;
    AudioParameterInt*    dynamicEQLowPassCutoff;
    AudioParameterInt*    dynamicEQHiPassCutoff;
    AudioParameterBool*   enableFig6[2];
    juce::Array<AudioParameterFloat*> dynamicEQThresholds[2];
    OwnedArray<juce::Array<AudioParameterFloat*>> dynamicEQBandGains[2];
    AudioParameterInt*    dynamicEQAttack[2];
    AudioParameterInt*    dynamicEQRelease[2];
    AudioParameterInt*    dynamicEQCompressionPct[2];
    
    juce::Array<int> getBandFrequenciesAudiometry()
    {
        juce::Array<int> bands;
        for (int i = 0; i < hlSimulator.GetNumberOfBands(); i++)
            bands.add (hlSimulator.GetBandFrequency (i));
        return bands;
    }
    
    juce::Array<int> getBandFrequenciesDynamicEQ()
    {
        auto* dynamicEQ = getDynamicEqualizer (0);
        
        juce::Array<int> bands;
        for (int i = 0; i < dynamicEQ->GetNumBands(); i++)
            bands.add (dynamicEQ->GetBandFrequency (i));
        return bands;
    }
    
private:
    //==============================================================================
    HAHLSimulation::CDynamicEqualizer* getDynamicEqualizer (int channel)
    {
        return simulator.GetDynamicEqualizer (ears[channel]);
    }
    
    Common::CEarPair<CMonoBuffer<float>>  bIn;
    Common::CEarPair<CMonoBuffer<float>>  bOut;
    HAHLSimulation::CHearingAidSim  simulator;
    HAHLSimulation::CHearingLossSim hlSimulator; // Used for Audiometry only
    
    void timerCallback() override {}
    
    void parameterChanged(const String& parameterID, float newValue) override;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HASPluginAudioProcessor)
};
