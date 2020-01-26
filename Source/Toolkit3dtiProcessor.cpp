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

#include "Utils.h"
#include "Toolkit3dtiProcessor.h"

void copySourceSettings(CSingleSourceRef oldSource, CSingleSourceRef newSource);

Toolkit3dtiProcessor::Impl::Ptr CreateImpl(double sampleRate, int samplesPerBlock) {
  Toolkit3dtiProcessor::Impl::Ptr impl( new Toolkit3dtiProcessor::Impl );
  
  // Declaration and initialization of stereo buffer
  impl->mOutputBuffer.left.resize(samplesPerBlock);
  impl->mOutputBuffer.right.resize(samplesPerBlock);
  
  Common::TAudioStateStruct audioState;
  audioState.bufferSize = samplesPerBlock;
  audioState.sampleRate = sampleRate;
  
  impl->mCore.SetAudioState(audioState);
  impl->mCore.SetHRTFResamplingStep(15);
  
  // Create listener
  impl->mListener = impl->mCore.CreateListener();
  
  // Environment setup
  impl->mEnvironment = impl->mCore.CreateEnvironment();
  impl->mEnvironment->SetReverberationOrder(TReverberationOrder::BIDIMENSIONAL); // Setting number of ambisonic channels for reverb processing
  
  return impl;
}

Toolkit3dtiProcessor::Toolkit3dtiProcessor()
  : enableCustomizedITD("0", "Custom Head Circumference", false),
    headCircumference("1", "Head Circumference", 450, 620, 550),
    enableNearDistanceEffect("2", "Near Distance Effect", true),
    enableFarDistanceEffect("3", "Far Distance Effect", false),
    spatializationMode("4", "SpatializationMode", 0, 2, 2),
    sourceDistanceAttenuation("5", "Source Distance Attenuation", NormalisableRange<float>(-6.f, 0.f, 0.1f), -6.f),
    reverbGain("6", "Reverb Gain", NormalisableRange<float>(-30.f, 6.f, 0.1f), -3.f),
    reverbDistanceAttenuation("7", "Reverb Distance Attenuation", NormalisableRange<float>(-6.f, 0.f, 0.1f), -3.f)
{
#if DEBUG
  ERRORHANDLER3DTI.SetVerbosityMode(VERBOSITYMODE_ERRORSANDWARNINGS);
  ERRORHANDLER3DTI.SetErrorLogStream(&std::cout, true);
#endif
  
  mTransform.SetPosition( Common::CVector3(1,0,0) );
  
  setup(44100.0, 512);
}

void Toolkit3dtiProcessor::setup(double sampleRate, int samplesPerBlock) {
  if ( pimpl != nullptr ) {
    auto audioState = pimpl->mCore.GetAudioState();
    double currentSampleRate = audioState.sampleRate;
    int currentBlockSize = audioState.bufferSize;
    if ( sampleRate == currentSampleRate
      && samplesPerBlock == currentBlockSize ) {
      // Session is already configured. Abort.
      return;
    }
  }
  
  // Create new internal implementation
  auto impl = CreateImpl(sampleRate, samplesPerBlock);
  
  // Load HRTF
  // If we have an existing section we reload
  // the same HRTF but of new sample rate
  int hrtfIndex = pimpl ? pimpl->hrtfIndex : 0;
  File hrtf = getBundledHRTF(hrtfIndex, sampleRate);
  
  // Load BRIR
  // If we have an existing section we reload
  // the same BRIR but of new sample rate
  int brirIndex = pimpl ? pimpl->brirIndex : 0;
  File brir = getBundledBRIR(brirIndex, sampleRate);
  
  reset(std::move(impl), hrtf, brir);
}

void Toolkit3dtiProcessor::reset(Impl::Ptr impl, const File& hrtf, const File& brir) {
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
  
  if ( !brir.existsAsFile() ) {
    DBG("BRIR file doesn't exist");
  }
  loadBRIR( *impl, brir );
  
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
  
  // Assign the new private implementation
  while (true) {
    if ( mtx.try_lock() ) {
      
      pimpl.reset(impl.get());
      mtx.unlock();
      break;
      
    } else {
      // Failed to aquire lock for assigning new implementation
      // Try again
    }
  }
}

