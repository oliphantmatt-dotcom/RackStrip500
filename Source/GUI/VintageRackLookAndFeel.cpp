#include "VintageRackLookAndFeel.h"
#include "FaceplateModuleComponent.h"

namespace rackstrip
{

// Boutique vintage hardware palette
const juce::Colour VintageRackLookAndFeel::Colors::faceplateDark        = juce::Colour::fromRGB(18, 22, 28);
const juce::Colour VintageRackLookAndFeel::Colors::faceplateBezel       = juce::Colour::fromRGB(175, 180, 188);
const juce::Colour VintageRackLookAndFeel::Colors::bakeliteCapDark      = juce::Colour::fromRGB(10, 12, 14);
const juce::Colour VintageRackLookAndFeel::Colors::bakeliteCapHighlight = juce::Colour::fromRGB(55, 60, 68);
const juce::Colour VintageRackLookAndFeel::Colors::aluminumSilver       = juce::Colour::fromRGB(225, 230, 238);
const juce::Colour VintageRackLookAndFeel::Colors::brassAccent          = juce::Colour::fromRGB(235, 190, 50);
const juce::Colour VintageRackLookAndFeel::Colors::goldAmberText        = juce::Colour(0xffe5c158); // #E5C158 Glowing amber/gold
const juce::Colour VintageRackLookAndFeel::Colors::silkscreenWhite      = juce::Colour(0xfff0ead6); // #F0EAD6 Off-white/cream
const juce::Colour VintageRackLookAndFeel::Colors::pointerMetallic      = juce::Colour::fromRGB(255, 255, 255);
const juce::Colour VintageRackLookAndFeel::Colors::tickMarkColor        = juce::Colour(0xfff0ebd8);
const juce::Colour VintageRackLookAndFeel::Colors::ledOff               = juce::Colour::fromRGB(28, 32, 38);
const juce::Colour VintageRackLookAndFeel::Colors::ledOnRed             = juce::Colour::fromRGB(255, 52, 45);
const juce::Colour VintageRackLookAndFeel::Colors::ledOnAmber           = juce::Colour::fromRGB(255, 175, 25);
const juce::Colour VintageRackLookAndFeel::Colors::ledOnGreen           = juce::Colour::fromRGB(45, 235, 95);
const juce::Colour VintageRackLookAndFeel::Colors::buttonTextOff         = juce::Colour::fromRGB(165, 172, 180);
const juce::Colour VintageRackLookAndFeel::Colors::buttonTextOn          = juce::Colour::fromRGB(255, 255, 255);

VintageRackLookAndFeel::VintageRackLookAndFeel()
{
    setColour(juce::Slider::rotarySliderFillColourId, Colors::brassAccent);
    setColour(juce::Slider::rotarySliderOutlineColourId, Colors::faceplateBezel);
    setColour(juce::Slider::textBoxTextColourId, Colors::goldAmberText);
    setColour(juce::Slider::textBoxBackgroundColourId, juce::Colour(0xff121314));
    setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff2a2d33));
    setColour(juce::ToggleButton::textColourId, Colors::silkscreenWhite);
    setColour(juce::TextButton::textColourOffId, Colors::buttonTextOff);
    setColour(juce::TextButton::textColourOnId, Colors::buttonTextOn);
    
    setDefaultSansSerifTypefaceName("Helvetica Neue");
}

juce::Label* VintageRackLookAndFeel::createSliderTextBox(juce::Slider& slider)
{
    auto* l = juce::LookAndFeel_V4::createSliderTextBox(slider);
    l->setFont(juce::Font("Helvetica Neue", 10.5f, juce::Font::bold));
    l->setJustificationType(juce::Justification::centred);
    
    // Recessed dark bezel (#121314) with glowing amber/gold (#E5C158) numeric text
    l->setColour(juce::Label::textColourId, Colors::goldAmberText);
    l->setColour(juce::Label::backgroundColourId, juce::Colour(0xff121314));
    l->setColour(juce::Label::outlineColourId, juce::Colour(0xff2a2d33));

    return l;
}

