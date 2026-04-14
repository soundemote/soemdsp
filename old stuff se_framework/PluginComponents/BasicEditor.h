#pragma once

#include "JuceHeader.h"
#include <algorithm>
#include <map>
#include <vector>


/*
To use PluginFileManager, create a single PluginFileManager instance in the
Editor. The editor itself must also inherit PluginFileManager::FileMemory.

Every editor must inherit PluginFileManager::FileMemory. PluginFileManager then
creates SubModuleFileManagers for each module type specified by
moduleTypeNameString.

SubModuleFileManager holds the preset folders and other file locations for that
module type to avoid storing that information for every single module.
FileMemory then stores the specific preset loaded and other information for
every instance of the module. SubModuleFileManager is managed by
PluginFileManager automatically.
*/

class PluginFileManager {
public:
    struct UserFolder {
        juce::String vendor; // user folder path for vendor
        // (C:\Users\Elan\Documents\Soundemote)
        juce::String module; // user folder path for module
        // (C:\Users\Elan\Documents\Soundemote\PrettyScope)
        juce::String keys;   // user folder path for keys
        // (C:\Users\Elan\Documents\Soundemote\Keys)
    } userFolder;

    struct ProgramFolder {
        juce::String vendor; // C:\Program Files\Soundemote
        juce::String module; // C:\Program Files\Soundemote\PrettyScope130
    } programFolder;

    struct Info {
        juce::String vendor;        // Soundemote
        juce::String vendorWebsite; // https://www.soundemote.com/
        juce::String
            moduleWebsite; // https://www.soundemote.com/plugins/prettyscope
        juce::String
            moduleDownloadLink; // http://www.elanhickler.com/Soundemote/Soundemote_-_PrettyScope.zip
        juce::String moduleTypeNameString; // "PrettyScope"
        juce::String moduleNameNoSpaces;   // "PrettyScope"
        juce::String moduleVersionString;  // "2.0.0"
    } info;

    struct FileMemory;

protected:
    struct SubModuleFileManager {
        SubModuleFileManager() {}

        SubModuleFileManager([[maybe_unused]] AudioModule* module,
            juce::String moduleTypeNameString, bool isGlobal,
            PluginFileManager* parent)
            : moduleTypeNameString(moduleTypeNameString), isGlobal(isGlobal),
            parent(parent) {
            setupPaths();
            updatePresetFileList();
        }

        PluginFileManager* parent = nullptr;

        std::vector<FileMemory*> subModuleMemories;

        juce::String moduleTypeNameString;

        juce::String presetPath_user;
        juce::String presetPath_factory;
        juce::String initPresetFile;

        bool isGlobal = false;

        void setupPaths() {
            if (isGlobal) {
                presetPath_user = parent->userFolder.vendor + globalPresetesFolder +
                    moduleTypeNameString + " " + "Presets";
                presetPath_factory = parent->programFolder.vendor +
                    globalPresetesFolder + moduleTypeNameString + " " +
                    "Factory Presets";
                initPresetFile = parent->programFolder.vendor + globalPresetesFolder +
                    moduleTypeNameString + " " + "init.xml";
            }
            else {
                presetPath_user = parent->userFolder.module + "/" +
                    moduleTypeNameString + " " + "Presets";
                presetPath_factory = parent->programFolder.module + "/" +
                    moduleTypeNameString + " " + "Factory Presets";
                initPresetFile = parent->programFolder.module + "/" +
                    moduleTypeNameString + " " + "init.xml";
            }

            // presetPath_user = FileHelper::toUniversalSlash(presetPath_user);
            // presetPath_factory = FileHelper::toUniversalSlash(presetPath_factory);
            // initPresetFile = FileHelper::toUniversalSlash(initPresetFile);
        }

        const juce::Array<juce::File>& getFileList() const {
            return presetFileList;
        }

