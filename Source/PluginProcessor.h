#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <atomic>
#include "DSP/ZdfAnalogEngine.h"

class RackStrip500AudioProcessor : public juce::AudioProcessor
{
public:
    RackStrip500AudioProcessor();
    ~RackStrip500AudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using juce::AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override { return "RackStrip500"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    // Thread-safe Gain Reduction readouts for GUI meters (in dB)
    float getCompGainReductionDb() const noexcept { return compGrDb.load(std::memory_order_relaxed); }
    float getDeessGainReductionDb() const noexcept { return deesserGrDb.load(std::memory_order_relaxed); }
    float getGateGainReductionDb() const noexcept { return gateGainReductionDb.load(std::memory_order_relaxed); }
    float getVULevel() const noexcept { return inputLevelDb.load(std::memory_order_relaxed); }

    // Public atomic variables for direct UI access
    std::atomic<float> inputLevelDb { -100.0f };
    std::atomic<float> deesserGrDb { 0.0f };
    std::atomic<float> compGrDb { 0.0f };

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    juce::AudioProcessorValueTreeState apvts;

    // Cached Parameter Pointers
    std::atomic<float>* preampHpfFreqParam     = nullptr;
    std::atomic<float>* preampLowBumpParam     = nullptr;
    std::atomic<float>* preampDriveParam       = nullptr;
    std::atomic<float>* preampBypassParam      = nullptr;

    std::atomic<float>* deesserThreshParam     = nullptr;
    std::atomic<float>* deesserFreqParam       = nullptr;
    std::atomic<float>* deesserMixParam        = nullptr;
    std::atomic<float>* deesserBypassParam     = nullptr;

    std::atomic<float>* gateThreshParam        = nullptr;
    std::atomic<float>* gateRatioParam         = nullptr;
    std::atomic<float>* gateAttackParam        = nullptr;
    std::atomic<float>* gateReleaseParam       = nullptr;
    std::atomic<float>* gateBypassParam        = nullptr;

    std::atomic<float>* eqLowGainParam         = nullptr;
    std::atomic<float>* eqLowFreqParam         = nullptr;
    std::atomic<float>* eqLmfGainParam         = nullptr;
    std::atomic<float>* eqLmfFreqParam         = nullptr;
    std::atomic<float>* eqLmfQParam            = nullptr;
    std::atomic<float>* eqHmfGainParam         = nullptr;
    std::atomic<float>* eqHmfFreqParam         = nullptr;
    std::atomic<float>* eqHmfQParam            = nullptr;
    std::atomic<float>* eqHighGainParam        = nullptr;
    std::atomic<float>* eqHighFreqParam        = nullptr;
    std::atomic<float>* eqBypassParam          = nullptr;

    std::atomic<float>* compModeParam          = nullptr; // 0 = VCA, 1 = FET (Distressor)
    std::atomic<float>* compThreshParam        = nullptr;
    std::atomic<float>* compRatioParam         = nullptr;
    std::atomic<float>* compAllButtonsParam    = nullptr; // 1176 All-Buttons Mode
    std::atomic<float>* compAttackParam        = nullptr;
    std::atomic<float>* compReleaseParam       = nullptr;
    std::atomic<float>* compMixParam           = nullptr;
    std::atomic<float>* compBypassParam        = nullptr;

    // Distressor Specific Parameters
    std::atomic<float>* distressorRatioParam   = nullptr;
    std::atomic<float>* distModeParam          = nullptr; // 0 = Clean, 1 = Dist 2, 2 = Dist 3
    std::atomic<float>* detectorFilterParam    = nullptr; // 0 = Flat, 1 = HPF, 2 = Bandpass

    std::atomic<float>* outputTrimParam        = nullptr;
    std::atomic<float>* outputPhaseInvertParam = nullptr;
    std::atomic<float>* analogNoiseParam       = nullptr;

    // DSP Engine State
    double currentSampleRate = 44100.0;

    // Zero-Delay Feedback (ZDF) Preamp Filters
    rackstrip::dsp::ZdfSvf zdfPreampHpf;
    rackstrip::dsp::ZdfSvf zdfPreampLowBump;
    rackstrip::dsp::TransformerStage preampTransformer;

    // De-Esser Detector Filter & Envelope
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> deesserBandpass;
    float deesserEnv = 0.0f;

    // Noise Gate
    juce::dsp::NoiseGate<float> noiseGate;

    // Zero-Delay Feedback (ZDF) 4-Band EQ
    rackstrip::dsp::ZdfSvf zdfEqLow;
    rackstrip::dsp::ZdfSvf zdfEqLmf;
    rackstrip::dsp::ZdfSvf zdfEqHmf;
    rackstrip::dsp::ZdfSvf zdfEqHigh;

    // State-Space 1176 FET Compressor Engine & Output Transformer
    rackstrip::dsp::Fet1176Engine fet1176Engine;
    rackstrip::dsp::TransformerStage outputTransformer;

    // VCA Dynamics State (for optional VCA mode)
    float vcaRmsEnv = 0.0f;
    juce::LinearSmoothedValue<float> smoothedCompMode;

    // Sidechain Filters for FET
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> fetScHpf;
    juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>> fetScBandpass;

    // Thread-safe feedback for GUI
    std::atomic<float> gateGainReductionDb  { 0.0f };

    // Parameter Smoothers
    juce::LinearSmoothedValue<float> smoothedDrive;
    juce::LinearSmoothedValue<float> smoothedDeesserMix;
    juce::LinearSmoothedValue<float> smoothedCompMix;
    juce::LinearSmoothedValue<float> smoothedOutputTrim;
    juce::LinearSmoothedValue<float> smoothedAnalogNoiseLevel;

    // Analog Noise State
    juce::Random randomGenerator;

    // Pre-allocated buffers
    juce::AudioBuffer<float> dryBufferComp;
    juce::AudioBuffer<float> sidechainBuffer;
    juce::AudioBuffer<float> vcaOutBuffer;
    juce::AudioBuffer<float> fetOutBuffer;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackStrip500AudioProcessor)
};
