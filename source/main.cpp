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
#include <set>

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
//static bool currentMenuLoaded = true;
static bool freshSpawn = true;
//static bool refreshGui = false; (moved)
static bool reloadMenu = false;
static bool reloadMenu2 = false;
static bool reloadMenu3 = false;
static bool isDownloaded = false;

static bool redrawWidget = false;

static size_t nestedMenuCount = 0;

// Command mode globals
static const std::vector<std::string> commandSystems = {DEFAULT_STR, ERISTA_STR, MARIKO_STR};
static const std::vector<std::string> commandModes = {DEFAULT_STR, SLOT_STR, TOGGLE_STR, OPTION_STR, FORWARDER_STR, TEXT_STR, TABLE_STR};
static const std::vector<std::string> commandGroupings = {DEFAULT_STR, "split", "split2", "split3", "split4"};
static const std::string MODE_PATTERN = ";mode=";
static const std::string GROUPING_PATTERN = ";grouping=";
static const std::string SYSTEM_PATTERN = ";system=";

// Table option patterns
static const std::string BACKGROUND_PATTERN = ";background="; // true or false
static const std::string HEADER_PATTERN = ";header=";
static const std::string ALIGNMENT_PATTERN = ";alignment=";
static const std::string GAP_PATTERN =";gap=";
static const std::string OFFSET_PATTERN = ";offset=";
static const std::string SPACING_PATTERN = ";spacing=";

static std::string currentMenu = OVERLAYS_STR;
static std::string lastPage = LEFT_STR;
static std::string lastPackagePath;
static std::string lastPackageName;
static std::string lastPackageMenu;

static std::string lastMenu = "";
static std::string lastMenuMode = "";
static std::string lastKeyName = "";
static bool hideUserGuide = false;

static std::unordered_map<std::string, std::string> selectedFooterDict;

static std::shared_ptr<tsl::elm::ListItem> selectedListItem;
static std::shared_ptr<tsl::elm::ListItem> lastSelectedListItem;

static bool lastRunningInterpreter = false;



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
    static auto last_call = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    bool shouldAbort = false;

    if (now - last_call < std::chrono::milliseconds(20)) {
        return false;  // Exit if the minimum interval hasn't passed
    }
    last_call = now;  // Update last_call to the current time

    // Helper lambda to update the UI and manage completion state
    auto updateUI = [&](std::atomic<int>& percentage, const std::string& symbol) {
        int currentPercentage = percentage.load(std::memory_order_acquire);
        if (currentPercentage != -1) {
            if (currentPercentage != lastPercentage) {
                lastSelectedListItem->setValue(symbol + " " + std::to_string(currentPercentage) + "%");
                lastPercentage = currentPercentage;
            }
            if (currentPercentage == 100) {
                inProgress = true;  // This seems to be intended to indicate task completion, but setting 'true' here every time might be a mistake?
                percentage.store(-1, std::memory_order_release);
            }
            return true;
        }
        return false;
    };

    if (!updateUI(downloadPercentage, DOWNLOAD_SYMBOL) &&
        !updateUI(unzipPercentage, UNZIP_SYMBOL) &&
        !updateUI(copyPercentage, COPY_SYMBOL) &&
        lastPercentage == -1 && inProgress) {
        lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
        inProgress = false;
    }

    if (threadFailure.load(std::memory_order_acquire)) {
        threadFailure.store(false, std::memory_order_release);
        commandSuccess = false;
    }

    if ((keysHeld & KEY_R) && (keysHeld & ~(KEY_DLEFT | KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_ZL | KEY_ZR | KEY_R)) == 0 && !stillTouching) {
        commandSuccess = false;
        abortDownload.store(true, std::memory_order_release);
        abortUnzip.store(true, std::memory_order_release);
        abortFileOp.store(true, std::memory_order_release);
        abortCommand.store(true, std::memory_order_release);
        shouldAbort = true;
    }

    //if (!shouldAbort) {
    //    svcSleepThread(10'000'000); // 10 ms sleep
    //}

    return shouldAbort;
}



// Forward declaration of the MainMenu class.
class MainMenu;

class UltrahandSettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, dropdownSelection, settingsIniPath;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false;
    std::string languagesVersion = APP_VERSION;
    int MAX_PRIORITY = 20;
    std::string comboLabel;

    void addToggleListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, bool state, const std::string& key) {
        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(title, state, ON, OFF);
        toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get(), key](bool state) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
            setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, key, state ? FALSE_STR : TRUE_STR);
            reinitializeWidgetVars();
            redrawWidget = true;
        });
        list->addItem(toggleListItem.release());
    }

    // Helper function to add toggle list items
    void addToggleItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, bool& stateVar, const std::string& iniKey, const std::function<void()>& onChangeCallback = nullptr) {
        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(title, stateVar, ON, OFF);
        toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get(), &stateVar, iniKey, onChangeCallback](bool state) mutable {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
            setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, state ? TRUE_STR : FALSE_STR);
            if (stateVar != state) {
                stateVar = state;
                if (onChangeCallback) onChangeCallback();
            }
        });
        list->addItem(toggleListItem.release());
    }

    void addListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, const std::string& value, const std::string& targetMenu) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(title);
        listItem->setValue(value);
        listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){}), targetMenu](uint64_t keys) {
            if (runningInterpreter.load(std::memory_order_acquire))
                return false;

            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
            }
            if (keys & KEY_A) {
                shiftItemFocus(listItemPtr);
                tsl::changeTo<UltrahandSettingsMenu>(targetMenu);
                selectedListItem = listItemPtr;
                simulatedSelectComplete = true;
                return true;
            }
            return false;
        });
        list->addItem(listItem.release());
    }

    void handleSelection(std::unique_ptr<tsl::elm::List>& list, const std::vector<std::string>& items, const std::string& defaultItem, const std::string& iniKey, const std::string& targetMenu) {
        for (const auto& item : items) {
            auto mappedItem = convertComboToUnicode(item);
            if (mappedItem.empty()) mappedItem = item;
    
            auto listItem = std::make_unique<tsl::elm::ListItem>(mappedItem);
            if (item == defaultItem) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }
            listItem->setClickListener([item, mappedItem, defaultItem, iniKey, targetMenu,
                listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;
    
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    if (item != defaultItem) {
                        setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, item);
                        if (targetMenu == "keyComboMenu")
                            setIniFileValue(TESLA_CONFIG_INI_PATH, TESLA_STR, iniKey, item);
                        reloadMenu = true;
                    }
                    lastSelectedListItem->setValue("");
                    selectedListItem->setValue(mappedItem);
                    listItemPtr->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
    
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
        }
    }


    void addUpdateButton(std::unique_ptr<tsl::elm::List>& list, const std::string& title, const std::string& downloadUrl, const std::string& targetPath, const std::string& movePath) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(title);
        listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){}), downloadUrl, targetPath, movePath](uint64_t keys) {
            if (runningInterpreter.load(std::memory_order_acquire)) {
                return false;
            }

            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
            }
            std::vector<std::vector<std::string>> interpreterCommands;
            if (keys & KEY_A) {
                isDownloadCommand = true;
                

                if (movePath == LANG_PATH) {
                    interpreterCommands = {
                        {"try:"},
                        {"delete", targetPath},
                        {"download", downloadUrl, DOWNLOADS_PATH},
                        {"unzip", targetPath, movePath},
                        {"delete", targetPath}
                    };
                } else {
                    interpreterCommands = {
                        {"try:"},
                        {"delete", targetPath},
                        {"download", downloadUrl, DOWNLOADS_PATH},
                        {"move", targetPath, movePath}
                    };
                }
                runningInterpreter.store(true, std::memory_order_release);
                enqueueInterpreterCommands(std::move(interpreterCommands), "", "");
                startInterpreterThread();

                listItemPtr->setValue(INPROGRESS_SYMBOL);
                shiftItemFocus(listItemPtr);
                lastSelectedListItem = listItemPtr;
                lastRunningInterpreter = true;
                simulatedSelectComplete = true;
                lastSelectedListItem->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem.release());
    }


    //void addLanguageUpdateButton(std::unique_ptr<tsl::elm::List>& list, const std::string& title, const std::string& downloadUrl, const std::string& targetPath, const std::string& extractPath) {
    //    auto listItem = std::make_unique<tsl::elm::ListItem>(title);
    //    listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){}), downloadUrl, targetPath, extractPath](uint64_t keys) {
    //        if (runningInterpreter.load(std::memory_order_acquire)) {
    //            return false;
    //        }
    //
    //        if (simulatedSelect && !simulatedSelectComplete) {
    //            keys |= KEY_A;
    //            simulatedSelect = false;
    //        }
    //
    //        if (keys & KEY_A) {
    //            isDownloadCommand = true;
    //            std::vector<std::vector<std::string>> interpreterCommands = {
    //                {"try:"},
    //                {"delete", targetPath},
    //                {"download", downloadUrl, DOWNLOADS_PATH},
    //                {"unzip", targetPath, extractPath},
    //                {"delete", targetPath}
    //            };
    //            runningInterpreter.store(true, std::memory_order_release);
    //            enqueueInterpreterCommands(std::move(interpreterCommands), "", "");
    //            startInterpreterThread();
    //
    //            listItemPtr->setValue(INPROGRESS_SYMBOL);
    //            shiftItemFocus(listItemPtr);
    //            lastSelectedListItem = listItemPtr;
    //            lastRunningInterpreter = true;
    //            simulatedSelectComplete = true;
    //            lastSelectedListItem->triggerClickAnimation();
    //            return true;
    //        }
    //        return false;
    //    });
    //    list->addItem(listItem.release());
    //}
    
    
