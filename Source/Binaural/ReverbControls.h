/**
 * \class ReverbControls
 *
 * \brief Declaration of ReverbControls interface.
 * \date  November 2021
 *
 * \authors Reactify Music LLP: R. Hrafnkelsson ||
 * Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
 * \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
 *
 * \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
 * \b Website: http://3d-tune-in.eu/
 *
 * \b Copyright: University of Malaga and Imperial College London - 2019
 *
 * \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
 *
 * \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#pragma once

#include <JuceHeader.h>
#include "ReverbProcessor.h"
#include "Utils.h"

//==============================================================================
/*
 */

class ReverbControls  : public Component, public ChangeListener, public Slider::Listener
{
public:
    //==========================================================================
    /** Constructor */
    ReverbControls (ReverbProcessor& processor);
    
    /** Destructor */
    ~ReverbControls();
    
    //==========================================================================
    /** Component */
    void paint (Graphics& g) override;
    
    void resized() override;
    
    //==========================================================================
    void updateGui();
    
    //==========================================================================
    void sliderValueChanged (Slider* slider) override
    {
        if (slider == &gainSlider)
            mReverb.reverbLevel = (float)slider->getValue();
    }
    
    void brirMenuChanged()
    {
        mReverb.reverbBRIR = brirMenu.getSelectedItemIndex();
    }
    
    //==========================================================================
    /** ChangeListenerCallback from ReverbProcessor */
    void changeListenerCallback (ChangeBroadcaster* source) override;
    
    ToggleButton bypassToggle;
    
private:
    //==========================================================================
    void updateBypass();
    void updateBrirLabel();
    
    //==========================================================================
    ReverbProcessor& mReverb;
    
    //==========================================================================
    ComboBox brirMenu;
    Label gainLabel;
    Slider gainSlider;
    std::unique_ptr<FileChooser> fc;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ReverbControls)
};
