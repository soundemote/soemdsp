#include "PresetInterface.h"

PresetInterface::PresetInterface()
{
	addAndMakeVisible(listBox);
	listBox.setColour(ListBox::backgroundColourId, Colours::transparentWhite);
	listBox.setModel(&listModel);
	listBox.setRowHeight(18);
	for (int i = 0; i < 40; ++i)
		listModel.items.push_back("need to maek scrollbar appear");
	listBox.updateContent();
	
	auto rootTreeItem = new TreeItem(4);
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(0))->setText("Factory");
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(0))->isHeader = true;
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(0))->setOpenness(juce::TreeViewItem::Openness::opennessOpen);

	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(1))->setText("CoolPatchesOnly");
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(1))->isHeader = true;
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(1))->setOpenness(juce::TreeViewItem::Openness::opennessOpen);

	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(2))->setText("My New Library");
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(2))->isHeader = true;
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(2))->setOpenness(juce::TreeViewItem::Openness::opennessOpen);

	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(3))->setText("MEGA MEGA LIBRAREEEE");
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(3))->isHeader = true;
	dynamic_cast<TreeItem*>(rootTreeItem->getSubItem(3))->setOpenness(juce::TreeViewItem::Openness::opennessOpen);
	for (int i = 0; i < 3; ++i)
		rootTreeItem->getSubItem(0)->addSubItem(new TreeItem("CAT1"));
	for (int i = 0; i < 5; ++i)
		rootTreeItem->getSubItem(1)->addSubItem(new TreeItem("category"));
	for (int i = 0; i < 2; ++i)
		rootTreeItem->getSubItem(2)->addSubItem(new TreeItem("a category"));
	for (int i = 0; i < 20; ++i)
		rootTreeItem->getSubItem(3)->addSubItem(new TreeItem("CATEGORY"));

	addAndMakeVisible(tree);
	tree.setIndentSize(11);
	tree.setRootItem(rootTreeItem);
	tree.setRootItemVisible(false);

	addAndMakeVisible(presetPage);

	addAndMakeVisible(close_button);
	close_button.clickedFunc = [this]()
	{
		setVisible(false);
	};

	setLookAndFeel(&skin);
	presetPage.description_textEditor.setLookAndFeel(&skin2);
	presetPage.author_textEditor.setLookAndFeel(&skin3);

	addAndMakeVisible(factoryPresetButton);
	addAndMakeVisible(userPresetButton);
}

void PresetInterface::resized()
{
	// get area inside border
	auto area = getLocalBounds().reduced(1);

	pageTitle_area = area.removeFromTop(25);

	// seperator
	area.removeFromTop(25);

	// inset
	area.reduce(2, 2);

	// divide remaining space by 3
	auto remainingAreaWidthDiv3 = area.getWidth() / 3;

	presetFolders_area = area.removeFromLeft(remainingAreaWidthDiv3).reduced(2);
	presetList_area = area.removeFromLeft(remainingAreaWidthDiv3).reduced(2);
	presetEditor_area = area.reduced(2);

	auto temp_presetFolders_area = presetFolders_area;
	temp_presetFolders_area.removeFromTop(4);

	auto presetVisibilityButton_area = temp_presetFolders_area.removeFromTop(RESOURCES->images.presetWindow_f.getImage(0).getBounds().getHeight());

	{ // factoryPresetButton bounds
		auto tempArea = presetVisibilityButton_area.removeFromLeft(presetVisibilityButton_area.getWidth() / 2);
		factoryPresetButton.setBounds(ElanRect::setWidth(tempArea, RESOURCES->images.presetWindow_f.getImage(0).getBounds().getWidth(), TopLeft));
		factoryPresetButton.setBounds(ElanRect::place(factoryPresetButton.getBounds(), tempArea, MidRight, MidRight).translated(-3, -10));
	}

	{	// userPresetButton bounds
		auto tempArea = presetVisibilityButton_area;
		userPresetButton.setBounds(ElanRect::setWidth(tempArea, RESOURCES->images.presetWindow_f.getImage(0).getBounds().getWidth(), TopLeft));
		userPresetButton.setBounds(ElanRect::place(factoryPresetButton.getBounds(), tempArea, MidLeft, MidLeft).translated(+3, -10));
	}

	tree.setBounds(ElanRect::pad(presetFolders_area, 6, 28, 6, 6));
	listBox.setBounds(presetList_area.reduced(6));
	presetPage.setBounds(presetEditor_area.reduced(6));

	close_button.setBounds(ElanRect::place(close_button.getBounds(), presetEditor_area, MidCenter, TopRight).translated(-20, 0));
}

