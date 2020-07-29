/**
* \class AnechoicProcessor
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

#include "Utils.h"
#include "AnechoicProcessor.h"

void copySourceSettings(CSingleSourceRef oldSource, CSingleSourceRef newSource);

AnechoicProcessor::Impl::Ptr CreateImpl(Binaural::CCore& core) {
  auto impl = std::make_unique<AnechoicProcessor::Impl> (core);
  
  auto blockSize = core.GetAudioState().bufferSize;
    
  // Declaration and initialization of stereo buffer
  impl->mOutputBuffer.left .resize(blockSize);
  impl->mOutputBuffer.right.resize(blockSize);
  
  // Create listener if necessary
  // NOTE(Ragnar): There can only be one listener and
  // CreateListnener() returns nullptr if one exists
  if (auto listener = core.GetListener())
      impl->mListener = listener;
  else
      impl->mListener = core.CreateListener();
    
  return impl;
}

AnechoicProcessor::AnechoicProcessor(Binaural::CCore& core)
  : enableCustomizedITD("0", "Custom Head Circumference", false),
    headCircumference("1", "Head Circumference", 450, 620, 550),
    enableNearDistanceEffect("2", "Near Distance Effect", true),
    enableFarDistanceEffect("3", "Far Distance Effect", false),
    spatializationMode("4", "SpatializationMode", 0, 2, 2),
    sourceDistanceAttenuation("5", "Source Distance Attenuation", NormalisableRange<float>(-6.f, 0.f, 0.1f), -6.f)
  , mCore (core)
  , pimpl (CreateImpl (core))
{
#if DEBUG
  ERRORHANDLER3DTI.SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);
  ERRORHANDLER3DTI.SetErrorLogStream(&std::cout, true);
#endif
  
  mTransform.SetPosition( Common::CVector3(1,0,0) );
}

void AnechoicProcessor::setup(double sampleRate, int samplesPerBlock) {
  // Create new internal implementation
  auto impl = CreateImpl (mCore);
  
  // Load HRTF
  // If we have an existing section we reload
  // the same HRTF but of new sample rate
  int hrtfIndex = pimpl ? pimpl->hrtfIndex : 0;
  File hrtf = getBundledHRTF(hrtfIndex, sampleRate);
  
  reset(std::move(impl), hrtf);
}

void AnechoicProcessor::reset(Impl::Ptr impl, const File& hrtf) {
  if ( !hrtf.existsAsFile() ) {
    DBG("HRTF file doesn't exist");
  }
  loadHRTF( *impl, hrtf );
  
  auto sampleRate = impl->mCore.GetAudioState().sampleRate;
  
  // Load HRTF ILD
  File hrtfILD = ILDDirectory().getChildFile(SampleRateToDefaultHRTF_ILD.at((int)sampleRate));
  if ( !hrtfILD.existsAsFile() ) {
    DBG("HRTF ILD file doesn't exist");
  }
  loadHRTF_ILD( *impl, hrtfILD );
  
  // Load near field ILD
  File nearFieldConf = ILDDirectory().getChildFile(SampleRateToNearFieldILD.at((int)sampleRate));
  if ( !nearFieldConf.existsAsFile() ) {
    DBG("Near field ILD file doesn't exist");
  }
  
  DBG("Loading ILD (near field): " + nearFieldConf.getFullPathName());
  if ( !ILD::CreateFrom3dti_ILDNearFieldEffectTable( nearFieldConf.getFullPathName().toStdString(), impl->mListener )) {
    DBG("Unable to load ILD Near Field Effect simulation file. Near (ILD) will not work");
  }
  
  // Add a sound source
  auto position = getSourcePosition();
  addSoundSource(*impl.get(), position);
  
  if ( pimpl != nullptr ) {
    for ( int i = 0; i < pimpl->sources.size(); i++ ) {
      auto newSource = impl->sources[i];
      auto oldSource = pimpl->sources[i];
      copySourceSettings(oldSource, newSource);
    }
  }
  
  const ScopedLock sl (loadLock);
  // Assign the new private implementation
  pimpl = std::move(impl);
}

void AnechoicProcessor::processAnechoic (AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
  if ( pimpl == nullptr) {
    buffer.clear();
    return;
  }
  
  const ScopedTryLock sl (loadLock);
  if (! sl.isLocked())
    return;
    
  updateParameters(*pimpl);

  // Process audio
  auto bufferSize = buffer.getNumSamples();
  
  // Initializes buffer with zeros
  _3dti_clear(pimpl->mOutputBuffer);

  // Getting the processed audio
  // Declaration, initialization and filling mono buffers
  CMonoBuffer<float> input(bufferSize);

  // Fill input buffer with incoming audio
  std::memcpy(input.data(), buffer.getReadPointer(0), bufferSize*sizeof(float));

  // Spatialize
  if ( pimpl->mListener->GetHRTF()->IsHRTFLoaded() ) {
    for ( auto const& source : getSources() ) {
      CMonoBufferPairf anechoicBuffer;

      source->SetBuffer(input);
      source->ProcessAnechoic(anechoicBuffer.left, anechoicBuffer.right);

      pimpl->mOutputBuffer.left  += anechoicBuffer.left;
      pimpl->mOutputBuffer.right += anechoicBuffer.right;
    }
  }

  // Fill the output with processed audio
  // Incoming buffer should have two channels
  // for spatialised audio but we check just in case
  int numChannels = std::max(buffer.getNumChannels(), 2);
  
  for ( int i = 0; i < bufferSize; i++ ) {
    switch (numChannels) {
      case 2:
        buffer.getWritePointer(1)[i] = pimpl->mOutputBuffer.right[i];
      default:
        buffer.getWritePointer(0)[i] = pimpl->mOutputBuffer.left[i];
    }
  }
}

void AnechoicProcessor::updateParameters(Impl& impl) {
  if ( enableCustomizedITD ) {
    impl.mListener->EnableCustomizedITD();
  } else {
    impl.mListener->DisableCustomizedITD();
  }
  
  auto headradius_cm = headCircumference / (2.0 * M_PI * 10.0);
  impl.mListener->SetHeadRadius(headradius_cm / 100.0);
  
  for ( auto const& source : impl.sources ) {
    source->SetSpatializationMode((Binaural::TSpatializationMode)spatializationMode.get());
    
    if ( enableNearDistanceEffect ) {
      source->EnableNearFieldEffect();
    } else {
      source->DisableNearFieldEffect();
    }
    
    if ( enableFarDistanceEffect ) {
      source->EnableFarDistanceEffect();
    } else {
      source->DisableFarDistanceEffect();
    }
    
    source->SetSourceTransform( mTransform );
  }
  
  auto magnitudes = impl.mCore.GetMagnitudes();
  magnitudes.SetAnechoicDistanceAttenuation(sourceDistanceAttenuation);
  impl.mCore.SetMagnitudes(magnitudes);
}

bool AnechoicProcessor::loadHRTF(int bundledIndex) {
  auto sampleRate = pimpl->mCore.GetAudioState().sampleRate;
  return loadHRTF (getBundledHRTF(bundledIndex, sampleRate));
}

bool AnechoicProcessor::loadHRTF(const File& file) {
  reset (CreateImpl (mCore), file);
  return true;
}

// TODO: Move to private implmentation
bool AnechoicProcessor::loadHRTF(Impl& impl, const File& file) {
  DBG("Loading HRTF: " << file.getFullPathName());
  bool success = false;
  success = loadResourceFile(impl, file, true);
  impl.hrtfIndex = hrtfPathToBundledIndex(file);
  impl.hrtfPath = file;
  return success;
}

bool AnechoicProcessor::loadHRTF_ILD(const File& file) {
  return loadHRTF_ILD(*pimpl, file);
}

bool AnechoicProcessor::loadHRTF_ILD(Impl& impl, const File& file) {
  auto path = file.getFullPathName().toStdString();
  DBG("Loading HRTF ILD: " << path);
  auto fileSampleRate = ILD::GetSampleRateFrom3dti(path);
  if ( impl.mCore.GetAudioState().sampleRate != fileSampleRate ) {
    // TODO(Ragnar): Report error
    DBG("Error: HRTF ILD file sample rate doesn't match current session");
    return false;
  }
  
  bool success = ILD::CreateFrom3dti_ILDSpatializationTable(path, impl.mListener);
  if ( !success ) {
    // TODO(Ragnar): Report error
    DBG("Error: Unable to load HRTF ILD file");
  }
  return success;
}

bool AnechoicProcessor::loadResourceFile(Impl &impl, const File& file, bool isHRTF) {
  int sampleRate = impl.mCore.GetAudioState().sampleRate;
  int fileSampleRate = checkResourceSampleRate(file, isHRTF);
  // TODO: Throw exception / return error and trigger warning from editor
  if ( fileSampleRate != sampleRate ) {
    AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Wrong sample rate",
                                     "Please select a file that matches the project sample rate",
                                     "OK");
    return false;
  }
  bool isSofa = isSofaFile(file);
  auto path = file.getFullPathName().toStdString();
  if ( isHRTF ) {
    bool specificDelays;
    return isSofa ? HRTF::CreateFromSofa(path, impl.mListener, specificDelays) : HRTF::CreateFrom3dti(path, impl.mListener);
  }
  return false;
}

void AnechoicProcessor::addSoundSource(Impl &impl, Common::CVector3& position) {
  if ( impl.sources.size() == 1 ) {
    DBG("Only one source allowed at this time.");
    return;
  }
  
  auto source = impl.mCore.CreateSingleSourceDSP();
  auto sourcePosition = Common::CTransform();
  sourcePosition.SetPosition(position);
  source->SetSourceTransform(sourcePosition);
  source->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
  source->DisableNearFieldEffect();
  source->EnableAnechoicProcess();
  source->EnableReverbProcess();
  source->EnableDistanceAttenuationReverb();
  source->EnableDistanceAttenuationAnechoic();
  
  impl.sources.push_back(source);
}

void copySourceSettings(CSingleSourceRef oldSource, CSingleSourceRef newSource) {
  newSource->SetSourceTransform(oldSource->GetSourceTransform());
  newSource->SetSpatializationMode(oldSource->GetSpatializationMode());
  
  if ( oldSource->IsNearFieldEffectEnabled() ) {
    newSource->EnableNearFieldEffect();
  } else {
    newSource->DisableNearFieldEffect();
  }
  if ( oldSource->IsAnechoicProcessEnabled() ) {
    newSource->EnableAnechoicProcess();
  } else {
    newSource->DisableAnechoicProcess();
  }
  if ( oldSource->IsReverbProcessEnabled() ) {
    newSource->EnableReverbProcess();
  } else {
    newSource->DisableReverbProcess();
  }
  if ( oldSource->IsDistanceAttenuationEnabledAnechoic() ) {
    newSource->EnableDistanceAttenuationAnechoic();
  } else {
    newSource->DisableDistanceAttenuationAnechoic();
  }
  if ( oldSource->IsDistanceAttenuationEnabledReverb() ) {
    newSource->EnableDistanceAttenuationReverb();
  } else {
    newSource->DisableDistanceAttenuationReverb();
  }
}
