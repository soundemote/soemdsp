#include "BasicEditor.h"

const String PluginFileManager::globalPresetesFolder = "/_Global Presets/";

String PluginFileManager::getVendor()
{
	return info.vendor;
}

String PluginFileManager::getVendorWebsite()
{
	return info.vendorWebsite;
}

String PluginFileManager::getModuleWebsite()
{
	return info.moduleWebsite;
}

String PluginFileManager::getKeyFileFolder()
{
	return userFolder.keys;
}

String PluginFileManager::getCurrentValidKeyFile()
{
	return currentValidKeyFile;
}

String PluginFileManager::getModuleName()
{
	return info.moduleTypeNameString;
}

String PluginFileManager::getModuleNameNoSpaces()
{
	return info.moduleNameNoSpaces;
}

String PluginFileManager::getModuleVerionString()
{
	return info.moduleVersionString;
}

String PluginFileManager::getDebugText()
{
	return debugText;
}

KeyFileMessageBox::KeyFileMessageBox()
{
	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);

	headlineTextField = new RTextField("Message Box");
	headlineTextField->setJustification(Justification::centred);
	headlineTextField->setNoBackgroundAndOutline(true);
	wcs.setAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	headlineTextField->setColourScheme(wcs);
	addAndMakeVisible(headlineTextField);

	bodyTextField = new RTextEditor("MessageBoxBodyTextEditor");
	bodyTextField->setColourScheme(wcs);
	bodyTextField->setMultiLine(true, true);
	bodyTextField->setReadOnly(true);
	bodyTextField->setPopupMenuEnabled(false);
	bodyTextField->setInterceptsMouseClicks(false, false);
	bodyTextField->setText("This is a box in which some message can be displayed");
	addAndMakeVisible(bodyTextField);

	runDemoButton = new RClickButton("Run demo");
	runDemoButton->addRButtonListener(this);
	addWidget(runDemoButton);

	keyFileButton = new RClickButton("Open Key File Location");
	keyFileButton->addRButtonListener(this);
	addWidget(keyFileButton);

	buyNowButton = new RClickButton("Buy Now at www.Soundemote.com");
	buyNowButton->addRButtonListener(this);
	greenButtonPainter.setEnableColor(239, 239, 239);
	greenButtonPainter.setDisableColor(160, 253, 154);
	buyNowButton->setPainter(&greenButtonPainter);
	addWidget(buyNowButton);
}

KeyFileMessageBox::~KeyFileMessageBox()
{
	deleteAllChildren();
}

void KeyFileMessageBox::setHeadlineText(const String& newText)
{
	headlineTextField->setText(newText);
}

void KeyFileMessageBox::setBodyText(const String& newText)
{
	bodyTextField->setText(newText, false);
}

void BasicEditorWithLicensing::showRegistrationDialog()
{
	attemptToFindValidKey();

	if (BasicEditor::modulePtr->getIsInDemoMode())
	{
		if (!Timer::isTimerRunning())
		{
			alertBox.setHeadlineText("How to Register " + pluginFileManager.info.moduleTypeNameString);
			alertBox.runDemoButton->setButtonText("OK");
			alertBox.buyNowButton->setVisible(true);
			alertBox.setBodyText
			(
				"Hello pilot! " + pluginFileManager.info.moduleTypeNameString +
				" demo has timed out. Re-load plugin to continue trial.\n\n" +
				"Or unlock the full version by purchasing a license at " +
				pluginFileManager.info.vendorWebsite +
				". Put the license file here:\n\n" +
				pluginFileManager.userFolder.keys +
				"\n\n-Soundemote team"
			);
		}
		else
		{
			alertBox.setHeadlineText("How to Register " + pluginFileManager.info.moduleTypeNameString);
			alertBox.runDemoButton->setButtonText("Run Demo");
			alertBox.buyNowButton->setVisible(true);
			alertBox.setBodyText
			(
				"Hello pilot! " + pluginFileManager.info.moduleTypeNameString +
				" is in demo mode. This means the plugin will stop working after 20 minutes. Your settings may be saved, but not loaded. \n\nUnlock the full version of " +
				pluginFileManager.info.moduleTypeNameString +
				" by purchasing a license at " +
				pluginFileManager.info.vendorWebsite +
				".\n\nPut the license file here:\n\n" +
				pluginFileManager.userFolder.keys +
				"\n\n-Soundemote team"
			);
		}
	}
	else
	{
		alertBox.setHeadlineText("Have fun!");
		alertBox.setBodyText
		(
			"Dear " + keyValidator.name + ", thank you for supporting " +
			pluginFileManager.info.moduleTypeNameString +
			".\n\nKey file was found:\n\n" +
			pluginFileManager.currentValidKeyFile +
			"\n\nEnjoy!\n-Soundemote team"
		);
		alertBox.runDemoButton->setButtonText("Alright!");
		alertBox.buyNowButton->setVisible(false);
	}

	alertBox.setVisible(true);
	resized();
}

