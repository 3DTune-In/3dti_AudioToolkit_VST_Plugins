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
 * \b Copyright: University of Malaga and Imperial College London - 2019
 *
 * \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
 *
 * \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#pragma once

#include <JuceHeader.h>
#include <ff_buffers/ff_buffers_AudioBufferFIFO.h>
#include "ReverbProcessor.h"

//==============================================================================
/**
*/

using CSingleSourceRef = std::shared_ptr<Binaural::CSingleSourceDSP>;

class ReverbPluginProcessor  : public AudioProcessor
                             , private AudioProcessorValueTreeState::Listener
                             , private Timer
{
public:
  //============================================================================
  ReverbPluginProcessor();
  ~ReverbPluginProcessor();

  //============================================================================
  void prepareToPlay (double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;

 #ifndef JucePlugin_PreferredChannelConfigurations
  bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
 #endif

  void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

  //============================================================================
  AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override;

  //============================================================================
  const String getName() const override;

  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;

  //============================================================================
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram (int index) override;
  const String getProgramName (int index) override;
  void changeProgramName (int index, const String& newName) override;

  //============================================================================
  void getStateInformation (MemoryBlock& destData) override;
  void setStateInformation (const void* data, int sizeInBytes) override;
  
  //============================================================================
  ReverbProcessor&   getReverbProcessor() { return mReverb; }
  
  AudioProcessorValueTreeState treeState;
  
private:
    //==========================================================================
  void timerCallback() override
  {
  }
  
  void updateHostParameters();
  
  void parameterChanged(const String& parameterID, float newValue) override;
    
  AudioBuffer<float>     scratchBufferStereo, scratchBufferQuad;
  AudioBufferFIFO<float> inFifo, outFifo;
   
  //============================================================================
  Binaural::CCore mCore;
  ReverbProcessor mReverb {mCore};
  
  //==============================================================================
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbPluginProcessor)
};
