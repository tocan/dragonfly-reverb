/*
 * Dragonfly Reverb
 * copyright (c) 2019-2020 Michael Willis, Rob van den Berg, Shane Dunne
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of
 * the License, or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * For a full copy of the GNU General Public License see the LICENSE file.
 */

#include "DragonflyPlateProcessor.h"
#include "DragonflyPlateEditor.h"

#define HEADER_IMAGE_HEIGHT 118
#define HEADER_IMAGE_X 10
#define HEADER_IMAGE_Y 0
#define INFO_IMAGE_HEIGHT 140

#define KNOB_WIDTH 70
#define INSET 12
#define GUTTER 8
#define GAP 4
#define CBHEIGHT 20
#define TOPOFFSET 12
#define NUDGE 3

#define BACKGROUND_COLOUR Colour(61, 61, 61)
#define ICON_COLOUR       Colour(183, 204, 204)
#define KNOB_COLOUR       Colour(179, 149, 50)
#define RING_COLOUR       Colour(220, 228, 238)

DragonflyPlateEditor::DragonflyPlateEditor (DragonflyPlateProcessor& p)
    : AudioProcessorEditor (&p)
    , processor (p)
    , infoButton(BinaryData::helpcircle_svg, BACKGROUND_COLOUR, ICON_COLOUR, ICON_COLOUR.brighter(1.0f), RING_COLOUR)
    , dryLevelKnob(DragonflyPlateParameters::dryLevelMin, DragonflyPlateParameters::dryLevelMax, DragonflyPlateParameters::dryLevelLabel)
    , labeledDryLevelKnob(DragonflyPlateParameters::dryLevelName, dryLevelKnob)
    , wetLevelKnob(DragonflyPlateParameters::wetLevelMin, DragonflyPlateParameters::wetLevelMax, DragonflyPlateParameters::wetLevelLabel)
    , labeledWetLevelKnob(DragonflyPlateParameters::wetLevelName, wetLevelKnob)
    , widthKnob(DragonflyPlateParameters::widthMin, DragonflyPlateParameters::widthMax, DragonflyPlateParameters::widthLabel)
    , labeledWidthKnob(DragonflyPlateParameters::widthName, widthKnob)
    , predelayKnob(DragonflyPlateParameters::predelayMin, DragonflyPlateParameters::predelayMax, DragonflyPlateParameters::predelayLabel)
    , labeledPredelayKnob(DragonflyPlateParameters::predelayName, predelayKnob)
    , decayKnob(DragonflyPlateParameters::decayMin, DragonflyPlateParameters::decayMax, DragonflyPlateParameters::decayLabel)
    , labeledDecayKnob(DragonflyPlateParameters::decayName, decayKnob)
    , lowCutKnob(DragonflyPlateParameters::lowCutMin, DragonflyPlateParameters::lowCutMax, DragonflyPlateParameters::lowCutLabel)
    , labeledLowCutKnob(DragonflyPlateParameters::lowCutName, lowCutKnob)
    , highCutKnob(DragonflyPlateParameters::highCutMin, DragonflyPlateParameters::highCutMax, DragonflyPlateParameters::highCutLabel)
    , labeledHighCutKnob(DragonflyPlateParameters::highCutName, highCutKnob)
    , dampenKnob(DragonflyPlateParameters::dampenMin, DragonflyPlateParameters::dampenMax, DragonflyPlateParameters::dampenLabel)
    , labeledDampenKnob(DragonflyPlateParameters::dampenName, dampenKnob)
{
    setLookAndFeel(lookAndFeel);
    lookAndFeel->setColour(ResizableWindow::backgroundColourId, BACKGROUND_COLOUR);
    lookAndFeel->setColour(GroupComponent::outlineColourId, ICON_COLOUR);
    lookAndFeel->setColour(Slider::rotarySliderFillColourId, KNOB_COLOUR);
    lookAndFeel->setColour(Slider::rotarySliderOutlineColourId, RING_COLOUR);

    headerImage = ImageCache::getFromMemory(BinaryData::plateheader_png, BinaryData::plateheader_pngSize);

    mainGroup.setText("Reverb Type");
    addAndMakeVisible(&mainGroup);

    infoButton.onClick = [this]() { infoImage.setVisible(true); };
    addAndMakeVisible(infoButton);

    dryLevelKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::dryLevelDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledDryLevelKnob);
    wetLevelKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::wetLevelDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledWetLevelKnob);
    algIndexCombo.setEditableText(false);
    algIndexCombo.setJustificationType(Justification::centredLeft);
    DragonflyPlateParameters::populateAlgorithmsComboBox(algIndexCombo);
    addAndMakeVisible(algIndexCombo);
    widthKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::widthDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledWidthKnob);
    predelayKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::predelayDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledPredelayKnob);
    decayKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::decayDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledDecayKnob);
    lowCutKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::lowCutDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledLowCutKnob);
    highCutKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::highCutDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledHighCutKnob);
    dampenKnob.setDoubleClickReturnValue(true, double(DragonflyPlateParameters::dampenDefault), ModifierKeys::noModifiers);
    addAndMakeVisible(labeledDampenKnob);

    // Add infoImage last so when it's displayed, it will cover the other controls
    infoImage.setImage(ImageCache::getFromMemory(BinaryData::plateinfo_png, BinaryData::plateinfo_pngSize));
    infoImage.onMouseDown = [this]() { infoImage.setVisible(false); };
    addChildComponent(infoImage);

    processor.parameters.attachControls(
        dryLevelKnob,
        wetLevelKnob,
        algIndexCombo,
        widthKnob,
        predelayKnob,
        decayKnob,
        lowCutKnob,
        highCutKnob,
        dampenKnob );

    setSize (2 * INSET + 2 * GUTTER + 8 * KNOB_WIDTH + 7 * GAP, HEADER_IMAGE_HEIGHT + INFO_IMAGE_HEIGHT);
}

DragonflyPlateEditor::~DragonflyPlateEditor()
{
    processor.parameters.detachControls();
    setLookAndFeel(nullptr);
}

void DragonflyPlateEditor::resized()
{
    auto bounds = getLocalBounds();
    bounds.removeFromTop(HEADER_IMAGE_HEIGHT);
    infoImage.setBounds(bounds);

    auto groupArea = bounds.reduced(INSET);
    mainGroup.setBounds(groupArea);
    auto cbArea = groupArea;
    cbArea.removeFromLeft(108);
    cbArea = cbArea.removeFromTop(CBHEIGHT);
    cbArea.removeFromRight(16);
    infoButton.setBounds(cbArea.removeFromRight(cbArea.getHeight()).withSizeKeepingCentre(24, 24));
    cbArea.setY(cbArea.getY() - NUDGE);
    algIndexCombo.setBounds(cbArea.removeFromLeft(90));

    groupArea.reduce(2 * GUTTER, GUTTER);
    groupArea.removeFromTop(TOPOFFSET);
    int width = (groupArea.getWidth() - 7 * GAP) / 8;
    labeledDryLevelKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledWetLevelKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledWidthKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledPredelayKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledDecayKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledLowCutKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledHighCutKnob.setBounds(groupArea.removeFromLeft(width));
    groupArea.removeFromLeft(GAP);
    labeledDampenKnob.setBounds(groupArea);
}

void DragonflyPlateEditor::paint (Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(lookAndFeel->findColour(ResizableWindow::backgroundColourId));

    g.drawImageAt(headerImage, HEADER_IMAGE_X, HEADER_IMAGE_Y);
}