void VintageRackLookAndFeel::drawRotarySlider(juce::Graphics& g,
                                              int x, int y, int width, int height,
                                              float sliderPosProportional,
                                              float rotaryStartAngle, float rotaryEndAngle,
                                              juce::Slider& slider)
{
    const float rawDiameter = (float)juce::jmin(width, height);
    const float diameter = juce::jmax(18.0f, rawDiameter - 24.0f);
    const float radius = diameter * 0.5f;

    const auto center = juce::Point<float>((float)x + (float)width * 0.5f, (float)y + (float)height * 0.5f);
    const float currentAngle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

    // Identify control type
    juce::String styleProp = slider.getProperties().getWithDefault("knobStyle", "").toString();
    juce::String sliderName = slider.getName().toLowerCase();
    juce::String componentId = slider.getComponentID().toLowerCase();

    bool isPointerKnob = (styleProp == "pointer")
                         || sliderName.contains("comp")
                         || componentId.contains("comp")
                         || slider.getProperties().getWithDefault("isPointer", false);

    bool isSilverSkirt = (styleProp == "silverSkirt" || styleProp == "skirted")
                         || sliderName.contains("eq")
                         || componentId.contains("eq")
                         || slider.getProperties().getWithDefault("isSilverSkirt", false);

    bool isBipolar = componentId.contains("gain")
                     || componentId.contains("trim")
                     || slider.getProperties().getWithDefault("isBipolar", false);

    // Band Color for EQ Knobs:
    juce::Colour eqCapColor = juce::Colour(0xffe5a93c); // LF: Yellow
    if (componentId.contains("lmf") || sliderName.contains("lmf"))
        eqCapColor = juce::Colour(0xff2563eb); // LMF: Royal Blue
    else if (componentId.contains("hmf") || sliderName.contains("hmf"))
        eqCapColor = juce::Colour(0xff16a34a); // HMF: Racing Green
    else if (componentId.contains("high") || sliderName.contains("hf"))
        eqCapColor = juce::Colour(0xffdc2626); // HF: Crimson Red

    // 1. Engraved Circular Dial Scale & Tick Marks
    const float tickOuterRadius = radius + 9.0f;
    const float tickInnerRadius = radius + 3.5f;
    int numTicks = slider.getProperties().getWithDefault("numTicks", 11);

    juce::Colour tickCol = Colors::silkscreenWhite;
    if (auto* faceplate = slider.findParentComponentOfClass<FaceplateModuleComponent>())
    {
        if (faceplate->getModuleStyle() == FaceplateModuleComponent::ModuleStyle::SilverAluminum)
            tickCol = juce::Colour(0xff181a1d); // High contrast dark silkscreen on 1176 faceplate
    }

    if (!isSilverSkirt)
    {
        drawKnobTickMarks(g, center, tickInnerRadius, tickOuterRadius, rotaryStartAngle, rotaryEndAngle, numTicks, 5, tickCol, isBipolar, componentId);
    }

    // 2. Soft Gaussian Outer Drop Shadow
    const float knobRadius = radius;
    juce::Rectangle<float> knobBounds(center.x - knobRadius, center.y - knobRadius, knobRadius * 2.0f, knobRadius * 2.0f);

    {
        juce::Graphics::ScopedSaveState state(g);
        for (float r = 1.0f; r <= 6.0f; r += 1.0f)
        {
            float alpha = 0.35f - (r * 0.05f);
            g.setColour(juce::Colours::black.withAlpha(juce::jmax(0.02f, alpha)));
            g.fillEllipse(knobBounds.translated(0.0f, r + 1.5f).expanded(r * 0.75f));
        }
    }

    // 3. Render 3D Vector Shaded Knob
    if (isPointerKnob)
    {
        drawPointerKnob(g, knobBounds, currentAngle);
    }
    else if (isSilverSkirt)
    {
        drawSilverSkirtKnob(g, knobBounds, currentAngle, sliderPosProportional, rotaryStartAngle, rotaryEndAngle, isBipolar, eqCapColor);
    }
    else
    {
        bool isBrushedAluminum = slider.getProperties().getWithDefault("brushedAluminum", false);
        drawStandardBakeliteKnob(g, knobBounds, currentAngle, isBrushedAluminum);
    }
}