void KeyFileMessageBox::rButtonClicked(RButton* button)
{
	File path = fileManagerPtr->userFolder.keys+"_KEY FILES GO HERE.txt";

	if (button == keyFileButton)
	{
		if (!path.exists())
		{
			if (path.create())
				path.appendText("Put all Soundemote key files in this folder.\n\n-Soundemote team\n\n"
				"@@@@@@@@@///               ///@@@@@@@@@\n"
				"@@@@@@///                     ///@@@@@@\n"
				"@@@@//                           //@@@@\n"
				"@@@//              ########       //@@@\n"
				"@/               ###       ##        /@\n"
				"//            //////////#####        //\n"
				"/          //    ###                  /\n"
				"/          //      ########           /\n"
				"/            /////////                /\n"
				"//                    //             //\n"
				"@/                    //             /@\n"
				"@@@//      //////////             //@@@\n"
				"@@@@//                           //@@@@\n"
				"@@@@@@///                     ///@@@@@@\n"
				"@@@@@@@@@///               ///@@@@@@@@@");
		}

		path.revealToUser();

		if (!path.exists())
		{
			bodyTextField->setText("Error: Could not create or open path, try manually creating or locating it:\n\n"+fileManagerPtr->userFolder.keys);
			return;
		}
	}
	else if (button == buyNowButton)
		URL(fileManagerPtr->info.moduleWebsite).launchInDefaultBrowser();

	setVisible(false);

	getParentComponent()->resized();
}

void KeyFileMessageBox::resized()
{
	buyNowButton->centreWithSize(250, 20);
	buyNowButton->setTopLeftPosition(buyNowButton->getX(), getHeight()-20-4);

	runDemoButton->setBounds(16+80-10, getHeight()-20-4-20-4, 70, 20);

	keyFileButton->setBounds(runDemoButton->getRight()+8, getHeight()-20-4-20-4, 150, 20);

	headlineTextField->setBounds(4, 4, getWidth()-8, 16);
	bodyTextField->setBounds(16, 24, getWidth()-32, getHeight()+4-20-4-20-4-17);
}

BasicEditor::BasicEditor(BasicModule * modulePtr)
	: AudioModuleEditor(modulePtr)
	, modulePtr(modulePtr)
{
	ScopedLock scopedLock(*lock);

	//if (fileManagerPtr)
	//{
	//	AudioModuleEditor::webLink->setURL(fileManagerPtr->info.vendorWebsite);
	//	AudioModuleEditor::webLink->setButtonText(fileManagerPtr->info.vendorWebsite);
	//	AudioModuleEditor::webLink->setDescription("Click here to visit " + fileManagerPtr->info.vendor + " on the web.");
	//}

	setWidgetAppearance(jura::ColourScheme::DARK_ON_BRIGHT);
	setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

	setAsTopLevelEditor(false);
	infoField->setVisible(false);
	webLink->setVisible(false);
	setupButton->setVisible(false);
}

