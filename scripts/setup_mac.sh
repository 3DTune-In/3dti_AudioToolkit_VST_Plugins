#!/bin/bash

echo "Update submodules"
git submodule update --init --recursive

echo "Building Projucer"
xcodebuild -project "../libs/JUCE/extras/Projucer/Builds/MacOSX/Projucer.xcodeproj" -scheme "Projucer - App" -configuration Release -sdk macosx -jobs 8

echo "Copy resources"
sudo mkdir "/Library/Application Support/eu.3d-tune-in.plugins"
sudo cp -r "../libs/3dti_AudioToolkit/resources" "/Library/Application Support/eu.3d-tune-in.plugins/Resources"