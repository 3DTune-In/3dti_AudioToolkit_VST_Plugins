git submodule update --init --recursive
msbuild.exe ".\libs\3dti_AudioToolkit\3dti_ResourceManager\third_party_libraries\sofacoustics\libsofa\build\win\libsofa.sln" /p:Configuration=Release /property:Platform=x64 /p:PlatformToolset=v142 /m
xcopy .\libs\3dti_AudioToolkit\resources "$env:APPDATA\eu.3d-tune-in.plugins\Resources\" /s
xcopy .\libs\3dti_AudioToolkit\3dti_ResourceManager\third_party_libraries\sofacoustics\libsofa\dependencies\lib\win\x64 "$env:windir\System32\" /s
xcopy .\libs\3dti_AudioToolkit\3dti_ResourceManager\third_party_libraries\sofacoustics\libsofa\lib\libsofa_x64.lib "$env:windir\System32\" /s
pause