void PresetInterface::paint(Graphics & g)
{
	// background
	g.setColour(Colours::black);
	g.fillRect(getLocalBounds());

	// fills
	g.setColour({ 27, 25, 27 });
	g.fillRect(pageTitle_area);
	g.setColour({ 19, 12, 13 });
	g.fillRect(presetFolders_area);
	g.fillRect(presetList_area);
	g.fillRect(presetEditor_area);

	// borders
	g.setColour({ 28,41,41 });
	g.drawRect(presetFolders_area);
	g.drawRect(presetList_area);
	g.drawRect(presetEditor_area);
}

void PresetInterface::paintOverChildren(Graphics & g)
{
	// draw border
	g.setColour({ 81, 81, 81 });
	g.drawRect(getLocalBounds());

	// get area inside border
	auto area = getLocalBounds().reduced(1);

	// draw page title
	g.setColour(Colours::white);
	g.setFont(RESOURCES->fonts.RobotoBold.withPointHeight(14.f));
	g.drawText("PRESET BROWSER", ElanRect::pad(pageTitle_area, 8, 1, 0, 0), Justification::centredLeft);
}

void PresetInterface::TreeItem::paintOpenCloseButton(Graphics & g, const Rectangle<float>& area, Colour /*backgroundColour*/, bool /*isMouseOver*/)
{
	juce::Rectangle<float> bounds{ 7.2f, 3.6f };
	bounds = ElanRect::place(bounds, area, MidCenter, MidCenter);

	Path p = TrianglePath::pointerWithFlatBase(bounds, 2);
	float rotation = isOpen() ? 180.f : 90.f;
	p.applyTransform(juce::AffineTransform().rotated(juce::degreesToRadians(rotation), bounds.getCentreX(), bounds.getCentreY()));

	g.setColour({ 191,191,191 });
	g.fillPath(p);
}

void PresetInterface::TreeItem::paintItem(Graphics & g, int width, int height)
{
	if (isSelected())
	{
		juce::Rectangle<int> bounds{ 0,0, width-8, height };
		g.setColour({ 69, 67, 67 });
		g.fillRect(bounds);
	}

	if (isHeader)
	{
		// prepare for drawing text
		g.setColour(Colours::white);
		g.setFont(RESOURCES->fonts.RobotoBlack.withPointHeight(13.f));
	}
	else
	{
		// draw bottom seperator line
		g.setColour({ 36, 32, 32 });
		g.fillRect(juce::Rectangle<int>{0, 0, width-8, height}.removeFromBottom(1));

		// prepare for drawing text
		g.setColour({ 190, 200, 182 });
		g.setFont(RESOURCES->fonts.RobotoMedium.withPointHeight(11.f));
	}

	g.drawText(text.toUpperCase(), 4, 1, width, height-1, Justification::left);
}

void PresetInterface::ListModel::paintListBoxItem(int rowNumber, Graphics & g, int width, int height, bool rowIsSelected)
{
	if (rowIsSelected)
		g.setColour(Colours::white);
	else
		g.setColour(Colours::grey);

	g.setFont(RESOURCES->fonts.RobotoMedium.withPointHeight(13.f));
	g.drawText(rowNumber < int(items.size()) ? items[rowNumber] : "error", ElanRect::pad({ 0,0,width, height }, 2, 2, 2, 2), Justification::centredLeft);
}

void PresetInterface::Skin::fillTextEditorBackground(Graphics & g, int width, int height, TextEditor & textEditor)
{
	juce::Rectangle<int> area{ 0,0, width, height };
	g.setColour(Colours::black.withAlpha(.5f));

	if (textEditor.hasKeyboardFocus(true))
	{
		g.fillRect(area);
	}
	else
	{
		g.fillRect(area.removeFromTop(1));
		g.fillRect(area.removeFromBottom(1));
	}
}

void PresetInterface::Skin::drawTextEditorOutline(Graphics & g, int width, int height, TextEditor & textEditor)
{
	if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly())
	{
		g.setColour(Colours::white.withAlpha(.3f));
		g.drawRect(0, 0, width, height, 1);
	}
}

void PresetInterface::Skin2::fillTextEditorBackground(Graphics & g, int width, int height, TextEditor & textEditor)
{
	juce::Rectangle<int> area{ 0,0, width, height };
	g.setColour(Colours::black.withAlpha(.5f));

	if (textEditor.hasKeyboardFocus(true))
	{
		g.fillRect(area);
	}
}

