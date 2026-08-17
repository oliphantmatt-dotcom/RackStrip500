#include "VUMeterComponent.h"
#include "VintageRackLookAndFeel.h"

namespace rackstrip
{

VUMeterComponent::VUMeterComponent(MeterType meterType, const juce::String& title)
    : type(meterType), titleText(title)
{
    setBufferedToImage(true);
}

void VUMeterComponent::setLevel(float levelDb)
{
    if (type == MeterType::AnalogVU || type == MeterType::GainReductionVU)
    {
        // Realistic ballistic inertia (spring-mass-damper with VU meter standard response)
        float target = levelDb;
        float force = (target - needlePosition) * 0.20f; 
        needleVelocity += force;
        needleVelocity *= 0.76f; // Damping
        needlePosition += needleVelocity;
        
        // Clamping to meter dial range
        if (needlePosition < -40.0f) { needlePosition = -40.0f; needleVelocity = 0.0f; }
        if (needlePosition > 10.0f)  { needlePosition = 10.0f; needleVelocity = 0.0f; }
        
        if (std::abs(needlePosition - currentLevelDb) > 0.04f)
        {
            currentLevelDb = needlePosition;
            repaint();
        }
    }
    else
    {
        if (std::abs(currentLevelDb - levelDb) > 0.1f)
        {
            currentLevelDb = levelDb;
            repaint();
        }
    }
}

void VUMeterComponent::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced(2.0f);

    if (type == MeterType::AnalogVU || type == MeterType::GainReductionVU)
    {
        // 1. Recessed Bezel Housing
        g.setColour(juce::Colour(0xff0c0e12));
        g.fillRoundedRectangle(bounds, 4.5f);

        // Outer metal bezel
        juce::ColourGradient bezelGrad(juce::Colour(0xff424852), bounds.getX(), bounds.getY(),
                                       juce::Colour(0xff181a1d), bounds.getRight(), bounds.getBottom(), false);
        g.setGradientFill(bezelGrad);
        g.drawRoundedRectangle(bounds, 4.5f, 1.5f);

        // 2. Inner Recessed Drop Shadow
        auto innerMeterBounds = bounds.reduced(3.5f);
        
        if (type == MeterType::GainReductionVU)
            drawGainReductionVU(g, innerMeterBounds);
        else
            drawAnalogVU(g, innerMeterBounds);

        // Inner shadow overlay for sunken depth
        juce::Path clipPath;
        clipPath.addRoundedRectangle(innerMeterBounds, 2.5f);
        g.setColour(juce::Colours::black.withAlpha(0.40f));
        g.drawRoundedRectangle(innerMeterBounds, 2.5f, 1.5f);
        g.setColour(juce::Colours::black.withAlpha(0.25f));
        g.drawHorizontalLine(static_cast<int>(innerMeterBounds.getY() + 1.0f), innerMeterBounds.getX(), innerMeterBounds.getRight());
    }
    else
    {
        drawGainReduction(g, bounds);
    }
}

