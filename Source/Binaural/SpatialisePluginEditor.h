/**
* \class Toolkit3dtiPluginAudioProcessorEditor
*
* \brief Declaration of Toolkit3dtiPluginAudioProcessorEditor interface.
* \date  June 2019
*
* \authors Reactify Music LLP: R. Hrafnkelsson ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2021
*
* \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
*/

#pragma once

#include <JuceHeader.h>
#include "AnechoicControls.h"
#include "v1/ReverbControls.h"
#include "SourceControls.h"
#include "SpatializerWidget.h"
#include "ElevationDial.h"

class Toolkit3dtiPluginAudioProcessor;

//==============================================================================
/**
*/
class Toolkit3dtiPluginAudioProcessorEditor : public AudioProcessorEditor, public Timer {
public:
  Toolkit3dtiPluginAudioProcessorEditor (Toolkit3dtiPluginAudioProcessor&);
  ~Toolkit3dtiPluginAudioProcessorEditor();

  //============================================================================
  void paint (Graphics&) override;
  void resized() override;
  
  void timerCallback() override {
    anechoicControls.updateGui();
    reverbControls.updateGui();
    sourceControls.updateGui();
    spatializerWidget.updateGui();
    
    bool anechoicEnabled = anechoicControls.bypassToggle.getToggleState();
    anechoicControls.setAlpha(anechoicEnabled + 0.4f);
    sourceControls.setAlpha(anechoicEnabled + 0.4f);
    
    bool reverbEnabled = reverbControls.bypassToggle.getToggleState();
    reverbControls.setAlpha(reverbEnabled + 0.4f);
  }
  
  void mouseDown(const MouseEvent &e) override {
    aboutText.setVisible(false);
  };
  
private:
  // This reference is provided as a quick way for your editor to
  // access the processor object that created it.
  Toolkit3dtiPluginAudioProcessor& processor;

  // Sub components
  SourceControls sourceControls;
  ReverbControls reverbControls;
  AnechoicControls anechoicControls;
  SpatializerWidget spatializerWidget;
  TextEditor aboutText;
  TextButton aboutButton;
  Label pluginVersionLabel;
  Label toolkitVersionLabel;
  
  Image _3dTuneInLogo;
  Image imperialLogo;
  Image umaLogo;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Toolkit3dtiPluginAudioProcessorEditor)
};
