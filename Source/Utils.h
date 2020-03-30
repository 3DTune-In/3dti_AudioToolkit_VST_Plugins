/**
* \class Utils
*
* \brief Declaration of Utils interface.
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

#include <unordered_map>
#include <Common/Buffer.h>
#include <Common/CommonDefinitions.h>
#include <Common/Vector3.h>
#include <HRTF/HRTFFactory.h>
#include <HRTF/HRTFCereal.h>
#include <BRIR/BRIRFactory.h>
#include <BRIR/BRIRCereal.h>
#include "../JuceLibraryCode/JuceHeader.h"

static StringArray BundledHRTFs = {
  "3DTI_HRTF_IRC1008_256s",
  "3DTI_HRTF_IRC1013_256s",
  "3DTI_HRTF_IRC1022_256s",
  "3DTI_HRTF_IRC1031_256s",
  "3DTI_HRTF_IRC1032_256s",
  "3DTI_HRTF_IRC1048_256s",
  "3DTI_HRTF_IRC1053_256s",
  "Load 3DTI",
  "Load SOFA",
};

static StringArray BundledBRIRs = {
  "3DTI_BRIR_small",
  "3DTI_BRIR_medium",
  "3DTI_BRIR_large",
  "Load 3DTI",
  "Load SOFA",
};

static const std::unordered_map<int, String> SampleRateToDefaultHRTF_ILD {
  { 44100, "HRTF_ILD_44100.3dti-ild" },
  { 48000, "HRTF_ILD_48000.3dti-ild" },
  { 96000, "HRTF_ILD_96000.3dti-ild" }
};

static const std::unordered_map<int, String> SampleRateToNearFieldILD {
  { 44100, "NearFieldCompensation_ILD_44100.3dti-ild" },
  { 48000, "NearFieldCompensation_ILD_48000.3dti-ild" },
  { 96000, "NearFieldCompensation_ILD_96000.3dti-ild" }
};

// Returns bundled index of path
static inline int hrtfPathToBundledIndex(const File& path) {
  String fileName = path.getFileNameWithoutExtension();
  auto index = BundledHRTFs.indexOf(fileName.upToLastOccurrenceOf("_", false, false), false);
  if ( index == -1 ) {
    bool isSofa = path.getFileExtension() == "sofa";
    return isSofa ? BundledHRTFs.size()-1 : BundledHRTFs.size()-2;
  }
  return index;
}

static inline int brirPathToBundledIndex(const File& path) {
  String fileName = path.getFileNameWithoutExtension();
  auto index = BundledBRIRs.indexOf(fileName.upToLastOccurrenceOf("_", false, false), false);
  if ( index == -1 ) {
    bool isSofa = path.getFileExtension() == "sofa";
    return isSofa ? BundledHRTFs.size()-1 : BundledHRTFs.size()-2;
  }
  return index;
}

static inline bool isSofaFile(const File& file) {
  return file.getFileExtension() == ".sofa";
}

static inline int checkResourceSampleRate(const File& file, bool isHRTF) {
  auto path = file.getFullPathName().toStdString();
  if ( isHRTF ) {
    return isSofaFile(file) ? HRTF::GetSampleRateFromSofa(path) : HRTF::GetSampleRateFrom3dti(path);
  } else {
    return isSofaFile(file) ? BRIR::GetSampleRateFromSofa(path) : BRIR::GetSampleRateFrom3dti(path);
  }
}

static inline Point<float> azimuthToPoint(float radians) {
  return Point<float>( sinf(radians), cosf(radians) * -1.f );
}

template <typename T>
static Point<T> vecToPoint( Common::CVector3 vec ) {
  return Point<T>( vec.x, vec.y );
}

static constexpr auto &vecToPointf = vecToPoint<float>;

static inline String vectorToString( Common::CVector3& vec ) {
  return String (vec.x) + ", " + String (vec.y) + ", " + String (vec.z);
}

static inline void setLabelStyle( Label& label ) {
  label.setFont (Font (15.0f, Font::plain));
  label.setJustificationType (Justification::centred);
  label.setEditable (false, false, false);
  label.setColour (TextEditor::textColourId, Colours::black);
  label.setColour (TextEditor::backgroundColourId, Colour (0x00000000));
}

static inline float mapElevationToSliderValue(float e) {
  if (e == 0.f || e == 360.f) return 0.f;
  return (e > 270.f) ? jmap<float>(e, 271.f, 360.f, -89.f, 0.f) : e;
}

static inline float mapSliderValueToElevation(float s) {
  return (s < 0.f) ? jmap<float>(s, -89.f, 0.f, 271.f, 360.f) : jmap<float>(s, 0.f, 89.f, 0.0f, 89.f);
}

static inline void mapParameterToSlider( Slider& slider, const AudioParameterInt& parameter, int interval = 1 ) {
  slider.setRange( (double)parameter.getRange().getStart(), (double)parameter.getRange().getEnd(), (double)interval );
  slider.setValue( parameter );
}

static inline void mapParameterToSlider( Slider& slider, const AudioParameterFloat& parameter ) {
  slider.setRange( (double)parameter.range.start, (double)parameter.range.end, (double)parameter.range.interval );
  slider.setValue( parameter );
}

static inline Point<int> getCentre(Component& component) {
  return component.getLocalBounds().getCentre();
}

static inline Point<float> getCentref(Component& component) {
  return component.getLocalBounds().getCentre().toFloat();
}

//
// Audio Utils
//

static inline void _3dti_clear(Common::CEarPair<CMonoBuffer<float>>& buffer) {
  CMonoBuffer<float>& l = buffer.left;
  CMonoBuffer<float>& r = buffer.right;
  for (auto i = 0; i < l.size(); ++i) {
    l[i] = 0.f;
    r[i] = 0.f;
  }
}

//
// File utils
//
static inline bool isWindows() {
#if JUCE_WINDOWS
  return true;
#else
  return false;
#endif
}

static inline juce::File resourceDirectory() {
  if ( isWindows() ) {
    return File::getSpecialLocation(File::userApplicationDataDirectory).getChildFile("eu.3d-tune-in.toolkitplugin");
  }
  return File::getSpecialLocation(File::currentExecutableFile).getParentDirectory().getParentDirectory();
}

static inline juce::File HRTFDirectory() {
  return resourceDirectory().getChildFile(String("Resources/HRTF/"));
}

static inline juce::File BRIRDirectory() {
  return resourceDirectory().getChildFile(String("Resources/BRIR/"));
}

static inline juce::File ILDDirectory() {
  return resourceDirectory().getChildFile(String("Resources/ILD/"));
}

static inline File getBundledHRTF(int index, double sampleRate) {
  index = jlimit(0, BundledHRTFs.size()-3, index);
  auto hrtfName = "3DTI/" + BundledHRTFs[index] + "_" + String(sampleRate) + "hz.3dti-hrtf";
  return HRTFDirectory().getChildFile(hrtfName);
}

static inline File getBundledBRIR(int index, double sampleRate) {
  index = jlimit(0, BundledBRIRs.size()-3, index);
  auto brirName = "3DTI/" + BundledBRIRs[index] + "_" + String(sampleRate) + "Hz.3dti-brir";
  return BRIRDirectory().getChildFile( brirName );
}