        void updatePresetFileList() {
            FileSearchPath searchPaths;
            searchPaths.add(presetPath_user);
            searchPaths.add(presetPath_factory);
            presetFileList = searchPaths.findChildFiles(
                juce::File::TypesOfFileToFind::findFiles, true, "*.xml");

            if (presetFileList.isEmpty())
                return;

            presetFileList.sort(sorter);

            for (auto& m : subModuleMemories)
                m->fileListWasUpdated();
        }

    protected:
        struct Sorter {
        public:
            static int compareElements(const juce::File& a, const juce::File& /*b*/) {
                jassertfalse; // for some reason I can't get BasicEditor.h to find StringIterator from StringHelper.h so it is commented out for now
                juce::String aStr;// = StringIterator(a.getFileNameWithoutExtension()).prepareForNaturalSort();
                juce::String bStr;// = StringIterator(b.getFileNameWithoutExtension()).prepareForNaturalSort();

                if (aStr == bStr) {
                    if (a.getParentDirectory().getFullPathName().contains("Factory"))
                        return -1;
                }

                return aStr.compareNatural(bStr);
            }
        } sorter;

        juce::Array<juce::File> presetFileList;
    };

public:
    struct FileMemory {
        friend class PluginFileManager;
        FileMemory() {}

        juce::File getNextFile() {

            updateCurrentPreset(wrapSingleOctave_Positive(
                currentPresetIndex + 1,
                subModuleFileManagerPtr->getFileList().size()));
            return currentPresetFile;
        }

        juce::File getPreviousFile() {
            if (currentPresetIndex < 0)
                currentPresetIndex = 0;

            updateCurrentPreset(wrapSingleOctave_Negative(
                currentPresetIndex - 1,
                subModuleFileManagerPtr->getFileList().size()));
            return currentPresetFile;
        }

        void setPresetFile(juce::File file) { updateCurrentPreset(file); }

        void fileListWasUpdated() { updateCurrentPreset(currentPresetFile); }

        juce::String getInitPresetPath() {
            return subModuleFileManagerPtr->initPresetFile;
        }

        juce::String getFactoryPresetFolder() {
            return subModuleFileManagerPtr->presetPath_factory;
        }
        juce::String getUserPresetFolder() {
            return subModuleFileManagerPtr->presetPath_user;
        }

        void updateFileList() { subModuleFileManagerPtr->updatePresetFileList(); }

        const juce::Array<juce::File>& getFiles() {
            return subModuleFileManagerPtr->getFileList();
        }

    protected:
        int currentPresetIndex = 0;
        juce::File currentPresetFile;
        SubModuleFileManager* subModuleFileManagerPtr = nullptr;

        void updateCurrentPreset(int idx) {
            if (subModuleFileManagerPtr->getFileList().isEmpty())
                return;

            currentPresetIndex = idx;
            currentPresetFile = subModuleFileManagerPtr->getFileList()[idx];
        }

        void updateCurrentPreset(juce::File file) {
            if (subModuleFileManagerPtr->getFileList().isEmpty())
                return;

            currentPresetIndex = subModuleFileManagerPtr->getFileList().indexOf(file);
            currentPresetFile = subModuleFileManagerPtr
                ->getFileList()[std::max(0, currentPresetIndex)];
        }
    };

    PluginFileManager(AudioModule* audioModulePtr, FileMemory* fileManagerMemory,
        juce::String moduleTypeNameString,
        juce::String moduleNameNoSpaces, juce::String moduleWebsite,
        juce::String moduleDownloadLink) {
        parentModulePtr = audioModulePtr;

        info.vendor = JucePlugin_Manufacturer;
        info.vendorWebsite = JucePlugin_ManufacturerWebsite;
        info.moduleVersionString = JucePlugin_VersionString;

        info.moduleWebsite = moduleWebsite;
        info.moduleDownloadLink = moduleDownloadLink;

        info.moduleTypeNameString = moduleTypeNameString;
        info.moduleNameNoSpaces = moduleNameNoSpaces;

        setupPaths();

        registerModule(audioModulePtr, fileManagerMemory, moduleTypeNameString,
            false);
    }