void VintageRackLookAndFeel::drawKnobTickMarks(juce::Graphics& g,
                                               juce::Point<float> center,
                                               float innerRadius,
                                               float outerRadius,
                                               float startAngle,
                                               float endAngle,
                                               int numTicks,
                                               int majorTickInterval,
                                               juce::Colour tickColor,
                                               bool isBipolarCenterZero,
                                               const juce::String& knobType)
{
    if (numTicks <= 1) return;

    const float angleStep = (endAngle - startAngle) / (float)(numTicks - 1);
    const int centerIndex = (numTicks - 1) / 2;

    // Check for 1176 Attack/Release (1-7 scale markings)
    bool is1176AttRel = knobType.contains("attack") || knobType.contains("release");

    for (int i = 0; i < numTicks; ++i)
    {
        const float angle = startAngle + (float)i * angleStep;
        const bool isCenter = isBipolarCenterZero && (i == centerIndex);
        const bool isMajor = (i % majorTickInterval == 0) || (i == 0) || (i == numTicks - 1) || isCenter;

        const float currentInnerRadius = (isMajor || isCenter) ? innerRadius - 2.0f : innerRadius;
        const float currentOuterRadius = isCenter ? outerRadius + 2.5f : (isMajor ? outerRadius + 1.0f : outerRadius);
        const float strokeWidth        = isCenter ? 2.0f : (isMajor ? 1.5f : 1.0f);

        juce::Point<float> pInner(center.x + currentInnerRadius * std::sin(angle),
                                  center.y - currentInnerRadius * std::cos(angle));
        juce::Point<float> pOuter(center.x + currentOuterRadius * std::sin(angle),
                                  center.y - currentOuterRadius * std::cos(angle));

        // Engraved white enamel tick marks
        g.setColour(isCenter ? juce::Colour(0xffe5c158) : tickColor.withAlpha(isMajor ? 0.95f : 0.55f));
        g.drawLine(juce::Line<float>(pInner, pOuter), strokeWidth);

        // Dot markers at major intervals
        if (isMajor && !isCenter)
        {
            juce::Point<float> pDot(center.x + (outerRadius + 3.0f) * std::sin(angle),
                                    center.y - (outerRadius + 3.0f) * std::cos(angle));
            g.fillEllipse(juce::Rectangle<float>(2.0f, 2.0f).withCentre(pDot));
        }
    }

    // Numerical scale markings for 1176 Attack / Release (1 to 7)
    if (is1176AttRel)
    {
        g.setFont(juce::Font("Helvetica Neue", 7.5f, juce::Font::bold));
        g.setColour(tickColor.withAlpha(0.85f));
        for (int i = 1; i <= 7; ++i)
        {
            float tVal = (float)(i - 1) / 6.0f;
            float tAngle = startAngle + tVal * (endAngle - startAngle);
            float rText = outerRadius + 6.0f;
            juce::Point<float> pt(center.x + rText * std::sin(tAngle), center.y - rText * std::cos(tAngle));
            g.drawText(juce::String(i), juce::Rectangle<float>(12.0f, 10.0f).withCentre(pt), juce::Justification::centred, false);
        }
    }
}

