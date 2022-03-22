#!/bin/bash

echo "Deleting build products"
rm -rf ../JuceLibraryCode
rm -rf ../Builds/MacOSX/build/Release
rm -rf ../3dti_Anechoic/JuceLibraryCode
rm -rf ../3dti_Anechoic/Builds/MacOSX/build/Release
rm -rf ../3dti_Hearing_Aid_Simulator/JuceLibraryCode
rm -rf ../3dti_Hearing_Aid_Simulator/Builds/MacOSX/build/Release
rm -rf ../3dti_Hearing_Loss_Simulator/JuceLibraryCode
rm -rf ../3dti_Hearing_Loss_Simulator/Builds/MacOSX/build/Release
rm -rf ../3dti_Reverb/JuceLibraryCode
rm -rf ../3dti_Reverb/Builds/MacOSX/build/Release

echo "Build Projucer"
xcodebuild -project ../libs/JUCE/extras/Projucer/Builds/MacOSX/Projucer.xcodeproj -scheme "Projucer - App" -configuration Release -jobs 8

echo "Generate Xcode projects"
../libs/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer --resave ../3dti_Spatialisation.jucer
../libs/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer --resave ../3dti_Anechoic/3dti_Anechoic.jucer
../libs/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer --resave ../3dti_Hearing_Aid_Simulator/3dti_Hearing_Aid_Simulator.jucer
../libs/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer --resave ../3dti_Hearing_Loss_Simulator/3dti_Hearing_Loss_Simulator.jucer
../libs/JUCE/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer --resave ../3dti_Reverb/3dti_Reverb.jucer

echo "Building Xcode projects"
xcodebuild -project "../Builds/MacOSX/3DTI Spatialisation.xcodeproj" -scheme "3DTI Spatialisation - All" -configuration Release -sdk macosx -jobs 8
xcodebuild -project "../3dti_Anechoic/Builds/MacOSX/3DTI Anechoic Spatialisation.xcodeproj" -scheme "3DTI Anechoic Spatialisation - All" -configuration Release -sdk macosx -jobs 8
xcodebuild -project "../3dti_Hearing_Aid_Simulator/Builds/MacOSX/3DTI Hearing Aid Simulator.xcodeproj" -scheme "3DTI Hearing Aid Simulator - All" -configuration Release -sdk macosx -jobs 8
xcodebuild -project "../3dti_Hearing_Loss_Simulator/Builds/MacOSX/3DTI Hearing Loss Simulator.xcodeproj" -scheme "3DTI Hearing Loss Simulator - All" -configuration Release -sdk macosx -jobs 8
xcodebuild -project "../3dti_Reverb/Builds/MacOSX/3DTI 3D Reverb.xcodeproj" -scheme "3DTI 3D Reverb - All" -configuration Release -sdk macosx -jobs 8
