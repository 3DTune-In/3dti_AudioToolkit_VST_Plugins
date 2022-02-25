/**
* \class AnechoicControls
*
* \brief Declaration of AnechoicControls interface.
* \date  February 2022
*
* \authors Reactify Music LLP: R. Hrafnkelsson ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2022
*
* \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
*/

#pragma once

#include <JuceHeader.h>
#include "SpatialisePluginProcessor.h"

//==============================================================================
/*
 */
class AnechoicControls  : public Component,
                          public ChangeListener,
                          public Slider::Listener
{
public:
  //============================================================================
  AnechoicControls (AnechoicProcessor& processor, AudioProcessorValueTreeState& params);
  
  ~AnechoicControls();
  
  void updateGui();
  
  void paint (Graphics& g) override;
  
  void resized() override;
  
  void sliderValueChanged(Slider* slider) override;
    
  void changeListenerCallback (ChangeBroadcaster* source) override;
  
  ToggleButton bypassToggle;
  
private:
  //============================================================================
  void updateBypass();
  void updateHeadCircumference();
  void updateHrtfLabelText();
  void updateNearFieldCorrection();
  void updateFarFieldCorrection();
  void updateQualitySetting();
  void updateDistanceAttenuation();
  
  AnechoicProcessor& mCore;
  AudioProcessorValueTreeState& mParameters;
  
  ComboBox hrtfMenu;
  AudioProcessorValueTreeState::ComboBoxAttachment buttonAttachment;
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
