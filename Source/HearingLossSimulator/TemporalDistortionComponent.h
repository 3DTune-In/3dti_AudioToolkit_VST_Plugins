/**
 * \class TemporalDistortionComponent
 *
 * \brief Declaration of TemporalDistortionComponent interface.
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
#include "Presets.h"
#include "PluginProcessor.h"

//==============================================================================
/*
*/
class TemporalDistortionComponent : public Component, public Button::Listener, public Slider::Listener, public ComboBox::Listener
{
public:
    TemporalDistortionComponent (HLSPluginAudioProcessor& p)
      : processor (p),
        headingLabel ("Temporal Distortion", "Temporal Distortion"),
        leftRightSynchronicityLabel ("L/R Sync", "L/R Sync")
    {
        headingLabel.setFont (Font (20));
        addAndMakeVisible (headingLabel);
        
        for (int i = 0; i < 2; i++)
        {
            auto& component = jitterComponents[i];
            
            for (auto* slider : component.sliders)
                slider->addListener (this);
            
            component.onToggle.addListener (this);
            component.presetSelector.addListener (this);
            
            addAndMakeVisible (component);
        }
        
        linkToggle.setButtonText ("Link");
        linkToggle.addListener (this);
        addAndMakeVisible (linkToggle);
         
        leftRightSynchronicityLabel.setFont (Font (14));
        leftRightSynchronicityLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (leftRightSynchronicityLabel);
        
        leftRightSynchronicitySlider.setRange (0.0, 1.0);
        leftRightSynchronicitySlider.setNumDecimalPlacesToDisplay (1);
        leftRightSynchronicitySlider.setSliderStyle (Slider::SliderStyle::RotaryVerticalDrag);
        leftRightSynchronicitySlider.setTextBoxStyle (Slider::TextBoxBelow,
                                                      false,
                                                      34,
                                                      20);
        leftRightSynchronicitySlider.addListener (this);
        addAndMakeVisible (leftRightSynchronicitySlider);
    }

    ~TemporalDistortionComponent()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
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

        auto jitterComponentWidth = bounds.proportionOfWidth (0.45f);
        jitterComponents[0].setBounds (bounds.removeFromLeft (jitterComponentWidth));
        jitterComponents[1].setBounds (bounds.removeFromRight (jitterComponentWidth));
        
        auto syncBounds = bounds.withSizeKeepingCentre (bounds.getWidth(), bounds.proportionOfHeight (0.8f));
        leftRightSynchronicityLabel.setBounds (syncBounds.removeFromBottom (22));
        leftRightSynchronicitySlider.setBounds (syncBounds);
    }
    
    void updateGUIState()
    {
        bool linked = processor.temporalDistortionLink->get();
        
        linkToggle.setToggleState (linked, dontSendNotification);
        
        for (int i = 0; i < 2; i++)
        {
            auto& component = jitterComponents[i];
            
            int channel = (! processor.temporalDistortionLink->get()) * i;
            
            bool mainEnabled = processor.enableSimulation[i]->get();
            bool temporalDistEnabled = processor.temporalDistortionEnabled[channel]->get();
            
            component.setEnabled (mainEnabled
                               && temporalDistEnabled);
            component.onToggle.setEnabled (mainEnabled);
            component.onToggle.setToggleState (temporalDistEnabled, dontSendNotification);
            
            component.sliders[0]->setValue ((double)processor.jitterBandLimit[channel]->get(), dontSendNotification);
            component.sliders[1]->setValue ((double)processor.jitterNoisePower[channel]->get(), dontSendNotification);
            component.sliders[2]->setValue ((double)processor.jitterNoiseAutocorrelationCutoff[channel]->get(), dontSendNotification);
            
            int selectedPreset = selectedPresets().temporalDistortionPresets[channel];
            component.presetSelector.setSelectedItemIndex (selectedPreset, dontSendNotification);
        }
        
        leftRightSynchronicitySlider.setValue ((double)processor.jitterLeftRightSynchronicity->get());
    }
    
    void buttonClicked (Button* button) override
    {
        auto* parentComponent = button->getParentComponent();
        if (parentComponent != this)
        {
            auto channel = parentComponent == &jitterComponents[1];
            *processor.temporalDistortionEnabled[channel] = button->getToggleState();
        }
        else
            *processor.temporalDistortionLink = !processor.temporalDistortionLink->get();
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &leftRightSynchronicitySlider)
            *processor.jitterLeftRightSynchronicity = (float)slider->getValue();
        else
        {
            auto* parentComponent = slider->getParentComponent();
            if (parentComponent != this)
            {
                auto channel = parentComponent == &jitterComponents[1];
                
                switch (jitterComponents[channel].sliders.indexOf (slider))
                {
                    case 0:
                        *processor.jitterBandLimit[channel] = slider->getValue();
                        break;
                    case 1:
                        *processor.jitterNoisePower[channel] = slider->getValue();
                        break;
                    case 2:
                        *processor.jitterNoiseAutocorrelationCutoff[channel] = slider->getValue();
                        break;
                    default: break;
                }
            }
        }
    }

