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

#include <HAHLSimulation/ButterworthMultibandExpander.h>
#include "../Common/Constants.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

#define NUM_LEVELS 3
#define INITIAL_FREQ_HZ   125
#define OCTAVE_BAND_STEP  1
#define Q_BAND_PASS_FILTERS 1.4142
#define CUTOFF_FREQ_LPF_Hz  3000.0f
#define CUTOFF_FREQ_HPF_Hz  500.0f
#define HA_GAIN_RANGE_dB    65

#define dBs_SPL_for_0_dBs_fs 100.0f

//==============================================================================
HASPluginAudioProcessor::HASPluginAudioProcessor()
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
{
    addParameter (channelLink = new AudioParameterBool ("channel_link", "Channel Link", false));
    addParameter (channelEnabled[0] = new AudioParameterBool ("channel_enabled_l", "Ch Enabled L", true));
    addParameter (channelEnabled[1] = new AudioParameterBool ("channel_enabled_r", "Ch Enabled R", true));
    addParameter (channelGain[0] = new AudioParameterFloat ("channel_gain_left",
                                                            "Ch Gain Left",
                                                            NormalisableRange<float> (0.0f, Decibels::decibelsToGain (24.0f)),
                                                            1.0f,
                                                            String(),
                                                            AudioProcessorParameter::genericParameter,
                                                            [] (float value, int) { return String (value, 1); },
                                                            [] (const String& text) { return text.getFloatValue(); }));
    addParameter (channelGain[1] = new AudioParameterFloat ("channel_gain_right",
                                                            "Ch Gain Right",
                                                            NormalisableRange<float> (0.0f, Decibels::decibelsToGain (24.0f)),
                                                            1.0f,
                                                            String(),
                                                            AudioProcessorParameter::genericParameter,
                                                            [] (float value, int) { return String (value, 1); },
                                                            [] (const String& text) { return text.getFloatValue(); }));
    
    addParameter (enableQuantisationPre  = new AudioParameterBool ("enable_quantisation_pre_eq", "Quantisation Pre EQ", false));
    addParameter (enableQuantisationPost = new AudioParameterBool ("enable_quantisation_post_eq", "Quantisation Post EQ", false));
    addParameter (quantisationNumBits = new AudioParameterInt ("quantisation_num_bits", "Quantisation Num Bits", 6, 24, 16));
    
    addParameter (hearingLossLink = new AudioParameterBool ("hearning_loss_link", "Loss Link", false));
    hearingLoss[0] = new AudioParameterFloat* [Constants::NUM_BANDS];
    hearingLoss[1] = new AudioParameterFloat* [Constants::NUM_BANDS];
    
    for (int i = 0; i < Constants::NUM_BANDS; i++)
    {
        addParameter (hearingLoss[0][i] = new AudioParameterFloat ("hearing_loss_left_" + String(i),
                                                                   "Losses L " + String(i),
                                                                   NormalisableRange<float> (0.0f, 80.0f),
                                                                   0.0f,
                                                                   String(),
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) { return String (value, 1); },
                                                                   [] (const String& text) { return text.getFloatValue(); }));
        addParameter (hearingLoss[1][i] = new AudioParameterFloat ("hearing_loss_right_" + String(i),
                                                                   "Losses R " + String(i),
                                                                   NormalisableRange<float> (0.0f, 80.0f),
                                                                   0.0f,
                                                                   String(),
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) { return String (value, 1); },
                                                                   [] (const String& text) { return text.getFloatValue(); }));
    }
    
    
    addParameter (dynamicEQLink = new AudioParameterBool ("dynamic_eq_link", "Dynamic EQ Link", false));
    addParameter (dynamicEQLowPassEnabled  = new AudioParameterBool ("dynamic_eq_enable_lopass", "LPF Enabled", false));
    addParameter (dynamicEQHiPassEnabled  = new AudioParameterBool ("dynamic_eq_enable_hipass", "HPF Enabled", false));
    addParameter (dynamicEQLowPassCutoff = new AudioParameterInt ("dynamic_eq_lpf_cutoff", "LPF Cutoff", 63, 16000, CUTOFF_FREQ_LPF_Hz));
    addParameter (dynamicEQHiPassCutoff = new AudioParameterInt ("dynamic_eq_hpf_cutoff", "HPF Cutoff", 63, 16000, CUTOFF_FREQ_HPF_Hz));
    
    addParameter (enableFig6[0]  = new AudioParameterBool ("enable_fig6_l", "Fig 6 L", true));
    addParameter (enableFig6[1]  = new AudioParameterBool ("enable_fig6_r", "Fig 6 R", true));
    
    for (int level = 0; level < NUM_LEVELS; level++)
    {
        dynamicEQThresholds[0].add (new AudioParameterFloat ("dynamic_eq_threshold_left_" + String(level),
                                                             "Threshold Left " + String(level),
                                                             NormalisableRange<float> (-80.0f, 0.0f),
                                                             -20.0f * (level + 1),
                                                             String(),
                                                             AudioProcessorParameter::genericParameter,
                                                             [] (float value, int) { return String (value, 1); },
                                                             [] (const String& text) { return text.getFloatValue(); }));
        addParameter (dynamicEQThresholds[0].getLast());
        
        dynamicEQThresholds[1].add (new AudioParameterFloat ("dynamic_eq_threshold_right_" + String(level),
                                                             "Threshold Right " + String(level),
                                                             NormalisableRange<float> (-80.0f, 0.0f),
                                                             0.0f,
                                                             String(),
                                                             AudioProcessorParameter::genericParameter,
                                                             [] (float value, int) { return String (value, 1); },
                                                             [] (const String& text) { return text.getFloatValue(); }));
        addParameter (dynamicEQThresholds[1].getLast());
    }

    for (int band = 0; band < BANDS_NUMBER; band++)
    {
        dynamicEQBandGains[0].add (new juce::Array<AudioParameterFloat*>());
        dynamicEQBandGains[1].add (new juce::Array<AudioParameterFloat*>());
        
        for (int level = 0; level < NUM_LEVELS; level++)
        {
            dynamicEQBandGains[0].getLast()->add (new AudioParameterFloat ("band_gain_left_" + String(band) + "_" + String (level),
                                                                           "Gain L " + String(band) + "_" + String (level),
                                                                           NormalisableRange<float> (0.0f, 65.0f),
                                                                           0.0f,
                                                                           String(),
                                                                           AudioProcessorParameter::genericParameter,
                                                                           [] (float value, int) { return String (value, 1); },
                                                                           [] (const String& text) { return text.getFloatValue(); }));
            dynamicEQBandGains[1].getLast()->add (new AudioParameterFloat ("band_gain_right_" + String(band) + "_" + String (level),
                                                                           "Gain R " + String(band) + "_" + String (level),
                                                                           NormalisableRange<float> (0.0f, 65.0f),
                                                                           0.0f,
                                                                           String(),
                                                                           AudioProcessorParameter::genericParameter,
                                                                           [] (float value, int) { return String (value, 1); },
                                                                           [] (const String& text) { return text.getFloatValue(); }));
            addParameter (dynamicEQBandGains[0].getLast()->getLast());
            addParameter (dynamicEQBandGains[1].getLast()->getLast());
        }
    }
    
    addParameter (dynamicEQAttack[0] = new AudioParameterInt ("dynamic_eq_attack_left", "Attack Left", 0, 300, 100));
    addParameter (dynamicEQAttack[1] = new AudioParameterInt ("dynamic_eq_attack_right", "Attack Right", 0, 300, 100));
    addParameter (dynamicEQRelease[0] = new AudioParameterInt ("dynamic_eq_release_left", "Release Left", 0, 2000, 1000));
    addParameter (dynamicEQRelease[1] = new AudioParameterInt ("dynamic_eq_release_right", "Release Right", 0, 2000, 1000));
    addParameter (dynamicEQCompressionPct[0] = new AudioParameterInt ("dynamic_eq_compression_pct_left", "Comp Pct Left", 0, 120, 100));
    addParameter (dynamicEQCompressionPct[1] = new AudioParameterInt ("dynamic_eq_compression_pct_right", "Comp Pct Right", 0, 120, 100));
}