void Toolkit3dtiProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages) {
  if ( pimpl.get() == nullptr ) {
    return;
  }
  
  // Try to aquire lock
  // If it fails we are in internal setup process
  if ( !mtx.try_lock() ) {
    return;
  }
  
  updateParameters(*pimpl.get());

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

#ifndef DEBUG // NOTE(Ragnar): Reverb processing is too heavy for debug mode

  bool reverbEnabled = getSources().front()->IsReverbProcessEnabled();
  if ( reverbEnabled || reverbPower > 0.f ) {
    // Reverberation processing of all sources
    CMonoBufferPairf reverbBuffer;
    pimpl->mEnvironment->ProcessVirtualAmbisonicReverb(reverbBuffer.left, reverbBuffer.right);
    auto reverbGain  = Decibels::decibelsToGain(this->reverbGain.get());
    reverbBuffer.left.ApplyGain(reverbGain);
    reverbBuffer.right.ApplyGain(reverbGain);

    // Adding reverberated sound to the output mix
    if ( reverbBuffer.left.size() == bufferSize ) {
      // Checking size of buffer because if a BRIR is not loaded
      // it will be zero and trigger a crash when added to output
      pimpl->mOutputBuffer.left  += reverbBuffer.left;
      pimpl->mOutputBuffer.right += reverbBuffer.right;
    }

    reverbPower = reverbBuffer.left.GetPower();
  }
  
#endif

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
  
  mtx.unlock();
}

void Toolkit3dtiProcessor::updateParameters(Impl& impl) {
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
  magnitudes.SetReverbDistanceAttenuation(reverbDistanceAttenuation);
  impl.mCore.SetMagnitudes(magnitudes);
}

bool Toolkit3dtiProcessor::loadHRTF(int bundledIndex) {
  auto sampleRate = pimpl->mCore.GetAudioState().sampleRate;
  return loadHRTF(getBundledHRTF(bundledIndex, sampleRate));
}

bool Toolkit3dtiProcessor::loadHRTF(const File& file) {
  auto sampleRate = pimpl->mCore.GetAudioState().sampleRate;
  auto blockSize  = pimpl->mCore.GetAudioState().bufferSize;
  File brir = getBundledBRIR(pimpl->brirIndex, sampleRate);
  reset(CreateImpl(sampleRate, blockSize), file, brir);
  return true;
}

// TODO: Move to private implmentation
bool Toolkit3dtiProcessor::loadHRTF(Impl& impl, const File& file) {
  DBG("Loading HRTF: " << file.getFullPathName());
  bool success = false;
  success = loadResourceFile(impl, file, true);
  impl.hrtfIndex = hrtfPathToBundledIndex(file);
  impl.hrtfPath = file;
  return success;
}

bool Toolkit3dtiProcessor::loadHRTF_ILD(const File& file) {
  return loadHRTF_ILD(*pimpl, file);
}

bool Toolkit3dtiProcessor::loadHRTF_ILD(Impl& impl, const File& file) {
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

bool Toolkit3dtiProcessor::loadBRIR(int bundledIndex) {
  auto sampleRate = pimpl->mCore.GetAudioState().sampleRate;
  return loadBRIR(getBundledBRIR(bundledIndex, sampleRate));
}

bool Toolkit3dtiProcessor::loadBRIR(const File& file) {
  auto sampleRate = pimpl->mCore.GetAudioState().sampleRate;
  auto blockSize  = pimpl->mCore.GetAudioState().bufferSize;
  File hrtf = getBundledHRTF(pimpl->hrtfIndex, sampleRate);
  reset(CreateImpl(sampleRate, blockSize), hrtf, file);
  return true;
}

bool Toolkit3dtiProcessor::loadBRIR(Impl &impl, const File& file) {
  DBG("Loading BRIR: " << file.getFullPathName());
  bool success = loadResourceFile(impl, file, false);
  impl.brirIndex = brirPathToBundledIndex(file);
  impl.brirPath = file;
  return success;
}

bool Toolkit3dtiProcessor::loadResourceFile(Impl &impl, const File& file, bool isHRTF) {
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
  } else {
    return isSofa ? BRIR::CreateFromSofa(path, impl.mEnvironment) : BRIR::CreateFrom3dti(path, impl.mEnvironment);
  }
  return false;
}

void Toolkit3dtiProcessor::addSoundSource(Impl &impl, Common::CVector3& position) {
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
