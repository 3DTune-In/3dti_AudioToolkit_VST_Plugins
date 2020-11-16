/**
* \class ReverbControls
*
* \brief Declaration of ReverbControls interface.
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

#include "ReverbControls.h"

ReverbControls::ReverbControls(Toolkit3dtiPluginAudioProcessor& p)
  : mProcessor(p),
    mReverb (p.getReverbProcessor()),
    gainLabel("Gain Label", "Gain (dB)"),
    distanceAttenuationLabel("Distance Label", "dB attenuation per double distance")
{
  std::vector<String> options = {"Small", "Medium", "Large", "Load 3DTI", "Load SOFA"};
  for ( int i = 0; i < options.size(); i++ ) {
    brirMenu.addItem( options[i], i+1 ); // IDs must be non-zero
  }
  brirMenu.onChange = [this] { brirMenuChanged(); };
  brirMenu.setSelectedId(1, dontSendNotification);
  addAndMakeVisible( brirMenu );
  
  setLabelStyle( gainLabel );
  gainLabel.setJustificationType( Justification::left );
  addAndMakeVisible( gainLabel );
  
  mapParameterToSlider( gainSlider, mReverb.reverbGain );
  gainSlider.setTextValueSuffix(" dB");
  gainSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
  gainSlider.addListener( this );
  addAndMakeVisible( gainSlider );
  
  distanceAttenuationToggle.setButtonText("On/Off");
  distanceAttenuationToggle.setToggleState(true, dontSendNotification);
  distanceAttenuationToggle.onClick = [this] { updateDistanceAttenuation(); };
  addAndMakeVisible( distanceAttenuationToggle );
  
  setLabelStyle( distanceAttenuationLabel );
  distanceAttenuationLabel.setJustificationType( Justification::left );
  addAndMakeVisible( distanceAttenuationLabel );
  
  mapParameterToSlider( distanceAttenuationSlider, mReverb.reverbDistanceAttenuation );
  distanceAttenuationSlider.setTextValueSuffix(" dB");
  distanceAttenuationSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
  distanceAttenuationSlider.addListener( this );
  addAndMakeVisible( distanceAttenuationSlider );
  
  bypassToggle.setButtonText("On/Off");
  bypassToggle.setToggleState(true, dontSendNotification);
  bypassToggle.onClick = [this] { updateBypass(); };
  addAndMakeVisible( bypassToggle );
  
  updateGui();
}

void ReverbControls::updateGui() {
  gainSlider.setValue(mReverb.reverbGain.get(), dontSendNotification);
  distanceAttenuationSlider.setValue(mReverb.reverbDistanceAttenuation, dontSendNotification );
  
  if ( !mProcessor.getSources().empty() ) {
    auto source = mProcessor.getSources().front();
    bypassToggle.setToggleState(source->IsReverbProcessEnabled(), dontSendNotification);
    bool distanceAttenuationEnabled = source->IsDistanceAttenuationEnabledReverb();
    distanceAttenuationToggle.setToggleState(distanceAttenuationEnabled, dontSendNotification);
    distanceAttenuationLabel.setEnabled(distanceAttenuationEnabled);
    distanceAttenuationSlider.setEnabled(distanceAttenuationEnabled);
  }
  
  auto brirIndex = mReverb.getBrirIndex();
  if ( brirIndex != brirMenu.getSelectedItemIndex() && brirIndex < BundledBRIRs.size()-2 ) { // Show filename if custom file is selected
    brirMenu.setSelectedItemIndex(brirIndex, dontSendNotification);
  }
}

void ReverbControls::loadCustomBRIR(String fileTypes) {
  fc.reset (new FileChooser ("Choose a file to open...",
                             BRIRDirectory(),
                             fileTypes,
                             true));
  
  fc->launchAsync(FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                  [this] (const FileChooser& chooser)
                  {
                    String chosen;
                    auto results = chooser.getURLResults();
                    
                    auto result = results.getFirst();
                    
                    chosen << (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                               : result.toString (false));
                    
                    mReverb.loadBRIR(File(chosen.removeCharacters("\n")));
                    
                    updateBrirLabel();
                  });
}

void ReverbControls::updateBypass() {
  bool enabled = bypassToggle.getToggleState();
  if ( enabled  ) {
    mProcessor.getSources().front()->EnableReverbProcess();
  } else {
    mProcessor.getSources().front()->DisableReverbProcess();
  }
  setAlpha( enabled + 0.4f );
}

void ReverbControls::updateBrirLabel() {
  auto brir = mReverb.getBrirPath().getFileNameWithoutExtension().upToLastOccurrenceOf("_", false, false);
  brirMenu.setText(brir, dontSendNotification);
}

void ReverbControls::updateDistanceAttenuation() {
  auto source = mProcessor.getSources().front();
  if ( distanceAttenuationToggle.getToggleState() ) {
    source->EnableDistanceAttenuationReverb();
  } else {
    source->DisableDistanceAttenuationReverb();
  }
}
