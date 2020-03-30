/**
* \class SourceControls
*
* \brief Declaration of SourceControls interface.
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

#pragma once

#include "Utils.h"
#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
 */
class SourceControls : public Component, public Slider::Listener {
public:
  SourceControls(Toolkit3dtiPluginAudioProcessor& processor)
   :  mCore(processor.getCore()),
      azimuthLabel("Azimuth Label", "Azimuth"),
      distanceLabel("Distance Label", "Distance"),
      elevationLabel("Elevation Label", "Elevation"),
      xLabel("X Label", "X"),
      yLabel("Y Label", "Y"),
      zLabel("Z Label", "Z")
  {
    setLabelStyle( azimuthLabel );
    azimuthLabel.setJustificationType( Justification::left );
    addAndMakeVisible( azimuthLabel );
    
    setLabelStyle( distanceLabel );
    distanceLabel.setJustificationType( Justification::left );
    addAndMakeVisible( distanceLabel );
    
    setLabelStyle( elevationLabel );
    elevationLabel.setJustificationType( Justification::left );
    addAndMakeVisible( elevationLabel );
    
    setLabelStyle( xLabel );
    xLabel.setJustificationType( Justification::left );
    addAndMakeVisible( xLabel );
    
    setLabelStyle( yLabel );
    yLabel.setJustificationType( Justification::left );
    addAndMakeVisible( yLabel );
    
    setLabelStyle( zLabel );
    zLabel.setJustificationType( Justification::left );
    addAndMakeVisible( zLabel );
    
    azimuthSlider.setRange( 0.0f, 359.99f, 2 );
    azimuthSlider.setTextValueSuffix(" deg");
    azimuthSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
    azimuthSlider.addListener( this );
    addAndMakeVisible( azimuthSlider );
    
    elevationSlider.setRange( -89, 89, 1 );
    elevationSlider.setTextValueSuffix(" deg");
    elevationSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
    elevationSlider.addListener( this );
    addAndMakeVisible( elevationSlider );
    
    distanceSlider.setRange( mCore.getHeadRadius()+0.01f, 40.f, 0.01 );
    distanceSlider.setTextValueSuffix(" m");
    distanceSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
    distanceSlider.addListener( this );
    addAndMakeVisible( distanceSlider );
    
    xSlider.setRange( -40.0, 40.0, 0.01 );
    xSlider.setTextValueSuffix(" m");
    xSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
    xSlider.addListener( this );
    addAndMakeVisible( xSlider );
    
    ySlider.setRange( -40.0, 40.0, 0.01 );
    ySlider.setTextValueSuffix(" m");
    ySlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
    ySlider.addListener( this );
    addAndMakeVisible( ySlider );
    
    zSlider.setRange( -40.0, 40.0, 0.01 );
    zSlider.setTextValueSuffix(" m");
    zSlider.setTextBoxStyle( Slider::TextBoxRight, false, 65, 24 );
    zSlider.addListener( this );
    addAndMakeVisible( zSlider );
  }
  
  ~SourceControls()
  {
  }
  
  void updateGui() {
    auto position = mCore.getSourcePosition();
    distanceSlider.setValue(position.GetDistance(), dontSendNotification);
    azimuthSlider.setValue(position.GetAzimuthDegrees(), dontSendNotification);
    elevationSlider.setValue(mapElevationToSliderValue(position.GetElevationDegrees()), dontSendNotification);
    xSlider.setValue( position.x, dontSendNotification );
    ySlider.setValue( position.y, dontSendNotification );
    zSlider.setValue( position.z, dontSendNotification );
  }

  void paint (Graphics& g) override {
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.setColour (Colours::grey);
    g.drawRect (getLocalBounds(), 1);
  }
  
  void resized() override {
    auto area = getLocalBounds();
    azimuthLabel.setBounds( 10, 10, 80, 24 );
    azimuthSlider.setBounds( 6, azimuthLabel.getBottom(), area.getWidth()-18, 24 );
    elevationLabel.setBounds( 10, azimuthSlider.getBottom(), 80, 24 );
    elevationSlider.setBounds( 6, elevationLabel.getBottom(), area.getWidth()-18, 24 );
    distanceLabel.setBounds( 10, elevationSlider.getBottom(), 80, 24 );
    distanceSlider.setBounds( 6, distanceLabel.getBottom(), area.getWidth()-18, 24 );
    xLabel.setBounds( 10, distanceSlider.getBottom(), 80, 24 );
    xSlider.setBounds( 6, xLabel.getBottom(), area.getWidth()-18, 24 );
    yLabel.setBounds( 10, xSlider.getBottom(), 80, 24 );
    ySlider.setBounds( 6, yLabel.getBottom(), area.getWidth()-18, 24 );
    zLabel.setBounds( 10, ySlider.getBottom(), 80, 24 );
    zSlider.setBounds( 6, zLabel.getBottom(), area.getWidth()-18, 24 );
  }
  
  void sliderValueChanged( Slider* slider ) override {
    auto position = mCore.getSourcePosition();
    
    if ( slider == &azimuthSlider ) {
      position.SetFromAED(slider->getValue(), position.GetElevationDegrees(), position.GetDistance());
    } else if ( slider == &elevationSlider ) {
      auto value = slider->getValue();
      position.SetFromAED(position.GetAzimuthDegrees(), mapSliderValueToElevation(value), position.GetDistance());
    } else if ( slider == &distanceSlider ) {
      position.SetFromAED(position.GetAzimuthDegrees(), position.GetElevationDegrees(), slider->getValue());
    } else if ( slider == &xSlider ) {
      position.x = slider->getValue();
    } else if ( slider == &ySlider ) {
      position.y = slider->getValue();
    } else if ( slider == &zSlider ) {
      position.z = slider->getValue();
    }
    
    mCore.setSourcePosition(position);
    
    repaint();
    updateGui();
  }
  
private:
  Toolkit3dtiProcessor& mCore;
  
  Label  azimuthLabel;
  Label  distanceLabel;
  Label  elevationLabel;
  Label  xLabel;
  Label  yLabel;
  Label  zLabel;
  
  Slider azimuthSlider;
  Slider distanceSlider;
  Slider elevationSlider;
  Slider xSlider;
  Slider ySlider;
  Slider zSlider;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SourceControls)
};
