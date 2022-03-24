/**
* \class ReverbProcessor
*
* \brief Declaration of Toolkit3dtiProcessor interface.
* \date  November 2021
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

#include <BRIR/BRIRFactory.h>
#include <BRIR/BRIRCereal.h>
#include "Utils.h"
#include "ReverbProcessor.h"

//==============================================================================
ReverbProcessor::ReverbProcessor (Binaural::CCore& core)
  :  reverbEnabled ("Reverb Enabled", "Reverb Enabled", true)
  ,  reverbLevel ("Reverb Level", "Reverb Level", NormalisableRange<float> (-30.f, 6.f, 0.1f), -3.f)
  ,  reverbOrder ("Reverb Order", "Reverb Order", 0, 2, 1)
  ,  reverbBRIR ("Reverb BRIR", "Reverb BRIR", 0, BundledBRIRs.size() + 1, 0)
  ,  mCore (core)
{
    // Environment setup
    mEnvironment = core.CreateEnvironment();
    mEnvironment->SetReverberationOrder (TReverberationOrder::BIDIMENSIONAL);
    
    reverbBRIR.addListener (this);
}

ReverbProcessor::~ReverbProcessor()
{
    reverbBRIR.removeListener (this);
}

//==============================================================================
void ReverbProcessor::setup (double sampleRate, int samplesPerBlock)
{
    loadBRIR (getBundledBRIR (reverbBRIR.get(), sampleRate));
}

//==============================================================================
void ReverbProcessor::process (AudioBuffer<float>& buffer)
{
    if (isLoading.load() || ! reverbEnabled.get())
    {
        buffer.clear();
        return;
    }
    
    updateParameters();
    
    Common::CEarPair<CMonoBuffer<float>> outputBuffer;
    mEnvironment->ProcessVirtualAmbisonicReverb (outputBuffer.left,
                                                 outputBuffer.right);
    
    // If BRIR is not loaded, buffer will be set to zero
    if (outputBuffer.left.size() == 0)
    {
        buffer.clear();
        return;
    }
    
    // Fill the output with processed audio
    // Incoming buffer should have two channels
    // for spatialised audio
    jassert (buffer.getNumChannels() >= 2);
    
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        buffer.getWritePointer(0)[i] = outputBuffer.left[i];
        buffer.getWritePointer(1)[i] = outputBuffer.right[i];
    }
    
    auto reverbGain  = Decibels::decibelsToGain (reverbLevel.get());
    buffer.applyGain (reverbGain);
    
    mPower = outputBuffer.left.GetPower();
}

void ReverbProcessor::process (AudioBuffer<float>& quadIn, AudioBuffer<float>& stereoOut)
{
    if (isLoading.load() || ! reverbEnabled.get())
    {
        stereoOut.clear();
        return;
    }
    
    jassert (stereoOut.getNumChannels() == 2);
    
    updateParameters();
    
    int numSamples = stereoOut.getNumSamples();
    
    CMonoBuffer<float> input (numSamples);
    
    for (int ch = 0; ch < quadIn.getNumChannels(); ch++)
    {
        // Fill input buffer with incoming audio
        std::memcpy (input.data(), quadIn.getReadPointer (ch), numSamples*sizeof(float));
        
        CStereoBuffer<float> outputBuffer;
        
        mEnvironment->ProcessEncodedChannelReverb (TBFormatChannel(ch), input, outputBuffer);

        // Fill the output with processed audio
        jassert (outputBuffer.GetNChannels() == 2);
        
        auto outLeft  = outputBuffer.GetMonoChannel (0);
        auto outRight = outputBuffer.GetMonoChannel (1);
        
        numSamples = (int)outputBuffer.GetNsamples();

        jassert (numSamples <= stereoOut.getNumSamples());
        
        for (int i = 0; i < numSamples; i++)
        {
            stereoOut.getWritePointer(0)[i] += outLeft[i];
            stereoOut.getWritePointer(1)[i] += outRight[i];
        }
    }
    
    auto reverbGain = Decibels::decibelsToGain (reverbLevel.get());
    stereoOut.applyGain (reverbGain);
    
    mPower = stereoOut.getRMSLevel (0, 0, numSamples);
}

//==============================================================================
StringArray ReverbProcessor::getBRIROptions()
{
    return {"Small", "Medium", "Large", "Library", "Trapezoid", "Load File ..."};
}

//==============================================================================
void ReverbProcessor::handleAsyncUpdate()
{
    fc.reset (new FileChooser ("Choose a file to open...",
                               BRIRDirectory(),
                               "*.sofa;*.3dti-brir",
                               true));
  
    const WeakReference<ReverbProcessor> safePointer (this);
    
    fc->launchAsync (FileBrowserComponent::openMode, [this, safePointer] (const FileChooser& f)
    {
        if (safePointer.wasObjectDeleted())
            return;
        
        auto results = f.getURLResults();
        
        if (results.isEmpty())
        {
            resetBRIRIndex();
            
            return;
        }
        
        loadBRIR (results.getFirst().getLocalFile());
    });
}

//==============================================================================
void ReverbProcessor::parameterValueChanged (int parameterIndex, float newValue)
{
    int index = roundToInt (reverbBRIR.convertFrom0to1 (newValue));
    
    if (index == reverbBRIR.getRange().getEnd() - 1)
    {
        // The last parameter value is reserved for custom BRIRs. In this case
        // we trigger and async update to launch the FileChooser menu
        triggerAsyncUpdate();
    }
    else
    {
        loadBRIR (getBundledBRIR (index, getSampleRate()));
    }
}

void ReverbProcessor::parameterGestureChanged (int parameterIndex, bool gestureIsStarting)
{
}

//==============================================================================
bool ReverbProcessor::loadBRIR (const File& file)
{
    isLoading.store (true);
    
    DBG ("Loading BRIR: " << file.getFullPathName());
    
    if (! file.existsAsFile())
    {
        DBG ("BRIR file doesn't exist");
            
        resetBRIRIndex();
        
        isLoading.store (false);
        
        return false;
    }
    
    int fileSampleRate = checkResourceSampleRate (file, false);
    
    if (fileSampleRate != getSampleRate())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Wrong sample rate",
                                          "Please select a file that matches the project sample rate",
                                          "OK");
        resetBRIRIndex();
        
        isLoading.store (false);
       
        return false;
    }
    
    bool isSofa = isSofaFile (file);
    auto path   = file.getFullPathName().toStdString();
    
    bool success = false;
    
    if (isSofa ? BRIR::CreateFromSofa (path, mEnvironment)
               : BRIR::CreateFrom3dti (path, mEnvironment))
    {
        mBRIRPath = file;
        
        resetBRIRIndex();
        
        success = true;
    }
    
    isLoading.store (false);
    isLoaded.store (true);
    
    return success;
}

//==============================================================================
void ReverbProcessor::resetBRIRIndex()
{
    int selectedIndex = brirPathToBundledIndex (getBRIRPath());
    
    if (selectedIndex >= 0)
        reverbBRIR = selectedIndex;
    else
        reverbBRIR = reverbBRIR.getRange().getEnd() - 1;
    
    sendChangeMessage();
}

//==============================================================================
void ReverbProcessor::updateParameters()
{
    mEnvironment->SetReverberationOrder (TReverberationOrder (reverbOrder.get()));
}

//==============================================================================
double ReverbProcessor::getSampleRate()
{
    return mCore.GetAudioState().sampleRate;
}
