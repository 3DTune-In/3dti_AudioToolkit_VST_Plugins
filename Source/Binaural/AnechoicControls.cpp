/**
* \class AnechoicControls
*
* \brief Declaration of AnechoicControls interface.
* \date  February 2022
*
* \authors Reactify Music LLP: R. Hrafnkelsson ||
* Coordinated by , A. Reyes-Lecuona (University of Malaga) and L.Picinali (Imperial College London) ||
* \b Contact: areyes@uma.es and l.picinali@imperial.ac.uk
*
* \b Project: 3DTI (3D-games for TUNing and lEarnINg about hearing aids) ||
* \b Website: http://3d-tune-in.eu/
*
* \b Copyright: University of Malaga and Imperial College London - 2022
*
* \b Licence: This copy of the 3D Tune-In Toolkit Plugin is licensed to you under the terms described in the LICENSE.md file included in this distribution.
*
* \b Acknowledgement: This project has received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
*/

#include "Utils.h"
#include "AnechoicControls.h"

AnechoicControls::AnechoicControls (AnechoicProcessor& processor,
                                    AudioProcessorValueTreeState& params)
  : mCore (processor)
  , mParameters (params)
  , buttonAttachment (params, "HRTF", hrtfMenu)
  , distanceAttenuationLabel("Distance Label", "dB attenuation per double distance")
{
  setOpaque (true);
    
  hrtfMenu.addItemList (BundledHRTFs, 1);
  addAndMakeVisible (hrtfMenu);
  
  headCircumferenceToggle.setButtonText( "Custom Head Circumference" );
  headCircumferenceToggle.onClick = [this] { updateHeadCircumference(); };
  addAndMakeVisible(headCircumferenceToggle);
  
  mapParameterToSlider( headCircumferenceSlider, mCore.headCircumference );
  headCircumferenceSlider.setTextValueSuffix(" mm");
  headCircumferenceSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
  headCircumferenceSlider.addListener( this );
  headCircumferenceSlider.setValue( mCore.headCircumference, dontSendNotification );
  addAndMakeVisible( headCircumferenceSlider );
  
  headCircumferenceSlider.setEnabled( headCircumferenceToggle.getToggleState() );
  
  bypassToggle.setButtonText("On/Off");
  bypassToggle.setToggleState(true, dontSendNotification);
  bypassToggle.onClick = [this] { updateBypass(); };
  addAndMakeVisible( bypassToggle );
  
  nearFieldToggle.setButtonText( "Near field correction (IIR)" );
  nearFieldToggle.onClick = [this] { updateNearFieldCorrection(); };
  addAndMakeVisible( nearFieldToggle );
  
  farFieldToggle.setButtonText( "Far field correction (IIR)" );
  farFieldToggle.onClick = [this] { updateFarFieldCorrection(); };
  addAndMakeVisible( farFieldToggle );
  
  distanceAttenuationToggle.setButtonText("On/Off");
  distanceAttenuationToggle.setToggleState(true, dontSendNotification);
  distanceAttenuationToggle.onClick = [this] { updateDistanceAttenuation(); };
  addAndMakeVisible( distanceAttenuationToggle );
  
  qualityToggle.setButtonText( "High Quality" );
  qualityToggle.onClick = [this] { updateQualitySetting(); };
  // addAndMakeVisible( qualityToggle );
  
  setLabelStyle( distanceAttenuationLabel );
  distanceAttenuationLabel.setJustificationType (Justification::right);
  addAndMakeVisible( distanceAttenuationLabel );
  
  mapParameterToSlider( distanceAttenuationSlider, mCore.sourceDistanceAttenuation );
  distanceAttenuationSlider.setTextValueSuffix(" dB");
  distanceAttenuationSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
  distanceAttenuationSlider.addListener( this );
  addAndMakeVisible( distanceAttenuationSlider );
  
  updateGui();
  updateHrtfLabelText();
    
  mCore.addChangeListener (this);
}

AnechoicControls::~AnechoicControls()
{
    mCore.removeChangeListener (this);
}

void AnechoicControls::updateGui() {
  headCircumferenceToggle.setToggleState( mCore.enableCustomizedITD, dontSendNotification );
  headCircumferenceSlider.setValue( mCore.headCircumference, dontSendNotification );
  headCircumferenceSlider.setEnabled( headCircumferenceToggle.getToggleState() );
  
  if ( !mCore.getSources().empty() ) {
    auto source = mCore.getSources().front();
    bypassToggle.setToggleState(source->IsAnechoicProcessEnabled(), dontSendNotification);
    
    bool distanceAttenuationEnabled = source->IsDistanceAttenuationEnabledAnechoic();
    distanceAttenuationToggle.setToggleState(distanceAttenuationEnabled, dontSendNotification);
    distanceAttenuationSlider.setValue(mCore.sourceDistanceAttenuation, dontSendNotification);
    distanceAttenuationLabel.setEnabled(distanceAttenuationEnabled);
    distanceAttenuationSlider.setEnabled(distanceAttenuationEnabled);
  }
  
  nearFieldToggle.setToggleState( mCore.enableNearDistanceEffect, dontSendNotification);
  farFieldToggle.setToggleState( mCore.enableFarDistanceEffect, dontSendNotification);
  qualityToggle.setToggleState( mCore.spatializationMode, dontSendNotification);
}

