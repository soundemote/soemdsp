#pragma once

class PresetInformation
{
public:
	PresetInformation() = default;
	~PresetInformation() = default;

	struct State
	{
		State() {}
		State(String name_, String author_, String category_, String description_)
			: name(name_)
			, author(author_)
			, category(category_)
			, description(description_)
		{}
			
		String name = "untitled";
		String author = "Soundemote";
		String category = "LEAD";
		String description = "no description";

		bool nameWasChanged = false;

	} state;

	void setState(const State & v)
	{
		state.nameWasChanged = v.name != state.name;
		state = v;
		listeners.callExcluding(sender, &SenderReceiver::stateChanged);
	}

	void xmlToState(const XmlElement& v)
	{
		auto xmlState = v.getChildByName("PatchInformation");

		if (xmlState == nullptr)
		{
			//listeners.call(&SenderReceiver::stateChanged);
			return;
		}

		state.name = xmlState->getStringAttribute("Name");
		state.author = xmlState->getStringAttribute("Author");
		state.category = xmlState->getStringAttribute("Category");
		state.description = xmlState->getStringAttribute("Description");

		//listeners.callExcluding(sender, &SenderReceiver::stateChanged);
	}

	XmlElement* stateToXml(CriticalSection * lock) const
	{
		ScopedLock scopedLock(*lock);

		XmlElement* xmlState = new XmlElement("PatchInformation");

		xmlState->setAttribute("Name", state.name);
		xmlState->setAttribute("Author", state.author);
		xmlState->setAttribute("Category", state.category);
		xmlState->setAttribute("Description", state.description);

		return xmlState;
	}

	class SenderReceiver
	{
		friend PresetInformation;
	public:
		virtual ~SenderReceiver() = default;

		virtual void stateChanged() {};

		void setState(const State & v)
		{
			presetInformation->sender = this;
			presetInformation->setState(v);
		}

		State & getState()
		{
			return presetInformation->state;
		}

		PresetInformation * getParent()
		{
			return presetInformation;
		}

	protected:
		PresetInformation * presetInformation = nullptr;
	};

	void addListener(SenderReceiver * l) { listeners.add(l); l->presetInformation = this; }
	void removeListener(SenderReceiver * l) { listeners.remove(l);  l->presetInformation = this; }

protected:
	SenderReceiver * sender;
	ListenerList<SenderReceiver> listeners;
};

