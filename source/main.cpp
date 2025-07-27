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


using namespace ult;

// Memory ordering constants for cleaner syntax
constexpr auto acquire = std::memory_order_acquire;
constexpr auto acq_rel = std::memory_order_acq_rel;
constexpr auto release = std::memory_order_release;


// Placeholder replacement
const std::string valuePlaceholder = "{value}";
const std::string indexPlaceholder = "{index}";
const size_t valuePlaceholderLength = valuePlaceholder.length();
const size_t indexPlaceholderLength = indexPlaceholder.length();

static std::string selectedPackage; // for package forwarders

// Overlay booleans
static bool returningToMain = false;
static bool returningToHiddenMain = false;
static bool returningToSettings = false;
static bool returningToPackage = false;
static bool returningToSubPackage = false;
static bool returningToSelectionMenu = false;
static bool languageWasChanged = false;
static bool themeWasChanged = false;

//static bool skipJumpReset = false; // for overrridng the default main menu jump to implementation // moved to utils

//static bool inMainMenu = false; // moved to libtesla
static bool wasInHiddenMode = false;
static bool inHiddenMode = false;
static bool inSettingsMenu = false;
static bool inSubSettingsMenu = false;
static bool inPackageMenu = false;
static bool inSubPackageMenu = false;
static bool inScriptMenu = false;
static bool inSelectionMenu = false;


//static bool currentMenuLoaded = true;
static bool freshSpawn = true;
//static bool refreshPage = false; //moved to libtesla
static bool reloadMenu = false;
static bool reloadMenu2 = false;
//static bool reloadMenu3 = false;
static bool triggerMenuReload = false;
static bool triggerMenuReload2 = false;


static size_t nestedMenuCount = 0;

// Command mode globals
static const std::vector<std::string> commandSystems = {DEFAULT_STR, ERISTA_STR, MARIKO_STR};
static const std::vector<std::string> commandModes = {DEFAULT_STR, SLOT_STR, TOGGLE_STR, OPTION_STR, FORWARDER_STR, TEXT_STR, TABLE_STR, TRACKBAR_STR, STEP_TRACKBAR_STR, NAMED_STEP_TRACKBAR_STR};
static const std::vector<std::string> commandGroupings = {DEFAULT_STR, "split", "split2", "split3", "split4", "split5"};
static const std::string MODE_PATTERN = ";mode=";
static const std::string GROUPING_PATTERN = ";grouping=";
static const std::string SYSTEM_PATTERN = ";system=";
static const std::string WIDGET_PATTERN = ";widget=";

static const std::string MINI_PATTERN = ";mini=";
static const std::string SELECTION_MINI_PATTERN = ";selection_mini=";

// Table option patterns
static const std::string POLLING_PATTERN = ";polling=";
static const std::string SCROLLABLE_PATTERN = ";scrollable=";
static const std::string TOP_PIVOT_PATTERN = ";top_pivot=";
static const std::string BOTTOM_PIVOT_PATTERN = ";bottom_pivot=";
static const std::string BACKGROUND_PATTERN = ";background="; // true or false
static const std::string HEADER_INDENT_PATTERN = ";header_indent="; // true or false
//static const std::string HEADER_PATTERN = ";header=";
static const std::string ALIGNMENT_PATTERN = ";alignment=";
static const std::string WRAPPING_MODE_PATTERN = ";wrapping_mode="; // "none", "char", "word"
static const std::string WRAPPING_INDENT_PATTERN = ";wrapping_indent="; // true or false
static const std::string START_GAP_PATTERN =";start_gap=";
static const std::string END_GAP_PATTERN =";end_gap=";
static const std::string END_GAP_PATTERN_ALIAS =";gap=";
static const std::string OFFSET_PATTERN = ";offset=";
static const std::string SPACING_PATTERN = ";spacing=";
static const std::string INFO_TEXT_COLOR_PATTERN = ";info_text_color=";
static const std::string SECTION_TEXT_COLOR_PATTERN = ";section_text_color=";

// Trackbar option patterns
static const std::string MIN_VALUE_PATTERN = ";min_value=";
static const std::string MAX_VALUE_PATTERN = ";max_value=";
static const std::string STEPS_PATTERN = ";steps=";
static const std::string UNITS_PATTERN = ";units=";
static const std::string UNLOCKED_PATTERN = ";unlocked=";
static const std::string ON_EVERY_TICK_PATTERN = ";on_every_tick=";

static std::string currentMenu = OVERLAYS_STR;
static std::string lastPage = LEFT_STR;
static std::string lastPackagePath;
static std::string lastPackageName;
static std::string lastPackageMenu;
static std::string lastPageHeader;

static std::string lastMenu = "";
static std::string lastMenuMode = "";
static std::string lastKeyName = "";
static bool hideUserGuide = false;
//static bool hideHidden = false;

static std::string lastCommandMode;


static std::unordered_map<std::string, std::string> selectedFooterDict;

static tsl::elm::ListItem* selectedListItem;
static tsl::elm::ListItem* lastSelectedListItem;
static tsl::elm::ListItem* forwarderListItem;
//static tsl::elm::ListItem* dropdownListItem;

static bool lastRunningInterpreter = false;



template<typename Map, typename Func = std::function<std::string(const std::string&)>, typename... Args>
std::string getValueOrDefault(const Map& data, const std::string& key, const std::string& defaultValue, Func formatFunc = nullptr, Args... args) {
    auto it = data.find(key);
    if (it != data.end()) {
        return formatFunc ? formatFunc(it->second, args...) : it->second;
    }
    return defaultValue;
}


inline void clearMemory() {
    
    hexSumCache.clear();
    selectedFooterDict.clear(); // Clears all data from the map, making it empty again
    //selectedListItem = nullptr;
    //lastSelectedListItem = nullptr;
    //forwarderListItem = nullptr;
}

void shiftItemFocus(tsl::elm::Element* element) {
    tsl::Overlay::get()->getCurrentGui()->requestFocus(element, tsl::FocusDirection::None);
}



/**
 * @brief Handles updates and checks when the interpreter is running.
 *
 * This function processes the progression of download, unzip, and copy operations,
 * updates the user interface accordingly, and handles the thread failure and abort conditions.
 *
 * @param keysDown A bitset representing keys that are pressed down.
 * @param keysHeld A bitset representing keys that are held down.
 * @return `true` if the operation needs to abort, `false` otherwise.
 */
bool handleRunningInterpreter(uint64_t& keysDown, uint64_t& keysHeld) {
    static int lastPct = -1;
    static uint8_t lastOp = 255;
    static bool inProg = true;
    static uint8_t currentOpIndex = 0;  // Track which operation to check first
    
    // FIX: More robust abort handling
    if (((keysDown & KEY_R) && !(keysHeld & ~KEY_R & ALL_KEYS_MASK) && !stillTouching.load(acquire)) || externalAbortCommands.load(std::memory_order_relaxed)) {
        // Set all abort flags with proper ordering
        abortDownload.store(true, release);
        abortUnzip.store(true, release);
        abortFileOp.store(true, release);
        abortCommand.store(true, release);
        externalAbortCommands.store(false, release);
        // Reset UI state
        commandSuccess = false;
        lastPct = -1;
        lastOp = 255;
        inProg = true;
        currentOpIndex = 0;  // Reset operation tracking
        
        return true;
    }
    
    // FIX: Check abort flags with acquire ordering
    if (abortDownload.load(acquire) ||
        abortUnzip.load(acquire) || 
        abortFileOp.load(acquire) || 
        abortCommand.load(acquire)) {
        return true;
    }
    
    // Other input handling
    if ((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK) && !stillTouching.load(acquire)) {
        tsl::Overlay::get()->hide();
    }
    
    if (threadFailure.load(acquire)) {
        threadFailure.store(false, release);
        commandSuccess = false;
    }
    
    // FIX: Ultra-optimized progress tracking - single operation check
    static std::atomic<int>* const pcts[] = {&downloadPercentage, &unzipPercentage, &copyPercentage};
    static const std::string* const syms[] = {&DOWNLOAD_SYMBOL, &UNZIP_SYMBOL, &COPY_SYMBOL};
    
    int currentPct = -1;
    uint8_t currentOp = 255;
    
    // If we know operations are sequential, check current index first
    int pct = pcts[currentOpIndex]->load(acquire);
    
    bool displayed100 = false;
    if (pct >= 0 && pct < 100) {
        // Current operation is active
        currentPct = pct;
        currentOp = currentOpIndex;
    } else if (pct == 100) {
        displayPercentage.store(100, release);
        if (lastSelectedListItem)
            lastSelectedListItem->setValue(*syms[currentOpIndex] + " 100%");
        displayed100 = true;

        // Current operation completed, mark and advance
        pcts[currentOpIndex]->store(-1, release);
        currentOpIndex = (currentOpIndex + 1) % 3;
        
        // Check if next operation is already active
        pct = pcts[currentOpIndex]->load(acquire);
        if (pct >= 0 && pct < 100) {
            currentPct = pct;
            currentOp = currentOpIndex;
        }
    } else {
        // Current operation not active, do a quick scan for any active operation
        for (uint8_t i = 0; i < 3; ++i) {
            pct = pcts[i]->load(acquire);
            if (pct >= 0 && pct < 100) {
                currentPct = pct;
                currentOp = i;
                currentOpIndex = i;
                break;
            }
        }
    }
    
    // Update UI only when necessary
    if (currentOp != 255 && (currentPct != lastPct || currentOp != lastOp)) {
        if (!displayed100) {
            displayPercentage.store(currentPct, release);
            if (lastSelectedListItem)
                lastSelectedListItem->setValue(*syms[currentOp] + " " + ult::to_string(currentPct) + "%");
        }
        lastPct = currentPct;
        lastOp = currentOp;
        inProg = true;  // Reset inProg when we have active operations
    } else if (currentOp == 255 && inProg) {  // Remove lastPct < 0 condition
        displayPercentage.store(-1, release);
        if (lastSelectedListItem)
            lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
        inProg = false;
        lastPct = -1;  // Reset lastPct for next cycle
    }
    
    return false;
}



// Forward declaration of the MainMenu class.
class MainMenu;


static std::string lastSelectedListItemFooter;

class UltrahandSettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, dropdownSelection, settingsIniPath;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false;
    std::string languagesVersion = APP_VERSION;
    int MAX_PRIORITY = 20;
    std::string comboLabel;
    //std::string lastSelectedListItemFooter = "";
    
    bool rightAlignmentState;

    void addListItem(tsl::elm::List* list, const std::string& title, const std::string& value, const std::string& targetMenu) {
        auto* listItem = new tsl::elm::ListItem(title);
        listItem->setValue(value);
        listItem->setClickListener([listItem, targetMenu](uint64_t keys) {
            if (runningInterpreter.load(acquire))
                return false;

            if (simulatedSelect.exchange(false, acq_rel)) {
                keys |= KEY_A;
            }
            if (keys & KEY_A) {

                if (targetMenu == "softwareUpdateMenu") {
                    deleteFileOrDirectory(SETTINGS_PATH+"RELEASE.ini");
                    downloadFile(LATEST_RELEASE_INFO_URL, SETTINGS_PATH);
                    downloadPercentage.store(-1, release);
                } else if (targetMenu == "themeMenu") {
                    if (!isFile(THEMES_PATH+"ultra.ini")) {
                        downloadFile(INCLUDED_THEME_FOLDER_URL+"ultra.ini", THEMES_PATH);
                        downloadPercentage.store(-1, release);
                    }
                    if (!isFile(THEMES_PATH+"classic.ini")) {
                        downloadFile(INCLUDED_THEME_FOLDER_URL+"classic.ini", THEMES_PATH);
                        downloadPercentage.store(-1, release);
                    }
                }

                tsl::changeTo<UltrahandSettingsMenu>(targetMenu);
                //selectedListItem = nullptr;
                selectedListItem = listItem;
                return true;
            }
            return false;
        });
        list->addItem(listItem);
    }
    
    // Remove all combos from other overlays / packages
    void removeKeyComboFromAllOthers(const std::string& keyCombo) {
        // Declare variables once for reuse across both scopes
        std::string existingCombo;
        std::string comboListStr;
        std::vector<std::string> comboList;
        bool modified;
        std::string newComboStr;
        
        // Process overlays first
        {
            auto overlaysIniData = getParsedDataFromIniFile(OVERLAYS_INI_FILEPATH);
            bool overlaysModified = false;
            
            const auto overlayNames = getOverlayNames(); // Get all overlay names
            
            for (const auto& overlayName : overlayNames) {
                auto overlayIt = overlaysIniData.find(overlayName);
                if (overlayIt == overlaysIniData.end()) continue; // Skip if overlay not in INI
                
                auto& overlaySection = overlayIt->second;
                
                // 1. Remove from main key_combo field if it matches
                auto keyComboIt = overlaySection.find(KEY_COMBO_STR);
                if (keyComboIt != overlaySection.end()) {
                    existingCombo = keyComboIt->second;
                    if (!existingCombo.empty() && tsl::hlp::comboStringToKeys(existingCombo) == tsl::hlp::comboStringToKeys(keyCombo)) {
                        overlaySection[KEY_COMBO_STR] = "";
                        overlaysModified = true;
                    }
                }
                
                // 2. Remove from mode_combos list if any element matches
                auto modeCombosIt = overlaySection.find("mode_combos");
                if (modeCombosIt != overlaySection.end()) {
                    comboListStr = modeCombosIt->second;
                    if (!comboListStr.empty()) {
                        comboList = splitIniList(comboListStr);
                        modified = false;
                        
                        for (std::string& combo : comboList) {
                            if (!combo.empty() && tsl::hlp::comboStringToKeys(combo) == tsl::hlp::comboStringToKeys(keyCombo)) {
                                combo.clear();
                                modified = true;
                            }
                        }
                        
                        if (modified) {
                            newComboStr = "(" + joinIniList(comboList) + ")";
                            overlaySection["mode_combos"] = newComboStr;
                            overlaysModified = true;
                        }
                    }
                }
            }
            
            // Write back if modified, then clear memory
            if (overlaysModified) {
                saveIniFileData(OVERLAYS_INI_FILEPATH, overlaysIniData);
            }
            // overlaysIniData automatically cleared when scope ends
        }
        
        // Process packages second (overlays INI data is already cleared)
        {
            auto packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
            bool packagesModified = false;
            
            const auto packageNames = getPackageNames(); // Get all package names
            
            for (const auto& packageName : packageNames) {
                auto packageIt = packagesIniData.find(packageName);
                if (packageIt == packagesIniData.end()) continue; // Skip if package not in INI
                
                auto& packageSection = packageIt->second;
                auto keyComboIt = packageSection.find(KEY_COMBO_STR);
                if (keyComboIt != packageSection.end()) {
                    existingCombo = keyComboIt->second; // Reusing the same variable
                    if (!existingCombo.empty() && tsl::hlp::comboStringToKeys(existingCombo) == tsl::hlp::comboStringToKeys(keyCombo)) {
                        packageSection[KEY_COMBO_STR] = "";
                        packagesModified = true;
                    }
                }
            }
            
            // Write back if modified, then clear memory
            if (packagesModified) {
                saveIniFileData(PACKAGES_INI_FILEPATH, packagesIniData);
            }
            // packagesIniData automatically cleared when scope ends
        }
    }

    void handleSelection(tsl::elm::List* list, const std::vector<std::string>& items, const std::string& defaultItem, const std::string& iniKey, const std::string& targetMenu) {
        //tsl::elm::ListItem* listItem;
        std::string mappedItem;
        for (const auto& item : items) {
            //auto mappedItem = convertComboToUnicode(item); // moved to ListItem class in libTesla
            //if (mappedItem.empty()) mappedItem = item;
            mappedItem = item;
            if (targetMenu == KEY_COMBO_STR)
                convertComboToUnicode(mappedItem);
    
            tsl::elm::ListItem* listItem = new tsl::elm::ListItem(mappedItem);
            if (item == defaultItem) {
                listItem->setValue(CHECKMARK_SYMBOL);
                //lastSelectedListItem = nullptr;
                lastSelectedListItem = listItem;
            }
            listItem->setClickListener([this, item, mappedItem, defaultItem, iniKey, targetMenu, listItem](uint64_t keys) {
                if (runningInterpreter.load(acquire))
                    return false;
                
                if (simulatedSelect.exchange(false, acq_rel)) {
                    keys |= KEY_A;
                }
                if (keys & KEY_A) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, item);
                    
                    if (targetMenu == KEY_COMBO_STR) {
                        // Also set it in tesla config
                        setIniFileValue(TESLA_CONFIG_INI_PATH, TESLA_STR, iniKey, item);

                        // Remove this key combo from any overlays using it (both key_combo and mode_combos)
                        this->removeKeyComboFromAllOthers(item);
                            
                        // Reload the overlay key combos to reflect changes
                        tsl::hlp::loadEntryKeyCombos();
                    }
                    
                    reloadMenu = true;
                    
                    if (lastSelectedListItem)
                        lastSelectedListItem->setValue("");
                    if (selectedListItem)
                        selectedListItem->setValue(mappedItem);

                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                    shiftItemFocus(listItem);
                    if (lastSelectedListItem)
                        lastSelectedListItem->triggerClickAnimation();
                    
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
        }
    }


    void addUpdateButton(tsl::elm::List* list, const std::string& title, const std::string& downloadUrl, const std::string& targetPath, const std::string& movePath, const std::string& versionLabel) {
        auto* listItem = new tsl::elm::ListItem(title);
        listItem->setValue(versionLabel, true);
        if (isVersionGreaterOrEqual(versionLabel.c_str(), APP_VERSION) && versionLabel != APP_VERSION)
            listItem->setValueColor(tsl::onTextColor);

        listItem->setClickListener([listItem, title, downloadUrl, targetPath, movePath](uint64_t keys) {
            static bool executingCommands = false;
            if (runningInterpreter.load(acquire)) {
                return false;
            } else {
                if (executingCommands && commandSuccess && movePath != LANG_PATH) {
                    triggerMenuReload = true;
                }
                executingCommands = false;
            }

            if (simulatedSelect.exchange(false, acq_rel)) {
                keys |= KEY_A;
            }
            std::vector<std::vector<std::string>> interpreterCommands;
            if (keys & KEY_A) {
                executingCommands = true;
                isDownloadCommand = true;
                const bool disableLoaderUpdate = isFile(FLAGS_PATH+"NO_LOADER_UPDATES.flag");
                if (title == UPDATE_ULTRAHAND) {
                    const std::string versionLabel = cleanVersionLabel(parseValueFromIniSection((SETTINGS_PATH+"RELEASE.ini"), "Release Info", "latest_version"));
                    std::string loaderUrl, loaderPlusUrl;
                    if (isVersionGreaterOrEqual(amsVersion,"1.8.0")) {
                        loaderUrl = NX_OVLLOADER_ZIP_URL;
                        loaderPlusUrl = NX_OVLLOADER_PLUS_ZIP_URL;
                    } else {
                        loaderUrl = OLD_NX_OVLLOADER_ZIP_URL;
                        loaderPlusUrl = OLD_NX_OVLLOADER_PLUS_ZIP_URL;
                    }

                    // Build base commands that are always needed
                    interpreterCommands = {
                        {"try:"},
                        {"delete", targetPath},
                        {"download", UPDATER_PAYLOAD_URL, PAYLOADS_PATH},
                        {"download", INCLUDED_THEME_FOLDER_URL + "ultra.ini", THEMES_PATH},
                        {"download", INCLUDED_THEME_FOLDER_URL + "ultra-blue.ini", THEMES_PATH}
                    };
                    
                    // Conditionally add loader downloads only if not disabled
                    if (!disableLoaderUpdate) {
                        interpreterCommands.push_back({"download", loaderUrl, EXPANSION_PATH});
                        interpreterCommands.push_back({"download", loaderPlusUrl, EXPANSION_PATH});
                    }
                    
                    // Add the main download
                    interpreterCommands.push_back({"download", downloadUrl, DOWNLOADS_PATH});
                    
                    // Conditionally add version label
                    if (!versionLabel.empty()) {
                        interpreterCommands.push_back({"set-json-val", HB_APPSTORE_JSON, "version", versionLabel});
                    }
                } else {
                    interpreterCommands = {
                        {"try:"},
                        {"delete", targetPath},
                        {"download", downloadUrl, DOWNLOADS_PATH}
                    };
                }
                
                if (movePath == LANG_PATH) { // for language update commands
                    interpreterCommands.push_back({"unzip", targetPath, movePath});
                } else {
                    //interpreterCommands.push_back({"download", INCLUDED_THEME_URL, THEMES_PATH});
                    interpreterCommands.push_back({"move", targetPath, movePath});
                    if (!disableLoaderUpdate)
                        interpreterCommands.push_back({"unzip", EXPANSION_PATH + loaderTitle + ".zip", ROOT_PATH});
                }
                
                interpreterCommands.push_back({"delete", targetPath});


                runningInterpreter.store(true, release);
                executeInterpreterCommands(std::move(interpreterCommands), "", "");
                //startInterpreterThread();

                listItem->setValue(INPROGRESS_SYMBOL);
                //lastSelectedListItem = nullptr;
                lastSelectedListItem = listItem;
                shiftItemFocus(listItem);
                lastRunningInterpreter = true;
                if (lastSelectedListItem)
                    lastSelectedListItem->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem);
    }

    // Helper function to create toggle list items
    auto createToggleListItem(tsl::elm::List* list, const std::string& title, bool& state, const std::string& iniKey, const bool invertLogic = false, const bool useReloadMenu2 = false, const bool isMini = true) {
        auto* toggleListItem = new tsl::elm::ToggleListItem(title, invertLogic ? !state : state, ON, OFF, isMini);
        toggleListItem->setStateChangedListener([&, listItem = toggleListItem, iniKey, invertLogic, useReloadMenu2](bool newState) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItem, tsl::FocusDirection::None);
            
            // Calculate the actual logical state first
            const bool actualState = invertLogic ? !newState : newState;
            
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, actualState ? TRUE_STR : FALSE_STR);

            static bool firstState = actualState;

            if (iniKey == "clean_version_labels" || iniKey == "hide_overlay_versions" || iniKey == "hide_package_versions" || iniKey == "highlight_versions" || iniKey == "highlight_titles" || iniKey == "page_swap" || iniKey == "hide_hidden") {
                
                if (iniKey == "page_swap")
                    triggerMenuReload = firstState != state;
                reloadMenu = true;
            } else if (iniKey == "memory_expansion") {
                if (!isFile(EXPANSION_PATH + "nx-ovlloader.zip")) {
                    if (isVersionGreaterOrEqual(amsVersion,"1.8.0"))
                        downloadFile(NX_OVLLOADER_ZIP_URL, EXPANSION_PATH);
                    else
                        downloadFile(OLD_NX_OVLLOADER_ZIP_URL, EXPANSION_PATH);
                    downloadPercentage.store(-1, release);
                }
                if (!isFile(EXPANSION_PATH + "nx-ovlloader+.zip")) {
                    if (isVersionGreaterOrEqual(amsVersion,"1.8.0"))
                        downloadFile(NX_OVLLOADER_PLUS_ZIP_URL, EXPANSION_PATH);
                    else
                        downloadFile(OLD_NX_OVLLOADER_PLUS_ZIP_URL, EXPANSION_PATH);
                    downloadPercentage.store(-1, release);
                }
                if (!isFileOrDirectory(EXPANSION_PATH + "nx-ovlloader.zip") || !isFileOrDirectory(EXPANSION_PATH + "nx-ovlloader+.zip")) {
                    listItem->setState(loaderTitle == "nx-ovlloader+");
                } else {
                    executeCommands({
                        {"try:"},
                        {"del", EXPANSION_PATH + (actualState ? "nx-ovlloader+/" : "nx-ovlloader/")},
                        {"unzip", EXPANSION_PATH + (actualState ? "nx-ovlloader+.zip" : "nx-ovlloader.zip"),
                         EXPANSION_PATH + (actualState ? "nx-ovlloader+/" : "nx-ovlloader/")},
                        {"mv", EXPANSION_PATH + (actualState ? "nx-ovlloader+/" : "nx-ovlloader/"), "/"}
                    });
                }
            //} else if (iniKey == "hide_clock" || iniKey == "hide_soc_temp" || iniKey == "hide_pcb_temp" ||
            //    iniKey == "hide_battery" || iniKey == "hide_widget_backdrop" || iniKey == "dynamic_widget_colors" ||
            //    iniKey == "center_widget_alignment" || iniKey == "extended_widget_backdrop") {
            //    reinitializeWidgetVars();
            } else if (iniKey == "right_alignment") {
                triggerMenuReload2 = firstState != state;
            //} else if (iniKey == "dynamic_logo") {
            //    useDynamicLogo = !useDynamicLogo;
            //} else if (iniKey == "launch_combos") {
            //    useLaunchCombos = !useLaunchCombos;
            //} else if (iniKey == "opaque_screenshots") {
            //    useOpaqueScreenshots = !useOpaqueScreenshots;
            }

            state = !state;
            
            if (useReloadMenu2) reloadMenu2 = true;
        });
        list->addItem(toggleListItem);
    }
        

    std::vector<std::string> filesList;
    