void PresetInterface::Skin2::drawTextEditorOutline(Graphics & g, int width, int height, TextEditor & textEditor)
{
	if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly())
	{
		g.setColour(Colours::white.withAlpha(.3f));
		g.drawRect(0, 0, width, height, 1);
	}
}

void PresetInterface::Skin3::fillTextEditorBackground(Graphics & g, int width, int height, TextEditor & textEditor)
{
	juce::Rectangle<int> area{ 0,0, width, height };
	g.setColour(Colours::black.withAlpha(.5f));

	if (textEditor.hasKeyboardFocus(true))
		g.fillRect(area);
	else
		g.fillRect(area.removeFromTop(1));
}

void PresetInterface::Skin3::drawTextEditorOutline(Graphics & g, int width, int height, TextEditor & textEditor)
{
	if (textEditor.hasKeyboardFocus(true) && !textEditor.isReadOnly())
	{
		g.setColour(Colours::white.withAlpha(.3f));
		g.drawRect(0, 0, width, height, 1);
	}
}

void PresetInterface::Skin::drawScrollbar(Graphics& g, ScrollBar& /*scrollbar*/, int x, int y, int width, int height, bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown)
{
	ignoreUnused(isMouseDown);

	Rectangle<int> localBounds{ x,y, width, height };
	auto thumbBounds = [&]()->Rectangle<int>
	{
		if (isScrollbarVertical)
			return { x, thumbStartPosition, width, thumbSize };

		return { thumbStartPosition, y, thumbSize, height };
	}();

	g.setColour(Colour(51.f / 360.f, 6.f / 100.f, 100.f / 100.f, .1f));
	g.fillRect(localBounds.reduced(2));

	auto c = Colour{ 132, 125, 125 };
	g.setColour(isMouseOver || isMouseDown ? c.brighter(0.25f) : c);
	g.fillRect(thumbBounds.reduced(2));
}

PresetInfoEditor::PresetInfoEditor()
{
	addAndMakeVisible(name_textEditor);
	name_textEditor.setIndents(4, 2);
	name_textEditor.setColour(TextEditor::textColourId, { 191, 191, 191 });
	name_textEditor.setFont(RESOURCES->fonts.RobotoBold.withPointHeight(11.f));

	addAndMakeVisible(category_textEditor);
	category_textEditor.setIndents(4, 2);
	category_textEditor.setColour(TextEditor::textColourId, { 191, 191, 191 });
	category_textEditor.setFont(RESOURCES->fonts.RobotoBold.withPointHeight(11.f));

	addAndMakeVisible(author_textEditor);
	author_textEditor.setColour(TextEditor::textColourId, { 191, 191, 191 });
	author_textEditor.setFont(RESOURCES->fonts.RobotoLight.withPointHeight(12.f));

	addAndMakeVisible(description_textEditor);
	description_textEditor.setColour(TextEditor::textColourId, { 191, 191, 191 });
	description_textEditor.setFont(RESOURCES->fonts.RobotoRegular.withPointHeight(12.f));
	description_textEditor.setMultiLine(true);
	description_textEditor.setReturnKeyStartsNewLine(true);

	addAndMakeVisible(name_label);
	addAndMakeVisible(category_label);
	addAndMakeVisible(author_label);
	addAndMakeVisible(description_label);

	addAndMakeVisible(cancel_button);
	addAndMakeVisible(save_button);
	addAndMakeVisible(rename_button);
	addAndMakeVisible(delete_button);
}

