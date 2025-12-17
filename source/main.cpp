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
 *  Copyright (c) 2023-2025 ppkantorski
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

static std::mutex transitionMutex;


// Placeholder replacement
const std::string valuePlaceholder = "{value}";
const std::string indexPlaceholder = "{index}";
const size_t valuePlaceholderLength = valuePlaceholder.length();
const size_t indexPlaceholderLength = indexPlaceholder.length();

static std::string selectedPackage; // for package forwarders

static std::string nextToggleState;

// Overlay booleans
static bool returningToMain = false;
static bool returningToHiddenMain = false;
static bool returningToSettings = false;
static bool returningToPackage = false;
static bool returningToSubPackage = false;
static bool returningToSelectionMenu = false;
//static bool languageWasChanged = false; // moved to tsl_utils
static bool themeWasChanged = false;

//static bool skipJumpReset.store(false, release); // for overrridng the default main menu jump to implementation // moved to utils

//static bool inMainMenu = false; // moved to libtesla
static bool wasInHiddenMode = false;
//static bool inHiddenMode = false;
//static bool inSettingsMenu = false;
//static bool inSubSettingsMenu = false;
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
static const std::vector<std::string> commandModes = {DEFAULT_STR, HOLD_STR, SLOT_STR, TOGGLE_STR, OPTION_STR, FORWARDER_STR, TEXT_STR, TABLE_STR, TRACKBAR_STR, STEP_TRACKBAR_STR, NAMED_STEP_TRACKBAR_STR};
static const std::vector<std::string> commandGroupings = {DEFAULT_STR, "split", "split2", "split3", "split4", "split5"};
static const std::string MODE_PATTERN = ";mode=";
static const std::string GROUPING_PATTERN = ";grouping=";
static const std::string FOOTER_PATTERN = ";footer=";
static const std::string FOOTER_HIGHLIGHT_PATTERN = ";footer_highlight=";
static const std::string HOLD_PATTERN = ";hold=";
static const std::string SYSTEM_PATTERN = ";system=";
static const std::string WIDGET_PATTERN = ";widget=";

static const std::string MINI_PATTERN = ";mini=";
static const std::string SELECTION_MINI_PATTERN = ";selection_mini=";

// Toggle option patterns
static const std::string PROGRESS_PATTERN = ";progress=";

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


static const size_t SYSTEM_PATTERN_LEN = SYSTEM_PATTERN.length();
static const size_t MODE_PATTERN_LEN = MODE_PATTERN.length();
static const size_t GROUPING_PATTERN_LEN = GROUPING_PATTERN.length();
static const size_t FOOTER_PATTERN_LEN = FOOTER_PATTERN.length();
static const size_t FOOTER_HIGHLIGHT_PATTERN_LEN = FOOTER_HIGHLIGHT_PATTERN.length();
static const size_t HOLD_PATTERN_LEN = HOLD_PATTERN.length();
static const size_t MINI_PATTERN_LEN = MINI_PATTERN.length();
static const size_t SELECTION_MINI_PATTERN_LEN = SELECTION_MINI_PATTERN.length();
static const size_t PROGRESS_PATTERN_LEN = PROGRESS_PATTERN.length();
static const size_t POLLING_PATTERN_LEN = POLLING_PATTERN.length();
static const size_t SCROLLABLE_PATTERN_LEN = SCROLLABLE_PATTERN.length();
static const size_t TOP_PIVOT_PATTERN_LEN = TOP_PIVOT_PATTERN.length();
static const size_t BOTTOM_PIVOT_PATTERN_LEN = BOTTOM_PIVOT_PATTERN.length();
static const size_t BACKGROUND_PATTERN_LEN = BACKGROUND_PATTERN.length();
static const size_t HEADER_INDENT_PATTERN_LEN = HEADER_INDENT_PATTERN.length();
static const size_t START_GAP_PATTERN_LEN = START_GAP_PATTERN.length();
static const size_t END_GAP_PATTERN_LEN = END_GAP_PATTERN.length();
static const size_t END_GAP_PATTERN_ALIAS_LEN = END_GAP_PATTERN_ALIAS.length();
static const size_t OFFSET_PATTERN_LEN = OFFSET_PATTERN.length();
static const size_t SPACING_PATTERN_LEN = SPACING_PATTERN.length();
static const size_t SECTION_TEXT_COLOR_PATTERN_LEN = SECTION_TEXT_COLOR_PATTERN.length();
static const size_t INFO_TEXT_COLOR_PATTERN_LEN = INFO_TEXT_COLOR_PATTERN.length();
static const size_t ALIGNMENT_PATTERN_LEN = ALIGNMENT_PATTERN.length();
static const size_t WRAPPING_MODE_PATTERN_LEN = WRAPPING_MODE_PATTERN.length();
static const size_t WRAPPING_INDENT_PATTERN_LEN = WRAPPING_INDENT_PATTERN.length();
static const size_t MIN_VALUE_PATTERN_LEN = MIN_VALUE_PATTERN.length();
static const size_t MAX_VALUE_PATTERN_LEN = MAX_VALUE_PATTERN.length();
static const size_t UNITS_PATTERN_LEN = UNITS_PATTERN.length();
static const size_t STEPS_PATTERN_LEN = STEPS_PATTERN.length();
static const size_t UNLOCKED_PATTERN_LEN = UNLOCKED_PATTERN.length();
static const size_t ON_EVERY_TICK_PATTERN_LEN = ON_EVERY_TICK_PATTERN.length();
static const size_t TRUE_STR_LEN = TRUE_STR.length();
static const size_t FALSE_STR_LEN = FALSE_STR.length();


static std::string currentMenu = OVERLAYS_STR;
//static std::string lastPage = LEFT_STR;
//static std::string lastPackagePath;
//static std::string lastPackageName;
static std::string lastPackageMenu;
//static std::string lastPageHeader;

static std::string lastMenu = "";
static std::string lastMenuMode = "";
static std::string lastKeyName = "";
static bool hideUserGuide = false;
//static bool hideHidden = false; // moved to tesla.hpp
static bool hideDelete = false;
//static bool hideForceSupport = true;
static bool hideUnsupported = false;

static std::string lastCommandMode;
static bool lastCommandIsHold;
static bool lastFooterHighlight;
static bool lastFooterHighlightDefined; 

static std::unordered_map<std::string, std::string> selectedFooterDict;

static tsl::elm::ListItem* selectedListItem;
static tsl::elm::ListItem* lastSelectedListItem;
//static tsl::elm::ListItem* forwarderListItem;
//static tsl::elm::ListItem* dropdownListItem;

static std::atomic<bool> lastRunningInterpreter{false};



template<typename Map, typename Func = std::function<std::string(const std::string&)>, typename... Args>
std::string getValueOrDefault(const Map& data, const std::string& key, const std::string& defaultValue, Func formatFunc = nullptr, Args... args) {
    auto it = data.find(key);
    if (it != data.end()) {
        return formatFunc ? formatFunc(it->second, args...) : it->second;
    }
    return defaultValue;
}


