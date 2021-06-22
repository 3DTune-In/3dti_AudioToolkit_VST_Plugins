/**
* \class ElevationDial
*
* \brief Declaration of ElevationDial interface.
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

//==============================================================================
/*
*/

class ElevationDial : public Slider {
public:
  
  // TODO(Ragnar): Better implementation of this
  bool hitTest (int x, int y) override {
    // const int thumbHeight = getLookAndFeel().getSliderThumbRadius(*this);
    const int thumbCentre = valueToProportionOfLength (getValue()) * getHeight();
    // DBG("Thumb centre: " + String(thumbCentre));
    // return (std::abs(y - thumbCentre) < thumbHeight);
    bool isAtEdges = (thumbCentre < 25 || thumbCentre > 95);
    int cutoff = isAtEdges ? 50 : 65;
    return x > cutoff;
  }

};
