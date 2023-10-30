/********************************************************************************
 * File: main.cpp
 * Author: ppkantorski
 * Description: 
 *   This file contains the main program logic for the Ultrahand Overlay project,
 *   an overlay executor designed for versatile crafting and management of overlays.
 *   It defines various functions, menu structures, and interaction logic to
 *   facilitate the seamless execution and customization of overlays within the project.
 * 
 *   Key Features:
 *   - Dynamic overlay loading and execution.
 *   - Integration with menu systems and submenus.
 *   - Configuration options through INI files.
 *   - Toggles for enabling/disabling specific commands.
 * 
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 * 
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 *
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#define NDEBUG
#define STBTT_STATIC
#define TESLA_INIT_IMPL

#include <tesla.hpp>
#include <utils.hpp>


// Define external functions and variables
extern void logMessage(const std::string& message);
extern bool isFileOrDirectory(const std::string& path);
extern void createDirectory(const std::string& path);
extern std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& iniPath, bool ignoreComments);
extern std::map<std::string, std::map<std::string, std::string>> getParsedDataFromIniFile(const std::string& iniFilePath);
extern std::vector<std::string> getSubdirectories(const std::string& directoryPath);
extern std::string formatPriorityString(const std::string& priority, const int& desiredWidth);
extern void setIniFileValue(const std::string& iniFilePath, const std::string& section, const std::string& key, const std::string& value);
extern std::string getNameFromPath(const std::string& path);
extern std::string getParentDirNameFromPath(const std::string& path);
extern std::string dropExtension(const std::string& fileName);
extern std::string preprocessPath(const std::string& path);
extern std::vector<std::string> getFilesListByWildcards(const std::string& pathPattern);
//extern std::vector<std::vector<std::string>> getSourceReplacement(const std::vector<std::vector<std::string>> commands, const std::string& entry, size_t entryIndex);
//extern bool interpretAndExecuteCommand(std::vector<std::vector<std::string>>& commands, const std::string packagePath, const std::string keyName);



// Overlay booleans
//static bool shouldCloseMenu = false;
static bool returningToMain = false;
static bool returningToHiddenMain = false;
static bool returningToSettings = false;
static bool returningToPackage = false;
static bool returningToSubPackage = false;
static bool inMainMenu = false;
static bool inHiddenMode = false;
static bool inSettingsMenu = false;
static bool inSubSettingsMenu = false;
static bool inPackageMenu = false;
static bool inSubPackageMenu = false;
static bool inScriptMenu = false;
static bool inSelectionMenu = false;
static bool defaultMenuLoaded = true;
static bool freshSpawn = true;
static bool refreshGui = false;
static bool reloadMenu = false;
static bool reloadMenu2 = false;
static bool reloadMenu3 = false;
static bool isDownloaded = false;

static bool redrawWidget = false;
static bool showMenu = false;

static tsl::elm::OverlayFrame *rootFrame = nullptr;
static tsl::elm::List *list = nullptr;

// Command mode globals
static std::vector<std::string> commandModes = {"default", "toggle", "option"};
static std::vector<std::string> commandGroupings = {"default", "split"};
static std::string modePattern = ";mode=";
static std::string groupingPattern = ";grouping=";

static std::string lastMenu = "";
static std::string lastMenuMode = "";
static std::string lastKeyName = "";
static std::unordered_map<std::string, std::string> selectedFooterDict;
static auto selectedListItem = new tsl::elm::ListItem("");
static auto lastSelectedListItem = new tsl::elm::ListItem("");




// Command key defintitions
const static auto SCRIPT_KEY = KEY_MINUS;
const static auto SYSTEM_SETTINGS_KEY = KEY_PLUS;
const static auto SETTINGS_KEY = KEY_Y;
const static auto STAR_KEY = KEY_X;




// Forward declaration of the MainMenu class.
class MainMenu;

class UltrahandSettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, dropdownSelection, settingsIniPath;
    bool isInSection, inQuotes, isFromMainMenu;
    std::string languagesVersion = std::string(APP_VERSION);
    
    int MAX_PRIORITY = 20;
    
    std::vector<std::string> defaultCombos = {"ZL+ZR+DDOWN", "ZL+ZR+DRIGHT", "ZL+ZR+DUP", "ZL+ZR+DLEFT", "L+R+DDOWN", "L+R+DRIGHT", "L+R+DUP", "L+R+DLEFT", "L+DDOWN+RS"};
    std::unordered_map<std::string, std::string> comboMap = {
        {"ZL+ZR+DDOWN", "\uE0E6+\uE0E7+\uE0EC"},
        {"ZL+ZR+DRIGHT", "\uE0E6+\uE0E7+\uE0EE"},
        {"ZL+ZR+DUP", "\uE0E6+\uE0E7+\uE0EB"},
        {"ZL+ZR+DLEFT", "\uE0E6+\uE0E7+\uE0ED"},
        {"L+R+DDOWN", "\uE0E4+\uE0E5+\uE0EC"},
        {"L+R+DRIGHT", "\uE0E4+\uE0E5+\uE0EE"},
        {"L+R+DUP", "\uE0E4+\uE0E5+\uE0EB"},
        {"L+R+DLEFT", "\uE0E4+\uE0E5+\uE0ED"},
        {"L+DDOWN+RS", "\uE0E4+\uE0EC+\uE0C5"}
    };
    std::vector<std::string> defaultLanguages = {"en", "es", "fr", "de", "ja", "kr", "it", "nl", "pt", "ru", "zh-cn", "zh-tw"};
public:
    /**
     * @brief Constructs a `ScriptOverlay` instance.
     *
     * Initializes a new instance of the `ScriptOverlay` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     */
    UltrahandSettingsMenu(const std::string& selection = "") : dropdownSelection(selection) {}
    
    /**
     * @brief Destroys the `ScriptOverlay` instance.
     *
     * Cleans up any resources associated with the `ScriptOverlay` instance.
     */
    ~UltrahandSettingsMenu() {}
    
    /**
     * @brief Creates the graphical user interface (GUI) for the configuration overlay.
     *
     * This function initializes and sets up the GUI elements for the configuration overlay,
     * allowing users to modify settings in the INI file.
     *
     * @return A pointer to the GUI element representing the configuration overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        
        //rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel);
        
        if (dropdownSelection.empty())
            inSettingsMenu = true;
        else
            inSubSettingsMenu = true;
        
        
        list = new tsl::elm::List();
        
        
        if (dropdownSelection.empty()) {
            list->addItem(new tsl::elm::CategoryHeader(MAIN_SETTINGS));
            
            std::string fileContent = getFileContents(settingsConfigIniPath);
            
            std::string defaultLang = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_lang");
            std::string defaultMenu = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_menu");
            std::string keyCombo = trim(parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "key_combo"));
            
            
            if (defaultLang.empty())
                defaultLang = "en";
            if (defaultMenu.empty())
                defaultMenu = "packages";
            if (keyCombo.empty())
                keyCombo = "ZL+ZR+DDOWN";
            
            
            //auto toggleListItem = new tsl::elm::ToggleListItem("Default Menu", false, "Packages", OVERLAYS);
            //toggleListItem->setState((defaultMenu == "packages"));
            //toggleListItem->setStateChangedListener([this, toggleListItem](bool state) {
            //    setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", state ? "packages" : "overlays");
            //});
            //list->addItem(toggleListItem);
            
            
            //auto listItem = new tsl::elm::ListItem("Default Menu");
            //listItem->setValue(defaultMenu);
            //
            //// Envolke selectionOverlay in optionMode
            //
            //listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
            //    if (keys & KEY_A) {
            //        tsl::changeTo<UltrahandSettingsMenu>("defaultMenu");
            //        selectedListItem = listItem;
            //        return true;
            //    }
            //    return false;
            //});
            //list->addItem(listItem);
            
            auto listItem = new tsl::elm::ListItem(KEY_COMBO);
            listItem->setValue(comboMap[keyCombo]);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("keyComboMenu");
                    selectedListItem = listItem;
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            
            listItem = new tsl::elm::ListItem(LANGUAGE);
            listItem->setValue(defaultLang);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("languageMenu");
                    selectedListItem = listItem;
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            
            
            
            
            listItem = new tsl::elm::ListItem(SOFTWARE_UPDATE);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("softwareUpdateMenu");
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            
            
            
            list->addItem(new tsl::elm::CategoryHeader(UI_SETTINGS));
            
            
            
            listItem = new tsl::elm::ListItem(WIDGET);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("widgetMenu");
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            
            
            listItem = new tsl::elm::ListItem(VERSION_LABELS);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("versionLabelMenu");
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            
            
            
            
        } else if (dropdownSelection == "defaultMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader("Default Menu"));
            
            std::string defaultMenu = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_menu");
            
            std::vector<std::string> defaultMenuModes = {"overlays", "packages"};
            
            for (const auto& defaultMenuMode : defaultMenuModes) {
                
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(defaultMenuMode);
                
                if (defaultMenuMode == defaultMenu) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                }
                
                listItem->setClickListener([this, defaultMenuMode, listItem](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    if (keys & KEY_A) {
                        setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(defaultMenuMode);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItem;
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem);
            }
        
        
        
        } else if (dropdownSelection == "keyComboMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(KEY_COMBO));
            
            std::string defaultCombo = trim(parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "key_combo"));
            
            
            for (const auto& combo : defaultCombos) {
                
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(comboMap[combo]);
                
                if (combo == defaultCombo) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                }
                
                listItem->setClickListener([this, combo, defaultCombo, listItem](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    if (keys & KEY_A) {
                        if (combo != defaultCombo) {
                            setIniFileValue(settingsConfigIniPath, "ultrahand", "key_combo", combo);
                            reloadMenu = true;
                        }
                        
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(comboMap[combo]);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItem;
                        
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem);
            }
        
        } else if (dropdownSelection == "languageMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(LANGUAGE));
            
            std::string defaulLang = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_lang");
            
            
            
            for (const auto& defaultLangMode : defaultLanguages) {
                std::string langFile = "/config/ultrahand/lang/"+defaultLangMode+".json";
                bool skipLang = (!isFileOrDirectory(langFile));
                if (defaultLangMode != "en") {
                    if (skipLang)
                        continue;
                }
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(defaultLangMode);
                
                if (defaultLangMode == defaulLang) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                }
                
                listItem->setClickListener([this, skipLang, defaultLangMode, defaulLang, langFile, listItem](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    if (keys & KEY_A) {
                        //if (defaultLangMode != defaulLang) {
                        setIniFileValue(settingsConfigIniPath, "ultrahand", "default_lang", defaultLangMode);
                        reloadMenu = true;
                        reloadMenu2 = true;
                        
                        parseLanguage(langFile);
                        
                        if (skipLang)
                            reinitializeLangVars();
                        //}
                        
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(defaultLangMode);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItem;
                        
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem);
            }
        } else if (dropdownSelection == "softwareUpdateMenu") {
            list->addItem(new tsl::elm::CategoryHeader(SOFTWARE_UPDATE));
            
            auto listItem = new tsl::elm::ListItem(UPDATE_ULTRAHAND);
            
            // Envolke selectionOverlay in optionMode
            
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    deleteFileOrDirectory("/config/ultrahand/downloads/ovlmenu.ovl");
                    isDownloaded = downloadFile("https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/ovlmenu.ovl", "/config/ultrahand/downloads/");
                    if (isDownloaded) {
                        moveFileOrDirectory("/config/ultrahand/downloads/ovlmenu.ovl", "/switch/.overlays/ovlmenu.ovl");
                        listItem->setValue(CHECKMARK_SYMBOL);
                        languagesVersion = "latest";
                    } else
                        listItem->setValue(CROSSMARK_SYMBOL, false);
                    
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            https://github.com/ppkantorski/Ultrahand-Overlay/releases/download/v1.4.2/lang.zip
            listItem = new tsl::elm::ListItem(UPDATE_LANGUAGES);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    deleteFileOrDirectory("/config/ultrahand/downloads/ovlmenu.ovl");
                    if (languagesVersion == "latest")
                        isDownloaded = downloadFile("https://github.com/ppkantorski/Ultrahand-Overlay/releases/latest/download/lang.zip", "/config/ultrahand/downloads/");
                    else
                        isDownloaded = downloadFile("https://github.com/ppkantorski/Ultrahand-Overlay/releases/download/v"+languagesVersion+"/lang.zip", "/config/ultrahand/downloads/");
                    if (isDownloaded) {
                        unzipFile("/config/ultrahand/downloads/lang.zip", "/config/ultrahand/downloads/lang/");
                        deleteFileOrDirectory("/config/ultrahand/downloads/lang.zip");
                        deleteFileOrDirectory("/config/ultrahand/lang/");
                        moveFileOrDirectory("/config/ultrahand/downloads/lang/", "/config/ultrahand/lang/");
                        listItem->setValue(CHECKMARK_SYMBOL);
                    } else
                        listItem->setValue(CROSSMARK_SYMBOL, false);
                    
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            
            list->addItem(new tsl::elm::CategoryHeader(OVERLAY_INFO));
            
            
            constexpr int lineHeight = 20;  // Adjust the line height as needed
            constexpr int xOffset = 120;    // Adjust the horizontal offset as needed
            constexpr int fontSize = 16;    // Adjust the font size as needed
            int numEntries = 0;   // Adjust the number of entries as needed
            
            std::string packageSectionString = "";
            std::string packageInfoString = "";
            
            packageSectionString += TITLE+'\n';
            packageInfoString += std::string("Ultrahand Overlay")+'\n';
            numEntries++;
            
            packageSectionString += VERSION+'\n';
            packageInfoString += std::string(APP_VERSION)+'\n';
            numEntries++;
            
            packageSectionString += CREATOR+'\n';
            packageInfoString += "b0rd2dEAth\n";
            numEntries++;
            
            std::string aboutHeaderText = ABOUT+'\n';
            std::string::size_type aboutHeaderLength = aboutHeaderText.length();
            std::string aboutText = "Ultrahand Overlay is a versatile tool that enables you to create and share custom command-based packages.";
            
            packageSectionString += aboutHeaderText;
            
            // Split the about text into multiple lines with proper word wrapping
            constexpr int maxLineLength = 28;  // Adjust the maximum line length as needed
            std::string::size_type startPos = 0;
            std::string::size_type spacePos = 0;
            
            while (startPos < aboutText.length()) {
                std::string::size_type endPos = std::min(startPos + maxLineLength, aboutText.length());
                std::string line = aboutText.substr(startPos, endPos - startPos);
                
                // Check if the current line ends with a space; if not, find the last space in the line
                if (endPos < aboutText.length() && aboutText[endPos] != ' ') {
                    spacePos = line.find_last_of(' ');
                    if (spacePos != std::string::npos) {
                        endPos = startPos + spacePos;
                        line = aboutText.substr(startPos, endPos - startPos);
                    }
                }
                
                packageInfoString += line + '\n';
                startPos = endPos + 1;
                numEntries++;
                
                // Add corresponding newline to the packageSectionString
                if (startPos < aboutText.length()) {
                    packageSectionString += std::string(aboutHeaderLength, ' ') + '\n';
                }
            }
            
            
            std::string creditsHeaderText = CREDITS+'\n';
            std::string::size_type creditsHeaderLength = creditsHeaderText.length();
            std::string creditsText = "Special thanks to B3711, ComplexNarrative, Faker_dev, MasaGratoR, meha, WerWolv, HookedBehemoth and many others. <3";
            
            packageSectionString += creditsHeaderText;
            
            // Split the about text into multiple lines with proper word wrapping
            //constexpr int maxLineLength = 28;  // Adjust the maximum line length as needed
            startPos = 0;
            spacePos = 0;
            
            while (startPos < creditsText.length()) {
                std::string::size_type endPos = std::min(startPos + maxLineLength, creditsText.length());
                std::string line = creditsText.substr(startPos, endPos - startPos);
                
                // Check if the current line ends with a space; if not, find the last space in the line
                if (endPos < creditsText.length() && creditsText[endPos] != ' ') {
                    spacePos = line.find_last_of(' ');
                    if (spacePos != std::string::npos) {
                        endPos = startPos + spacePos;
                        line = creditsText.substr(startPos, endPos - startPos);
                    }
                }
                
                packageInfoString += line + '\n';
                startPos = endPos + 1;
                numEntries++;
                
                // Add corresponding newline to the packageSectionString
                if (startPos < aboutText.length()) {
                    packageSectionString += std::string(creditsHeaderLength, ' ') + '\n';
                }
            }
            
            
            // Remove trailing newline character
            if ((packageSectionString != "") && (packageSectionString.back() == '\n')) {
                packageSectionString = packageSectionString.substr(0, packageSectionString.size() - 1);
            }
            if ((packageInfoString != "") && (packageInfoString.back() == '\n')) {
                packageInfoString = packageInfoString.substr(0, packageInfoString.size() - 1);
            }
            
            
            if ((packageSectionString != "") && (packageInfoString != "")) {
                list->addItem(new tsl::elm::CustomDrawer([lineHeight, xOffset, fontSize, packageSectionString, packageInfoString](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
                    renderer->drawString(packageSectionString.c_str(), false, x, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
                    renderer->drawString(packageInfoString.c_str(), false, x + xOffset, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
                }), fontSize * numEntries + lineHeight);
            }
            
        } else if (dropdownSelection == "widgetMenu") {
            
            //std::string hideClock = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_clock");
            //std::string hideBattery = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_battery");
            //std::string hideSOCTemp = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_soc_temp");
            //std::string hidePCBTemp = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_soc_temp");
            
            
            list->addItem(new tsl::elm::CategoryHeader(WIDGET));
            
            auto toggleListItem = new tsl::elm::ToggleListItem(CLOCK, false, ON, OFF);
            toggleListItem->setState((hideClock == "false"));
            toggleListItem->setStateChangedListener([this, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_clock", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem);
            
            
            toggleListItem = new tsl::elm::ToggleListItem(BATTERY, false, ON, OFF);
            toggleListItem->setState((hideBattery == "false"));
            toggleListItem->setStateChangedListener([this, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_battery", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem);
            
            toggleListItem = new tsl::elm::ToggleListItem(SOC_TEMPERATURE, false, ON, OFF);
            toggleListItem->setState((hideSOCTemp == "false"));
            toggleListItem->setStateChangedListener([this, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_soc_temp", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem);
            
            toggleListItem = new tsl::elm::ToggleListItem(PCB_TEMPERATURE, false, ON, OFF);
            toggleListItem->setState((hidePCBTemp == "false"));
            toggleListItem->setStateChangedListener([this, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_pcb_temp", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem);
            
        } else if (dropdownSelection == "versionLabelMenu") {
            cleanVersionLabels = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "clean_version_labels");
            hideOverlayVersions = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_overlay_versions");
            hidePackageVersions = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_package_versions");
            
            if (cleanVersionLabels.empty())
                cleanVersionLabels = "false";
            if (hideOverlayVersions.empty())
                hideOverlayVersions = "false";
            if (hidePackageVersions.empty())
                hidePackageVersions = "false";
            
            list->addItem(new tsl::elm::CategoryHeader(VERSION_LABELS));
            
            std::string defaulLang = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_lang");
            
            
               
            auto toggleListItem = new tsl::elm::ToggleListItem(CLEAN_LABELS, false, ON, OFF);
            toggleListItem->setState((cleanVersionLabels == "true"));
            toggleListItem->setStateChangedListener([this, cleanVersionLabels, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "clean_version_labels", state ? "true" : "false");
                if ((cleanVersionLabels == "true") != state) {
                    if (cleanVersionLabels == "false")
                        versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" "+cleanVersionLabel(loaderInfo)+std::string(")"); // Still needs to parse nx-ovlloader instead of hard coding it
                    else
                        versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" v"+cleanVersionLabel(loaderInfo)+std::string(")");
                    //reloadMenu2 = true;
                    //reloadMenu = true;
                    reinitializeVersionLabels();
                }
                
            });
            list->addItem(toggleListItem);
            
            
            toggleListItem = new tsl::elm::ToggleListItem(OVERLAY_LABELS, false, ON, OFF);
            toggleListItem->setState((hideOverlayVersions == "false"));
            toggleListItem->setStateChangedListener([this, hideOverlayVersions, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_overlay_versions", state ? "false" : "true");
                if ((hideOverlayVersions == "false") != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem);
            
            toggleListItem = new tsl::elm::ToggleListItem(PACKAGE_LABELS, false, ON, OFF);
            toggleListItem->setState((hidePackageVersions == "false"));
            toggleListItem->setStateChangedListener([this, hidePackageVersions, toggleListItem](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_package_versions", state ? "false" : "true");
                if ((hidePackageVersions == "false") != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem);
            
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN + ": " + settingsIniPath));
        
        rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel);
        rootFrame->setContent(list);
        return rootFrame;
    }
    
    /**
     * @brief Handles user input for the configuration overlay.
     *
     * This function processes user input and responds accordingly within the configuration overlay.
     * It captures key presses and performs actions based on user interactions.
     *
     * @param keysDown   A bitset representing keys that are currently pressed.
     * @param keysHeld   A bitset representing keys that are held down.
     * @param touchInput Information about touchscreen input.
     * @param leftJoyStick Information about the left joystick input.
     * @param rightJoyStick Information about the right joystick input.
     * @return `true` if the input was handled within the overlay, `false` otherwise.
     */
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                if (reloadMenu3) {
                    tsl::goBack();
                    tsl::changeTo<UltrahandSettingsMenu>();
                    reloadMenu3 = false;
                }
                
                if (keysHeld & KEY_B) {
                    //tsl::Overlay::get()->close();
                    //svcSleepThread(300'000'000);
                    //tsl::goBack();
                    inSettingsMenu = false;
                    if (lastMenu != "hiddenMenuMode")
                        returningToMain = true;
                    else
                        returningToHiddenMain = true;
                    lastMenu = "settingsMenu";
                    
                    tsl::goBack();
                    
                    if (reloadMenu) {
                        tsl::changeTo<MainMenu>(lastMenuMode);
                        reloadMenu = false;
                    }
                    
                    //tsl::Overlay::get()->close();
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {
            if (keysHeld & KEY_B) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inSubSettingsMenu = false;
                returningToSettings = true;
                tsl::goBack();
                
                if (reloadMenu2) {
                    tsl::goBack();
                    tsl::changeTo<UltrahandSettingsMenu>();
                    reloadMenu2 = false;
                }
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        
        
        if (returningToSettings && !(keysHeld & KEY_B)){
            returningToSettings = false;
            inSettingsMenu = true;
        }
        
        if (redrawWidget) {
            reinitializeWidgetVars();
        }
        
        if (keysHeld & KEY_B)
            return false;
        
        return false;
        //return handleOverlayMenuInput(inScriptMenu, keysHeld, KEY_B);
    }
};



class SettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, dropdownSelection, settingsIniPath;
    bool isInSection, inQuotes, isFromMainMenu;
    int MAX_PRIORITY = 20;
public:
    /**
     * @brief Constructs a `ScriptOverlay` instance.
     *
     * Initializes a new instance of the `ScriptOverlay` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     */
    SettingsMenu(const std::string& name, const std::string& mode, const std::string& overlayName="", const std::string& selection = "") : entryName(name), entryMode(mode), overlayName(overlayName), dropdownSelection(selection) {}
    
    /**
     * @brief Destroys the `ScriptOverlay` instance.
     *
     * Cleans up any resources associated with the `ScriptOverlay` instance.
     */
    ~SettingsMenu() {}
    
    /**
     * @brief Creates the graphical user interface (GUI) for the configuration overlay.
     *
     * This function initializes and sets up the GUI elements for the configuration overlay,
     * allowing users to modify settings in the INI file.
     *
     * @return A pointer to the GUI element representing the configuration overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        std::string header = entryName;
        if (entryMode == "overlay") {
            settingsIniPath = overlaysIniFilePath;
            header = overlayName;
        } else if (entryMode == "package")
            settingsIniPath = packagesIniFilePath;
        
        if (dropdownSelection.empty())
            inSettingsMenu = true;
        else
            inSubSettingsMenu = true;
        
        
        list = new tsl::elm::List();
        
        
        
        if (dropdownSelection.empty()) {
            list->addItem(new tsl::elm::CategoryHeader(header+" "+SETTINGS));
            
            
            std::string fileContent = getFileContents(settingsIniPath);
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, "priority");
            
            std::string hideOption = parseValueFromIniSection(settingsIniPath, entryName, "hide");
            bool hide = false;
            
            std::string useOverlayLaunchArgs = parseValueFromIniSection(settingsIniPath, entryName, "use_launch_args");
            
            
            if (hideOption.empty())
                hideOption = "false";
            
            if (hideOption == "true")
                hide = true;
            
            
            //// Capitalize entryMode
            //std::string starLabel(entryMode);
            //starLabel[0] = std::toupper(starLabel[0]);
            //
            //// Envoke toggling
            //auto toggleListItem = new tsl::elm::ToggleListItem("Star "+starLabel, hide, ON, OFF);
            //toggleListItem->setStateChangedListener([this, toggleListItem](bool state) {
            //    if (!state) {
            //        setIniFileValue(settingsIniPath, entryName, "star", "true");
            //        toggleListItem->setState(!state);
            //    } else {
            //        setIniFileValue(settingsIniPath, entryName, "star", "false");
            //        toggleListItem->setState(!state);
            //    }
            //});
            //list->addItem(toggleListItem);
            
            
            
            // Capitalize entryMode
            std::string hideLabel(entryMode);
            //hideLabel[0] = std::toupper(hideLabel[0]);
            
            if (hideLabel == "overlay")
                hideLabel = HIDE_OVERLAY;
            else if (hideLabel == "package")
                hideLabel = HIDE_PACKAGE;
            
            
            
            // Envoke toggling
            auto toggleListItem = new tsl::elm::ToggleListItem(hideLabel, false, ON, OFF);
            toggleListItem->setState(hide);
            toggleListItem->setStateChangedListener([this, hide, toggleListItem](bool state) {
                setIniFileValue(settingsIniPath, entryName, "hide", state ? "true" : "false");
                if (hide != state)
                    reloadMenu = true; // this reloads before main menu
                if (!state)
                    reloadMenu2 = true; // this reloads at main menu
            });
            list->addItem(toggleListItem);
            
            
            
            auto listItem = new tsl::elm::ListItem(SORT_PRIORITY);
            listItem->setValue(priorityValue);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([this, listItem](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<SettingsMenu>(entryName, entryMode, overlayName, "priority");
                    selectedListItem = listItem;
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
            
            if (entryMode == "overlay") {
                // Envoke toggling
                toggleListItem = new tsl::elm::ToggleListItem(LAUNCH_ARGUMENTS, false, ON, OFF);
                toggleListItem->setState((useOverlayLaunchArgs=="true"));
                toggleListItem->setStateChangedListener([this, useOverlayLaunchArgs, toggleListItem](bool state) {
                    setIniFileValue(settingsIniPath, entryName, "use_launch_args", state ? "true" : "false");
                    if ((useOverlayLaunchArgs=="true") != state)
                        reloadMenu = true; // this reloads before main menu
                    if (!state)
                        reloadMenu2 = true; // this reloads at main menu
                });
                list->addItem(toggleListItem);
            }
            
            
        } else if (dropdownSelection == "priority") {
            list->addItem(new tsl::elm::CategoryHeader(SORT_PRIORITY));
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, "priority");
            
            for (int i = 0; i <= MAX_PRIORITY; ++i) { // for i in range 0->20 with 20 being the max value
                std::string iStr = std::to_string(i);
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(iStr);
                
                if (iStr == priorityValue) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                }
                
                listItem->setClickListener([this, iStr, priorityValue, listItem](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    if (keys & KEY_A) {
                        if (iStr != priorityValue)
                            reloadMenu = true;
                        setIniFileValue(settingsIniPath, entryName, "priority", iStr);
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(iStr);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItem;
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem);
            }
            
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN+": " + settingsIniPath));
        
        rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel);
        //rootFrame = new tsl::elm::OverlayFrame(entryName, "Ultrahand Settings");
        rootFrame->setContent(list);
        return rootFrame;
    }
    
    /**
     * @brief Handles user input for the configuration overlay.
     *
     * This function processes user input and responds accordingly within the configuration overlay.
     * It captures key presses and performs actions based on user interactions.
     *
     * @param keysDown   A bitset representing keys that are currently pressed.
     * @param keysHeld   A bitset representing keys that are held down.
     * @param touchInput Information about touchscreen input.
     * @param leftJoyStick Information about the left joystick input.
     * @param rightJoyStick Information about the right joystick input.
     * @return `true` if the input was handled within the overlay, `false` otherwise.
     */
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                if (keysHeld & KEY_B) {
                    //tsl::Overlay::get()->close();
                    //svcSleepThread(300'000'000);
                    //tsl::goBack();
                    inSettingsMenu = false;
                    if (lastMenu != "hiddenMenuMode")
                        returningToMain = true;
                    else
                        returningToHiddenMain = true;
                    
                    tsl::goBack();
                    
                    if (reloadMenu) {
                        if (lastMenu == "hiddenMenuMode") {
                            tsl::goBack();
                            inMainMenu = false;
                            inHiddenMode = true;
                        } else
                            reloadMenu = false;
                        tsl::changeTo<MainMenu>(lastMenuMode);
                    }
                    
                    lastMenu = "settingsMenu";
                    //tsl::Overlay::get()->close();
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {
            if (keysHeld & KEY_B) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inSubSettingsMenu = false;
                returningToSettings = true;
                tsl::goBack();
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        
        
        if (returningToSettings && !(keysHeld & KEY_B)){
            returningToSettings = false;
            inSettingsMenu = true;
        }
        
        
        if (keysHeld & KEY_B)
            return false;
        
        return false;
        //return handleOverlayMenuInput(inScriptMenu, keysHeld, KEY_B);
    }
};