/* TODO: ParameterInfo::InfoSender should be named Sender but it clashes with ParameterValueInput::Sender inside VectorSlider.h ElanModulatableSlider class*/
class ParameterValueInput
	: public TextEditor::Listener
	, public MouseListener
{
public:
	ParameterValueInput(Component * componentToPlaceTextEditor, Component * componentHoldingTextEditor, TextEditor & actualTextEditor)
		: parentComponent(componentToPlaceTextEditor)
		, textEditorComponent(componentHoldingTextEditor)
		, textEditor(actualTextEditor)
	{
		parentComponent->addChildComponent(componentHoldingTextEditor);
		textEditorComponent->setAlwaysOnTop(true);
		textEditor.setInputRestrictions(20, "1234567890.-+");
		textEditor.addListener(this);
		parentComponent->addMouseListener(this, true);
		textEditor.setJustification(juce::Justification::verticallyCentred);
	}
	~ParameterValueInput() = default;

	class InputSender
	{
	public:
		InputSender() = default;
		virtual ~InputSender() = default;

		virtual void setStringValue(String v) = 0;
		virtual String getStringValue() = 0;

		ParameterValueInput * parameterValueInput = nullptr;

		void setParameterValueInputReceiver(ParameterValueInput * v)
		{
			parameterValueInput = v;
		}
	};

	struct State
	{
		Component * component = nullptr;
		InputSender * sender = nullptr;
		juce::Rectangle<int> componentBounds;
	} state;

	//class Listener
	//{
	//	friend ParameterValueInput;
	//public:
	//	Listener() = default;
	//	virtual ~Listener() = default;
	//	virtual void ParameterValueInputStateChanged() {}

	//	State & getParameterValueInputState() { return parameterValueInput->state; }

	//protected:
	//	ParameterValueInput * parameterValueInput;
	//	TextEditor & editor;
	//};

	//void addListener(Listener * l)
	//{
	//	listeners.add(l);
	//	l->parameterValueInput = this;
	//}

	//void removeListener(Listener * l)
	//{
	//	listeners.remove(l);
	//	l->parameterValueInput = this;
	//}

	void update()
	{
		if (state.component == nullptr)
			return;

		state.componentBounds = parentComponent->getLocalArea(state.component, state.component->getLocalBounds());

		state.sender = dynamic_cast<InputSender*>(state.component);

		if (state.sender == nullptr)
			return;

		textEditorComponent->setVisible(true);
		textEditor.setText(state.sender->getStringValue());
		textEditor.selectAll();
		textEditor.grabKeyboardFocus();

		setTextEditorBoundsFunction();
	}

	void mouseDoubleClick(const MouseEvent& event) override
	{
		state.component = event.eventComponent;
		update();

		//listeners.call(&Listener::ParameterValueInputStateChanged);
	}

	std::function<void()> setTextEditorBoundsFunction = [this]()
	{
		textEditorComponent->setCentrePosition(state.componentBounds.getCentre());
	};

protected:
	ListenerList<Listener> listeners;
	Component * parentComponent;
	Component * textEditorComponent;
	TextEditor & textEditor;

	void textEditorEscapeKeyPressed(TextEditor&) override
	{
		textEditorComponent->setVisible(false);
	}

	void textEditorReturnKeyPressed(TextEditor&) override
	{
		if (state.sender == nullptr)
			return;

		textEditorComponent->setVisible(false);
		state.sender->setStringValue(textEditor.getText());
	}

	void mouseDown(const MouseEvent& event) override
	{
		if (
			event.eventComponent == textEditorComponent
			|| event.eventComponent == &textEditor
			|| event.eventComponent == state.component
			)
			return;

		textEditorComponent->setVisible(false);
	}

	void mouseDrag(const MouseEvent& event) override
	{
		if (state.sender == nullptr)
			return;
		if (event.eventComponent != state.component)
			return;

		textEditor.setText(state.sender->getStringValue());
	}

	void mouseWheelMove(const MouseEvent& event, const MouseWheelDetails&) override
	{
		if (state.sender == nullptr)
			return;
		if (event.eventComponent != state.component)
			return;

		textEditor.setText(state.sender->getStringValue());
	}
};

class ParameterInfo
{
public:
	ParameterInfo() = default;
	~ParameterInfo() = default;

protected:
	struct State
	{
		String name;
		String description;
		String value;
	} state;

public:
	class Listener
	{
		friend ParameterInfo;
	public:
		Listener() = default;
		virtual ~Listener() = default;
		virtual void infoComponentChanged() = 0;

		State & getParameterInfoState() { return parameterInfo->state; }

	protected:
		ParameterInfo * parameterInfo;
	};

	/* TODO: ParameterInfo::InfoSender should be named Sender but it clashes with ParameterValueInput::Sender inside VectorSlider.h ElanModulatableSlider class*/
	class InfoSender
	{
	public:
		InfoSender() = default;
		~InfoSender() = default;

		// Change the sender to use another sender instead of itself.
		InfoSender * sender = this;
		String name;
		String description;
		std::function<String()> valueStringFunction = []() { return String(); };

		// Put this function in mouse callbacks such as mouseEnter, mosueDown, mouseDrag
		void updateParameterInfo()
		{
			if (parameterInfo != nullptr)
				parameterInfo->updateInfo(sender);
		}

		void setParameterInfoReceiver(ParameterInfo * v)
		{
			parameterInfo = v;
		}

	protected:
		ParameterInfo * parameterInfo = nullptr;		
	};

	void addListener(Listener * l)
	{
		listeners.add(l);
		l->parameterInfo = this;
	}

	void removeListener(Listener * l)
	{
		listeners.remove(l);
		l->parameterInfo = this;
	}

protected:
	void updateInfo(ParameterInfo::InfoSender * sender)
	{
		state.name = sender->name;
		state.description = sender->description;
		state.value = sender->valueStringFunction();
		listeners.call(&Listener::infoComponentChanged);
	}

	ListenerList<Listener> listeners;
};

