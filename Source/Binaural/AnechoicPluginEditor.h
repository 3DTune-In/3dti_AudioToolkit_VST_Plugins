/**
* \class AnechoicPluginProcessorEditor
*
* \brief Declaration of AnechoicPluginProcessorEditor interface.
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

#pragma once

#include <JuceHeader.h>
#include "Common/AboutBanner.h"
#include "AnechoicControls.h"
#include "SourceControls.h"
#include "SourceReverbControls.h"
#include "SpatializerWidget.h"
#include "ElevationDial.h"
#include "AnechoicPluginProcessor.h"

//==============================================================================
/**
*/
class AnechoicPluginProcessorEditor : public AudioProcessorEditor, public Timer
{
public:
  AnechoicPluginProcessorEditor (AnechoicPluginProcessor&);
  ~AnechoicPluginProcessorEditor();

  //============================================================================
  void paint (Graphics&) override;
  void paintOverChildren (Graphics& g) override;
  void resized() override;
  
  void timerCallback() override
  {
    anechoicControls.updateGui();
    sourceControls.updateGui();
    spatializerWidget.updateGui();

    bool anechoicEnabled = anechoicControls.bypassToggle.getToggleState();
    anechoicControls.setAlpha(anechoicEnabled + 0.4f);
    sourceControls.setAlpha(anechoicEnabled + 0.4f);
  }
  
  void mouseDown (const MouseEvent &e) override {
     aboutText.setVisible (false);
  };
  
private:
  //============================================================================
  AnechoicPluginProcessor& processor;

  AboutBanner aboutBanner;

  SourceControls sourceControls;
  AnechoicControls anechoicControls;
  ReverbControls reverbControls;
  SpatializerWidget spatializerWidget;

  TextEditor aboutText;
  Label pluginVersionLabel;
  Label toolkitVersionLabel;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnechoicPluginProcessorEditor)
};