inline void clearMemory() {
    //hexSumCache = {};
    selectedFooterDict = {};
    clearIniMutexCache();
    clearHexSumCache();
    //hexSumCache.clear();
    //selectedFooterDict.clear(); // Clears all data from the map, making it empty again
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
    
    static bool wasHoldingR = false;

    const bool isHoldingR = keysHeld & KEY_R && !(keysHeld & ~KEY_R & ALL_KEYS_MASK);
    const bool releasedR = wasHoldingR && !(isHoldingR);
    wasHoldingR = isHoldingR;

    // FIX: More robust abort handling
    if ((releasedR && !(keysHeld & ~KEY_R & ALL_KEYS_MASK) && !stillTouching.load(acquire)) || externalAbortCommands.load(acquire)) {
        // Set all abort flags with proper ordering
        abortDownload.store(true, release);
        abortUnzip.store(true, release);
        abortFileOp.store(true, release);
        abortCommand.store(true, release);
        externalAbortCommands.store(false, release);
        // Reset UI state
        commandSuccess.store(false, release);
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
    
    if (threadFailure.exchange(false, std::memory_order_acq_rel)) {
        commandSuccess.store(false, release);
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
        if (lastSelectedListItem && nextToggleState.empty())
            lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
        inProg = false;
        lastPct = -1;  // Reset lastPct for next cycle
    }
    
    return false;
}




static u64 holdStartTick = 0;
static std::string lastSelectedListItemFooter;
static std::vector<std::vector<std::string>> storedCommands;

bool processHold(uint64_t keysDown, uint64_t keysHeld, u64& holdStartTick, bool& isHolding,
                        std::function<void()> onComplete,
                        std::function<void()> onRelease = nullptr,
                        bool resetStoredCommands = false) {
    if (!lastSelectedListItem) {
        isHolding = false;
        return false;
    }
    if (!(keysHeld & KEY_A)) {
        // Key released — reset everything
        triggerExitFeedback();
        isHolding = false;
        displayPercentage.store(0, std::memory_order_release);
        runningInterpreter.store(false, std::memory_order_release);
        if (lastSelectedListItem) {
            if (resetStoredCommands) {
                // Use lastFooterHighlightDefined to determine the highlight parameter
                // If not defined, infer from lastCommandMode
                bool highlightParam = true;
                if (lastFooterHighlightDefined) {
                    highlightParam = !lastFooterHighlight;
                } else {
                    // Default behavior based on command mode
                    highlightParam = !(lastCommandMode == SLOT_STR || lastCommandMode == OPTION_STR) || lastCommandMode.empty();
                }
                lastSelectedListItem->setValue(lastSelectedListItemFooter, highlightParam);
                lastSelectedListItemFooter.clear();
            } else {
                lastSelectedListItem->setValue("", true);
            }
            lastSelectedListItem = nullptr;
            lastFooterHighlight = lastFooterHighlightDefined = false;
        }
        if (resetStoredCommands) {
            storedCommands.clear();
            lastCommandMode.clear();
            lastCommandIsHold = false;
            lastKeyName.clear();
        }

        if (onRelease) onRelease();
        return true;
    }
    // Directional feedback
    if (keysDown & KEY_UP) lastSelectedListItem->shakeHighlight(tsl::FocusDirection::Up);
    else if (keysDown & KEY_DOWN) lastSelectedListItem->shakeHighlight(tsl::FocusDirection::Down);
    else if (keysDown & KEY_LEFT) lastSelectedListItem->shakeHighlight(tsl::FocusDirection::Left);
    else if (keysDown & KEY_RIGHT) lastSelectedListItem->shakeHighlight(tsl::FocusDirection::Right);
    // Update hold progress
    const u64 elapsedMs = armTicksToNs(armGetSystemTick() - holdStartTick) / 1000000;
    const int percentage = std::min(100, static_cast<int>((elapsedMs * 100) / 5000));
    displayPercentage.store(percentage, std::memory_order_release);
    if (percentage > 20 && (percentage % 30) == 0)
        triggerRumbleDoubleClick.store(true, std::memory_order_release);
    // Completed hold
    if (percentage >= 100) {
        isHolding = false;
        displayPercentage.store(-1, std::memory_order_release);
        if (lastSelectedListItem) {
            lastSelectedListItem->enableClickAnimation();
            lastSelectedListItem->triggerClickAnimation();
            lastSelectedListItem->disableClickAnimation();
        }
        if (onComplete) onComplete();
        //lastSelectedListItem = nullptr;
        return true;
    }
    return true; // Continue holding
}

static std::string returnJumpItemName;
static std::string returnJumpItemValue;

// Forward declaration of the MainMenu class.
class MainMenu;

class UltrahandSettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, dropdownSelection, settingsIniPath;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false;
    std::string languagesVersion = APP_VERSION;
    int MAX_PRIORITY = 20;
    std::string comboLabel;
    //std::string lastSelectedListItemFooter = "";
    //bool notifyRebootIsRequiredNow = false;

    bool exitOnBack = false;
    bool softwareHasUpdated = false;

    bool rightAlignmentState;

    u64 holdStartTick;
    bool isHolding = false;
    //tsl::elm::ListItem* deleteListItem = nullptr;
    bool exitItemFocused = false;

    void addListItem(tsl::elm::List* list, const std::string& title, const std::string& value, const std::string& targetMenu) {
        auto* listItem = new tsl::elm::ListItem(title);
        listItem->setValue(value);
        listItem->setClickListener([listItem, targetMenu](uint64_t keys) {
            if (runningInterpreter.load(acquire))
                return false;

            if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {

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
            
            //const auto overlayNames = getOverlayNames(); // Get all overlay names
            
            for (const auto& overlayName : getOverlayNames()) {
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
            
            //const auto packageNames = getPackageNames(); // Get all package names
            
            for (const auto& packageName : getPackageNames()) {
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
                

                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
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

    
    void addUpdateButton(tsl::elm::List* list, const std::string& title, const std::string& versionLabel) {
        auto* listItem = new tsl::elm::ListItem(title);
        listItem->setValue(versionLabel, true);
        if (isVersionGreaterOrEqual(versionLabel.c_str(), APP_VERSION) && versionLabel != APP_VERSION)
            listItem->setValueColor(tsl::onTextColor);
    
        listItem->setClickListener([this, listItem, title](uint64_t keys) {
            static bool executingCommands = false;
            if (runningInterpreter.load(acquire)) {
                return false;
            } else {
                if (executingCommands && commandSuccess.load(acquire) && title != UPDATE_LANGUAGES) {
                    softwareHasUpdated = true;
                    triggerMenuReload = true;
                }
                executingCommands = false;
            }
    
            if (!(keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                return false;
            }
    
            executingCommands = true;
            isDownloadCommand.store(true, release);
            
            std::vector<std::vector<std::string>> interpreterCommands = {
                {"try:"}
            };
    
            // === UPDATE_ULTRAHAND case ===
            if (title == UPDATE_ULTRAHAND) {
                const bool disableLoaderUpdate = isFile(FLAGS_PATH+"NO_LOADER_UPDATES.flag");
                const bool disableSoundEffectsUpdate = isFile(FLAGS_PATH+"NO_SOUND_EFFECTS_UPDATES.flag");
                const std::string versionLabel = cleanVersionLabel(parseValueFromIniSection((SETTINGS_PATH+"RELEASE.ini"), "Release Info", "latest_version"));
                
                // Lambda to add backup/restore commands
                auto addBackupCommands = [&]() {
                    if (disableSoundEffectsUpdate) {
                        interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"sounds/"});
                        interpreterCommands.push_back({"cp", SOUNDS_PATH, DOWNLOADS_PATH+"sounds/"});
                    }
                    if (disableLoaderUpdate) {
                        interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"420000000007E51A/"});
                        interpreterCommands.push_back({"cp", "/atmosphere/contents/420000000007E51A/", DOWNLOADS_PATH+"420000000007E51A/"});
                    }
                };
                
                auto addRestoreAndLoaderCommands = [&]() {
                    if (disableSoundEffectsUpdate) {
                        interpreterCommands.push_back({"mv", DOWNLOADS_PATH+"sounds/", SOUNDS_PATH});
                    }
                    if (disableLoaderUpdate) {
                        interpreterCommands.push_back({"mv", DOWNLOADS_PATH+"420000000007E51A/", "/atmosphere/contents/420000000007E51A/"});
                    } else if (!isVersionGreaterOrEqual(amsVersion,"1.8.0")) {
                        interpreterCommands.push_back({"download", OLD_NX_OVLLOADER_ZIP_URL, DOWNLOADS_PATH});
                        interpreterCommands.push_back({"unzip", DOWNLOADS_PATH + "nx-ovlloader.zip", ROOT_PATH});
                        interpreterCommands.push_back({"delete", DOWNLOADS_PATH + "nx-ovlloader.zip"});
                    }
                };
                
                auto addVersionUpdate = [&]() {
                    if (!versionLabel.empty()) {
                        interpreterCommands.push_back({"set-json-val", HB_APPSTORE_JSON, "version", versionLabel});
                    }
                };
                
                // Try #1: Update via update.ini
                interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"update.ini"});
                interpreterCommands.push_back({"download-no-retry", LATEST_UPDATER_INI_URL, DOWNLOADS_PATH}); // if fails, move to next try
                addBackupCommands();
                interpreterCommands.push_back({"exec", "update", DOWNLOADS_PATH+"update.ini"});
                interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"update.ini"});
                addRestoreAndLoaderCommands();
                addVersionUpdate();
                
                // Try #2: Update via sdout.zip
                interpreterCommands.push_back({"try:"});
                interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"sdout.zip"});
                interpreterCommands.push_back({"download", ULTRAHAND_REPO_URL + "releases/latest/download/sdout.zip", DOWNLOADS_PATH});
                addBackupCommands();
                interpreterCommands.push_back({"unzip", DOWNLOADS_PATH+"sdout.zip", ROOT_PATH});
                interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"sdout.zip"});
                addRestoreAndLoaderCommands();
                addVersionUpdate();
            } 
            // === UPDATE_LANGUAGES case ===
            else if (title == UPDATE_LANGUAGES) {
                interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"lang.zip"});
                interpreterCommands.push_back({"download", ULTRAHAND_REPO_URL + "releases/latest/download/lang.zip", DOWNLOADS_PATH});
                interpreterCommands.push_back({"unzip", DOWNLOADS_PATH+"lang.zip", LANG_PATH});
                interpreterCommands.push_back({"delete", DOWNLOADS_PATH+"lang.zip"});
            }
    
            runningInterpreter.store(true, release);
            executeInterpreterCommands(std::move(interpreterCommands), "", "");
            listItem->disableClickAnimation();
            listItem->setValue(INPROGRESS_SYMBOL);
            lastSelectedListItem = listItem;
            shiftItemFocus(listItem);
            lastRunningInterpreter.store(true, std::memory_order_release);
            if (lastSelectedListItem)
                lastSelectedListItem->triggerClickAnimation();
            return true;
        });
        list->addItem(listItem);
    }

    // Helper function to create toggle list items
    auto createToggleListItem(tsl::elm::List* list, const std::string& title, bool& state, const std::string& iniKey,
                              const bool invertLogic = false, const bool useReloadMenu = false, const bool useReloadMenu2 = false, const bool isMini = true) {

        auto* toggleListItem = new tsl::elm::ToggleListItem(title, invertLogic ? !state : state, ON, OFF, isMini);
        toggleListItem->setStateChangedListener([this, &state, iniKey, invertLogic, useReloadMenu, useReloadMenu2, listItem = toggleListItem, firstState = std::make_shared<std::optional<bool>>()](bool newState) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItem, tsl::FocusDirection::None);
            
            // Calculate the actual logical state first
            const bool actualState = invertLogic ? !newState : newState;
            
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, actualState ? TRUE_STR : FALSE_STR);

            // Store actualState on first click
            if (!firstState->has_value()) {
                *firstState = actualState;
            }

            if (iniKey == "page_swap") {
                triggerMenuReload = firstState->value() != state;
            //} else if (iniKey == "memory_expansion") {
            //    if (!isFile(EXPANSION_PATH + "nx-ovlloader.zip")) {
            //        if (isVersionGreaterOrEqual(amsVersion,"1.8.0"))
            //            downloadFile(NX_OVLLOADER_ZIP_URL, EXPANSION_PATH);
            //        else
            //            downloadFile(OLD_NX_OVLLOADER_ZIP_URL, EXPANSION_PATH);
            //        downloadPercentage.store(-1, release);
            //    }
            //    if (!isFile(EXPANSION_PATH + "nx-ovlloader+.zip")) {
            //        if (isVersionGreaterOrEqual(amsVersion,"1.8.0"))
            //            downloadFile(NX_OVLLOADER_PLUS_ZIP_URL, EXPANSION_PATH);
            //        else
            //            downloadFile(OLD_NX_OVLLOADER_PLUS_ZIP_URL, EXPANSION_PATH);
            //        downloadPercentage.store(-1, release);
            //    }
            //    if (!isFile(EXPANSION_PATH + "nx-ovlloader.zip") || !isFile(EXPANSION_PATH + "nx-ovlloader+.zip")) {
            //        listItem->setState(loaderTitle == "nx-ovlloader+");
            //    } else {
            //        //executeCommands({
            //        //    //{"try:"},
            //        //    //{"del", EXPANSION_PATH + (actualState ? "nx-ovlloader+/" : "nx-ovlloader/")},
            //        //    {"unzip", EXPANSION_PATH + (actualState ? "nx-ovlloader+.zip" : "nx-ovlloader.zip"), "/"}
            //        //    //{"notify", REBOOT_IS_REQUIRED}
            //        //});
            //        if (unzipFile(EXPANSION_PATH + (actualState ? "nx-ovlloader+.zip" : "nx-ovlloader.zip"), "sdmc:/") && (firstState->value() != state)) {
            //            notifyRebootIsRequiredNow = true;
            //        }
            //    }
            } else if (iniKey == "right_alignment") {
                if (!state) {
                    //const auto [horizontalUnderscanPixels, verticalUnderscanPixels] = tsl::gfx::getUnderscanPixels();
                    tsl::gfx::Renderer::get().setLayerPos(1280-32 - tsl::gfx::getUnderscanPixels().first, 0);
                    ult::layerEdge = (1280-448);
                } else {
                    tsl::gfx::Renderer::get().setLayerPos(0, 0);
                    ult::layerEdge = 0;
                }
            } else if (iniKey == "notifications") {
                if (!state) {
                    if (!ult::isFile(NOTIFICATIONS_FLAG_FILEPATH)) {
                        FILE* file = std::fopen((NOTIFICATIONS_FLAG_FILEPATH).c_str(), "w");
                        if (file) {
                            std::fclose(file);
                        }
                    }
                } else {
                    deleteFileOrDirectory(NOTIFICATIONS_FLAG_FILEPATH);
                }
            } else if (iniKey == "sound_effects") {
                if (actualState) {
                    Audio::initialize();
                } else {
                    Audio::exit();
                }
            }

            state = !state;
            
            if (useReloadMenu) reloadMenu = true;
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
        //tsl::elm::clearFrameCache();
    }

    virtual tsl::elm::Element* createUI() override {
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();
        
        const std::array<const std::string*, 14> defaultLanguagesRepresentation = {
            &ENGLISH, &SPANISH, &FRENCH, &GERMAN, &JAPANESE, &KOREAN, &ITALIAN,
            &DUTCH, &PORTUGUESE, &RUSSIAN, &UKRAINIAN, &POLISH, 
            &SIMPLIFIED_CHINESE, &TRADITIONAL_CHINESE
        };
        const std::vector<std::string> defaultLanguages = {"en", "es", "fr", "de", "ja", "ko", "it", "nl", "pt", "ru", "uk", "pl", "zh-cn", "zh-tw"};
        
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

            addGap(list, 12);

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
                if (defaultLangMode != "en" && !isFile(langFile))  {index++; continue;}

                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(*defaultLanguagesRepresentation[index]);

                listItem->setValue(defaultLangMode);
                if (defaultLangMode == defaulLang) {
                    lastSelectedListItemFooter = defaultLangMode;
                    listItem->setValue(defaultLangMode+" "+CHECKMARK_SYMBOL);
                    //lastSelectedListItem = nullptr;
                    lastSelectedListItem = listItem;
                }
                listItem->setClickListener([skipLang = !isFile(langFile), defaultLangMode, defaulLang, langFile, listItem](uint64_t keys) {
                    //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem, [](auto*){})](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;

                    static bool hasNotTriggeredAnimation = false;

                    if (hasNotTriggeredAnimation) { // needed because of swapTo
                        listItem->triggerClickAnimation();
                        hasNotTriggeredAnimation = false;
                    }
                    static bool triggerClick = false;

                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        triggerClick = true;
                    }
                    if (triggerClick && tsl::elm::s_currentScrollVelocity <= 1.0f && tsl::elm::s_currentScrollVelocity >= -1.0f) {
                        triggerClick = false;
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR, defaultLangMode);
                        reloadMenu = reloadMenu2 = true;
                        parseLanguage(langFile);
                        if (skipLang && defaultLangMode == "en") reinitializeLangVars();
                        if (lastSelectedListItem)
                            lastSelectedListItem->setValue(lastSelectedListItemFooter);
                        if (selectedListItem)
                            selectedListItem->setValue(defaultLangMode);
                        listItem->setValue(defaultLangMode + " " + CHECKMARK_SYMBOL);
                        //lastSelectedListItem = nullptr;
                        lastSelectedListItem = listItem;
                        shiftItemFocus(listItem);
                        //if (lastSelectedListItem)
                        //    lastSelectedListItem->triggerClickAnimation();
                        lastSelectedListItemFooter = defaultLangMode;
                        languageWasChanged.store(true, release);
                        hasNotTriggeredAnimation = true;
                        //executeCommands({
                        //    {"refresh-to", "", CHECKMARK_SYMBOL, FALSE_STR}
                        //});
                        tsl::swapTo<UltrahandSettingsMenu>("languageMenu");

                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
                index++;
            }
        } else if (dropdownSelection == "softwareUpdateMenu") {
            const std::string fullVersionLabel = cleanVersionLabel(parseValueFromIniSection((SETTINGS_PATH+"RELEASE.ini"), "Release Info", "latest_version"));

            if (isVersionGreaterOrEqual(fullVersionLabel.c_str(), APP_VERSION) && fullVersionLabel != APP_VERSION && tsl::notification) {
                tsl::notification->showNow(NOTIFY_HEADER + NEW_UPDATE_IS_AVAILABLE);
            
            }

            addHeader(list, SOFTWARE_UPDATE);
            addUpdateButton(list, UPDATE_ULTRAHAND, fullVersionLabel);
            addUpdateButton(list, UPDATE_LANGUAGES, fullVersionLabel);

            PackageHeader overlayHeader;
            overlayHeader.title = "Ultrahand Overlay";
            overlayHeader.version = APP_VERSION;
            overlayHeader.creator = "ppkantorski";
            overlayHeader.about = "Ultrahand Overlay is a versatile tool that enables you to create and share custom command-based packages.";
            overlayHeader.credits = "Special thanks to B3711, ComplexNarrative, ssky, MasaGratoR, meha, WerWolv, HookedBehemoth and many others. ♥";
            addPackageInfo(list, overlayHeader, OVERLAY_STR);
            overlayHeader.clear();
        
        } else if (dropdownSelection == "systemMenu") {
            
            // Version info formatting - stack allocated, optimal size
            char versionString[32];
            snprintf(versionString, sizeof(versionString), "HOS %sAMS %s", 
                     hosVersion, amsVersion);
            
            const std::string hekateVersion = extractVersionFromBinary("sdmc:/bootloader/update.bin");
            
            addHeader(list, DEVICE_INFO);
            
            SetSysProductModel model = SetSysProductModel_Invalid;
            setsysGetProductModel(&model);
            
            // Switch is optimal for enum - generates jump table on ARMv8-A
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
            
            // Pre-allocate and reuse tableData - avoid repeated allocations
            std::vector<std::vector<std::string>> tableData;
            tableData.reserve(7); // Max size needed is 7 rows
            
            // Cache bootloader string to avoid redundant empty() check
            const std::string hekateStr = hekateVersion.empty() ? UNAVAILABLE_SELECTION : hekateVersion;
            
            tableData = {
                {FIRMWARE, "", versionString},
                {"└ hekate", "", hekateStr},
                {LOCAL_IP, "", getLocalIpAddress()}
            };
            addTable(list, tableData, "", 164, 20, 28, 4);
            
            // Hardware and storage info - clear() is cheap, reuses capacity
            tableData.clear();
            tableData = {
                {HARDWARE, "", modelRev},
                {MEMORY, "", memorySize},
                {"└ " + VENDOR, "", memoryVendor},
                {"└ " + MODEL, "", memoryModel},
                {STORAGE, "", usingEmunand ? "emuMMC" : "sysMMC"},
                {"└ eMMC ", "", getStorageInfo("emmc")},
                {"└ SD Card", "", getStorageInfo("sdmc")}
            };
            addTable(list, tableData, "", 164, 20, 28, 4);
            
            // CPU, GPU, and SOC info
            tableData.clear();
            tableData = {{"", "", "CPU      GPU      SOC"}};
            addTable(list, tableData, "", 163, 9, 3, 0, DEFAULT_STR, "section", "section", RIGHT_STR, true);
            
            tableData.clear();
            
            // Branchless validation check - single bitwise OR instead of 6 comparisons
            if ((cpuSpeedo0 | cpuSpeedo2 | socSpeedo0 | cpuIDDQ | gpuIDDQ | socIDDQ) != 0) {
                tableData = {
                    {"Speedo", "", customAlign(cpuSpeedo0) + " "+DIVIDER_SYMBOL+" " + customAlign(cpuSpeedo2) + " "+DIVIDER_SYMBOL+" " + customAlign(socSpeedo0)},
                    {"IDDQ", "", customAlign(cpuIDDQ) + " "+DIVIDER_SYMBOL+" " + customAlign(gpuIDDQ) + " "+DIVIDER_SYMBOL+" " + customAlign(socIDDQ)}
                };
            } else {
                tableData = {
                    {"Speedo", "", "⋯    "+DIVIDER_SYMBOL+"    ⋯    "+DIVIDER_SYMBOL+"    ⋯  "},
                    {"IDDQ", "", "⋯    "+DIVIDER_SYMBOL+"    ⋯    "+DIVIDER_SYMBOL+"    ⋯  "}
                };
            }
            addTable(list, tableData, "", 164, 20, -2, 4);
            
            //addHeader(list, COMMANDS);

            addGap(list, 33);
            
            // Get system memory info
            u64 RAM_Used_system_u, RAM_Total_system_u;
            svcGetSystemInfo(&RAM_Used_system_u, 1, INVALID_HANDLE, 2);
            svcGetSystemInfo(&RAM_Total_system_u, 0, INVALID_HANDLE, 2);
            
            // Stack buffer for RAM string - optimal size
            char ramString[24];
            const float freeRamMB = static_cast<float>(RAM_Total_system_u - RAM_Used_system_u) / (1024.0f * 1024.0f);
            snprintf(ramString, sizeof(ramString), "%.2f MB %s", freeRamMB, FREE.c_str());
            
            // Nested ternary compiles to conditional select (CSEL) on ARMv8-A - no branches
            const char* ramColor = freeRamMB >= 9.0f ? "healthy_ram" : (freeRamMB >= 3.0f ? "neutral_ram" : "bad_ram");
            
            tableData.clear();
            tableData = {{SYSTEM_RAM, "", ramString}};
            addTable(list, tableData, "", 165+2, 19-2, 19-2, 0, "header", ramColor, DEFAULT_STR, RIGHT_STR, true, true);

            //addGap(list, 12);

            // Memory expansion toggle - single evaluation, short-circuit OR
            //useMemoryExpansion = ult::expandedMemory || 
            //                     (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "memory_expansion") == TRUE_STR);
            //createToggleListItem(list, MEMORY_EXPANSION, useMemoryExpansion, "memory_expansion", false, false, false, false);
            
            // At the top of your function/class, get current heap size
            //OverlayHeapSize currentHeapSize = getCurrentHeapSize();
            
            // Read custom overlay memory from INI
            const std::string customMemoryStr = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, MEMORY_STR, "custom_overlay_memory_MB");
            
            u32 customMemoryMB = 0;
            bool hasIniEntry = false;
            
            if (!customMemoryStr.empty()) {
                // Manual parsing without try-catch
                int parsedValue = 0;
                bool isValid = true;
                
                // Check if string contains only digits
                for (char c : customMemoryStr) {
                    if (c < '0' || c > '9') {
                        isValid = false;
                        break;
                    }
                }
                
                if (isValid && !customMemoryStr.empty()) {
                    parsedValue = std::atoi(customMemoryStr.c_str());
                    // Validate: must be even, greater than 8
                    if (parsedValue > 8 && parsedValue % 2 == 0) {
                        customMemoryMB = static_cast<u32>(parsedValue);
                        heapSizeCache.customSizeMB = customMemoryMB;
                        hasIniEntry = true;
                    }
                }
            }
            
            // Get current heap size in MB
            const u32 currentHeapMB = bytesToMB(static_cast<u64>(currentHeapSize));
            
            // Check if current heap is larger than 8MB but no INI entry exists
            // This handles the case where memory was set but INI was removed/not present
            if (!hasIniEntry && currentHeapMB > 8) {
                customMemoryMB = currentHeapMB;
            }
            
            // Create step descriptions for the trackbar
            std::vector<std::string> heapSizeLabels = {"4 MB", "6 MB", "8 MB"};
            
            // Add custom size if valid (either from INI or current heap > 8MB)
            if (customMemoryMB > 8) {
                heapSizeLabels.push_back(std::to_string(customMemoryMB) + " MB");
            }
            
            // Create the V2 trackbar
            auto* heapTrackbar = new tsl::elm::NamedStepTrackBarV2(
                OVERLAY_MEMORY,
                "",  // Empty packagePath - callback will handle everything
                heapSizeLabels,
                nullptr, nullptr, {}, "",  // No command system needed
                false,   // unlockedTrackbar - can drag immediately
                false   // executeOnEveryTick - only save on release
            );
            
            // Set the initial position based on current heap size
            u8 initialStep = 1; // Default to 6MB (index 1)
            
            if (currentHeapMB == 4) {
                initialStep = 0;
            } else if (currentHeapMB == 6) {
                initialStep = 1;
            } else if (currentHeapMB == 8) {
                initialStep = 2;
            } else if (customMemoryMB > 8 && currentHeapMB == customMemoryMB) {
                initialStep = 3;  // Custom size is always last (index 3 now)
            }
            
            // Track the last MB value the slider was at
            auto lastSliderMB = std::make_shared<u32>(currentHeapMB);
            
            // Create trackers for each notification type
            //auto soundEnabledShown = std::make_shared<bool>(false);
            //auto wallpaperEnabledShown = std::make_shared<bool>(false);
            //auto wallpaperDisabledShown = std::make_shared<bool>(false);
            //auto soundDisabledShown = std::make_shared<bool>(false);
            //auto notEnoughMemoryShown = std::make_shared<bool>(false);
            
            // Use simple callback - gets called when user releases the trackbar
            heapTrackbar->setSimpleCallback([this, freeRamMB, lastSliderMB, customMemoryMB, hasIniEntry](s16 value, s16 index) {
            //heapTrackbar->setSimpleCallback([this, freeRamMB, lastSliderMB, soundEnabledShown, wallpaperEnabledShown, 
            //                                  wallpaperDisabledShown, soundDisabledShown, notEnoughMemoryShown, 
            //                                  customMemoryMB, hasIniEntry](s16 value, s16 index) {
                // Map step index → heap size
                u64 newHeapBytes;
                u32 newMB;
                
                switch (index) {
                    case 0: newHeapBytes = 0x400000;  newMB = 4; break;   // 4MB
                    case 1: newHeapBytes = 0x600000;  newMB = 6; break;   // 6MB
                    case 2: newHeapBytes = 0x800000;  newMB = 8; break;   // 8MB
                    case 3: 
                        // Only allow custom size if it came from INI entry
                        if (hasIniEntry && customMemoryMB > 8) {
                            newHeapBytes = mbToBytes(customMemoryMB);
                            newMB = customMemoryMB;
                        } else {
                            return;  // Invalid or temporary custom size
                        }
                        break;
                    default: return;
                }
                
                OverlayHeapSize newHeapSize = static_cast<OverlayHeapSize>(newHeapBytes);
                
                const u32 oldMB = bytesToMB(static_cast<u64>(currentHeapSize));
                const u32 previousSliderMB = *lastSliderMB;
                
                // If no actual change, do nothing
                if (newMB == previousSliderMB) {
                    return;
                }
                
                // Determine slider direction
                bool isSliderShrinking = (newMB < previousSliderMB);
                bool isSliderGrowing = (newMB > previousSliderMB);
                
                // If growing heap relative to ORIGINAL size, check if we have enough memory
                if (newMB > oldMB) {
                    // Calculate total memory that would be available after freeing current heap
                    const float totalAvailableMB = freeRamMB + static_cast<float>(oldMB);
                    
                    // Safety margin
                    constexpr float SAFETY_MARGIN_MB = 5.3f;
                
                    // Check if new heap size fits in total available memory
                    if (static_cast<float>(newMB) > (totalAvailableMB - SAFETY_MARGIN_MB)) {
                        // Not enough memory - REJECT the change
                        if (tsl::notification) {
                            //if (!*notEnoughMemoryShown) {
                            //    tsl::notification->showNow(NOTIFY_HEADER+"Not enough memory.");
                            //    *notEnoughMemoryShown = true;
                            //}
                            tsl::notification->showNow(NOTIFY_HEADER + NOT_ENOUGH_MEMORY);
                        }
                        setOverlayHeapSize(currentHeapSize);
                        this->exitOnBack = false;
                        *lastSliderMB = newMB;
                        return;
                    }
                }
                
                // Change is allowed (either shrinking or enough memory for growth)
                setOverlayHeapSize(newHeapSize);
                this->exitOnBack = (currentHeapSize != newHeapSize);
                
                // Show feature notifications based on slider direction
                if (tsl::notification) {
                    if (isSliderShrinking) {
                        // Going down - check for disabled features
                        if (previousSliderMB >= 8 && newMB < 8) {
                            // Wallpaper disabled
                            //if (!*wallpaperDisabledShown ) {
                            //    tsl::notification->showNow(NOTIFY_HEADER+"Wallpaper support disabled.", 23);
                            //    *wallpaperDisabledShown = true;
                            //}
                            //*wallpaperEnabledShown = false;
                            tsl::notification->showNow(NOTIFY_HEADER + WALLPAPER_SUPPORT_DISABLED, 23);
                        } else if (previousSliderMB >= 6 && newMB < 6) {
                            // Sound disabled
                            //if (!*soundDisabledShown) {
                            //    tsl::notification->showNow(NOTIFY_HEADER+"Sound support disabled.", 23);
                            //    *soundDisabledShown = true;
                            //}
                            //*soundEnabledShown = false;
                            tsl::notification->showNow(NOTIFY_HEADER + SOUND_SUPPORT_DISABLED, 23);
                        }
                    } else if (isSliderGrowing) {
                        // Going up - check for enabled features
                        if (previousSliderMB < 8 && newMB >= 8) {
                            // Wallpaper enabled
                            //if (!*wallpaperEnabledShown) {
                            //    tsl::notification->showNow(NOTIFY_HEADER+"Wallpaper support enabled.", 23);
                            //    *wallpaperEnabledShown = true;
                            //}
                            //*wallpaperDisabledShown = false;
                            tsl::notification->showNow(NOTIFY_HEADER + WALLPAPER_SUPPORT_ENABLED, 23);
                        } else if (previousSliderMB < 6 && newMB >= 6) {
                            // Sound enabled
                            //if (!*soundEnabledShown) {
                            //    tsl::notification->showNow(NOTIFY_HEADER+"Sound support enabled.", 23);
                            //    *soundEnabledShown = true;
                            //}
                            //*soundDisabledShown = false;
                            tsl::notification->showNow(NOTIFY_HEADER + SOUND_SUPPORT_ENABLED, 23);
                        }
                    }
                }
                
                // Update slider position after successful change
                *lastSliderMB = newMB;
            });
            
            heapTrackbar->setProgress(initialStep);
            heapTrackbar->disableClickAnimation();
            list->addItem(heapTrackbar);

            addGap(list, 12);

            // Add an "Exit Overlay System" menu item
            auto* exitItem = new tsl::elm::ListItem(EXIT_OVERLAY_SYSTEM, "", true);
            exitItem->setValue("", true);
            exitItem->setClickListener([this, exitItem](uint64_t keys) {
                if ((keys & KEY_A) && !(keys & ~KEY_A & ALL_KEYS_MASK)) {

                    if (!isHolding) {
                        isHolding = true;
                        runningInterpreter.store(true, release);
                        exitItem->setValue(INPROGRESS_SYMBOL);
                        lastSelectedListItem = exitItem;
                        holdStartTick = armGetSystemTick();
                        displayPercentage.store(1, std::memory_order_release);
                    }
                    return true;
                }
                return false;
            });
            exitItem->disableClickAnimation();
            list->addItem(exitItem);

            // Reboot required info
            //tableData.clear();
            //tableData = {{"", "", REBOOT_REQUIRED}};
            //addTable(list, tableData, "", 164, 28, 0, 0, DEFAULT_STR, DEFAULT_STR, DEFAULT_STR, RIGHT_STR, true);
        
                
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

                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", DEFAULT_STR);
                    deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                    if (isFile(defaultTheme)) {
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

                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
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

                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
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

                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
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
            createToggleListItem(list, DYNAMIC_COLORS, dynamicWidgetColors, "dynamic_widget_colors");
            createToggleListItem(list, CENTER_ALIGNMENT, centerWidgetAlignment, "center_widget_alignment");
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
            useNotifications = getBoolValue("notifications", true); // TRUE_STR default
            createToggleListItem(list, NOTIFICATIONS, useNotifications, "notifications");

            if (!ult::limitedMemory) {
                useSoundEffects = getBoolValue("sound_effects", false); // TRUE_STR default
                createToggleListItem(list, SOUND_EFFECTS, useSoundEffects, "sound_effects");
            }
            useHapticFeedback = getBoolValue("haptic_feedback", false); // FALSE_STR default
            createToggleListItem(list, HAPTIC_FEEDBACK, useHapticFeedback, "haptic_feedback");
            useOpaqueScreenshots = getBoolValue("opaque_screenshots", true); // TRUE_STR default
            createToggleListItem(list, OPAQUE_SCREENSHOTS, useOpaqueScreenshots, "opaque_screenshots");
            useSwipeToOpen = getBoolValue("swipe_to_open", true); // TRUE_STR default
            createToggleListItem(list, SWIPE_TO_OPEN, useSwipeToOpen, "swipe_to_open");
            rightAlignmentState = useRightAlignment = getBoolValue("right_alignment"); // FALSE_STR default
            createToggleListItem(list, RIGHT_SIDE_MODE, useRightAlignment, "right_alignment");


            addHeader(list, MENU_SETTINGS);
            hideUserGuide = getBoolValue("hide_user_guide", false); // FALSE_STR default
            createToggleListItem(list, USER_GUIDE, hideUserGuide, "hide_user_guide", true, true, true);
            hideHidden = getBoolValue("hide_hidden", false); // FALSE_STR default
            createToggleListItem(list, SHOW_HIDDEN, hideHidden, "hide_hidden", true, true);
            hideDelete = getBoolValue("hide_delete", false); // FALSE_STR default
            createToggleListItem(list, SHOW_DELETE, hideDelete, "hide_delete", true);
            if (requiresLNY2) {
                //hideForceSupport = getBoolValue("hide_force_support", true); // FALSE_STR default
                //createToggleListItem(list, "Show Force Support", hideForceSupport, "hide_force_support", true);
                hideUnsupported = getBoolValue("hide_unsupported", false); // FALSE_STR default
                createToggleListItem(list, SHOW_UNSUPPORTED, hideUnsupported, "hide_unsupported", true, true, true);
            }
            usePageSwap = getBoolValue("page_swap", false); // FALSE_STR default
            createToggleListItem(list, PAGE_SWAP, usePageSwap, "page_swap", false, true);

            hideOverlayVersions = getBoolValue("hide_overlay_versions", false); // FALSE_STR default
            createToggleListItem(list, OVERLAY_VERSIONS, hideOverlayVersions, "hide_overlay_versions", true, true);
            hidePackageVersions = getBoolValue("hide_package_versions", false); // FALSE_STR default
            createToggleListItem(list, PACKAGE_VERSIONS, hidePackageVersions, "hide_package_versions", true, true);
            cleanVersionLabels = getBoolValue("clean_version_labels", false); // FALSE_STR default
            createToggleListItem(list, CLEAN_VERSIONS, cleanVersionLabels, "clean_version_labels", false, true, true);


            addHeader(list, THEME_SETTINGS);
            useDynamicLogo = getBoolValue("dynamic_logo", true); // TRUE_STR default
            createToggleListItem(list, DYNAMIC_LOGO, useDynamicLogo, "dynamic_logo");
            useSelectionBG = getBoolValue("selection_bg", true); // TRUE_STR default
            createToggleListItem(list, SELECTION_BACKGROUND, useSelectionBG, "selection_bg", false, true);
            useSelectionText = getBoolValue("selection_text", false); // TRUE_STR default
            createToggleListItem(list, SELECTION_TEXT, useSelectionText, "selection_text", false, true);
            useSelectionValue = getBoolValue("selection_value", false); // FALSE_STR default
            createToggleListItem(list, SELECTION_VALUE, useSelectionValue, "selection_value", false, true);
            useLibultrahandTitles = getBoolValue("libultrahand_titles", false); // FALSE_STR default
            createToggleListItem(list, LIBULTRAHAND_TITLES, useLibultrahandTitles, "libultrahand_titles", false, true);
            useLibultrahandVersions = getBoolValue("libultrahand_versions", true); // TRUE_STR default
            createToggleListItem(list, LIBULTRAHAND_VERSIONS, useLibultrahandVersions, "libultrahand_versions", false, true);
            usePackageTitles = getBoolValue("package_titles", false); // TRUE_STR default
            createToggleListItem(list, PACKAGE_TITLES, usePackageTitles, "package_titles", false, true);
            usePackageVersions = getBoolValue("package_versions", true); // TRUE_STR default
            createToggleListItem(list, PACKAGE_VERSIONS, usePackageVersions, "package_versions", false, true);

            //addHeader(list, "Version Labels");

        } else {
            addBasicListItem(list, FAILED_TO_OPEN + ": " + settingsIniPath);
        }

        auto* rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        if (inSubSettingsMenu && ((dropdownSelection == "languageMenu") || (dropdownSelection == KEY_COMBO_STR) || (dropdownSelection == "themeMenu") || (dropdownSelection == "wallpaperMenu"))) {
            {
                //std::lock_guard<std::mutex> lock(jumpItemMutex);
                jumpItemName = "";
                jumpItemValue = CHECKMARK_SYMBOL;
                jumpItemExactMatch.store(false, release);
                //g_overlayFilename = "";
            }
            //list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
        } else {
            if (languageWasChanged.exchange(false, std::memory_order_acq_rel)) {
                {
                    //std::lock_guard<std::mutex> lock(jumpItemMutex);
                    jumpItemName = LANGUAGE;
                    jumpItemValue = "";
                    jumpItemExactMatch.store(true, release);
                    //g_overlayFilename = "";
                }
                //languageWasChanged.store(false, release);
                //list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
            } else if (themeWasChanged) {
                {
                    //std::lock_guard<std::mutex> lock(jumpItemMutex);
                    jumpItemName = THEME;
                    jumpItemValue = "";
                    jumpItemExactMatch.store(true, release);
                    //g_overlayFilename = "";
                }
                themeWasChanged = false;
                //list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
            } else {
                {
                    //std::lock_guard<std::mutex> lock(jumpItemMutex);
                    jumpItemName = "";
                    jumpItemValue = "";
                    jumpItemExactMatch.store(true, release);
                    //g_overlayFilename = "";
                }
                //list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
            }
        }
        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));

        rootFrame->setContent(list);
        return rootFrame;

        //return returnRootFrame(list, CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        // Handle delete item continuous hold behavior
        if (isHolding) {
            processHold(keysDown, keysHeld, holdStartTick, isHolding, [this]() {
                if (requestOverlayExit()) {
                    ult::launchingOverlay.store(true, std::memory_order_release);
                    tsl::Overlay::get()->close();
                }
            });
        }

        const bool isRunningInterp = runningInterpreter.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
        
        if (lastRunningInterpreter.exchange(false, std::memory_order_acq_rel)) {
            isDownloadCommand.store(false, release);
            if (lastSelectedListItem) {
                lastSelectedListItem->setValue(commandSuccess.load(acquire) ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
                lastSelectedListItem->enableClickAnimation();
                lastSelectedListItem = nullptr;
            }

            closeInterpreterThread();
            resetPercentages();

            if (!commandSuccess.load()) {
                triggerRumbleDoubleClick.store(true, std::memory_order_release);
            }

            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
            //lastRunningInterpreter.store(false, std::memory_order_release);
            return true;
        }
        
        if (goBackAfter.exchange(false, std::memory_order_acq_rel)) {
            disableSound.store(true, std::memory_order_release);
            simulatedBack.store(true, std::memory_order_release);
            return true;
        }

        //if (refreshPage.load(acquire)) {
        //    //tsl::goBack();tsl::changeTo<UltrahandSettingsMenu>(targetMenu);
        //    tsl::swapTo<UltrahandSettingsMenu>("languageMenu");
        //    refreshPage.store(false, release);
        //}
        
        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                //if (simulatedNextPage.load(acquire))
                //    simulatedNextPage.store(false, release);
                //if (simulatedMenu.load(acquire))
                //    simulatedMenu.store(false, release);
                simulatedNextPage.exchange(false, std::memory_order_acq_rel);
                simulatedMenu.exchange(false, std::memory_order_acq_rel);
                
                const bool isTouching = stillTouching.load(acquire);
                const bool backKeyPressed = !isTouching && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
                
                if (backKeyPressed) {

                    allowSlide.exchange(false, std::memory_order_acq_rel);
                    unlockedSlide.exchange(false, std::memory_order_acq_rel);
                    inSettingsMenu = false;
                    returningToMain = (lastMenu != "hiddenMenuMode");
                    returningToHiddenMain = !returningToMain;
                    lastMenu = "settingsMenu";
                    
                    if (reloadMenu) {
                        //sl::pop(2);
                        tsl::swapTo<MainMenu>(SwapDepth(3), lastMenuMode);
                        reloadMenu = false;
                    } else {
                        tsl::goBack();
                    }
                    return true;
                }
            } else {
                returningToSettings = false;
            }
        } else if (inSubSettingsMenu) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            const bool isTouching = stillTouching.load(acquire);
            const bool backKeyPressed = !isTouching && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
            
            if (backKeyPressed) {
                if (exitOnBack) {
                    ult::launchingOverlay.store(true, std::memory_order_release);
                    tsl::Overlay::get()->close();
                    return true;
                }

                allowSlide.exchange(false, std::memory_order_acq_rel);
                unlockedSlide.exchange(false, std::memory_order_acq_rel);
                inSubSettingsMenu = false;
                returningToSettings = true;
                
                if (reloadMenu2) {
                    //tsl::goBack(2);
                    tsl::swapTo<UltrahandSettingsMenu>(SwapDepth(3));
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
        
        if (triggerExit.exchange(false, std::memory_order_acq_rel)) {
            ult::launchingOverlay.store(true, std::memory_order_release);

            if (softwareHasUpdated && requestOverlayExit()) {
            } else {
                tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            }
            tsl::Overlay::get()->close(); // Close the overlay
        }
        
        return false;
    }
};



int settingsMenuPageDepth = 0;
std::string rootEntryName, rootEntryMode, rootTitle, rootVersion;

bool modeComboModified = false;

class SettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, title, version, dropdownSelection, settingsIniPath;
    
    bool requiresAMS110Handling = false;

    bool isInSection, inQuotes, isFromMainMenu;
    int MAX_PRIORITY = 20;

    std::string modeTitle;

    u64 holdStartTick;
    bool isHolding = false;
    //tsl::elm::ListItem* deleteListItem = nullptr;
    bool deleteItemFocused = false;

