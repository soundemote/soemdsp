#pragma once

#include "MouseKeyboardImages.h"

#include "elan_juce_helpers/source/MidiHelper.h"

#include <deque>
/*
Make subclass of KeyboardKeyBase to provide custom components for keys.
Just override the paintKey method and calculateTopLeftPosition.
*/
class KeyboardKeyBase : public Component
{
	friend class MouseKeyboard;
public:
	/*
	shouldBeOnTop: IMPORTANT, you usually want to set this to true if the key is a sharp (black) key so that those keys graphically and functionally overlap white keys
	*/
	KeyboardKeyBase(bool shouldBeOnTop = false) : shouldBeOnTop(shouldBeOnTop) {}
	~KeyboardKeyBase() {};

	void pressKey(); // sets key to "down" status and calls repaint
	void liftKey(); // sets key to "up" status and calls repaint

	void sustainKey(); // toggles key to be "stuck down" status calls repaint
	void unsustainKey(); // untoggles key to be unstuck and status calls repaint

	double getPitch(); // gets midi pitch from 0 to 127, is a float to allow for microtuning or pitch offset;
	void setPitch(double v); // sets midi pitch value from 0 to 127, is a float to allow for microtuning or pitch offset

	double getVelocity(); // velocity 0 to 1 value representing mouse away from top 
	void setVelocity(double v);

	bool getIsMouseOver(); // asks if mouse is hovering over key
	bool getIsPressed(); // asks if mouse is pressing down key
	bool getIsSustained(); // asks if not is toggled / stuck down

	/*
	you usually want to set this to true if the key is a sharp (black) key so that those keys graphically and functionally overlap white keys
	if you later change this value, call ... TODO: what to call
	*/
	bool shouldBeOnTop = false; 

protected:
	void setIsMouseOver(bool v);

	double pitch = -1; // midi note pitch from 0 to 127, float to allow for microtuning or pitch offsets
	double velocity = -1; // velocity 0 to 1 value representing mouse away from component's top

	bool isMouseOver = false; // mouse is currently over key
	bool isPressed = false; // mouse has pressed or is held/dragged on key
	bool isSustained = false; // key is toggled down and will remain down until toggled again

	/* OVERRIDE THIS
	Make use of these methods to control drawing:
	getIsPressed(): draw key based on if the note is clicked/held
	getIsSustained(): draw key based on if the note is held down/toggled
	getIsMouseOver(): draw key based on if mouse is hovering over, will be true if isPressed is true.
	getVelocity(): 0 to 1 mouse distance from top of component
	*/
	virtual void paintKey(Graphics& g) = 0;

	/* OVERRIDE THIS
	This function is used by the parent keyboard component to set each key component bounds.
	index: the index with which the key was added to the keyboard component, most likely you should be sure to call addKey() in order of pitch
	boundsOfLastKeyAdded: makes it easier to calculate bounds if your bounds are based on accumulative positioning.
	*/
	virtual Rectangle<int> calculateBounds(int index) = 0;

	void paint(Graphics& g) override
	{
		paintKey(g);
	}

private:
	int indexInParent = -1;
};

/*
Use this class so you don't need to create your own.
This class expects keys to be added in order of pitch and start from C
If you don't want to start at C, use the "setKeyOffset"
*/
struct KeyImageResource
{
	TwoStateImage a{ MouseKeyboardBinary::key_image_a_png, MouseKeyboardBinary::key_image_a_pngSize, MouseKeyboardBinary::key_image_a_pressed_png, MouseKeyboardBinary::key_image_a_pressed_pngSize };
	TwoStateImage cf{ MouseKeyboardBinary::key_image_cf_png, MouseKeyboardBinary::key_image_cf_pngSize, MouseKeyboardBinary::key_image_cf_pressed_png, MouseKeyboardBinary::key_image_cf_pressed_pngSize };
	TwoStateImage d{ MouseKeyboardBinary::key_image_d_png, MouseKeyboardBinary::key_image_d_pngSize, MouseKeyboardBinary::key_image_d_pressed_png, MouseKeyboardBinary::key_image_d_pressed_pngSize };
	TwoStateImage eb{ MouseKeyboardBinary::key_image_eb_png, MouseKeyboardBinary::key_image_eb_pngSize, MouseKeyboardBinary::key_image_eb_pressed_png, MouseKeyboardBinary::key_image_eb_pressed_pngSize };
	TwoStateImage g{ MouseKeyboardBinary::key_image_g_png, MouseKeyboardBinary::key_image_g_pngSize, MouseKeyboardBinary::key_image_g_pressed_png, MouseKeyboardBinary::key_image_g_pressed_pngSize };
	TwoStateImage sharp{ MouseKeyboardBinary::key_image_sharp_png, MouseKeyboardBinary::key_image_sharp_pngSize, MouseKeyboardBinary::key_image_sharp_pressed_png, MouseKeyboardBinary::key_image_sharp_pressed_pngSize };
	TwoStateImage last{ MouseKeyboardBinary::key_image_last_png, MouseKeyboardBinary::key_image_last_pngSize, MouseKeyboardBinary::key_image_last_pressed_png, MouseKeyboardBinary::key_image_last_pressed_pngSize };
};

