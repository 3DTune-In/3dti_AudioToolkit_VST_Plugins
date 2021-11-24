
..\libs\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe --resave ..\3dti_Spatialisation.jucer
..\libs\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe --resave ..\3dti_Anechoic\3dti_Anechoic.jucer
..\libs\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe --resave ..\3dti_Hearing_Aid_Simulator\3dti_Hearing_Aid_Simulator.jucer
..\libs\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe --resave ..\3dti_Hearing_Loss_Simulator\3dti_Hearing_Loss_Simulator.jucer
..\libs\JUCE\extras\Projucer\Builds\VisualStudio2019\x64\Release\App\Projucer.exe --resave ..\3dti_Reverb\3dti_Reverb.jucer

msbuild.exe "..\Builds\VisualStudio2017\3DTI Spatialisation.sln" /p:Configuration=Release /property:Platform=x64 /p:PlatformToolset=v142 /m
msbuild.exe "..\3dti_Anechoic\Builds\VisualStudio2017\3DTI Anechoic Spatialisation.sln" /p:Configuration=Release /property:Platform=x64 /p:PlatformToolset=v142 /m
msbuild.exe "..\3dti_Hearing_Aid_Simulator\Builds\VisualStudio2017\3DTI Hearing Aid Simulator.sln" /p:Configuration=Release /property:Platform=x64 /p:PlatformToolset=v142 /m
msbuild.exe "..\3dti_Hearing_Loss_Simulator\Builds\VisualStudio2017\3DTI Hearing Loss Simulator.sln" /p:Configuration=Release /property:Platform=x64 /p:PlatformToolset=v142 /m
msbuild.exe "..\3dti_Reverb\Builds\VisualStudio2017\3DTI 3D Reverb.sln" /p:Configuration=Release /property:Platform=x64 /p:PlatformToolset=v142 /m

pause