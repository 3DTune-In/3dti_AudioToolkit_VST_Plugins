/**
 * \class HLSPluginAudioProcessorEditor
 *
 * \brief Declaration of HLSPluginAudioProcessorEditor interface.
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
 * \b Acknowledgement: This project HLS received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#include "Presets.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HLSPluginAudioProcessorEditor::HLSPluginAudioProcessorEditor (HLSPluginAudioProcessor& p)
  : AudioProcessorEditor(&p),
    processor(p),
    channelSwitchComponent (p),
    nonLinearAttenuatorComponent (p),
    temporalDistortionComponent (p),
    frequencySmearingComponent (p)
{
    //addAndMakeVisible (aboutText);
    addAndMakeVisible (aboutButton);
    addAndMakeVisible (toolkitVersionLabel);
    
    _3dTuneInLogo = ImageCache::getFromMemory(BinaryData::_3DTI_Logo_png, BinaryData::_3DTI_Logo_pngSize);
    imperialLogo = ImageCache::getFromMemory(BinaryData::Imperial_Logo_png, BinaryData::Imperial_Logo_pngSize);
    umaLogo = ImageCache::getFromMemory(BinaryData::UMA_Logo_png, BinaryData::UMA_Logo_pngSize);
    
    aboutText.setMultiLine(true);
    aboutText.setFont(Font(16.0f, Font::plain));
    aboutText.setText(String::fromUTF8(BinaryData::About_txt, BinaryData::About_txtSize));
    aboutText.setReadOnly(true);
    aboutText.setAlpha(0.9f);
    aboutText.setVisible (false);
    
    aboutButton.setButtonText("About");
    aboutButton.onClick = [this] {
        aboutText.setVisible(! aboutText.isVisible());
    };
    addAndMakeVisible (aboutButton);
    
    toolkitVersionLabel.setFont(Font(15.f, Font::plain));
    toolkitVersionLabel.setText("Version " +  String(JucePlugin_VersionString) + " (3DTI Toolkit v1.4)", dontSendNotification);
            
    audiogramComponent.reset (new AudiogramComponent (p.getBandFrequencies(), 2));
    audiogramComponent->addListener (this);
    audiogramComponent->linkToggle.onClick = [this] {
        *processor.hearingLossLink = audiogramComponent->linkToggle.getToggleState();
    };
    
    addAndMakeVisible (channelSwitchComponent);
    addAndMakeVisible (audiogramComponent.get());
    addAndMakeVisible (nonLinearAttenuatorComponent);
    addAndMakeVisible (temporalDistortionComponent);
    addAndMakeVisible (frequencySmearingComponent);
    addChildComponent (aboutText);
    
    setSize (800, 800);
    
    startTimer (30);
}

HLSPluginAudioProcessorEditor::~HLSPluginAudioProcessorEditor() {
    audiogramComponent->removeListener (this);
    stopTimer();
}

void HLSPluginAudioProcessorEditor::timerCallback()
{
    channelSwitchComponent.updateGUIState();
    
    bool linked = processor.hearingLossLink->get();
    
    audiogramComponent->linkToggle.setToggleState (linked, dontSendNotification);
    
    for (int ch = 0; ch < processor.getTotalNumOutputChannels(); ch++)
    {
        int channel = (! linked) * ch;
        
        for (int i = 0; i < Constants::NUM_BANDS; i++)
            audiogramComponent->setSliderValue (ch, i, (double)processor.hearingLoss[channel][i]->get());
        
        bool channelEnabled = processor.enableSimulation[ch]->get();
        audiogramComponent->setChannelEnabled (ch, channelEnabled);
        
        int selectedPreset = selectedPresets().audiometryPreset[channel];
        audiogramComponent->presetSelectors[ch].setSelectedItemIndex (selectedPreset, dontSendNotification);
    }
    
    nonLinearAttenuatorComponent.updateGUIState();
    temporalDistortionComponent.updateGUIState();
    frequencySmearingComponent.updateGUIState();
}

//==============================================================================
void HLSPluginAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colour (24, 31, 34));
    
    auto bounds = getLocalBounds();
    
    auto aboutRect = bounds.removeFromTop (proportionOfHeight (0.06));
    g.setColour(getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
    g.fillRect(aboutRect);
    g.setColour (Colours::grey);
    g.drawRect(aboutRect);
    
    auto aboutRectf = aboutRect.toFloat();
    int amountToRemove = aboutRectf.proportionOfWidth (0.17);
    g.drawImage (umaLogo, aboutRectf.removeFromRight (amountToRemove).reduced (4), RectanglePlacement::centred);
    g.drawImage (imperialLogo, aboutRectf.removeFromRight (amountToRemove).reduced (4), RectanglePlacement::centred);
    g.drawImage (_3dTuneInLogo, aboutRectf.removeFromRight (amountToRemove).reduced (4), RectanglePlacement::centred);
}
                                    
void HLSPluginAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    auto aboutBounds = bounds.removeFromTop (proportionOfHeight (0.06));
    aboutButton.setBounds (aboutBounds.removeFromLeft (74).reduced (8));
    toolkitVersionLabel.setBounds (aboutBounds);
    
    aboutText.setBounds (bounds);

    channelSwitchComponent.setBounds (bounds.removeFromTop (proportionOfHeight (0.05)));
    audiogramComponent->setBounds (bounds.removeFromTop (proportionOfHeight (0.25f)));
    nonLinearAttenuatorComponent.setBounds (bounds.removeFromTop (proportionOfHeight (0.1)));
    temporalDistortionComponent.setBounds (bounds.removeFromTop (bounds.proportionOfHeight (0.5)));
    frequencySmearingComponent.setBounds (bounds);
}

void HLSPluginAudioProcessorEditor::audiogramPresetSelected (int channel, int index)
{
    for (int i = 0; i < Constants::NUM_BANDS; i++)
        *processor.hearingLoss[channel][i] = hearingLossPresetValues[index][i];
    
    selectedPresets().audiometryPreset[channel] = index;
}

void HLSPluginAudioProcessorEditor::audiogramSliderValueChanged (int channel, int band, double value)
{
    *processor.hearingLoss[channel][band] = (float)value;
}