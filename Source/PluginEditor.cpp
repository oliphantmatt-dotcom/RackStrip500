#include "PluginProcessor.h"
#include "PluginEditor.h"

RackStrip500AudioProcessorEditor::RackStrip500AudioProcessorEditor (RackStrip500AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setLookAndFeel(&vintageLookAndFeel);

    // Setup 4 Channel Strip Modules
    addAndMakeVisible(preampModule);
    addAndMakeVisible(gateModule);
    addAndMakeVisible(eqModule);
    addAndMakeVisible(compModule);

    // =========================================================================
    // 1. PREAMP CONTROLS
    // =========================================================================
    preampModule.addAndMakeVisible(inputVuMeter);
    setupRotaryControl(preampHpfSlider, "preamp_hpf_freq", "HPF FREQ", preampModule);
    setupRotaryControl(preampDriveSlider, "preamp_drive", "DRIVE", preampModule);
    setupRotaryControl(outputTrimSlider, "out_trim", "OUT TRIM", preampModule, true);

    setupToggleButton(preampLowBumpButton, "preamp_low_bump", preampModule);
    setupToggleButton(phaseInvertButton, "out_phase_invert", preampModule);
    setupToggleButton(analogNoiseButton, "analog_noise", preampModule);
    setupToggleButton(preampBypassButton, "preamp_bypass", preampModule);

    // =========================================================================
    // 2. NOISE GATE CONTROLS
    // =========================================================================
    setupRotaryControl(gateThreshSlider, "gate_thresh", "THRESHOLD", gateModule);
    setupRotaryControl(gateRatioSlider, "gate_ratio", "RATIO", gateModule);
    setupRotaryControl(gateAttackSlider, "gate_attack", "ATTACK", gateModule);
    setupRotaryControl(gateReleaseSlider, "gate_release", "RELEASE", gateModule);
    setupToggleButton(gateBypassButton, "gate_bypass", gateModule);

    // =========================================================================
    // 3. 4-BAND EQ CONTROLS
    // =========================================================================
    // Band Title Headers
    juce::Label* eqTitles[] = { &eqLowTitleLabel, &eqLmfTitleLabel, &eqHmfTitleLabel, &eqHighTitleLabel };
    for (auto* t : eqTitles)
    {
        t->setFont(juce::Font("Helvetica Neue", 12.5f, juce::Font::bold));
        t->setJustificationType(juce::Justification::centred);
        t->setColour(juce::Label::textColourId, rackstrip::VintageRackLookAndFeel::Colors::silkscreenWhite);
        eqModule.addAndMakeVisible(t);
    }

    // LF Band
    setupRotaryControl(eqLowGainSlider, "eq_low_gain", "GAIN", eqModule, true);
    setupRotaryControl(eqLowFreqSlider, "eq_low_freq", "FREQ", eqModule);
    eqModule.addAndMakeVisible(eqLowShelfButton);

    // LMF Band
    setupRotaryControl(eqLmfGainSlider, "eq_lmf_gain", "GAIN", eqModule, true);
    setupRotaryControl(eqLmfFreqSlider, "eq_lmf_freq", "FREQ", eqModule);
    setupRotaryControl(eqLmfQSlider, "eq_lmf_q", "Q", eqModule);

    // HMF Band
    setupRotaryControl(eqHmfGainSlider, "eq_hmf_gain", "GAIN", eqModule, true);
    setupRotaryControl(eqHmfFreqSlider, "eq_hmf_freq", "FREQ", eqModule);
    setupRotaryControl(eqHmfQSlider, "eq_hmf_q", "Q", eqModule);

    // HF Band
    setupRotaryControl(eqHighGainSlider, "eq_high_gain", "GAIN", eqModule, true);
    setupRotaryControl(eqHighFreqSlider, "eq_high_freq", "FREQ", eqModule);
    setupToggleButton(eqBypassButton, "eq_bypass", eqModule);

    // =========================================================================
    // 4. 1176 DYNAMICS CONTROLS
    // =========================================================================
    compModule.addAndMakeVisible(compGrMeter);

    // 2x2 Grid of 1176 Knobs
    setupRotaryControl(compInputGainSlider, "comp_thresh", "INPUT", compModule);
    setupRotaryControl(compOutputGainSlider, "comp_mix", "OUTPUT", compModule);
    setupRotaryControl(compAttackSlider, "comp_attack", "ATTACK", compModule);
    setupRotaryControl(compReleaseSlider, "comp_release", "RELEASE", compModule);

    setupRatioButtons();
    setupToggleButton(compBypassButton, "comp_bypass", compModule);

    // Window Constraints & Persistence: Read from APVTS state tree with default 1080x640
    int savedWidth  = audioProcessor.getAPVTS().state.getProperty ("editorWidth", 1080);
    int savedHeight = audioProcessor.getAPVTS().state.getProperty ("editorHeight", 640);

    setResizable (true, true);
    setResizeLimits (980, 580, 1920, 1080);
    setSize (savedWidth, savedHeight);

    startTimerHz(30);
}

