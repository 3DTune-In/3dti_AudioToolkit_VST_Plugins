/**
 * \class FrequencySmearingComponent
 *
 * \brief Declaration of FrequencySmearingComponent interface.
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
class FrequencySmearingComponent : public Component, public Button::Listener, public ComboBox::Listener
{
public:
    FrequencySmearingComponent (HLSPluginAudioProcessor& p);
    
    ~FrequencySmearingComponent() { channelComponents.clear(); }

    void paint (Graphics& g) override;

    void resized() override;
    
    void buttonClicked (Button* button) override;
    
    void comboBoxChanged (ComboBox* comboBox) override;
    
    void updateGUIState()
    {
        bool linked = frequencySmearProcessor.freqSmearLink->get();
        
        linkToggle.setToggleState (linked, dontSendNotification);
        
        for (int i = 0; i < 2; i++)
        {
            int channel = (! linked) * i;
            bool mainEnabled = mainProcessor.enableSimulation[i]->get();
            bool freqSmearEnabled = frequencySmearProcessor.freqSmearEnabled[channel]->get();
            
            auto* component = channelComponents[i];
            component->setEnabled (freqSmearEnabled
                                && mainEnabled);
            component->onToggle.setEnabled (mainEnabled);
            component->onToggle.setToggleState (freqSmearEnabled, dontSendNotification);
            
            int mode = frequencySmearProcessor.freqSmearType[channel]->get();
            int selectedPreset = selectedPresets().frequencySmearPresets[channel][mode];
            component->presetSelector.setSelectedItemIndex (selectedPreset, dontSendNotification);
            
            // TODO: Get current parameters from FrequencySmearProcessor
            juce::Array<AudioParameterFloat*> params;
            if (mode == FrequencySmearingProcessor::BaerMoore)
            {
                params.add (frequencySmearProcessor.freqSmearSpectralBroadFactorUp[channel]);
                params.add (frequencySmearProcessor.freqSmearSpectralBroadFactorDown[channel]);
            }
            else
            {
                params.add (frequencySmearProcessor.freqSmearSpectralFrequencyDown[channel]);
                params.add (frequencySmearProcessor.freqSmearSpectralFrequencyUp[channel]);
                params.add (frequencySmearProcessor.freqSmearSpectralBufferSizeDown[channel]);
                params.add (frequencySmearProcessor.freqSmearSpectralBufferSizeUp[channel]);
            }
            component->updateGUIState (mode, params);
        }
    }

private:
    HLSPluginAudioProcessor& mainProcessor;
    FrequencySmearingProcessor& frequencySmearProcessor;
    
    //==============================================================================
    struct FrequencySmearParameterComponent : public Component
    {
        FrequencySmearParameterComponent (juce::Array<AudioParameterFloat*> p) : parameters (p) {}
        
        void loadPreset (std::vector<float> preset)
        {
            for (int i = 0; i < preset.size(); i++)
                *parameters[i] = preset[i];
        }
        
        virtual const std::vector<std::vector<float>> presets() { return {{0.0f}}; };
        
        virtual void updateGUIState(juce::Array<AudioParameterFloat*> params) {};
    
        juce::Array<AudioParameterFloat*> parameters;
    };
    
    struct BaerAndMooreComponent : public FrequencySmearParameterComponent, public Slider::Listener
    {
        BaerAndMooreComponent (juce::Array<AudioParameterFloat*> parameters);
        
        void resized() override;
        
        void sliderValueChanged (Slider* slider) override
        {
            int index = slider == &downwardSlider;
            *parameters[index] = slider->getValue();
        }
        
        const std::vector<std::vector<float>> presets() override {
            return {{1.01f, 1.01f},
                    {1.6f, 1.1f},
                    {2.4f, 1.6f},
                    {4.0f, 2.0f}};
        };
        
        void updateGUIState (juce::Array<AudioParameterFloat*> p) override
        {
            upwardSlider.setValue (p[0]->get(), dontSendNotification);
            downwardSlider.setValue (p[1]->get(), dontSendNotification);
        }
        
        Label  spectralBroadeningLabel;
        Label  upwardLabel;
        Slider upwardSlider;
        Label  downwardLabel;
        Slider downwardSlider;
    };
    
    struct Graf3DTIComponent : public FrequencySmearParameterComponent, public Slider::Listener
    {
        Graf3DTIComponent (juce::Array<AudioParameterFloat*> p);
        
        void resized() override;
        
        void sliderValueChanged (Slider* slider) override
        {
            int index = sliders.indexOf (slider);
            *parameters[index] = slider->getValue();
        }
        
        const std::vector<std::vector<float>> presets() override {
            return {{0.0f, 0.0f, 15.0f, 15.0f},
                    {35.0f, 35.0f, 15.0f, 15.0f},
                    {150.f, 150.0f, 100.0f, 100.0f},
                    {650.f, 650.0f, 150.0f, 150.0f}};
        };
        
        void updateGUIState(juce::Array<AudioParameterFloat*> p) override
        {
            for (int i = 0; i < sliders.size(); i++)
                sliders[i]->setValue (p[i]->get(), dontSendNotification);
        }
        
        Label  smearingLabel;
        Label  smearingDownwardLabel;
        Slider smearingDownwardSlider;
        Label  smearingUpwardLabel;
        Slider smearingUpwardSlider;
        
        Label  bufferSizeLabel;
        Label  bufferSizeDownwardLabel;
        Slider bufferSizeDownwardSlider;
        Label  bufferSizeUpwardLabel;
        Slider bufferSizeUpwardSlider;
        
        juce::Array<Slider*> sliders;
    };
    
    //==============================================================================
    struct Channel : public Component, public ComboBox::Listener
    {
        Channel (int index, FrequencySmearingProcessor& processor);
        
        void resized() override;
        
        void comboBoxChanged (ComboBox* comboBox) override;
        
        void setEnabled (bool enabled);
        
        void updateGUIState (int smearType, juce::Array<AudioParameterFloat*> params)
        {
            int selectedIndex = modeSelector.getSelectedItemIndex();
            if (smearType != selectedIndex
             || smearType != (dynamic_cast<Graf3DTIComponent*>(currentComponent) != nullptr))
            {
                if (smearType == 0)
                    setCurrentComponent (&baerAndMooreComponent);
                else
                    setCurrentComponent (&graf3dtiComponent);
                
                modeSelector.setSelectedItemIndex (smearType, dontSendNotification);
            }
            
            if (currentComponent)
                currentComponent->updateGUIState (params);
        }
        
        void setCurrentComponent (FrequencySmearParameterComponent* component)
        {
            if (currentComponent != nullptr)
                currentComponent->setVisible (false);
            
            currentComponent = component;
            
            component->setVisible (true);
            
            resized();
        }
        
        ToggleButton    onToggle;
        ComboBox        modeSelector;
        ComboBox        presetSelector;
        
        BaerAndMooreComponent baerAndMooreComponent;
        Graf3DTIComponent     graf3dtiComponent;
        
        FrequencySmearParameterComponent* currentComponent;
    };
    
    Label         headingLabel;
    ToggleButton  linkToggle;
    OwnedArray<Channel> channelComponents;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencySmearingComponent)
};
