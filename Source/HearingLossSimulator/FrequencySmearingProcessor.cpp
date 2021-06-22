/**
 * \class FrequencySmearingProcessor
 *
 * \brief Declaration of FrequencySmearingProcessor interface.
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
 * \b Acknowledgement: This project HLS received funding from the European Union's Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.
 */

#include "FrequencySmearingProcessor.h"

static Common::T_ear ears[2] = {Common::T_ear::LEFT, Common::T_ear::RIGHT};

//==============================================================================
template <typename Type>
static bool compare (Type one, Type two)
{
    return (std::abs (one - two) > std::numeric_limits<Type>::epsilon());
}

//==============================================================================
FrequencySmearingProcessor::FrequencySmearingProcessor (HAHLSimulation::CHearingLossSim& sim)
  :  simulator (sim)
{
    StringArray longSuffix = {"left", "right"};
    StringArray shortSuffix = {"L", "R"};
    
    freqSmearLink = new AudioParameterBool ("freq_smear_link",
                                            "Freq Smear Link",
                                            false);
    for (int i = 0; i < 2; i++)
    {
        freqSmearEnabled[i] = new AudioParameterBool ("freq_smear_enabled_" + longSuffix[i],
                                                      "Freq Smear On " + shortSuffix[i],
                                                      false);
        freqSmearType[i] = new AudioParameterBool ("freq_smear_type_" + longSuffix[i],
                                                   "FS Type " + shortSuffix[i],
                                                   false,
                                                   String(),
                                                   [] (bool value, int maximumStringLength) { return value ? "Graf&3DTI" : "Baer&Moore"; },
                                                   [] (const String& text) { return text == "Graf&3DTI"; });
        freqSmearSpectralBroadFactorUp[i] = new AudioParameterFloat ("spectral_broadening_factor_upwards_" + longSuffix[i],
                                                                     "Spctl Broad Up " + shortSuffix[i],
                                                                     NormalisableRange<float> (1.01f, 8.0f),
                                                                     1.01f,
                                                                     String(),
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) { return String (value, 2); },
                                                                     [] (const String& text) { return text.getFloatValue(); });
        freqSmearSpectralBroadFactorDown[i] = new AudioParameterFloat ("spectral_broadening_factor_downwards_" + longSuffix[i],
                                                                       "Spctl Broad Dn " + shortSuffix[i],
                                                                       NormalisableRange<float> (1.01f, 8.0f),
                                                                       1.01f,
                                                                       String(),
                                                                       AudioProcessorParameter::genericParameter,
                                                                       [] (float value, int) { return String (value, 2); },
                                                                       [] (const String& text) { return text.getFloatValue(); });
        freqSmearSpectralFrequencyDown[i] = new AudioParameterFloat ("frequency_smear_frequency_downwards_" + longSuffix[i],
                                                                     "Freq Smear Dn " + shortSuffix[i],
                                                                     NormalisableRange<float> (0.0f, 1000.0f),
                                                                     0.0f,
                                                                     String(),
                                                                     AudioProcessorParameter::genericParameter,
                                                                     [] (float value, int) { return String (value, 1); },
                                                                     [] (const String& text) { return text.getFloatValue(); });
        freqSmearSpectralFrequencyUp[i] = new AudioParameterFloat ("frequency_smear_frequency_upwards_" + longSuffix[i],
                                                                   "Freq Smear Up " + shortSuffix[i],
                                                                   NormalisableRange<float> (0.0f, 1000.0f),
                                                                   0.0f,
                                                                   String(),
                                                                   AudioProcessorParameter::genericParameter,
                                                                   [] (float value, int) { return String (value, 1); },
                                                                   [] (const String& text) { return text.getFloatValue(); });
        freqSmearSpectralBufferSizeDown[i] = new AudioParameterFloat ("frequency_smear_buffer_size_down_" + longSuffix[i],
                                                                      "FS Buf Sz Dn " + shortSuffix[i],
                                                                      NormalisableRange<float> (1.0f, 255.0f),
                                                                      1.0f,
                                                                      String(),
                                                                      AudioProcessorParameter::genericParameter,
                                                                      [] (float value, int) { return String ((int)value); },
                                                                      [] (const String& text) { return text.getFloatValue(); });
        freqSmearSpectralBufferSizeUp[i] = new AudioParameterFloat ("frequency_smear_buffer_size_up_" + longSuffix[i],
                                                                    "FS Buf Sz Up " + shortSuffix[i],
                                                                    NormalisableRange<float> (1.0f, 255.0f),
                                                                    1.0f,
                                                                    String(),
                                                                    AudioProcessorParameter::genericParameter,
                                                                    [] (float value, int) { return String ((int)value); },
                                                                    [] (const String& text) { return text.getFloatValue(); });
        channels.add (new Channel);
    }
}

