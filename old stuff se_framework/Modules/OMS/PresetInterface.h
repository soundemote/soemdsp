#pragma once

class PresetInfoEditor
	: public Component
	, public PresetInformation::SenderReceiver
{
public:
	PresetInfoEditor();

	void updateState()
	{
		PresetInformation::SenderReceiver::setState({
			name_textEditor.getText(),
			author_textEditor.getText(),
			category_textEditor.getText(),
			description_textEditor.getText()
			});
	}

	~PresetInfoEditor() = default;

	SharedResourcePointer<Resources> RESOURCES;

	ElanImageButton cancel_button{ &RESOURCES->images.preset_cancel.getImage() };
	ElanImageButton save_button{ &RESOURCES->images.preset_save.getImage() };
	ElanImageButton rename_button{ &RESOURCES->images.preset_rename.getImage() };
	ElanImageButton delete_button{ &RESOURCES->images.preset_delete.getImage() };

	PresetButton load_preset_dropdown;
	PresetButton save_preset_dropdown;
	PresetButton manage_preset_dropdown;

	TextEditor name_textEditor;
	TextEditor author_textEditor;
	TextEditor category_textEditor;
	TextEditor description_textEditor;

protected:
	juce::Rectangle<int> button_area;

	StaticText name_label
	{
		"PRESET NAME",
		RESOURCES->fonts.RobotoBold,
		11.f,
		{95, 100, 91},
		juce::Justification::centredLeft
	};
	StaticText category_label
	{
		"CATEGORY",
		RESOURCES->fonts.RobotoBold,
		11.f,
		{95, 100, 91},
		juce::Justification::centredLeft
	};
	StaticText author_label
	{
		"AUTHOR",
		RESOURCES->fonts.RobotoBold,
		11.f,
		{95, 100, 91},
		juce::Justification::centredLeft
	};
	StaticText description_label
	{
		"DESCRIPTION",
		RESOURCES->fonts.RobotoBold,
		11.f,
		{95, 100, 91},
		juce::Justification::centredLeft
	};

	void stateChanged() override
	{
		name_textEditor.setText(PresetInformation::SenderReceiver::getState().name);
		author_textEditor.setText(PresetInformation::SenderReceiver::getState().author);
		category_textEditor.setText(PresetInformation::SenderReceiver::getState().category);
		description_textEditor.setText(PresetInformation::SenderReceiver::getState().description);
	}

	void resized() override;

	void paint(Graphics & g) override;
};

class PresetInterface : public Component
{
public:
	class Skin : public LookAndFeel_V4
	{
		void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& textEditor) override;

		void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& textEditor) override;

		int getDefaultScrollbarWidth() override { return 8; }

		void drawScrollbar(Graphics& g, ScrollBar& scrollbar,
			int x, int y, int width, int height,
			bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
			bool isMouseOver, bool isMouseDown) override;
	};

	class Skin2 : public LookAndFeel_V4
	{
		void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& textEditor) override;

		void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& textEditor) override;
	};

	class Skin3 : public LookAndFeel_V4
	{
		void fillTextEditorBackground(Graphics& g, int width, int height, TextEditor& textEditor) override;

		void drawTextEditorOutline(Graphics& g, int width, int height, TextEditor& textEditor) override;
	};

	class TreeItem : public TreeViewItem
	{
	public:
		TreeItem(int numSubItems)
		{
			for (int i = 0; i < numSubItems; ++i)
				addSubItem(new TreeItem(0));
		}
		TreeItem(String text_)
		{
			text = text_;
		}
		~TreeItem() = default;

		bool mightContainSubItems() override { return getNumSubItems() != 0; }

		int getItemHeight() const { return 20; }

		void setText(const String & v) { text = v; }

		void paintOpenCloseButton(Graphics& g, const Rectangle<float>& area, Colour backgroundColour, bool isMouseOver) override;
		void paintItem(Graphics& g, int width, int height) override;

		bool isHeader = false;

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TreeItem)

	protected:
		SharedResourcePointer<Resources> RESOURCES;
		String text;
	};

	class ListModel : public ListBoxModel
	{
	public:
		ListModel() = default;
		~ListModel() = default;

		SharedResourcePointer<Resources> RESOURCES;

		int getNumRows() override { return (int)items.size(); }

		void paintListBoxItem(int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override;

		vector<String> items{ "this is a long preset name omg so long!", "this is a long preset name", "somewhat long preset name" };
	};

public:
	PresetInterface();

	~PresetInterface()
	{
		setLookAndFeel(nullptr);
		presetPage.description_textEditor.setLookAndFeel(nullptr);
		presetPage.author_textEditor.setLookAndFeel(nullptr);
		tree.deleteRootItem();
	}

protected:
	SharedResourcePointer<Resources> RESOURCES;

public:
	PresetInfoEditor presetPage;
	ElanImageButton close_button{ &RESOURCES->images.preset_x.getImage() };
	Skin skin;
	Skin2 skin2;
	Skin3 skin3;
	TreeView tree;
	ListBox listBox;
	ListModel listModel;
	UserPresetButton userPresetButton;
	FactoryPresetButton factoryPresetButton;

protected:
	juce::Rectangle<int> presetFolders_area;
	juce::Rectangle<int> presetList_area;
	juce::Rectangle<int> presetEditor_area;
	juce::Rectangle<int> pageTitle_area;

	void resized() override;

	void paint(Graphics& g) override;

	void paintOverChildren(Graphics& g) override;
};
