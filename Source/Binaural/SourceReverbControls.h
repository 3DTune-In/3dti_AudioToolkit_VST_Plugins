/**
* \class ReverbControls
*
* \brief Declaration of ReverbControls interface.
* \date  November 2021
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
#include "AnechoicProcessor.h"
#include "AnechoicPluginProcessor.h"
#include "Utils.h"

//==============================================================================
/*
 */

class ReverbControls : public Component, public Slider::Listener {
public:
  ReverbControls (AnechoicPluginProcessor& processor);
  
  ~ReverbControls() {}
  
  void paint (Graphics& g) override {
      g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
      g.setColour (Colours::white);
      g.setFont (18.0f);
      g.drawText ("3D Reverb send",
                  getLocalBounds().withTrimmedBottom( getLocalBounds().getHeight() - 32 ),
                  Justification::centred,
                  true);
  }
  
  void resized() override
  {
    bypassToggle.setBounds( 10, 4, 80, 24 );
      
    auto area = getLocalBounds();
      
    distanceAttenuationToggle.setBounds( 10, 40, 70, 24);
    distanceAttenuationLabel.setBounds (distanceAttenuationToggle.getRight(), distanceAttenuationToggle.getY(), area.getWidth()-80, 24);
    distanceAttenuationSlider.setBounds( 6, distanceAttenuationToggle.getBottom() + 4, area.getWidth()-18, 24);
  }
  
  void updateGui();
  
  void sliderValueChanged (Slider* slider) override {
      mCore.reverbDistanceAttenuation = (float)slider->getValue();
  }

  void loadCustomBRIR(String fileTypes);
  
private:
    //==========================================================================
    AnechoicPluginProcessor& mProcessor;
    AnechoicProcessor& mCore;
    
    ToggleButton bypassToggle;
    AudioProcessorValueTreeState::ButtonAttachment bypassAttachment {mProcessor.treeState, "Enable Reverb", bypassToggle};
  
    ToggleButton distanceAttenuationToggle;
    AudioProcessorValueTreeState::ButtonAttachment buttonAttachment {mProcessor.treeState, "Enable Rev Dist Attenuation", distanceAttenuationToggle};
    
    Label distanceAttenuationLabel;
    Slider distanceAttenuationSlider;
    AudioProcessorValueTreeState::SliderAttachment sliderAttachment {mProcessor.treeState, "Reverb Attenuation", distanceAttenuationSlider};
  
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbControls)
};