RackStrip500AudioProcessorEditor::~RackStrip500AudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void RackStrip500AudioProcessorEditor::setupRotaryControl(juce::Slider& slider,
                                                         const juce::String& paramId,
                                                         const juce::String& labelText,
                                                         juce::Component& parent,
                                                         bool isBipolar)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 48, 16);
    slider.setPopupDisplayEnabled(false, false, this);
    slider.setComponentID(paramId);

    if (isBipolar)
        slider.getProperties().set("isBipolar", true);

    if (paramId.startsWith("eq_"))
        slider.getProperties().set("knobStyle", "silverSkirt");
    else if (paramId.startsWith("comp_"))
        slider.getProperties().set("knobStyle", "pointer");

    // Explicit Unit Text Formatting for Value Readout Box
    if (paramId.contains("gain") || paramId.contains("trim") || paramId.contains("drive"))
    {
        slider.textFromValueFunction = [](double val) {
            juce::String prefix = (val > 0.05) ? "+" : "";
            return prefix + juce::String(val, 1) + " dB";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            return txt.removeCharacters(" dB+").getDoubleValue();
        };
    }
    else if (paramId.contains("freq"))
    {
        slider.textFromValueFunction = [](double val) {
            if (val >= 1000.0)
                return juce::String(val / 1000.0, 1) + " kHz";
            return juce::String(juce::roundToInt(val)) + " Hz";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            if (txt.containsIgnoreCase("k"))
                return txt.removeCharacters(" kHzHz").getDoubleValue() * 1000.0;
            return txt.removeCharacters(" Hz").getDoubleValue();
        };
    }
    else if (paramId.contains("attack") || paramId.contains("release"))
    {
        slider.textFromValueFunction = [](double val) {
            if (val < 10.0)
                return juce::String(val, 2) + " ms";
            return juce::String(juce::roundToInt(val)) + " ms";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            return txt.removeCharacters(" ms").getDoubleValue();
        };
    }
    else if (paramId.contains("thresh"))
    {
        slider.textFromValueFunction = [](double val) {
            return juce::String(val, 1) + " dB";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            return txt.removeCharacters(" dB").getDoubleValue();
        };
    }
    else if (paramId.contains("ratio"))
    {
        slider.textFromValueFunction = [](double val) {
            return juce::String(val, 1) + ":1";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            return txt.removeCharacters(" :1").getDoubleValue();
        };
    }
    else if (paramId.contains("_q"))
    {
        slider.textFromValueFunction = [](double val) {
            return juce::String(val, 2) + " Q";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            return txt.removeCharacters(" Q").getDoubleValue();
        };
    }
    else if (paramId.contains("mix"))
    {
        slider.textFromValueFunction = [](double val) {
            return juce::String(juce::roundToInt(val)) + "%";
        };
        slider.valueFromTextFunction = [](const juce::String& txt) {
            return txt.removeCharacters(" %").getDoubleValue();
        };
    }

    parent.addAndMakeVisible(slider);
    sliderAttachments.push_back(std::make_unique<SliderAttachment>(audioProcessor.getAPVTS(), paramId, slider));

    // High-Contrast Bold Header Label
    auto label = std::make_unique<juce::Label>("", labelText);
    label->setJustificationType(juce::Justification::centred);
    label->setFont(juce::Font("Helvetica Neue", 11.0f, juce::Font::bold));

    juce::Colour textCol = rackstrip::VintageRackLookAndFeel::Colors::silkscreenWhite;
    if (paramId.startsWith("comp_"))
        textCol = juce::Colour(0xff141619); // High contrast dark silkscreen on silver aluminum faceplate

    label->setColour(juce::Label::textColourId, textCol);
    parent.addAndMakeVisible(label.get());
    controlLabels[&slider] = std::move(label);
}

void RackStrip500AudioProcessorEditor::setupToggleButton(juce::ToggleButton& button,
                                                        const juce::String& paramId,
                                                        juce::Component& parent)
{
    parent.addAndMakeVisible(button);
    buttonAttachments.push_back(std::make_unique<ButtonAttachment>(audioProcessor.getAPVTS(), paramId, button));
}

