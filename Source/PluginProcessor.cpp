/**
* \class Toolkit3dtiPluginAudioProcessor
*
* \brief Declaration of Toolkit3dtiPluginAudioProcessor interface.
* \date  June 2019
*
* \authors Reactify Music LLP: R. Hrafnkelsson ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2019
*
* \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

void addBooleanHostParameter(AudioProcessorValueTreeState& treeState, String name, int value) {
  const auto bypassValueToText = [](float value) {
    return value < 0.5f ? "Off" : "On";
  };
  
  const auto bypassTextToValue = [](const String& text) {
    if (text == "On") { return 1.0f; }
    return 0.0f;
  };
  
  treeState.createAndAddParameter(name, name, "", NormalisableRange<float>(0.f, 1.f, 1.f),
                                  value, bypassValueToText, bypassTextToValue, false, true,
                                  true, AudioProcessorParameter::genericParameter, true);
}

//==============================================================================
Toolkit3dtiPluginAudioProcessor::Toolkit3dtiPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::mono(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       ),
#endif
      treeState(*this, nullptr),
      inFifo (2, 512),
      outFifo(2, 512)
{
  auto position = getCore().getSourcePosition();
  
  treeState.createAndAddParameter("Azimuth", "Azimuth", "", NormalisableRange<float>(-180.f, 180.f), position.GetAzimuthDegrees(), [](float value) { return String (value, 1); }, nullptr);
  treeState.addParameterListener("Azimuth", this);
  
  treeState.createAndAddParameter("Elevation", "Elevation", "", NormalisableRange<float>(-89.f, 89.f), position.GetElevationDegrees(), [](float value) { return String (value, 0); }, nullptr);
  treeState.addParameterListener("Elevation", this);

  treeState.createAndAddParameter("Distance", "Distance", "", NormalisableRange<float>(0.f, 40.f), position.GetDistance(), [](float value) { return String (value, 2); }, nullptr);
  treeState.addParameterListener("Distance", this);
  
  treeState.createAndAddParameter("X", "X", "", NormalisableRange<float>(-40.f, 40.f), position.x, [](float value) { return String (value, 2); }, nullptr);
  treeState.addParameterListener("X", this);
  
  treeState.createAndAddParameter("Y", "Y", "", NormalisableRange<float>(-40.f, 40.f), position.y, [](float value) { return String (value, 2); }, nullptr);
  treeState.addParameterListener("Y", this);
  
  treeState.createAndAddParameter("Z", "Z", "", NormalisableRange<float>(-40.f, 40.f), position.z, [](float value) { return String (value, 2); }, nullptr);
  treeState.addParameterListener("Z", this);
  
  treeState.createAndAddParameter("Source Attenuation", "Src Attenuation", "", getCore().sourceDistanceAttenuation.range, getCore().sourceDistanceAttenuation.get(), nullptr, nullptr);
  treeState.addParameterListener("Source Attenuation", this);
  
  treeState.createAndAddParameter("Reverb Gain", "Reverb Gain", "", getCore().reverbGain.range, getCore().reverbGain.get(), nullptr, nullptr);
  treeState.addParameterListener("Reverb Gain", this);
  
  treeState.createAndAddParameter("Reverb Attenuation", "Rev Attenuation", "", getCore().reverbDistanceAttenuation.range, getCore().reverbDistanceAttenuation.get(), nullptr, nullptr);
  treeState.addParameterListener("Reverb Attenuation", this);
  
  addBooleanHostParameter(treeState, "Near Field", getCore().enableNearDistanceEffect);
  treeState.addParameterListener("Near Field", this);
  
  addBooleanHostParameter(treeState, "Far Field", getCore().enableFarDistanceEffect);
  treeState.addParameterListener("Far Field", this);
  
  addBooleanHostParameter(treeState, "Custom Head", getCore().enableCustomizedITD);
  treeState.addParameterListener("Custom Head", this);
  
  treeState.createAndAddParameter("Head Circumference", "Head Circumference", "", NormalisableRange<float>(getCore().headCircumference.getRange().getStart(), getCore().headCircumference.getRange().getEnd()), getCore().headCircumference.get(), [](float value) { return String (value, 0); }, nullptr);
  treeState.addParameterListener("Head Circumference", this);
  
  addBooleanHostParameter(treeState, "Enable Anechoic", true);
  treeState.addParameterListener("Enable Anechoic", this);
  
  addBooleanHostParameter(treeState, "Enable Reverb", true);
  treeState.addParameterListener("Enable Reverb", this);
  
  treeState.createAndAddParameter("HRTF", "HRTF", "", NormalisableRange<float>(0, BundledHRTFs.size()-1), 0, [](float value) { return String (value, 0); }, nullptr);
  treeState.addParameterListener("HRTF", this);
  
  treeState.createAndAddParameter("BRIR", "BRIR", "", NormalisableRange<float>(0, BundledBRIRs.size()-1), 0, [](float value) { return String (value, 0); }, nullptr);
  treeState.addParameterListener("BRIR", this);

  treeState.state = ValueTree("3D Tune-In Parameters");
}

Toolkit3dtiPluginAudioProcessor::~Toolkit3dtiPluginAudioProcessor()
{
}

//==============================================================================
const String Toolkit3dtiPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Toolkit3dtiPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Toolkit3dtiPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Toolkit3dtiPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Toolkit3dtiPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Toolkit3dtiPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Toolkit3dtiPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Toolkit3dtiPluginAudioProcessor::setCurrentProgram (int index)
{
}

const String Toolkit3dtiPluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void Toolkit3dtiPluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void Toolkit3dtiPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) {
  inFifo.clear();
  inFifo.setSize (getTotalNumOutputChannels(), samplesPerBlock * 2);
  outFifo.clear();
  outFifo.setSize(getTotalNumOutputChannels(), samplesPerBlock * 2);
   
  scratchBuffer.setSize(getTotalNumOutputChannels(), samplesPerBlock);
    
  getCore().setup (sampleRate, samplesPerBlock);
  
  startTimer(60);
}

void Toolkit3dtiPluginAudioProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Toolkit3dtiPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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
 
void Toolkit3dtiPluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
  ScopedNoDenormals noDenormals;
  
  auto inChannels  = getTotalNumInputChannels();
  auto outChannels = getTotalNumOutputChannels();

  for (auto i = inChannels; i < outChannels; ++i)
    buffer.clear(i, 0, buffer.getNumSamples());
  
  int bufferSize = buffer.getNumSamples();
    
  // Some hosts send buffers of varying sizes so we maintain
  // an internal buffer to pass the correct size to the 3dti core
  int index = 0;
  while (index < bufferSize)
  {
    int chunkSize   = std::min(128, bufferSize-index);
    int numChannels = buffer.getNumChannels();
      
    AudioBuffer<float> temp(numChannels, chunkSize);
      
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
      temp.copyFrom(ch, 0, buffer, ch, index, chunkSize);
      
    inFifo.addToFifo(temp);
      
    if (inFifo.getNumReady() >= getBlockSize())
    {
      inFifo.readFromFifo(scratchBuffer, getBlockSize());
        
      // Main process
      getCore().processBlock(scratchBuffer, midiMessages);
          
      outFifo.addToFifo(scratchBuffer);
    }
      
    index += chunkSize;
  }
    
  if (outFifo.getNumReady() < bufferSize)
  {
    int diff = bufferSize - outFifo.getNumReady();
    outFifo.addSilenceToFifo(diff);

    // Update the host latency
    int latency = getLatencySamples() + diff;
    setLatencySamples(latency);
  }
    
  outFifo.readFromFifo(buffer);
}

//==============================================================================
bool Toolkit3dtiPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* Toolkit3dtiPluginAudioProcessor::createEditor()
{
    return new Toolkit3dtiPluginAudioProcessorEditor (*this);
}

//==============================================================================
void Toolkit3dtiPluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Toolkit3dtiPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void Toolkit3dtiPluginAudioProcessor::updateHostParameters() {
  auto position = getCore().getSourcePosition();
  
  std::unordered_map<String, float> parameters = {
    {"Azimuth", mapAzimuthToSliderValue(position.GetAzimuthDegrees())},
    {"Distance", position.GetDistance()},
    {"Elevation", mapElevationToSliderValue(position.GetElevationDegrees())},
    {"X", position.x},
    {"Y", position.y},
    {"Z", position.z},
    {"Source Attenuation", getCore().sourceDistanceAttenuation},
    {"Reverb Gain", getCore().reverbGain},
    {"Reverb Attenuation", getCore().reverbDistanceAttenuation},
    {"Near Field", getCore().enableNearDistanceEffect},
    {"Far Field", getCore().enableFarDistanceEffect},
    {"Custom Head", getCore().enableCustomizedITD},
    {"Head Circumference", getCore().headCircumference},
    {"Enable Anechoic", getCore().getSources().front()->IsAnechoicProcessEnabled()},
    {"Enable Reverb", getCore().getSources().front()->IsReverbProcessEnabled()},
    {"HRFT", getCore().getHrtfIndex() },
    {"BRIR", getCore().getBrirIndex() },
  };

  for ( auto const & parameter : parameters ) {
    if ( AudioProcessorParameter* p = treeState.getParameter(parameter.first) ) {
      const float newValue = treeState.getParameterRange(parameter.first).convertTo0to1(parameter.second);
      
      if ( fabs(p->getValue() - newValue) > std::numeric_limits<float>::epsilon() )
        p->setValueNotifyingHost(newValue);
    }
  }
}

void Toolkit3dtiPluginAudioProcessor::parameterChanged(const String& parameterID, float newValue) {
  auto position = getCore().getSourcePosition();
  
  if ( parameterID == "Azimuth" ) {
    position.SetFromAED( mapSliderValueToAzimuth(newValue), position.GetElevationDegrees(), position.GetDistance() );
  } else if ( parameterID == "Distance" ) {
    position.SetFromAED( position.GetAzimuthDegrees(), position.GetElevationDegrees(), newValue );
  } else if ( parameterID == "Elevation" ) {
    position.SetFromAED( position.GetAzimuthDegrees(), mapSliderValueToElevation(newValue), position.GetDistance() );
  } else if ( parameterID == "X" ) {
    position.x = newValue;
  } else if ( parameterID == "Y" ) {
    position.y = newValue;
  } else if ( parameterID == "Z" ) {
    position.z = newValue;
  } else if ( parameterID == "Source Attenuation" ) {
    getCore().sourceDistanceAttenuation = newValue;
  } else if ( parameterID == "Reverb Gain" ) {
    getCore().reverbGain = newValue;
  } else if ( parameterID == "Reverb Attenuation" ) {
    getCore().reverbDistanceAttenuation = newValue;
  } else if ( parameterID == "Near Field" ) {
    getCore().enableNearDistanceEffect = (int)(newValue + 0.49f);
  } else if ( parameterID == "Far Field" ) {
    getCore().enableFarDistanceEffect = (int)(newValue + 0.49f);
  } else if ( parameterID == "Custom Head" ) {
    getCore().enableCustomizedITD = (int)(newValue + 0.49f);
  } else if ( parameterID == "Head Circumference" ) {
    if ( getCore().enableCustomizedITD ){
      getCore().headCircumference = newValue;
    }
  } else if ( parameterID == "Enable Anechoic" ) {
    if ( auto source = getCore().getSources().front() ) {
      bool enabled = (bool)newValue;
      if ( enabled ) {
        source->EnableAnechoicProcess();
      } else {
        source->DisableAnechoicProcess();
      }
    }
  } else if ( parameterID == "Enable Reverb" ) {
    if ( auto source = getCore().getSources().front() ) {
      bool enabled = (bool)newValue;
      if ( enabled ) {
        getCore().getSources().front()->EnableReverbProcess();
      } else {
        getCore().getSources().front()->DisableReverbProcess();
      }
    }
  } else if ( parameterID == "HRTF" ) {
    getCore().loadHRTF((int)newValue);
  } else if ( parameterID == "BRIR" ) {
    getCore().loadBRIR((int)newValue);
  }

  getCore().setSourcePosition(position);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Toolkit3dtiPluginAudioProcessor();
}
