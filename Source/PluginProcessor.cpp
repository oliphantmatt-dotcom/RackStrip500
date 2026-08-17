#include "PluginProcessor.h"
#include "PluginEditor.h"

RackStrip500AudioProcessor::RackStrip500AudioProcessor()
    : AudioProcessor (BusesProperties()
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    // Preamp
    preampHpfFreqParam     = apvts.getRawParameterValue ("preamp_hpf_freq");
    preampLowBumpParam     = apvts.getRawParameterValue ("preamp_low_bump");
    preampDriveParam       = apvts.getRawParameterValue ("preamp_drive");
    preampBypassParam      = apvts.getRawParameterValue ("preamp_bypass");

    // De-Esser
    deesserThreshParam     = apvts.getRawParameterValue ("deesser_thresh");
    deesserFreqParam       = apvts.getRawParameterValue ("deesser_freq");
    deesserMixParam        = apvts.getRawParameterValue ("deesser_mix");
    deesserBypassParam     = apvts.getRawParameterValue ("deesser_bypass");

    // Noise Gate
    gateThreshParam        = apvts.getRawParameterValue ("gate_thresh");
    gateRatioParam         = apvts.getRawParameterValue ("gate_ratio");
    gateAttackParam        = apvts.getRawParameterValue ("gate_attack");
    gateReleaseParam       = apvts.getRawParameterValue ("gate_release");
    gateBypassParam        = apvts.getRawParameterValue ("gate_bypass");

    // 4-Band EQ
    eqLowGainParam         = apvts.getRawParameterValue ("eq_low_gain");
    eqLowFreqParam         = apvts.getRawParameterValue ("eq_low_freq");
    eqLmfGainParam         = apvts.getRawParameterValue ("eq_lmf_gain");
    eqLmfFreqParam         = apvts.getRawParameterValue ("eq_lmf_freq");
    eqLmfQParam            = apvts.getRawParameterValue ("eq_lmf_q");
    eqHmfGainParam         = apvts.getRawParameterValue ("eq_hmf_gain");
    eqHmfFreqParam         = apvts.getRawParameterValue ("eq_hmf_freq");
    eqHmfQParam            = apvts.getRawParameterValue ("eq_hmf_q");
    eqHighGainParam        = apvts.getRawParameterValue ("eq_high_gain");
    eqHighFreqParam        = apvts.getRawParameterValue ("eq_high_freq");
    eqBypassParam          = apvts.getRawParameterValue ("eq_bypass");

    // Dynamics (Shared)
    compModeParam          = apvts.getRawParameterValue ("comp_mode");
    compThreshParam        = apvts.getRawParameterValue ("comp_thresh");
    compRatioParam         = apvts.getRawParameterValue ("comp_ratio");
    compAllButtonsParam    = apvts.getRawParameterValue ("comp_all_buttons");
    compAttackParam        = apvts.getRawParameterValue ("comp_attack");
    compReleaseParam       = apvts.getRawParameterValue ("comp_release");
    compMixParam           = apvts.getRawParameterValue ("comp_mix");
    compBypassParam        = apvts.getRawParameterValue ("comp_bypass");

    // Distressor Specific
    distressorRatioParam   = apvts.getRawParameterValue ("distressor_ratio");
    distModeParam          = apvts.getRawParameterValue ("dist_mode");
    detectorFilterParam    = apvts.getRawParameterValue ("detector_filter");

    // Master
    outputTrimParam        = apvts.getRawParameterValue ("out_trim");
    outputPhaseInvertParam = apvts.getRawParameterValue ("out_phase_invert");
    analogNoiseParam       = apvts.getRawParameterValue ("analog_noise");
}