/**
 * @brief The `ScriptOverlay` class handles configuration overlay functionality.
 *
 * This class manages the configuration overlay, allowing users to modify settings
 * in the INI file. It provides functions for creating, updating, and cleaning INI files
 * as well as handling user interactions related to configuration.
 */
class ScriptOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey;
    bool isInSection, inQuotes, isFromMainMenu;

public:
    /**
     * @brief Constructs a `ScriptOverlay` instance.
     *
     * Initializes a new instance of the `ScriptOverlay` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     */
    ScriptOverlay(const std::string& file, const std::string& key = "", const bool& fromMainMenu=false) : filePath(file), specificKey(key), isFromMainMenu(fromMainMenu) {}
    
    /**
     * @brief Destroys the `ScriptOverlay` instance.
     *
     * Cleans up any resources associated with the `ScriptOverlay` instance.
     */
    ~ScriptOverlay() {}
    
    /**
     * @brief Creates the graphical user interface (GUI) for the configuration overlay.
     *
     * This function initializes and sets up the GUI elements for the configuration overlay,
     * allowing users to modify settings in the INI file.
     *
     * @return A pointer to the GUI element representing the configuration overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        inScriptMenu = true;
        std::string packageName = getNameFromPath(filePath);
        if (packageName == ".packages")
            packageName = ROOT_PACKAGE;
        
        list = new tsl::elm::List();
        
        std::string packageFile = filePath + packageFileName;
        std::string fileContent = getFileContents(packageFile);
        
        if (!fileContent.empty()) {
            std::string line;
            std::istringstream iss(fileContent);
            std::string currentCategory;
            isInSection = false;
            while (std::getline(iss, line)) {
                if (line.empty() || line.find_first_not_of('\n') == std::string::npos)
                    continue;
                
                if (line.front() == '[' && line.back() == ']') {
                    if (!specificKey.empty()) {
                        if (line.substr(1, line.size() - 2) == specificKey) {
                            currentCategory = line.substr(1, line.size() - 2);
                            isInSection = true;
                            list->addItem(new tsl::elm::CategoryHeader(currentCategory));
                        } else {
                            currentCategory.clear();
                            isInSection = false;
                        }
                    } else {
                        currentCategory = line.substr(1, line.size() - 2);
                        isInSection = true;
                        list->addItem(new tsl::elm::CategoryHeader(currentCategory));
                    }
                } else if (isInSection) {
                    auto listItem = new tsl::elm::ListItem(line);
                    listItem->setClickListener([line, this, listItem](uint64_t keys) {
                        if (keys & KEY_A) {
                            std::istringstream iss(line);
                            std::string part;
                            std::vector<std::vector<std::string>> commandVec;
                            std::vector<std::string> commandParts;
                            inQuotes = false;
                            
                            while (std::getline(iss, part, '\'')) {
                                if (!part.empty()) {
                                    if (!inQuotes) {
                                        std::istringstream argIss(part);
                                        std::string arg;
                                        while (argIss >> arg) {
                                            commandParts.emplace_back(arg);
                                        }
                                    } else
                                        commandParts.emplace_back(part);
                                }
                                inQuotes = !inQuotes;
                            }
                            
                            commandVec.emplace_back(std::move(commandParts));
                            interpretAndExecuteCommand(commandVec, filePath, specificKey);
                            listItem->setValue(CHECKMARK_SYMBOL);
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                }
            }
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN+": " + packageFile));
        
        rootFrame = new tsl::elm::OverlayFrame(packageName, "Ultrahand Script");
        rootFrame->setContent(list);
        return rootFrame;
    }
    
    /**
     * @brief Handles user input for the configuration overlay.
     *
     * This function processes user input and responds accordingly within the configuration overlay.
     * It captures key presses and performs actions based on user interactions.
     *
     * @param keysDown   A bitset representing keys that are currently pressed.
     * @param keysHeld   A bitset representing keys that are held down.
     * @param touchInput Information about touchscreen input.
     * @param leftJoyStick Information about the left joystick input.
     * @param rightJoyStick Information about the right joystick input.
     * @return `true` if the input was handled within the overlay, `false` otherwise.
     */
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (inScriptMenu) {
            if (keysHeld & KEY_B) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inScriptMenu = false;
                if (isFromMainMenu == false) {
                    if (lastMenu == "packageMenu")
                        returningToPackage = true;
                    else if (lastMenu == "subPackageMenu")
                        returningToSubPackage = true;
                } else
                    returningToMain = true;
                tsl::goBack();
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        if (keysHeld & KEY_B)
            return false;
        return false;
        //return handleOverlayMenuInput(inScriptMenu, keysHeld, KEY_B);
    }
};



/**
 * @brief The `SelectionOverlay` class manages the selection overlay functionality.
 *
 * This class handles the selection overlay, allowing users to interact with and select various options.
 * It provides functions for creating the graphical user interface (GUI), handling user input, and executing commands.
 */
class SelectionOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
    std::vector<std::vector<std::string>> commands;
    std::string specifiedFooterKey;
    bool toggleState = false;
    std::string packageConfigIniPath;
    std::string commandMode, commandGrouping;
