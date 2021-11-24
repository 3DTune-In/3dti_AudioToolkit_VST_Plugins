/**
* \class SpatializerWidget
*
* \brief Declaration of SpatializerWidget interface.
* \date  June 2019
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

#pragma once

#include <JuceHeader.h>
#include "ElevationDial.h"

//==============================================================================
/*
 */

// The distance in meters at the edge of screen coordinates
static const float RANGE_METERS = 40.f;
static const int kMargins = 20;

class SpatializerWidget : public Component, public Slider::Listener {
public:
  SpatializerWidget(AnechoicProcessor& core) : mCore(core) {
    setOpaque (true);
    elevationDial.setSliderStyle( Slider::Rotary );
    elevationDial.setRange( -89, 89, 1 );
    elevationDial.setValue(0, dontSendNotification);
    elevationDial.setTextBoxStyle( Slider::NoTextBox, false, 60, 30 );
    elevationDial.addListener( this );
    addAndMakeVisible( elevationDial );
    elevationDial.setBounds(0, 200, 120, 120);
    elevationDial.setRotaryParameters(degreesToRadians(0.f), degreesToRadians(180.f), true);
  }
  
  ~SpatializerWidget() {}
  
  void paint (Graphics& g) override {
    g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));  // clear the background
    
    g.setColour(Colours::grey);
    
    auto localBounds = getLocalBounds().reduced (kMargins);
    auto centrePoint = localBounds.getCentre().toFloat();
    
    // Draw listener head and distance radii
    // Note(Ragnar): Add a scale factor pixels > meters
    for ( auto radius : { mCore.getHeadRadius(), 1.5f, 7.f, 20.f, RANGE_METERS } ) {
      auto x = scaledRange.convertFrom0to1(radius / RANGE_METERS) * 2.f; // * 2 for diameter
      
      auto rect = Rectangle<float>(0,0, x, x).withCentre(centrePoint);
      
      String distance;
      
      if ( radius < 1.f ) {
        g.setColour(Colours::slategrey);
        g.fillEllipse(rect);
        distance = String(radius, 2) + "m";
      } else {
        g.setColour(Colours::grey);
        g.drawEllipse(rect, 1);
        distance = String(radius, 0) + "m";
      }
      
      g.setColour(Colours::ghostwhite);
      
      if ( radius == RANGE_METERS )
        continue;
      
      g.drawFittedText(distance,
                       rect.toNearestIntEdges().withTrimmedLeft(4).withTrimmedBottom(16),
                       Justification::left,
                       1);
    }
    
    // Draw axes
    for ( auto radius : { 20.f } ) {
      auto x = scaledRange.convertFrom0to1(radius / RANGE_METERS) * 2.f; // * 2 for diameter
      
      auto rect = Rectangle<float>(0,0, x, x).withCentre(centrePoint);
    
      g.drawLine(rect.getX(), rect.getCentreY(), rect.getRight(), rect.getCentreY());
      g.drawLine(rect.getCentreX(), rect.getY(), rect.getCentreX(), rect.getBottom());
      
      rect = rect.withSizeKeepingCentre(x*0.707f, x*0.707f);
      g.setColour(Colours::grey);
      g.drawLine(rect.getX(), rect.getY(), rect.getRight(), rect.getBottom());
      g.drawLine(rect.getX(), rect.getBottom(), rect.getRight(), rect.getY());
    }
    
    auto position = mCore.getSourcePosition(0);
    
    auto drawPosition = position;
    drawPosition.SetFromAED(position.GetAzimuthDegrees(), 0, position.GetDistance());
    auto distance = scaledRange.convertFrom0to1(position.GetDistance() / RANGE_METERS);
    auto angle = 360.f - drawPosition.GetAzimuthDegrees();
    auto point = Point<float>().getPointOnCircumference(distance, degreesToRadians(angle));
    
    // Draw source
    g.setColour (Colours::blueviolet.brighter());
    auto sourceRect = Rectangle<float>(0,0,20,20);
    sourceRect.setCentre(point + centrePoint);
    g.fillEllipse(sourceRect);
  
    // Draw source coordinate info
    g.setColour (Colours::white);
    g.setFont(15.0f);
    Rectangle<int> positionRect(100, 20);
    positionRect.setCentre( sourceRect.getCentre().x, sourceRect.getCentre().y - 12 );
    g.drawFittedText("x: "  + String(position.x > 0.f ? "  " : "") + String(position.x, 2)
                  + "\ny: " + String(position.y > 0.f ? "  " : "") + String(position.y, 2)
                  + "\nz: " + String(position.z > 0.f ? "  " : "") + String(position.z, 2),
                     positionRect.withX(positionRect.getX() - 26),
                     Justification::left,
                     1);

    // Position elevation dial
    elevationDial.setCentrePosition(sourceRect.getCentre().toInt() + Point<int>(30,0));
  }
  
  void resized() override {
    auto height = getLocalBounds().reduced (kMargins).getHeight();
    scaledRange = NormalisableRange<float>(0, height * 0.5f, 1.f, 3.5f);
  }
  
  void mouseDown(const MouseEvent&) override {
    mouseIsDown = true;
  }
  
  void mouseUp(const MouseEvent&) override {
    mouseIsDown = false;
  }
  
  void mouseDrag(const MouseEvent& event) override {
    auto point = event.position-getCentref(*this);
    auto angle = Point<float>().getAngleToPoint(point);
    auto azimuth = 360.f - radiansToDegrees(angle);
    auto distance = point.getDistanceFromOrigin();
#if DEBUG
    if ( distance > scaledRange.end ) {
      // We get an assertion failure in scaledRange if outside of bounds
      return;
    }
#endif
    
    static constexpr auto minValue = 0.001f;
    auto maxValue = RANGE_METERS;
    auto value = scaledRange.convertTo0to1(distance) * maxValue;
    auto distanceScaled = jlimit<float>(minValue, maxValue, value);
    
    auto source = mCore.getSources().front();
    auto position = mCore.getSourcePosition(source);
    position.SetFromAED (azimuth, position.GetElevationDegrees(), distanceScaled);
    mCore.setSourcePosition(source, position);
    
    applyElevationRotation();
    updateGui();
  }
  
  void updateGui() {
    if ( !mouseIsDown ) {
      auto elevation = mCore.getSourcePosition(0).GetElevationDegrees();
      elevationDial.setValue(mapElevationToSliderValue(elevation) * -1.f, // Elevation slider range is swapped because
                             dontSendNotification);                       // it plays better with the rotary style
    }
    repaint();
  }
  
  //==============================================================================
  void sliderValueChanged( Slider* slider ) override {
    applyElevationRotation();
    updateGui();
  }
  
private:
  AnechoicProcessor& mCore;
  
  void applyElevationRotation() {                                              // Elevation slider range is swapped becauses
    auto degrees = mapSliderValueToElevation(elevationDial.getValue()) * -1.f; // it plays better with the rotary style
    auto position = mCore.getSourcePosition (mCore.getSources().front());
    position.SetFromAED(position.GetAzimuthDegrees(), degrees, position.GetDistance());
    
    auto source = mCore.getSources().front();
    mCore.setSourcePosition (source, position);
  }
  
  bool mouseIsDown;
  ElevationDial elevationDial;
  // Used to scale screen coordinates to spatialised position
  NormalisableRange<float> scaledRange;
  
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SpatializerWidget)
};