void VintageRackLookAndFeel::drawStandardBakeliteKnob(juce::Graphics& g,
                                                      juce::Rectangle<float> bounds,
                                                      float angle,
                                                      bool isBrushedAluminum)
{
    const auto center = bounds.getCentre();
    const float radius = bounds.getWidth() * 0.5f;

    // 1. Machined Metallic Bezel with Top-Left Specular Highlight
    juce::ColourGradient bezelGrad(juce::Colour(0xfff8fafc), center.x - radius * 0.7f, center.y - radius * 0.7f,
                                   juce::Colour(0xff363a42), center.x + radius * 0.8f, center.y + radius * 0.8f, false);
    g.setGradientFill(bezelGrad);
    g.fillEllipse(bounds);

    g.setColour(juce::Colour(0x70000000));
    g.drawEllipse(bounds, 1.2f);

    auto capBounds = bounds.reduced(radius * 0.08f);
    const float capRadius = capBounds.getWidth() * 0.5f;

    if (isBrushedAluminum)
    {
        juce::ColourGradient alumGrad(juce::Colour(0xfff6f8fb), center.x - capRadius * 0.6f, center.y - capRadius * 0.6f,
                                      juce::Colour(0xff686e7a), center.x + capRadius * 0.8f, center.y + capRadius * 0.8f, true);
        alumGrad.addColour(0.35, juce::Colour(0xffd0d5dc));
        alumGrad.addColour(0.70, juce::Colour(0xff989fa9));
        g.setGradientFill(alumGrad);
        g.fillEllipse(capBounds);
    }
    else
    {
        // 2. Gloss-Black Bakelite Body with Top-Left Specular Reflection
        juce::ColourGradient bakeliteGrad(juce::Colour(0xff454b56), center.x - capRadius * 0.5f, center.y - capRadius * 0.5f,
                                          juce::Colour(0xff08090b), center.x + capRadius * 0.8f, center.y + capRadius * 0.8f, true);
        bakeliteGrad.addColour(0.45, juce::Colour(0xff14161a));
        g.setGradientFill(bakeliteGrad);
        g.fillEllipse(capBounds);

        // 3. Side Knurling / Fluting (Alternating 1px Highlight #FFFFFF33 and Shadow #00000088)
        const int numRibs = 26;
        for (int i = 0; i < numRibs; ++i)
        {
            float ribAngle = (float)i * (juce::MathConstants<float>::twoPi / (float)numRibs);
            juce::Point<float> p1(center.x + (capRadius - 3.2f) * std::sin(ribAngle), center.y - (capRadius - 3.2f) * std::cos(ribAngle));
            juce::Point<float> p2(center.x + capRadius * std::sin(ribAngle), center.y - capRadius * std::cos(ribAngle));

            g.setColour(juce::Colour(0x88000000));
            g.drawLine(juce::Line<float>(p1, p2), 1.4f);

            float hAngle = ribAngle + 0.04f;
            juce::Point<float> h1(center.x + (capRadius - 3.2f) * std::sin(hAngle), center.y - (capRadius - 3.2f) * std::cos(hAngle));
            juce::Point<float> h2(center.x + capRadius * std::sin(hAngle), center.y - capRadius * std::cos(hAngle));
            g.setColour(juce::Colour(0x33ffffff));
            g.drawLine(juce::Line<float>(h1, h2), 1.0f);
        }

        // 4. Concentric Spun Brass / Machined Aluminum Center Inlay with Beveled Chamfer
        auto brassBounds = capBounds.reduced(capRadius * 0.40f);
        juce::ColourGradient brassGrad(juce::Colour(0xffffe27a), brassBounds.getX(), brassBounds.getY(),
                                       juce::Colour(0xff7a5912), brassBounds.getRight(), brassBounds.getBottom(), true);
        brassGrad.addColour(0.35, Colors::brassAccent);
        brassGrad.addColour(0.70, juce::Colour(0xffa8801d));
        g.setGradientFill(brassGrad);
        g.fillEllipse(brassBounds);

        g.setColour(juce::Colour(0x50ffffff));
        g.drawEllipse(brassBounds.reduced(0.8f), 0.9f);
        g.setColour(juce::Colour(0x88000000));
        g.drawEllipse(brassBounds, 1.2f);
    }

    // 5. Inset Razor-Sharp Enamel Pointer Line with Shadow Groove
    const float ptrInner = capRadius * 0.20f;
    const float ptrOuter = capRadius - 1.2f;
    juce::Point<float> pStart(center.x + ptrInner * std::sin(angle), center.y - ptrInner * std::cos(angle));
    juce::Point<float> pEnd(center.x + ptrOuter * std::sin(angle), center.y - ptrOuter * std::cos(angle));

    g.setColour(juce::Colours::black.withAlpha(0.65f));
    g.drawLine(juce::Line<float>(pStart.translated(0.8f, 0.8f), pEnd.translated(0.8f, 0.8f)), 2.6f);

    g.setColour(Colors::pointerMetallic);
    g.drawLine(juce::Line<float>(pStart, pEnd), 2.0f);
}