private:
    
    void comboBoxChanged (ComboBox* comboBox) override
    {
        auto* parentComponent = comboBox->getParentComponent();
        int channel = (int)(parentComponent == &jitterComponents[1]);
        int selectedIndex = comboBox->getSelectedItemIndex();
        switch (selectedIndex)
        {
            case 0:
                *processor.jitterBandLimit[channel] = 0;
                *processor.jitterNoisePower[channel] = 0.0f;
                *processor.jitterNoiseAutocorrelationCutoff[channel] = 0;
                break;
            case 1:
                *processor.jitterBandLimit[channel] = 3;
                *processor.jitterNoisePower[channel] = 0.4f;
                *processor.jitterNoiseAutocorrelationCutoff[channel] = 700;
                break;
            case 2:
                *processor.jitterBandLimit[channel] = 4;
                *processor.jitterNoisePower[channel] = 0.8f;
                *processor.jitterNoiseAutocorrelationCutoff[channel] = 850;
                break;
            case 3:
                *processor.jitterBandLimit[channel] = 6;
                *processor.jitterNoisePower[channel] = 1.0f;
                *processor.jitterNoiseAutocorrelationCutoff[channel] = 1000;
                break;
            default:
                break;
        }
        
        selectedPresets().temporalDistortionPresets[channel] = selectedIndex;
    }
    
    struct JitterComponent : public Component
    {
        JitterComponent ()
          : headingLabel ("Jitter", "Jitter"),
            bandLimitLabel ("Band Limit", "Band Limit"),
            noisePowerLabel ("Noise Power", "Noise Power"),
            noiseAutocorrelationCutoffLabel ("Noise Autocorrelation Cutoff", "Noise Autocorrelation Cutoff")
        {
            headingLabel.setFont (Font (18));
            addAndMakeVisible (headingLabel);
            
            auto presets = {"None", "Mild", "Moderate", "Severe"};
            presetSelector.setEditableText (false);
            presetSelector.setJustificationType (Justification::left);
            presetSelector.addItemList (presets, 1);
            presetSelector.setSelectedItemIndex (0, dontSendNotification);
            addAndMakeVisible (presetSelector);
            
            onToggle.setButtonText ("On/Off");
            onToggle.setToggleState (false, dontSendNotification);
            addAndMakeVisible (onToggle);
            
            Slider* bandLimitSlider;
            addAndMakeVisible (bandLimitSlider = new Slider);
            bandLimitSlider->setNormalisableRange (NormalisableRange<double> (0.0, 6.0, 1.0));
            bandLimitSlider->setNumDecimalPlacesToDisplay (0);
            bandLimitSlider->setSliderStyle (Slider::SliderStyle::RotaryVerticalDrag);
            bandLimitSlider->setTextBoxStyle (Slider::TextBoxBelow,
                                              false,
                                              40,
                                              20);
            bandLimitSlider->textFromValueFunction = [] (double value) {
                static std::map<double, String> mappedLabels = {
                    {0.0, "200"},
                    {1.0, "400"},
                    {2.0, "800"},
                    {3.0, "1k6"},
                    {4.0, "3k2"},
                    {5.0, "6k4"},
                    {6.0, "12k8"},
                };
                return mappedLabels[value];
            };
            sliders.add (bandLimitSlider);
            
            Font labelFont (13);
            
            bandLimitLabel.setFont (labelFont);
            bandLimitLabel.setJustificationType (Justification::centred);
            addAndMakeVisible (bandLimitLabel);
            
            Slider* noisePowerSlider;
            addAndMakeVisible (noisePowerSlider = new Slider);
            noisePowerSlider->setNormalisableRange (NormalisableRange<double> (0.0, 1.0));
            noisePowerSlider->setNumDecimalPlacesToDisplay (1);
            noisePowerSlider->setSliderStyle (Slider::SliderStyle::RotaryVerticalDrag);
            noisePowerSlider->setTextBoxStyle (Slider::TextBoxBelow,
                                              false,
                                              40,
                                              20);
            sliders.add (noisePowerSlider);
            
            noisePowerLabel.setFont (labelFont);
            noisePowerLabel.setJustificationType (Justification::centred);
            addAndMakeVisible (noisePowerLabel);
            
            Slider* noiseAutocorrelationCutoffSlider;
            addAndMakeVisible (noiseAutocorrelationCutoffSlider = new Slider);
            noiseAutocorrelationCutoffSlider->setNormalisableRange (NormalisableRange<double> (0.0, 1000.0, 1.0));
            noiseAutocorrelationCutoffSlider->setNumDecimalPlacesToDisplay (0);
            noiseAutocorrelationCutoffSlider->setSliderStyle (Slider::SliderStyle::RotaryVerticalDrag);
            noiseAutocorrelationCutoffSlider->setTextBoxStyle (Slider::TextBoxBelow,
                                                               false,
                                                               40,
                                                               20);
            noiseAutocorrelationCutoffSlider->textFromValueFunction = [] (double value) {
                return value == 1000.0 ? "1k" : String (value, 0);
            };
            sliders.add (noiseAutocorrelationCutoffSlider);
            
            noiseAutocorrelationCutoffLabel.setFont (labelFont);
            noiseAutocorrelationCutoffLabel.setJustificationType (Justification::centred);
            addAndMakeVisible (noiseAutocorrelationCutoffLabel);
        }
        
        void paint (Graphics& g) override
        {
        }

        void resized() override
        {
            auto bounds = getLocalBounds().reduced (8);
            
            auto headingBounds = bounds.removeFromTop (24);
            headingLabel.setBounds (headingBounds.removeFromLeft (proportionOfWidth (0.25)));
            presetSelector.setBounds (headingBounds.removeFromLeft (headingBounds.proportionOfWidth (0.55)));
            onToggle.setBounds (headingBounds.removeFromRight (80));
            
            float width = (float)bounds.getWidth() / 3.0f;
            
            bounds.removeFromTop (4); // Spacer

            auto labelBounds = bounds.removeFromTop (40);
            bandLimitLabel.setBounds (labelBounds.removeFromLeft (width));
            noisePowerLabel.setBounds (labelBounds.removeFromLeft (width));
            noiseAutocorrelationCutoffLabel.setBounds (labelBounds);
            
            sliders[0]->setBounds (bounds.removeFromLeft (width));
            sliders[1]->setBounds (bounds.removeFromLeft (width));
            sliders[2]->setBounds (bounds);
        }
        
        void setEnabled (bool enabled)
        {
            onToggle.setToggleState (enabled, dontSendNotification);
            presetSelector.setEnabled (enabled);
            for (auto const& slider : sliders)
                slider->setEnabled (enabled);
        }
        
        Label           headingLabel;
        ComboBox        presetSelector;
        ToggleButton    onToggle;
        Label           bandLimitLabel;
        Label           noisePowerLabel;
        Label           noiseAutocorrelationCutoffLabel;
        
        OwnedArray<Slider> sliders;
    };
    
    HLSPluginAudioProcessor& processor;
    
    Label           headingLabel;
    ToggleButton    linkToggle;
    JitterComponent jitterComponents[2];
    Label           leftRightSynchronicityLabel;
    Slider          leftRightSynchronicitySlider;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporalDistortionComponent)
};