public:
    SettingsMenu(const std::string& name, const std::string& mode, const std::string& title = "", const std::string& version = "", const std::string& selection = "", bool _requiresAMS110Handling = false)
        : entryName(name), entryMode(mode), title(title), version(version), dropdownSelection(selection), requiresAMS110Handling(_requiresAMS110Handling) {
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
        lastSelectedListItem = nullptr;
        //tsl::elm::clearFrameCache();
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

        //auto forceSupportNotificationShown = std::make_shared<bool>(false);

        auto* toggleListItem = new tsl::elm::ToggleListItem(label, initialState, ON, OFF);
        toggleListItem->setState(currentValue != FALSE_STR);
        toggleListItem->setStateChangedListener([this, iniKey, listItem = toggleListItem, handleReload](bool state) {
        //toggleListItem->setStateChangedListener([this, iniKey, listItem = toggleListItem, handleReload, forceSupportNotificationShown](bool state) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItem, tsl::FocusDirection::None);
            setIniFileValue(this->settingsIniPath, this->entryName, iniKey, state ? TRUE_STR : FALSE_STR);
            if (handleReload) {
                reloadMenu = state || (reloadMenu2 = !state);
            }
            if (iniKey == "force_support") {
                if (state && tsl::notification) {
                    // First time for THIS notification OR wait until not active
                    //if (!*forceSupportNotificationShown) {
                    //    tsl::notification->showNow(NOTIFY_HEADER+FORCED_SUPPORT_WARNING, 20);
                    //    *forceSupportNotificationShown = true;
                    //}
                    tsl::notification->showNow(NOTIFY_HEADER+FORCED_SUPPORT_WARNING, 20);
                }
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


            if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
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
    
    void addDeleteItem(tsl::elm::List* list, bool isOverlay) {
        //static const std::vector<std::vector<std::string>> tableData = {
        //    {"", "", ""}
        //};
        //addTable(list, tableData, "", 165, 0, 10, 0, DEFAULT_STR, DEFAULT_STR, DEFAULT_STR, RIGHT_STR, true, false, false, true, "none", false);

        addGap(list, 12);

        auto* deleteListItem = new tsl::elm::ListItem(isOverlay ? DELETE_OVERLAY : DELETE_PACKAGE);
        deleteListItem->setValue("", true);
        
        deleteListItem->setClickListener([this, deleteListItem](uint64_t keys) -> bool {
            if (runningInterpreter.load(std::memory_order_acquire))
                return false;
    
            // Start holding when A key is first pressed
            if ((keys & KEY_A) && !(keys & ~KEY_A & ALL_KEYS_MASK)) {
                if (!isHolding) {
                    isHolding = true;
                    runningInterpreter.store(true, release);
                    deleteListItem->setValue(INPROGRESS_SYMBOL);
                    lastSelectedListItem = deleteListItem;
                    holdStartTick = armGetSystemTick();
                    displayPercentage.store(1, std::memory_order_release);
                }
                return true;
            }
            
            return false;
        });
        deleteListItem->disableClickAnimation();
        
        list->addItem(deleteListItem);
    }
    
    // Helper lambdas and common setup
    virtual tsl::elm::Element* createUI() override {
        settingsIniPath = (entryMode == OVERLAY_STR) ? OVERLAYS_INI_FILEPATH : PACKAGES_INI_FILEPATH;
        const std::string header = title + (!version.empty() ? (" " + version) : "");
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();
        
        // Load settings INI once
        auto settingsData = getParsedDataFromIniFile(settingsIniPath);
        auto secIt = settingsData.find(entryName);
        
        // Optimized value getter
        auto getValue = [&](const std::string& key) -> std::string {
            return (secIt != settingsData.end() && secIt->second.count(key)) 
                ? secIt->second.at(key) : "";
        };
        
        // Common click handler for navigation
        auto navClick = [this](const std::string& name, const std::string& mode, 
                              const std::string& overlayName, const std::string& overlayVersion,
                              const std::string& selection, tsl::elm::ListItem* item) {
            return [=](uint64_t keys) -> bool {
                if (runningInterpreter.load(acquire)) return false;
                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                    inMainMenu.store(false, std::memory_order_release);
                    tsl::changeTo<SettingsMenu>(name, mode, overlayName, overlayVersion, selection);
                    selectedListItem = item;
                    if (lastSelectedListItem) lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            };
        };
        
        auto* list = new tsl::elm::List();
    
        if (inSettingsMenu) {
            addHeader(list, SETTINGS + " "+DIVIDER_SYMBOL+" " + header);
    
            // Key Combo Item
            {
                const std::string currentCombo = getValue(KEY_COMBO_STR);
                std::string displayCombo = currentCombo.empty() ? OPTION_SYMBOL : currentCombo;
                if (!currentCombo.empty()) convertComboToUnicode(displayCombo);
                
                auto* item = new tsl::elm::ListItem(KEY_COMBO);
                item->setValue(displayCombo);
                item->setClickListener(navClick(entryName, entryMode, title, version, KEY_COMBO_STR, item));
                list->addItem(item);
            }
    
            // Hide Toggle
            createAndAddToggleListItem(list, (entryMode == OVERLAY_STR) ? HIDE_OVERLAY : HIDE_PACKAGE,
                false, HIDE_STR, getValue(HIDE_STR), settingsIniPath, entryName, true);
    
            // Sort Priority Item
            {
                auto* item = new tsl::elm::ListItem(SORT_PRIORITY);
                item->setValue(getValue(PRIORITY_STR));
                item->setClickListener(navClick(entryName, entryMode, title, version, PRIORITY_STR, item));
                list->addItem(item);
            }
    
            // Mode-specific items
            if (entryMode == OVERLAY_STR) {
                createAndAddToggleListItem(list, LAUNCH_ARGUMENTS, false, USE_LAUNCH_ARGS_STR,
                    getValue(USE_LAUNCH_ARGS_STR), settingsIniPath, entryName);
    
                const std::vector<std::string> modeList = splitIniList(getValue("mode_args"));
                if (!modeList.empty()) {
                    auto* item = new tsl::elm::ListItem(LAUNCH_MODES);
                    item->setValue(DROPDOWN_SYMBOL);
                    item->setClickListener(navClick(entryName, entryMode, title, version, MODE_STR, item));
                    list->addItem(item);
                }
                //if (!hideForceSupport && requiresLNY2 && requiresAMS110Handling) {
                if (requiresLNY2 && requiresAMS110Handling) {
                    createAndAddToggleListItem(list, FORCE_AMS110_SUPPORT,
                        false, "force_support", getValue("force_support"), settingsIniPath, entryName, true);
                }
            } else if (entryMode == PACKAGE_STR) {
                auto* item = new tsl::elm::ListItem(OPTIONS);
                item->setValue(DROPDOWN_SYMBOL);
                item->setClickListener(navClick(entryName, entryMode, title, version, "options", item));
                list->addItem(item);
            }
    
            if (!hideDelete) addDeleteItem(list , (entryMode == OVERLAY_STR));
            
        } else if (dropdownSelection == MODE_STR) {
            const std::vector<std::string> modeList = splitIniList(getValue("mode_args"));
            const std::vector<std::string> comboList = splitIniList(getValue("mode_combos"));
            const std::vector<std::string> labelList = splitIniList(getValue("mode_labels"));
            
            if (!modeList.empty()) {
                addTable(list, {{MODE, "", KEY_COMBO}}, "", 165+2, 19-2, 19-2, 0, 
                        "header", "header", DEFAULT_STR, RIGHT_STR, true, true, false, true, "none", false);
                
                std::vector<std::string> combos = comboList;
                if (combos.size() < modeList.size()) combos.resize(modeList.size(), "");
                
                for (size_t i = 0; i < modeList.size(); ++i) {
                    const std::string& mode = modeList[i];
                    const std::string displayName = (i < labelList.size() && !labelList[i].empty()) ? labelList[i] : mode;
                    std::string comboDisplay = combos[i].empty() ? OPTION_SYMBOL : combos[i];
                    convertComboToUnicode(comboDisplay);
                    
                    auto* item = new tsl::elm::ListItem(displayName);
                    item->setValue(comboDisplay);
                    item->setClickListener([=, this](uint64_t keys) -> bool {
                        if (runningInterpreter.load(acquire)) return false;
                        if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                            inMainMenu.store(false, std::memory_order_release);
                            tsl::changeTo<SettingsMenu>(entryName, OVERLAY_STR, mode, "", "mode_combo_" + std::to_string(i));
                            selectedListItem = item;
                            if (lastSelectedListItem) lastSelectedListItem->triggerClickAnimation();
                            return true;
                        }
                        return false;
                    });
                    list->addItem(item);
                }
            }
            
        } else if (dropdownSelection == "options") {
            addHeader(list, "Options");
            createAndAddToggleListItem(list, QUICK_LAUNCH, false, USE_QUICK_LAUNCH_STR, getValue(USE_QUICK_LAUNCH_STR), settingsIniPath, entryName);
            createAndAddToggleListItem(list, BOOT_COMMANDS, true, USE_BOOT_PACKAGE_STR, getValue(USE_BOOT_PACKAGE_STR), settingsIniPath, entryName);
            createAndAddToggleListItem(list, EXIT_COMMANDS, true, USE_EXIT_PACKAGE_STR, getValue(USE_EXIT_PACKAGE_STR), settingsIniPath, entryName);
            createAndAddToggleListItem(list, ERROR_LOGGING, false, USE_LOGGING_STR, getValue(USE_LOGGING_STR), settingsIniPath, entryName);
            
        } else if (dropdownSelection == PRIORITY_STR) {
            addHeader(list, SORT_PRIORITY);
            const std::string priorityValue = getValue(PRIORITY_STR);
            for (int i = 0; i <= MAX_PRIORITY; ++i) {
                createAndAddListItem(list, ult::to_string(i), priorityValue, settingsIniPath, entryName, true);
            }
            
        } else if (dropdownSelection == KEY_COMBO_STR) {
            addHeader(list, KEY_COMBO);
            const std::string currentCombo = getValue(KEY_COMBO_STR);
            
            // Get global default combo
            auto uhData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto uhIt = uhData.find(ULTRAHAND_PROJECT_NAME);
            std::string globalDefault = (uhIt != uhData.end() && uhIt->second.count(KEY_COMBO_STR))
                ? uhIt->second.at(KEY_COMBO_STR) : "";
            trim(globalDefault);
            
            // No combo option
            {
                auto* item = new tsl::elm::ListItem(OPTION_SYMBOL);
                if (currentCombo.empty()) {
                    item->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = item;
                }
                item->setClickListener([=, this](uint64_t keys) -> bool {
                    if (runningInterpreter.load(acquire)) return false;
                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        setIniFileValue(settingsIniPath, entryName, KEY_COMBO_STR, "");
                        tsl::hlp::loadEntryKeyCombos();
                        reloadMenu2 = true;
                        if (lastSelectedListItem) lastSelectedListItem->setValue("");
                        if (selectedListItem) selectedListItem->setValue(OPTION_SYMBOL);
                        item->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = item;
                        shiftItemFocus(item);
                        if (lastSelectedListItem) lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(item);
            }
    
            // Predefined combos
            std::string mapped;
            for (const auto& combo : defaultCombos) {
                if (combo == globalDefault) continue;
                
                mapped = combo;
                convertComboToUnicode(mapped);
                
                auto* item = new tsl::elm::ListItem(mapped);
                if (combo == currentCombo) {
                    item->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = item;
                }
                item->setClickListener([=, this](uint64_t keys) -> bool {
                    if (runningInterpreter.load(acquire)) return false;
                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        if (combo != currentCombo) {
                            removeKeyComboFromOthers(combo, entryName);
                            setIniFileValue(settingsIniPath, entryName, KEY_COMBO_STR, combo);
                            tsl::hlp::loadEntryKeyCombos();
                        }
                        reloadMenu2 = true;
                        if (lastSelectedListItem) lastSelectedListItem->setValue("");
                        if (selectedListItem) selectedListItem->setValue(mapped);
                        item->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = item;
                        shiftItemFocus(item);
                        if (lastSelectedListItem) lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(item);
            }
            
        } else if (dropdownSelection.rfind("mode_combo_", 0) == 0) {
            const size_t idx = std::stoi(dropdownSelection.substr(11)); // 11 = strlen("mode_combo_")
            
            const std::vector<std::string> labelList = splitIniList(getValue("mode_labels"));
            const std::string labelText = (idx < labelList.size() && !labelList[idx].empty()) ? 
                                   labelList[idx] : "'" + title + "'";
            modeTitle = (idx < labelList.size() && !labelList[idx].empty()) ? labelList[idx] : title;
            
            addHeader(list, KEY_COMBO + " "+DIVIDER_SYMBOL+" " + labelText);
    
            std::vector<std::string> comboList = splitIniList(getValue("mode_combos"));
            if (idx >= comboList.size()) comboList.resize(idx + 1, "");
            const std::string currentCombo = comboList[idx];
    
            // Get global default
            auto uhData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto uhIt = uhData.find(ULTRAHAND_PROJECT_NAME);
            std::string globalDefault = (uhIt != uhData.end() && uhIt->second.count(KEY_COMBO_STR))
                ? uhIt->second.at(KEY_COMBO_STR) : "";
            trim(globalDefault);
    
            // No combo option
            {
                auto* item = new tsl::elm::ListItem(OPTION_SYMBOL);
                if (currentCombo.empty()) {
                    item->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = item;
                }
                item->setClickListener([=, this](uint64_t keys) mutable -> bool {
                    if (runningInterpreter.load(acquire)) return false;
                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        auto combos = comboList; // copy for modification
                        combos[idx] = "";
                        const std::string newStr = "(" + joinIniList(combos) + ")";
                        removeKeyComboFromOthers(newStr, entryName);
                        setIniFileValue(settingsIniPath, entryName, "mode_combos", newStr);
                        tsl::hlp::loadEntryKeyCombos();
                        modeComboModified = true;
                        if (lastSelectedListItem) lastSelectedListItem->setValue("");
                        if (selectedListItem) selectedListItem->setValue(OPTION_SYMBOL);
                        item->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = item;
                        shiftItemFocus(item);
                        if (lastSelectedListItem) lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(item);
            }
    
            // Valid combos
            std::string mapped;
            for (const auto& combo : defaultCombos) {
                if (combo == globalDefault) continue;
                
                mapped = combo;
                convertComboToUnicode(mapped);
                
                auto* item = new tsl::elm::ListItem(mapped);
                if (combo == currentCombo) {
                    item->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = item;
                }
                item->setClickListener([=, this](uint64_t keys) mutable -> bool {
                    if (runningInterpreter.load(acquire)) return false;
                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        if (combo != comboList[idx]) {
                            removeKeyComboFromOthers(combo, entryName);
                            const std::string comboStr = parseValueFromIniSection(settingsIniPath, entryName, "mode_combos");
                            auto combos = splitIniList(comboStr);
                            if (idx >= combos.size()) combos.resize(idx + 1, "");
                            combos[idx] = combo;
                            setIniFileValue(settingsIniPath, entryName, "mode_combos", "(" + joinIniList(combos) + ")");
                            tsl::hlp::loadEntryKeyCombos();
                        }
                        modeComboModified = true;
                        if (lastSelectedListItem) lastSelectedListItem->setValue("");
                        if (selectedListItem) selectedListItem->setValue(mapped);
                        item->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = item;
                        shiftItemFocus(item);
                        if (lastSelectedListItem) lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(item);
            }
        } else {
            addBasicListItem(list, FAILED_TO_OPEN + ": " + settingsIniPath);
        }
    
        auto* rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        
        if (inSubSettingsMenu && (dropdownSelection == KEY_COMBO_STR || dropdownSelection == PRIORITY_STR || 
                                 dropdownSelection.rfind("mode_combo_", 0) == 0)) {
            jumpItemName = "";
            jumpItemValue = CHECKMARK_SYMBOL;
            jumpItemExactMatch.store(true, release);
            //g_overlayFilename = "";
        }
        
        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
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
        
        //const bool isRunningInterp = runningInterpreter.load(acquire);
        //
        //if (isRunningInterp) {
        //    return handleRunningInterpreter(keysDown, keysHeld);
        //}
        //
        //if (lastRunningInterpreter.load(acquire)) {
        //    isDownloadCommand.store(false, release);
        //    if (lastSelectedListItem)
        //        lastSelectedListItem->setValue(commandSuccess.load(acquire) ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
        //    closeInterpreterThread();
        //    lastRunningInterpreter.store(false, std::memory_order_release);
        //    return true;
        //}
        
        if (goBackAfter.exchange(false, std::memory_order_acq_rel)) {
            disableSound.store(true, std::memory_order_release);
            simulatedBack.store(true, std::memory_order_release);
            return true;
        }

        static bool runAfter = false;
        if (runAfter) {
            runAfter = false;
            // Reset navigation state properly
            inSettingsMenu = false;
            inSubSettingsMenu = false;
            
            // Clear the current selection
            ////g_overlayFilename = "";
            //entryName = "";
            
            // Determine return destination
            if (lastMenu != "hiddenMenuMode")
                returningToMain = true;
            else
                returningToHiddenMain = true;
            
            // Determine pop count and hidden mode settings
            int popCount;
            if (lastMenu == "hiddenMenuMode") {
                popCount = 3;
                inMainMenu.store(false, std::memory_order_release);
                inHiddenMode.store(true, std::memory_order_release);
                if (entryMode == OVERLAY_STR)
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, TRUE_STR);
                else
                    popCount = 2;
            } else {
                popCount = 2;
            }
            
            runningInterpreter.store(false, release);
            jumpItemName = rootTitle;
            jumpItemValue = rootVersion;
            //g_overlayFilename = "";
            jumpItemExactMatch.store(false, release);
            skipJumpReset.store(true, release);
            
            tsl::swapTo<MainMenu>(SwapDepth(popCount), lastMenuMode);
            return true;
        }

        // Handle delete item continuous hold behavior
        if (isHolding) {
            processHold(keysDown, keysHeld, holdStartTick, isHolding, [this]() {
                std::string targetPath;
                bool hasTarget = false;
        
                if (!entryName.empty() && entryMode == OVERLAY_STR) {
                    targetPath = OVERLAY_PATH + entryName;
                    hasTarget = true;
                } else if (!entryName.empty()) {
                    targetPath = PACKAGE_PATH + entryName + "/";
                    hasTarget = true;
                }
        
                if (hasTarget) {
                    deleteFileOrDirectory(targetPath);
                    removeIniSection(settingsIniPath, entryName);
        
                    if (lastSelectedListItem) {
                        //lastSelectedListItem->enableClickAnimation();
                        //lastSelectedListItem->triggerClickAnimation();
                        lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
                        //lastSelectedListItem->disableClickAnimation();
                        lastSelectedListItem = nullptr;
                    }
        
                    triggerRumbleDoubleClick.store(true, std::memory_order_release);
                    triggerMoveSound.store(true, std::memory_order_release);
                    runAfter = true;
                } else if (lastSelectedListItem) {
                    lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
                    lastSelectedListItem = nullptr;
                }
            }, nullptr, false); // false = do NOT reset storedCommands
        }


        if (inSettingsMenu && !inSubSettingsMenu) {
            if (!returningToSettings) {
                if (simulatedNextPage.load(acquire))
                    simulatedNextPage.store(false, release);
                if (simulatedMenu.load(acquire))
                    simulatedMenu.store(false, release);

                const bool isTouching = stillTouching.load(acquire);
                const bool backKeyPressed = !isTouching && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
    
                // Note: Original code uses !stillTouching without .load() - preserving this exactly
                if (backKeyPressed) {
                    if (allowSlide.load(acquire))
                        allowSlide.store(false, release);
                    if (unlockedSlide.load(acquire))
                        unlockedSlide.store(false, release);
                    inSettingsMenu = false;
                    
                    // Determine return destination
                    if (lastMenu != "hiddenMenuMode")
                        returningToMain = true;
                    else
                        returningToHiddenMain = true;
                    
                    if (reloadMenu) {
                        reloadMenu = false;
                        
                        // Determine pop count and hidden mode settings
                        size_t popCount;
                        if (lastMenu == "hiddenMenuMode") {
                            popCount = 3;
                            inMainMenu.store(false, std::memory_order_release);
                            inHiddenMode.store(true, std::memory_order_release);
                            if (entryMode == OVERLAY_STR)
                                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, TRUE_STR);
                            else
                                popCount = 2;
                        } else {
                            popCount = 2;
                        }
                        
                        //tsl::pop(popCount);
                        //{
                        //    //std::lock_guard<std::mutex> lock(jumpItemMutex);
                        jumpItemName = rootTitle;
                        jumpItemValue = rootVersion;
                        //g_overlayFilename = "";
                        jumpItemExactMatch.store(false, release);
                        skipJumpReset.store(true, release);
                        //}
                        
                        tsl::swapTo<MainMenu>(SwapDepth(popCount), lastMenuMode);
                    } else {
                        tsl::goBack();
                    }
                    
                    lastMenu = "settingsMenu";
                    return true;
                }
            }
        } else if (inSubSettingsMenu) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            // Note: Original code uses stillTouching.load() here - preserving this difference
            const bool isTouching = stillTouching.load(acquire);
            const bool backKeyPressed = !isTouching && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
            
            
            if (backKeyPressed) {
                allowSlide.exchange(false, std::memory_order_acq_rel);
                unlockedSlide.exchange(false, std::memory_order_acq_rel);
                

                if (dropdownSelection == MODE_STR) {
                    modeTitle = LAUNCH_MODES;
                    reloadMenu2 = true;
                }


                else if (dropdownSelection.rfind("mode_combo_", 0) != 0) {
                    inSubSettingsMenu = false;
                    returningToSettings = true;
                }
    
                // Step 1: Go back one menu level
                // Step 2: If reload is needed, change to SettingsMenu with focus
                if (reloadMenu2) {
                    reloadMenu2 = false;
                    //tsl::goBack(2);
                    
                    //{
                    //    //std::lock_guard<std::mutex> lock(jumpItemMutex);
                    // Provide jump target context
                    jumpItemName = modeTitle;
                    jumpItemValue = "";
                    jumpItemExactMatch.store(true, release);
                    //g_overlayFilename = "";
                    //}
    
                    tsl::swapTo<SettingsMenu>(
                        SwapDepth(2),
                        rootEntryName,
                        rootEntryMode,
                        rootTitle,
                        rootVersion
                    );
                } else {
                    if (modeComboModified) {
                        // Go back to MODE_STR screen to refresh the combo display
                        jumpItemName = modeTitle;  // Focus on the mode that was just edited
                        jumpItemValue = "";
                        jumpItemExactMatch.store(true, release);
                        //g_overlayFilename = "";
                        
                        tsl::swapTo<SettingsMenu>(
                            SwapDepth(2),
                            rootEntryName,
                            rootEntryMode,
                            rootTitle,
                            rootVersion,
                            MODE_STR  // Go back to the modes list screen
                        );
                        // Keep modeComboModified true for the next level up
                    } else {
                        tsl::goBack();
                    }
                }

                if (modeComboModified) {
                    modeComboModified = false;
                    //jumpItemName = MODES;
                    //jumpItemValue = "";
                    //jumpItemExactMatch.store(true, release);
                    ////g_overlayFilename = "";

                    //reloadMenu2 = true;
                }
    
                return true;
            }
        }
        
        if (returningToSettings && !(keysDown & KEY_B)) {
            returningToSettings = false;
            inSettingsMenu = true;
        }
    
        if (triggerExit.exchange(false, std::memory_order_acq_rel)) {
            ult::launchingOverlay.store(true, std::memory_order_release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        return false;
    }
};



// For persistent versions and colors across nested packages (when not specified)
std::string packageRootLayerTitle;
std::string packageRootLayerName;
std::string packageRootLayerVersion;
std::string packageRootLayerColor;
//bool packageRootLayerIsStarred = false;

//std::string lastPackageHeader;

bool overrideTitle = false, overrideVersion = false;





class ScriptOverlay : public tsl::Gui {
private:
    std::vector<std::vector<std::string>> commands;
    std::string filePath, specificKey;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false, isFromPackage = false, isFromSelectionMenu = false;
    bool tableMode = false;

    std::string lastPackageHeader;
    bool showWidget = false;

    //u64 holdStartTick;
    //bool isHolding = false;

    void addListItem(tsl::elm::List* list, const std::string& line) {
        auto* listItem = new tsl::elm::ListItem(line);

        listItem->setClickListener([filePath=filePath, specificKey=specificKey, listItem, line](uint64_t keys) {
            if (runningInterpreter.load(acquire)) return false;

            if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                
                std::vector<std::vector<std::string>> commandVec;
                std::vector<std::string> commandParts;
                std::string currentPart;
                bool inQuotes = false;

                for (char ch : line) {
                    if (ch == '\'') {
                        inQuotes = !inQuotes;
                        if (!inQuotes) {
                            commandParts.emplace_back(std::move(currentPart));
                            //currentPart.clear();
                            currentPart.shrink_to_fit();
                        }
                    } else if (ch == ' ' && !inQuotes) {
                        if (!currentPart.empty()) {
                            commandParts.emplace_back(std::move(currentPart));
                            //currentPart.clear();
                            currentPart.shrink_to_fit();
                        }
                    } else {
                        currentPart += ch;
                    }
                }
                if (!currentPart.empty()) {
                    commandParts.emplace_back(std::move(currentPart));
                }

                commandVec.emplace_back(std::move(commandParts));
                commandParts.shrink_to_fit();

                executeInterpreterCommands(std::move(commandVec), filePath, specificKey);
                listItem->disableClickAnimation();
                //startInterpreterThread();
                listItem->setValue(INPROGRESS_SYMBOL);

                lastSelectedListItem = listItem;
               
                lastRunningInterpreter.store(true, std::memory_order_release);
                listItem->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem);
    }

public:
    ScriptOverlay(std::vector<std::vector<std::string>>&& cmds, const std::string& file, const std::string& key = "",
        const std::string& fromMenu = "", bool tableMode = false, const std::string& _lastPackageHeader = "", bool showWidget = false)
        : commands(cmds), filePath(file), specificKey(key), tableMode(tableMode), lastPackageHeader(_lastPackageHeader), showWidget(showWidget) {

            isFromMainMenu = (fromMenu == "main");
            isFromPackage = (fromMenu == "package");
            isFromSelectionMenu = (fromMenu == "selection");
            triggerRumbleClick.store(true, std::memory_order_release);
            triggerSettingsSound.store(true, std::memory_order_release);
        }

    ~ScriptOverlay() {
        // reset cache (script overlay doesnt use it)
        //tsl::elm::clearFrameCache();
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
                    addHeader(list, specificKey+" "+DIVIDER_SYMBOL+" "+"Try"+" #"+ult::to_string(tryCount));
                    continue;
                }
                if (command[0] == "on:") {
                    index++;
                    addHeader(list, specificKey+" "+DIVIDER_SYMBOL+" "+ON);
                    continue;
                }
                if (command[0] == "off:") {
                    index++;
                    addHeader(list, specificKey+" "+DIVIDER_SYMBOL+" "+OFF);
                    continue;
                }
                combinedCommand = joinCommands(command); // Join commands into a single line for display
                addListItem(list, combinedCommand);
                index++;
                command = {};
                //command.clear();
                //command.shrink_to_fit();
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
                command = {};
                //command.clear();
                //command.shrink_to_fit();
            }
            
            // Use default parameters for the table view
            constexpr size_t tableColumnOffset = 163;
            constexpr size_t tableStartGap = 20;
            constexpr size_t tableEndGap = 9;
            constexpr size_t tableSpacing = 4;
            const std::string& tableSectionTextColor = DEFAULT_STR;
            const std::string& tableInfoTextColor = DEFAULT_STR;
            const std::string& tableAlignment = LEFT_STR;
            constexpr bool hideTableBackground = false;
            constexpr bool useHeaderIndent = false;
            constexpr bool isPolling = false;
            constexpr bool isScrollableTable = true;
            const std::string wrappingMode = "char";
            constexpr bool useWrappedTextIndent = true;

            std::vector<std::vector<std::string>> dummyTableData;


            //addDummyListItem(list);
            addHeader(list, specificKey);

            addDummyListItem(list);
            // Draw the table using the sectionLines and empty infoLines
            drawTable(list, dummyTableData, sectionLines, infoLines, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
                      tableSectionTextColor, tableInfoTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isPolling, isScrollableTable, wrappingMode, useWrappedTextIndent);

            if (!packageSourcePath.empty()) {

                std::vector<std::string> sourceCommands = readListFromFile(packageSourcePath, 0, true);
                sectionLines.clear();
                //sectionLines.shrink_to_fit();
                infoLines.clear();
                //infoLines.shrink_to_fit();
                sectionLine = "";
                for (auto& command : sourceCommands) {
                    // Each command will be treated as a section with no corresponding info
                    //sectionLine = joinCommands(command);  // Combine command parts into a section line
                    sectionLines.push_back(std::move(command));              // Add to section lines
                    command.shrink_to_fit();
                    infoLines.push_back("");                          // Empty info line
                }
                //sourceCommands.clear();
                sourceCommands.shrink_to_fit();

                const std::string packageSourceName = getNameFromPath(packageSourcePath);

                addHeader(list, packageSourceName);
                drawTable(list, dummyTableData, sectionLines, infoLines, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
                          tableSectionTextColor, tableInfoTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isPolling, isScrollableTable, wrappingMode, useWrappedTextIndent);
            }
            //addDummyListItem(list);
        }

        const std::string packageVersion = isFromMainMenu ? "" : packageRootLayerVersion;


        auto* rootFrame = new tsl::elm::OverlayFrame(packageName,
           !lastPackageHeader.empty() ? lastPackageHeader + "?Ultrahand Script" : (packageVersion.empty() ? CAPITAL_ULTRAHAND_PROJECT_NAME + " Script" : packageVersion + " "+DIVIDER_SYMBOL+" " + CAPITAL_ULTRAHAND_PROJECT_NAME + " Script"),
           noClickableItems);
        //list->disableCaching(!isFromSelectionMenu);
        rootFrame->setContent(list);
        if (showWidget)
            rootFrame->m_showWidget = true;
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        const bool isRunningInterp = runningInterpreter.load(acquire);
        

        if (isRunningInterp) return handleRunningInterpreter(keysDown, keysHeld);
        
        if (lastRunningInterpreter.exchange(false, std::memory_order_acq_rel)) {
            isDownloadCommand.store(false, release);
            if (lastSelectedListItem) {
                lastSelectedListItem->setValue(commandSuccess.load(acquire) ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
                lastSelectedListItem->enableClickAnimation();
                lastSelectedListItem = nullptr;
            }
            closeInterpreterThread();

            if (!commandSuccess.load()) {
                triggerRumbleDoubleClick.store(true, std::memory_order_release);
            }
            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
            //lastRunningInterpreter.store(false, std::memory_order_release);
            return true;
        }
        
        if (goBackAfter.exchange(false, std::memory_order_acq_rel)) {
            disableSound.store(true, std::memory_order_release);
            simulatedBack.store(true, std::memory_order_release);
            return true;
        }
        
        if (inScriptMenu) {
            //if (simulatedNextPage.load(acquire))
            //    simulatedNextPage.store(false, release);
            //if (simulatedMenu.load(acquire))
            //    simulatedMenu.store(false, release);
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            
            const bool isTouching = stillTouching.load(acquire);
            const bool backKeyPressed = !isTouching && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
            
            if (backKeyPressed) {
                if (allowSlide.load(acquire))
                    allowSlide.store(false, release);
                if (unlockedSlide.load(acquire))
                    unlockedSlide.store(false, release);
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
        
        if (triggerExit.exchange(false, std::memory_order_acq_rel)) {
            //triggerExit.store(false, release);
            ult::launchingOverlay.store(true, std::memory_order_release);
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

    bool showWidget = false;

    bool usingProgress = false;
    bool isHold = false;
    bool isMini = false;

    size_t maxItemsLimit = 250;     // 0 = uncapped, any other value = max size
    
    // Helper function to apply size limit to any vector
    void applyItemsLimit(std::vector<std::string>& vec) {
        if (maxItemsLimit == 0 || vec.size() <= maxItemsLimit) return;
        vec.resize(maxItemsLimit);
        vec.shrink_to_fit();
    }

public:
    SelectionOverlay(const std::string& path, const std::string& key, const std::string& footerKey, const std::string& _lastPackageHeader, const std::vector<std::vector<std::string>>& commands, bool showWidget = false)
        : filePath(path), specificKey(key), specifiedFooterKey(footerKey), lastPackageHeader(_lastPackageHeader), selectionCommands(commands), showWidget(showWidget) {
        std::lock_guard<std::mutex> lock(transitionMutex);
        //lastSelectedListItemFooter2 = "";
        lastSelectedListItem = nullptr;
        //selectedFooterDict.clear();
        tsl::clearGlyphCacheNow.store(true, release);
    }

    ~SelectionOverlay() {
        std::lock_guard<std::mutex> lock(transitionMutex);
        //tsl::elm::clearFrameCache();
        //0lastSelectedListItemFooter2 = "";
        lastSelectedListItem = nullptr;
        //selectedFooterDict.clear();
        tsl::clearGlyphCacheNow.store(true, release);
    }
        
    void processSelectionCommands() {
        if (ult::expandedMemory) maxItemsLimit = 0; // uncapped for loader+
    
        removeEmptyCommands(selectionCommands);
    
        bool inEristaSection = false;
        bool inMarikoSection = false;
        std::string currentSection = GLOBAL_STR;

        // Track all source file paths for placeholder replacement
        std::string _iniFilePath;
        std::string _hexFilePath;
        std::string _listString;
        std::string _listFilePath;
        std::string _jsonString;
        std::string _jsonFilePath;
        
        
        // Use string_view for read-only operations to avoid copying
        //std::string commandName;
        std::string filterEntry;
        std::vector<std::string> matchedFiles, tempFiles;
    
        // Pre-cache pattern lengths for better performance
        //static const size_t SYSTEM_PATTERN_LEN = SYSTEM_PATTERN.length();
        //static const size_t MODE_PATTERN_LEN = MODE_PATTERN.length();
        //static const size_t GROUPING_PATTERN_LEN = GROUPING_PATTERN.length();
        //static const size_t SELECTION_MINI_PATTERN_LEN = SELECTION_MINI_PATTERN.length();
        //static const size_t HOLD_PATTERN_LEN = HOLD_PATTERN.length();
        //static const size_t PROGRESS_PATTERN_LEN = PROGRESS_PATTERN.length();
        
        bool afterSource = false;
        //updateGeneralPlaceholders();
        
        for (auto& cmd : selectionCommands) {

            //if (afterSource) {
            //    // Apply placeholder replacements in-place
            //    for (auto& arg : cmd) {
            //        replacePlaceholdersInArg(arg, generalPlaceholders);
            //    }
            //    static bool runOnce = true;
            //    if (runOnce) {
            //        _iniFilePath = "";
            //        _hexFilePath = "";
            //        _listString = "";
            //        _listFilePath = "";
            //        _jsonString = "";
            //        _jsonFilePath = "";
            //    }
            //}
            // Apply placeholder replacements in-place
            for (auto& arg : cmd) {
                replacePlaceholdersInArg(arg, generalPlaceholders);
            }

            const std::string& commandName = cmd[0]; // Now assigns to string_view - no copy
    
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
                } else if (commandName.size() > HOLD_PATTERN_LEN && 
                           commandName.compare(0, HOLD_PATTERN_LEN, HOLD_PATTERN) == 0) {
                    isHold = (commandName.substr(HOLD_PATTERN_LEN) == TRUE_STR);
                } else if (commandName.size() > PROGRESS_PATTERN_LEN && 
                           commandName.compare(0, PROGRESS_PATTERN_LEN, PROGRESS_PATTERN) == 0) {
                    usingProgress = (commandName.substr(PROGRESS_PATTERN_LEN) == TRUE_STR);
                }
    
                if (commandMode == TOGGLE_STR) {
                    if (commandName == "on:")
                        currentSection = ON_STR;
                    else if (commandName == "off:")
                        currentSection = OFF_STR;
                }
    
                if (cmd.size() > 1) {
                    // Apply ALL placeholder replacements using the comprehensive function
                    // This handles nesting, recursion, and proper order automatically
                    if (!afterSource)
                        applyPlaceholderReplacements(cmd, _hexFilePath, _iniFilePath, _listString, _listFilePath, _jsonString, _jsonFilePath);
    
                    // Now handle source declarations
                    if (commandName == "ini_file") {
                        _iniFilePath = cmd[1];
                        preprocessPath(_iniFilePath, filePath);
                        continue;
                    } else if (commandName == "hex_file") {
                        _hexFilePath = cmd[1];
                        preprocessPath(_hexFilePath, filePath);
                        continue;
                    } else if (commandName == "list") {
                        _listString = cmd[1];
                        removeQuotes(_listString);
                        continue;
                    } else if (commandName == "list_file") {
                        _listFilePath = cmd[1];
                        preprocessPath(_listFilePath, filePath);
                        continue;
                    } else if (commandName == "json") {
                        _jsonString = cmd[1];
                        removeQuotes(_jsonString);
                        continue;
                    } else if (commandName == "json_file") {
                        _jsonFilePath = cmd[1];
                        preprocessPath(_jsonFilePath, filePath);
                        continue;
                    } else if (commandName == "filter") {
                        // Avoid copying by directly assigning and then processing
                        filterEntry = std::move(cmd[1]); // Move instead of copy
                        //cmd[1].shrink_to_fit();
                        removeQuotes(filterEntry);
                        if (sourceType == FILE_STR) {
                            preprocessPath(filterEntry, filePath);
                        }
    
                        if (filterEntry.find('*') != std::string::npos) {
                            // Get files directly into temporary, avoid intermediate storage
                            tempFiles.clear();
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
                            //tempFiles.clear(); // automatically cleared since it is moved out of list.
                        } else {
                            if (currentSection == GLOBAL_STR)
                                filterList.push_back(std::move(filterEntry));
                            else if (currentSection == ON_STR)
                                filterListOn.push_back(std::move(filterEntry));
                            else if (currentSection == OFF_STR)
                                filterListOff.push_back(std::move(filterEntry));
                            //filterEntry.shrink_to_fit();
                        }
                        filterEntry.shrink_to_fit();
                    } else if (commandName == "file_source") {
                        sourceType = FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            pathPattern = cmd[1];
                            preprocessPath(pathPattern, filePath);
                            updateGeneralPlaceholders();
                            replacePlaceholdersInArg(pathPattern, generalPlaceholders);
                            // Get files directly, avoid storing in intermediate variable
                            tempFiles.clear();
                            tempFiles = getFilesListByWildcards(pathPattern, maxItemsLimit);
                            filesList.insert(filesList.end(), 
                                           std::make_move_iterator(tempFiles.begin()),
                                           std::make_move_iterator(tempFiles.end()));
                            //tempFiles.clear();
                        } else if (currentSection == ON_STR) {
                            pathPatternOn = cmd[1];
                            preprocessPath(pathPatternOn, filePath);
                            tempFiles.clear();
                            tempFiles = getFilesListByWildcards(pathPatternOn, maxItemsLimit);
                            filesListOn.insert(filesListOn.end(), 
                                             std::make_move_iterator(tempFiles.begin()),
                                             std::make_move_iterator(tempFiles.end()));
                            //tempFiles.clear();
                            sourceTypeOn = FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            pathPatternOff = cmd[1];
                            preprocessPath(pathPatternOff, filePath);
                            tempFiles.clear();
                            tempFiles = getFilesListByWildcards(pathPatternOff, maxItemsLimit);
                            filesListOff.insert(filesListOff.end(), 
                                              std::make_move_iterator(tempFiles.begin()),
                                              std::make_move_iterator(tempFiles.end()));
                            //tempFiles.clear();
                            sourceTypeOff = FILE_STR;
                        }
                        afterSource = true;
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
                        afterSource = true;
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
                        afterSource = true;
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
                        afterSource = true;
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
                        afterSource = true;
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
                        afterSource = true;
                    }
                }

                if (commandMode == TOGGLE_STR) {
                    if (currentSection == GLOBAL_STR) {
                        selectionCommandsOn.push_back(cmd);
                        selectionCommandsOff.push_back(std::move(cmd));
                        cmd.shrink_to_fit();
                    } else if (currentSection == ON_STR) {
                        selectionCommandsOn.push_back(std::move(cmd));
                        cmd.shrink_to_fit();
                    } else if (currentSection == OFF_STR) {
                        selectionCommandsOff.push_back(std::move(cmd));
                        cmd.shrink_to_fit();
                    }
                }
                //if (commandMode == TOGGLE_STR)
                //    cmd.clear();
            }
        }
    }

    virtual tsl::elm::Element* createUI() override {
        std::lock_guard<std::mutex> lock(transitionMutex);
        inSelectionMenu = true;
        
    
        auto* list = new tsl::elm::List();
        packageConfigIniPath = filePath + CONFIG_FILENAME;
    
        commandSystem = commandSystems[0];
        commandMode = commandModes[0];
        commandGrouping = commandGroupings[0];
    
        processSelectionCommands();
    
        std::vector<std::string> selectedItemsListOn, selectedItemsListOff;
        std::string currentPackageHeader;
        
    
        if (commandMode == DEFAULT_STR || commandMode == OPTION_STR) {
            if (sourceType == FILE_STR) {
                selectedItemsList = std::move(filesList);
                filesList.shrink_to_fit();
            }
            else if (sourceType == LIST_STR || sourceType == LIST_FILE_STR) {
                selectedItemsList = (sourceType == LIST_STR) ? stringToList(listString) : readListFromFile(listPath, maxItemsLimit);
                listString = "";
                listPath = "";
                //listString.clear();
                //listPath.clear();
            }
            else if (sourceType == INI_FILE_STR) {
                selectedItemsList = parseSectionsFromIni(iniPath);
                iniPath.clear();
            }
            else if (sourceType == JSON_STR || sourceType == JSON_FILE_STR) {
                populateSelectedItemsListFromJson(sourceType, (sourceType == JSON_STR) ? jsonString : jsonPath, jsonKey, selectedItemsList);
                jsonPath = "";
                jsonString = "";

                //jsonPath.clear();
                //jsonPath.shrink_to_fit();
                //jsonString.clear();
                //jsonString.shrink_to_fit();
            }
            applyItemsLimit(selectedItemsList);

        } else if (commandMode == TOGGLE_STR) {
            if (sourceTypeOn == FILE_STR) {
                selectedItemsListOn = std::move(filesListOn);
                filesListOn.shrink_to_fit();
            }
            else if (sourceTypeOn == LIST_STR || sourceTypeOn == LIST_FILE_STR) {
                selectedItemsListOn = (sourceTypeOn == LIST_STR) ? stringToList(listStringOn) : readListFromFile(listPathOn, maxItemsLimit);
                listStringOn = "";
                listPathOn = "";
                //listStringOn.clear();
                //listPathOn.clear();
            }
            else if (sourceTypeOn == INI_FILE_STR) {
                selectedItemsListOn = parseSectionsFromIni(iniPathOn);
                iniPathOn = "";
                //iniPathOn.clear();
            }
            else if (sourceTypeOn == JSON_STR || sourceTypeOn == JSON_FILE_STR) {
                populateSelectedItemsListFromJson(sourceTypeOn, (sourceTypeOn == JSON_STR) ? jsonStringOn : jsonPathOn, jsonKeyOn, selectedItemsListOn);
                jsonPathOff = "";
                jsonStringOff = "";

                //jsonPathOn.clear();
                //jsonPathOn.shrink_to_fit();
                //jsonStringOn.clear();
                //jsonStringOn.shrink_to_fit();
            }
            applyItemsLimit(selectedItemsListOn);

            if (sourceTypeOff == FILE_STR) {
                selectedItemsListOff = std::move(filesListOff);
                filesListOff.shrink_to_fit();
            }
            else if (sourceTypeOff == LIST_STR || sourceTypeOff == LIST_FILE_STR) {
                selectedItemsListOff = (sourceTypeOff == LIST_STR) ? stringToList(listStringOff) : readListFromFile(listPathOff, maxItemsLimit);
                listStringOff = "";
                listPathOff = "";
                //listStringOff.clear();
                //listPathOff.clear();
            }
            else if (sourceTypeOff == INI_FILE_STR) {
                selectedItemsListOff = parseSectionsFromIni(iniPathOff);
                iniPathOff = "";
                //iniPathOff.clear();
            }
            else if (sourceTypeOff == JSON_STR || sourceTypeOff == JSON_FILE_STR) {
                populateSelectedItemsListFromJson(sourceTypeOff, (sourceTypeOff == JSON_STR) ? jsonStringOff : jsonPathOff, jsonKeyOff, selectedItemsListOff);
                jsonPathOff = "";
                jsonStringOff = "";

                //jsonPathOff.clear();
                //jsonStringOff.shrink_to_fit();
                //jsonStringOff.clear();
                //jsonStringOff.shrink_to_fit();
            }//
            applyItemsLimit(selectedItemsListOff);

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
        }
    
        if (commandGrouping == DEFAULT_STR) {
            std::string cleanSpecificKey = specificKey.substr(1);
            removeTag(cleanSpecificKey);
            addHeader(list, cleanSpecificKey);
            currentPackageHeader = cleanSpecificKey;
        }
    
        //tsl::elm::ListItem* listItem;
        size_t pos;
        std::string parentDirName;
        std::string footer;
        //std::string optionName;
    
        if (selectedItemsList.empty()) {
            if (commandGrouping != DEFAULT_STR) {
                std::string cleanSpecificKey = specificKey.substr(1);
                removeTag(cleanSpecificKey);
                addHeader(list, cleanSpecificKey);
                currentPackageHeader = cleanSpecificKey;
            }
            //tsl::elm::ListItem* listItem = new tsl::elm::ListItem(EMPTY);
            //list->addItem(listItem);

            //addDummyListItem(list);
            //auto* warning = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer* renderer, u16 x, u16 y, u16 w, u16 h){
            //    renderer->drawString("\uE150", false, 180, 274+50, 90, (tsl::defaultTextColor));
            //    renderer->drawString(SELECTION_IS_EMPTY, false, 110, 360+50, 25, (tsl::defaultTextColor));
            //});
            //list->addItem(warning);

            addSelectionIsEmptyDrawer(list);
            noClickableItems = true;
        }
    
        std::string tmpSelectedItem;
        const size_t selectedItemsSize = selectedItemsList.size(); // Cache size to avoid repeated calls
        
        
        // Pre-process filtered items for reduced memory imprint
        for (size_t i = 0; i < selectedItemsSize; ++i) {
            std::string& selectedItem = selectedItemsList[i];
            
            //if (isFileOrDirectory(selectedItem)) {
            const std::string itemName = getNameFromPath(selectedItem);
            if (itemName.front() == '.') { // Skip hidden items
                selectedItem = "";
                continue;
            }
            //}

            if (commandMode == TOGGLE_STR) {
                auto it = std::find(filterListOn.begin(), filterListOn.end(), selectedItem);
                if (it != filterListOn.end()) {
                    filterListOn.erase(it);
                    selectedItem = "";
                    continue;
                }
                it = std::find(filterListOff.begin(), filterListOff.end(), selectedItem);
                if (it != filterListOff.end()) {
                    filterListOff.erase(it);
                    selectedItem = "";
                    continue;
                }
            } else {
                auto it = std::find(filterList.begin(), filterList.end(), selectedItem);
                if (it != filterList.end()) {
                    filterList.erase(it);
                    selectedItem = "";
                    continue;
                }
            }
        }

        filterList = {};
        filterListOn = {};
        filterListOff = {};

        //filterList.clear();
        //filterList.shrink_to_fit();
        //filterListOn.clear();
        //filterListOn.shrink_to_fit();
        //filterListOff.clear();
        //filterListOff.shrink_to_fit();

        std::string itemName;
        for (size_t i = 0; i < selectedItemsSize; ++i) {
            std::string& selectedItem = selectedItemsList[i];
            

            itemName = getNameFromPath(selectedItem);
            if (itemName.empty()) // Skip empty lines
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
            //    if (commandMode == TOGGLE_STR) {
            //        auto it = std::find(filterListOn.begin(), filterListOn.end(), itemName);
            //        if (it != filterListOn.end()) {
            //            filterListOn.erase(it);
            //            selectedItem.clear();
            //            continue;
            //        }
            //        it = std::find(filterListOff.begin(), filterListOff.end(), itemName);
            //        if (it != filterListOff.end()) {
            //            filterListOff.erase(it);
            //            selectedItem.clear();
            //            continue;
            //        }
            //    } else {
            //        auto it = std::find(filterList.begin(), filterList.end(), itemName);
            //        if (it != filterList.end()) {
            //            filterList.erase(it);
            //            selectedItem.clear();
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

                if (isHold)
                    listItem->disableClickAnimation();
                
                listItem->setClickListener([this, i, selectedItem, footer, listItem, currentPackageHeader, itemName](uint64_t keys) {
                    
                    if (runningInterpreter.load(acquire)) {
                        return false;
                    }
                    
                    if (((keys & KEY_A) && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        
                        isDownloadCommand.store(false, release);
                        runningInterpreter.store(true, release);
                        
                        auto modifiedCmds = getSourceReplacement(selectionCommands, selectedItem, i, filePath);
                        if (isHold) {
                            lastSelectedListItemFooter = footer;
                            //lastFooterHighlight = commandFooterHighlight;  // STORE THE VALUE
                            //lastFooterHighlightDefined = commandFooterHighlightDefined;  // STORE WHETHER IT WAS DEFINED
                            listItem->setValue(INPROGRESS_SYMBOL);
                            lastSelectedListItem = listItem;
                            holdStartTick = armGetSystemTick();
                            storedCommands = std::move(modifiedCmds);
                            lastCommandMode = commandMode;
                            lastCommandIsHold = true;
                            lastKeyName = specificKey;
                            return true;
                        }

                        executeInterpreterCommands(std::move(modifiedCmds), filePath, specificKey);
                        listItem->disableClickAnimation();
                        //startInterpreterThread(filePath);
                        
                        listItem->setValue(INPROGRESS_SYMBOL);
                        
                        
                        if (commandMode == OPTION_STR) {
                            selectedFooterDict[specifiedFooterKey] = listItem->getText();
                            if (lastSelectedListItem && listItem && lastSelectedListItem != listItem) {
                            
                                lastSelectedListItem->setValue(lastSelectedListItemFooter2, true);
                                
                            }
                            lastSelectedListItemFooter2 = footer;
                            
                        }
                        
                        lastSelectedListItem = listItem;
                        //lastFooterHighlight = commandFooterHighlight;  // STORE THE VALUE
                        //lastFooterHighlightDefined = commandFooterHighlightDefined;  // STORE WHETHER IT WAS DEFINED
                        shiftItemFocus(listItem);
    
                        lastRunningInterpreter.store(true, std::memory_order_release);
                        if (lastSelectedListItem)
                            lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
    
                    else if (keys & SCRIPT_KEY && !(keys & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                        //inSelectionMenu = false;
    
                        auto modifiedCmds = getSourceReplacement(selectionCommands, selectedItem, i, filePath);
                        applyPlaceholderReplacementsToCommands(modifiedCmds, filePath);
                        //tsl::elm::g_cachedTop.disabled = true;
                        //tsl::elm::g_cachedBottom.disabled = true;
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), filePath, itemName, "selection", false, currentPackageHeader, showWidget);
                        return true;
                    }
    
                    return false;
                });
                list->addItem(listItem);
    
            } else if (commandMode == TOGGLE_STR) {
                auto* toggleListItem = new tsl::elm::ToggleListItem(itemName, false, ON, OFF, isMini, true);
    
                // Use const iterators for better performance
                const bool toggleStateOn = std::find(selectedItemsListOn.cbegin(), selectedItemsListOn.cend(), selectedItem) != selectedItemsListOn.cend();
                toggleListItem->setState(toggleStateOn);
    
                toggleListItem->setStateChangedListener([this, i, toggleListItem, selectedItem, itemName](bool state) {
                    if (runningInterpreter.load(std::memory_order_acquire)) {
                        return;
                    }

                    
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
                    
                    if (usingProgress)
                        toggleListItem->setValue(INPROGRESS_SYMBOL);

                    nextToggleState = !state ? CAPITAL_OFF_STR : CAPITAL_ON_STR;
                    runningInterpreter.store(true, release);
                    lastRunningInterpreter.store(true, release);
                    lastSelectedListItem = toggleListItem;
                    //interpretAndExecuteCommands(std::move(modifiedCmds), filePath, specificKey);
                    executeInterpreterCommands(std::move(modifiedCmds), filePath, specificKey);
                    //resetPercentages();
                
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
                    auto modifiedCmds = getSourceReplacement(!state ? selectionCommandsOn : selectionCommandsOff, currentSelectedItems[i], i, filePath);
                    applyPlaceholderReplacementsToCommands(modifiedCmds, filePath);
                    //tsl::elm::g_cachedTop.disabled = true;
                    //tsl::elm::g_cachedBottom.disabled = true;
                    tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), filePath, itemName, "selection", false, currentPackageHeader, showWidget);
                });
    
                list->addItem(toggleListItem);
            }
            selectedItem = "";
            //selectedItem.clear();
            //selectedItem.shrink_to_fit();
        }

        // clear everything
        selectedItemsList = {};
        selectedItemsListOn = {};
        selectedItemsListOff = {};

        //selectedItemsList.clear();
        //selectedItemsList.shrink_to_fit();
        //selectedItemsListOn.clear();
        //selectedItemsListOn.shrink_to_fit();
        //selectedItemsListOff.clear();
        //selectedItemsListOff.shrink_to_fit();
        
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
    
        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
        
        //list->disableCaching(true);
        rootFrame->setContent(list);
        if (showWidget)
            rootFrame->m_showWidget = true;

        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        bool isHolding = (lastCommandIsHold && runningInterpreter.load(std::memory_order_acquire));
        if (isHolding) {
            processHold(keysDown, keysHeld, holdStartTick, isHolding, [&]() {
                // Execute interpreter commands if needed
                displayPercentage.store(-1, std::memory_order_release);
                //lastCommandMode.clear();
                lastCommandIsHold = false;
                //lastKeyName.clear();
                lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
                //lastSelectedListItemFooter.clear();
                //lastSelectedListItem->enableClickAnimation();
                //lastSelectedListItem->triggerClickAnimation();
                //lastSelectedListItem->disableClickAnimation();
                triggerEnterFeedback();
                executeInterpreterCommands(std::move(storedCommands), filePath, lastKeyName);
                lastRunningInterpreter.store(true, std::memory_order_release);
            }, nullptr, true); // true = reset storedCommands
            return true;
        }

        const bool isRunningInterp = runningInterpreter.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
        
        if (lastRunningInterpreter.exchange(false, std::memory_order_acq_rel)) {
            isDownloadCommand.store(false, release);
        
            if (lastSelectedListItem) {
                const bool success = commandSuccess.load(acquire);
        
                if (nextToggleState.empty()) {
                    // No toggle state, just show a check or cross
                    lastSelectedListItem->setValue(success ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
                } else {
                    // Update displayed value
                    lastSelectedListItem->setValue(
                        success
                            ? nextToggleState
                            : (nextToggleState == CAPITAL_ON_STR ? CAPITAL_OFF_STR : CAPITAL_ON_STR)
                    );
        
                    // Update toggle state (single simplified line)
                    static_cast<tsl::elm::ToggleListItem*>(lastSelectedListItem)
                        ->setState(nextToggleState == CAPITAL_ON_STR ? success : !success);
        
                    nextToggleState.clear();
                }
        
                lastSelectedListItem->enableClickAnimation();
                //lastSelectedListItem = nullptr;
            }
        
            closeInterpreterThread();
            resetPercentages();
            


            if (!commandSuccess.load()) {
                triggerRumbleDoubleClick.store(true, std::memory_order_release);
            }

            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
            //lastRunningInterpreter.store(false, std::memory_order_release);
            return true;
        }
        
        if (goBackAfter.exchange(false, std::memory_order_acq_rel)) {
            disableSound.store(true, std::memory_order_release);
            simulatedBack.store(true, std::memory_order_release);
            return true;
        }
        
        // Cache touching state for refresh operations (used in same logical block)
        const bool isTouching = stillTouching.load(acquire);
        
        if (refreshPage.load(acquire) && !isTouching) {
            //tsl::goBack();
            tsl::swapTo<SelectionOverlay>(filePath, specificKey, specifiedFooterKey, lastPackageHeader, selectionCommands, showWidget);
            refreshPage.store(false, release);
        }
        
        if (refreshPackage.load(acquire) && !isTouching) {
            tsl::goBack();
        }
        
        if (inSelectionMenu) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            
            // Check touching state again for the key handling (different timing context)
            const bool isTouchingForKeys = stillTouching.load(acquire);
            const bool backKeyPressed = !isTouchingForKeys && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
            
            if (backKeyPressed) {
                allowSlide.exchange(false, std::memory_order_acq_rel);
                unlockedSlide.exchange(false, std::memory_order_acq_rel);
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
                if (commandMode == OPTION_STR && isFile(packageConfigIniPath)) {
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
        
        if (triggerExit.exchange(false, std::memory_order_acq_rel)) {
            ult::launchingOverlay.store(true, std::memory_order_release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        return false;
    }
};

std::vector<std::vector<std::string>> gatherPromptCommands(
    const std::string& dropdownSection,
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>>&& options) {
    
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
    
    std::vector<std::vector<std::string>> commands;

    for (auto& nextOption : options) {
        const std::string sectionName = std::move(nextOption.first);
        nextOption.first.shrink_to_fit();
        commands = std::move(nextOption.second);
        nextOption.second.shrink_to_fit();
        
        // Check if this is the start of the relevant section
        if (sectionName == dropdownSection) {
            inRelevantSection = true;
            commands.clear();
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
            for (auto& cmd : commands) {
                fullCmd.clear();
                //fullCmd.shrink_to_fit();
                
                for (auto& part : cmd) {
                    splitParts = splitString(part, " ");
                    fullCmd.insert(fullCmd.end(), splitParts.begin(), splitParts.end());
                    part.clear();
                }
                
                if (!fullCmd.empty()) {
                    promptCommands.push_back(fullCmd);
                }
                cmd.clear();
            }
        }
        commands.clear();
    }
    //commands.clear();
    
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


// For returning with menu redrawing
struct ReturnContext {
    std::string packagePath;
    std::string sectionName;
    std::string currentPage;
    std::string packageName;
    std::string pageHeader;
    std::string option;
    size_t nestedLayer = 0;

    void clear() {
        packagePath.clear();
        sectionName.clear();
        currentPage.clear();
        packageName.clear();
        pageHeader.clear();
        option.clear();
        nestedLayer = 0;
    }
};


//static std::vector<std::vector<std::string>> storedCommands;

struct CommandSettings {
    bool isMini = false;
    bool isSlot = false;
};

static CommandSettings parseCommandSettings(std::vector<std::vector<std::string>>& commands) {
    CommandSettings settings;
    
    if (commands.empty() || commands.size() > 2) {
        return settings;
    }
    
    bool foundMini = false;
    bool foundMode = false;
    bool modeIsSlot = false;
    bool miniValue = false;
    
    for (const auto& command : commands) {
        //if (!command.empty()) {
        {
            const std::string& commandName = command[0];
            const size_t cmdLen = commandName.length();
            
            if (cmdLen > 1 && commandName[0] == ';' && commandName[1] == 'm') {
                // Check for ;mini=
                if (!foundMini && cmdLen >= MINI_PATTERN_LEN + TRUE_STR_LEN && commandName[2] == 'i') {
                    if (commandName.compare(0, MINI_PATTERN_LEN, MINI_PATTERN) == 0) {
                        switch (commandName[MINI_PATTERN_LEN]) {
                            case 't':
                                if (commandName.compare(MINI_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                    foundMini = true;
                                    miniValue = true;
                                }
                                break;
                            case 'f':
                                if (commandName.compare(MINI_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                    foundMini = true;
                                    miniValue = false;
                                }
                                break;
                        }
                    }
                }
                
                // Check for ;mode=
                if (!foundMode && cmdLen >= MODE_PATTERN_LEN + SLOT_STR.length() && commandName[2] == 'o') {
                    if (commandName.compare(0, MODE_PATTERN_LEN, MODE_PATTERN) == 0) {
                        foundMode = true;
                        if (commandName[MODE_PATTERN_LEN] == 's') {
                            modeIsSlot = (commandName.compare(MODE_PATTERN_LEN, SLOT_STR.length(), SLOT_STR) == 0);
                        }
                    }
                }
                
                if (foundMini && foundMode) {
                    break;
                }
            }
        }
    }
    
    // Apply the settings
    if (foundMini) {
        settings.isMini = miniValue;
    } else if (modeIsSlot) {
        settings.isSlot = true;
    }
    
    // Determine if commands should be cleared
    const bool shouldClearCommands = (commands.size() == 1 && (foundMini || foundMode)) ||
                                   (commands.size() == 2 && foundMini && foundMode);
    
    if (shouldClearCommands) {
        commands.clear();
    }
    
    return settings;
}

//ReturnContext returnTo;
static std::stack<ReturnContext> returnContextStack;


class PackageMenu; // forwarding

// returns if there are or are not cickable items.
bool drawCommandsMenu(
    tsl::elm::List* list,
    const std::string& packageIniPath,
    const std::string& packageConfigIniPath,
    const PackageHeader& packageHeader,
    const std::string& pageHeader,
    std::string& pageLeftName,
    std::string& pageRightName,
    const std::string& packagePath,
    const std::string& currentPage,
    const std::string& packageName,
    const std::string& dropdownSection,
    const size_t nestedLayer,
    std::string& pathPattern,
    std::string& pathPatternOn,
    std::string& pathPatternOff,
    bool& usingPages,
    const bool packageMenuMode,
    const bool showWidget = false) {

    tsl::hlp::ini::IniData packageConfigData;
    //tsl::elm::ListItem* listItem;
    //auto toggleListItem = new tsl::elm::ToggleListItem("", true, "", "");
    

    bool toggleStateOn;
    
    bool skipSection = false;
    bool skipSystem = false;
    
    std::string lastSection;
    std::string drawLocation;
    
    std::string commandName;
    std::string commandFooter;
    bool commandFooterHighlight;
    bool commandFooterHighlightDefined;
    bool isHold;
    std::string commandSystem;
    std::string commandMode;
    std::string commandGrouping;
    
    std::string currentSection;
    std::string defaultToggleState;
    std::string sourceType, sourceTypeOn, sourceTypeOff;
    
    std::string packageSource;
    //std::string jsonPath, jsonPathOn, jsonPathOff;
    //std::string jsonKey, jsonKeyOn, jsonKeyOff;
    
    //std::string iniFilePath;

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

    bool usingProgress;

    bool isPolling;
    bool isScrollableTable;
    bool usingTopPivot, usingBottomPivot;
    bool onlyTables = true;

    //std::vector<std::string> entryList;

    std::string commandNameLower;

    std::string cleanOptionName;

    std::string lastPackageHeader;

    bool isMini;

    // update general placeholders
    updateGeneralPlaceholders();

    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath);
    for (size_t i = 0; i < options.size(); ++i) {
        optionName.clear();
        commands.clear();
        commandsOn.clear();
        commandsOff.clear();
        tableData.clear();

        auto& option = options[i];
        
        optionName = std::move(option.first);
        //option.first.shrink_to_fit();

        commands = std::move(option.second);
        //option.second.shrink_to_fit();

        // Remove all empty command strings
        removeEmptyCommands(commands);

        //option.first.clear();
        //option.second.clear();
        //option.second.shrink_to_fit(); 

        footer = "";
        useSelection = false;

        isMini = false;

        // toggle settings
        usingProgress = false;

        // Table settings
        isPolling = false;
        isScrollableTable = true;
        usingTopPivot = false;
        usingBottomPivot = false;
        hideTableBackground = false;
        useHeaderIndent = false;
        tableStartGap = 20;
        tableEndGap = 9;
        tableColumnOffset = 164;
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
        commandFooterHighlight = false;
        commandFooterHighlightDefined = false;
        isHold = false;
        commandSystem = DEFAULT_STR;
        commandMode = DEFAULT_STR;
        commandGrouping = DEFAULT_STR;
        
        defaultToggleState = "";
        currentSection = GLOBAL_STR;
        sourceType = DEFAULT_STR;
        sourceTypeOn = DEFAULT_STR;
        sourceTypeOff = DEFAULT_STR;
        
        
        bool isSlot = false;

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
                if (optionName.front() == '*' && commands.size() > 0 && commands.size() <= 2) {
                     auto settings = parseCommandSettings(commands);
                     isMini = settings.isMini;
                     isSlot = settings.isSlot;
                }
                if (commands.size() == 0) {
                    if (optionName == dropdownSection)
                        skipSection = false;
                    else
                        skipSection = true;
                    continue;
                }
            } else {
                                
                if (optionName.front() == '*' && commands.size() > 0 && commands.size() <= 2) {
                     auto settings = parseCommandSettings(commands);
                     isMini = settings.isMini;
                     isSlot = settings.isSlot;
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
                        
                        // First, scan for patterns to establish defaults
                        bool foundFooter = false;
                        bool foundFooterHighlight = false;
                        
                        for (const auto& command : commands) {
                            //if (!command.empty()) {
                            {
                                const std::string& _commandName = command[0];
                                const size_t cmdLen = _commandName.length();
                                
                                // Quick check: if starts with ';' and has at least one more char
                                if (cmdLen > 1 && _commandName[0] == ';') {
                                    const char secondChar = _commandName[1];
                                    
                                    switch (secondChar) {
                                        case 'f':
                                            // Could be ;footer= or ;footer_highlight=
                                            if (!foundFooter && cmdLen > FOOTER_PATTERN_LEN && _commandName[7] == '=') {
                                                if (_commandName.compare(0, FOOTER_PATTERN_LEN, FOOTER_PATTERN) == 0) {
                                                    commandFooter = _commandName.substr(FOOTER_PATTERN_LEN);
                                                    // If there are additional arguments, append them (handles spaces in footer)
                                                    for (size_t j = 1; j < command.size(); ++j) {
                                                        commandFooter += " " + command[j];
                                                    }
                                                    removeQuotes(commandFooter);
                                                    foundFooter = true;
                                                    if (foundFooterHighlight) break; // Early exit
                                                }
                                            }
                                            
                                            if (!foundFooterHighlight && cmdLen >= FOOTER_HIGHLIGHT_PATTERN_LEN + TRUE_STR_LEN && _commandName[7] == '_') {
                                                if (_commandName.compare(0, FOOTER_HIGHLIGHT_PATTERN_LEN, FOOTER_HIGHLIGHT_PATTERN) == 0) {
                                                    switch (_commandName[FOOTER_HIGHLIGHT_PATTERN_LEN]) {
                                                        case 't':
                                                            if (_commandName.compare(FOOTER_HIGHLIGHT_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                                commandFooterHighlight = true;
                                                                commandFooterHighlightDefined = true;
                                                                foundFooterHighlight = true;
                                                            }
                                                            break;
                                                        case 'f':
                                                            if (_commandName.compare(FOOTER_HIGHLIGHT_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                                commandFooterHighlight = false;
                                                                commandFooterHighlightDefined = true;
                                                                foundFooterHighlight = true;
                                                            }
                                                            break;
                                                    }
                                                }
                                            }
                                            break;
                                    }
                                    
                                    // Early exit if both found
                                    if (foundFooter && foundFooterHighlight) {
                                        break;
                                    }
                                }
                            }
                        }
                    
                        // Then, load from config INI (overwrites defaults if they exist)
                        packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                        auto optionIt = packageConfigData.find(optionName);
                        if (optionIt != packageConfigData.end()) {
                            auto footerIt = optionIt->second.find(FOOTER_STR);
                            if (footerIt != optionIt->second.end()) {
                                commandFooter = footerIt->second;
                            }
                    
                            // Load footer_highlight from config INI if it exists
                            auto footerHighlightIt = optionIt->second.find("footer_highlight");
                            if (footerHighlightIt != optionIt->second.end()) {
                                commandFooterHighlight = (footerHighlightIt->second == TRUE_STR);
                                commandFooterHighlightDefined = true;
                            }
                        }
                        packageConfigData.clear();
                    
                        // override loading of the command footer

                        tsl::elm::ListItem* listItem;
                        if (!commandFooter.empty() && commandFooter != NULL_STR){
                            footer = commandFooter;
                            cleanOptionName = optionName.substr(1);
                            //removeTag(cleanOptionName);
                            listItem = new tsl::elm::ListItem(cleanOptionName, "", isMini, true);
                            listItem->setValue(footer, commandFooterHighlightDefined ? !commandFooterHighlight : false); 
                        } else {
                            footer = !isSlot ? DROPDOWN_SYMBOL : OPTION_SYMBOL;
                            cleanOptionName = optionName.substr(1);
                            //removeTag(cleanOptionName);
                            // Create reference to PackageMenu with dropdownSection set to optionName
                            listItem = new tsl::elm::ListItem(cleanOptionName, footer, isMini, true);
                        }
                        
                        if (packageMenuMode) {
                            listItem->setClickListener([packagePath, dropdownSection, currentPage, packageName, nestedLayer, i,
                                packageIniPath, optionName, cleanOptionName, pageHeader, lastPackageHeader, showWidget](s64 keys) {
                                
                                if (runningInterpreter.load(acquire))
                                    return false;

                                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                                    inPackageMenu = false;

                                    nestedMenuCount++;
                                    //tsl::clearGlyphCacheNow.store(true, release);
                                    // Store return info

                                    returnContextStack.push({
                                        .packagePath = packagePath,
                                        .sectionName = dropdownSection,
                                        .currentPage = currentPage,
                                        .packageName = packageName,
                                        .pageHeader = pageHeader,    // Move this before nestedLayer
                                        .option = cleanOptionName,
                                        .nestedLayer = nestedLayer          // Move this to the end
                                    });

                                    tsl::swapTo<PackageMenu>(packagePath, optionName, currentPage, packageName, nestedMenuCount, lastPackageHeader);
                                    
                                    return true;
                                
                                } else if (keys & SCRIPT_KEY && !(keys & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                                    //if (inPackageMenu)
                                    //    inPackageMenu = false;
                                    //if (inSubPackageMenu)
                                    //    inSubPackageMenu = false;
                                    
                                                        
                                    // Gather the prompt commands for the current dropdown section
                                    //const std::vector<std::vector<std::string>> promptCommands = gatherPromptCommands(optionName, options);
                                    
                                    //auto options = loadOptionsFromIni(packageIniPath);
                                    // Pass all gathered commands to the ScriptOverlay
                                    tsl::changeTo<ScriptOverlay>(std::move(gatherPromptCommands(optionName, std::move(loadOptionsFromIni(packageIniPath)))), packagePath, optionName, "package", true, lastPackageHeader, showWidget);
                                    return true;
                                }
                                return false;
                            });
                            listItem->disableClickAnimation();
                        } else {
                            listItem->setClickListener([optionName, cleanOptionName, i, packageIniPath, lastPackageHeader, showWidget](s64 keys) {
                                if (runningInterpreter.load(acquire))
                                    return false;

                                if (simulatedMenu.load(std::memory_order_acquire)) {
                                    keys |= SYSTEM_SETTINGS_KEY;
                                }

                                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                                    inPackageMenu = false;

                                    tsl::changeTo<MainMenu>("", optionName);
                                    return true;
                                } else if (keys & SCRIPT_KEY && !(keys & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                                    //if (inMainMenu.load(acquire)) {
                                    //    inMainMenu.store(false, std::memory_order_release);
                                    //}
                                    
                                    
                                    // Gather the prompt commands for the current dropdown section
                                    //const std::vector<std::vector<std::string>> promptCommands = gatherPromptCommands(optionName, options);
                                    //auto options = loadOptionsFromIni(packageIniPath);

                                    tsl::changeTo<ScriptOverlay>(std::move(gatherPromptCommands(optionName, std::move(loadOptionsFromIni(packageIniPath)))), PACKAGE_PATH, optionName, "main", true, lastPackageHeader, showWidget);
                                    return true;
                                } else if (keys & SYSTEM_SETTINGS_KEY && !(keys & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK)) {
                                    skipJumpReset.store(false, std::memory_order_release);
                                    returnJumpItemName = cleanOptionName;
                                    returnJumpItemValue = "";
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
            //removeEmptyCommands(commands);

            // Initial processing of commands (DUPLICATE CODE)
            for (auto& cmd : commands) {
                //if (cmd.empty()) continue;
            
                commandName = cmd[0];
                
                // Quick check for section markers
                if (commandName.length() == 7) {
                    if ((commandName[0] == 'e' || commandName[0] == 'E') && 
                        commandName[1] == 'r' && commandName[2] == 'i' && 
                        commandName[3] == 's' && commandName[4] == 't' && 
                        commandName[5] == 'a' && commandName[6] == ':') {
                        inEristaSection = true;
                        inMarikoSection = false;
                        continue;
                    } else if ((commandName[0] == 'm' || commandName[0] == 'M') && 
                               commandName[1] == 'a' && commandName[2] == 'r' && 
                               commandName[3] == 'i' && commandName[4] == 'k' && 
                               commandName[5] == 'o' && commandName[6] == ':') {
                        inEristaSection = false;
                        inMarikoSection = true;
                        continue;
                    }
                }
                
                if ((inEristaSection && !inMarikoSection && usingErista) || 
                    (!inEristaSection && inMarikoSection && usingMariko) || 
                    (!inEristaSection && !inMarikoSection)) {
                    
                    // Quick check: if starts with ';' and has at least one more char
                    if (commandName.length() > 1 && commandName[0] == ';') {
                        const char secondChar = commandName[1];
                        
                        // Branch on second character for faster pattern matching
                        switch (secondChar) {
                            case 's':
                                if (commandName.compare(0, SYSTEM_PATTERN_LEN, SYSTEM_PATTERN) == 0) {
                                    commandSystem = commandName.substr(SYSTEM_PATTERN_LEN);
                                    if (std::find(commandSystems.begin(), commandSystems.end(), commandSystem) == commandSystems.end())
                                        commandSystem = commandSystems[0];
                                    continue;
                                }
                                if (commandName.compare(0, SCROLLABLE_PATTERN_LEN, SCROLLABLE_PATTERN) == 0) {
                                    switch (commandName[SCROLLABLE_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(SCROLLABLE_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                isScrollableTable = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(SCROLLABLE_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                isScrollableTable = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                if (commandName.compare(0, START_GAP_PATTERN_LEN, START_GAP_PATTERN) == 0) {
                                    tableStartGap = ult::stoi(commandName.substr(START_GAP_PATTERN_LEN));
                                    continue;
                                }
                                if (commandName.compare(0, SPACING_PATTERN_LEN, SPACING_PATTERN) == 0) {
                                    tableSpacing = ult::stoi(commandName.substr(SPACING_PATTERN_LEN));
                                    continue;
                                }
                                if (commandName.compare(0, SECTION_TEXT_COLOR_PATTERN_LEN, SECTION_TEXT_COLOR_PATTERN) == 0) {
                                    tableSectionTextColor = commandName.substr(SECTION_TEXT_COLOR_PATTERN_LEN);
                                    continue;
                                }
                                if (commandName.compare(0, STEPS_PATTERN_LEN, STEPS_PATTERN) == 0) {
                                    steps = ult::stoi(commandName.substr(STEPS_PATTERN_LEN));
                                    continue;
                                }
                                break;
                                
                            case 'm':
                                if (commandName.compare(0, MODE_PATTERN_LEN, MODE_PATTERN) == 0) {
                                    commandMode = commandName.substr(MODE_PATTERN_LEN);
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
                                }
                                if (commandName.compare(0, MINI_PATTERN_LEN, MINI_PATTERN) == 0) {
                                    switch (commandName[MINI_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(MINI_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                isMini = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(MINI_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                isMini = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                if (commandName.compare(0, MIN_VALUE_PATTERN_LEN, MIN_VALUE_PATTERN) == 0) {
                                    minValue = ult::stoi(commandName.substr(MIN_VALUE_PATTERN_LEN));
                                    continue;
                                }
                                if (commandName.compare(0, MAX_VALUE_PATTERN_LEN, MAX_VALUE_PATTERN) == 0) {
                                    maxValue = ult::stoi(commandName.substr(MAX_VALUE_PATTERN_LEN));
                                    continue;
                                }
                                break;
                                
                            case 'g':
                                if (commandName.compare(0, GROUPING_PATTERN_LEN, GROUPING_PATTERN) == 0) {
                                    commandGrouping = commandName.substr(GROUPING_PATTERN_LEN);
                                    if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end())
                                        commandGrouping = commandGroupings[0];
                                    continue;
                                }
                                if (commandName.compare(0, END_GAP_PATTERN_ALIAS_LEN, END_GAP_PATTERN_ALIAS) == 0) {
                                    tableEndGap = ult::stoi(commandName.substr(END_GAP_PATTERN_ALIAS_LEN));
                                    continue;
                                }
                                break;
                                
                            case 'f':
                                if (commandName.compare(0, FOOTER_HIGHLIGHT_PATTERN_LEN, FOOTER_HIGHLIGHT_PATTERN) == 0) {
                                    switch (commandName[FOOTER_HIGHLIGHT_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(FOOTER_HIGHLIGHT_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                commandFooterHighlight = true;
                                                commandFooterHighlightDefined = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(FOOTER_HIGHLIGHT_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                commandFooterHighlight = false;
                                                commandFooterHighlightDefined = true;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                if (commandName.compare(0, FOOTER_PATTERN_LEN, FOOTER_PATTERN) == 0) {
                                    commandFooter = commandName.substr(FOOTER_PATTERN_LEN);
                                    // If there are additional arguments, append them (handles spaces in footer)
                                    for (size_t j = 1; j < cmd.size(); ++j) {
                                        commandFooter += " " + cmd[j];
                                    }
                                    removeQuotes(commandFooter);
                                    continue;
                                }
                                break;
                                
                            case 'h':
                                if (commandName.compare(0, HOLD_PATTERN_LEN, HOLD_PATTERN) == 0) {
                                    switch (commandName[HOLD_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(HOLD_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                isHold = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(HOLD_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                isHold = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                if (commandName.compare(0, HEADER_INDENT_PATTERN_LEN, HEADER_INDENT_PATTERN) == 0) {
                                    switch (commandName[HEADER_INDENT_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(HEADER_INDENT_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                useHeaderIndent = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(HEADER_INDENT_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                useHeaderIndent = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                                
                            case 'p':
                                if (commandName.compare(0, PROGRESS_PATTERN_LEN, PROGRESS_PATTERN) == 0) {
                                    switch (commandName[PROGRESS_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(PROGRESS_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                usingProgress = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(PROGRESS_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                usingProgress = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                if (commandName.compare(0, POLLING_PATTERN_LEN, POLLING_PATTERN) == 0) {
                                    switch (commandName[POLLING_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(POLLING_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                isPolling = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(POLLING_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                isPolling = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                                
                            case 't':
                                if (commandName.compare(0, TOP_PIVOT_PATTERN_LEN, TOP_PIVOT_PATTERN) == 0) {
                                    switch (commandName[TOP_PIVOT_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(TOP_PIVOT_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                usingTopPivot = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(TOP_PIVOT_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                usingTopPivot = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                                
                            case 'b':
                                if (commandName.compare(0, BOTTOM_PIVOT_PATTERN_LEN, BOTTOM_PIVOT_PATTERN) == 0) {
                                    switch (commandName[BOTTOM_PIVOT_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(BOTTOM_PIVOT_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                usingBottomPivot = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(BOTTOM_PIVOT_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                usingBottomPivot = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                if (commandName.compare(0, BACKGROUND_PATTERN_LEN, BACKGROUND_PATTERN) == 0) {
                                    switch (commandName[BACKGROUND_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(BACKGROUND_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                hideTableBackground = false;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(BACKGROUND_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                hideTableBackground = true;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                                
                            case 'e':
                                if (commandName.compare(0, END_GAP_PATTERN_LEN, END_GAP_PATTERN) == 0) {
                                    tableEndGap = ult::stoi(commandName.substr(END_GAP_PATTERN_LEN));
                                    continue;
                                }
                                break;
                                
                            case 'o':
                                if (commandName.compare(0, OFFSET_PATTERN_LEN, OFFSET_PATTERN) == 0) {
                                    tableColumnOffset = ult::stoi(commandName.substr(OFFSET_PATTERN_LEN));
                                    continue;
                                }
                                if (commandName.compare(0, ON_EVERY_TICK_PATTERN_LEN, ON_EVERY_TICK_PATTERN) == 0) {
                                    switch (commandName[ON_EVERY_TICK_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(ON_EVERY_TICK_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                onEveryTick = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(ON_EVERY_TICK_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                onEveryTick = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                                
                            case 'i':
                                if (commandName.compare(0, INFO_TEXT_COLOR_PATTERN_LEN, INFO_TEXT_COLOR_PATTERN) == 0) {
                                    tableInfoTextColor = commandName.substr(INFO_TEXT_COLOR_PATTERN_LEN);
                                    continue;
                                }
                                break;
                                
                            case 'a':
                                if (commandName.compare(0, ALIGNMENT_PATTERN_LEN, ALIGNMENT_PATTERN) == 0) {
                                    tableAlignment = commandName.substr(ALIGNMENT_PATTERN_LEN);
                                    continue;
                                }
                                break;
                                
                            case 'w':
                                if (commandName.compare(0, WRAPPING_MODE_PATTERN_LEN, WRAPPING_MODE_PATTERN) == 0) {
                                    tableWrappingMode = commandName.substr(WRAPPING_MODE_PATTERN_LEN);
                                    continue;
                                }
                                if (commandName.compare(0, WRAPPING_INDENT_PATTERN_LEN, WRAPPING_INDENT_PATTERN) == 0) {
                                    switch (commandName[WRAPPING_INDENT_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(WRAPPING_INDENT_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                useWrappingIndent = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(WRAPPING_INDENT_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                useWrappingIndent = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                                
                            case 'u':
                                if (commandName.compare(0, UNITS_PATTERN_LEN, UNITS_PATTERN) == 0) {
                                    units = commandName.substr(UNITS_PATTERN_LEN);
                                    removeQuotes(units);
                                    continue;
                                }
                                if (commandName.compare(0, UNLOCKED_PATTERN_LEN, UNLOCKED_PATTERN) == 0) {
                                    switch (commandName[UNLOCKED_PATTERN_LEN]) {
                                        case 't':
                                            if (commandName.compare(UNLOCKED_PATTERN_LEN, TRUE_STR_LEN, TRUE_STR) == 0) {
                                                unlockedTrackbar = true;
                                            }
                                            break;
                                        case 'f':
                                            if (commandName.compare(UNLOCKED_PATTERN_LEN, FALSE_STR_LEN, FALSE_STR) == 0) {
                                                unlockedTrackbar = false;
                                            }
                                            break;
                                    }
                                    continue;
                                }
                                break;
                        }
                        
                        // If we got here, it's a `;` command that didn't match any pattern
                        // Treat it as a comment and skip it
                        continue;
                    }
                    
                    if (commandMode == TOGGLE_STR) {
                        if (commandName.compare(0, 3, "on:") == 0)
                            currentSection = ON_STR;
                        else if (commandName.compare(0, 4, "off:") == 0)
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
                        continue;
                    }
                    
                    if (cmd.size() > 1) {
                        if (commandName == "file_source") {
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

            
            if (isFile(packageConfigIniPath)) {
                packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                bool shouldSaveINI = false;
                
                // Only sync footer if pattern was provided
                shouldSaveINI |= syncIniValue(packageConfigData, packageConfigIniPath, optionName, FOOTER_STR, commandFooter);
                
                // Always try to load footer_highlight from config.ini if it exists
                auto optionIt = packageConfigData.find(optionName);
                if (optionIt != packageConfigData.end()) {
                    auto footerHighlightIt = optionIt->second.find("footer_highlight");
                    if (footerHighlightIt != optionIt->second.end() && !footerHighlightIt->second.empty()) {
                        // Load the non-empty value from config.ini
                        commandFooterHighlight = (footerHighlightIt->second == TRUE_STR);
                        commandFooterHighlightDefined = true;
                    }
                }
                
                // Only write footer_highlight back if it was defined in package INI but missing/empty in config
                if (commandFooterHighlightDefined && optionIt != packageConfigData.end()) {
                    auto it = optionIt->second.find("footer_highlight");
                    if (it == optionIt->second.end() || it->second.empty()) {
                        packageConfigData[optionName]["footer_highlight"] = commandFooterHighlight ? TRUE_STR : FALSE_STR;
                        shouldSaveINI = true;
                    }
                }
                
                // Save all changes in one batch write
                if (shouldSaveINI)
                    saveIniFileData(packageConfigIniPath, packageConfigData);
                packageConfigData.clear();
            } else { // write default data if settings are not loaded
                // Load any existing data first (might be empty if file doesn't exist)
                packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                // Only add footer if pattern was provided
                if (!commandFooter.empty() && commandFooter != NULL_STR) {
                    packageConfigData[optionName][FOOTER_STR] = commandFooter;
                }
                // Only add footer_highlight if it was defined in package INI
                if (commandFooterHighlightDefined) {
                    packageConfigData[optionName]["footer_highlight"] = commandFooterHighlight ? TRUE_STR : FALSE_STR;
                }
                
                // Save the config file once
                if ((!commandFooter.empty() && commandFooter != NULL_STR) || commandFooterHighlightDefined) {
                    saveIniFileData(packageConfigIniPath, packageConfigData);
                }
                packageConfigData.clear();
            }
            
            
            // Get Option name and footer
            const std::string originalOptionName = optionName;
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
                        tableColumnOffset = 164;
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
                        interpretAndExecuteCommands, getSourceReplacement, commands, originalOptionName, false, false, -1, unlockedTrackbar, onEveryTick);
                
                    // Set the SCRIPT_KEY listener
                    trackBar->setScriptKeyListener([commands, keyName = originalOptionName, packagePath, lastPackageHeader, showWidget, trackBar]() {
                        
                        
                        //const std::string valueStr = parseValueFromIniSection(packagePath+"config.ini", keyName, "value");
                        //std::string indexStr = parseValueFromIniSection(packagePath+"config.ini", keyName, "index");


                        // Use trackbar's CURRENT state instead of reading from file
                        const std::string valueStr = ult::to_string(trackBar->getProgress());
                        const std::string indexStr = ult::to_string(trackBar->getIndex());

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
                
                        const bool isFromMainMenu = (packagePath == PACKAGE_PATH);

                        // Switch to ScriptOverlay
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader, showWidget);
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
                        interpretAndExecuteCommands, getSourceReplacement, commands, originalOptionName, false, unlockedTrackbar, onEveryTick);
                    
                    // Set the SCRIPT_KEY listener
                    stepTrackBar->setScriptKeyListener([commands, keyName = originalOptionName, packagePath, lastPackageHeader, showWidget, stepTrackBar]() {
                        const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                        
                        // Parse the value and index from the INI file
                        //const std::string valueStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "value");
                        //std::string indexStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "index");


                        // Use stepTrackBar's CURRENT state
                        const std::string valueStr = ult::to_string(stepTrackBar->getProgress());
                        const std::string indexStr = ult::to_string(stepTrackBar->getIndex());

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
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader, showWidget);
                    });
                    
                    // Add the StepTrackBarV2 to the list
                    list->addItem(stepTrackBar);

                    continue;
                } else if (commandMode == NAMED_STEP_TRACKBAR_STR) {
                    //entryList.clear();
                    //entryList.shrink_to_fit();
                    std::vector<std::string> entryList = {};
                    
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
                        interpretAndExecuteCommands, getSourceReplacement, commands, originalOptionName, unlockedTrackbar, onEveryTick);
                    
                    // Set the SCRIPT_KEY listener
                    namedStepTrackBar->setScriptKeyListener([commands, keyName = originalOptionName, packagePath, entryList=entryList, lastPackageHeader, showWidget, namedStepTrackBar]() {
                        const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                    
                        // Parse the value and index from the INI file
                        //std::string valueStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "value");
                        //std::string indexStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "index");


                        // Use namedStepTrackBar's CURRENT state
                        const std::string indexStr = ult::to_string(namedStepTrackBar->getIndex());
                        const size_t entryIndex = std::min(static_cast<size_t>(namedStepTrackBar->getIndex()), entryList.size() - 1);
                        const std::string valueStr = entryList[entryIndex];
                    
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
                        tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader, showWidget);
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
                        //removeTag(cleanOptionName);
                        listItem = new tsl::elm::ListItem(cleanOptionName, footer, isMini, true);
                    }
                    else {
                        cleanOptionName = optionName;
                        //removeTag(cleanOptionName);
                        listItem = new tsl::elm::ListItem(cleanOptionName, "", isMini, true);

                        if (commandMode == OPTION_STR)
                            listItem->setValue(footer, commandFooterHighlightDefined ? !commandFooterHighlight : false);
                        else
                            listItem->setValue(footer, commandFooterHighlightDefined ? !commandFooterHighlight : true);
                    }
                    
                    if (footer == UNAVAILABLE_SELECTION || footer == NOT_AVAILABLE_STR || (footer.find(NULL_STR) != std::string::npos))
                        listItem->setValue(UNAVAILABLE_SELECTION, true);
                    if (commandMode == FORWARDER_STR) {

                        const std::string& forwarderPackagePath = getParentDirFromPath(packageSource);
                        const std::string& forwarderPackageIniName = getNameFromPath(packageSource);
                        listItem->setClickListener([commands, keyName = originalOptionName, dropdownSection, packagePath, currentPage, packageName, nestedLayer, cleanOptionName, listItem,
                            forwarderPackagePath, forwarderPackageIniName, lastPackageHeader, pageHeader, showWidget, i](s64 keys) mutable {

                            if (simulatedMenu.load(std::memory_order_acquire)) {
                                keys |= SYSTEM_SETTINGS_KEY;
                            }

                            if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                                interpretAndExecuteCommands(std::move(getSourceReplacement(commands, keyName, i, packagePath)), packagePath, keyName);
                                resetPercentages();

                                nestedMenuCount++;
                                //lastPackagePath = forwarderPackagePath;
                                //lastPackageName = forwarderPackageIniName;
                                if (dropdownSection.empty())
                                    lastPackageMenu = "packageMenu";
                                else
                                    lastPackageMenu = "subPackageMenu";

                                // set forwarder pointer for updating
                                //forwarderListItem = listItem;
                                //lastCommandMode = FORWARDER_STR;
                                //lastKeyName = keyName;
                                

                                // Fix: Don't pass section headers as pageHeader when returning to main package
                                //std::string returnPageHeader = "";
                                //if (nestedLayer > 0) {
                                //    // Only preserve pageHeader for nested contexts, not main package
                                //    returnPageHeader = lastPackageHeader;
                                //}


                                returnContextStack.push({
                                    .packagePath = packagePath,
                                    .sectionName = dropdownSection,
                                    .currentPage = currentPage,
                                    .packageName = packageName,
                                    .pageHeader = pageHeader,    // Move this before nestedLayer
                                    .option = cleanOptionName,
                                    .nestedLayer = nestedLayer          // Move this to the end
                                });
                                


                                allowSlide.exchange(false, std::memory_order_acq_rel);
                                unlockedSlide.exchange(false, std::memory_order_acq_rel);

                                //tsl::clearGlyphCacheNow.store(true, release);
                                tsl::swapTo<PackageMenu>(forwarderPackagePath, "", LEFT_STR, forwarderPackageIniName, nestedMenuCount, lastPackageHeader);
                                
                                return true;
                            } else if (keys & SCRIPT_KEY && !(keys & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
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
                                tsl::changeTo<ScriptOverlay>(std::move(getSourceReplacement(commands, keyName, i, packagePath)), packagePath, selectionItem, isFromMainMenu ? "main" : "package", true, lastPackageHeader, showWidget);
                                return true;
                            } else if ((packagePath == PACKAGE_PATH) && (keys & SYSTEM_SETTINGS_KEY) && !(keys & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK)) {
                                skipJumpReset.store(false, std::memory_order_release);
                                returnJumpItemName = cleanOptionName;
                                returnJumpItemValue = "";
                                return true;
                            }
                            return false;
                        });
                        listItem->disableClickAnimation();
                    } else {
                        listItem->setClickListener([commands, keyName = originalOptionName, cleanOptionName, dropdownSection, packagePath, packageName,
                            footer, lastSection, listItem, lastPackageHeader, commandMode, showWidget, i](uint64_t keys) {
                            //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem, [](auto*){})](uint64_t keys) {
                            
                            if (runningInterpreter.load(acquire))
                                return false;

                            if (simulatedMenu.load(std::memory_order_acquire)) {
                                keys |= SYSTEM_SETTINGS_KEY;
                            }

                            if (((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK)))) {
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
                                        //std::lock_guard<std::mutex> lock(jumpItemMutex);
                                        jumpItemName = "";
                                        jumpItemValue = CHECKMARK_SYMBOL;
                                        jumpItemExactMatch.store(true, release);
                                        //g_overlayFilename = "";
                                    } else {
                                        //std::lock_guard<std::mutex> lock(jumpItemMutex);
                                        jumpItemName = "";
                                        jumpItemValue = "";
                                        jumpItemExactMatch.store(true, release);
                                        //g_overlayFilename = "";
                                    }

                                    tsl::changeTo<SelectionOverlay>(packagePath, keyName, newKey, lastPackageHeader, commands, showWidget);
                                    //lastKeyName = keyName;
                                }
                                return true;
                            } else if (keys & SCRIPT_KEY && !(keys & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                                const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inMainMenu) {
                                //    isFromMainMenu = true;
                                //    inMainMenu.store(false, std::memory_order_release);
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
                                tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, selectionItem, isFromMainMenu ? "main" : "package", true, lastPackageHeader, showWidget);
                                return true;
                            } else if ((packagePath == PACKAGE_PATH) && (keys & SYSTEM_SETTINGS_KEY) && !(keys & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK)) {
                                skipJumpReset.store(false, std::memory_order_release);
                                returnJumpItemName = cleanOptionName;
                                returnJumpItemValue = "";
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
                        //removeTag(cleanOptionName);
                        tsl::elm::ListItem* listItem = new tsl::elm::ListItem(cleanOptionName, "", isMini, true);

                        if (commandMode == DEFAULT_STR)
                            listItem->setValue(footer, commandFooterHighlightDefined ? !commandFooterHighlight : true);
                        else
                            listItem->setValue(footer, commandFooterHighlightDefined ? !commandFooterHighlight : false);
                        

                        if (isHold)
                            listItem->disableClickAnimation();

                        listItem->setClickListener([i, commands, keyName = originalOptionName, cleanOptionName, packagePath, packageName,
                            selectedItem, listItem, lastPackageHeader, commandMode, footer, isHold, showWidget, commandFooterHighlight, commandFooterHighlightDefined](uint64_t keys) {
                            
                            if (runningInterpreter.load(acquire)) {
                                return false;
                            }

                            if (simulatedMenu.load(std::memory_order_acquire)) {
                                keys |= SYSTEM_SETTINGS_KEY;
                            }

                            if (((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK)))) {
                                isDownloadCommand.store(false, release);
                                runningInterpreter.store(true, release);

                                auto modifiedCmds = getSourceReplacement(commands, selectedItem, i, packagePath);
                                if (isHold) {
                                    lastSelectedListItemFooter = footer;
                                    lastFooterHighlight = commandFooterHighlight;  // STORE THE VALUE
                                    lastFooterHighlightDefined = commandFooterHighlightDefined;  // STORE WHETHER IT WAS DEFINED
                                    listItem->setValue(INPROGRESS_SYMBOL);
                                    lastSelectedListItem = listItem;
                                    holdStartTick = armGetSystemTick();
                                    storedCommands = std::move(modifiedCmds);
                                    lastCommandMode = commandMode;
                                    lastCommandIsHold = true;
                                    lastKeyName = keyName;
                                    return true;
                                }

                                //logMessage("selectedItem: "+selectedItem+" keyName: "+keyName);
                                executeInterpreterCommands(std::move(modifiedCmds), packagePath, keyName);
                                //startInterpreterThread(packagePath);
                                //listItem->disableClickAnimation();
                                listItem->setValue(INPROGRESS_SYMBOL);
                                
                                //lastSelectedListItem = nullptr;
                                lastSelectedListItem = listItem;
                                lastFooterHighlight = commandFooterHighlight;  // STORE THE VALUE
                                lastFooterHighlightDefined = commandFooterHighlightDefined;  // STORE WHETHER IT WAS DEFINED
                                shiftItemFocus(listItem);
                                lastCommandMode = commandMode;
                                lastKeyName = keyName;

                                lastRunningInterpreter.store(true, std::memory_order_release);
                                if (lastSelectedListItem)
                                    lastSelectedListItem->triggerClickAnimation();
                                return true;
                            }  else if (keys & SCRIPT_KEY && !(keys & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                                const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                auto modifiedCmds = getSourceReplacement(commands, selectedItem, i, packagePath);
                                applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                                tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader, showWidget);
                                return true;
                            } else if ((packagePath == PACKAGE_PATH) && (keys & SYSTEM_SETTINGS_KEY) && !(keys & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK)) {
                                skipJumpReset.store(false, std::memory_order_release);
                                returnJumpItemName = cleanOptionName;
                                returnJumpItemValue = "";
                                return true;
                            }
                            return false;
                        });
                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(listItem);
                    } else if (commandMode == TOGGLE_STR) {
                        cleanOptionName = optionName;
                        //removeTag(cleanOptionName);
                        auto* toggleListItem = new tsl::elm::ToggleListItem(cleanOptionName, false, ON, OFF, isMini, true);

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
                        
                        toggleListItem->setStateChangedListener([i, usingProgress, toggleListItem, commandsOn, commandsOff, keyName = originalOptionName, packagePath,
                            pathPatternOn, pathPatternOff](bool state) {
                            if (runningInterpreter.load(std::memory_order_acquire)) {
                                return;
                            }
                            
                            
                            tsl::Overlay::get()->getCurrentGui()->requestFocus(toggleListItem, tsl::FocusDirection::None);
                            
                            if (usingProgress)
                                toggleListItem->setValue(INPROGRESS_SYMBOL);
                            nextToggleState = !state ? CAPITAL_OFF_STR : CAPITAL_ON_STR;
                            lastKeyName = keyName;
                            runningInterpreter.store(true, release);
                            lastRunningInterpreter.store(true, release);
                            lastSelectedListItem = toggleListItem;

                            // Now pass the preprocessed paths to getSourceReplacement
                            //interpretAndExecuteCommands(std::move(state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                            //    getSourceReplacement(commandsOff, pathPatternOff, i, packagePath)), packagePath, keyName);
                            
                            executeInterpreterCommands(std::move(state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                                getSourceReplacement(commandsOff, pathPatternOff, i, packagePath)), packagePath, keyName);
                            //resetPercentages();
                            // Set the ini file value after executing the command
                            //setIniFileValue((packagePath + CONFIG_FILENAME), keyName, FOOTER_STR, state ? CAPITAL_ON_STR : CAPITAL_OFF_STR);
                            
                        });

                        // Set the script key listener (for SCRIPT_KEY)
                        toggleListItem->setScriptKeyListener([i, commandsOn, commandsOff, keyName = originalOptionName, packagePath,
                            pathPatternOn, pathPatternOff, lastPackageHeader, showWidget](bool state) {

                            const bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                            //if (inPackageMenu)
                            //    inPackageMenu = false;
                            //if (inSubPackageMenu)
                            //    inSubPackageMenu = false;

                            // Custom logic for SCRIPT_KEY handling
                            auto modifiedCmds = state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                                getSourceReplacement(commandsOff, pathPatternOff, i, packagePath);

                            applyPlaceholderReplacementsToCommands(modifiedCmds, packagePath);
                            tsl::changeTo<ScriptOverlay>(std::move(modifiedCmds), packagePath, keyName, isFromMainMenu ? "main" : "package", false, lastPackageHeader, showWidget);
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
    optionName.clear();
    commands.clear();
    commandsOn.clear();
    commandsOff.clear();
    tableData.clear();

    if (onlyTables) {
        //auto dummyItem = new tsl::elm::DummyListItem();
        //list->addItem(dummyItem, 0, 1);
        addDummyListItem(list, 1); // assuming a header is always above
        //addDummyListItem(list);
        //list->disableCaching(); // causes a visual glitch when entered fresh on return, may look into again later.
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
    PackageMenu(const std::string& path, const std::string& sectionName = "", const std::string& page = LEFT_STR,
        const std::string& _packageName = PACKAGE_FILENAME, const size_t _nestedlayer = 0, const std::string& _pageHeader = "") :
        packagePath(path), dropdownSection(sectionName), currentPage(page), packageName(_packageName), nestedLayer(_nestedlayer), pageHeader(_pageHeader) {
            std::lock_guard<std::mutex> lock(transitionMutex);
            //if (nestedLayer > 0)
            //    hexSumCache.clear();
            //{
            //    //std::lock_guard<std::mutex> lock(jumpItemMutex);
            //    if (!skipJumpReset.load(acquire)) {
            //        jumpItemName = "";
            //        jumpItemValue = "";
            //        jumpItemExactMatch.store(true, release);
            //        //g_overlayFilename = "";
            //    } else
            //        skipJumpReset.store(false, release);
            //}

            if (!skipJumpReset.load(acquire)) {
                jumpItemName = "";
                jumpItemValue = "";
                jumpItemExactMatch.store(true, release);
                //g_overlayFilename = "";
            } else
                skipJumpReset.store(false, release);
            //tsl::clearGlyphCacheNow.store(true, release);
            settingsInitialized.exchange(true, acq_rel);
        }

    /**
     * @brief Destroys the `PackageMenu` instance.
     *
     * Cleans up any resources associated with the `PackageMenu` instance.
     */
    ~PackageMenu() {
        std::lock_guard<std::mutex> lock(transitionMutex);

        if (returningToMain || returningToHiddenMain) {
            tsl::clearGlyphCacheNow.store(true, release);
            clearMemory();
            
            packageRootLayerTitle = packageRootLayerVersion = packageRootLayerColor = "";
            overrideTitle = false;
            overrideVersion = false;

            if (isFile(packagePath + EXIT_PACKAGE_FILENAME)) {
                const bool useExitPackage = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_EXIT_PACKAGE_STR) == FALSE_STR);
                
                if (useExitPackage) {
                    // Load only the commands from the specific section (bootCommandName)
                    auto exitCommands = loadSpecificSectionFromIni(packagePath + EXIT_PACKAGE_FILENAME, "exit");
                    
                    if (!exitCommands.empty()) {
                        //bool resetCommandSuccess = false;
                        //if (!commandSuccess.load(acquire)) resetCommandSuccess = true;
                        const bool resetCommandSuccess = !commandSuccess.load(std::memory_order_acquire);
                        
                        interpretAndExecuteCommands(std::move(exitCommands), packagePath, "exit");
                        resetPercentages();

                        if (resetCommandSuccess) {
                            commandSuccess.store(false, release);
                        }
                    }
                }
            }
            lastSelectedListItem = nullptr;

        //} else {
        //    hexSumCache.clear();
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
        std::lock_guard<std::mutex> lock(transitionMutex);
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
        
        const bool showWidget = (!packageHeader.show_widget.empty() && packageHeader.show_widget == TRUE_STR);
        
        std::string pageLeftName, pageRightName;
        bool noClickableItems = drawCommandsMenu(list, packageIniPath, packageConfigIniPath, packageHeader, this->pageHeader, pageLeftName, pageRightName,
            this->packagePath, this->currentPage, this->packageName, this->dropdownSection, this->nestedLayer,
            this->pathPattern, this->pathPatternOn, this->pathPatternOff, this->usingPages, true, showWidget
        );
        

        if (nestedLayer == 0) {

            if (!packageRootLayerTitle.empty())
                overrideTitle = true;
            if (!packageRootLayerVersion.empty())
                overrideVersion = true;

            if (!packageHeader.title.empty() && packageRootLayerTitle.empty())
                packageRootLayerTitle = packageHeader.title;
            if (!packageHeader.display_title.empty())
                packageRootLayerTitle = packageHeader.display_title;
            if (!packageHeader.version.empty() && packageRootLayerVersion.empty())
                packageRootLayerVersion = packageHeader.version;
            if (!packageHeader.color.empty() && packageRootLayerColor.empty())
                packageRootLayerColor = packageHeader.color;
        }
        if (packageHeader.title.empty() || overrideTitle)
            packageHeader.title = packageRootLayerTitle;
        if (!packageHeader.display_title.empty() || overrideTitle)
            packageHeader.display_title = packageRootLayerTitle;
        if (packageHeader.version.empty() || overrideVersion)
            packageHeader.version = packageRootLayerVersion;
        if (packageHeader.color.empty())
            packageHeader.color = packageRootLayerColor;

        auto* rootFrame = new tsl::elm::OverlayFrame(
           (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(packagePath)),
           ((!pageHeader.empty() && packageHeader.show_version != TRUE_STR) ? pageHeader: (packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + " "+DIVIDER_SYMBOL+" Ultrahand Package" : "Ultrahand Package")),
           noClickableItems,
           "",
           packageHeader.color,
           (usingPages && currentPage == RIGHT_STR) ? pageLeftName : "",
           (usingPages && currentPage == LEFT_STR) ? pageRightName : ""
        );

        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch);
        rootFrame->setContent(list);
        if (showWidget)
            rootFrame->m_showWidget = true;
        return rootFrame;
    }
    

    //void handleForwarderFooter() {
    //    //if (lastCommandMode == FORWARDER_STR && isFile(packageConfigIniPath)) {
    //    //    auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
    //    //    auto it = packageConfigData.find(lastKeyName);
    //    //    if (it != packageConfigData.end()) {
    //    //        auto& optionSection = it->second;
    //    //        auto footerIt = optionSection.find(FOOTER_STR);
    //    //        if (footerIt != optionSection.end() && (footerIt->second.find(NULL_STR) == std::string::npos)) {
    //    //            if (forwarderListItem)
    //    //                forwarderListItem->setValue(footerIt->second);
    //    //        }
    //    //    }
    //    //    lastCommandMode = "";
    //    //}
    //}

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
        
        bool isHolding = (lastCommandIsHold && runningInterpreter.load(std::memory_order_acquire));
        if (isHolding) {
            processHold(keysDown, keysHeld, holdStartTick, isHolding, [&]() {
                // Execute interpreter commands if needed
                displayPercentage.store(-1, std::memory_order_release);
                //lastCommandMode.clear();
                lastCommandIsHold = false;
                //lastKeyName.clear();
                lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
                //lastSelectedListItemFooter.clear();
                //lastSelectedListItem->enableClickAnimation();
                //lastSelectedListItem->triggerClickAnimation();
                //lastSelectedListItem->disableClickAnimation();
                triggerEnterFeedback();
                executeInterpreterCommands(std::move(storedCommands), packagePath, lastKeyName);
                lastRunningInterpreter.store(true, std::memory_order_release);
            }, nullptr, true); // true = reset storedCommands
            return true;
        }
        

        const bool isRunningInterp = runningInterpreter.load(acquire);
        const bool isTouching = stillTouching.load(acquire);
        
        if (isRunningInterp) {
            return handleRunningInterpreter(keysDown, keysHeld);
        }
            
        if (lastRunningInterpreter.exchange(false, std::memory_order_acq_rel)) {
            //tsl::clearGlyphCacheNow.store(true, release);
            isDownloadCommand.store(false, release);
        
            if (lastSelectedListItem) {
                const bool success = commandSuccess.load(acquire);
        
                if (lastCommandMode == OPTION_STR || lastCommandMode == SLOT_STR) {
                    if (success) {
                        if (isFile(packageConfigIniPath)) {
                            auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                            if (auto it = packageConfigData.find(lastKeyName); it != packageConfigData.end()) {
                                auto& optionSection = it->second;
                
                                // Update footer if valid
                                if (auto footerIt = optionSection.find(FOOTER_STR);
                                    footerIt != optionSection.end() &&
                                    footerIt->second.find(NULL_STR) == std::string::npos)
                                {
                                    // Check if footer_highlight exists in the INI
                                    auto footerHighlightIt = optionSection.find("footer_highlight");
                                    if (footerHighlightIt != optionSection.end()) {
                                        // Use the value from the INI
                                        bool footerHighlightFromIni = (footerHighlightIt->second == TRUE_STR);
                                        lastSelectedListItem->setValue(footerIt->second, !footerHighlightFromIni);
                                    } else {
                                        // footer_highlight not defined in INI, use default
                                        lastSelectedListItem->setValue(footerIt->second, false);
                                    }
                                }
                            }
                
                            lastCommandMode.clear();
                        } else {
                            lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
                        }
                    } else {
                        lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
                    }
                } else {
                    // Handle toggle or checkmark logic
                    if (nextToggleState.empty()) {
                        lastSelectedListItem->setValue(success ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
                    } else {
                        // Determine the final state to save
                        const std::string finalState = success 
                            ? nextToggleState 
                            : (nextToggleState == CAPITAL_ON_STR ? CAPITAL_OFF_STR : CAPITAL_ON_STR);
                        
                        // Update displayed value
                        lastSelectedListItem->setValue(finalState);
                
                        // Update toggle state
                        static_cast<tsl::elm::ToggleListItem*>(lastSelectedListItem)
                            ->setState(finalState == CAPITAL_ON_STR);
                
                        // Save to config file
                        setIniFileValue(packageConfigIniPath, lastKeyName, FOOTER_STR, finalState);
                        
                        lastKeyName.clear();
                        nextToggleState.clear();
                    }
                }
        
                lastSelectedListItem->enableClickAnimation();
                lastSelectedListItem = nullptr;
                lastFooterHighlight = lastFooterHighlightDefined = false;
            }
        
            closeInterpreterThread();
            resetPercentages();
            

            if (!commandSuccess.load()) {
                triggerRumbleDoubleClick.store(true, std::memory_order_release);
            }

            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
            //lastRunningInterpreter.store(false, std::memory_order_release);
            return true;
        }

        if (ult::refreshWallpaperNow.exchange(false, std::memory_order_acq_rel)) {
            closeInterpreterThread();
            ult::reloadWallpaper();
            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
        }
    
        if (goBackAfter.exchange(false, std::memory_order_acq_rel)) {
            disableSound.store(true, std::memory_order_release);
            simulatedBack.store(true, std::memory_order_release);
            return true;
        }
    
        if (!returningToPackage && !isTouching) {
            if (refreshPage.exchange(false, std::memory_order_acq_rel)) {
                
                // Function to handle the transition and state resetting
                auto handleMenuTransition = [&] {
                    //const std::string lastPackagePath = packagePath;
                    //const std::string lastDropdownSection = dropdownSection;
                    //const std::string lastPage = currentPage;
                    //const std::string lastPackageName = packageName;
                    //const size_t lastNestedLayer = nestedLayer;
                    
                    inSubPackageMenu = false;
                    inPackageMenu = false;
    
                    //selectedListItem = nullptr;
                    //lastSelectedListItem = nullptr;
                    //tsl::clearGlyphCacheNow.store(true, release);
                    //tsl::goBack();
                    tsl::swapTo<PackageMenu>(packagePath, dropdownSection, currentPage, packageName, nestedLayer, pageHeader);
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
            if (refreshPackage.load(acquire)) {
                if (nestedMenuCount == nestedLayer) {
                    //astPackagePath = packagePath;
                    //lastPage = currentPage;
                    //lastPackageName = PACKAGE_FILENAME;
                    
                    //tsl::goBack(nestedMenuCount+1);
                    //nestedMenuCount = 0;
    
                    tsl::swapTo<PackageMenu>(SwapDepth(nestedMenuCount+1), packagePath, "");
                    nestedMenuCount = 0;
                    inPackageMenu = true;
                    inSubPackageMenu = false;
                    refreshPackage.store(false, release);
                    return true;
                }
            }
        }
        
        if (usingPages) {
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            {
                //std::lock_guard<std::mutex> lock(ult::simulatedNextPageMutex);
                if (simulatedNextPage.exchange(false, std::memory_order_acq_rel)) {
                    if (currentPage == LEFT_STR) {
                        keysDown |= KEY_DRIGHT;
                    }
                    else if (currentPage == RIGHT_STR) {
                        keysDown |= KEY_DLEFT;
                    }
                }
            }
    
            // Cache slide-related values
            
            const bool onTrack = onTrackBar.load(acquire);
            const bool slideAllowed = allowSlide.load(acquire);
            const bool slideUnlocked = unlockedSlide.load(acquire);
            const bool slideCondition = ((!slideAllowed && onTrack && !slideUnlocked) || (onTrack && keysHeld & KEY_R)) || !onTrack;
            
            // Helper lambda for slide transitions
            auto resetSlideState = [&]() {
                //if (allowSlide.load(acquire))
                allowSlide.store(false, release);
                //if (unlockedSlide.load(acquire))
                unlockedSlide.store(false, release);
            };
    
            if (currentPage == LEFT_STR) {
                if (!isTouching && slideCondition && (keysDown & KEY_RIGHT) && (!onTrack ? !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK) : !(keysHeld & ~KEY_R & ~KEY_RIGHT & ALL_KEYS_MASK))) {
                    {
                        std::lock_guard<std::mutex> lock(tsl::elm::s_safeToSwapMutex);
                        if (tsl::elm::s_safeToSwap.load(acquire)) {
                            //lastPage = RIGHT_STR;
                            tsl::swapTo<PackageMenu>(packagePath, dropdownSection, RIGHT_STR, packageName, nestedLayer, pageHeader);
                            resetSlideState();
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                        }
                        return true;
                    }
                    
                }
            } else if (currentPage == RIGHT_STR) {
                if (!isTouching && slideCondition && (keysDown & KEY_LEFT) && (!onTrack ? !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK) : !(keysHeld & ~KEY_R & ~KEY_LEFT & ALL_KEYS_MASK))) {
                    {
                        std::lock_guard<std::mutex> lock(tsl::elm::s_safeToSwapMutex);
                        if (tsl::elm::s_safeToSwap.load(acquire)) {
                            //lastPage = LEFT_STR;
                            tsl::swapTo<PackageMenu>(packagePath, dropdownSection, LEFT_STR, packageName, nestedLayer, pageHeader);
                            resetSlideState();
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                        }
                        return true;
                    }
                    
                }
            } 
        }
        
        // Common back key condition
        const bool backKeyPressed = !isTouching && (((keysDown & KEY_B) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
        
        // Helper lambda for common back key handling logic
        auto handleBackKeyCommon = [&]() {
            //handleForwarderFooter();
            allowSlide.exchange(false, std::memory_order_acq_rel);
            unlockedSlide.exchange(false, std::memory_order_acq_rel);
            
            // Check if we have return context items to process first
            if (!returnContextStack.empty()) {
                // Don't set returning flags yet - let the return context handle navigation
                return false; // Let tryReturnContext() handle this
            }
    

            if (nestedMenuCount == 0) {
                inPackageMenu = false;
                if (!inHiddenMode.load(std::memory_order_acquire))
                    returningToMain = true;
                else
                    returningToHiddenMain = true;
                
                if (!selectedPackage.empty()) {
                    while (!returnContextStack.empty()) {
                        returnContextStack.pop();
                    }
                    ult::launchingOverlay.store(true, std::memory_order_release);
                    tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                    exitingUltrahand.store(true, release);
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
                //{
                //    //std::lock_guard<std::mutex> lock(jumpItemMutex);
                //    jumpItemName = (packageRootLayerIsStarred ? STAR_SYMBOL + "  " + packageRootLayerTitle : packageRootLayerTitle) + "?" + packageRootLayerName;
                //    jumpItemValue = hidePackageVersions ? "" : packageRootLayerVersion;
                //    jumpItemExactMatch.store(true, release);
                //    //g_overlayFilename = "";
                //    skipJumpReset.store(true, release);
                //}
                jumpItemName.clear();
                jumpItemValue.clear();
                
                //tsl::clearGlyphCacheNow.store(true, release);
                //tsl::pop();
                
                //if (returningToMain) {
                //    tsl::changeTo<MainMenu>(PACKAGES_STR);
                //} else {
                //    tsl::changeTo<MainMenu>();
                //}
                tsl::swapTo<MainMenu>();
            } else {
                //tsl::clearGlyphCacheNow.store(true, release);
                tsl::goBack();
                
            }
        };
    
        // Helper lambda for return context handling  
        auto tryReturnContext = [&]() -> bool {
            if (!returnContextStack.empty()) {
                ReturnContext returnTo = returnContextStack.top();
                returnContextStack.pop();
                
                // Decrement nestedMenuCount when returning via context
                if (nestedMenuCount > 0)
                    nestedMenuCount--;
                
                jumpItemName = returnTo.option;
                jumpItemValue = "";
                jumpItemExactMatch = false;
                skipJumpReset.store(true, std::memory_order_release);
                
                // Set appropriate states for return
                inSubPackageMenu = false;
                inPackageMenu = false;  
                returningToPackage = true;
                lastMenu = "packageMenu";
                
                tsl::swapTo<PackageMenu>(
                    SwapDepth(2),
                    returnTo.packagePath,      // Where we came FROM
                    returnTo.sectionName,      // Source section
                    returnTo.currentPage,      // Source page
                    returnTo.packageName,      // Source package
                    returnTo.nestedLayer,      // Source nesting level
                    returnTo.pageHeader        // Source header
                );
                return true;
            }
            return false;
        };
        
        // Helper lambda for normal back navigation
        auto handleNormalBack = [&]() -> bool {
            if (handleBackKeyCommon()) return true;
            
            // Try return context first (for forwarders)
            if (tryReturnContext()) return true;
            
            // Normal back navigation
            if (nestedMenuCount == 0) {
                handleMainMenuReturn();
            } else {
                tsl::goBack();
            }
            return true;
        };
        
        // Handle main package menu (dropdownSection is empty)
        if (!returningToPackage && inPackageMenu && nestedMenuCount == nestedLayer) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            if (!usingPages || (usingPages && currentPage == LEFT_STR)) {
                if (backKeyPressed) {
                    return handleNormalBack();
                }
            } else if (usingPages && currentPage == RIGHT_STR) {
                if (backKeyPressed) {
                    //lastPage = LEFT_STR; 
                    return handleNormalBack();
                }
            }
        }
        
        // Handle sub-package menu (dropdownSection is not empty)
        if (!returningToSubPackage && inSubPackageMenu) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
            
            if (!usingPages || (usingPages && currentPage == LEFT_STR)) {
                if (backKeyPressed) {
                    //handleForwarderFooter();
                    allowSlide.exchange(false, std::memory_order_acq_rel);
                    unlockedSlide.exchange(false, std::memory_order_acq_rel);
                    
                    // Try return context first, fallback to normal navigation
                    if (tryReturnContext()) return true;
                    
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
                    tsl::goBack();
                    return true;
                }
            } else if (usingPages && currentPage == RIGHT_STR) {
                if (backKeyPressed) {
                    //handleForwarderFooter();
                    allowSlide.exchange(false, std::memory_order_acq_rel);
                    unlockedSlide.exchange(false, std::memory_order_acq_rel);
                    
                    // Try return context first, fallback to normal navigation
                    if (tryReturnContext()) return true;
                    
                    inSubPackageMenu = false;
                    returningToPackage = true;
                    lastMenu = "packageMenu";
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
                //lastPackagePath = packagePath;
                //lastPackageName = PACKAGE_FILENAME;
            }
        }
        
        if (returningToSubPackage && !(keysDown & KEY_B)){
            lastPackageMenu = "";
            returningToPackage = false;
            returningToSubPackage = false;
            inPackageMenu = false;
            inSubPackageMenu = true;
            if (nestedMenuCount == 0 && nestedLayer == 0) {
                //lastPackagePath = packagePath;
                //lastPackageName = PACKAGE_FILENAME;
            }
        }
        
        if (triggerExit.exchange(false, std::memory_order_acq_rel)) {
            while (!returnContextStack.empty()) {
                returnContextStack.pop();
            }
            ult::launchingOverlay.store(true, std::memory_order_release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        // Fallback for lost navigations
        if (backKeyPressed) {
            if (!selectedPackage.empty()) {
                ult::launchingOverlay.store(true, std::memory_order_release);
                exitingUltrahand.store(true, release);
                tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                tsl::Overlay::get()->close();
                return true;
            }
    
            allowSlide.exchange(false, std::memory_order_acq_rel);
            unlockedSlide.exchange(false, std::memory_order_acq_rel);
            
            // Try return context first for any lost navigation scenarios
            if (tryReturnContext()) return true;
            
            // Fallback to normal navigation
            inSubPackageMenu = false;
            returningToPackage = true;
            lastMenu = "packageMenu";
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
        std::lock_guard<std::mutex> lock(transitionMutex);
        //tsl::gfx::FontManager::clearCache();
        {
            //std::lock_guard<std::mutex> lock(jumpItemMutex);
            if (skipJumpReset.exchange(false, std::memory_order_acq_rel)) {
                return;
            }
            jumpItemName = std::move(returnJumpItemName);
            jumpItemValue = std::move(returnJumpItemValue);
            jumpItemExactMatch.store(false, release);
        }
        settingsInitialized.store(true, release);
    }
    /**
     * @brief Destroys the `MainMenu` instance.
     *
     * Cleans up any resources associated with the `MainMenu` instance.
     */
    ~MainMenu() {
        std::lock_guard<std::mutex> lock(transitionMutex);
    }
    
    /**
     * @brief Creates the graphical user interface (GUI) for the main menu overlay.
     *
     * This function initializes and sets up the GUI elements for the main menu overlay,
     * allowing users to navigate and access various submenus.
     *
     * @return A pointer to the GUI element representing the main menu overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        std::lock_guard<std::mutex> lock(transitionMutex);
    
        // Handle hidden mode flags
        {
            auto iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
            auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
            bool needsUpdate = false;
            
            auto overlayIt = ultrahandSection.find(IN_HIDDEN_OVERLAY_STR);
            if (overlayIt != ultrahandSection.end() && overlayIt->second == TRUE_STR) {
                inMainMenu.store(false, std::memory_order_release);
                inHiddenMode.store(true, std::memory_order_release);
                hiddenMenuMode = OVERLAYS_STR;
                skipJumpReset.store(true, release);
                ultrahandSection[IN_HIDDEN_OVERLAY_STR] = FALSE_STR;
                needsUpdate = true;
            } else {
                auto packageIt = ultrahandSection.find(IN_HIDDEN_PACKAGE_STR);
                if (packageIt != ultrahandSection.end() && packageIt->second == TRUE_STR) {
                    inMainMenu.store(false, std::memory_order_release);
                    inHiddenMode.store(true, std::memory_order_release);
                    hiddenMenuMode = PACKAGES_STR;
                    skipJumpReset.store(true, release);
                    ultrahandSection[IN_HIDDEN_PACKAGE_STR] = FALSE_STR;
                    needsUpdate = true;
                }
            }
            
            if (needsUpdate) {
                saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
            }
        }
    
        if (!inHiddenMode.load(std::memory_order_acquire) && dropdownSection.empty())
            inMainMenu.store(true, std::memory_order_release);
        else
            inMainMenu.store(false, std::memory_order_release);
        
        lastMenuMode = hiddenMenuMode;
        
        static bool hasInitialized = false;
        if (!hasInitialized) {
            if (!inOverlay) {
                currentMenu = usePageSwap ? PACKAGES_STR : OVERLAYS_STR;
            }
            hasInitialized = true;
        }
        
        if (toPackages) {
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "to_packages", FALSE_STR);
            toPackages = false;
            currentMenu = PACKAGES_STR;
        }
    
        menuMode = !hiddenMenuMode.empty() ? hiddenMenuMode : currentMenu;
        
        auto* list = new tsl::elm::List();
        bool noClickableItems = false;
        
        if (menuMode == OVERLAYS_STR) {
            createOverlaysMenu(list);
        } else if (menuMode == PACKAGES_STR) {
            noClickableItems = createPackagesMenu(list);
        }
    
        auto* rootFrame = new tsl::elm::OverlayFrame(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel, noClickableItems, menuMode+hiddenMenuMode+dropdownSection, "", "", "");
        
        list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
        //if (g_overlayFilename != "ovlmenu.ovl") {
        //    list->jumpToItem(jumpItemName, jumpItemValue, jumpItemExactMatch.load(acquire));
        //} else {
        //    //g_overlayFilename = "";
        //}

        
        rootFrame->setContent(list);
        return rootFrame;
    }
    
        
    void createOverlaysMenu(tsl::elm::List* list) {
        inOverlaysPage.store(true, std::memory_order_release);
        inPackagesPage.store(false, std::memory_order_release);
    
        addHeader(list, (!inHiddenMode.load(std::memory_order_acquire) ? OVERLAYS : HIDDEN_OVERLAYS)+" "+DIVIDER_SYMBOL+" \uE0E3 "+SETTINGS+" "+DIVIDER_SYMBOL+" \uE0E2 "+FAVORITE);
        
        std::vector<std::string> overlayFiles = getFilesListByWildcards(OVERLAY_PATH+"*.ovl");
        
        #if !USING_FSTREAM_DIRECTIVE
        if (!isFile(OVERLAYS_INI_FILEPATH)) {
            FILE* createFile = fopen(OVERLAYS_INI_FILEPATH.c_str(), "w");
            if (createFile) fclose(createFile);
        }
        #else
        if (!isFile(OVERLAYS_INI_FILEPATH)) {
            std::ofstream createFile(OVERLAYS_INI_FILEPATH);
            createFile.close();
        }
        #endif
    
        if (overlayFiles.empty()) return;
    
        std::set<std::string> overlaySet;
        bool drawHiddenTab = false;
        
        // Scope to immediately free INI data after processing
        {
            auto overlaysIniData = getParsedDataFromIniFile(OVERLAYS_INI_FILEPATH);
            bool overlaysNeedsUpdate = false;
            bool foundOvlmenu = false;
    
            // Filter in one pass, remove ovlmenu and dot files
            overlayFiles.erase(
                std::remove_if(overlayFiles.begin(), overlayFiles.end(),
                    [&foundOvlmenu](const std::string& file) {
                        const std::string fileName = getNameFromPath(file);
                        if (!foundOvlmenu && fileName == "ovlmenu.ovl") {
                            foundOvlmenu = true;
                            return true;
                        }
                        return fileName.front() == '.';
                    }),
                overlayFiles.end()
            );
    
            // Pre-allocate string buffers to avoid repeated allocations
            std::string overlayFileName, overlayEntryKey;
            overlayFileName.reserve(64);
            overlayEntryKey.reserve(256);
    
            for (auto& overlayFile : overlayFiles) {
                overlayFileName = getNameFromPath(overlayFile);
                overlayFile.clear(); // Free memory immediately
                
                auto it = overlaysIniData.find(overlayFileName);
                if (it == overlaysIniData.end()) {
                    const auto& [result, overlayName, overlayVersion, usingLibUltrahand, supportsAMS110] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                    if (result != ResultSuccess) continue;
    
                    auto& overlaySection = overlaysIniData[overlayFileName];
                    overlaySection[PRIORITY_STR] = "20";
                    overlaySection[STAR_STR] = FALSE_STR;
                    overlaySection[HIDE_STR] = FALSE_STR;
                    overlaySection[USE_LAUNCH_ARGS_STR] = FALSE_STR;
                    overlaySection[LAUNCH_ARGS_STR] = "";
                    overlaySection["custom_name"] = "";
                    overlaySection["custom_version"] = "";
                    overlaysNeedsUpdate = true;
    
                    // Build entry key with single allocation
                    //overlayEntryKey.clear();
                    //overlayEntryKey.reserve(4 + overlayName.size() + 1 + overlayName.size() + 1 + 
                    //                       overlayVersion.size() + 1 + overlayFileName.size() + 8);

                    overlayEntryKey = "0020" + overlayName + ':' + overlayName + ':' + 
                                      overlayVersion + ':' + overlayFileName + ':' + 
                                      (usingLibUltrahand ? '1' : '0') + ':' + 
                                      (supportsAMS110 ? '1' : '0') + ":0";
                    overlaySet.emplace(std::move(overlayEntryKey));
                } else {
                    const std::string& hide = getValueOrDefault(it->second, HIDE_STR, FALSE_STR);
                    const bool isHidden = (hide == TRUE_STR);
                    
                    if (isHidden) {
                        if (!hideUnsupported || !requiresLNY2 || 
                            usingLNY2(OVERLAY_PATH + overlayFileName) || 
                            getValueOrDefault(it->second, "force_support", FALSE_STR) == TRUE_STR) {
                            drawHiddenTab = true;
                        }
                    }
                    
                    if (inHiddenMode.load(std::memory_order_acquire) == isHidden) {
                        const auto& [result, overlayName, overlayVersion, usingLibUltrahand, supportsAMS110] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                        if (result != ResultSuccess) continue;
                        
                        const std::string& priority = (it->second.find(PRIORITY_STR) != it->second.end()) ? formatPriorityString(it->second[PRIORITY_STR]) : "0020";
                        const std::string& starred = getValueOrDefault(it->second, STAR_STR, FALSE_STR);
                        const std::string& customName = getValueOrDefault(it->second, "custom_name", "");
                        const std::string& customVersion = getValueOrDefault(it->second, "custom_version", "");
                        
                        const std::string& assignedName = !customName.empty() ? customName : overlayName;
                        const std::string& assignedVersion = !customVersion.empty() ? customVersion : overlayVersion;
                        const bool forceAMS110Support = getValueOrDefault(it->second, "force_support", FALSE_STR) == TRUE_STR;
    
                        // Build entry key with single allocation
                        //overlayEntryKey.clear();
                        //const bool isStarred = (starred == TRUE_STR);
                        //const size_t estimatedSize = (isStarred ? 3 : 0) + priority.size() + 
                        //                            assignedName.size() * 2 + assignedVersion.size() + 
                        //                            overlayFileName.size() + 10;
                        //overlayEntryKey.reserve(estimatedSize);
                        
                        overlayEntryKey = (starred == TRUE_STR ? "-1:" : "") + priority + assignedName + ':' + 
                                          assignedName + ':' + assignedVersion + ':' + overlayFileName + ':' + 
                                          (usingLibUltrahand ? '1' : '0') + ':' + 
                                          (supportsAMS110 ? '1' : '0') + ':' + 
                                          (forceAMS110Support ? '1' : '0');
                        overlaySet.emplace(std::move(overlayEntryKey));
                    }
                }
            }
            
            if (overlaysNeedsUpdate) {
                saveIniFileData(OVERLAYS_INI_FILEPATH, overlaysIniData);
            }
        } // overlaysIniData freed here
        
        overlayFiles.clear();
        overlayFiles.shrink_to_fit();
        
        if (overlaySet.empty()) {
            addSelectionIsEmptyDrawer(list);
        } else {
            // Pre-allocate string buffers for parsing loop
            std::string overlayFileName, overlayName, overlayVersion, newOverlayName, displayVersion;
            overlayFileName.reserve(64);
            overlayName.reserve(128);
            overlayVersion.reserve(64);
            newOverlayName.reserve(192);
            displayVersion.reserve(64);
            

            // Helper to build return name for overlays (defined before listItem)
            auto buildOverlayReturnName = [](bool isStarred, const std::string& fileName, const std::string& displayName) -> std::string {
                std::string name;
                if (!isStarred) {
                    name.reserve(STAR_SYMBOL.size() + 2 + displayName.size() + 1 + fileName.size());
                    name = STAR_SYMBOL;
                    name += "  ";
                    name += displayName;
                } else {
                    name = displayName;
                }
                name += '?';
                name += fileName;
                return name;
            };


            // Process overlay set and add to list
            for (const auto& taintedOverlayFileName : overlaySet) {
                overlayFileName.clear();
                overlayName.clear();
                overlayVersion.clear();
                const bool overlayStarred = (taintedOverlayFileName.compare(0, 3, "-1:") == 0);
                bool usingLibUltrahand = false;
                bool supportsAMS110 = false;
                bool forceAMS110Support = false;
                
                // Parse from the end - optimized with fewer branches
                const size_t len = taintedOverlayFileName.size();
                size_t pos = len;
                size_t positions[6];
                size_t count = 0;
                
                // Unrolled loop for better performance on ARMv8-A
                while (pos > 0 && count < 6) {
                    pos = taintedOverlayFileName.rfind(':', pos - 1);
                    if (pos == std::string::npos) break;
                    positions[count++] = pos;
                }
                
                if (count == 6) {
                    // Direct character access instead of substr for single chars
                    forceAMS110Support = (taintedOverlayFileName[positions[0] + 1] == '1');
                    supportsAMS110 = (taintedOverlayFileName[positions[1] + 1] == '1');
                    usingLibUltrahand = (taintedOverlayFileName[positions[2] + 1] == '1');
                    overlayFileName.assign(taintedOverlayFileName, positions[3] + 1, positions[2] - positions[3] - 1);
                    overlayVersion.assign(taintedOverlayFileName, positions[4] + 1, positions[3] - positions[4] - 1);
                    overlayName.assign(taintedOverlayFileName, positions[5] + 1, positions[4] - positions[5] - 1);
                }
    
                const bool requiresAMS110Handling = (requiresLNY2 && !supportsAMS110 && !forceAMS110Support);
                if (hideUnsupported && requiresAMS110Handling)
                    continue;
    
                const std::string overlayFile = OVERLAY_PATH + overlayFileName;
                if (!isFile(overlayFile)) continue;
                
                // Build newOverlayName with single allocation
                newOverlayName.clear();
                if (overlayStarred) {
                    newOverlayName.reserve(STAR_SYMBOL.size() + 2 + overlayName.size() + 1 + overlayFileName.size());
                    newOverlayName = STAR_SYMBOL;
                    newOverlayName += "  ";
                    newOverlayName += overlayName;
                } else {
                    newOverlayName = overlayName;
                }
                newOverlayName += '?';
                newOverlayName += overlayFileName;
                
                const bool newStarred = !overlayStarred;
                
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(newOverlayName, "", false, false);
                
                if (!hideOverlayVersions) {
                    displayVersion = getFirstLongEntry(overlayVersion);
                    if (cleanVersionLabels) displayVersion = cleanVersionLabel(displayVersion);
                    listItem->setValue(displayVersion, true);
                    listItem->setValueColor(usingLibUltrahand ? (useLibultrahandVersions ? tsl::ultOverlayVersionTextColor : tsl::overlayVersionTextColor) : tsl::overlayVersionTextColor);
                }
                listItem->setTextColor(requiresAMS110Handling ? tsl::warningTextColor : usingLibUltrahand ? (useLibultrahandTitles ? tsl::ultOverlayTextColor : tsl::overlayTextColor) : tsl::overlayTextColor);
                
                if (overlayFileName == lastOverlayFilename) {
                    lastOverlayFilename = "";
                    jumpItemName = newOverlayName;
                    jumpItemValue = hideOverlayVersions ? "" : displayVersion;
                    jumpItemExactMatch.store(true, std::memory_order_release);
                }
                                
                listItem->setClickListener([overlayFile, newStarred, overlayFileName, overlayName, overlayVersion, requiresAMS110Handling, supportsAMS110, buildOverlayReturnName](s64 keys) {
                    if (runningInterpreter.load(std::memory_order_acquire)) return false;
                    
                    if (simulatedMenu.load(std::memory_order_acquire)) {
                        keys |= SYSTEM_SETTINGS_KEY;
                    }
                    
                    // Check for single key press (no other keys)
                    const s64 cleanKeys = keys & ALL_KEYS_MASK;
                    
                    if ((keys & KEY_A && cleanKeys == KEY_A)) {
                        if (!requiresAMS110Handling) {
                            disableSound.store(true, std::memory_order_release);
                            
                            std::string useOverlayLaunchArgs, overlayLaunchArgs;
                            {
                                auto overlaysIniData = getParsedDataFromIniFile(OVERLAYS_INI_FILEPATH);
                                auto sectionIt = overlaysIniData.find(overlayFileName);
                                if (sectionIt != overlaysIniData.end()) {
                                    auto useArgsIt = sectionIt->second.find(USE_LAUNCH_ARGS_STR);
                                    if (useArgsIt != sectionIt->second.end()) useOverlayLaunchArgs = useArgsIt->second;
                                    auto argsIt = sectionIt->second.find(LAUNCH_ARGS_STR);
                                    if (argsIt != sectionIt->second.end()) overlayLaunchArgs = argsIt->second;
                                }
                                removeQuotes(overlayLaunchArgs);
                            }
                            
                            {
                                auto iniData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
                                auto& ultrahandSection = iniData[ULTRAHAND_PROJECT_NAME];
                                if (inHiddenMode.load(std::memory_order_acquire)) ultrahandSection[IN_HIDDEN_OVERLAY_STR] = TRUE_STR;
                                ultrahandSection[IN_OVERLAY_STR] = TRUE_STR;
                                saveIniFileData(ULTRAHAND_CONFIG_INI_PATH, iniData);
                            }
                            
                            launchComboHasTriggered.store(true, std::memory_order_acquire);
                            ult::launchingOverlay.store(true, std::memory_order_release);
                            if (useOverlayLaunchArgs == TRUE_STR) tsl::setNextOverlay(overlayFile, overlayLaunchArgs);
                            else tsl::setNextOverlay(overlayFile);
                            
                            tsl::Overlay::get()->close(true);
                            return true;
                        
                        } else {
                            if (tsl::notification) {
                                tsl::notification->showNow(ult::NOTIFY_HEADER+INCOMPATIBLE_WARNING, 22);
                            }
                            
                            return true;
                        }
                    }
                    
                    if (keys & STAR_KEY && cleanKeys == STAR_KEY) {
                        if (!overlayFile.empty()) {
                            setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, STAR_STR, newStarred ? TRUE_STR : FALSE_STR);
                        }
                        
                        skipJumpReset.store(true, std::memory_order_release);
                        jumpItemName = buildOverlayReturnName(!newStarred, overlayFileName, overlayName);
                        jumpItemValue = hideOverlayVersions ? "" : overlayVersion;
                        jumpItemExactMatch.store(true, std::memory_order_release);


                        
                        wasInHiddenMode = inHiddenMode.load(std::memory_order_acquire);
                        if (wasInHiddenMode) {
                            inMainMenu.store(false, std::memory_order_release);
                            reloadMenu2 = true;
                        }
                        
                        refreshPage.store(true, std::memory_order_release);
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerMoveSound.store(true, std::memory_order_release);
                        return true;
                    }
                    
                    if (keys & SETTINGS_KEY && cleanKeys == SETTINGS_KEY) {
                        if (!inHiddenMode.load(std::memory_order_acquire)) {
                            lastMenu = "";
                            inMainMenu.store(false, std::memory_order_release);
                        } else {
                            lastMenu = "hiddenMenuMode";
                            inHiddenMode.store(false, std::memory_order_release);
                        }
                        
                        returnJumpItemName = buildOverlayReturnName(newStarred, overlayFileName, overlayName);
                        returnJumpItemValue = hideOverlayVersions ? "" : overlayVersion;
                        jumpItemName = jumpItemValue = "";
                        
                        tsl::changeTo<SettingsMenu>(overlayFileName, OVERLAY_STR, overlayName, overlayVersion, "", !supportsAMS110);
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerSettingsSound.store(true, std::memory_order_release);
                        return true;
                    }
                    
                    if (keys & SYSTEM_SETTINGS_KEY && cleanKeys == SYSTEM_SETTINGS_KEY) {
                        returnJumpItemName = buildOverlayReturnName(newStarred, overlayFileName, overlayName);
                        returnJumpItemValue = hideOverlayVersions ? "" : overlayVersion;
                        return true;
                    }
                    
                    return false;
                });

                if (requiresAMS110Handling) {
                    listItem->isLocked = true;
                }
                listItem->disableClickAnimation();
                list->addItem(listItem);
            }
            
            overlaySet.clear();
        }
        
        if (drawHiddenTab && !inHiddenMode.load(std::memory_order_acquire) && !hideHidden) {
            tsl::elm::ListItem* listItem = new tsl::elm::ListItem(HIDDEN, DROPDOWN_SYMBOL);
            listItem->setClickListener([](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire)) return false;
                if (simulatedMenu.load(std::memory_order_acquire)) {
                    keys |= SYSTEM_SETTINGS_KEY;
                }
    
                if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                    jumpItemName = "";
                    jumpItemValue = "";
                    jumpItemExactMatch.store(true, std::memory_order_release);
                    inMainMenu.store(false, std::memory_order_release);
                    inHiddenMode.store(true, std::memory_order_release);
                    tsl::changeTo<MainMenu>(OVERLAYS_STR);
                    return true;
                } else if (keys & SYSTEM_SETTINGS_KEY && !(keys & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK)) {
                    returnJumpItemName = "";
                    returnJumpItemValue = DROPDOWN_SYMBOL;
                    return true;
                }
                return false;
            });
            list->addItem(listItem);
        }
    }
    
    bool createPackagesMenu(tsl::elm::List* list) {
        if (!isFile(PACKAGE_PATH + PACKAGE_FILENAME)) {
            deleteFileOrDirectory(PACKAGE_PATH + CONFIG_FILENAME);

            FILE* packageFileOut = fopen((PACKAGE_PATH + PACKAGE_FILENAME).c_str(), "w");
            if (packageFileOut) {
                constexpr const char packageContent[] = "[*Reboot To]\n[*Boot Entry]\nini_file_source /bootloader/hekate_ipl.ini\nfilter config\nreboot boot '{ini_file_source(*)}'\n[hekate - \uE073]\nreboot HEKATE\n[hekate UMS - \uE073\uE08D]\nreboot UMS\n\n[Commands]\n[Shutdown - ]\n;hold=true\nshutdown\n";
                fwrite(packageContent, sizeof(packageContent) - 1, 1, packageFileOut);
                fclose(packageFileOut);
            }
        }
    
        inOverlaysPage.store(false, std::memory_order_release);
        inPackagesPage.store(true, std::memory_order_release);
    
        bool noClickableItems = false;
    
        if (dropdownSection.empty()) {
            createDirectory(PACKAGE_PATH);
            
            if (!isFile(PACKAGES_INI_FILEPATH)) {
                FILE* createFile = fopen(PACKAGES_INI_FILEPATH.c_str(), "w");
                if (createFile) fclose(createFile);
            }
    
            std::set<std::string> packageSet;
            bool drawHiddenTab = false;
            
            // Scope to immediately free INI data
            {
                auto packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                auto subdirectories = getSubdirectories(PACKAGE_PATH);
    
                subdirectories.erase(
                    std::remove_if(subdirectories.begin(), subdirectories.end(),
                        [](const std::string& dirName) { return dirName.front() == '.'; }),
                    subdirectories.end()
                );
    
                bool packagesNeedsUpdate = false;
    
                for (const auto& packageName : subdirectories) {
                    auto packageIt = packagesIniData.find(packageName);
                    if (packageIt == packagesIniData.end()) {
                        const PackageHeader packageHeader = getPackageHeaderFromIni(PACKAGE_PATH + packageName+ "/" +PACKAGE_FILENAME);
                        
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
                
                        const std::string assignedName = packageHeader.title.empty() ? packageName : packageHeader.title;
                        std::string entry;
                        entry.reserve(128);
                        entry = "0020:";
                        entry += assignedName;
                        entry += ":";
                        entry += packageHeader.version;
                        entry += ":";
                        entry += packageName;
                        packageSet.emplace(std::move(entry));
                    } else {
                        const std::string hide = (packageIt->second.find(HIDE_STR) != packageIt->second.end()) ? packageIt->second[HIDE_STR] : FALSE_STR;
                        if (hide == TRUE_STR) drawHiddenTab = true;
                        
                        if (inHiddenMode.load(std::memory_order_acquire) == (hide == TRUE_STR)) {
                            PackageHeader packageHeader = getPackageHeaderFromIni(PACKAGE_PATH + packageName+ "/" +PACKAGE_FILENAME);
                            if (cleanVersionLabels) {
                                packageHeader.version = cleanVersionLabel(packageHeader.version);
                                removeQuotes(packageHeader.version);
                            }
                            
                            const std::string priority = (packageIt->second.find(PRIORITY_STR) != packageIt->second.end()) ? formatPriorityString(packageIt->second[PRIORITY_STR]) : "0020";
                            const std::string starred = (packageIt->second.find(STAR_STR) != packageIt->second.end()) ? packageIt->second[STAR_STR] : FALSE_STR;
                            const std::string customName = getValueOrDefault(packageIt->second, "custom_name", "");
                            const std::string customVersion = getValueOrDefault(packageIt->second, "custom_version", "");
                            
                            const std::string assignedName = !customName.empty() ? customName : (packageHeader.title.empty() ? packageName : packageHeader.title);
                            const std::string assignedVersion = !customVersion.empty() ? customVersion : packageHeader.version;
                            
                            std::string entry;
                            entry.reserve(128);
                            if (starred == TRUE_STR) {
                                entry = "-1:";
                            }
                            entry += priority;
                            entry += ":";
                            entry += assignedName;
                            entry += ":";
                            entry += assignedVersion;
                            entry += ":";
                            entry += packageName;
                            packageSet.emplace(std::move(entry));
                        }
                    }
                }
    
                if (packagesNeedsUpdate) {
                    saveIniFileData(PACKAGES_INI_FILEPATH, packagesIniData);
                }
            } // packagesIniData freed here
            
            std::string packageName, packageVersion, newPackageName;
    
            // Helper to build return name (defined before listItem)
            auto buildReturnName = [](bool isStarred, const std::string& pkgName, const std::string& displayName) -> std::string {
                std::string name;
                if (!isStarred) {
                    name.reserve(STAR_SYMBOL.size() + 2 + displayName.size() + 1 + pkgName.size());
                    name = STAR_SYMBOL;
                    name += "  ";
                    name += displayName;
                } else {
                    name = displayName;
                }
                name += "?";
                name += pkgName;
                return name;
            };


            bool firstItem = true;
            for (const auto& taintedPackageName : packageSet) {
                if (firstItem) {
                    addHeader(list, (!inHiddenMode.load(std::memory_order_acquire) ? PACKAGES : HIDDEN_PACKAGES)+" "+DIVIDER_SYMBOL+" \uE0E3 "+SETTINGS+" "+DIVIDER_SYMBOL+" \uE0E2 "+FAVORITE);
                    firstItem = false;
                }
                
                packageName.clear();
                packageVersion.clear();
                newPackageName.clear();
                const bool packageStarred = (taintedPackageName.compare(0, 3, "-1:") == 0);
                const std::string& tempPackageName = packageStarred ? taintedPackageName : taintedPackageName;
                const size_t offset = packageStarred ? 3 : 0;
                
                // Parse from the end more efficiently
                size_t pos = tempPackageName.size();
                size_t count = 0;
                size_t positions[3];
                
                while (pos > offset && count < 3) {
                    pos = tempPackageName.rfind(':', pos - 1);
                    if (pos == std::string::npos || pos < offset) break;
                    positions[count++] = pos;
                }
                
                if (count == 3) {
                    packageName = tempPackageName.substr(positions[0] + 1);
                    packageVersion = tempPackageName.substr(positions[1] + 1, positions[0] - positions[1] - 1);
                    newPackageName = tempPackageName.substr(positions[2] + 1, positions[1] - positions[2] - 1);
                }
                
                const std::string packageFilePath = PACKAGE_PATH + packageName + "/";
                if (!isFileOrDirectory(packageFilePath)) continue;
    
                const bool newStarred = !packageStarred;
    
                std::string displayName;
                if (packageStarred) {
                    displayName.reserve(STAR_SYMBOL.size() + 2 + newPackageName.size() + 1 + packageName.size());
                    displayName = STAR_SYMBOL;
                    displayName += "  ";
                    displayName += newPackageName;
                } else {
                    displayName = newPackageName;
                }
                displayName += "?";
                displayName += packageName;
    
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(displayName, "", false, false);
                if (!hidePackageVersions) {
                    listItem->setValue(packageVersion, true);
                    listItem->setValueColor(usePackageVersions ? tsl::ultPackageVersionTextColor : tsl::packageVersionTextColor);
                }
                listItem->setTextColor(usePackageTitles ? tsl::ultPackageTextColor : tsl::packageTextColor);
                listItem->disableClickAnimation();
                
                listItem->setClickListener([packageFilePath, newStarred, packageName, newPackageName, packageVersion, packageStarred, buildReturnName](s64 keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    
                    if (simulatedMenu.load(std::memory_order_acquire)) {
                        keys |= SYSTEM_SETTINGS_KEY;
                    }
                
                    // Check for single key press (no other keys)
                    const s64 cleanKeys = keys & ALL_KEYS_MASK;
                    
                    if (keys & KEY_A && cleanKeys == KEY_A) {
                        inMainMenu.store(false, std::memory_order_release);
                        
                        // Check for boot package
                        if (isFile(packageFilePath + BOOT_PACKAGE_FILENAME)) {
                            bool useBootPackage = true;
                            const auto packagesIniData = getParsedDataFromIniFile(PACKAGES_INI_FILEPATH);
                            auto sectionIt = packagesIniData.find(packageName);
                            
                            if (sectionIt != packagesIniData.end()) {
                                auto bootIt = sectionIt->second.find(USE_BOOT_PACKAGE_STR);
                                useBootPackage = (bootIt == sectionIt->second.end()) || (bootIt->second != FALSE_STR);
                                
                                if (!selectedPackage.empty()) {
                                    auto quickIt = sectionIt->second.find(USE_QUICK_LAUNCH_STR);
                                    useBootPackage = useBootPackage && 
                                                   ((quickIt == sectionIt->second.end()) || (quickIt->second != TRUE_STR));
                                }
                            }
                
                            if (useBootPackage) {
                                auto bootCommands = loadSpecificSectionFromIni(packageFilePath + BOOT_PACKAGE_FILENAME, "boot");
                                if (!bootCommands.empty()) {
                                    const bool resetCommandSuccess = !commandSuccess.load(std::memory_order_acquire);
                                    interpretAndExecuteCommands(std::move(bootCommands), packageFilePath, "boot");
                                    resetPercentages();
                                    if (resetCommandSuccess) commandSuccess.store(false, release);
                                }
                            }
                        }

                        packageRootLayerTitle = newPackageName;
                        packageRootLayerName = packageName;
                        packageRootLayerVersion = packageVersion;
                
                        returnJumpItemName = buildReturnName(newStarred, packageName, newPackageName);
                        returnJumpItemValue = hidePackageVersions ? "" : packageVersion;
                
                        tsl::clearGlyphCacheNow.store(true, release);
                        tsl::swapTo<PackageMenu>(SwapDepth(2), packageFilePath, "");
                        return true;
                    }
                    
                    if (keys & STAR_KEY && cleanKeys == STAR_KEY) {
                        if (!packageName.empty()) {
                            setIniFileValue(PACKAGES_INI_FILEPATH, packageName, STAR_STR, newStarred ? TRUE_STR : FALSE_STR);
                        }
                        
                        skipJumpReset.store(true, release);
                        jumpItemName = buildReturnName(!newStarred, packageName, newPackageName);
                        jumpItemValue = hidePackageVersions ? "" : packageVersion;
                        jumpItemExactMatch.store(true, release);
                        
                        wasInHiddenMode = inHiddenMode.load(std::memory_order_acquire);
                        if (wasInHiddenMode) {
                            inMainMenu.store(false, std::memory_order_release);
                            reloadMenu2 = true;
                        }
                        
                        refreshPage.store(true, release);
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerMoveSound.store(true, std::memory_order_release);
                        return true;
                    }
                    
                    if (keys & SETTINGS_KEY && cleanKeys == SETTINGS_KEY) {
                        if (!inHiddenMode.load(std::memory_order_acquire)) {
                            lastMenu = "";
                            inMainMenu.store(false, std::memory_order_release);
                        } else {
                            lastMenu = "hiddenMenuMode";
                            inHiddenMode.store(false, std::memory_order_release);
                        }
                        
                        returnJumpItemName = buildReturnName(newStarred, packageName, newPackageName);
                        returnJumpItemValue = hidePackageVersions ? "" : packageVersion;
                        jumpItemName = jumpItemValue = "";
                        
                        tsl::changeTo<SettingsMenu>(packageName, PACKAGE_STR, newPackageName, packageVersion);
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerSettingsSound.store(true, std::memory_order_release);
                        return true;
                    }
                    
                    if (keys & SYSTEM_SETTINGS_KEY && cleanKeys == SYSTEM_SETTINGS_KEY) {
                        returnJumpItemName = buildReturnName(newStarred, packageName, newPackageName);
                        returnJumpItemValue = hidePackageVersions ? "" : packageVersion;
                        return true;
                    }
                    
                    return false;
                });
                
                list->addItem(listItem);
            }
    
            packageSet.clear();
            
            if (drawHiddenTab && !inHiddenMode.load(std::memory_order_acquire) && !hideHidden) {
                tsl::elm::ListItem* listItem = new tsl::elm::ListItem(HIDDEN, DROPDOWN_SYMBOL);
                listItem->setClickListener([](uint64_t keys) {
                    if (runningInterpreter.load(acquire)) return false;
                    if (simulatedMenu.load(std::memory_order_acquire)) {
                        keys |= SYSTEM_SETTINGS_KEY;
                    }
    
                    if ((keys & KEY_A && !(keys & ~KEY_A & ALL_KEYS_MASK))) {
                        inMainMenu.store(false, std::memory_order_release);
                        inHiddenMode.store(true, std::memory_order_release);
                        tsl::changeTo<MainMenu>(PACKAGES_STR);
                        return true;
                    } else if (keys & SYSTEM_SETTINGS_KEY && !(keys & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK)) {
                        returnJumpItemName = "";
                        returnJumpItemValue = DROPDOWN_SYMBOL;
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
            }
        }
        
        if (!inHiddenMode.load(std::memory_order_acquire)) {
            std::string pageLeftName, pageRightName, pathPattern, pathPatternOn, pathPatternOff;
            bool usingPages = false;
            
            const PackageHeader packageHeader = getPackageHeaderFromIni(PACKAGE_PATH);
            noClickableItems = drawCommandsMenu(list, packageIniPath, packageConfigIniPath, packageHeader, "", pageLeftName, pageRightName,
                PACKAGE_PATH, LEFT_STR, "package.ini", this->dropdownSection, 0, pathPattern, pathPatternOn, pathPatternOff, usingPages, false);
    
            if (!hideUserGuide && dropdownSection.empty()) addHelpInfo(list);
        }
        
        return noClickableItems;
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
        
        bool isHolding = (lastCommandIsHold && runningInterpreter.load(std::memory_order_acquire));
        if (isHolding) {
            processHold(keysDown, keysHeld, holdStartTick, isHolding, [&]() {
                // Execute interpreter commands if needed
                displayPercentage.store(-1, std::memory_order_release);
                //lastCommandMode.clear();
                //lastKeyName.clear();
                lastCommandIsHold = false;
                lastSelectedListItem->setValue(INPROGRESS_SYMBOL);
                //lastSelectedListItemFooter.clear();
                //lastSelectedListItem->enableClickAnimation();
                //lastSelectedListItem->triggerClickAnimation();
                //lastSelectedListItem->disableClickAnimation();
                triggerEnterFeedback();
                executeInterpreterCommands(std::move(storedCommands), packageIniPath, lastKeyName);
                lastRunningInterpreter.store(true, std::memory_order_release);
            }, nullptr, true); // true = reset storedCommands
            return true;
        }

        if (ult::launchingOverlay.load(acquire)) return true;

        const bool isRunningInterp = runningInterpreter.load(acquire);
        const bool isTouching = stillTouching.load(acquire);
        
        if (isRunningInterp)
            return handleRunningInterpreter(keysDown, keysHeld);
    
        if (lastRunningInterpreter.exchange(false, std::memory_order_acq_rel)) {
            //tsl::clearGlyphCacheNow.store(true, release);
            isDownloadCommand.store(false, release);
        
            if (lastSelectedListItem) {
                const bool success = commandSuccess.load(acquire);
        
                if (lastCommandMode == OPTION_STR || lastCommandMode == SLOT_STR) {
                    if (success) {
                        if (isFile(packageConfigIniPath)) {
                            auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                            if (auto it = packageConfigData.find(lastKeyName); it != packageConfigData.end()) {
                                auto& optionSection = it->second;
                
                                // Update footer if valid
                                if (auto footerIt = optionSection.find(FOOTER_STR);
                                    footerIt != optionSection.end() &&
                                    footerIt->second.find(NULL_STR) == std::string::npos)
                                {
                                    // Check if footer_highlight exists in the INI
                                    auto footerHighlightIt = optionSection.find("footer_highlight");
                                    if (footerHighlightIt != optionSection.end()) {
                                        // Use the value from the INI
                                        bool footerHighlightFromIni = (footerHighlightIt->second == TRUE_STR);
                                        lastSelectedListItem->setValue(footerIt->second, !footerHighlightFromIni);
                                    } else {
                                        // footer_highlight not defined in INI, use default
                                        lastSelectedListItem->setValue(footerIt->second, false);
                                    }
                                }
                            }
                
                            lastCommandMode.clear();
                        } else {
                            lastSelectedListItem->setValue(CHECKMARK_SYMBOL);
                        }
                    } else {
                        lastSelectedListItem->setValue(CROSSMARK_SYMBOL);
                    }
                } else {
                    // Handle toggle or checkmark logic
                    if (nextToggleState.empty()) {
                        lastSelectedListItem->setValue(success ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);
                    } else {
                        // Determine the final state to save
                        const std::string finalState = success 
                            ? nextToggleState 
                            : (nextToggleState == CAPITAL_ON_STR ? CAPITAL_OFF_STR : CAPITAL_ON_STR);
                        
                        // Update displayed value
                        lastSelectedListItem->setValue(finalState);
                
                        // Update toggle state
                        static_cast<tsl::elm::ToggleListItem*>(lastSelectedListItem)
                            ->setState(finalState == CAPITAL_ON_STR);
                
                        // Save to config file
                        setIniFileValue(packageConfigIniPath, lastKeyName, FOOTER_STR, finalState);
                        
                        lastKeyName.clear();
                        nextToggleState.clear();
                    }
                }
        
                lastSelectedListItem->enableClickAnimation();
                lastSelectedListItem = nullptr;
                lastFooterHighlight = lastFooterHighlightDefined = false;
            }
        
            closeInterpreterThread();
            resetPercentages();

            if (!commandSuccess.load()) {
                triggerRumbleDoubleClick.store(true, std::memory_order_release);
            }
            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
            //lastRunningInterpreter.store(false, std::memory_order_release);
            return true;
        }
        
        if (ult::refreshWallpaperNow.exchange(false, std::memory_order_acq_rel)) {
            closeInterpreterThread();
            ult::reloadWallpaper();

            if (!limitedMemory && useSoundEffects) {
                reloadSoundCacheNow.store(true, std::memory_order_release);
                //ult::Audio::initialize();
            }
        }

        if (goBackAfter.exchange(false, std::memory_order_acq_rel)) {
            disableSound.store(true, std::memory_order_release);
            simulatedBack.store(true, std::memory_order_release);
            return true;
        }
    
        if (refreshPage.load(acquire) && !isTouching) {
            refreshPage.store(false, release);
            //tsl::pop();
            tsl::swapTo<MainMenu>(hiddenMenuMode, dropdownSection);
            if (wasInHiddenMode) {
                //std::lock_guard<std::mutex> lock(jumpItemMutex);
                skipJumpReset.store(true, release);
                jumpItemName = HIDDEN;
                jumpItemValue = DROPDOWN_SYMBOL;
                jumpItemExactMatch.store(true, release);
                //g_overlayFilename = "";
                wasInHiddenMode = false;
            }
            return true;
        }
    
        // Common condition for back key handling
        const bool backKeyPressed = !isTouching && ((((keysDown & KEY_B)) && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)));
        
        if (!dropdownSection.empty() && !returningToMain) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            simulatedMenu.exchange(false, std::memory_order_acq_rel);
    
            if (backKeyPressed) {
                allowSlide.exchange(false, std::memory_order_acq_rel);
                unlockedSlide.exchange(false, std::memory_order_acq_rel);
                returningToMain = true;
                tsl::goBack();
                return true;
            }
        }
        
        if (inMainMenu.load(acquire) && !inHiddenMode.load(std::memory_order_acquire) && dropdownSection.empty()) {
            if (triggerMenuReload || triggerMenuReload2) {
                triggerMenuReload = triggerMenuReload2 = false;
                disableSound.store(true, std::memory_order_release);
                ult::launchingOverlay.store(true, std::memory_order_release);
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

                tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl", "--skipCombo --comboReturn");
                tsl::Overlay::get()->close();
            }
            
            if (!freshSpawn && !returningToMain && !returningToHiddenMain) {
                //if (simulatedNextPage.exchange(false, acq_rel)) {
                //    const bool toPackages = (!usePageSwap && menuMode != PACKAGES_STR) || (usePageSwap && menuMode != OVERLAYS_STR);
                //    keysDown |= toPackages ? (usePageSwap ? KEY_DLEFT : KEY_DRIGHT) : (usePageSwap ? KEY_DRIGHT : KEY_DLEFT);
                //}
                const bool onLeftPage = (!usePageSwap && menuMode != PACKAGES_STR) || (usePageSwap && menuMode != OVERLAYS_STR);
                {
                    //std::lock_guard<std::mutex> lock(ult::simulatedNextPageMutex);
                    if (simulatedNextPage.exchange(false, std::memory_order_acq_rel)) {
                        if (onLeftPage) {
                            keysDown |= KEY_DRIGHT;
                        }
                        else {
                            keysDown |= KEY_DLEFT;
                        }
                    }
                }
                
                // Cache slide conditions
                
                const bool onTrack = onTrackBar.load(acquire);
                const bool slideAllowed = allowSlide.load(acquire);
                const bool slideUnlocked = unlockedSlide.load(acquire);
                const bool slideCondition = ((!slideAllowed && !slideUnlocked && onTrack) || (onTrack && keysHeld & KEY_R)) || !onTrack;
                
                // Helper lambda to reset navigation state
                auto resetNavState = [&]() {
                    {
                        //std::lock_guard<std::mutex> lock(jumpItemMutex);
                        //g_overlayFilename = "";
                        jumpItemName = "";
                        jumpItemValue = "";
                        jumpItemExactMatch.store(true, release);
                    }
                    //if (allowSlide.load(acquire))
                    allowSlide.store(false, release);
                    //if (unlockedSlide.load(acquire))
                    unlockedSlide.store(false, release);
                };
                
                //const bool switchToPackages = (!usePageSwap && menuMode != PACKAGES_STR) || (usePageSwap && menuMode != OVERLAYS_STR);
                if (onLeftPage && !isTouching && slideCondition && (keysDown & KEY_RIGHT) && (!onTrack ? !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK) : !(keysHeld & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK))) {
                    //bool safeToSwap = false;
                    {
                        std::lock_guard<std::mutex> lock(tsl::elm::s_safeToSwapMutex);
                        //safeToSwap = tsl::elm::s_safeToSwap.load(acquire);
                        
                        //if (simulatedNextPage.load(acquire))
                        //    simulatedNextPage.store(false, release);
                        if (tsl::elm::s_safeToSwap.load(acquire)) {
                            //tsl::elm::s_safeToSwap.store(false, release);
                            currentMenu = usePageSwap ? OVERLAYS_STR : PACKAGES_STR;
                            //tsl::pop();
                            tsl::swapTo<MainMenu>();
                            resetNavState();
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                        }
                        return true;
                    }
                    
                    
                }
                
                //const bool switchToOverlays = (!usePageSwap && menuMode != OVERLAYS_STR) || (usePageSwap && menuMode != PACKAGES_STR);
                if (!onLeftPage && !isTouching && slideCondition && (keysDown & KEY_LEFT) && (!onTrack ? !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK): !(keysHeld & ~KEY_LEFT & ~KEY_R  & ALL_KEYS_MASK))) {
                    //bool safeToSwap = false;
                    {
                        std::lock_guard<std::mutex> lock(tsl::elm::s_safeToSwapMutex);
                        //safeToSwap = tsl::elm::s_safeToSwap.load(acquire);
                        
                        //if (simulatedNextPage.load(acquire))
                        //    simulatedNextPage.store(false, release);
                        if (tsl::elm::s_safeToSwap.load(acquire)) {
                            //tsl::elm::s_safeToSwap.store(false, release);
                            currentMenu = usePageSwap ? PACKAGES_STR : OVERLAYS_STR;
                            //tsl::pop();
                            tsl::swapTo<MainMenu>();
                            resetNavState();
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                        }
                        return true;
                    }
                }
    
                if (backKeyPressed) {
                    allowSlide.exchange(false, std::memory_order_acq_rel);
                    unlockedSlide.exchange(false, std::memory_order_acq_rel);
                    if (tsl::notification && tsl::notification->isActive()) {
                        tsl::Overlay::get()->closeAfter();
                        tsl::Overlay::get()->hide(true);
                    } else {
                        ult::launchingOverlay.store(true, std::memory_order_release);
                        exitingUltrahand.store(true, release);

                        tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                        tsl::Overlay::get()->close();
                    }

                    
                    return true;
                }
    
                if (simulatedMenu.load(std::memory_order_acquire)) {
                    keysDown |= SYSTEM_SETTINGS_KEY;
                }
    
                if (!isTouching && (((keysDown & SYSTEM_SETTINGS_KEY && !(keysHeld & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK))))) {
                    inMainMenu.store(false, std::memory_order_release);
                    skipJumpReset.store(false, std::memory_order_release);
                    tsl::changeTo<UltrahandSettingsMenu>();
                    triggerRumbleClick.store(true, std::memory_order_release);
                    triggerSettingsSound.store(true, std::memory_order_release);
                    return true;
                }
            }
        }
        
        if (!inMainMenu.load(acquire) && inHiddenMode.load(std::memory_order_acquire) && !returningToHiddenMain && !returningToMain) {
            simulatedNextPage.exchange(false, std::memory_order_acq_rel);
            //simulatedMenu.exchange(false, std::memory_order_acq_rel);
    
            if (backKeyPressed) {
                // Check if we're in hidden mode with no underlying menu to go back to
                if (hiddenMenuMode == OVERLAYS_STR || hiddenMenuMode == PACKAGES_STR) {
                    inMainMenu.store(true, std::memory_order_release);
                    inHiddenMode.store(false, std::memory_order_release);
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

                    
                    {
                        //std::lock_guard<std::mutex> lock(jumpItemMutex);
                        skipJumpReset.store(true, release);
                        jumpItemName = HIDDEN;
                        jumpItemValue = DROPDOWN_SYMBOL;
                        jumpItemExactMatch.store(true, release);
                        //g_overlayFilename = "";
                    }
                    returningToMain = true;
                    //tsl::pop();
                    tsl::swapTo<MainMenu>();
                    return true;
                }
    
                returningToMain = true;
                inHiddenMode.exchange(false, std::memory_order_acq_rel);
                
                if (reloadMenu2) {
                    //tsl::pop();
                    tsl::swapTo<MainMenu>();
                    reloadMenu2 = false;
                    return true;
                }
                
                allowSlide.exchange(false, std::memory_order_acq_rel);
                unlockedSlide.exchange(false, std::memory_order_acq_rel);
                tsl::goBack();
                return true;
            }

            if (simulatedMenu.load(std::memory_order_acquire)) {
                keysDown |= SYSTEM_SETTINGS_KEY;
            }
            
            if (!isTouching && (((keysDown & SYSTEM_SETTINGS_KEY && !(keysHeld & ~SYSTEM_SETTINGS_KEY & ALL_KEYS_MASK))))) {
                inMainMenu.store(false, std::memory_order_release);
                skipJumpReset.store(false, std::memory_order_release);
                lastMenu = "hiddenMenuMode";  // ADD THIS LINE
                tsl::changeTo<UltrahandSettingsMenu>();
                triggerRumbleClick.store(true, std::memory_order_release);
                triggerSettingsSound.store(true, std::memory_order_release);
                return true;
            }
        }
        
        if (freshSpawn && !(keysDown & KEY_B))
            freshSpawn = false;
        
        if (returningToMain && !(keysDown & KEY_B)) {
            returningToMain = false;
            inMainMenu.store(true, std::memory_order_release);
        }
        if (returningToHiddenMain && !(keysDown & KEY_B)) {
            returningToHiddenMain = false;
            inHiddenMode.store(true, std::memory_order_release);
        }
    
        if (triggerExit.exchange(false, std::memory_order_acq_rel)) {
            //triggerExit.store(false, release);
            ult::launchingOverlay.store(true, std::memory_order_release);
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }
        
        //svcSleepThread(10'000'000);
        return false;
    }
};


// Extract the settings initialization logic into a separate method
void initializeSettingsAndDirectories() {
    versionLabel = cleanVersionLabel(APP_VERSION) + " " + DIVIDER_SYMBOL + " " + loaderTitle + " " + cleanVersionLabel(loaderInfo);
    std::string defaultLang = "en";

    // Create necessary directories
    createDirectory(PACKAGE_PATH);
    createDirectory(LANG_PATH);
    createDirectory(FLAGS_PATH);
    createDirectory(NOTIFICATIONS_PATH);
    createDirectory(THEMES_PATH);
    createDirectory(WALLPAPERS_PATH);
    createDirectory(SOUNDS_PATH);
    
    bool settingsLoaded = false;
    bool needsUpdate = false;
    
    std::map<std::string, std::map<std::string, std::string>> iniData;

    // Check if file didn't exist
    if (isFile(ULTRAHAND_CONFIG_INI_PATH)) {
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
    setDefaultValue("hide_delete", FALSE_STR, hideDelete);
    if (requiresLNY2) {
        //setDefaultValue("hide_force_support", TRUE_STR, hideForceSupport);
        setDefaultValue("hide_unsupported", FALSE_STR, hideUnsupported);
    }
    setDefaultValue("clean_version_labels", FALSE_STR, cleanVersionLabels);
    setDefaultValue("hide_overlay_versions", FALSE_STR, hideOverlayVersions);
    setDefaultValue("hide_package_versions", FALSE_STR, hidePackageVersions);

    setDefaultValue("dynamic_logo", TRUE_STR, useDynamicLogo);
    setDefaultValue("selection_bg", TRUE_STR, useSelectionBG);
    setDefaultValue("selection_text", FALSE_STR, useSelectionText);
    setDefaultValue("selection_value", FALSE_STR, useSelectionValue);
    setDefaultValue("libultrahand_titles", FALSE_STR, useLibultrahandTitles);
    setDefaultValue("libultrahand_versions", TRUE_STR, useLibultrahandVersions);
    setDefaultValue("package_titles", FALSE_STR, usePackageTitles);
    setDefaultValue("package_versions", TRUE_STR, usePackageVersions);
    //setDefaultValue("memory_expansion", FALSE_STR, useMemoryExpansion);
    setDefaultValue("launch_combos", TRUE_STR, useLaunchCombos);
    setDefaultValue("notifications", TRUE_STR, useNotifications);
    setDefaultValue("sound_effects", TRUE_STR, useSoundEffects);
    setDefaultValue("haptic_feedback", FALSE_STR, useHapticFeedback);
    setDefaultValue("page_swap", FALSE_STR, usePageSwap);
    setDefaultValue("swipe_to_open", TRUE_STR, useSwipeToOpen);
    setDefaultValue("right_alignment", FALSE_STR, useRightAlignment);
    setDefaultValue("opaque_screenshots", TRUE_STR, useOpaqueScreenshots);
    
    setDefaultStrValue(DEFAULT_LANG_STR, defaultLang, defaultLang);

    // Widget settings - now properly loaded into variables
    setDefaultValue("hide_clock", FALSE_STR, hideClock);
    setDefaultValue("hide_battery", TRUE_STR, hideBattery);
    setDefaultValue("hide_pcb_temp", TRUE_STR, hidePCBTemp);
    setDefaultValue("hide_soc_temp", TRUE_STR, hideSOCTemp);
    setDefaultValue("dynamic_widget_colors", TRUE_STR, dynamicWidgetColors);
    setDefaultValue("hide_widget_backdrop", FALSE_STR, hideWidgetBackdrop);
    setDefaultValue("center_widget_alignment", TRUE_STR, centerWidgetAlignment);
    setDefaultValue("extended_widget_backdrop", FALSE_STR, extendedWidgetBackdrop);
    
    // Datetime format string setting
    setDefaultStrValue("datetime_format", DEFAULT_DT_FORMAT, datetimeFormat);
    
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

    if (useNotifications) {
        if (!isFile(NOTIFICATIONS_FLAG_FILEPATH)) {
            FILE* file = std::fopen((NOTIFICATIONS_FLAG_FILEPATH).c_str(), "w");
            if (file) {
                std::fclose(file);
            }
        }
    } else {
        deleteFileOrDirectory(NOTIFICATIONS_FLAG_FILEPATH);
    }
    
    // Load language file
    const std::string langFile = LANG_PATH + defaultLang + ".json";
    if (isFile(langFile))
        parseLanguage(langFile);
    else {
        if (defaultLang == "en")
            reinitializeLangVars();
    }
    
    // Initialize theme
    initializeTheme();
    tsl::initializeThemeVars();
    updateMenuCombos = copyTeslaKeyComboToUltrahand();
    
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
        //tsl::gfx::FontManager::preloadPersistentGlyphs("0123456789%●", 20);
        //tsl::gfx::FontManager::preloadPersistentGlyphs(""+ult::HIDE+""+ult::CANCEL, 23);
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
                if (isFile(packageFilePath + BOOT_PACKAGE_FILENAME)) {
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
                            //bool resetCommandSuccess = false;
                            //if (!commandSuccess.load(acquire)) resetCommandSuccess = true;
                            const bool resetCommandSuccess = !commandSuccess.load(std::memory_order_acquire);
                            
                            interpretAndExecuteCommands(std::move(bootCommands), packageFilePath, "boot");
                            resetPercentages();
    
                            if (resetCommandSuccess) {
                                commandSuccess.store(false, release);
                            }
                        }
                    }
                }
    
                // Set the necessary global variables with PROPER NAMES
                //lastPackagePath = packageFilePath;
                //lastPackageName = PACKAGE_FILENAME;
                packageRootLayerTitle = assignedOverlayName;  // Use proper title
                packageRootLayerVersion = assignedOverlayVersion;  // Use proper version
                
                inMainMenu.store(false, std::memory_order_release);
                
                // Return PackageMenu directly instead of MainMenu
                return initially<PackageMenu>(packageFilePath, "");
            } else {
                // Package not found, clear the selection and fall back to main menu
                selectedPackage.clear();
            }
        }
        if (firstBoot && tsl::notification) {
            tsl::notification->show("  "+ULTRAHAND_HAS_STARTED);
        }
        //settingsInitialized.exchange(true, acq_rel);
        // Default behavior - load main menu
        return initially<MainMenu>();
    }
    
    
    /**
     * @brief Initializes essential services and resources.
     *
     * This function initializes essential services and resources required for the overlay to function properly.
     * It sets up file system mounts, initializes network services, and performs other necessary tasks.
     */
    virtual void initServices() override {
        tsl::overrideBackButton = true; // for properly overriding the always go back functionality of KEY_B

        constexpr SocketInitConfig socketInitConfig = {
            // TCP buffers
            .tcp_tx_buf_size     = 16 * 1024,   // 16 KB default
            .tcp_rx_buf_size     = 16 * 1024*2,   // 16 KB default
            .tcp_tx_buf_max_size = 64 * 1024,   // 64 KB default max
            .tcp_rx_buf_max_size = 64 * 1024*2,   // 64 KB default max
            
            // UDP buffers
            .udp_tx_buf_size     = 512,         // 512 B default
            .udp_rx_buf_size     = 512,         // 512 B default
        
            // Socket buffer efficiency
            .sb_efficiency       = 1,           // 0 = default, balanced memory vs CPU
                                                // 1 = prioritize memory efficiency (smaller internal allocations)
            .bsd_service_type    = BsdServiceType_Auto // Auto-select service
        };
        socketInitialize(&socketInitConfig);

        // read commands from root package's boot_package.ini
        if (firstBoot) {
            // Delete all pending notification jsons
            {
                std::lock_guard<std::mutex> jsonLock(tsl::notificationJsonMutex);
                deleteFileOrDirectoryByPattern(ult::NOTIFICATIONS_PATH + "*.notify");
            }

            // Load and execute "initial_boot" commands if they exist
            executeIniCommands(PACKAGE_PATH + BOOT_PACKAGE_FILENAME, "boot");
            
            const bool disableFuseReload = (parseValueFromIniSection(FUSE_DATA_INI_PATH, FUSE_STR, "disable_reload") == TRUE_STR);
            if (!disableFuseReload)
                deleteFileOrDirectory(FUSE_DATA_INI_PATH);

            // initialize expanded memory on boot
            //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "memory_expansion", (loaderTitle == "nx-ovlloader+") ? TRUE_STR : FALSE_STR);

            //if (tsl::notification)
            //    tsl::notification->show("  "+ULTRAHAND_HAS_STARTED);
            
        }

        unpackDeviceInfo();
        
        
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
        closeInterpreterThread(); // just in case ¯\_(ツ)_/¯

        if (exitingUltrahand.load(acquire))
            executeIniCommands(PACKAGE_PATH + EXIT_PACKAGE_FILENAME, "exit");

        //cleanupCurl();
        curl_global_cleanup();

        //if (!ult::limitedMemory)
        socketExit();
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
        if (argv[arg][0] != '-') continue;
        
        if (strcmp(argv[arg], "--package") == 0 && arg + 1 < argc) {
            selectedPackage.clear();
            selectedPackage.reserve(64); // Reserve reasonable amount
            
            for (u8 nextArg = arg + 1; nextArg < argc && argv[nextArg][0] != '-'; nextArg++) {
                if (!selectedPackage.empty()) selectedPackage += ' ';
                selectedPackage += argv[nextArg];
                arg = nextArg;
            }
            break;
        }
    }
    return tsl::loop<Overlay, tsl::impl::LaunchFlags::None>(argc, argv);
}