public:
    /**
     * @brief Constructs a `SelectionOverlay` instance.
     *
     * Initializes a new instance of the `SelectionOverlay` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     * @param cmds A vector of vectors containing commands for the overlay (optional).
     */
    SelectionOverlay(const std::string& path, const std::string& key = "", const std::vector<std::vector<std::string>>& cmds = {}, const std::string& footerKey = "")
        : filePath(path), specificKey(key), commands(cmds), specifiedFooterKey(footerKey) {
            lastSelectedListItem = new tsl::elm::ListItem("");
        }
    /**
     * @brief Destroys the `SelectionOverlay` instance.
     *
     * Cleans up any resources associated with the `SelectionOverlay` instance.
     */
    ~SelectionOverlay() {}
    
    /**
     * @brief Creates the graphical user interface (GUI) for the selection overlay.
     *
     * Initializes and sets up the GUI elements for the selection overlay, allowing users to interact
     * with and select various options.
     *
     * @return A pointer to the GUI element representing the selection overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        inSelectionMenu = true;
        PackageHeader packageHeader = getPackageHeaderFromIni(filePath+packageFileName);
        
        list = new tsl::elm::List();
        
        packageConfigIniPath = filePath + configFileName;
        
        commandMode = commandModes[0];
        commandGrouping = commandGroupings[0];
        
        std::string currentSection = "global";
        std::string sourceType = "default", sourceTypeOn = "default", sourceTypeOff = "default"; 
        std::string jsonPath, jsonPathOn, jsonPathOff;
        std::string jsonKey, jsonKeyOn, jsonKeyOff;
        
        
        std::vector<std::vector<std::string>> commandsOn;
        std::vector<std::vector<std::string>> commandsOff;
        std::string listString, listStringOn, listStringOff;
        std::vector<std::string> listData, listDataOn, listDataOff;
        std::string jsonString, jsonStringOn, jsonStringOff;
        json_t* jsonData = nullptr;
        json_t* jsonDataOn = nullptr;
        json_t* jsonDataOff = nullptr;
        
        
        // initial processing of commands
        for (const auto& cmd : commands) {
            
            if (!cmd.empty()) { // Isolate command settings
                // Extract the command mode
                if (cmd[0].find(modePattern) == 0) {
                    commandMode = cmd[0].substr(modePattern.length());
                    if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                        commandMode = commandModes[0]; // reset to default if commandMode is unknown
                } else if (cmd[0].find(groupingPattern) == 0) {// Extract the command grouping
                    commandGrouping = cmd[0].substr(groupingPattern.length());
                    if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                        commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                }
                
                // Extract the command grouping
                if (commandMode == "toggle") {
                    if (cmd[0].find("on:") == 0)
                        currentSection = "on";
                    else if (cmd[0].find("off:") == 0)
                        currentSection = "off";
                    
                    // Seperation of command chuncks
                    if (currentSection == "global") {
                        commandsOn.push_back(cmd);
                        commandsOff.push_back(cmd);
                    } else if (currentSection == "on")
                        commandsOn.push_back(cmd);
                    else if (currentSection == "off")
                        commandsOff.push_back(cmd);
                } else if (commandMode == "option") {
                    // 
                    
                }
                
            }
            if (cmd.size() > 1) { // Pre-process advanced commands
                if (cmd[0] == "filter") {
                    if (currentSection == "global")
                        filterList.push_back(cmd[1]);
                    else if (currentSection == "on")
                        filterListOn.push_back(cmd[1]);
                    else if (currentSection == "off")
                        filterListOff.push_back(cmd[1]);
                } else if (cmd[0] == "file_source") {
                    if (currentSection == "global") {
                        pathPattern = cmd[1];
                        filesList = getFilesListByWildcards(pathPattern);
                        sourceType = "file";
                    } else if (currentSection == "on") {
                        pathPatternOn = cmd[1];
                        filesListOn = getFilesListByWildcards(pathPatternOn);
                        sourceTypeOn = "file";
                    } else if (currentSection == "off") {
                        pathPatternOff = cmd[1];
                        filesListOff = getFilesListByWildcards(pathPatternOff);
                        sourceTypeOff = "file";
                    }
                } else if (cmd[0] == "json_file_source") {
                    if (currentSection == "global") {
                        jsonPath = preprocessPath(cmd[1]);
                        //jsonData = readJsonFromFile(jsonPath);
                        sourceType = "json_file";
                        if (cmd.size() > 2)
                            jsonKey = cmd[2]; //json display key
                    } else if (currentSection == "on") {
                        jsonPathOn = preprocessPath(cmd[1]);
                        //jsonDataOn = readJsonFromFile(jsonPathOn);
                        sourceTypeOn = "json_file";
                        if (cmd.size() > 2)
                            jsonKeyOn = cmd[2]; //json display key
                    } else if (currentSection == "off") {
                        jsonPathOff = preprocessPath(cmd[1]);
                        //jsonDataOff = readJsonFromFile(jsonPathOff);
                        sourceTypeOff = "json_file";
                        if (cmd.size() > 2)
                            jsonKeyOff = cmd[2]; //json display key
                    }
                } else if (cmd[0] == "list_source") {
                    if (currentSection == "global") {
                        listString = removeQuotes(cmd[1]);
                        //listData = stringToList(removeQuotes(cmd[1]));
                        sourceType = "list";
                    } else if (currentSection == "on") {
                        listStringOn = removeQuotes(cmd[1]);
                        //listDataOn = stringToList(removeQuotes(cmd[1]));
                        sourceTypeOn = "list";
                    } else if (currentSection == "off") {
                        listStringOff = removeQuotes(cmd[1]);
                        //listDataOff = stringToList(removeQuotes(cmd[1]));
                        sourceTypeOff = "list";
                    }
                } else if (cmd[0] == "json_source") {
                    if (currentSection == "global") {
                        jsonString = removeQuotes(cmd[1]); // convert string to jsonData
                        //jsonData = stringToJson(cmd[1]); // convert string to jsonData
                        sourceType = "json";
                        
                        if (cmd.size() > 2)
                            jsonKey = cmd[2]; //json display key
                    } else if (currentSection == "on") {
                        jsonStringOn = removeQuotes(cmd[1]); // convert string to jsonData
                        //jsonDataOn = stringToJson(cmd[1]); // convert string to jsonData
                        sourceTypeOn = "json";
                        
                        if (cmd.size() > 2)
                            jsonKeyOn = cmd[2]; //json display key
                        
                    } else if (currentSection == "off") {
                        jsonStringOff = removeQuotes(cmd[1]); // convert string to jsonData
                        //jsonDataOff = stringToJson(cmd[1]); // convert string to jsonData
                        sourceTypeOff = "json";
                        
                        if (cmd.size() > 2)
                            jsonKeyOff = cmd[2]; //json display key
                    }
                }
            } 
        }
        
        // items can be paths, commands, or variables depending on source
        std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;
        
        // Get the list of files matching the pattern
        if (commandMode == "default" || commandMode == "option") {
            if (sourceType == "file")
                selectedItemsList = filesList;
            else if (sourceType == "list")
                selectedItemsList = stringToList(listString);
            else if ((sourceType == "json") || (sourceType == "json_file")) {
                if (sourceType == "json")
                    jsonData = stringToJson(jsonString);
                else if (sourceType == "json_file")
                    jsonData = readJsonFromFile(jsonPath);
                
                // Populate items list based upon jsonKey
                if ((jsonData) && json_is_array(jsonData)) {
                    size_t arraySize = json_array_size(jsonData);
                    for (size_t i = 0; i < arraySize; ++i) {
                        json_t* item = json_array_get(jsonData, i);
                        if (item && json_is_object(item)) {
                            json_t* keyValue = json_object_get(item, jsonKey.c_str());
                            if (keyValue && json_is_string(keyValue)) {
                                const char* name = json_string_value(keyValue);
                                selectedItemsList.push_back(std::string(name));
                            }
                        }
                    }
                }
                // Free jsonDataOn
                if (jsonData != nullptr) {
                    json_decref(jsonData);
                    jsonData = nullptr;
                }
            }
        } else if (commandMode == "toggle") {
            if (sourceTypeOn == "file")
                selectedItemsListOn = filesListOn;
            else if (sourceTypeOn == "list")
                selectedItemsListOn = stringToList(listStringOn);
            else if ((sourceTypeOn == "json") || (sourceTypeOn == "json_file")) {
                if (sourceTypeOn == "json")
                    jsonDataOn = stringToJson(jsonStringOn);
                else if (sourceTypeOn == "json_file")
                    jsonDataOn = readJsonFromFile(jsonPathOn);
                
                
                // Populate items list based upon jsonKey
                if ((jsonDataOn) && json_is_array(jsonDataOn)) {
                    size_t arraySize = json_array_size(jsonDataOn);
                    for (size_t i = 0; i < arraySize; ++i) {
                        json_t* item = json_array_get(jsonDataOn, i);
                        if (item && json_is_object(item)) {
                            json_t* keyValue = json_object_get(item, jsonKeyOn.c_str());
                            if (keyValue && json_is_string(keyValue)) {
                                const char* name = json_string_value(keyValue);
                                selectedItemsListOn.push_back(std::string(name));
                            }
                        }
                    }
                }
                // Free jsonDataOn
                if (jsonDataOn != nullptr) {
                    json_decref(jsonDataOn);
                    jsonDataOn = nullptr;
                }
            }
            
            if (sourceTypeOff == "file")
                selectedItemsListOff = filesListOff;
            else if (sourceTypeOff == "list")
                selectedItemsListOff = stringToList(listStringOff);
            else if ((sourceTypeOff == "json") || (sourceTypeOff == "json_file")) {
                if (sourceTypeOff == "json")
                    jsonDataOff = stringToJson(jsonStringOff);
                else if (sourceTypeOff == "json_file")
                    jsonDataOff = readJsonFromFile(jsonPathOff);
                
                
                // Populate items list based upon jsonKey
                if ((jsonDataOff) && json_is_array(jsonDataOff)) {
                    size_t arraySize = json_array_size(jsonDataOff);
                    for (size_t i = 0; i < arraySize; ++i) {
                        json_t* item = json_array_get(jsonDataOff, i);
                        if (item && json_is_object(item)) {
                            json_t* keyValue = json_object_get(item, jsonKeyOff.c_str());
                            if (keyValue && json_is_string(keyValue)) {
                                const char* name = json_string_value(keyValue);
                                selectedItemsListOff.push_back(std::string(name));
                            }
                        }
                    }
                }
                // Free jsonDataOff
                if (jsonDataOff != nullptr) {
                    json_decref(jsonDataOff);
                    jsonDataOff = nullptr;
                }
            }
            
            
            // Apply On Filter
            filterItemsList(filterListOn, selectedItemsListOn);
            
            // Apply Off Filter
            filterItemsList(filterListOff, selectedItemsListOff);
            
            
            selectedItemsList.reserve(selectedItemsListOn.size() + selectedItemsListOff.size());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOn.begin(), selectedItemsListOn.end());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOff.begin(), selectedItemsListOff.end());
            
            
            // WARNING: This assumes items list is a path list. (May need a long term solution still.)
            if (commandGrouping == "split") {
                
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    std::string parentDirA = getParentDirNameFromPath(a);
                    std::string parentDirB = getParentDirNameFromPath(b);
                    
                    // Compare parent directory names
                    if (parentDirA != parentDirB)
                        return parentDirA < parentDirB;
                    else {
                        // Parent directory names are the same, compare filenames
                        std::string filenameA = getNameFromPath(a);
                        std::string filenameB = getNameFromPath(b);
                        
                        // Compare filenames
                        return filenameA < filenameB;
                    }
                });
            } else {
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    return getNameFromPath(a) < getNameFromPath(b);
                });
            }
        }
        
        // Apply filter to selectedItemsList
        filterItemsList(filterList, selectedItemsList);
        
        
        if (commandGrouping == "default")
            list->addItem(new tsl::elm::CategoryHeader(removeTag(specificKey.substr(1)))); // remove * from key
        
        
        // Add each file as a menu item
        for (size_t i = 0; i < selectedItemsList.size(); ++i) {
            const std::string& selectedItem = selectedItemsList[i];
            
            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, selectedItem);
            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, std::to_string(i));
            
            // For entries that are paths
            itemName = getNameFromPath(selectedItem);
            if (!isDirectory(preprocessPath(selectedItem)))
                itemName = dropExtension(itemName);
            
            parentDirName = getParentDirNameFromPath(selectedItem);
            
            if ((commandGrouping == "split") && (lastParentDirName.empty() || (lastParentDirName != parentDirName))){
                list->addItem(new tsl::elm::CategoryHeader(removeQuotes(parentDirName)));
                lastParentDirName = parentDirName.c_str();
            }
            
            
            if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                size_t pos = selectedItem.find(" - ");
                std::string footer = "";
                std::string optionName = selectedItem;
                if (pos != std::string::npos) {
                    footer = selectedItem.substr(pos + 2); // Assign the part after "&&" as the footer
                    optionName = selectedItem.substr(0, pos); // Strip the "&&" and everything after it
                }
                auto listItem = new tsl::elm::ListItem(optionName);
                
                if (commandMode == "option") {
                    if (selectedFooterDict[specifiedFooterKey] == selectedItem) { // needs to be fixed
                        lastSelectedListItem = listItem;
                        listItem->setValue(CHECKMARK_SYMBOL);
                    } else
                        listItem->setValue(footer);
                } else
                    listItem->setValue(footer, true);
                
                //if ((commandMode == "option") && selectedFooterDict[specificKey] == selectedItem) {
                //    lastSelectedListItem = listItem;
                //    listItem->setValue(CHECKMARK_SYMBOL);
                //} else {
                //    if (commandMode == "option") {
                //        listItem->setValue(footer);
                //    } else {
                //        listItem->setValue(footer, true);
                //    }
                //    
                //}
                
                //listItem->setValue(footer, true);
                
                
                if (sourceType == "json") { // For JSON wildcards
                    listItem->setClickListener([this, i, optionName, cmds=commands, footer, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                        if (keys & KEY_A) {
                            if (commandMode == "option") {
                                selectedFooterDict[specifiedFooterKey] = selectedItem;
                                lastSelectedListItem->setValue(footer, true);
                            }
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem, i); // replace source
                            //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                            refreshGui = interpretAndExecuteCommand(modifiedCmds, filePath, specificKey); // Execute modified 
                            
                            listItem->setValue(CHECKMARK_SYMBOL);
                            
                            if (commandMode == "option")
                                lastSelectedListItem = listItem;
                            
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                } else {
                    listItem->setClickListener([this, i, optionName, cmds=commands, footer, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                        if (keys & KEY_A) {
                            if (commandMode == "option") {
                                selectedFooterDict[specifiedFooterKey] = selectedItem;
                                lastSelectedListItem->setValue(footer, true);
                            }
                            
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem, i); // replace source
                            //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                            refreshGui = interpretAndExecuteCommand(modifiedCmds, filePath, specificKey); // Execute modified 
                            
                            listItem->setValue(CHECKMARK_SYMBOL);
                            
                            if (commandMode == "option")
                                lastSelectedListItem = listItem;
                            
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                }
            } else if (commandMode == "toggle") {
                auto toggleListItem = new tsl::elm::ToggleListItem(itemName, false, ON, OFF);
                
                // Set the initial state of the toggle item
                bool toggleStateOn = std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end();
                toggleListItem->setState(toggleStateOn);
                
                toggleListItem->setStateChangedListener([this, i, cmdsOn=commandsOn, cmdsOff=commandsOff, selectedItem, selectedItemsListOn, selectedItemsListOff, toggleListItem](bool state) {
                    if (!state) {
                        if (std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end()) {
                            // Toggle switched to On
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOn, selectedItem, i); // replace source
                            refreshGui = interpretAndExecuteCommand(modifiedCmds, filePath, specificKey); // Execute modified 
                        } else
                            toggleListItem->setState(!state);
                    } else {
                        if (std::find(selectedItemsListOff.begin(), selectedItemsListOff.end(), selectedItem) != selectedItemsListOff.end()) {
                            // Toggle switched to Off
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOff, selectedItem, i); // replace source
                            refreshGui = interpretAndExecuteCommand(modifiedCmds, filePath, specificKey); // Execute modified 
                        } else
                            toggleListItem->setState(!state);
                    }
                });
                list->addItem(toggleListItem);
            }
            //count++;
        }
        
        rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(filePath), "Ultrahand Package", "", packageHeader.color);
        rootFrame->setContent(list);
        
        return rootFrame;
    }
    
    /**
     * @brief Handles user input for the selection overlay.
     *
     * Processes user input and responds accordingly within the selection overlay.
     * Captures key presses and performs actions based on user interactions.
     *
     * @param keysDown A bitset representing keys that are currently pressed.
     * @param keysHeld A bitset representing keys that are held down.
     * @param touchInput Information about touchscreen input.
     * @param leftJoyStick Information about the left joystick input.
     * @param rightJoyStick Information about the right joystick input.
     * @return `true` if the input was handled within the overlay, `false` otherwise.
     */
    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (refreshGui) {
            tsl::changeTo<SelectionOverlay>(filePath, specificKey, commands, specifiedFooterKey);
            refreshGui = false;
        }
        
        if (inSelectionMenu) {
            if (keysHeld & KEY_B) {
                inSelectionMenu = false;
                
                if (lastMenu == "packageMenu")
                    returningToPackage = true;
                else if (lastMenu == "subPackageMenu")
                    returningToSubPackage = true;
                
                if (commandMode == "option") {
                    if (isFileOrDirectory(packageConfigIniPath)) {
                        auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                        if (packageConfigData.count(specificKey) > 0) {
                            auto& optionSection = packageConfigData[specificKey];
                            if (optionSection.count("footer") > 0) {
                                auto& commandFooter = optionSection["footer"];
                                if (commandFooter != "null")
                                    selectedListItem->setValue(commandFooter);
                            }
                        }
                    }
                }
                
                tsl::goBack();
                return true;
            }
        } 
        if (keysHeld & KEY_B)
            return false;
        
        return false;
    }
};


/**
 * @brief The `PackageMenu` class handles sub-menu overlay functionality.
 *
 * This class manages sub-menu overlays, allowing users to interact with specific menu options.
 * It provides functions for creating, updating, and navigating sub-menus, as well as handling user interactions related to sub-menu items.
 */
class PackageMenu : public tsl::Gui {
private:
    tsl::hlp::ini::IniData packageConfigData;
    std::string packagePath, dropdownSection, currentPage, pathReplace, pathReplaceOn, pathReplaceOff;
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
    bool usingPages = false;
public:
    /**
     * @brief Constructs a `PackageMenu` instance for a specific sub-menu path.
     *
     * Initializes a new instance of the `PackageMenu` class for the given sub-menu path.
     *
     * @param path The path to the sub-menu.
     */
    PackageMenu(const std::string& path, const std::string& sectionName = "", const std::string& page = "left", const std::string& mode = "package") : packagePath(path), dropdownSection(sectionName), currentPage(page) {}
    /**
     * @brief Destroys the `PackageMenu` instance.
     *
     * Cleans up any resources associated with the `PackageMenu` instance.
     */
    ~PackageMenu() {
        //if (inPackageMenu) {
        //    selectedFooterDict.clear(); // Clears all data from the map, making it empty again
        //}
    }
    
