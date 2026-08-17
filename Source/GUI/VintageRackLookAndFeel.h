#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rackstrip
{

class VintageRackLookAndFeel : public juce::LookAndFeel_V4
{
public:
    VintageRackLookAndFeel();
    ~VintageRackLookAndFeel() override = default;

    struct Colors
    {
        static const juce::Colour faceplateDark;        
        static const juce::Colour faceplateBezel;       
        static const juce::Colour bakeliteCapDark;      
        static const juce::Colour bakeliteCapHighlight; 
        static const juce::Colour aluminumSilver;       
        static const juce::Colour brassAccent;          
        static const juce::Colour goldAmberText;        // #E5C158 Glowing amber/gold readout
        static const juce::Colour silkscreenWhite;      // #F0EAD6 Off-white/cream
        static const juce::Colour pointerMetallic;      
        static const juce::Colour tickMarkColor;        
        static const juce::Colour ledOff;               
        static const juce::Colour ledOnRed;             
        static const juce::Colour ledOnAmber;           
        static const juce::Colour ledOnGreen;           
        static const juce::Colour buttonTextOff;         
        static const juce::Colour buttonTextOn;          
    };

    void drawRotarySlider(juce::Graphics& g,
                          int x, int y, int width, int height,
                          float sliderPosProportional,
                          float rotaryStartAngle, float rotaryEndAngle,
                          juce::Slider& slider) override;

    juce::Label* createSliderTextBox(juce::Slider& slider) override;

    void drawToggleButton(juce::Graphics& g,
                          juce::ToggleButton& button,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted,
                               bool shouldDrawButtonAsDown) override;

    void drawButtonText(juce::Graphics& g,
                        juce::TextButton& button,
                        bool shouldDrawButtonAsHighlighted,
                        bool shouldDrawButtonAsDown) override;

    void drawKnobTickMarks(juce::Graphics& g,
                           juce::Point<float> center,
                           float innerRadius,
                           float outerRadius,
                           float startAngle,
                           float endAngle,
                           int numTicks = 11,
                           int majorTickInterval = 5,
                           juce::Colour tickColor = juce::Colour(0xfff0ebd8),
                           bool isBipolarCenterZero = false,
                           const juce::String& knobType = "");

    void drawStandardBakeliteKnob(juce::Graphics& g,
                                  juce::Rectangle<float> bounds,
                                  float angle,
                                  bool isBrushedAluminum);

    void drawPointerKnob(juce::Graphics& g,
                         juce::Rectangle<float> bounds,
                         float angle);

    void drawSilverSkirtKnob(juce::Graphics& g,
                             juce::Rectangle<float> bounds,
                             float angle,
                             float sliderPosProportional,
                             float startAngle,
                             float endAngle,
                             bool isBipolarCenterZero = false,
                             juce::Colour capAccentColor = juce::Colour(0xffe5a93c));

    void drawDualConcentricKnob(juce::Graphics& g,
                                juce::Rectangle<float> bounds,
                                float angle,
                                bool isInnerConcentric,
                                juce::Slider& slider);

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VintageRackLookAndFeel)
};

} // namespace rackstrip
