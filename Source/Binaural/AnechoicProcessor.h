/**
 * \class AnechoicProcessor
 *
 * \brief Declaration of AnechoicProcessor interface.
 * \date  February 2022
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

#include <BinauralSpatializer/3DTI_BinauralSpatializer.h>
#include <JuceHeader.h>

using CSingleSourceRef = shared_ptr<Binaural::CSingleSourceDSP>;
using CMonoBufferPair  = Common::CEarPair<CMonoBuffer<float>>;

// Utility function to copy all settings to a new source. Does NOT update position
void copySourceSettings(CSingleSourceRef oldSource, CSingleSourceRef newSource);

class AnechoicProcessor   : public  ChangeBroadcaster,
                            public  AudioProcessorValueTreeState::Listener
{
public:
    //============================================================================
    AnechoicProcessor (Binaural::CCore& core);
    
    ~AnechoicProcessor();
    
    //============================================================================
    void setup (double sampleRate/*, int frameSize TODO: Currently fixed*/);
    
    void processBlock (AudioBuffer<float>& monoIn, AudioBuffer<float>& stereoOut);
    
    //============================================================================
    void addSoundSource(const Common::CVector3& position);
    
    const std::vector<CSingleSourceRef>& getSources() { return mSources; }
    
    //==========================================================================
    bool loadHRTF (const File& file);
    
    File getHrtfPath() const { return hrtfPath; }
    
    //==========================================================================
    inline float getHeadRadius() const {
        return (headCircumference / (2.f * M_PI)) * 0.001f;
    }
    
    // TODO: Source Position as AudioParameterValueTree?
    inline void setSourcePosition(const CSingleSourceRef source, Common::CVector3 pos)
    {
        auto headRadius = getHeadRadius();
        auto distance = pos.GetDistance();
        if ( distance <= headRadius ) {
            auto newDistance = distance + (headRadius-distance) + 0.01f;
            pos.SetFromAED(pos.GetAzimuthDegrees(), pos.GetElevationDegrees(), newDistance);
        }
        
        auto it = std::find (mSources.begin(), mSources.end(), source);
        if ( it != mSources.end() )
        {
            auto idx = std::distance (mSources.begin(), it);
            mTransforms[idx].SetPosition (pos);
        }
        else DBG ("Source not found");
    }
    
    inline Common::CVector3 getSourcePosition (int index = 0)
    {
        if (index > (int)mSources.size() - 1)
            return Common::CVector3 (0, 1, 0);
        
        return mTransforms[index].GetPosition();
    }
    
    inline Common::CVector3 getSourcePosition (CSingleSourceRef source)
    {
        if (mSources.empty())
            return Common::CVector3 (0, 1, 0);
        
        return getSourcePosition (0);
    }
    
    //==========================================================================
    AudioParameterBool enableCustomizedITD;
    AudioParameterInt  headCircumference;
    AudioParameterBool enableNearDistanceEffect;
    AudioParameterBool enableFarDistanceEffect;
    AudioParameterInt  spatializationMode;
    // AudioParameterFloat sourceGain; // ranges from -12 to + 12 dB
    AudioParameterFloat sourceDistanceAttenuation; // ranges from -6 to 0 dB
    AudioParameterBool  enableReverbDistanceAttenuation;
    AudioParameterFloat reverbDistanceAttenuation;
    
    std::atomic<bool> isLoading {false};
    
    void parameterChanged (const String& parameterID, float newValue) override;
    
private:
    //============================================================================
    void updateParameters();
    bool __loadHRTF (const File& file);
    bool __loadHRTF_ILD (const File& file);
    bool loadResourceFile (const File& file);
    void loadCustomHRTF (String fileTypes, std::function<void(File)> chosen);
    
    //============================================================================
    double mSampleRate;
    Binaural::CCore& mCore;
    std::shared_ptr<Binaural::CListener>    mListener;
    CMonoBufferPair                         mOutputBuffer;
    std::vector<CSingleSourceRef>           mSources;
    std::vector<Common::CTransform>         mTransforms;
    
    File hrtfPath;
    std::unique_ptr<FileChooser> fc;
};