void PresetInfoEditor::resized()
{
	auto area = ElanRect::pad(getLocalBounds(), 25+3+5+2+2, 8, 6, 0);

	name_label.setBounds(area.removeFromTop(19).translated(+5, 1));
	name_textEditor.setBounds(area.removeFromTop(22));

	category_label.setBounds(area.removeFromTop(19).translated(+5, 1));
	category_textEditor.setBounds(area.removeFromTop(22));
	category_textEditor.setJustification(Justification::verticallyCentred);

	author_label.setBounds(area.removeFromTop(19).translated(+5, 1));
	author_textEditor.setBounds(area.removeFromTop(22));
	author_textEditor.setJustification(Justification::verticallyCentred);

	area.removeFromTop(30); // horizontal space

	// DELETE, RENAME, CANCEL, SAVE
	area = ElanRect::pad({ 0, area.getY(), getWidth(), area.getHeight() }, 6, 0, 6, 0);
	button_area = ElanRect::setWidth(area.removeFromBottom(33), getWidth(), MidCenter);
	auto temp_button_area = ElanRect::setWidth(button_area, area.getWidth(), MidCenter);
	delete_button.setSizeToImageAndCenterWithinBounds(temp_button_area.removeFromLeft(55));
	temp_button_area.removeFromLeft(5); // vertical space
	rename_button.setSizeToImageAndCenterWithinBounds(temp_button_area.removeFromLeft(67));
	save_button.setSizeToImageAndCenterWithinBounds(temp_button_area.removeFromRight(47));
	temp_button_area.removeFromRight(5); // vertical space
	cancel_button.setSizeToImageAndCenterWithinBounds(temp_button_area.removeFromRight(47));

	// DESCRIPTION
	description_textEditor.setBounds(ElanRect::pad(area, 5, Edge::Bottom));
	description_label.setBounds(ElanRect::place(area.withHeight(19), area, BottomLeft, TopLeft).translated(+5, 0));
}

void PresetInfoEditor::paint(Graphics & g)
{
	// background
	g.setColour({ 51, 32, 34 });
	
	int y = author_textEditor.getBottom() + (description_label.getY() - author_textEditor.getBottom()) / 2;

	juce::Rectangle<float> area{ 0, 0, (float)getWidth(), (float)y };

	juce::Rectangle<float> betweenArea{
		ElanRect::getPoint(author_textEditor.getBounds().toFloat(), BottomLeft),
		ElanRect::getPoint(description_label.getBounds().toFloat(), TopRight)
	};

	juce::Rectangle<float> betweenArea2 = { 0, 0, (float)getWidth(), 8 };
	betweenArea2.setBounds(0, ElanRect::place(betweenArea2, betweenArea, MidLeft, MidLeft).getY()+2, (float)getWidth(), 5);

	juce::Rectangle<float> betweenArea3{ ElanRect::getPoint(betweenArea2, BottomLeft).toFloat(), ElanRect::getPoint(description_textEditor.getBounds().toFloat(), TopRight) };
	betweenArea3.setRight(betweenArea2.getRight());

	juce::Rectangle<float> betweenArea4{ betweenArea3.translated(0, -3) };

	{
		Path p;
		p.startNewSubPath(ElanRect::getPoint(area, TopLeft));
		p.lineTo(ElanRect::getPoint(area, TopRight));
		p.lineTo(ElanRect::getPoint(betweenArea4, MidRight));
		p.lineTo(ElanRect::getPoint(ElanRect::setWidth(betweenArea4, 23.f, MidCenter), MidRight));
		p.lineTo(ElanRect::getPoint(ElanRect::setWidth(betweenArea4, 23.f, MidCenter), TopLeft));
		p.lineTo(ElanRect::getPoint(betweenArea4, TopLeft));
		p.closeSubPath();
		g.setColour({ 51, 32, 34 });
		g.fillPath(p);
	}

	area = { 0, (float)description_label.getY()-2, (float)getWidth(), (float)getHeight() };

	{
		Path p;
		p.startNewSubPath(ElanRect::getPoint(betweenArea2, BottomLeft));
		p.lineTo(ElanRect::getPoint(ElanRect::setWidth(betweenArea3, 23.f, MidCenter), TopLeft));
		p.lineTo(ElanRect::getPoint(ElanRect::setWidth(betweenArea3, 23.f, MidCenter), MidRight));
		p.lineTo(ElanRect::getPoint(betweenArea3, MidRight));
		p.lineTo(ElanRect::getPoint(area.toFloat(), BottomRight));
		p.lineTo(ElanRect::getPoint(area.toFloat(), BottomLeft));
		p.closeSubPath();
		g.setColour({ 51, 32, 34 });
		g.fillPath(p);
	}

	auto imageBounds = ElanRect::scale(RESOURCES->images.presetWindow_box_f.getImage().getBounds().toFloat(), 0.5f, TopLeft);
	imageBounds.setY(float(name_label.getY() + 2));
	g.setColour(Colours::black);
	g.drawRect(ElanRect::pad(imageBounds, 0.f, -2.f, -2.f, -2.f), 2.f);
	g.drawImage(RESOURCES->images.presetWindow_box_f.getImage(), imageBounds);


	//g.setColour(Colours::green);
	//g.drawRect(betweenArea2);

	//g.setColour(Colours::blue);
	//g.drawRect(betweenArea3);
}