    /**
     * @brief Creates the graphical user interface (GUI) for the sub-menu overlay.
     *
     * This function initializes and sets up the GUI elements for the sub-menu overlay,
     * allowing users to interact with specific menu options.
     *
     * @return A pointer to the GUI element representing the sub-menu overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        if (dropdownSection.empty()){
            inPackageMenu = true;
            lastMenu = "packageMenu";
        } else {
            inSubPackageMenu = true;
            lastMenu = "subPackageMenu";
        }
        
        
        // Load options from INI file in the subdirectory
        std::string packageIniPath = packagePath + packageFileName;
        std::string packageConfigIniPath = packagePath + configFileName;
        PackageHeader packageHeader = getPackageHeaderFromIni(packageIniPath);
        
        //rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color);
        list = new tsl::elm::List();
        auto listItem = static_cast<tsl::elm::ListItem*>(nullptr);
        
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath);
        
        
        
        bool skipSection = false;
        // Populate the sub menu with options
        //for (const auto& option : options) {
        std::string lastSection = "";
        std::string pageLeftName = "";
        std::string pageRightName = "";
        std::string drawLocation = "";
        
        
        for (size_t i = 0; i < options.size(); ++i) {
            auto& option = options[i];
            
            std::string optionName = option.first;
            auto commands = option.second;
            
            std::string footer; 
            bool useSelection = false;
            
            
            std::string commandFooter = "null";
            std::string commandMode = "default";
            std::string commandGrouping = "default";
            
            std::string currentSection = "global";
            std::string sourceType = "default", sourceTypeOn = "default", sourceTypeOff = "default"; //"file", "json_file", "json", "list"
            //std::string sourceType, sourceTypeOn, sourceTypeOff; //"file", "json_file", "json", "list"
            std::string jsonPath, jsonPathOn, jsonPathOff;
            std::string jsonKey, jsonKeyOn, jsonKeyOff;
            
            
            std::vector<std::vector<std::string>> commandsOn;
            std::vector<std::vector<std::string>> commandsOff;
            std::vector<std::string> listData, listDataOn, listDataOff;
            
            // items can be paths, commands, or variables depending on source
            //std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;
            
            if (drawLocation.empty() || currentPage == drawLocation || (optionName[0] == '@')) {
                
                // Custom header implementation
                if (!dropdownSection.empty()) {
                    if (i == 0) {
                        // Add a section break with small text to indicate the "Commands" section
                        //if (dropdownSection[0] == '*') {
                        //    dropdownSection = dropdownSection.substr(1);
                        //}
                        list->addItem(new tsl::elm::CategoryHeader(removeTag(dropdownSection.substr(1))));
                        skipSection = true;
                        lastSection = dropdownSection;
                    }
                    if (commands.size() == 0) {
                        if (optionName == dropdownSection)
                            skipSection = false;
                        else
                            skipSection = true;
                        continue;
                    }
                } else {
                    if (commands.size() == 0) {
                        if (optionName[0] == '@') {
                            if (drawLocation.empty()) {
                                pageLeftName = optionName.substr(1);
                                drawLocation = "left";
                            } else {
                                pageRightName = optionName.substr(1);
                                usingPages = true;
                                drawLocation = "right";
                            }
                        } else if (optionName[0] == '*') {
                            // Create reference to PackageMenu with dropdownSection set to optionName
                            listItem = new tsl::elm::ListItem(removeTag(optionName.substr(1)), DROPDOWN_SYMBOL);
                            
                            listItem->setClickListener([this, optionName](s64 key) {
                                if (key & KEY_A) {
                                    inPackageMenu = false;
                                    tsl::changeTo<PackageMenu>(packagePath, optionName);
                                    return true;
                                }
                                return false;
                            });
                            list->addItem(listItem);
                            
                            
                            skipSection = true;
                        } else {
                            if (optionName != lastSection) {
                                
                                if (removeTag(optionName) == PACKAGE_INFO || removeTag(optionName) == "Package Info") {
                                    // Add a section break with small text to indicate the "Commands" section
                                    list->addItem(new tsl::elm::CategoryHeader(PACKAGE_INFO));
                                    lastSection = optionName;
                                    
                                    
                                    constexpr int lineHeight = 20;  // Adjust the line height as needed
                                    constexpr int xOffset = 120;    // Adjust the horizontal offset as needed
                                    constexpr int fontSize = 16;    // Adjust the font size as needed
                                    int numEntries = 0;   // Adjust the number of entries as needed
                                    
                                    std::string packageSectionString = "";
                                    std::string packageInfoString = "";
                                    if (packageHeader.version != "") {
                                        packageSectionString += VERSION+"\n";
                                        packageInfoString += (packageHeader.version+"\n").c_str();
                                        numEntries++;
                                    }
                                    if (packageHeader.creator != "") {
                                        packageSectionString += CREATOR+"\n";
                                        packageInfoString += (packageHeader.creator+"\n").c_str();
                                        numEntries++;
                                    }
                                    if (packageHeader.about != "") {
                                        std::string aboutHeaderText = ABOUT+"\n";
                                        std::string::size_type aboutHeaderLength = aboutHeaderText.length();
                                        std::string aboutText = packageHeader.about;
                                        
                                        packageSectionString += aboutHeaderText;
                                        
                                        // Split the about text into multiple lines with proper word wrapping
                                        constexpr int maxLineLength = 28;  // Adjust the maximum line length as needed
                                        std::string::size_type startPos = 0;
                                        std::string::size_type spacePos = 0;
                                        
                                        while (startPos < aboutText.length()) {
                                            std::string::size_type endPos = std::min(startPos + maxLineLength, aboutText.length());
                                            std::string line = aboutText.substr(startPos, endPos - startPos);
                                            
                                            // Check if the current line ends with a space; if not, find the last space in the line
                                            if (endPos < aboutText.length() && aboutText[endPos] != ' ') {
                                                spacePos = line.find_last_of(' ');
                                                if (spacePos != std::string::npos) {
                                                    endPos = startPos + spacePos;
                                                    line = aboutText.substr(startPos, endPos - startPos);
                                                }
                                            }
                                            
                                            packageInfoString += line + '\n';
                                            startPos = endPos + 1;
                                            numEntries++;
                                            
                                            // Add corresponding newline to the packageSectionString
                                            if (startPos < aboutText.length())
                                                packageSectionString += std::string(aboutHeaderLength, ' ') + '\n';
                                        }
                                    }
                                    
                                    
                                    // Remove trailing newline character
                                    if ((packageSectionString != "") && (packageSectionString.back() == '\n'))
                                        packageSectionString = packageSectionString.substr(0, packageSectionString.size() - 1);
                                    if ((packageInfoString != "") && (packageInfoString.back() == '\n'))
                                        packageInfoString = packageInfoString.substr(0, packageInfoString.size() - 1);
                                    
                                    
                                    if ((packageSectionString != "") && (packageInfoString != "")) {
                                        list->addItem(new tsl::elm::CustomDrawer([lineHeight, xOffset, fontSize, packageSectionString, packageInfoString](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
                                            renderer->drawString(packageSectionString.c_str(), false, x, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
                                            renderer->drawString(packageInfoString.c_str(), false, x + xOffset, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
                                        }), fontSize * numEntries + lineHeight);
                                    }
                                } else {
                                    // Add a section break with small text to indicate the "Commands" section
                                    list->addItem(new tsl::elm::CategoryHeader(removeTag(optionName)));
                                    lastSection = optionName;
                                }
                            }
                            skipSection = false;
                        }
                        
                        
                        continue;
                    } else if (i == 0) {
                        // Add a section break with small text to indicate the "Commands" section
                        list->addItem(new tsl::elm::CategoryHeader(COMMANDS));
                        skipSection = false;
                        lastSection = "Commands";
                    }
                }
                
                //if (optionName == "Section 1") {
                //    for (const auto& cmd : commands) {
                //        for (const auto& x : cmd) {
                //            logMessage("Commands: "+x);
                //        }
                //    }
                //}
                
                // initial processing of commands
                for (const auto& cmd : commands) {
                    
                    if (!cmd.empty()) { // Isolate command settings
                        // Extract the command mode
                        if (cmd[0].find(modePattern) == 0) {
                            commandMode = cmd[0].substr(modePattern.length());
                            if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                commandMode = commandModes[0]; // reset to default if commandMode is unknown
                        } else if (cmd[0].find(groupingPattern) == 0) {// Extract the command grouping
                            commandGrouping = cmd[0].substr(groupingPattern.length());
                            if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                        }
                        
                        // Extract the command grouping
                        if (commandMode == "toggle") {
                            if (cmd[0].find("on:") == 0)
                                currentSection = "on";
                            else if (cmd[0].find("off:") == 0)
                                currentSection = "off";
                            
                            // Seperation of command chuncks
                            if (currentSection == "global") {
                                commandsOn.push_back(cmd);
                                commandsOff.push_back(cmd);
                            } else if (currentSection == "on")
                                commandsOn.push_back(cmd);
                            else if (currentSection == "off")
                                commandsOff.push_back(cmd);
                        }
                        
                    }
                    if (cmd.size() > 1) { // Pre-process advanced commands
                        //if (cmd[0] == "filter") {
                        //    if (currentSection == "global") {
                        //        filterList.push_back(cmd[1]);
                        //    } else if (currentSection == "on") {
                        //        filterListOn.push_back(cmd[1]);
                        //    } else if (currentSection == "off") {
                        //        filterListOff.push_back(cmd[1]);
                        //    }
                        if (cmd[0] == "file_source") {
                            if (currentSection == "global") {
                                pathPattern = cmd[1];
                                //filesList = getFilesListByWildcards(pathPattern);
                                sourceType = "file";
                            } else if (currentSection == "on") {
                                pathPatternOn = cmd[1];
                                //filesListOn = getFilesListByWildcards(pathPatternOn);
                                sourceTypeOn = "file";
                            } else if (currentSection == "off") {
                                pathPatternOff = cmd[1];
                                //filesListOff = getFilesListByWildcards(pathPatternOff);
                                sourceTypeOff = "file";
                            }
                        }
                    }
                }
                
                if (isFileOrDirectory(packageConfigIniPath)) {
                    packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    
                    
                    if (packageConfigData.count(optionName) > 0) {
                        auto& optionSection = packageConfigData[optionName];
                        
                        // For hiding the versions of overlays/packages
                        if (optionSection.count("mode") > 0)
                            commandMode = optionSection["mode"];
                        else
                            setIniFileValue(packageConfigIniPath, optionName, "mode", commandMode);
                        
                        if (optionSection.count("grouping") > 0)
                            commandGrouping = optionSection["grouping"];
                        else
                            setIniFileValue(packageConfigIniPath, optionName, "grouping", commandGrouping);
                        
                        if (optionSection.count("footer") > 0)
                            commandFooter = optionSection["footer"];
                        else
                            setIniFileValue(packageConfigIniPath, optionName, "footer", commandFooter);
                        
                    }
                } else { // write data if settings are not loaded
                    setIniFileValue(packageConfigIniPath, optionName, "mode", commandMode);
                    setIniFileValue(packageConfigIniPath, optionName, "grouping", commandGrouping);
                    setIniFileValue(packageConfigIniPath, optionName, "footer", "null");
                }
                
                
                
                // Get Option name and footer
                if (optionName[0] == '*') { 
                    useSelection = true;
                    optionName = optionName.substr(1); // Strip the "*" character on the left
                    footer = DROPDOWN_SYMBOL;
                } else {
                    size_t pos = optionName.find(" - ");
                    if (pos != std::string::npos) {
                        footer = optionName.substr(pos + 2); // Assign the part after "&&" as the footer
                        optionName = optionName.substr(0, pos); // Strip the "&&" and everything after it
                    }
                }
                
                if (commandMode == "option") {
                    // override loading of the command footer
                    if (commandFooter != "null")
                        footer = commandFooter;
                    else
                        footer = OPTION_SYMBOL;
                }
                
                if (skipSection == false) { // for skipping the drawing of sections
                    if (useSelection) { // For wildcard commands (dropdown menus)
                        
                        if ((footer == DROPDOWN_SYMBOL) || (footer.empty()))
                            listItem = new tsl::elm::ListItem(removeTag(optionName), footer);
                        else {
                            listItem = new tsl::elm::ListItem(removeTag(optionName));
                            if (commandMode == "option")
                                listItem->setValue(footer);
                            else
                                listItem->setValue(footer, true);
                            
                        }
                        
                        if (footer == UNAVAILABLE_SELECTION)
                            listItem->setValue(footer, true);
                        
                        //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                        listItem->setClickListener([cmds = commands, keyName = option.first, this, packagePath = this->packagePath, footer, lastSection, listItem](uint64_t keys) {
                            if ((keys & KEY_A) && (footer != UNAVAILABLE_SELECTION)) {
                                if (inPackageMenu)
                                    inPackageMenu = false;
                                if (inSubPackageMenu)
                                    inSubPackageMenu = false;
                                
                                selectedListItem = listItem;
                                
                                std::string newKey = "";
                                if (inPackageMenu) {
                                    newKey = lastSection + keyName;
                                    if (selectedFooterDict.find(newKey) == selectedFooterDict.end())
                                        selectedFooterDict[newKey] = footer;
                                } else {
                                    newKey = "sub_" + lastSection + keyName;
                                    if (selectedFooterDict.find(newKey) == selectedFooterDict.end())
                                        selectedFooterDict[newKey] = footer;
                                }
                                tsl::changeTo<SelectionOverlay>(packagePath, keyName, cmds, newKey);
                                lastKeyName = keyName;
                                
                                return true;
                            } else if (keys & SCRIPT_KEY) {
                                if (inPackageMenu)
                                    inPackageMenu = false;
                                if (inSubPackageMenu)
                                    inSubPackageMenu = false;
                                tsl::changeTo<ScriptOverlay>(packagePath, keyName);
                                return true;
                            }
                            return false;
                        });
                        
                        list->addItem(listItem);
                    } else { // For everything else
                        
                        const std::string& selectedItem = optionName;
                        
                        // For entries that are paths
                        itemName = getNameFromPath(selectedItem);
                        if (!isDirectory(preprocessPath(selectedItem)))
                            itemName = dropExtension(itemName);
                        parentDirName = getParentDirNameFromPath(selectedItem);
                        
                        
                        if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                            auto listItem = new tsl::elm::ListItem(removeTag(optionName));
                            if (commandMode == "default")
                                listItem->setValue(footer, true);
                            else
                                listItem->setValue(footer);
                            
                            
                            if (sourceType == "json") { // For JSON wildcards
                                listItem->setClickListener([this, i, cmds=commands, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                    if (keys & KEY_A) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, keyName, i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                        listItem->setValue(CHECKMARK_SYMBOL);
                                        return true;
                                    }  else if (keys & SCRIPT_KEY) {
                                        if (inPackageMenu)
                                            inPackageMenu = false;
                                        if (inSubPackageMenu)
                                            inSubPackageMenu = false;
                                        tsl::changeTo<ScriptOverlay>(packagePath, keyName);
                                        return true;
                                    }
                                    
                                    return false;
                                });
                                list->addItem(listItem);
                            } else {
                                listItem->setClickListener([this, i, cmds=commands, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                    if (keys & KEY_A) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, keyName, i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                        
                                        listItem->setValue(CHECKMARK_SYMBOL);
                                        return true;
                                    }  else if (keys & SCRIPT_KEY) {
                                        if (inPackageMenu)
                                            inPackageMenu = false;
                                        if (inSubPackageMenu)
                                            inSubPackageMenu = false;
                                        tsl::changeTo<ScriptOverlay>(packagePath, keyName);
                                        return true;
                                    }
                                    return false;
                                });
                                list->addItem(listItem);
                            }
                        } else if (commandMode == "toggle") {
                            
                            
                            auto toggleListItem = new tsl::elm::ToggleListItem(removeTag(optionName), false, ON, OFF);
                            // Set the initial state of the toggle item
                            bool toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn));
                            
                            toggleListItem->setState(toggleStateOn);
                            
                            toggleListItem->setStateChangedListener([this, i, cmdsOn=commandsOn, cmdsOff=commandsOff, toggleStateOn, keyName = option.first](bool state) {
                                if (!state) {
                                    // Toggle switched to On
                                    if (toggleStateOn) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOn, preprocessPath(pathPatternOn), i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                    } else {
                                        // Handle the case where the command should only run in the source_on section
                                        // Add your specific code here
                                    }
                                } else {
                                    // Toggle switched to Off
                                    if (!toggleStateOn) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOff, preprocessPath(pathPatternOff), i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                    } else {
                                        // Handle the case where the command should only run in the source_off section
                                        // Add your specific code here
                                    }
                                }
                            });
                            list->addItem(toggleListItem);
                        }
                    }
                }
            }
        }
        
        
        if (usingPages) {
            if (currentPage == "left")
                rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color, "", pageRightName);
            else if (currentPage == "right")
                rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color, pageLeftName, "");
        } else
            rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color);
        rootFrame->setContent(list);
        
        return rootFrame;
    }
    
    /**
     * @brief Handles user input for the sub-menu overlay.
     *
     * Processes user input and responds accordingly within the sub-menu overlay.
     * Captures key presses and performs actions based on user interactions.
     *
     * @param keysDown A bitset representing keys that are currently pressed.
     * @param keysHeld A bitset representing keys that are held down.
     * @param touchInput Information about touchscreen input.
     * @param leftJoyStick Information about the left joystick input.
     * @param rightJoyStick Information about the right joystick input.
     * @return `true` if the input was handled within the overlay, `false` otherwise.
     */
    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (refreshGui) {
            tsl::changeTo<PackageMenu>(packagePath);
            refreshGui = false;
        }
        
