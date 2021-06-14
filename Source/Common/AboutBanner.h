
#pragma once

#include <JuceHeader.h>

//==============================================================================
/*
*/
class AboutBanner : public Component
{
public:
    AboutBanner()
    {
        setOpaque (true);
        
        _3dTuneInLogo = ImageCache::getFromMemory(BinaryData::_3DTI_Logo_png, BinaryData::_3DTI_Logo_pngSize);
        imperialLogo = ImageCache::getFromMemory(BinaryData::Imperial_Logo_png, BinaryData::Imperial_Logo_pngSize);
        umaLogo = ImageCache::getFromMemory(BinaryData::UMA_Logo_png, BinaryData::UMA_Logo_pngSize);
        
        button.setButtonText("About");
        addAndMakeVisible( button );
        
        toolkitVersionLabel.setFont(Font(15.f, Font::plain));
        toolkitVersionLabel.setText("Version " +  String(JucePlugin_VersionString) + " (3DTI Toolkit v1.4)", dontSendNotification);
        addAndMakeVisible (toolkitVersionLabel);
    }

    ~AboutBanner()
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colour (24, 31, 34));
        
        auto r = getLocalBounds().reduced (0, 4).toFloat();
        auto size = r.proportionOfWidth (0.17);
        g.drawImage (umaLogo, r.removeFromRight (size).reduced (2), RectanglePlacement::centred);
        g.drawImage (imperialLogo, r.removeFromRight (size).reduced (2), RectanglePlacement::centred);
        g.drawImage (_3dTuneInLogo, r.removeFromRight (size).reduced (2), RectanglePlacement::centred);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        button.setBounds (r.removeFromLeft (80).reduced (12));
        toolkitVersionLabel.setBounds (r);
    }

    TextButton button;
    
private:
    Label toolkitVersionLabel;
    
    Image _3dTuneInLogo;
    Image imperialLogo;
    Image umaLogo;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutBanner)
};