void VUMeterComponent::drawAnalogVU(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    juce::Graphics::ScopedSaveState state(g);
    g.reduceClipRegion(bounds.toNearestInt());

    // 1. Vintage Warm Amber-Cream Dial Face (#F5EEDC to #E8D9B5)
    juce::ColourGradient dialGrad(juce::Colour(0xfff5eedc), bounds.getCentreX(), bounds.getY(),
                                  juce::Colour(0xffe8d9b5), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(dialGrad);
    g.fillRoundedRectangle(bounds, 2.5f);

    // 2. Incandescent Backlight Glow
    juce::ColourGradient bottomLampGlow(juce::Colour(0x90ffdf96), bounds.getCentreX(), bounds.getBottom(),
                                        juce::Colour(0x00ffe1a0), bounds.getCentreX(), bounds.getCentreY() - 10.0f, true);
    g.setGradientFill(bottomLampGlow);
    g.fillRoundedRectangle(bounds, 2.5f);

    juce::ColourGradient topLampGlow(juce::Colour(0x45ffeaaf), bounds.getCentreX(), bounds.getY(),
                                     juce::Colour(0x00ffeaaf), bounds.getCentreX(), bounds.getCentreY(), true);
    g.setGradientFill(topLampGlow);
    g.fillRoundedRectangle(bounds, 2.5f);

    auto pivotPoint = juce::Point<float>(bounds.getCentreX(), bounds.getBottom() + bounds.getHeight() * 0.45f);
    float radius = bounds.getHeight() * 1.30f;

    const float startAngle = -42.0f;
    const float endAngle   = 42.0f;

    // 3. Curved dB Scale (-20 to +3 dB, with 0 to +3 zone in vivid red)
    float zeroVUAngle = juce::jmap(0.0f, -20.0f, 3.0f, startAngle, endAngle);

    // Black Arc (-20 to 0 VU)
    juce::Path blackArc;
    blackArc.addCentredArc(pivotPoint.x, pivotPoint.y, radius - 16.0f, radius - 16.0f, 0.0f,
                           juce::degreesToRadians(startAngle), juce::degreesToRadians(zeroVUAngle), true);
    g.setColour(juce::Colour(0xdd1a1c1e));
    g.strokePath(blackArc, juce::PathStrokeType(1.6f));

    // Vivid Red Arc (0 to +3 VU)
    juce::Path redArc;
    redArc.addCentredArc(pivotPoint.x, pivotPoint.y, radius - 16.0f, radius - 16.0f, 0.0f,
                         juce::degreesToRadians(zeroVUAngle), juce::degreesToRadians(endAngle), true);
    g.setColour(juce::Colour(0xffd32f2f)); // Vivid Red
    g.strokePath(redArc, juce::PathStrokeType(2.4f));

    // Scale Tick Marks and Numerical Labels
    struct VUTick { float db; const char* label; bool isMajor; };
    const VUTick ticks[] = {
        { -20.0f, "20", true },
        { -10.0f, "10", true },
        {  -7.0f, "7",  true },
        {  -5.0f, "5",  true },
        {  -3.0f, "3",  true },
        {  -2.0f, "2",  false },
        {  -1.0f, "1",  false },
        {   0.0f, "0",  true },
        {   1.0f, "+1", true },
        {   2.0f, "+2", false },
        {   3.0f, "+3", true }
    };

    g.setFont(juce::Font("Helvetica Neue", 8.0f, juce::Font::bold));

    for (const auto& tick : ticks)
    {
        float angleDeg = juce::jmap(tick.db, -20.0f, 3.0f, startAngle, endAngle);
        float angleRad = juce::degreesToRadians(angleDeg);

        bool isRedZone = (tick.db >= 0.0f);
        juce::Colour tickColor = isRedZone ? juce::Colour(0xffd32f2f) : juce::Colour(0xee1c1e22);

        float rArc = radius - 16.0f;
        float tickLength = tick.isMajor ? 6.5f : 4.0f;

        juce::Point<float> p1(pivotPoint.x + rArc * std::sin(angleRad),
                              pivotPoint.y - rArc * std::cos(angleRad));
        juce::Point<float> p2(pivotPoint.x + (rArc + tickLength) * std::sin(angleRad),
                              pivotPoint.y - (rArc + tickLength) * std::cos(angleRad));

        g.setColour(tickColor);
        g.drawLine(juce::Line<float>(p1, p2), tick.isMajor ? 1.5f : 1.0f);

        if (tick.isMajor)
        {
            float rText = rArc + 12.0f;
            juce::Point<float> pText(pivotPoint.x + rText * std::sin(angleRad),
                                     pivotPoint.y - rText * std::cos(angleRad));

            auto textRect = juce::Rectangle<float>(18.0f, 10.0f).withCentre(pText);
            g.setColour(tickColor);
            g.drawText(tick.label, textRect, juce::Justification::centred, false);
        }
    }

    // Vintage "VU" badge and title label
    g.setFont(juce::Font("Georgia", 11.5f, juce::Font::bold));
    g.setColour(juce::Colour(0xaa22252a));
    auto vuBadgeRect = juce::Rectangle<float>(40.0f, 16.0f).withCentre({ bounds.getCentreX(), bounds.getCentreY() + 8.0f });
    g.drawText("VU", vuBadgeRect, juce::Justification::centred, false);

    g.setFont(juce::Font("Helvetica Neue", 8.5f, juce::Font::bold));
    g.setColour(juce::Colour(0x8022252a));
    g.drawText(titleText, bounds.removeFromBottom(16.0f), juce::Justification::centredTop, false);

    // 4. Tapered Needle Calculation & Rendering
    float mappedLevel = juce::jmap(currentLevelDb, -20.0f, 3.0f, startAngle, endAngle);
    mappedLevel = juce::jlimit(startAngle - 6.0f, endAngle + 6.0f, mappedLevel);
    float needleAngleRad = juce::degreesToRadians(mappedLevel);

    float needleLength = radius - 6.0f;
    float needleTipWidth = 0.8f;
    float needleBaseWidth = 3.2f;

    float cosA = std::cos(needleAngleRad);
    float sinA = std::sin(needleAngleRad);

    juce::Point<float> nTip(pivotPoint.x + needleLength * sinA,
                            pivotPoint.y - needleLength * cosA);

    juce::Point<float> nBaseL(pivotPoint.x + needleBaseWidth * 0.5f * cosA,
                             pivotPoint.y + needleBaseWidth * 0.5f * sinA);
    juce::Point<float> nBaseR(pivotPoint.x - needleBaseWidth * 0.5f * cosA,
                             pivotPoint.y - needleBaseWidth * 0.5f * sinA);

    juce::Point<float> nTipL(nTip.x + needleTipWidth * 0.5f * cosA,
                            nTip.y + needleTipWidth * 0.5f * sinA);
    juce::Point<float> nTipR(nTip.x - needleTipWidth * 0.5f * cosA,
                            nTip.y - needleTipWidth * 0.5f * sinA);

    juce::Path needlePath;
    needlePath.startNewSubPath(nBaseL);
    needlePath.lineTo(nTipL);
    needlePath.lineTo(nTipR);
    needlePath.lineTo(nBaseR);
    needlePath.closeSubPath();

    // 5. Needle 3D Drop Shadow
    {
        juce::Graphics::ScopedSaveState shadowState(g);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillPath(needlePath, juce::AffineTransform::translation(2.0f, 2.5f));
    }

    // 6. Draw Tapered Needle
    g.setColour(juce::Colour(0xffc62828));
    g.fillPath(needlePath);

    g.setColour(juce::Colour(0x80ffffff));
    g.drawLine(juce::Line<float>(pivotPoint, nTip), 0.7f);

    // 7. Bottom Pivot Cap
    auto capBounds = juce::Rectangle<float>(18.0f, 18.0f).withCentre({ pivotPoint.x, bounds.getBottom() + 3.0f });

    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(capBounds.translated(0.0f, 1.5f));

    juce::ColourGradient capGrad(juce::Colour(0xff525862), capBounds.getX(), capBounds.getY(),
                                 juce::Colour(0xff121417), capBounds.getRight(), capBounds.getBottom(), true);
    capGrad.addColour(0.4, juce::Colour(0xff2d3138));
    g.setGradientFill(capGrad);
    g.fillEllipse(capBounds);

    g.setColour(juce::Colour(0xff15171a));
    g.drawEllipse(capBounds, 1.2f);

    g.setColour(juce::Colour(0x60ffffff));
    g.fillEllipse(juce::Rectangle<float>(4.0f, 4.0f).withCentre(capBounds.getCentre().translated(-1.5f, -1.5f)));
}

void VUMeterComponent::drawGainReductionVU(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    juce::Graphics::ScopedSaveState state(g);
    g.reduceClipRegion(bounds.toNearestInt());

    // 1. Vintage Warm Amber-Cream Dial Face (#F5EEDC to #E8D9B5)
    juce::ColourGradient dialGrad(juce::Colour(0xfff5eedc), bounds.getCentreX(), bounds.getY(),
                                  juce::Colour(0xffe8d9b5), bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill(dialGrad);
    g.fillRoundedRectangle(bounds, 2.5f);

    // 2. Incandescent Backlight Glow
    juce::ColourGradient bottomLampGlow(juce::Colour(0x90ffdf96), bounds.getCentreX(), bounds.getBottom(),
                                        juce::Colour(0x00ffe1a0), bounds.getCentreX(), bounds.getCentreY() - 10.0f, true);
    g.setGradientFill(bottomLampGlow);
    g.fillRoundedRectangle(bounds, 2.5f);

    auto pivotPoint = juce::Point<float>(bounds.getCentreX(), bounds.getBottom() + bounds.getHeight() * 0.45f);
    float radius = bounds.getHeight() * 1.30f;

    // Authentic 1176 GR Scale: 0 dB on right (+35 deg), -20 dB on left (-35 deg)
    const float startAngle = -38.0f; // -20 dB GR
    const float endAngle   = 38.0f;  // 0 dB GR (Rest position)

    // GR Arc
    juce::Path grArc;
    grArc.addCentredArc(pivotPoint.x, pivotPoint.y, radius - 16.0f, radius - 16.0f, 0.0f,
                        juce::degreesToRadians(startAngle), juce::degreesToRadians(endAngle), true);
    g.setColour(juce::Colour(0xdd1a1c1e));
    g.strokePath(grArc, juce::PathStrokeType(1.8f));

    // 1176 GR Ticks: 0 to 20 dB of Gain Reduction
    struct GRTick { float grDb; const char* label; bool isMajor; };
    const GRTick grTicks[] = {
        { 20.0f, "20", true },
        { 15.0f, "15", true },
        { 10.0f, "10", true },
        {  7.0f, "7",  true },
        {  5.0f, "5",  true },
        {  3.0f, "3",  true },
        {  2.0f, "2",  false },
        {  1.0f, "1",  false },
        {  0.0f, "0",  true }
    };

    g.setFont(juce::Font("Helvetica Neue", 8.0f, juce::Font::bold));

    for (const auto& tick : grTicks)
    {
        // 0 GR = endAngle (+38 deg), 20 GR = startAngle (-38 deg)
        float angleDeg = juce::jmap(tick.grDb, 20.0f, 0.0f, startAngle, endAngle);
        float angleRad = juce::degreesToRadians(angleDeg);

        float rArc = radius - 16.0f;
        float tickLength = tick.isMajor ? 6.5f : 4.0f;

        juce::Point<float> p1(pivotPoint.x + rArc * std::sin(angleRad),
                              pivotPoint.y - rArc * std::cos(angleRad));
        juce::Point<float> p2(pivotPoint.x + (rArc + tickLength) * std::sin(angleRad),
                              pivotPoint.y - (rArc + tickLength) * std::cos(angleRad));

        g.setColour(juce::Colour(0xee1c1e22));
        g.drawLine(juce::Line<float>(p1, p2), tick.isMajor ? 1.5f : 1.0f);

        if (tick.isMajor)
        {
            float rText = rArc + 12.0f;
            juce::Point<float> pText(pivotPoint.x + rText * std::sin(angleRad),
                                     pivotPoint.y - rText * std::cos(angleRad));

            auto textRect = juce::Rectangle<float>(18.0f, 10.0f).withCentre(pText);
            g.drawText(tick.label, textRect, juce::Justification::centred, false);
        }
    }

    // "GAIN REDUCTION" title
    g.setFont(juce::Font("Helvetica Neue", 8.0f, juce::Font::bold));
    g.setColour(juce::Colour(0xaa22252a));
    auto grBadgeRect = juce::Rectangle<float>(80.0f, 14.0f).withCentre({ bounds.getCentreX(), bounds.getCentreY() + 8.0f });
    g.drawText("GAIN REDUCTION", grBadgeRect, juce::Justification::centred, false);

    // Needle position: currentLevelDb is positive GR amount in dB (0 = 0dB GR, 20 = 20dB GR)
    float grAmount = juce::jlimit(0.0f, 25.0f, std::abs(currentLevelDb));
    float needleDeg = juce::jmap(grAmount, 20.0f, 0.0f, startAngle, endAngle);
    needleDeg = juce::jlimit(startAngle - 5.0f, endAngle + 5.0f, needleDeg);
    float needleAngleRad = juce::degreesToRadians(needleDeg);

    float needleLength = radius - 6.0f;
    float needleTipWidth = 0.8f;
    float needleBaseWidth = 3.2f;

    float cosA = std::cos(needleAngleRad);
    float sinA = std::sin(needleAngleRad);

    juce::Point<float> nTip(pivotPoint.x + needleLength * sinA,
                            pivotPoint.y - needleLength * cosA);

    juce::Point<float> nBaseL(pivotPoint.x + needleBaseWidth * 0.5f * cosA,
                             pivotPoint.y + needleBaseWidth * 0.5f * sinA);
    juce::Point<float> nBaseR(pivotPoint.x - needleBaseWidth * 0.5f * cosA,
                             pivotPoint.y - needleBaseWidth * 0.5f * sinA);

    juce::Point<float> nTipL(nTip.x + needleTipWidth * 0.5f * cosA,
                            nTip.y + needleTipWidth * 0.5f * sinA);
    juce::Point<float> nTipR(nTip.x - needleTipWidth * 0.5f * cosA,
                            nTip.y - needleTipWidth * 0.5f * sinA);

    juce::Path needlePath;
    needlePath.startNewSubPath(nBaseL);
    needlePath.lineTo(nTipL);
    needlePath.lineTo(nTipR);
    needlePath.lineTo(nBaseR);
    needlePath.closeSubPath();

    // Needle 3D Drop Shadow
    {
        juce::Graphics::ScopedSaveState shadowState(g);
        g.setColour(juce::Colours::black.withAlpha(0.35f));
        g.fillPath(needlePath, juce::AffineTransform::translation(2.0f, 2.5f));
    }

    // Needle in dark crimson
    g.setColour(juce::Colour(0xffc62828));
    g.fillPath(needlePath);

    g.setColour(juce::Colour(0x80ffffff));
    g.drawLine(juce::Line<float>(pivotPoint, nTip), 0.7f);

    // Bottom Pivot Cap
    auto capBounds = juce::Rectangle<float>(18.0f, 18.0f).withCentre({ pivotPoint.x, bounds.getBottom() + 3.0f });

    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(capBounds.translated(0.0f, 1.5f));

    juce::ColourGradient capGrad(juce::Colour(0xff525862), capBounds.getX(), capBounds.getY(),
                                 juce::Colour(0xff121417), capBounds.getRight(), capBounds.getBottom(), true);
    capGrad.addColour(0.4, juce::Colour(0xff2d3138));
    g.setGradientFill(capGrad);
    g.fillEllipse(capBounds);

    g.setColour(juce::Colour(0xff15171a));
    g.drawEllipse(capBounds, 1.2f);

    g.setColour(juce::Colour(0x60ffffff));
    g.fillEllipse(juce::Rectangle<float>(4.0f, 4.0f).withCentre(capBounds.getCentre().translated(-1.5f, -1.5f)));
}

void VUMeterComponent::drawGainReduction(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // 16-segment horizontal LED Gain Reduction Array fallback
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font("Helvetica Neue", 10.0f, juce::Font::bold));
    g.drawText(titleText, bounds.removeFromTop(12.0f), juce::Justification::centredLeft, false);

    int numLeds = 16;
    auto barArea = bounds.reduced(0.0f, 2.0f);
    float ledWidth = (barArea.getWidth() - (numLeds - 1) * 2.0f) / (float)numLeds;

    float currentGR = juce::jlimit(0.0f, 30.0f, std::abs(currentLevelDb));
    int activeCount = juce::roundToInt((currentGR / 30.0f) * numLeds);

    for (int i = 0; i < numLeds; ++i)
    {
        auto ledRect = juce::Rectangle<float>(barArea.getX() + i * (ledWidth + 2.0f), barArea.getY(), ledWidth, barArea.getHeight());
        bool isActive = (i < activeCount);
        
        juce::Colour ledColor;
        if (i < 4)       ledColor = rackstrip::VintageRackLookAndFeel::Colors::ledOnGreen;
        else if (i < 12) ledColor = rackstrip::VintageRackLookAndFeel::Colors::ledOnAmber;
        else             ledColor = rackstrip::VintageRackLookAndFeel::Colors::ledOnRed;

        if (isActive)
        {
            g.setColour(ledColor.withAlpha(0.55f));
            g.fillRoundedRectangle(ledRect.expanded(1.5f), 1.0f);
            
            g.setColour(ledColor);
            g.fillRoundedRectangle(ledRect, 1.0f);

            g.setColour(juce::Colours::white.withAlpha(0.6f));
            g.drawRoundedRectangle(ledRect, 1.0f, 0.6f);
        }
        else
        {
            g.setColour(juce::Colour(0xff181a1e));
            g.fillRoundedRectangle(ledRect, 1.0f);
            g.setColour(juce::Colours::black.withAlpha(0.6f));
            g.drawRoundedRectangle(ledRect, 1.0f, 1.0f);
        }
    }
}

} // namespace rackstrip