public:
    UltrahandSettingsMenu(const std::string& selection = "") : dropdownSelection(selection) {}

    ~UltrahandSettingsMenu() {}

    virtual tsl::elm::Element* createUI() override {
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();
        
        const std::vector<std::string> defaultLanguages = {"en", "es", "fr", "de", "ja", "ko", "it", "nl", "pt", "ru", "zh-cn", "zh-tw"};
        const std::vector<std::string> defaultCombos = {"ZL+ZR+DDOWN", "ZL+ZR+DRIGHT", "ZL+ZR+DUP", "ZL+ZR+DLEFT", "L+R+DDOWN", "L+R+DRIGHT", "L+R+DUP", "L+R+DLEFT", "L+DDOWN", "R+DDOWN", "ZL+ZR+PLUS", "L+R+PLUS", "ZL+PLUS", "ZR+PLUS", "MINUS+PLUS", "L+DDOWN+RS"};
        
        auto list = std::make_unique<tsl::elm::List>();
        
        if (dropdownSelection.empty()) {
            list->addItem(new tsl::elm::CategoryHeader(MAIN_SETTINGS));
            std::string defaultLang = parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR);
            std::string keyCombo = trim(parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR));
            defaultLang = defaultLang.empty() ? "en" : defaultLang;
            keyCombo = keyCombo.empty() ? "ZL+ZR+DDOWN" : keyCombo;

            comboLabel = convertComboToUnicode(keyCombo);
            if (comboLabel.empty()) comboLabel = keyCombo;
            addListItem(list, KEY_COMBO, comboLabel, "keyComboMenu");
            addListItem(list, LANGUAGE, defaultLang, "languageMenu");
            addListItem(list, SOFTWARE_UPDATE, DROPDOWN_SYMBOL, "softwareUpdateMenu");

            list->addItem(new tsl::elm::CategoryHeader(UI_SETTINGS));
            std::string currentTheme = parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme");
            currentTheme = (currentTheme.empty() || currentTheme == DEFAULT_STR) ? DEFAULT : currentTheme;
            addListItem(list, THEME, currentTheme, "themeMenu");
            addListItem(list, WIDGET, DROPDOWN_SYMBOL, "widgetMenu");
            addListItem(list, MISCELLANEOUS, DROPDOWN_SYMBOL, "miscMenu");

        } else if (dropdownSelection == "keyComboMenu") {
            list->addItem(new tsl::elm::CategoryHeader(KEY_COMBO));
            std::string defaultCombo = trim(parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR));
            handleSelection(list, defaultCombos, defaultCombo, KEY_COMBO_STR, "keyComboMenu");
        } else if (dropdownSelection == "languageMenu") {
            list->addItem(new tsl::elm::CategoryHeader(LANGUAGE));
            std::string defaulLang = parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR);
            for (const auto& defaultLangMode : defaultLanguages) {
                std::string langFile = LANG_PATH + defaultLangMode + ".json";
                if (defaultLangMode != "en" && !isFileOrDirectory(langFile)) continue;
                auto listItem = std::make_unique<tsl::elm::ListItem>(defaultLangMode);
                if (defaultLangMode == defaulLang) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                listItem->setClickListener([skipLang = !isFileOrDirectory(langFile), defaultLangMode, defaulLang, langFile,
                    listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                    if (runningInterpreter.load(std::memory_order_acquire)) return false;
                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        shiftItemFocus(listItemPtr);
                        setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR, defaultLangMode);
                        reloadMenu = reloadMenu2 = true;
                        parseLanguage(langFile);
                        if (skipLang) reinitializeLangVars();
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(defaultLangMode);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItemPtr;
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());
            }
        } else if (dropdownSelection == "softwareUpdateMenu") {
            list->addItem(new tsl::elm::CategoryHeader(SOFTWARE_UPDATE));
            addUpdateButton(list, UPDATE_ULTRAHAND, ULTRAHAND_REPO_URL + "releases/latest/download/ovlmenu.ovl", "/config/ultrahand/downloads/ovlmenu.ovl", "/switch/.overlays/ovlmenu.ovl");
            addUpdateButton(list, UPDATE_LANGUAGES, ULTRAHAND_REPO_URL + "releases/latest/download/lang.zip", "/config/ultrahand/downloads/lang.zip", LANG_PATH);

            PackageHeader overlayHeader;
            overlayHeader.title = "Ultrahand Overlay";
            overlayHeader.version = APP_VERSION;
            overlayHeader.creator = "ppkantorski";
            overlayHeader.about = "Ultrahand Overlay is a versatile tool that enables you to create and share custom command-based packages.";
            overlayHeader.credits = "Special thanks to B3711, ComplexNarrative, Faker_dev, MasaGratoR, meha, WerWolv, HookedBehemoth and many others. <3";
            addPackageInfo(list, overlayHeader, OVERLAY_STR);
            overlayHeader.clear();

        } else if (dropdownSelection == "themeMenu") {
            list->addItem(new tsl::elm::CategoryHeader(THEME));
            std::string currentTheme = parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme");
            currentTheme = currentTheme.empty() ? DEFAULT_STR : currentTheme;
            auto listItem = std::make_unique<tsl::elm::ListItem>(DEFAULT);
            if (currentTheme == DEFAULT_STR) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }
            listItem->setClickListener([defaultTheme = THEMES_PATH + "default.ini", listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire)) return false;
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    shiftItemFocus(listItemPtr);
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", DEFAULT_STR);
                    deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                    if (isFileOrDirectory(defaultTheme)) copyFileOrDirectory(defaultTheme, THEME_CONFIG_INI_PATH);
                    else initializeTheme();
                    tsl::initializeThemeVars();
                    reloadMenu = reloadMenu2 = true;
                    lastSelectedListItem->setValue("");
                    selectedListItem->setValue(DEFAULT);
                    listItemPtr->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());

            std::vector<std::string> themeFilesList = getFilesListByWildcards(THEMES_PATH + "*.ini");
            std::sort(themeFilesList.begin(), themeFilesList.end());
            for (const auto& themeFile : themeFilesList) {
                std::string themeName = dropExtension(getNameFromPath(themeFile));
                if (themeName == DEFAULT_STR) continue;
                listItem = std::make_unique<tsl::elm::ListItem>(themeName);
                if (themeName == currentTheme) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                listItem->setClickListener([themeName, themeFile, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                    if (runningInterpreter.load(std::memory_order_acquire)) return false;
                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        shiftItemFocus(listItemPtr);
                        setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", themeName);
                        deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                        copyFileOrDirectory(themeFile, THEME_CONFIG_INI_PATH);
                        initializeTheme();
                        tsl::initializeThemeVars();
                        reloadMenu = reloadMenu2 = true;
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(themeName);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItemPtr;
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());
            }
        } else if (dropdownSelection == "widgetMenu") {
            list->addItem(new tsl::elm::CategoryHeader(WIDGET));
            addToggleListItem(list, CLOCK, !hideClock, "hide_clock");
            addToggleListItem(list, SOC_TEMPERATURE, !hideSOCTemp, "hide_soc_temp");
            addToggleListItem(list, PCB_TEMPERATURE, !hidePCBTemp, "hide_pcb_temp");
            addToggleListItem(list, BATTERY, !hideBattery, "hide_battery");

        } else if (dropdownSelection == "miscMenu") {
            list->addItem(new tsl::elm::CategoryHeader(MENU_ITEMS));
        
            // Helper function to create toggle list items
            auto createToggleListItem = [&](const std::string& title, bool& state, const std::string& iniKey, bool invertLogic = false, bool useReloadMenu2 = false) {
                auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(title, invertLogic ? !state : state, ON, OFF);
                toggleListItem->setStateChangedListener([&, listItemRaw = toggleListItem.get(), iniKey, invertLogic, useReloadMenu2](bool newState) {
                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, newState ? (invertLogic ? FALSE_STR : TRUE_STR) : (invertLogic ? TRUE_STR : FALSE_STR));
                    if ((invertLogic ? !state : state) != newState) {
                        state = invertLogic ? !newState : newState;
                        if (iniKey == "clean_version_labels") {
                            versionLabel = APP_VERSION + std::string("   (") + extractTitle(loaderInfo) + (state ? " v" : " ") + cleanVersionLabel(loaderInfo) + std::string(")");
                            reinitializeVersionLabels();
                        }
                        reloadMenu = true;
                        if (useReloadMenu2) reloadMenu2 = true;
                    }
                });
                list->addItem(toggleListItem.release());
            };
        
            hideUserGuide = (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_user_guide") == TRUE_STR);
            createToggleListItem(USER_GUIDE, hideUserGuide, "hide_user_guide", true);
        
            cleanVersionLabels = (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "clean_version_labels") == TRUE_STR);
            createToggleListItem(CLEAN_VERSIONS, cleanVersionLabels, "clean_version_labels", false, true);
        
            hideOverlayVersions = (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_overlay_versions") == TRUE_STR);
            createToggleListItem(OVERLAY_VERSIONS, hideOverlayVersions, "hide_overlay_versions", true);
        
            hidePackageVersions = (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_package_versions") == TRUE_STR);
            createToggleListItem(PACKAGE_VERSIONS, hidePackageVersions, "hide_package_versions", true);
        
            list->addItem(new tsl::elm::CategoryHeader(EFFECTS));
        
            useOpaqueScreenshots = (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "opaque_screenshots") == TRUE_STR);
            createToggleListItem(OPAQUE_SCREENSHOTS, useOpaqueScreenshots, "opaque_screenshots");
        
            progressAnimation = (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "progress_animation") == TRUE_STR);
            createToggleListItem(PROGRESS_ANIMATION, progressAnimation, "progress_animation");
        
        } else {
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN + ": " + settingsIniPath));
        }

        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        rootFrame->setContent(list.release());
        return rootFrame.release();
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
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
                    returningToMain = (lastMenu != "hiddenMenuMode");
                    returningToHiddenMain = !returningToMain;
                    lastMenu = "settingsMenu";
                    tsl::goBack();
                    if (reloadMenu) {
                        tsl::changeTo<MainMenu>(lastMenuMode);
                        reloadMenu = false;
                    }
                    simulatedBackComplete = true;
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
                simulatedBackComplete = true;
                return true;
            }
        }

        if (returningToSettings && !(keysHeld & KEY_B)) {
            returningToSettings = false;
            inSettingsMenu = true;
            tsl::impl::parseOverlaySettings();
        }

        if (redrawWidget) reinitializeWidgetVars();

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }

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
     * @brief Constructs a `SettingsMenu` instance.
     *
     * Initializes a new instance of the `SettingsMenu` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     */
    SettingsMenu(const std::string& name, const std::string& mode, const std::string& overlayName="", const std::string& selection = "") :
        entryName(name), entryMode(mode), overlayName(overlayName), dropdownSelection(selection) {}
    
    /**
     * @brief Destroys the `SettingsMenu` instance.
     *
     * Cleans up any resources associated with the `SettingsMenu` instance.
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
        if (entryMode == OVERLAY_STR) {
            settingsIniPath = OVERLAYS_INI_FILEPATH;
            header = overlayName;
        } else if (entryMode == PACKAGE_STR)
            settingsIniPath = PACKAGES_INI_FILEPATH;
        
        if (dropdownSelection.empty())
            inSettingsMenu = true;
        else
            inSubSettingsMenu = true;
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();
        
        if (dropdownSelection.empty()) {
            list->addItem(new tsl::elm::CategoryHeader(header+" "+SETTINGS));
            
            
            std::string fileContent = getFileContents(settingsIniPath);
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, PRIORITY_STR);
            
            std::string hideOption = parseValueFromIniSection(settingsIniPath, entryName, HIDE_STR);
            if (hideOption.empty())
                hideOption = FALSE_STR;
            
            
            bool hide = false;
            if (hideOption == TRUE_STR)
                hide = true;
            
            std::string useOverlayLaunchArgs = parseValueFromIniSection(settingsIniPath, entryName, USE_LAUNCH_ARGS_STR);
            
            
            // Capitalize entryMode
            std::string hideLabel = entryMode;
            
            
            if (hideLabel == OVERLAY_STR)
                hideLabel = HIDE_OVERLAY;
            else if (hideLabel == PACKAGE_STR)
                hideLabel = HIDE_PACKAGE;
            
            
            // Envoke toggling
            auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(hideLabel, false, ON, OFF);
            toggleListItem->setState(hide);
            toggleListItem->setStateChangedListener([&settingsIniPath = this->settingsIniPath, &entryName = this->entryName, listItemRaw = toggleListItem.get()](bool state) {
                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                setIniFileValue(settingsIniPath, entryName, HIDE_STR, state ? TRUE_STR : FALSE_STR);
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
            
            listItem->setClickListener([&entryName = this->entryName, &entryMode = this->entryMode, &overlayName = this->overlayName,
                listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    tsl::changeTo<SettingsMenu>(entryName, entryMode, overlayName, PRIORITY_STR);
                    selectedListItem.reset();
                    selectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            if (entryMode == OVERLAY_STR) {
                // Envoke toggling
                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(LAUNCH_ARGUMENTS, false, ON, OFF);
                toggleListItem->setState((useOverlayLaunchArgs==TRUE_STR));
                toggleListItem->setStateChangedListener([&settingsIniPath = settingsIniPath, &entryName = entryName, useOverlayLaunchArgs,
                    listItemRaw = toggleListItem.get()](bool state) {

                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                    setIniFileValue(settingsIniPath, entryName, USE_LAUNCH_ARGS_STR, state ? TRUE_STR : FALSE_STR);
                    if ((useOverlayLaunchArgs==TRUE_STR) != state)
                        reloadMenu = true; // this reloads before main menu
                    if (!state) {
                        reloadMenu = true;
                        reloadMenu2 = true; // this reloads at main menu
                    }
                });
                list->addItem(toggleListItem.release());
            }
            
            
        } else if (dropdownSelection == PRIORITY_STR) {
            list->addItem(new tsl::elm::CategoryHeader(SORT_PRIORITY));
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, PRIORITY_STR);
            
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
                
                listItem->setClickListener([&settingsIniPath = this->settingsIniPath, &entryName = this->entryName, iStr, priorityValue,
                    listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    
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
                        setIniFileValue(settingsIniPath, entryName, PRIORITY_STR, iStr);
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(iStr);
                        listItemPtr->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
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
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
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
            //while (!interpreterThreadExit.load(std::memory_order_acquire)) {svcSleepThread(50'000'000);}
            
            //resetPercentages();
            
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


class ScriptOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey, fileName;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false;

    void addListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& line) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(line);
        std::shared_ptr<tsl::elm::ListItem> listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});

        listItem->setClickListener([this, listItemPtr, line](uint64_t keys) {
            if (runningInterpreter.load(std::memory_order_acquire)) return false;
            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
            }
            if (keys & KEY_A) {
                std::istringstream iss(line), argIss;
                std::string part, arg;
                std::vector<std::vector<std::string>> commandVec;
                std::vector<std::string> commandParts;
                bool inQuotes = false;

                while (std::getline(iss, part, '\'')) {
                    if (!part.empty()) {
                        if (!inQuotes) {
                            argIss.clear();
                            argIss.str(part);
                            while (argIss >> arg) commandParts.emplace_back(arg);
                        } else {
                            commandParts.emplace_back(part);
                        }
                    }
                    inQuotes = !inQuotes;
                }
                commandVec.emplace_back(std::move(commandParts));
                interpretAndExecuteCommands(std::move(commandVec), filePath, specificKey);

                listItemPtr->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
                simulatedSelectComplete = true;
                listItemPtr->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem.release());
    }

public:
    ScriptOverlay(const std::string& file, const std::string& key = "", bool fromMainMenu = false, const std::string& _fileName = PACKAGE_FILENAME)
        : filePath(file), specificKey(key), fileName(_fileName), isFromMainMenu(fromMainMenu) {}

    virtual tsl::elm::Element* createUI() override {
        inScriptMenu = true;
        std::string packageName = getNameFromPath(filePath);
        if (packageName == ".packages") packageName = ROOT_PACKAGE;

        auto list = std::make_unique<tsl::elm::List>();
        const std::string& packageFile = filePath + fileName;
        const std::string& fileContent = getFileContents(packageFile);

        if (!fileContent.empty()) {
            std::string line, currentCategory;
            std::istringstream iss(fileContent);

            while (std::getline(iss, line)) {
                if (line.empty() || line.find_first_not_of('\n') == std::string::npos) continue;

                if (line.front() == '[' && line.back() == ']') {
                    currentCategory = line.substr(1, line.size() - 2);
                    isInSection = specificKey.empty() || currentCategory == specificKey;
                    if (isInSection) list->addItem(new tsl::elm::CategoryHeader(currentCategory));
                } else if (isInSection) {
                    addListItem(list, line);
                }
            }
        } else {
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN + ": " + packageFile));
        }

        PackageHeader packageHeader = getPackageHeaderFromIni(packageFile);
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(packageName, packageHeader.version.empty() ? 
            CAPITAL_ULTRAHAND_PROJECT_NAME + " Script" : packageHeader.version + "   (" + CAPITAL_ULTRAHAND_PROJECT_NAME + " Script)");
        rootFrame->setContent(list.release());
        return rootFrame.release();
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (runningInterpreter.load(std::memory_order_acquire)) return handleRunningInterpreter(keysHeld);
        if (lastRunningInterpreter) {
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
                returningToPackage = !isFromMainMenu && lastMenu == "packageMenu";
                returningToSubPackage = !isFromMainMenu && lastMenu == "subPackageMenu";
                returningToMain = isFromMainMenu;
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

    // Variables moved from createUI to class scope
    std::vector<std::string> filesList, filesListOn, filesListOff;
    std::vector<std::string> filterList, filterListOn, filterListOff;
    std::string sourceType, sourceTypeOn, sourceTypeOff;
    std::string jsonPath, jsonPathOn, jsonPathOff;
    std::string jsonKey, jsonKeyOn, jsonKeyOff;
    std::string listPath, listPathOn, listPathOff;
    std::string listString, listStringOn, listStringOff;
    std::string jsonString, jsonStringOn, jsonStringOff;

public:
    SelectionOverlay(const std::string& path, const std::string& key = "", const std::vector<std::vector<std::string>>& cmds = {}, const std::string& footerKey = "")
        : filePath(path), specificKey(key), commands(std::move(cmds)), specifiedFooterKey(footerKey) {
        lastSelectedListItem.reset();
    }

    ~SelectionOverlay() {
        lastSelectedListItem.reset();
    }

    void processCommands() {
        commands.erase(std::remove_if(commands.begin(), commands.end(),
            [](const std::vector<std::string>& vec) {
                return vec.empty();
            }),
            commands.end());

        bool inEristaSection = false;
        bool inMarikoSection = false;
        std::string currentSection = GLOBAL_STR;

        for (const auto& cmd : commands) {
            std::string commandName = cmd[0];

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
                if (commandName.find(SYSTEM_PATTERN) == 0) {
                    commandSystem = commandName.substr(SYSTEM_PATTERN.length());
                    if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                        commandSystem = commandSystems[0];
                } else if (commandName.find(MODE_PATTERN) == 0) {
                    commandMode = commandName.substr(MODE_PATTERN.length());
                    if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                        commandMode = commandModes[0];
                } else if (commandName.find(GROUPING_PATTERN) == 0) {
                    commandGrouping = commandName.substr(GROUPING_PATTERN.length());
                    if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                        commandGrouping = commandGroupings[0];
                }

                if (commandMode == TOGGLE_STR) {
                    if (commandName == "on:")
                        currentSection = ON_STR;
                    else if (commandName == "off:")
                        currentSection = OFF_STR;

                    if (currentSection == GLOBAL_STR) {
                        commandsOn.push_back(cmd);
                        commandsOff.push_back(cmd);
                    } else if (currentSection == ON_STR)
                        commandsOn.push_back(cmd);
                    else if (currentSection == OFF_STR)
                        commandsOff.push_back(cmd);
                }

                if (cmd.size() > 1) {
                    if (commandName == "filter") {
                        if (currentSection == GLOBAL_STR)
                            filterList.push_back(std::move(cmd[1]));
                        else if (currentSection == ON_STR)
                            filterListOn.push_back(std::move(cmd[1]));
                        else if (currentSection == OFF_STR)
                            filterListOff.push_back(std::move(cmd[1]));
                    } else if (commandName == "file_source") {
                        sourceType = FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            pathPattern = cmd[1];
                            filesList = getFilesListByWildcards(pathPattern);
                        } else if (currentSection == ON_STR) {
                            pathPatternOn = cmd[1];
                            filesListOn = getFilesListByWildcards(pathPatternOn);
                            sourceTypeOn = FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            pathPatternOff = cmd[1];
                            filesListOff = getFilesListByWildcards(pathPatternOff);
                            sourceTypeOff = FILE_STR;
                        }
                    } else if (commandName == "json_file_source") {
                        sourceType = JSON_FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            jsonPath = preprocessPath(cmd[1], filePath);
                            if (cmd.size() > 2)
                                jsonKey = cmd[2];
                        } else if (currentSection == ON_STR) {
                            jsonPathOn = preprocessPath(cmd[1], filePath);
                            sourceTypeOn = JSON_FILE_STR;
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2];
                        } else if (currentSection == OFF_STR) {
                            jsonPathOff = preprocessPath(cmd[1], filePath);
                            sourceTypeOff = JSON_FILE_STR;
                            if (cmd.size() > 2)
                                jsonKeyOff = cmd[2];
                        }
                    } else if (commandName == "list_file_source") {
                        sourceType = LIST_FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            listPath = preprocessPath(cmd[1], filePath);
                        } else if (currentSection == ON_STR) {
                            listPathOn = preprocessPath(cmd[1], filePath);
                            sourceTypeOn = LIST_FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            listPathOff = preprocessPath(cmd[1], filePath);
                            sourceTypeOff = LIST_FILE_STR;
                        }
                    } else if (commandName == "list_source") {
                        sourceType = LIST_STR;
                        if (currentSection == GLOBAL_STR) {
                            listString = removeQuotes(cmd[1]);
                        } else if (currentSection == ON_STR) {
                            listStringOn = removeQuotes(cmd[1]);
                            sourceTypeOn = LIST_STR;
                        } else if (currentSection == OFF_STR) {
                            listStringOff = removeQuotes(cmd[1]);
                            sourceTypeOff = LIST_STR;
                        }
                    } else if (commandName == "json_source") {
                        sourceType = JSON_STR;
                        if (currentSection == GLOBAL_STR) {
                            jsonString = removeQuotes(cmd[1]);
                            if (cmd.size() > 2)
                                jsonKey = cmd[2];
                        } else if (currentSection == ON_STR) {
                            jsonStringOn = removeQuotes(cmd[1]);
                            sourceTypeOn = JSON_STR;
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2];
                        } else if (currentSection == OFF_STR) {
                            jsonStringOff = removeQuotes(cmd[1]);
                            sourceTypeOff = JSON_STR;
                            if (cmd.size() > 2)
                                jsonKeyOff = cmd[2];
                        }
                    }
                }
            }
        }
    }

    virtual tsl::elm::Element* createUI() override {
        inSelectionMenu = true;
        PackageHeader packageHeader = getPackageHeaderFromIni(filePath + PACKAGE_FILENAME);

        auto list = std::make_unique<tsl::elm::List>();
        packageConfigIniPath = filePath + CONFIG_FILENAME;

        commandSystem = commandSystems[0];
        commandMode = commandModes[0];
        commandGrouping = commandGroupings[0];

        processCommands();

        std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;

        if (commandMode == DEFAULT_STR || commandMode == OPTION_STR) {
            if (sourceType == FILE_STR)
                selectedItemsList = std::move(filesList);
            else if (sourceType == LIST_STR || sourceType == LIST_FILE_STR)
                selectedItemsList = (sourceType == LIST_STR) ? stringToList(listString) : readListFromFile(listPath);
            else if (sourceType == JSON_STR || sourceType == JSON_FILE_STR) {
                populateSelectedItemsList(sourceType, (sourceType == JSON_STR) ? jsonString : jsonPath, jsonKey, selectedItemsList);
                jsonPath.clear();
                jsonString.clear();
            }
        } else if (commandMode == TOGGLE_STR) {
            if (sourceTypeOn == FILE_STR)
                selectedItemsListOn = std::move(filesListOn);
            else if (sourceTypeOn == LIST_STR || sourceTypeOn == LIST_FILE_STR)
                selectedItemsListOn = (sourceTypeOn == LIST_STR) ? stringToList(listStringOn) : readListFromFile(listPathOn);
            else if (sourceTypeOn == JSON_STR || sourceTypeOn == JSON_FILE_STR) {
                populateSelectedItemsList(sourceTypeOn, (sourceTypeOn == JSON_STR) ? jsonStringOn : jsonPathOn, jsonKeyOn, selectedItemsListOn);
                jsonPathOn.clear();
                jsonStringOn.clear();
            }

            if (sourceTypeOff == FILE_STR)
                selectedItemsListOff = std::move(filesListOff);
            else if (sourceTypeOff == LIST_STR || sourceTypeOff == LIST_FILE_STR)
                selectedItemsListOff = (sourceTypeOff == LIST_STR) ? stringToList(listStringOff) : readListFromFile(listPathOff);
            else if (sourceTypeOff == JSON_STR || sourceTypeOff == JSON_FILE_STR) {
                populateSelectedItemsList(sourceTypeOff, (sourceTypeOff == JSON_STR) ? jsonStringOff : jsonPathOff, jsonKeyOff, selectedItemsListOff);
                jsonPathOff.clear();
                jsonStringOff.clear();
            }

            filterItemsList(filterListOn, selectedItemsListOn);
            filterListOn.clear();

            filterItemsList(filterListOff, selectedItemsListOff);
            filterListOff.clear();

            selectedItemsList.reserve(selectedItemsListOn.size() + selectedItemsListOff.size());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOn.begin(), selectedItemsListOn.end());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOff.begin(), selectedItemsListOff.end());

            if (sourceType == FILE_STR && (commandGrouping == "split" || commandGrouping == "split2" || commandGrouping == "split3" || commandGrouping == "split4")) {
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

        filterItemsList(filterList, selectedItemsList);
        filterList.clear();

        if (commandGrouping == DEFAULT_STR)
            list->addItem(new tsl::elm::CategoryHeader(removeTag(specificKey.substr(1))));

        std::unique_ptr<tsl::elm::ListItem> listItem;
        size_t pos;
        std::string parentDirName;
        std::string footer;
        std::string optionName;
        //bool toggleStateOn;

        if (selectedItemsList.empty()) {
            listItem = std::make_unique<tsl::elm::ListItem>(EMPTY);
            list->addItem(listItem.release());
        }

        for (size_t i = 0; i < selectedItemsList.size(); ++i) {
            const std::string& selectedItem = selectedItemsList[i];

            itemName = getNameFromPath(selectedItem);
            if (itemName.front() == '.') // Skip hidden items
                continue;

            if (!isDirectory(preprocessPath(selectedItem, filePath)))
                itemName = dropExtension(itemName);

            if (sourceType == FILE_STR) {
                if (commandGrouping == "split") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem));

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                } else if (commandGrouping == "split2") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem));

                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3);
                        groupingName = groupingName.substr(0, pos);
                    }

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                } else if (commandGrouping == "split3") {
                    groupingName = removeQuotes(getNameFromPath(selectedItem));

                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3);
                        groupingName = groupingName.substr(0, pos);
                    }

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                } else if (commandGrouping == "split4") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem, 2));
                    itemName = trim(removeQuotes(dropExtension(getNameFromPath(selectedItem))));
                    footer = removeQuotes(getParentDirNameFromPath(selectedItem));

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
            }

            if (commandMode == DEFAULT_STR || commandMode == OPTION_STR) {
                if (sourceType != FILE_STR && commandGrouping != "split2" && commandGrouping != "split3" && commandGrouping != "split4") {
                    pos = selectedItem.find(" - ");
                    footer = "";
                    itemName = selectedItem;
                    if (pos != std::string::npos) {
                        footer = selectedItem.substr(pos + 2);
                        itemName = selectedItem.substr(0, pos);
                    }
                } else if (commandGrouping == "split2") {
                    footer = dropExtension(getNameFromPath(selectedItem));
                }

                listItem = std::make_unique<tsl::elm::ListItem>(itemName);

                if (commandMode == OPTION_STR) {
                    if (selectedFooterDict[specifiedFooterKey] == itemName) {
                        lastSelectedListItem.reset();
                        lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*) {});
                        lastSelectedListItemFooter = footer;
                        listItem->setValue(CHECKMARK_SYMBOL);
                    } else {
                        if (pos != std::string::npos) {
                            listItem->setValue(footer, true);
                        } else {
                            listItem->setValue(footer);
                        }
                    }
                } else {
                    listItem->setValue(footer, true);
                }

                listItem->setClickListener([&commands = this->commands, &filePath = this->filePath, &specificKey = this->specificKey, &commandMode = this->commandMode,
                    &specifiedFooterKey = this->specifiedFooterKey, &lastSelectedListItemFooter = this->lastSelectedListItemFooter, i, footer, selectedItem,
                    listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*) {})](uint64_t keys) {

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
                        if (commandMode == OPTION_STR) {
                            selectedFooterDict[specifiedFooterKey] = listItemPtr->getText();
                            if (lastSelectedListItem)
                                lastSelectedListItem->setValue(lastSelectedListItemFooter, true);
                            lastSelectedListItemFooter = footer;
                        }

                        lastSelectedListItem.reset();
                        lastSelectedListItem = listItemPtr;

                        lastRunningInterpreter = true;
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());

            } else if (commandMode == TOGGLE_STR) {
                auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(itemName, false, ON, OFF);

                bool toggleStateOn = std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end();
                toggleListItem->setState(toggleStateOn);

                toggleListItem->setStateChangedListener([&commandsOn = this->commandsOn, &commandsOff = this->commandsOff, &filePath = this->filePath,
                    &specificKey = this->specificKey, i, selectedItem, listItemRaw = toggleListItem.get()](bool state) {

                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                    interpretAndExecuteCommands(getSourceReplacement(!state ? commandsOn : commandsOff, selectedItem, i, filePath), filePath, specificKey);
                });
                list->addItem(toggleListItem.release());
            }
        }

        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(filePath),
            packageHeader.version != "" ? packageHeader.version + "   (Ultrahand Package)" : "Ultrahand Package", "", packageHeader.color);
        rootFrame->setContent(list.release());

        return rootFrame.release();
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (runningInterpreter.load(std::memory_order_acquire)) {
            return handleRunningInterpreter(keysHeld);
        }
        if (lastRunningInterpreter) {
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

                if (lastPackageMenu == "subPackageMenu") {
                    returningToSubPackage = true;
                } else {
                    returningToPackage = true;
                }

                if (commandMode == OPTION_STR && isFileOrDirectory(packageConfigIniPath)) {
                    auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    auto it = packageConfigData.find(specificKey);
                    if (it != packageConfigData.end()) {
                        auto& optionSection = it->second;
                        auto footerIt = optionSection.find(FOOTER_STR);
                        if (footerIt != optionSection.end() && footerIt->second != NULL_STR) {
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
    size_t nestedLayer = 0;
public:
    /**
     * @brief Constructs a `PackageMenu` instance for a specific sub-menu path.
     *
     * Initializes a new instance of the `PackageMenu` class for the given sub-menu path.
     *
     * @param path The path to the sub-menu.
     */
    PackageMenu(const std::string& path, const std::string sectionName = "", const std::string& page = LEFT_STR, const std::string& _packageName = PACKAGE_FILENAME, const size_t _nestedlayer = 0) :
        packagePath(path), dropdownSection(sectionName), currentPage(page), packageName(_packageName), nestedLayer(_nestedlayer) {}
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
        std::string packageConfigIniPath = packagePath + CONFIG_FILENAME;

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
        std::vector<std::vector<std::string>> tableData;
        
        //std::vector<std::string> listData, listDataOn, listDataOff;
        
        std::string footer;
        bool useSelection;
        size_t pos;
        
        bool inEristaSection;
        bool inMarikoSection;
        
        bool hideTableHeader, hideTableBackground;
        size_t tableStartGap, tableEndGap, tableColumnOffset, tableSpacing;
        std::string tableAlignment;

        
        for (size_t i = 0; i < options.size(); ++i) {
            auto& option = options[i];
            
            optionName = option.first;
            commands = std::move(option.second);
            
            footer = "";
            useSelection = false;
            hideTableHeader = false;
            hideTableBackground = false;
            tableStartGap = 20;
            tableEndGap = 3;
            tableColumnOffset = 160;
            tableSpacing = 0;
            tableAlignment = RIGHT_STR;
            
            commandFooter = NULL_STR;
            commandSystem = DEFAULT_STR;
            commandMode = DEFAULT_STR;
            commandGrouping = DEFAULT_STR;
            
            defaultToggleState = "";

            currentSection = GLOBAL_STR;
            sourceType = DEFAULT_STR;
            sourceTypeOn = DEFAULT_STR;
            sourceTypeOff = DEFAULT_STR;

            
            tableData.clear();
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
                            addPackageInfo(list, packageHeader);
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
                                drawLocation = LEFT_STR;
                            } else {
                                pageRightName = optionName.substr(1);
                                usingPages = true;
                                drawLocation = RIGHT_STR;
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
                                        addPackageInfo(list, packageHeader);
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
                    } else if (i == 0 && optionName.front() != '$') {
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

                        if (commandName.find(SYSTEM_PATTERN) == 0) {// Extract the command system
                            commandSystem = commandName.substr(SYSTEM_PATTERN.length());
                            if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                                commandSystem = commandSystems[0]; // reset to default if commandSystem is unknown
                            continue;
                        } else if (commandName.find(MODE_PATTERN) == 0) { // Extract the command mode
                            commandMode = commandName.substr(MODE_PATTERN.length());
                            if (commandMode.find(TOGGLE_STR) != std::string::npos) {
                                delimiterPos = commandMode.find('?');
                                if (delimiterPos != std::string::npos) {
                                    defaultToggleState = commandMode.substr(delimiterPos + 1);
                                }
                                commandMode = TOGGLE_STR;
                            }
                            else if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end()) {
                                commandMode = commandModes[0]; // reset to default if commandMode is unknown
                            }
                            continue;
                        } else if (commandName.find(GROUPING_PATTERN) == 0) {// Extract the command grouping
                            commandGrouping = commandName.substr(GROUPING_PATTERN.length());
                            if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                            continue;
                        } else if (commandName.find(BACKGROUND_PATTERN) == 0) {// Extract the command grouping
                            hideTableBackground = (commandName.substr(BACKGROUND_PATTERN.length()) == FALSE_STR);
                            continue;
                        } else if (commandName.find(HEADER_PATTERN) == 0) {// Extract the command grouping
                            hideTableHeader = (commandName.substr(HEADER_PATTERN.length()) == FALSE_STR);
                            continue;
                        } else if (commandName.find(GAP_PATTERN) == 0) {// Extract the command grouping
                            tableEndGap = std::stoi(commandName.substr(GAP_PATTERN.length()));
                            continue;
                        } else if (commandName.find(OFFSET_PATTERN) == 0) {// Extract the command grouping
                            tableColumnOffset = std::stoi(commandName.substr(OFFSET_PATTERN.length()));
                            continue;
                        } else if (commandName.find(SPACING_PATTERN) == 0) {// Extract the command grouping
                            tableSpacing = std::stoi(commandName.substr(SPACING_PATTERN.length()));
                            continue;
                        } else if (commandName.find(ALIGNMENT_PATTERN) == 0) {// Extract the command grouping
                            tableAlignment = commandName.substr(ALIGNMENT_PATTERN.length());
                            continue;
                        }
                        
                        // Extract the command grouping
                        if (commandMode == TOGGLE_STR) {
                            if (commandName.find("on:") == 0)
                                currentSection = ON_STR;
                            else if (commandName.find("off:") == 0)
                                currentSection = OFF_STR;
                            
                            // Seperation of command chuncks
                            if (currentSection == GLOBAL_STR) {
                                commandsOn.push_back(cmd);
                                commandsOff.push_back(cmd);
                            } else if (currentSection == ON_STR)
                                commandsOn.push_back(cmd);
                            else if (currentSection == OFF_STR)
                                commandsOff.push_back(cmd);
                        } else if (commandMode == TABLE_STR) {
                            tableData.push_back(cmd);
                            continue;
                        }



                        if (cmd.size() > 1) { // Pre-process advanced commands
                            if (commandName == "file_source") {
                                if (currentSection == GLOBAL_STR) {
                                    pathPattern = cmd[1];
                                    sourceType = FILE_STR;
                                } else if (currentSection == ON_STR) {
                                    pathPatternOn = cmd[1];
                                    sourceTypeOn = FILE_STR;
                                } else if (currentSection == OFF_STR) {
                                    pathPatternOff = cmd[1];
                                    sourceTypeOff = FILE_STR;
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
                
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, MODE_STR, commandMode);
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                    updateIniData(packageConfigData, packageConfigIniPath, optionName, FOOTER_STR, commandFooter);
                    
                    packageConfigData.clear();
                } else { // write default data if settings are not loaded
                    setIniFileValue(packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                    setIniFileValue(packageConfigIniPath, optionName, MODE_STR, commandMode);
                    setIniFileValue(packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                    setIniFileValue(packageConfigIniPath, optionName, FOOTER_STR, NULL_STR);
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
                
                if (commandMode == OPTION_STR || (commandMode == TOGGLE_STR && !useSelection)) {
                    // override loading of the command footer
                    if (commandFooter != NULL_STR)
                        footer = commandFooter;
                    else
                        footer = OPTION_SYMBOL;
                } else if (commandMode == SLOT_STR) {
                    if (commandFooter != NULL_STR)
                        footer = commandFooter;
                    else
                        footer = OPTION_SYMBOL;
                }

                skipSystem = false;
                if (commandSystem == ERISTA_STR && !usingErista) {
                    skipSystem = true;
                } else if (commandSystem == MARIKO_STR && !usingMariko) {
                    skipSystem = true;
                }
                
                if (!skipSection && !skipSystem) { // for skipping the drawing of sections
                    // For handling table mode
                    if (optionName.front() == '$') { 
                        if (!hideTableHeader) {
                            optionName = optionName.substr(1);
                            list->addItem(new tsl::elm::CategoryHeader(optionName));
                        } else
                            hideTableHeader = false;
                        addTable(list, tableData, this->packagePath, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing, tableAlignment, hideTableBackground);
                        continue;
                    }

                    if (useSelection) { // For wildcard commands (dropdown menus)
                        
                        if ((footer == DROPDOWN_SYMBOL) || (footer.empty()))
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName), footer);
                        else {
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                            if (commandMode == OPTION_STR)
                                listItem->setValue(footer);
                            else
                                listItem->setValue(footer, true);
                        }
                        
                        if (footer == UNAVAILABLE_SELECTION || footer == NOT_AVAILABLE_STR)
                            listItem->setValue(UNAVAILABLE_SELECTION, true);

                        if (commandMode == FORWARDER_STR) {
                            const std::string& forwarderPackagePath = getParentDirFromPath(packageSource);
                            const std::string& forwarderPackageIniName = getNameFromPath(packageSource);

                            listItem->setClickListener([commands, keyName = option.first, &dropdownSection = this->dropdownSection, &packagePath = this->packagePath, forwarderPackagePath, forwarderPackageIniName, &nestedLayer=this->nestedLayer](s64 keys) mutable {


                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }
                                
                                if (keys & KEY_A) {
                                    auto commandsCopy = commands;
                                    interpretAndExecuteCommands(std::move(commandsCopy), packagePath, keyName); // Now correctly moved
                                    nestedMenuCount++;
                                    lastPackagePath = forwarderPackagePath;
                                    lastPackageName = forwarderPackageIniName;
                                    if (dropdownSection.empty())
                                        lastPackageMenu = "packageMenu";
                                    else
                                        lastPackageMenu = "subPackageMenu";
                                    tsl::changeTo<PackageMenu>(forwarderPackagePath, "", LEFT_STR, forwarderPackageIniName, nestedMenuCount);
                                    simulatedSelectComplete = true;
                                    return true;
                                }
                                return false;
                            });
                        } else {
                            
                            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                            listItem->setClickListener([commands, keyName = option.first, &dropdownSection = this->dropdownSection, &packagePath = this->packagePath,  &packageName = this->packageName, footer, lastSection,
                                listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                                
                                if (runningInterpreter.load(std::memory_order_acquire))
                                    return false;

                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }

                                if ((keys & KEY_A)) {
                                    if (footer != UNAVAILABLE_SELECTION && footer != NOT_AVAILABLE_STR) {
                                        if (inPackageMenu)
                                            inPackageMenu = false;
                                        if (inSubPackageMenu)
                                            inSubPackageMenu = false;

                                        if (dropdownSection.empty())
                                            lastPackageMenu = "packageMenu";
                                        else
                                            lastPackageMenu = "subPackageMenu";
                                        
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
                        
                        if (commandMode == FORWARDER_STR) {
                            const std::string& forwarderPackagePath = getParentDirFromPath(packageSource);
                            const std::string& forwarderPackageIniName = getNameFromPath(packageSource);

                            listItem->setClickListener([commands, keyName = option.first, &dropdownSection = this->dropdownSection, &packagePath = this->packagePath, forwarderPackagePath, forwarderPackageIniName, &nestedLayer=this->nestedLayer](s64 keys) mutable {


                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }
                                
                                if (keys & KEY_A) {
                                    auto commandsCopy = commands;
                                    interpretAndExecuteCommands(std::move(commandsCopy), packagePath, keyName); // Now correctly moved
                                    nestedMenuCount++;
                                    lastPackagePath = forwarderPackagePath;
                                    lastPackageName = forwarderPackageIniName;
                                    if (dropdownSection.empty())
                                        lastPackageMenu = "packageMenu";
                                    else
                                        lastPackageMenu = "subPackageMenu";
                                    tsl::changeTo<PackageMenu>(forwarderPackagePath, "", LEFT_STR, forwarderPackageIniName, nestedMenuCount);
                                    simulatedSelectComplete = true;
                                    return true;
                                }
                                return false;
                            });
                        }  else if (commandMode == DEFAULT_STR  || commandMode == SLOT_STR|| commandMode == OPTION_STR) { // for handiling toggles
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                            if (commandMode == DEFAULT_STR)
                                listItem->setValue(footer, true);
                            else
                                listItem->setValue(footer);
                            
                            
                            listItem->setClickListener([i, commands, keyName = option.first, &packagePath = this->packagePath, &packageName = this->packageName, selectedItem,
                                listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
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
                                    lastSelectedListItem->triggerClickAnimation();
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
                        } else if (commandMode == TOGGLE_STR) {
                            
                            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(removeTag(optionName), false, ON, OFF);
                            
                            // Set the initial state of the toggle item
                            if (!pathPatternOn.empty())
                                toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn, packagePath));
                            else {
                                if ((footer != CAPITAL_ON_STR && footer != CAPITAL_OFF_STR) && !defaultToggleState.empty()) {
                                    if (defaultToggleState == ON_STR)
                                        footer = CAPITAL_ON_STR;
                                    else if (defaultToggleState == OFF_STR)
                                        footer = CAPITAL_OFF_STR;
                                }

                                toggleStateOn = (footer == CAPITAL_ON_STR);
                            }
                            
                            toggleListItem->setState(toggleStateOn);
                            
                            toggleListItem->setStateChangedListener([i, commandsOn, commandsOff, keyName = option.first, &packagePath = this->packagePath,
                                &pathPatternOn = this->pathPatternOn, &pathPatternOff = this->pathPatternOff, listItemRaw = toggleListItem.get()](bool state) {

                                tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                                interpretAndExecuteCommands(state ? getSourceReplacement(commandsOn, preprocessPath(pathPatternOn, packagePath), i, packagePath) :
                                    getSourceReplacement(commandsOff, preprocessPath(pathPatternOff, packagePath), i, packagePath), packagePath, keyName);
                                setIniFileValue((packagePath + CONFIG_FILENAME).c_str(), keyName.c_str(), FOOTER_STR, state ? CAPITAL_ON_STR : CAPITAL_OFF_STR);

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
            (usingPages && currentPage == RIGHT_STR) ? pageLeftName : "",
            (usingPages && currentPage == LEFT_STR) ? pageRightName : ""
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
            //while (!interpreterThreadExit.load(std::memory_order_acquire)) {svcSleepThread(50'000'000);}
            
            //resetPercentages();
            
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
                lastPackagePath = packagePath;
                inSubPackageMenu = false;
                inPackageMenu = false;
                if (lastPage == RIGHT_STR) {
                    tsl::goBack();
                } else {
                    tsl::goBack();
                }
                inPackageMenu = true;
                lastPage = LEFT_STR;
                selectedListItem.reset();
                lastSelectedListItem.reset();
                tsl::changeTo<PackageMenu>(lastPackagePath);
                //lastPage == LEFT_STR;
            }

            if (inSubPackageMenu) {
                lastPackagePath = packagePath;
                inSubPackageMenu = false;
                inPackageMenu = false;
                if (lastPage == RIGHT_STR) {
                    tsl::goBack();
                    tsl::goBack();
                } else {
                    tsl::goBack();
                    tsl::goBack();
                }
                inPackageMenu = true;
                lastPage = LEFT_STR;
                selectedListItem.reset();
                lastSelectedListItem.reset();
                tsl::changeTo<PackageMenu>(lastPackagePath);
            }
            
        }

        if (usingPages) {
            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedNextPage && !simulatedNextPageComplete) {
                if (currentPage == LEFT_STR) {
                    keysHeld |= KEY_DRIGHT;
                    simulatedNextPage = false;
                }
                else if (currentPage == RIGHT_STR) {
                    keysHeld |= KEY_DLEFT;
                    simulatedNextPage = false;
                }
                else {
                    simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }
            }
            if (currentPage == LEFT_STR) {
                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    lastPage = RIGHT_STR;
                    //lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackagePath, dropdownSection, RIGHT_STR, lastPackageName, nestedMenuCount);
                    simulatedNextPageComplete = true;
                    return true;
                }
            } else if (currentPage == RIGHT_STR) {
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    lastPage = LEFT_STR;
                    //lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackagePath, dropdownSection, LEFT_STR, lastPackageName, nestedMenuCount);
                    simulatedNextPageComplete = true;
                    return true;
                }
            } 

        }
        
        if (!returningToPackage && inPackageMenu && nestedMenuCount == nestedLayer) {
            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedNextPage && !simulatedNextPageComplete) {
                simulatedNextPage = false;
                simulatedNextPageComplete = true;
            }

            if (!usingPages || (usingPages && lastPage == LEFT_STR)) {
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
                        //returningToPackage = true;
                        if (lastPackageMenu == "subPackageMenu") {
                            returningToSubPackage = true;
                            //lastPackageMenu = "";
                        } else {
                            returningToPackage = true;
                        }
                    }
                    
                    // Free-up memory
                    clearMemory();

                    tsl::goBack();
                    simulatedBackComplete = true;
                    return true;
                }
            } else if (usingPages && lastPage == RIGHT_STR) {
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
                        
                        if (lastPackageMenu == "subPackageMenu") {
                            returningToSubPackage = true;
                            //lastPackageMenu = "";
                        } else {
                            returningToPackage = true;
                        }
                    }
                    
                    // Free-up memory
                    clearMemory();
                    
                    lastPage = LEFT_STR;
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

            if (!usingPages || (usingPages && lastPage == LEFT_STR)) {
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
            } else if (usingPages && lastPage == RIGHT_STR) {
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
        if (returningToPackage && !returningToSubPackage && !(keysHeld & KEY_B)){
            //lastPackageMenu = "packageMenu";
            lastPackageMenu = "";
            returningToPackage = false;
            inPackageMenu = true;
            if (nestedMenuCount == 0 && nestedLayer == 0) {
                lastPackagePath = packagePath;
                lastPackageName = PACKAGE_FILENAME;
            }
        }
        
        if (returningToSubPackage && !(keysHeld & KEY_B)){
            //lastPackageMenu = "subPackageMenu";
            lastPackageMenu = "";
            returningToPackage = false;
            returningToSubPackage = false;
            inSubPackageMenu = true;
            simulatedBackComplete = true;
            if (nestedMenuCount == 0 && nestedLayer == 0) {
                lastPackagePath = packagePath;
                lastPackageName = PACKAGE_FILENAME;
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
 * @brief The `MainMenu` class handles the main menu overlay functionality.
 *
 * This class manages the main menu overlay, allowing users to navigate and access various submenus.
 * It provides functions for creating, updating, and navigating the main menu, as well as handling user interactions related to menu navigation.
 */
class MainMenu : public tsl::Gui {
private:
    std::string packageIniPath = PACKAGE_PATH + PACKAGE_FILENAME;
    std::string packageConfigIniPath = PACKAGE_PATH + CONFIG_FILENAME;
    std::string menuMode, fullPath, optionName, priority, starred, hide;
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
        if (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR) == TRUE_STR) {
            inMainMenu = false;
            inHiddenMode = true;
            hiddenMenuMode = OVERLAYS_STR;
            setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, FALSE_STR);
        }

        if (!inHiddenMode)
            inMainMenu = true;
        
        tsl::hlp::ini::IniData settingsData, packageConfigData;
        std::string packagePath, pathReplace, pathReplaceOn, pathReplaceOff;
        std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
        std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
        
        bool skipSystem = false;
        lastMenuMode = hiddenMenuMode;
        
        menuMode = OVERLAYS_STR;
        
        createDirectory(PACKAGE_PATH);
        createDirectory(SETTINGS_PATH);
        
        bool settingsLoaded = false;
        
        auto setDefaultValue = [](const auto& ultrahandSection, const std::string& section, const std::string& defaultValue, bool& settingFlag) {
            if (ultrahandSection.count(section) > 0) {
                settingFlag = (ultrahandSection.at(section) == TRUE_STR);
            } else {
                setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, section, defaultValue);
                settingFlag = (defaultValue == TRUE_STR);
            }
        };
        
        auto setDefaultStrValue = [](const auto& ultrahandSection, const std::string& section, const std::string& defaultValue, std::string& settingValue) {
            if (ultrahandSection.count(section) > 0) {
                settingValue = ultrahandSection.at(section);
            } else {
                setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, section, defaultValue);
            }
        };
        
        if (isFileOrDirectory(SETTINGS_CONFIG_INI_PATH)) {
            auto settingsData = getParsedDataFromIniFile(SETTINGS_CONFIG_INI_PATH);
            if (settingsData.count(ULTRAHAND_PROJECT_NAME) > 0) {
                auto& ultrahandSection = settingsData[ULTRAHAND_PROJECT_NAME];
                
                setDefaultValue(ultrahandSection, "hide_user_guide", FALSE_STR, hideUserGuide);
                setDefaultValue(ultrahandSection, "clean_version_labels", FALSE_STR, cleanVersionLabels);
                setDefaultValue(ultrahandSection, "hide_overlay_versions", FALSE_STR, hideOverlayVersions);
                setDefaultValue(ultrahandSection, "hide_package_versions", FALSE_STR, hidePackageVersions);
                setDefaultValue(ultrahandSection, "opaque_screenshots", TRUE_STR, useOpaqueScreenshots);
                setDefaultValue(ultrahandSection, "progress_animation", FALSE_STR, progressAnimation);
                
                setDefaultStrValue(ultrahandSection, DEFAULT_LANG_STR, defaultLang, defaultLang);
                
                if (ultrahandSection.count("datetime_format") == 0) {
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "datetime_format", DEFAULT_DT_FORMAT);
                }
                
                if (ultrahandSection.count("hide_clock") == 0) {
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_clock", FALSE_STR);
                }
                
                if (ultrahandSection.count("hide_battery") == 0) {
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_battery", TRUE_STR);
                }
                
                if (ultrahandSection.count("hide_pcb_temp") == 0) {
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_pcb_temp", TRUE_STR);
                }
                
                if (ultrahandSection.count("hide_soc_temp") == 0) {
                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_soc_temp", TRUE_STR);
                }
                
                settingsLoaded = ultrahandSection.count(IN_OVERLAY_STR) > 0;
            }
            settingsData.clear();
        } else {
            updateMenuCombos = true;
        }

        
        if (!settingsLoaded) { // Write data if settings are not loaded
            setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR, defaultLang);
            setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, FALSE_STR);
            initializingSpawn = true;
        }
        
        
        std::string langFile = LANG_PATH+defaultLang+".json";
        if (isFileOrDirectory(langFile))
            parseLanguage(langFile);
        else
            reinitializeLangVars();
        
        // write default theme
        initializeTheme();
        copyTeslaKeyComboToUltrahand();
        
        menuMode = currentMenu.c_str();
        
        versionLabel = std::string(APP_VERSION) + "   (" + extractTitle(loaderInfo) + " " + (cleanVersionLabels ? "" : "v") + cleanVersionLabel(loaderInfo) + ")";
        //versionLabel = (cleanVersionLabels) ? std::string(APP_VERSION) : (std::string(APP_VERSION) + "   (" + extractTitle(loaderInfo) + " v" + cleanVersionLabel(loaderInfo) + ")");
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();

        std::unique_ptr<tsl::elm::ListItem> listItem;
        
        if (!hiddenMenuMode.empty())
            menuMode = hiddenMenuMode;
        
        
        // Overlays menu
        if (menuMode == OVERLAYS_STR) {
            //closeInterpreterThread();

            list->addItem(new tsl::elm::CategoryHeader(!inHiddenMode ? OVERLAYS : HIDDEN_OVERLAYS));
            
            
            // Load overlay files
            std::vector<std::string> overlayFiles = getFilesListByWildcards(OVERLAY_PATH+"*.ovl");
            
            
            // Check if the overlays INI file exists
            std::ifstream overlaysIniFile(OVERLAYS_INI_FILEPATH);
            if (!overlaysIniFile.is_open()) {
                // The INI file doesn't exist, so create an empty one.
                std::ofstream createFile(OVERLAYS_INI_FILEPATH);
                if (createFile.is_open())
                    initializingSpawn = true;
            }

            overlaysIniFile.close(); // Close the file


            // load overlayList from OVERLAYS_INI_FILEPATH.  this will be the overlayFilenames
            std::set<std::string> overlayList;
            std::set<std::string> hiddenOverlayList;
            
            std::string overlayFileName;
            
            // Load subdirectories
            if (!overlayFiles.empty()) {
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> overlaysIniData = getParsedDataFromIniFile(OVERLAYS_INI_FILEPATH);
                
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
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, PRIORITY_STR, "20");
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, STAR_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, HIDE_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, USE_LAUNCH_ARGS_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, LAUNCH_ARGS_STR, "''");
                        const auto& [result, overlayName, overlayVersion] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                        if (result != ResultSuccess) continue;
                        overlayList.insert("0020"+(overlayName)+":" + overlayFileName);
                    } else {
                        const std::string& priority = getValueOrDefault(it->second, PRIORITY_STR, "20", formatPriorityString, 1);
                        const std::string& starred = getValueOrDefault(it->second, STAR_STR, FALSE_STR);
                        const std::string& hide = getValueOrDefault(it->second, HIDE_STR, FALSE_STR);
                        const std::string& useLaunchArgs = getValueOrDefault(it->second, USE_LAUNCH_ARGS_STR, FALSE_STR);
                        const std::string& launchArgs = getValueOrDefault(it->second, LAUNCH_ARGS_STR, "''");
                        
                        const auto& [result, overlayName, overlayVersion] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                        if (result != ResultSuccess) continue;
                        
                        const std::string& baseOverlayInfo = priority + (overlayName) + ":" + overlayName + ":" + overlayVersion + ":" + overlayFileName;
                        const std::string& fullOverlayInfo = (starred == TRUE_STR) ? "-1:" + baseOverlayInfo : baseOverlayInfo;
                
                        if (hide == FALSE_STR) {
                            overlayList.insert(fullOverlayInfo);
                        } else {
                            hiddenOverlayList.insert(fullOverlayInfo);
                        }
                    }
                }


                
                overlaysIniData.clear();
                
                //std::sort(overlayList.begin(), overlayList.end());
                //std::sort(hiddenOverlayList.begin(), hiddenOverlayList.end());
                
                
                if (inHiddenMode) {
                    overlayList = hiddenOverlayList;
                    hiddenOverlayList.clear();
                }
                
                
                bool overlayStarred;
                
                std::string overlayFile, newOverlayName;
                size_t lastUnderscorePos, secondLastUnderscorePos, thirdLastUnderscorePos;
                
                bool newStarred;
                
                for (const auto& taintedOverlayFileName : overlayList) {
                    overlayFileName = "";
                    overlayStarred = false;
                    overlayVersion = "";
                    overlayName = "";
                    
                    // Detect if starred
                    if ((taintedOverlayFileName.substr(0, 3) == "-1:"))
                        overlayStarred = true;
                    
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
                    
                    
                    overlayFile = OVERLAY_PATH+overlayFileName;
                    
                    newOverlayName = overlayName.c_str();
                    if (overlayStarred)
                        newOverlayName = STAR_SYMBOL+" "+newOverlayName;
                    
                    
                    // Toggle the starred status
                    newStarred = !overlayStarred;
                    
                    
                    //logMessage(overlayFile);
                    if (isFileOrDirectory(overlayFile)) {
                        listItem = std::make_unique<tsl::elm::ListItem>(newOverlayName);
                        if (cleanVersionLabels)
                            overlayVersion = cleanVersionLabel(overlayVersion);
                        if (!hideOverlayVersions)
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
                                
                                
                                std::string useOverlayLaunchArgs = parseValueFromIniSection(OVERLAYS_INI_FILEPATH, overlayFileName, USE_LAUNCH_ARGS_STR);
                                std::string overlayLaunchArgs = removeQuotes(parseValueFromIniSection(OVERLAYS_INI_FILEPATH, overlayFileName, LAUNCH_ARGS_STR));
                                
                                if (inHiddenMode) {
                                    setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, TRUE_STR);
                                }
                                
                                setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
                                if (useOverlayLaunchArgs == TRUE_STR)
                                    tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                                else
                                    tsl::setNextOverlay(overlayFile);
                                
                                tsl::Overlay::get()->close();
                                simulatedSelectComplete = true;

                                return true;
                            } else if (keys & STAR_KEY) {
                                
                                if (!overlayFile.empty()) {
                                    // Update the INI file with the new value
                                    setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, STAR_STR, newStarred ? TRUE_STR : FALSE_STR);
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
                                
                                tsl::changeTo<SettingsMenu>(overlayFileName, OVERLAY_STR, overlayName);
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
                            tsl::changeTo<MainMenu>(OVERLAYS_STR);
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
        if (menuMode == PACKAGES_STR ) {
            

            if (dropdownSection.empty()) {
                // Create the directory if it doesn't exist
                createDirectory(PACKAGE_PATH);
                
                
                std::fstream packagesIniFile(PACKAGES_INI_FILEPATH, std::ios::in);
                if (!packagesIniFile.is_open()) {
                    std::ofstream createFile(PACKAGES_INI_FILEPATH); // Create an empty INI file if it doesn't exist
                    createFile.close();
                    initializingSpawn = true;
                } else {
                    packagesIniFile.close();
                }
                
                std::set<std::string> packageList;
                std::set<std::string> hiddenPackageList;
                
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                // Load subdirectories
                std::vector<std::string> subdirectories = getSubdirectories(PACKAGE_PATH);
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
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, PRIORITY_STR, "20");
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, STAR_STR, FALSE_STR);
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, HIDE_STR, FALSE_STR);
                        packageList.insert("0020" + (packageName) +":" + packageName);
                    } else {
                        // Process existing package data
                        priority = (packageIt->second.find(PRIORITY_STR) != packageIt->second.end()) ? 
                                    formatPriorityString(packageIt->second[PRIORITY_STR]) : "0020";
                        starred = (packageIt->second.find(STAR_STR) != packageIt->second.end()) ? 
                                  packageIt->second[STAR_STR] : FALSE_STR;
                        hide = (packageIt->second.find(HIDE_STR) != packageIt->second.end()) ? 
                               packageIt->second[HIDE_STR] : FALSE_STR;
                        
                        const std::string& basePackageInfo = priority + (packageName) +":" + packageName;
                        const std::string& fullPackageInfo = (starred == TRUE_STR) ? "-1:" + basePackageInfo : basePackageInfo;
                
                        if (hide == FALSE_STR) {
                            packageList.insert(fullPackageInfo);
                        } else {
                            hiddenPackageList.insert(fullPackageInfo);
                        }
                    }
                }

                packagesIniData.clear();
                subdirectories.clear();
                
                //std::sort(packageList.begin(), packageList.end());
                //std::sort(hiddenPackageList.begin(), hiddenPackageList.end());
                
                if (inHiddenMode) {
                    packageList = hiddenPackageList;
                    hiddenPackageList.clear();
                }
                
                std::string taintedPackageName;
                std::string packageName;
                bool packageStarred;
                std::string newPackageName;
                std::string packageFilePath;
                bool newStarred;
                PackageHeader packageHeader;
                
                size_t lastUnderscorePos;

                bool firstItem = true;
                for (const auto& taintedPackageName : packageList) {
                //for (size_t i = 0; i < packageList.size(); ++i) {
                    //taintedPackageName = packageList[i];
                    if (firstItem) {
                        list->addItem(new tsl::elm::CategoryHeader(!inHiddenMode ? PACKAGES : HIDDEN_PACKAGES));
                        firstItem = false;
                    }

                    
                   // packageName = taintedPackageName.c_str();
                    packageStarred = false;
                    
                    if ((taintedPackageName.length() >= 2) && (taintedPackageName.substr(0, 3) == "-1:")) {
                        // strip first two characters
                        //packageName = packageName.substr(3);
                        packageStarred = true;
                    }

                    lastUnderscorePos = taintedPackageName.rfind(':');
                    if (lastUnderscorePos != std::string::npos) {
                        // Extract overlayFileName starting from the character after the last underscore
                        packageName = taintedPackageName.substr(lastUnderscorePos + 1);
                    }
                    
                    //packageName = packageName.substr(5);
                    
                    newPackageName = (packageStarred) ? (STAR_SYMBOL + " " + packageName) : packageName;
                    
                    packageFilePath = PACKAGE_PATH + packageName+ "/";
                    
                    // Toggle the starred status
                    newStarred = !packageStarred;
                    
                    
                    if (isFileOrDirectory(packageFilePath)) {
                        packageHeader = getPackageHeaderFromIni(packageFilePath+PACKAGE_FILENAME);
                        
                        listItem = std::make_unique<tsl::elm::ListItem>(newPackageName);
                        if (cleanVersionLabels)
                            packageHeader.version = removeQuotes(cleanVersionLabel(packageHeader.version));
                        if (!hidePackageVersions)
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
                                if (isFileOrDirectory(packageFilePath+BOOT_PACKAGE_FILENAME)) {
                                    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> bootOptions = loadOptionsFromIni(packageFilePath+BOOT_PACKAGE_FILENAME, true);
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
                                lastPackagePath = packageFilePath;
                                lastPackageName = PACKAGE_FILENAME;
                                tsl::changeTo<PackageMenu>(packageFilePath, "");
                                simulatedSelectComplete = true;
                                return true;
                            } else if (keys & STAR_KEY) {
                                if (!packageName.empty())
                                    setIniFileValue(PACKAGES_INI_FILEPATH, packageName, STAR_STR, newStarred ? TRUE_STR : FALSE_STR); // Update the INI file with the new value
                                
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
                                
                                tsl::changeTo<SettingsMenu>(packageName, PACKAGE_STR);
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
                            tsl::changeTo<MainMenu>(PACKAGES_STR);
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
                
                std::string commandFooter = NULL_STR;
                std::string commandSystem = DEFAULT_STR;
                std::string commandMode = DEFAULT_STR;
                std::string commandGrouping = DEFAULT_STR;
                
                std::string currentSection = GLOBAL_STR;

                std::string defaultToggleState;

                std::string sourceType = DEFAULT_STR, sourceTypeOn = DEFAULT_STR, sourceTypeOff = DEFAULT_STR; 
                
                std::string jsonPath, jsonPathOn, jsonPathOff;
                std::string jsonKey, jsonKeyOn, jsonKeyOff;
                
                std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
                std::vector<std::vector<std::string>> tableData;
                
                
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
                    
                    commandFooter = NULL_STR;
                    commandSystem = DEFAULT_STR;
                    commandMode = DEFAULT_STR;
                    commandGrouping = DEFAULT_STR;
                    
                    currentSection = GLOBAL_STR;

                    defaultToggleState = "";
                    sourceType = DEFAULT_STR;
                    sourceTypeOn = DEFAULT_STR;
                    sourceTypeOff = DEFAULT_STR; 
                    
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
                        } else if (i == 0 && optionName.front() != '$') { // Add a section break with small text to indicate the "Commands" section
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
                            if (commandName.find(SYSTEM_PATTERN) == 0) {// Extract the command system
                                commandSystem = commandName.substr(SYSTEM_PATTERN.length());
                                if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                                    commandSystem = commandSystems[0]; // reset to default if commandSystem is unknown
                            } else if (commandName.find(MODE_PATTERN) == 0) {
                                commandMode = commandName.substr(MODE_PATTERN.length());
                                if (commandMode.find(TOGGLE_STR) != std::string::npos) {
                                    delimiterPos = commandMode.find('?');
                                    if (delimiterPos != std::string::npos) {
                                        defaultToggleState = commandMode.substr(delimiterPos + 1);
                                    }
                                    commandMode = TOGGLE_STR;
                                }
                                else if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                    commandMode = commandModes[0]; // reset to default if commandMode is unknown

                            } else if (commandName.find(GROUPING_PATTERN) == 0) {// Extract the command grouping
                                commandGrouping = commandName.substr(GROUPING_PATTERN.length());
                                if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                    commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                            }
                            
                            // Extract the command grouping
                            if (commandMode == TOGGLE_STR) {
                                if (commandName.find("on:") == 0)
                                    currentSection = ON_STR;
                                else if (commandName.find("off:") == 0)
                                    currentSection = OFF_STR;
                                
                                // Seperation of command chuncks
                                if (currentSection == GLOBAL_STR) {
                                    commandsOn.push_back(cmd);
                                    commandsOff.push_back(cmd);
                                } else if (currentSection == ON_STR)
                                    commandsOn.push_back(cmd);
                                else if (currentSection == OFF_STR)
                                    commandsOff.push_back(cmd);
                            } else if (commandMode == TABLE_STR) {
                                tableData.push_back(cmd);
                            }

                            if (cmd.size() > 1) { // Pre-process advanced commands
                                if (commandName == "file_source") {
                                    if (currentSection == GLOBAL_STR) {
                                        pathPattern = cmd[1];
                                        sourceType = FILE_STR;
                                    } else if (currentSection == ON_STR) {
                                        pathPatternOn = cmd[1];
                                        sourceTypeOn = FILE_STR;
                                    } else if (currentSection == OFF_STR) {
                                        pathPatternOff = cmd[1];
                                        sourceTypeOff = FILE_STR;
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
                    
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, MODE_STR, commandMode);
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                        updateIniData(packageConfigData, packageConfigIniPath, optionName, FOOTER_STR, commandFooter);
                    
                        packageConfigData.clear();
                    } else { // write default data if settings are not loaded
                        setIniFileValue(packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                        setIniFileValue(packageConfigIniPath, optionName, MODE_STR, commandMode);
                        setIniFileValue(packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                        setIniFileValue(packageConfigIniPath, optionName, FOOTER_STR, NULL_STR);
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
                    
                    if (commandMode == OPTION_STR || (commandMode == TOGGLE_STR && !useSelection)) {
                        // override loading of the command footer
                        if (commandFooter != NULL_STR)
                            footer = commandFooter;
                        else
                            footer = OPTION_SYMBOL;
                    } else if (commandMode == SLOT_STR) {
                        if (commandFooter != NULL_STR)
                            footer = commandFooter;
                        else
                            footer = OPTION_SYMBOL;
                    }

                    skipSystem = false;
                    if (commandSystem == ERISTA_STR && !usingErista) {
                        skipSystem = true;
                    } else if (commandSystem == MARIKO_STR && !usingMariko) {
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
                            listItem->setClickListener([commands, keyName = option.first, packagePath = PACKAGE_PATH](uint64_t keys) {
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
                            if (!isDirectory(preprocessPath(selectedItem, PACKAGE_PATH)))
                                itemName = dropExtension(itemName);
                            parentDirName = getParentDirNameFromPath(selectedItem);
                            
                            
                            if (commandMode == DEFAULT_STR || commandMode == SLOT_STR || commandMode == OPTION_STR) { // for handiling toggles
                                listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                                listItem->setValue(footer, true);
                                
                                if (sourceType == JSON_STR) { // For JSON wildcards
                                    
                                    listItem->setClickListener([i, commands, keyName = option.first, selectedItem,
                                        listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
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
                                            enqueueInterpreterCommands(getSourceReplacement(commands, selectedItem, i, PACKAGE_PATH), PACKAGE_PATH, keyName);
                                            startInterpreterThread();

                                            listItemPtr->setValue(INPROGRESS_SYMBOL);

                                            shiftItemFocus(listItemPtr);

                                            lastSelectedListItem.reset();
                                            lastSelectedListItem = listItemPtr;
                                            

                                            lastRunningInterpreter = true;
                                            
                                            
                                            simulatedSelectComplete = true;
                                            lastSelectedListItem->triggerClickAnimation();
                                            return true;
                                        } else if (keys & SCRIPT_KEY) {
                                            inMainMenu = false; // Set boolean to true when entering a submenu
                                            tsl::changeTo<ScriptOverlay>(PACKAGE_PATH, keyName, true);
                                            return true;
                                        }
                                        
                                        return false;
                                    });
                                    list->addItem(listItem.release());
                                } else {
                                    
                                    listItem->setClickListener([i, commands, keyName = option.first, selectedItem,
                                        listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                                        
                                        if (runningInterpreter.load(std::memory_order_acquire))
                                            return false;

                                        if (simulatedSelect && !simulatedSelectComplete) {
                                            keys |= KEY_A;
                                            simulatedSelect = false;
                                        }
                                        

                                        if ((keys & KEY_A)) {
                                            isDownloadCommand = false;
                                            runningInterpreter.store(true, std::memory_order_release);
                                            enqueueInterpreterCommands(getSourceReplacement(commands, selectedItem, i, PACKAGE_PATH), PACKAGE_PATH, keyName);
                                            startInterpreterThread();
                                            
                                            listItemPtr->setValue(INPROGRESS_SYMBOL);

                                            shiftItemFocus(listItemPtr);

                                            lastSelectedListItem.reset();
                                            lastSelectedListItem = listItemPtr;
                                            
                                            
                                            lastRunningInterpreter = true;
                                            simulatedSelectComplete = true;
                                            lastSelectedListItem->triggerClickAnimation();
                                            return true;
                                        } else if (keys & SCRIPT_KEY) {
                                            inMainMenu = false; // Set boolean to true when entering a submenu
                                            tsl::changeTo<ScriptOverlay>(PACKAGE_PATH, keyName, true);
                                            return true;
                                        }
                                        return false;
                                    });
                                    list->addItem(listItem.release());
                                }
                            } else if (commandMode == TOGGLE_STR) {
                                
                                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(removeTag(optionName), false, ON, OFF);
                                
                                // Set the initial state of the toggle item
                                if (!pathPatternOn.empty())
                                    toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn, PACKAGE_PATH));
                                else {
                                    if ((footer != CAPITAL_ON_STR && footer != CAPITAL_OFF_STR) && !defaultToggleState.empty()) {
                                        if (defaultToggleState == ON_STR)
                                            footer = CAPITAL_ON_STR;
                                        else if (defaultToggleState == OFF_STR)
                                            footer = CAPITAL_OFF_STR;
                                    }

                                    toggleStateOn = (footer == CAPITAL_ON_STR);
                                }
                                
                                toggleListItem->setState(toggleStateOn);
                                
                                toggleListItem->setStateChangedListener([i, pathPatternOn, pathPatternOff, commandsOn, commandsOff,keyName = option.first, listItemRaw = toggleListItem.get()](bool state) {
                                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                                    interpretAndExecuteCommands(getSourceReplacement(state ? commandsOn : commandsOff,
                                        preprocessPath(state ? pathPatternOn : pathPatternOff), i, PACKAGE_PATH), PACKAGE_PATH, keyName);
                                    setIniFileValue((PACKAGE_PATH + CONFIG_FILENAME).c_str(), keyName.c_str(), FOOTER_STR, state ? CAPITAL_ON_STR : CAPITAL_OFF_STR);
                                });
                                list->addItem(toggleListItem.release());
                            }
                        }
                    }
                }
                
                if (!hideUserGuide && dropdownSection.empty())
                    addHelpInfo(list);
            }
        }
        if (initializingSpawn) {
            
            initializingSpawn = false;
            list.reset();
            return createUI(); 
        }
        
        filesList.clear();

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel, menuMode+hiddenMenuMode+dropdownSection);
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel, menuMode+hiddenMenuMode+dropdownSection);

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
            ////while (!interpreterThreadExit.load(std::memory_order_acquire)) {svcSleepThread(50'000'000);}
            
            //resetPercentages();
            
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
                    if (menuMode != PACKAGES_STR) {
                        keysHeld |= KEY_DRIGHT;
                        simulatedNextPage = false;
                    }
                    else if (menuMode != OVERLAYS_STR) {
                        keysHeld |= KEY_DLEFT;
                        simulatedNextPage = false;
                    } else {
                        simulatedNextPage = false;
                        simulatedNextPageComplete = true;
                    }
                }

                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    if (menuMode != PACKAGES_STR) {
                        currentMenu = PACKAGES_STR;
                        selectedListItem.reset();
                        lastSelectedListItem.reset();
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        //startInterpreterThread();
                        simulatedNextPageComplete = true;
                        return true;
                    }
                }
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    if (menuMode != OVERLAYS_STR) {
                        currentMenu = OVERLAYS_STR;
                        selectedListItem.reset();
                        lastSelectedListItem.reset();
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        //closeInterpreterThread();
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
                    //if (menuMode != PACKAGES_STR) startInterpreterThread();
                    
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
                    if (parseValueFromIniSection(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR) == FALSE_STR) {
                        inMainMenu = true;
                        inHiddenMode = false;
                        hiddenMenuMode = "";
                        setIniFileValue(SETTINGS_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, "");
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
        tsl::initializeThemeVars();
        initializeCurl();
    }
    
    /**
     * @brief Exits and cleans up services and resources.
     *
     * This function is responsible for exiting and cleaning up services and resources
     * when the overlay is no longer in use. It should release any allocated resources and
     * properly shut down services to avoid memory leaks.
     */
    virtual void exitServices() override {
        cleanupCurl();
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
    virtual void onShow() override {
        //playClickVibration();
        //logMessage("onShow isHidden: "+std::to_string(isHidden.load()));
        //std::string file_path = "sdmc:/config/ultrahand/open.wav";
        //
        //if (play_audio(file_path) != 0) {
        //    logMessage("Failed to play audio.");
        //}
    } 
    
    /**
     * @brief Performs actions when the overlay becomes visible.
     *
     * This function is called when the overlay transitions from an invisible state to a visible state.
     * It can be used to perform actions or updates specific to the overlay's visibility.
     */
    virtual void onHide() override {
        //logMessage("onHide isHidden: "+std::to_string(isHidden.load()));
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
