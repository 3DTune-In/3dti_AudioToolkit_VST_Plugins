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

#include <HAHLSimulation/ButterworthMultibandExpander.h>
#include <HAHLSimulation/GammatoneMultibandExpander.h>
#include "../Common/Constants.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

#define NUM_BANDS Constants::NUM_BANDS

static Common::T_ear ears[2] = {Common::T_ear::LEFT, Common::T_ear::RIGHT};


//==============================================================================
HLSPluginAudioProcessor::HLSPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
                  .withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
                  .withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
                  )
#endif
    , frequencySmearProcessor (simulator)
{
    addParameter (hearingLossLink = new AudioParameterBool ("hearning_loss_link", "Hearing Loss Link", false));
    hearingLoss[0] = new AudioParameterFloat* [NUM_BANDS];
    hearingLoss[1] = new AudioParameterFloat* [NUM_BANDS];
    
    // addParameter (channelsLink = new AudioParameterBool ("channels_link", "Channel Link", false));
    
    addParameter (enableSimulation[0] = new AudioParameterBool ("enable_simulation_left", "Enable Left", false));
    addParameter (enableSimulation[1] = new AudioParameterBool ("enable_simulation_right", "Enable Right", false));
    
    for (int i = 0; i < NUM_BANDS; i++)
    {
        addParameter (hearingLoss[0][i] = new AudioParameterFloat ("band_gain_left_" + String(i),
                                                                   "Band Gain L " + String(i),
                                                                   NormalisableRange<float> (0.0f, 80.0f),
                                                                   0.0f,
                                                                   String(),
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) { return String (value, 1); },
                                                                   [] (const String& text) { return text.getFloatValue(); }));
        addParameter (hearingLoss[1][i] = new AudioParameterFloat ("band_gain_right_" + String(i),
                                                                   "Band Gain R " + String(i),
                                                                   NormalisableRange<float> (0.0f, 80.0f),
                                                                   0.0f,
                                                                   String(),
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) { return String (value, 1); },
                                                                   [] (const String& text) { return text.getFloatValue(); }));
    }
    
    // Non-linear Attenuation Settings
    addParameter (nonLinearAttenuationEnabled[0] = new AudioParameterBool ("non_linear_attenuation_enabled_left", "Non Linear Att L", true));
    addParameter (nonLinearAttenuationEnabled[1] = new AudioParameterBool ("non_linear_attenuation_enabled_right", "Non Linear Att R", true));
    addParameter (nonLinearAttenuationLink = new AudioParameterBool ("non_linear_attenuation_link", "Non Linear Att Link", false));
    addParameter (nonLinearAttenuationFilterType[0] = new AudioParameterInt ("filterbank_type_left",
                                                             "Filter Type L",
                                                             0, 1, 0,
                                                             String(),
                                                             [] (int value, int maxLength) { return (value == 0) ? "Butterworth" : "Gammatone"; },
                                                             [] (const String& text) { return (text.toLowerCase() == "butterworth") ? 0 : 1; }));
    
    addParameter (nonLinearAttenuationFilterType[1] = new AudioParameterInt ("filterbank_type_right",
                                                             "Filter Type R",
                                                             0, 1, 0,
                                                             String(),
                                                             [] (int value, int maxLength) { return (value == 0) ? "Butterworth" : "Gammatone"; },
                                                             [] (const String& text) { return (text.toLowerCase() == "butterworth") ? 0 : 1; }));
    
    // Temporal Distortion Settings
    addParameter (temporalDistortionEnabled[0] = new AudioParameterBool ("temporal_distortion_enabled_left", "Temporal Dist L", false));
    addParameter (temporalDistortionEnabled[1] = new AudioParameterBool ("temporal_distortion_enabled_right", "Temporal Dist R", false));
    addParameter (temporalDistortionLink = new AudioParameterBool ("temporal_distortion_link", "Temp Dist Link", false));
    addParameter (jitterBandLimit[0] = new AudioParameterInt ("jitter_band_limit_left",
                                                              "Jitter Band Limit Left",
                                                              0, 6, 0));
    addParameter (jitterBandLimit[1] = new AudioParameterInt ("jitter_band_limit_right",
                                                              "Jitter Band Limit Right",
                                                              0, 6, 0));
    addParameter (jitterNoisePower[0] = new AudioParameterFloat ("jitter_noise_power_left",
                                                                 "Jitter Noise L",
                                                                 NormalisableRange<float> (0.0f, 1.0f),
                                                                 0.0f,
                                                                 String(),
                                                                 AudioProcessorParameter::genericParameter,
                                                                 [] (float value, int) { return String (value, 1); },
                                                                 [] (const String& text) { return text.getFloatValue(); }));
    addParameter (jitterNoisePower[1] = new AudioParameterFloat ("jitter_noise_power_right",
                                                                 "Jitter Noise R",
                                                                 NormalisableRange<float> (0.0f, 1.0f),
                                                                 0.0f,
                                                                 String(),
                                                                 AudioProcessorParameter::genericParameter,
                                                                 [] (float value, int) { return String (value, 1); },
                                                                 [] (const String& text) { return text.getFloatValue(); }));
    addParameter (jitterNoiseAutocorrelationCutoff[0] = new AudioParameterInt ("jitter_autocorrelation_cutoff_left",
                                                                               "Jitter Autocorrelation Cutoff Left",
                                                                               0, 1000, 0));
    addParameter (jitterNoiseAutocorrelationCutoff[1] = new AudioParameterInt ("jitter_autocorrelation_cutoff_right",
                                                                               "Jitter Autocorrelation Cutoff Right",
                                                                               0, 1000, 0));
    addParameter (jitterLeftRightSynchronicity = new AudioParameterFloat ("jitter_left_right_synchronicity",
                                                                          "Jitter L/R Sync",
                                                                          NormalisableRange<float> (0.0f, 1.0f),
                                                                          0.0f,
                                                                          String(),
                                                                          AudioProcessorParameter::genericParameter,
                                                                          [] (float value, int) { return String (value, 1); },
                                                                          [] (const String& text) { return text.getFloatValue(); }));

    addParameter (frequencySmearProcessor.freqSmearLink);
    for (int i = 0; i < 2; i++)
    {
        addParameter (frequencySmearProcessor.freqSmearEnabled[i]);
        addParameter (frequencySmearProcessor.freqSmearType[i]);
        addParameter (frequencySmearProcessor.freqSmearSpectralBroadFactorUp[i]);
        addParameter (frequencySmearProcessor.freqSmearSpectralBroadFactorDown[i]);
        addParameter (frequencySmearProcessor.freqSmearSpectralFrequencyDown[i]);
        addParameter (frequencySmearProcessor.freqSmearSpectralFrequencyUp[i]);
        addParameter (frequencySmearProcessor.freqSmearSpectralBufferSizeDown[i]);
        addParameter (frequencySmearProcessor.freqSmearSpectralBufferSizeUp[i]);
    }
    
    startTimerHz (30);
}