RackStrip500AudioProcessor::~RackStrip500AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout RackStrip500AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Preamp
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "preamp_hpf_freq", 1 }, "Preamp HPF Freq", juce::NormalisableRange<float>(20.0f, 300.0f, 1.0f, 0.5f), 20.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "preamp_low_bump", 1 }, "Preamp Low Bump", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "preamp_drive", 1 }, "Preamp Drive (dB)", juce::NormalisableRange<float>(0.0f, 24.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "preamp_bypass", 1 }, "Preamp Bypass", false));

    // De-Esser
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "deesser_thresh", 1 }, "De-Esser Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "deesser_freq", 1 }, "De-Esser Freq", juce::NormalisableRange<float>(3000.0f, 10000.0f, 1.0f, 0.5f), 6000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "deesser_mix", 1 }, "De-Esser Mix (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "deesser_bypass", 1 }, "De-Esser Bypass", false));

    // Noise Gate
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "gate_thresh", 1 }, "Gate Threshold", juce::NormalisableRange<float>(-80.0f, 0.0f, 0.1f), -80.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "gate_ratio", 1 }, "Gate Ratio", juce::NormalisableRange<float>(1.0f, 10.0f, 0.1f), 2.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "gate_attack", 1 }, "Gate Attack (ms)", juce::NormalisableRange<float>(0.1f, 50.0f, 0.1f, 0.3f), 1.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "gate_release", 1 }, "Gate Release (ms)", juce::NormalisableRange<float>(10.0f, 1000.0f, 1.0f, 0.3f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "gate_bypass", 1 }, "Gate Bypass", false));

    // 4-Band EQ
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_low_gain", 1 }, "EQ Low Gain", juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_low_freq", 1 }, "EQ Low Freq", juce::NormalisableRange<float>(30.0f, 450.0f, 1.0f, 0.5f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_lmf_gain", 1 }, "EQ LMF Gain", juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_lmf_freq", 1 }, "EQ LMF Freq", juce::NormalisableRange<float>(200.0f, 2500.0f, 1.0f, 0.5f), 1000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_lmf_q", 1 }, "EQ LMF Q", juce::NormalisableRange<float>(0.5f, 5.0f, 0.05f), 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_hmf_gain", 1 }, "EQ HMF Gain", juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_hmf_freq", 1 }, "EQ HMF Freq", juce::NormalisableRange<float>(1000.0f, 8000.0f, 1.0f, 0.5f), 3000.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_hmf_q", 1 }, "EQ HMF Q", juce::NormalisableRange<float>(0.5f, 5.0f, 0.05f), 0.7f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_high_gain", 1 }, "EQ High Gain", juce::NormalisableRange<float>(-15.0f, 15.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "eq_high_freq", 1 }, "EQ High Freq", juce::NormalisableRange<float>(3000.0f, 16000.0f, 1.0f, 0.5f), 8000.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "eq_bypass", 1 }, "EQ Bypass", false));

    // Shared Dynamics
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "comp_mode", 1 }, "Compressor Mode", juce::StringArray{"1176 FET", "VCA"}, 0));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "comp_thresh", 1 }, "Comp Threshold", juce::NormalisableRange<float>(-40.0f, 0.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "comp_ratio", 1 }, "Comp Ratio", juce::NormalisableRange<float>(2.0f, 20.0f, 0.1f), 4.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "comp_all_buttons", 1 }, "Comp All-Buttons Mode", false));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "comp_attack", 1 }, "Comp Attack (ms)", juce::NormalisableRange<float>(0.02f, 10.0f, 0.01f, 0.3f), 0.5f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "comp_release", 1 }, "Comp Release (ms)", juce::NormalisableRange<float>(50.0f, 1100.0f, 1.0f, 0.3f), 200.0f));
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "comp_mix", 1 }, "Comp Mix (%)", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), 100.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "comp_bypass", 1 }, "Comp Bypass", false));

    // Distressor Specific
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "distressor_ratio", 1 }, "Distressor Ratio", juce::StringArray{"1:1", "2:1", "3:1", "4:1", "6:1", "10:1", "20:1", "NUKE"}, 3));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "dist_mode", 1 }, "Distortion Mode", juce::StringArray{"Clean", "Dist 2", "Dist 3"}, 0));
    params.push_back (std::make_unique<juce::AudioParameterChoice>(juce::ParameterID { "detector_filter", 1 }, "Detector Filter", juce::StringArray{"Flat", "HPF", "Bandpass"}, 0));

    // Master
    params.push_back (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { "out_trim", 1 }, "Output Trim (dB)", juce::NormalisableRange<float>(-24.0f, 24.0f, 0.1f), 0.0f));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "out_phase_invert", 1 }, "Phase Invert", false));
    params.push_back (std::make_unique<juce::AudioParameterBool>  (juce::ParameterID { "analog_noise", 1 }, "Analog Noise", false));

    return { params.begin(), params.end() };
}

