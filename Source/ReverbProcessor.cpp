/**
* \class ReverbProcessor
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
#include "ReverbProcessor.h"

ReverbProcessor::ReverbProcessor (Binaural::CCore& core)
  :  reverbGain ("6", "Reverb Gain", NormalisableRange<float> (-30.f, 6.f, 0.1f), -3.f)
  ,  reverbDistanceAttenuation ("7", "Reverb Distance Attenuation", NormalisableRange<float> (-6.f, 0.f, 0.1f), -3.f)
  ,  mCore (core)
{
    // Environment setup
    mEnvironment = core.CreateEnvironment();
    mEnvironment->SetReverberationOrder(TReverberationOrder::BIDIMENSIONAL);
}

void ReverbProcessor::setup (double sampleRate, int samplesPerBlock)
{
    // Load BRIR
    // If we have an existing section we reload
    // the same BRIR but of new sample rate
    File brir = getBundledBRIR (mBRIRIndex, sampleRate);
    
    if (! brir.existsAsFile())
    {
        DBG ("BRIR file doesn't exist");
        jassertfalse;
        return;
    }
    
    loadBRIR (brir);
}

void ReverbProcessor::process (AudioBuffer<float>& buffer)
{
    if (isLoading)
        return;
    
    auto magnitudes = mCore.GetMagnitudes();
    magnitudes.SetReverbDistanceAttenuation (reverbDistanceAttenuation);
    mCore.SetMagnitudes (magnitudes);
    
    mEnvironment->ProcessVirtualAmbisonicReverb (mOutputBuffer.left,
                                                 mOutputBuffer.right);
    
    auto reverbGain  = Decibels::decibelsToGain(this->reverbGain.get());
    mOutputBuffer.left .ApplyGain(reverbGain);
    mOutputBuffer.right.ApplyGain(reverbGain);
    
    // Fill the output with processed audio
    // Incoming buffer should have two channels
    // for spatialised audio but we check just in case
    int numChannels = std::max (buffer.getNumChannels(), 2);
    int numSamples  = buffer.getNumSamples();
    
    for ( int i = 0; i < numSamples; i++ ) {
        switch (numChannels) {
            case 2:
                buffer.getWritePointer(1)[i] = mOutputBuffer.right[i];
            default:
                buffer.getWritePointer(0)[i] = mOutputBuffer.left[i];
        }
    }
    
    mPower = mOutputBuffer.left.GetPower();
}

bool ReverbProcessor::loadBRIR (int bundledIndex)
{
    return loadBRIR (getBundledBRIR (bundledIndex, getSampleRate()));
}

bool ReverbProcessor::loadBRIR (const File& file)
{
    isLoading = true;
    
    DBG ("Loading BRIR: " << file.getFullPathName());
    
    int fileSampleRate = checkResourceSampleRate (file, false);
    // TODO: Throw exception / return error and trigger warning from editor
    if (fileSampleRate != getSampleRate())
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                          "Wrong sample rate",
                                          "Please select a file that matches the project sample rate",
                                          "OK");
        return false;
    }
    
    bool isSofa = isSofaFile (file);
    auto path   = file.getFullPathName().toStdString();
    
    bool success = false;
    
    if (isSofa ? BRIR::CreateFromSofa (path, mEnvironment)
               : BRIR::CreateFrom3dti (path, mEnvironment))
    {
        mBRIRIndex = brirPathToBundledIndex (file);
        mBRIRPath  = file;
        
        success = true;
    }
    
    isLoading = false;
    
    return success;
}

double ReverbProcessor::getSampleRate()
{
    return mCore.GetAudioState().sampleRate;
}