    void setupPaths() {
        desktopPath =
            juce::File::getSpecialLocation(juce::File::userDesktopDirectory)
            .getFullPathName();
        keyFileNameNoExt = info.moduleNameNoSpaces + "Key";

#ifdef _WIN32
        programFolder.vendor =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::globalApplicationsDirectory)
            .getFullPathName() +
            "/" + info.vendor;
        userFolder.vendor =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userDocumentsDirectory)
            .getFullPathName() +
            "/" + info.vendor;
#elif __APPLE__
        programFolder.vendor = "/Library/Audio/Presets/" + info.vendor;
        userFolder.vendor =
            juce::File("~/Documents").getFullPathName() + "/" + info.vendor;
#elif __linux__
        programFolder.vendor =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::globalApplicationsDirectory)
            .getFullPathName() +
            "/" + info.vendor;
        userFolder.vendor =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userDocumentsDirectory)
            .getFullPathName() +
            "/" + info.vendor;
#endif

#if JUCE_DEBUG
        debugText = " DEBUG ";
#endif

        // FileHelper::toSystemSlash(desktopPath);

        // userFolder.keys = FileHelper::toSystemSlash(userFolder.vendor + "/Keys");
        // userFolder.module = FileHelper::toSystemSlash(userFolder.vendor + "/" +
        // info.moduleTypeNameString);

        // programFolder.module = FileHelper::toSystemSlash(programFolder.vendor +
        // "/" + info.moduleTypeNameString);
    }

    virtual ~PluginFileManager() = default;

    juce::String desktopPath;
    juce::String debugText;

    juce::String keyFileNameNoExt;
    juce::String currentValidKeyFile;

    AudioModule* parentModulePtr = nullptr;
    FileMemory* parentModuleMemory = nullptr;

    juce::String getModuleHeadlineString() {
        return PluginFileManager::info.moduleTypeNameString + " " +
            PluginFileManager::info.moduleVersionString + " " +
            PluginFileManager::debugText +
            parentModulePtr->getModuleNameAppendix();
    }

    void registerModule(AudioModule* module, FileMemory* fileManagerMemory,
        juce::String moduleTypeNameString, bool isGlobal) {
        jassert(!isModuleRegistered(module));

        subModuleManagers[moduleTypeNameString] = { module, moduleTypeNameString,
                                                   isGlobal, this };
        subModuleManagers[moduleTypeNameString].subModuleMemories.push_back(
            fileManagerMemory);
        subModuleManagerTypeNameLookup[module] = moduleTypeNameString;
        registeredModules.push_back(module);

        module->setActiveDirectory(
            subModuleManagers[moduleTypeNameString].presetPath_user);
        fileManagerMemory->subModuleFileManagerPtr =
            &subModuleManagers[moduleTypeNameString];
    }

    void unregisterModule(AudioModule* /*module*/) {
        // jassert(isModuleRegistered(module));

        // auto iter = subModuleManagerTypeNameLookup.find(module);

        // auto& memoryList = subModuleManagers[iter->second].subModuleMemories;
        // memoryList.erase(std::find(memoryList.begin(), memoryList.end(),
        // module));

        // auto iter2 = std::find(registeredModules.begin(),
        // registeredModules.end(), module); registeredModules.erase(iter2);

        // if (memoryList.size() == 0)
        //{
        //	subModuleManagers.erase(iter->second);
        //	subModuleManagerTypeNameLookup.erase(iter);
        // }
    }

    bool isModuleRegistered(AudioModule* module) {
        return std::find(registeredModules.begin(), registeredModules.end(),
            module) != registeredModules.end();
    }

    void setModuleWebsite(juce::String v) { info.moduleWebsite = v; }

    void setModuleNameString(const juce::String& moduleName_,
        const juce::String& moduleNameNoSpaces_) {
        info.moduleTypeNameString = moduleName_;
        info.moduleNameNoSpaces = moduleNameNoSpaces_;
        parentModulePtr->setModuleName(moduleNameNoSpaces_);
        setupPaths();
    }

    void setModuleVersionString(const juce::String& v) {
        info.moduleVersionString = v;
    }

    void setModuleDownloadLink(const juce::String& v) {
        info.moduleDownloadLink = v;
    }
    juce::String getVendor();

    juce::String getVendorWebsite();

    juce::String getModuleWebsite();

    juce::String getKeyFileFolder();

    juce::String getCurrentValidKeyFile();

    juce::String getModuleName();

    juce::String getModuleNameNoSpaces();

    juce::String getModuleVerionString();

    juce::String getDebugText();

    juce::String getBuildString() {
        return info.moduleTypeNameString + debugText + " v" +
            info.moduleVersionString;
    }

