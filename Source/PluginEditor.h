#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <map>
#include "PluginProcessor.h"
#include "GUI/FaceplateModuleComponent.h"
#include "GUI/VintageRackLookAndFeel.h"
#include "GUI/VUMeterComponent.h"

class RackStrip500AudioProcessorEditor : public juce::AudioProcessorEditor,
                                         public juce::Timer
{
public:
    explicit RackStrip500AudioProcessorEditor(RackStrip500AudioProcessor&);
    ~RackStrip500AudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void paintOverChildren(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;

private:
    RackStrip500AudioProcessor& audioProcessor;

    rackstrip::VintageRackLookAndFeel vintageLookAndFeel;

    // 4 Authentic 500-Series Channel Strip Modules
    FaceplateModuleComponent preampModule { "PREAMP", FaceplateModuleComponent::ModuleStyle::DarkSteel }; 
    FaceplateModuleComponent gateModule   { "GATE",   FaceplateModuleComponent::ModuleStyle::DarkSteel }; 
    FaceplateModuleComponent eqModule     { "4-BAND EQ", FaceplateModuleComponent::ModuleStyle::ConsoleBlueGrey }; 
    FaceplateModuleComponent compModule   { "1176 DYNAMICS", FaceplateModuleComponent::ModuleStyle::SilverAluminum }; 

    // --- 1. PREAMP CONTROLS ---
    rackstrip::VUMeterComponent inputVuMeter { rackstrip::VUMeterComponent::MeterType::AnalogVU, "INPUT VU" };
    juce::Slider preampHpfSlider;
    juce::Slider preampDriveSlider;
    juce::Slider outputTrimSlider;
    juce::ToggleButton preampLowBumpButton { "LOW BUMP" };
    juce::ToggleButton phaseInvertButton   { "PHASE" };
    juce::ToggleButton analogNoiseButton   { "ANALOG" };
    juce::ToggleButton preampBypassButton  { "BYPASS" };

    // --- 2. NOISE GATE CONTROLS ---
    juce::Slider gateThreshSlider;
    juce::Slider gateRatioSlider;
    juce::Slider gateAttackSlider;
    juce::Slider gateReleaseSlider;
    juce::ToggleButton gateBypassButton { "BYPASS" };

    // --- 3. 4-BAND EQ CONTROLS ---
    // EQ Band Title Headers
    juce::Label eqLowTitleLabel  { "eq_low_title", "LF" };
    juce::Label eqLmfTitleLabel  { "eq_lmf_title", "LMF" };
    juce::Label eqHmfTitleLabel  { "eq_hmf_title", "HMF" };
    juce::Label eqHighTitleLabel { "eq_high_title", "HF" };

    // LF Band
    juce::Slider eqLowGainSlider;
    juce::Slider eqLowFreqSlider;
    juce::ToggleButton eqLowShelfButton { "BELL" };

    // LMF Band
    juce::Slider eqLmfGainSlider;
    juce::Slider eqLmfFreqSlider;
    juce::Slider eqLmfQSlider;

    // HMF Band
    juce::Slider eqHmfGainSlider;
    juce::Slider eqHmfFreqSlider;
    juce::Slider eqHmfQSlider;

    // HF Band
    juce::Slider eqHighGainSlider;
    juce::Slider eqHighFreqSlider;
    juce::ToggleButton eqBypassButton { "BYPASS" };

    // --- 4. 1176 DYNAMICS CONTROLS ---
    rackstrip::VUMeterComponent compGrMeter { rackstrip::VUMeterComponent::MeterType::GainReductionVU, "GAIN REDUCTION" };
    
    // 2x2 Grid of Knobs
    juce::Slider compInputGainSlider;  // Attached to comp_thresh (Compression Drive)
    juce::Slider compOutputGainSlider; // Attached to comp_mix / makeup
    juce::Slider compAttackSlider;     // Attached to comp_attack (1-7)
    juce::Slider compReleaseSlider;    // Attached to comp_release (1-7)

    // Bank of 5 Ratio Push-Buttons
    juce::ToggleButton ratio4Button   { "4:1" };
    juce::ToggleButton ratio8Button   { "8:1" };
    juce::ToggleButton ratio12Button  { "12:1" };
    juce::ToggleButton ratio20Button  { "20:1" };
    juce::ToggleButton ratioAllButton { "ALL" };

    juce::ToggleButton compBypassButton { "BYPASS" };

    // Label & Attachment Storage
    std::map<juce::Slider*, std::unique_ptr<juce::Label>> controlLabels;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;

    std::vector<std::unique_ptr<SliderAttachment>> sliderAttachments;
    std::vector<std::unique_ptr<ButtonAttachment>> buttonAttachments;

    void setupRotaryControl(juce::Slider& slider, const juce::String& paramId, const juce::String& labelText, juce::Component& parent, bool isBipolar = false);
    void setupToggleButton(juce::ToggleButton& button, const juce::String& paramId, juce::Component& parent);
    void setupRatioButtons();
    void layoutRotarySlot(juce::Slider& slider, juce::Rectangle<int> bounds, int labelHeight = 14);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RackStrip500AudioProcessorEditor)
};