HASPluginAudioProcessor::~HASPluginAudioProcessor()
{
    delete [] hearingLoss[0];
    delete [] hearingLoss[1];
    dynamicEQThresholds[0].clear();
    dynamicEQThresholds[1].clear();
    dynamicEQBandGains[0].clear();
    dynamicEQBandGains[1].clear();
}

//==============================================================================
const String HASPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HASPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool HASPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool HASPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double HASPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int HASPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int HASPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void HASPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String HASPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void HASPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void HASPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // HL Simulator Setup
    // TODO: Abstract into separate class
    hlSimulator.Setup ((int)sampleRate, 100, 9, samplesPerBlock);
    
    for (int i = 0; i < 2; i++)
    {
        auto butterWorthExpander = make_shared<HAHLSimulation::CButterworthMultibandExpander>();
        butterWorthExpander->Setup ((int)getSampleRate(), HL_INITIAL_FREQ_HZ, HL_BANDS_NUMBER, true);
        butterWorthExpander->SetNumberOfFiltersPerBand (HL_FILTERS_PER_BAND);
        hlSimulator.SetMultibandExpander (ears[i], butterWorthExpander);
    }
    
    // HA Simulator Setup
    simulator.Setup ((int)sampleRate,
                     NUM_LEVELS,
                     INITIAL_FREQ_HZ,
                     BANDS_NUMBER,
                     OCTAVE_BAND_STEP,
                     CUTOFF_FREQ_LPF_Hz,
                     CUTOFF_FREQ_HPF_Hz,
                     Q_LOW_PASS_FILTER,
                     Q_BAND_PASS_FILTERS,
                     Q_HIGH_PASS_FILTER);
    
    bIn.left .assign (samplesPerBlock, 0.0f);
    bIn.right.assign (samplesPerBlock, 0.0f);
}

void HASPluginAudioProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HASPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void HASPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
    
    auto inChannels  = getTotalNumInputChannels();
    auto outChannels = getTotalNumOutputChannels();
    
    for (auto i = inChannels; i < outChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    
    // We currently assume working in stereo
    jassert (buffer.getNumChannels() >= 2);
    
    int numSamples = buffer.getNumSamples();

    if (enableQuantisationPre->get())
        simulator.EnableQuantizationBeforeEqualizer();
    else
        simulator.DisableQuantizationBeforeEqualizer();

    if (enableQuantisationPost->get())
        simulator.EnableQuantizationAfterEqualizer();
    else
        simulator.DisableQuantizationAfterEqualizer();
    
    simulator.SetQuantizationBits (quantisationNumBits->get());
    
    float lowPassCutoff = dynamicEQLowPassEnabled->get() ? dynamicEQLowPassCutoff->get() : 22050.0f;
    simulator.SetLowPassFilter (lowPassCutoff, Q_LOW_PASS_FILTER);
    
    float hiPassCutoff = dynamicEQHiPassCutoff->get() * dynamicEQHiPassEnabled->get();
    simulator.SetHighPassFilter (hiPassCutoff, Q_HIGH_PASS_FILTER);
    
    for (int i = 0; i < 2; i++)
    {
        int channel = channelLink->get() ? 0 : i;
        
        if (channelEnabled[channel]->get())
            simulator.EnableHearingAidSimulation (ears[i]);
        else
            simulator.DisableHearingAidSimulation (ears[i]);
        
        simulator.SetOverallGain (ears[i], channelGain[channel]->get());
        
        auto dynEQChannel = dynamicEQLink->get() ? 0 : i;
        
        if (enableFig6[dynEQChannel]->get())
        {
            std::vector<float> losses;
            
            int lossChannel = dynamicEQLink->get() || hearingLossLink->get() ? 0 : dynEQChannel;
            
            static constexpr int firstBandOfInterest = 1;
            
            for (int band = firstBandOfInterest; band < BANDS_NUMBER + firstBandOfInterest; band++)
                losses.push_back (hearingLoss[lossChannel][band]->get());
            
            simulator.SetDynamicEqualizerUsingFig6 (ears[i], losses, dBs_SPL_for_0_dBs_fs);
            
            auto* dynamicEQ = simulator.GetDynamicEqualizer (ears[i]);
            for (int y = 0; y < BANDS_NUMBER; y++)
                for (int i = 0; i < NUM_LEVELS; i++)
                    *(dynamicEQBandGains[dynEQChannel][y]->getUnchecked (i)) = dynamicEQ->GetLevelBandGain_dB (i, y);
            
            *enableFig6[dynEQChannel] = false;
        }
        
        auto* dynamicEQ = simulator.GetDynamicEqualizer (ears[i]);
        for (int level = 0; level < NUM_LEVELS; level++)
        {
            auto threshold = dynamicEQThresholds[dynEQChannel][level]->get();
            dynamicEQ->SetLevelThreshold (level, threshold);
            
            for (int band = 0; band < BANDS_NUMBER; band++)
                dynamicEQ->SetLevelBandGain_dB (level, band, dynamicEQBandGains[dynEQChannel][band]->getUnchecked(level)->get());
        }
        
        dynamicEQ->SetAttack_ms (dynamicEQAttack[dynEQChannel]->get());
        dynamicEQ->SetRelease_ms (dynamicEQRelease[dynEQChannel]->get());
        dynamicEQ->SetCompressionPercentage (dynamicEQCompressionPct[dynEQChannel]->get());
        
        simulator.DisableNormalization (ears[i]);
    }
    
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
bool HASPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* HASPluginAudioProcessor::createEditor()
{
    return new HASPluginAudioProcessorEditor (*this);
}

//==============================================================================
void HASPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
}

void HASPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
}

void HASPluginAudioProcessor::parameterChanged(const String& parameterID, float newValue)
{
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HASPluginAudioProcessor();
}
