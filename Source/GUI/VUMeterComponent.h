#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace rackstrip
{

class VUMeterComponent : public juce::Component
{
public:
    enum class MeterType
    {
        AnalogVU,
        GainReductionVU,
        GainReductionLED
    };

    VUMeterComponent(MeterType meterType = MeterType::AnalogVU, const juce::String& title = "METER");
    ~VUMeterComponent() override = default;

    void setLevel(float levelDb);
    void paint(juce::Graphics& g) override;

private:
    void drawAnalogVU(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGainReductionVU(juce::Graphics& g, juce::Rectangle<float> bounds);
    void drawGainReduction(juce::Graphics& g, juce::Rectangle<float> bounds);

    MeterType type;
    juce::String titleText;
    float currentLevelDb { 0.0f };
    
    // Ballistic Inertia State
    float needlePosition { 0.0f };
    float needleVelocity { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VUMeterComponent)
};

} // namespace rackstrip
