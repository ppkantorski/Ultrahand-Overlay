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
 *  Licensed under GPLv2
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/

#define NDEBUG
#define STBTT_STATIC
#define TESLA_INIT_IMPL

#include <ultra.hpp>
#include <tesla.hpp>
#include <utils.hpp>

// Overlay booleans
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
//static bool defaultMenuLoaded = true;
static bool freshSpawn = true;
//static bool refreshGui = false; (moved)
static bool reloadMenu = false;
static bool reloadMenu2 = false;
static bool reloadMenu3 = false;
static bool isDownloaded = false;

static bool redrawWidget = false;

static size_t nestedMenuCount = 0;

// Command mode globals
static std::vector<std::string> commandSystems = {defaultStr, eristaStr, marikoStr};
static std::vector<std::string> commandModes = {defaultStr, toggleStr, optionStr, forwarderStr};
static std::vector<std::string> commandGroupings = {defaultStr, "split", "split2", "split3", "split4"};
static std::string modePattern = ";mode=";
static std::string groupingPattern = ";grouping=";
static std::string systemPattern = ";system=";

static std::string defaultMenu = overlaysStr;

static std::string lastPage = leftStr;
static std::string lastPackage = "";
static std::string lastMenu = "";
static std::string lastMenuMode = "";
static std::string lastKeyName = "";

static std::unordered_map<std::string, std::string> selectedFooterDict;

static std::shared_ptr<tsl::elm::ListItem> selectedListItem;
static std::shared_ptr<tsl::elm::ListItem> lastSelectedListItem;

static bool lastRunningInterpreter = false;



static std::string hideUserGuide = falseStr;


// Command key defintitions
const static auto SCRIPT_KEY = KEY_MINUS;
const static auto SYSTEM_SETTINGS_KEY = KEY_PLUS;
const static auto SETTINGS_KEY = KEY_Y;
const static auto STAR_KEY = KEY_X;

template<typename Map, typename Func = std::function<std::string(const std::string&)>, typename... Args>
std::string getValueOrDefault(const Map& data, const std::string& key, const std::string& defaultValue, Func formatFunc = nullptr, Args... args) {
    auto it = data.find(key);
    if (it != data.end()) {
        return formatFunc ? formatFunc(it->second, args...) : it->second;
    }
    return defaultValue;
}


void clearMemory() {
    directoryCache.clear();
    hexSumCache.clear();
    selectedFooterDict.clear(); // Clears all data from the map, making it empty again
    selectedListItem.reset();
    lastSelectedListItem.reset();
}

void shiftItemFocus(auto& listItemPtr) {
    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemPtr.get(), tsl::FocusDirection::None);
}

void updateIniData(const std::map<std::string, std::map<std::string, std::string>>& packageConfigData,
                   const std::string& packageConfigIniPath,
                   const std::string& optionName,
                   const std::string& key,
                   std::string& value) {
    auto optionIt = packageConfigData.find(optionName);
    if (optionIt != packageConfigData.end()) {
        auto it = optionIt->second.find(key);
        if (it != optionIt->second.end()) {
            value = it->second;  // Update value only if the key exists
        } else {
            setIniFileValue(packageConfigIniPath, optionName, key, value); // Set INI file value if key not found
        }
    }
}



/**
 * @brief Handles updates and checks when the interpreter is running.
 *
 * This function processes the progression of download, unzip, and copy operations,
 * updates the user interface accordingly, and handles the thread failure and abort conditions.
 *
 * @param keysHeld A bitset representing keys that are held down.
 * @param stillTouching Boolean indicating if the touchscreen is being interacted with.
 * @param lastSelectedListItem Reference to the UI element displaying the current status.
 * @param commandSuccess Reference to a boolean tracking the overall command success.
 * @return `true` if the operation needs to abort, `false` otherwise.
 */
bool handleRunningInterpreter(uint64_t& keysHeld) {
    static int lastPercentage = -1;
    static bool inProgress = true;
    bool shouldAbort = false;
    int currentPercentage;

    if (downloadPercentage.load(std::memory_order_acquire) != -1) {
        currentPercentage = downloadPercentage.load(std::memory_order_acquire);
        if (currentPercentage != lastPercentage) {
            lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(currentPercentage) + "%");
            lastPercentage = currentPercentage;
        }
        if (currentPercentage == 100) {
            inProgress = true;
            downloadPercentage.store(-1, std::memory_order_release);
        }
    } else if (unzipPercentage.load(std::memory_order_acquire) != -1) {
        currentPercentage = unzipPercentage.load(std::memory_order_acquire);
        if (currentPercentage != lastPercentage) {
            lastSelectedListItem->setValue(UNZIP_SYMBOL + " " + std::to_string(currentPercentage) + "%");
            lastPercentage = currentPercentage;
        }
        if (currentPercentage == 100) {
            inProgress = true;
            unzipPercentage.store(-1, std::memory_order_release);
        }
    } else if (copyPercentage.load(std::memory_order_acquire) != -1) {
        currentPercentage = copyPercentage.load(std::memory_order_acquire);
        if (currentPercentage != lastPercentage) {
            lastSelectedListItem->setValue(COPY_SYMBOL + " " + std::to_string(currentPercentage) + "%");
            lastPercentage = currentPercentage;
        }
        if (currentPercentage == 100) {
            inProgress = true;
            copyPercentage.store(-1, std::memory_order_release);
        }
    } else if (lastPercentage == -1 && inProgress) {
        lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
        inProgress = false;
    }

    if (threadFailure.load(std::memory_order_acquire)) {
        threadFailure.store(false, std::memory_order_release);
        commandSuccess = false;
    }

    // Check for back button press
    if ((keysHeld & KEY_R) && !(keysHeld & (KEY_DLEFT | KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_ZL | KEY_ZR)) && !stillTouching) {
        commandSuccess = false;
        abortDownload.store(true, std::memory_order_release);
        abortUnzip.store(true, std::memory_order_release);
        abortFileOp.store(true, std::memory_order_release);
        abortCommand.store(true, std::memory_order_release);
        shouldAbort = true;
    }

    return shouldAbort;
}



// Forward declaration of the MainMenu class.
class MainMenu;

class UltrahandSettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, dropdownSelection, settingsIniPath;
    bool isInSection, inQuotes, isFromMainMenu;
    std::string languagesVersion = std::string(APP_VERSION);
    
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
        
        if (dropdownSelection.empty())
            inSettingsMenu = true;
        else
            inSubSettingsMenu = true;
        
        std::vector<std::string> defaultCombos = 
            {"ZL+ZR+DDOWN", "ZL+ZR+DRIGHT", "ZL+ZR+DUP", "ZL+ZR+DLEFT", "L+R+DDOWN", "L+R+DRIGHT", "L+R+DUP", "L+R+DLEFT", "ZL+ZR+PLUS", "L+R+PLUS", "MINUS+PLUS", "L+DDOWN+RS"};
        std::unordered_map<std::string, std::string> comboMap = {
            {"ZL+ZR+DDOWN", "\uE0E6+\uE0E7+\uE0EC"},
            {"ZL+ZR+DRIGHT", "\uE0E6+\uE0E7+\uE0EE"},
            {"ZL+ZR+DUP", "\uE0E6+\uE0E7+\uE0EB"},
            {"ZL+ZR+DLEFT", "\uE0E6+\uE0E7+\uE0ED"},
            {"L+R+DDOWN", "\uE0E4+\uE0E5+\uE0EC"},
            {"L+R+DRIGHT", "\uE0E4+\uE0E5+\uE0EE"},
            {"L+R+DUP", "\uE0E4+\uE0E5+\uE0EB"},
            {"L+R+DLEFT", "\uE0E4+\uE0E5+\uE0ED"},
            {"ZL+ZR+PLUS", "\uE0E6+\uE0E7+\uE0B5"},
            {"L+R+PLUS", "\uE0E4+\uE0E5+\uE0B5"},
            {"MINUS+PLUS", "\uE0B6+\uE0B5"},
            {"L+DDOWN+RS", "\uE0E4+\uE0EC+\uE0C5"}
        };
        std::vector<std::string> defaultLanguages = {"en", "es", "fr", "de", "ja", "ko", "it", "nl", "pt", "ru", "zh-cn", "zh-tw"};
        
        auto list = std::make_unique<tsl::elm::List>();
        
        if (dropdownSelection.empty()) {
            list->addItem(new tsl::elm::CategoryHeader(MAIN_SETTINGS));
            
            std::string fileContent = getFileContents(settingsConfigIniPath);
            
            std::string defaultLang = parseValueFromIniSection(settingsConfigIniPath, projectName, defaultLangStr);
            std::string keyCombo = trim(parseValueFromIniSection(settingsConfigIniPath, projectName, keyComboStr));
            
            
            if (defaultLang.empty())
                defaultLang = "en";
            if (keyCombo.empty())
                keyCombo = "ZL+ZR+DDOWN";
            
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(KEY_COMBO);
            listItem->setValue(comboMap[keyCombo]);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list

                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }

                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    tsl::changeTo<UltrahandSettingsMenu>("keyComboMenu");
                    selectedListItem.reset();
                    selectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            listItem = std::make_unique<tsl::elm::ListItem>(LANGUAGE);
            listItem->setValue(defaultLang);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    tsl::changeTo<UltrahandSettingsMenu>("languageMenu");
                    selectedListItem.reset();
                    selectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    return true;
                }

                return false;
            });
            list->addItem(listItem.release());
            
            
            listItem = std::make_unique<tsl::elm::ListItem>(SOFTWARE_UPDATE);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    tsl::changeTo<UltrahandSettingsMenu>("softwareUpdateMenu");
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            list->addItem(new tsl::elm::CategoryHeader(UI_SETTINGS));
            
            std::string currentTheme = parseValueFromIniSection(settingsConfigIniPath, projectName, "current_theme");
            if (currentTheme.empty() || currentTheme == defaultStr)
                currentTheme = DEFAULT;
            listItem = std::make_unique<tsl::elm::ListItem>(THEME);
            listItem->setValue(currentTheme);
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    tsl::changeTo<UltrahandSettingsMenu>("themeMenu");
                    selectedListItem.reset();
                    selectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            listItem = std::make_unique<tsl::elm::ListItem>(WIDGET);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    tsl::changeTo<UltrahandSettingsMenu>("widgetMenu");
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            listItem = std::make_unique<tsl::elm::ListItem>(MISCELLANEOUS);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    tsl::changeTo<UltrahandSettingsMenu>("miscMenu");
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());

        } else if (dropdownSelection == "keyComboMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(KEY_COMBO));
            
            std::string defaultCombo = trim(parseValueFromIniSection(settingsConfigIniPath, projectName, keyComboStr));
            
            std::unique_ptr<tsl::elm::ListItem> listItem;
            for (const auto& combo : defaultCombos) {
                
                listItem = std::make_unique<tsl::elm::ListItem>(comboMap[combo]);
                
                if (combo == defaultCombo) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                
                listItem->setClickListener([combo, mappedCombo=comboMap[combo], defaultCombo, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    
                    if (runningInterpreter.load(std::memory_order_acquire))
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        shiftItemFocus(listItemPtr);
                        if (combo != defaultCombo) {
                            setIniFileValue(settingsConfigIniPath, projectName, keyComboStr, combo);
                            setIniFileValue(teslaSettingsConfigIniPath, teslaStr, keyComboStr, combo);
                            reloadMenu = true;
                        }
                        
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(mappedCombo);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;
                        
                        simulatedSelectComplete = true;
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem.release());
            }
            
        } else if (dropdownSelection == "languageMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(LANGUAGE));
            
            std::string defaulLang = parseValueFromIniSection(settingsConfigIniPath, projectName, defaultLangStr);
            
            std::string langFile;
            bool skipLang;
            
            std::unique_ptr<tsl::elm::ListItem> listItem;
            for (const auto& defaultLangMode : defaultLanguages) {
                langFile = langPath+defaultLangMode+".json";
                skipLang = (!isFileOrDirectory(langFile));
                if (defaultLangMode != "en") {
                    if (skipLang)
                        continue;
                }
                
                listItem = std::make_unique<tsl::elm::ListItem>(defaultLangMode);
                
                if (defaultLangMode == defaulLang) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                
                listItem->setClickListener([skipLang, defaultLangMode, defaulLang, langFile, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    
                    if (runningInterpreter.load(std::memory_order_acquire))
                        return false;


                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        shiftItemFocus(listItemPtr);
                        //if (defaultLangMode != defaulLang) {
                        setIniFileValue(settingsConfigIniPath, projectName, defaultLangStr, defaultLangMode);
                        reloadMenu = true;
                        reloadMenu2 = true;
                        
                        parseLanguage(langFile);
                        
                        if (skipLang)
                            reinitializeLangVars();
                        
                        
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(defaultLangMode);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;
                        
                        simulatedSelectComplete = true;
                        
                        return true;
                    }
                    
                    return false;
                });
                
                list->addItem(listItem.release());
            }
            
        } else if (dropdownSelection == "softwareUpdateMenu") {
            list->addItem(new tsl::elm::CategoryHeader(SOFTWARE_UPDATE));
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(UPDATE_ULTRAHAND);
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                //static bool lastRunningInterpreter = false;
                if (runningInterpreter.load(std::memory_order_acquire)) {
                    return false;
                }

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }

                if ((keys & KEY_A)) {
                    isDownloadCommand = true;
                    std::vector<std::vector<std::string>> interpreterCommands = {
                        {"delete", "/config/ultrahand/downloads/ovlmenu.ovl"},
                        {"download", ultrahandRepo + "releases/latest/download/ovlmenu.ovl", downloadsPath},
                        {"move", "/config/ultrahand/downloads/ovlmenu.ovl", "/switch/.overlays/ovlmenu.ovl"}
                    };
                    runningInterpreter.store(true, std::memory_order_release);
                    enqueueInterpreterCommands(std::move(interpreterCommands), "", "");
                    startInterpreterThread();

                    listItemPtr->setValue(INPROGRESS_SYMBOL);

                    shiftItemFocus(listItemPtr);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = listItemPtr;
                    
                    
                    lastRunningInterpreter = true;
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            listItem = std::make_unique<tsl::elm::ListItem>(UPDATE_LANGUAGES);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                //static bool lastRunningInterpreter = false;

                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                
                if ((keys & KEY_A)) {
                    isDownloadCommand = true;
                    std::vector<std::vector<std::string>> interpreterCommands = {
                        {"delete", "/config/ultrahand/downloads/ovlmenu.ovl"},
                        {"download", ultrahandRepo + "releases/latest/download/lang.zip", downloadsPath},
                        {"unzip", "/config/ultrahand/downloads/lang.zip", "/config/ultrahand/downloads/lang/"},
                        {"delete", "/config/ultrahand/downloads/lang.zip"},
                        {"delete", langPath},
                        {"move", "/config/ultrahand/downloads/lang/", langPath}
                    };
                    runningInterpreter.store(true, std::memory_order_release);
                    enqueueInterpreterCommands(std::move(interpreterCommands), "", "");
                    startInterpreterThread();
                    //runningInterpreter.store(true, std::memory_order_release);
                    //lastRunningInterpreter = true;
                    listItemPtr->setValue(INPROGRESS_SYMBOL);

                    shiftItemFocus(listItemPtr);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = listItemPtr;
                    
                    
                    lastRunningInterpreter = true;
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            //list->addItem(new tsl::elm::CategoryHeader(OVERLAY_INFO));
            PackageHeader overlayHeader;
            overlayHeader.title = "Ultrahand Overlay";
            overlayHeader.version = std::string(APP_VERSION);
            overlayHeader.creator = "ppkantorski";
            overlayHeader.about = "Ultrahand Overlay is a versatile tool that enables you to create and share custom command-based packages.";
            overlayHeader.credits = "Special thanks to B3711, ComplexNarrative, Faker_dev, MasaGratoR, meha, WerWolv, HookedBehemoth and many others. <3";
            addAppInfo(list, overlayHeader, overlayStr);
            overlayHeader.clear(); // free memory
            
        } else if (dropdownSelection == "themeMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(THEME));
            
            std::string currentTheme = parseValueFromIniSection(settingsConfigIniPath, projectName, "current_theme");
            
            if (currentTheme.empty())
                currentTheme = defaultStr;
            
            std::vector<std::string> themeFilesList = getFilesListByWildcards(themesPath+"*.ini");
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(DEFAULT);
            
            std::string defaultTheme = themesPath+"default.ini";
            
            if (currentTheme == defaultStr) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem.reset();
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }
            
            listItem->setClickListener([defaultTheme, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list

                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }

                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);

                    //if (defaultLangMode != defaultLang) {
                    setIniFileValue(settingsConfigIniPath, projectName, "current_theme", defaultStr);
                    deleteFileOrDirectory(themeConfigIniPath);
                    
                    if (isFileOrDirectory(defaultTheme))
                        copyFileOrDirectory(defaultTheme, themeConfigIniPath);
                    else
                        initializeTheme(); // write default theme
                    
                    reloadMenu = true;
                    reloadMenu2 = true;
                    
                    lastSelectedListItem->setValue("");
                    selectedListItem->setValue(DEFAULT);
                    listItemPtr->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = listItemPtr;
                    
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            
            list->addItem(listItem.release());
            
            std::string themeName;
            
            sort(themeFilesList.begin(), themeFilesList.end());
            
            for (const auto& themeFile : themeFilesList) {
                themeName = dropExtension(getNameFromPath(themeFile));
                
                if (themeName == defaultStr)
                    continue;
                
                listItem = std::make_unique<tsl::elm::ListItem>(themeName);
                
                if (themeName == currentTheme) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                
                listItem->setClickListener([themeName, currentTheme, themeFile, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    
                    if (runningInterpreter.load(std::memory_order_acquire))
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }

                    if (keys & KEY_A) {
                        shiftItemFocus(listItemPtr);
                        //if (defaultLangMode != defaultLang) {
                        setIniFileValue(settingsConfigIniPath, projectName, "current_theme", themeName);
                        deleteFileOrDirectory(themeConfigIniPath);
                        copyFileOrDirectory(themeFile, themeConfigIniPath);
                        
                        initializeTheme();
                        
                        reloadMenu = true;
                        reloadMenu2 = true;
                        
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(themeName);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;
                        
                        simulatedSelectComplete = true;
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem.release());
            }
        } else if (dropdownSelection == "widgetMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(WIDGET));
            
            auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(CLOCK, false, ON, OFF);
            toggleListItem->setState((hideClock == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_clock", state ? falseStr : trueStr);
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(SOC_TEMPERATURE, false, ON, OFF);
            toggleListItem->setState((hideSOCTemp == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_soc_temp", state ? falseStr : trueStr);
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(PCB_TEMPERATURE, false, ON, OFF);
            toggleListItem->setState((hidePCBTemp == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_pcb_temp", state ? falseStr : trueStr);
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(BATTERY, false, ON, OFF);
            toggleListItem->setState((hideBattery == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_battery", state ? falseStr : trueStr);
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            
        } else if (dropdownSelection == "miscMenu") {
            list->addItem(new tsl::elm::CategoryHeader(MENU_ITEMS));
            hideUserGuide = parseValueFromIniSection(settingsConfigIniPath, projectName, "hide_user_guide");
            
            auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(USER_GUIDE, false, ON, OFF);
            toggleListItem->setState((hideUserGuide == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_user_guide", state ? falseStr : trueStr);
                if ((hideUserGuide == falseStr) != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem.release());
            
            
            
            cleanVersionLabels = parseValueFromIniSection(settingsConfigIniPath, projectName, "clean_version_labels");
            hideOverlayVersions = parseValueFromIniSection(settingsConfigIniPath, projectName, "hide_overlay_versions");
            hidePackageVersions = parseValueFromIniSection(settingsConfigIniPath, projectName, "hide_package_versions");
            
            if (cleanVersionLabels.empty())
                cleanVersionLabels = falseStr;
            if (hideOverlayVersions.empty())
                hideOverlayVersions = falseStr;
            if (hidePackageVersions.empty())
                hidePackageVersions = falseStr;
            
            list->addItem(new tsl::elm::CategoryHeader(VERSION_LABELS));
            
            std::string defaulLang = parseValueFromIniSection(settingsConfigIniPath, projectName, defaultLangStr);
            
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(CLEAN_LABELS, false, ON, OFF);
            toggleListItem->setState((cleanVersionLabels == trueStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "clean_version_labels", state ? trueStr : falseStr);
                if ((cleanVersionLabels == trueStr) != state) {
                    versionLabel = APP_VERSION + std::string("   (") + extractTitle(loaderInfo) + (cleanVersionLabels == falseStr ? " " : " v") + cleanVersionLabel(loaderInfo) + std::string(")");
                    reinitializeVersionLabels();
                    reloadMenu2 = true;
                    reloadMenu = true;
                }
                
            });
            list->addItem(toggleListItem.release());
            
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(OVERLAY_LABELS, false, ON, OFF);
            toggleListItem->setState((hideOverlayVersions == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_overlay_versions", state ? falseStr : trueStr);
                if ((hideOverlayVersions == falseStr) != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(PACKAGE_LABELS, false, ON, OFF);
            toggleListItem->setState((hidePackageVersions == falseStr));
            toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsConfigIniPath, projectName, "hide_package_versions", state ? falseStr : trueStr);
                if ((hidePackageVersions == falseStr) != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem.release());
            
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN + ": " + settingsIniPath));
        

        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(upperProjectName, versionLabel);
        rootFrame->setContent(list.release());

        return rootFrame.release();
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
        
        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
            while (!interpreterThreadExit.load()) {svcSleepThread(50'000'000);}

            resetPercentages();

            isDownloadCommand = false;
            lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }


        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {

                if (reloadMenu3) {
                    tsl::goBack();
                    tsl::changeTo<UltrahandSettingsMenu>();
                    reloadMenu3 = false;
                }

                if (simulatedNextPage && !simulatedNextPageComplete) {
                    simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }

                if (simulatedMenu && !simulatedMenuComplete) {
                    simulatedMenu = false;
                    simulatedMenuComplete = true;
                }

                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }

                if ((keysHeld & KEY_B) && !stillTouching) {
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

                    simulatedBackComplete = true;
                    //tsl::Overlay::get()->close();
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {

            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }

            if ((keysHeld & KEY_B) && !stillTouching) {
                inSubSettingsMenu = false;
                returningToSettings = true;
                tsl::goBack();
                
                if (reloadMenu2) {
                    tsl::goBack();
                    tsl::changeTo<UltrahandSettingsMenu>();
                    reloadMenu2 = false;
                }
                //tsl::Overlay::get()->close();
                simulatedBackComplete = true;
                return true;
            }

        }
        
        if (returningToSettings && !(keysHeld & KEY_B)){
            returningToSettings = false;
            inSettingsMenu = true;
        }
        
        if (redrawWidget)
            reinitializeWidgetVars();


        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
        //svcSleepThread(10'000'000);
        return false;
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
        if (entryMode == overlayStr) {
            settingsIniPath = overlaysIniFilePath;
            header = overlayName;
        } else if (entryMode == packageStr)
            settingsIniPath = packagesIniFilePath;
        
        if (dropdownSelection.empty())
            inSettingsMenu = true;
        else
            inSubSettingsMenu = true;
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();
        
        if (dropdownSelection.empty()) {
            list->addItem(new tsl::elm::CategoryHeader(header+" "+SETTINGS));
            
            
            std::string fileContent = getFileContents(settingsIniPath);
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, priorityStr);
            
            std::string hideOption = parseValueFromIniSection(settingsIniPath, entryName, hideStr);
            if (hideOption.empty())
                hideOption = falseStr;
            
            
            bool hide = false;
            if (hideOption == trueStr)
                hide = true;
            
            std::string useOverlayLaunchArgs = parseValueFromIniSection(settingsIniPath, entryName, useLaunchArgsStr);
            
            
            // Capitalize entryMode
            std::string hideLabel = entryMode;
            
            
            if (hideLabel == overlayStr)
                hideLabel = HIDE_OVERLAY;
            else if (hideLabel == packageStr)
                hideLabel = HIDE_PACKAGE;
            
            
            // Envoke toggling
            auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(hideLabel, false, ON, OFF);
            toggleListItem->setState(hide);
            toggleListItem->setStateChangedListener([&settingsIniPath = this->settingsIniPath, &entryName = this->entryName, listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsIniPath, entryName, hideStr, state ? trueStr : falseStr);
                if (state)
                    reloadMenu = true; // this reloads before main menu
                else {
                    reloadMenu = true;
                    reloadMenu2 = true; // this reloads at main menu
                }
            });
            list->addItem(toggleListItem.release());
            
            
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(SORT_PRIORITY);
            listItem->setValue(priorityValue);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([&entryName = this->entryName, &entryMode = this->entryMode, &overlayName = this->overlayName, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    tsl::changeTo<SettingsMenu>(entryName, entryMode, overlayName, priorityStr);
                    selectedListItem.reset();
                    selectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            if (entryMode == overlayStr) {
                // Envoke toggling
                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(LAUNCH_ARGUMENTS, false, ON, OFF);
                toggleListItem->setState((useOverlayLaunchArgs==trueStr));
                toggleListItem->setStateChangedListener([&settingsIniPath = settingsIniPath, &entryName = entryName, useOverlayLaunchArgs, listItemRaw = toggleListItem.get()](bool state) {
                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                    setIniFileValue(settingsIniPath, entryName, useLaunchArgsStr, state ? trueStr : falseStr);
                    if ((useOverlayLaunchArgs==trueStr) != state)
                        reloadMenu = true; // this reloads before main menu
                    if (!state) {
                        reloadMenu = true;
                        reloadMenu2 = true; // this reloads at main menu
                    }
                });
                list->addItem(toggleListItem.release());
            }
            
            
        } else if (dropdownSelection == priorityStr) {
            list->addItem(new tsl::elm::CategoryHeader(SORT_PRIORITY));
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, priorityStr);
            
            std::unique_ptr<tsl::elm::ListItem> listItem;

            std::string iStr;
            for (int i = 0; i <= MAX_PRIORITY; ++i) { // for i in range 0->20 with 20 being the max value
                iStr = std::to_string(i);
                listItem = std::make_unique<tsl::elm::ListItem>(iStr);
                
                if (iStr == priorityValue) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                
                listItem->setClickListener([&settingsIniPath = this->settingsIniPath, &entryName = this->entryName, iStr, priorityValue, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    
                    if (runningInterpreter.load(std::memory_order_acquire))
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        shiftItemFocus(listItemPtr);
                        if (iStr != priorityValue)
                            reloadMenu = true;
                        setIniFileValue(settingsIniPath, entryName, priorityStr, iStr);
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(iStr);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;
                        simulatedSelectComplete = true;
                        return true;
                    }
                    return false;
                });
                
                list->addItem(listItem.release());
            }
            
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN+": " + settingsIniPath));
        

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel);
        
        //auto rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel);
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(upperProjectName, versionLabel);
        rootFrame->setContent(list.release());
        
        return rootFrame.release();
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

        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
            while (!interpreterThreadExit.load()) {svcSleepThread(50'000'000);}
            
            resetPercentages();
            
            isDownloadCommand = false;
            lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                if (simulatedNextPage && !simulatedNextPageComplete) {
                    simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }

                if (simulatedMenu && !simulatedMenuComplete) {
                    simulatedMenu = false;
                    simulatedMenuComplete = true;
                }

                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }

                if ((keysHeld & KEY_B) && !stillTouching) {
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
                    simulatedBackComplete = true;
                    //tsl::Overlay::get()->close();
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {
            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }

            if ((keysHeld & KEY_B) && !stillTouching) {
                inSubSettingsMenu = false;
                returningToSettings = true;
                tsl::goBack();
                //tsl::Overlay::get()->close();
                simulatedBackComplete = true;
                return true;
            }
        }
        
        
        if (returningToSettings && !(keysHeld & KEY_B)){
            returningToSettings = false;
            inSettingsMenu = true;
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
        return false;
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
    std::string fileName;

public:
    /**
     * @brief Constructs a `ScriptOverlay` instance.
     *
     * Initializes a new instance of the `ScriptOverlay` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     */
    ScriptOverlay(const std::string& file, const std::string& key = "", const bool& fromMainMenu=false, const std::string& _fileName = packageFileName) : filePath(file), specificKey(key), isFromMainMenu(fromMainMenu), fileName(_fileName) {}
    
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
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();
        
        const std::string& packageFile = filePath + fileName;
        const std::string& fileContent = getFileContents(packageFile);
        
        if (!fileContent.empty()) {
            std::string line;
            std::istringstream iss(fileContent);
            std::string currentCategory;
            isInSection = false;
            std::unique_ptr<tsl::elm::ListItem> listItem;
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
                    listItem = std::make_unique<tsl::elm::ListItem>(line);
                    listItem->setClickListener([&inQuotes = this->inQuotes, &filePath = this->filePath, &specificKey = this->specificKey, line, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                        
                        if (runningInterpreter.load(std::memory_order_acquire))
                            return false;
                        if (simulatedSelect && !simulatedSelectComplete) {
                            keys |= KEY_A;
                            simulatedSelect = false;
                        }
                        if (keys & KEY_A) {
                            std::istringstream iss(line);
                            std::string part;
                            std::vector<std::vector<std::string>> commandVec;
                            std::vector<std::string> commandParts;
                            inQuotes = false;
                            
                            std::istringstream argIss;  // Declare outside to avoid re-construction
                            std::string arg;            // Declare outside to reuse memory
                            
                            while (std::getline(iss, part, '\'')) {
                                if (!part.empty()) {
                                    if (!inQuotes) {
                                        argIss.clear();        // Clear any error flags and existing content
                                        argIss.str(part);      // Set new string to be processed
                                        while (argIss >> arg) {
                                            commandParts.emplace_back(arg);
                                        }
                                    } else {
                                        commandParts.emplace_back(part);
                                    }
                                }
                                inQuotes = !inQuotes;  // Toggle the inQuotes state
                            }
                            
                            commandVec.emplace_back(std::move(commandParts));
                            
                            interpretAndExecuteCommands(std::move(commandVec), filePath, specificKey);
                            commandVec.clear();
                            
                            if (commandSuccess)
                                listItemPtr->setValue(CHECKMARK_SYMBOL);
                            else
                                listItemPtr->setValue(CROSSMARK_SYMBOL);

                            simulatedSelectComplete = true;
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem.release());
                }
            }
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN+": " + packageFile));
        
        PackageHeader packageHeader = getPackageHeaderFromIni(packageFile);

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame(packageName, "Ultrahand Script");
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(packageName, packageHeader.version != "" ? packageHeader.version + "   (" + upperProjectName + " Script)" : upperProjectName + " Script");
        rootFrame->setContent(list.release());
        //rootFrame->setContent(list);

        //list = nullptr;
        return rootFrame.release();
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
        
        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
            while (!interpreterThreadExit.load()) {svcSleepThread(50'000'000);}
            
            resetPercentages();
            
            isDownloadCommand = false;
            lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        if (inScriptMenu) {
            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }

            if ((keysHeld & KEY_B) && !stillTouching) {
                inScriptMenu = false;
                if (isFromMainMenu == false) {
                    if (lastMenu == "packageMenu")
                        returningToPackage = true;
                    else if (lastMenu == "subPackageMenu")
                        returningToSubPackage = true;
                } else
                    returningToMain = true;
                tsl::goBack();
                simulatedBackComplete = true;
                return true;
            }
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
        return false;
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
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, groupingName, lastGroupingName;
    std::vector<std::vector<std::string>> commands;
    std::vector<std::vector<std::string>> commandsOn;
    std::vector<std::vector<std::string>> commandsOff;
    std::string specifiedFooterKey;
    bool toggleState = false;
    std::string packageConfigIniPath;
    std::string commandSystem, commandMode, commandGrouping;
    
    std::string lastSelectedListItemFooter = "";
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
            //lastSelectedListItem = nullptr;
            lastSelectedListItem.reset();
        }
    /**
     * @brief Destroys the `SelectionOverlay` instance.
     *
     * Cleans up any resources associated with the `SelectionOverlay` instance.
     */
    ~SelectionOverlay() {
        lastSelectedListItem.reset();
        //lastSelectedListItem = nullptr;
    }
    
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
        std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();
        
        packageConfigIniPath = filePath + configFileName;
        
        commandSystem = commandSystems[0];
        commandMode = commandModes[0];
        commandGrouping = commandGroupings[0];
        
        std::string currentSection = globalStr;
        std::string sourceType = defaultStr, sourceTypeOn = defaultStr, sourceTypeOff = defaultStr; 
        std::string jsonPath, jsonPathOn, jsonPathOff;
        std::string jsonKey, jsonKeyOn, jsonKeyOff;
        
        
        std::string listString, listStringOn, listStringOff;
        //std::vector<std::string> listData, listDataOn, listDataOff;
        std::string jsonString, jsonStringOn, jsonStringOff;
        std::string commandName;
        
        bool inEristaSection = false;
        bool inMarikoSection = false;
        
        // Remove all empty command strings
        commands.erase(std::remove_if(commands.begin(), commands.end(),
            [](const std::vector<std::string>& vec) {
                return vec.empty(); // Check if the vector is empty
                // Add other conditions as necessary
            }),
            commands.end());

        // initial processing of commands
        for (const auto& cmd : commands) {
            //if (cmd.empty()) { // Isolate command settings
            //    continue;
            //}
            
            commandName = cmd[0];
            
            if (stringToLowercase(commandName) == "erista:") {
                inEristaSection = true;
                inMarikoSection = false;
                continue;
            } else if (stringToLowercase(commandName) == "mariko:") {
                inEristaSection = false;
                inMarikoSection = true;
                continue;
            }
            
            if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
                
                
                if (commandName.find(systemPattern) == 0) {// Extract the command system
                    commandSystem = commandName.substr(systemPattern.length());
                    if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                        commandSystem = commandSystems[0]; // reset to default if commandMode is unknown
                } else if (commandName.find(modePattern) == 0) { // Extract the command mode
                    commandMode = commandName.substr(modePattern.length());
                    if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                        commandMode = commandModes[0]; // reset to default if commandMode is unknown
                } else if (commandName.find(groupingPattern) == 0) {// Extract the command grouping
                    commandGrouping = commandName.substr(groupingPattern.length());
                    if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                        commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                }
                
                // Extract the command grouping
                if (commandMode == toggleStr) {
                    if (commandName == "on:")
                        currentSection = lowerOnStr;
                    else if (commandName == "off:")
                        currentSection = lowerOffStr;
                    
                    // Seperation of command chuncks
                    if (currentSection == globalStr) {
                        commandsOn.push_back(cmd);
                        commandsOff.push_back(cmd);
                    } else if (currentSection == lowerOnStr)
                        commandsOn.push_back(cmd);
                    else if (currentSection == lowerOffStr)
                        commandsOff.push_back(cmd);
                }
                
                if (cmd.size() > 1) { // Pre-process advanced commands
                    if (commandName == "filter") {
                        if (currentSection == globalStr)
                            filterList.push_back(cmd[1]);
                        else if (currentSection == lowerOnStr)
                            filterListOn.push_back(cmd[1]);
                        else if (currentSection == lowerOffStr)
                            filterListOff.push_back(cmd[1]);
                    } else if (commandName == "file_source") {
                        sourceType = fileStr;
                        if (currentSection == globalStr) {
                            pathPattern = cmd[1];
                            filesList = getFilesListByWildcards(pathPattern);
                        } else if (currentSection == lowerOnStr) {
                            pathPatternOn = cmd[1];
                            filesListOn = getFilesListByWildcards(pathPatternOn);
                            sourceTypeOn = fileStr;
                        } else if (currentSection == lowerOffStr) {
                            pathPatternOff = cmd[1];
                            filesListOff = getFilesListByWildcards(pathPatternOff);
                            sourceTypeOff = fileStr;
                        }
                    } else if (commandName == "json_file_source") {
                        sourceType = jsonFileStr;
                        if (currentSection == globalStr) {
                            jsonPath = preprocessPath(cmd[1], filePath);
                            if (cmd.size() > 2)
                                jsonKey = cmd[2]; //json display key
                        } else if (currentSection == lowerOnStr) {
                            jsonPathOn = preprocessPath(cmd[1], filePath);
                            sourceTypeOn = jsonFileStr;
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2]; //json display key
                        } else if (currentSection == lowerOffStr) {
                            jsonPathOff = preprocessPath(cmd[1], filePath);
                            sourceTypeOff = jsonFileStr;
                            if (cmd.size() > 2)
                                jsonKeyOff = cmd[2]; //json display key
                        }
                    } else if (commandName == "list_source") {
                        sourceType = _listStr;
                        if (currentSection == globalStr) {
                            listString = removeQuotes(cmd[1]);
                        } else if (currentSection == lowerOnStr) {
                            listStringOn = removeQuotes(cmd[1]);
                            sourceTypeOn = _listStr;
                        } else if (currentSection == lowerOffStr) {
                            listStringOff = removeQuotes(cmd[1]);
                            sourceTypeOff = _listStr;
                        }
                    } else if (commandName == "json_source") {
                        sourceType = _jsonStr;
                        if (currentSection == globalStr) {
                            jsonString = removeQuotes(cmd[1]); // convert string to jsonData
                            
                            if (cmd.size() > 2)
                                jsonKey = cmd[2]; //json display key
                        } else if (currentSection == lowerOnStr) {
                            jsonStringOn = removeQuotes(cmd[1]); // convert string to jsonData
                            sourceTypeOn = _jsonStr;
                            
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2]; //json display key
                            
                        } else if (currentSection == lowerOffStr) {
                            jsonStringOff = removeQuotes(cmd[1]); // convert string to jsonData
                            sourceTypeOff = _jsonStr;
                            
                            if (cmd.size() > 2)
                                jsonKeyOff = cmd[2]; //json display key
                        }
                    }
                }
            }
        }
        
        
        
        // items can be paths, commands, or variables depending on source
        std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;
        
        // Get the list of files matching the pattern
        if (commandMode == defaultStr || commandMode == optionStr) {
            if (sourceType == fileStr)
                selectedItemsList = filesList;
            else if (sourceType == _listStr)
                selectedItemsList = stringToList(listString);
            else if ((sourceType == _jsonStr) || (sourceType == jsonFileStr)) {
                populateSelectedItemsList(sourceType, (sourceType == _jsonStr) ? jsonString : jsonPath, jsonKey, selectedItemsList);
                jsonPath = "";
                jsonString = "";
            }
        } else if (commandMode == toggleStr) {
            if (sourceTypeOn == fileStr)
                selectedItemsListOn = filesListOn;
            else if (sourceTypeOn == _listStr)
                selectedItemsListOn = stringToList(listStringOn);
            else if ((sourceTypeOn == _jsonStr) || (sourceTypeOn == jsonFileStr)) {
                populateSelectedItemsList(sourceTypeOn, (sourceTypeOn == _jsonStr) ? jsonStringOn : jsonPathOn, jsonKeyOn, selectedItemsListOn);
                jsonPathOn = "";
                jsonStringOn = "";
                
            }
            
            if (sourceTypeOff == fileStr)
                selectedItemsListOff = filesListOff;
            else if (sourceTypeOff == _listStr)
                selectedItemsListOff = stringToList(listStringOff);
            else if ((sourceTypeOff == _jsonStr) || (sourceTypeOff == jsonFileStr)) {
                populateSelectedItemsList(sourceTypeOff, (sourceTypeOff == _jsonStr) ? jsonStringOff : jsonPathOff, jsonKeyOff, selectedItemsListOff);
                jsonPathOff = "";
                jsonStringOff = "";
            }
            
            
            // Apply On Filter
            filterItemsList(filterListOn, selectedItemsListOn);
            filterListOn.clear();
            
            // Apply Off Filter
            filterItemsList(filterListOff, selectedItemsListOff);
            filterListOff.clear();
            
            selectedItemsList.reserve(selectedItemsListOn.size() + selectedItemsListOff.size());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOn.begin(), selectedItemsListOn.end());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOff.begin(), selectedItemsListOff.end());
            
            
            // WARNING: This assumes items list is a path list. (May need a long term solution still.)
            if (sourceType == fileStr && (commandGrouping == "split" || commandGrouping == "split2" || commandGrouping == "split3" || commandGrouping == "split4")) {
                
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    const std::string& parentDirA = getParentDirNameFromPath(a);
                    const std::string& parentDirB = getParentDirNameFromPath(b);
                    
                    return (parentDirA != parentDirB) ? (parentDirA < parentDirB) : (getNameFromPath(a) < getNameFromPath(b));

                });
            } else {
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    return getNameFromPath(a) < getNameFromPath(b);
                });
            }
        }
        
        // Apply filter to selectedItemsList
        filterItemsList(filterList, selectedItemsList);
        filterList.clear();
        
        if (commandGrouping == defaultStr)
            list->addItem(new tsl::elm::CategoryHeader(removeTag(specificKey.substr(1)))); // remove * from key
        
        // initialize variables
        std::unique_ptr<tsl::elm::ListItem> listItem;
        size_t pos;
        std::string parentDirName;
        std::string footer;
        std::string optionName;
        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>("", true, "", "");
        bool toggleStateOn;
        
        if (selectedItemsList.empty()){
            listItem = std::make_unique<tsl::elm::ListItem>("Empty");
            list->addItem(listItem.release());
        }

        // Add each file as a menu item
        for (size_t i = 0; i < selectedItemsList.size(); ++i) {
            const std::string& selectedItem = selectedItemsList[i];
            
            // For entries that are paths
            itemName = getNameFromPath(selectedItem);
            if (itemName.front()=='.') // Skip hidden items
                continue;

            if (!isDirectory(preprocessPath(selectedItem, filePath)))
                itemName = dropExtension(itemName);
            
            if (sourceType == fileStr) {
                if (commandGrouping == "split") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem));
                    
                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)){
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
                else if (commandGrouping == "split2") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem));

                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3); // Assign the part after " - " as the footer
                        groupingName = groupingName.substr(0, pos); // Strip the " - " and everything after it
                    }

                    
                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)){
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
                else if (commandGrouping == "split3") {
                    groupingName = removeQuotes(getNameFromPath(selectedItem));

                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3); // Assign the part after " - " as the footer
                        groupingName = groupingName.substr(0, pos); // Strip the " - " and everything after it
                    }

                    
                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)){
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
                else if (commandGrouping == "split4") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem, 2));
                    itemName = trim(removeQuotes(dropExtension(getNameFromPath(selectedItem))));
                    footer = removeQuotes(getParentDirNameFromPath(selectedItem));

                    
                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)){
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
            }
            
            
            if (commandMode == defaultStr || commandMode == optionStr) { // for handiling toggles

                if (sourceType != fileStr && commandGrouping != "split2" && commandGrouping != "split3" && commandGrouping != "split4") {
                    pos = selectedItem.find(" - ");
                    footer = "";
                    itemName = selectedItem;
                    if (pos != std::string::npos) {
                        footer = selectedItem.substr(pos + 2); // Assign the part after " - " as the footer
                        itemName = selectedItem.substr(0, pos); // Strip the " - " and everything after it
                    }
                } else if (commandGrouping == "split2") {
                    footer = dropExtension(getNameFromPath(selectedItem));
                }


                listItem = std::make_unique<tsl::elm::ListItem>(itemName);
                
                if (commandMode == optionStr) {
                    if (selectedFooterDict[specifiedFooterKey] == itemName) {
                        lastSelectedListItem.reset();
                        lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                        lastSelectedListItemFooter = footer;
                        listItem->setValue(CHECKMARK_SYMBOL);
                    } else {
                        if (pos != std::string::npos) {
                            listItem->setValue(footer, true);
                        } else {
                            listItem->setValue(footer);
                        }
                    }
                } else
                    listItem->setValue(footer, true);
                
                //
                
                //auto& commands = this->commands; // Assuming 'commands' is a member of the class
                listItem->setClickListener([&commands = this->commands, &filePath = this->filePath, &specificKey = this->specificKey, &commandMode = this->commandMode,
                    &specifiedFooterKey = this->specifiedFooterKey, &lastSelectedListItemFooter = this->lastSelectedListItemFooter, i, footer, selectedItem, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {

                    //static bool lastRunningInterpreter = false;
                    if (runningInterpreter.load(std::memory_order_acquire)) {
                        return false;
                    }

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }

                    if ((keys & KEY_A)) {
                        isDownloadCommand = false;
                        runningInterpreter.store(true, std::memory_order_release);
                        enqueueInterpreterCommands(getSourceReplacement(commands, selectedItem, i, filePath), filePath, specificKey);
                        startInterpreterThread();
                        
                        listItemPtr->setValue(INPROGRESS_SYMBOL);

                        shiftItemFocus(listItemPtr);
                        if (commandMode == optionStr) {
                            selectedFooterDict[specifiedFooterKey] = listItemPtr->getText();
                            if (lastSelectedListItem)
                                lastSelectedListItem->setValue(lastSelectedListItemFooter, true);
                            lastSelectedListItemFooter = footer;
                        }

                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;
                        

                        lastRunningInterpreter = true;
                        simulatedSelectComplete = true;
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());
                
            } else if (commandMode == toggleStr) {
                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(itemName, false, ON, OFF);
                
                // Set the initial state of the toggle item
                toggleStateOn = std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end();
                toggleListItem->setState(toggleStateOn);
                
                toggleListItem->setStateChangedListener([&commandsOn = this->commandsOn, &commandsOff = this->commandsOff, &filePath = this->filePath,
                    &specificKey = this->specificKey, i, selectedItem, listItemRaw = toggleListItem.get()](bool state) {

                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                    interpretAndExecuteCommands(getSourceReplacement(!state ? commandsOn : commandsOff, selectedItem, i, filePath), filePath, specificKey);
                });
                list->addItem(toggleListItem.release());
            }
        }
        
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(filePath), packageHeader.version != "" ? packageHeader.version + "   (Ultrahand Package)" : "Ultrahand Package", "", packageHeader.color);
        rootFrame->setContent(list.release());

        return rootFrame.release();
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

        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
            while (!interpreterThreadExit.load()) {svcSleepThread(50'000'000);}
            
            resetPercentages();
            
            isDownloadCommand = false;
            lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        if (refreshGui && !stillTouching) {
            tsl::goBack();
            tsl::changeTo<SelectionOverlay>(filePath, specificKey, commands, specifiedFooterKey);
            refreshGui = false;
        }
        
        if (inSelectionMenu) {
            
            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }

            if ((keysHeld & KEY_B) && !stillTouching) {
                inSelectionMenu = false;
                
                if (lastMenu == "packageMenu")
                    returningToPackage = true;
                else if (lastMenu == "subPackageMenu")
                    returningToSubPackage = true;
                
                if (commandMode == optionStr && isFileOrDirectory(packageConfigIniPath)) {
                    auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    auto it = packageConfigData.find(specificKey);
                    if (it != packageConfigData.end()) {
                        auto& optionSection = it->second;
                        auto footerIt = optionSection.find(footerStr);
                        if (footerIt != optionSection.end() && footerIt->second != nullStr) {
                            selectedListItem->setValue(footerIt->second);
                        }
                    }
                }
                
                
                tsl::goBack();
                simulatedBackComplete = true;
                return true;
            }
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
        //svcSleepThread(10'000'000);
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
    std::string packagePath, dropdownSection, currentPage, pathReplace, pathReplaceOn, pathReplaceOff;
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    bool usingPages = false;
    std::string packageName;
