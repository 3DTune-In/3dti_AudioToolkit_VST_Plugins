/**
* \class ReverbPluginProcessor
*
* \brief Declaration of ReverbPluginProcessor interface.
* \date  November 2021
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

#include "ReverbPluginProcessor.h"
#include "ReverbPluginEditor.h"

static constexpr int kTOOLKIT_BUFFER_SIZE = 512; // TODO(Ragnar): Make variable

void addBooleanHostParameter(AudioProcessorValueTreeState& treeState, String name, int value) {
  const auto bypassValueToText = [](float value) {
    return value < 0.5f ? "Off" : "On";
  };
  
  const auto bypassTextToValue = [](const String& text) {
    if (text == "On") { return 1.0f; }
    return 0.0f;
  };
  
  using Parameter = AudioProcessorValueTreeState::Parameter;
  treeState.createAndAddParameter (std::make_unique<Parameter> (name, name, "", NormalisableRange<float>(0.f, 1.f, 1.f),
                                                                value, bypassValueToText, bypassTextToValue, false, true,
                                                                true, AudioProcessorParameter::genericParameter, true));
}

//==============================================================================
ReverbPluginProcessor::ReverbPluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                       .withInput  ("Reverb Inputs", AudioChannelSet::quadraphonic(), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                       )
#endif
      , treeState (*this, nullptr)
      , inFifo (2, 512)
      , outFifo (2, 512)
{
    using Parameter = AudioProcessorValueTreeState::Parameter;
    
    addBooleanHostParameter (treeState, "Reverb Enabled", getReverbProcessor().reverbEnabled);
    treeState.addParameterListener ("Reverb Enabled", this);
    
    treeState.createAndAddParameter (std::make_unique<Parameter> ("Reverb Level", "Reverb Level", "", getReverbProcessor().reverbLevel.range, getReverbProcessor().reverbLevel.get(), nullptr, nullptr));
    treeState.addParameterListener ("Reverb Level", this);
  
    treeState.createAndAddParameter (std::make_unique<Parameter> ("BRIR", "BRIR", "", NormalisableRange<float>(0, BundledBRIRs.size()-1), 0, [](float value) { return String (value, 0); }, nullptr));
    treeState.addParameterListener ("BRIR", this);
    
    treeState.state = ValueTree("3DTI Reverb Parameters");
    
#if DEBUG
    ERRORHANDLER3DTI.SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);
    ERRORHANDLER3DTI.SetErrorLogStream(&std::cout, true);
#endif
}

ReverbPluginProcessor::~ReverbPluginProcessor()
{
}

//==============================================================================
const String ReverbPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ReverbPluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ReverbPluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ReverbPluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ReverbPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ReverbPluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ReverbPluginProcessor::getCurrentProgram()
{
    return 0;
}

void ReverbPluginProcessor::setCurrentProgram (int index)
{
}

const String ReverbPluginProcessor::getProgramName (int index)
{
    return {};
}

void ReverbPluginProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void ReverbPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  const int blockSizeInternal = kTOOLKIT_BUFFER_SIZE;
    
  auto numSideChainInputs = getChannelCountOfBus (true, 1);
        
  inFifo.setSize (numSideChainInputs, std::max (blockSizeInternal, samplesPerBlock) + 1);
  inFifo.clear();
  
  outFifo.setSize (2, std::max (samplesPerBlock, blockSizeInternal) * 2);
  outFifo.clear();
   
  scratchBufferStereo.setSize (2, blockSizeInternal);
  scratchBufferStereo.clear();

  scratchBufferQuad.setSize (numSideChainInputs, blockSizeInternal);
  scratchBufferQuad.clear();
    
  mCore.SetAudioState ({(int)sampleRate, blockSizeInternal});
    
  mReverb.setup (sampleRate, blockSizeInternal);
  
  // startTimer(30);
}

void ReverbPluginProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ReverbPluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    return true;
}
#endif
 
void ReverbPluginProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
  
    auto inChannels  = getTotalNumInputChannels();
    auto outChannels = getTotalNumOutputChannels();
    
    for (auto i = inChannels; i < outChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    auto mainInput    = getBusBuffer (buffer, true, 0); // Stereo I/O
    auto reverbInputs = getBusBuffer (buffer, true, 1); // Quad side chain input
    
    auto numSamples = mainInput.getNumSamples();
    
    const int blockSizeInternal = kTOOLKIT_BUFFER_SIZE;
    // Some hosts send buffers of varying sizes so we maintain
    // an internal buffer to pass the correct size to the 3dti core
    
    inFifo.addToFifo (reverbInputs);

    while (inFifo.getNumReady() >= blockSizeInternal)
    {
        scratchBufferStereo.clear();
        
        inFifo.readFromFifo (scratchBufferQuad, blockSizeInternal);
        mReverb.process (scratchBufferQuad, scratchBufferStereo);
        outFifo.addToFifo (scratchBufferStereo);
    }

    int numReady = outFifo.getNumReady();
    if (numReady < numSamples)
    {
        int diff = numSamples - numReady;
        outFifo.addSilenceToFifo (diff);

        // Update the host latency
        int latency = getLatencySamples() + diff;
        setLatencySamples (latency);
    }
    
    AudioSampleBuffer reverb (mainInput.getNumChannels(), numSamples);
    outFifo.readFromFifo (reverb);
    
    for (int ch = 0; ch < mainInput.getNumChannels(); ch++)
        mainInput.addFrom (ch, 0, reverb, ch, 0, numSamples);
}

//==============================================================================
bool ReverbPluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* ReverbPluginProcessor::createEditor()
{
    return new ReverbPluginProcessorEditor (*this);
}

//==============================================================================
void ReverbPluginProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void ReverbPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void ReverbPluginProcessor::updateHostParameters()
{
    std::unordered_map<String, float> parameters = {
      {"Reverb Enabled", getReverbProcessor().reverbEnabled},
      {"Reverb Level", getReverbProcessor().reverbLevel},
      {"BRIR", getReverbProcessor().reverbBRIR.get() },
    };

    for ( auto const & parameter : parameters )
    {
        if ( AudioProcessorParameter* p = treeState.getParameter(parameter.first) )
        {
            const float newValue = treeState.getParameterRange(parameter.first).convertTo0to1(parameter.second);
      
            if ( fabs(p->getValue() - newValue) > std::numeric_limits<float>::epsilon() )
                p->setValueNotifyingHost(newValue);
        }
    }
}

void ReverbPluginProcessor::parameterChanged(const String& parameterID, float newValue)
{
    DBG ("Changing Parameter: " + parameterID);
    if ( parameterID == "Reverb Enabled" ) {
        mReverb.reverbEnabled = (bool)newValue;
    } else if ( parameterID == "Reverb Level" ) {
        mReverb.reverbLevel = newValue;
    } else if ( parameterID == "BRIR" ) {
        mReverb.reverbBRIR = roundToInt (newValue);
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ReverbPluginProcessor();
}
