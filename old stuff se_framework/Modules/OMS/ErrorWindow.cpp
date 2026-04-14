#include "ErrorWindow.h"

void ErrorWindow::showErrorMessage(int buttonToUseForReturnKey, int buttonToUseForEscapeKey)
{
	jassert(!buttons.empty()); // no buttons were added
	jassert(buttonToUseForReturnKey >= 0 && buttonToUseForReturnKey < (int)buttons.size()); // button out of range
	jassert(buttonToUseForEscapeKey >= 0 && buttonToUseForEscapeKey < (int)buttons.size()); // button out of range

	setVisible(true);

	selectedButtonID = buttonToUseForReturnKey;
	setButtonSelected(buttons[selectedButtonID].get());
	buttonIDForEscapeKey = buttonToUseForEscapeKey;

	setBounds(getParentComponent()->getLocalBounds());
	grabKeyboardFocus();
}

void ErrorWindow::showErrorMessage(String title_, String message_, int buttonToUseForReturnKey, int buttonToUseForEscapeKey)
{
	title = title_;
	message = message_;
	showErrorMessage(buttonToUseForReturnKey, buttonToUseForEscapeKey);
}

void ErrorWindow::addButton(String text, std::function<void()> callback)
{
	auto button = std::make_unique<ErrorMessageButton>();

	addAndMakeVisible(button.get());
	button.get()->setButtonText(text);
	button.get()->onClick = callback;
	button.get()->setLookAndFeel(&buttonSkin);
	button.get()->addListener(this);

	buttons.push_back(std::move(button));
}

void ErrorWindow::clearButtons()
{
	removeAllChildren();

	for (auto& b : buttons)
		b->setLookAndFeel(nullptr);

	buttons.clear();
}

void ErrorWindow::resized()
{
	errorWindowBounds = ElanRect::place(errorWindowBounds, getLocalBounds(), MidCenter, MidCenter);

	circle_area = ElanRect::place(circle_area, errorWindowBounds, TopLeft, TopLeft).translated(+12, 25);

	button_area = errorWindowBounds.reduced(1).removeFromBottom(26);
	auto button_area_clickable = button_area.reduced(1);

	// set button size according to text width
	for (const auto & b : buttons)
	{
		auto width = buttonSkin.font.getStringWidth(b.get()->getButtonText());
		width = std::max(45, width + 10);

		b.get()->setSize(width, button_area_clickable.getHeight());
	}

	// place buttons
	for (auto i = buttons.size(); i-- > 0; )
	{
		auto & b = *buttons[i].get();
		jassert(button_area_clickable.getWidth() >= b.getWidth() + 1); // not enough room for button, make error window wider

		b.setBounds(button_area_clickable.removeFromRight(b.getWidth()));
		button_area_clickable.removeFromRight(1);
	}
}

void ErrorWindow::paint(Graphics & g)
{
	{ // blackout
		g.fillAll(Colours::black.withAlpha(0.6f));
	}

	{ // background
		auto gradient = juce::ColourGradient::vertical(
			{ 150, 157, 144 }, (float)errorWindowBounds.getY(),
			{ 190, 200, 182 }, (float)errorWindowBounds.getBottom()
		);
		gradient.addColour(0.18, { 190, 200, 182 });
		g.setGradientFill(gradient);
		g.fillRect(errorWindowBounds);
	}

	{// cirlcle
		g.setColour({ 233, 57, 64 });
		g.fillEllipse(circle_area.toFloat());
	}

	{ // X
		auto circleCenter = ElanRect::getPoint(circle_area.toFloat(), MidCenter);
		auto x_area = ElanRect::place({ 0.f,0.f,14.f,14.f }, circleCenter, MidCenter);
		Path p = ElanPath::plus(x_area, 1.5f);

		AffineTransform transform;
		transform = transform.rotated(degreesToRadians(45.f), circleCenter.getX(), circleCenter.getY());
		p.applyTransform(transform);

		g.setColour({ 255, 255, 255 });
		g.fillPath(p);
	}

	{ // main highlight
		//auto gradient = juce::ColourGradient::vertical(
		//	Colour{ 219, 219, 219 }.withAlpha(0.0f), errorWindowBounds.getY(),
		//	Colour{ 255, 255, 255 }.withAlpha(0.5f), errorWindowBounds.getBottom()
		//);
		//gradient.addColour(0.45, Colour{ 219, 219, 219 }.withAlpha(0.0f));

		auto gradient = juce::ColourGradient::vertical(
			Colour{ 219, 219, 219 }.withAlpha(0.5f), (float)errorWindowBounds.getY(),
			Colour{ 255, 255, 255 }.withAlpha(0.0f), (float)errorWindowBounds.getBottom()
		);
		gradient.addColour(0.55, Colour{ 219, 219, 219 }.withAlpha(0.0f));
		g.setGradientFill(gradient);
		g.fillRect(errorWindowBounds);
	}

	{ // top shine
		g.setColour({ 210, 213, 208 });
		g.fillRect(errorWindowBounds.withHeight(1));
	}

	{ // button area background			
		auto gradient = juce::ColourGradient::vertical(
			{ 23, 23, 29 }, (float)button_area.getY(),
			{ 46, 46, 57 }, (float)button_area.getBottom()
		);
		g.setGradientFill(gradient);
		g.fillRect(button_area);
	}

	{ // button area highlight
		Path p;
		p.startNewSubPath(ElanRect::getPoint(button_area, TopLeft).toFloat());
		p.lineTo(ElanRect::getPoint(button_area, TopLeft).translated(132, 0).toFloat());
		p.lineTo(ElanRect::getPoint(button_area, BottomLeft).translated(108, 0).toFloat());
		p.lineTo(ElanRect::getPoint(button_area, BottomLeft).toFloat());
		p.closeSubPath();

		g.setColour(Colours::white.withAlpha(0.04f));
		g.fillPath(p);
	}

	{ // extra main highlight
		auto area = ElanRect::pad(errorWindowBounds, 0, 1, 0, 0);

		Path p;
		p.startNewSubPath(area.getTopLeft().toFloat());
		p.lineTo(ElanRect::getPoint(area, TopRight).toFloat());
		p.lineTo(float(area.getX() + 153), (float)button_area.getY());
		p.lineTo((float)area.getX(), (float)button_area.getY());
		p.closeSubPath();

		g.setColour(Colours::white.withAlpha(0.06f));
		g.fillPath(p);
	}

	{ // extra shines
		g.setColour({ 217, 217, 217 });
		g.fillRect(button_area.withHeight(1).translated(0, -2));

		g.setColour({ 161, 202, 171 });
		g.fillRect(button_area.withHeight(1).translated(0, -3));
	}

	{ // title
		g.setColour({ 0, 0, 0 });
		g.setFont(RESOURCES->fonts.RobotoMedium.withPointHeight(10));
		g.drawText(title, ElanRect::pad(errorWindowBounds, 49, 10, 0, 0), Justification::topLeft);
	}

	{ // message
		g.setColour({ 73, 82, 67 });
		g.setFont(RESOURCES->fonts.RobotoMedium.withPointHeight(10));
		auto textArea = ElanRect::pad(errorWindowBounds, 49, 30, 15, 0);
		g.drawMultiLineText(message, textArea.getX(), textArea.getY() + 10, textArea.getWidth());
	}
}

