#include "MouseKeyboard.h"

#include "MouseKeyboardImages.cpp"

void KeyboardKeyBase::pressKey()
{
	isPressed = true;
	repaint();
}

void KeyboardKeyBase::liftKey()
{
	isPressed = false;
	repaint();
}

void KeyboardKeyBase::sustainKey()
{
	isSustained = true;
	pressKey();
	repaint();
}

void KeyboardKeyBase::unsustainKey()
{
	isSustained = false;
	liftKey();
	repaint();
}

void KeyboardKeyBase::setIsMouseOver(bool v)
{
	isMouseOver = v;
	repaint();
}

bool KeyboardKeyBase::getIsSustained()
{
	return isSustained;
}

bool KeyboardKeyBase::getIsMouseOver()
{
	return isMouseOver;
}

bool KeyboardKeyBase::getIsPressed()
{
	return isPressed;
}

void KeyboardKeyBase::setVelocity(double v)
{
	velocity = v;
}

void KeyboardKeyBase::setPitch(double v)
{
	pitch = v;
}

double KeyboardKeyBase::getVelocity()
{
	return velocity;
}

double KeyboardKeyBase::getPitch()
{
	return pitch;
}

int DefaultKeyboardKey::getImageWidth()
{
	return keyImageUp.getBounds().getWidth();
}

int DefaultKeyboardKey::getImageHeight()
{
	return keyImageUp.getBounds().getHeight();
}

void MouseKeyboard::constructKeyboardKeys(int numKeysToCreate, std::function<KeyboardKeyBase* (int)> construct)
{
	removeAllChildren();
	keyComponents.clear();
	addedOrder.clear();

	for (int i = 0; i < numKeysToCreate; ++i)
	{
		std::unique_ptr<KeyboardKeyBase> key(construct(static_cast<int>(keyComponents.size())));

		key->indexInParent = static_cast<int>(keyComponents.size());
		key->setInterceptsMouseClicks(false, false);
		key->setBounds(key->calculateBounds(key->indexInParent));

		// add to gui
		addAndMakeVisible(key.get(), key->shouldBeOnTop ? -1 : 0);

		// add to pitch order array to keep track of pitch order / order added
		addedOrder.push_back(key.get());

		// add to array of key children pointers
		keyComponents.insert(key->shouldBeOnTop ? keyComponents.begin() : keyComponents.end(), std::move(key));
		/* KEY IS NULLPTR BELOW THIS LINE */
	}

	lowKeyToDisplay = 0;
	highKeyToDisplay = numKeysToCreate - 1;
}

void MouseKeyboard::setKeysToDisplay(int lowest, int highest)
{
	int sz = static_cast<int>(addedOrder.size());

	jassert(lowest >= 0);
	jassert(highest < sz);
	jassert(highest > lowest);

	lowKeyToDisplay = lowest;
	highKeyToDisplay = highest;

	int tA = 0;

	// center keyboard on desired range
	auto centerPoint = addedOrder[lowest]->getX() + (addedOrder[highest]->getRight() - addedOrder[lowest]->getX()) * 0.5;

	tA += static_cast<int>(getLocalBounds().getCentreX() - centerPoint);

	// stick nearest white key to left
	int count = 0;
	while (count < sz && addedOrder[count]->getRight() + tA < 0)
	{
		++count;
		if (MIDI(count).isSharp())
			++count;
	}

	tA += -(addedOrder[std::min(count, sz - 1)]->getX() + tA);

	// if gap, center keyboard
	if (addedOrder.back()->getRight() + tA < getWidth())
		tA += static_cast<int>((getWidth() - (addedOrder.back()->getRight() + tA)) * 0.5);

	// do final translation
	for (auto& k : addedOrder)
		k->setBounds(k->getBounds().translated(tA, 0));
}

KeyboardKeyBase* MouseKeyboard::getKeyAtPoint(juce::Point<int> v)
{
	for (const auto& key : keyComponents)
		if (key->getBounds().contains(v))
			return key.get();

	return nullptr;
}

void MouseKeyboard::mouseEnter(const MouseEvent & e)
{
	currentKey = getKeyAtPoint(e.getPosition());
}

