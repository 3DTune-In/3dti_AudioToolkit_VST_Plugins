/**
 * \class ChannelSwitchComponent
 *
 * \brief Declaration of ChannelSwitchComponent interface.
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

#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/

class ChannelSwitchComponent : public Component, public Button::Listener
{
public:
    ChannelSwitchComponent (HLSPluginAudioProcessor& p) : processor (p)
    {
        channelComponents[0].headingLabel.setText ("Left Channel", dontSendNotification);
        channelComponents[1].headingLabel.setText ("Right Channel", dontSendNotification);

        for (int i = 0; i < 2; i++)
        {
            addAndMakeVisible (channelComponents[i]);
            
            channelComponents[i].onOffToggle.onClick = [this, i] {
                bool isOn = channelComponents[i].onOffToggle.getToggleState();
                *processor.enableSimulation[i] = isOn;
            };
        }
        
        linkButton.setButtonText ("Link");
        linkButton.addListener (this);
        // addAndMakeVisible (linkButton);
        
        updateGUIState();
    }
    
    ~ChannelSwitchComponent() {}
    
    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        channelComponents[0].setBounds (bounds.removeFromLeft (proportionOfWidth (0.5)).reduced (8));
        channelComponents[1].setBounds (bounds.reduced (8));
    }
    
    void updateGUIState()
    {
        // linkButton.setToggleState (processor.channelsLink->get(), dontSendNotification);
        
        for (int i = 0; i < 2; i++)
        {
            channelComponents[i].onOffToggle.setToggleState (processor.enableSimulation[i]->get(), dontSendNotification);
        }
    }
    
    void buttonClicked (Button* button) override
    {
        // *processor.channelsLink = !processor.channelsLink->get();
    }
    
private:
    class SingleChannelComponent    : public Component, public Slider::Listener
    {
    public:
        SingleChannelComponent (String title = "Channel") : headingLabel (title, title)
        {
            headingLabel.setText (title, dontSendNotification);
            headingLabel.setFont (Font (20));
            addAndMakeVisible (headingLabel);
            
            onOffToggle.setButtonText ("On/Off");
            onOffToggle.setToggleState (true, dontSendNotification);
            onOffToggle.onClick = [this] { updateBypass(); };
            addAndMakeVisible (onOffToggle);
            
            /*
            addAndMakeVisible (gainLabel);
            
            gainSlider.setRange (-24.0f, 24.0f);
            gainSlider.setValue (0.0f);
            gainSlider.setNumDecimalPlacesToDisplay (0);
            gainSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
            gainSlider.setTextBoxStyle (Slider::TextBoxLeft,
                                               false,
                                               34,
                                               20);
            gainSlider.addListener (this);
            addAndMakeVisible (gainSlider);
            */
        }

        ~SingleChannelComponent()
        {
        }

        void resized() override
        {
            auto bounds = getLocalBounds();
            
            headingLabel.setBounds (bounds.removeFromTop (22));
            onOffToggle.setBounds (headingLabel.getBounds().removeFromRight (80));
            
            /*
            auto gainBounds = bounds.removeFromTop (bounds.proportionOfHeight (0.4f));
            gainLabel.setBounds (gainBounds.removeFromLeft (gainBounds.proportionOfWidth (0.15f)));
            gainSlider.setBounds (gainBounds);
             */
        }
        
        void sliderValueChanged(Slider* slider) override
        {
        }

        void updateBypass() {}
        
        Label           headingLabel;
        
        ToggleButton    onOffToggle;
        
        // Label           gainLabel;
        // Slider          gainSlider;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SingleChannelComponent)
    };
    
    HLSPluginAudioProcessor& processor;
    
    TextButton linkButton;
    SingleChannelComponent channelComponents[2];
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChannelSwitchComponent)
};