public:
    UltrahandSettingsMenu(const std::string& selection = "") : dropdownSelection(selection) {
        lastSelectedListItemFooter = "";
    }

    ~UltrahandSettingsMenu() {
        lastSelectedListItemFooter = "";
    }

    virtual tsl::elm::Element* createUI() override {
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();
        
        const std::vector<std::string> defaultLanguagesRepresentation = {ENGLISH, SPANISH, FRENCH, GERMAN, JAPANESE, KOREAN, ITALIAN, DUTCH, PORTUGUESE, RUSSIAN, UKRAINIAN, POLISH, SIMPLIFIED_CHINESE, TRADITIONAL_CHINESE};
        static const std::vector<std::string> defaultLanguages = {"en", "es", "fr", "de", "ja", "ko", "it", "nl", "pt", "ru", "uk", "pl", "zh-cn", "zh-tw"};
        
        auto* list = new tsl::elm::List();
        
        if (dropdownSelection.empty()) {
            addHeader(list, MAIN_SETTINGS);
            
            // Load INI once and extract all values
            auto ultrahandIniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto sectionIt = ultrahandIniData.find(ULTRAHAND_PROJECT_NAME);
            
            // Extract all values with defaults
            std::string defaultLang = "";
            std::string keyCombo = "";
            std::string currentTheme = "";
            std::string currentWallpaper = "";
            
            if (sectionIt != ultrahandIniData.end()) {
                auto langIt = sectionIt->second.find(DEFAULT_LANG_STR);
                if (langIt != sectionIt->second.end()) {
                    defaultLang = langIt->second;
                }
                
                auto comboIt = sectionIt->second.find(KEY_COMBO_STR);
                if (comboIt != sectionIt->second.end()) {
                    keyCombo = comboIt->second;
                }
                
                auto themeIt = sectionIt->second.find("current_theme");
                if (themeIt != sectionIt->second.end()) {
                    currentTheme = themeIt->second;
                }
                
                if (expandedMemory) {
                    auto wallpaperIt = sectionIt->second.find("current_wallpaper");
                    if (wallpaperIt != sectionIt->second.end()) {
                        currentWallpaper = wallpaperIt->second;
                    }
                }
            }
            
            // Apply defaults and processing
            trim(keyCombo);
            defaultLang = defaultLang.empty() ? "en" : defaultLang;
            keyCombo = keyCombo.empty() ? defaultCombos[0] : keyCombo;
            convertComboToUnicode(keyCombo);
            currentTheme = (currentTheme.empty() || currentTheme == DEFAULT_STR) ? DEFAULT : currentTheme;
            if (expandedMemory) {
                currentWallpaper = (currentWallpaper.empty() || currentWallpaper == OPTION_SYMBOL) ? OPTION_SYMBOL : currentWallpaper;
            }
            
            addListItem(list, KEY_COMBO, keyCombo, KEY_COMBO_STR);
            addListItem(list, LANGUAGE, defaultLang, "languageMenu");
            addListItem(list, SYSTEM, DROPDOWN_SYMBOL, "systemMenu");
            addListItem(list, SOFTWARE_UPDATE, DROPDOWN_SYMBOL, "softwareUpdateMenu");
            addHeader(list, UI_SETTINGS);
            addListItem(list, THEME, currentTheme, "themeMenu");
            if (expandedMemory) {
                addListItem(list, WALLPAPER, currentWallpaper, "wallpaperMenu");
            }
            addListItem(list, WIDGET, DROPDOWN_SYMBOL, "widgetMenu");
            addListItem(list, MISCELLANEOUS, DROPDOWN_SYMBOL, "miscMenu");
        } else if (dropdownSelection == KEY_COMBO_STR) {
            addHeader(list, KEY_COMBO);
            std::string defaultCombo = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR);
            trim(defaultCombo);
            handleSelection(list, defaultCombos, defaultCombo, KEY_COMBO_STR, KEY_COMBO_STR);
        } else if (dropdownSelection == "languageMenu") {
            addHeader(list, LANGUAGE);
            const std::string defaulLang = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR);
            size_t index = 0;
            std::string langFile;
            //tsl::elm::ListItem* listItem;

            for (const auto& defaultLangMode : defaultLanguages) {
                langFile = LANG_PATH + defaultLangMode + ".json";
                if (defaultLangMode != "en" && !isFileOrDirectory(langFile))  {index++; continue;}

                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(defaultLanguagesRepresentation[index]);

                listItem->setValue(defaultLangMode);
                if (defaultLangMode == defaulLang) {
                    lastSelectedListItemFooter = defaultLangMode;
                    listItem->setValue(CHECKMARK_SYMBOL);
                    //lastSelectedListItem = nullptr;
                    lastSelectedListItem = listItem;
                }
                listItem->setClickListener([skipLang = !isFileOrDirectory(langFile), defaultLangMode, defaulLang, langFile, listItem](uint64_t keys) {
                    //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem, [](auto*){})](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedSelect.exchange(false, acq_rel)) {
                        keys |= KEY_A;
                    }
                    if (keys & KEY_A) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR, defaultLangMode);
                        reloadMenu = reloadMenu2 = true;
                        parseLanguage(langFile);
                        if (skipLang && defaultLangMode == "en") reinitializeLangVars();
                        if (lastSelectedListItem)
                            lastSelectedListItem->setValue(lastSelectedListItemFooter);
                        if (selectedListItem)
                            selectedListItem->setValue(defaultLangMode);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        //lastSelectedListItem = nullptr;
                        lastSelectedListItem = listItem;
                        shiftItemFocus(listItem);
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        lastSelectedListItemFooter = defaultLangMode;
                        languageWasChanged = true;
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
                index++;
            }
        } else if (dropdownSelection == "softwareUpdateMenu") {
            const std::string fullVersionLabel = cleanVersionLabel(parseValueFromIniSection((SETTINGS_PATH+"RELEASE.ini"), "Release Info", "latest_version"));

            addHeader(list, SOFTWARE_UPDATE);
            addUpdateButton(list, UPDATE_ULTRAHAND, ULTRAHAND_REPO_URL + "releases/latest/download/ovlmenu.ovl", "/config/ultrahand/downloads/ovlmenu.ovl", "/switch/.overlays/ovlmenu.ovl", fullVersionLabel);
            addUpdateButton(list, UPDATE_LANGUAGES, ULTRAHAND_REPO_URL + "releases/latest/download/lang.zip", "/config/ultrahand/downloads/lang.zip", LANG_PATH, fullVersionLabel);

            PackageHeader overlayHeader;
            overlayHeader.title = "Ultrahand Overlay";
            overlayHeader.version = APP_VERSION;
            overlayHeader.creator = "ppkantorski";
            overlayHeader.about = "Ultrahand Overlay is a versatile tool that enables you to create and share custom command-based packages.";
            overlayHeader.credits = "Special thanks to B3711, ComplexNarrative, ssky, MasaGratoR, meha, WerWolv, HookedBehemoth and many others. ♥";
            addPackageInfo(list, overlayHeader, OVERLAY_STR);
            overlayHeader.clear();

        } else if (dropdownSelection == "systemMenu") {
            
            // Version info formatting with a reduced buffer
            char versionString[32];  // Reduced buffer size to 32
            snprintf(versionString, sizeof(versionString), "HOS %sAMS %s", 
                     hosVersion, amsVersion);
            
            const std::string hekateVersion = extractVersionFromBinary("sdmc:/bootloader/update.bin");
            
            addHeader(list, DEVICE_INFO);
            
            SetSysProductModel model = SetSysProductModel_Invalid;
            setsysGetProductModel(&model);
            
            const char* modelRev;
            switch (model) {
                case SetSysProductModel_Iowa: modelRev = "IowaTegra X1+ (Mariko)"; break;
                case SetSysProductModel_Hoag: modelRev = "HoagTegra X1+ (Mariko)"; break;
                case SetSysProductModel_Calcio: modelRev = "CalcioTegra X1+ (Mariko)"; break;
                case SetSysProductModel_Aula: modelRev = "AulaTegra X1+ (Mariko)"; break;
                case SetSysProductModel_Nx: modelRev = "IcosaTegra X1 (Erista)"; break;
                case SetSysProductModel_Copper: modelRev = "CopperTegra X1 (Erista)"; break;
                default: modelRev = UNAVAILABLE_SELECTION.c_str(); break;
            }
            ASSERT_FATAL(nifmInitialize(NifmServiceType_User)); // for local IP
            std::vector<std::vector<std::string>> tableData = {
                {FIRMWARE, "", versionString},
                {BOOTLOADER, "", hekateVersion.empty() ? "fusee" : "hekate " + hekateVersion},
                {LOCAL_IP, "", getLocalIpAddress()}
            };
            nifmExit();
            addTable(list, tableData, "", 163, 20, 28, 4);
            
            // Hardware and storage info
            tableData = {
                {HARDWARE, "", modelRev},
                {MEMORY, "", memorySize},
                {"└ " + VENDOR, "", memoryVendor},
                {"└ " + MODEL, "", memoryModel},
                {STORAGE, "", usingEmunand ? "emuMMC" : "sysMMC"},
                {"└ eMMC ", "", getStorageInfo("emmc")},
                {"└ SD Card", "", getStorageInfo("sdmc")}
            };
            addTable(list, tableData, "", 163, 20, 30, 4);
            
            // CPU, GPU, and SOC info
            tableData = {
                {"", "", "CPU      GPU      SOC"}
            };
            addTable(list, tableData, "", 162, 9, 3, 0, DEFAULT_STR, "section", "section", RIGHT_STR, true);
            
            tableData.clear();
            tableData.resize(2);
            
            if (cpuSpeedo0 != 0 && cpuSpeedo2 != 0 && socSpeedo0 != 0 && cpuIDDQ != 0 && gpuIDDQ != 0 && socIDDQ != 0) {
                tableData[0] = {
                    "Speedo", "",
                    customAlign(cpuSpeedo0) + "  " + customAlign(cpuSpeedo2) + "  " + customAlign(socSpeedo0)
                };
                tableData[1] = {
                    "IDDQ", "",
                    customAlign(cpuIDDQ) + "  " + customAlign(gpuIDDQ) + "  " + customAlign(socIDDQ)
                };
            } else {
                tableData[0] = {"Speedo", "", "⋯        ⋯        ⋯  "};
                tableData[1] = {"IDDQ", "", "⋯        ⋯        ⋯  "};
            }
            addTable(list, tableData, "", 163, 20, -2, 4);
            
            // The part that was moved to the end
            addHeader(list, COMMANDS);
            
            // Get system memory info and format it
            u64 RAM_Used_system_u, RAM_Total_system_u;
            svcGetSystemInfo(&RAM_Used_system_u, 1, INVALID_HANDLE, 2);
            svcGetSystemInfo(&RAM_Total_system_u, 0, INVALID_HANDLE, 2);
            
            // Calculate free RAM and store in a smaller buffer
            char ramString[24];  // Reduced buffer size to 24
            float freeRamMB = (static_cast<float>(RAM_Total_system_u - RAM_Used_system_u) / (1024.0f * 1024.0f));
            snprintf(ramString, sizeof(ramString), "%.2f MB %s", freeRamMB, FREE.c_str());

            //std::string ramColor;
            //if (freeRamMB >= 9.0f){
            //    ramColor = "healthy_ram"; // Green: R=0, G=15, B=0
            //} else if (freeRamMB >= 3.0f) {
            //    ramColor = "neutral_ram"; // Orange-ish: R=15, G=10, B=0 → roughly RGB888: 255, 170, 0
            //} else {
            //    ramColor = "bad_ram"; // Red: R=15, G=0, B=0
            //}
            const std::string ramColor = freeRamMB >= 9.0f ? "healthy_ram" : freeRamMB >= 3.0f ? "neutral_ram" : "bad_ram";
            
            // Reuse tableData with minimal reallocation
            tableData = {
                {NOTICE, "", UTILIZES + " 2 MB (" + ramString + ")"}
            };
            addTable(list, tableData, "", 163, 8, 7, 0, DEFAULT_STR, DEFAULT_STR, ramColor, RIGHT_STR, true);
            // Memory expansion toggle
            useMemoryExpansion = (ult::expandedMemory || 
                                  parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "memory_expansion") == TRUE_STR);
            createToggleListItem(list, MEMORY_EXPANSION, useMemoryExpansion, "memory_expansion", false, true, false);

            // Reboot required info
            tableData = {
                {"", "", REBOOT_REQUIRED}  // Direct reuse without reallocation
            };
            addTable(list, tableData, "", 163, 28, 0, 0, DEFAULT_STR, DEFAULT_STR, DEFAULT_STR, RIGHT_STR, true);
        
        } else if (dropdownSelection == "themeMenu") {
            addHeader(list, THEME);
            std::string currentTheme = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme");
            currentTheme = currentTheme.empty() ? DEFAULT_STR : currentTheme;
            auto* listItem = new tsl::elm::ListItem(DEFAULT);
            if (currentTheme == DEFAULT_STR) {
                listItem->setValue(CHECKMARK_SYMBOL);
                //lastSelectedListItem = nullptr;
                lastSelectedListItem = listItem;
            }
            listItem->setClickListener([defaultTheme = THEMES_PATH + "default.ini", listItem](uint64_t keys) {
                if (runningInterpreter.load(acquire)) return false;
                if (simulatedSelect.exchange(false, acq_rel)) {
                    keys |= KEY_A;
                }
                if (keys & KEY_A) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", DEFAULT_STR);
                    deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                    if (isFileOrDirectory(defaultTheme)) {
                        copyFileOrDirectory(defaultTheme, THEME_CONFIG_INI_PATH);
                        copyPercentage.store(-1, release);
                    }
                    else initializeTheme();
                    tsl::initializeThemeVars();
                    reloadMenu = reloadMenu2 = true;
                    if (lastSelectedListItem)
                        lastSelectedListItem->setValue("");
                    if (selectedListItem)
                        selectedListItem->setValue(DEFAULT);
                    listItem->setValue(CHECKMARK_SYMBOL);
                    //lastSelectedListItem = nullptr;
                    lastSelectedListItem = listItem;
                    shiftItemFocus(listItem);
                    if (lastSelectedListItem)
                        lastSelectedListItem->triggerClickAnimation();
                    themeWasChanged = true;
                    return true;
                }
                return false;
            });
            list->addItem(listItem);

            filesList = getFilesListByWildcards(THEMES_PATH + "*.ini");
            std::sort(filesList.begin(), filesList.end());

            std::string themeName;
            for (const auto& themeFile : filesList) {
                themeName = getNameFromPath(themeFile);
                dropExtension(themeName);
                if (themeName == DEFAULT_STR) continue;

                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(themeName);
                if (themeName == currentTheme) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    //lastSelectedListItem = nullptr;
                    lastSelectedListItem = listItem;
                }
                listItem->setClickListener([themeName, themeFile, listItem](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedSelect.exchange(false, acq_rel)) {
                        keys |= KEY_A;
                    }
                    if (keys & KEY_A) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", themeName);
                        //deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                        copyFileOrDirectory(themeFile, THEME_CONFIG_INI_PATH);
                        copyPercentage.store(-1, release);
                        initializeTheme();
                        tsl::initializeThemeVars();
                        reloadMenu = reloadMenu2 = true;
                        if (lastSelectedListItem)
                            lastSelectedListItem->setValue("");
                        if (selectedListItem)
                            selectedListItem->setValue(themeName);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        //lastSelectedListItem = nullptr;
                        lastSelectedListItem = listItem;
                        shiftItemFocus(listItem);
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        themeWasChanged = true;
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
            }
        } else if (dropdownSelection == "wallpaperMenu") {
            addHeader(list, WALLPAPER);
            std::string currentWallpaper = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper");
            currentWallpaper = currentWallpaper.empty() ? OPTION_SYMBOL : currentWallpaper;

            auto* listItem = new tsl::elm::ListItem(OPTION_SYMBOL);
            if (currentWallpaper == OPTION_SYMBOL) {
                listItem->setValue(CHECKMARK_SYMBOL);
                //lastSelectedListItem = nullptr;
                lastSelectedListItem = listItem;
            }

            listItem->setClickListener([listItem](uint64_t keys) {
                if (runningInterpreter.load(acquire)) return false;
                if (simulatedSelect.exchange(false, acq_rel)) {
                    keys |= KEY_A;
                }
                if (keys & KEY_A) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper", "");
                    deleteFileOrDirectory(WALLPAPER_PATH);
                    reloadWallpaper();
                    //reloadMenu = reloadMenu2 = true;
                    if (lastSelectedListItem)
                        lastSelectedListItem->setValue("");
                    if (selectedListItem)
                        selectedListItem->setValue(OPTION_SYMBOL);
                    listItem->setValue(CHECKMARK_SYMBOL);
                    //lastSelectedListItem = nullptr;
                    lastSelectedListItem = listItem;
                    shiftItemFocus(listItem);
                    if (lastSelectedListItem)
                        lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem);

            filesList = getFilesListByWildcards(WALLPAPERS_PATH + "*.rgba");
            std::sort(filesList.begin(), filesList.end());

            std::string wallpaperName;
            for (const auto& wallpaperFile : filesList) {
                wallpaperName = getNameFromPath(wallpaperFile);
                dropExtension(wallpaperName);
                if (wallpaperName == DEFAULT_STR) continue;

                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(wallpaperName);
                if (wallpaperName == currentWallpaper) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    //lastSelectedListItem = nullptr;
                    lastSelectedListItem = listItem;
                }
                listItem->setClickListener([wallpaperName, wallpaperFile, listItem](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedSelect.exchange(false, acq_rel)) {
                        keys |= KEY_A;
                    }
                    if (keys & KEY_A) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper", wallpaperName);
                        //deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                        copyFileOrDirectory(wallpaperFile, WALLPAPER_PATH);
                        copyPercentage.store(-1, release);
                        reloadWallpaper();
                        if (lastSelectedListItem)
                            lastSelectedListItem->setValue("");
                        if (selectedListItem)
                            selectedListItem->setValue(wallpaperName);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        //lastSelectedListItem = nullptr;
                        lastSelectedListItem = listItem;
                        shiftItemFocus(listItem);
                        if (selectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
            }
        } else if (dropdownSelection == "widgetMenu") {
            addHeader(list, WIDGET_ITEMS);
            createToggleListItem(list, CLOCK, hideClock, "hide_clock", true);
            createToggleListItem(list, SOC_TEMPERATURE, hideSOCTemp, "hide_soc_temp", true);
            createToggleListItem(list, PCB_TEMPERATURE, hidePCBTemp, "hide_pcb_temp", true);
            createToggleListItem(list, BATTERY, hideBattery, "hide_battery", true);
            createToggleListItem(list, BACKDROP, hideWidgetBackdrop, "hide_widget_backdrop", true);

            addHeader(list, WIDGET_SETTINGS);
            createToggleListItem(list, DYNAMIC_COLORS, dynamicWidgetColors, "dynamic_widget_colors", false);
            createToggleListItem(list, CENTER_ALIGNMENT, centerWidgetAlignment, "center_widget_alignment", false);
            createToggleListItem(list, EXTENDED_BACKDROP, extendedWidgetBackdrop, "extended_widget_backdrop", true);

        } else if (dropdownSelection == "miscMenu") {
            // Load INI section once instead of 14 separate file reads
            auto ultrahandSection = getKeyValuePairsFromSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME);
            
            // Helper lambda to safely get boolean values
            auto getBoolValue = [&](const std::string& key, bool defaultValue = false) -> bool {
                auto it = ultrahandSection.find(key);
                return (it != ultrahandSection.end()) ? (it->second == TRUE_STR) : defaultValue;
            };
            
            addHeader(list, FEATURES);
            useLaunchCombos = getBoolValue("launch_combos", true); // TRUE_STR default
            createToggleListItem(list, LAUNCH_COMBOS, useLaunchCombos, "launch_combos");
            useOpaqueScreenshots = getBoolValue("opaque_screenshots", true); // TRUE_STR default
            createToggleListItem(list, OPAQUE_SCREENSHOTS, useOpaqueScreenshots, "opaque_screenshots");
            useSwipeToOpen = getBoolValue("swipe_to_open", true); // TRUE_STR default
            createToggleListItem(list, SWIPE_TO_OPEN, useSwipeToOpen, "swipe_to_open");
            addHeader(list, "Menu Settings");
            
            hideUserGuide = getBoolValue("hide_user_guide", false); // FALSE_STR default
            createToggleListItem(list, USER_GUIDE, hideUserGuide, "hide_user_guide", true);
            useDynamicLogo = getBoolValue("dynamic_logo", true); // TRUE_STR default
            createToggleListItem(list, DYNAMIC_LOGO, useDynamicLogo, "dynamic_logo");
            hideHidden = getBoolValue("hide_hidden", false); // FALSE_STR default
            createToggleListItem(list, SHOW_HIDDEN, hideHidden, "hide_hidden", true);
            usePageSwap = getBoolValue("page_swap", false); // FALSE_STR default
            createToggleListItem(list, PAGE_SWAP, usePageSwap, "page_swap", false);
            rightAlignmentState = useRightAlignment = getBoolValue("right_alignment", false); // FALSE_STR default
            createToggleListItem(list, RIGHT_SIDE_MODE, useRightAlignment, "right_alignment");
            addHeader(list, "libultrahand Detection");
            highlightTitles = getBoolValue("highlight_titles", false); // FALSE_STR default
            createToggleListItem(list, "Highlight Titles", highlightTitles, "highlight_titles", false);
            highlightVersions = getBoolValue("highlight_versions", true); // TRUE_STR default
            createToggleListItem(list, "Highlight Versions", highlightVersions, "highlight_versions", false);
            
            highlightPackages = getBoolValue("highlight_packages", true); // TRUE_STR default
            createToggleListItem(list, "Highlight Packages", highlightPackages, "highlight_packages", false);
            addHeader(list, "Version Labels");
            hideOverlayVersions = getBoolValue("hide_overlay_versions", false); // FALSE_STR default
            createToggleListItem(list, OVERLAY_VERSIONS, hideOverlayVersions, "hide_overlay_versions", true);
            
            hidePackageVersions = getBoolValue("hide_package_versions", false); // FALSE_STR default
            createToggleListItem(list, PACKAGE_VERSIONS, hidePackageVersions, "hide_package_versions", true);
            
            cleanVersionLabels = getBoolValue("clean_version_labels", false); // FALSE_STR default
            createToggleListItem(list, CLEAN_VERSIONS, cleanVersionLabels, "clean_version_labels", false, true);

        } else {
            addBasicListItem(list, FAILED_TO_OPEN + ": " + settingsIniPath);
        }

        auto* rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        if (inSubSettingsMenu && ((dropdownSelection == "languageMenu") || (dropdownSelection == KEY_COMBO_STR) || (dropdownSelection == "themeMenu") || (dropdownSelection == "wallpaperMenu"))) {
            jumpItemName = "";
            jumpItemValue = "";
            jumpItemExactMatch = true;
            g_overlayFilename = "";
            list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        } else {
            if (languageWasChanged) {
                jumpItemName = LANGUAGE;
                jumpItemValue = "";
                jumpItemExactMatch = true;
                g_overlayFilename = "";
                languageWasChanged = false;
                list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
            } else if (themeWasChanged) {
                jumpItemName = THEME;
                jumpItemValue = "";
                jumpItemExactMatch = true;
                g_overlayFilename = "";
                themeWasChanged = false;
                list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
            } else {
                jumpItemName = "";
                jumpItemValue = "";
                jumpItemExactMatch = true;
                g_overlayFilename = "";
                list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
            }
        }

        rootFrame->setContent(list);
        return rootFrame;

        //return returnRootFrame(list, CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        const bool isRunningInterp = runningInterpreter.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
        
        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            if (lastSelectedListItem)
                lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }
        
        if (goBackAfter) {
            goBackAfter = false;
            simulatedBack.exchange(true, acq_rel);
            return true;
        }
        
        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                simulatedNextPage.exchange(false, acq_rel);
                simulatedMenu.exchange(false, acq_rel);
                
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
                
                const bool isTouching = stillTouching.load(acquire);
                const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
                
                if (backKeyPressed) {
                    allowSlide.exchange(false, acq_rel);
                    unlockedSlide.exchange(false, acq_rel);
                    inSettingsMenu = false;
                    returningToMain = (lastMenu != "hiddenMenuMode");
                    returningToHiddenMain = !returningToMain;
                    lastMenu = "settingsMenu";
                    
                    if (reloadMenu) {
                        tsl::pop(2);
                        tsl::changeTo<MainMenu>(lastMenuMode);
                        reloadMenu = false;
                    } else {
                        tsl::goBack();
                    }
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {
            simulatedNextPage.exchange(false, acq_rel);
            
            //if (simulatedBack.exchange(false, acq_rel)) {
            //    keysDown |= KEY_B;
            //}
            
            const bool isTouching = stillTouching.load(acquire);
            const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
            
            if (backKeyPressed) {
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
                inSubSettingsMenu = false;
                returningToSettings = true;
                
                if (reloadMenu2) {
                    tsl::goBack(2);
                    tsl::changeTo<UltrahandSettingsMenu>();
                    reloadMenu2 = false;
                } else {
                    tsl::goBack();
                }
                return true;
            }
        }
        
        if (returningToSettings && !(keysDown & KEY_B)) {
            returningToSettings = false;
            inSettingsMenu = true;
            tsl::impl::parseOverlaySettings();
        }
        
        if (triggerExit.load(acquire)) {
            triggerExit.store(false, release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        return false;
    }
};



int settingsMenuPageDepth = 0;
std::string rootEntryName, rootEntryMode, rootTitle, rootVersion;

class SettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, title, version, dropdownSelection, settingsIniPath;
   
    bool isInSection, inQuotes, isFromMainMenu;
    int MAX_PRIORITY = 20;

    std::string modeTitle;

public:
    SettingsMenu(const std::string& name, const std::string& mode, const std::string& title = "", const std::string& version = "", const std::string& selection = "")
        : entryName(name), entryMode(mode), title(title), version(version), dropdownSelection(selection) {
            // Only store once
            if (settingsMenuPageDepth == 0) {
               rootEntryName = name;
               rootEntryMode = mode;
               rootTitle = title;
               rootVersion = version;
            }
            settingsMenuPageDepth++;
        }

    ~SettingsMenu() {
        if (settingsMenuPageDepth > 0) {
            settingsMenuPageDepth--;
        }
    }

    void createAndAddToggleListItem(
        tsl::elm::List* list,
        const std::string& label,
        bool initialState,
        const std::string& iniKey,
        std::string currentValue,
        const std::string& settingsIniPath,
        const std::string& entryName,
        bool handleReload = false
    ) {
        if (currentValue.empty() && !initialState) currentValue = FALSE_STR;

        auto* toggleListItem = new tsl::elm::ToggleListItem(label, initialState, ON, OFF);
        toggleListItem->setState(currentValue != FALSE_STR);
        toggleListItem->setStateChangedListener([this, iniKey, listItem = toggleListItem, handleReload](bool state) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItem, tsl::FocusDirection::None);
            setIniFileValue(this->settingsIniPath, this->entryName, iniKey, state ? TRUE_STR : FALSE_STR);
            if (handleReload) {
                reloadMenu = state || (reloadMenu2 = !state);
            }
        });
        list->addItem(toggleListItem);
    }

    void createAndAddListItem(
        tsl::elm::List* list,
        const std::string& iStr,
        const std::string& priorityValue,
        const std::string& settingsIniPath,
        const std::string& entryName,
        bool isMini = false
    ) {
        auto* listItem = new tsl::elm::ListItem(iStr, "", isMini);

        if (iStr == priorityValue) {
            listItem->setValue(CHECKMARK_SYMBOL);
            lastSelectedListItem = listItem;
        }

        listItem->setClickListener([settingsIniPath=settingsIniPath, entryName=entryName, iStr, priorityValue, listItem](uint64_t keys) {
            if (runningInterpreter.load(acquire)) return false;

            if (simulatedSelect.exchange(false, acq_rel)) {
                keys |= KEY_A;
            }

            if (keys & KEY_A) {
                if (iStr != priorityValue) {
                    reloadMenu = true; // Modify the global variable
                } else {
                    reloadMenu = false;
                }
    
                setIniFileValue(settingsIniPath, entryName, PRIORITY_STR, iStr);
                if (lastSelectedListItem)
                    lastSelectedListItem->setValue("");
                if (selectedListItem)
                    selectedListItem->setValue(iStr);
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = listItem;
                shiftItemFocus(listItem);
                if (lastSelectedListItem)
                    lastSelectedListItem->triggerClickAnimation();
            }
            return false;
        });

        list->addItem(listItem);
    }
    
    virtual tsl::elm::Element* createUI() override {
        settingsIniPath = (entryMode == OVERLAY_STR) ? OVERLAYS_INI_FILEPATH : PACKAGES_INI_FILEPATH;
        const std::string header = title + (!version.empty() ? (" " + version) : "");
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();
        
        // Load settings INI once for the entire method
        auto settingsIniData = getParsedDataFromIniFile(settingsIniPath);
        auto sectionIt = settingsIniData.find(entryName);
        
        // Helper lambda to safely get values from loaded settings data
        auto getSettingsValue = [&](const std::string& key) -> std::string {
            return (sectionIt != settingsIniData.end() && sectionIt->second.count(key)) 
                ? sectionIt->second.at(key) : "";
        };
        
        auto* list = new tsl::elm::List();
    
        if (inSettingsMenu) {
            addHeader(list, SETTINGS + "  " + header);
    
            {
                const std::string currentKeyCombo = getSettingsValue(KEY_COMBO_STR);
                std::string displayCombo = currentKeyCombo.empty() ? OPTION_SYMBOL : currentKeyCombo;
                
                // Convert combo to unicode for display
                if (!currentKeyCombo.empty()) {
                    convertComboToUnicode(displayCombo);
                }
                
                auto* keyComboItem = new tsl::elm::ListItem(KEY_COMBO);
                keyComboItem->setValue(displayCombo);
                keyComboItem->setClickListener([entryName=entryName, entryMode=entryMode, overlayName=title, overlayVersion=version, listItem = keyComboItem](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedSelect.exchange(false, acq_rel)) {
                        keys |= KEY_A;
                    }
                    if (keys & KEY_A) {
                        inMainMenu = false;
                        tsl::changeTo<SettingsMenu>(entryName, entryMode, overlayName, overlayVersion, KEY_COMBO_STR);
                        selectedListItem = listItem;
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(keyComboItem);
            }
    
            createAndAddToggleListItem(
                list,
                (entryMode == OVERLAY_STR) ? HIDE_OVERLAY : HIDE_PACKAGE,
                false,
                HIDE_STR,
                getSettingsValue(HIDE_STR),
                settingsIniPath,
                entryName,
                true
            );
    
            auto* listItem = new tsl::elm::ListItem(SORT_PRIORITY);
            listItem->setValue(getSettingsValue(PRIORITY_STR));
            listItem->setClickListener([entryName=entryName, entryMode=entryMode, overlayName=title, overlayVersion=version, listItem](uint64_t keys) {
                if (runningInterpreter.load(acquire)) return false;
                if (simulatedSelect.exchange(false, acq_rel)) {
                    keys |= KEY_A;
                }
                if (keys & KEY_A) {
                    inMainMenu = false;
                    tsl::changeTo<SettingsMenu>(entryName, entryMode, overlayName, overlayVersion, PRIORITY_STR);
                    selectedListItem = listItem;
                    if (lastSelectedListItem)
                        lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
    
            // Add Key Combo selection for overlays only
            if (entryMode == OVERLAY_STR) {
                createAndAddToggleListItem(
                    list,
                    LAUNCH_ARGUMENTS,
                    false,
                    USE_LAUNCH_ARGS_STR,
                    getSettingsValue(USE_LAUNCH_ARGS_STR),
                    settingsIniPath,
                    entryName
                );
    
                const std::string argStr = getSettingsValue("mode_args");
                const std::string comboStr = getSettingsValue("mode_combos");
                const std::string labelStr = getSettingsValue("mode_labels");
                
                const std::vector<std::string> modeList = splitIniList(argStr);   // (-mini, -micro)
                const std::vector<std::string> comboList = splitIniList(comboStr);     // (ZL+ZR+DRIGHT, '')
                const std::vector<std::string> labelList = splitIniList(labelStr);     // (Mini Mode, Micro Mode)
                
                if (!modeList.empty()) {
    
                    std::vector<std::vector<std::string>> tableData = {
                        {"", "", ""}  // Direct reuse without reallocation
                    };
                    addTable(list, tableData, "", 163, 0, 19-5, 0, "header", "header", DEFAULT_STR, RIGHT_STR, true, false, false, true, "none", false);
                    tableData = {
                        {MODE, "", KEY_COMBO}  // Direct reuse without reallocation
                    };
                    addTable(list, tableData, "", 165, 19-2, 19-2, 0, "header", "header", DEFAULT_STR, RIGHT_STR, true, true, false, true, "none", false);
                    
                    // Use already loaded comboStr instead of re-reading
                    std::vector<std::string> comboListMutable = splitIniList(comboStr);
                    // Ensure comboList is same size as modeList, fill with empty if missing
                    if (comboListMutable.size() < modeList.size())
                        comboListMutable.resize(modeList.size(), "");
                    
                    std::string mode, displayName, comboDisplay;
                    for (size_t i = 0; i < modeList.size(); ++i) {
                        mode = modeList[i];
                        displayName = (i < labelList.size() && !labelList[i].empty()) ? labelList[i] : mode;
                        const std::string& combo = comboListMutable[i];  // reference so we can modify
    
                        // Display combo or OPTION_SYMBOL if empty
                        comboDisplay = combo.empty() ? OPTION_SYMBOL : combo;
                        convertComboToUnicode(comboDisplay);
    
                        auto* item = new tsl::elm::ListItem(displayName);
                        item->setValue(comboDisplay);
    
                        // Capture by pointer/ref so we can update comboList[i]
                        item->setClickListener([entryName = entryName, settingsIniPath = settingsIniPath, i, mode, &comboListMutable, item, this](uint64_t keys) mutable {
                            if (runningInterpreter.load(acquire)) return false;
                            if (simulatedSelect.exchange(false, acq_rel)) {
                                keys |= KEY_A;
                            }
                            if (keys & KEY_A) {
                                inMainMenu = false;
    
                                // Open a new SettingsMenu specifically for editing this mode combo
                                tsl::changeTo<SettingsMenu>(
                                    entryName,
                                    OVERLAY_STR,
                                    mode,
                                    "",
                                    "mode_combo_" + std::to_string(i)
                                );
                                selectedListItem = item;
                                if (lastSelectedListItem)
                                    lastSelectedListItem->triggerClickAnimation();
                                return true;
                            }
                            return false;
                        });
    
                        list->addItem(item);
                    }
                }
            } else if (entryMode == PACKAGE_STR) {
                createAndAddToggleListItem(
                    list,
                    BOOT_COMMANDS,
                    true,
                    USE_BOOT_PACKAGE_STR,
                    getSettingsValue(USE_BOOT_PACKAGE_STR),
                    settingsIniPath,
                    entryName
                );
                createAndAddToggleListItem(
                    list,
                    EXIT_COMMANDS,
                    true,
                    USE_EXIT_PACKAGE_STR,
                    getSettingsValue(USE_EXIT_PACKAGE_STR),
                    settingsIniPath,
                    entryName
                );
                createAndAddToggleListItem(
                    list,
                    "Quick Launch",
                    false,
                    USE_QUICK_LAUNCH_STR,
                    getSettingsValue(USE_QUICK_LAUNCH_STR),
                    settingsIniPath,
                    entryName
                );
                createAndAddToggleListItem(
                    list,
                    ERROR_LOGGING,
                    false,
                    USE_LOGGING_STR,
                    getSettingsValue(USE_LOGGING_STR),
                    settingsIniPath,
                    entryName
                );
            }
        } else if (dropdownSelection == PRIORITY_STR) {
            addHeader(list, SORT_PRIORITY);
            const std::string priorityValue = getSettingsValue(PRIORITY_STR);
            for (int i = 0; i <= MAX_PRIORITY; ++i) {
                createAndAddListItem(
                    list,
                    ult::to_string(i),
                    priorityValue,
                    settingsIniPath,
                    entryName,
                    true
                );
            }
        } else if (dropdownSelection == KEY_COMBO_STR) {
            addHeader(list, KEY_COMBO);
            const std::string currentKeyCombo = getSettingsValue(KEY_COMBO_STR);
            
            // Load ultrahand config INI for global default combo
            auto ultrahandConfigData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto ultrahandSectionIt = ultrahandConfigData.find(ULTRAHAND_PROJECT_NAME);
            std::string globalDefaultCombo = (ultrahandSectionIt != ultrahandConfigData.end() && ultrahandSectionIt->second.count(KEY_COMBO_STR))
                ? ultrahandSectionIt->second.at(KEY_COMBO_STR) : "";
            trim(globalDefaultCombo);
            
            // Add "No Key Combo" option first
            auto* listItem = new tsl::elm::ListItem(OPTION_SYMBOL);
            if (currentKeyCombo.empty()) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = listItem;
            }
            listItem->setClickListener([entryName=entryName, settingsIniPath=settingsIniPath, listItem](uint64_t keys) {
                if (runningInterpreter.load(acquire)) return false;
                if (simulatedSelect.exchange(false, acq_rel)) {
                    keys |= KEY_A;
                }
                if (keys & KEY_A) {
                    // Remove key combo from this overlay
                    setIniFileValue(settingsIniPath, entryName, KEY_COMBO_STR, "");
                    tsl::hlp::loadEntryKeyCombos(); // reload combos
                    reloadMenu2 = true;
                    
                    if (lastSelectedListItem)
                        lastSelectedListItem->setValue("");
                    if (selectedListItem)
                        selectedListItem->setValue(OPTION_SYMBOL);
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                    shiftItemFocus(listItem);
                    if (lastSelectedListItem)
                        lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
    
            // Add predefined key combos
            std::string mappedCombo;
            for (const auto& combo : defaultCombos) {
                if (combo == globalDefaultCombo) {
                    continue; // Skip the global default combo
                }
                
    
                mappedCombo = combo;
                convertComboToUnicode(mappedCombo);
                
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(mappedCombo);
                if (combo == currentKeyCombo) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = listItem;
                }
                listItem->setClickListener([entryName=entryName, settingsIniPath=settingsIniPath, combo, mappedCombo, currentKeyCombo, listItem](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedSelect.exchange(false, acq_rel)) {
                        keys |= KEY_A;
                    }
                    if (keys & KEY_A) {
                        if (combo != currentKeyCombo) {
                            // Remove this key combo from any other overlays first
                            removeKeyComboFromOthers(combo, entryName);
                            // Set the new key combo for this overlay
                            setIniFileValue(settingsIniPath, entryName, KEY_COMBO_STR, combo);
                            //reloadMenu = true;
                            tsl::hlp::loadEntryKeyCombos(); // reload combos
                        }
                        reloadMenu2 = true;
                        if (lastSelectedListItem)
                            lastSelectedListItem->setValue("");
                        if (selectedListItem)
                            selectedListItem->setValue(mappedCombo);
                        listItem->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = listItem;
                        shiftItemFocus(listItem);
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
            }
        } else if (dropdownSelection.rfind("mode_combo_", 0) == 0) {  // starts with "mode_combo_"
            // Extract index from dropdownSelection suffix
            const std::string indexStr = dropdownSelection.substr(std::string("mode_combo_").length());
            const size_t idx = std::stoi(indexStr);
            
            // Read labels and use label if available, otherwise fall back to title with quotes
            const std::string labelStr = getSettingsValue("mode_labels");
            const std::vector<std::string> labelList = splitIniList(labelStr);
            const std::string labelText = (idx < labelList.size() && !labelList[idx].empty()) ? 
                                   labelList[idx] : 
                                   "'" + title + "'";
            if ((idx < labelList.size() && !labelList[idx].empty()))
                modeTitle = labelList[idx];
            else
                modeTitle = title;
            const std::string headerText = KEY_COMBO + "  " + labelText;
            //labelList.clear();
            addHeader(list, headerText);
    
        
            // Read current combo list and ensure it's large enough
            std::string comboStr = getSettingsValue("mode_combos");
            std::vector<std::string> comboList = splitIniList(comboStr);
            if (idx >= comboList.size()) comboList.resize(idx + 1, "");
        
            std::string currentCombo = comboList[idx];
        
            // Load ultrahand config INI for global default combo
            auto ultrahandConfigData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto ultrahandSectionIt = ultrahandConfigData.find(ULTRAHAND_PROJECT_NAME);
            std::string globalDefaultCombo = (ultrahandSectionIt != ultrahandConfigData.end() && ultrahandSectionIt->second.count(KEY_COMBO_STR))
                ? ultrahandSectionIt->second.at(KEY_COMBO_STR) : "";
            trim(globalDefaultCombo);
        
            // Add "No Combo" option
            auto* noComboItem = new tsl::elm::ListItem(OPTION_SYMBOL);
            if (currentCombo.empty()) {
                noComboItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = noComboItem;
            }
            noComboItem->setClickListener([entryName=entryName, settingsIniPath=settingsIniPath, idx, comboList, noComboItem](uint64_t keys) mutable {
                if (runningInterpreter.load(acquire)) return false;
                if (simulatedSelect.exchange(false, acq_rel)) keys |= KEY_A;
                if (keys & KEY_A) {
                    comboList[idx] = "";
                    const std::string newComboStr = "(" + joinIniList(comboList) + ")";
                    removeKeyComboFromOthers(newComboStr, entryName);
                    setIniFileValue(settingsIniPath, entryName, "mode_combos", newComboStr);
                    tsl::hlp::loadEntryKeyCombos(); // reload combos
                    reloadMenu2 = true;
                    if (lastSelectedListItem)
                        lastSelectedListItem->setValue("");
                    if (selectedListItem)
                        selectedListItem->setValue(OPTION_SYMBOL);
                    noComboItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = noComboItem;
                    shiftItemFocus(noComboItem);
                    if (lastSelectedListItem)
                        lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(noComboItem);
        
            // Add valid combos
            std::string mappedCombo;
    
            for (const auto& combo : defaultCombos) {
                if (combo == globalDefaultCombo) continue; // Skip global default combo
        
                mappedCombo = combo;
                convertComboToUnicode(mappedCombo);
        
                auto* comboItem = new tsl::elm::ListItem(mappedCombo);
                if (combo == currentCombo) {
                    comboItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = comboItem;
                }
        
                comboItem->setClickListener([entryName=entryName, settingsIniPath=settingsIniPath, idx, combo, comboList, mappedCombo, comboItem](uint64_t keys) mutable {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedSelect.exchange(false, acq_rel)) keys |= KEY_A;
                    if (keys & KEY_A) {
                        if (combo != comboList[idx]) {
                            // Remove same combo from other overlays (both key_combo + mode_combos)
                            removeKeyComboFromOthers(combo, entryName);
                            
                            // Re-read the combo list after cleaning to get the updated values
                            std::string comboStr = parseValueFromIniSection(settingsIniPath, entryName, "mode_combos");
                            comboList = splitIniList(comboStr);
                            if (idx >= comboList.size()) comboList.resize(idx + 1, "");
                            
                            comboList[idx] = combo;
                            std::string newComboStr = "(" + joinIniList(comboList) + ")";
                            setIniFileValue(settingsIniPath, entryName, "mode_combos", newComboStr);
                            tsl::hlp::loadEntryKeyCombos(); // reload combos
                        }
                        
                        reloadMenu2 = true;
                        if (lastSelectedListItem)
                            lastSelectedListItem->setValue("");
                        if (selectedListItem)
                            selectedListItem->setValue(mappedCombo);
                        comboItem->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = comboItem;
                        shiftItemFocus(comboItem);
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
        
                list->addItem(comboItem);
            }
        } else {
            addBasicListItem(list, FAILED_TO_OPEN + ": " + settingsIniPath);
        }
    
        //auto* rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        //rootFrame->setContent(list);
        //return rootFrame.release();
    
        auto* rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        if (inSubSettingsMenu &&
            (dropdownSelection == KEY_COMBO_STR ||
             dropdownSelection == PRIORITY_STR ||
             dropdownSelection.rfind("mode_combo_", 0) == 0)) {
    
            jumpItemName = "";
            jumpItemValue = "";
            jumpItemExactMatch = true;
            g_overlayFilename = "";
        }
        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
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
        
        const bool isRunningInterp = runningInterpreter.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
        
        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            if (lastSelectedListItem)
                lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }
        
        if (goBackAfter) {
            goBackAfter = false;
            simulatedBack.exchange(true, acq_rel);
            return true;
        }


        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                simulatedNextPage.exchange(false, acq_rel);
                simulatedMenu.exchange(false, acq_rel);
    
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
                const bool isTouching = stillTouching.load(acquire);
                const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
    
                // Note: Original code uses !stillTouching without .load() - preserving this exactly
                if (backKeyPressed) {
                    allowSlide.exchange(false, acq_rel);
                    unlockedSlide.exchange(false, acq_rel);
                    inSettingsMenu = false;
                    
                    // Determine return destination
                    if (lastMenu != "hiddenMenuMode")
                        returningToMain = true;
                    else
                        returningToHiddenMain = true;
                    
                    if (reloadMenu) {
                        reloadMenu = false;
                        
                        // Determine pop count and hidden mode settings
                        int popCount;
                        if (lastMenu == "hiddenMenuMode") {
                            popCount = 3;
                            inMainMenu = false;
                            inHiddenMode = true;
                            if (entryMode == OVERLAY_STR)
                                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, TRUE_STR);
                            else
                                popCount = 2;
                        } else {
                            popCount = 2;
                        }
                        
                        tsl::pop(popCount);
                        jumpItemName = rootTitle;
                        jumpItemValue = rootVersion;
                        g_overlayFilename = "";
                        jumpItemExactMatch = false;
                        skipJumpReset = true;
                        
                        tsl::changeTo<MainMenu>(lastMenuMode);
                    } else {
                        tsl::goBack();
                    }
                    
                    lastMenu = "settingsMenu";
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {
            simulatedNextPage.exchange(false, acq_rel);
            simulatedMenu.exchange(false, acq_rel);
        
            //if (simulatedBack.exchange(false, acq_rel)) {
            //    keysDown |= KEY_B;
            //}
        
            // Note: Original code uses stillTouching.load() here - preserving this difference
            const bool isTouching = stillTouching.load(acquire);
            const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
            
            
            if (backKeyPressed) {
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
                inSubSettingsMenu = false;
                returningToSettings = true;
    
                // Step 1: Go back one menu level
                // Step 2: If reload is needed, change to SettingsMenu with focus
                if (reloadMenu2) {
                    reloadMenu2 = false;
                    tsl::goBack(2);
    
                    // Provide jump target context
                    jumpItemName = modeTitle;
                    jumpItemValue = "";
                    jumpItemExactMatch = true;
                    g_overlayFilename = "";
    
                    tsl::changeTo<SettingsMenu>(
                        rootEntryName,
                        rootEntryMode,
                        rootTitle,
                        rootVersion
                    );
                } else {
                    tsl::goBack();
                }
    
                return true;
            }
        }
        
        if (returningToSettings && !(keysDown & KEY_B)) {
            returningToSettings = false;
            inSettingsMenu = true;
        }
    
        if (triggerExit.load(acquire)) {
            triggerExit.store(false, release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        return false;
    }
};



// For persistent versions and colors across nested packages (when not specified)
std::string packageRootLayerTitle;
std::string packageRootLayerVersion;
std::string packageRootLayerColor;
bool packageRootLayerIsStarred = false;

//std::string lastPackageHeader;

bool overrideTitle = false, overrideVersion = false;





class ScriptOverlay : public tsl::Gui {
private:
    std::vector<std::vector<std::string>> commands;
    std::string filePath, specificKey;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false, isFromPackage = false, isFromSelectionMenu = false;
    bool tableMode = false;

    std::string lastPackageHeader;

    void addListItem(tsl::elm::List* list, const std::string& line) {
        auto* listItem = new tsl::elm::ListItem(line);

        listItem->setClickListener([filePath=filePath, specificKey=specificKey, listItem, line](uint64_t keys) {
            if (runningInterpreter.load(acquire)) return false;
            if (simulatedSelect.exchange(false, acq_rel)) {
                keys |= KEY_A;
            }
            if (keys & KEY_A) {
                std::vector<std::vector<std::string>> commandVec;
                std::vector<std::string> commandParts;
                std::string currentPart;
                bool inQuotes = false;

                for (char ch : line) {
                    if (ch == '\'') {
                        inQuotes = !inQuotes;
                        if (!inQuotes) {
                            commandParts.emplace_back(std::move(currentPart));
                            currentPart.clear();
                            //currentPart.shrink_to_fit();
                        }
                    } else if (ch == ' ' && !inQuotes) {
                        if (!currentPart.empty()) {
                            commandParts.emplace_back(std::move(currentPart));
                            currentPart.clear();
                            //currentPart.shrink_to_fit();
                        }
                    } else {
                        currentPart += ch;
                    }
                }
                if (!currentPart.empty()) {
                    commandParts.emplace_back(std::move(currentPart));
                }

                commandVec.emplace_back(std::move(commandParts));

                executeInterpreterCommands(std::move(commandVec), filePath, specificKey);
                //startInterpreterThread();
                listItem->setValue(INPROGRESS_SYMBOL);

                lastSelectedListItem = listItem;
               
                lastRunningInterpreter = true;
                listItem->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem);
    }

public:
    ScriptOverlay(std::vector<std::vector<std::string>>&& cmds, const std::string& file, const std::string& key = "", const std::string& fromMenu = "", bool tableMode = false, const std::string& _lastPackageHeader = "")
        : commands(cmds), filePath(file), specificKey(key), tableMode(tableMode), lastPackageHeader(_lastPackageHeader) {
            isFromMainMenu = (fromMenu == "main");
            isFromPackage = (fromMenu == "package");
            isFromSelectionMenu = (fromMenu == "selection");
        }

    virtual tsl::elm::Element* createUI() override {
        inScriptMenu = true;
        std::string packageName = getNameFromPath(filePath);
        if (packageName == ".packages") packageName = ROOT_PACKAGE;
        else if (!packageRootLayerTitle.empty()) packageName = packageRootLayerTitle;
        auto* list = new tsl::elm::List();
        
        
        bool noClickableItems = false;
        if (!tableMode) {
            size_t index = 0, tryCount = 0;
            std::string combinedCommand;
            // If not in table mode, loop through commands and display each command as a list item
            for (auto& command : commands) {
                if (index == 0 && command[0] != "try:" && command[0] != "on:" && command[0] != "off:") {
                    addHeader(list, specificKey);
                }
                if (command[0] == "try:") {
                    tryCount++;
                    index++;
                    addHeader(list, specificKey+""+"Try"+" #"+ult::to_string(tryCount));
                    continue;
                }
                if (command[0] == "on:") {
                    index++;
                    addHeader(list, specificKey+""+ON);
                    continue;
                }
                if (command[0] == "off:") {
                    index++;
                    addHeader(list, specificKey+""+OFF);
                    continue;
                }
                combinedCommand = joinCommands(command); // Join commands into a single line for display
                addListItem(list, combinedCommand);
                index++;
                command.clear();
                command.shrink_to_fit();
            }
        } else {

            noClickableItems = true;
            std::vector<std::string> sectionLines;  // Holds the sections (commands)
            std::vector<std::string> infoLines;     // Holds the info (empty in this case)
            // Table mode: Collect command data for the table
            std::string sectionLine;

            std::string packageSourcePath;

            for (auto& command : commands) {
                if (command.size() > 1 && command[0] == "package_source") {
                    packageSourcePath = command[1];
                    preprocessPath(packageSourcePath, filePath);
                }
                // Each command will be treated as a section with no corresponding info
                sectionLine = joinCommands(command);  // Combine command parts into a section line
                sectionLines.push_back(sectionLine);              // Add to section lines
                infoLines.push_back("");                          // Empty info line

                command.clear();
                command.shrink_to_fit();
            }
            
            // Use default parameters for the table view
            const size_t tableColumnOffset = 163;
            const size_t tableStartGap = 20;
            const size_t tableEndGap = 9;
            const size_t tableSpacing = 10;
            const std::string tableSectionTextColor = DEFAULT_STR;
            const std::string tableInfoTextColor = DEFAULT_STR;
            const std::string tableAlignment = LEFT_STR;
            const bool hideTableBackground = false;
            const bool useHeaderIndent = false;
            const bool isPolling = false;
            const bool isScrollableTable = true;
            const std::string wrappingMode = "char";
            const bool useWrappedTextIndent = true;

            static std::vector<std::vector<std::string>> dummyTableData;


            //addDummyListItem(list);
            addHeader(list, specificKey);

            addDummyListItem(list);
            // Draw the table using the sectionLines and empty infoLines
            drawTable(list, dummyTableData, sectionLines, infoLines, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
                      tableSectionTextColor, tableInfoTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isPolling, isScrollableTable, wrappingMode, useWrappedTextIndent);

            if (!packageSourcePath.empty()) {

                std::vector<std::string> sourceCommands = readListFromFile(packageSourcePath);
                sectionLines.clear();
                //sectionLines.shrink_to_fit();
                infoLines.clear();
                //infoLines.shrink_to_fit();
                sectionLine = "";
                for (const auto& command : sourceCommands) {
                    // Each command will be treated as a section with no corresponding info
                    //sectionLine = joinCommands(command);  // Combine command parts into a section line
                    sectionLines.push_back(command);              // Add to section lines
                    infoLines.push_back("");                          // Empty info line
                }

                std::string packageSourceName = getNameFromPath(packageSourcePath);

                addHeader(list, packageSourceName);
                drawTable(list, dummyTableData, sectionLines, infoLines, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
                          tableSectionTextColor, tableInfoTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isPolling, isScrollableTable, wrappingMode, useWrappedTextIndent);
            }
        }

        std::string packageVersion = isFromMainMenu ? "" : packageRootLayerVersion;


        auto* rootFrame = new tsl::elm::OverlayFrame(packageName,
           !lastPackageHeader.empty() ? lastPackageHeader + "?Ultrahand Script" : (packageVersion.empty() ? CAPITAL_ULTRAHAND_PROJECT_NAME + " Script" : packageVersion + "  " + CAPITAL_ULTRAHAND_PROJECT_NAME + " Script"),
           noClickableItems);
        list->disableCaching();
        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        const bool isRunningInterp = runningInterpreter.load(acquire);
        
        if (isRunningInterp) return handleRunningInterpreter(keysDown, keysHeld);
        
        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            if (lastSelectedListItem)
                lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }
        
        if (goBackAfter) {
            goBackAfter = false;
            simulatedBack.exchange(true, acq_rel);
            return true;
        }
        
        if (inScriptMenu) {
            simulatedNextPage.exchange(false, acq_rel);
            simulatedMenu.exchange(false, acq_rel);
            
            //if (simulatedBack.exchange(false, acq_rel)) {
            //    keysDown |= KEY_B;
            //}
            
            const bool isTouching = stillTouching.load(acquire);
            const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
            
            if (backKeyPressed) {
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
                inScriptMenu = false;
                
                // Handle return destination logic
                if (isFromPackage) {
                    returningToPackage = lastMenu == "packageMenu";
                    returningToSubPackage = lastMenu == "subPackageMenu";
                }
                else if (isFromSelectionMenu) {
                    returningToSelectionMenu = isFromSelectionMenu;
                }
                else if (isFromMainMenu) {
                    returningToMain = isFromMainMenu;
                }
                
                tsl::goBack();
                return true;
            }
        }
        
        if (triggerExit.load(acquire)) {
            triggerExit.store(false, release);
            tsl::setNextOverlay(OVERLAY_PATH + "ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        return false;
    }

private:
    std::string joinCommands(const std::vector<std::string>& commandParts) {
        std::string combinedCommand;
    
        // Check if this is a section header (e.g., "[*Boot Entry]" or "[hekate]")
        if (!commandParts.empty() && commandParts.front().front() == '[' && commandParts.back().back() == ']') {
            for (const auto& part : commandParts) {
                combinedCommand += part + " ";  // Simply join the parts with spaces
            }
            return combinedCommand.substr(0, combinedCommand.size() - 1); // Return joined section header
        }
    
        // Regular command processing
        std::string argument;
        for (const auto& part : commandParts) {
            argument = part;

            // If the argument is exactly '', skip processing (preserve the empty quotes)
            if (argument == "") {
                argument = "''";
                //continue;  // Do nothing, preserve as is
            }

            // If the argument already has quotes, skip it
            if ((argument.front() == '"' && argument.back() == '"') ||
                (argument.front() == '\'' && argument.back() == '\'')) {
                combinedCommand += argument + " ";
                continue; // Already quoted, skip
            }
    
            // If the argument contains no spaces, do not add quotes
            if (argument.find(' ') == std::string::npos) {
                combinedCommand += argument + " ";
                continue;  // No spaces, so no need for quotes
            }
    
            // If the argument contains a single quote, wrap it in double quotes
            if (argument.find('\'') != std::string::npos) {
                combinedCommand += "\"" + argument + "\" ";
            }
            // If the argument contains a double quote, wrap it in single quotes
            else if (argument.find('"') != std::string::npos) {
                combinedCommand += "\'" + argument + "\' ";
            }
            // If the argument contains spaces but no quotes, wrap it in single quotes by default
            else {
                combinedCommand += "\'" + argument + "\' ";
            }
        }
    
        return combinedCommand.substr(0, combinedCommand.size() - 1); // Remove trailing space
    }


};


// Set to globals for reduced lambda overhead

/**
 * @brief The `SelectionOverlay` class manages the selection overlay functionality.
 *
 * This class handles the selection overlay, allowing users to interact with and select various options.
 * It provides functions for creating the graphical user interface (GUI), handling user input, and executing commands.
 */
class SelectionOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, groupingName, lastGroupingName;
    std::string specifiedFooterKey;
    bool toggleState = false;
    std::string packageConfigIniPath;
    std::string commandSystem, commandMode, commandGrouping;

    std::string lastPackageHeader;

    // Variables moved from createUI to class scope
    std::vector<std::string> filesList, filesListOn, filesListOff;
    std::vector<std::string> filterList, filterListOn, filterListOff;
    std::string sourceType, sourceTypeOn, sourceTypeOff;
    std::string jsonPath, jsonPathOn, jsonPathOff;
    std::string jsonKey, jsonKeyOn, jsonKeyOff;
    std::string listPath, listPathOn, listPathOff;
    std::string iniPath, iniPathOn, iniPathOff;
    std::string listString, listStringOn, listStringOff;
    std::string jsonString, jsonStringOn, jsonStringOff;
    std::vector<std::string> selectedItemsList;
    std::string hexPath;

    std::vector<std::vector<std::string>> selectionCommands = {};
    std::vector<std::vector<std::string>> selectionCommandsOn = {};
    std::vector<std::vector<std::string>> selectionCommandsOff = {};
    std::string lastSelectedListItemFooter2 = "";
    
    std::unordered_map<int, int> toggleCount;
    std::unordered_map<int, bool> currentPatternIsOriginal; 
    // For handling on/off file_source toggle states
    std::unordered_map<int, std::string> currentSelectedItems;
    std::unordered_map<int, bool> isInitialized;



    bool isMini = false;

    size_t maxItemsLimit = 200;     // 0 = uncapped, any other value = max size
    
    // Helper function to apply size limit to any vector
    void applyItemsLimit(std::vector<std::string>& vec) {
        if (maxItemsLimit == 0 || vec.size() <= maxItemsLimit) return;
        vec.resize(maxItemsLimit);
        //vec.shrink_to_fit();
    }

public:
    SelectionOverlay(const std::string& path, const std::string& key, const std::string& footerKey, const std::string& _lastPackageHeader, const std::vector<std::vector<std::string>>& commands)
        : filePath(path), specificKey(key), specifiedFooterKey(footerKey), lastPackageHeader(_lastPackageHeader), selectionCommands(commands) {
        lastSelectedListItemFooter2 = "";
        lastSelectedListItem = nullptr;
        //selectedFooterDict.clear();
        tsl::clearGlyphCacheNow.store(true, release);
    }

    ~SelectionOverlay() {
        lastSelectedListItemFooter2 = "";
        lastSelectedListItem = nullptr;
        //selectedFooterDict.clear();

        //selectionCommands.clear();
        //selectionCommands.shrink_to_fit();
        //selectionCommandsOn.clear();
        //selectionCommandsOn.shrink_to_fit();
        //selectionCommandsOff.clear();
        //selectionCommandsOff.shrink_to_fit();
        //currentSelectedItems.clear();
        //isInitialized.clear();
        //toggleCount.clear();
        //currentPatternIsOriginal.clear();
        tsl::clearGlyphCacheNow.store(true, release);
    }
        
    void processSelectionCommands() {
        if (ult::expandedMemory) maxItemsLimit = 0; // uncapped for loader+
    
        removeEmptyCommands(selectionCommands);
    
        bool inEristaSection = false;
        bool inMarikoSection = false;
        std::string currentSection = GLOBAL_STR;
        std::string iniFilePath;
        
        
        // Use string_view for read-only operations to avoid copying
        std::string commandName;
        std::string filterEntry;
        std::vector<std::string> matchedFiles, tempFiles;
    
        // Pre-cache pattern lengths for better performance
        const size_t SYSTEM_PATTERN_LEN = SYSTEM_PATTERN.length();
        const size_t MODE_PATTERN_LEN = MODE_PATTERN.length();
        const size_t GROUPING_PATTERN_LEN = GROUPING_PATTERN.length();
        const size_t SELECTION_MINI_PATTERN_LEN = SELECTION_MINI_PATTERN.length();
    
        updateGeneralPlaceholders();
        
        for (auto& cmd : selectionCommands) {
            // Apply placeholder replacements in-place
            for (auto& arg : cmd) {
                replacePlaceholdersInArg(arg, generalPlaceholders);
            }
    
            commandName = cmd[0]; // Now assigns to string_view - no copy
    
            // Keep original case-insensitive logic
            if (stringToLowercase(commandName) == "erista:") {
                inEristaSection = true;
                inMarikoSection = false;
                continue;
            } else if (stringToLowercase(commandName) == "mariko:") {
                inEristaSection = false;
                inMarikoSection = true;
                continue;
            }
    
            if ((inEristaSection && !inMarikoSection && usingErista) || 
                (!inEristaSection && inMarikoSection && usingMariko) || 
                (!inEristaSection && !inMarikoSection)) {
                
                // Optimized pattern matching with bounds checking
                if (commandName.size() > SYSTEM_PATTERN_LEN && 
                    commandName.compare(0, SYSTEM_PATTERN_LEN, SYSTEM_PATTERN) == 0) {
                    commandSystem.assign(commandName, SYSTEM_PATTERN_LEN, commandName.size() - SYSTEM_PATTERN_LEN);
                    if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                        commandSystem = commandSystems[0];
                } else if (commandName.size() > MODE_PATTERN_LEN && 
                           commandName.compare(0, MODE_PATTERN_LEN, MODE_PATTERN) == 0) {
                    commandMode = commandName.substr(MODE_PATTERN_LEN);
                    if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end())
                        commandMode = commandModes[0];
                } else if (commandName.size() > GROUPING_PATTERN_LEN && 
                           commandName.compare(0, GROUPING_PATTERN_LEN, GROUPING_PATTERN) == 0) {
                    commandGrouping = commandName.substr(GROUPING_PATTERN_LEN);
                    if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                        commandGrouping = commandGroupings[0];
                } else if (commandName.size() > SELECTION_MINI_PATTERN_LEN && 
                           commandName.compare(0, SELECTION_MINI_PATTERN_LEN, SELECTION_MINI_PATTERN) == 0) {
                    isMini = (commandName.substr(SELECTION_MINI_PATTERN_LEN) == TRUE_STR);
                }
    
                if (commandMode == TOGGLE_STR) {
                    if (commandName == "on:")
                        currentSection = ON_STR;
                    else if (commandName == "off:")
                        currentSection = OFF_STR;
                }
    
                if (cmd.size() > 1) {
                    if (!iniFilePath.empty()) {
                        applyReplaceIniPlaceholder(cmd[1], INI_FILE_STR, iniFilePath);
                    }
    
                    if (commandName == "ini_file") {
                        iniFilePath = cmd[1];
                        preprocessPath(iniFilePath, filePath);
                        continue;
                    } else if (commandName == "filter") {
                        // Avoid copying by directly assigning and then processing
                        filterEntry = std::move(cmd[1]); // Move instead of copy
                        removeQuotes(filterEntry);
                        if (sourceType == FILE_STR) {
                            preprocessPath(filterEntry, filePath);
                        }
    
                        if (filterEntry.find('*') != std::string::npos) {
                            // Get files directly into temporary, avoid intermediate storage
                            //tempFiles.clear();
                            tempFiles = getFilesListByWildcards(filterEntry, maxItemsLimit);
                            if (currentSection == GLOBAL_STR) {
                                filterList.insert(filterList.end(), 
                                                std::make_move_iterator(tempFiles.begin()),
                                                std::make_move_iterator(tempFiles.end()));
                            } else if (currentSection == ON_STR) {
                                filterListOn.insert(filterListOn.end(), 
                                                  std::make_move_iterator(tempFiles.begin()),
                                                  std::make_move_iterator(tempFiles.end()));
                            } else if (currentSection == OFF_STR) {
                                filterListOff.insert(filterListOff.end(), 
                                                   std::make_move_iterator(tempFiles.begin()),
                                                   std::make_move_iterator(tempFiles.end()));
                            }
                            tempFiles.clear();
                            // tempFiles automatically destroyed here, freeing memory
                        } else {
                            if (currentSection == GLOBAL_STR)
                                filterList.push_back(std::move(filterEntry));
                            else if (currentSection == ON_STR)
                                filterListOn.push_back(std::move(filterEntry));
                            else if (currentSection == OFF_STR)
                                filterListOff.push_back(std::move(filterEntry));
                        }
                    } else if (commandName == "file_source") {
                        sourceType = FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            pathPattern = cmd[1];
                            preprocessPath(pathPattern, filePath);
                            // Get files directly, avoid storing in intermediate variable
                            //tempFiles.clear();
                            filesList = getFilesListByWildcards(pathPattern, maxItemsLimit);
                            //filesList.insert(filesList.end(), 
                            //               std::make_move_iterator(tempFiles.begin()),
                            //               std::make_move_iterator(tempFiles.end()));
                            //tempFiles.clear();
                        } else if (currentSection == ON_STR) {
                            pathPatternOn = cmd[1];
                            preprocessPath(pathPatternOn, filePath);
                            //tempFiles.clear();
                            filesListOn = getFilesListByWildcards(pathPatternOn, maxItemsLimit);
                            //filesListOn.insert(filesListOn.end(), 
                            //                 std::make_move_iterator(tempFiles.begin()),
                            //                 std::make_move_iterator(tempFiles.end()));
                            //tempFiles.clear();
                            sourceTypeOn = FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            pathPatternOff = cmd[1];
                            preprocessPath(pathPatternOff, filePath);
                            //tempFiles.clear();
                            filesListOff = getFilesListByWildcards(pathPatternOff, maxItemsLimit);
                            //filesListOff.insert(filesListOff.end(), 
                            //                  std::make_move_iterator(tempFiles.begin()),
                            //                  std::make_move_iterator(tempFiles.end()));
                            //tempFiles.clear();
                            sourceTypeOff = FILE_STR;
                        }
                    } else if (commandName == "json_file_source") {
                        sourceType = JSON_FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            jsonPath = cmd[1];
                            preprocessPath(jsonPath, filePath);
                            if (cmd.size() > 2)
                                jsonKey = cmd[2];
                        } else if (currentSection == ON_STR) {
                            jsonPathOn = cmd[1];
                            preprocessPath(jsonPathOn, filePath);
                            sourceTypeOn = JSON_FILE_STR;
                            if (cmd.size() > 2)
                                jsonKeyOn = cmd[2];
                        } else if (currentSection == OFF_STR) {
                            jsonPathOff = cmd[1];
                            preprocessPath(jsonPathOff, filePath);
                            sourceTypeOff = JSON_FILE_STR;
                            if (cmd.size() > 2)
                                jsonKeyOff = cmd[2];
                        }
                    } else if (commandName == "list_file_source") {
                        sourceType = LIST_FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            listPath = cmd[1];
                            preprocessPath(listPath, filePath);
                        } else if (currentSection == ON_STR) {
                            listPathOn = cmd[1];
                            preprocessPath(listPathOn, filePath);
                            sourceTypeOn = LIST_FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            listPathOff = cmd[1];
                            preprocessPath(listPathOff, filePath);
                            sourceTypeOff = LIST_FILE_STR;
                        }
                    } else if (commandName == "list_source") {
                        sourceType = LIST_STR;
                        if (currentSection == GLOBAL_STR) {
                            listString = cmd[1];
                            removeQuotes(listString);
                        } else if (currentSection == ON_STR) {
                            listStringOn = cmd[1];
                            removeQuotes(listStringOn);
                            sourceTypeOn = LIST_STR;
                        } else if (currentSection == OFF_STR) {
                            listStringOff = cmd[1];
                            removeQuotes(listStringOff);
                            sourceTypeOff = LIST_STR;
                        }
                    } else if (commandName == "ini_file_source") {
                        sourceType = INI_FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            iniPath = cmd[1];
                            preprocessPath(iniPath, filePath);
                        } else if (currentSection == ON_STR) {
                            iniPathOn = cmd[1];
                            preprocessPath(iniPathOn, filePath);
                            sourceTypeOn = INI_FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            iniPathOff = cmd[1];
                            preprocessPath(iniPathOff, filePath);
                            sourceTypeOff = INI_FILE_STR;
                        }
                    } else if (commandName == "json_source") {
                        sourceType = JSON_STR;
                        if (currentSection == GLOBAL_STR) {
                            jsonString = cmd[1];
                            removeQuotes(jsonString);
                            if (cmd.size() > 2) {
                                jsonKey = cmd[2];
                                removeQuotes(jsonKey);
                            }
                        } else if (currentSection == ON_STR) {
                            jsonStringOn = cmd[1];
                            removeQuotes(jsonStringOn);
                            sourceTypeOn = JSON_STR;
                            if (cmd.size() > 2) {
                                jsonKeyOn = cmd[2];
                                removeQuotes(jsonKeyOn);
                            }
                        } else if (currentSection == OFF_STR) {
                            jsonStringOff = cmd[1];
                            removeQuotes(jsonStringOff);
                            sourceTypeOff = JSON_STR;
                            if (cmd.size() > 2) {
                                jsonKeyOff = cmd[2];
                                removeQuotes(jsonKeyOff);
                            }
                        }
                    }
                }

                if (commandMode == TOGGLE_STR) {
                    if (currentSection == GLOBAL_STR) {
                        selectionCommandsOn.push_back(cmd);
                        selectionCommandsOff.push_back(std::move(cmd));
                    } else if (currentSection == ON_STR)
                        selectionCommandsOn.push_back(std::move(cmd));
                    else if (currentSection == OFF_STR)
                        selectionCommandsOff.push_back(std::move(cmd));
                }
                //if (commandMode == TOGGLE_STR)
                //    cmd.clear();
            }
        }
    }

    virtual tsl::elm::Element* createUI() override {
        inSelectionMenu = true;
        
    
        auto* list = new tsl::elm::List();
        packageConfigIniPath = filePath + CONFIG_FILENAME;
    
        commandSystem = commandSystems[0];
        commandMode = commandModes[0];
        commandGrouping = commandGroupings[0];
    
        processSelectionCommands();
    
        std::vector<std::string> selectedItemsListOn, selectedItemsListOff;
        std::string currentPackageHeader;
        std::string itemName;
    
        if (commandMode == DEFAULT_STR || commandMode == OPTION_STR) {
            if (sourceType == FILE_STR)
                selectedItemsList = std::move(filesList);
            else if (sourceType == LIST_STR || sourceType == LIST_FILE_STR)
                selectedItemsList = (sourceType == LIST_STR) ? stringToList(listString) : readListFromFile(listPath, maxItemsLimit);
            else if (sourceType == INI_FILE_STR)
                selectedItemsList = parseSectionsFromIni(iniPath);
            else if (sourceType == JSON_STR || sourceType == JSON_FILE_STR) {
                populateSelectedItemsListFromJson(sourceType, (sourceType == JSON_STR) ? jsonString : jsonPath, jsonKey, selectedItemsList);
                jsonPath.clear();
                //jsonPath.shrink_to_fit();
                jsonString.clear();
                //jsonString.shrink_to_fit();
            }
            applyItemsLimit(selectedItemsList);
            filterItemsList(filterList, selectedItemsList);
            filterList.clear();
            filterList.shrink_to_fit();

        } else if (commandMode == TOGGLE_STR) {
            if (sourceTypeOn == FILE_STR)
                selectedItemsListOn = std::move(filesListOn);
            else if (sourceTypeOn == LIST_STR || sourceTypeOn == LIST_FILE_STR)
                selectedItemsListOn = (sourceTypeOn == LIST_STR) ? stringToList(listStringOn) : readListFromFile(listPathOn, maxItemsLimit);
            else if (sourceTypeOn == INI_FILE_STR)
                selectedItemsListOn = parseSectionsFromIni(iniPathOn);
            else if (sourceTypeOn == JSON_STR || sourceTypeOn == JSON_FILE_STR) {
                populateSelectedItemsListFromJson(sourceTypeOn, (sourceTypeOn == JSON_STR) ? jsonStringOn : jsonPathOn, jsonKeyOn, selectedItemsListOn);
                jsonPathOn.clear();
                //jsonPathOn.shrink_to_fit();
                jsonStringOn.clear();
                //jsonStringOn.shrink_to_fit();
            }
            applyItemsLimit(selectedItemsListOn);

            if (sourceTypeOff == FILE_STR)
                selectedItemsListOff = std::move(filesListOff);
            else if (sourceTypeOff == LIST_STR || sourceTypeOff == LIST_FILE_STR)
                selectedItemsListOff = (sourceTypeOff == LIST_STR) ? stringToList(listStringOff) : readListFromFile(listPathOff, maxItemsLimit);
            else if (sourceTypeOff == INI_FILE_STR)
                selectedItemsListOff = parseSectionsFromIni(iniPathOff);
            else if (sourceTypeOff == JSON_STR || sourceTypeOff == JSON_FILE_STR) {
                populateSelectedItemsListFromJson(sourceTypeOff, (sourceTypeOff == JSON_STR) ? jsonStringOff : jsonPathOff, jsonKeyOff, selectedItemsListOff);
                jsonPathOff.clear();
                //jsonStringOff.shrink_to_fit();
                jsonStringOff.clear();
                //jsonStringOff.shrink_to_fit();
            }
            applyItemsLimit(selectedItemsListOff);
            filterItemsList(filterListOn, selectedItemsListOn);
            filterListOn.clear();
            filterListOn.shrink_to_fit();
            filterItemsList(filterListOff, selectedItemsListOff);
            filterListOff.clear();
            filterListOff.shrink_to_fit();
            //if (sourceType == FILE_STR) {
            //    filterItemsList(filterListOn, selectedItemsListOn);
            //    //filterListOn.clear();
            //    //filterListOn.shrink_to_fit();
            //    
            //    filterItemsList(filterListOff, selectedItemsListOff);
            //    //filterListOff.clear();
            //    //filterListOff.shrink_to_fit();
            //}


            //selectedItemsList.reserve(selectedItemsListOn.size() + selectedItemsListOff.size());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOn.begin(), selectedItemsListOn.end());
            selectedItemsList.insert(selectedItemsList.end(), selectedItemsListOff.begin(), selectedItemsListOff.end());

        }
    
        if (sourceType == FILE_STR) {
            if (commandGrouping == "split2" || commandGrouping == "split4") {
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    const std::string& parentDirA = getParentDirNameFromPath(a);
                    const std::string& parentDirB = getParentDirNameFromPath(b);
                    return (parentDirA != parentDirB) ? (parentDirA < parentDirB) : (getNameFromPath(a) < getNameFromPath(b));
                });
            } else if (commandGrouping == "split5") {
                std::sort(selectedItemsList.begin(), selectedItemsList.end(),
                    [](const std::string& a, const std::string& b) {
                        std::string ga = getParentDirNameFromPath(a);
                        std::string gb = getParentDirNameFromPath(b);
                        removeQuotes(ga);
                        removeQuotes(gb);
            
                        const size_t posA = ga.find(" - ");
                        const size_t posB = gb.find(" - ");
            
                        if (posA != posB) {
                            if (posA == std::string::npos) return true;
                            if (posB == std::string::npos) return false;
                        }
            
                        const int cmp = ga.compare(0, posA, gb, 0, posB);
                        if (cmp != 0) return cmp < 0;
            
                        if (posA == std::string::npos) return false;
                        return ga.compare(posA + 3, std::string::npos, gb, posB + 3, std::string::npos) < 0;
                    });
            } else {
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    return getNameFromPath(a) < getNameFromPath(b);
                });
            }
    
            //filterItemsList(filterList, selectedItemsList);
        }
    
        if (commandGrouping == DEFAULT_STR) {
            std::string cleanSpecificKey = specificKey;
            removeTag(cleanSpecificKey);
            addHeader(list, cleanSpecificKey);
            currentPackageHeader = cleanSpecificKey;
        }
    
        //tsl::elm::ListItem* listItem;
        size_t pos;
        std::string parentDirName;
        std::string footer;
        std::string optionName;
    
        if (selectedItemsList.empty()) {
            if (commandGrouping != DEFAULT_STR) {
                std::string cleanSpecificKey = specificKey;
                removeTag(cleanSpecificKey);
                addHeader(list, cleanSpecificKey);
                currentPackageHeader = cleanSpecificKey;
            }
            tsl::elm::ListItem* listItem = new tsl::elm::ListItem(EMPTY);
            list->addItem(listItem);
        }
    
        std::string tmpSelectedItem;
        const size_t selectedItemsSize = selectedItemsList.size(); // Cache size to avoid repeated calls
    
        for (size_t i = 0; i < selectedItemsSize; ++i) {
            std::string& selectedItem = selectedItemsList[i];
    
            itemName = getNameFromPath(selectedItem);
            if (itemName.front() == '.') // Skip hidden items
                continue;
    
            tmpSelectedItem = selectedItem;
            preprocessPath(tmpSelectedItem, filePath);
            if (!isDirectory(tmpSelectedItem))
                dropExtension(itemName);
    
            if (sourceType == FILE_STR) {
                if (commandGrouping == "split") {
                    groupingName = getParentDirNameFromPath(selectedItem);
                    removeQuotes(groupingName);
    
                    if (lastGroupingName != groupingName) { // Simplified comparison
                        addHeader(list, groupingName);
                        currentPackageHeader = groupingName;
                        lastGroupingName = groupingName;
                    }
                } else if (commandGrouping == "split2") {
                    groupingName = getParentDirNameFromPath(selectedItem);
                    removeQuotes(groupingName);
    
                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3);
                        groupingName = groupingName.substr(0, pos);
                    }
    
                    if (lastGroupingName != groupingName) { // Simplified comparison
                        addHeader(list, groupingName);
                        currentPackageHeader = groupingName;
                        lastGroupingName = groupingName;
                    }
                } else if (commandGrouping == "split3") {
                    groupingName = getNameFromPath(selectedItem);
                    removeQuotes(groupingName);
    
                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3);
                        groupingName = groupingName.substr(0, pos);
                    }
    
                    if (lastGroupingName != groupingName) { // Simplified comparison
                        addHeader(list, groupingName);
                        currentPackageHeader = groupingName;
                        lastGroupingName = groupingName;
                    }
                } else if (commandGrouping == "split4") {
                    groupingName = getParentDirNameFromPath(selectedItem, 2);
                    removeQuotes(groupingName);
                    itemName = getNameFromPath(selectedItem);
                    dropExtension(itemName);
                    removeQuotes(itemName);
                    trim(itemName);
                    footer = getParentDirNameFromPath(selectedItem);
                    removeQuotes(footer);
    
                    if (lastGroupingName != groupingName) { // Simplified comparison
                        addHeader(list, groupingName);
                        currentPackageHeader = groupingName;
                        lastGroupingName = groupingName;
                    }
                } else if (commandGrouping == "split5") {
                    groupingName = getParentDirNameFromPath(selectedItem);
                    removeQuotes(groupingName);
    
                    pos = groupingName.find(" - ");
                    if (pos != std::string::npos) {
                        itemName = groupingName.substr(pos + 3);
                        groupingName = groupingName.substr(0, pos);
                    }
    
                    if (lastGroupingName != groupingName) { // Simplified comparison
                        addHeader(list, groupingName);
                        currentPackageHeader = groupingName;
                        lastGroupingName = groupingName;
                    }
                }
            //} else {
            //    if (commandMode == TOGGLE_STR && sourceType == FILE_STR) {
            //        if (std::find(filterListOn.cbegin(), filterListOn.cend(), itemName) != filterListOn.cend() ||
            //            std::find(filterListOff.cbegin(), filterListOff.cend(), itemName) != filterListOff.cend()) {
            //            continue;
            //        }
            //    } else if (commandMode != TOGGLE_STR) {
            //        if (std::find(filterList.cbegin(), filterList.cend(), itemName) != filterList.cend()) {
            //            continue;
            //        }
            //    }
            }
    
            if (commandMode == DEFAULT_STR || commandMode == OPTION_STR) {
                if (sourceType != FILE_STR && commandGrouping != "split2" && commandGrouping != "split3" && commandGrouping != "split4" && commandGrouping != "split5") {
                    pos = selectedItem.find(" - ");
                    footer = "";
                    itemName = selectedItem;
                    if (pos != std::string::npos) {
                        footer = selectedItem.substr(pos + 2);
                        itemName = selectedItem.substr(0, pos);
                    }
                } else if (commandGrouping == "split2") {
                    footer = getNameFromPath(selectedItem);
                    dropExtension(footer);
                }
    
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(itemName, "", isMini);
    
                // for handling footers that use translations / replacements
                applyLangReplacements(footer, true);
                convertComboToUnicode(footer);
                applyLangReplacements(specifiedFooterKey, true);
                convertComboToUnicode(specifiedFooterKey);
    
                applyLangReplacements(itemName, true);
                convertComboToUnicode(itemName);
                applyLangReplacements(selectedFooterDict[specifiedFooterKey], true);
                convertComboToUnicode(selectedFooterDict[specifiedFooterKey]);
    
                if (commandMode == OPTION_STR) {
                    if (selectedFooterDict[specifiedFooterKey] == itemName) {
                        lastSelectedListItem = listItem;
                        lastSelectedListItemFooter2 = footer;
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
    
                listItem->setClickListener([this, i, selectedItem, footer, listItem, currentPackageHeader, itemName](uint64_t keys) {
    
                    if (runningInterpreter.load(acquire)) {
                        return false;
                    }
    
                    if (simulatedSelect.exchange(false, acq_rel)) {
                        keys |= KEY_A;
                    }
    
                    if ((keys & KEY_A)) {
                        isDownloadCommand = false;
                        runningInterpreter.store(true, release);
    
                        executeInterpreterCommands(getSourceReplacement(selectionCommands, selectedItem, i, filePath), filePath, specificKey);
                        //startInterpreterThread(filePath);
    
                        listItem->setValue(INPROGRESS_SYMBOL);
    
                        
                        if (commandMode == OPTION_STR) {
                            selectedFooterDict[specifiedFooterKey] = listItem->getText();
                            if (lastSelectedListItem && listItem && lastSelectedListItem != listItem) {
                            
                                lastSelectedListItem->setValue(lastSelectedListItemFooter, true);
                                
                            }
                            lastSelectedListItemFooter = footer;
                            
                        }
                        
                        lastSelectedListItem = listItem;
                        shiftItemFocus(listItem);
    
                        lastRunningInterpreter = true;
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
    
                    else if (keys & SCRIPT_KEY) {
                        //inSelectionMenu = false;
    
                        auto modifiedCmds = getSourceReplacement(selectionCommands, selectedItem, i, filePath);
                        applyPlaceholderReplacementsToCommands(modifiedCmds, filePath);
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), filePath, itemName, "selection", false, currentPackageHeader);
                        return true;
                    }
    
                    return false;
                });
                list->addItem(listItem);
    
            } else if (commandMode == TOGGLE_STR) {
                auto* toggleListItem = new tsl::elm::ToggleListItem(itemName, false, ON, OFF, isMini);
    
                // Use const iterators for better performance
                const bool toggleStateOn = std::find(selectedItemsListOn.cbegin(), selectedItemsListOn.cend(), selectedItem) != selectedItemsListOn.cend();
                toggleListItem->setState(toggleStateOn);
    
                toggleListItem->setStateChangedListener([this, i, toggleListItem, selectedItem, itemName](bool state) {
    
                    tsl::Overlay::get()->getCurrentGui()->requestFocus(toggleListItem, tsl::FocusDirection::None);
                
                    if (toggleCount.find(i) == toggleCount.end()) toggleCount[i] = 0;
                    if (isInitialized.find(i) == isInitialized.end() || !isInitialized[i]) {
                        currentSelectedItems[i] = selectedItem;
                        isInitialized[i] = true;
                        currentPatternIsOriginal[i] = true;  // start in original pattern
                    }
    
                    const auto& activeCommands = !state ? selectionCommandsOn : selectionCommandsOff;
                    const auto& inactiveCommands = !state ? selectionCommandsOff : selectionCommandsOn;
                
                    // Optimized pattern search with early exit
                    std::string oldPattern, newPattern;
                    for (const auto& cmd : inactiveCommands) {
                        if (cmd.size() > 1 && cmd[0] == "file_source") {
                            oldPattern = cmd[1];
                            break; // Early exit once found
                        }
                    }
                    for (const auto& cmd : activeCommands) {
                        if (cmd.size() > 1 && cmd[0] == "file_source") {
                            newPattern = cmd[1];
                            break; // Early exit once found
                        }
                    }
    
                    preprocessPath(oldPattern,filePath);
                    preprocessPath(newPattern,filePath);
    
                    std::string pathToUse;
                    
                    if (toggleCount[i] % 2 == 0) {
                        // Even toggle: use original selectedItemsList[i] (pattern A)
                        pathToUse = selectedItem;
                        currentPatternIsOriginal[i] = true;
                    } else {
                        // Odd toggle: resolve from previous path
                        if (currentPatternIsOriginal[i]) {
                            // currentSelectedItems[i] corresponds to pattern A, resolve to pattern B
                            pathToUse = resolveWildcardFromKnownPath(oldPattern, currentSelectedItems[i], newPattern);
                            currentPatternIsOriginal[i] = false;
                        } else {
                            // currentSelectedItems[i] corresponds to pattern B, resolve back to pattern A
                            pathToUse = resolveWildcardFromKnownPath(newPattern, currentSelectedItems[i], oldPattern);
                            currentPatternIsOriginal[i] = true;
                        }
                    }
                
                    auto modifiedCmds = getSourceReplacement(activeCommands, pathToUse, i, filePath);
                
                    if (sourceType == FILE_STR) {
                        // Optimized search with early exit
                        for (const auto& cmd : modifiedCmds) {
                            if (cmd.size() > 1 && cmd[0] == "sourced_path") {
                                currentSelectedItems[i] = cmd[1];
                                break; // Early exit once found
                            }
                        }
                    }
                
                    interpretAndExecuteCommands(std::move(modifiedCmds), filePath, specificKey);
                    resetPercentages();
                
                    toggleCount[i]++;
                });
                                
                // Set the script key listener (for SCRIPT_KEY)
                toggleListItem->setScriptKeyListener([this, i, currentPackageHeader, itemName, selectedItem](bool state) {
                    // Initialize currentSelectedItem for this index if it does not exist
                    if (isInitialized.find(i) == isInitialized.end() || !isInitialized[i]) {
                        currentSelectedItems[i] = selectedItem;
                        isInitialized[i] = true;
                    }
    
                    //inSelectionMenu = false;
                    // Custom logic for SCRIPT_KEY handling
                    auto modifiedCmds = getSourceReplacement(state ? selectionCommandsOn : selectionCommandsOff, currentSelectedItems[i], i, filePath);
                    applyPlaceholderReplacementsToCommands(modifiedCmds, filePath);
                    tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), filePath, itemName, "selection", false, currentPackageHeader);
                });
    
                list->addItem(toggleListItem);
            }
            selectedItem.clear();
            selectedItem.shrink_to_fit();
        }

        // NOW you can clear everything
        selectedItemsList.clear();
        selectedItemsList.shrink_to_fit();
        selectedItemsListOn.clear();
        selectedItemsListOn.shrink_to_fit();
        selectedItemsListOff.clear();
        selectedItemsListOff.shrink_to_fit();
        
        if (!packageRootLayerTitle.empty())
            overrideTitle = true;
        if (!packageRootLayerVersion.empty())
            overrideVersion = true;
        
        PackageHeader packageHeader = getPackageHeaderFromIni(filePath + PACKAGE_FILENAME);
        if (!packageHeader.title.empty() && packageRootLayerTitle.empty())
            packageRootLayerTitle = packageHeader.title;
        if (!packageHeader.version.empty() && packageRootLayerVersion.empty())
            packageRootLayerVersion = packageHeader.version;
        if (!packageHeader.color.empty() && packageRootLayerColor.empty())
            packageRootLayerColor = packageHeader.color;
    
        if (packageHeader.title.empty() || overrideTitle)
            packageHeader.title = packageRootLayerTitle;
        if (packageHeader.version.empty() || overrideVersion)
            packageHeader.version = packageRootLayerVersion;
        if (packageHeader.color.empty())
            packageHeader.color = packageRootLayerColor;

        tsl::elm::OverlayFrame* rootFrame;
        
        if (filePath == PACKAGE_PATH) {
           rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        } else {
           rootFrame = new tsl::elm::OverlayFrame(
               (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(filePath)),
               !lastPackageHeader.empty() ? lastPackageHeader : (packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + "  Ultrahand Package" : "Ultrahand Package"),
               noClickableItems,
               "",
               packageHeader.color);
        }
    
        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        
        list->disableCaching();
        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        const bool isRunningInterp = runningInterpreter.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
        
        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            if (lastSelectedListItem)
                lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }
        
        if (goBackAfter) {
            goBackAfter = false;
            simulatedBack.exchange(true, acq_rel);
            return true;
        }
        
        // Cache touching state for refresh operations (used in same logical block)
        const bool isTouching = stillTouching.load(acquire);
        
        if (refreshPage && !isTouching) {
            tsl::goBack();
            tsl::changeTo<SelectionOverlay>(filePath, specificKey, specifiedFooterKey, lastPackageHeader, selectionCommands);
            refreshPage = false;
        }
        
        if (refreshPackage && !isTouching) {
            tsl::goBack();
        }
        
        if (inSelectionMenu) {
            simulatedNextPage.exchange(false, acq_rel);
            simulatedMenu.exchange(false, acq_rel);
            
            //if (simulatedBack.exchange(false, acq_rel)) {
            //    keysDown |= KEY_B;
            //}
            
            // Check touching state again for the key handling (different timing context)
            const bool isTouchingForKeys = stillTouching.load(acquire);
            const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouchingForKeys;
            
            if (backKeyPressed) {
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
                inSelectionMenu = false;
                
                // Determine return destination
                if (filePath == PACKAGE_PATH) {
                    returningToMain = true;
                } else {
                    if (lastPackageMenu == "subPackageMenu") {
                        returningToSubPackage = true;
                    } else {
                        returningToPackage = true;
                    }
                }
                
                // Handle package config footer logic
                if (commandMode == OPTION_STR && isFileOrDirectory(packageConfigIniPath)) {
                    const auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    auto it = packageConfigData.find(specificKey);
                    if (it != packageConfigData.end()) {
                        auto& optionSection = it->second;
                        auto footerIt = optionSection.find(FOOTER_STR);
                        if (footerIt != optionSection.end() && (footerIt->second.find(NULL_STR) == std::string::npos)) {
                            if (selectedListItem)
                                selectedListItem->setValue(footerIt->second);
                        }
                    }
                }
                //lastSelectedListItem = nullptr;
                tsl::goBack();
                return true;
            }
        }
        
        if (returningToSelectionMenu && !(keysDown & KEY_B)){
            returningToSelectionMenu = false;
            inSelectionMenu = true;
        }
        
        if (triggerExit.load(acquire)) {
            triggerExit.store(false, release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        return false;
    }
};

std::vector<std::vector<std::string>> gatherPromptCommands(
    const std::string& dropdownSection,
    const std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>>& options) {
    
    std::vector<std::vector<std::string>> promptCommands;
    
    bool inRelevantSection = false;
    bool isFirstSection = true;
    
    // Pre-define vectors outside loop to avoid repeated allocations
    std::vector<std::string> fillerCommand;
    std::vector<std::string> sectionCommand;
    std::vector<std::string> fullCmd;
    std::vector<std::string> splitParts;
    
    // Pre-allocate filler command (whitespace)
    fillerCommand.push_back("\u00A0");
    
    for (const auto& nextOption : options) {
        const std::string& sectionName = nextOption.first;
        const std::vector<std::vector<std::string>>& commands = nextOption.second;
        
        // Check if this is the start of the relevant section
        if (sectionName == dropdownSection) {
            inRelevantSection = true;
            continue;
        }
        
        // Stop capturing if we encounter a new section with no commands (empty section)
        if (inRelevantSection && commands.empty()) {
            break;
        }
        
        // Gather commands if we are in the relevant section
        if (inRelevantSection) {
            // Add section header as a command (with brackets)
            if (!sectionName.empty()) {
                if (!isFirstSection) {
                    promptCommands.push_back(fillerCommand); // Add whitespace separator
                } else {
                    isFirstSection = false;
                }
                
                // Clear and prepare section command
                sectionCommand.clear();
                //sectionCommand.shrink_to_fit();
                sectionCommand.push_back("[" + sectionName + "]");
                promptCommands.push_back(sectionCommand);
            }
            
            // Process each command by splitting on spaces
            for (const auto& cmd : commands) {
                fullCmd.clear();
                //fullCmd.shrink_to_fit();
                
                for (const auto& part : cmd) {
                    splitParts = splitString(part, " ");
                    fullCmd.insert(fullCmd.end(), splitParts.begin(), splitParts.end());
                }
                
                if (!fullCmd.empty()) {
                    promptCommands.push_back(fullCmd);
                }
            }
        }
    }
    
    // Return placeholder if no commands are found
    if (promptCommands.empty()) {
        promptCommands.push_back({UNAVAILABLE_SELECTION});
    }
    
    return promptCommands;
}

//auto clearAndDelete = [](auto& vec) {
//    for (auto* p : vec) delete p;
//    vec.clear();
//    vec.shrink_to_fit();
//};


class PackageMenu; // forwarding

// returns if there are or are not cickable items.
bool drawCommandsMenu(tsl::elm::List* list,
                    const std::string& packageIniPath,
                    const std::string& packageConfigIniPath,
                    const PackageHeader& packageHeader, std::string& pageLeftName, std::string& pageRightName,
                    const std::string& packagePath, const std::string& currentPage, const std::string& packageName, const std::string& dropdownSection, const size_t nestedLayer,
                    std::string& pathPattern, std::string& pathPatternOn, std::string& pathPatternOff, bool& usingPages, const bool packageMenuMode = true) {

    tsl::hlp::ini::IniData packageConfigData;
    //tsl::elm::ListItem* listItem;
    //auto toggleListItem = new tsl::elm::ToggleListItem("", true, "", "");
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath);
    
    bool toggleStateOn;
    
    bool skipSection = false;
    bool skipSystem = false;
    
    std::string lastSection;
    std::string drawLocation;
    
    std::string commandName;
    std::string commandFooter;
    std::string commandSystem;
    std::string commandMode;
    std::string commandGrouping;
    
    std::string currentSection;
    std::string defaultToggleState;
    std::string sourceType, sourceTypeOn, sourceTypeOff;
    
    std::string packageSource;
    //std::string jsonPath, jsonPathOn, jsonPathOff;
    //std::string jsonKey, jsonKeyOn, jsonKeyOff;
    
    std::string iniFilePath;

    std::string optionName;
    std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
    std::vector<std::vector<std::string>> tableData;
    
    std::string itemName, parentDirName;

    s16 minValue;
    s16 maxValue;
    std::string units;
    size_t steps;
    bool unlockedTrackbar;
    bool onEveryTick;
    std::string footer;
    bool useSelection;
    size_t pos;
    
    bool inEristaSection, inMarikoSection;
    bool _inEristaSection, _inMarikoSection;
    
    bool hideTableBackground, useHeaderIndent;
    size_t tableStartGap, tableEndGap, tableColumnOffset, tableSpacing;
    std::string tableSectionTextColor, tableInfoTextColor, tableAlignment;
    std::string tableWrappingMode;

    bool useWrappingIndent;

    size_t delimiterPos;

    bool isPolling;
    bool isScrollableTable;
    bool usingTopPivot, usingBottomPivot;
    bool onlyTables = true;

    std::vector<std::string> entryList;

    std::string commandNameLower;

    std::string cleanOptionName;

    std::string lastPackageHeader;

    bool isMini;

    // update general placeholders
    updateGeneralPlaceholders();


    for (size_t i = 0; i < options.size(); ++i) {
        commands.clear();
        tableData.clear();
        commandsOn.clear();
        commandsOff.clear();

        auto& option = options[i];
        
        optionName = std::move(option.first);

        commands = std::move(option.second);

        option.first.clear();
        option.second.clear();
        option.second.shrink_to_fit(); 

        footer = "";
        useSelection = false;

        isMini = false;

        // Table settings
        isPolling = false;
        isScrollableTable = true;
        usingTopPivot = false;
        usingBottomPivot = false;
        hideTableBackground = false;
        useHeaderIndent = false;
        tableStartGap = 20;
        tableEndGap = 9;
        tableColumnOffset = 163;
        tableSpacing = 0;
        tableSectionTextColor = DEFAULT_STR;
        tableInfoTextColor = DEFAULT_STR;
        tableAlignment = RIGHT_STR;
        tableWrappingMode = "none";
        useWrappingIndent = false;

        // Trackbar settings
        minValue = 0;
        maxValue = 100;
        units = "";
        steps = 0;
        unlockedTrackbar = true;
        onEveryTick = false;
        commandFooter = "";
        commandSystem = DEFAULT_STR;
        commandMode = DEFAULT_STR;
        commandGrouping = DEFAULT_STR;
        
        defaultToggleState = "";
        currentSection = GLOBAL_STR;
        sourceType = DEFAULT_STR;
        sourceTypeOn = DEFAULT_STR;
        sourceTypeOff = DEFAULT_STR;
        
        
        
        if (drawLocation.empty() || (currentPage == drawLocation) || (optionName.front() == '@')) {
            
            // Custom header implementation
            if (!dropdownSection.empty()) {
                if (i == 0) {
                    // Add a section break with small text to indicate the "Commands" section
                    std::string headerTitle = dropdownSection.substr(1);
                    removeTag(headerTitle);

                    addHeader(list, headerTitle);
                    lastPackageHeader = headerTitle;
                    //wasHeader = true;
                    skipSection = true;
                    lastSection = dropdownSection;
                }
                cleanOptionName = optionName;
                removeTag(cleanOptionName);
                if (cleanOptionName == PACKAGE_INFO || cleanOptionName == "Package Info") {
                    if (!skipSection) {
                        lastSection = optionName;
                        addPackageInfo(list, packageHeader);
                    }
                }
                if (commands.size() == 1) {
                    // Check if the first command has at least one element
                    if (!commands[0].empty()) {
                        const std::string& commandName = commands[0][0];
                        
                        // Use starts_with if C++20 is available, otherwise use compare
                        if (commandName.starts_with(MINI_PATTERN)) { // C++20
                        // if (commandName.compare(0, MINI_PATTERN.length(), MINI_PATTERN) == 0) { // Pre-C++20
                            
                            // Calculate expected minimum length
                            const size_t expectedMinLength = MINI_PATTERN.length() + TRUE_STR.length();
                            
                            if (commandName.length() >= expectedMinLength) {
                                std::string suffix = commandName.substr(MINI_PATTERN.length());
                                
                                // Only proceed if suffix matches expected values
                                if (suffix == TRUE_STR) {
                                    isMini = true;
                                    commands.clear();
                                    //commands.shrink_to_fit();
                                } else if (suffix == FALSE_STR) {
                                    isMini = false;
                                    commands.clear();
                                    //commands.shrink_to_fit();
                                }
                                // Invalid suffix: no action taken, preserves current state
                            }
                        }
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
                if (commands.size() == 1) {
                    // Check if the first command has at least one element
                    if (!commands[0].empty()) {
                        const std::string& commandName = commands[0][0];
                        
                        // Use starts_with if C++20 is available, otherwise use compare
                        if (commandName.starts_with(MINI_PATTERN)) { // C++20
                        // if (commandName.compare(0, MINI_PATTERN.length(), MINI_PATTERN) == 0) { // Pre-C++20
                            
                            // Calculate expected minimum length
                            const size_t expectedMinLength = MINI_PATTERN.length() + TRUE_STR.length();
                            
                            if (commandName.length() >= expectedMinLength) {
                                const std::string suffix = commandName.substr(MINI_PATTERN.length());
                                
                                // Only proceed if suffix matches expected values
                                if (suffix == TRUE_STR) {
                                    isMini = true;
                                    commands.clear();
                                    //commands.shrink_to_fit();
                                } else if (suffix == FALSE_STR) {
                                    isMini = false;
                                    commands.clear();
                                    //commands.shrink_to_fit();
                                }
                                // Invalid suffix: no action taken, preserves current state
                            }
                        }
                    }
                }

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
                        if (i == 0) {
                            // Add a section break with small text to indicate the "Commands" section
                            addHeader(list, COMMANDS);
                            lastPackageHeader = COMMANDS;
                            //wasHeader = true;
                            skipSection = false;
                            lastSection = "Commands";
                        }
                        //commandFooter = parseValueFromIniSection(packageConfigIniPath, optionName, FOOTER_STR);

                        packageConfigData = getParsedDataFromIniFile(packageConfigIniPath); // reuse variable (better memory management)
                        auto optionIt = packageConfigData.find(optionName);
                        if (optionIt != packageConfigData.end()) {
                            auto footerIt = optionIt->second.find(FOOTER_STR);
                            if (footerIt != optionIt->second.end()) {
                                commandFooter = footerIt->second;
                            }
                        }
                        packageConfigData.clear();

                        // override loading of the command footer

                        tsl::elm::ListItem* listItem;
                        if (!commandFooter.empty() && commandFooter != NULL_STR){
                            footer = commandFooter;
                            cleanOptionName = optionName.substr(1);
                            removeTag(cleanOptionName);
                            listItem = new tsl::elm::ListItem(cleanOptionName, "", isMini, true);
                            listItem->setValue(footer);
                        } else {
                            footer = DROPDOWN_SYMBOL;
                            cleanOptionName = optionName.substr(1);
                            removeTag(cleanOptionName);
                            // Create reference to PackageMenu with dropdownSection set to optionName
                            listItem = new tsl::elm::ListItem(cleanOptionName, footer, isMini, true);
                        }
                        
                        if (packageMenuMode) {
                            listItem->setClickListener([packagePath, currentPage, packageName, i, packageIniPath, optionName, lastPackageHeader](s64 keys) {
                                
                                if (runningInterpreter.load(acquire))
                                    return false;
                                if (simulatedSelect.exchange(false, acq_rel)) {
                                    keys |= KEY_A;
                                }
                                if (keys & KEY_A) {
                                    inPackageMenu = false;
                                    tsl::clearGlyphCacheNow.store(true, release);
                                    tsl::changeTo<PackageMenu>(packagePath, optionName, currentPage, packageName, 0, lastPackageHeader);
                                    
                                    return true;
                                
                                } else if (keys & SCRIPT_KEY) {
                                    if (inPackageMenu)
                                        inPackageMenu = false;
                                    if (inSubPackageMenu)
                                        inSubPackageMenu = false;
                                    
                                                        
                                    // Gather the prompt commands for the current dropdown section
                                    //const std::vector<std::vector<std::string>> promptCommands = gatherPromptCommands(optionName, options);
                                    
                                    //auto options = loadOptionsFromIni(packageIniPath);
                                    // Pass all gathered commands to the ScriptOverlay
                                    tsl::changeTo<ScriptOverlay>(std::move(gatherPromptCommands(optionName, loadOptionsFromIni(packageIniPath))), packagePath, optionName, "package", true, lastPackageHeader);
                                    return true;
                                }
                                return false;
                            });
                        } else {
                            listItem->setClickListener([optionName, i, packageIniPath, lastPackageHeader](s64 keys) {
                                if (runningInterpreter.load(acquire))
                                    return false;
                                if (simulatedSelect.exchange(false, acq_rel)) {
                                    keys |= KEY_A;
                                }
                                if (keys & KEY_A) {
                                    inPackageMenu = false;

                                    tsl::changeTo<MainMenu>("", optionName);
                                    return true;
                                } else if (keys & SCRIPT_KEY) {
                                    if (inMainMenu) {
                                        inMainMenu = false;
                                    }
                                    
                                    
                                    // Gather the prompt commands for the current dropdown section
                                    //const std::vector<std::vector<std::string>> promptCommands = gatherPromptCommands(optionName, options);
                                    //auto options = loadOptionsFromIni(packageIniPath);

                                    tsl::changeTo<ScriptOverlay>(std::move(gatherPromptCommands(optionName, loadOptionsFromIni(packageIniPath))), PACKAGE_PATH, optionName, "main", true, lastPackageHeader);
                                    return true;
                                }
                                return false;
                            });
                        }
                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(listItem);
                        
                        
                        skipSection = true;
                    } else {
                        if (optionName != lastSection) {
                            cleanOptionName = optionName;
                            removeTag(cleanOptionName);
                            if (cleanOptionName == PACKAGE_INFO || cleanOptionName == "Package Info") {
                                //logMessage("pre-before adding app info");
                                if (!skipSection) {
                                    lastSection = optionName;
                                    //logMessage("before adding app info");
                                    addPackageInfo(list, packageHeader);
                                    //logMessage("after adding app info");
                                }
                            } else {
                                // Add a section break with small text to indicate the "Commands" section
                                addHeader(list, cleanOptionName);
                                lastPackageHeader = cleanOptionName;

                                //wasHeader = true;
                                lastSection = optionName;
                            }
                        }
                        skipSection = false;
                    }
                    
                    
                    continue;
                } else if (i == 0) {
                    // Add a section break with small text to indicate the "Commands" section
                    addHeader(list, COMMANDS);
                    lastPackageHeader = COMMANDS;
                    //wasHeader = true;
                    skipSection = false;
                    lastSection = "Commands";
                }
            }
            
            // Call the function
            //processCommands(cmdOptions, cmdData);


            inEristaSection = false;
            inMarikoSection = false;
            
            // Remove all empty command strings
            removeEmptyCommands(commands);

            // Initial processing of commands (DUPLICATE CODE)
            for (auto& cmd : commands) {
                for (auto& arg : cmd) {
                    // Replace general placeholders
                    replacePlaceholdersInArg(arg, generalPlaceholders);
                }
                
                if (cmd.empty()) continue;

                commandName = cmd[0];
                
                commandNameLower = stringToLowercase(commandName);
                if (commandNameLower == "erista:") {
                    inEristaSection = true;
                    inMarikoSection = false;
                    continue;
                } else if (commandNameLower == "mariko:") {
                    inEristaSection = false;
                    inMarikoSection = true;
                    continue;
                }
                
                if ((inEristaSection && !inMarikoSection && usingErista) || 
                    (!inEristaSection && inMarikoSection && usingMariko) || 
                    (!inEristaSection && !inMarikoSection)) {
                    
                    if (commandName.find(SYSTEM_PATTERN) == 0) {
                        commandSystem = commandName.substr(SYSTEM_PATTERN.length());
                        if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                            commandSystem = commandSystems[0];
                        continue;
                    } else if (commandName.find(MODE_PATTERN) == 0) {
                        commandMode = commandName.substr(MODE_PATTERN.length());
                        if (commandMode.find(TOGGLE_STR) != std::string::npos) {
                            delimiterPos = commandMode.find('?');
                            if (delimiterPos != std::string::npos) {
                                defaultToggleState = commandMode.substr(delimiterPos + 1);
                            }
                            commandMode = TOGGLE_STR;
                        } else if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end()) {
                            commandMode = commandModes[0];
                        }
                        continue;
                    } else if (commandName.find(GROUPING_PATTERN) == 0) {
                        commandGrouping = commandName.substr(GROUPING_PATTERN.length());
                        if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                            commandGrouping = commandGroupings[0];
                        continue;
                    } else if (commandName.find(MINI_PATTERN) == 0) {
                        isMini = (commandName.substr(MINI_PATTERN.length()) == TRUE_STR);
                    } else if (commandName.find(POLLING_PATTERN) == 0) {
                        isPolling = (commandName.substr(POLLING_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(SCROLLABLE_PATTERN) == 0) {
                        isScrollableTable = (commandName.substr(SCROLLABLE_PATTERN.length()) != FALSE_STR);
                        continue;
                    } else if (commandName.find(TOP_PIVOT_PATTERN) == 0) {
                        usingTopPivot = (commandName.substr(TOP_PIVOT_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(BOTTOM_PIVOT_PATTERN) == 0) {
                        usingBottomPivot = (commandName.substr(BOTTOM_PIVOT_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(BACKGROUND_PATTERN) == 0) {
                        hideTableBackground = (commandName.substr(BACKGROUND_PATTERN.length()) == FALSE_STR);
                        continue;
                    } else if (commandName.find(HEADER_INDENT_PATTERN) == 0) {
                        useHeaderIndent = (commandName.substr(HEADER_INDENT_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(START_GAP_PATTERN) == 0) {
                        tableStartGap = ult::stoi(commandName.substr(START_GAP_PATTERN.length()));
                        continue;
                    } else if (commandName.find(END_GAP_PATTERN) == 0) {
                        tableEndGap = ult::stoi(commandName.substr(END_GAP_PATTERN.length()));
                        continue;
                    } else if (commandName.find(END_GAP_PATTERN_ALIAS) == 0) {
                        tableEndGap = ult::stoi(commandName.substr(END_GAP_PATTERN_ALIAS.length()));
                        continue;
                    } else if (commandName.find(OFFSET_PATTERN) == 0) {
                        tableColumnOffset = ult::stoi(commandName.substr(OFFSET_PATTERN.length()));
                        continue;
                    } else if (commandName.find(SPACING_PATTERN) == 0) {
                        tableSpacing = ult::stoi(commandName.substr(SPACING_PATTERN.length()));
                        continue;
                    } else if (commandName.find(SECTION_TEXT_COLOR_PATTERN) == 0) {
                        tableSectionTextColor = commandName.substr(SECTION_TEXT_COLOR_PATTERN.length());
                        continue;
                    } else if (commandName.find(INFO_TEXT_COLOR_PATTERN) == 0) {
                        tableInfoTextColor = commandName.substr(INFO_TEXT_COLOR_PATTERN.length());
                        continue;
                    } else if (commandName.find(ALIGNMENT_PATTERN) == 0) {
                        tableAlignment = commandName.substr(ALIGNMENT_PATTERN.length());
                        continue;
                    } else if (commandName.find(WRAPPING_MODE_PATTERN) == 0) {
                        tableWrappingMode = commandName.substr(WRAPPING_MODE_PATTERN.length());
                        continue;
                    } else if (commandName.find(WRAPPING_INDENT_PATTERN) == 0) {
                        useWrappingIndent = (commandName.substr(WRAPPING_INDENT_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(MIN_VALUE_PATTERN) == 0) {
                        minValue = ult::stoi(commandName.substr(MIN_VALUE_PATTERN.length()));
                        continue;
                    } else if (commandName.find(MAX_VALUE_PATTERN) == 0) {
                        maxValue = ult::stoi(commandName.substr(MAX_VALUE_PATTERN.length()));
                        continue;
                    } else if (commandName.find(UNITS_PATTERN) == 0) {
                        units = commandName.substr(UNITS_PATTERN.length());
                        removeQuotes(units);
                        continue;
                    } else if (commandName.find(STEPS_PATTERN) == 0) {
                        steps = ult::stoi(commandName.substr(STEPS_PATTERN.length()));
                        continue;
                    } else if (commandName.find(UNLOCKED_PATTERN) == 0) {
                        unlockedTrackbar = (commandName.substr(UNLOCKED_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(ON_EVERY_TICK_PATTERN) == 0) {
                        onEveryTick = (commandName.substr(ON_EVERY_TICK_PATTERN.length()) == TRUE_STR);
                        continue;
                    } else if (commandName.find(";") == 0) {
                        continue;
                    }
                    
                    if (commandMode == TOGGLE_STR) {
                        if (commandName.find("on:") == 0)
                            currentSection = ON_STR;
                        else if (commandName.find("off:") == 0)
                            currentSection = OFF_STR;
                        
                        if (currentSection == GLOBAL_STR) {
                            commandsOn.push_back(cmd);
                            commandsOff.push_back(cmd);
                        } else if (currentSection == ON_STR) {
                            commandsOn.push_back(cmd);
                        } else if (currentSection == OFF_STR) {
                            commandsOff.push_back(cmd);
                        }
                    } else if (commandMode == TABLE_STR) {
                        tableData.push_back(cmd);
                        continue;
                    } else if (commandMode == TRACKBAR_STR || commandMode == STEP_TRACKBAR_STR || commandMode == NAMED_STEP_TRACKBAR_STR) {
                        //commands.push_back(cmd);
                        continue;
                    }
                    
                    if (cmd.size() > 1) {
                        if (commandName == "ini_file") {
                            iniFilePath = cmd[1];
                            preprocessPath(iniFilePath, packagePath);
                        } else if (commandName == "file_source") {
                            if (currentSection == GLOBAL_STR) {
                                pathPattern = cmd[1];
                                preprocessPath(pathPattern, packagePath);
                                sourceType = FILE_STR;
                            } else if (currentSection == ON_STR) {
                                pathPatternOn = cmd[1];
                                preprocessPath(pathPatternOn, packagePath);
                                sourceTypeOn = FILE_STR;
                            } else if (currentSection == OFF_STR) {
                                pathPatternOff = cmd[1];
                                preprocessPath(pathPatternOff, packagePath);
                                sourceTypeOff = FILE_STR;
                            }
                        } else if (commandName == "package_source") {
                            packageSource = cmd[1];
                            preprocessPath(packageSource, packagePath);
                        }
                    }
                }
            }

            
            if (isFileOrDirectory(packageConfigIniPath)) {
                packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                syncIniValue(packageConfigData, packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                syncIniValue(packageConfigData, packageConfigIniPath, optionName, MODE_STR, commandMode);
                syncIniValue(packageConfigData, packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                syncIniValue(packageConfigData, packageConfigIniPath, optionName, FOOTER_STR, commandFooter);
                
                packageConfigData.clear();
            } else { // write default data if settings are not loaded
                // Load any existing data first (might be empty if file doesn't exist)
                packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                // Add the default values
                packageConfigData[optionName][SYSTEM_STR] = commandSystem;
                packageConfigData[optionName][MODE_STR] = commandMode;
                packageConfigData[optionName][GROUPING_STR] = commandGrouping;
                
                // Save the config file once
                saveIniFileData(packageConfigIniPath, packageConfigData);
                packageConfigData.clear();
            }
            
            
            // Get Option name and footer
            if (!optionName.empty() && optionName[0] == '*') { 
                useSelection = true;
                optionName.erase(0, 1); // Remove first character in-place
                footer = DROPDOWN_SYMBOL;
            } else {
                pos = optionName.find(" - ");
                if (pos != std::string::npos) {
                    footer.assign(optionName, pos + 3, std::string::npos); // Direct assignment from substring
                    optionName.resize(pos); // Truncate in-place instead of substr
                }
            }
            
            if ((commandMode == OPTION_STR) || (commandMode == SLOT_STR) || (commandMode == TOGGLE_STR && !useSelection)) {
                footer = OPTION_SYMBOL;
            }

            // override loading of the command footer
            if (!commandFooter.empty() && commandFooter != NULL_STR)
                footer = commandFooter;

            skipSystem = false;
            if (commandSystem == ERISTA_STR && !usingErista) {
                skipSystem = true;
            } else if (commandSystem == MARIKO_STR && !usingMariko) {
                skipSystem = true;
            }
            
            if (!skipSection && !skipSystem) { // for skipping the drawing of sections
                if (commandMode == TABLE_STR) {
                    if (useHeaderIndent) {
                        tableColumnOffset = 165;
                        tableStartGap = tableEndGap = 19-2; // for perfect alignment for header tables
                        isScrollableTable = false;
                        lastPackageHeader = getFirstSectionText(tableData, packagePath);
                    }

                    if (usingTopPivot) {
                        if (list->getLastIndex() == 0)
                            onlyTables = false;

                        addDummyListItem(list);
                    }


                    addTable(list, tableData, packagePath, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
                        tableSectionTextColor, tableInfoTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isPolling, isScrollableTable, tableWrappingMode, useWrappingIndent);
                    tableData.clear();

                    if (usingBottomPivot) {
                        addDummyListItem(list);
                    }

                    continue;
                } else if (commandMode == TRACKBAR_STR) {
                    onlyTables = false;
                
                    // Create TrackBarV2 instance and configure it
                    auto trackBar = new tsl::elm::TrackBarV2(optionName, packagePath, minValue, maxValue, units,
                        interpretAndExecuteCommands, getSourceReplacement, commands, optionName, false, false, -1, unlockedTrackbar, onEveryTick);
                
                    // Set the SCRIPT_KEY listener
                    trackBar->setScriptKeyListener([commands, keyName = optionName, packagePath, lastPackageHeader]() {
                        const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                        
                        //const std::string valueStr = parseValueFromIniSection(packagePath+"config.ini", keyName, "value");
                        //std::string indexStr = parseValueFromIniSection(packagePath+"config.ini", keyName, "index");


                        std::string valueStr = "";
                        std::string indexStr = "";
                        
                        {
                            auto configIniData = getParsedDataFromIniFile(packagePath + "config.ini");
                            auto sectionIt = configIniData.find(keyName);
                            if (sectionIt != configIniData.end()) {
                                auto valueIt = sectionIt->second.find("value");
                                if (valueIt != sectionIt->second.end()) {
                                    valueStr = valueIt->second;
                                }
                                
                                auto indexIt = sectionIt->second.find("index");
                                if (indexIt != sectionIt->second.end()) {
                                    indexStr = indexIt->second;
                                }
                            }
                        }

                        if (!isValidNumber(indexStr))
                            indexStr = "0";

                        // Handle the commands and placeholders for the trackbar
                        auto modifiedCmds = getSourceReplacement(commands, keyName, ult::stoi(indexStr), packagePath);

                        //auto modifiedCmds = getSourceReplacement(commands, valueStr, m_index, m_packagePath);
                        
                        // Placeholder replacement
                        //const std::string valuePlaceholder = "{value}";
                        //const std::string indexPlaceholder = "{index}";
                        //const size_t valuePlaceholderLength = valuePlaceholder.length();
                        //const size_t indexPlaceholderLength = indexPlaceholder.length();

                        size_t pos;
                        for (auto& cmd : modifiedCmds) {
                            for (auto& arg : cmd) {
                                pos = 0;
                                while ((pos = arg.find(valuePlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, valuePlaceholderLength, valueStr);
                                    pos += valueStr.length();
                                }
                                pos = 0;
                                while ((pos = arg.find(indexPlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, indexPlaceholderLength, indexStr);
                                    pos += indexStr.length();
                                }
                            }
                        }

                        applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                
                        // Switch to ScriptOverlay
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader);
                    });
                
                    // Add the TrackBarV2 to the list after setting the necessary listeners
                    list->addItem(trackBar);
                
                    continue;
                } else if (commandMode == STEP_TRACKBAR_STR) {
                    if (steps == 0) { // assign minimum steps
                        steps = std::abs(maxValue - minValue) +1;
                    }
                    onlyTables = false;
                    
                    auto stepTrackBar = new tsl::elm::StepTrackBarV2(optionName, packagePath, steps, minValue, maxValue, units,
                        interpretAndExecuteCommands, getSourceReplacement, commands, optionName, false, unlockedTrackbar, onEveryTick);
                    
                    // Set the SCRIPT_KEY listener
                    stepTrackBar->setScriptKeyListener([commands, keyName = optionName, packagePath, lastPackageHeader]() {
                        const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                        
                        // Parse the value and index from the INI file
                        //const std::string valueStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "value");
                        //std::string indexStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "index");


                        std::string valueStr = "";
                        std::string indexStr = "";
                        
                        {
                            auto configIniData = getParsedDataFromIniFile(packagePath + "config.ini");
                            auto sectionIt = configIniData.find(keyName);
                            if (sectionIt != configIniData.end()) {
                                auto valueIt = sectionIt->second.find("value");
                                if (valueIt != sectionIt->second.end()) {
                                    valueStr = valueIt->second;
                                }
                                
                                auto indexIt = sectionIt->second.find("index");
                                if (indexIt != sectionIt->second.end()) {
                                    indexStr = indexIt->second;
                                }
                            }
                        }
                        
                        if (!isValidNumber(indexStr))
                            indexStr = "0";

                        // Get and modify the commands with the appropriate replacements
                        auto modifiedCmds = getSourceReplacement(commands, keyName, ult::stoi(indexStr), packagePath);
                        
                        // Placeholder replacement for value and index
                        //const std::string valuePlaceholder = "{value}";
                        //const std::string indexPlaceholder = "{index}";
                        //const size_t valuePlaceholderLength = valuePlaceholder.length();
                        //const size_t indexPlaceholderLength = indexPlaceholder.length();
                        
                        size_t pos;
                        for (auto& cmd : modifiedCmds) {
                            for (auto& arg : cmd) {
                                pos = 0;
                                while ((pos = arg.find(valuePlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, valuePlaceholderLength, valueStr);
                                    pos += valueStr.length();
                                }
                                pos = 0;
                                while ((pos = arg.find(indexPlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, indexPlaceholderLength, indexStr);
                                    pos += indexStr.length();
                                }
                            }
                        }
                        
                        // Apply placeholder replacements and switch to ScriptOverlay
                        applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader);
                    });
                    
                    // Add the StepTrackBarV2 to the list
                    list->addItem(stepTrackBar);

                    continue;
                } else if (commandMode == NAMED_STEP_TRACKBAR_STR) {
                    //entryList.clear();
                    //entryList.shrink_to_fit();
                    entryList = {};
                    
                    _inEristaSection = false;
                    _inMarikoSection = false;
                    
                    //std::string commandName;
                    for (auto it = commands.begin(); it != commands.end(); /* no increment here */) {
                        auto& cmd = *it;
                        if (cmd.empty()) {
                            it = commands.erase(it);
                            continue;
                        }
                        
                        commandName = cmd[0];
                        
                        if (commandName == "erista:") {
                            _inEristaSection = true;
                            _inMarikoSection = false;
                            it = commands.erase(it);
                            continue;
                        }
                        else if (commandName == "mariko:") {
                            _inEristaSection = false;
                            _inMarikoSection = true;
                            it = commands.erase(it);
                            continue;
                        }
                    
                        if ((_inEristaSection && usingMariko) || (_inMarikoSection && usingErista)) {
                            it = commands.erase(it);
                            continue;
                        }
                    
                        if (cmd.size() > 1) {
                            if (cmd[0] == "list_source") {
                                std::string listString = cmd[1];
                                removeQuotes(listString);
                                entryList = stringToList(listString);
                                break;
                            }
                            else if (cmd[0] == "list_file_source") {
                                std::string listPath = cmd[1];
                                preprocessPath(listPath, packagePath);
                                entryList = readListFromFile(listPath);
                                break;
                            }
                            else if (cmd[0] == "ini_file_source") {
                                std::string iniPath = cmd[1];
                                preprocessPath(iniPath, packagePath);
                                entryList = parseSectionsFromIni(iniPath);
                                break;
                            }
                        }
                    
                        if (cmd.size() > 2) {
                            if (cmd[0] == "json_source") {
                                std::string jsonString = cmd[1];
                                removeQuotes(jsonString);
                                std::string jsonKey = cmd[2];
                                removeQuotes(jsonKey);
                                populateSelectedItemsListFromJson(JSON_STR, jsonString, jsonKey, entryList);
                                break;
                            }
                            else if (cmd[0] == "json_file_source") {
                                std::string jsonPath = cmd[1];
                                preprocessPath(jsonPath, packagePath);
                                std::string jsonKey = cmd[2];
                                removeQuotes(jsonKey);
                                populateSelectedItemsListFromJson(JSON_FILE_STR, jsonPath, jsonKey, entryList);
                                break;
                            }
                        }
                        
                        ++it;
                    }
                    onlyTables = false;

                    // Create NamedStepTrackBarV2 instance and configure it
                    auto namedStepTrackBar = new tsl::elm::NamedStepTrackBarV2(optionName, packagePath, entryList,
                        interpretAndExecuteCommands, getSourceReplacement, commands, optionName, unlockedTrackbar, onEveryTick);
                    
                    // Set the SCRIPT_KEY listener
                    namedStepTrackBar->setScriptKeyListener([commands, keyName = optionName, packagePath, entryList, lastPackageHeader]() {
                        const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                    
                        // Parse the value and index from the INI file
                        //std::string valueStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "value");
                        //std::string indexStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "index");


                        std::string valueStr = "";
                        std::string indexStr = "";
                        
                        {
                            auto configIniData = getParsedDataFromIniFile(packagePath + "config.ini");
                            auto sectionIt = configIniData.find(keyName);
                            if (sectionIt != configIniData.end()) {
                                auto valueIt = sectionIt->second.find("value");
                                if (valueIt != sectionIt->second.end()) {
                                    valueStr = valueIt->second;
                                }
                                
                                auto indexIt = sectionIt->second.find("index");
                                if (indexIt != sectionIt->second.end()) {
                                    indexStr = indexIt->second;
                                }
                            }
                        }
                    
                        // Fallback if indexStr is not a valid number
                        if (!isValidNumber(indexStr))
                            indexStr = "0";
                    
                        // Ensure the index is within the bounds of the entryList
                        const size_t entryIndex = std::min(static_cast<size_t>(ult::stoi(indexStr)), entryList.size() - 1);
                        valueStr = entryList[entryIndex];  // Update valueStr based on the current entry in the list
                    
                        // Get and modify the commands with the appropriate replacements
                        auto modifiedCmds = getSourceReplacement(commands, keyName, entryIndex, packagePath);
                    
                        // Placeholder replacement for value and index
                        //const std::string valuePlaceholder = "{value}";
                        //const std::string indexPlaceholder = "{index}";
                        //const size_t valuePlaceholderLength = valuePlaceholder.length();
                        //const size_t indexPlaceholderLength = indexPlaceholder.length();

                        size_t pos;

                        for (auto& cmd : modifiedCmds) {
                            for (auto& arg : cmd) {
                                pos = 0;
                                while ((pos = arg.find(valuePlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, valuePlaceholderLength, valueStr);
                                    pos += valueStr.length();
                                }
                                pos = 0;
                                while ((pos = arg.find(indexPlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, indexPlaceholderLength, indexStr);
                                    pos += indexStr.length();
                                }
                            }
                        }
                    
                        // Apply placeholder replacements and switch to ScriptOverlay
                        applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader);
                    });
                    entryList.clear();
                    
                    // Add the NamedStepTrackBarV2 to the list
                    list->addItem(namedStepTrackBar);

                    continue;
                }
                if (useSelection) { // For wildcard commands (dropdown menus)
                    tsl::elm::ListItem* listItem;
                    if ((footer == DROPDOWN_SYMBOL) || (footer.empty()) || footer == commandFooter) {
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        listItem = new tsl::elm::ListItem(cleanOptionName, footer, isMini, true);
                    }
                    else {
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        listItem = new tsl::elm::ListItem(cleanOptionName, "", isMini, true);

                        if (commandMode == OPTION_STR)
                            listItem->setValue(footer);
                        else
                            listItem->setValue(footer, true);
                    }
                    
                    if (footer == UNAVAILABLE_SELECTION || footer == NOT_AVAILABLE_STR || (footer.find(NULL_STR) != std::string::npos))
                        listItem->setValue(UNAVAILABLE_SELECTION, true);
                    if (commandMode == FORWARDER_STR) {

                        const std::string& forwarderPackagePath = getParentDirFromPath(packageSource);
                        const std::string& forwarderPackageIniName = getNameFromPath(packageSource);
                        listItem->setClickListener([commands, keyName = optionName, dropdownSection, packagePath, listItem,
                            forwarderPackagePath, forwarderPackageIniName, lastPackageHeader, i](s64 keys) mutable {
                            if (simulatedSelect.exchange(false, acq_rel)) {
                                keys |= KEY_A;
                            }
                            
                            if (keys & KEY_A) {
                                interpretAndExecuteCommands(std::move(getSourceReplacement(commands, keyName, i, packagePath)), packagePath, keyName);
                                resetPercentages();

                                nestedMenuCount++;
                                lastPackagePath = forwarderPackagePath;
                                lastPackageName = forwarderPackageIniName;
                                if (dropdownSection.empty())
                                    lastPackageMenu = "packageMenu";
                                else
                                    lastPackageMenu = "subPackageMenu";

                                // set forwarder pointer for updating
                                forwarderListItem = listItem;
                                lastCommandMode = FORWARDER_STR;
                                lastKeyName = keyName;

                                allowSlide.exchange(false, acq_rel);
                                unlockedSlide.exchange(false, acq_rel);

                                tsl::clearGlyphCacheNow.store(true, release);
                                tsl::changeTo<PackageMenu>(forwarderPackagePath, "", LEFT_STR, forwarderPackageIniName, nestedMenuCount, lastPackageHeader);
                                
                                return true;
                            } else if (keys & SCRIPT_KEY) {
                                const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inPackageMenu) {
                                //    inPackageMenu = false;
                                //    lastMenu = "packageMenu";
                                //}
                                //if (inSubPackageMenu) {
                                //    inSubPackageMenu = false;
                                //    lastMenu = "subPackageMenu";
                                //}

                                //auto modifiedCmds = getSourceReplacement(commands, keyName, i, packagePath);
                                
                                std::string selectionItem = keyName;
                                removeTag(selectionItem);
                                // add lines ;mode=forwarder and package_source 'forwarderPackagePath' to front of modifiedCmds
                                tsl::changeTo<ScriptOverlay>(std::move(getSourceReplacement(commands, keyName, i, packagePath)), packagePath, selectionItem, isFromMainMenu ? "main" : "package", true, lastPackageHeader);
                                return true;
                            }
                            return false;
                        });
                    } else {
                        listItem->setClickListener([commands, keyName = optionName, dropdownSection, packagePath, packageName,
                            footer, lastSection, listItem, lastPackageHeader, commandMode, i](uint64_t keys) {
                            //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem, [](auto*){})](uint64_t keys) {
                            
                            if (runningInterpreter.load(acquire))
                                return false;
                            if (simulatedSelect.exchange(false, acq_rel)) {
                                keys |= KEY_A;
                            }
                            if ((keys & KEY_A)) {
                                if (footer != UNAVAILABLE_SELECTION && footer != NOT_AVAILABLE_STR && (footer.find(NULL_STR) == std::string::npos)) {
                                    if (inPackageMenu)
                                        inPackageMenu = false;
                                    if (inSubPackageMenu)
                                        inSubPackageMenu = false;
                                    if (dropdownSection.empty())
                                        lastPackageMenu = "packageMenu";
                                    else
                                        lastPackageMenu = "subPackageMenu";
                                    
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
                                    
                                    if (commandMode == OPTION_STR || commandMode == SLOT_STR) {
                                        jumpItemName = "";
                                        jumpItemValue = "";
                                        jumpItemExactMatch = true;
                                        g_overlayFilename = "";
                                    } else {
                                        jumpItemName = "";
                                        jumpItemValue = "";
                                        jumpItemExactMatch = true;
                                        g_overlayFilename = "";
                                    }

                                    tsl::changeTo<SelectionOverlay>(packagePath, keyName, newKey, lastPackageHeader, commands);
                                    //lastKeyName = keyName;
                                }
                                return true;
                            } else if (keys & SCRIPT_KEY) {
                                const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inMainMenu) {
                                //    isFromMainMenu = true;
                                //    inMainMenu = false;
                                //}
                                //if (inPackageMenu) {
                                //    inPackageMenu = false;
                                //    lastMenu = "packageMenu";
                                //}
                                //if (inSubPackageMenu) {
                                //    inSubPackageMenu = false;
                                //    lastMenu = "subPackageMenu";
                                //}

                                std::string selectionItem = keyName;
                                removeTag(selectionItem);
                                auto modifiedCmds = commands;
                                applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                                tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, selectionItem, isFromMainMenu ? "main" : "package", true, lastPackageHeader);
                                return true;
                            }
                            return false;
                        });
                    }
                    onlyTables = false;
                    
                    list->addItem(listItem);
                } else { // For everything else
                    
                    const std::string& selectedItem = optionName;
                    
                    // For entries that are paths
                    itemName = getNameFromPath(selectedItem);
                    std::string tmpSelectedItem = selectedItem;
                    preprocessPath(tmpSelectedItem, packagePath);
                    if (!isDirectory(tmpSelectedItem))
                        dropExtension(itemName);
                    parentDirName = getParentDirNameFromPath(selectedItem);
                    if (commandMode == DEFAULT_STR  || commandMode == SLOT_STR || commandMode == OPTION_STR) { // for handiling toggles
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        tsl::elm::ListItem* listItem = new tsl::elm::ListItem(cleanOptionName, "", isMini, true);
                        if (commandMode == DEFAULT_STR)
                            listItem->setValue(footer, true);
                        else
                            listItem->setValue(footer);
                        
                        
                        listItem->setClickListener([i, commands, keyName = optionName, packagePath, packageName,
                            selectedItem, listItem, lastPackageHeader, commandMode](uint64_t keys) {
                            
                            if (runningInterpreter.load(acquire)) {
                                return false;
                            }
                            if (simulatedSelect.exchange(false, acq_rel)) {
                                keys |= KEY_A;
                            }
                            if ((keys & KEY_A)) {
                                isDownloadCommand = false;
                                runningInterpreter.store(true, release);
                                executeInterpreterCommands(getSourceReplacement(commands, selectedItem, i, packagePath), packagePath, keyName);
                                //startInterpreterThread(packagePath);
                                listItem->setValue(INPROGRESS_SYMBOL);
                                
                                //lastSelectedListItem = nullptr;
                                lastSelectedListItem = listItem;
                                shiftItemFocus(listItem);
                                lastCommandMode = commandMode;
                                lastKeyName = selectedItem;

                                lastRunningInterpreter = true;
                                if (lastSelectedListItem)
                                    lastSelectedListItem->triggerClickAnimation();
                                return true;
                            }  else if (keys & SCRIPT_KEY) {
                                bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inMainMenu) {
                                //    isFromMainMenu = true;
                                //    inMainMenu = false;
                                //}
                                //if (inPackageMenu) {
                                //    inPackageMenu = false;
                                //    lastMenu = "packageMenu";
                                //}
                                //if (inSubPackageMenu) {
                                //    inSubPackageMenu = false;
                                //    lastMenu = "subPackageMenu";
                                //}
                                auto modifiedCmds = getSourceReplacement(commands, selectedItem, i, packagePath);
                                applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                                tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader);
                                return true;
                            }
                            return false;
                        });
                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(listItem);
                    } else if (commandMode == TOGGLE_STR) {
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        auto* toggleListItem = new tsl::elm::ToggleListItem(cleanOptionName, false, ON, OFF, isMini);

                        // Set the initial state of the toggle item
                        if (!pathPatternOn.empty()){
                            //preprocessPath(pathPatternOn, packagePath);
                            toggleStateOn = isFileOrDirectory(pathPatternOn);
                        }
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
                        
                        toggleListItem->setStateChangedListener([i, commandsOn, commandsOff, keyName = optionName, packagePath,
                            pathPatternOn, pathPatternOff, listItem = toggleListItem](bool state) {
                            
                            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItem, tsl::FocusDirection::None);
                            
                            // Now pass the preprocessed paths to getSourceReplacement
                            interpretAndExecuteCommands(state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                                getSourceReplacement(commandsOff, pathPatternOff, i, packagePath), packagePath, keyName);
                            
                            resetPercentages();
                            // Set the ini file value after executing the command
                            setIniFileValue((packagePath + CONFIG_FILENAME), keyName, FOOTER_STR, state ? CAPITAL_ON_STR : CAPITAL_OFF_STR);
                            
                        });

                        // Set the script key listener (for SCRIPT_KEY)
                        toggleListItem->setScriptKeyListener([i, commandsOn, commandsOff, keyName = optionName, packagePath,
                            pathPatternOn, pathPatternOff, lastPackageHeader](bool state) {

                            const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                            //if (inPackageMenu)
                            //    inPackageMenu = false;
                            //if (inSubPackageMenu)
                            //    inSubPackageMenu = false;

                            // Custom logic for SCRIPT_KEY handling
                            auto modifiedCmds = state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                                getSourceReplacement(commandsOff, pathPatternOff, i, packagePath);

                            applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                            tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader);
                        });


                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(toggleListItem);
                    }
                }
            }
        }
    }

    options.clear();
    commands.clear();
    commandsOn.clear();
    commandsOff.clear();
    tableData.clear();

    if (onlyTables) {
        //auto dummyItem = new tsl::elm::DummyListItem();
        //list->addItem(dummyItem, 0, 1);
        addDummyListItem(list, 1); // assuming a header is always above
        //addDummyListItem(list);
        list->disableCaching();
    }

    return onlyTables;
}




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
    std::string pageHeader;

    std::string packageIniPath;
    std::string packageConfigIniPath;
    //bool menuIsGenerated = false;

public:
    /**
     * @brief Constructs a `PackageMenu` instance for a specific sub-menu path.
     *
     * Initializes a new instance of the `PackageMenu` class for the given sub-menu path.
     *
     * @param path The path to the sub-menu.
     */
    PackageMenu(const std::string& path, const std::string& sectionName = "", const std::string& page = LEFT_STR, const std::string& _packageName = PACKAGE_FILENAME, const size_t _nestedlayer = 0, const std::string& _pageHeader = "") :
        packagePath(path), dropdownSection(sectionName), currentPage(page), packageName(_packageName), nestedLayer(_nestedlayer), pageHeader(_pageHeader) {
            hexSumCache.clear();
            if (!skipJumpReset) {
                jumpItemName = "";
                jumpItemValue = "";
                jumpItemExactMatch = true;
                g_overlayFilename = "";
            } else
                skipJumpReset = false;
            //tsl::clearGlyphCacheNow.store(true, release);
            settingsInitialized.exchange(true, acq_rel);
        }

    /**
     * @brief Destroys the `PackageMenu` instance.
     *
     * Cleans up any resources associated with the `PackageMenu` instance.
     */
    ~PackageMenu() {
        hexSumCache.clear();

        if (returningToMain || returningToHiddenMain) {
            tsl::clearGlyphCacheNow.store(true, release);
            clearMemory();
            
            packageRootLayerTitle = "";
            packageRootLayerVersion = "";
            packageRootLayerColor = "";
            overrideTitle = false;
            overrideVersion = false;

            if (isFileOrDirectory(packagePath + EXIT_PACKAGE_FILENAME)) {
                const bool useExitPackage = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_EXIT_PACKAGE_STR) == FALSE_STR);
                
                if (useExitPackage) {
                    // Load only the commands from the specific section (bootCommandName)
                    auto exitCommands = loadSpecificSectionFromIni(packagePath + EXIT_PACKAGE_FILENAME, "exit");
                    
                    if (!exitCommands.empty()) {
                        bool resetCommandSuccess = false;
                        if (!commandSuccess) resetCommandSuccess = true;
                        
                        interpretAndExecuteCommands(std::move(exitCommands), packagePath, "exit");
                        resetPercentages();

                        if (resetCommandSuccess) {
                            commandSuccess = false;
                        }
                    }
                }
            }

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
        //menuIsGenerated = false;
        if (dropdownSection.empty()){
            inPackageMenu = true;
            lastMenu = "packageMenu";
        } else {
            inSubPackageMenu = true;
            lastMenu = "subPackageMenu";
        }
        
        auto* list = new tsl::elm::List();

        packageIniPath = packagePath + packageName;
        packageConfigIniPath = packagePath + CONFIG_FILENAME;

        PackageHeader packageHeader = getPackageHeaderFromIni(packageIniPath);
        
        
        std::string pageLeftName, pageRightName;
        bool noClickableItems = drawCommandsMenu(list, packageIniPath, packageConfigIniPath, packageHeader, pageLeftName, pageRightName,
            this->packagePath, this->currentPage, this->packageName, this->dropdownSection, this->nestedLayer,
            this->pathPattern, this->pathPatternOn, this->pathPatternOff, this->usingPages
        );
        

        if (nestedLayer == 0) {

            if (!packageRootLayerTitle.empty())
                overrideTitle = true;
            if (!packageRootLayerVersion.empty())
                overrideVersion = true;

            if (!packageHeader.title.empty() && packageRootLayerTitle.empty())
                packageRootLayerTitle = packageHeader.title;
            if (!packageHeader.version.empty() && packageRootLayerVersion.empty())
                packageRootLayerVersion = packageHeader.version;
            if (!packageHeader.color.empty() && packageRootLayerColor.empty())
                packageRootLayerColor = packageHeader.color;
        }
        if (packageHeader.title.empty() || overrideTitle)
            packageHeader.title = packageRootLayerTitle;
        if (packageHeader.version.empty() || overrideVersion)
            packageHeader.version = packageRootLayerVersion;
        if (packageHeader.color.empty())
            packageHeader.color = packageRootLayerColor;

        auto* rootFrame = new tsl::elm::OverlayFrame(
           (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(packagePath)),
           ((!pageHeader.empty() && packageHeader.show_version != TRUE_STR) ? pageHeader: (packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + "  Ultrahand Package" : "Ultrahand Package")),
           noClickableItems,
           "",
           packageHeader.color,
           (usingPages && currentPage == RIGHT_STR) ? pageLeftName : "",
           (usingPages && currentPage == LEFT_STR) ? pageRightName : ""
        );
        list->jumpToItem(jumpItemName,jumpItemValue);
        rootFrame->setContent(list);
        rootFrame->m_showWidget = (!packageHeader.show_widget.empty() && packageHeader.show_widget == TRUE_STR);
        return rootFrame;
    }
    

    void handleForwarderFooter() {
        if (lastCommandMode == FORWARDER_STR && isFileOrDirectory(packageConfigIniPath)) {
            auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
            auto it = packageConfigData.find(lastKeyName);
            if (it != packageConfigData.end()) {
                auto& optionSection = it->second;
                auto footerIt = optionSection.find(FOOTER_STR);
                if (footerIt != optionSection.end() && (footerIt->second.find(NULL_STR) == std::string::npos)) {
                    if (forwarderListItem)
                        forwarderListItem->setValue(footerIt->second);
                }
            }
            lastCommandMode = "";
        }
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

        const bool isRunningInterp = runningInterpreter.load(acquire);
        const bool isTouching = stillTouching.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
    
        if (lastRunningInterpreter) {
            //tsl::clearGlyphCacheNow.store(true, release);
            isDownloadCommand = false;
            if (lastSelectedListItem) {
                if (lastCommandMode == OPTION_STR || lastCommandMode == SLOT_STR) {
                    if (commandSuccess) {
                        if (isFileOrDirectory(packageConfigIniPath)) {
                            auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                            auto it = packageConfigData.find(lastKeyName);
                            if (it != packageConfigData.end()) {
                                auto& optionSection = it->second;
                                auto footerIt = optionSection.find(FOOTER_STR);
                                if (footerIt != optionSection.end() && (footerIt->second.find(NULL_STR) == std::string::npos)) {
                                    lastSelectedListItem->setValue(footerIt->second);
                                }
                            }
                            lastCommandMode = "";
                        } else {
                            lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
                        }
                    } else {
                        lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
                    }
                }
                else
                    lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            }
    
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }
    
        if (goBackAfter) {
            goBackAfter = false;
            simulatedBack.exchange(true, acq_rel);
            return true;
        }
    
        if (!returningToPackage && !isTouching) {
            if (refreshPage) {
                refreshPage = false;
                
                // Function to handle the transition and state resetting
                auto handleMenuTransition = [&] {
                    lastPackagePath = packagePath;
                    const std::string lastDropdownSection = dropdownSection;
                    lastPage = currentPage;
                    const std::string lastPackageName = packageName;
                    const size_t lastNestedLayer = nestedLayer;
                    
                    inSubPackageMenu = false;
                    inPackageMenu = false;
    
                    //selectedListItem = nullptr;
                    //lastSelectedListItem = nullptr;
                    //tsl::clearGlyphCacheNow.store(true, release);
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackagePath, lastDropdownSection, lastPage, lastPackageName, lastNestedLayer, pageHeader);
                };
                
                if (inPackageMenu) {
                    handleMenuTransition();
                    inPackageMenu = true;
                    return true;
                } 
                else if (inSubPackageMenu) {
                    handleMenuTransition();
                    inSubPackageMenu = true;
                    return true;
                }
            }
            if (refreshPackage) {
                if (nestedMenuCount == nestedLayer) {
                    lastPackagePath = packagePath;
                    lastPage = currentPage;
                    lastPackageName = PACKAGE_FILENAME;
                    
                    tsl::goBack(nestedMenuCount+1);
                    nestedMenuCount = 0;
    
                    tsl::changeTo<PackageMenu>(lastPackagePath, "");
                    inPackageMenu = true;
                    inSubPackageMenu = false;
                    refreshPackage = false;
                    return true;
                }
            }
        }
        
        if (usingPages) {
            simulatedMenu.exchange(false, acq_rel);
            
            if (simulatedNextPage.exchange(false, acq_rel)) {
                if (currentPage == LEFT_STR) {
                    keysDown |= KEY_DRIGHT;
                }
                else if (currentPage == RIGHT_STR) {
                    keysDown |= KEY_DLEFT;
                }
            }
    
            // Cache slide-related values
            const bool safeToSwap = tsl::elm::s_safeToSwap.load(acquire);
            const bool onTrack = onTrackBar.load(acquire);
            const bool slideAllowed = allowSlide.load(acquire);
            const bool slideUnlocked = unlockedSlide.load(acquire);
            const bool slideCondition = ((!slideAllowed && onTrack && !slideUnlocked) || (keysDown & KEY_R)) || !onTrack;
            
            // Helper lambda for slide transitions
            auto resetSlideState = [&]() {
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
            };
    
            if (currentPage == LEFT_STR) {
                if (safeToSwap && (keysDown & KEY_RIGHT) && !(keysHeld & KEY_LEFT) && 
                    !(keysDown & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK) && !isTouching && slideCondition) {
    
                    resetSlideState();
                    lastPage = RIGHT_STR;
                    tsl::pop();
                    tsl::changeTo<PackageMenu>(lastPackagePath, dropdownSection, RIGHT_STR, lastPackageName, nestedMenuCount, pageHeader);
                    return true;
                }
            } else if (currentPage == RIGHT_STR) {
                if (safeToSwap && (keysDown & KEY_LEFT) && !(keysHeld & KEY_RIGHT) && 
                    !(keysDown & ~KEY_LEFT & ~KEY_R & ALL_KEYS_MASK) && !isTouching && slideCondition) {
                    
                    resetSlideState();
                    lastPage = LEFT_STR;
                    tsl::pop();
                    tsl::changeTo<PackageMenu>(lastPackagePath, dropdownSection, LEFT_STR, lastPackageName, nestedMenuCount, pageHeader);
                    return true;
                }
            } 
        }
        
        // Common back key condition
        const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
        
        // Helper lambda for common back key handling logic
        auto handleBackKeyCommon = [&]() {
            handleForwarderFooter();
            allowSlide.exchange(false, acq_rel);
            unlockedSlide.exchange(false, acq_rel);
            
            if (nestedMenuCount == 0) {
                inPackageMenu = false;
                if (!inHiddenMode)
                    returningToMain = true;
                else
                    returningToHiddenMain = true;
                
                if (!selectedPackage.empty()) {
                    tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                    exitingUltrahand = true;
                    tsl::Overlay::get()->close();
                    return true;
                }
            }
            if (nestedMenuCount > 0) {
                nestedMenuCount--;
                if (lastPackageMenu == "subPackageMenu") {
                    returningToSubPackage = true;
                } else {
                    returningToPackage = true;
                }
            }
            return false;
        };
        
        // Helper lambda for main menu return handling
        auto handleMainMenuReturn = [&]() {
            if (returningToMain || returningToHiddenMain) {
                if (returningToHiddenMain) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_PACKAGE_STR, TRUE_STR);
                }
                jumpItemName = packageRootLayerIsStarred ? STAR_SYMBOL + "  " + packageRootLayerTitle : packageRootLayerTitle;
                jumpItemValue = hidePackageVersions ? "" : packageRootLayerVersion;
                jumpItemExactMatch = true;
                g_overlayFilename = "";
                skipJumpReset = true;
                
                //tsl::clearGlyphCacheNow.store(true, release);
                tsl::pop();
                
                if (returningToMain) {
                    tsl::changeTo<MainMenu>(PACKAGES_STR);
                } else {
                    tsl::changeTo<MainMenu>();
                }
            } else {
                //tsl::clearGlyphCacheNow.store(true, release);
                tsl::goBack();
                
            }
        };
        
        if (!returningToPackage && inPackageMenu && nestedMenuCount == nestedLayer) {
            simulatedMenu.exchange(false, acq_rel);
            simulatedNextPage.exchange(false, acq_rel);
            
            if (!usingPages || (usingPages && lastPage == LEFT_STR)) {
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
                if (backKeyPressed) {
                    if (handleBackKeyCommon()) return true;
                    // Free-up memory
                    //clearMemory();
                    handleMainMenuReturn();
                    return true;
                }
            } else if (usingPages && lastPage == RIGHT_STR) {
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
                if (backKeyPressed) {
                    if (handleBackKeyCommon()) return true;
                    // Free-up memory
                    //clearMemory();
                    lastPage = LEFT_STR;
                    handleMainMenuReturn();
                    return true;
                }
            }
        }
        
        if (!returningToSubPackage && inSubPackageMenu) {
            simulatedMenu.exchange(false, acq_rel);
            simulatedNextPage.exchange(false, acq_rel);
            
            if (!usingPages || (usingPages && lastPage == LEFT_STR)) {
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
                if (backKeyPressed) {
                    handleForwarderFooter();
                    allowSlide.exchange(false, acq_rel);
                    unlockedSlide.exchange(false, acq_rel);
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    //tsl::clearGlyphCacheNow.store(true, release);
                    tsl::goBack();
                    
                    return true;
                }
            } else if (usingPages && lastPage == RIGHT_STR) {
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
                if (backKeyPressed) {
                    handleForwarderFooter();
                    allowSlide.exchange(false, acq_rel);
                    unlockedSlide.exchange(false, acq_rel);
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    //tsl::clearGlyphCacheNow.store(true, release);
                    tsl::goBack();
                    
                    return true;
                }
            }
        }
        
        if (returningToPackage && !returningToSubPackage && !(keysDown & KEY_B)){
            lastPackageMenu = "";
            returningToPackage = false;
            returningToSubPackage = false;
            inPackageMenu = true;
            inSubPackageMenu = false;
            if (nestedMenuCount == 0 && nestedLayer == 0) {
                lastPackagePath = packagePath;
                lastPackageName = PACKAGE_FILENAME;
            }
        }
        
        if (returningToSubPackage && !(keysDown & KEY_B)){
            lastPackageMenu = "";
            returningToPackage = false;
            returningToSubPackage = false;
            inPackageMenu = false;
            inSubPackageMenu = true;
            if (nestedMenuCount == 0 && nestedLayer == 0) {
                lastPackagePath = packagePath;
                lastPackageName = PACKAGE_FILENAME;
            }
        }
        
        if (triggerExit.load(acquire)) {
            triggerExit.store(false, release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
    
        //if (simulatedBack.exchange(false, acq_rel)) {
        //    keysDown |= KEY_B;
        //}
        if (backKeyPressed) { // for catching lost navigations
            if (!selectedPackage.empty()) {
                tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                exitingUltrahand = true;
                tsl::Overlay::get()->close();
                return true;
            }
    
            allowSlide.exchange(false, acq_rel);
            unlockedSlide.exchange(false, acq_rel);
            inSubPackageMenu = false;
            returningToPackage = true;
            lastMenu = "packageMenu";
            //tsl::clearGlyphCacheNow.store(true, release);
            tsl::goBack();
            
            return true;
        }
        
        return false;
    };
};


bool toPackages = false;
bool inOverlay = false;
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
    //bool initializingSpawn = false;
    //std::string defaultLang = "en";

public:
    /**
     * @brief Constructs a `MainMenu` instance.
     *
     * Initializes a new instance of the `MainMenu` class with the necessary parameters.
     */
    MainMenu(const std::string& hiddenMenuMode = "", const std::string& sectionName = "") : hiddenMenuMode(hiddenMenuMode), dropdownSection(sectionName) {
        //tsl::gfx::FontManager::clearCache();
        if (skipJumpReset) {
            skipJumpReset = false;
            return;
        }
        jumpItemName = "";
        jumpItemValue = "";
        jumpItemExactMatch = true;
        settingsInitialized.exchange(true, acq_rel);
    }
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
        //menuIsGenerated = false;
    
        //if (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR) == TRUE_STR) {
        //    inMainMenu = false;
        //    inHiddenMode = true;
        //    hiddenMenuMode = OVERLAYS_STR;
        //    skipJumpReset = true;
        //    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, FALSE_STR);
        //}
        //
        //else if (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_PACKAGE_STR) == TRUE_STR) {
        //    inMainMenu = false;
        //    inHiddenMode = true;
        //    hiddenMenuMode = PACKAGES_STR;
        //    skipJumpReset = true;
        //    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_PACKAGE_STR, FALSE_STR);
        //}

        {
            // Load INI once and check both values
            auto iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
            
            bool needsUpdate = false;
            
            // Check for hidden overlay
            auto overlayIt = ultrahandSection.find(IN_HIDDEN_OVERLAY_STR);
            if (overlayIt != ultrahandSection.end() && overlayIt->second == TRUE_STR) {
                inMainMenu = false;
                inHiddenMode = true;
                hiddenMenuMode = OVERLAYS_STR;
                skipJumpReset = true;
                ultrahandSection[IN_HIDDEN_OVERLAY_STR] = FALSE_STR;
                needsUpdate = true;
            }
            // Check for hidden package
            else {
                auto packageIt = ultrahandSection.find(IN_HIDDEN_PACKAGE_STR);
                if (packageIt != ultrahandSection.end() && packageIt->second == TRUE_STR) {
                    inMainMenu = false;
                    inHiddenMode = true;
                    hiddenMenuMode = PACKAGES_STR;
                    skipJumpReset = true;
                    ultrahandSection[IN_HIDDEN_PACKAGE_STR] = FALSE_STR;
                    needsUpdate = true;
                }
            }
            
            // Write back only if changes were made
            if (needsUpdate) {
                saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
            }
        }

    
        if (!inHiddenMode && dropdownSection.empty())
            inMainMenu = true;
        else
            inMainMenu = false;
        
        // Pre-declare all variables used in loops and throughout the function
        std::string packagePath;
        
        bool noClickableItems = false;
        
        lastMenuMode = hiddenMenuMode;
        
        
        // Loop variables
        bool usingLibUltrahand = false;
        std::string overlayFileName, overlayName, overlayVersion, assignedOverlayName, assignedOverlayVersion;
        std::string baseOverlayInfo, fullOverlayInfo, priority, starred, hide, useLaunchArgs, launchArgs;
        std::string customName, customVersion, overlayFile, newOverlayName;
        std::string taintedOverlayFileName, packageName, packageVersion, tempPackageName, newPackageName, packageFilePath;
        //size_t lastColonPos, secondLastColonPos, thirdLastColonPos;
        bool overlayStarred, newStarred, packageStarred;
        bool foundOvlmenu = false;
        
        static bool hasInitialized = false;
        if (!hasInitialized) {
            if (!inOverlay) {
                if (!usePageSwap)
                    currentMenu = OVERLAYS_STR;
                else
                    currentMenu = PACKAGES_STR;
            }
            
            hasInitialized = true;
            
        }
        
        if (toPackages) {
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "to_packages", FALSE_STR); // this is handled within tesla.hpp
            toPackages = false;
            currentMenu = PACKAGES_STR;
        }
    
        menuMode = currentMenu;
        
        //versionLabel = cleanVersionLabel(APP_VERSION) + "  " + loaderTitle + " " + cleanVersionLabel(loaderInfo);
        //versionLabel = (cleanVersionLabels) ? std::string(APP_VERSION) : (std::string(APP_VERSION) + "   (" + extractTitle(loaderInfo) + " v" + cleanVersionLabel(loaderInfo) + ")");
        
        auto* list = new tsl::elm::List();
        //list = std::make_unique<tsl::elm::List>();
    
        
        
        if (!hiddenMenuMode.empty())
            menuMode = hiddenMenuMode;
        
        
        // Overlays menu
        if (menuMode == OVERLAYS_STR) {
            inOverlaysPage = true;
            inPackagesPage = false;
            //closeInterpreterThread();
    
            addHeader(list, !inHiddenMode ? OVERLAYS : HIDDEN_OVERLAYS);
            
            
            // Load overlay files
            std::vector<std::string> overlayFiles = getFilesListByWildcards(OVERLAY_PATH+"*.ovl");
            
            
            #if !USING_FSTREAM_DIRECTIVE
            
            // Check if the overlays INI file exists
            FILE* overlaysIniFile = fopen(OVERLAYS_INI_FILEPATH.c_str(), "r"); // Use .c_str() to convert to const char*
            if (!overlaysIniFile) {
                // The INI file doesn't exist, so create an empty one
                FILE* createFile = fopen(OVERLAYS_INI_FILEPATH.c_str(), "w"); // Use .c_str() here as well
                if (createFile) {
                    //initializingSpawn = true;
                    fclose(createFile); // Close the file after creating it
                }
            } else {
                fclose(overlaysIniFile); // Close the file if it exists
            }
            
            
            #else
    
            // Check if the overlays INI file exists
            std::ifstream overlaysIniFile(OVERLAYS_INI_FILEPATH);
            if (!overlaysIniFile.is_open()) {
                // The INI file doesn't exist, so create an empty one.
                std::ofstream createFile(OVERLAYS_INI_FILEPATH);
                //if (createFile.is_open()) {
                //initializingSpawn = true;
                createFile.close(); // Close the file after creating it
                //}
            }
    
            overlaysIniFile.close(); // Close the file
    
            #endif
    
            // load overlaySet from OVERLAYS_INI_FILEPATH.  this will be the overlayFilenames
            std::set<std::string> overlaySet;
            //std::set<std::string> hiddenOverlaySet;
            
            // Load subdirectories
            if (!overlayFiles.empty()) {
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> overlaysIniData = getParsedDataFromIniFile(OVERLAYS_INI_FILEPATH);
                
                // Single pass overlay filtering
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
    
                auto it = overlaysIniData.end();
                // Process overlay files
                bool drawHiddenTab = false;

                // Track if we need to write back
                bool overlaysNeedsUpdate = false;

                for (const auto& overlayFile : overlayFiles) {
                    overlayFileName = getNameFromPath(overlayFile);
                    
                    it = overlaysIniData.find(overlayFileName);
                    if (it == overlaysIniData.end()) {
                        // Initialization of new entries IN MEMORY (no file I/O)
                        auto& overlaySection = overlaysIniData[overlayFileName];
                        overlaySection[PRIORITY_STR] = "20";
                        overlaySection[STAR_STR] = FALSE_STR;
                        overlaySection[HIDE_STR] = FALSE_STR;
                        overlaySection[USE_LAUNCH_ARGS_STR] = FALSE_STR;
                        overlaySection[LAUNCH_ARGS_STR] = "";
                        overlaySection["custom_name"] = "";
                        overlaySection["custom_version"] = "";
                        overlaysNeedsUpdate = true;
                        
                        const auto& [result, overlayName, overlayVersion, usingLibUltrahand] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                        if (result != ResultSuccess) continue;
                
                        // Use retrieved overlay info
                        assignedOverlayName = overlayName;
                        assignedOverlayVersion = overlayVersion;
                    
                        //baseOverlayInfo = "0020" + assignedOverlayName + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + overlayFileName;
                        baseOverlayInfo = "0020" + assignedOverlayName + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + overlayFileName + ":" + (usingLibUltrahand ? "1" : "0");
                        overlaySet.insert(baseOverlayInfo);
                    } else {
                        hide = getValueOrDefault(it->second, HIDE_STR, FALSE_STR);
                        if (hide == TRUE_STR)
                            drawHiddenTab = true;
                        if ((!inHiddenMode && hide == FALSE_STR) || (inHiddenMode && hide == TRUE_STR)) {
                            priority = getValueOrDefault(it->second, PRIORITY_STR, "20", formatPriorityString, 1);
                            starred = getValueOrDefault(it->second, STAR_STR, FALSE_STR);
                            
                            useLaunchArgs = getValueOrDefault(it->second, USE_LAUNCH_ARGS_STR, FALSE_STR);
                            launchArgs = getValueOrDefault(it->second, LAUNCH_ARGS_STR, "");
                            customName = getValueOrDefault(it->second, "custom_name", "");
                            customVersion = getValueOrDefault(it->second, "custom_version", "");
                            
                            const auto& [result, overlayName, overlayVersion, usingLibUltrahand] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                            if (result != ResultSuccess) continue;
                            
                            assignedOverlayName = !customName.empty() ? customName : overlayName;
                            assignedOverlayVersion = !customVersion.empty() ? customVersion : overlayVersion;
                            
                            //baseOverlayInfo = priority + assignedOverlayName + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + overlayFileName;
                            baseOverlayInfo = priority + assignedOverlayName + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + overlayFileName + ":" + (usingLibUltrahand ? "1" : "0");
                            fullOverlayInfo = (starred == TRUE_STR) ? "-1:" + baseOverlayInfo : baseOverlayInfo;
                            
                            //if (!inHiddenMode) {
                            overlaySet.insert(fullOverlayInfo);
                            //} else {
                            //    hiddenOverlaySet.insert(fullOverlayInfo);
                            //}
                        }
                    }
                }

                // Write back file only if changes were made
                if (overlaysNeedsUpdate) {
                    saveIniFileData(OVERLAYS_INI_FILEPATH, overlaysIniData);
                }

                overlaysIniData.clear();
                
                //if (inHiddenMode) {
                //    overlaySet = std::move(hiddenOverlaySet);
                //    //hiddenOverlaySet.clear();
                //}
                
                // Process overlay list items
                for (const auto& taintedOverlayFileName : overlaySet) {
                    usingLibUltrahand = false;
                    overlayFileName = "";
                    overlayStarred = false;
                    overlayVersion = "";
                    overlayName = "";
                    
                    // Detect if starred
                    overlayStarred = (taintedOverlayFileName.substr(0, 3) == "-1:");
                    
                    // Find the position of the last colon (usingLibUltrahand flag)
                    const size_t lastColonPos = taintedOverlayFileName.rfind(':');
                    if (lastColonPos != std::string::npos) {
                        // Extract usingLibUltrahand flag
                        usingLibUltrahand = (taintedOverlayFileName.substr(lastColonPos + 1) == "1");
                        
                        // Find the position of the second-to-last colon (overlayFileName)
                        const size_t secondLastColonPos = taintedOverlayFileName.rfind(':', lastColonPos - 1);
                        if (secondLastColonPos != std::string::npos) {
                            // Extract overlayFileName
                            overlayFileName = taintedOverlayFileName.substr(secondLastColonPos + 1, lastColonPos - secondLastColonPos - 1);
                            
                            // Find the position of the third-to-last colon (overlayVersion)
                            const size_t thirdLastColonPos = taintedOverlayFileName.rfind(':', secondLastColonPos - 1);
                            if (thirdLastColonPos != std::string::npos) {
                                // Extract overlayVersion
                                overlayVersion = taintedOverlayFileName.substr(thirdLastColonPos + 1, secondLastColonPos - thirdLastColonPos - 1);
                                
                                // Find the position of the fourth-to-last colon (overlayName)
                                const size_t fourthLastColonPos = taintedOverlayFileName.rfind(':', thirdLastColonPos - 1);
                                if (fourthLastColonPos != std::string::npos) {
                                    overlayName = taintedOverlayFileName.substr(fourthLastColonPos + 1, thirdLastColonPos - fourthLastColonPos - 1);
                                }
                            }
                        }
                    }

                    overlayFile = OVERLAY_PATH+overlayFileName;
                    newOverlayName = overlayStarred ? STAR_SYMBOL+"  "+overlayName : overlayName;
                    
                    // Toggle the starred status
                    newStarred = !overlayStarred;
                    
                    if (isFileOrDirectory(overlayFile)) {
                        tsl::elm::ListItem* listItem = new tsl::elm::ListItem(newOverlayName);
                        overlayVersion = getFirstLongEntry(overlayVersion);
                        //std::string originalOverlayVersion = overlayVersion.c_str();
                        if (cleanVersionLabels)
                            overlayVersion = cleanVersionLabel(overlayVersion);
                        if (!hideOverlayVersions) {
                            //listItem->setValue(overlayVersion, true, true);
                            listItem->setValue(overlayVersion, true);

                            if (usingLibUltrahand) {
                                listItem->setValueColor(highlightVersions ? tsl::overlayVersionHighlightTextColor : tsl::overlayVersionTextColor);
                            }
                            else {
                                listItem->setValueColor(tsl::overlayVersionTextColor);
                            }
                        }

                        if (usingLibUltrahand) {
                            listItem->setTextColor(highlightTitles ? tsl::overlayEntryHighlightTextColor : tsl::overlayEntryTextColor);
                        }
                        else {
                            listItem->setTextColor(tsl::overlayEntryTextColor);
                        }
                        

                        if (overlayFileName == g_overlayFilename) {
                            jumpItemName = newOverlayName;
                            jumpItemValue = hideOverlayVersions ? "" : overlayVersion;
                            jumpItemExactMatch = true;
                        }

                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([overlayFile, newStarred, overlayFileName, overlayName, overlayVersion](s64 keys) {
                            
                            if (runningInterpreter.load(acquire))
                                return false;
                            
    
                            if (simulatedSelect.exchange(false, acq_rel)) {
                                keys |= KEY_A;
                            }
    
                            if (keys & KEY_A) {
                                
                                //std::string useOverlayLaunchArgs = parseValueFromIniSection(OVERLAYS_INI_FILEPATH, overlayFileName, USE_LAUNCH_ARGS_STR);
                                //std::string overlayLaunchArgs = parseValueFromIniSection(OVERLAYS_INI_FILEPATH, overlayFileName, LAUNCH_ARGS_STR);

                                std::string useOverlayLaunchArgs;
                                std::string overlayLaunchArgs;
                                {
                                    // Load INI once and extract both values
                                    auto overlaysIniData = getParsedDataFromIniFile(OVERLAYS_INI_FILEPATH);
                                    
                                    auto sectionIt = overlaysIniData.find(overlayFileName);
                                    if (sectionIt != overlaysIniData.end()) {
                                        auto useArgsIt = sectionIt->second.find(USE_LAUNCH_ARGS_STR);
                                        if (useArgsIt != sectionIt->second.end()) {
                                            useOverlayLaunchArgs = useArgsIt->second;
                                        }
                                        
                                        auto argsIt = sectionIt->second.find(LAUNCH_ARGS_STR);
                                        if (argsIt != sectionIt->second.end()) {
                                            overlayLaunchArgs = argsIt->second;
                                        }
                                    }
                                    removeQuotes(overlayLaunchArgs);
                                }

                                
                                
                                //if (inHiddenMode) {
                                //    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, TRUE_STR);
                                //}
                                //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp

                                {
                                    // Load INI data once and modify in memory
                                    auto iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
                                    auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
                                    
                                    // Make all changes in memory
                                    if (inHiddenMode) {
                                        ultrahandSection[IN_HIDDEN_OVERLAY_STR] = TRUE_STR;
                                    }
                                    ultrahandSection[IN_OVERLAY_STR] = TRUE_STR; // this is handled within tesla.hpp
                                    
                                    // Write back once
                                    saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
                                }


                                if (useOverlayLaunchArgs == TRUE_STR)
                                    tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                                else
                                    tsl::setNextOverlay(overlayFile);
                                
                                tsl::Overlay::get()->close();
                                
                                return true;
                            } else if (keys & STAR_KEY) {
                                
                                if (!overlayFile.empty()) {
                                    // Update the INI file with the new value
                                    setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, STAR_STR, newStarred ? TRUE_STR : FALSE_STR);
                                    // Now, you can use the newStarred value for further processing if needed
                                }
                                skipJumpReset = true;
                                jumpItemName = newStarred ? STAR_SYMBOL + "  " + overlayName : overlayName;
                                jumpItemValue = hideOverlayVersions ? "" : overlayVersion;
                                jumpItemExactMatch = true;
                                // Also clear the global overlay filename since we're not on the main overlay list
                                g_overlayFilename = "";

                                wasInHiddenMode = inHiddenMode;

                                if (inHiddenMode) {
                                    //tsl::goBack();
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                    reloadMenu2 = true;
                                }
                                refreshPage = true;
                                
                                return true;
                            } else if (keys & SETTINGS_KEY) {
                                if (!inHiddenMode) {
                                    lastMenu = "";
                                    inMainMenu = false;
                                } else {
                                    lastMenu = "hiddenMenuMode";
                                    inHiddenMode = false;
                                }
                                jumpItemName = newStarred ? STAR_SYMBOL + "  " + overlayName : overlayName;
                                jumpItemValue = hideOverlayVersions ? "" : overlayVersion;
                                jumpItemExactMatch = true;
                                // Also clear the global overlay filename since we're not on the main overlay list
                                g_overlayFilename = "";
                                
                                tsl::changeTo<SettingsMenu>(overlayFileName, OVERLAY_STR, overlayName, overlayVersion);
                                return true;
                            }
                            return false;
                        });
                        list->addItem(listItem);
                    }
                    //if (listItem != nullptr)
                        
                }
                //overlaySet.clear();
                
                if (drawHiddenTab && !inHiddenMode && !hideHidden) {
                    tsl::elm::ListItem* listItem = new tsl::elm::ListItem(HIDDEN, DROPDOWN_SYMBOL);
                    
                    listItem->setClickListener([](uint64_t keys) {
                        if (runningInterpreter.load(acquire))
                            return false;
    
                        if (simulatedSelect.exchange(false, acq_rel)) {
                            keys |= KEY_A;
                        }
    
                        if (keys & KEY_A) {
                            // reset tracking
                            g_overlayFilename = "";
                            jumpItemName = "";
                            jumpItemValue = "";
                            jumpItemExactMatch = true;
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>(OVERLAYS_STR);
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem);
                }
            }
        }
        
        
        // Packages menu
        if (menuMode == PACKAGES_STR ) {

            if (!isFileOrDirectory(PACKAGE_PATH + PACKAGE_FILENAME)) {
            #if !USING_FSTREAM_DIRECTIVE
                // Using stdio.h functions (FILE* and fprintf)
                FILE* packageFileOut = fopen((PACKAGE_PATH + PACKAGE_FILENAME).c_str(), "w");
                if (packageFileOut) {
                    fprintf(packageFileOut,
                        "[*Reboot To]\n"
                        "[*Boot Entry]\n"
                        "ini_file_source /bootloader/hekate_ipl.ini\n"
                        "filter config\n"
                        "reboot boot '{ini_file_source(*)}'\n"
                        "[hekate - \uE073]\n"
                        "reboot HEKATE\n"
                        "[hekate UMS - \uE073\uE08D]\n"
                        "reboot UMS\n"
                        "\n[Commands]\n"
                        "[Shutdown - \uE0F3]\n"
                        "shutdown\n"
                    );
                    fclose(packageFileOut); // Close the file after writing
                }
            #else
                // Using ofstream
                std::ofstream packageFileOut(PACKAGE_PATH + PACKAGE_FILENAME);
                if (packageFileOut) {
                    packageFileOut <<
                        "[*Reboot To]\n"
                        "[*Boot Entry]\n"
                        "ini_file_source /bootloader/hekate_ipl.ini\n"
                        "filter config\n"
                        "reboot boot '{ini_file_source(*)}'\n"
                        "[hekate - \uE073]\n"
                        "reboot HEKATE\n"
                        "[hekate UMS - \uE073\uE08D]\n"
                        "reboot UMS\n"
                        "\n[Commands]\n"
                        "[Shutdown - \uE0F3]\n"
                        "shutdown\n";
                    packageFileOut.close();
                }
            #endif
            }
    
            inOverlaysPage = false;
            inPackagesPage = true;
    
            if (dropdownSection.empty()) {
                // Create the directory if it doesn't exist
                createDirectory(PACKAGE_PATH);
                
                
                #if !USING_FSTREAM_DIRECTIVE
                // Using stdio.h functions (FILE* and fopen)
                FILE* packagesIniFile = fopen(PACKAGES_INI_FILEPATH.c_str(), "r");
                if (!packagesIniFile) {
                    // The file doesn't exist, so create an empty one
                    FILE* createFile = fopen(PACKAGES_INI_FILEPATH.c_str(), "w");
                    if (createFile) {
                        //initializingSpawn = true;
                        fclose(createFile); // Close the file after creating it
                    }
                } else {
                    fclose(packagesIniFile); // Close the file if it exists
                }
                #else
                // Using fstream
                std::fstream packagesIniFile(PACKAGES_INI_FILEPATH, std::ios::in);
                if (!packagesIniFile.is_open()) {
                    std::ofstream createFile(PACKAGES_INI_FILEPATH); // Create an empty INI file if it doesn't exist
                    createFile.close();
                    //initializingSpawn = true;
                } else {
                    packagesIniFile.close();
                }
                #endif
    
                
                std::set<std::string> packageSet;
                //std::set<std::string> hiddenPackageSet;
                
                // Load the INI file and parse its content.
                std::map<std::string, std::map<std::string, std::string>> packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                // Load subdirectories
                std::vector<std::string> subdirectories = getSubdirectories(PACKAGE_PATH);
    
                // Remove subdirectories starting with a dot in single pass
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
    
                PackageHeader packageHeader;
    
                auto packageIt = packagesIniData.end();

                bool drawHiddenTab = false;
                bool packagesNeedsUpdate = false;  // Track if we need to write back

                for (const auto& packageName: subdirectories) {
                    packageIt = packagesIniData.find(packageName);
                    if (packageIt == packagesIniData.end()) {
                        // Get package header info first for new packages
                        packageHeader = getPackageHeaderFromIni(PACKAGE_PATH + packageName+ "/" +PACKAGE_FILENAME);
                        
                        // Initialize missing package data IN MEMORY (no file I/O)
                        auto& packageSection = packagesIniData[packageName];
                        packageSection[PRIORITY_STR] = "20";
                        packageSection[STAR_STR] = FALSE_STR;
                        packageSection[HIDE_STR] = FALSE_STR;
                        packageSection[USE_BOOT_PACKAGE_STR] = TRUE_STR;
                        packageSection[USE_EXIT_PACKAGE_STR] = TRUE_STR;
                        packageSection[USE_QUICK_LAUNCH_STR] = FALSE_STR;
                        packageSection["custom_name"] = "";
                        packageSection["custom_version"] = "";
                        packagesNeedsUpdate = true;
                
                        assignedOverlayName = packageHeader.title.empty() ? packageName : packageHeader.title;
                        assignedOverlayVersion = packageHeader.version;
                
                        baseOverlayInfo = "0020:" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + packageName;
                        packageSet.insert(baseOverlayInfo);
    
                    } else {
                        // Process existing package data
                        hide = (packageIt->second.find(HIDE_STR) != packageIt->second.end()) ? 
                               packageIt->second[HIDE_STR] : FALSE_STR;

                        if (hide == TRUE_STR)
                            drawHiddenTab = true;
                        if ((!inHiddenMode && hide == FALSE_STR) || (inHiddenMode && hide == TRUE_STR)) {
                            priority = (packageIt->second.find(PRIORITY_STR) != packageIt->second.end()) ? 
                                        formatPriorityString(packageIt->second[PRIORITY_STR]) : "0020";
                            starred = (packageIt->second.find(STAR_STR) != packageIt->second.end()) ? 
                                      packageIt->second[STAR_STR] : FALSE_STR;
                            
                            customName = getValueOrDefault(packageIt->second, "custom_name", "");
                            customVersion = getValueOrDefault(packageIt->second, "custom_version", "");
                            
                            packageHeader = getPackageHeaderFromIni(PACKAGE_PATH + packageName+ "/" +PACKAGE_FILENAME);
                            
                            if (cleanVersionLabels) {
                                packageHeader.version = cleanVersionLabel(packageHeader.version);
                                removeQuotes(packageHeader.version);
                            }
                            
                            assignedOverlayName = !customName.empty() ? customName : 
                                                 (packageHeader.title.empty() ? packageName : packageHeader.title);
                            assignedOverlayVersion = !customVersion.empty() ? customVersion : packageHeader.version;
                            
                            baseOverlayInfo = priority + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + packageName;
                            fullOverlayInfo = (starred == TRUE_STR) ? "-1:" + baseOverlayInfo : baseOverlayInfo;
                            //if (!inHiddenMode) {
                            packageSet.insert(fullOverlayInfo);
                            //} else {
                            //    hiddenPackageSet.insert(fullOverlayInfo);
                            //}
                        }
                    }
                }

                // Write back file only if changes were made
                if (packagesNeedsUpdate) {
                    saveIniFileData(PACKAGES_INI_FILEPATH, packagesIniData);
                }
    
                packagesIniData.clear();
                subdirectories.clear();
                //subdirectories.shrink_to_fit();
                
                //if (inHiddenMode) {
                //    packageSet = std::move(hiddenPackageSet);
                //    //hiddenPackageSet.clear();
                //}

                bool firstItem = true;
                for (const auto& taintedPackageName : packageSet) {
                    if (firstItem) {
                        addHeader(list, !inHiddenMode ? PACKAGES : HIDDEN_PACKAGES);
                        firstItem = false;
                    }
                    
                    packageName = "";
                    packageVersion = "";
                    packageStarred = false;
                    
                    // Detect if starred
                    packageStarred = (taintedPackageName.substr(0, 3) == "-1:");
                    tempPackageName = packageStarred ? taintedPackageName.substr(3) : taintedPackageName;
                    
                    // Find the position of the last colon
                    const size_t lastColonPos = tempPackageName.rfind(':');
                    if (lastColonPos != std::string::npos) {
                        // Extract packageName starting from the character after the last colon
                        packageName = tempPackageName.substr(lastColonPos + 1);
                        
                        // Now, find the position of the second-to-last colon
                        const size_t secondLastColonPos = tempPackageName.rfind(':', lastColonPos - 1);
                        
                        if (secondLastColonPos != std::string::npos) {
                            // Extract packageVersion between the two colons
                            packageVersion = tempPackageName.substr(secondLastColonPos + 1, lastColonPos - secondLastColonPos - 1);
                            // Now, find the position of the third-to-last colon
                            const size_t thirdLastColonPos = tempPackageName.rfind(':', secondLastColonPos - 1);
                            if (thirdLastColonPos != std::string::npos)
                                newPackageName = tempPackageName.substr(thirdLastColonPos + 1, secondLastColonPos - thirdLastColonPos - 1);
                        }
                    }
                    
                    packageFilePath = PACKAGE_PATH + packageName + "/";
                    
                    // Toggle the starred status
                    newStarred = !packageStarred;
                    
                    if (isFileOrDirectory(packageFilePath)) {

                        tsl::elm::ListItem* listItem = new tsl::elm::ListItem(packageStarred ? STAR_SYMBOL + "  " + newPackageName : newPackageName);
                        if (!hidePackageVersions) {
                            listItem->setValue(packageVersion, true);
                            listItem->setValueColor((highlightVersions && highlightPackages) ? tsl::packageVersionHighlightTextColor : tsl::packageVersionTextColor);
                        }

                        listItem->setTextColor(highlightTitles ? tsl::packageEntryHighlightTextColor : tsl::packageEntryTextColor);
                        listItem->disableClickAnimation();
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([packageFilePath, newStarred, packageName, newPackageName, packageVersion, packageStarred](s64 keys) {
                            if (runningInterpreter.load(acquire)) {
                                return false;
                            }
                            
                            if (simulatedSelect.exchange(false, acq_rel)) {
                                keys |= KEY_A;
                            }
                            
                            if (keys & KEY_A) {
                                inMainMenu = false;
                                
                                if (isFileOrDirectory(packageFilePath + BOOT_PACKAGE_FILENAME)) {
                                    //bool useBootPackage = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, packageName, USE_BOOT_PACKAGE_STR) == FALSE_STR);
                                    //if (!selectedPackage.empty())
                                    //    useBootPackage = (useBootPackage && !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, packageName, USE_QUICK_LAUNCH_STR) == TRUE_STR));

                                    bool useBootPackage = true;
                                    {
                                        // Load INI data once and extract both values
                                        const auto packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                                        auto sectionIt = packagesIniData.find(selectedPackage);
                                        
                                        
                                        if (sectionIt != packagesIniData.end()) {
                                            auto bootIt = sectionIt->second.find(USE_BOOT_PACKAGE_STR);
                                            useBootPackage = (bootIt == sectionIt->second.end()) || (bootIt->second != FALSE_STR);
                                            
                                            if (!selectedPackage.empty()) {
                                                auto quickIt = sectionIt->second.find(USE_QUICK_LAUNCH_STR);
                                                const bool useQuickLaunch = (quickIt != sectionIt->second.end()) && (quickIt->second == TRUE_STR);
                                                useBootPackage = useBootPackage && !useQuickLaunch;
                                            }
                                        }
                                    }

                                    if (useBootPackage) {
                                        // Load only the commands from the specific section (bootCommandName)
                                        auto bootCommands = loadSpecificSectionFromIni(packageFilePath + BOOT_PACKAGE_FILENAME, "boot");
                                    
                                        if (!bootCommands.empty()) {
                                            bool resetCommandSuccess = false;
                                            if (!commandSuccess) resetCommandSuccess = true;
                                            
                                            interpretAndExecuteCommands(std::move(bootCommands), packageFilePath, "boot");
                                            resetPercentages();
    
                                            if (resetCommandSuccess) {
                                                commandSuccess = false;
                                            }
                                        }
                                    }
                                }
                                

                                lastPackagePath = packageFilePath;
                                lastPackageName = PACKAGE_FILENAME;
    
                                packageRootLayerTitle = newPackageName;
                                packageRootLayerVersion = packageVersion;
                                packageRootLayerIsStarred = packageStarred;
                                //tsl::clearGlyphCacheNow.store(true, release);
                                
                                tsl::clearGlyphCacheNow.store(true, release);
                                tsl::pop(2);
                                
                                tsl::changeTo<PackageMenu>(packageFilePath, "");

                                return true;
                            } else if (keys & STAR_KEY) {
                                if (!packageName.empty())
                                    setIniFileValue(PACKAGES_INI_FILEPATH, packageName, STAR_STR, newStarred ? TRUE_STR : FALSE_STR);
                                
                                skipJumpReset = true;
                                jumpItemName = newStarred ? STAR_SYMBOL + "  " + newPackageName : newPackageName;
                                jumpItemValue = hidePackageVersions ? "" : packageVersion;
                                jumpItemExactMatch = true;
                                // Also clear the global overlay filename since we're not on the main overlay list
                                g_overlayFilename = "";

                                wasInHiddenMode = inHiddenMode;
                                if (inHiddenMode) {
                                    inMainMenu = false;
                                    inHiddenMode = true;
                                    reloadMenu2 = true;
                                }
                                refreshPage = true;
    
                                return true;
                            } else if (keys & SETTINGS_KEY) {
                                
                                if (!inHiddenMode) {
                                    lastMenu = "";
                                    inMainMenu = false;
                                } else {
                                    lastMenu = "hiddenMenuMode";
                                    inHiddenMode = false;
                                }
                                jumpItemName = newStarred ? STAR_SYMBOL + "  " + newPackageName : newPackageName;
                                jumpItemValue = hidePackageVersions ? "" : packageVersion;
                                jumpItemExactMatch = true;
                                // Also clear the global overlay filename since we're not on the main overlay list
                                g_overlayFilename = "";
                                
                                tsl::changeTo<SettingsMenu>(packageName, PACKAGE_STR, newPackageName, packageVersion);
                                return true;
                            }
                            return false;
                        });
                        
                        if (listItem != nullptr)
                            list->addItem(listItem);
                    }
                }
                //packageSet.clear();
                
                if (drawHiddenTab && !inHiddenMode && !hideHidden) {
                    tsl::elm::ListItem* listItem = new tsl::elm::ListItem(HIDDEN, DROPDOWN_SYMBOL);
                    listItem->setClickListener([](uint64_t keys) {
                        if (runningInterpreter.load(acquire))
                            return false;
                        
                        if (simulatedSelect.exchange(false, acq_rel)) {
                            keys |= KEY_A;
                        }
                        
                        if (keys & KEY_A) {
                            inMainMenu = false;
                            inHiddenMode = true;
                            tsl::changeTo<MainMenu>(PACKAGES_STR);
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem);
                }
            }
            
            // ********* THIS PART NEEDS TO MIRROR WHAT IS WITHIN SUBMENU *********
            
            if (!inHiddenMode) {
                packagePath = PACKAGE_PATH;
                const std::string packageName = "package.ini";
                std::string pageLeftName = "";
                std::string pageRightName = "";
                const std::string currentPage = "left";
                const size_t nestedLayer = 0;
                std::string pathPattern, pathPatternOn, pathPatternOff;
                bool usingPages = false;
                
                PackageHeader packageHeader = getPackageHeaderFromIni(PACKAGE_PATH);
                noClickableItems = drawCommandsMenu(list, packageIniPath, packageConfigIniPath, packageHeader, pageLeftName, pageRightName,
                    packagePath, currentPage, packageName, this->dropdownSection, nestedLayer,
                    pathPattern, pathPatternOn, pathPatternOff, usingPages, false);
    
                if (!hideUserGuide && dropdownSection.empty())
                    addHelpInfo(list);
            }
        }
        
        //if (initializingSpawn) {
        //    initializingSpawn = false;
        //    return createUI(); 
        //}
        

        auto* rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel, noClickableItems, menuMode+hiddenMenuMode+dropdownSection, "", "", "");
        if (g_overlayFilename != "ovlmenu.ovl")
            list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        else
            g_overlayFilename = "";
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
        
        const bool isRunningInterp = runningInterpreter.load(acquire);
        const bool isTouching = stillTouching.load(acquire);
        
        if (isRunningInterp)
            return handleRunningInterpreter(keysDown, keysHeld);
    
        if (lastRunningInterpreter) {
            isDownloadCommand = false;
            if (lastSelectedListItem) {
                if (lastCommandMode == OPTION_STR || lastCommandMode == SLOT_STR) {
                    if (commandSuccess) {
                        if (isFileOrDirectory(packageConfigIniPath)) {
                            auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                            auto it = packageConfigData.find(lastKeyName);
                            if (it != packageConfigData.end()) {
                                auto& optionSection = it->second;
                                auto footerIt = optionSection.find(FOOTER_STR);
                                if (footerIt != optionSection.end() && (footerIt->second.find(NULL_STR) == std::string::npos)) {
                                    lastSelectedListItem->setValue(footerIt->second);
                                }
                            }
                            //lastSelectedListItem = nullptr;
                            lastCommandMode = "";
                        } else {
                            lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
                        }
                    } else {
                        lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
                    }
                }
                else
                    lastSelectedListItem->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
            }
    
            closeInterpreterThread();
            lastRunningInterpreter = false;
            return true;
        }
        
        if (goBackAfter) {
            goBackAfter = false;
            simulatedBack.exchange(true, acq_rel);
            return true;
        }
    
        if (refreshPage && !isTouching) {
            refreshPage = false;
            tsl::pop();
            tsl::changeTo<MainMenu>(hiddenMenuMode, dropdownSection);
            if (wasInHiddenMode) {
                skipJumpReset = true;
                jumpItemName = HIDDEN;
                jumpItemValue = DROPDOWN_SYMBOL;
                jumpItemExactMatch = true;
                g_overlayFilename = "";
                wasInHiddenMode = false;
            }
            return true;
        }
    
        // Common condition for back key handling
        const bool backKeyPressed = (keysDown & KEY_B || simulatedBack.exchange(false, acq_rel)) && !isTouching;
        
        if (!dropdownSection.empty() && !returningToMain) {
            simulatedNextPage.exchange(false, acq_rel);
            simulatedMenu.exchange(false, acq_rel);
    
            //if (simulatedBack.exchange(false, acq_rel)) {
            //    keysDown |= KEY_B;
            //}
    
            if (backKeyPressed) {
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
                returningToMain = true;
                tsl::goBack();
                return true;
            }
        }
        
        if (inMainMenu && !inHiddenMode && dropdownSection.empty()) {
            if (triggerMenuReload || triggerMenuReload2) {
                triggerMenuReload = triggerMenuReload2 = false;
                //if (menuMode == PACKAGES_STR)
                //    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "to_packages", FALSE_STR);
                //
                //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR);

                // Load INI data once and modify in memory
                {
                    auto iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
                    auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
                    
                    // Make all changes in memory
                    if (menuMode == PACKAGES_STR) {
                        ultrahandSection["to_packages"] = FALSE_STR;
                    }
                    ultrahandSection[IN_OVERLAY_STR] = TRUE_STR;
                    
                    // Write back once
                    saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
                }

                tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl", "--skipCombo");
                tsl::Overlay::get()->close();
            }
            
            if (!freshSpawn && !returningToMain && !returningToHiddenMain) {
                if (simulatedNextPage.exchange(false, acq_rel)) {
                    const bool toPackages = (!usePageSwap && menuMode != PACKAGES_STR) || (usePageSwap && menuMode != OVERLAYS_STR);
                    keysDown |= toPackages ? (usePageSwap ? KEY_DLEFT : KEY_DRIGHT) : (usePageSwap ? KEY_DRIGHT : KEY_DLEFT);
                }
                
                // Cache slide conditions
                const bool safeToSwap = tsl::elm::s_safeToSwap.load(acquire);
                const bool onTrack = onTrackBar.load(acquire);
                const bool slideAllowed = allowSlide.load(acquire);
                const bool slideUnlocked = unlockedSlide.load(acquire);
                const bool slideCondition = ((!slideAllowed && !slideUnlocked && onTrack) || (keysDown & KEY_R)) || !onTrack;
                
                // Helper lambda to reset navigation state
                auto resetNavState = [&]() {
                    g_overlayFilename = "";
                    jumpItemName = "";
                    jumpItemValue = "";
                    jumpItemExactMatch = true;
                    allowSlide.exchange(false, acq_rel);
                    unlockedSlide.exchange(false, acq_rel);
                };
                
                if (safeToSwap && (keysDown & KEY_RIGHT) && !(keysHeld & KEY_LEFT) && 
                    !(keysDown & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK) && !isTouching && slideCondition) {
                    
                    resetNavState();
                    const bool switchToPackages = (!usePageSwap && menuMode != PACKAGES_STR) || (usePageSwap && menuMode != OVERLAYS_STR);
                    if (switchToPackages) {
                        currentMenu = usePageSwap ? OVERLAYS_STR : PACKAGES_STR;
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        return true;
                    }
                }
    
                if (safeToSwap && (keysDown & KEY_LEFT) && !(keysHeld & KEY_RIGHT) && 
                    !(keysDown & ~KEY_LEFT & ~KEY_R & ALL_KEYS_MASK) && !isTouching && slideCondition) {
                    
                    resetNavState();
                    const bool switchToOverlays = (!usePageSwap && menuMode != OVERLAYS_STR) || (usePageSwap && menuMode != PACKAGES_STR);
                    if (switchToOverlays) {
                        currentMenu = usePageSwap ? PACKAGES_STR : OVERLAYS_STR;
                        tsl::pop();
                        tsl::changeTo<MainMenu>();
                        return true;
                    }
                }
    
                //if (simulatedBack.exchange(false, acq_rel)) {
                //    keysDown |= KEY_B;
                //}
    
                if (backKeyPressed) {
                    allowSlide.exchange(false, acq_rel);
                    unlockedSlide.exchange(false, acq_rel);
                    tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                    exitingUltrahand = true;
                    tsl::Overlay::get()->close();
                    return true;
                }
    
                if (simulatedMenu.exchange(false, acq_rel)) {
                    keysDown |= SYSTEM_SETTINGS_KEY;
                }
    
                if ((keysDown & SYSTEM_SETTINGS_KEY) && !isTouching) {
                    inMainMenu = false;
                    tsl::changeTo<UltrahandSettingsMenu>();
                    return true;
                }
            }
        }
        
        if (!inMainMenu && inHiddenMode && !returningToHiddenMain && !returningToMain) {
            simulatedNextPage.exchange(false, acq_rel);
            simulatedMenu.exchange(false, acq_rel);
    
            //if (simulatedBack.exchange(false, acq_rel)) {
            //    keysDown |= KEY_B;
            //}
    
            if (backKeyPressed) {
                // Check if we're in hidden mode with no underlying menu to go back to
                if (hiddenMenuMode == OVERLAYS_STR || hiddenMenuMode == PACKAGES_STR) {
                    inMainMenu = true;
                    inHiddenMode = false;
                    hiddenMenuMode = "";
                    //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, "");
                    //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_PACKAGE_STR, "");
                    {
                        // Load INI data once and modify in memory
                        auto iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
                        auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
                        
                        // Clear both values in memory
                        ultrahandSection[IN_HIDDEN_OVERLAY_STR] = "";
                        ultrahandSection[IN_HIDDEN_PACKAGE_STR] = "";
                        
                        // Write back once
                        saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
                    }

                    tsl::pop();
                    skipJumpReset = true;
                    jumpItemName = HIDDEN;
                    jumpItemValue = DROPDOWN_SYMBOL;
                    jumpItemExactMatch = true;
                    g_overlayFilename = "";
                    returningToMain = true;
                    tsl::changeTo<MainMenu>();
                    return true;
                }
    
                returningToMain = true;
                inHiddenMode = false;
                
                if (reloadMenu2) {
                    tsl::pop();
                    tsl::changeTo<MainMenu>();
                    reloadMenu2 = false;
                    return true;
                }
                
                allowSlide.exchange(false, acq_rel);
                unlockedSlide.exchange(false, acq_rel);
                tsl::goBack();
                return true;
            }
        }
        
        if (freshSpawn && !(keysDown & KEY_B))
            freshSpawn = false;
        
        if (returningToMain && !(keysDown & KEY_B)) {
            returningToMain = false;
            inMainMenu = true;
        }
        if (returningToHiddenMain && !(keysDown & KEY_B)) {
            returningToHiddenMain = false;
            inHiddenMode = true;
        }
    
        if (triggerExit.load(acquire)) {
            triggerExit.store(false, release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        //svcSleepThread(10'000'000);
        return false;
    }
};


// Extract the settings initialization logic into a separate method
void initializeSettingsAndDirectories() {
    versionLabel = cleanVersionLabel(APP_VERSION) + "  " + loaderTitle + " " + cleanVersionLabel(loaderInfo);
    std::string defaultLang = "en";

    // Create necessary directories
    createDirectory(PACKAGE_PATH);
    createDirectory(LANG_PATH);
    createDirectory(FLAGS_PATH);
    createDirectory(THEMES_PATH);
    createDirectory(WALLPAPERS_PATH);
    
    bool settingsLoaded = false;
    bool needsUpdate = false;
    
    std::map<std::string, std::map<std::string, std::string>> iniData;

    // Check if file didn't exist
    if (!isFileOrDirectory(ULTRAHAND_CONFIG_INI_PATH)) {
        updateMenuCombos = true;
    } else {
        // Always try to load INI data (will be empty if file doesn't exist)
        iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
        for (int i = 0; i < 3; i++) {
            if (iniData.empty() || iniData[ULTRAHAND_PROJECT_NAME].empty()) {
                svcSleepThread(100'000);
                iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            } else {
                break;
            }
        }
    }


    auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
    
    // Efficient lambdas that modify in-memory data and track updates
    auto setDefaultValue = [&](const std::string& section, const std::string& defaultValue, bool& settingFlag) {
        if (ultrahandSection.count(section) > 0) {
            settingFlag = (ultrahandSection.at(section) == TRUE_STR);
        } else {
            ultrahandSection[section] = defaultValue;
            settingFlag = (defaultValue == TRUE_STR);
            needsUpdate = true;
        }
    };
    
    auto setDefaultStrValue = [&](const std::string& section, const std::string& defaultValue, std::string& settingValue) {
        if (ultrahandSection.count(section) > 0) {
            settingValue = ultrahandSection.at(section);
        } else {
            ultrahandSection[section] = defaultValue;
            settingValue = defaultValue;
            needsUpdate = true;
        }
    };
    
    // Set default values for various settings (works for both existing and new files)
    setDefaultValue("hide_user_guide", FALSE_STR, hideUserGuide);
    setDefaultValue("hide_hidden", FALSE_STR, hideHidden);
    setDefaultValue("clean_version_labels", FALSE_STR, cleanVersionLabels);
    setDefaultValue("hide_overlay_versions", FALSE_STR, hideOverlayVersions);
    setDefaultValue("hide_package_versions", FALSE_STR, hidePackageVersions);
    setDefaultValue("highlight_titles", FALSE_STR, highlightTitles);
    setDefaultValue("highlight_versions", TRUE_STR, highlightVersions);
    setDefaultValue("highlight_packages", TRUE_STR, highlightPackages);
    setDefaultValue("memory_expansion", FALSE_STR, useMemoryExpansion);
    setDefaultValue("dynamic_logo", TRUE_STR, useDynamicLogo);
    setDefaultValue("launch_combos", TRUE_STR, useLaunchCombos);
    setDefaultValue("page_swap", FALSE_STR, usePageSwap);
    setDefaultValue("swipe_to_open", TRUE_STR, useSwipeToOpen);
    setDefaultValue("right_alignment", FALSE_STR, useRightAlignment);
    setDefaultValue("opaque_screenshots", TRUE_STR, useOpaqueScreenshots);
    
    setDefaultStrValue(DEFAULT_LANG_STR, defaultLang, defaultLang);

    // Ensure certain settings are set in the INI file if they don't exist (in memory)
    if (ultrahandSection.count("datetime_format") == 0) {
        ultrahandSection["datetime_format"] = DEFAULT_DT_FORMAT;
        needsUpdate = true;
    }

    if (ultrahandSection.count("hide_clock") == 0) {
        ultrahandSection["hide_clock"] = FALSE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("hide_battery") == 0) {
        ultrahandSection["hide_battery"] = TRUE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("hide_pcb_temp") == 0) {
        ultrahandSection["hide_pcb_temp"] = TRUE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("hide_soc_temp") == 0) {
        ultrahandSection["hide_soc_temp"] = TRUE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("dynamic_widget_colors") == 0) {
        ultrahandSection["dynamic_widget_colors"] = TRUE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("hide_widget_backdrop") == 0) {
        ultrahandSection["hide_widget_backdrop"] = FALSE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("center_widget_alignment") == 0) {
        ultrahandSection["center_widget_alignment"] = TRUE_STR;
        needsUpdate = true;
    }

    if (ultrahandSection.count("extended_widget_backdrop") == 0) {
        ultrahandSection["extended_widget_backdrop"] = FALSE_STR;
        needsUpdate = true;
    }
    
    // Check if settings were previously loaded
    settingsLoaded = ultrahandSection.count(IN_OVERLAY_STR) > 0;

    // Handle the 'to_packages' option if it exists
    if (ultrahandSection.count("to_packages") > 0) {
        trim(ultrahandSection["to_packages"]);
        toPackages = (ultrahandSection["to_packages"] == TRUE_STR);
    }
    
    // Handle the 'in_overlay' setting
    if (settingsLoaded) {
        inOverlay = (ultrahandSection[IN_OVERLAY_STR] == TRUE_STR);
    }
    
    // If settings weren't previously loaded, add the missing defaults
    if (!settingsLoaded) {
        ultrahandSection[DEFAULT_LANG_STR] = defaultLang;
        ultrahandSection[IN_OVERLAY_STR] = FALSE_STR;
        needsUpdate = true;
    }
    
    // Only write back to file if we made changes
    if (needsUpdate) {
        saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
    }
    
    // Load language file
    const std::string langFile = LANG_PATH + defaultLang + ".json";
    if (isFileOrDirectory(langFile))
        parseLanguage(langFile);
    else {
        if (defaultLang == "en")
            reinitializeLangVars();
    }
    
    // Initialize theme
    initializeTheme();
    tsl::initializeThemeVars();
    copyTeslaKeyComboToUltrahand();
    
    // Set current menu based on settings
    static bool hasInitialized = false;
    if (!hasInitialized) {
        if (!usePageSwap)
            currentMenu = OVERLAYS_STR;
        else
            currentMenu = PACKAGES_STR;
        hasInitialized = true;
    }
}

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
        tsl::overrideBackButton = true; // for properly overriding the always go back functionality of KEY_B

        ASSERT_FATAL(socketInitializeDefault());
        initializeCurl();

        // read commands from root package's boot_package.ini
        if (firstBoot) {
            // Load and execute "initial_boot" commands if they exist
            executeIniCommands(PACKAGE_PATH + BOOT_PACKAGE_FILENAME, "boot");
            
            const bool disableFuseReload = (parseValueFromIniSection(FUSE_DATA_INI_PATH, FUSE_STR, "disable_reload") == TRUE_STR);
            if (!disableFuseReload)
                deleteFileOrDirectory(FUSE_DATA_INI_PATH);

            // initialize expanded memory on boot
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "memory_expansion", (loaderTitle == "nx-ovlloader+") ? TRUE_STR : FALSE_STR);
        }
        
        unpackDeviceInfo();
        
    }
    
    /**
     * @brief Exits and cleans up services and resources.
     *
     * This function is responsible for exiting and cleaning up services and resources
     * when the overlay is no longer in use. It should release any allocated resources and
     * properly shut down services to avoid memory leaks.
     */
    virtual void exitServices() override {
        if (exitingUltrahand)
            executeIniCommands(PACKAGE_PATH + EXIT_PACKAGE_FILENAME, "exit");

        cleanupCurl();
        socketExit();
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
        //settingsInitialized.exchange(false, acq_rel);
        tsl::gfx::FontManager::preloadPersistentGlyphs("0123456789%●", 20);
        tsl::gfx::FontManager::preloadPersistentGlyphs(""+ult::HIDE+""+ult::CANCEL, 23);
        initializeSettingsAndDirectories();

        // Check if a package was specified via command line
        if (!selectedPackage.empty()) {
            
            const std::string packageFilePath = PACKAGE_PATH + selectedPackage + "/";
            
            // Check if the package directory exists
            if (isFileOrDirectory(packageFilePath)) {
                // GET PROPER PACKAGE TITLE AND VERSION (like main menu does)
                PackageHeader packageHeader = getPackageHeaderFromIni(packageFilePath + PACKAGE_FILENAME);
                
                // Load packages.ini to check for custom name/version
                const std::map<std::string, std::map<std::string, std::string>> packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                
                std::string customName = "";
                std::string customVersion = "";
                std::string assignedOverlayName;
                std::string assignedOverlayVersion;
                
                // Check if package exists in packages.ini
                auto packageIt = packagesIniData.find(selectedPackage);
                if (packageIt != packagesIniData.end()) {
                    // Get custom name and version if they exist
                    customName = getValueOrDefault(packageIt->second, "custom_name", "");
                    customVersion = getValueOrDefault(packageIt->second, "custom_version", "");
                }

                
                // Apply version cleaning if needed (same logic as main menu)
                if (cleanVersionLabels) {
                    packageHeader.version = cleanVersionLabel(packageHeader.version);
                    removeQuotes(packageHeader.version);
                }
                
                // Determine final name and version (same logic as main menu)
                assignedOverlayName = !customName.empty() ? customName : 
                                     (packageHeader.title.empty() ? selectedPackage : packageHeader.title);
                assignedOverlayVersion = !customVersion.empty() ? customVersion : packageHeader.version;
                
                // Handle boot package logic (similar to your KEY_A handler)
                if (isFileOrDirectory(packageFilePath + BOOT_PACKAGE_FILENAME)) {
                    //bool useBootPackage = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, selectedPackage, USE_BOOT_PACKAGE_STR) == FALSE_STR);
                    //if (!selectedPackage.empty())
                    //    useBootPackage = (useBootPackage && !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, selectedPackage, USE_QUICK_LAUNCH_STR) == TRUE_STR));


                    bool useBootPackage = true;
                    {
                        // Load INI data once and extract both values
                        const auto packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                        auto sectionIt = packagesIniData.find(selectedPackage);
                        
                        
                        if (sectionIt != packagesIniData.end()) {
                            auto bootIt = sectionIt->second.find(USE_BOOT_PACKAGE_STR);
                            useBootPackage = (bootIt == sectionIt->second.end()) || (bootIt->second != FALSE_STR);
                            
                            if (!selectedPackage.empty()) {
                                auto quickIt = sectionIt->second.find(USE_QUICK_LAUNCH_STR);
                                const bool useQuickLaunch = (quickIt != sectionIt->second.end()) && (quickIt->second == TRUE_STR);
                                useBootPackage = useBootPackage && !useQuickLaunch;
                            }
                        }
                    }

                    if (useBootPackage) {
                        // Load only the commands from the specific section (bootCommandName)
                        auto bootCommands = loadSpecificSectionFromIni(packageFilePath + BOOT_PACKAGE_FILENAME, "boot");
                    
                        if (!bootCommands.empty()) {
                            bool resetCommandSuccess = false;
                            if (!commandSuccess) resetCommandSuccess = true;
                            
                            interpretAndExecuteCommands(std::move(bootCommands), packageFilePath, "boot");
                            resetPercentages();
    
                            if (resetCommandSuccess) {
                                commandSuccess = false;
                            }
                        }
                    }
                }
    
                // Set the necessary global variables with PROPER NAMES
                lastPackagePath = packageFilePath;
                lastPackageName = PACKAGE_FILENAME;
                packageRootLayerTitle = assignedOverlayName;  // Use proper title
                packageRootLayerVersion = assignedOverlayVersion;  // Use proper version
                
                inMainMenu = false;
                
                // Return PackageMenu directly instead of MainMenu
                return initially<PackageMenu>(packageFilePath, "");
            } else {
                // Package not found, clear the selection and fall back to main menu
                selectedPackage.clear();
            }
        }
        //settingsInitialized.exchange(true, acq_rel);
        // Default behavior - load main menu
        return initially<MainMenu>();
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
    for (u8 arg = 0; arg < argc; arg++) {
        if (argv[arg][0] != '-') continue;  // Check first character
        
        if (strcasecmp(argv[arg], "--package") == 0) {
            if (arg + 1 < argc) {
                selectedPackage = "";
                
                // Collect all arguments until we hit another flag or end of args
                for (u8 nextArg = arg + 1; nextArg < argc; nextArg++) {
                    // Stop if we hit another flag (starts with -)
                    if (argv[nextArg][0] == '-') {
                        arg = nextArg - 1; // Set arg to the argument before the flag
                        break;
                    }
                    
                    // Add space if this isn't the first word
                    if (!selectedPackage.empty()) {
                        selectedPackage += " ";
                    }
                    selectedPackage += argv[nextArg];
                    arg = nextArg; // Update arg to track where we are
                }
                
                trim(selectedPackage);
            }
            break;
        } 
    }
    return tsl::loop<Overlay, tsl::impl::LaunchFlags::None>(argc, argv);
}
