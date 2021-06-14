/**
* \class ReverbPluginProcessorEditor
*
* \brief Declaration of ReverbPluginProcessorEditor interface.
* \date  December 2020
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
#include "Common/AboutBanner.h"
#include "ReverbControls.h"
#include "ReverbPluginProcessor.h"

//==============================================================================
/**
*/
class ReverbPluginProcessorEditor : public AudioProcessorEditor, public Timer
{
public:
  ReverbPluginProcessorEditor (ReverbPluginProcessor&);
  ~ReverbPluginProcessorEditor();

  //============================================================================
  void paint (Graphics&) override;
  void resized() override;
  
  void timerCallback() override;
  
  void mouseDown(const MouseEvent &e) override {
    aboutText.setVisible(false);
  };
  
private:
  //============================================================================
  ReverbPluginProcessor& processor;

  AboutBanner aboutBanner;
  ReverbControls reverbControls;

  TextEditor aboutText;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbPluginProcessorEditor)
};
