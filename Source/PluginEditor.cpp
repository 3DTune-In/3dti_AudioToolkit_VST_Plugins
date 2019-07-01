/**
* \class Toolkit3dtiPluginAudioProcessorEditor
*
* \brief Declaration of Toolkit3dtiPluginAudioProcessorEditor interface.
* \date  June 2019
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
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Toolkit3dtiPluginAudioProcessorEditor::Toolkit3dtiPluginAudioProcessorEditor (Toolkit3dtiPluginAudioProcessor& p)
    : AudioProcessorEditor(&p),
      processor(p),
      sourceControls(p),
      reverbControls(p),
      anechoicControls(p),
      spatializerWidget(p.getCore())
{
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  setSize (1024, 810);
  
  addAndMakeVisible( sourceControls );
  addAndMakeVisible( reverbControls );
  addAndMakeVisible( anechoicControls );
  addAndMakeVisible( spatializerWidget );
  addAndMakeVisible( aboutText );
  addAndMakeVisible( aboutButton );
  addAndMakeVisible( toolkitVersionLabel );
  
  _3dTuneInLogo = ImageCache::getFromMemory(BinaryData::_3DTI_Logo_png, BinaryData::_3DTI_Logo_pngSize);
  imperialLogo = ImageCache::getFromMemory(BinaryData::Imperial_Logo_png, BinaryData::Imperial_Logo_pngSize);
  umaLogo = ImageCache::getFromMemory(BinaryData::UMA_Logo_png, BinaryData::UMA_Logo_pngSize);
  
  aboutText.setMultiLine(true);
  aboutText.setFont(Font(16.0f, Font::plain));
  aboutText.setText(String::fromUTF8(BinaryData::About_txt, BinaryData::About_txtSize));
  aboutText.setReadOnly(true);
  aboutText.setAlpha(0.9f);
  aboutText.setVisible(false);
  
  aboutButton.setButtonText("About");
  aboutButton.onClick = [this] { aboutText.setVisible(!aboutText.isVisible()); };
  addAndMakeVisible( aboutButton );
  
  toolkitVersionLabel.setFont(Font(15.f, Font::plain));
  toolkitVersionLabel.setText("Version " +  String(JucePlugin_VersionString) + " (3DTI Toolkit v1.3)", dontSendNotification);

  startTimer(30);
}

Toolkit3dtiPluginAudioProcessorEditor::~Toolkit3dtiPluginAudioProcessorEditor() {
  stopTimer();
}

//==============================================================================
void Toolkit3dtiPluginAudioProcessorEditor::paint (Graphics& g) {
  g.fillAll(Colour(24,31,34));
  
  auto aboutRect = Rectangle<int>(10, 10, (getLocalBounds().getWidth()/3)-20, 45);
  g.setColour(getLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
  g.fillRect(aboutRect);
  g.setColour (Colours::grey);
  g.drawRect(aboutRect);
  
  g.drawImageAt(_3dTuneInLogo, anechoicControls.getRight() + 14, 20);
  g.drawImageAt(imperialLogo, anechoicControls.getRight() + 246, 27);
  g.drawImageAt(umaLogo, anechoicControls.getRight() + 474, 15);
}

void Toolkit3dtiPluginAudioProcessorEditor::resized() {
  auto area = getLocalBounds();
  
  aboutButton.setBounds(22, 20, 50, 24);
  toolkitVersionLabel.setBounds(aboutButton.getRight()+6, aboutButton.getY(), area.getWidth()-aboutButton.getWidth()-6, 24);
  
  anechoicControls.setBounds(10, 54, (area.getWidth()/3)-20, 252);
  reverbControls.setBounds(10, anechoicControls.getBottom()-1, anechoicControls.getWidth(), 184);
  sourceControls.setBounds(10, reverbControls.getBottom()-1, anechoicControls.getWidth(), 310);
  spatializerWidget.setBounds(anechoicControls.getRight() + 10, 99, area.getWidth() - anechoicControls.getRight() - 20, area.getHeight() - 110);
  aboutText.setBounds(spatializerWidget.getBoundsInParent());
}