HLSPluginAudioProcessor::~HLSPluginAudioProcessor()
{
    delete [] hearingLoss[0];
    delete [] hearingLoss[1];
}

//==============================================================================
const String HLSPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HLSPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool HLSPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool HLSPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double HLSPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HLSPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int HLSPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HLSPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String HLSPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void HLSPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void HLSPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    simulator.Setup ((int)sampleRate, 100, NUM_BANDS, samplesPerBlock);

    // Set up multiband expanders
    for (int i = 0; i < getTotalNumOutputChannels(); i++)
        setMultiBandExpander (i, MultibandExpanderType::Butterworth);
    
    // TODO: Move into dedicated class
    butterWorthExpanders.clear();
    gammatoneExpanders.clear();
    
    for (int i = 0; i < getTotalNumOutputChannels(); i++)
    {
        auto butterWorthExpander = shared_ptr<CButterworthMultibandExpander> (new CButterworthMultibandExpander);
        butterWorthExpander->Setup ((int)sampleRate, HL_INITIAL_FREQ_HZ, NUM_BANDS, true);
        butterWorthExpander->SetNumberOfFiltersPerBand (HL_FILTERS_PER_BAND);
        butterWorthExpanders.add (butterWorthExpander);
        
        auto gammatoneExpander = shared_ptr<CGammatoneMultibandExpander> (new CGammatoneMultibandExpander);
        gammatoneExpander->Setup ((int)sampleRate, HL_INITIAL_FREQ_HZ, NUM_BANDS, true);
        vector<float> bandLimits = { 88.3883476483184f,    176.776695296637f,    353.553390593274f,    707.106781186548f,    1414.21356237310f,    2828.42712474619f,    5656.85424949238f,    11313.7084989848f };
        gammatoneExpander->SetGroups(bandLimits);
        gammatoneExpanders.add (gammatoneExpander);
    }
    
    frequencySmearProcessor.prepareToPlay (sampleRate, samplesPerBlock);

    // Set up buffers
    bIn.left .assign (samplesPerBlock, 0.0f);
    bIn.right.assign (samplesPerBlock, 0.0f);
    
    bOut.left .assign (samplesPerBlock, 0.0f);
    bOut.right.assign (samplesPerBlock, 0.0f);
}

void HLSPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HLSPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;
    
    // This checks if the input layout matches the output layout
#if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif
    
    return true;
#endif
}
#endif

void HLSPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    
    auto inChannels  = getTotalNumInputChannels();
    auto outChannels = getTotalNumOutputChannels();
    
    for (auto i = inChannels; i < outChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    
    // We currently assume working in stereo
    jassert (buffer.getNumChannels() == 2);
    
    int numSamples = buffer.getNumSamples();
    
    /*
     /// Update parameters
     */
    for (int ch = 0; ch < getTotalNumOutputChannels(); ch++)
    {
        if (enableSimulation[ch]->get())
            simulator.EnableHearingLossSimulation (ears[ch]);
        else
            simulator.DisableHearingLossSimulation (ears[ch]);
        
        // Non linear attenuation
        int channel = (! nonLinearAttenuationLink->get()) * ch;
        if (nonLinearAttenuationEnabled[ch]->get())
            simulator.EnableMultibandExpander (ears[ch]);
        else
            simulator.DisableMultibandExpander (ears[ch]);
        
        if (nonLinearAttenuationFilterType[channel]->get() == MultibandExpanderType::Butterworth)
            simulator.SetMultibandExpander (ears[ch], butterWorthExpanders[ch]);
        else
            simulator.SetMultibandExpander (ears[ch], gammatoneExpanders[ch]);
        
        // Audiometry settings
        channel = (! hearingLossLink->get()) * ch;
        for (int i = 0; i < NUM_BANDS; i++)
            simulator.SetHearingLevel_dBHL (ears[ch],  i, hearingLoss[channel][i]->get());
    }
    
    // Temporal Distortion Settings
    auto* temporalDistortionSimulator = simulator.GetTemporalDistortionSimulator();
    
    for (int i = 0; i < getTotalNumOutputChannels(); i++)
    {
        int channel = (! temporalDistortionLink->get()) * i;
        
        if (temporalDistortionEnabled[channel]->get())
            temporalDistortionSimulator->EnableTemporalDistortionSimulator (ears[i]);
        else
            temporalDistortionSimulator->DisableTemporalDistortionSimulator (ears[i]);
        
        temporalDistortionSimulator->SetBandUpperLimit (ears[i], jitterBandLimits[jitterBandLimit[channel]->get()]);
        temporalDistortionSimulator->SetWhiteNoisePower (ears[i], jitterNoisePower[channel]->get());
        temporalDistortionSimulator->SetNoiseAutocorrelationFilterCutoffFrequency (ears[i], jitterNoiseAutocorrelationCutoff[channel]->get());
    }
    temporalDistortionSimulator->SetLeftRightNoiseSynchronicity (jitterLeftRightSynchronicity->get());

    // frequencySmearProcessor.processBlock (buffer);
    
    // Fill input buffer and clear output
    for (int i = 0; i < numSamples; i++)
    {
        bIn.left[i]  = buffer.getReadPointer (0)[i];
        bIn.right[i] = buffer.getReadPointer (1)[i];
    }
    
    simulator.Process (bIn, bOut);

    for (int i = 0; i < numSamples; i++)
    {
        buffer.getWritePointer (0)[i] = bOut.left[i];
        buffer.getWritePointer (1)[i] = bOut.right[i];
    }
}

//==============================================================================
bool HLSPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* HLSPluginAudioProcessor::createEditor()
{
    return new HLSPluginAudioProcessorEditor (*this);
}

//==============================================================================
void HLSPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
}

void HLSPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

void HLSPluginAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
}

void HLSPluginAudioProcessor::setMultiBandExpander (int channel, MultibandExpanderType type)
{
    std::shared_ptr<HAHLSimulation::CMultibandExpander> expander;
    if (type == MultibandExpanderType::Butterworth)
    {
        auto butterWorthExpander = make_shared<HAHLSimulation::CButterworthMultibandExpander>();
        butterWorthExpander->Setup ((int)getSampleRate(), HL_INITIAL_FREQ_HZ, NUM_BANDS, true);
        butterWorthExpander->SetNumberOfFiltersPerBand (HL_FILTERS_PER_BAND);
        expander = butterWorthExpander;
    }
    else
    {
        auto gammatoneExpander = make_shared<HAHLSimulation::CGammatoneMultibandExpander>();
        gammatoneExpander->Setup ((int)getSampleRate(), HL_INITIAL_FREQ_HZ, NUM_BANDS, true);
        vector<float> bandLimits = { 88.3883476483184f,    176.776695296637f,    353.553390593274f,    707.106781186548f,    1414.21356237310f,    2828.42712474619f,    5656.85424949238f,    11313.7084989848f };
        gammatoneExpander->SetGroups(bandLimits);
        expander = gammatoneExpander;
    }
    
    // NOTE(Ragnar): Processing a block of silence just after instantiation
    // seems to alleviate some noise & clicks when switching expander types
    CMonoBuffer<float> silence (getBlockSize(), 0.0f);
    expander->Process (silence, silence);
    
    const ScopedLock lock (getCallbackLock());
    simulator.SetMultibandExpander (ears[channel], expander);
    *nonLinearAttenuationFilterType[channel] = type;
}

void HLSPluginAudioProcessor::timerCallback()
{
    // updateFrequencySmearingParametersIfNeeded();
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HLSPluginAudioProcessor();
}
