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

#include <HRTF/HRTFFactory.h>
#include <HRTF/HRTFCereal.h>
#include <ILD/ILDCereal.h>
#include "../Utils.h"
#include "AnechoicProcessor.h"

void initSource (CSingleSourceRef source, const Common::CVector3& position)
{
    auto sourcePosition = Common::CTransform();
    sourcePosition.SetPosition(position);
    source->SetSourceTransform(sourcePosition);
    source->SetSpatializationMode(Binaural::TSpatializationMode::HighQuality);
    source->DisableNearFieldEffect();
    source->EnableAnechoicProcess();
    source->EnableReverbProcess();
    source->EnableDistanceAttenuationReverb();
    source->EnableDistanceAttenuationAnechoic();
}

AnechoicProcessor::AnechoicProcessor (Binaural::CCore& core)
  :  enableCustomizedITD("0", "Custom Head Circumference", false)
  ,  headCircumference("1", "Head Circumference", 450, 620, 550)
  ,  enableNearDistanceEffect("2", "Near Distance Effect", true)
  ,  enableFarDistanceEffect("3", "Far Distance Effect", false)
  ,  spatializationMode("4", "SpatializationMode", 0, 2, 2)
  ,  sourceDistanceAttenuation("5", "Source Distance Attenuation", NormalisableRange<float>(-6.f, 0.f, 0.1f), -6.f)
  ,  enableReverbDistanceAttenuation ("6", "Enable Rev Dist Attenuation", true)
  ,  reverbDistanceAttenuation("7", "Reverb Distance Attenuation", NormalisableRange<float>(-6.f, 0.f, 0.1f), -3.f)
  ,  mCore (core)
  ,  mListener (core.CreateListener())
{
}

AnechoicProcessor::~AnechoicProcessor()
{
    stopTimer();
}

void AnechoicProcessor::setup (double sampleRate)
{
    auto blockSize = mCore.GetAudioState().bufferSize;
    
    // Declaration and initialization of stereo buffer
    mOutputBuffer.left .resize (blockSize);
    mOutputBuffer.right.resize (blockSize);
    // Load HRTF
    
    auto loadedHrtf = getHrtfPath();
    
    if (loadedHrtf.exists())
    {
        if (sampleRate != mSampleRate)
        {
            int index = hrtfPathToBundledIndex (loadedHrtf);
            
            if (index >= 0)
            {
                loadHRTF (getBundledHRTF (index, sampleRate));
            }
        }
        
        mSampleRate = sampleRate;
        
        return;
    }
    
    loadHRTF (getBundledHRTF (0, sampleRate));
    
    mSampleRate = sampleRate;
    
    JUCE_ASSERT_MESSAGE_MANAGER_EXISTS;
    startTimerHz (2);
    
    timerCallback();
}

void AnechoicProcessor::timerCallback()
{
    if (mHRTFsToLoad.size() > 0)
    {
        if (isLoading.load())
            return;

        auto path = mHRTFsToLoad[0];
        reset (path);
        
        mHRTFsToLoad.removeAllInstancesOf (path);
    }
}

void AnechoicProcessor::reset (const File& hrtf)
{
    isLoading.store (true);
    
    if (! hrtf.existsAsFile())
    {
        DBG ("HRTF file doesn't exist");
        isLoading.store (false);
        return;
    }
    
    __loadHRTF(hrtf);

    auto sampleRate = mCore.GetAudioState().sampleRate;

    // Load HRTF ILD
    File hrtfILD = ILDDirectory().getChildFile(SampleRateToDefaultHRTF_ILD.at((int)sampleRate));
    if (! hrtfILD.existsAsFile())
    {
        DBG ("HRTF ILD file doesn't exist");
        isLoading.store (false);
        return;
    }
    
    __loadHRTF_ILD (hrtfILD);

    // Load near field ILD
    File nearFieldConf = ILDDirectory().getChildFile (SampleRateToNearFieldILD.at((int)sampleRate));
    if (! nearFieldConf.existsAsFile() ) {
        DBG ("Near field ILD file doesn't exist");
        isLoading.store (false);
        return;
    }


    DBG("Loading ILD (near field): " + nearFieldConf.getFullPathName());
    if ( !ILD::CreateFrom3dti_ILDNearFieldEffectTable( nearFieldConf.getFullPathName().toStdString(), mListener )) {
        DBG ("Unable to load ILD Near Field Effect simulation file. Near (ILD) will not work");
    }
    
    // Re-enable processing
    isLoading.store (false);
    
    sendChangeMessage();
}

void AnechoicProcessor::processBlock (AudioBuffer<float>& monoIn, AudioBuffer<float>& stereoOut)
{
    if (isLoading.load())
    {
        stereoOut.clear();
        return;
    }
    
    updateParameters();
    
    // Initializes buffer with zeros
    _3dti_clear (mOutputBuffer);
    
    // Getting the processed audio
    // Declaration, initialization and filling mono buffers
    CMonoBuffer<float> input = juceTo3dti (monoIn);
    
    // Spatialize
    if (mListener->GetHRTF()->IsHRTFLoaded())
    {
        for ( auto const& source : getSources() )
        {
            CMonoBufferPair anechoicBuffer;
            
            source->SetBuffer(input);
            source->ProcessAnechoic(anechoicBuffer.left, anechoicBuffer.right);
            
            mOutputBuffer.left  += anechoicBuffer.left;
            mOutputBuffer.right += anechoicBuffer.right;
        }
    }
    
    // Fill the output with processed audio
    // Incoming buffer should have two channels
    // for spatialised audio but we check just in case
    int numChannels = std::max (stereoOut.getNumChannels(), 2);
    
    for (int i = 0; i < stereoOut.getNumSamples(); i++)
    {
        switch (numChannels) {
            case 2:
                stereoOut.getWritePointer(1)[i] = mOutputBuffer.right[i];
            default:
                stereoOut.getWritePointer(0)[i] = mOutputBuffer.left[i];
        }
    }
}