void VintageRackLookAndFeel::drawPointerKnob(juce::Graphics& g,
                                             juce::Rectangle<float> bounds,
                                             float angle)
{
    const auto center = bounds.getCentre();
    const float radius = bounds.getWidth() * 0.5f;

    // 1. Machined Base with Top-Left Highlight
    juce::ColourGradient baseGrad(juce::Colour(0xff757c88), center.x - radius * 0.7f, center.y - radius * 0.7f,
                                  juce::Colour(0xff141619), center.x + radius * 0.8f, center.y + radius * 0.8f, true);
    g.setGradientFill(baseGrad);
    g.fillEllipse(bounds);

    // 2. 1176 Gloss-Black Bakelite Teardrop Pointer Beak
    const float beakLength = radius * 1.05f;
    const float beakBaseWidth = radius * 0.40f;

    juce::Path pointerShape;
    pointerShape.addEllipse(bounds.reduced(radius * 0.16f));

    juce::Point<float> tip(center.x + beakLength * std::sin(angle), center.y - beakLength * std::cos(angle));
    float normAngle1 = angle + juce::MathConstants<float>::halfPi;
    float normAngle2 = angle - juce::MathConstants<float>::halfPi;

    juce::Point<float> b1(center.x + beakBaseWidth * std::sin(normAngle1), center.y - beakBaseWidth * std::cos(normAngle1));
    juce::Point<float> b2(center.x + beakBaseWidth * std::sin(normAngle2), center.y - beakBaseWidth * std::cos(normAngle2));

    juce::Path beakPath;
    beakPath.startNewSubPath(b1);
    beakPath.lineTo(tip);
    beakPath.lineTo(b2);
    beakPath.closeSubPath();

    // Drop shadow under snout
    {
        juce::Graphics::ScopedSaveState state(g);
        g.setColour(juce::Colours::black.withAlpha(0.45f));
        g.fillPath(beakPath, juce::AffineTransform::translation(1.2f, 1.8f));
    }

    // Gloss Bakelite Shading with top-left specular highlight
    juce::ColourGradient bodyGrad(juce::Colour(0xff454a54), center.x - radius * 0.5f, center.y - radius * 0.5f,
                                  juce::Colour(0xff090a0c), center.x + radius * 0.7f, center.y + radius * 0.7f, true);
    bodyGrad.addColour(0.5, juce::Colour(0xff181b20));

    g.setGradientFill(bodyGrad);
    g.fillPath(pointerShape);
    g.fillPath(beakPath);

    g.setColour(juce::Colour(0xff0f1114));
    g.strokePath(pointerShape, juce::PathStrokeType(1.2f));
    g.strokePath(beakPath, juce::PathStrokeType(1.2f));

    // 3. Spun Aluminum Center Cap with Beveled Chamfer
    auto innerDiscBounds = bounds.reduced(radius * 0.44f);
    juce::ColourGradient discGrad(juce::Colour(0xffffffff), innerDiscBounds.getX(), innerDiscBounds.getY(),
                                  juce::Colour(0xff727882), innerDiscBounds.getRight(), innerDiscBounds.getBottom(), true);
    discGrad.addColour(0.4, juce::Colour(0xffc8ccd4));
    g.setGradientFill(discGrad);
    g.fillEllipse(innerDiscBounds);

    g.setColour(juce::Colour(0x90000000));
    g.drawEllipse(innerDiscBounds, 1.2f);
    g.setColour(juce::Colour(0x50ffffff));
    g.drawEllipse(innerDiscBounds.reduced(0.8f), 0.8f);

    // 4. Inset Pointer Line filled with Enamel White (#FFFFFF)
    juce::Point<float> pStripeStart(center.x + (radius * 0.12f) * std::sin(angle), center.y - (radius * 0.12f) * std::cos(angle));
    juce::Point<float> pStripeEnd(center.x + (beakLength - 1.2f) * std::sin(angle), center.y - (beakLength - 1.2f) * std::cos(angle));

    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawLine(juce::Line<float>(pStripeStart.translated(0.8f, 0.8f), pStripeEnd.translated(0.8f, 0.8f)), 2.6f);

    g.setColour(juce::Colours::white);
    g.drawLine(juce::Line<float>(pStripeStart, pStripeEnd), 2.0f);
}