void ErrorWindow::ButtonSkin::drawButtonBackground(Graphics& g, Button& b, const Colour&, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
	if (shouldDrawButtonAsDown)
	{
		// background
		auto gradient = juce::ColourGradient::vertical(
			{ 246, 248, 245 }, 0.f,
			{ 233, 237, 231 }, (float)b.getHeight()
		);
		gradient.addColour(0.82, { 233, 237, 231 });
		g.setGradientFill(gradient);
		g.fillAll();

		// top detail
		g.setColour({ 30, 30, 36 });
		g.fillRect(b.getLocalBounds().removeFromTop(1));

		// bottom detail
		g.setColour({ 31, 31, 38 });
		g.fillRect(b.getLocalBounds().removeFromBottom(1));
	}
	else if (shouldDrawButtonAsHighlighted)
	{
		// background
		auto gradient = juce::ColourGradient::vertical(
			{ 255, 255, 255 }, 0.f,
			{ 233, 237, 231 }, (float)b.getHeight()
		);
		gradient.addColour(0.41, { 244, 246, 243 });
		gradient.addColour(0.82, { 233, 237, 231 });
		g.setGradientFill(gradient);
		g.fillAll();

		// extra highlight
		g.setColour(Colours::white.withAlpha(0.7f));
		g.fillAll();

		// bottom detail
		g.setColour({ 178, 177, 184 });
		g.fillRect(b.getLocalBounds().removeFromBottom(1));
	}
	else
	{
		// background
		auto gradient = juce::ColourGradient::vertical(
			{ 246, 248, 245 }, 0.f,
			{ 233, 237, 231 }, (float)b.getHeight()
		);
		gradient.addColour(0.82, { 233, 237, 231 });
		g.setGradientFill(gradient);
		g.fillAll();

		// bottom detail
		g.setColour({ 55,55,79 });
		g.fillRect(b.getLocalBounds().removeFromBottom(1));
	}

	if (dynamic_cast<ErrorMessageButton*>(&b)->getIsSelected())
	{
		auto area = ElanRect::pad(b.getLocalBounds(), 2, 2, 2, 3);

		g.setColour({ 135, 135, 149 });

		g.fillPath(ElanPath::corner(ElanRect::setSize(area, 3, TopLeft).toFloat(), 1, TopLeft));
		g.fillPath(ElanPath::corner(ElanRect::setSize(area, 3, TopRight).toFloat(), 1, TopRight));
		g.fillPath(ElanPath::corner(ElanRect::setSize(area, 3, BottomRight).toFloat(), 1, BottomRight));
		g.fillPath(ElanPath::corner(ElanRect::setSize(area, 3, BottomLeft).toFloat(), 1, BottomLeft));
	}
}

void ErrorWindow::ButtonSkin::drawButtonText(Graphics& g, TextButton& b, bool, bool)
{
	g.setFont(font);
	g.setColour({ 55,61,51 });
	g.drawText(b.getButtonText(), b.getLocalBounds(), Justification::centred);
}
