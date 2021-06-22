/**
 * \class NonLinearAttenuatorComponent
 *
 * \brief Declaration of NonLinearAttenuatorComponent interface.
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
class NonLinearAttenuatorComponent : public Component, public Button::Listener, public ComboBox::Listener
{
public:
    NonLinearAttenuatorComponent (HLSPluginAudioProcessor& p)
      : processor (p),
        headingLabel ("Non-Linear Attenuation", "Non-Linear Attenuation")
   {
       headingLabel.setFont (Font (20));
       addAndMakeVisible (headingLabel);
       
       linkToggle.setButtonText ("Link");
       linkToggle.addListener (this);
       addAndMakeVisible (linkToggle);

       for (int ch = 0; ch < 2; ch++)
       {
           auto& channel = channels[ch];
           channel.expanderTypeSelector.addListener (this);
           channel.onToggle.addListener (this);
           addAndMakeVisible (channel);
       }
    }

    ~NonLinearAttenuatorComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background

        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);
        
        using Track = Grid::TrackInfo;
        
        Grid headingGrid;
        headingGrid.templateRows    = { Track (22_px) };
        headingGrid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr)};
        headingGrid.items = {
            GridItem (headingLabel),
            GridItem (linkToggle).withWidth (60.0f).withJustifySelf (GridItem::JustifySelf::center)
        };
        headingGrid.performLayout (bounds.removeFromTop (22));
        
        channels[0].setBounds (bounds.removeFromLeft (proportionOfWidth (0.5)));
        channels[1].setBounds (bounds);
    }
    
    void buttonClicked (Button* button) override
    {
        auto* parentComponent = button->getParentComponent();
        if (parentComponent != this)
        {
            auto channel = parentComponent == &channels[1];
            *processor.nonLinearAttenuationEnabled[channel] = button->getToggleState();
        }
        
        if (button == &linkToggle)
            *processor.nonLinearAttenuationLink = !processor.nonLinearAttenuationLink->get();
    }
    
    void comboBoxChanged (ComboBox* comboBox) override
    {
        int selectedIndex = comboBox->getSelectedItemIndex();
        int channel = comboBox->getParentComponent() == &channels[1];
        *processor.nonLinearAttenuationFilterType[channel] = selectedIndex;
    }
    
    void updateGUIState()
    {
        linkToggle.setToggleState (processor.nonLinearAttenuationLink->get(), dontSendNotification);
        
        for (int i = 0; i < 2; i++)
        {
            auto& component = channels[i];
            
            int channel = (! processor.nonLinearAttenuationLink->get()) * i;
            bool mainEnabled = processor.enableSimulation[i]->get();
            bool nonLinearAttenuationEnabled = processor.nonLinearAttenuationEnabled[channel]->get();
            
            component.setEnabled (mainEnabled
                               && nonLinearAttenuationEnabled);
            component.onToggle.setEnabled (mainEnabled);
            component.onToggle.setToggleState (nonLinearAttenuationEnabled, dontSendNotification);

            component.expanderTypeSelector.setSelectedItemIndex (processor.nonLinearAttenuationFilterType[channel]->get(),
                                                                 dontSendNotification);
        }
    }

private:
    
    struct Channel : Component
    {
        Channel()
        {
            StringArray options = {"Butterworth", "Gammatone"};

            expanderTypeSelector.setEditableText (false);
            expanderTypeSelector.setJustificationType (Justification::centred);
            expanderTypeSelector.addItemList (options, 1);
            expanderTypeSelector.setSelectedItemIndex (0, dontSendNotification);
            addAndMakeVisible (expanderTypeSelector);
            
            onToggle.setButtonText ("On/Off");
            onToggle.setToggleState (false, dontSendNotification);
            addAndMakeVisible (onToggle);
        }
        
        void resized() override
        {
            auto bounds = getLocalBounds().reduced (8);
            
            onToggle.setBounds (bounds.removeFromRight (80));
            expanderTypeSelector.setBounds (bounds.reduced (20, 0));
        }
        
        void setEnabled (bool enabled)
        {
            onToggle.setToggleState (enabled, dontSendNotification);
            expanderTypeSelector.setEnabled (enabled);
        }

        ComboBox expanderTypeSelector;
        ToggleButton onToggle;
    };
    
    HLSPluginAudioProcessor& processor;
    
    Label    headingLabel;
    ToggleButton linkToggle;
    Channel  channels[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonLinearAttenuatorComponent)
};