void AnechoicProcessor::parameterChanged (const String& parameterID, float newValue)
{
    int index = std::floor (newValue);
    
    if (index < BundledHRTFs.size() - 2)
    {
        auto sampleRate = mCore.GetAudioState().sampleRate;
        loadHRTF (getBundledHRTF (index, sampleRate));
    }
    else
    {
        String fileTypes = index == BundledHRTFs.size() ? "*.sofa" : "*.3dti-hrtf";
        
        loadCustomHrtf (fileTypes, [this] (File hrtf) {
            loadHRTF (hrtf);
        });
    }
}

void AnechoicProcessor::updateParameters()
{
    if ( enableCustomizedITD ) {
        mListener->EnableCustomizedITD();
    } else {
        mListener->DisableCustomizedITD();
    }
    
    auto headradius_cm = headCircumference / (2.0 * M_PI * 10.0);
    mListener->SetHeadRadius(headradius_cm / 100.0);
    
    const auto numSources = mSources.size();
    for ( auto i = 0; i < numSources; i++ )
    {
        auto const& source = mSources[i];
        
        source->SetSpatializationMode ((Binaural::TSpatializationMode)spatializationMode.get());
        
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
        
        if (enableReverbDistanceAttenuation)
            source->EnableDistanceAttenuationReverb();
        else
            source->DisableDistanceAttenuationReverb();
        
        source->SetSourceTransform( mTransforms[i] );
    }
    
    auto magnitudes = mCore.GetMagnitudes();
    magnitudes.SetAnechoicDistanceAttenuation (sourceDistanceAttenuation);
    magnitudes.SetReverbDistanceAttenuation (reverbDistanceAttenuation);    
    mCore.SetMagnitudes(magnitudes);
}

bool AnechoicProcessor::loadHRTF (const File& file)
{
    if (file == hrtfPath)
        return false;
    
    return mHRTFsToLoad.addIfNotAlreadyThere (file);
}

bool AnechoicProcessor::__loadHRTF (const File& file)
{
    DBG("Loading HRTF: " << file.getFullPathName());
    bool success = loadResourceFile(file);
    if (success)
    {
        hrtfPath = file;
    }
    return success;
}

bool AnechoicProcessor::__loadHRTF_ILD (const File& file)
{
    auto path = file.getFullPathName().toStdString();
    DBG("Loading HRTF ILD: " << path);
    auto fileSampleRate = ILD::GetSampleRateFrom3dti(path);
    if ( mCore.GetAudioState().sampleRate != fileSampleRate ) {
        // TODO(Ragnar): Report error
        DBG("Error: HRTF ILD file sample rate doesn't match current session");
        return false;
    }
    
    bool success = ILD::CreateFrom3dti_ILDSpatializationTable (path, mListener);
    if ( !success ) {
        // TODO(Ragnar): Report error
        DBG("Error: Unable to load HRTF ILD file");
    }
    return success;
}

bool AnechoicProcessor::loadResourceFile(const File& file)
{
    int sampleRate = mCore.GetAudioState().sampleRate;
    int fileSampleRate = checkResourceSampleRate (file, true);
    // TODO: Throw exception / return error and trigger warning from editor
    if (fileSampleRate != sampleRate)
    {
        AlertWindow::showMessageBoxAsync(AlertWindow::WarningIcon, "Wrong sample rate",
                                         "Please select a file that matches the project sample rate",
                                         "OK");
        return false;
    }
    
    bool specificDelays;
    auto path = file.getFullPathName().toStdString();
    bool success = isSofaFile(file) ? HRTF::CreateFromSofa (path, mListener, specificDelays)
                                    : HRTF::CreateFrom3dti (path, mListener);
    
    return success;
}

void AnechoicProcessor::addSoundSource (const Common::CVector3& position) {
    auto source = mCore.CreateSingleSourceDSP();
    initSource (source, position);
    
    mSources.push_back (source);
    mTransforms.push_back (Common::CTransform());
    mTransforms.back().SetPosition (Common::CVector3 (1,0,0));
}

void AnechoicProcessor::loadCustomHrtf (String fileTypes, std::function<void(File)> callback)
{
    fc.reset (new FileChooser ("Choose a file to open...",
                               HRTFDirectory(),
                               fileTypes,
                               true));
  
    fc->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles, [callback] (const FileChooser& fc)
    {
        auto results = fc.getURLResults();
        
        if (results.isEmpty())
        {
            callback (File());
            return;
        }
        
        auto result = results.getFirst();
        
        String chosen;
        chosen << (result.isLocalFile() ? result.getLocalFile().getFullPathName()
                 : result.toString (false));

        callback (File (chosen.removeCharacters("\n")));
    });
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