void AnechoicControls::paint (Graphics& g)
{
  g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
  g.setColour (Colours::white);
  g.setFont (18.0f);
  g.drawText ("Anechoic Path", getLocalBounds().withTrimmedBottom( getLocalBounds().getHeight() - 32 ),
              Justification::centred, true);
}

void AnechoicControls::resized() {
  auto area = getLocalBounds();
  bypassToggle.setBounds( 10, 4, 80, 24 );
  hrtfMenu.setBounds( 12, 40, area.getWidth()-24, 22 );
  headCircumferenceToggle.setBounds( 10, hrtfMenu.getBottom() + 8, area.getWidth()-20, 26);
  headCircumferenceSlider.setBounds( 6, headCircumferenceToggle.getBottom(), area.getWidth()-18, 24);
  nearFieldToggle.setBounds(10, headCircumferenceSlider.getBottom(), area.getWidth()-20, 32);
  farFieldToggle.setBounds(10, nearFieldToggle.getBottom(), area.getWidth()-20, 32);
  // qualityToggle.setBounds(10, farFieldToggle.getBottom(), area.getWidth()-20, 30);
  distanceAttenuationToggle.setBounds( 10, farFieldToggle.getBottom() + 4, 80, 24);
  distanceAttenuationLabel.setBounds( 94, distanceAttenuationToggle.getY(), area.getWidth()-100, 24);
  distanceAttenuationSlider.setBounds( 6, distanceAttenuationToggle.getBottom() + 4, area.getWidth()-20, 24);
}

void AnechoicControls::sliderValueChanged (Slider* slider)
{
  if ( slider == &headCircumferenceSlider ) {
    updateHeadCircumference();
  } else {
    mCore.sourceDistanceAttenuation = (float)slider->getValue();
  }
}

void AnechoicControls::changeListenerCallback (ChangeBroadcaster *source)
{
    updateHrtfLabelText();
}

void AnechoicControls::updateBypass() {
  bool enabled = bypassToggle.getToggleState();
  if ( enabled  ) {
    mCore.getSources().front()->EnableAnechoicProcess();
  } else {
    mCore.getSources().front()->DisableAnechoicProcess();
  }
  setAlpha( enabled + 0.4f );
}

void AnechoicControls::updateHeadCircumference() {
  bool enabled = headCircumferenceToggle.getToggleState();
  mCore.enableCustomizedITD = enabled;
  mCore.headCircumference = headCircumferenceSlider.getValue();
  
  // TODO: Show circumference in mm
  // IM.slHeadCircumference.footer = "mm (r: " + Utils.FloatToString(r_cm, 1) + " cm)";
  headCircumferenceSlider.setEnabled(enabled);
}

void AnechoicControls::updateHrtfLabelText()
{
    auto* parameter = mParameters.getParameter ("HRTF");
    
    auto hrtfValue = parameter->getValue();
    int  hrtfIndex = std::floor (parameter->convertFrom0to1 (hrtfValue));

    if (hrtfIndex >= BundledHRTFs.size() - 2)
    {
        // Show filename if custom file is selected
        auto hrtf = mCore.getHrtfPath().getFileNameWithoutExtension().upToLastOccurrenceOf ("_", false, false);
        hrtfMenu.setText (hrtf, dontSendNotification);
    }
    else
    {
        hrtfMenu.setSelectedItemIndex (hrtfIndex, dontSendNotification);
    }
}

void AnechoicControls::updateNearFieldCorrection() {
  mCore.enableNearDistanceEffect = nearFieldToggle.getToggleState();
}

void AnechoicControls::updateFarFieldCorrection() {
  mCore.enableFarDistanceEffect = farFieldToggle.getToggleState();
}

void AnechoicControls::updateQualitySetting() {
  auto mode = qualityToggle.getToggleState() ? Binaural::HighQuality : Binaural::HighPerformance;
  mCore.spatializationMode = mode;
}

void AnechoicControls::updateDistanceAttenuation() {
  auto source = mCore.getSources().front();
  if ( distanceAttenuationToggle.getToggleState() ) {
    source->EnableDistanceAttenuationAnechoic();
  } else {
    source->DisableDistanceAttenuationAnechoic();
  }
}