void BasicEditor::createWidgets()
{
  infoField->setVisible(true);
  
  for (myparams * p : modulePtr->paramManager)
  {
    switch (p->type)
    {
      case myparams::Type::BUTTON:
        addWidget(p->widget = new ElanModulatableButton(p->text));
        break;
      case myparams::Type::CLICKBUTTON:
        addWidget(p->widget = new ModulatableClickButton(p->text));
        break;
      case myparams::Type::SLIDER:
        addWidget(p->widget = new rsModulatableSlider());
        break;
      case myparams::Type::KNOB:
        break;
      case myparams::Type::COMBOBOX:
        jassert(p->menuItems.size() != 0); /* You forgot to add menu items! */
        addWidget(p->widget = new ElanModulatableComboBox());
        break;
    }
    p->widget->assignParameter(p->ptr);
    
    if (auto ptr = dynamic_cast<rsModulatableSlider *>(p->widget))
    {
      // need to assign parameter before setting slider name
      ptr->setStringConversionFunction(p->stringConvertFunc);
      ptr->setSliderName(p->text);
    }
    
    p->widget->setDescriptionField(infoField);
    p->widget->setDescription(p->helpText);
  }
  
}

BasicEditorWithLicensing::BasicEditorWithLicensing(BasicModule* modulePtr, PluginFileManager::FileMemory* fileManagerMemory, String moduleTypeNameString, String moduleNameNoSpaces, String moduleWebsite, String moduleDownloadLink)
	: BasicEditor(modulePtr)
	, pluginFileManager(modulePtr, fileManagerMemory, moduleTypeNameString, moduleNameNoSpaces, moduleWebsite, moduleDownloadLink)
{
  keyFileButton = new RClickButton("Register");
  keyFileButton->addRButtonListener(this);
  addWidget(keyFileButton);

  alertBox.fileManagerPtr = &pluginFileManager;
  alertBox.setAlwaysOnTop(true);
  addChildComponent(alertBox);
}

void BasicEditorWithLicensing::rButtonClicked(jura::RButton * button)
{
	if (button != keyFileButton)
		return;

	showRegistrationDialog();
}

String BasicEditorWithLicensing::calculateHeadlineText()
{
	String base = pluginFileManager.info.moduleTypeNameString+
		" "+
		pluginFileManager.info.moduleVersionString+
		pluginFileManager.debugText;

	if (!isTimerRunning() && BasicEditor::modulePtr->getIsInDemoMode())
		return base+" - Demo timed out";
	else if (BasicEditor::modulePtr->getIsInDemoMode())
		return base+" - Demo running";
	else
		return base;
}

void BasicEditorWithLicensing::timerCallback()
{
	stopTimer();

	if (BasicEditor::modulePtr->getIsInDemoMode())
	{
		BasicEditor::modulePtr->setDemoTimedOut(true);

		showAlertBox
		(
			pluginFileManager.info.moduleTypeNameString + ": Demo timed out!",
			"Hello pilot! "+pluginFileManager.info.moduleTypeNameString+
			" demo has timed out. Re-load plugin to continue trial.\n\n"+
			"Or unlock the full version by purchasing a license at "+
			pluginFileManager.info.vendorWebsite+
			". Put the license file here:\n\n"+
			pluginFileManager.userFolder.keys+
			"\n\n-Soundemote team"
		);

		alertBox.runDemoButton->setButtonText("Close");
	}
}

void BasicEditorWithLicensing::showAlertBox(const String & headline, const String & body)
{
	alertBox.setHeadlineText(headline);
	alertBox.setBodyText(body);

	if (BasicEditor::modulePtr->getIsInDemoMode() && Timer::isTimerRunning())
		alertBox.runDemoButton->setButtonText("Run Demo");
	else
		alertBox.runDemoButton->setButtonText("Close");

	setHeadlineText(calculateHeadlineText());

	attemptToFindValidKey();
	alertBox.setVisible(true);
}

