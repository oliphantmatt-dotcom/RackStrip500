#include "FaceplateModuleComponent.h"

FaceplateModuleComponent::FaceplateModuleComponent(const juce::String& moduleTitle,
                                                     ModuleStyle style,
                                                     bool shouldShowScrews)
    : titleText(moduleTitle),
      moduleStyle(style),
      showScrews(shouldShowScrews)
{
    updateColorsForStyle();
    setOpaque(true);
    setBufferedToImage(true);
}

FaceplateModuleComponent::FaceplateModuleComponent(const juce::String& moduleTitle,
                                                     juce::Colour colour,
                                                     bool shouldShowScrews)
    : titleText(moduleTitle),
      moduleStyle(ModuleStyle::Custom),
      baseColour(colour),
      secondaryColour(colour.darker(0.25f)),
      showScrews(shouldShowScrews)
{
    headerTextColour = juce::Colour(0xffdcdfe2);
    setOpaque(true);
    setBufferedToImage(true);
}

void FaceplateModuleComponent::updateColorsForStyle()
{
    switch (moduleStyle)
    {
        case ModuleStyle::DarkSteel:
            baseColour = juce::Colour(0xff16181a);
            secondaryColour = juce::Colour(0xff22252a);
            headerTextColour = juce::Colour(0xffdcdfe2);
            break;

        case ModuleStyle::SilverAluminum:
            baseColour = juce::Colour(0xffc2c6ca);
            secondaryColour = juce::Colour(0xffdee2e6);
            headerTextColour = juce::Colour(0xff181a1d); // high-contrast black/dark-slate
            break;

        case ModuleStyle::ConsoleBlueGrey:
            baseColour = juce::Colour(0xff263947);
            secondaryColour = juce::Colour(0xff344c5e);
            headerTextColour = juce::Colour(0xffeceff2);
            break;

        case ModuleStyle::Custom:
        default:
            break;
    }
}

void FaceplateModuleComponent::setModuleTitle(const juce::String& title)
{
    titleText = title;
    repaint();
}

void FaceplateModuleComponent::setModuleStyle(ModuleStyle style)
{
    moduleStyle = style;
    updateColorsForStyle();
    repaint();
}

void FaceplateModuleComponent::setBaseColour(juce::Colour colour)
{
    moduleStyle = ModuleStyle::Custom;
    baseColour = colour;
    secondaryColour = colour.darker(0.25f);
    repaint();
}

juce::Rectangle<int> FaceplateModuleComponent::getContentBounds() const
{
    auto bounds = getLocalBounds();
    const int topMargin = 50;   
    const int bottomMargin = showScrews ? 22 : 10;
    const int sideMargin = 8;

    return bounds.withTrimmedTop(topMargin)
                 .withTrimmedBottom(bottomMargin)
                 .withTrimmedLeft(sideMargin)
                 .withTrimmedRight(sideMargin);
}

void FaceplateModuleComponent::resized() {}

void FaceplateModuleComponent::paint(juce::Graphics& g)
{
    auto fBounds = getLocalBounds().toFloat();

    // 1. Brushed Metal Background Gradient
    drawBrushedMetalBackground(g, fBounds);

    // 2. Horizontal Module Separation Bevels (1px highlight on top, 1px shadow on bottom)
    g.setColour(juce::Colour(0x22ffffff)); // #FFFFFF22 highlight on top
    g.drawHorizontalLine(0, 0.0f, fBounds.getRight());
    g.drawHorizontalLine(1, 0.0f, fBounds.getRight());

    g.setColour(juce::Colour(0x66000000)); // #00000066 shadow on bottom
    g.drawHorizontalLine(static_cast<int>(fBounds.getBottom() - 1.0f), 0.0f, fBounds.getRight());
    g.drawHorizontalLine(static_cast<int>(fBounds.getBottom() - 2.0f), 0.0f, fBounds.getRight());

    // Vertical Module Slot Bevels (left highlight, right deep shadow)
    g.setColour(juce::Colour(0x1fffffff));
    g.drawVerticalLine(0, 0.0f, fBounds.getBottom());
    g.setColour(juce::Colour(0x55000000));
    g.drawVerticalLine(static_cast<int>(fBounds.getRight() - 1.0f), 0.0f, fBounds.getBottom());

    // 3. Engraved Header Plate
    auto headerBounds = fBounds.removeFromTop(44.0f);
    drawEngravedHeader(g, headerBounds);

    // 4. Dark Gunmetal Chassis Screws with Cross-Head Slots
    if (showScrews)
    {
        const float screwRadius = 5.5f;
        const float marginX = 10.0f;
        const float marginY = 10.0f;

        drawRackScrew(g, { marginX, marginY }, screwRadius, 28.0f);
        drawRackScrew(g, { fBounds.getWidth() - marginX, marginY }, screwRadius, 118.0f);
        drawRackScrew(g, { marginX, fBounds.getHeight() - marginY }, screwRadius, 205.0f);
        drawRackScrew(g, { fBounds.getWidth() - marginX, fBounds.getHeight() - marginY }, screwRadius, 65.0f);
    }
}