        if (usingPages) {
            if (currentPage == "left") {
                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR))) {
                    tsl::changeTo<PackageMenu>(packagePath, dropdownSection, "right");
                    return true;
                }
            } else if (currentPage == "right") {
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR))) {
                    //tsl::changeTo<PackageMenu>(packagePath, dropdownSection, "left");
                    tsl::goBack();
                    return true;
                }
            } 
        }
        
        if (!returningToPackage && inPackageMenu) {
            if ((keysHeld & KEY_B)) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inPackageMenu = false;
                returningToMain = true;
                tsl::goBack();
                tsl::goBack();
                tsl::changeTo<MainMenu>();
                
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        
        if (!returningToSubPackage && inSubPackageMenu) {
            if ((keysHeld & KEY_B)) {
                inSubPackageMenu = false;
                returningToPackage = true;
                lastMenu = "packageMenu";
                tsl::goBack();
                
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        
        
        if (keysHeld & KEY_B)
            return false;
        
        if (returningToPackage && !(keysHeld & KEY_B)){
            returningToPackage = false;
            inPackageMenu = true;
        }
        
        if (returningToSubPackage && !(keysHeld & KEY_B)){
            returningToSubPackage = false;
            inSubPackageMenu = true;
        }
        
        return false;
        
        //return handleOverlayMenuInput(inPackageMenu, keysHeld, KEY_B);
    }
};



/**
 * @brief The `MainMenu` class handles the main menu overlay functionality.
 *
 * This class manages the main menu overlay, allowing users to navigate and access various submenus.
 * It provides functions for creating, updating, and navigating the main menu, as well as handling user interactions related to menu navigation.
 */
class MainMenu : public tsl::Gui {
private:
    tsl::hlp::ini::IniData settingsData, themesData, packageConfigData;
    std::string packageIniPath = packageDirectory + packageFileName;
    std::string packageConfigIniPath = packageDirectory + configFileName;
    std::string menuMode, defaultMenuMode, inOverlayString, fullPath, optionName, priority, starred, hide;
    bool useDefaultMenu = false;
    bool useOverlayLaunchArgs = false;
    std::string hiddenMenuMode;
    bool initializingSpawn = false;
    
    std::string defaultLang = "en";
    std::string packagePath, pathReplace, pathReplaceOn, pathReplaceOff;
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
public:
    /**
     * @brief Constructs a `MainMenu` instance.
     *
     * Initializes a new instance of the `MainMenu` class with the necessary parameters.
     */
    MainMenu(const std::string& hiddenMenuMode = "") : hiddenMenuMode(hiddenMenuMode) {}
    /**
     * @brief Destroys the `MainMenu` instance.
     *
     * Cleans up any resources associated with the `MainMenu` instance.
     */
    ~MainMenu() {}
    
    /**
     * @brief Creates the graphical user interface (GUI) for the main menu overlay.
     *
     * This function initializes and sets up the GUI elements for the main menu overlay,
     * allowing users to navigate and access various submenus.
     *
     * @return A pointer to the GUI element representing the main menu overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        if (!inHiddenMode)
            inMainMenu = true;
        
        lastMenuMode = hiddenMenuMode;
        
        //defaultMenuMode = "last_menu";
        defaultMenuMode = "overlays";
        menuMode = "overlays";
        
        createDirectory(packageDirectory);
        createDirectory(settingsPath);
        
        bool settingsLoaded = false;
        if (isFileOrDirectory(settingsConfigIniPath)) {
            settingsData = getParsedDataFromIniFile(settingsConfigIniPath);
            if (settingsData.count("ultrahand") > 0) {
                auto& ultrahandSection = settingsData["ultrahand"];
                
                if (ultrahandSection.count("clean_version_labels") > 0)
                    cleanVersionLabels = ultrahandSection["clean_version_labels"];
                else {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "clean_version_labels", "true");
                    cleanVersionLabels = "false";
                }
                
                // For hiding the versions of overlays/packages
                if (ultrahandSection.count("hide_overlay_versions") > 0)
                    hideOverlayVersions = ultrahandSection["hide_overlay_versions"];
                else {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_overlay_versions", "false");
                    hideOverlayVersions = "false";
                }
                if (ultrahandSection.count("hide_package_versions") > 0)
                    hidePackageVersions = ultrahandSection["hide_package_versions"];
                else {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_package_versions", "false");
                    hidePackageVersions = "false";
                }
                
                if (ultrahandSection.count("last_menu") > 0) {
                    menuMode = ultrahandSection["last_menu"];
                    if (ultrahandSection.count("default_menu") > 0) {
                        defaultMenuMode = ultrahandSection["default_menu"];
                        if (ultrahandSection.count("in_overlay") > 0)
                            settingsLoaded = true;
                    }
                }
                
                if (ultrahandSection.count("default_lang") > 0)
                    defaultLang = ultrahandSection["default_lang"];
                else
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "default_lang", defaultLang);
                
                if (ultrahandSection.count("datetime_format") == 0)
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "datetime_format", DEFAULT_DT_FORMAT);
                
                if (ultrahandSection.count("hide_clock") == 0)
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_clock", "false");
                if (ultrahandSection.count("hide_battery") == 0)
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_battery", "true");
                if (ultrahandSection.count("hide_pcb_temp") == 0)
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_pcb_temp", "true");
                if (ultrahandSection.count("hide_soc_temp") == 0)
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_soc_temp", "true");
                
                //if (ultrahandSection.count("in_overlay") > 0) {
                //    inOverlayString = ultrahandSection["in_overlay"];
                //    if (inOverlayString == "true") {
                //        setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "false");
                //    }
                //    settingsLoaded = true;
                //}
            }
        }
        if (!settingsLoaded) { // write data if settings are not loaded
            setIniFileValue(settingsConfigIniPath, "ultrahand", "default_lang", defaultLang);
            setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
            setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", menuMode);
            setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "false");
        }
        
        
        //if (!showMenu) {
        //    rootFrame = new tsl::elm::OverlayFrame("","");
        //    rootFrame->setContent(list);
        //    return rootFrame;
        //}
        
        std::string langFile = "/config/ultrahand/lang/"+defaultLang+".json";
        if (isFileOrDirectory(langFile))
            parseLanguage(langFile);
        
        // write default theme
        if (isFileOrDirectory(themeConfigIniPath)) {
            themesData = getParsedDataFromIniFile(themeConfigIniPath);
            if (themesData.count("theme") > 0) {
                auto& themedSection = themesData["theme"];
                
                if (themedSection.count("clock_color") == 0)
                    setIniFileValue(themeConfigIniPath, "theme", "clock_color", "#FFFFFF");
                
                if (themedSection.count("battery_color") == 0)
                    setIniFileValue(themeConfigIniPath, "theme", "battery_color", "#FFFFFF");
                
                if (themedSection.count("text_color") == 0)
                    setIniFileValue(themeConfigIniPath, "theme", "text_color", "#FFFFFF");
                
                if (themedSection.count("trackbar_color") == 0)
                    setIniFileValue(themeConfigIniPath, "theme", "trackbar_color", "#555555");
                
                if (themedSection.count("highlight_color_1") == 0)
                    setIniFileValue(themeConfigIniPath, "theme", "highlight_color_1", "#2288CC");
                
                if (themedSection.count("highlight_color_2") == 0)
                    setIniFileValue(themeConfigIniPath, "theme", "highlight_color_2", "#88FFFF");
                
            } else {
                setIniFileValue(themeConfigIniPath, "theme", "clock_color", "#FFFFFF");
                setIniFileValue(themeConfigIniPath, "theme", "battery_color", "#FFFFFF");
                setIniFileValue(themeConfigIniPath, "theme", "text_color", "#FFFFFF");
                setIniFileValue(themeConfigIniPath, "theme", "trackbar_color", "#555555");
                setIniFileValue(themeConfigIniPath, "theme", "highlight_color_1", "#2288CC");
                setIniFileValue(themeConfigIniPath, "theme", "highlight_color_2", "#88FFFF");
            }
        } else {
            setIniFileValue(themeConfigIniPath, "theme", "clock_color", "#FFFFFF");
            setIniFileValue(themeConfigIniPath, "theme", "battery_color", "#FFFFFF");
            setIniFileValue(themeConfigIniPath, "theme", "text_color", "#FFFFFF");
            setIniFileValue(themeConfigIniPath, "theme", "trackbar_color", "#555555");
            setIniFileValue(themeConfigIniPath, "theme", "highlight_color_1", "#2288CC");
            setIniFileValue(themeConfigIniPath, "theme", "highlight_color_2", "#88FFFF");
        }
        
        copyTeslaKeyComboToUltrahand();
        
        //setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "false");
        
        
        if ((defaultMenuMode == "overlays") || (defaultMenuMode == "packages")) {
            if (defaultMenuLoaded) {
                menuMode = defaultMenuMode.c_str();
                defaultMenuLoaded = false;
            }
        } else {
            defaultMenuMode = "last_menu";
            setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
        }
        
        if (cleanVersionLabels == "true")
            versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" "+cleanVersionLabel(loaderInfo)+std::string(")"); // Still needs to parse nx-ovlloader instead of hard coding it
        else
            versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" v"+cleanVersionLabel(loaderInfo)+std::string(")");
        
        list = new tsl::elm::List();
        
        
        
        if (!hiddenMenuMode.empty())
            menuMode = hiddenMenuMode;
        
        
        // Overlays menu
        if (menuMode == "overlays") {
            if (!inHiddenMode)
                list->addItem(new tsl::elm::CategoryHeader(OVERLAYS));
            else
                list->addItem(new tsl::elm::CategoryHeader(HIDDEN_OVERLAYS));
            
            
            // Load overlay files
            std::vector<std::string> overlayFiles = getFilesListByWildcard(overlayDirectory+"*.ovl");
            //std::sort(overlayFiles.begin(), overlayFiles.end()); // Sort overlay files alphabetically
            
            
            FILE* overlaysIniFile = fopen(overlaysIniFilePath.c_str(), "r");
            if (!overlaysIniFile) {
                fclose(fopen(overlaysIniFilePath.c_str(), "w")); // The INI file doesn't exist, so create an empty one.
                initializingSpawn = true;
            } else
                fclose(overlaysIniFile); // The file exists, so close it.
            
            // load overlayList from overlaysIniFilePath.  this will be the overlayFilenames
            std::vector<std::string> overlayList;
            std::vector<std::string> hiddenOverlayList;
            
            
            // Load subdirectories
            if (!overlayFiles.empty()) {
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> overlaysIniData = getParsedDataFromIniFile(overlaysIniFilePath);
                
                for (const auto& overlayFile : overlayFiles) {
                    
                    std::string overlayFileName = getNameFromPath(overlayFile);
                    
                    if (overlayFileName == "ovlmenu.ovl" or overlayFileName.substr(0, 1) == ".")
                        continue;
                    
                    //overlayList.push_back(overlayFileName);
                    
                    // Check if the overlay name exists in the INI data.
                    if (overlaysIniData.find(overlayFileName) == overlaysIniData.end()) {
                        // The entry doesn't exist; initialize it.
                        overlayList.push_back("0020_"+overlayFileName);
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "priority", "20");
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "star", "false");
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "hide", "false");
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "use_launch_args", "false");
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "launch_args", "");
                        
                    } else {
                        // Read priority and starred status from ini
                        priority = "0020";
                        starred = "false";
                        hide = "false";
                        
                        // Check if the "priority" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("priority") != overlaysIniData[overlayFileName].end()) {
                            priority = formatPriorityString(overlaysIniData[overlayFileName]["priority"]);
                        } else
                            setIniFileValue(overlaysIniFilePath, overlayFileName, "priority", "20");
                        
                        // Check if the "star" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("star") != overlaysIniData[overlayFileName].end()) {
                            starred = overlaysIniData[overlayFileName]["star"];
                        } else
                            setIniFileValue(overlaysIniFilePath, overlayFileName, "star", "false");
                        
                        // Check if the "hide" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("hide") != overlaysIniData[overlayFileName].end()) {
                            hide = overlaysIniData[overlayFileName]["hide"];
                        } else
                            setIniFileValue(overlaysIniFilePath, overlayFileName, "hide", "false");
                        
                        // Check if the "hide" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("use_launch_args") != overlaysIniData[overlayFileName].end()) {
                            //useOverlayLaunchArgs = (overlaysIniData[overlayFileName]["use_launch_args"] == "true");
                        } else
                            setIniFileValue(overlaysIniFilePath, overlayFileName, "use_launch_args", "false");
                        
                        // Check if the "hide" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("launch_args") != overlaysIniData[overlayFileName].end()) {
                            //overlayLaunchArgs = overlaysIniData[overlayFileName]["launch_args"];
                        } else
                            setIniFileValue(overlaysIniFilePath, overlayFileName, "launch_args", "");
                        
                        
                        
                        
                        // Get the name and version of the overlay file
                        auto [result, overlayName, overlayVersion] = getOverlayInfo(overlayDirectory+overlayFileName);
                        if (result != ResultSuccess)
                            continue;
                        
                        if (hide == "false") {
                            if (starred == "true")
                                overlayList.push_back("-1_"+priority+"_"+overlayName+"_"+overlayVersion+"_"+overlayFileName);
                            else
                                overlayList.push_back(priority+"_"+overlayName+"_"+overlayVersion+"_"+overlayFileName);
                            
                        } else {
                            if (starred == "true")
                                hiddenOverlayList.push_back("-1_"+priority+"_"+overlayName+"_"+overlayVersion+"_"+overlayFileName);
                            else
                                hiddenOverlayList.push_back(priority+"_"+overlayName+"_"+overlayVersion+"_"+overlayFileName);
                            
                        }
                    }
                }
                
                std::sort(overlayList.begin(), overlayList.end());
                std::sort(hiddenOverlayList.begin(), hiddenOverlayList.end());
                
                
                if (inHiddenMode)
                    overlayList = hiddenOverlayList;
                
                
                for (const auto& taintedOverlayFileName : overlayList) {
                    
                    //logMessage(taintedOverlayFileName);
                    
                    std::string overlayFileName;
                    std::string overlayStarred = "false";
                    
                    std::string overlayVersion, overlayName;
                    
                    // Detect if starred
                    if ((taintedOverlayFileName.substr(0, 3) == "-1_"))
                        overlayStarred = "true";
                    
                    // Find the position of the last underscore
                    size_t lastUnderscorePos = taintedOverlayFileName.rfind('_');
                    // Check if an underscore was found
                    if (lastUnderscorePos != std::string::npos) {
                        // Extract overlayFileName starting from the character after the last underscore
                        overlayFileName = taintedOverlayFileName.substr(lastUnderscorePos + 1);
                        
                        // Now, find the position of the second-to-last underscore
                        size_t secondLastUnderscorePos = taintedOverlayFileName.rfind('_', lastUnderscorePos - 1);
                        
                        if (secondLastUnderscorePos != std::string::npos) {
                            // Extract overlayName between the two underscores
                            overlayVersion = taintedOverlayFileName.substr(secondLastUnderscorePos + 1, lastUnderscorePos - secondLastUnderscorePos - 1);
                            // Now, find the position of the second-to-last underscore
                            size_t thirdLastUnderscorePos = taintedOverlayFileName.rfind('_', secondLastUnderscorePos - 1);
                            if (secondLastUnderscorePos != std::string::npos)
                                overlayName = taintedOverlayFileName.substr(thirdLastUnderscorePos + 1, secondLastUnderscorePos - thirdLastUnderscorePos - 1);
                        }
                    }
                    
                    
                    //logMessage(overlayFileName);
                    
                    std::string overlayFile = overlayDirectory+overlayFileName;
                    //logMessage(overlayFile);
                    
                    //// Get the name and version of the overlay file
                    //auto [result, overlayName, overlayVersion] = getOverlayInfo(overlayFile);
                    //if (result != ResultSuccess)
                    //    continue;
                    
                    //logMessage(overlayName);
                    
                    std::string newOverlayName = overlayName.c_str();
                    if (overlayStarred == "true")
                        newOverlayName = STAR_SYMBOL+" "+newOverlayName;
                    
                    
                    
                    // Toggle the starred status
                    std::string newStarred = (overlayStarred == "true") ? "false" : "true";
                    
                    tsl::elm::ListItem* listItem = nullptr;
                    
                    //logMessage(overlayFile);
                    if (isFileOrDirectory(overlayFile)) {
                        listItem = new tsl::elm::ListItem(newOverlayName);
                        if (cleanVersionLabels == "true")
                            overlayVersion = cleanVersionLabel(overlayVersion);
                        if (hideOverlayVersions != "true")
                            listItem->setValue(overlayVersion, true);
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([this, overlayFile, newStarred, overlayFileName, overlayName](s64 key) {
                            if (key & KEY_A) {
                                // Load the overlay here
                                //inMainMenu = false;
                                //inOverlay = true;
                                setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "true"); // this is handled within tesla.hpp
                                //if (useOverlayLaunchArgs) {
                                //    logMessage("LaunchArgs: "+overlayLaunchArgs);
                                //    tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                                //} else
                                //    tsl::setNextOverlay(overlayFile);
                                //logMessage("LaunchArgs: "+overlayLaunchArgs);
                                std::string useOverlayLaunchArgs = parseValueFromIniSection(overlaysIniFilePath, overlayFileName, "use_launch_args");
                                std::string overlayLaunchArgs = parseValueFromIniSection(overlaysIniFilePath, overlayFileName, "launch_args");
                                
                                
                                if (useOverlayLaunchArgs == "true")
                                    tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                                else
                                    tsl::setNextOverlay(overlayFile);
                                //envSetNextLoad(overlayPath, "");
                                tsl::Overlay::get()->close();
                                //inMainMenu = true;
                                return true;
                            } else if (key & STAR_KEY) {
                                std::string tmpMode(hiddenMenuMode);
                                if (!overlayFile.empty()) {
                                    // Update the INI file with the new value
                                    setIniFileValue(overlaysIniFilePath, overlayFileName, "star", newStarred);
                                    // Now, you can use the newStarred value for further processing if needed
                                }
                                if (inHiddenMode) {
                                    tsl::goBack();
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                }
                                tsl::changeTo<MainMenu>(tmpMode);
                                //lastMenuMode = tmpMode;
                                return true;
                            } else if (key & SETTINGS_KEY) {
                                if (!inHiddenMode) {
                                    lastMenu = "";
                                    inMainMenu = false;
                                } else {
                                    lastMenu = "hiddenMenuMode";
                                    inHiddenMode = false;
                                }
                                
                                tsl::changeTo<SettingsMenu>(overlayFileName, "overlay", overlayName);
                                return true;
                            }
                            return false;
                        });
                    }
                    if (listItem != nullptr)
                        list->addItem(listItem);
                }
                
                if (!hiddenOverlayList.empty() && !inHiddenMode) {
                    auto listItem = new tsl::elm::ListItem(HIDDEN, DROPDOWN_SYMBOL);
                    
                    //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                    listItem->setClickListener([this](uint64_t keys) {
                        if (keys & KEY_A) {
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>("overlays");
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem);
                }
            }
        }
        
        
        
        // Packages menu
        if (menuMode == "packages" ) {
            
            // Create the directory if it doesn't exist
            createDirectory(packageDirectory);
            
            
            FILE* packagesIniFile = fopen(packagesIniFilePath.c_str(), "r");
            if (!packagesIniFile) {
                fclose(fopen(packagesIniFilePath.c_str(), "w")); // The INI file doesn't exist, so create an empty one.
                initializingSpawn = true;
            } else
                fclose(packagesIniFile); // The file exists, so close it.
            
            std::vector<std::string> packageList;
            std::vector<std::string> hiddenPackageList;
            
            // Load the INI file and parse its content.
            std::map<std::string, std::map<std::string, std::string>> packagesIniData = getParsedDataFromIniFile(packagesIniFilePath);
            // Load subdirectories
            std::vector<std::string> subdirectories = getSubdirectories(packageDirectory);
            //for (size_t i = 0; i < subdirectories.size(); ++i) {
            for (const auto& packageName: subdirectories) {
                if (packageName.substr(0, 1) == ".")
                    continue;
                // Check if the overlay name exists in the INI data.
                if (packagesIniData.find(packageName) == packagesIniData.end()) {
                    // The entry doesn't exist; initialize it.
                    packageList.push_back("0020_"+packageName);
                    setIniFileValue(packagesIniFilePath, packageName, "priority", "20");
                    setIniFileValue(packagesIniFilePath, packageName, "star", "false");
                    setIniFileValue(packagesIniFilePath, packageName, "hide", "false");
                } else {
                    // Read priority and starred status from ini
                    priority = "0020";
                    starred = "false";
                    hide = "false";
                    
                    // Check if the "priority" key exists in overlaysIniData for overlayFileName
                    if (packagesIniData.find(packageName) != packagesIniData.end() &&
                        packagesIniData[packageName].find("priority") != packagesIniData[packageName].end()) {
                        priority = formatPriorityString(packagesIniData[packageName]["priority"]);
                    } else
                        setIniFileValue(packagesIniFilePath, packageName, "priority", "20");
                    
                    // Check if the "star" key exists in overlaysIniData for overlayFileName
                    if (packagesIniData.find(packageName) != packagesIniData.end() &&
                        packagesIniData[packageName].find("star") != packagesIniData[packageName].end()) {
                        starred = packagesIniData[packageName]["star"];
                    } else
                        setIniFileValue(packagesIniFilePath, packageName, "star", "false");
                    
                    // Check if the "star" key exists in overlaysIniData for overlayFileName
                    if (packagesIniData.find(packageName) != packagesIniData.end() &&
                        packagesIniData[packageName].find("hide") != packagesIniData[packageName].end()) {
                        hide = packagesIniData[packageName]["hide"];
                    } else
                        setIniFileValue(packagesIniFilePath, packageName, "hide", "false");
                    
                    if (hide == "false") {
                        if (starred == "true")
                            packageList.push_back("-1_"+priority+"_"+packageName);
                        else
                            packageList.push_back(priority+"_"+packageName);
                    } else {
                        if (starred == "true")
                            hiddenPackageList.push_back("-1_"+priority+"_"+packageName);
                        else
                            hiddenPackageList.push_back(priority+"_"+packageName);
                    }
                }
            }
            std::sort(packageList.begin(), packageList.end());
            std::sort(hiddenPackageList.begin(), hiddenPackageList.end());
            
            if (inHiddenMode)
                packageList = hiddenPackageList;
            
            for (size_t i = 0; i < packageList.size(); ++i) {
                auto taintePackageName = packageList[i];
                if (i == 0) {
                    if (!inHiddenMode)
                        list->addItem(new tsl::elm::CategoryHeader(PACKAGES));
                    else
                        list->addItem(new tsl::elm::CategoryHeader(HIDDEN_PACKAGES));
                }
                //bool usingStar = false;
                std::string packageName = taintePackageName.c_str();
                std::string packageStarred = "false";
                
                if ((packageName.length() >= 2) && (packageName.substr(0, 3) == "-1_")) {
                    // strip first two characters
                    packageName = packageName.substr(3);
                    packageStarred = "true";
                }
                
                packageName = packageName.substr(5);
                
                std::string newPackageName = packageName.c_str();
                if (packageStarred == "true")
                    newPackageName = STAR_SYMBOL+" "+newPackageName;
                
                std::string packageFilePath = packageDirectory + packageName+ "/";
                
                // Toggle the starred status
                std::string newStarred = (packageStarred == "true") ? "false" : "true";
                
                tsl::elm::ListItem* listItem = nullptr;
                if (isFileOrDirectory(packageFilePath)) {
                    PackageHeader packageHeader = getPackageHeaderFromIni(packageFilePath+packageFileName);
                    //if (count == 0) {
                    //    // Add a section break with small text to indicate the "Packages" section
                    //    list->addItem(new tsl::elm::CategoryHeader(PACKAGES));
                    //}
                    
                    listItem = new tsl::elm::ListItem(newPackageName);
                    if (cleanVersionLabels == "true")
                        packageHeader.version = cleanVersionLabel(packageHeader.version);
                    if (hidePackageVersions != "true")
                       listItem->setValue(packageHeader.version, true);
                    
                    // Add a click listener to load the overlay when clicked upon
                    listItem->setClickListener([this, packageFilePath, newStarred, packageName](s64 key) {
                        if (key & KEY_A) {
                            if (!inHiddenMode)
                                inMainMenu = false;
                            else
                                inHiddenMode = false;
                            
                            // read commands from package's boot_package.ini
                            
                            if (isFileOrDirectory(packageFilePath+bootPackageFileName)) {
                                std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> bootOptions = loadOptionsFromIni(packageFilePath+bootPackageFileName, true);
                                if (bootOptions.size() > 0) {
                                    for (const auto& bootOption:bootOptions) {
                                        std::string bootOptionName = bootOption.first;
                                        auto bootCommands = bootOption.second;
                                        if (bootOptionName == "boot") {
                                            refreshGui = interpretAndExecuteCommand(bootCommands, packageFilePath+bootPackageFileName, bootOptionName); // Execute modified 
                                            break;
                                        }
                                    }
                                }
                            }
                            
                            
                            tsl::changeTo<PackageMenu>(packageFilePath, "");
                            
                            return true;
                        } else if (key & STAR_KEY) {
                            std::string tmpMode(hiddenMenuMode);
                            if (!packageName.empty())
                                setIniFileValue(packagesIniFilePath, packageName, "star", newStarred); // Update the INI file with the new value
                            
                            if (inHiddenMode) {
                                tsl::goBack();
                                inMainMenu = false;
                                inHiddenMode = true;
                            }
                            tsl::changeTo<MainMenu>(tmpMode);
                            //lastMenuMode = tmpMode;
                            return true;
                        } else if (key & SETTINGS_KEY) {
                            
                            if (!inHiddenMode) {
                                lastMenu = "";
                                inMainMenu = false;
                            } else {
                                lastMenu = "hiddenMenuMode";
                                inHiddenMode = false;
                            }
                            
                            tsl::changeTo<SettingsMenu>(packageName, "package");
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                    //count++;
                }
            }
            
            if (!hiddenPackageList.empty() && !inHiddenMode) {
                auto listItem = new tsl::elm::ListItem(HIDDEN, DROPDOWN_SYMBOL);
                
                //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                listItem->setClickListener([this](uint64_t keys) {
                    if (keys & KEY_A) {
                        inMainMenu = false;
                        inHiddenMode = true;
                        tsl::changeTo<MainMenu>("packages");
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem);
            }
            
            
            // ********* THIS PART ALWAYS NEEDS TO MIRROR WHAT IS WITHIN SUBMENU (perhaps create a new method?)*********
            
            
            if (!inHiddenMode) {
                // Load options from INI file
                std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath, true);
                
                // initialize packageConfigIniPath text file
                
                for (size_t i = 0; i < options.size(); ++i) {
                    auto option = options[i];
                    
                    std::string optionName = option.first;
                    auto commands = option.second;
                    
                    std::string footer; 
                    bool useSelection = false;
                    
                    
                    std::string commandFooter = "null";
                    std::string commandMode = "default";
                    std::string commandGrouping = "default";
                    
                    std::string currentSection = "global";
                    std::string sourceType = "default", sourceTypeOn = "default", sourceTypeOff = "default"; 
                    //std::string sourceType, sourceTypeOn, sourceTypeOff; //"file", "json_file", "json", "list"
                    std::string jsonPath, jsonPathOn, jsonPathOff;
                    std::string jsonKey, jsonKeyOn, jsonKeyOff;
                    
                    
                    std::vector<std::vector<std::string>> commandsOn;
                    std::vector<std::vector<std::string>> commandsOff;
                    std::vector<std::string> listData, listDataOn, listDataOff;
                    
                    
                    if (commands.size() == 0) {
                        // Add a section break with small text to indicate the "Commands" section
                        list->addItem(new tsl::elm::CategoryHeader(removeTag(optionName)));
                        continue;
                    } else if (i == 0) // Add a section break with small text to indicate the "Commands" section
                        list->addItem(new tsl::elm::CategoryHeader(COMMANDS));
                    
                    
                    
                    
                    // items can be paths, commands, or variables depending on source
                    //std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;
                    
                    // initial processing of commands
                    for (const auto& cmd : commands) {
                        
                        
                        if (!cmd.empty()) { // Isolate command settings
                            // Extract the command mode
                            if (cmd[0].find(modePattern) == 0) {
                                commandMode = cmd[0].substr(modePattern.length());
                                if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                    commandMode = commandModes[0]; // reset to default if commandMode is unknown
                            } else if (cmd[0].find(groupingPattern) == 0) {// Extract the command grouping
                                commandGrouping = cmd[0].substr(groupingPattern.length());
                                if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                    commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                            }
                            
                            // Extract the command grouping
                            if (commandMode == "toggle") {
                                if (cmd[0].find("on:") == 0)
                                    currentSection = "on";
                                else if (cmd[0].find("off:") == 0)
                                    currentSection = "off";
                                
                                // Seperation of command chuncks
                                if (currentSection == "global") {
                                    commandsOn.push_back(cmd);
                                    commandsOff.push_back(cmd);
                                } else if (currentSection == "on")
                                    commandsOn.push_back(cmd);
                                else if (currentSection == "off")
                                    commandsOff.push_back(cmd);
                            }
                        }
                        if (cmd.size() > 1) { // Pre-process advanced commands
                            //if (cmd[0] == "filter") {
                            //    if (currentSection == "global") {
                            //        filterList.push_back(cmd[1]);
                            //    } else if (currentSection == "on") {
                            //        filterListOn.push_back(cmd[1]);
                            //    } else if (currentSection == "off") {
                            //        filterListOff.push_back(cmd[1]);
                            //    }
                            if (cmd[0] == "file_source") {
                                if (currentSection == "global") {
                                    pathPattern = cmd[1];
                                    //filesList = getFilesListByWildcards(pathPattern);
                                    sourceType = "file";
                                } else if (currentSection == "on") {
                                    pathPatternOn = cmd[1];
                                    //filesListOn = getFilesListByWildcards(pathPatternOn);
                                    sourceTypeOn = "file";
                                } else if (currentSection == "off") {
                                    pathPatternOff = cmd[1];
                                    //filesListOff = getFilesListByWildcards(pathPatternOff);
                                    sourceTypeOff = "file";
                                }
                            }
                        }
                    }
                    
                    
                    
                    
                    if (isFileOrDirectory(packageConfigIniPath)) {
                        packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                        
                        
                        if (packageConfigData.count(optionName) > 0) {
                            auto& optionSection = packageConfigData[optionName];
                            
                            // For hiding the versions of overlays/packages
                            if (optionSection.count("mode") > 0)
                                commandMode = optionSection["mode"];
                            else
                                setIniFileValue(packageConfigIniPath, optionName, "mode", commandMode);
                            
                            if (optionSection.count("grouping") > 0)
                                commandGrouping = optionSection["grouping"];
                            else
                                setIniFileValue(packageConfigIniPath, optionName, "grouping", commandGrouping);
                            
                            
                            if (optionSection.count("footer") > 0)
                                commandFooter = optionSection["footer"];
                            else
                                setIniFileValue(packageConfigIniPath, optionName, "footer", commandFooter);
                            
                        }
                    } else { // write data if settings are not loaded
                        setIniFileValue(packageConfigIniPath, optionName, "mode", commandMode);
                        setIniFileValue(packageConfigIniPath, optionName, "grouping", commandGrouping);
                        setIniFileValue(packageConfigIniPath, optionName, "footer", commandFooter);
                    }
                    
                    
                    
                    // get Option Name and footer
                    if (optionName[0] == '*') { 
                        useSelection = true;
                        optionName = optionName.substr(1); // Strip the "*" character on the left
                        footer = DROPDOWN_SYMBOL;
                    } else {
                        size_t pos = optionName.find(" - ");
                        if (pos != std::string::npos) {
                            footer = optionName.substr(pos + 2); // Assign the part after "&&" as the footer
                            optionName = optionName.substr(0, pos); // Strip the "&&" and everything after it
                        }
                    }
                    
                    // override loading of the command footer
                    if (commandFooter != "null")
                        footer = commandFooter;
                    
                    
                    if (useSelection) { // For wildcard commands (dropdown menus)
                        auto listItem = static_cast<tsl::elm::ListItem*>(nullptr);
                        if ((footer == DROPDOWN_SYMBOL) || (footer.empty()))
                            listItem = new tsl::elm::ListItem(removeTag(optionName), footer);
                        else {
                            listItem = new tsl::elm::ListItem(removeTag(optionName));
                            listItem->setValue(footer, true);
                        }
                        
                        //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                        listItem->setClickListener([this, cmds = commands, keyName = option.first, packagePath = packageDirectory, listItem](uint64_t keys) {
                            if (keys & KEY_A) {
                                inMainMenu = false;
                                tsl::changeTo<SelectionOverlay>(packagePath, keyName, cmds);
                                return true;
                            } else if (keys & SCRIPT_KEY) {
                                inMainMenu = false; // Set boolean to true when entering a submenu
                                tsl::changeTo<ScriptOverlay>(packagePath, keyName, true);
                                return true;
                            }
                            return false;
                        });
                        
                        list->addItem(listItem);
                    } else { // For everything else
                        
                        const std::string& selectedItem = optionName;
                        
                        // For entries that are paths
                        itemName = getNameFromPath(selectedItem);
                        if (!isDirectory(preprocessPath(selectedItem)))
                            itemName = dropExtension(itemName);
                        parentDirName = getParentDirNameFromPath(selectedItem);
                        
                        
                        if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                            auto listItem = new tsl::elm::ListItem(removeTag(optionName));
                            listItem->setValue(footer, true);
                            
                            if (sourceType == "json") { // For JSON wildcards
                                listItem->setClickListener([this, i, cmds=commands, packagePath = packageDirectory, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                    if (keys & KEY_A) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem, i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                        
                                        listItem->setValue(CHECKMARK_SYMBOL);
                                        return true;
                                    } else if (keys & SCRIPT_KEY) {
                                        inMainMenu = false; // Set boolean to true when entering a submenu
                                        tsl::changeTo<ScriptOverlay>(packagePath, keyName, true);
                                        return true;
                                    }
                                    
                                    return false;
                                });
                                list->addItem(listItem);
                            } else {
                                listItem->setClickListener([this, i, cmds=commands, packagePath = packageDirectory, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                    if (keys & KEY_A) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem, i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                        
                                        listItem->setValue(CHECKMARK_SYMBOL);
                                        return true;
                                    } else if (keys & SCRIPT_KEY) {
                                        inMainMenu = false; // Set boolean to true when entering a submenu
                                        tsl::changeTo<ScriptOverlay>(packagePath, keyName, true);
                                        return true;
                                    }
                                    return false;
                                });
                                list->addItem(listItem);
                            }
                        } else if (commandMode == "toggle") {
                            
                            
                            auto toggleListItem = new tsl::elm::ToggleListItem(removeTag(optionName), false, ON, OFF);
                            // Set the initial state of the toggle item
                            bool toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn));
                            
                            toggleListItem->setState(toggleStateOn);
                            
                            toggleListItem->setStateChangedListener([this, i, cmdsOn=commandsOn, cmdsOff=commandsOff, toggleStateOn, packagePath = packageDirectory, keyName = option.first](bool state) {
                                if (!state) {
                                    // Toggle switched to On
                                    if (toggleStateOn) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOn, preprocessPath(pathPatternOn), i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                    } else {
                                        // Handle the case where the command should only run in the source_on section
                                        // Add your specific code here
                                    }
                                } else {
                                    // Toggle switched to Off
                                    if (!toggleStateOn) {
                                        std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOff, preprocessPath(pathPatternOff),  i); // replace source
                                        //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                        refreshGui = interpretAndExecuteCommand(modifiedCmds, packagePath, keyName); // Execute modified 
                                    } else {
                                        // Handle the case where the command should only run in the source_off section
                                        // Add your specific code here
                                    }
                                }
                            });
                            list->addItem(toggleListItem);
                        }
                    }
                }
            }
        }
        if (initializingSpawn) {
            initializingSpawn = false;
            return createUI(); 
        }
        
        
        rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel, menuMode+hiddenMenuMode);
        rootFrame->setContent(list);
        return rootFrame;
    }
    
    /**
     * @brief Handles user input for the main menu overlay.
     *
     * Processes user input and responds accordingly within the main menu overlay.
     * Captures key presses and performs actions based on user interactions.
     *
     * @param keysDown A bitset representing keys that are currently pressed.
     * @param keysHeld A bitset representing keys that are held down.
     * @param touchInput Information about touchscreen input.
     * @param leftJoyStick Information about the left joystick input.
     * @param rightJoyStick Information about the right joystick input.
     * @return `true` if the input was handled within the overlay, `false` otherwise.
     */
    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        if (refreshGui) {
            tsl::changeTo<MainMenu>(lastMenuMode);
            refreshGui = false;
        }
        
        
        if (inMainMenu && !inHiddenMode){
            if (isDownloaded) // for handling software updates
                tsl::Overlay::get()->close();
            
            if (!freshSpawn && !returningToMain && !returningToHiddenMain) {
                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR))) {
                    if (menuMode != "packages") {
                        setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", "packages");
                        tsl::changeTo<MainMenu>();
                        return true;
                    }
                }
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR))) {
                    if (menuMode != "overlays") {
                        setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", "overlays");
                        tsl::goBack();
                        tsl::changeTo<MainMenu>();
                        return true;
                    }
                }
                if (keysHeld & KEY_B) {
                    //inMainMenu = false;
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", defaultMenuMode);
                    tsl::Overlay::get()->close();
                    return true;
                }
                if (keysHeld & SYSTEM_SETTINGS_KEY)
                    tsl::changeTo<UltrahandSettingsMenu>();
            }
        }
        if (!inMainMenu && inHiddenMode) {
            if (!returningToHiddenMain && !returningToMain) {
                if (keysHeld & KEY_B) {
                    returningToMain = true;
                    inHiddenMode = false;
                    
                    if (reloadMenu2) {
                        tsl::goBack();
                        tsl::changeTo<MainMenu>();
                        reloadMenu2 = false;
                        return true;
                    }
                    
                    tsl::goBack();
                    return true;
                }
            }
        }
        
        
        if (keysHeld & KEY_B)
            return false;
        
        if (freshSpawn && !(keysHeld & KEY_B))
            freshSpawn = false;
        
        if (returningToMain && !(keysHeld & KEY_B)){
            returningToMain = false;
            inMainMenu = true;
            selectedFooterDict.clear();
        }
        if (returningToHiddenMain && !(keysHeld & KEY_B)){
            returningToHiddenMain = false;
            inHiddenMode = true;
            selectedFooterDict.clear();
        }
        
        if (redrawWidget) {
            reinitializeWidgetVars();
            redrawWidget = false;
        }
        
        return false;
    }
};


