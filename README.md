# 3D Tune-In Toolkit VST plugin

This repository includes a series of VST interfaces for the 3D Tune-In (3DTI) Toolkit, so that its functionalities can be integrated into a DAW application. The current version of this VST plugins includes five different plugins, full binaural spatialisation (anechoic + reverb in the same plugin), anechoic binaural spatialization, binaural reverb, hearing loss and hearing aid simulators.
For technical details on the spatialisation features, please refer to:

* Picinali, L., Hrafnkelsson, R., & Reyes-Lecuona, A. (2019, March). The 3D Tune-In Toolkit VST Binaural Audio Plugin. In Audio Engineering Society Conference: 2019 AES International Conference on Immersive and Interactive Audio. Audio Engineering Society. 

The 3DTI Toolkit is a standard C++ library for audio spatialisation and simulation using headphones developed within the 3D Tune-In (3DTI) project (http://www.3d-tune-in.eu), which aims at using 3D sound and simulating hearing loss and hearing aids within virtual environments and games. The Toolkit allows the design and rendering of highly realistic and immersive 3D audio, and the simulation of virtual hearing aid devices and of different typologies of hearing loss.

More information about the 3D Tune-In Toolkit can be found in the open-source GitHub repository at https://github.com/3DTune-In/3dti_AudioToolkit/. Technical details about the 3D Tune-In Toolkit spatialiser are described in: 

* Cuevas-Rodríguez M, Picinali L, González-Toledo D, Garre C, de la Rubia-Cuestas E, Molina-Tanco L and Reyes-Lecuona A. (2019) 3D Tune-In Toolkit: An open-source library for real-time binaural spatialisation. PLOS ONE 14(3): e0211899. https://doi.org/10.1371/journal.pone.0211899


**The structure of the repository is as follows:**
```
3dti_AudioToolkit_VST_Plugins
├── 3dti_Anechoic
├── 3dti_Hearing_Aid_Simulator
├── 3dti_Hearing_Loss_Simulator
├── 3dti_Reverb
├── Resources
├── Source
├── docs/images
├── extras
├── libs
├── scripts
└── 3dti_Spatialisation.jucer
```

## How to build
Start by running the setup script for your platform `scripts/setup_win.bat` or `scripts\setup_mac.sh`. This will initialise the submodules, build the dependencies and copy the required resources into the right place. Note that you'll need to have `MSBuild (Win)` or `Xcode (Mac)` and in your path. Once this is complete, you can use the provided build scripts in the same directory to make a release build of all the plugins. Alternatively, you can open the provided Projucer projects and generate a project for Xcode or Visual Studio. The 3DTI Plugins are built with JUCE, which is included as a submodule in `libs/JUCE`. The Projucer is JUCE's project generator and can be built from source in that directory, or downloaded separately from [juce.com](https://www.juce.com/).

## How to install
Build from source (see section above) or download prebuilt binaries from the [Releases](https://github.com/3DTune-In/3dti_AudioToolkit_VST_Plugins/releases) section, available in AU / VST2 / VST3 formats for Windows and MacOS.

## DAWs
The 3dti_audiotoolkit_vst has been tested on the following DAWs:
- Audacity (Win/Mac)
- Reaper (Win/Mac)
- Ableton Live 11 (Mac)
- Logic Pro (Mac)
- Max 8 (Win/Mac)

## Copyright and License

The 3D Tune-In Toolkit VST plugin, Copyright (c) 2019 Imperial College London and University of Malaga, is distributed as open source under GPLv3. See [LICENSE](LICENSE) for further information.

## Credits

This software was developed by a team coordinated by 
-	Lorenzo Picinali ([Imperial College London](https://www.imperial.ac.uk/)). Contact: l.picinali@imperial.ac.uk 
-	Arcadio Reyes-Lecuona ([University of Malaga](https://www.uma.es/)). Contact: areyes@uma.es  

The 3D Tune-In Toolkit VST plugin was developed by: 
- [Ragnar Hrafnkelsson](https://github.com/ragnaringi)

## Acknowledgements

![European Union](docs/images/EU_flag.png "European Union") This project has received funding from the European Union’s Horizon 2020 research and innovation programme under grant agreements No 644051 and 726765.

We would also like to acknowledge Angel Rodríguez-Rivero for his help in testing the first release of this software. 
