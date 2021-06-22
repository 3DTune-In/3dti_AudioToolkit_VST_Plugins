/**
 * \class AudiogramComponent
 *
 * \brief Declaration of AudiogramComponent interface.
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
#include "Utilities.h"
#include "StackedSliderComponent.h"

//==============================================================================
/*
*/
class AudiogramComponent : public Component, public ComboBox::Listener
{
public:
    struct Listener
    {
        virtual ~Listener() = default;
        
        virtual void audiogramPresetSelected (int channel, int index) {};
        virtual void audiogramSliderValueChanged (int channel, int band, double value) {};
    };
    
    //==============================================================================
    AudiogramComponent (juce::Array<int> bands, int numChannels = 2, StringArray presets = {"None", "Mild", "Moderate", "Severe"})
      : headingLabel ("Audiometry", "Audiometry")
    {
        headingLabel.setFont (Font (20));
        addAndMakeVisible (headingLabel);
        
        bypassToggle.setButtonText ("On/Off");
        bypassToggle.setToggleState (true, dontSendNotification);
        // bypassToggle.onClick = [this] { setEnabled (bypassToggle.getToggleState()); };
        // addAndMakeVisible (bypassToggle);
        
        linkToggle.setButtonText ("Link");
        linkToggle.setToggleState (true, dontSendNotification);
        addAndMakeVisible (linkToggle);
        
        for (int ch = 0; ch < numChannels; ch++)
        {
            auto& presetSelector = presetSelectors[ch];
            presetSelector.setEditableText (false);
            presetSelector.setJustificationType (Justification::left);
            presetSelector.addItemList (presets, 1);
            presetSelector.setSelectedItemIndex (0, dontSendNotification);
            presetSelector.addListener (this);
            addAndMakeVisible (presetSelector);
        
            auto rangeReversed = NormalisableRange<double> (0.0, 80.0,
                                                           [] (auto rangeStart, auto rangeEnd, auto normalised)
                                                                { return jmap (normalised, rangeEnd, rangeStart); },
                                                           [] (auto rangeStart, auto rangeEnd, auto value)
                                                                { return jmap (value, rangeEnd, rangeStart, 0.0, 1.0); });
            
            auto eqComponent = new StackedSliderComponent (bands.size(), rangeReversed, 0.0, Slider::SliderStyle::LinearVertical, Slider::TextBoxBelow, 0);
            eqComponents.add (eqComponent);
            addAndMakeVisible (eqComponent);
            
            for (int i = 0; i < bands.size(); i++)
            {
                auto* label = eqComponent->getLabels()[i];
                label->setJustificationType (Justification::centred);
                label->setText (abbreviatedFrequency (bands[i]), dontSendNotification);
            }
            
            eqComponent->onSliderValueChange = [this, ch] (Slider*, int index, double value) {
                listeners.call ([ch, index, value] (Listener& l) {
                    l.audiogramSliderValueChanged (ch, index, value);
                });
            };
        }
    }

    ~AudiogramComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);
        
        int x  = eqComponents.getLast()->getX();
        int y1 = eqComponents.getLast()->getY() + 16;
        int y2 = eqComponents.getLast()->getBottom() - 34;
        g.drawLine (x, y1, x, y2);
        g.drawLine (x-1, y1, x-1, y2);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);
        
        using Track = Grid::TrackInfo;
        
        Grid headingGrid;
        headingGrid.templateRows    = { Track (22_px) };
        headingGrid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr), Track (1_fr) };
        headingGrid.items = {
            GridItem (headingLabel),
            GridItem (presetSelectors[0]),
            GridItem (linkToggle).withWidth (60.0f).withJustifySelf (GridItem::JustifySelf::center),
            GridItem (presetSelectors[1]),
            GridItem (bypassToggle).withWidth (70.0f).withJustifySelf (GridItem::JustifySelf::end)
        };
        headingGrid.performLayout (bounds.removeFromTop (22));

        eqComponents[0]->setBounds (bounds.removeFromLeft (bounds.proportionOfWidth (0.5f)));
        eqComponents[1]->setBounds (bounds);
    }
    
    void setSliderValue (int channel, int band, double value)
    {
        eqComponents[channel]->getSliders()[band]->setValue (value, dontSendNotification);
    }
    
    void addListener (Listener *listener) { listeners.add (listener); };
    
    void removeListener (Listener *listener) { listeners.remove (listener); }

    void setChannelEnabled (int channel, bool enabled)
    {
        presetSelectors[channel].setEnabled (enabled);
        eqComponents[channel]->setEnabled (enabled);
    }
    
    //==============================================================================
    ToggleButton    bypassToggle;
    ToggleButton    linkToggle;
    ComboBox        presetSelectors[2];
    
private:
    //==============================================================================
    void comboBoxChanged (ComboBox* comboBoxThatHasChanged) override
    {
        int channel = (int)(comboBoxThatHasChanged == &presetSelectors[1]);
        
        listeners.call ([channel, comboBoxThatHasChanged] (Listener& l) {
            l.audiogramPresetSelected (channel, comboBoxThatHasChanged->getSelectedItemIndex());
        });
    }
    
    //==============================================================================
    Label           headingLabel;
    OwnedArray<StackedSliderComponent> eqComponents;
    ListenerList<Listener> listeners;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudiogramComponent)
};
