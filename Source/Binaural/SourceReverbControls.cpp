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

#include "SourceReverbControls.h"

ReverbControls::ReverbControls (AnechoicPluginProcessor& p)
  :  mProcessor(p)
  ,  mCore (p.getCore())
  ,  distanceAttenuationLabel("Distance Label", "Reverb dB attenuation per double distance")
{
    distanceAttenuationToggle.setButtonText ("On/Off");
    addAndMakeVisible (distanceAttenuationToggle);

    setLabelStyle (distanceAttenuationLabel);
    distanceAttenuationLabel.setJustificationType (Justification::right);
    addAndMakeVisible (distanceAttenuationLabel);

    distanceAttenuationSlider.setTextValueSuffix (" dB");
    distanceAttenuationSlider.setTextBoxStyle (Slider::TextBoxRight, false, 65, 24);
    addAndMakeVisible (distanceAttenuationSlider );
  
    bypassToggle.setButtonText ("On/Off");
    addAndMakeVisible( bypassToggle );
  
    updateGui();
}

void ReverbControls::updateGui()
{
    bool distanceAttenuationEnabled = mProcessor.getCore().enableReverbDistanceAttenuation;
    distanceAttenuationLabel.setEnabled (distanceAttenuationEnabled);
    distanceAttenuationSlider.setEnabled (distanceAttenuationEnabled);
    setAlpha ((float)bypassToggle.getToggleState() + 0.4f);
}
