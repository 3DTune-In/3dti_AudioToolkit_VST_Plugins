/**
 * \class AmbisonicEncoder
 *
 * \brief Declaration of AmbisonicEncoder interface.
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

#include <BinauralSpatializer/3DTI_BinauralSpatializer.h>
#include <JuceHeader.h>

using CSingleSourceRef = std::shared_ptr<Binaural::CSingleSourceDSP>;

//==============================================================================
class AmbisonicEncoder
{
public:
    //==========================================================================
    AmbisonicEncoder (Binaural::CCore& core);
    
    void processBlock (const std::vector<CSingleSourceRef>& sources, AudioBuffer<float>& quadOut);
    
private:
    //==========================================================================
    Binaural::CCore& mCore;
    Common::CDistanceAttenuator distanceAttenuatorReverb;
};