void RackStrip500AudioProcessorEditor::setupRatioButtons()
{
    juce::ToggleButton* ratioBtns[] = { &ratio4Button, &ratio8Button, &ratio12Button, &ratio20Button, &ratioAllButton };
    const float ratioValues[] = { 4.0f, 8.0f, 12.0f, 20.0f, 20.0f };

    for (int i = 0; i < 5; ++i)
    {
        auto* btn = ratioBtns[i];
        compModule.addAndMakeVisible(btn);
        btn->setRadioGroupId(101); // True mutually exclusive radio group

        bool isAll = (i == 4);
        float val = ratioValues[i];

        btn->onClick = [this, isAll, val] {
            if (auto* allParam = audioProcessor.getAPVTS().getParameter("comp_all_buttons"))
                allParam->setValueNotifyingHost(isAll ? 1.0f : 0.0f);

            if (auto* ratioParam = audioProcessor.getAPVTS().getParameter("comp_ratio"))
                ratioParam->setValueNotifyingHost(ratioParam->convertTo0to1(val));
        };
    }

    // Initial state sync
    bool isAllActive = false;
    if (auto* allParam = audioProcessor.getAPVTS().getRawParameterValue("comp_all_buttons"))
        isAllActive = (allParam->load() > 0.5f);

    if (isAllActive)
    {
        ratioAllButton.setToggleState(true, juce::dontSendNotification);
    }
    else
    {
        float currentRatio = audioProcessor.getAPVTS().getRawParameterValue("comp_ratio")->load();
        if (currentRatio <= 4.5f)       ratio4Button.setToggleState(true, juce::dontSendNotification);
        else if (currentRatio <= 8.5f)  ratio8Button.setToggleState(true, juce::dontSendNotification);
        else if (currentRatio <= 12.5f) ratio12Button.setToggleState(true, juce::dontSendNotification);
        else                            ratio20Button.setToggleState(true, juce::dontSendNotification);
    }
}

void RackStrip500AudioProcessorEditor::layoutRotarySlot(juce::Slider& slider, juce::Rectangle<int> bounds, int labelHeight)
{
    auto labelBounds = bounds.removeFromTop(labelHeight);
    if (controlLabels.count(&slider) && controlLabels[&slider] != nullptr)
        controlLabels[&slider]->setBounds(labelBounds);

    bounds.removeFromTop(2);
    slider.setBounds(bounds);
}

