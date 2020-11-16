/**
 * \class StackedSliderComponent
 *
 * \brief Declaration of StackedSliderComponent interface.
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

//==============================================================================
/*
*/

template <class SliderT>
class StackedSliderComponentT : public Component, public Slider::Listener
{
public:
    StackedSliderComponentT (int numberOfSliders, NormalisableRange<double> range, float defaultValue, Slider::SliderStyle style = Slider::SliderStyle::LinearVertical, Slider::TextEntryBoxPosition textBoxPosition = Slider::TextBoxBelow, int decimalPlaces = 1)
      : vertical (true)
    {
        for (int i = 0; i < numberOfSliders; i++)
        {
            auto slider = new SliderT;
            slider->setNormalisableRange (range);
            slider->setValue (defaultValue);
            slider->setNumDecimalPlacesToDisplay (decimalPlaces);
            slider->setSliderStyle (style);
            slider->setTextBoxStyle (textBoxPosition,
                                     false,
                                     34,
                                     20);
            slider->addListener (this);
            sliders.add (slider);
            
            addAndMakeVisible (slider);
            
            auto* label = new Label ("100", "100");
            label->setJustificationType (Justification::centred);
            labels.add (label);
            addAndMakeVisible (label);
        }
    }

    ~StackedSliderComponentT()
    {
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        
        for (int i = sliders.size() - 1; i >= 0; i--)
        {
            if (vertical)
            {
                auto sliderBounds = i == 0 ? bounds : bounds.removeFromRight (proportionOfWidth (1.0f / sliders.size()));
                labels[i]->setBounds (sliderBounds.removeFromTop (30));
                sliders[i]->setBounds (sliderBounds);
            }
            else
                sliders[i]->setBounds (i == 0 ? bounds : bounds.removeFromBottom (proportionOfHeight (1.0f / sliders.size())));
        }
    }
    
    void sliderValueChanged (Slider* slider) override
    {
        if (onSliderValueChange)
            onSliderValueChange (slider, sliders.indexOf ((SliderT*)slider), slider->getValue());
    }
    
    std::function<void(Slider* slider, int index, double value)> onSliderValueChange;
    
    const OwnedArray<Label>&  getLabels()  const { return labels; }
    
    const OwnedArray<SliderT>& getSliders() const { return sliders; }

private:
    bool vertical = true;
    OwnedArray<Label> labels;
    OwnedArray<SliderT> sliders;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StackedSliderComponentT)
};

using StackedSliderComponent = StackedSliderComponentT<Slider>;