bool RackStrip500AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void RackStrip500AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Strictly enforce 0-sample latency across all host DAWs
    setLatencySamples(0);

    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels = static_cast<juce::uint32> (getTotalNumInputChannels());

    // Prepare Zero-Delay Feedback (ZDF) Filters
    zdfPreampHpf.prepare (sampleRate);
    zdfPreampLowBump.prepare (sampleRate);
    preampTransformer.prepare (sampleRate);

    deesserBandpass.prepare (spec);
    noiseGate.prepare (spec);

    zdfEqLow.prepare (sampleRate);
    zdfEqLmf.prepare (sampleRate);
    zdfEqHmf.prepare (sampleRate);
    zdfEqHigh.prepare (sampleRate);

    fet1176Engine.prepare (sampleRate);
    outputTransformer.prepare (sampleRate);

    // Reset Envelopes & States
    deesserEnv = 0.0f;
    vcaRmsEnv = 0.0f;
    compGrDb.store (0.0f);
    deesserGrDb.store (0.0f);
    gateGainReductionDb.store (0.0f);
    inputLevelDb.store (-100.0f);

    // Reset Smoothers
    constexpr float rampTimeSec = 0.02f;
    smoothedDrive.reset (sampleRate, rampTimeSec);
    smoothedDeesserMix.reset (sampleRate, rampTimeSec);
    smoothedCompMix.reset (sampleRate, rampTimeSec);
    smoothedOutputTrim.reset (sampleRate, rampTimeSec);
    smoothedAnalogNoiseLevel.reset (sampleRate, rampTimeSec);

    smoothedCompMode.reset (sampleRate, 0.01f);
    smoothedCompMode.setCurrentAndTargetValue (compModeParam->load());

    // Allocate scratch buffers
    dryBufferComp.setSize (getTotalNumInputChannels(), samplesPerBlock);
    sidechainBuffer.setSize (getTotalNumInputChannels(), samplesPerBlock);
    vcaOutBuffer.setSize (getTotalNumInputChannels(), samplesPerBlock);
    fetOutBuffer.setSize (getTotalNumInputChannels(), samplesPerBlock);
}

void RackStrip500AudioProcessor::releaseResources() {}

