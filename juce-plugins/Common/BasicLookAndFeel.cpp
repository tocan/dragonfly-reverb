#include "BasicLookAndFeel.h"

// Change the look of JUCE's "rotary sliders" so they're more like traditional knobs. This code is adapted
// from the example at https://www.juce.com/doc/tutorial_look_and_feel_customisation.

void BasicLookAndFeel::drawRotarySlider(Graphics& g, int x, int y, int width, int height, float sliderPos,
    const float rotaryStartAngle, const float rotaryEndAngle,
    Slider& slider)
{
    // basic geometry
    const float radius = jmin(width / 2, height / 2) - 10.0f;
    const float centreX = x + width * 0.5f;
    const float centreY = y + height * 0.5f;
    const float rx = centreX - radius;
    const float ry = centreY - radius;
    const float rw = radius * 2.0f;
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // fill
    g.setColour(findColour(Slider::rotarySliderFillColourId));
    g.fillEllipse(rx, ry, rw, rw);

    // outline
    g.setColour(findColour(Slider::rotarySliderOutlineColourId));
    g.drawEllipse(rx, ry, rw, rw, 3.0f);

    // pointer
    Path p;
    const float pointerLength = radius * 0.5f;
    const float pointerThickness = 3.0f;
    p.addRectangle(-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
    p.applyTransform(AffineTransform::rotation(angle).translated(centreX, centreY));
    g.setColour(findColour(Slider::rotarySliderOutlineColourId));
    g.fillPath(p);
}