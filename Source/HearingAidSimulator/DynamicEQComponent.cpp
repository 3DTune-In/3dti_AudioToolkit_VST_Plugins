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


#include "DynamicEQComponent.h"

DynamicEQComponent::DynamicEQComponent (HASPluginAudioProcessor& p)
  : processor (p),
    headingLabel ("Dynamic Equaliser", "Dynamic Equaliser"),
    lpFilterLabel ("LPF", "LPF"),
    hpFilterLabel ("HPF", "HPF"),
    channelComponents ({
        new Channel (false),
        new Channel (true),
    })
{
    headingLabel.setFont (Font (20));
    addAndMakeVisible (headingLabel);
    
    linkToggle.setButtonText ("Link");
    linkToggle.setToggleState (true, dontSendNotification);
    linkToggle.addListener (this);
    addAndMakeVisible (linkToggle);
    
    addAndMakeVisible (lpFilterLabel);
    addAndMakeVisible (hpFilterLabel);
    
    lpfToggle.setButtonText ("");
    lpfToggle.setToggleState (true, dontSendNotification);
    lpfToggle.addListener (this);
    addAndMakeVisible (lpfToggle);
    
    lpFilterSlider.setRange (63.0f, 16000.0f);
    lpFilterSlider.setValue (0.0f);
    lpFilterSlider.setNumDecimalPlacesToDisplay (0);
    lpFilterSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    lpFilterSlider.setTextBoxStyle (Slider::TextBoxLeft,
                                    false,
                                    50,
                                    20);
    lpFilterSlider.setSkewFactor (0.5);
    lpFilterSlider.addListener (this);
    addAndMakeVisible (lpFilterSlider);
    
    hpfToggle.setButtonText ("");
    hpfToggle.setToggleState (true, dontSendNotification);
    hpfToggle.addListener (this);
    addAndMakeVisible (hpfToggle);
    
    hpFilterSlider.setRange (63.0f, 16000.0f);
    hpFilterSlider.setValue (0.0f);
    hpFilterSlider.setNumDecimalPlacesToDisplay (0);
    hpFilterSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    hpFilterSlider.setTextBoxStyle (Slider::TextBoxLeft,
                                    false,
                                    50,
                                    20);
    hpFilterSlider.setSkewFactor (0.5);
    hpFilterSlider.addListener (this);
    addAndMakeVisible (hpFilterSlider);
    
    auto bands = processor.getBandFrequenciesDynamicEQ();
    
    for (auto* channel : channelComponents)
    {
        addAndMakeVisible (channel);
        
        int index = 0;
        for (auto& label : channel->eqComponent.getLabels())
            label->setText (abbreviatedFrequency (bands[index++]), dontSendNotification);
        
        channel->thresholdSlider.addListener (this);
        channel->fig6Button.addListener (this);
        channel->attackSlider.addListener (this);
        channel->releaseSlider.addListener (this);
        channel->compressionPercentSlider.addListener (this);
        
        channel->eqComponent.onSliderValueChange = [this, channel] (Slider* slider, int index, double value) {
            auto ch = channelComponents.indexOf (channel);
            *(processor.dynamicEQBandGains[ch][index]->getUnchecked(0)) = value;
            *(processor.dynamicEQBandGains[ch][index]->getUnchecked(1)) = slider->getMaxValue();
            *(processor.dynamicEQBandGains[ch][index]->getUnchecked(2)) = slider->getMinValue();
        };
    }
}

