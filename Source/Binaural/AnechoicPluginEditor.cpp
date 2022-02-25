/**
* \class AnechoicPluginProcessorEditor
*
* \brief Declaration of AnechoicPluginProcessorEditor interface.
* \date  November 2021
*
* \authors Reactify Music LLP: R. Hrafnkelsson ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2021
*
* \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
*/

#include "Utils.h"
#include "AnechoicPluginProcessor.h"
#include "AnechoicPluginEditor.h"

//==============================================================================
AnechoicPluginProcessorEditor::AnechoicPluginProcessorEditor (AnechoicPluginProcessor& p)
  :  AudioProcessorEditor (&p)
  ,  processor (p)
  ,  sourceControls (p.getCore())
  ,  anechoicControls (p.getCore(), p.treeState)
  ,  reverbControls (p)
  ,  spatializerWidget (p.getCore())
{
    setOpaque (true);
    
    aboutText.setMultiLine (true);
    aboutText.setFont (Font(16.0f, Font::plain));
    aboutText.setText (String::fromUTF8(BinaryData::About_Anechoic_txt, BinaryData::About_Anechoic_txtSize));
    aboutText.setReadOnly (true);
    aboutText.setAlpha (0.9f);
    aboutText.setVisible (false);
    aboutText.setEnabled (false);
    aboutText.addMouseListener (this, true);

    toolkitVersionLabel.setFont (Font (15.f, Font::plain));
    toolkitVersionLabel.setText ("Version " +  String(JucePlugin_VersionString) + " (3DTI Toolkit v1.4)", dontSendNotification);

    aboutBanner.button.onClick = [this] { aboutText.setVisible(!aboutText.isVisible()); };

    addAndMakeVisible (aboutBanner);
    addAndMakeVisible (sourceControls);
    addAndMakeVisible (anechoicControls);
    addAndMakeVisible (reverbControls);
    addAndMakeVisible (spatializerWidget);
    addChildComponent (aboutText);

    setSize (900, 726);
    
    startTimerHz (30);
}

AnechoicPluginProcessorEditor::~AnechoicPluginProcessorEditor()
{
   stopTimer();
}

//==============================================================================
void AnechoicPluginProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colour (24,31,34));
}

void AnechoicPluginProcessorEditor::paintOverChildren (Graphics& g)
{
    g.setColour (Colours::grey);
    auto sourceBounds = anechoicControls.getBoundsInParent();
    auto reverbBounds = reverbControls.getBoundsInParent();
    sourceBounds.setBottom (reverbBounds.getY());
    g.drawRect (sourceBounds);
    g.drawRect (reverbBounds);
}

void AnechoicPluginProcessorEditor::resized()
{
    auto r = getLocalBounds();
  
    aboutBanner.setBounds (r.removeFromTop (50));
  
    auto controlsBounds = r.removeFromLeft (r.proportionOfWidth (0.33));
    anechoicControls.setBounds (controlsBounds.removeFromTop (252));
    sourceControls.setBounds (controlsBounds.removeFromTop (324));
    reverbControls.setBounds (controlsBounds);
    spatializerWidget.setBounds (r);

    aboutText.setBounds(r);
}
