/**
* \class Toolkit3dtiProcessor
*
* \brief Declaration of Toolkit3dtiProcessor interface.
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

#include <BinauralSpatializer/3DTI_BinauralSpatializer.h>
#include <HRTF/HRTFFactory.h>
#include <HRTF/HRTFCereal.h>
#include <BRIR/BRIRFactory.h>
#include <BRIR/BRIRCereal.h>
#include <ILD/ILDCereal.h>
#include <JuceHeader.h>

using CListenerRef = shared_ptr<Binaural::CListener>;
using CEnvironmentRef = shared_ptr<Binaural::CEnvironment>;
using CSingleSourceRef = shared_ptr<Binaural::CSingleSourceDSP>;
using CMonoBufferPairf = Common::CEarPair<CMonoBuffer<float>>;

class Toolkit3dtiProcessor {
public:
    
  struct Impl {
    using Ptr = std::unique_ptr<Impl>;
        
    Binaural::CCore                 mCore;
    CEnvironmentRef                 mEnvironment;
    CListenerRef                    mListener;
    CMonoBufferPairf                mOutputBuffer;
    std::vector<CSingleSourceRef>   sources;
    int  hrtfIndex;
    int  brirIndex;
    File hrtfPath;
    File brirPath;
  };
  
  //============================================================================
  Toolkit3dtiProcessor();
  
  //============================================================================
  void setup(double sampleRate, int frameSize);
  
  void processAnechoic (AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
  void processReverb   (AudioBuffer<float>& buffer, MidiBuffer& midiMessages);
  
  //============================================================================
  // Note(Ragnar): Currently never larger than 1
  const std::vector<CSingleSourceRef>& getSources() {
    return pimpl->sources;
  }
  
  //============================================================================
  bool loadHRTF(int bundledIndex); // A number between 0-6 for bundled HRTFs
  bool loadHRTF(const File& file);
  bool loadHRTF_ILD(const File& file);
  bool loadBRIR(int bundledIndex); // A number between 0-2 for bundled HRTFs
  bool loadBRIR(const File& file);
  
  int getHrtfIndex() const { return pimpl->hrtfIndex; };
  int getBrirIndex() const { return pimpl->brirIndex; };
  const File& getHrtfPath() const { return pimpl->hrtfPath; }
  const File& getBrirPath() const { return pimpl->brirPath; }
  
  //============================================================================
  float getHeadRadius() const {
    return (headCircumference / (2.f * M_PI)) * 0.001f;
  }
  
  // TODO: Source Position as AudioParameterValueTree?
  void setSourcePosition( Common::CVector3 pos ) {
    auto headRadius = getHeadRadius();
    auto distance = pos.GetDistance();
    if ( distance <= headRadius ) {
      auto newDistance = distance + (headRadius-distance) + 0.01f;
      pos.SetFromAED(pos.GetAzimuthDegrees(), pos.GetElevationDegrees(), newDistance);
    }
    mTransform.SetPosition( pos );
  }
  
  Common::CVector3 getSourcePosition() { return mTransform.GetPosition(); }
  
  AudioParameterBool enableCustomizedITD;
  AudioParameterInt  headCircumference;
  AudioParameterBool enableNearDistanceEffect;
  AudioParameterBool enableFarDistanceEffect;
  AudioParameterInt spatializationMode;
  // AudioParameterFloat sourceGain; // ranges from -12 to + 12 dB
  AudioParameterFloat sourceDistanceAttenuation; // ranges from -6 to 0 dB
  AudioParameterFloat reverbGain; // ranges from -12 to + 12 dB
  AudioParameterFloat reverbDistanceAttenuation; // ranges from -6 to 0 dB
  
private:
  CriticalSection loadLock;
  
  void reset(Impl::Ptr p, const File& hrtf, const File& brir);
  void updateParameters(Impl& impl);
  void addSoundSource(Impl& impl, Common::CVector3& position);
  bool loadResourceFile(Impl& impl, const File& file, bool isHRTF);
  bool loadHRTF(Impl& impl, const File& file);
  bool loadHRTF_ILD(Impl& impl, const File& file);
  bool loadBRIR(Impl& impl, const File& file);
  
  //============================================================================
  Impl::Ptr pimpl;
  
  Common::CTransform mTransform;    // Source transform
  
  float reverbPower = 0.f;
};
