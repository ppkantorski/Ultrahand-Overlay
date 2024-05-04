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

#include <tesla.hpp>
#include <utils.hpp>
#include <fstream>

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
//static bool refreshGui = false;
static bool reloadMenu = false;
static bool reloadMenu2 = false;
static bool reloadMenu3 = false;
static bool isDownloaded = false;

static bool redrawWidget = false;


// Command mode globals
static std::vector<std::string> commandSystems = {"default", "erista", "mariko"};
static std::vector<std::string> commandModes = {"default", "toggle", "option"};
static std::vector<std::string> commandGroupings = {"default", "split", "split2", "split3", "split4"};
static std::string modePattern = ";mode=";
static std::string groupingPattern = ";grouping=";
static std::string systemPattern = ";system=";

static std::string defaultMenu = "overlays";

static std::string lastPage = "left";
static std::string lastPackage = "";
static std::string lastMenu = "";
static std::string lastMenuMode = "";
static std::string lastKeyName = "";

static std::unordered_map<std::string, std::string> selectedFooterDict;
//static auto selectedListItem = static_cast<tsl::elm::ListItem*>(nullptr);
//static auto lastSelectedListItem = static_cast<tsl::elm::ListItem*>(nullptr);
static std::shared_ptr<tsl::elm::ListItem> selectedListItem;
static std::shared_ptr<tsl::elm::ListItem> lastSelectedListItem;


static bool lastRunningInterpreter = false;

//static tsl::elm::OverlayFrame* rootFrame = nullptr;
//std::unique_ptr<tsl::elm::OverlayFrame> rootFrame = std::make_unique<tsl::elm::OverlayFrame>("", "");
//std::unique_ptr<tsl::elm::List> list = std::make_unique<tsl::elm::List>();