protected:
    std::map<juce::String, SubModuleFileManager> subModuleManagers;
    std::map<AudioModule*, juce::String> subModuleManagerTypeNameLookup;
    std::vector<AudioModule*> registeredModules;

    static const juce::String globalPresetesFolder;

    void normalize(juce::String& s) { s = s.replaceCharacter('\\', '/'); }
};

class KeyFileMessageBox : public ColourSchemeComponent, public RButtonListener {
public:
    KeyFileMessageBox();
    ~KeyFileMessageBox();

    void setHeadlineText(const juce::String& newText);
    void setBodyText(const juce::String& newText);

    void resized() override;

    PluginFileManager* fileManagerPtr;

    RClickButton* runDemoButton;
    RClickButton* keyFileButton;
    RClickButton* buyNowButton;

protected:
    RTextEditor* bodyTextField;
    RTextField* headlineTextField;

    ButtonWithCustomColor greenButtonPainter;

    jura::WidgetColourScheme wcs;

    void rButtonClicked(RButton* button) override;
};

class JuraBasicEditor : public jura::AudioModuleEditor {
public:
    JuraBasicEditor(ModulatableAudioModule* modulePtr)
        : AudioModuleEditor(modulePtr) {
        setPresetSectionPosition(AudioModuleEditor::positions::INVISIBLE);

        setAsTopLevelEditor(false);
        infoField->setVisible(false);
        webLink->setVisible(false);
        setupButton->setVisible(false);
    }

    virtual ~JuraBasicEditor() = default;

    virtual void paint(Graphics& g) override {
        g.setColour(Colour::fromRGB(14, 14, 14));
        g.fillRect(getLocalBounds().toFloat());
    };
    virtual void paintOverChildren(Graphics& /*g*/) override {};

protected:
};

class BasicEditor : public jura::AudioModuleEditor {
public:
    BasicEditor(BasicModule* modulePtr);

    virtual ~BasicEditor() = default;

    void setScale(float v) { scale = v; }

    void setOffset(juce::Point<int> v) { offset = v; }

    float getScale() { return scale; }

    juce::Point<int> getOffset() { return offset; }

    virtual void createWidgets();

    virtual void paint(Graphics& g) override {
        ColourSchemeComponent::paint(g);
        g.setColour(editorColourScheme.outline);
        for (int i = 0; i < guiLayoutRectangles.size(); i++) {
            fillRectWithBilinearGradient(
                g, guiLayoutRectangles[i], editorColourScheme.topLeft,
                editorColourScheme.topRight, editorColourScheme.bottomLeft,
                editorColourScheme.bottomRight);
            g.drawRect(guiLayoutRectangles[i], 2);
        }

        drawHeadline(g);
    };
    virtual void paintOverChildren(Graphics& g) override {
        g.setColour({ 127, 127, 127 });
        g.drawRect(getLocalBounds().toFloat(), 2.f);
    };

protected:
    float scale = 1;
    juce::Point<int> offset;

    BasicModule* modulePtr = nullptr;
};

class BasicEditorWithLicensing : public BasicEditor, public Timer {
public:
    BasicEditorWithLicensing(BasicModule* modulePtr,
        PluginFileManager::FileMemory* fileManagerMemory,
        juce::String moduleTypeNameString,
        juce::String moduleNameNoSpaces,
        juce::String moduleWebsite,
        juce::String moduleDownloadLink);
    virtual ~BasicEditorWithLicensing() = default;

    void showRegistrationDialog();