void VintageRackLookAndFeel::drawSilverSkirtKnob(juce::Graphics& g,
                                                 juce::Rectangle<float> bounds,
                                                 float angle,
                                                 float sliderPosProportional,
                                                 float startAngle,
                                                 float endAngle,
                                                 bool isBipolarCenterZero,
                                                 juce::Colour capAccentColor)
{
    juce::ignoreUnused(sliderPosProportional);
    const auto center = bounds.getCentre();
    const float radius = bounds.getWidth() * 0.5f;

    // 1. Machined Silver Skirt Base with Radial Lighting
    juce::ColourGradient skirtGrad(juce::Colour(0xffffffff), center.x - radius * 0.65f, center.y - radius * 0.65f,
                                   juce::Colour(0xff808792), center.x + radius * 0.75f, center.y + radius * 0.75f, true);
    skirtGrad.addColour(0.30, juce::Colour(0xffdfe3e9));
    skirtGrad.addColour(0.65, juce::Colour(0xffa8afb9));
    skirtGrad.addColour(0.90, juce::Colour(0xff6e747f));
    g.setGradientFill(skirtGrad);
    g.fillEllipse(bounds);

    g.setColour(juce::Colour(0x55000000));
    g.drawEllipse(bounds, 1.2f);

    // 2. Skirt Graduation Ticks (0-10 style markings around skirt)
    const int skirtTicks = 11;
    const float skirtTickStep = (endAngle - startAngle) / (float)(skirtTicks - 1);
    const int centerIdx = (skirtTicks - 1) / 2;

    for (int i = 0; i < skirtTicks; ++i)
    {
        float tAngle = startAngle + (float)i * skirtTickStep;
        float rInner = radius * 0.76f;
        float rOuter = radius * 0.94f;
        bool isCenter = isBipolarCenterZero && (i == centerIdx);

        if (i % 5 == 0 || isCenter) rInner = radius * 0.68f;

        juce::Point<float> p1(center.x + rInner * std::sin(tAngle), center.y - rInner * std::cos(tAngle));
        juce::Point<float> p2(center.x + rOuter * std::sin(tAngle), center.y - rOuter * std::cos(tAngle));

        g.setColour(isCenter ? juce::Colour(0xffd32f2f) : juce::Colour(0xff22252a));
        g.drawLine(juce::Line<float>(p1, p2), (i % 5 == 0 || isCenter) ? 1.4f : 0.8f);
    }

    // Center "0" reference legend for bipolar gain knobs
    if (isBipolarCenterZero)
    {
        g.setFont(juce::Font("Helvetica Neue", 7.0f, juce::Font::bold));
        g.setColour(juce::Colour(0xffd32f2f));
        juce::Point<float> p0(center.x, center.y - radius * 0.84f);
        g.drawText("0", juce::Rectangle<float>(10.0f, 8.0f).withCentre(p0), juce::Justification::centred, false);
    }

    // 3. Raised Fluted Bakelite Cylinder
    auto innerCylinderBounds = bounds.reduced(radius * 0.28f);
    const float innerRadius = innerCylinderBounds.getWidth() * 0.5f;

    g.setColour(juce::Colours::black.withAlpha(0.45f));
    g.fillEllipse(innerCylinderBounds.translated(0.0f, 1.8f));

    juce::ColourGradient cylGrad(juce::Colour(0xff343840), center.x - innerRadius * 0.5f, center.y - innerRadius * 0.5f,
                                 juce::Colour(0xff0c0e11), center.x + innerRadius * 0.7f, center.y + innerRadius * 0.7f, true);
    g.setGradientFill(cylGrad);
    g.fillEllipse(innerCylinderBounds);

    // Fluted side grooves
    const int numGrooves = 18;
    for (int i = 0; i < numGrooves; ++i)
    {
        float gAngle = (float)i * (juce::MathConstants<float>::twoPi / (float)numGrooves);
        juce::Point<float> g1(center.x + (innerRadius - 2.5f) * std::sin(gAngle), center.y - (innerRadius - 2.5f) * std::cos(gAngle));
        juce::Point<float> g2(center.x + innerRadius * std::sin(gAngle), center.y - innerRadius * std::cos(gAngle));
        g.setColour(juce::Colour(0x88000000));
        g.drawLine(juce::Line<float>(g1, g2), 1.3f);
    }

    // 4. Center Colored Console Inlay Cap (Yellow LF, Blue LMF, Green HMF, Red HF)
    auto capBounds = innerCylinderBounds.reduced(innerRadius * 0.32f);
    juce::ColourGradient capGrad(capAccentColor.brighter(0.45f), capBounds.getX(), capBounds.getY(),
                                 capAccentColor.darker(0.65f), capBounds.getRight(), capBounds.getBottom(), true);
    capGrad.addColour(0.35, capAccentColor);
    capGrad.addColour(0.70, capAccentColor.darker(0.3f));
    g.setGradientFill(capGrad);
    g.fillEllipse(capBounds);

    g.setColour(juce::Colour(0x70000000));
    g.drawEllipse(capBounds, 1.1f);
    g.setColour(juce::Colour(0x50ffffff));
    g.drawEllipse(capBounds.reduced(0.8f), 0.8f);

    // 5. Razor-Sharp Indicator Line filled with Enamel White
    juce::Point<float> pInner(center.x + (innerRadius * 0.15f) * std::sin(angle), center.y - (innerRadius * 0.15f) * std::cos(angle));
    juce::Point<float> pOuter(center.x + (radius * 0.98f) * std::sin(angle), center.y - (radius * 0.98f) * std::cos(angle));

    g.setColour(juce::Colours::black.withAlpha(0.6f));
    g.drawLine(juce::Line<float>(pInner.translated(0.8f, 0.8f), pOuter.translated(0.8f, 0.8f)), 2.4f);

    g.setColour(juce::Colours::white);
    g.drawLine(juce::Line<float>(pInner, pOuter), 2.0f);
}