void RackStrip500AudioProcessorEditor::resized()
{
    // Update persisted window dimensions in APVTS state tree
    audioProcessor.getAPVTS().state.setProperty ("editorWidth", getWidth(), nullptr);
    audioProcessor.getAPVTS().state.setProperty ("editorHeight", getHeight(), nullptr);

    auto area = getLocalBounds();
    const int totalWidth = area.getWidth();

    // Proportional Module Allocation: PREAMP (22%), GATE (20%), 4-BAND EQ (34%), 1176 DYNAMICS (24%)
    const int preampWidth = juce::roundToInt(totalWidth * 0.22f);
    const int gateWidth   = juce::roundToInt(totalWidth * 0.20f);
    const int eqWidth     = juce::roundToInt(totalWidth * 0.34f);
    const int compWidth   = totalWidth - preampWidth - gateWidth - eqWidth;

    preampModule.setBounds(area.removeFromLeft(preampWidth));
    gateModule.setBounds(area.removeFromLeft(gateWidth));
    eqModule.setBounds(area.removeFromLeft(eqWidth));
    compModule.setBounds(area.removeFromLeft(compWidth));

    // =========================================================================
    // 1. PREAMP MODULE LAYOUT (No overlap, clean dedicated vertical slots)
    // =========================================================================
    {
        auto pBounds = preampModule.getContentBounds();
        
        // Slot 1: Input VU Meter
        inputVuMeter.setBounds(pBounds.removeFromTop(94).reduced(4, 2));
        pBounds.removeFromTop(10);

        // 3 Rotary Knobs with 8px vertical spacing
        const int buttonGridH = 54;
        const int remainingKnobH = pBounds.getHeight() - buttonGridH - 24;
        const int singleSlotH = remainingKnobH / 3;

        layoutRotarySlot(preampHpfSlider, pBounds.removeFromTop(singleSlotH).reduced(6, 0));
        pBounds.removeFromTop(8);
        layoutRotarySlot(preampDriveSlider, pBounds.removeFromTop(singleSlotH).reduced(6, 0));
        pBounds.removeFromTop(8);
        layoutRotarySlot(outputTrimSlider, pBounds.removeFromTop(singleSlotH).reduced(6, 0));

        pBounds.removeFromTop(10);

        // 2x2 Grid of Push-Buttons
        auto btnRow1 = pBounds.removeFromTop(22).reduced(2, 0);
        pBounds.removeFromTop(4);
        auto btnRow2 = pBounds.removeFromTop(22).reduced(2, 0);

        const int halfW = btnRow1.getWidth() / 2;
        preampLowBumpButton.setBounds(btnRow1.removeFromLeft(halfW).reduced(2, 0));
        phaseInvertButton.setBounds(btnRow1.reduced(2, 0));

        analogNoiseButton.setBounds(btnRow2.removeFromLeft(halfW).reduced(2, 0));
        preampBypassButton.setBounds(btnRow2.reduced(2, 0));
    }

    // =========================================================================
    // 2. NOISE GATE MODULE LAYOUT (Strictly Non-Overlapping Slots)
    // =========================================================================
    {
        auto gBounds = gateModule.getContentBounds();
        gBounds.removeFromTop(6);

        const int gBtnH = 26;
        const int gKnobAreaH = gBounds.getHeight() - gBtnH - 32;
        const int gSlotH = gKnobAreaH / 4;

        layoutRotarySlot(gateThreshSlider, gBounds.removeFromTop(gSlotH).reduced(6, 0));
        gBounds.removeFromTop(8);
        layoutRotarySlot(gateRatioSlider, gBounds.removeFromTop(gSlotH).reduced(6, 0));
        gBounds.removeFromTop(8);
        layoutRotarySlot(gateAttackSlider, gBounds.removeFromTop(gSlotH).reduced(6, 0));
        gBounds.removeFromTop(8);
        layoutRotarySlot(gateReleaseSlider, gBounds.removeFromTop(gSlotH).reduced(6, 0));

        gBounds.removeFromTop(10);
        gateBypassButton.setBounds(gBounds.removeFromTop(24).reduced(16, 0));
    }

    // =========================================================================
    // 3. 4-BAND EQ MODULE LAYOUT (4 Dedicated Vertical Slots Per Column)
    // =========================================================================
    {
        auto eBounds = eqModule.getContentBounds();
        eBounds.removeFromTop(4);

        const int colW = eBounds.getWidth() / 4;
        auto colLF  = eBounds.removeFromLeft(colW);
        auto colLMF = eBounds.removeFromLeft(colW);
        auto colHMF = eBounds.removeFromLeft(colW);
        auto colHF  = eBounds;

        const int bottomH = 44;
        const int knobAreaH = colLF.getHeight() - 20 - bottomH - 28;
        const int singleKnobSlotH = knobAreaH / 2;

        // LF Column
        eqLowTitleLabel.setBounds(colLF.removeFromTop(18));
        colLF.removeFromTop(4);
        layoutRotarySlot(eqLowGainSlider, colLF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colLF.removeFromTop(8); // 8px vertical padding between gain textbox and freq label
        layoutRotarySlot(eqLowFreqSlider, colLF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colLF.removeFromTop(8);
        eqLowShelfButton.setBounds(colLF.removeFromTop(24).reduced(4, 0));

        // LMF Column
        eqLmfTitleLabel.setBounds(colLMF.removeFromTop(18));
        colLMF.removeFromTop(4);
        layoutRotarySlot(eqLmfGainSlider, colLMF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colLMF.removeFromTop(8); // 8px vertical padding between gain textbox and freq label
        layoutRotarySlot(eqLmfFreqSlider, colLMF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colLMF.removeFromTop(8);
        layoutRotarySlot(eqLmfQSlider, colLMF.removeFromTop(bottomH).reduced(2, 0), 14);

        // HMF Column
        eqHmfTitleLabel.setBounds(colHMF.removeFromTop(18));
        colHMF.removeFromTop(4);
        layoutRotarySlot(eqHmfGainSlider, colHMF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colHMF.removeFromTop(8); // 8px vertical padding between gain textbox and freq label
        layoutRotarySlot(eqHmfFreqSlider, colHMF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colHMF.removeFromTop(8);
        layoutRotarySlot(eqHmfQSlider, colHMF.removeFromTop(bottomH).reduced(2, 0), 14);

        // HF Column
        eqHighTitleLabel.setBounds(colHF.removeFromTop(18));
        colHF.removeFromTop(4);
        layoutRotarySlot(eqHighGainSlider, colHF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colHF.removeFromTop(8); // 8px vertical padding between gain textbox and freq label
        layoutRotarySlot(eqHighFreqSlider, colHF.removeFromTop(singleKnobSlotH).reduced(2, 0), 14);
        colHF.removeFromTop(8);
        eqBypassButton.setBounds(colHF.removeFromTop(24).reduced(4, 0));
    }

    // =========================================================================
    // 4. 1176 DYNAMICS MODULE LAYOUT (Authentic 1176 Architecture)
    // =========================================================================
    {
        auto cBounds = compModule.getContentBounds();
        
        // Slot 1: Recessed 1176 GR VU Meter
        compGrMeter.setBounds(cBounds.removeFromTop(94).reduced(4, 2));
        cBounds.removeFromTop(10);

        // Slot 2: 2x2 Grid of Knobs
        const int bottomControlsH = 66;
        const int gridH = cBounds.getHeight() - bottomControlsH - 24;
        const int rowH = gridH / 2;

        auto row1 = cBounds.removeFromTop(rowH);
        cBounds.removeFromTop(10); // 10px vertical padding between row1 textboxes and row2 labels
        auto row2 = cBounds.removeFromTop(rowH);

        const int halfW = row1.getWidth() / 2;
        layoutRotarySlot(compInputGainSlider, row1.removeFromLeft(halfW).reduced(2, 0), 14);
        layoutRotarySlot(compOutputGainSlider, row1.reduced(2, 0), 14);

        layoutRotarySlot(compAttackSlider, row2.removeFromLeft(halfW).reduced(2, 0), 14);
        layoutRotarySlot(compReleaseSlider, row2.reduced(2, 0), 14);

        cBounds.removeFromTop(10);

        // Slot 3: Horizontal Bank of 5 Ratio Push-Buttons
        auto ratioArea = cBounds.removeFromTop(24).reduced(2, 0);
        const int btnW = ratioArea.getWidth() / 5;

        ratio4Button.setBounds(ratioArea.removeFromLeft(btnW).reduced(1, 0));
        ratio8Button.setBounds(ratioArea.removeFromLeft(btnW).reduced(1, 0));
        ratio12Button.setBounds(ratioArea.removeFromLeft(btnW).reduced(1, 0));
        ratio20Button.setBounds(ratioArea.removeFromLeft(btnW).reduced(1, 0));
        ratioAllButton.setBounds(ratioArea.reduced(1, 0));

        cBounds.removeFromTop(8);
        compBypassButton.setBounds(cBounds.removeFromTop(24).reduced(24, 0));
    }
}

void RackStrip500AudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff0c0e12)); // Deep rack chassis background
}

void RackStrip500AudioProcessorEditor::paintOverChildren(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    // Subtle, large background watermark rendered across the entire chassis and faceplates
    g.setFont(juce::Font("Helvetica Neue", 54.0f, juce::Font::bold));
    g.setColour(juce::Colours::white.withAlpha(0.045f));
    g.drawText("HOPESTUDIO", bounds, juce::Justification::centred, false);
}

void RackStrip500AudioProcessorEditor::timerCallback()
{
    inputVuMeter.setLevel(audioProcessor.inputLevelDb.load());
    compGrMeter.setLevel(audioProcessor.compGrDb.load());

    // Sync Ratio buttons
    bool isAll = false;
    if (auto* allVal = audioProcessor.getAPVTS().getRawParameterValue("comp_all_buttons"))
        isAll = (allVal->load() > 0.5f);

    if (isAll)
    {
        if (!ratioAllButton.getToggleState())
            ratioAllButton.setToggleState(true, juce::dontSendNotification);
    }
    else
    {
        float currentRatio = audioProcessor.getAPVTS().getRawParameterValue("comp_ratio")->load();
        if (currentRatio <= 4.5f)
        {
            if (!ratio4Button.getToggleState()) ratio4Button.setToggleState(true, juce::dontSendNotification);
        }
        else if (currentRatio <= 8.5f)
        {
            if (!ratio8Button.getToggleState()) ratio8Button.setToggleState(true, juce::dontSendNotification);
        }
        else if (currentRatio <= 12.5f)
        {
            if (!ratio12Button.getToggleState()) ratio12Button.setToggleState(true, juce::dontSendNotification);
        }
        else
        {
            if (!ratio20Button.getToggleState()) ratio20Button.setToggleState(true, juce::dontSendNotification);
        }
    }
}