void FaceplateModuleComponent::drawBrushedMetalBackground(juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Directional vertical gradient
    juce::ColourGradient grad(baseColour, bounds.getTopLeft(),
                              secondaryColour, bounds.getBottomLeft(), false);

    // Add mid-tones for rich metallic depth
    if (moduleStyle == ModuleStyle::SilverAluminum)
    {
        grad.addColour(0.25, juce::Colour(0xffd5d9dd));
        grad.addColour(0.65, juce::Colour(0xffc8ccd0));
    }
    else if (moduleStyle == ModuleStyle::DarkSteel)
    {
        grad.addColour(0.4, juce::Colour(0xff1d2024));
        grad.addColour(0.8, juce::Colour(0xff181a1e));
    }
    else if (moduleStyle == ModuleStyle::ConsoleBlueGrey)
    {
        grad.addColour(0.35, juce::Colour(0xff2d4252));
        grad.addColour(0.75, juce::Colour(0xff22323f));
    }

    g.setGradientFill(grad);
    g.fillRect(bounds);

    // Micro brushed texture lines
    float fineAlpha = (moduleStyle == ModuleStyle::SilverAluminum) ? 0.035f : 0.018f;
    float darkAlpha = (moduleStyle == ModuleStyle::SilverAluminum) ? 0.045f : 0.022f;

    g.setColour(juce::Colours::white.withAlpha(fineAlpha));
    for (float y = bounds.getY(); y < bounds.getBottom(); y += 2.0f)
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());

    g.setColour(juce::Colours::black.withAlpha(darkAlpha));
    for (float y = bounds.getY() + 1.0f; y < bounds.getBottom(); y += 2.0f)
        g.drawHorizontalLine(static_cast<int>(y), bounds.getX(), bounds.getRight());
}

void FaceplateModuleComponent::drawEngravedHeader(juce::Graphics& g, juce::Rectangle<float> headerBounds)
{
    auto plateBounds = headerBounds.reduced(8.0f, 6.0f);

    if (moduleStyle == ModuleStyle::SilverAluminum)
    {
        juce::ColourGradient insetGrad(juce::Colour(0xffafb3b8), plateBounds.getX(), plateBounds.getY(),
                                       juce::Colour(0xffd4d8dc), plateBounds.getX(), plateBounds.getBottom(), false);
        g.setGradientFill(insetGrad);
        g.fillRoundedRectangle(plateBounds, 3.0f);

        g.setColour(juce::Colour(0x40000000));
        g.drawRoundedRectangle(plateBounds, 3.0f, 1.0f);

        // 1px bottom highlight
        g.setColour(juce::Colour(0x60ffffff));
        g.drawHorizontalLine(static_cast<int>(plateBounds.getBottom()), plateBounds.getX() + 2.0f, plateBounds.getRight() - 2.0f);
    }
    else
    {
        juce::ColourGradient plateGrad(juce::Colour(0xff101215), plateBounds.getX(), plateBounds.getY(),
                                       juce::Colour(0xff191c20), plateBounds.getX(), plateBounds.getBottom(), false);
        g.setGradientFill(plateGrad);
        g.fillRoundedRectangle(plateBounds, 3.0f);

        g.setColour(juce::Colour(0x80000000));
        g.drawRoundedRectangle(plateBounds, 3.0f, 1.2f);

        g.setColour(juce::Colour(0x20ffffff));
        g.drawRoundedRectangle(plateBounds.reduced(1.0f), 2.0f, 1.0f);
    }

    // High-contrast bold typography
    g.setFont(juce::Font("Helvetica Neue", 13.0f, juce::Font::bold));

    if (moduleStyle == ModuleStyle::SilverAluminum)
    {
        g.setColour(juce::Colours::white.withAlpha(0.6f));
        g.drawText(titleText, plateBounds.translated(0.0f, 1.0f), juce::Justification::centred, false);
    }
    else
    {
        g.setColour(juce::Colours::black.withAlpha(0.7f));
        g.drawText(titleText, plateBounds.translated(0.0f, 1.0f), juce::Justification::centred, false);
    }

    g.setColour(headerTextColour);
    g.drawText(titleText, plateBounds, juce::Justification::centred, false);
}