class DefaultKeyboardKey : public KeyboardKeyBase
{
	SharedResourcePointer<KeyImageResource> RESOURCES;
public:
	/*
	note: midi note that the key will send when activated
	isLastKey: mark the last added as true so that the nice end key graphic can be used
	*/
	DefaultKeyboardKey(int note, bool isLastKey) : KeyboardKeyBase(MIDI(note).isSharp())
	{
		pitch = note;

		switch(MIDI(note).getClassValue())
		{
			// the reason for the weird organization is that certain notes use the same image
		case 0: // C
		case 5: // F
			setKeyImages
			(
				RESOURCES->cf.getImage(0),
				RESOURCES->cf.getImage(1)
			);
			break;
		case 2: // D
			setKeyImages
			(
				RESOURCES->d.getImage(0),
				RESOURCES->d.getImage(1)
			);
			break;
		case 1: // C#
		case 3: // D#
		case 6: // F#
		case 8: // G#
		case 10: // A#
			setKeyImages
			(
				RESOURCES->sharp.getImage(0),
				RESOURCES->sharp.getImage(1)
			);
			break;
		case 4: // E
		case 11: // B
			setKeyImages
			(
				RESOURCES->eb.getImage(0),
				RESOURCES->eb.getImage(1)
			);
			break;
		case 7: // G
			setKeyImages
			(
				RESOURCES->g.getImage(0),
				RESOURCES->g.getImage(1)
			);
			break;
		case 9: // A
			setKeyImages
			(
				RESOURCES->a.getImage(0),
				RESOURCES->a.getImage(1)
			);
			break;
		}

		if (isLastKey)
		{
			setKeyImages
			(
				RESOURCES->last.getImage(0),
				RESOURCES->last.getImage(1)
			);
		}
	}

	void setKeyImages(const Image& up, const Image& down)
	{
		keyImageUp = up;
		keyImageDn = down;
	}

	virtual Rectangle<int> calculateBounds(int index) override
	{
		const vector<int> blackKeyLocations{ 14, 37, 77, 99, 121 };

		const int whiteKeySpacing = 21;
		const int xOffset = whiteKeySpacing * MIDI(index).getOctave() * 7;

		Rectangle<int> bounds{0, 0, getImageWidth(), getImageHeight()};		

		if (shouldBeOnTop)
			bounds.setX(xOffset + blackKeyLocations[MIDI(index).getSharpIndex()]);
		else
			bounds.setX(xOffset + whiteKeySpacing * MIDI(index).getNaturalIndex());

		return bounds;
	}

	int getImageWidth();
	int getImageHeight();

protected:
	virtual void paintKey(Graphics& g) override
	{
		if (getIsPressed())
			g.drawImageAt(keyImageDn, 0, 0);
		else
			g.drawImageAt(keyImageUp, 0, 0);

		if (getIsSustained())
			g.fillAll(Colours::red.withAlpha(0.3f));

		if (getIsMouseOver())
			g.fillAll(Colours::white.withAlpha(0.2f));
	}

	void paint(Graphics & g) override
	{
		paintKey(g);
	}
	
	Image keyImageUp;
	Image keyImageDn;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DefaultKeyboardKey)
};

/*
numKeysToCreate: number of child components to create and store
numKeysToDisplay: number of child components to actually display
construct: constructs the key component where index parameter is passed into function and represents the creation order

Example usage:
```
MouseKeyboard mouseKeyboard;
mouseKeyboard.constructKeyboardKeys(61, [](int index) { return new DefaultKeyboardKey(24 + index, index == 60); });
mouseKeyboard.addListener(this);
addAndMakeVisible(mouseKeyboard);
```
*/
class MouseKeyboard : public Component
{
protected:
	struct State
	{
		double note = -1;
		double velocity = -1;
		bool isEventDown = false;
	} state;

public:
	class Listener
	{
		friend MouseKeyboard;
	public:
		virtual ~Listener() = default;
		virtual void keyStateChanged() = 0;

		State& getKeyboardState() { return keyboard->state; }

	protected:
		MouseKeyboard* keyboard = nullptr;
	};

	void addListener(Listener* l) { listeners.add(l); l->keyboard = this; }
	void removeListener(Listener* l) { listeners.remove(l);  l->keyboard = this; }

	MouseKeyboard() = default;
	~MouseKeyboard() = default;

	void constructKeyboardKeys(int numKeysToCreate, std::function<KeyboardKeyBase* (int)> construct);
	void setKeysToDisplay(int lowest, int highest);

	State & getState() { return state; }

protected:	

	vector<std::unique_ptr<KeyboardKeyBase>> keyComponents;
	vector<KeyboardKeyBase*> addedOrder;

	ListenerList<Listener> listeners;

	std::vector<KeyboardKeyBase*> keyUpQueue;
	std::vector<KeyboardKeyBase*> keyDownQueue;

	KeyboardKeyBase* currentKey = nullptr; // key that the mouse is currently over
	KeyboardKeyBase* lastKey = nullptr; // key that the mouse was previously over
	KeyboardKeyBase* sustainedKey = nullptr; // key that is sustained/toggled

	int lowKeyToDisplay = 0;
	int highKeyToDisplay = 127;

	KeyboardKeyBase* getKeyAtPoint(juce::Point<int> v);

	void scheduleKeyPress(KeyboardKeyBase* key);
	void scheduleKeyLift(KeyboardKeyBase* key);
	void runKeyChangeSchedule(); // in order to press newly clicked keys before lifting keys
	void updateState(KeyboardKeyBase* key);

	void mouseEnter(const MouseEvent& event) override;
	void mouseExit(const MouseEvent& event) override;
	void mouseUp(const MouseEvent& event) override;
	void mouseDown(const MouseEvent& event) override;
	void mouseMove(const MouseEvent& event) override;
	void mouseDrag(const MouseEvent& event) override;

	void resized() override
	{
		setKeysToDisplay(lowKeyToDisplay, highKeyToDisplay);
	}

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MouseKeyboard)
};