void BasicEditorWithLicensing::attemptToFindValidKey()
{
	vector<String> filesToLookFor
	{
		pluginFileManager.info.moduleTypeNameString,
		pluginFileManager.info.moduleTypeNameString.replaceCharacter(' ','_'),
		pluginFileManager.info.moduleNameNoSpaces,
	};

	for (const auto & keyFileNameNoExt : filesToLookFor)
	{
		// search for a key file, and switch to demo-mode if no valid keyfile is found:
		Array<File> foundKeyFiles;
		File(pluginFileManager.getKeyFileFolder()).findChildFiles(foundKeyFiles, File::findFiles, false, keyFileNameNoExt+"*.xml");

		for (const auto & f : foundKeyFiles)
		{
			checkIfKeyFileIsValid(f);

			if (pluginFileManager.currentValidKeyFile.isNotEmpty())
			{
				BasicEditor::modulePtr->setIsInDemoMode(false);
				stopTimer();
			}
			else
			{
				BasicEditor::modulePtr->setIsInDemoMode(true);
				rButtonClicked(keyFileButton);
			}
		}

		if (!BasicEditor::modulePtr->getIsInDemoMode())
			break;
	}

	setHeadlineText(calculateHeadlineText());
}

void BasicEditorWithLicensing::initializePlugIn()
{
	attemptToFindValidKey();

	if (BasicEditor::modulePtr->getIsInDemoMode())
	{
		startTimer(20 * 60 * 1000); // 20 minutes
		//Timer::startTimer(5 * 1000); // 5 seconds for test

		rButtonClicked(keyFileButton);
	}
	else
	{		
		alertBox.setVisible(false);
	}

	setHeadlineText(calculateHeadlineText());
}

bool BasicEditorWithLicensing::checkIfKeyFileIsValid(const juce::File & keyFileToCheck)
{
	XmlDocument keyDocument(keyFileToCheck);
	std::unique_ptr<XmlElement> keyXml = keyDocument.getDocumentElement();
	String pluginName = pluginFileManager.info.moduleTypeNameString;
	String keys = pluginFileManager.userFolder.keys;

	if (keyXml == nullptr)
		return false;

	// read out the encoded serial and the key from the xml-element:
	juce::String encodedSerial  = keyXml->getStringAttribute("Serial", {});
	juce::String keyToBeChecked = keyXml->getStringAttribute("Key", {});

	if (encodedSerial.length() < 8 || keyToBeChecked.length() < 64)
	{
		showAlertBox(pluginName + ": Invalid Keyfile!",
			"A keyfile was found at:\n\n"+keys+"/\n\n...but it did not contain valid data. Plugin will run in demo mode.");
		////////delete keyXml;
		return false;
	}

	// read out the licensee's data (name and address) and pass it to the validator:
	juce::String licenseeName = keyXml->getStringAttribute("Licensee", "No Name");
	juce::String licenseeAddress = keyXml->getStringAttribute("Address", {});
	char* licenseeNameAndAddress =
		toZeroTerminatedString(licenseeName+", "+licenseeAddress);
	keyValidator.setLicenseeNameAndEmail(licenseeNameAndAddress);
	keyValidator.name = licenseeName;
	keyValidator.email = licenseeAddress;

	// convert encoded serial to chcaracter array and pass it to the validator:
	char* serialCharArray = toZeroTerminatedString(encodedSerial);
	keyValidator.setEncodedSerialNumber(serialCharArray);

	// generate the correct key-string for comparison::
	int numCharacters = keyToBeChecked.length();
	char* correctKeyCharArray = keyValidator.getKeyString(numCharacters);
	juce::String correctKey = correctKeyCharArray;

	// free temporarily allocated memory:
	////////delete keyXml;
	if (licenseeNameAndAddress != NULL)
		delete[] licenseeNameAndAddress;
	if (serialCharArray != NULL)
		delete[] serialCharArray;
	if (correctKeyCharArray != NULL)
		delete[] correctKeyCharArray;

	// compare the key-string form the xml-file to the correct string:
	if (keyToBeChecked == correctKey)
	{
		pluginFileManager.currentValidKeyFile = keyFileToCheck.getFullPathName();
		BasicEditor::modulePtr->setIsInDemoMode(false);
		return true;
	}
	else
	{
		showAlertBox(pluginName+": Invalid Keyfile!",
			juce::String("A keyfile was found at:\n\n"+keys+"/\n\n...but the key is invalid. PlugIn will run in demo mode."));
		return false;
	}
}

