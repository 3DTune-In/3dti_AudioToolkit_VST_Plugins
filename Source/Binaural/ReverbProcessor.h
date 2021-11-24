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

#pragma once

#include <BinauralSpatializer/3DTI_BinauralSpatializer.h>
#include <JuceHeader.h>

//==============================================================================
class ReverbProcessor  : public  ChangeBroadcaster,
                         private Thread,
                         private AsyncUpdater,
                         private AudioProcessorParameter::Listener
{
public:
    //==========================================================================
    /** Constructor */
    ReverbProcessor (Binaural::CCore& core);
    
    /** Destructor */
    ~ReverbProcessor();
    
    /** Initialise internals. Will load the first of bundled BRIRs. */
    void setup (double sampleRate, int samplesPerBlock);
    
    //==========================================================================
    /***/
    void process (AudioBuffer<float>& buffer);
    
    /***/
    void process (AudioBuffer<float>& quadIn, AudioBuffer<float>& stereoOut);
    
    //==========================================================================
    /** Attempts to load a BRIR file.
        
        @see reverbBRIR
     */
    bool loadBRIR (const File& file);
    
    /** @returns the file path of the currently loaded BRIR */
    const File&     getBRIRPath() const { return mBRIRPath;  }
    
    /***/
    float           getPower()     const { return mPower; };
    
    //==========================================================================
    /** Public parameters */
    AudioParameterBool  reverbEnabled;
    AudioParameterFloat reverbLevel;               // ranges from -30 to +6 dB
    AudioParameterInt   reverbOrder;               // ranges from 0 to 2
    
    /** The index of the currently selected BRIR. Setting this will trigger a BRIR reload.
        
        Ranges from 0 to bundled BRIRs + 1, with max value launching a FileChooser process.
     */
    AudioParameterInt   reverbBRIR;
    
    /** @returns the names of bundled BRIRs with "Load File ..." at the end of the array */
    static StringArray getBRIROptions();
    
private:
    //==========================================================================
    /** Thread */
    void run() override;
    
    /** AsyncUpdater */
    void handleAsyncUpdate() override;
    
    /** AudioProcessorParameter::Listener */
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override;
    
    //==========================================================================
    bool doLoadBRIR (const File& file);
    
    void resetBRIRIndex();
    
    void updateParameters();
    
    double getSampleRate();
    
    //==========================================================================
    std::atomic<bool> isLoading {false};
    
    Binaural::CCore& mCore;
    std::shared_ptr<Binaural::CEnvironment> mEnvironment;
    
    File mBRIRPath;
    Array<File, CriticalSection> mBRIRsToLoad;
    std::unique_ptr<FileChooser> fc;
    
    float mPower;
    
    JUCE_DECLARE_WEAK_REFERENCEABLE (ReverbProcessor)
};