    jura::RClickButton* keyFileButton;
    KeyFileMessageBox alertBox;
    se::KeyGenerator keyValidator;

    void initializePlugIn();

    bool checkIfKeyFileIsValid(const juce::File& keyFileToCheck);

    void showAlertBox(const juce::String& headline, const juce::String& body);

    bool getIsInDemoMode() { return isInDemoMode; }

protected:
    bool isInDemoMode;

    void timerCallback() override;

    juce::String calculateHeadlineText();

    void attemptToFindValidKey();

    void resized() override {
        BasicEditor::resized();

        AudioModuleEditor::stateWidgetSet->setBounds(
            getHeadlineRight(), 6, getWidth() - getHeadlineRight() - 60 - 4, 16);
        BasicEditorWithLicensing::keyFileButton->setBounds(
            stateWidgetSet->getRight(), 6, 60, 16);

        alertBox.centreWithSize(400, 250);
    }

    int productIndex = 0;

    // Opens registration help alert box
    void rButtonClicked(jura::RButton* button) override;

    PluginFileManager pluginFileManager;
};

class PresetBarDropdownSkin : public LookAndFeel_V4 {
public:
    void setSize(int w, int h) {
        width = w;
        height = h;
    }

    void setScale(float v) { scale = v; }

    struct Colors {
        Colour normal = { 162, 162, 203 };
    } colors;

    FontContainer popupMenuFont{ "Arial" };

protected:
    float scale = 1.0;
    int width = 0;
    int height = 0;

    void drawPopupMenuBackground(Graphics& g, int, int) override {
        g.fillAll({ 38, 38, 48 });
    }

    Font getPopupMenuFont() override {
        return popupMenuFont.withPointHeight(10.f * scale);
    }

    void getIdealPopupMenuItemSize(const juce::String& text, bool isSeparator,
        int standardMenuItemHeight, int& idealWidth,
        int& idealHeight) override {
        if (isSeparator) {
            idealWidth = 50;
            idealHeight = int(7 * scale);
        }
        else {
            auto font = getPopupMenuFont();

            if (standardMenuItemHeight > 0 &&
                font.getHeight() > standardMenuItemHeight / 1.3f)
                font.setHeight(standardMenuItemHeight / 1.3f);

            idealHeight = standardMenuItemHeight > 0
                ? standardMenuItemHeight
                : roundToInt(font.getHeight() * 1.3f);
            idealWidth = font.getStringWidth(text) + idealHeight * 2;
        }
    }

    int getPopupMenuBorderSize() override { return 0; }

    void drawPopupMenuItem(Graphics& g, const Rectangle<int>& area,
        bool isSeparator, bool isActive, bool isHighlighted,
        bool /*isTicked*/, bool /*hasSubMenu*/,
        const juce::String& text,
        const juce::String& /*shortcutKeyText*/,
        const Drawable* icon,
        const Colour* textColourToUse) override {
        auto textColour =
            (textColourToUse == nullptr ? findColour(PopupMenu::textColourId)
                : *textColourToUse);

        auto r = area.reduced(1);

        if (isHighlighted && isActive) {
            g.setColour({ 160, 43, 73 });
            if (!isSeparator)
                g.fillRect(r);

            g.setColour(findColour(PopupMenu::highlightedTextColourId));
        }
        else {
            g.setColour(textColour.withMultipliedAlpha(isActive ? 1.0f : 0.5f));
        }

        if (isSeparator) {
            g.setColour({ 85, 85, 121 });
            g.drawLine((float)area.getX(), (float)area.getCentreY(),
                (float)area.getWidth(), (float)area.getCentreY(), 1.f * scale);
        }
        else {
            if (icon != nullptr) {
                auto iconArea = r.removeFromLeft(area.getHeight()).toFloat();
                iconArea = ElanRect::fit(
                    icon->getDrawableBounds(),
                    iconArea.withSizeKeepingCentre(11 * scale, 11 * scale), MidCenter,
                    MidCenter);
                unique_ptr<Drawable> iconToUse =
                    unique_ptr<Drawable>(icon->createCopy());
                if (isHighlighted) {
                    iconToUse.get()->replaceColour({ 176, 128, 191 }, { 205, 205, 205 });
                    iconToUse.get()->replaceColour({ 116, 156, 105 }, { 205, 205, 205 });
                    iconToUse.get()->replaceColour(colors.normal, { 205, 205, 205 });
                }
                iconToUse.get()->drawWithin(
                    g, iconArea.toNearestInt().toFloat(),
                    RectanglePlacement(RectanglePlacement::centred), 1.f);
            }

            g.setFont(getPopupMenuFont());
            r.removeFromRight(3);
            g.setColour({ 204, 204, 204 });
            g.drawText(text, r, Justification::centredLeft, 1);
        }
    }
};

