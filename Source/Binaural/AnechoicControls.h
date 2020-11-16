/**
* \class AnechoicControls
*
* \brief Declaration of AnechoicControls interface.
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
#include "PluginProcessor.h"

//==============================================================================
/*
 */
class AnechoicControls  : public Component, public Slider::Listener
{
public:
  AnechoicControls(Toolkit3dtiPluginAudioProcessor& processor);
  
  ~AnechoicControls() {}
  
  void updateGui();
  
  void paint (Graphics& g) override {
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);
    g.setColour(Colours::white);
    g.setFont(18.0f);
    g.drawText("Anechoic Path", getLocalBounds().withTrimmedBottom( getLocalBounds().getHeight() - 32 ),
                Justification::centred, true);
  }
  
  void resized() override;
  
  void sliderValueChanged(Slider* slider) override {
    if ( slider == &headCircumferenceSlider ) {
      updateHeadCircumference();
    } else {
      mCore.sourceDistanceAttenuation = (float)slider->getValue();
    }
  }
  
  ToggleButton bypassToggle;
  
private:
  
  void hrtfMenuChanged();
  void loadCustomHrtf(String fileTypes);
  void updateBypass();
  void updateHeadCircumference();
  void updateHrtfLabelText();
  void updateNearFieldCorrection();
  void updateFarFieldCorrection();
  void updateQualitySetting();
  void updateDistanceAttenuation();
  
  Toolkit3dtiPluginAudioProcessor& mProcessor;
  AnechoicProcessor& mCore;
  
  ComboBox hrtfMenu;
  ToggleButton headCircumferenceToggle;
  Slider headCircumferenceSlider;
  std::unique_ptr<FileChooser> fc;
  
  ToggleButton nearFieldToggle;
  ToggleButton farFieldToggle;
  ToggleButton qualityToggle;
  ToggleButton distanceAttenuationToggle;
  Label distanceAttenuationLabel;
  Slider distanceAttenuationSlider;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AnechoicControls)
};
