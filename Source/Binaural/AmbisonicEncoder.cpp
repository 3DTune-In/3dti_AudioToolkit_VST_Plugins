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

#include "AmbisonicEncoder.h"

AmbisonicEncoder::AmbisonicEncoder (Binaural::CCore& core) : mCore (core)
{
}

void AmbisonicEncoder::processBlock (const std::vector<CSingleSourceRef>& sources, AudioBuffer<float>& quadOut)
{
    static constexpr float WScale = 0.707107f;
    
    quadOut.clear();
    
    /////////////////////////////////////////
    // 1-st Order Virtual Ambisonics Encoder
    /////////////////////////////////////////
    jassert (quadOut.getNumChannels() == 4);
    
    // Go through each source
    for (auto source : sources)
    {
        // Check source flags for reverb process
        if (! source->IsReverbProcessEnabled())
            continue;
        
        // Get azimuth, elevation and distance from listener to each source
        // We precompute everything, to minimize per-sample computations.
        auto sourceTransform = source->GetSourceTransform();
        auto vectorToSource  = mCore.GetListener()->GetListenerTransform().GetVectorTo (sourceTransform);

        float sourceElevation = vectorToSource.GetElevationRadians();
        float sinElevationAbs = std::fabs (std::sin (sourceElevation)); // TEST: adding power to W channel to compensate for the lack of Z channel
        float sinElevation = std::sin (sourceElevation);
        float cosElevation = std::cos (sourceElevation);

        float cosAcosE = 0.0f;
        float sinAcosE = 0.0f;

        if (! Common::CMagnitudes::AreSame (0.0f, cosElevation, EPSILON))
        {
            float sourceAzimuth = vectorToSource.GetAzimuthRadians();
            float cosAzimuth = std::cos (sourceAzimuth);
            float sinAzimuth = std::sin (sourceAzimuth);
            cosAcosE = cosAzimuth * cosElevation;
            sinAcosE = sinAzimuth * cosElevation;
        }

        float sourceDistance = vectorToSource.GetDistance();

        // Check if the source is in the same position as the listener head. If yes, do not apply spatialization to this source
        if (sourceDistance < mCore.GetListener()->GetHeadRadius())
            continue;

        CMonoBuffer<float> sourceBuffer = source->GetBuffer();
        jassert (sourceBuffer.size() > 0);
        
        if (sourceBuffer.empty())
            continue;

        // Apply Distance Attenuation
        float distanceAttenuation_ReverbConstant = mCore.GetMagnitudes().GetReverbDistanceAttenuation();

        if (source->IsDistanceAttenuationEnabledReverb())
        {
            distanceAttenuatorReverb.Process (sourceBuffer, sourceDistance, distanceAttenuation_ReverbConstant, mCore.GetAudioState().bufferSize, mCore.GetAudioState().sampleRate);
        }

        auto numSamples = mCore.GetAudioState().bufferSize;
        jassert (sourceBuffer.size() >= numSamples
              && quadOut.getNumSamples() >= numSamples);

        for (int nSample = 0; nSample < numSamples; nSample++)
        {
            // Value from the input buffer
            float newSample = sourceBuffer[nSample];

            if (true /* Bi-dimensional. TODO: Support other order types */)
            {
                // Add partial contribution of this source to each B-format channel
                quadOut.getWritePointer (0)[nSample] += newSample * WScale;
                quadOut.getWritePointer (1)[nSample] += newSample * cosAcosE;
                quadOut.getWritePointer (1)[nSample] += newSample * sinElevationAbs; // Adding power to X channel to compensate for the lack of Z channel
                quadOut.getWritePointer (2)[nSample] += newSample * sinAcosE;
            }
            else // Three-dimiensional
            {
                // Add partial contribution of this source to each B-format channel
                quadOut.getWritePointer (0)[nSample] += newSample * WScale;
                quadOut.getWritePointer (1)[nSample] += newSample * cosAcosE;
                quadOut.getWritePointer (2)[nSample] += newSample * sinElevation;
                quadOut.getWritePointer (3)[nSample] += newSample * sinAcosE;
            }
        }
    }
}