void FrequencySmearingProcessor::updateSettingsIfNeeded()
{
    for (int ch = 0; ch < getChannels().size(); ch++)
    {
        int channel = (! freqSmearLink->get()) * ch;
        
        if (freqSmearType[channel]->get() == FrequencySmearType::BaerMoore)
        {
            auto baerMooreSmearing = dynamic_pointer_cast<CBaerMooreFrequencySmearing> (simulator.GetFrequencySmearingSimulator (ears[ch]));
            
            if (! baerMooreSmearing)
            {
                simulator.SetFrequencySmearer (ears[ch], channels[ch]->baerMooreSmearing);
                baerMooreSmearing = dynamic_pointer_cast<CBaerMooreFrequencySmearing> (simulator.GetFrequencySmearingSimulator (ears[ch]));
            }
    
            float value = freqSmearSpectralBroadFactorUp[channel]->get();
            if (compare (value, previousSpectralBroadFactorUp[ch]))
            {
                baerMooreSmearing->SetUpwardBroadeningFactor (value);
                previousSpectralBroadFactorUp[ch] = value;
            }
    
            value = freqSmearSpectralBroadFactorDown[channel]->get();
            if (compare (value, previousSpectralBroadFactorDown[ch]))
            {
                baerMooreSmearing->SetDownwardBroadeningFactor (value);
                previousSpectralBroadFactorDown[ch] = freqSmearSpectralBroadFactorDown[channel]->get();
            }
        }
        else
        {
            // Graf & 3dti
            auto graf3dtiSmearing = dynamic_pointer_cast<HAHLSimulation::CGraf3DTIFrequencySmearing> (simulator.GetFrequencySmearingSimulator (ears[ch]));
            
            if (! graf3dtiSmearing)
            {
                simulator.SetFrequencySmearer (ears[ch], channels[ch]->graf3dtiSmearing);
                graf3dtiSmearing = dynamic_pointer_cast<CGraf3DTIFrequencySmearing> (simulator.GetFrequencySmearingSimulator (ears[ch]));
            }
            
            simulator.SetFrequencySmearer (ears[ch], dynamic_pointer_cast<CFrequencySmearing> (channels[ch]->graf3dtiSmearing));
            
            float value = freqSmearSpectralFrequencyDown[channel]->get();
            if (compare (previousSpectralFrequencyDown[ch], value))
            {
                graf3dtiSmearing->SetDownwardSmearing_Hz (value);
                previousSpectralFrequencyDown[ch] = value;
            }
            
            value = freqSmearSpectralFrequencyUp[channel]->get();
            if (compare (previousSpectralFrequencyUp[ch], value))
            {
                graf3dtiSmearing->SetUpwardSmearing_Hz (value);
                previousSpectralFrequencyUp[ch] = value;
            }
            
            value = freqSmearSpectralBufferSizeDown[channel]->get();
            if (compare (previousSpectralBufferSizeDown[ch], value))
            {
                graf3dtiSmearing->SetDownwardSmearingBufferSize ((int)value);
                previousSpectralBufferSizeDown[ch] = value;
            }
            
            value = freqSmearSpectralBufferSizeUp[channel]->get();
            if (compare (previousSpectralBufferSizeUp[ch], value))
            {
                graf3dtiSmearing->SetUpwardSmearingBufferSize ((int)value);
                previousSpectralBufferSizeUp[ch] = value;
            }
        }
        
        if (freqSmearEnabled[channel]->get())
            simulator.EnableFrequencySmearing (ears[ch]);
        else
            simulator.DisableFrequencySmearing (ears[ch]);
    }
}

//==============================================================================
FrequencySmearingProcessor::Channel::Channel()
{
    baerMooreSmearing = make_shared<HAHLSimulation::CBaerMooreFrequencySmearing>();
    graf3dtiSmearing  = make_shared<HAHLSimulation::CGraf3DTIFrequencySmearing>();
}

void FrequencySmearingProcessor::Channel::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    baerMooreSmearing->Setup (samplesPerBlock, (float)sampleRate);
    graf3dtiSmearing->Setup (samplesPerBlock, (float)sampleRate);
}