void FaceplateModuleComponent::drawRackScrew(juce::Graphics& g, juce::Point<float> center, float radius, float angleDegrees)
{
    auto screwBounds = juce::Rectangle<float>(radius * 2.0f, radius * 2.0f).withCentre(center);

    // 1. Recessed Chassis Counterbore Hole & Drop Shadow
    g.setColour(juce::Colours::black.withAlpha(0.70f));
    g.fillEllipse(screwBounds.translated(0.0f, 1.0f).expanded(1.0f));

    // 2. Realistic Dark Gunmetal Screw Head (#2A2E35 to #16181B)
    juce::ColourGradient gunmetalGrad(juce::Colour(0xff555c68), center.x - radius * 0.6f, center.y - radius * 0.6f,
                                      juce::Colour(0xff16181b), center.x + radius * 0.7f, center.y + radius * 0.7f, true);
    gunmetalGrad.addColour(0.35, juce::Colour(0xff333842));
    gunmetalGrad.addColour(0.75, juce::Colour(0xff22252a));
    g.setGradientFill(gunmetalGrad);
    g.fillEllipse(screwBounds);

    // Beveled rim
    g.setColour(juce::Colour(0x60ffffff));
    g.drawEllipse(screwBounds.reduced(0.5f), 0.8f);
    g.setColour(juce::Colour(0xff121315));
    g.drawEllipse(screwBounds, 1.0f);

    // 3. Phillips Cross-Head Slots (Two intersecting perpendicular recessed slots)
    float angleRad1 = juce::degreesToRadians(angleDegrees);
    float angleRad2 = angleRad1 + juce::MathConstants<float>::halfPi;
    float slotLen = radius * 0.65f;

    // Slot 1
    juce::Point<float> a1(center.x + slotLen * std::sin(angleRad1), center.y - slotLen * std::cos(angleRad1));
    juce::Point<float> a2(center.x - slotLen * std::sin(angleRad1), center.y + slotLen * std::cos(angleRad1));

    // Slot 2
    juce::Point<float> b1(center.x + slotLen * std::sin(angleRad2), center.y - slotLen * std::cos(angleRad2));
    juce::Point<float> b2(center.x - slotLen * std::sin(angleRad2), center.y + slotLen * std::cos(angleRad2));

    // Deep slot recess
    g.setColour(juce::Colour(0xff090a0b));
    g.drawLine(juce::Line<float>(a1, a2), 1.8f);
    g.drawLine(juce::Line<float>(b1, b2), 1.8f);

    // Micro highlight on slot edge
    g.setColour(juce::Colours::white.withAlpha(0.28f));
    juce::Point<float> ha1(a1.x + 0.5f * std::cos(angleRad1), a1.y + 0.5f * std::sin(angleRad1));
    juce::Point<float> ha2(a2.x + 0.5f * std::cos(angleRad1), a2.y + 0.5f * std::sin(angleRad1));
    g.drawLine(juce::Line<float>(ha1, ha2), 0.7f);
}
