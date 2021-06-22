/**
 * \class ChannelSettingsComponent
 *
 * \brief Declaration of ChannelSettingsComponent interface.
 * \date  November 2020
 *
 * \authors Reactify Music LLP: R. Hrafnkelsson ||
 * Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
 * \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
 *
 * \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
 * \b Website: http://3d-tune-in.eu/
 *
 * \b Copyright: University of Malaga and Imperial College London - 2020
 *
 * \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
 *
 * \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#include "ChannelSettingsComponent.h"


ChannelSettingsComponent::SingleChannelComponent::SingleChannelComponent (String heading, bool mirrored)
  : headingLabel (heading, heading),
    gainLabel ("Gain", "Gain  [dB]"),
    mirrored (mirrored)
{
    headingLabel.setFont (Font (20));
    headingLabel.setJustificationType (mirrored ? Justification::right : Justification::left);
    addAndMakeVisible (headingLabel);
    
    onOffToggle.setButtonText ("On/Off");
    onOffToggle.setToggleState (true, dontSendNotification);
    onOffToggle.onClick = [this] { updateBypass(); };
    addAndMakeVisible (onOffToggle);
    
    addAndMakeVisible (gainLabel);
    
    gainSlider.setRange (-24.0f, 24.0f);
    gainSlider.setValue (0.0f);
    gainSlider.setNumDecimalPlacesToDisplay (1);
    gainSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    gainSlider.setTextBoxStyle (Slider::TextBoxLeft,
                                       false,
                                       50,
                                       20);
    addAndMakeVisible (gainSlider);
}

ChannelSettingsComponent::SingleChannelComponent::~SingleChannelComponent()
{
}

void ChannelSettingsComponent::SingleChannelComponent::resized()
{
    auto r = getLocalBounds();
    
    auto headingBounds = r.removeFromTop (22);
    
    if (mirrored)
    {
        headingLabel.setBounds (headingBounds.removeFromRight (headingBounds.proportionOfWidth (0.4f)));
        onOffToggle.setBounds (headingBounds.removeFromRight (80));
    }
    else
    {
        headingLabel.setBounds (headingBounds.removeFromLeft (headingBounds.proportionOfWidth (0.4f)));
        onOffToggle.setBounds (headingBounds.removeFromLeft (80));
    }
    
    
    r.removeFromTop (8);
    
    gainLabel.setBounds (r.removeFromLeft (r.proportionOfWidth (0.2f)));
    gainSlider.setBounds (r);
}
