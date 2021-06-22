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

#include "FrequencySmearingComponent.h"

void setupSlider (Slider& slider, float min, float max, int decimalPlaces, Slider::Listener* listener)
{
    slider.setNormalisableRange (NormalisableRange<double> (min, max));
    slider.setNumDecimalPlacesToDisplay (decimalPlaces);
    slider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    slider.setTextBoxStyle (Slider::TextBoxLeft,
                            false,
                            40,
                            20);
    slider.addListener (listener);
}

//==============================================================================
FrequencySmearingComponent::BaerAndMooreComponent::BaerAndMooreComponent (juce::Array<AudioParameterFloat*> p)
  : FrequencySmearParameterComponent(p),
    spectralBroadeningLabel ("Spectral Broadening Factor", "Spectral Broadening Factor"),
    upwardLabel ("Upward", "Upward"),
    downwardLabel ("Downward", "Downward")
{
    jassert (p.size() == 2);
    
    spectralBroadeningLabel.setFont (Font (14));
    addAndMakeVisible (spectralBroadeningLabel);
    
    upwardLabel.setFont (Font (14));
    addAndMakeVisible (upwardLabel);
    
    setupSlider (upwardSlider, 1.01f, 8.0f, 2, this);
    addAndMakeVisible (upwardSlider);
    
    downwardLabel.setFont (Font (14));
    addAndMakeVisible (downwardLabel);
    
    setupSlider (downwardSlider, 1.01f, 8.0f, 2, this);
    addAndMakeVisible (downwardSlider);
}

void FrequencySmearingComponent::BaerAndMooreComponent::resized()
{
    auto bounds = getLocalBounds();
    
    spectralBroadeningLabel.setBounds (bounds.removeFromTop (20));
    
    auto upwardBounds = bounds.removeFromTop (bounds.proportionOfHeight (0.5f));
    upwardLabel.setBounds (upwardBounds.removeFromLeft (upwardBounds.proportionOfWidth (0.3f)));
    upwardSlider.setBounds (upwardBounds);
    
    downwardLabel.setBounds (bounds.removeFromLeft (bounds.proportionOfWidth (0.3f)));
    downwardSlider.setBounds (bounds);
}

//==============================================================================
FrequencySmearingComponent::Graf3DTIComponent::Graf3DTIComponent (juce::Array<AudioParameterFloat*> p)
  : FrequencySmearParameterComponent (p),
    smearingLabel ("Smearing [Hz]", "Smearing [Hz]"),
    smearingDownwardLabel ("Downward", "Downward"),
    smearingUpwardLabel ("Upward", "Upward"),
    bufferSizeLabel ("Buffer Size [samples]", "Buffer Size [samples]"),
    bufferSizeDownwardLabel ("Downward", "Downward"),
    bufferSizeUpwardLabel ("Upward", "Upward")
{
    addAndMakeVisible (smearingLabel);
    addAndMakeVisible (smearingDownwardLabel);
    addAndMakeVisible (smearingDownwardSlider);
    addAndMakeVisible (smearingUpwardLabel);
    addAndMakeVisible (smearingUpwardSlider);
    addAndMakeVisible (bufferSizeLabel);
    addAndMakeVisible (bufferSizeDownwardLabel);
    addAndMakeVisible (bufferSizeDownwardSlider);
    addAndMakeVisible (bufferSizeUpwardLabel);
    addAndMakeVisible (bufferSizeUpwardSlider);
    
    setupSlider (smearingDownwardSlider, 0, 1000, 0, this);
    sliders.add (&smearingDownwardSlider);
    
    setupSlider (smearingUpwardSlider, 0, 1000, 0, this);
    sliders.add (&smearingUpwardSlider);
    
    setupSlider (bufferSizeUpwardSlider, 1, 255, 0, this);
    sliders.add (&bufferSizeUpwardSlider);
    
    setupSlider (bufferSizeDownwardSlider, 1, 255, 0, this);
    sliders.add (&bufferSizeDownwardSlider);
    
    jassert (sliders.size() == parameters.size());
}

void FrequencySmearingComponent::Graf3DTIComponent::resized()
{
    auto bounds = getLocalBounds();
    auto componentHeight = (float)bounds.getHeight() / 6.0f;
        
    auto labelBounds = bounds.removeFromLeft (proportionOfWidth (0.4f));
    smearingLabel.setBounds (labelBounds.removeFromTop (componentHeight));
    smearingDownwardLabel.setBounds (labelBounds.removeFromTop (componentHeight));
    smearingUpwardLabel.setBounds (labelBounds.removeFromTop (componentHeight));
    bufferSizeLabel.setBounds (labelBounds.removeFromTop (componentHeight));
    bufferSizeDownwardLabel.setBounds (labelBounds.removeFromTop (componentHeight));
    bufferSizeUpwardLabel.setBounds (labelBounds);
    
    bounds.removeFromTop (componentHeight);
    smearingDownwardSlider.setBounds (bounds.removeFromTop (componentHeight));
    smearingUpwardSlider.setBounds (bounds.removeFromTop (componentHeight));
    bounds.removeFromTop (componentHeight);
    bufferSizeDownwardSlider.setBounds (bounds.removeFromTop (componentHeight));
    bufferSizeUpwardSlider.setBounds (bounds);
}

