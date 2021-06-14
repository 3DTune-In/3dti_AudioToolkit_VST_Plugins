/**
* \class ReverbPluginProcessorEditor
*
* \brief Declaration of ReverbPluginProcessorEditor interface.
* \date  November 2020
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

#include "Utils.h"
#include "ReverbPluginProcessor.h"
#include "ReverbPluginEditor.h"

//==============================================================================
ReverbPluginProcessorEditor::ReverbPluginProcessorEditor (ReverbPluginProcessor& p)
    : AudioProcessorEditor(&p)
    , processor(p)
    , reverbControls (p.getReverbProcessor())
{
    setOpaque (true);
    
    aboutText.setMultiLine (true);
    aboutText.setFont (Font (16.0f, Font::plain));
    aboutText.setText (String::fromUTF8 (BinaryData::About_Reverb_txt, BinaryData::About_Reverb_txtSize));
    aboutText.setReadOnly (true);
    aboutText.setAlpha (0.9f);
    aboutText.setVisible (false);
    aboutText.addMouseListener (this, true);

    aboutBanner.button.onClick = [this] { aboutText.setVisible(!aboutText.isVisible()); };
    
    addAndMakeVisible (aboutBanner);
    addAndMakeVisible (reverbControls);
    addChildComponent (aboutText);
    
    setSize (600, 200);

    startTimer (30);
}

ReverbPluginProcessorEditor::~ReverbPluginProcessorEditor() {
  stopTimer();
}

//==============================================================================
void ReverbPluginProcessorEditor::paint (Graphics& g)
{
  g.fillAll (Colour (24,31,34));
}

void ReverbPluginProcessorEditor::resized()
{
    auto r = getLocalBounds();
  
    aboutBanner.setBounds (r.removeFromTop (50));
    reverbControls.setBounds (r);
    aboutText.setBounds(r);
}

//==============================================================================
void ReverbPluginProcessorEditor::timerCallback()
{
    reverbControls.updateGui();
}