void VintageRackLookAndFeel::drawDualConcentricKnob(juce::Graphics& g,
                                                    juce::Rectangle<float> bounds,
                                                    float angle,
                                                    bool isInnerConcentric,
                                                    juce::Slider& slider)
{
    juce::ignoreUnused(slider);
    drawStandardBakeliteKnob(g, bounds, angle, isInnerConcentric);
}

void VintageRackLookAndFeel::drawToggleButton(juce::Graphics& g,
                                              juce::ToggleButton& button,
                                              bool shouldDrawButtonAsHighlighted,
                                              bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted);
    auto bounds = button.getLocalBounds().toFloat().reduced(2.0f);
    if (bounds.isEmpty()) return;

    const bool isToggled = button.getToggleState();

    if (shouldDrawButtonAsDown)
        bounds = bounds.translated(0.0f, 1.0f);

    // 1. Dark Recessed Chassis Cutout with Drop Shadow
    g.setColour(juce::Colour(0xff090a0c));
    g.fillRoundedRectangle(bounds, 3.5f);

    g.setColour(juce::Colours::black.withAlpha(0.7f));
    g.drawRoundedRectangle(bounds, 3.5f, 1.2f);

    auto btnFace = bounds.reduced(1.5f);

    // 2. 1.5px Beveled Chrome Bezel
    juce::ColourGradient chromeGrad(juce::Colour(0xffd8dee6), btnFace.getX(), btnFace.getY(),
                                    juce::Colour(0xff48505c), btnFace.getRight(), btnFace.getBottom(), false);
    g.setGradientFill(chromeGrad);
    g.drawRoundedRectangle(btnFace, 2.5f, 1.5f);

    auto innerLens = btnFace.reduced(1.0f);

    // 3. Matte Black Button Body & Intense Diffused LED Backlighting
    if (isToggled)
    {
        juce::String txt = button.getButtonText().toUpperCase();
        juce::Colour ledColor = Colors::ledOnAmber;

        if (txt.contains("BYPASS") || txt.contains("PHASE") || txt.contains("ALL"))
            ledColor = Colors::ledOnRed;
        else if (txt.contains("BUMP") || txt.contains("ANALOG") || txt.contains("4:1") || txt.contains("8:1") || txt.contains("12:1") || txt.contains("20:1"))
            ledColor = Colors::ledOnAmber;
        else if (txt.contains("BELL") || txt.contains("SHELF"))
            ledColor = Colors::ledOnGreen;

        // Outer LED bloom
        g.setColour(ledColor.withAlpha(0.35f));
        g.drawRoundedRectangle(btnFace.expanded(1.5f), 3.0f, 1.5f);

        // Diffused glowing lens
        juce::ColourGradient diffuseGlow(juce::Colours::white, innerLens.getCentreX(), innerLens.getCentreY(),
                                         ledColor.darker(0.3f), innerLens.getRight(), innerLens.getBottom(), true);
        diffuseGlow.addColour(0.25, ledColor.brighter(0.4f));
        diffuseGlow.addColour(0.70, ledColor);
        g.setGradientFill(diffuseGlow);
        g.fillRoundedRectangle(innerLens, 2.0f);

        g.setColour(juce::Colours::white.withAlpha(0.75f));
        g.drawRoundedRectangle(innerLens, 2.0f, 0.8f);

        g.setFont(juce::Font("Helvetica Neue", 10.5f, juce::Font::bold));

        g.setColour(ledColor.brighter(0.9f).withAlpha(0.85f));
        g.drawText(button.getButtonText(), innerLens.toNearestInt(), juce::Justification::centred, true);

        g.setColour(juce::Colours::white);
        g.drawText(button.getButtonText(), innerLens.toNearestInt(), juce::Justification::centred, true);
    }
    else
    {
        // Matte Black Body (#181A1D to #0E1012)
        juce::ColourGradient offGrad(juce::Colour(0xff22252a), innerLens.getX(), innerLens.getY(),
                                     juce::Colour(0xff101214), innerLens.getX(), innerLens.getBottom(), false);
        g.setGradientFill(offGrad);
        g.fillRoundedRectangle(innerLens, 2.0f);

        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawHorizontalLine(static_cast<int>(innerLens.getY()), innerLens.getX() + 1.0f, innerLens.getRight() - 1.0f);

        g.setFont(juce::Font("Helvetica Neue", 10.5f, juce::Font::bold));
        g.setColour(Colors::buttonTextOff);
        g.drawText(button.getButtonText(), innerLens.toNearestInt(), juce::Justification::centred, true);
    }
}

