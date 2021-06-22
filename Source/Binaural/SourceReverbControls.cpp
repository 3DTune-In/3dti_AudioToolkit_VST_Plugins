/**
* \class ReverbControls
*
* \brief Declaration of ReverbControls interface.
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

#include "SourceReverbControls.h"

ReverbControls::ReverbControls (AnechoicPluginProcessor& p)
  :  mProcessor(p)
  ,  mCore (p.getCore())
  // ,  gainLabel("Gain Label", "Reverb Gain (dB)")
  ,  distanceAttenuationLabel("Distance Label", "Reverb dB attenuation per double distance")
{
//  setLabelStyle( gainLabel );
//  gainLabel.setJustificationType( Justification::left );
//  addAndMakeVisible( gainLabel );
//
//  mapParameterToSlider( gainSlider, mReverb.reverbLevel );
//  gainSlider.setTextValueSuffix(" dB");
//  gainSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
//  gainSlider.addListener( this );
//  addAndMakeVisible( gainSlider );
  
    distanceAttenuationToggle.setButtonText ("On/Off");
    distanceAttenuationToggle.setToggleState (true, dontSendNotification);
    distanceAttenuationToggle.onClick = [this] { updateDistanceAttenuation(); };
    addAndMakeVisible (distanceAttenuationToggle);

    setLabelStyle (distanceAttenuationLabel);
    distanceAttenuationLabel.setFont (Font (13.0f, Font::plain));
    distanceAttenuationLabel.setJustificationType (Justification::right);
    addAndMakeVisible (distanceAttenuationLabel);

    mapParameterToSlider (distanceAttenuationSlider, mCore.reverbDistanceAttenuation);
    distanceAttenuationSlider.setTextValueSuffix (" dB");
    distanceAttenuationSlider.setTextBoxStyle (Slider::TextBoxRight, false, 65, 24);
    addAndMakeVisible (distanceAttenuationSlider );
    distanceAttenuationSlider.onValueChange = [=] {
        mCore.reverbDistanceAttenuation = distanceAttenuationSlider.getValue();
    };
  
    bypassToggle.setButtonText("On/Off");
    bypassToggle.setToggleState(true, dontSendNotification);
    bypassToggle.onClick = [this] { updateBypass(); };
    addAndMakeVisible( bypassToggle );
  
  updateGui();
}

void ReverbControls::updateGui() {
    // gainSlider.setValue(mReverb.reverbLevel.get(), dontSendNotification);
    distanceAttenuationSlider.setValue (mCore.reverbDistanceAttenuation, dontSendNotification);
}

void ReverbControls::updateBypass() {
  bool enabled = bypassToggle.getToggleState();
  if ( enabled  ) {
    mProcessor.getSources().front()->EnableReverbProcess();
  } else {
    mProcessor.getSources().front()->DisableReverbProcess();
  }
  setAlpha( enabled + 0.4f );
}

//void ReverbControls::updateBrirLabel() {
//  auto brir = mReverb.getBrirPath().getFileNameWithoutExtension().upToLastOccurrenceOf("_", false, false);
//  brirMenu.setText(brir, dontSendNotification);
//}

void ReverbControls::updateDistanceAttenuation() {
  auto source = mProcessor.getSources().front();
  if (distanceAttenuationToggle.getToggleState()) {
    source->EnableDistanceAttenuationReverb();
  } else {
    source->DisableDistanceAttenuationReverb();
  }
}