PresetBar::PresetBar(jura::AudioModule* audioModule_, PluginFileManager::FileMemory* fileManagerMemory_, PresetBarDropdownSkin* presetDropdownSkin_, StateLoadSaveWidgetSet* stateWidgetSet_)
	: moduleToEdit(audioModule_)
	, presetDropdownSkin(presetDropdownSkin_)
	, stateWidgetSet(stateWidgetSet_)
	, fileManagerMemory(fileManagerMemory_)
{
	addAndMakeVisible(save_button);
	addAndMakeVisible(left_button);
	addAndMakeVisible(right_button);
	addAndMakeVisible(presetName_button);
	addChildComponent(textEditor);
	textEditor.setInputRestrictions(100);
	save_button.getProperties().set("BubbleHelp", "SAVE");

	textEditor.onReturnKey = [&]()
	{
		textEditor.setVisible(false);

		if (textEditor.isEmpty())
			return;

		if (stateWidgetSet->getWatchedStateManager() == nullptr)
			return;

		jura::StateFileManager* underlyingStateFileManager = dynamic_cast<jura::StateFileManager*> (stateWidgetSet->getWatchedStateManager());
		if (underlyingStateFileManager == nullptr)
			return;

		String userPresetFolder = fileManagerMemory->getUserPresetFolder();

		try
		{
			if (!FileHelper::doesFolderExist(userPresetFolder))
				FileHelper::createFolder(userPresetFolder);
		}
		catch (exception&)
		{

		}

		File fileToLoad = File(userPresetFolder + "/" + textEditor.getText()).withFileExtension("xml");

		fileToLoad = FileHelper::getUnusedFilePath(fileToLoad.getFullPathName(), true);

		underlyingStateFileManager->saveStateToXmlFile(fileToLoad);
		fileManagerMemory->updateFileList();
		fileManagerMemory->setPresetFile(fileToLoad);
		setPresetName(fileToLoad.getFileNameWithoutExtension());
	};

	textEditor.onEscapeKey = [&]() { textEditor.setVisible(false); };

	////////getPresetIconFunction = [&](const File& preset)
	//{
	//	if (preset.getParentDirectory().getFullPathName().contains(fileManagerMemory->getFactoryPresetFolder()))
	//		return factoryIcon.createCopyWithColor(Colours::red, { 176, 128, 191 });

	//	return userIcon.createCopyWithColor(Colours::red, { 116, 156, 105 });
	//};

	left_button.paintFunction = [&](Graphics& g)
	{
		auto& b = left_button;
		auto area = left_button.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(colors.border);
		g.drawRect(area, scale);

		if (b.isDown())
			g.setColour(colors.clicked);
		else if (b.isOver())
			g.setColour(colors.hover);
		else
			g.setColour(colors.normal);
		area.reduce(area.getWidth() * 0.35f, area.getWidth() * 0.35f);
		g.fillPath(ElanPath::triangle(area, Edge::Left));
	};

	right_button.paintFunction = [&](Graphics& g)
	{
		auto& b = right_button;
		auto area = b.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(colors.border);
		g.fillPath(ElanPath::threeSides(area, scale, Edge::Left));

		if (b.isDown())
			g.setColour(colors.clicked);
		else if (b.isOver())
			g.setColour(colors.hover);
		else
			g.setColour(colors.normal);
		area.reduce(area.getWidth() * 0.35f, area.getWidth() * 0.35f);
		g.fillPath(ElanPath::triangle(area, Edge::Right));
	};

	presetName_button.paintFunction = [&](Graphics& g)
	{
		auto& b = presetName_button;
		auto area = b.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(colors.border);
		g.drawRect(area, scale);

		Colour color;
		if (b.isDown())
			color = colors.clicked;
		else if (b.isOver())
			color = { 255, 255, 255 };
		else
			color = colors.font;
		g.setColour(color);
		g.setFont(presetDropdownSkin->popupMenuFont.withPointHeight(10.f * scale));
		g.drawText(getPresetNameText(), ElanRect::pad(area.toNearestInt(), int(10 * scale), 0, 0, (int)round(scale)), Justification::centredLeft);
	};

	save_button.paintFunction = [this](Graphics& g)
	{
		auto& b = save_button;
		auto area = b.getLocalBounds().toFloat();

		if (b.isOver() && !b.isDown())
		{
			g.setColour(colors.hoverFill);
			g.fillRect(area);
		}

		g.setColour(colors.border);
		g.drawRect(area, scale);

		Colour color;
		if (b.isDown())
			color = colors.clicked;
		else if (b.isOver())
			color = colors.hover;
		else
			color = colors.normal;

		area.reduce(4.f * scale, 4.f * scale);
		saveIcon.drawWithColor(g, area.toNearestInt().toFloat(), Colours::red, color);
	};

	left_button.clickFunction = [&]()
	{
		if (stateWidgetSet->getWatchedStateManager() == nullptr)
			return;

		jura::StateFileManager* underlyingStateFileManager = dynamic_cast<jura::StateFileManager*> (stateWidgetSet->getWatchedStateManager());
		if (underlyingStateFileManager == nullptr)
			return;

		File fileToLoad = fileManagerMemory->getPreviousFile();
		if (underlyingStateFileManager->loadFile(fileToLoad))
		{
			stateWidgetSet->sendChangeMessage();
			setPresetName(fileToLoad.getFileNameWithoutExtension());
		}
		else
		{
			fileManagerMemory->updateFileList();
			setPresetName("click here for presets...");
		}
	};

	right_button.clickFunction = [&]()
	{
		if (stateWidgetSet->getWatchedStateManager() == nullptr)
			return;

		StateFileManager* underlyingStateFileManager = dynamic_cast<StateFileManager*> (stateWidgetSet->getWatchedStateManager());
		if (underlyingStateFileManager == nullptr)
			return;

		File fileToLoad = fileManagerMemory->getNextFile();
		if (underlyingStateFileManager->loadFile(fileToLoad))
		{
			stateWidgetSet->sendChangeMessage();
			setPresetName(fileToLoad.getFileNameWithoutExtension());
		}
		else
		{
			fileManagerMemory->updateFileList();
			setPresetName("click here for presets...");
		}
	};

	save_button.clickFunction = [&]()
	{
		save_button.resetState();
		textEditor.setVisible(true);
		textEditor.grabKeyboardFocus();
		textEditor.selectAll();
		textEditor.setText(getPresetNameText() == "click here for presets..." ? "" : getPresetNameText());
	};

	presetName_button.clickFunction = [&]()
	{
		presetName_button.resetState();
		PopupMenu menu;
		presetDropdownSkin->setSize(presetName_button.getWidth(), presetName_button.getHeight());
		menu.setLookAndFeel(presetDropdownSkin);

		MenuHelper::addCallbackItem(menu, "Init Preset", [&]()
			{
				if (stateWidgetSet->getWatchedStateManager() == nullptr)
					return;

				jura::StateFileManager* underlyingStateFileManager = dynamic_cast<jura::StateFileManager*> (stateWidgetSet->getWatchedStateManager());
				if (underlyingStateFileManager == nullptr)
					return;

				File initPreset = fileManagerMemory->getInitPresetPath();

				if (!FileHelper::doesFileExist(initPreset.getFullPathName()))
				{
					moduleToEdit->resetParametersToDefaultValues();
				}
				else
				{
					underlyingStateFileManager->loadStateFromXmlFile(initPreset);
					fileManagerMemory->setPresetFile(initPreset);
					setPresetName("init");
				}

				presetWasChangedFunction();
			});

		MenuHelper::addCallbackItem(menu, "Load Preset...", [&]()
			{
				if (stateWidgetSet->getWatchedStateManager() == nullptr)
					return;

				jura::StateFileManager* underlyingStateFileManager = dynamic_cast<jura::StateFileManager*> (stateWidgetSet->getWatchedStateManager());
				if (underlyingStateFileManager == nullptr)
					return;

				String userPresetFolder = fileManagerMemory->getUserPresetFolder();

				try
				{
					if (!FileHelper::doesFolderExist(userPresetFolder))
						FileHelper::createFolder(userPresetFolder);
				}
				catch (exception&) {}

				FileChooser chooser("Load preset xml", userPresetFolder, "*.xml", true);
				if (chooser.browseForFileToOpen())
				{
					File fileToLoad = chooser.getResult();
					underlyingStateFileManager->loadStateFromXmlFile(fileToLoad);
					fileManagerMemory->setPresetFile(fileToLoad);
					setPresetName(fileToLoad.getFileNameWithoutExtension());
				}

				presetWasChangedFunction();
			});

		MenuHelper::addCallbackItem(menu, "Copy preset", [&]()
			{
				XmlElement* xmlState = moduleToEdit->getStateAsXml("", true);
				if (xmlState == nullptr)
					return;

				juce::SystemClipboard::copyTextToClipboard(xmlState->createDocument(String()));
			});

		MenuHelper::addCallbackItem(menu, "Paste preset", [&]()
			{
				String clip = juce::SystemClipboard::getTextFromClipboard();
				XmlDocument doc(clip);

				std::unique_ptr <XmlElement> state = doc.getDocumentElement();

				if (state.get() == nullptr)
					return;

				setPresetName("from clipboard");
				moduleToEdit->setStateFromXml(*state.get(), "from clipboard", true);

				presetWasChangedFunction();
			});

		MenuHelper::addSeperator(menu);

		MenuHelper::addCallbackItem(menu, "OPEN: User presets folder", [&]()
			{
				try { FileHelper::openFolder(fileManagerMemory->getUserPresetFolder(), true); }
				catch (exception&)
				{

				};
			});

		MenuHelper::addCallbackItem(menu, "OPEN: Factory presets folder", [&]()
			{
				try { FileHelper::openFolder(fileManagerMemory->getFactoryPresetFolder(), true); }
				catch (exception&)
				{

				};
			});

		MenuHelper::addCallbackItem(menu, "Refresh presets", [&]()
			{
				fileManagerMemory->updateFileList();
			});

		MenuHelper::addSeperator(menu);

		for (int i = 0; i < fileManagerMemory->getFiles().size(); ++i)
		{
			auto f = fileManagerMemory->getFiles()[i];

			Drawable* icon = getPresetIconFunction(f);

			MenuHelper::addCallbackItem(menu, f.getFileNameWithoutExtension(), [&, f]()
				{
					if (stateWidgetSet->getWatchedStateManager() == nullptr)
						return;

					StateFileManager* underlyingStateFileManager = dynamic_cast<StateFileManager*> (stateWidgetSet->getWatchedStateManager());
					if (underlyingStateFileManager == nullptr)
						return;

					underlyingStateFileManager->loadFile(f);
					fileManagerMemory->setPresetFile(f);
					setPresetName(f.getFileNameWithoutExtension());

					presetWasChangedFunction();
				}, icon);
		}

		MenuHelper::show(&menu, &presetName_button);
	};
}