static std::string hideUserGuide = "false";


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
            
            std::string defaultLang = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_lang");
            //std::string defaultMenu = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_menu");
            std::string keyCombo = trim(parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "key_combo"));
            
            
            if (defaultLang.empty())
                defaultLang = "en";
            //if (defaultMenu.empty())
            //    defaultMenu = "packages";
            if (keyCombo.empty())
                keyCombo = "ZL+ZR+DDOWN";
            
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(KEY_COMBO);
            listItem->setValue(comboMap[keyCombo]);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }

                if (keys & KEY_A) {
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
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
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
            
            listItem->setClickListener([](uint64_t keys) { // Add 'command' to the capture list
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("softwareUpdateMenu");
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            list->addItem(new tsl::elm::CategoryHeader(UI_SETTINGS));
            
            std::string currentTheme = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "current_theme");
            if (currentTheme.empty() || currentTheme == "default")
                currentTheme = DEFAULT;
            listItem = std::make_unique<tsl::elm::ListItem>(THEME);
            listItem->setValue(currentTheme);
            listItem->setClickListener([listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
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
            
            listItem->setClickListener([](uint64_t keys) { // Add 'command' to the capture list
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("widgetMenu");
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            
            listItem = std::make_unique<tsl::elm::ListItem>(MISCELLANEOUS);
            listItem->setValue(DROPDOWN_SYMBOL);
            
            listItem->setClickListener([](uint64_t keys) { // Add 'command' to the capture list
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    tsl::changeTo<UltrahandSettingsMenu>("miscMenu");
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
        //} else if (dropdownSelection == "defaultMenu") {
        //    
        //    list->addItem(new tsl::elm::CategoryHeader("Default Menu"));
        //    
        //    std::string defaultMenu = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_menu");
        //    
        //    std::vector<std::string> defaultMenuModes = {"overlays", "packages"};
        //    
        //    std::unique_ptr<tsl::elm::ListItem> listItem;
        //
        //    for (const auto& defaultMenuMode : defaultMenuModes) {
        //        
        //        listItem = std::make_unique<tsl::elm::ListItem>(defaultMenuMode);
        //        
        //        if (defaultMenuMode == defaultMenu) {
        //            listItem->setValue(CHECKMARK_SYMBOL);
        //            lastSelectedListItem.reset();
        //            lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
        //        }
        //        
        //        listItem->setClickListener([defaultMenuMode, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
        //            bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        //            if (_runningInterpreter)
        //                return false;
        //
        //            if (simulatedSelect && !simulatedSelectComplete) {
        //                keys |= KEY_A;
        //                simulatedSelect = false;
        //            }
        //            if (keys & KEY_A) {
        //                setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
        //                lastSelectedListItem->setValue("");
        //                selectedListItem->setValue(defaultMenuMode);
        //                listItemPtr->setValue(CHECKMARK_SYMBOL);
        //                lastSelectedListItem.reset();
        //                lastSelectedListItem = listItemPtr;
        //                simulatedSelectComplete = true;
        //                return true;
        //            }
        //            return false;
        //        });
        //        
        //        list->addItem(listItem.release());
        //    }
        //    
        } else if (dropdownSelection == "keyComboMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(KEY_COMBO));
            
            std::string defaultCombo = trim(parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "key_combo"));
            
            std::unique_ptr<tsl::elm::ListItem> listItem;
            for (const auto& combo : defaultCombos) {
                
                listItem = std::make_unique<tsl::elm::ListItem>(comboMap[combo]);
                
                if (combo == defaultCombo) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                
                listItem->setClickListener([combo, mappedCombo=comboMap[combo], defaultCombo, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                    if (_runningInterpreter)
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        if (combo != defaultCombo) {
                            setIniFileValue(settingsConfigIniPath, "ultrahand", "key_combo", combo);
                            setIniFileValue(teslaSettingsConfigIniPath, "tesla", "key_combo", combo);
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
            
            std::string defaulLang = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "default_lang");
            
            std::string langFile;
            bool skipLang;
            
            std::unique_ptr<tsl::elm::ListItem> listItem;
            for (const auto& defaultLangMode : defaultLanguages) {
                langFile = "/config/ultrahand/lang/"+defaultLangMode+".json";
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
                    bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                    if (_runningInterpreter)
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
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
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter) {

                    //int currentPercentage = downloadPercentage.load(std::memory_order_acquire);
                    //logMessage("currentPercentage: "+std::to_string(currentPercentage));
                    if (downloadPercentage.load(std::memory_order_acquire) != -1) {
                        lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(downloadPercentage.load(std::memory_order_acquire))+"%");
                        if (downloadPercentage.load(std::memory_order_acquire) == 100)
                            downloadPercentage.store(-1, std::memory_order_release);
                    }

                    if (threadFailure.load(std::memory_order_acquire)) {
                        threadFailure.store(false, std::memory_order_release);
                        commandSuccess = false;
                        //lastRunningInterpreter = true;
                        //logMessage("killing command");
                    }

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
                        {"download", ultrahandRepo + "releases/latest/download/ovlmenu.ovl", "/config/ultrahand/downloads/"},
                        {"move", "/config/ultrahand/downloads/ovlmenu.ovl", "/switch/.overlays/ovlmenu.ovl"}
                    };
                    runningInterpreter.store(true, std::memory_order_release);
                    enqueueInterpreterCommand(std::move(interpreterCommands), "", "");
                    startInterpreterThread();
                    //runningInterpreter.store(true, std::memory_order_release);
                    //lastRunningInterpreter = true;
                    if (isDownloadCommand)
                        listItemPtr->setValue(DOWNLOAD_SYMBOL);
                    else
                        listItemPtr->setValue(INPROGRESS_SYMBOL);
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

                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter) {
                    //int currentPercentage = downloadPercentage.load(std::memory_order_acquire);
                    //logMessage("currentPercentage: "+std::to_string(currentPercentage));
                    if (downloadPercentage.load(std::memory_order_acquire) != -1) {
                        lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(downloadPercentage.load(std::memory_order_acquire))+"%");
                        if (downloadPercentage.load(std::memory_order_acquire) == 100)
                            downloadPercentage.store(-1, std::memory_order_release);
                    }
                    
                    if (threadFailure.load(std::memory_order_acquire)) {
                        threadFailure.store(false, std::memory_order_release);
                        commandSuccess = false;
                        //lastRunningInterpreter = true;
                        //logMessage("killing command");
                    }

                    return false;
                }

                //if (lastRunningInterpreter) {
                //    isDownloadCommand = false;
                //    if (commandSuccess)
                //        lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
                //    else
                //        lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
                //    ////closeInterpreterThread();
                //    lastRunningInterpreter = false;
                //    
                //    return true;
                //}

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                
                if ((keys & KEY_A)) {
                    isDownloadCommand = true;
                    std::vector<std::vector<std::string>> interpreterCommands = {
                        {"delete", "/config/ultrahand/downloads/ovlmenu.ovl"},
                        {"download", ultrahandRepo + "releases/latest/download/lang.zip", "/config/ultrahand/downloads/"},
                        {"unzip", "/config/ultrahand/downloads/lang.zip", "/config/ultrahand/downloads/lang/"},
                        {"delete", "/config/ultrahand/downloads/lang.zip"},
                        {"delete", "/config/ultrahand/lang/"},
                        {"move", "/config/ultrahand/downloads/lang/", "/config/ultrahand/lang/"}
                    };
                    runningInterpreter.store(true, std::memory_order_release);
                    enqueueInterpreterCommand(std::move(interpreterCommands), "", "");
                    startInterpreterThread();
                    //runningInterpreter.store(true, std::memory_order_release);
                    //lastRunningInterpreter = true;
                    if (isDownloadCommand)
                        listItemPtr->setValue(DOWNLOAD_SYMBOL);
                    else
                        listItemPtr->setValue(INPROGRESS_SYMBOL);
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
            addAppInfo(list, overlayHeader, "overlay");
            overlayHeader.clear(); // free memory
            
        } else if (dropdownSelection == "themeMenu") {
            
            list->addItem(new tsl::elm::CategoryHeader(THEME));
            
            std::string currentTheme = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "current_theme");
            
            if (currentTheme.empty())
                currentTheme = "default";
            
            std::vector<std::string> themeFilesList = getFilesListByWildcard(themesPath+"*.ini");
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(DEFAULT);
            
            std::string defaultTheme = themesPath+"default.ini";
            
            if (currentTheme == "default") {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem.reset();
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }
            
            listItem->setClickListener([defaultTheme, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }

                if (keys & KEY_A) {
                    
                    //if (defaultLangMode != defaultLang) {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "current_theme", "default");
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
                
                if (themeName == "default")
                    continue;
                
                listItem = std::make_unique<tsl::elm::ListItem>(themeName);
                
                if (themeName == currentTheme) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem.reset();
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                
                listItem->setClickListener([themeName, currentTheme, themeFile, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'this', 'i', and 'listItem' to the capture list
                    bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                    if (_runningInterpreter)
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }

                    if (keys & KEY_A) {
                        //if (defaultLangMode != defaultLang) {
                        setIniFileValue(settingsConfigIniPath, "ultrahand", "current_theme", themeName);
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
            toggleListItem->setState((hideClock == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_clock", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(SOC_TEMPERATURE, false, ON, OFF);
            toggleListItem->setState((hideSOCTemp == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_soc_temp", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(PCB_TEMPERATURE, false, ON, OFF);
            toggleListItem->setState((hidePCBTemp == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_pcb_temp", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(BATTERY, false, ON, OFF);
            toggleListItem->setState((hideBattery == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_battery", state ? "false" : "true");
                reinitializeWidgetVars();
                redrawWidget = true;
            });
            list->addItem(toggleListItem.release());
            
            
        } else if (dropdownSelection == "miscMenu") {
            list->addItem(new tsl::elm::CategoryHeader(MENU_ITEMS));
            hideUserGuide = parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "hide_user_guide");
            
            auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(USER_GUIDE, false, ON, OFF);
            toggleListItem->setState((hideUserGuide == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_user_guide", state ? "false" : "true");
                if ((hideUserGuide == "false") != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem.release());
            
            
            //list->addItem(new tsl::elm::CategoryHeader(VERSION_LABELS));
            
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
            
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(CLEAN_LABELS, false, ON, OFF);
            toggleListItem->setState((cleanVersionLabels == "true"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "clean_version_labels", state ? "true" : "false");
                if ((cleanVersionLabels == "true") != state) {
                    if (cleanVersionLabels == "false")
                        versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" "+cleanVersionLabel(loaderInfo)+std::string(")");
                    else
                        versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" v"+cleanVersionLabel(loaderInfo)+std::string(")");
                    reinitializeVersionLabels();
                    reloadMenu2 = true;
                    reloadMenu = true;
                }
                
            });
            list->addItem(toggleListItem.release());
            
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(OVERLAY_LABELS, false, ON, OFF);
            toggleListItem->setState((hideOverlayVersions == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_overlay_versions", state ? "false" : "true");
                if ((hideOverlayVersions == "false") != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem.release());
            
            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(PACKAGE_LABELS, false, ON, OFF);
            toggleListItem->setState((hidePackageVersions == "false"));
            toggleListItem->setStateChangedListener([](bool state) {
                setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_package_versions", state ? "false" : "true");
                if ((hidePackageVersions == "false") != state)
                    reloadMenu = true;
            });
            list->addItem(toggleListItem.release());
            
        } else
            list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN + ": " + settingsIniPath));
        

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel);
        //rootFrame->setContent(list);
        //list->clear();

        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>("Ultrahand", versionLabel);
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
        //if ((!returningToSettings && inSettingsMenu && !inSubSettingsMenu && simulatedBack) || (inSubSettingsMenu && simulatedBack)) {
        //    keysHeld |= KEY_B;
        //    simulatedBack = false;
        //    simulatedBackComplete = true;
        //}
        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        if (_runningInterpreter) {
            // Check for back button press
            if ((keysHeld & KEY_R) && !stillTouching) {
                commandSuccess = false;
                abortDownload.store(true, std::memory_order_release);
                abortUnzip.store(true, std::memory_order_release);
                abortFileOp.store(true, std::memory_order_release);
                abortCommand.store(true, std::memory_order_release);
                return true;
            }
            return false;
        }

        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            if (commandSuccess)
                lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
            else
                lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
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
                    
                    //if (parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "last_menu") == "overlays")
                    //    closeInterpreterThread();

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
        if (entryMode == "overlay") {
            settingsIniPath = overlaysIniFilePath;
            header = overlayName;
        } else if (entryMode == "package")
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
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, "priority");
            
            std::string hideOption = parseValueFromIniSection(settingsIniPath, entryName, "hide");
            if (hideOption.empty())
                hideOption = "false";
            
            
            bool hide = false;
            if (hideOption == "true")
                hide = true;
            
            std::string useOverlayLaunchArgs = parseValueFromIniSection(settingsIniPath, entryName, "use_launch_args");
            
            
            // Capitalize entryMode
            std::string hideLabel = entryMode;
            //hideLabel[0] = std::toupper(hideLabel[0]);
            
            if (hideLabel == "overlay")
                hideLabel = HIDE_OVERLAY;
            else if (hideLabel == "package")
                hideLabel = HIDE_PACKAGE;
            
            
            // Envoke toggling
            auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(hideLabel, false, ON, OFF);
            toggleListItem->setState(hide);
            toggleListItem->setStateChangedListener([&settingsIniPath = this->settingsIniPath, &entryName = this->entryName](bool state) {
                setIniFileValue(settingsIniPath, entryName, "hide", state ? "true" : "false");
                if (state)
                    reloadMenu = true; // this reloads before main menu
                else
                    reloadMenu2 = true; // this reloads at main menu
            });
            list->addItem(toggleListItem.release());
            
            
            
            auto listItem = std::make_unique<tsl::elm::ListItem>(SORT_PRIORITY);
            listItem->setValue(priorityValue);
            
            // Envolke selectionOverlay in optionMode
            
            listItem->setClickListener([&entryName = this->entryName, &entryMode = this->entryMode, &overlayName = this->overlayName, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                if (_runningInterpreter)
                    return false;

                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    tsl::changeTo<SettingsMenu>(entryName, entryMode, overlayName, "priority");
                    selectedListItem.reset();
                    selectedListItem = listItemPtr;
                    simulatedSelectComplete = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
            
            if (entryMode == "overlay") {
                // Envoke toggling
                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(LAUNCH_ARGUMENTS, false, ON, OFF);
                toggleListItem->setState((useOverlayLaunchArgs=="true"));
                toggleListItem->setStateChangedListener([&settingsIniPath = settingsIniPath, &entryName = entryName, useOverlayLaunchArgs](bool state) {
                    setIniFileValue(settingsIniPath, entryName, "use_launch_args", state ? "true" : "false");
                    if ((useOverlayLaunchArgs=="true") != state)
                        reloadMenu = true; // this reloads before main menu
                    if (!state)
                        reloadMenu2 = true; // this reloads at main menu
                });
                list->addItem(toggleListItem.release());
            }
            
            
        } else if (dropdownSelection == "priority") {
            list->addItem(new tsl::elm::CategoryHeader(SORT_PRIORITY));
            
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, "priority");
            
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
                    bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                    if (_runningInterpreter)
                        return false;

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        if (iStr != priorityValue)
                            reloadMenu = true;
                        setIniFileValue(settingsIniPath, entryName, "priority", iStr);
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
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>("Ultrahand", versionLabel);
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
        //if ((!returningToSettings && !inSubSettingsMenu && (inSettingsMenu && simulatedBack)) || (inSubSettingsMenu && simulatedBack)) {
        //    keysHeld |= KEY_B;
        //    simulatedBack = false;
        //    simulatedBackComplete = true;
        //}
        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        if (_runningInterpreter) {
            // Check for back button press
            if ((keysHeld & KEY_R) && !stillTouching) {
                commandSuccess = false;
                abortDownload.store(true, std::memory_order_release);
                abortUnzip.store(true, std::memory_order_release);
                abortFileOp.store(true, std::memory_order_release);
                abortCommand.store(true, std::memory_order_release);
                return true;
            }
            return false;
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
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();
        
        std::string packageFile = filePath + packageFileName;
        std::string fileContent = getFileContents(packageFile);
        
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
                        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                        if (_runningInterpreter)
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
                            
                            interpretAndExecuteCommand(std::move(commandVec), filePath, specificKey);
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
        

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame(packageName, "Ultrahand Script");
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(packageName, "Ultrahand Script");
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
        //if (simulatedBack && inScriptMenu) {
        //    keysHeld |= KEY_B;
        //    simulatedBack = false;
        //    simulatedBackComplete = true;
        //}
        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        if (_runningInterpreter) {
            // Check for back button press
            if ((keysHeld & KEY_R) && !stillTouching) {
                commandSuccess = false;
                abortDownload.store(true, std::memory_order_release);
                abortUnzip.store(true, std::memory_order_release);
                abortFileOp.store(true, std::memory_order_release);
                abortCommand.store(true, std::memory_order_release);
                return true;
            }
            return false;
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
                //tsl::Overlay::get()->close();
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
        
        std::string currentSection = "global";
        std::string sourceType = "default", sourceTypeOn = "default", sourceTypeOff = "default"; 
        std::string jsonPath, jsonPathOn, jsonPathOff;
        std::string jsonKey, jsonKeyOn, jsonKeyOff;
        
        
        std::string listString, listStringOn, listStringOff;
        std::vector<std::string> listData, listDataOn, listDataOff;
        std::string jsonString, jsonStringOn, jsonStringOff;
        std::string commandName;
        
        bool inEristaSection = false;
        bool inMarikoSection = false;
        
        // initial processing of commands
        for (const auto& cmd : commands) {
            if (cmd.empty()) { // Isolate command settings
                continue;
            }
            
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
                if (commandMode == "toggle") {
                    if (commandName.find("on:") == 0)
                        currentSection = "on";
                    else if (commandName.find("off:") == 0)
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
                
                if (cmd.size() > 1) { // Pre-process advanced commands
                    if (commandName == "filter") {
                        if (currentSection == "global")
                            filterList.push_back(cmd[1]);
                        else if (currentSection == "on")
                            filterListOn.push_back(cmd[1]);
                        else if (currentSection == "off")
                            filterListOff.push_back(cmd[1]);
                    } else if (commandName == "file_source") {
                        sourceType = "file";
                        if (currentSection == "global") {
                            pathPattern = cmd[1];
                            filesList = getFilesListByWildcards(pathPattern);
                        } else if (currentSection == "on") {
                            pathPatternOn = cmd[1];
                            filesListOn = getFilesListByWildcards(pathPatternOn);
                            sourceTypeOn = "file";
                        } else if (currentSection == "off") {
                            pathPatternOff = cmd[1];
                            filesListOff = getFilesListByWildcards(pathPatternOff);
                            sourceTypeOff = "file";
                        }
                    } else if (commandName == "json_file_source") {
                        sourceType = "json_file";
                        if (currentSection == "global") {
                            jsonPath = preprocessPath(cmd[1]);
                            if (cmd.size() > 2)
                                jsonKey = cmd[2]; //json display key
                        } else if (currentSection == "on") {
                            jsonPathOn = preprocessPath(cmd[1]);
                            sourceTypeOn = "json_file";
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2]; //json display key
                        } else if (currentSection == "off") {
                            jsonPathOff = preprocessPath(cmd[1]);
                            sourceTypeOff = "json_file";
                            if (cmd.size() > 2)
                                jsonKeyOff = cmd[2]; //json display key
                        }
                    } else if (commandName == "list_source") {
                        sourceType = "list";
                        if (currentSection == "global") {
                            listString = removeQuotes(cmd[1]);
                        } else if (currentSection == "on") {
                            listStringOn = removeQuotes(cmd[1]);
                            sourceTypeOn = "list";
                        } else if (currentSection == "off") {
                            listStringOff = removeQuotes(cmd[1]);
                            sourceTypeOff = "list";
                        }
                    } else if (commandName == "json_source") {
                        sourceType = "json";
                        if (currentSection == "global") {
                            jsonString = removeQuotes(cmd[1]); // convert string to jsonData
                            
                            if (cmd.size() > 2)
                                jsonKey = cmd[2]; //json display key
                        } else if (currentSection == "on") {
                            jsonStringOn = removeQuotes(cmd[1]); // convert string to jsonData
                            sourceTypeOn = "json";
                            
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2]; //json display key
                            
                        } else if (currentSection == "off") {
                            jsonStringOff = removeQuotes(cmd[1]); // convert string to jsonData
                            sourceTypeOff = "json";
                            
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
        if (commandMode == "default" || commandMode == "option") {
            if (sourceType == "file")
                selectedItemsList = filesList;
            else if (sourceType == "list")
                selectedItemsList = stringToList(listString);
            else if ((sourceType == "json") || (sourceType == "json_file")) {
                populateSelectedItemsList(sourceType, (sourceType == "json") ? jsonString : jsonPath, jsonKey, selectedItemsList);
                jsonPath = "";
                jsonString = "";
            }
        } else if (commandMode == "toggle") {
            if (sourceTypeOn == "file")
                selectedItemsListOn = filesListOn;
            else if (sourceTypeOn == "list")
                selectedItemsListOn = stringToList(listStringOn);
            else if ((sourceTypeOn == "json") || (sourceTypeOn == "json_file")) {
                populateSelectedItemsList(sourceTypeOn, (sourceTypeOn == "json") ? jsonStringOn : jsonPathOn, jsonKeyOn, selectedItemsListOn);
                jsonPathOn = "";
                jsonStringOn = "";
                
            }
            
            if (sourceTypeOff == "file")
                selectedItemsListOff = filesListOff;
            else if (sourceTypeOff == "list")
                selectedItemsListOff = stringToList(listStringOff);
            else if ((sourceTypeOff == "json") || (sourceTypeOff == "json_file")) {
                populateSelectedItemsList(sourceTypeOff, (sourceTypeOff == "json") ? jsonStringOff : jsonPathOff, jsonKeyOff, selectedItemsListOff);
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
            if ((commandGrouping == "split" || commandGrouping == "split2" || commandGrouping == "split3" || commandGrouping == "split4") && sourceType == "file") {
                
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
        filterList.clear();
        
        if (commandGrouping == "default")
            list->addItem(new tsl::elm::CategoryHeader(removeTag(specificKey.substr(1)))); // remove * from key
        
        // initialize variables
        std::unique_ptr<tsl::elm::ListItem> listItem;
        size_t pos;
        std::string parentDirName;
        std::string footer;
        std::string optionName;
        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>("", true, "", "");
        bool toggleStateOn;
        
        // Add each file as a menu item
        for (size_t i = 0; i < selectedItemsList.size(); ++i) {
            const std::string& selectedItem = selectedItemsList[i];
            
            // For entries that are paths
            itemName = getNameFromPath(selectedItem);
            if (!isDirectory(preprocessPath(selectedItem)))
                itemName = dropExtension(itemName);
            
            if (sourceType == "file") {
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
                        itemName = groupingName.substr(pos + 2); // Assign the part after " - " as the footer
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
                        itemName = groupingName.substr(pos + 2); // Assign the part after " - " as the footer
                        groupingName = groupingName.substr(0, pos); // Strip the " - " and everything after it
                    }

                    
                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)){
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
                else if (commandGrouping == "split4") {
                    groupingName = removeQuotes(getParentDirNameFromPath(selectedItem, 2));
                    itemName = removeQuotes(dropExtension(getNameFromPath(selectedItem)));
                    footer = removeQuotes(getParentDirNameFromPath(selectedItem));

                    
                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)){
                        list->addItem(new tsl::elm::CategoryHeader(groupingName));
                        lastGroupingName = groupingName.c_str();
                    }
                }
            }
            
            
            if (commandMode == "default" || commandMode == "option") { // for handiling toggles

                if (sourceType != "file" && commandGrouping != "split2" && commandGrouping != "split3" && commandGrouping != "split4") {
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
                
                if (commandMode == "option") {
                    if (selectedFooterDict[specifiedFooterKey] == itemName) { // needs to be fixed
                        logMessage("pre-listener itemName: "+itemName);
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
                    bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                    if (_runningInterpreter) {
                        
                        //int currentPercentage = downloadPercentage.load(std::memory_order_acquire);

                        //logMessage("currentPercentage: "+std::to_string(downloadPercentage.load(std::memory_order_acquire)));
                        if (downloadPercentage.load(std::memory_order_acquire) != -1) {
                            lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(downloadPercentage.load(std::memory_order_acquire))+"%");
                            if (downloadPercentage.load(std::memory_order_acquire) == 100)
                                downloadPercentage.store(-1, std::memory_order_release);
                        }

                        if (threadFailure.load(std::memory_order_acquire)) {
                            threadFailure.store(false, std::memory_order_release);
                            commandSuccess = false;
                            //lastRunningInterpreter = true;
                            //logMessage("killing command");
                        }

                        return false;
                    }

                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }

                    if ((keys & KEY_A)) {

                        //if (commandMode == "option") {
                        //    
                        //}
                        //std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(this->commands, selectedItem, i);
                        //applySourceReplacement(this->commands, selectedItem, i);
                        //this->commands = getSourceReplacement(this->commands, selectedItem, i);
                        isDownloadCommand = false;
                        runningInterpreter.store(true, std::memory_order_release);
                        enqueueInterpreterCommand(getSourceReplacement(commands, selectedItem, i), filePath, specificKey);
                        startInterpreterThread();
                        //lastRunningInterpreter = true;
                        //modifiedCmds.clear();
                        //runningInterpreter.store(true, std::memory_order_release);
                        
                        if (isDownloadCommand)
                            listItemPtr->setValue(DOWNLOAD_SYMBOL);
                        else
                            listItemPtr->setValue(INPROGRESS_SYMBOL);
                        if (commandMode == "option") {
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
                
            } else if (commandMode == "toggle") {
                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(itemName, false, ON, OFF);
                
                // Set the initial state of the toggle item
                toggleStateOn = std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end();
                toggleListItem->setState(toggleStateOn);
                
                toggleListItem->setStateChangedListener([&commandsOn = this->commandsOn, &commandsOff = this->commandsOff, &filePath = this->filePath,
                    &specificKey = this->specificKey, i, selectedItem](bool state) {

                    if (!state) {
                        interpretAndExecuteCommand(getSourceReplacement(commandsOn, selectedItem, i), filePath, specificKey); // Execute modified 
                        //toggleListItem->setState(!state);
                    } else {
                        interpretAndExecuteCommand(getSourceReplacement(commandsOff, selectedItem, i), filePath, specificKey); // Execute modified 
                        //toggleListItem->setState(!state);
                    }
                });
                list->addItem(toggleListItem.release());
            }
        }
        
        //filesList.clear();
        //selectedItemsList.clear();
        //selectedItemsListOn.clear();
        //selectedItemsListOff.clear();
        
        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(filePath), "Ultrahand Package", "", packageHeader.color);
        //rootFrame->setContent(list);
        //list->clear();
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(filePath), "Ultrahand Package", "", packageHeader.color);
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
        //if (inSelectionMenu && simulatedBack) {
        //    keysHeld |= KEY_B;
        //    simulatedBack = false;
        //    simulatedBackComplete = true;
        //}
        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        if (_runningInterpreter) {
            // Check for back button press
            if ((keysHeld & KEY_R) && !stillTouching) {
                commandSuccess = false;
                abortDownload.store(true, std::memory_order_release);
                abortUnzip.store(true, std::memory_order_release);
                abortFileOp.store(true, std::memory_order_release);
                abortCommand.store(true, std::memory_order_release);
                //closeInterpreterThread();
                return true;
            }

            return false;
        }

        if (lastRunningInterpreter) {
            isDownloadCommand = false;

            if (commandSuccess)
                lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
            else
                lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
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
       //lastSelectedListItem = nullptr;
       //selectedListItem = nullptr;
        //logMessage("Clearing footer dict...");
        //    selectedListItem = new tsl::elm::ListItem("");
        //    lastSelectedListItem = new tsl::elm::ListItem("");
        if (returningToMain) {
            hexSumCache.clear();
            selectedFooterDict.clear(); // Clears all data from the map, making it empty again
            selectedListItem.reset();
            lastSelectedListItem.reset();
            //selectedListItem = new tsl::elm::ListItem("");
            //lastSelectedListItem = new tsl::elm::ListItem("");
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
        std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
        
        // Load options from INI file in the subdirectory
        std::string packageIniPath = packagePath + packageFileName;
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

        // Populate the sub menu with options
        //for (const auto& option : options) {
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
        std::string sourceType, sourceTypeOn, sourceTypeOff;
        
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
            
            commandFooter = "null";
            commandSystem = "default";
            commandMode = "default";
            commandGrouping = "default";
            
            currentSection = "global";
            sourceType = "default";
            sourceTypeOn = "default";
            sourceTypeOff = "default"; //"file", "json_file", "json", "list"
            //std::string sourceType, sourceTypeOn, sourceTypeOff; //"file", "json_file", "json", "list"
            
            commandsOn.clear();
            commandsOff.clear();
            
            // items can be paths, commands, or variables depending on source
            //std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;
            
            if (drawLocation.empty() || currentPage == drawLocation || (optionName[0] == '@')) {
                
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
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName.substr(1)), DROPDOWN_SYMBOL);
                            
                            listItem->setClickListener([&packagePath=this->packagePath, optionName](s64 keys) {
                                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                                if (_runningInterpreter)
                                    return false;
                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }
                                if (keys & KEY_A) {
                                    inPackageMenu = false;
                                    selectedListItem.reset();
                                    lastSelectedListItem.reset();
                                    tsl::changeTo<PackageMenu>(packagePath, optionName);
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
                
                // initial processing of commands
                for (const auto& cmd : commands) {
                    if (cmd.empty()) { // Isolate command settings
                        continue;
                    }
                    
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
                            if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                commandMode = commandModes[0]; // reset to default if commandMode is unknown
                        } else if (commandName.find(groupingPattern) == 0) {// Extract the command grouping
                            commandGrouping = commandName.substr(groupingPattern.length());
                            if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                        }
                        
                        // Extract the command grouping
                        if (commandMode == "toggle") {
                            if (commandName.find("on:") == 0)
                                currentSection = "on";
                            else if (commandName.find("off:") == 0)
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
                        if (cmd.size() > 1) { // Pre-process advanced commands
                            if (commandName == "file_source") {
                                if (currentSection == "global") {
                                    pathPattern = cmd[1];
                                    sourceType = "file";
                                } else if (currentSection == "on") {
                                    pathPatternOn = cmd[1];
                                    sourceTypeOn = "file";
                                } else if (currentSection == "off") {
                                    pathPatternOff = cmd[1];
                                    sourceTypeOff = "file";
                                }
                            }
                        }
                    }
                }
                
                if (isFileOrDirectory(packageConfigIniPath)) {
                    packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    
                    if (packageConfigData.count(optionName) > 0) {
                        auto& optionSection = packageConfigData[optionName];
                        
                        // For hiding the versions of overlays/packages
                        if (optionSection.count("system") > 0)
                            commandSystem = optionSection["system"];
                        else
                            setIniFileValue(packageConfigIniPath, optionName, "system", commandSystem);

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
                    packageConfigData.clear();
                } else { // write data if settings are not loaded
                    setIniFileValue(packageConfigIniPath, optionName, "system", commandSystem);
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
                    pos = optionName.find(" - ");
                    if (pos != std::string::npos) {
                        footer = optionName.substr(pos + 2); // Assign the part after "&&" as the footer
                        optionName = optionName.substr(0, pos); // Strip the "&&" and everything after it
                    }
                }
                
                if (commandMode == "option" || (commandMode == "toggle" && !useSelection)) {
                    // override loading of the command footer
                    if (commandFooter != "null")
                        footer = commandFooter;
                    else
                        footer = OPTION_SYMBOL;
                }

                skipSystem = false;
                if (commandSystem == "erista" && !usingErista) {
                    skipSystem = true;
                } else if (commandSystem == "mariko" && !usingMariko) {
                    skipSystem = true;
                }
                
                if (!skipSection && !skipSystem) { // for skipping the drawing of sections
                    if (useSelection) { // For wildcard commands (dropdown menus)
                        
                        if ((footer == DROPDOWN_SYMBOL) || (footer.empty()))
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName), footer);
                        else {
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                            if (commandMode == "option")
                                listItem->setValue(footer);
                            else
                                listItem->setValue(footer, true);
                        }
                        
                        if (footer == UNAVAILABLE_SELECTION || footer == "Not available")
                            listItem->setValue(UNAVAILABLE_SELECTION, true);
                        
                        //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                        listItem->setClickListener([commands, keyName = option.first, &packagePath = this->packagePath, footer, lastSection, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                            bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                            if (_runningInterpreter)
                                return false;

                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }

                            if ((keys & KEY_A)) {
                                if (footer != UNAVAILABLE_SELECTION && footer != "Not available") {
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
                                tsl::changeTo<ScriptOverlay>(packagePath, keyName);
                                return true;
                            }
                            return false;
                        });
                        
                        list->addItem(listItem.release());
                    } else { // For everything else
                        
                        const std::string& selectedItem = optionName;
                        
                        // For entries that are paths
                        itemName = getNameFromPath(selectedItem);
                        if (!isDirectory(preprocessPath(selectedItem)))
                            itemName = dropExtension(itemName);
                        parentDirName = getParentDirNameFromPath(selectedItem);
                        
                        
                        if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                            listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                            if (commandMode == "default")
                                listItem->setValue(footer, true);
                            else
                                listItem->setValue(footer);
                            
                            
                            listItem->setClickListener([i, commands, keyName = option.first, &packagePath = this->packagePath, selectedItem, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                                //static bool lastRunningInterpreter = false;
                                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                                if (_runningInterpreter) {
                                    //int currentPercentage = downloadPercentage.load(std::memory_order_acquire);
                                    //logMessage("currentPercentage: "+std::to_string(currentPercentage));
                                    if (downloadPercentage.load(std::memory_order_acquire) != -1) {
                                        lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(downloadPercentage.load(std::memory_order_acquire))+"%");
                                        if (downloadPercentage.load(std::memory_order_acquire) == 100)
                                            downloadPercentage.store(-1, std::memory_order_release);
                                    }
                                    if (threadFailure.load(std::memory_order_acquire)) {
                                        threadFailure.store(false, std::memory_order_release);
                                        commandSuccess = false;
                                        //lastRunningInterpreter = true;
                                        //logMessage("killing command");
                                        //closeInterpreterThread();
                                    }
                                    return false;
                                }

                                if (simulatedSelect && !simulatedSelectComplete) {
                                    keys |= KEY_A;
                                    simulatedSelect = false;
                                }

                                if ((keys & KEY_A)) {

                                    //std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(commands, selectedItem, i);
                                    //applySourceReplacement(commands, selectedItem, i);
                                    //commands = getSourceReplacement(commands, selectedItem, i);
                                    isDownloadCommand = false;
                                    runningInterpreter.store(true, std::memory_order_release);
                                    enqueueInterpreterCommand(getSourceReplacement(commands, selectedItem, i), packagePath, keyName);
                                    startInterpreterThread();
                                    //lastRunningInterpreter = true;
                                    //modifiedCmds.clear();
                                    //runningInterpreter.store(true, std::memory_order_release);
                                    if (isDownloadCommand)
                                        listItemPtr->setValue(DOWNLOAD_SYMBOL);
                                    else
                                        listItemPtr->setValue(INPROGRESS_SYMBOL);
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
                                    tsl::changeTo<ScriptOverlay>(packagePath, keyName);
                                    return true;
                                }
                                return false;
                            });
                            list->addItem(listItem.release());
                        } else if (commandMode == "toggle") {
                            
                            toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(removeTag(optionName), false, ON, OFF);
                            
                            // Set the initial state of the toggle item
                            if (!pathPatternOn.empty())
                                toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn));
                            else
                                toggleStateOn = (footer == "On");
                            
                            toggleListItem->setState(toggleStateOn);
                            
                            toggleListItem->setStateChangedListener([i, commandsOn, commandsOff, keyName = option.first, &packagePath = this->packagePath, &pathPatternOn = this->pathPatternOn, &pathPatternOff = this->pathPatternOff](bool state) {
                                if (state) {
                                    //applySourceReplacement(commandsOn, preprocessPath(pathPatternOn), i);
                                    //commandsOn = getSourceReplacement(commandsOn, preprocessPath(pathPatternOn), i);
                                    interpretAndExecuteCommand(getSourceReplacement(commandsOn, preprocessPath(pathPatternOn), i), packagePath, keyName); // Execute modified
                                    setIniFileValue((packagePath+configFileName).c_str(), keyName.c_str(), "footer", "On");
                                } else {
                                    //applySourceReplacement(commandsOff, preprocessPath(pathPatternOff), i);
                                    //commandsOff = getSourceReplacement(commandsOff, preprocessPath(pathPatternOff), i);
                                    interpretAndExecuteCommand(getSourceReplacement(commandsOff, preprocessPath(pathPatternOff), i), packagePath, keyName); // Execute modified
                                    setIniFileValue((packagePath+configFileName).c_str(), keyName.c_str(), "footer", "Off");
                                }
                            });
                            list->addItem(toggleListItem.release());
                        }
                    }
                }
            }
        }
        
        options.clear();
        filesList.clear();

        //tsl::elm::OverlayFrame *rootFrame = nullptr;
        //rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color);

        std::unique_ptr<tsl::elm::OverlayFrame> rootFrame;

        if (usingPages) {
            if (currentPage == "left") {
                rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color, "", pageRightName);
            }
            else if (currentPage == "right") {
                rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color, pageLeftName, "");
            }
        } else {
            rootFrame = std::make_unique<tsl::elm::OverlayFrame>(getNameFromPath(packagePath), "Ultrahand Package", "", packageHeader.color);
        }

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
        
        //if ((!returningToPackage && inPackageMenu && simulatedBack) || (!returningToSubPackage && inSubPackageMenu && simulatedBack)) {
        //    keysHeld |= KEY_B;
        //    simulatedBack = false;
        //    simulatedBackComplete = true;
        //}
        
        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        if (_runningInterpreter) {
            // Check for back button press
            if ((keysHeld & KEY_R) && !stillTouching) {
                commandSuccess = false;
                abortDownload.store(true, std::memory_order_release);
                abortUnzip.store(true, std::memory_order_release);
                abortFileOp.store(true, std::memory_order_release);
                abortCommand.store(true, std::memory_order_release);
                //closeInterpreterThread();
                return true;
            }

            return false;
        }

        if (lastRunningInterpreter) {
            isDownloadCommand = false;

            if (commandSuccess)
                lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
            else
                lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        // Your existing logic for handling other inputs
        if (refreshGui && !returningToPackage && !stillTouching) {
            refreshGui = false;
            //tsl::changeTo<PackageMenu>(packagePath);
            //closeInterpreterThread();
            
            if (inPackageMenu) {
                lastPackage = packagePath;
                inSubPackageMenu = false;
                inPackageMenu = false;
                if (lastPage == "right") {
                    tsl::goBack();
                    //tsl::goBack();
                } else {
                    tsl::goBack();
                }
                inPackageMenu = true;
                lastPage = "left";
                selectedListItem.reset();
                lastSelectedListItem.reset();
                tsl::changeTo<PackageMenu>(lastPackage);
                //lastPage == "left";
            }

            if (inSubPackageMenu) {
                lastPackage = packagePath;
                inSubPackageMenu = false;
                inPackageMenu = false;
                if (lastPage == "right") {
                    tsl::goBack();
                    tsl::goBack();
                    //tsl::goBack();
                } else {
                    tsl::goBack();
                    tsl::goBack();
                }
                inPackageMenu = true;
                lastPage = "left";
                selectedListItem.reset();
                lastSelectedListItem.reset();
                tsl::changeTo<PackageMenu>(lastPackage);
                //lastPage == "left";
            }
            
        }

        if (usingPages) {
            if (simulatedMenu && !simulatedMenuComplete) {
                simulatedMenu = false;
                simulatedMenuComplete = true;
            }

            if (simulatedNextPage && !simulatedNextPageComplete) {
                if (currentPage == "left") {
                    keysHeld |= KEY_DRIGHT;
                    simulatedNextPage = false;
                }
                else if (currentPage == "right") {
                    keysHeld |= KEY_DLEFT;
                    simulatedNextPage = false;
                }
                else {
                    simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }
            }
            if (currentPage == "left") {
                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    lastPage = "right";
                    lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackage, dropdownSection, "right");
                    simulatedNextPageComplete = true;
                    return true;
                }
            } else if (currentPage == "right") {
                if ((keysHeld & KEY_DLEFT) && !(keysHeld & (KEY_DRIGHT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    //tsl::changeTo<PackageMenu>(packagePath, dropdownSection, "left");
                    lastPage = "left";
                    lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackage, dropdownSection, "left");
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

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }
            if (!usingPages || (usingPages && lastPage == "left")) {
                if ((keysHeld & KEY_B) && !stillTouching) {
                    ////closeInterpreterThread();
                    inPackageMenu = false;

                    if (!inHiddenMode)
                        returningToMain = true;
                    else
                        returningToHiddenMain = true;
                    
                    // Free-up memory
                    hexSumCache.clear();
                    selectedFooterDict.clear(); // Clears all data from the map, making it empty again
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    //selectedListItem = nullptr;
                    //lastSelectedListItem = nullptr;
                    
                    tsl::goBack();
                    //tsl::goBack();
                    //tsl::changeTo<MainMenu>();
                    
                    //tsl::Overlay::get()->close();
                    simulatedBackComplete = true;
                    return true;
                }
            } else if (usingPages && lastPage == "right") {
                if ((keysHeld & KEY_B) && !stillTouching) {
                    ////closeInterpreterThread();
                    inPackageMenu = false;

                    if (!inHiddenMode)
                        returningToMain = true;
                    else
                        returningToHiddenMain = true;

                    
                    // Free-up memory
                    hexSumCache.clear();
                    selectedFooterDict.clear(); // Clears all data from the map, making it empty again
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    //selectedListItem = nullptr;
                    //lastSelectedListItem = nullptr;
                    
                    lastPage = "left";
                    tsl::goBack();
                    //tsl::goBack();
                    //tsl::goBack();
                    //tsl::changeTo<MainMenu>();
                    
                    //tsl::Overlay::get()->close();
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

            if (simulatedBack && !simulatedBackComplete) {
                keysHeld |= KEY_B;
                simulatedBack = false;
            }
            if (!usingPages || (usingPages && lastPage == "left")) {
                if ((keysHeld & KEY_B) && !stillTouching) {
                    ////closeInterpreterThread();
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    tsl::goBack();
                    
                    //tsl::Overlay::get()->close();
                    simulatedBackComplete = true;
                    return true;
                }
            } else if (usingPages && lastPage == "right") {
                if ((keysHeld & KEY_B) && !stillTouching) {
                    ////closeInterpreterThread();
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    //tsl::goBack();
                    tsl::goBack();
                    
                    //tsl::Overlay::get()->close();
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
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
        }
        
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
        if (!inHiddenMode)
            inMainMenu = true;
        
        tsl::hlp::ini::IniData settingsData, packageConfigData;
        std::string packagePath, pathReplace, pathReplaceOn, pathReplaceOff;
        std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
        std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
        
        bool skipSystem = false;
        lastMenuMode = hiddenMenuMode;
        
        //defaultMenuMode = "last_menu";
        //defaultMenuMode = "overlays";
        menuMode = "overlays";
        
        createDirectory(packageDirectory);
        createDirectory(settingsPath);
        
        bool settingsLoaded = false;
        if (isFileOrDirectory(settingsConfigIniPath)) {
            settingsData = getParsedDataFromIniFile(settingsConfigIniPath);
            if (settingsData.count("ultrahand") > 0) {
                auto& ultrahandSection = settingsData["ultrahand"];
                if (ultrahandSection.count("in_overlay") > 0)
                    settingsLoaded = true;
                
                if (ultrahandSection.count("hide_user_guide") > 0)
                    hideUserGuide = ultrahandSection["hide_user_guide"];
                else {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_user_guide", "false");
                }
                
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
                
            }
            settingsData.clear();
        }
        if (!settingsLoaded) { // write data if settings are not loaded
            setIniFileValue(settingsConfigIniPath, "ultrahand", "default_lang", defaultLang);
            //setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
            //setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", menuMode);
            setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "false");
        }
        
        
        std::string langFile = "/config/ultrahand/lang/"+defaultLang+".json";
        if (isFileOrDirectory(langFile))
            parseLanguage(langFile);
        
        // write default theme
        initializeTheme();
        copyTeslaKeyComboToUltrahand();
        
        menuMode = defaultMenu.c_str();

        //if ((defaultMenu == "overlays") || (defaultMenu == "packages")) {
        //    if (defaultMenuLoaded) {
        //        menuMode = defaultMenu.c_str();
        //        defaultMenuLoaded = false;
        //    }
        //} else {
        //    defaultMenuMode = "last_menu";
        //    setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
        //}
        
        if (cleanVersionLabels == "true")
            versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" "+cleanVersionLabel(loaderInfo)+std::string(")"); // Still needs to parse nx-ovlloader instead of hard coding it
        else
            versionLabel = APP_VERSION+std::string("   (")+ extractTitle(loaderInfo)+" v"+cleanVersionLabel(loaderInfo)+std::string(")");
        
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();

        std::unique_ptr<tsl::elm::ListItem> listItem;
        
        if (!hiddenMenuMode.empty())
            menuMode = hiddenMenuMode;
        
        
        // Overlays menu
        if (menuMode == "overlays") {
            //closeInterpreterThread();

            if (!inHiddenMode)
                list->addItem(new tsl::elm::CategoryHeader(OVERLAYS));
            else
                list->addItem(new tsl::elm::CategoryHeader(HIDDEN_OVERLAYS));
            
            
            // Load overlay files
            std::vector<std::string> overlayFiles = getFilesListByWildcard(overlayDirectory+"*.ovl");
            
            
            // Check if the overlays INI file exists
            std::ifstream overlaysIniFile(overlaysIniFilePath);
            if (!overlaysIniFile.is_open()) {
                // The INI file doesn't exist, so create an empty one.
                std::ofstream createFile(overlaysIniFilePath);
                if (!createFile.is_open()) {
                    // Handle the case where the file couldn't be created
                    //initializingSpawn = false; // Or any other appropriate action
                } else {
                    // File created successfully
                    initializingSpawn = true;
                }
            } else {
                // The file exists
                //initializingSpawn = true; // Or any other appropriate action
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
                Result result;
                std::string overlayName, overlayVersion;
                
                for (const auto& overlayFile : overlayFiles) {
                    
                    overlayFileName = getNameFromPath(overlayFile);
                    
                    if (overlayFileName == "ovlmenu.ovl" or overlayFileName.substr(0, 1) == ".")
                        continue;
                    
                    
                    // Check if the overlay name exists in the INI data.
                    if (overlaysIniData.find(overlayFileName) == overlaysIniData.end()) {
                        // The entry doesn't exist; initialize it.
                        overlayList.push_back("0020:"+overlayFileName);
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
                        std::tie(result, overlayName, overlayVersion) = getOverlayInfo(overlayDirectory+overlayFileName);
                        if (result != ResultSuccess)
                            continue;
                        
                        if (hide == "false") {
                            if (starred == "true")
                                overlayList.push_back("-1:"+priority+":"+overlayName+":"+overlayVersion+":"+overlayFileName);
                            else
                                overlayList.push_back(priority+":"+overlayName+":"+overlayVersion+":"+overlayFileName);
                            
                        } else {
                            if (starred == "true")
                                hiddenOverlayList.push_back("-1:"+priority+":"+overlayName+":"+overlayVersion+":"+overlayFileName);
                            else
                                hiddenOverlayList.push_back(priority+":"+overlayName+":"+overlayVersion+":"+overlayFileName);
                            
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
                    overlayStarred = "false";
                    overlayVersion = "";
                    overlayName = "";
                    
                    // Detect if starred
                    if ((taintedOverlayFileName.substr(0, 3) == "-1:"))
                        overlayStarred = "true";
                    
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
                    if (overlayStarred == "true")
                        newOverlayName = STAR_SYMBOL+" "+newOverlayName;
                    
                    
                    // Toggle the starred status
                    newStarred = (overlayStarred == "true") ? "false" : "true";
                    
                    
                    //logMessage(overlayFile);
                    if (isFileOrDirectory(overlayFile)) {
                        listItem = std::make_unique<tsl::elm::ListItem>(newOverlayName);
                        if (cleanVersionLabels == "true")
                            overlayVersion = cleanVersionLabel(overlayVersion);
                        if (hideOverlayVersions != "true")
                            listItem->setValue(overlayVersion, true);
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([&hiddenMenuMode = this->hiddenMenuMode, overlayFile, newStarred, overlayFileName, overlayName](s64 keys) {
                            bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                            if (_runningInterpreter) {

                                return false;
                            }

                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }

                            if (keys & KEY_A) {
                                
                                setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "true"); // this is handled within tesla.hpp
                                std::string useOverlayLaunchArgs = parseValueFromIniSection(overlaysIniFilePath, overlayFileName, "use_launch_args");
                                std::string overlayLaunchArgs = parseValueFromIniSection(overlaysIniFilePath, overlayFileName, "launch_args");
                                
                                
                                if (useOverlayLaunchArgs == "true")
                                    tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                                else
                                    tsl::setNextOverlay(overlayFile);
                                
                                tsl::Overlay::get()->close();
                                simulatedSelectComplete = true;

                                return true;
                            } else if (keys & STAR_KEY) {
                                //std::string tmpMode(hiddenMenuMode);
                                if (!overlayFile.empty()) {
                                    // Update the INI file with the new value
                                    setIniFileValue(overlaysIniFilePath, overlayFileName, "star", newStarred);
                                    // Now, you can use the newStarred value for further processing if needed
                                }
                                if (inHiddenMode) {
                                    //tsl::goBack();
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                    reloadMenu2 = true;
                                }
                                tsl::changeTo<MainMenu>(hiddenMenuMode);
                                //lastMenuMode = tmpMode;
                                return true;
                            } else if (keys & SETTINGS_KEY) {
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
                        list->addItem(listItem.release());
                }
                overlayList.clear();
                
                if (!hiddenOverlayList.empty() && !inHiddenMode) {
                    listItem = std::make_unique<tsl::elm::ListItem>(HIDDEN, DROPDOWN_SYMBOL);
                    
                    //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                    listItem->setClickListener([](uint64_t keys) {
                        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                        if (_runningInterpreter)
                            return false;

                        if (simulatedSelect && !simulatedSelectComplete) {
                            keys |= KEY_A;
                            simulatedSelect = false;
                        }

                        if (keys & KEY_A) {
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>("overlays");
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
        if (menuMode == "packages" ) {
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
                for (const auto& packageName: subdirectories) {
                    if (packageName.substr(0, 1) == ".")
                        continue;
                    // Check if the overlay name exists in the INI data.
                    if (packagesIniData.find(packageName) == packagesIniData.end()) {
                        // The entry doesn't exist; initialize it.
                        packageList.push_back("0020:"+packageName);
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
                                packageList.push_back("-1:"+priority+":"+packageName);
                            else
                                packageList.push_back(priority+":"+packageName);
                        } else {
                            if (starred == "true")
                                hiddenPackageList.push_back("-1:"+priority+":"+packageName);
                            else
                                hiddenPackageList.push_back(priority+":"+packageName);
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
                        if (!inHiddenMode)
                            list->addItem(new tsl::elm::CategoryHeader(PACKAGES));
                        else
                            list->addItem(new tsl::elm::CategoryHeader(HIDDEN_PACKAGES));
                    }
                    //bool usingStar = false;
                    packageName = taintePackageName.c_str();
                    packageStarred = "false";
                    
                    if ((packageName.length() >= 2) && (packageName.substr(0, 3) == "-1:")) {
                        // strip first two characters
                        packageName = packageName.substr(3);
                        packageStarred = "true";
                    }
                    
                    packageName = packageName.substr(5);
                    
                    newPackageName = packageName.c_str();
                    if (packageStarred == "true")
                        newPackageName = STAR_SYMBOL+" "+newPackageName;
                    
                    packageFilePath = packageDirectory + packageName+ "/";
                    
                    // Toggle the starred status
                    newStarred = (packageStarred == "true") ? "false" : "true";
                    
                    //std::unique_ptr<tsl::elm::ListItem> listItem;
                    if (isFileOrDirectory(packageFilePath)) {
                        packageHeader = getPackageHeaderFromIni(packageFilePath+packageFileName);
                        
                        listItem = std::make_unique<tsl::elm::ListItem>(newPackageName);
                        if (cleanVersionLabels == "true")
                            packageHeader.version = removeQuotes(cleanVersionLabel(packageHeader.version));
                        if (hidePackageVersions != "true")
                           listItem->setValue(packageHeader.version, true);
                        
                        packageHeader.clear(); // free memory
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([&hiddenMenuMode = this->hiddenMenuMode, packageFilePath, newStarred, packageName](s64 keys) {
                            bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                            if (_runningInterpreter) {
    
                                return false;
                            }
                            
                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }
                            
                            if (keys & KEY_A) {
                                inMainMenu = false;
                                //inHiddenMode = false;
                                
                                // read commands from package's boot_package.ini
                                if (isFileOrDirectory(packageFilePath+bootPackageFileName)) {
                                    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> bootOptions = loadOptionsFromIni(packageFilePath+bootPackageFileName, true);
                                    if (bootOptions.size() > 0) {
                                        std::string bootOptionName;
                                        for (auto& bootOption:bootOptions) {
                                            bootOptionName = bootOption.first;
                                            auto& bootCommands = bootOption.second;
                                            if (bootOptionName == "boot") {
                                                interpretAndExecuteCommand(std::move(bootCommands), packageFilePath+bootPackageFileName, bootOptionName); // Execute modified
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
                                    setIniFileValue(packagesIniFilePath, packageName, "star", newStarred); // Update the INI file with the new value
                                
                                if (inHiddenMode) {
                                    //tsl::goBack();
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                    reloadMenu2 = true;
                                }
                                tsl::changeTo<MainMenu>(hiddenMenuMode);
                                return true;
                            } else if (keys & SETTINGS_KEY) {
                                
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
                        list->addItem(listItem.release());
                    }
                }
                packageList.clear();
                
                if (!hiddenPackageList.empty() && !inHiddenMode) {
                    listItem = std::make_unique<tsl::elm::ListItem>(HIDDEN, DROPDOWN_SYMBOL);
                    listItem->setClickListener([](uint64_t keys) {
                        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                        if (_runningInterpreter)
                            return false;
                        
                        if (simulatedSelect && !simulatedSelectComplete) {
                            keys |= KEY_A;
                            simulatedSelect = false;
                        }
                        
                        if (keys & KEY_A) {
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>("packages");
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
                
                std::string commandFooter = "null";
                std::string commandSystem = "default";
                std::string commandMode = "default";
                std::string commandGrouping = "default";
                
                std::string currentSection = "global";
                std::string sourceType = "default", sourceTypeOn = "default", sourceTypeOff = "default"; 
                //std::string sourceType, sourceTypeOn, sourceTypeOff; //"file", "json_file", "json", "list"
                std::string jsonPath, jsonPathOn, jsonPathOff;
                std::string jsonKey, jsonKeyOn, jsonKeyOff;
                
                std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
                //std::vector<std::string> listData, listDataOn, listDataOff;
                
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
                    
                    commandFooter = "null";
                    commandSystem = "default";
                    commandMode = "default";
                    commandGrouping = "default";
                    
                    currentSection = "global";
                    sourceType = "default";
                    sourceTypeOn = "default";
                    sourceTypeOff = "default"; 
                    
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
                        if (commands.size() == 0 && optionName[0] != '*') {
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
                    
                    if (optionName[0] == '*') {
                        // Create reference to PackageMenu with dropdownSection set to optionName
                        listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName.substr(1)), DROPDOWN_SYMBOL);
                        
                        listItem->setClickListener([optionName](s64 keys) {
                            bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                            if (_runningInterpreter)
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



                    //std::string commandName;
                    
                    // initial processing of commands
                    inEristaSection = false;
                    inMarikoSection = false;
                    
                    // initial processing of commands
                    for (const auto& cmd : commands) {
                        if (cmd.empty()) { // Isolate command settings
                            continue;
                        }
                        
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
                                if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                                    commandMode = commandModes[0]; // reset to default if commandMode is unknown
                            } else if (commandName.find(groupingPattern) == 0) {// Extract the command grouping
                                commandGrouping = commandName.substr(groupingPattern.length());
                                if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                    commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                            }
                            
                            // Extract the command grouping
                            if (commandMode == "toggle") {
                                if (commandName.find("on:") == 0)
                                    currentSection = "on";
                                else if (commandName.find("off:") == 0)
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
                            if (cmd.size() > 1) { // Pre-process advanced commands
                                if (commandName == "file_source") {
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
                    }
                    
                    
                    if (isFileOrDirectory(packageConfigIniPath)) {
                        packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                        
                        
                        if (packageConfigData.count(optionName) > 0) {
                            auto& optionSection = packageConfigData[optionName];
                            
                            // For hiding the versions of overlays/packages

                            if (optionSection.count("system") > 0)
                                commandSystem = optionSection["system"];
                            else
                                setIniFileValue(packageConfigIniPath, optionName, "system", commandSystem);

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
                        packageConfigData.clear();
                    } else { // write data if settings are not loaded
                        setIniFileValue(packageConfigIniPath, optionName, "system", commandSystem);
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
                        pos = optionName.find(" - ");
                        if (pos != std::string::npos) {
                            footer = optionName.substr(pos + 2); // Assign the part after "&&" as the footer
                            optionName = optionName.substr(0, pos); // Strip the "&&" and everything after it
                        }
                    }
                    
                    // override loading of the command footer
                    //if (commandFooter != "null")
                    //    footer = commandFooter;
                    if (commandMode == "option" || (commandMode == "toggle" && !useSelection)) {
                        // override loading of the command footer
                        if (commandFooter != "null")
                            footer = commandFooter;
                        else
                            footer = OPTION_SYMBOL;
                    }

                    skipSystem = false;
                    if (commandSystem == "erista" && !usingErista) {
                        skipSystem = true;
                    } else if (commandSystem == "mariko" && !usingMariko) {
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
                                bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                                if (_runningInterpreter)
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
                            if (!isDirectory(preprocessPath(selectedItem)))
                                itemName = dropExtension(itemName);
                            parentDirName = getParentDirNameFromPath(selectedItem);
                            
                            
                            if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                                listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName));
                                listItem->setValue(footer, true);
                                
                                if (sourceType == "json") { // For JSON wildcards
                                    
                                    listItem->setClickListener([i, commands, packagePath = packageDirectory, keyName = option.first, selectedItem, listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) { // Add 'command' to the capture list
                                        //static bool lastRunningInterpreter = false;

                                        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                                        if (_runningInterpreter) {
                                            //int currentPercentage = downloadPercentage.load(std::memory_order_acquire);
                                            //logMessage("currentPercentage: "+std::to_string(currentPercentage));
                                            if (downloadPercentage.load(std::memory_order_acquire) != -1) {
                                                lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(downloadPercentage.load(std::memory_order_acquire))+"%");
                                                if (downloadPercentage.load(std::memory_order_acquire) == 100)
                                                    downloadPercentage.store(-1, std::memory_order_release);
                                            }
                                            
                                            if (threadFailure.load(std::memory_order_acquire)) {
                                                threadFailure.store(false, std::memory_order_release);
                                                commandSuccess = false;
                                                //lastRunningInterpreter = true;
                                                //logMessage("killing command");
                                            }
                                            return false;
                                        }


                                        if (simulatedSelect && !simulatedSelectComplete) {
                                            keys |= KEY_A;
                                            simulatedSelect = false;
                                        }


                                        if ((keys & KEY_A)) {
                                            
                                            //std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(commands, selectedItem, i);
                                            //applySourceReplacement(commands, selectedItem, i);
                                            //commands = getSourceReplacement(commands, selectedItem, i);
                                            isDownloadCommand = false;
                                            runningInterpreter.store(true, std::memory_order_release);
                                            enqueueInterpreterCommand(getSourceReplacement(commands, selectedItem, i), packagePath, keyName);
                                            startInterpreterThread();
                                            //modifiedCmds.clear();
                                            //runningInterpreter.store(true, std::memory_order_release);
                                            //lastRunningInterpreter = true;
                                            if (isDownloadCommand)
                                                listItemPtr->setValue(DOWNLOAD_SYMBOL);
                                            else
                                                listItemPtr->setValue(INPROGRESS_SYMBOL);
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
                                        
                                        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
                                        if (_runningInterpreter) {
                                            //int currentPercentage = downloadPercentage.load(std::memory_order_acquire);
                                            //logMessage("currentPercentage: "+std::to_string(currentPercentage));
                                            if (downloadPercentage.load(std::memory_order_acquire) != -1) {
                                                lastSelectedListItem->setValue(DOWNLOAD_SYMBOL + " " + std::to_string(downloadPercentage.load(std::memory_order_acquire))+"%");
                                                if (downloadPercentage.load(std::memory_order_acquire) == 100)
                                                    downloadPercentage.store(-1, std::memory_order_release);
                                            }
                                            if (threadFailure.load(std::memory_order_acquire)) {
                                                threadFailure.store(false, std::memory_order_release);
                                                commandSuccess = false;
                                                //lastRunningInterpreter = true;
                                                //logMessage("killing command");
                                            }
                                            return false;
                                        }

                                        if (simulatedSelect && !simulatedSelectComplete) {
                                            keys |= KEY_A;
                                            simulatedSelect = false;
                                        }
                                        

                                        if ((keys & KEY_A)) {
                                            
                                            //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                            
                                            //std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(commands, selectedItem, i);
                                            //applySourceReplacement(commands, selectedItem, i);
                                            //commands = getSourceReplacement(commands, selectedItem, i);
                                            isDownloadCommand = false;
                                            runningInterpreter.store(true, std::memory_order_release);
                                            enqueueInterpreterCommand(getSourceReplacement(commands, selectedItem, i), packagePath, keyName);
                                            startInterpreterThread();
                                            //lastRunningInterpreter = true;
                                            //modifiedCmds.clear();
                                            //runningInterpreter.store(true, std::memory_order_release);
                                            
                                            if (isDownloadCommand)
                                                listItemPtr->setValue(DOWNLOAD_SYMBOL);
                                            else
                                                listItemPtr->setValue(INPROGRESS_SYMBOL);

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
                            } else if (commandMode == "toggle") {
                                
                                toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(removeTag(optionName), false, ON, OFF);
                                
                                // Set the initial state of the toggle item
                                if (!pathPatternOn.empty())
                                    toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn));
                                else
                                    toggleStateOn = (footer == "On");
                                
                                toggleListItem->setState(toggleStateOn);
                                
                                toggleListItem->setStateChangedListener([i, pathPatternOn, pathPatternOff, commandsOn, commandsOff, packagePath = packageDirectory, keyName = option.first](bool state) {
                                    if (state) {
                                        //applySourceReplacement(commandsOn, preprocessPath(pathPatternOn), i);
                                        //commandsOn = getSourceReplacement(commandsOn, preprocessPath(pathPatternOn), i);
                                        interpretAndExecuteCommand(getSourceReplacement(commandsOn, preprocessPath(pathPatternOn), i), packagePath, keyName); // Execute modified
                                        setIniFileValue((packagePath+configFileName).c_str(), keyName.c_str(), "footer", "On");
                                    } else {
                                        //applySourceReplacement(commandsOff, preprocessPath(pathPatternOff), i);
                                        //commandsOff = getSourceReplacement(commandsOff, preprocessPath(pathPatternOff), i);
                                        interpretAndExecuteCommand(getSourceReplacement(commandsOff, preprocessPath(pathPatternOff), i), packagePath, keyName); // Execute modified
                                        setIniFileValue((packagePath+configFileName).c_str(), keyName.c_str(), "footer", "Off");
                                    }
                                });
                                list->addItem(toggleListItem.release());
                            }
                        }
                    }
                }
                
                if (hideUserGuide != "true" && dropdownSection.empty())
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
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>("Ultrahand", versionLabel, menuMode+hiddenMenuMode+dropdownSection);

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

        //if ((!inHiddenMode && inMainMenu && simulatedBack) || (!inMainMenu && inHiddenMode && simulatedBack)) {
        //    keysHeld |= KEY_B;
        //    simulatedBack = false;
        //    simulatedBackComplete = true;
        //}

        bool _runningInterpreter = runningInterpreter.load(std::memory_order_acquire);
        if (_runningInterpreter) {
            // Check for back button press
            if ((keysHeld & KEY_R) && !stillTouching) {
                commandSuccess = false;
                abortDownload.store(true, std::memory_order_release);
                abortUnzip.store(true, std::memory_order_release);
                abortFileOp.store(true, std::memory_order_release);
                abortCommand.store(true, std::memory_order_release);
                return true;
            }
            return false;
        }

        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            
            if (commandSuccess)
                lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
            else
                lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }

        if (refreshGui && !stillTouching) {
            refreshGui = false;
            ////closeInterpreterThread();
            tsl::goBack();
            //setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", "packages");
            defaultMenu = "packages";
            returningToMain = true;
            tsl::changeTo<MainMenu>();
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
                    if (menuMode != "packages") {
                        keysHeld |= KEY_DRIGHT;
                        simulatedNextPage = false;
                    }
                    else if (menuMode != "overlays") {
                        keysHeld |= KEY_DLEFT;
                        simulatedNextPage = false;
                    } else {
                        simulatedNextPage = false;
                        simulatedNextPageComplete = true;
                    }
                }

                if ((keysHeld & KEY_DRIGHT) && !(keysHeld & (KEY_DLEFT | KEY_DUP | KEY_DDOWN | KEY_B | KEY_A | KEY_X | KEY_Y | KEY_L | KEY_R | KEY_ZL | KEY_ZR)) && !stillTouching) {
                    if (menuMode != "packages") {
                        //setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", "packages");
                        defaultMenu = "packages";
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
                    if (menuMode != "overlays") {
                        //setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", "overlays");
                        defaultMenu = "overlays";
                        selectedListItem.reset();
                        lastSelectedListItem.reset();
                        //closeInterpreterThread();
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        //tsl::Overlay::get()->getCurrentGui()->createUI();
                        //tsl::Overlay::get()->getCurrentGui()->removeFocus();
                        simulatedNextPageComplete = true;
                        return true;
                    }
                }

                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }

                if ((keysHeld & KEY_B) && !stillTouching) {
                    //inMainMenu = false;
                    //setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", defaultMenuMode);
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
                    //if (parseValueFromIniSection(settingsConfigIniPath, "ultrahand", "last_menu") == "overlays")
                    //    startInterpreterThread();
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
                    returningToMain = true;
                    inHiddenMode = false;
                    
                    if (reloadMenu2) {
                        tsl::goBack();
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
            selectedFooterDict.clear();
            hexSumCache.clear();
        }
        if (returningToHiddenMain && !(keysHeld & KEY_B)){
            returningToHiddenMain = false;
            inHiddenMode = true;
            selectedFooterDict.clear();
            hexSumCache.clear();
        }
        
        if (redrawWidget) {
            reinitializeWidgetVars();
            redrawWidget = false;
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
            tsl::Overlay::get()->close();
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
        //startInterpreterThread();
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
    virtual void onShow() override {
        //if (usingMariko) {
        //    logMessage("Using Mariko.");
        //}
        //if (usingErista) {
        //    logMessage("Using Erista.");
        //}
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
