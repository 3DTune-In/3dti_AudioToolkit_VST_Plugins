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


#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/*
*/

class ChannelSettingsComponent : public Component, public Button::Listener, public Slider::Listener
{
public:
    ChannelSettingsComponent (HASPluginAudioProcessor& p)
        : processor (p),
          channelComponents ({
              new SingleChannelComponent ("Left Channel", false),
              new SingleChannelComponent ("Right Channel", true)
          }),
          quantisationLabel ("Quantisation", "Quantisation"),
          quantisationBitsLabel ("Bits", "Bits")
    {
        for (auto* component : channelComponents)
        {
            component->gainSlider.addListener (this);
            component->onOffToggle.addListener (this);
            addAndMakeVisible (component);
        }
        
        linkToggle.setButtonText ("Link");
        linkToggle.setToggleState (true, dontSendNotification);
        linkToggle.addListener (this);
        addAndMakeVisible (linkToggle);
        
        quantisationLabel.setFont (Font (18));
        addAndMakeVisible (quantisationLabel);
    
        preprocessingToggle.setButtonText ("Pre Processing");
        preprocessingToggle.setToggleState (false, dontSendNotification);
        preprocessingToggle.addListener (this);
        addAndMakeVisible (preprocessingToggle);
        
        postprocessingToggle.setButtonText ("Post Processing");
        postprocessingToggle.setToggleState (false, dontSendNotification);
        postprocessingToggle.addListener (this);
        addAndMakeVisible (postprocessingToggle);
        
        quantisationBitsLabel.setJustificationType (Justification::left);
        addAndMakeVisible (quantisationBitsLabel);
        
        quantisationBitsSlider.setRange (6.0f, 24.0f);
        quantisationBitsSlider.setValue (0.0f);
        quantisationBitsSlider.setNumDecimalPlacesToDisplay (0);
        quantisationBitsSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
        quantisationBitsSlider.setTextBoxStyle (Slider::TextBoxLeft,
                                                false,
                                                34,
                                                20);
        quantisationBitsSlider.addListener (this);
        addAndMakeVisible (quantisationBitsSlider);
        
        updateGUIState();
    }
    
    ~ChannelSettingsComponent() {}
    
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (8);
        
        auto toggleBounds = r;
        toggleBounds = toggleBounds.removeFromTop (22);
        linkToggle.setBounds (toggleBounds.withSizeKeepingCentre (60, toggleBounds.getHeight()));
        
        auto channelBounds = r.removeFromTop (r.proportionOfHeight (0.6f));
        auto channelBoundsWidth = channelBounds.proportionOfWidth (0.5f);
        channelComponents[0]->setBounds (channelBounds.removeFromLeft (channelBoundsWidth));
        channelComponents[1]->setBounds (channelBounds.removeFromRight (channelBoundsWidth));
        
        auto width = r.proportionOfWidth (1.0f/6.0f);
        quantisationLabel.setBounds (r.removeFromLeft (width));
        preprocessingToggle.setBounds (r.removeFromLeft (width));
        postprocessingToggle.setBounds (r.removeFromLeft (width));
        
        r.removeFromLeft (3);
        quantisationBitsLabel.setBounds (r.removeFromLeft (50.0f));
        quantisationBitsSlider.setBounds (r);
    }
    
    void updateGUIState()
    {
        bool channelsLinked = processor.channelLink->get();
        
        linkToggle.setToggleState (channelsLinked, dontSendNotification);
        
        for (int i = 0; i < channelComponents.size(); i++)
        {
            int channel = channelsLinked ? 0 : i;
            channelComponents[i]->onOffToggle.setToggleState (processor.channelEnabled[channel]->get(), dontSendNotification);
            
            float gain = processor.channelGain[channel]->get();
            float db = Decibels::gainToDecibels (gain);
            channelComponents[i]->gainSlider.setValue (db, dontSendNotification);
        }
        preprocessingToggle.setToggleState (processor.enableQuantisationPre->get(), dontSendNotification);
        postprocessingToggle.setToggleState (processor.enableQuantisationPost->get(), dontSendNotification);
        quantisationBitsSlider.setValue (processor.quantisationNumBits->get(), dontSendNotification);
    }
    
    void buttonClicked (Button* button) override
    {
        if (button == &linkToggle)
        {
            *processor.channelLink = !processor.channelLink->get();
        }
        else if (button == &preprocessingToggle)
        {
            *processor.enableQuantisationPre = !processor.enableQuantisationPre->get();
        }
        else if (button == &postprocessingToggle)
        {
            *processor.enableQuantisationPost = !processor.enableQuantisationPost->get();
        }
        else
        {
            auto channel = button->getParentComponent() == channelComponents[1];
            *processor.channelEnabled[channel] = !processor.channelEnabled[channel]->get();
        }
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        auto* parentComponent = slider->getParentComponent();
        if (parentComponent != this)
        {
            if (slider == &((SingleChannelComponent *)parentComponent)->gainSlider)
            {
                auto gain = Decibels::decibelsToGain (slider->getValue());
                auto channel = parentComponent == channelComponents[1];
                *processor.channelGain[channel] = gain;
            }
        }
        else
        {
            *processor.quantisationNumBits = (int)slider->getValue();
        }
    }
    
private:
    struct SingleChannelComponent : public Component
    {
        SingleChannelComponent (String heading = "Channel", bool mirrored = false);

        ~SingleChannelComponent();

        void resized() override;

        void updateBypass() {}
        
        Label           headingLabel;
        
        ToggleButton    onOffToggle;
        
        Label           gainLabel;
        Slider          gainSlider;
        
        bool            mirrored;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleChannelComponent)
    };
    
    HASPluginAudioProcessor& processor;
    
    ToggleButton linkToggle;
    juce::OwnedArray<SingleChannelComponent> channelComponents;
    
    Label           quantisationLabel;
    ToggleButton    preprocessingToggle;
    ToggleButton    postprocessingToggle;
    
    Label           quantisationBitsLabel;
    Slider          quantisationBitsSlider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelSettingsComponent)
};

