/**
* \class AnechoicPluginProcessor
*
* \brief Declaration of AnechoicPluginProcessor interface.
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

#include "AnechoicPluginProcessor.h"
#include "AnechoicPluginEditor.h"

static constexpr int kTOOLKIT_BUFFER_SIZE = 512; // TODO(Ragnar): Make variable

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
AnechoicPluginProcessor::AnechoicPluginProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  JUCEApplication::isStandaloneApp() ? AudioChannelSet::stereo() : AudioChannelSet::mono(), true)
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                       .withOutput ("Sidechain", AudioChannelSet::quadraphonic(), true)
                       )
#endif
      , treeState (*this, nullptr)
{
    mSpatializer.addSoundSource (Common::CVector3(0,1,0));
    
    auto position = getCore().getSourcePosition(0);
  
    treeState.createAndAddParameter("Azimuth", "Azimuth", "", NormalisableRange<float>(0.0f, 359.99f), position.GetAzimuthDegrees(), [](float value) { return String (value, 1); }, nullptr);
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
  
    treeState.state = ValueTree("3DTI Anechoic Parameters");
    
#if DEBUG
    ERRORHANDLER3DTI.SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);
    ERRORHANDLER3DTI.SetErrorLogStream(&std::cout, true);
#endif
}

AnechoicPluginProcessor::~AnechoicPluginProcessor()
{
    stopTimer();
}

//==============================================================================
const String AnechoicPluginProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AnechoicPluginProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AnechoicPluginProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AnechoicPluginProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AnechoicPluginProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AnechoicPluginProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AnechoicPluginProcessor::getCurrentProgram()
{
    return 0;
}

void AnechoicPluginProcessor::setCurrentProgram (int index)
{
}

const String AnechoicPluginProcessor::getProgramName (int index)
{
    return {};
}

void AnechoicPluginProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void AnechoicPluginProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
  const int blockSizeInternal = kTOOLKIT_BUFFER_SIZE;
    
  // Set up anechoic buffers
  inFifoMain.clear();
  inFifoMain.setSize (1, blockSizeInternal + 1);
  
  outFifoMain.clear();
  outFifoMain.setSize (2, std::max(samplesPerBlock, blockSizeInternal) * 2);
   
  scratchBufferMain.setSize (2, blockSizeInternal);
    
  // Set up ambisonic buffers
  inFifoBuss.clear();
  inFifoBuss.setSize (4, blockSizeInternal + 1);
    
  outFifoBuss.clear();
  outFifoBuss.setSize (4, std::max (samplesPerBlock, blockSizeInternal) * 2);
    
  scratchBufferBuss.setSize (4, blockSizeInternal);
   
  // Initalise 3dti toolkit
  mCore.SetAudioState ({(int)sampleRate, blockSizeInternal});
    
  mSpatializer.setup (sampleRate/*,blockSizeInternal*/);
    
  startTimer (30);
}

void AnechoicPluginProcessor::releaseResources() {
  // When playback stops, you can use this as an opportunity to free up any
  // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AnechoicPluginProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainInputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainInputChannelSet() != AudioChannelSet::stereo())
        return false;
    
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    return true;
}
#endif
 
void AnechoicPluginProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;
  
    auto inChannels  = getTotalNumInputChannels();
    auto outChannels = getTotalNumOutputChannels();
    
    for (auto i = inChannels; i < outChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());
  
    AudioSampleBuffer mainInput = getBusBuffer (buffer, true, 0);
    AudioSampleBuffer sideChain = getBusBuffer (buffer, false, 1);
    
    AudioBuffer<float> tempMain (1, 1);

    int numSamples  = mainInput.getNumSamples();
    int numChannels = mainInput.getNumChannels();
    
    const int blockSizeInternal = kTOOLKIT_BUFFER_SIZE;
    // Some hosts send buffers of varying sizes so we maintain
    // an internal buffer to pass the correct size to the 3dti core
    for (int sample = 0; sample < numSamples; ++sample)
    {
        tempMain.clear();
        
        // Sum to mono in 'temp'
        for (int channel = 0; channel < numChannels; ++channel)
            tempMain.addSample (0, 0, mainInput.getSample (channel, sample));
        
        tempMain.applyGain (1.0f / (float)numChannels);
        
        inFifoMain.addToFifo (tempMain);
        
        if (inFifoMain.getFreeSpace() == 0)
        {
            AudioBuffer<float> monoIn (1, blockSizeInternal);
            
            inFifoMain.readFromFifo (monoIn, blockSizeInternal);
            
            bool isReady = ! mSpatializer.isLoading.load()
                          && mCore.GetListener()->GetHRTF()->IsHRTFLoaded();
            
            mSpatializer.processBlock (monoIn, scratchBufferMain);
            outFifoMain.addToFifo (scratchBufferMain);

            if (isReady)
            {
                mEncoder.processBlock (mSpatializer.getSources(), scratchBufferBuss);
                outFifoBuss.addToFifo (scratchBufferBuss);
            }
            else
            {
                outFifoBuss.addSilenceToFifo (blockSizeInternal);
            }
        }
    }
    
    int numReady = outFifoMain.getNumReady();
    if (numReady < numSamples)
    {
        int diff = numSamples - numReady;
        outFifoMain.addSilenceToFifo (diff);
        outFifoBuss.addSilenceToFifo (diff);

        // Update the host latency
        int latency = getLatencySamples() + diff;
        setLatencySamples (latency);
    }
    
    AudioSampleBuffer mainOutput = getBusBuffer (buffer, false, 0);
    
    outFifoMain.readFromFifo (mainOutput);
    
    if (sideChain.getNumChannels() >= outFifoBuss.getNumChannels())
        outFifoBuss.readFromFifo (sideChain);
    else
        outFifoBuss.removeSamplesFromFifo (numSamples);
}

//==============================================================================
bool AnechoicPluginProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* AnechoicPluginProcessor::createEditor()
{
    return new AnechoicPluginProcessorEditor (*this);
}

//==============================================================================
void AnechoicPluginProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void AnechoicPluginProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void AnechoicPluginProcessor::updateHostParameters()
{
  auto position = getCore().getSourcePosition (0);
  
  auto sources = getCore().getSources();
    
  std::unordered_map<String, float> parameters = {
    {"Azimuth", position.GetAzimuthDegrees()},
    {"Distance", position.GetDistance()},
    {"Elevation", mapElevationToSliderValue(position.GetElevationDegrees())},
    {"X", position.x},
    {"Y", position.y},
    {"Z", position.z},
    {"Source Attenuation", getCore().sourceDistanceAttenuation},
    {"Reverb Attenuation", getCore().reverbDistanceAttenuation},
    {"Near Field", getCore().enableNearDistanceEffect},
    {"Far Field", getCore().enableFarDistanceEffect},
    {"Custom Head", getCore().enableCustomizedITD},
    {"Head Circumference", getCore().headCircumference},
    {"HRFT", getCore().getHrtfIndex() },
  };

    if (sources.size() > 0)
    {
        parameters["Enable Anechoic"] = sources.front()->IsAnechoicProcessEnabled();
        parameters["Enable Reverb"] = sources.front()->IsReverbProcessEnabled();
    }

  for ( auto const & parameter : parameters ) {
    if ( AudioProcessorParameter* p = treeState.getParameter(parameter.first) ) {
      const float newValue = treeState.getParameterRange(parameter.first).convertTo0to1(parameter.second);
      
      if ( fabs(p->getValue() - newValue) > std::numeric_limits<float>::epsilon() )
        p->setValueNotifyingHost(newValue);
    }
  }
}

void AnechoicPluginProcessor::parameterChanged (const String& parameterID, float newValue)
{
  auto sources = getCore().getSources();
    
  auto position = getCore().getSourcePosition(0);
  
  if ( parameterID == "Azimuth" ) {
    DBG("Azimuth: " + String(newValue));
    position.SetFromAED( newValue, position.GetElevationDegrees(), position.GetDistance() );
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
        sources.front()->EnableReverbProcess();
      } else {
        sources.front()->DisableReverbProcess();
      }
    }
  } else if ( parameterID == "HRTF" ) {
    getCore().loadHRTF((int)newValue);
  }

  if (sources.size() > 0)
      getCore().setSourcePosition (sources.front(), position);
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AnechoicPluginProcessor();
}