void RackStrip500AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numSamples == 0 || numChannels == 0) return;

    // Calculate RMS for Input VU Meter
    float blockRms = 0.0f;
    for (int ch = 0; ch < numChannels; ++ch)
        blockRms += buffer.getRMSLevel (ch, 0, numSamples);
    blockRms /= (float)numChannels;
    float currentVu = juce::Decibels::gainToDecibels (blockRms + 1e-5f);

    float alphaVU = std::exp (-1.0f / (0.3f * currentSampleRate / numSamples));
    inputLevelDb.store (inputLevelDb.load() * alphaVU + currentVu * (1.0f - alphaVU), std::memory_order_relaxed);

    // =========================================================================
    // 1. PREAMP MODULE (ZDF High-Pass, Low Bump & Transformer Core Saturation)
    // =========================================================================
    bool preampBypassed = preampBypassParam->load() > 0.5f;
    if (!preampBypassed)
    {
        float hpfFreq = preampHpfFreqParam->load();
        zdfPreampHpf.update (hpfFreq, 0.707f, 0.0f, rackstrip::dsp::ZdfSvf::Mode::HighPass);

        bool lowBumpActive = preampLowBumpParam->load() > 0.5f;
        if (lowBumpActive)
            zdfPreampLowBump.update (80.0f, 0.707f, 3.0f, rackstrip::dsp::ZdfSvf::Mode::LowShelf);

        float targetDrive = preampDriveParam->load();
        smoothedDrive.setTargetValue (juce::Decibels::decibelsToGain (targetDrive));

        for (int i = 0; i < numSamples; ++i)
        {
            float driveGain = smoothedDrive.getNextValue();
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float s = buffer.getSample (ch, i);
                s = zdfPreampHpf.processSample (ch, s);

                if (lowBumpActive)
                    s = zdfPreampLowBump.processSample (ch, s);

                if (driveGain > 1.001f)
                    s = preampTransformer.processSample (ch, s, driveGain);

                buffer.setSample (ch, i, s);
            }
        }
    }

    // =========================================================================
    // 2. NOISE GATE MODULE
    // =========================================================================
    if (gateBypassParam->load() < 0.5f)
    {
        noiseGate.setThreshold (gateThreshParam->load());
        noiseGate.setRatio (gateRatioParam->load());
        noiseGate.setAttack (gateAttackParam->load());
        noiseGate.setRelease (gateReleaseParam->load());
        juce::dsp::AudioBlock<float> block (buffer);
        noiseGate.process (juce::dsp::ProcessContextReplacing<float> (block));
    }

    // =========================================================================
    // 3. ZERO-DELAY FEEDBACK (ZDF) 4-BAND EQ (Zero Latency, Proportional Q)
    // =========================================================================
    if (eqBypassParam->load() < 0.5f)
    {
        float lowGain  = eqLowGainParam->load();
        float lowFreq  = eqLowFreqParam->load();
        zdfEqLow.update (lowFreq, 0.707f, lowGain, rackstrip::dsp::ZdfSvf::Mode::LowShelf);

        float lmfGain  = eqLmfGainParam->load();
        float lmfFreq  = eqLmfFreqParam->load();
        float lmfQ     = eqLmfQParam->load();
        zdfEqLmf.update (lmfFreq, lmfQ, lmfGain, rackstrip::dsp::ZdfSvf::Mode::BellProportional);

        float hmfGain  = eqHmfGainParam->load();
        float hmfFreq  = eqHmfFreqParam->load();
        float hmfQ     = eqHmfQParam->load();
        zdfEqHmf.update (hmfFreq, hmfQ, hmfGain, rackstrip::dsp::ZdfSvf::Mode::BellProportional);

        float highGain = eqHighGainParam->load();
        float highFreq = eqHighFreqParam->load();
        zdfEqHigh.update (highFreq, 0.707f, highGain, rackstrip::dsp::ZdfSvf::Mode::HighShelf);

        for (int i = 0; i < numSamples; ++i)
        {
            for (int ch = 0; ch < numChannels; ++ch)
            {
                float s = buffer.getSample (ch, i);
                s = zdfEqLow.processSample (ch, s);
                s = zdfEqLmf.processSample (ch, s);
                s = zdfEqHmf.processSample (ch, s);
                s = zdfEqHigh.processSample (ch, s);
                buffer.setSample (ch, i, s);
            }
        }
    }

    // =========================================================================
    // 4. STATE-SPACE 1176 FET DYNAMICS & PEAK DETECTION
    // =========================================================================
    bool compBypassed = compBypassParam->load() > 0.5f;
    if (!compBypassed)
    {
        dryBufferComp.makeCopyOf (buffer);

        float thresh    = compThreshParam->load();
        float ratio     = compRatioParam->load();
        bool allButtons = (compAllButtonsParam != nullptr && compAllButtonsParam->load() > 0.5f);
        float attackMs  = compAttackParam->load();
        float releaseMs = compReleaseParam->load();

        smoothedCompMix.setTargetValue (compMixParam->load() * 0.01f);

        // Process State-Space 1176 FET compression
        fet1176Engine.processBlock (buffer, thresh, ratio, attackMs, releaseMs, allButtons, compGrDb);

        // Wet / Dry Mix
        for (int i = 0; i < numSamples; ++i)
        {
            float mix = smoothedCompMix.getNextValue();
            if (mix < 0.999f)
            {
                for (int ch = 0; ch < numChannels; ++ch)
                {
                    float wet = buffer.getSample (ch, i);
                    float dry = dryBufferComp.getSample (ch, i);
                    buffer.setSample (ch, i, dry * (1.0f - mix) + wet * mix);
                }
            }
        }
    }
    else
    {
        compGrDb.store (0.0f, std::memory_order_relaxed);
    }

    // =========================================================================
    // 5. MASTER OUTPUT (Transformer Output Saturation, Phase Invert & Trim)
    // =========================================================================
    float outTrimDb = outputTrimParam->load();
    smoothedOutputTrim.setTargetValue (juce::Decibels::decibelsToGain (outTrimDb));

    bool phaseInvert = outputPhaseInvertParam->load() > 0.5f;
    float phaseMul   = phaseInvert ? -1.0f : 1.0f;

    bool analogNoiseOn = analogNoiseParam->load() > 0.5f;
    smoothedAnalogNoiseLevel.setTargetValue (analogNoiseOn ? 0.00003f : 0.0f); // -90 dB analog noise floor

    for (int i = 0; i < numSamples; ++i)
    {
        float trimGain = smoothedOutputTrim.getNextValue();
        float noiseLev = smoothedAnalogNoiseLevel.getNextValue();

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float s = buffer.getSample (ch, i);

            // Output stage transformer saturation
            s = outputTransformer.processSample (ch, s, trimGain);
            s *= phaseMul;

            if (noiseLev > 0.0f)
                s += (randomGenerator.nextFloat() * 2.0f - 1.0f) * noiseLev;

            buffer.setSample (ch, i, s);
        }
    }
}

void RackStrip500AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();

    // Store current editor size if available
    if (auto* editor = getActiveEditor())
    {
        state.setProperty ("editorWidth", editor->getWidth(), nullptr);
        state.setProperty ("editorHeight", editor->getHeight(), nullptr);
    }

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void RackStrip500AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RackStrip500AudioProcessor();
}

juce::AudioProcessorEditor* RackStrip500AudioProcessor::createEditor()
{
    return new RackStrip500AudioProcessorEditor (*this);
}

bool RackStrip500AudioProcessor::hasEditor() const
{
    return true;
}
