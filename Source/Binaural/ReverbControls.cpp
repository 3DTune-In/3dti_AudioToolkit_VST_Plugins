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
 * \b Copyright: University of Malaga and Imperial College London - 2019
 *
 * \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
 *
 * \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#include "ReverbControls.h"

//==============================================================================
ReverbControls::ReverbControls (ReverbProcessor& p)
  : mReverb (p),
    gainLabel("Level Label", "Level [dB]")
{
    brirMenu.addItemList (mReverb.getBRIROptions(), 1);
    brirMenu.onChange = [this] { brirMenuChanged(); };
    brirMenu.setSelectedId (1, dontSendNotification);
    addAndMakeVisible (brirMenu);
    
    setLabelStyle (gainLabel);
    gainLabel.setJustificationType (Justification::left);
    addAndMakeVisible (gainLabel);
    
    mapParameterToSlider (gainSlider, mReverb.reverbLevel);
    gainSlider.setTextValueSuffix (" dB");
    gainSlider.setTextBoxStyle (Slider::TextBoxRight, false, 65, 24);
    gainSlider.addListener (this);
    addAndMakeVisible (gainSlider);
    
    bypassToggle.setButtonText ("On/Off");
    bypassToggle.setToggleState (true, dontSendNotification);
    bypassToggle.onClick = [this] { mReverb.reverbEnabled = bypassToggle.getToggleState(); };
    addAndMakeVisible (bypassToggle);
    
    mReverb.addChangeListener (this);
    
    updateGui();
}

ReverbControls::~ReverbControls()
{
    mReverb.removeChangeListener (this);
}

//==============================================================================
void ReverbControls::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour(ResizableWindow::backgroundColourId));
    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);
    g.setColour (Colours::white);
    g.setFont (18.0f);
    g.drawText ("Reverberation",
                getLocalBounds().withTrimmedBottom( getLocalBounds().getHeight() - 32 ),
                Justification::centred,
                true);
}

void ReverbControls::resized()
{
    auto area = getLocalBounds();
    bypassToggle.setBounds (10, 4, 80, 24);
    brirMenu.setBounds (12, 40, area.getWidth()-24, 22);
    gainLabel.setBounds (10, brirMenu.getBottom() + 16, area.getWidth()-20, 24);
    gainSlider.setBounds (6, gainLabel.getBottom(), area.getWidth()-18, 24);
}

//==============================================================================
void ReverbControls::updateGui()
{
    updateBypass();
    gainSlider.setValue (mReverb.reverbLevel.get(), dontSendNotification);
}

void ReverbControls::updateBypass()
{
    bypassToggle.setToggleState (mReverb.reverbEnabled.get(), dontSendNotification);
    
    bool enabled = bypassToggle.getToggleState();
    setAlpha (enabled + 0.4f);
}

void ReverbControls::updateBrirLabel()
{
    String text;
    
    int brirIndex = mReverb.reverbBRIR.get();
    
    if (brirIndex < mReverb.reverbBRIR.getRange().getEnd() - 1)
        text = mReverb.getBRIROptions()[brirIndex];
    else
        text = mReverb.getBRIRPath().getFileNameWithoutExtension();
    
    brirMenu.setText (text, dontSendNotification);
}

//==============================================================================
void ReverbControls::changeListenerCallback (ChangeBroadcaster *source)
{
    updateBrirLabel();
}