void VintageRackLookAndFeel::drawButtonBackground(juce::Graphics& g,
                                                  juce::Button& button,
                                                  const juce::Colour& backgroundColour,
                                                  bool shouldDrawButtonAsHighlighted,
                                                  bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    auto bounds = button.getLocalBounds().toFloat().reduced(1.0f);
    if (bounds.isEmpty()) return;

    const bool isToggled = button.getToggleState();

    if (isToggled)
    {
        juce::Colour activeGlowColour = (backgroundColour != juce::Colour()) ? backgroundColour : Colors::ledOnAmber;
        g.setColour(activeGlowColour);
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }
    else
    {
        g.setColour(juce::Colour(0xff22252a));
        g.fillRoundedRectangle(bounds, 3.0f);
        g.setColour(juce::Colour(0xff4a5059));
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
    }
}

void VintageRackLookAndFeel::drawButtonText(juce::Graphics& g,
                                             juce::TextButton& button,
                                             bool shouldDrawButtonAsHighlighted,
                                             bool shouldDrawButtonAsDown)
{
    juce::ignoreUnused(shouldDrawButtonAsHighlighted, shouldDrawButtonAsDown);
    g.setFont(juce::Font("Helvetica Neue", 11.0f, juce::Font::bold));
    g.setColour(button.getToggleState() ? Colors::buttonTextOn : Colors::buttonTextOff);
    g.drawText(button.getButtonText(), button.getLocalBounds(), juce::Justification::centred, true);
}

} // namespace rackstrip
