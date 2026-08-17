#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class FaceplateModuleComponent : public juce::Component
{
public:
    enum class ModuleStyle
    {
        DarkSteel,       // Preamp / Gate: Dark anodized steel (#16181A to #22252A)
        SilverAluminum,  // 1176 Compressor: Brushed silver-aluminum (#C2C6CA to #DEE2E6)
        ConsoleBlueGrey, // EQ Band: Vintage console blue-grey (#263947 to #344C5E)
        Custom
    };

    FaceplateModuleComponent(const juce::String& moduleTitle = "MODULE",
                             ModuleStyle style = ModuleStyle::DarkSteel,
                             bool shouldShowScrews = true);

    FaceplateModuleComponent(const juce::String& moduleTitle,
                             juce::Colour baseColour,
                             bool shouldShowScrews = true);

    ~FaceplateModuleComponent() override = default;

    void paint(juce::Graphics& g) override;
    void resized() override;

    juce::Rectangle<int> getContentBounds() const;

    void setModuleTitle(const juce::String& title);
    void setModuleStyle(ModuleStyle style);
    void setBaseColour(juce::Colour colour);

    ModuleStyle getModuleStyle() const { return moduleStyle; }

private:
    void drawBrushedMetalBackground(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawRackScrew(juce::Graphics& g, juce::Point<float> center, float radius, float angleDegrees = 45.0f);
    void drawEngravedHeader(juce::Graphics& g, juce::Rectangle<float> headerBounds);

    juce::String titleText;
    ModuleStyle moduleStyle { ModuleStyle::DarkSteel };
    juce::Colour baseColour { juce::Colour(0xff16181a) };
    juce::Colour secondaryColour { juce::Colour(0xff22252a) };
    juce::Colour headerTextColour { juce::Colour(0xffdcdfe2) };
    bool showScrews { true };

    void updateColorsForStyle();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FaceplateModuleComponent)
};