public:
    /**
     * @brief Constructs a `PackageMenu` instance for a specific sub-menu path.
     *
     * Initializes a new instance of the `PackageMenu` class for the given sub-menu path.
     *
     * @param path The path to the sub-menu.
     */
    PackageMenu(const std::string& path, const std::string& sectionName = "", const std::string& page = leftStr, const std::string& _packageName = packageFileName) : packagePath(path), dropdownSection(sectionName), currentPage(page), packageName(_packageName) {}
    /**
     * @brief Destroys the `PackageMenu` instance.
     *
     * Cleans up any resources associated with the `PackageMenu` instance.
     */
    ~PackageMenu() {
        if (returningToMain) {
            clearMemory();
        }
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
        
        tsl::hlp::ini::IniData packageConfigData;
        

        std::string packageIniPath = packagePath + packageName;
        std::string packageConfigIniPath = packagePath + configFileName;

        PackageHeader packageHeader = getPackageHeaderFromIni(packageIniPath);
        
        //rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color);
        //tsl::elm::List* list = new tsl::elm::List();
        auto list = std::make_unique<tsl::elm::List>();
        std::unique_ptr<tsl::elm::ListItem> listItem;
        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>("", true, "", "");
        bool toggleStateOn;
        
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath);
        
        
        bool skipSection = false;
        bool skipSystem = false;

        
        std::string lastSection = "";
        std::string pageLeftName = "";
        std::string pageRightName = "";
        std::string drawLocation = "";
        
        std::string commandName;
        std::string commandFooter;
        std::string commandSystem;
        std::string commandMode;
        std::string commandGrouping;
        
        std::string currentSection;

        std::string defaultToggleState;
        std::string sourceType, sourceTypeOn, sourceTypeOff;
        
        std::string packageSource;

        std::string jsonPath, jsonPathOn, jsonPathOff;
        std::string jsonKey, jsonKeyOn, jsonKeyOff;
        
        std::string optionName;
        std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
        //std::vector<std::string> listData, listDataOn, listDataOff;
        
        std::string footer;
        bool useSelection;
        size_t pos;
        
        bool inEristaSection;
        bool inMarikoSection;
        
        
        for (size_t i = 0; i < options.size(); ++i) {
            auto& option = options[i];
            
            optionName = option.first;
            commands = std::move(option.second);
            
            footer = "";
            useSelection = false;
            
            commandFooter = nullStr;
            commandSystem = defaultStr;
            commandMode = defaultStr;
            commandGrouping = defaultStr;
            
            defaultToggleState = "";

            currentSection = globalStr;
            sourceType = defaultStr;
            sourceTypeOn = defaultStr;
            sourceTypeOff = defaultStr;
            
            
            commandsOn.clear();
            commandsOff.clear();
            
            
            if (drawLocation.empty() || (currentPage == drawLocation) || (optionName.front() == '@')) {
                
                // Custom header implementation
                if (!dropdownSection.empty()) {
                    if (i == 0) {
                        // Add a section break with small text to indicate the "Commands" section
                        list->addItem(new tsl::elm::CategoryHeader(removeTag(dropdownSection.substr(1))));
                        skipSection = true;
                        lastSection = dropdownSection;
                    }
                    if (removeTag(optionName) == PACKAGE_INFO || removeTag(optionName) == "Package Info") {
                        if (!skipSection) {
                            lastSection = optionName;
                            addAppInfo(list, packageHeader);
                        }
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
                        if (optionName.front() == '@') {
                            if (drawLocation.empty()) {
                                pageLeftName = optionName.substr(1);
                                drawLocation = leftStr;
                            } else {
                                pageRightName = optionName.substr(1);
                                usingPages = true;
                                drawLocation = rightStr;
                            }
                        } else if (optionName.front() == '*') {
                            // Create reference to PackageMenu with dropdownSection set to optionName
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName.substr(1)), DROPDOWN_SYMBOL);
                            
                            listItem->setClickListener([&packagePath=this->packagePath, &currentPage=this->currentPage, &packageName=this->packageName, optionName](s64 keys) {
                                
                                if (runningInterpreter.load(std::memory_order_acquire))
                                    return false;
                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }
                                if (keys & KEY_A) {
                                    inPackageMenu = false;
                                    selectedListItem.reset();
                                    lastSelectedListItem.reset();
                                    tsl::changeTo<PackageMenu>(packagePath, optionName, currentPage, packageName);
                                    simulatedSelectComplete = true;
                                    return true;
                                }
                                return false;
                            });
                            list->addItem(listItem.release());
                            
                            
                            skipSection = true;
                        } else {
                            if (optionName != lastSection) {
                                
                                if (removeTag(optionName) == PACKAGE_INFO || removeTag(optionName) == "Package Info") {
                                    //logMessage("pre-before adding app info");
                                    if (!skipSection) {
                                        lastSection = optionName;
                                        //logMessage("before adding app info");
                                        addAppInfo(list, packageHeader);
                                        //logMessage("after adding app info");
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
                
                
                // initial processing of commands
                inEristaSection = false;
                inMarikoSection = false;
                size_t delimiterPos;

                // Remove all empty command strings
                commands.erase(std::remove_if(commands.begin(), commands.end(),
                    [](const std::vector<std::string>& vec) {
                        return vec.empty(); // Check if the vector is empty
                        // Add other conditions as necessary
                    }),
                    commands.end());


                // initial processing of commands
                for (const auto& cmd : commands) {
                    //if (cmd.empty()) { // Isolate command settings
                    //    continue;
                    //}
                    
                    commandName = cmd[0];
                    
                    if (stringToLowercase(commandName) == "erista:") {
                        inEristaSection = true;
                        inMarikoSection = false;
                        continue;
                    } else if (stringToLowercase(commandName) == "mariko:") {
                        inEristaSection = false;
                        inMarikoSection = true;
                        continue;
                    }
                    
                    if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {

                        if (commandName.find(systemPattern) == 0) {// Extract the command system
                            commandSystem = commandName.substr(systemPattern.length());
                            if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                                commandSystem = commandSystems[0]; // reset to default if commandSystem is unknown
                        } else if (commandName.find(modePattern) == 0) { // Extract the command mode
                            commandMode = commandName.substr(modePattern.length());
                            if (commandMode.find(toggleStr) != std::string::npos) {
                                delimiterPos = commandMode.find('?');
                                if (delimiterPos != std::string::npos) {
                                    defaultToggleState = commandMode.substr(delimiterPos + 1);
                                }
                                commandMode = toggleStr;
                            }
                            else if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                commandMode = commandModes[0]; // reset to default if commandMode is unknown
                        } else if (commandName.find(groupingPattern) == 0) {// Extract the command grouping
                            commandGrouping = commandName.substr(groupingPattern.length());
                            if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                        }
                        
                        // Extract the command grouping
                        if (commandMode == toggleStr) {
                            if (commandName.find("on:") == 0)
                                currentSection = lowerOnStr;
                            else if (commandName.find("off:") == 0)
                                currentSection = lowerOffStr;
                            
                            // Seperation of command chuncks
                            if (currentSection == globalStr) {
                                commandsOn.push_back(cmd);
                                commandsOff.push_back(cmd);
                            } else if (currentSection == lowerOnStr)
                                commandsOn.push_back(cmd);
                            else if (currentSection == lowerOffStr)
                                commandsOff.push_back(cmd);
                        }

                        if (cmd.size() > 1) { // Pre-process advanced commands
                            if (commandName == "file_source") {
                                if (currentSection == globalStr) {
                                    pathPattern = cmd[1];
                                    sourceType = fileStr;
                                } else if (currentSection == lowerOnStr) {
                                    pathPatternOn = cmd[1];
                                    sourceTypeOn = fileStr;
                                } else if (currentSection == lowerOffStr) {
                                    pathPatternOff = cmd[1];
                                    sourceTypeOff = fileStr;
                                }
                            } else if (commandName == "package_source") {
                                packageSource = preprocessPath(cmd[1], packagePath);
                            }
                        }
                    }
                }
                
                if (isFileOrDirectory(packageConfigIniPath)) {
                    packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    
                    //auto updateIniData = [&](const std::string& key, std::string& value) {
                    //    auto it = packageConfigData[optionName].find(key);
                    //    if (it != packageConfigData[optionName].end()) {
                    //        value = it->second;
                    //    } else {
                    //        setIniFileValue(packageConfigIniPath, optionName, key, value);
                    //    }
                    //};
                
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, systemStr, commandSystem);
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, modeStr, commandMode);
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, groupingStr, commandGrouping);
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, footerStr, commandFooter);
                    
                    packageConfigData.clear();
                } else { // write default data if settings are not loaded
                    setIniFileValue(packageConfigIniPath, optionName, systemStr, commandSystem);
                    setIniFileValue(packageConfigIniPath, optionName, modeStr, commandMode);
                    setIniFileValue(packageConfigIniPath, optionName, groupingStr, commandGrouping);
                    setIniFileValue(packageConfigIniPath, optionName, footerStr, nullStr);
                }

                
                
                // Get Option name and footer
                if (optionName.front() == '*') { 
                    useSelection = true;
                    optionName = optionName.substr(1); // Strip the "*" character on the left
                    footer = DROPDOWN_SYMBOL;
                } else {
                    pos = optionName.find(" - ");
                    if (pos != std::string::npos) {
                        footer = optionName.substr(pos + 2); // Assign the part after "&&" as the footer
                        optionName = optionName.substr(0, pos); // Strip the "&&" and everything after it
                    }
                }
                
                if (commandMode == optionStr || (commandMode == toggleStr && !useSelection)) {
                    // override loading of the command footer
                    if (commandFooter != nullStr)
                        footer = commandFooter;
                    else
                        footer = OPTION_SYMBOL;
                }

                skipSystem = false;
                if (commandSystem == eristaStr && !usingErista) {
                    skipSystem = true;
                } else if (commandSystem == marikoStr && !usingMariko) {
                    skipSystem = true;
                }
                
                if (!skipSection && !skipSystem) { // for skipping the drawing of sections
                    if (useSelection) { // For wildcard commands (dropdown menus)
                        
                        if ((footer == DROPDOWN_SYMBOL) || (footer.empty()))
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName), footer);
                        else {
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                            if (commandMode == optionStr)
                                listItem->setValue(footer);
                            else
                                listItem->setValue(footer, true);
                        }
                        
                        if (footer == UNAVAILABLE_SELECTION || footer == englishNotAvailable)
                            listItem->setValue(UNAVAILABLE_SELECTION, true);

                        if (commandMode == forwarderStr) {
                            const std::string forwarderPackagePath = getParentDirFromPath(packageSource);
                            const std::string forwarderPackageIniName = getNameFromPath(packageSource);

                            listItem->setClickListener([commands, keyName = option.first, &packagePath = this->packagePath, forwarderPackagePath, forwarderPackageIniName](s64 keys) mutable {


                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }
                                
                                if (keys & KEY_A) {
                                    interpretAndExecuteCommands(std::move(commands), packagePath, keyName); // Now correctly moved
                                    nestedMenuCount++;
                                    tsl::changeTo<PackageMenu>(forwarderPackagePath, "", leftStr, forwarderPackageIniName);
                                    simulatedSelectComplete = true;
                                    return true;
                                }
                                return false;
                            });
                        } else {
                            
                            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                            listItem->setClickListener([commands, keyName = option.first, &packagePath = this->packagePath,  &packageName = this->packageName, footer, lastSection, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                                
                                if (runningInterpreter.load(std::memory_order_acquire))
                                    return false;

                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }

                                if ((keys & KEY_A)) {
                                    if (footer != UNAVAILABLE_SELECTION && footer != englishNotAvailable) {
                                        if (inPackageMenu)
                                            inPackageMenu = false;
                                        if (inSubPackageMenu)
                                            inSubPackageMenu = false;
                                        
                                        selectedListItem.reset();
                                        selectedListItem = listItemPtr;
                                        
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
                                        lastSelectedListItem.reset();
                                        tsl::changeTo<SelectionOverlay>(packagePath, keyName, commands, newKey);
                                        lastKeyName = keyName;
                                    }

                                    simulatedSelectComplete = true;
                                    return true;
                                } else if (keys & SCRIPT_KEY) {
                                    if (inPackageMenu)
                                        inPackageMenu = false;
                                    if (inSubPackageMenu)
                                        inSubPackageMenu = false;
                                    tsl::changeTo<ScriptOverlay>(packagePath, keyName, false, packageName);
                                    return true;
                                }
                                return false;
                            });
                        }
                        
                        list->addItem(listItem.release());
                    } else { // For everything else
                        
                        const std::string& selectedItem = optionName;
                        
                        // For entries that are paths
                        itemName = getNameFromPath(selectedItem);
                        if (!isDirectory(preprocessPath(selectedItem, packagePath)))
                            itemName = dropExtension(itemName);
                        parentDirName = getParentDirNameFromPath(selectedItem);
                        
                        
                        if (commandMode == defaultStr || commandMode == optionStr) { // for handiling toggles
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                            if (commandMode == defaultStr)
                                listItem->setValue(footer, true);
                            else
                                listItem->setValue(footer);
                            
                            
                            listItem->setClickListener([i, commands, keyName = option.first, &packagePath = this->packagePath, &packageName = this->packageName, selectedItem, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                                //static bool lastRunningInterpreter = false;
                                
                                if (runningInterpreter.load(std::memory_order_acquire)) {
                                    return false;
                                }

                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }

                                if ((keys & KEY_A)) {

                                    isDownloadCommand = false;
                                    runningInterpreter.store(true, std::memory_order_release);
                                    enqueueInterpreterCommands(getSourceReplacement(commands, selectedItem, i, packagePath), packagePath, keyName);
                                    startInterpreterThread();

                                    listItemPtr->setValue(INPROGRESS_SYMBOL);


                                    shiftItemFocus(listItemPtr);
                                    lastSelectedListItem.reset();
                                    lastSelectedListItem = listItemPtr;
                                    
                                    lastRunningInterpreter = true;
                                    simulatedSelectComplete = true;
                                    return true;
                                }  else if (keys & SCRIPT_KEY) {
                                    if (inPackageMenu)
                                        inPackageMenu = false;
                                    if (inSubPackageMenu)
                                        inSubPackageMenu = false;
                                    tsl::changeTo<ScriptOverlay>(packagePath, keyName, false, packageName);
                                    return true;
                                }
                                return false;
                            });
                            list->addItem(listItem.release());
                        } else if (commandMode == toggleStr) {
                            
                            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(removeTag(optionName), false, ON, OFF);
                            
                            // Set the initial state of the toggle item
                            if (!pathPatternOn.empty())
                                toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn, packagePath));
                            else {
                                if ((footer != onStr && footer != offStr) && !defaultToggleState.empty()) {
                                    if (defaultToggleState == lowerOnStr)
                                        footer = onStr;
                                    else if (defaultToggleState == lowerOffStr)
                                        footer = offStr;
                                }

                                toggleStateOn = (footer == onStr);
                            }
                            
                            toggleListItem->setState(toggleStateOn);
                            
                            toggleListItem->setStateChangedListener([i, commandsOn, commandsOff, keyName = option.first, &packagePath = this->packagePath, &pathPatternOn = this->pathPatternOn, &pathPatternOff = this->pathPatternOff, listItemRaw = toggleListItem.get()](bool state) {
                                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                                interpretAndExecuteCommands(state ? getSourceReplacement(commandsOn, preprocessPath(pathPatternOn, packagePath), i, packagePath) : getSourceReplacement(commandsOff, preprocessPath(pathPatternOff, packagePath), i, packagePath), packagePath, keyName);
                                setIniFileValue((packagePath + configFileName).c_str(), keyName.c_str(), footerStr, state ? onStr : offStr);

                            });
                            list->addItem(toggleListItem.release());
                        }
                    }
                }
            }
        }
        
        options.clear();


        std::unique_ptr<tsl::elm::OverlayFrame> rootFrame = std::make_unique<tsl::elm::OverlayFrame>(
            getNameFromPath(packagePath),
            packageHeader.version != "" ? packageHeader.version + "   (Ultrahand Package)" : "Ultrahand Package",
            "",
            packageHeader.color,
            (usingPages && currentPage == rightStr) ? pageLeftName : "",
            (usingPages && currentPage == leftStr) ? pageRightName : ""
        );

        rootFrame->setContent(list.release());

        return rootFrame.release();
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
        
        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
            while (!interpreterThreadExit.load()) {svcSleepThread(50'000'000);}
            
            resetPercentages();
            
            isDownloadCommand = false;
            lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        // Your existing logic for handling other inputs
        if (refreshGui && !returningToPackage && !stillTouching) {
            refreshGui = false;
            
            if (inPackageMenu) {
                lastPackage = packagePath;
                inSubPackageMenu = false;
                inPackageMenu = false;
                if (lastPage == rightStr) {
                    tsl::goBack();
                } else {
                    tsl::goBack();
                }
                inPackageMenu = true;
                lastPage = leftStr;
                selectedListItem.reset();
                lastSelectedListItem.reset();
                tsl::changeTo<PackageMenu>(lastPackage);
                //lastPage == leftStr;
            }

            if (inSubPackageMenu) {
                lastPackage = packagePath;
                inSubPackageMenu = false;
                inPackageMenu = false;
                if (lastPage == rightStr) {
                    tsl::goBack();
                    tsl::goBack();
                } else {
                    tsl::goBack();
                    tsl::goBack();
                }
                inPackageMenu = true;
                lastPage = leftStr;
                selectedListItem.reset();
                lastSelectedListItem.reset();
                tsl::changeTo<PackageMenu>(lastPackage);
            }
            
        }

        if (usingPages) {
            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedNextPage && !simulatedNextPageComplete) {
                if (currentPage == leftStr) {
                    keysHeld |= KEY_DRIGHT;
                    simulatedNextPage = false;
                }
                else if (currentPage == rightStr) {
                    keysHeld |= KEY_DLEFT;
                    simulatedNextPage = false;
                }
                else {
                    simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }
            }
            if (currentPage == leftStr) {
                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    lastPage = rightStr;
                    lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackage, dropdownSection, rightStr);
                    simulatedNextPageComplete = true;
                    return true;
                }
            } else if (currentPage == rightStr) {
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    lastPage = leftStr;
                    lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackage, dropdownSection, leftStr);
                    simulatedNextPageComplete = true;
                    return true;
                }
            } 
        }
        
        if (!returningToPackage && inPackageMenu) {
            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (!usingPages || (usingPages && lastPage == leftStr)) {
                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }
                if ((keysHeld & KEY_B) && !stillTouching) {

                    if (nestedMenuCount == 0) {
                        inPackageMenu = false;
                        
                        if (!inHiddenMode)
                            returningToMain = true;
                        else
                            returningToHiddenMain = true;
                    }
                    if (nestedMenuCount > 0) {
                        nestedMenuCount--;
                        returningToPackage = true;
                    }
                    
                    // Free-up memory
                    clearMemory();

                    tsl::goBack();
                    simulatedBackComplete = true;
                    return true;
                }
            } else if (usingPages && lastPage == rightStr) {
                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }
                if ((keysHeld & KEY_B) && !stillTouching) {

                    if (nestedMenuCount == 0) {
                        inPackageMenu = false;
                        //inNestedPackageMenu = false;
                        
                        if (!inHiddenMode)
                            returningToMain = true;
                        else
                            returningToHiddenMain = true;
                    }
                    if (nestedMenuCount > 0) {
                        nestedMenuCount--;
                        returningToPackage = true;
                    }
                    
                    // Free-up memory
                    clearMemory();
                    
                    lastPage = leftStr;
                    tsl::goBack();
                    simulatedBackComplete = true;
                    return true;
                }
            }
        }
        
        if (!returningToSubPackage && inSubPackageMenu) {
            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (!usingPages || (usingPages && lastPage == leftStr)) {
                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }
                if ((keysHeld & KEY_B) && !stillTouching) {
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    tsl::goBack();
                    
                    simulatedBackComplete = true;
                    return true;
                }
            } else if (usingPages && lastPage == rightStr) {
                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }
                if ((keysHeld & KEY_B) && !stillTouching) {
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    //tsl::goBack();
                    tsl::goBack();

                    simulatedBackComplete = true;
                    return true;
                }
            }
        }
        if (returningToPackage && !(keysHeld & KEY_B)){
            returningToPackage = false;
            inPackageMenu = true;
        }
        
        if (returningToSubPackage && !(keysHeld & KEY_B)){
            returningToSubPackage = false;
            inSubPackageMenu = true;
            simulatedBackComplete = true;
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
        //svcSleepThread(10'000'000);
        return false;
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
    std::string packageIniPath = packageDirectory + packageFileName;
    std::string packageConfigIniPath = packageDirectory + configFileName;
    std::string menuMode, inOverlayString, fullPath, optionName, priority, starred, hide;
    bool useDefaultMenu = false;
    bool useOverlayLaunchArgs = false;
    std::string hiddenMenuMode, dropdownSection;
    bool initializingSpawn = false;
    std::string defaultLang = "en";
    
public:
    /**
     * @brief Constructs a `MainMenu` instance.
     *
     * Initializes a new instance of the `MainMenu` class with the necessary parameters.
     */
    MainMenu(const std::string& hiddenMenuMode = "", const std::string& sectionName = "") : hiddenMenuMode(hiddenMenuMode), dropdownSection(sectionName) {}
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
        if (parseValueFromIniSection(settingsConfigIniPath, projectName, inHiddenOverlayStr) == trueStr) {
            inMainMenu = false;
            inHiddenMode = true;
            hiddenMenuMode = overlaysStr;
            setIniFileValue(settingsConfigIniPath, projectName, inHiddenOverlayStr, falseStr);
        }

        if (!inHiddenMode)
            inMainMenu = true;
        
        tsl::hlp::ini::IniData settingsData, packageConfigData;
        std::string packagePath, pathReplace, pathReplaceOn, pathReplaceOff;
        std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
        std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
        
        bool skipSystem = false;
        lastMenuMode = hiddenMenuMode;
        
        menuMode = overlaysStr;
        
        createDirectory(packageDirectory);
        createDirectory(settingsPath);
        
        bool settingsLoaded = false;
        if (isFileOrDirectory(settingsConfigIniPath)) {
            std::string section;
            auto settingsData = getParsedDataFromIniFile(settingsConfigIniPath);
            if (settingsData.count(projectName) > 0) {
                auto& ultrahandSection = settingsData[projectName];
        
                // Handle each setting by checking existence and updating accordingly
                section = "hide_user_guide";
                if (ultrahandSection.count(section) > 0) {
                    hideUserGuide = ultrahandSection[section];
                } else {
                    setIniFileValue(settingsConfigIniPath, projectName, section, falseStr);
                }
                
                section = "clean_version_labels";
                if (ultrahandSection.count(section) > 0) {
                    cleanVersionLabels = ultrahandSection[section];
                } else {
                    setIniFileValue(settingsConfigIniPath, projectName, section, falseStr);
                    cleanVersionLabels = falseStr;
                }
        
                // Manage visibility settings with similar pattern
                section = "hide_overlay_versions";
                if (ultrahandSection.count(section) > 0) {
                    hideOverlayVersions = ultrahandSection[section];
                } else {
                    setIniFileValue(settingsConfigIniPath, projectName, section, falseStr);
                    hideOverlayVersions = falseStr;
                }
                
                section = "hide_package_versions";
                if (ultrahandSection.count(section) > 0) {
                    hidePackageVersions = ultrahandSection[section];
                } else {
                    setIniFileValue(settingsConfigIniPath, projectName, section, falseStr);
                    hidePackageVersions = falseStr;
                }
                
                section = defaultLangStr;
                if (ultrahandSection.count(section) > 0) {
                    defaultLang = ultrahandSection[section];
                } else {
                    setIniFileValue(settingsConfigIniPath, projectName, section, defaultLang);
                }
        
                // Ensure default values are set if the settings are missing
                section = "datetime_format";
                if (ultrahandSection.count(section) == 0) {
                    setIniFileValue(settingsConfigIniPath, projectName, section, DEFAULT_DT_FORMAT);
                }
        
                // Directly check and set default values
                section = "hide_clock";
                if (ultrahandSection.count(section) == 0) {
                    setIniFileValue(settingsConfigIniPath, projectName, section, falseStr);
                }
                section = "hide_battery";
                if (ultrahandSection.count(section) == 0) {
                    setIniFileValue(settingsConfigIniPath, projectName, section, trueStr);
                }
                section = "hide_pcb_temp";
                if (ultrahandSection.count(section) == 0) {
                    setIniFileValue(settingsConfigIniPath, projectName, section, trueStr);
                }
                section = "hide_soc_temp";
                if (ultrahandSection.count(section) == 0) {
                    setIniFileValue(settingsConfigIniPath, projectName, section, trueStr);
                }

                section = inOverlayStr;
                settingsLoaded = ultrahandSection.count(section) > 0;
            }
            settingsData.clear();
        }
        
        if (!settingsLoaded) { // Write data if settings are not loaded
            setIniFileValue(settingsConfigIniPath, projectName, defaultLangStr, defaultLang);
            setIniFileValue(settingsConfigIniPath, projectName, inOverlayStr, falseStr);
        }
        
        
        std::string langFile = langPath+defaultLang+".json";
        if (isFileOrDirectory(langFile))
            parseLanguage(langFile);
        else
            reinitializeLangVars();
        
        // write default theme
        initializeTheme();
        copyTeslaKeyComboToUltrahand();
        
        menuMode = defaultMenu.c_str();
        
        versionLabel = std::string(APP_VERSION) + "   (" + extractTitle(loaderInfo) + " " + (cleanVersionLabels == trueStr ? "" : "v") + cleanVersionLabel(loaderInfo) + ")";
        
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();

        std::unique_ptr<tsl::elm::ListItem> listItem;
        
        if (!hiddenMenuMode.empty())
            menuMode = hiddenMenuMode;
        
        
        // Overlays menu
        if (menuMode == overlaysStr) {
            //closeInterpreterThread();

            list->addItem(new tsl::elm::CategoryHeader(!inHiddenMode ? OVERLAYS : HIDDEN_OVERLAYS));
            
            
            // Load overlay files
            std::vector<std::string> overlayFiles = getFilesListByWildcards(overlayDirectory+"*.ovl");
            
            
            // Check if the overlays INI file exists
            std::ifstream overlaysIniFile(overlaysIniFilePath);
            if (!overlaysIniFile.is_open()) {
                // The INI file doesn't exist, so create an empty one.
                std::ofstream createFile(overlaysIniFilePath);
                if (createFile.is_open())
                    initializingSpawn = true;
            }

            overlaysIniFile.close(); // Close the file


            // load overlayList from overlaysIniFilePath.  this will be the overlayFilenames
            std::vector<std::string> overlayList;
            std::vector<std::string> hiddenOverlayList;
            
            std::string overlayFileName;
            
            // Load subdirectories
            if (!overlayFiles.empty()) {
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> overlaysIniData = getParsedDataFromIniFile(overlaysIniFilePath);
                
                std::string overlayName, overlayVersion;
                
                bool foundOvlmenu = false;  // Flag to indicate if "ovlmenu.ovl" has been found and removed
                
                overlayFiles.erase(
                    std::remove_if(
                        overlayFiles.begin(), 
                        overlayFiles.end(),
                        [&foundOvlmenu](const std::string& file) {
                            std::string fileName = getNameFromPath(file);
                            if (!foundOvlmenu && fileName == "ovlmenu.ovl") {
                                foundOvlmenu = true;  // Mark as found and continue to remove
                                return true;
                            }
                            return fileName.front() == '.';
                        }
                    ),
                    overlayFiles.end()
                );

                // Assuming the existence of appropriate utility functions and types are defined elsewhere.
                for (const auto& overlayFile : overlayFiles) {
                    const std::string& overlayFileName = getNameFromPath(overlayFile);
                
                    //if (overlayFileName == "ovlmenu.ovl" || overlayFileName.front() == '.') {
                    //    continue;
                    //}
                
                    auto it = overlaysIniData.find(overlayFileName);
                    if (it == overlaysIniData.end()) {
                        // Initialization of new entries
                        setIniFileValue(overlaysIniFilePath, overlayFileName, priorityStr, "20");
                        setIniFileValue(overlaysIniFilePath, overlayFileName, starStr, falseStr);
                        setIniFileValue(overlaysIniFilePath, overlayFileName, hideStr, falseStr);
                        setIniFileValue(overlaysIniFilePath, overlayFileName, useLaunchArgsStr, falseStr);
                        setIniFileValue(overlaysIniFilePath, overlayFileName, launchArgsStr, "''");
                        overlayList.push_back("0020:" + overlayFileName);
                    } else {
                        const std::string& priority = getValueOrDefault(it->second, priorityStr, "20", formatPriorityString, 1);
                        const std::string& starred = getValueOrDefault(it->second, starStr, falseStr);
                        const std::string& hide = getValueOrDefault(it->second, hideStr, falseStr);
                        const std::string& useLaunchArgs = getValueOrDefault(it->second, useLaunchArgsStr, falseStr);
                        const std::string& launchArgs = getValueOrDefault(it->second, launchArgsStr, "''");
                
                        const auto& [result, overlayName, overlayVersion] = getOverlayInfo(overlayDirectory + overlayFileName);
                        if (result != ResultSuccess) continue;
                
                        const std::string& baseOverlayInfo = priority + ":" + overlayName + ":" + overlayVersion + ":" + overlayFileName;
                        const std::string& fullOverlayInfo = (starred == trueStr) ? "-1:" + baseOverlayInfo : baseOverlayInfo;
                
                        if (hide == falseStr) {
                            overlayList.push_back(fullOverlayInfo);
                        } else {
                            hiddenOverlayList.push_back(fullOverlayInfo);
                        }
                    }
                }


                
                overlaysIniData.clear();
                
                std::sort(overlayList.begin(), overlayList.end());
                std::sort(hiddenOverlayList.begin(), hiddenOverlayList.end());
                
                
                if (inHiddenMode) {
                    overlayList = hiddenOverlayList;
                    hiddenOverlayList.clear();
                }
                
                //std::string overlayFileName;
                std::string overlayStarred;
                //std::string overlayVersion, overlayName;
                std::string overlayFile, newOverlayName;
                size_t lastUnderscorePos, secondLastUnderscorePos, thirdLastUnderscorePos;
                
                std::string newStarred;
                
                for (const auto& taintedOverlayFileName : overlayList) {
                    overlayFileName = "";
                    overlayStarred = falseStr;
                    overlayVersion = "";
                    overlayName = "";
                    
                    // Detect if starred
                    if ((taintedOverlayFileName.substr(0, 3) == "-1:"))
                        overlayStarred = trueStr;
                    
                    // Find the position of the last underscore
                    lastUnderscorePos = taintedOverlayFileName.rfind(':');
                    // Check if an underscore was found
                    if (lastUnderscorePos != std::string::npos) {
                        // Extract overlayFileName starting from the character after the last underscore
                        overlayFileName = taintedOverlayFileName.substr(lastUnderscorePos + 1);
                        
                        // Now, find the position of the second-to-last underscore
                        secondLastUnderscorePos = taintedOverlayFileName.rfind(':', lastUnderscorePos - 1);
                        
                        if (secondLastUnderscorePos != std::string::npos) {
                            // Extract overlayName between the two underscores
                            overlayVersion = taintedOverlayFileName.substr(secondLastUnderscorePos + 1, lastUnderscorePos - secondLastUnderscorePos - 1);
                            // Now, find the position of the second-to-last underscore
                            thirdLastUnderscorePos = taintedOverlayFileName.rfind(':', secondLastUnderscorePos - 1);
                            if (secondLastUnderscorePos != std::string::npos)
                                overlayName = taintedOverlayFileName.substr(thirdLastUnderscorePos + 1, secondLastUnderscorePos - thirdLastUnderscorePos - 1);
                        }
                    }
                    
                    
                    overlayFile = overlayDirectory+overlayFileName;
                    
                    newOverlayName = overlayName.c_str();
                    if (overlayStarred == trueStr)
                        newOverlayName = STAR_SYMBOL+" "+newOverlayName;
                    
                    
                    // Toggle the starred status
                    newStarred = (overlayStarred == trueStr) ? falseStr : trueStr;
                    
                    
                    //logMessage(overlayFile);
                    if (isFileOrDirectory(overlayFile)) {
                        listItem = std::make_unique<tsl::elm::ListItem>(newOverlayName);
                        if (cleanVersionLabels == trueStr)
                            overlayVersion = cleanVersionLabel(overlayVersion);
                        if (hideOverlayVersions != trueStr)
                            listItem->setValue(overlayVersion, true);
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([&hiddenMenuMode = this->hiddenMenuMode, overlayFile, newStarred, overlayFileName, overlayName](s64 keys) {
                            
                            if (runningInterpreter.load(std::memory_order_acquire))
                                return false;
                            

                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }

                            if (keys & KEY_A) {
                                
                                setIniFileValue(settingsConfigIniPath, projectName, inOverlayStr, trueStr); // this is handled within tesla.hpp
                                std::string useOverlayLaunchArgs = parseValueFromIniSection(overlaysIniFilePath, overlayFileName, useLaunchArgsStr);
                                std::string overlayLaunchArgs = removeQuotes(parseValueFromIniSection(overlaysIniFilePath, overlayFileName, launchArgsStr));
                                
                                if (inHiddenMode) {
                                    setIniFileValue(settingsConfigIniPath, projectName, inHiddenOverlayStr, trueStr);
                                }
                                
                                if (useOverlayLaunchArgs == trueStr)
                                    tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                                else
                                    tsl::setNextOverlay(overlayFile);
                                
                                tsl::Overlay::get()->close();
                                simulatedSelectComplete = true;

                                return true;
                            } else if (keys & STAR_KEY) {
                                
                                if (!overlayFile.empty()) {
                                    // Update the INI file with the new value
                                    setIniFileValue(overlaysIniFilePath, overlayFileName, starStr, newStarred);
                                    // Now, you can use the newStarred value for further processing if needed
                                }
                                if (inHiddenMode) {
                                    //tsl::goBack();
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                    reloadMenu2 = true;
                                }
                                refreshGui = true;

                                return true;
                            } else if (keys & SETTINGS_KEY) {
                                if (!inHiddenMode) {
                                    lastMenu = "";
                                    inMainMenu = false;
                                } else {
                                    lastMenu = "hiddenMenuMode";
                                    inHiddenMode = false;
                                }
                                
                                tsl::changeTo<SettingsMenu>(overlayFileName, overlayStr, overlayName);
                                return true;
                            }
                            return false;
                        });
                    }
                    if (listItem != nullptr)
                        list->addItem(listItem.release());
                }
                overlayList.clear();
                
                if (!hiddenOverlayList.empty() && !inHiddenMode) {
                    listItem = std::make_unique<tsl::elm::ListItem>(HIDDEN, DROPDOWN_SYMBOL);
                    
                    listItem->setClickListener([](uint64_t keys) {
                        if (runningInterpreter.load(std::memory_order_acquire))
                            return false;

                        if (simulatedSelect && !simulatedSelectComplete) {
                            keys |= KEY_A;
                            simulatedSelect = false;
                        }

                        if (keys & KEY_A) {
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>(overlaysStr);
                            simulatedSelectComplete = true;
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem.release());
                }
            }
        }
        
        
        // Packages menu
        if (menuMode == packagesStr ) {
            //startInterpreterThread();

            if (dropdownSection.empty()) {
                // Create the directory if it doesn't exist
                createDirectory(packageDirectory);
                
                
                std::fstream packagesIniFile(packagesIniFilePath, std::ios::in);
                if (!packagesIniFile.is_open()) {
                    std::ofstream createFile(packagesIniFilePath); // Create an empty INI file if it doesn't exist
                    createFile.close();
                    initializingSpawn = true;
                } else {
                    packagesIniFile.close();
                }
                
                std::vector<std::string> packageList;
                std::vector<std::string> hiddenPackageList;
                
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> packagesIniData = getParsedDataFromIniFile(packagesIniFilePath);
                // Load subdirectories
                std::vector<std::string> subdirectories = getSubdirectories(packageDirectory);
                //for (size_t i = 0; i < subdirectories.size(); ++i) {

                // Remove subdirectories starting with a dot
                subdirectories.erase(
                    std::remove_if(
                        subdirectories.begin(), 
                        subdirectories.end(),
                        [](const std::string& dirName) {
                            return dirName.front() == '.';
                        }
                    ),
                    subdirectories.end()
                );

                for (const auto& packageName: subdirectories) {
                    auto packageIt = packagesIniData.find(packageName);
                    if (packageIt == packagesIniData.end()) {
                        // Initialize missing package data
                        setIniFileValue(packagesIniFilePath, packageName, priorityStr, "20");
                        setIniFileValue(packagesIniFilePath, packageName, starStr, falseStr);
                        setIniFileValue(packagesIniFilePath, packageName, hideStr, falseStr);
                        packageList.push_back("0020:" + packageName);
                    } else {
                        // Process existing package data
                        priority = (packageIt->second.find(priorityStr) != packageIt->second.end()) ? 
                                    formatPriorityString(packageIt->second[priorityStr]) : "0020";
                        starred = (packageIt->second.find(starStr) != packageIt->second.end()) ? 
                                  packageIt->second[starStr] : falseStr;
                        hide = (packageIt->second.find(hideStr) != packageIt->second.end()) ? 
                               packageIt->second[hideStr] : falseStr;
                
                        const std::string& basePackageInfo = priority + ":" + packageName;
                        const std::string& fullPackageInfo = (starred == trueStr) ? "-1:" + basePackageInfo : basePackageInfo;
                
                        if (hide == falseStr) {
                            packageList.push_back(fullPackageInfo);
                        } else {
                            hiddenPackageList.push_back(fullPackageInfo);
                        }
                    }
                }

                packagesIniData.clear();
                subdirectories.clear();
                
                std::sort(packageList.begin(), packageList.end());
                std::sort(hiddenPackageList.begin(), hiddenPackageList.end());
                
                if (inHiddenMode) {
                    packageList = hiddenPackageList;
                    hiddenPackageList.clear();
                }
                
                std::string taintePackageName;
                std::string packageName;
                std::string packageStarred;
                std::string newPackageName;
                std::string packageFilePath;
                std::string newStarred;
                PackageHeader packageHeader;
                
                for (size_t i = 0; i < packageList.size(); ++i) {
                    taintePackageName = packageList[i];
                    if (i == 0) {
                         list->addItem(new tsl::elm::CategoryHeader(!inHiddenMode ? PACKAGES : HIDDEN_PACKAGES));
                    }
                    
                    packageName = taintePackageName.c_str();
                    packageStarred = falseStr;
                    
                    if ((packageName.length() >= 2) && (packageName.substr(0, 3) == "-1:")) {
                        // strip first two characters
                        packageName = packageName.substr(3);
                        packageStarred = trueStr;
                    }
                    
                    packageName = packageName.substr(5);
                    
                    newPackageName = (packageStarred == trueStr) ? (STAR_SYMBOL + " " + packageName) : packageName;
                    
                    packageFilePath = packageDirectory + packageName+ "/";
                    
                    // Toggle the starred status
                    newStarred = (packageStarred == trueStr) ? falseStr : trueStr;
                    
                    
                    if (isFileOrDirectory(packageFilePath)) {
                        packageHeader = getPackageHeaderFromIni(packageFilePath+packageFileName);
                        
                        listItem = std::make_unique<tsl::elm::ListItem>(newPackageName);
                        if (cleanVersionLabels == trueStr)
                            packageHeader.version = removeQuotes(cleanVersionLabel(packageHeader.version));
                        if (hidePackageVersions != trueStr)
                           listItem->setValue(packageHeader.version, true);
                        
                        packageHeader.clear(); // free memory
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([&hiddenMenuMode = this->hiddenMenuMode, packageFilePath, newStarred, packageName](s64 keys) {
                            if (runningInterpreter.load(std::memory_order_acquire)) {
                                return false;
                            }
                            
                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }
                            
                            if (keys & KEY_A) {
                                inMainMenu = false;
                                
                                
                                // read commands from package's boot_package.ini
                                if (isFileOrDirectory(packageFilePath+bootPackageFileName)) {
                                    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> bootOptions = loadOptionsFromIni(packageFilePath+bootPackageFileName, true);
                                    if (bootOptions.size() > 0) {
                                        std::string bootOptionName;
                                        for (auto& bootOption:bootOptions) {
                                            bootOptionName = bootOption.first;
                                            auto& bootCommands = bootOption.second;
                                            if (bootOptionName == "boot") {
                                                interpretAndExecuteCommands(std::move(bootCommands), packageFilePath, bootOptionName); // Execute modified
                                                break;
                                            }
                                        }
                                        bootOptions.clear();
                                    }
                                }
                                
                                tsl::changeTo<PackageMenu>(packageFilePath, "");
                                simulatedSelectComplete = true;
                                return true;
                            } else if (keys & STAR_KEY) {
                                if (!packageName.empty())
                                    setIniFileValue(packagesIniFilePath, packageName, starStr, newStarred); // Update the INI file with the new value
                                
                                if (inHiddenMode) {
                                    //tsl::goBack();
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                    reloadMenu2 = true;
                                }
                                refreshGui = true;

                                //tsl::changeTo<MainMenu>(hiddenMenuMode);
                                return true;
                            } else if (keys & SETTINGS_KEY) {
                                
                                if (!inHiddenMode) {
                                    lastMenu = "";
                                    inMainMenu = false;
                                } else {
                                    lastMenu = "hiddenMenuMode";
                                    inHiddenMode = false;
                                }
                                
                                tsl::changeTo<SettingsMenu>(packageName, packageStr);
                                return true;
                            }
                            return false;
                        });
                        list->addItem(listItem.release());
                    }
                }
                packageList.clear();
                
                if (!hiddenPackageList.empty() && !inHiddenMode) {
                    listItem = std::make_unique<tsl::elm::ListItem>(HIDDEN, DROPDOWN_SYMBOL);
                    listItem->setClickListener([](uint64_t keys) {
                        if (runningInterpreter.load(std::memory_order_acquire))
                            return false;
                        
                        if (simulatedSelect && !simulatedSelectComplete) {
                            keys |= KEY_A;
                            simulatedSelect = false;
                        }
                        
                        if (keys & KEY_A) {
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>(packagesStr);
                            simulatedSelectComplete = true;
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem.release());
                }
            }
            
            // ********* THIS PART NEEDS TO MIRROR WHAT IS WITHIN SUBMENU *********
            
            if (!inHiddenMode) {
                // Load options from INI file
                std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath, true);
                
                bool skipSection = false;

                // initialize packageConfigIniPath text file
                std::string lastSection = "";

                std::string optionName, footer;
                bool useSelection;
                
                std::string commandName;
                
                std::string commandFooter = nullStr;
                std::string commandSystem = defaultStr;
                std::string commandMode = defaultStr;
                std::string commandGrouping = defaultStr;
                
                std::string currentSection = globalStr;

                std::string defaultToggleState;

                std::string sourceType = defaultStr, sourceTypeOn = defaultStr, sourceTypeOff = defaultStr; 
                
                std::string jsonPath, jsonPathOn, jsonPathOff;
                std::string jsonKey, jsonKeyOn, jsonKeyOff;
                
                std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
                
                
                auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>("", true, "", "");
                bool toggleStateOn;
                
                bool inEristaSection;
                bool inMarikoSection;
                
                size_t pos;
                
                
                for (size_t i = 0; i < options.size(); ++i) {
                    auto& option = options[i];
                    
                    optionName = option.first;
                    commands = std::move(option.second);
                    
                    footer = "";
                    useSelection = false;
                    
                    commandFooter = nullStr;
                    commandSystem = defaultStr;
                    commandMode = defaultStr;
                    commandGrouping = defaultStr;
                    
                    currentSection = globalStr;

                    defaultToggleState = "";
                    sourceType = defaultStr;
                    sourceTypeOn = defaultStr;
                    sourceTypeOff = defaultStr; 
                    
                    jsonPath = "";
                    jsonPathOn = "";
                    jsonPathOff = "";
                    jsonKey = "";
                    jsonKeyOn = "";
                    jsonKeyOff = "";
                    
                    commandsOn.clear();
                    commandsOff.clear();
                    
                    // Custom header implementation
                    if (!dropdownSection.empty()) {
                        if (i == 0) {
                            // Add a section break with small text to indicate the "Commands" section
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
                        if (commands.size() == 0 && optionName.front() != '*') {
                            // Add a section break with small text to indicate the "Commands" section
                            if (optionName != lastSection)
                                list->addItem(new tsl::elm::CategoryHeader(removeTag(optionName)));
                            lastSection = optionName;
                            skipSection = false;
                            continue;
                        } else if (i == 0) { // Add a section break with small text to indicate the "Commands" section
                            list->addItem(new tsl::elm::CategoryHeader(COMMANDS));
                            skipSection = false;
                            lastSection = "Commands";
                        }
                    }
                    
                    if (optionName.front() == '*') {
                        // Create reference to PackageMenu with dropdownSection set to optionName
                        listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName.substr(1)), DROPDOWN_SYMBOL);
                        
                        listItem->setClickListener([optionName](s64 keys) {
                            if (runningInterpreter.load(std::memory_order_acquire))
                                return false;
                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }
                            if (keys & KEY_A) {
                                inPackageMenu = false;
                                tsl::changeTo<MainMenu>("", optionName);
                                simulatedSelectComplete = true;
                                return true;
                            }
                            return false;
                        });
                        list->addItem(listItem.release());
                        
                        skipSection = true;
                    }

                    
                    // initial processing of commands
                    inEristaSection = false;
                    inMarikoSection = false;
                    
                    size_t delimiterPos;

                    // Remove all empty command strings
                    commands.erase(std::remove_if(commands.begin(), commands.end(),
                        [](const std::vector<std::string>& vec) {
                            return vec.empty(); // Check if the vector is empty
                            // Add other conditions as necessary
                        }),
                        commands.end());

                    // initial processing of commands
                    for (const auto& cmd : commands) {
                        //f (cmd.empty()) { // Isolate command settings
                        //   continue;
                        //
                        
                        commandName = cmd[0];
                        
                        if (stringToLowercase(commandName) == "erista:") {
                            inEristaSection = true;
                            inMarikoSection = false;
                            continue;
                        } else if (stringToLowercase(commandName) == "mariko:") {
                            inEristaSection = false;
                            inMarikoSection = true;
                            continue;
                        }
                        
                        if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
                            // Extract the command mode
                            if (commandName.find(systemPattern) == 0) {// Extract the command system
                                commandSystem = commandName.substr(systemPattern.length());
                                if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                                    commandSystem = commandSystems[0]; // reset to default if commandSystem is unknown
                            } else if (commandName.find(modePattern) == 0) {
                                commandMode = commandName.substr(modePattern.length());
                                if (commandMode.find(toggleStr) != std::string::npos) {
                                    delimiterPos = commandMode.find('?');
                                    if (delimiterPos != std::string::npos) {
                                        defaultToggleState = commandMode.substr(delimiterPos + 1);
                                    }
                                    commandMode = toggleStr;
                                }
                                else if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                    commandMode = commandModes[0]; // reset to default if commandMode is unknown

                            } else if (commandName.find(groupingPattern) == 0) {// Extract the command grouping
                                commandGrouping = commandName.substr(groupingPattern.length());
                                if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                    commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                            }
                            
                            // Extract the command grouping
                            if (commandMode == toggleStr) {

                                if (commandName.find("on:") == 0)
                                    currentSection = lowerOnStr;
                                else if (commandName.find("off:") == 0)
                                    currentSection = lowerOffStr;
                                
                                // Seperation of command chuncks
                                if (currentSection == globalStr) {
                                    commandsOn.push_back(cmd);
                                    commandsOff.push_back(cmd);
                                } else if (currentSection == lowerOnStr)
                                    commandsOn.push_back(cmd);
                                else if (currentSection == lowerOffStr)
                                    commandsOff.push_back(cmd);
                            }
                            if (cmd.size() > 1) { // Pre-process advanced commands
                                if (commandName == "file_source") {
                                    if (currentSection == globalStr) {
                                        pathPattern = cmd[1];
                                        sourceType = fileStr;
                                    } else if (currentSection == lowerOnStr) {
                                        pathPatternOn = cmd[1];
                                        sourceTypeOn = fileStr;
                                    } else if (currentSection == lowerOffStr) {
                                        pathPatternOff = cmd[1];
                                        sourceTypeOff = fileStr;
                                    }
                                }
                            }
                        }
                    }
                    
                    
                    if (isFileOrDirectory(packageConfigIniPath)) {
                        packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    
                        //auto updateIniData = [&](const std::string& key, std::string& value) {
                        //    auto it = packageConfigData[optionName].find(key);
                        //    if (it != packageConfigData[optionName].end()) {
                        //        value = it->second;
                        //    } else {
                        //        setIniFileValue(packageConfigIniPath, optionName, key, value);
                        //    }
                        //};
                    
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, systemStr, commandSystem);
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, modeStr, commandMode);
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, groupingStr, commandGrouping);
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, footerStr, commandFooter);
                    
                        packageConfigData.clear();
                    } else { // write default data if settings are not loaded
                        setIniFileValue(packageConfigIniPath, optionName, systemStr, commandSystem);
                        setIniFileValue(packageConfigIniPath, optionName, modeStr, commandMode);
                        setIniFileValue(packageConfigIniPath, optionName, groupingStr, commandGrouping);
                        setIniFileValue(packageConfigIniPath, optionName, footerStr, nullStr);
                    }
                    
                    
                    // get Option Name and footer
                    if (optionName.front() == '*') { 
                        useSelection = true;
                        optionName = optionName.substr(1); // Strip the "*" character on the left
                        footer = DROPDOWN_SYMBOL;
                    } else {
                        pos = optionName.find(" - ");
                        if (pos != std::string::npos) {
                            footer = optionName.substr(pos + 2); // Assign the part after "&&" as the footer
                            optionName = optionName.substr(0, pos); // Strip the "&&" and everything after it
                        }
                    }
                    
                    if (commandMode == optionStr || (commandMode == toggleStr && !useSelection)) {
                        // override loading of the command footer
                        if (commandFooter != nullStr)
                            footer = commandFooter;
                        else
                            footer = OPTION_SYMBOL;
                    }

                    skipSystem = false;
                    if (commandSystem == eristaStr && !usingErista) {
                        skipSystem = true;
                    } else if (commandSystem == marikoStr && !usingMariko) {
                        skipSystem = true;
                    }
                    
                    if (!skipSection && !skipSystem) {
                        if (useSelection) { // For wildcard commands (dropdown menus)
                            std::unique_ptr<tsl::elm::ListItem> listItem;
                            if ((footer == DROPDOWN_SYMBOL) || (footer.empty()))
                                listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName), footer);
                            else {
                                listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                                listItem->setValue(footer, true);
                            }
                            
                            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                            listItem->setClickListener([commands, keyName = option.first, packagePath = packageDirectory](uint64_t keys) {
                                if (runningInterpreter.load(std::memory_order_acquire))
                                    return false;

                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }
                                if (keys & KEY_A) {
                                    inMainMenu = false;
                                    tsl::changeTo<SelectionOverlay>(packagePath, keyName, commands);
                                    simulatedSelectComplete = true;
                                    return true;
                                } else if (keys & SCRIPT_KEY) {
                                    inMainMenu = false; // Set boolean to true when entering a submenu
                                    tsl::changeTo<ScriptOverlay>(packagePath, keyName, true);
                                    return true;
                                }
                                return false;
                            });
                            
                            list->addItem(listItem.release());
                        } else { // For everything else
                            
                            const std::string& selectedItem = optionName;
                            
                            // For entries that are paths
                            itemName = getNameFromPath(selectedItem);
                            if (!isDirectory(preprocessPath(selectedItem, packageDirectory)))
                                itemName = dropExtension(itemName);
                            parentDirName = getParentDirNameFromPath(selectedItem);
                            
                            
                            if (commandMode == defaultStr || commandMode == optionStr) { // for handiling toggles
                                listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                                listItem->setValue(footer, true);
                                
                                if (sourceType == _jsonStr) { // For JSON wildcards
                                    
                                    listItem->setClickListener([i, commands, packagePath = packageDirectory, keyName = option.first, selectedItem, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                                        //static bool lastRunningInterpreter = false;

                                        if (runningInterpreter.load(std::memory_order_acquire))
                                            return false;

                                        if (simulatedSelect && !simulatedSelectComplete) {
                                            keys |= KEY_A;
                                            simulatedSelect = false;
                                        }


                                        if ((keys & KEY_A)) {
                                            isDownloadCommand = false;
                                            runningInterpreter.store(true, std::memory_order_release);
                                            enqueueInterpreterCommands(getSourceReplacement(commands, selectedItem, i, packagePath), packagePath, keyName);
                                            startInterpreterThread();

                                            listItemPtr->setValue(INPROGRESS_SYMBOL);

                                            shiftItemFocus(listItemPtr);

                                            lastSelectedListItem.reset();
                                            lastSelectedListItem = listItemPtr;
                                            

                                            lastRunningInterpreter = true;
                                            
                                            
                                            simulatedSelectComplete = true;
                                            return true;
                                        } else if (keys & SCRIPT_KEY) {
                                            inMainMenu = false; // Set boolean to true when entering a submenu
                                            tsl::changeTo<ScriptOverlay>(packagePath, keyName, true);
                                            return true;
                                        }
                                        
                                        return false;
                                    });
                                    list->addItem(listItem.release());
                                } else {
                                    
                                    listItem->setClickListener([i, commands, packagePath = packageDirectory, keyName = option.first, selectedItem, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                                        
                                        if (runningInterpreter.load(std::memory_order_acquire))
                                            return false;

                                        if (simulatedSelect && !simulatedSelectComplete) {
                                            keys |= KEY_A;
                                            simulatedSelect = false;
                                        }
                                        

                                        if ((keys & KEY_A)) {
                                            isDownloadCommand = false;
                                            runningInterpreter.store(true, std::memory_order_release);
                                            enqueueInterpreterCommands(getSourceReplacement(commands, selectedItem, i, packagePath), packagePath, keyName);
                                            startInterpreterThread();
                                            
                                            listItemPtr->setValue(INPROGRESS_SYMBOL);

                                            shiftItemFocus(listItemPtr);

                                            lastSelectedListItem.reset();
                                            lastSelectedListItem = listItemPtr;
                                            
                                            
                                            lastRunningInterpreter = true;
                                            simulatedSelectComplete = true;
                                            return true;
                                        } else if (keys & SCRIPT_KEY) {
                                            inMainMenu = false; // Set boolean to true when entering a submenu
                                            tsl::changeTo<ScriptOverlay>(packagePath, keyName, true);
                                            return true;
                                        }
                                        return false;
                                    });
                                    list->addItem(listItem.release());
                                }
                            } else if (commandMode == toggleStr) {
                                
                                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(removeTag(optionName), false, ON, OFF);
                                
                                // Set the initial state of the toggle item
                                if (!pathPatternOn.empty())
                                    toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn, packageDirectory));
                                else {
                                    if ((footer != onStr && footer != offStr) && !defaultToggleState.empty()) {
                                        if (defaultToggleState == lowerOnStr)
                                            footer = onStr;
                                        else if (defaultToggleState == lowerOffStr)
                                            footer = offStr;
                                    }

                                    toggleStateOn = (footer == onStr);
                                }
                                
                                toggleListItem->setState(toggleStateOn);
                                
                                toggleListItem->setStateChangedListener([i, pathPatternOn, pathPatternOff, commandsOn, commandsOff, packagePath = packageDirectory, keyName = option.first, listItemRaw = toggleListItem.get()](bool state) {
                                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                                    interpretAndExecuteCommands(getSourceReplacement(state ? commandsOn : commandsOff, preprocessPath(state ? pathPatternOn : pathPatternOff), i, packagePath), packagePath, keyName);
                                    setIniFileValue((packagePath + configFileName).c_str(), keyName.c_str(), footerStr, state ? onStr : offStr);
                                });
                                list->addItem(toggleListItem.release());
                            }
                        }
                    }
                }
                
                if (hideUserGuide != trueStr && dropdownSection.empty())
                    addHelpInfo(list);
            }
        }
        if (initializingSpawn) {
            initializingSpawn = false;
            useCombo2 = true;
            return createUI(); 
        }
        
        filesList.clear();

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel, menuMode+hiddenMenuMode+dropdownSection);
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(upperProjectName, versionLabel, menuMode+hiddenMenuMode+dropdownSection);

        rootFrame->setContent(list.release());
        
        return rootFrame.release();
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

        if (runningInterpreter.load(std::memory_order_acquire))
            return handleRunningInterpreter(keysHeld);
        
        if (lastRunningInterpreter) {
            while (!interpreterThreadExit.load()) {svcSleepThread(50'000'000);}
            
            resetPercentages();
            
            isDownloadCommand = false;
            lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        if (refreshGui && !stillTouching) {
            refreshGui = false;
            tsl::pop();
            tsl::changeTo<MainMenu>(hiddenMenuMode);
            return true;
        }

        if (!dropdownSection.empty() && !returningToMain) {
            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }

            if ((keysHeld & KEY_B) && !stillTouching) {
                returningToMain = true;
                tsl::goBack();
                simulatedBackComplete = true;
                return true;
            }
        }
        
        if (inMainMenu && !inHiddenMode && dropdownSection.empty()){
            if (isDownloaded) // for handling software updates
                tsl::Overlay::get()->close();
            
            if (!freshSpawn && !returningToMain && !returningToHiddenMain) {
                
                if (simulatedNextPage && !simulatedNextPageComplete) {
                    if (menuMode != packagesStr) {
                        keysHeld |= KEY_DRIGHT;
                        simulatedNextPage = false;
                    }
                    else if (menuMode != overlaysStr) {
                        keysHeld |= KEY_DLEFT;
                        simulatedNextPage = false;
                    } else {
                        simulatedNextPage = false;
                        simulatedNextPageComplete = true;
                    }
                }

                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    if (menuMode != packagesStr) {
                        defaultMenu = packagesStr;
                        selectedListItem.reset();
                        lastSelectedListItem.reset();
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        simulatedNextPageComplete = true;
                        return true;
                    }
                }
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    if (menuMode != overlaysStr) {
                        defaultMenu = overlaysStr;
                        selectedListItem.reset();
                        lastSelectedListItem.reset();
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        simulatedNextPageComplete = true;
                        return true;
                    }
                }

                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }

                if ((keysHeld & KEY_B) && !stillTouching) {
                    tsl::Overlay::get()->close();
                    simulatedBackComplete = true;
                    return true;
                }

                if (simulatedMenu && !simulatedMenuComplete) {
                    keysHeld |= SYSTEM_SETTINGS_KEY;
                    simulatedMenu = false;
                }

                if ((keysHeld & SYSTEM_SETTINGS_KEY) && !stillTouching) {
                    tsl::changeTo<UltrahandSettingsMenu>();
                    simulatedMenuComplete = true;
                    return true;
                }
            }
        }
        if (!inMainMenu && inHiddenMode) {
            if (!returningToHiddenMain && !returningToMain) {

                if (simulatedNextPage && !simulatedNextPageComplete) {
                    simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }

                if (simulatedMenu && !simulatedMenuComplete) {
                    simulatedMenu = false;
                    simulatedMenuComplete = true;
                }

                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }

                if ((keysHeld & KEY_B) && !stillTouching) {
                    if (parseValueFromIniSection(settingsConfigIniPath, projectName, inHiddenOverlayStr) == falseStr) {
                        inMainMenu = true;
                        inHiddenMode = false;
                        hiddenMenuMode = "";
                        setIniFileValue(settingsConfigIniPath, projectName, inHiddenOverlayStr, "");
                        tsl::pop();
                        returningToMain = true;
                        tsl::changeTo<MainMenu>();
                        simulatedBackComplete = true;
                        return true;
                    }

                    returningToMain = true;
                    inHiddenMode = false;
                    
                    if (reloadMenu2) {
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        reloadMenu2 = false;
                        simulatedBackComplete = true;
                        return true;
                    }
                    
                    tsl::goBack();
                    simulatedBackComplete = true;
                    return true;
                }
            }
        }
        
        if (freshSpawn && !(keysHeld & KEY_B))
            freshSpawn = false;
        
        if (returningToMain && !(keysHeld & KEY_B)){
            returningToMain = false;
            inMainMenu = true;
        }
        if (returningToHiddenMain && !(keysHeld & KEY_B)){
            returningToHiddenMain = false;
            inHiddenMode = true;
        }
        
        if (redrawWidget) {
            reinitializeWidgetVars();
            redrawWidget = false;
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
        //svcSleepThread(10'000'000);
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
        closeInterpreterThread(); // shouldn't be running, but run close anyways
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
    virtual void onShow() override {} 
    
    /**
     * @brief Performs actions when the overlay becomes visible.
     *
     * This function is called when the overlay transitions from an invisible state to a visible state.
     * It can be used to perform actions or updates specific to the overlay's visibility.
     */
    virtual void onHide() override {} 
    
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