//==============================================================================
FrequencySmearingComponent::Channel::Channel (int index, FrequencySmearingProcessor& processor)
  : baerAndMooreComponent ({
      processor.freqSmearSpectralBroadFactorUp  [index],
      processor.freqSmearSpectralBroadFactorDown[index],
    }),
    graf3dtiComponent ({
        processor.freqSmearSpectralFrequencyDown[index],
        processor.freqSmearSpectralFrequencyUp[index],
        processor.freqSmearSpectralBufferSizeDown[index],
        processor.freqSmearSpectralBufferSizeUp[index],
    }),
    currentComponent (nullptr)
{
    modeSelector.setEditableText (false);
    modeSelector.setJustificationType (Justification::centred);
    modeSelector.addItem ("Baer & Moore", 1);
    modeSelector.addItem ("Graf & 3DTI", 2);
    modeSelector.setSelectedItemIndex (0, dontSendNotification);
    addAndMakeVisible (modeSelector);
    
    StringArray presets = {"None", "Mild", "Moderate", "Severe"};
    
    presetSelector.setEditableText (false);
    presetSelector.setJustificationType (Justification::left);
    presetSelector.addItemList (presets, 1);
    presetSelector.setSelectedItemIndex (0, dontSendNotification);
    presetSelector.addListener (this);
    addAndMakeVisible (presetSelector);
    
    onToggle.setButtonText ("On/Off");
    onToggle.setToggleState (false, dontSendNotification);
    addAndMakeVisible (onToggle);
    
    addChildComponent (graf3dtiComponent);
    addChildComponent (baerAndMooreComponent);
    setCurrentComponent (&baerAndMooreComponent);
}

void FrequencySmearingComponent::Channel::resized()
{
    auto bounds = getLocalBounds();
    auto headerBounds = bounds.removeFromTop (24);
    onToggle.setBounds (headerBounds.removeFromRight (80));
    modeSelector.setBounds (headerBounds.removeFromLeft (headerBounds.proportionOfWidth (0.55)).reduced (4, 0));
    presetSelector.setBounds (headerBounds.reduced (4, 2));
 
    bounds.removeFromTop (8);
    
    if (currentComponent)
        currentComponent->setBounds (bounds);
}

void FrequencySmearingComponent::Channel::comboBoxChanged (ComboBox* comboBox)
{
    currentComponent->loadPreset (currentComponent->presets()[comboBox->getSelectedItemIndex()]);
}

void FrequencySmearingComponent::Channel::setEnabled (bool enabled)
{
    onToggle.setToggleState (enabled, dontSendNotification);
    modeSelector.setEnabled (enabled);
    presetSelector.setEnabled (enabled);
    if (currentComponent)
        currentComponent->setEnabled (enabled);
}

//==============================================================================
FrequencySmearingComponent::FrequencySmearingComponent (HLSPluginAudioProcessor& p)
  : mainProcessor (p),
    frequencySmearProcessor (p.frequencySmearProcessor),
    headingLabel ("Frequency Smearing", "Frequency Smearing")
{
    headingLabel.setFont (Font (20));
    addAndMakeVisible (headingLabel);
    
    linkToggle.setButtonText ("Link");
    linkToggle.addListener (this);
    addAndMakeVisible (linkToggle);

    for (int i = 0; i < 2; i++)
    {
        auto* component = new Channel (i, frequencySmearProcessor);
        component->onToggle.addListener (this);
        
        component->presetSelector.addListener (this);
        component->modeSelector.addListener (this);
        
        addAndMakeVisible (component);
        channelComponents.add (component);
    }
}

void FrequencySmearingComponent::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);
}

void FrequencySmearingComponent::resized()
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
    
    // Spacer
    bounds.removeFromTop (8);
    
    auto subComponentWidth = bounds.proportionOfWidth (0.45f);
    channelComponents[0]->setBounds (bounds.removeFromLeft (subComponentWidth));
    channelComponents[1]->setBounds (bounds.removeFromRight (subComponentWidth));
}

void FrequencySmearingComponent::buttonClicked (Button* button)
{
    auto* parentComponent = button->getParentComponent();
    if (parentComponent != this)
    {
         auto channel = parentComponent == channelComponents[1];
         *frequencySmearProcessor.freqSmearEnabled[channel] = button->getToggleState();
    }
    
    if (button == &linkToggle)
        *frequencySmearProcessor.freqSmearLink = !frequencySmearProcessor.freqSmearLink->get();
}

void FrequencySmearingComponent::comboBoxChanged (ComboBox* comboBox)
{
    int index = comboBox->getSelectedItemIndex();
    
    auto* parentComponent = comboBox->getParentComponent();
    if (parentComponent != this)
    {
        auto channel =   channelComponents[1] == parentComponent;
        if (comboBox == &channelComponents[channel]->modeSelector)
        {
            *frequencySmearProcessor.freqSmearType[channel] = index;
        }
        else
        {
            int mode = channelComponents[channel]->modeSelector.getSelectedItemIndex();
            selectedPresets().frequencySmearPresets[channel][mode] = index;
        }
    }
}