/**
 * @brief The `Overlay` class manages the main overlay functionality.
 *
 * This class is responsible for handling the main overlay, which provides access to various application features and options.
 * It initializes necessary services, handles user input, and manages the transition between different menu modes.
 */
class Overlay : public tsl::Overlay {
public:
    /**
     * @brief Initializes essential services and resources.
     *
     * This function initializes essential services and resources required for the overlay to function properly.
     * It sets up file system mounts, initializes network services, and performs other necessary tasks.
     */
    virtual void initServices() override {
        loaderInfo = envGetLoaderInfo();
        versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" v"+cleanVersionLabel(loaderInfo)+std::string(")");
        fsdevMountSdmc();
        splInitialize();
        spsmInitialize();
        i2cInitialize();
        ASSERT_FATAL(socketInitializeDefault());
        ASSERT_FATAL(nifmInitialize(NifmServiceType_User));
        ASSERT_FATAL(smInitialize());
    }
    
    /**
     * @brief Exits and cleans up services and resources.
     *
     * This function is responsible for exiting and cleaning up services and resources
     * when the overlay is no longer in use. It should release any allocated resources and
     * properly shut down services to avoid memory leaks.
     */
    virtual void exitServices() override {
        socketExit();
        nifmExit();
        i2cExit();
        smExit();
        spsmExit();
        splExit();
        fsdevUnmountAll();
    }
    