void MouseKeyboard::mouseExit(const MouseEvent&)
{
	if (lastKey != nullptr)
	{
		lastKey->setIsMouseOver(false);
		if (!lastKey->getIsSustained())
		{
			lastKey->liftKey();
			scheduleKeyLift(lastKey);
		}

		runKeyChangeSchedule();
	}
}

void MouseKeyboard::mouseMove(const MouseEvent& e)
{
	currentKey = getKeyAtPoint(e.getPosition());

	if (lastKey == currentKey)
		return;

	if (lastKey != nullptr) // if mouse moved to another key, hover state for last key is false
		lastKey->setIsMouseOver(false);

	if (currentKey != nullptr) // set initial key hover state to true
		currentKey->setIsMouseOver(true);

	lastKey = currentKey;
}

void MouseKeyboard::mouseDown(const MouseEvent & e)
{
	if (currentKey == nullptr) // mouse down did not occur on a key
		return;

	// toggle/hold key down
	if (e.mods.isCommandDown() || e.mods.isAltDown() || e.mods.isShiftDown() || e.mods.isRightButtonDown())
	{
		if (sustainedKey == nullptr) // no key is currently sustained, so sustain it and no need to un-toggle another key
		{
			sustainedKey = currentKey;

			sustainedKey->sustainKey();
			scheduleKeyPress(sustainedKey);
		}
		else if (sustainedKey == currentKey) // sustained key was clicked with modifier, so un-toggle
		{			
			sustainedKey->unsustainKey();
			scheduleKeyLift(sustainedKey);

			sustainedKey = nullptr;
		}
		else // change the currently sustained key (since keyboard is monophonic at the moment)
		{
			sustainedKey->unsustainKey();
			scheduleKeyLift(sustainedKey);

			sustainedKey = currentKey;

			sustainedKey->sustainKey();
			scheduleKeyPress(sustainedKey);
		}
	}
	else // no mouse modifier, do normal key press
	{
		currentKey->pressKey(); // press key visually

		if (currentKey->getIsSustained()) // do not schedule key press if key is already down/toggled
			return;

		scheduleKeyPress(currentKey);
	}

	runKeyChangeSchedule();	
}

void MouseKeyboard::mouseUp(const MouseEvent& /*e*/)
{
	if (currentKey == nullptr) // no key was initially pressed, do nothing
		return;
	
	currentKey->liftKey();

	if (!currentKey->getIsSustained())
		scheduleKeyLift(currentKey);

	runKeyChangeSchedule();
	
}

void MouseKeyboard::mouseDrag(const MouseEvent & e)
{
	auto* key = getKeyAtPoint(e.getPosition());

	if (key == nullptr) // mouse is not being dragged over a key, do nothing
		return;

	currentKey = key;

	if (lastKey == currentKey) // mouse is still over current key, do nothing
		return;

	if (lastKey != nullptr) // mouse has moved to a new key, lift previous key
	{
		lastKey->liftKey();
		lastKey->setIsMouseOver(false);
		if (!lastKey->getIsSustained())
			scheduleKeyLift(lastKey);
	}

	// press the key the mouse is over, don't schedule key press if already sustained/toggled
	currentKey->setIsMouseOver(true);
	currentKey->pressKey();
	if (!currentKey->getIsSustained())
		scheduleKeyPress(currentKey);	

	lastKey = currentKey; // remember last key

	runKeyChangeSchedule();
}

void MouseKeyboard::scheduleKeyPress(KeyboardKeyBase* key)
{
	key->setVelocity(clamp<float>(getMouseXYRelative().getY() / float(getHeight()), 0, 1));
	keyDownQueue.push_back(key);
}

void MouseKeyboard::scheduleKeyLift(KeyboardKeyBase* key)
{
	keyUpQueue.push_back(key);
}

void MouseKeyboard::runKeyChangeSchedule()
{
	for (const auto & key : keyDownQueue)
		updateState(key);

	for (const auto & key : keyUpQueue)
		updateState(key);

	keyDownQueue.clear();
	keyUpQueue.clear();
}

void MouseKeyboard::updateState(KeyboardKeyBase* key)
{
	state.note = key->getPitch();
	state.velocity = key->getVelocity();
	state.isEventDown = key->getIsPressed();
	listeners.call(&Listener::keyStateChanged);
}
