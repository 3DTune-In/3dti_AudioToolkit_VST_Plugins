/**
 * \class DynamicEQComponent
 *
 * \brief Declaration of DynamicEQComponent interface.
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
#include "../Common/Utilities.h"
#include "../Common/StackedSliderComponent.h"

//==============================================================================
/*
*/

class MouseAwareSlider : public Slider
{
public:
    
    void mouseDown (const MouseEvent& e) override
    {
        mouseIsDown = true;
        Slider::mouseDown (e);
    }

    void mouseUp (const MouseEvent& e) override
    {
        mouseIsDown = false;
        Slider::mouseUp (e);
    }
 
    bool mouseIsDown{false};
};

//==============================================================================
class DynamicEQComponent : public Component, public Slider::Listener, public Button::Listener
{
    class Channel;
    class AttackReleaseComponent;
public:
    DynamicEQComponent (HASPluginAudioProcessor& p);

    ~DynamicEQComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.setColour (Colours::grey);
        g.drawRect (getLocalBounds(), 1);   // draw an outline around the component
        
        int x  = channelComponents[1]->getX();
        int y1 = channelComponents[1]->getY() + 32;
        int y2 = channelComponents[1]->getBottom() - 64;
        g.drawLine (x, y1, x, y2);
        g.drawLine (x-1, y1, x-1, y2);
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (8);
        
        using Track = Grid::TrackInfo;
        
        Grid headingGrid;
        headingGrid.templateRows    = { Track (22_px) };
        headingGrid.templateColumns = { Track (1_fr), Track (1_fr), Track (1_fr) };
        headingGrid.items = {
            GridItem (headingLabel),
            GridItem (linkToggle).withWidth (60.0f).withJustifySelf (GridItem::JustifySelf::center),
            GridItem (),
        };
        headingGrid.performLayout (bounds.removeFromTop (22));
        
        auto filterBounds = bounds.removeFromTop (50).reduced (0, 8);
        auto hpfBounds = filterBounds.removeFromLeft (filterBounds.proportionOfWidth (0.5f));
        hpfToggle.setBounds (hpfBounds.removeFromLeft (30));
        hpFilterLabel.setBounds (hpfBounds.removeFromLeft (50));
        hpFilterSlider.setBounds (hpfBounds);
        lpfToggle.setBounds (filterBounds.removeFromLeft (30));
        lpFilterLabel.setBounds (filterBounds.removeFromLeft (50));
        lpFilterSlider.setBounds (filterBounds);
        