    /**
     * @brief Performs actions when the overlay becomes visible.
     *
     * This function is called when the overlay transitions from an invisible state to a visible state.
     * It can be used to perform actions or updates specific to the overlay's visibility.
     */
    virtual void onShow() override {
        //if (rootFrame != nullptr) {
        //    if (inMainMenu && redrawMenu) {
        //        //tsl::Overlay::get()->getCurrentGui()->removeFocus();
        //        //rebuildUI();
        //        showMenu = true;
        //        tsl::changeTo<MainMenu>(lastMenuMode);
        //        //rootFrame->invalidate();
        //        //tsl::Overlay::get()->getCurrentGui()->requestFocus(rootFrame, tsl::FocusDirection::None);
        //    }
        //}
        //redrawMenu = true;
    } 
    
    /**
     * @brief Performs actions when the overlay becomes visible.
     *
     * This function is called when the overlay transitions from an invisible state to a visible state.
     * It can be used to perform actions or updates specific to the overlay's visibility.
     */
    virtual void onHide() override {
        //if (inMainMenu) {
        //    redrawMenu = false;
        //}
    } 
    
    /**
     * @brief Loads the initial graphical user interface (GUI) for the overlay.
     *
     * This function is responsible for loading the initial GUI when the overlay is launched.
     * It returns a unique pointer to the GUI element that will be displayed as the overlay's starting interface.
     * You can also pass arguments to the constructor of the GUI element if needed.
     *
     * @return A unique pointer to the initial GUI element.
     */
    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainMenu>();  // Initial Gui to load. It's possible to pass arguments to its constructor like this
    }
};


/**
 * @brief The entry point of the application.
 *
 * This function serves as the entry point for the application. It takes command-line arguments,
 * initializes necessary services, and starts the main loop of the overlay. The `argc` parameter
 * represents the number of command-line arguments, and `argv` is an array of C-style strings
 * containing the actual arguments.
 *
 * @param argc The number of command-line arguments.
 * @param argv An array of C-style strings representing command-line arguments.
 * @return The application's exit code.
 */
int main(int argc, char* argv[]) {
    return tsl::loop<Overlay, tsl::impl::LaunchFlags::None>(argc, argv);
}