class PresetBar : public Component {
public:
    PresetBar(jura::AudioModule* audioModule_,
        PluginFileManager::FileMemory* fileManagerMemory_,
        PresetBarDropdownSkin* presetDropdownSkin_,
        StateLoadSaveWidgetSet* stateWidgetSet_);

    ~PresetBar() { setLookAndFeel(nullptr); }

    LeftClickButton save_button;
    LeftClickButton left_button;
    LeftClickButton right_button;
    LeftClickButton presetName_button;

    VectorGraphic factoryIcon{
        R"SVG(<svg viewBox="0 0 10 11"><path fill="#9999af" d="M0 0v11h10V0zm9 10H1V1h8zM4 4v1h2v1H4v2H3V3h4v1z"/></svg>)SVG" };
    VectorGraphic userIcon{
        R"SVG(<svg viewBox="0 0 10 11"><path fill="#9999af" d="M0 0v11h10V0zm9 10H1V1h8zM7 3v5H3V3h1v4h2V3z"/></svg>)SVG" };
    VectorGraphic saveIcon{
        R"SVG(<svg viewBox="0 0 15 15"><path fill="#a2a2cb" d="M15 0v15H0V0h3v6H1v8h13V6h-2V0zM9 8V0H6v8H5l3 3 3-3z"/></svg>)SVG" };

    std::function<Drawable* (const juce::File&)> getPresetIconFunction;
    std::function<void()> presetWasChangedFunction = []() {};

    class PresetNameTextEditor : public TextEditor {
        void mouseDown(const MouseEvent& event) override {
            if (event.eventComponent == this)
                return;

            setVisible(false);
        }
    };

    PresetNameTextEditor textEditor;

    void setPresetName(juce::String v) {
        presetNameText = v;
        presetName_button.repaint();
    }

    void setFloatBounds(Rectangle<float> v, float scale_) {
        scale = scale_;
        thickness = std::max(1.f, scale);
        setBounds(v.toNearestInt());
        presetDropdownSkin->setScale(scale);
    }

    juce::String getPresetNameText() { return presetNameText; }

    struct Colors {
        Colour border{ 69, 71, 97 };
        Colour hoverFill{ 64, 64, 92 };
        Colour font{ 211, 211, 213 };
        Colour normal{ 162, 162, 203 };
        Colour hover{ 181, 181, 213 };
        Colour clicked{ 122, 122, 152 };
    } colors;

    AudioModule* moduleToEdit = nullptr;
    PresetBarDropdownSkin* presetDropdownSkin = nullptr;
    PluginFileManager::FileMemory* fileManagerMemory = nullptr;
    StateLoadSaveWidgetSet* stateWidgetSet = nullptr;

protected:
    juce::String presetNameText = "click here for presets...";

    void paint(Graphics& g) override { g.fillAll({ 16, 15, 28 }); }

    void resized() override {
        auto area = getLocalBounds();

        left_button.setBounds(area.removeFromLeft(area.getHeight()));
        right_button.setBounds(area.removeFromLeft(area.getHeight()));
        save_button.setBounds(area.removeFromRight(area.getHeight()));
        presetName_button.setBounds(area.expanded((int)thickness, 0));

        textEditor.setBounds(presetName_button.getBounds());
    }

    float scale = 1;
    float thickness = 1;
};