        auto r = bounds.reduced (0, 0);
        channelComponents[0]->setBounds (r.removeFromLeft (r.proportionOfWidth (0.5f)));
        channelComponents[1]->setBounds (r);
    }
    
    void updateGUIState()
    {
        linkToggle.setToggleState (processor.dynamicEQLink->get(), dontSendNotification);
        
        bool lowPassEnabled = processor.dynamicEQLowPassEnabled->get();
        lpfToggle.setToggleState (lowPassEnabled, dontSendNotification);
        lpFilterSlider.setEnabled (lowPassEnabled);
        lpFilterSlider.setValue (processor.dynamicEQLowPassCutoff->get(), dontSendNotification);
        
        bool hiPassEnabled = processor.dynamicEQHiPassEnabled->get();
        hpfToggle.setToggleState (hiPassEnabled, dontSendNotification);
        hpFilterSlider.setEnabled (hiPassEnabled);
        hpFilterSlider.setValue (processor.dynamicEQHiPassCutoff->get(), dontSendNotification);

        for (int ch = 0; ch < channelComponents.size(); ch++)
        {
            int channel = processor.dynamicEQLink->get() ? 0 : ch;

            auto* component = channelComponents[ch];
            
            for (auto level = 0; level < 3; level++)
            {
                auto& thresholdSlider = component->thresholdSlider;
                
                if (! thresholdSlider.mouseIsDown)
                {
                    auto threshold = processor.dynamicEQThresholds[channel][level]->get() * -1.0f;
                    if (level == 0)
                        thresholdSlider.setValue (threshold, dontSendNotification);
                    else if (level == 1)
                        thresholdSlider.setMaxValue (threshold, dontSendNotification);
                    else
                        thresholdSlider.setMinValue (threshold, dontSendNotification);
                }
             
                auto& sliders = component->eqComponent.getSliders();
                
                for (int band = 0; band < sliders.size(); band++)
                {
                    if (sliders[band]->mouseIsDown)
                        continue;
                    
                    float value = processor.dynamicEQBandGains[channel][band]->getUnchecked (level)->get();
                    if (level == 0)
                        sliders[band]->setValue (value, dontSendNotification);
                    else if (level == 1)
                        sliders[band]->setMaxValue (value, dontSendNotification);
                    else
                        sliders[band]->setMinValue (value, dontSendNotification);
                }
            }
            
            component->attackSlider.setValue (processor.dynamicEQAttack[channel]->get(), dontSendNotification);
            component->releaseSlider.setValue (processor.dynamicEQRelease[channel]->get(), dontSendNotification);
            component->compressionPercentSlider.setValue (processor.dynamicEQCompressionPct[channel]->get(), dontSendNotification);
        }
    }
    
    void setChannelEnabled (int channel, bool enabled)
    {
        channelComponents[channel]->setEnabled (enabled);
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        auto* parent = slider->getParentComponent();
        if (parent != this)
        {
            int channel = (parent == channelComponents[1]);
            if (slider == &(channelComponents[channel]->thresholdSlider))
            {
                *processor.dynamicEQThresholds[channel].getUnchecked (0) = slider->getValue() * -1.f;
                *processor.dynamicEQThresholds[channel].getUnchecked (1) = slider->getMaxValue() * -1.f;
                *processor.dynamicEQThresholds[channel].getUnchecked (2) = slider->getMinValue() * -1.f;
            }
            else if (slider == &(channelComponents[channel]->attackSlider))
                *processor.dynamicEQAttack[channel] = slider->getValue();
            else if (slider == &(channelComponents[channel]->releaseSlider))
                *processor.dynamicEQRelease[channel] = slider->getValue();
            else if (slider == &(channelComponents[channel]->compressionPercentSlider))
                *processor.dynamicEQCompressionPct[channel] = slider->getValue();
        }
        else
        {
            if (slider == &lpFilterSlider)
                *processor.dynamicEQLowPassCutoff = (int)slider->getValue();
            else
                *processor.dynamicEQHiPassCutoff = (int)slider->getValue();
        }
    }
    
    void buttonClicked (Button* button) override
    {
        if (button == &linkToggle)
        {
            *processor.dynamicEQLink = !processor.dynamicEQLink->get();
        }
        else if (button == &hpfToggle)
        {
            *processor.dynamicEQHiPassEnabled = ! processor.dynamicEQHiPassEnabled->get();
        }
        else if (button == &lpfToggle)
        {
            *processor.dynamicEQLowPassEnabled = ! processor.dynamicEQLowPassEnabled->get();
        }
        else
        {
            auto* parent = button->getParentComponent();
            int channel = (parent == channelComponents[1]);
            if (button == &(channelComponents[channel]->fig6Button))
                *processor.enableFig6[channel] = true;
        }
    }

private:
    //==============================================================================
    struct Channel : public Component
    {
        Channel (bool mirrored = false);
        
        void resized() override;
        
        bool          mirrored;
        TextButton    fig6Button;
        
        StackedSliderComponentT<MouseAwareSlider> eqComponent;
        Label  thresholdLabel;
        MouseAwareSlider thresholdSlider;
        
        Label attackLabel, releaseLabel, compressionPercentLabel;
        Slider attackSlider, releaseSlider, compressionPercentSlider;
    };
    
    //==============================================================================
    HASPluginAudioProcessor& processor;
    
    Label headingLabel;
    ToggleButton linkToggle;
    ToggleButton lpfToggle, hpfToggle;
    Label lpFilterLabel, hpFilterLabel;
    Slider lpFilterSlider, hpFilterSlider;
    OwnedArray<Channel> channelComponents;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DynamicEQComponent)
};