DynamicEQComponent::Channel::Channel (bool mirrored)
  : mirrored (mirrored),
    eqComponent (BANDS_NUMBER, Range<double> (0.0, 65.0), 0.0, Slider::SliderStyle::ThreeValueVertical, Slider::TextBoxBelow, 0),
    thresholdLabel ("Threshold", "Threshold"),
    attackLabel ("Attack", "Attack"),
    releaseLabel ("Release", "Release"),
    compressionPercentLabel ("Compression (%)", "Compression (%)")
{
    fig6Button.setButtonText ("Fig6");
    addAndMakeVisible (fig6Button);
    
    // juce::Array<Colour> slidercolours = {Colours::green, Colours::red, getLookAndFeel().findColour (Slider::thumbColourId)};
    addAndMakeVisible (eqComponent);
    for (auto* s : eqComponent.getSliders())
        s->setPopupDisplayEnabled (true, true, this, -1);
    
    addAndMakeVisible (thresholdLabel);
    
    thresholdSlider.setSliderStyle (Slider::SliderStyle::ThreeValueVertical);
    thresholdSlider.setPopupDisplayEnabled (true, true, this, -1);
    auto rangeReversed = NormalisableRange<double> (0.0, 80.0,
                                                    [] (auto rangeStart, auto rangeEnd, auto normalised)
                                                         { return jmap (normalised, rangeEnd, rangeStart); },
                                                    [] (auto rangeStart, auto rangeEnd, auto value)
                                                         { return jmap (value, rangeEnd, rangeStart, 0.0, 1.0); });
    thresholdSlider.setNormalisableRange (rangeReversed);
    thresholdSlider.textFromValueFunction = [] (double value) {
        return String (value * -1.0, 1);
    };
    thresholdSlider.setMaxValue (60.0, dontSendNotification, true);
    thresholdSlider.setMinValue (20.0, dontSendNotification, true);
    thresholdSlider.setValue (40.0);
    thresholdSlider.setTextBoxStyle (Slider::TextBoxBelow,
                             false,
                             50,
                             20);
    addAndMakeVisible (thresholdSlider);
    
    auto justification = mirrored ? Justification::left : Justification::right;
    attackLabel.setJustificationType (justification);
    releaseLabel.setJustificationType (justification);
    compressionPercentLabel.setJustificationType (justification);
    addAndMakeVisible (attackLabel);
    addAndMakeVisible (releaseLabel);
    addAndMakeVisible (compressionPercentLabel);
    
    attackSlider.setRange (0.0f, 300.0f);
    attackSlider.setValue (0.0f);
    attackSlider.setNumDecimalPlacesToDisplay (0);
    attackSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    attackSlider.setTextBoxStyle (mirrored ? Slider::TextBoxLeft : Slider::TextBoxRight,
                                  false,
                                  50,
                                  20);
    addAndMakeVisible (attackSlider);
    
    releaseSlider.setRange (0.0f, 2000.0f);
    releaseSlider.setValue (1000.0f);
    releaseSlider.setNumDecimalPlacesToDisplay (0);
    releaseSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    releaseSlider.setTextBoxStyle (mirrored ? Slider::TextBoxLeft : Slider::TextBoxRight,
                                   false,
                                   50,
                                   20);
    addAndMakeVisible (releaseSlider);
    
    compressionPercentSlider.setRange (0.0f, 120.0f);
    compressionPercentSlider.setValue (100.0f);
    compressionPercentSlider.setNumDecimalPlacesToDisplay (0);
    compressionPercentSlider.setSliderStyle (Slider::SliderStyle::LinearHorizontal);
    compressionPercentSlider.setTextBoxStyle (mirrored ? Slider::TextBoxLeft : Slider::TextBoxRight,
                                   false,
                                   50,
                                   20);
    addAndMakeVisible (compressionPercentSlider);
}

void DynamicEQComponent::Channel::resized()
    {
        auto r = getLocalBounds();
        auto eqBounds = r.removeFromTop (r.proportionOfHeight (0.65));
        
        Rectangle<int> thresholdBounds;
        
        if (mirrored)
            thresholdBounds = eqBounds.removeFromLeft (eqBounds.proportionOfWidth (0.2f));
        else
            thresholdBounds = eqBounds.removeFromRight (eqBounds.proportionOfWidth (0.2f));

        thresholdLabel.setBounds (thresholdBounds.removeFromTop (30));
        thresholdSlider.setBounds (thresholdBounds);
        
        eqComponent.setBounds (eqBounds);
        
        r.removeFromTop (8);
        
        auto sectionHeight = r.proportionOfHeight (0.33f);
        if (mirrored)
        {
            fig6Button.setBounds (r.removeFromLeft (r.proportionOfWidth (0.2f)).reduced (8).withHeight (36));
            auto attackBounds = r.removeFromTop (sectionHeight);
            attackLabel.setBounds (attackBounds.removeFromLeft (attackBounds.proportionOfWidth (0.25f)));
            attackSlider.setBounds (attackBounds);
            auto releaseBounds = r.removeFromTop (sectionHeight);
            releaseLabel.setBounds (releaseBounds.removeFromLeft (releaseBounds.proportionOfWidth (0.25f)));
            releaseSlider.setBounds (releaseBounds);
            compressionPercentLabel.setBounds (r.removeFromLeft (r.proportionOfWidth (0.4f)));
            compressionPercentSlider.setBounds (r);
        }
        else
        {
            fig6Button.setBounds (r.removeFromRight (r.proportionOfWidth (0.2f)).reduced (8).withHeight (36));
            auto attackBounds = r.removeFromTop (sectionHeight);
            attackLabel.setBounds (attackBounds.removeFromRight (attackBounds.proportionOfWidth (0.25f)));
            attackSlider.setBounds (attackBounds);
            auto releaseBounds = r.removeFromTop (sectionHeight);
            releaseLabel.setBounds (releaseBounds.removeFromRight (releaseBounds.proportionOfWidth (0.25f)));
            releaseSlider.setBounds (releaseBounds);
            compressionPercentLabel.setBounds (r.removeFromRight (r.proportionOfWidth (0.4f)));
            compressionPercentSlider.setBounds (r);
        }
    }
