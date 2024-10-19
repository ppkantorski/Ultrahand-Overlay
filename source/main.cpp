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


// Overlay booleans
static bool returningToMain = false;
static bool returningToHiddenMain = false;
static bool returningToSettings = false;
static bool returningToPackage = false;
static bool returningToSubPackage = false;
static bool returningToSelectionMenu = false;
//static bool inMainMenu = false; // moved to libtesla
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
static bool reloadMenu3 = false;
static bool triggerMenuReload = false;

static bool redrawWidget = false;

static size_t nestedMenuCount = 0;

// Command mode globals
static const std::vector<std::string> commandSystems = {DEFAULT_STR, ERISTA_STR, MARIKO_STR};
static const std::vector<std::string> commandModes = {DEFAULT_STR, SLOT_STR, TOGGLE_STR, OPTION_STR, FORWARDER_STR, TEXT_STR, TABLE_STR, TRACKBAR_STR, STEP_TRACKBAR_STR, NAMED_STEP_TRACKBAR_STR};
static const std::vector<std::string> commandGroupings = {DEFAULT_STR, "split", "split2", "split3", "split4"};
static const std::string MODE_PATTERN = ";mode=";
static const std::string GROUPING_PATTERN = ";grouping=";
static const std::string SYSTEM_PATTERN = ";system=";

// Table option patterns
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

static std::unordered_map<std::string, std::string> selectedFooterDict;

static std::shared_ptr<tsl::elm::ListItem> selectedListItem;
static std::shared_ptr<tsl::elm::ListItem> lastSelectedListItem;

static bool lastRunningInterpreter = false;





//struct CommandOptions {
//    bool& inEristaSection;
//    bool& inMarikoSection;
//    bool& usingErista;
//    bool& usingMariko;
//    std::string& commandName;
//    std::string& commandSystem;
//    std::string& commandMode;
//    std::string& commandGrouping;
//    std::string& currentSection;
//    std::string& pathPattern;
//    std::string& sourceType;
//    std::string& pathPatternOn;
//    std::string& sourceTypeOn;
//    std::string& pathPatternOff;
//    std::string& sourceTypeOff;
//    std::string& defaultToggleState;
//    bool& hideTableBackground;
//    size_t& tableStartGap;
//    size_t& tableEndGap;
//    size_t& tableColumnOffset;
//    size_t& tableSpacing;
//    std::string& tableSectionTextColor;
//    std::string& tableInfoTextColor;
//    std::string& tableAlignment;
//    s16& minValue;
//    s16& maxValue;
//    std::string& units;
//    size_t& steps;
//    bool& unlockedTrackbar;
//    bool& onEveryTick;
//
//    std::string& packageSource;
//    std::string& packagePath;
//    std::string& filePath;
//    std::string& iniFilePath;
//    std::vector<std::string>& filesList;
//    std::vector<std::string>& filesListOn;
//    std::vector<std::string>& filesListOff;
//    std::vector<std::string>& filterList;
//    std::vector<std::string>& filterListOn;
//    std::vector<std::string>& filterListOff;
//    std::string& jsonString;
//    std::string& jsonStringOn;
//    std::string& jsonStringOff;
//    std::string& jsonKey;
//    std::string& jsonKeyOn;
//    std::string& jsonKeyOff;
//    std::string& jsonPath;
//    std::string& jsonPathOn;
//    std::string& jsonPathOff;
//    std::string& listPath;
//    std::string& listPathOn;
//    std::string& listPathOff;
//    std::string& listString;
//    std::string& listStringOn;
//    std::string& listStringOff;
//    std::string& iniPath;
//    std::string& iniPathOn;
//    std::string& iniPathOff;
//};
//
//struct CommandData {
//    std::vector<std::vector<std::string>>& commands;
//    std::vector<std::vector<std::string>>& commandsOn;
//    std::vector<std::vector<std::string>>& commandsOff;
//    std::vector<std::vector<std::string>>& tableData;
//};
//
//
//
//void processCommands(CommandOptions& options, CommandData& data) {
//    options.inEristaSection = false;
//    options.inMarikoSection = false;
//    size_t delimiterPos;
//
//    // Remove all empty command strings
//    data.commands.erase(std::remove_if(data.commands.begin(), data.commands.end(),
//        [](const std::vector<std::string>& vec) {
//            return vec.empty();
//        }),
//        data.commands.end());
//
//    // Initial processing of commands
//    std::string iniFilePath;
//    std::string commandNameLower;
//
//    for (auto& cmd : data.commands) {
//        options.commandName = cmd[0];
//
//        commandNameLower = stringToLowercase(options.commandName);
//        if (commandNameLower == "erista:") {
//            options.inEristaSection = true;
//            options.inMarikoSection = false;
//            continue;
//        } else if (commandNameLower == "mariko:") {
//            options.inEristaSection = false;
//            options.inMarikoSection = true;
//            continue;
//        }
//
//        if ((options.inEristaSection && !options.inMarikoSection && options.usingErista) || 
//            (!options.inEristaSection && options.inMarikoSection && options.usingMariko) || 
//            (!options.inEristaSection && !options.inMarikoSection)) {
//
//            if (options.commandName.find(SYSTEM_PATTERN) == 0) {
//                options.commandSystem = options.commandName.substr(SYSTEM_PATTERN.length());
//                if (std::find(commandSystems.begin(), commandSystems.end(), options.commandSystem) == commandSystems.end())
//                    options.commandSystem = commandSystems[0];
//                continue;
//            } else if (options.commandName.find(MODE_PATTERN) == 0) {
//                options.commandMode = options.commandName.substr(MODE_PATTERN.length());
//                if (options.commandMode.find(TOGGLE_STR) != std::string::npos) {
//                    delimiterPos = options.commandMode.find('?');
//                    if (delimiterPos != std::string::npos) {
//                        options.defaultToggleState = options.commandMode.substr(delimiterPos + 1);
//                    }
//                    options.commandMode = TOGGLE_STR;
//                } else if (std::find(commandModes.begin(), commandModes.end(), options.commandMode) == commandModes.end()) {
//                    options.commandMode = commandModes[0];
//                }
//                continue;
//            } else if (options.commandName.find(GROUPING_PATTERN) == 0) {
//                options.commandGrouping = options.commandName.substr(GROUPING_PATTERN.length());
//                if (std::find(commandGroupings.begin(), commandGroupings.end(), options.commandGrouping) == commandGroupings.end())
//                    options.commandGrouping = commandGroupings[0];
//                continue;
//            } else if (options.commandName.find(BACKGROUND_PATTERN) == 0) {
//                options.hideTableBackground = (options.commandName.substr(BACKGROUND_PATTERN.length()) == FALSE_STR);
//                continue;
//            } else if (options.commandName.find(GAP_PATTERN) == 0) {
//                options.tableEndGap = std::stoi(options.commandName.substr(GAP_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(OFFSET_PATTERN) == 0) {
//                options.tableColumnOffset = std::stoi(options.commandName.substr(OFFSET_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(SPACING_PATTERN) == 0) {
//                options.tableSpacing = std::stoi(options.commandName.substr(SPACING_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(SECTION_TEXT_COLOR_PATTERN) == 0) {
//                options.tableSectionTextColor = options.commandName.substr(SECTION_TEXT_COLOR_PATTERN.length());
//                continue;
//            } else if (options.commandName.find(INFO_TEXT_COLOR_PATTERN) == 0) {
//                options.tableInfoTextColor = options.commandName.substr(INFO_TEXT_COLOR_PATTERN.length());
//                continue;
//            } else if (options.commandName.find(ALIGNMENT_PATTERN) == 0) {
//                options.tableAlignment = options.commandName.substr(ALIGNMENT_PATTERN.length());
//                continue;
//
//            } else if (options.commandName.find(MIN_VALUE_PATTERN) == 0) {
//                options.minValue = std::stoi(options.commandName.substr(MIN_VALUE_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(MAX_VALUE_PATTERN) == 0) {
//                options.maxValue = std::stoi(options.commandName.substr(MAX_VALUE_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(UNITS_PATTERN) == 0) {
//                options.units = removeQuotes(options.commandName.substr(UNITS_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(STEPS_PATTERN) == 0) {
//                options.steps = std::stoi(options.commandName.substr(STEPS_PATTERN.length()));
//                continue;
//            } else if (options.commandName.find(UNLOCKED_PATTERN) == 0) {
//                options.unlockedTrackbar = (options.commandName.substr(UNLOCKED_PATTERN.length()) == TRUE_STR);
//                continue;
//            } else if (options.commandName.find(ON_EVERY_TICK_PATTERN) == 0) {
//                options.onEveryTick = (options.commandName.substr(ON_EVERY_TICK_PATTERN.length()) == TRUE_STR);
//                continue;
//            } else if (options.commandName.find(";") == 0) {
//                continue;
//            }
//
//            if (options.commandMode == TOGGLE_STR) {
//                if (options.commandName.find("on:") == 0)
//                    options.currentSection = ON_STR;
//                else if (options.commandName.find("off:") == 0)
//                    options.currentSection = OFF_STR;
//
//                if (options.currentSection == GLOBAL_STR) {
//                    data.commandsOn.push_back(cmd);
//                    data.commandsOff.push_back(cmd);
//                } else if (options.currentSection == ON_STR) {
//                    data.commandsOn.push_back(cmd);
//                } else if (options.currentSection == OFF_STR) {
//                    data.commandsOff.push_back(cmd);
//                }
//            } else if (options.commandMode == TABLE_STR) {
//                data.tableData.push_back(cmd);
//                continue;
//            } else if (options.commandMode == TRACKBAR_STR || options.commandMode == STEP_TRACKBAR_STR || options.commandMode == NAMED_STEP_TRACKBAR_STR) {
//                //data.commands.push_back(cmd);
//                continue;
//            }
//
//            if (cmd.size() > 1) {
//                if (!iniFilePath.empty())
//                    cmd[1] = replaceIniPlaceholder(cmd[1], INI_FILE_STR, options.iniFilePath);
//
//                if (options.commandName == "ini_file") {
//                    iniFilePath = preprocessPath(cmd[1], options.filePath);
//                    continue;
//                } else if (options.commandName == "filter") {
//                    std::string filterEntry = removeQuotes(cmd[1]);
//                    if (options.sourceType == FILE_STR)
//                        filterEntry = preprocessPath(filterEntry, options.filePath);
//
//                    if (options.currentSection == GLOBAL_STR)
//                        options.filterList.push_back(std::move(filterEntry));
//                    else if (options.currentSection == ON_STR)
//                        options.filterListOn.push_back(std::move(filterEntry));
//                    else if (options.currentSection == OFF_STR)
//                        options.filterListOff.push_back(std::move(filterEntry));
//                } else if (options.commandName == "file_source") {
//                    options.sourceType = FILE_STR;
//                    if (options.currentSection == GLOBAL_STR) {
//                        //logMessage("cmd[1]: "+cmd[1]);
//                        options.pathPattern = preprocessPath(cmd[1], options.filePath);
//                        //logMessage("pathPattern: "+pathPattern);
//                        std::vector<std::string> newFiles = getFilesListByWildcards(options.pathPattern);
//                        options.filesList.insert(options.filesList.end(), newFiles.begin(), newFiles.end()); // Append new files
//                    } else if (options.currentSection == ON_STR) {
//                        options.pathPatternOn = preprocessPath(cmd[1], options.filePath);
//                        std::vector<std::string> newFilesOn = getFilesListByWildcards(options.pathPatternOn);
//                        options.filesListOn.insert(options.filesListOn.end(), newFilesOn.begin(), newFilesOn.end()); // Append new files
//                        options.sourceTypeOn = FILE_STR;
//                    } else if (options.currentSection == OFF_STR) {
//                        options.pathPatternOff = preprocessPath(cmd[1], options.filePath);
//                        std::vector<std::string> newFilesOff = getFilesListByWildcards(options.pathPatternOff);
//                        options.filesListOff.insert(options.filesListOff.end(), newFilesOff.begin(), newFilesOff.end()); // Append new files
//                        options.sourceTypeOff = FILE_STR;
//                    }
//                } else if (options.commandName == "json_file_source") {
//                    options.sourceType = JSON_FILE_STR;
//                    if (options.currentSection == GLOBAL_STR) {
//                        options.jsonPath = preprocessPath(cmd[1], options.filePath);
//                        if (cmd.size() > 2)
//                            options.jsonKey = cmd[2];
//                    } else if (options.currentSection == ON_STR) {
//                        options.jsonPathOn = preprocessPath(cmd[1], options.filePath);
//                        options.sourceTypeOn = JSON_FILE_STR;
//                        if (cmd.size() > 2)
//                            options.jsonKeyOn = cmd[2];
//                    } else if (options.currentSection == OFF_STR) {
//                        options.jsonPathOff = preprocessPath(cmd[1], options.filePath);
//                        options.sourceTypeOff = JSON_FILE_STR;
//                        if (cmd.size() > 2)
//                            options.jsonKeyOff = cmd[2];
//                    }
//                } else if (options.commandName == "list_file_source") {
//                    options.sourceType = LIST_FILE_STR;
//                    if (options.currentSection == GLOBAL_STR) {
//                        options.listPath = preprocessPath(cmd[1], options.filePath);
//                    } else if (options.currentSection == ON_STR) {
//                        options.listPathOn = preprocessPath(cmd[1], options.filePath);
//                        options.sourceTypeOn = LIST_FILE_STR;
//                    } else if (options.currentSection == OFF_STR) {
//                        options.listPathOff = preprocessPath(cmd[1], options.filePath);
//                        options.sourceTypeOff = LIST_FILE_STR;
//                    }
//                } else if (options.commandName == "list_source") {
//                    options.sourceType = LIST_STR;
//                    if (options.currentSection == GLOBAL_STR) {
//                        options.listString = removeQuotes(cmd[1]);
//                    } else if (options.currentSection == ON_STR) {
//                        options.listStringOn = removeQuotes(cmd[1]);
//                        options.sourceTypeOn = LIST_STR;
//                    } else if (options.currentSection == OFF_STR) {
//                        options.listStringOff = removeQuotes(cmd[1]);
//                        options.sourceTypeOff = LIST_STR;
//                    }
//                } else if (options.commandName == "ini_file_source") {
//                    options.sourceType = INI_FILE_STR;
//                    if (options.currentSection == GLOBAL_STR) {
//                        options.iniPath = preprocessPath(cmd[1], options.filePath);
//                    } else if (options.currentSection == ON_STR) {
//                        options.iniPathOn = preprocessPath(cmd[1], options.filePath);
//                        options.sourceTypeOn = INI_FILE_STR;
//                    } else if (options.currentSection == OFF_STR) {
//                        options.iniPathOff = preprocessPath(cmd[1], options.filePath);
//                        options.sourceTypeOff = INI_FILE_STR;
//                    }
//                } else if (options.commandName == "json_source") {
//                    options.sourceType = JSON_STR;
//                    if (options.currentSection == GLOBAL_STR) {
//                        options.jsonString = removeQuotes(cmd[1]);
//                        if (cmd.size() > 2)
//                            options.jsonKey = removeQuotes(cmd[2]);
//                    } else if (options.currentSection == ON_STR) {
//                        options.jsonStringOn = removeQuotes(cmd[1]);
//                        options.sourceTypeOn = JSON_STR;
//                        if (cmd.size() > 2)
//                            options.jsonKeyOn = removeQuotes(cmd[2]);
//                    } else if (options.currentSection == OFF_STR) {
//                        options.jsonStringOff = removeQuotes(cmd[1]);
//                        options.sourceTypeOff = JSON_STR;
//                        if (cmd.size() > 2)
//                            options.jsonKeyOff = removeQuotes(cmd[2]);
//                    }
//                }
//            }
//
//            if (cmd.size() > 1) {
//                if (options.commandName == "file_source") {
//                    if (options.currentSection == GLOBAL_STR) {
//                        options.pathPattern = preprocessPath(cmd[1], options.packagePath);
//                        options.sourceType = FILE_STR;
//                    } else if (options.currentSection == ON_STR) {
//                        options.pathPatternOn = preprocessPath(cmd[1], options.packagePath);
//                        options.sourceTypeOn = FILE_STR;
//                    } else if (options.currentSection == OFF_STR) {
//                        options.pathPatternOff = preprocessPath(cmd[1], options.packagePath);
//                        options.sourceTypeOff = FILE_STR;
//                    }
//                } else if (options.commandName == "package_source") {
//                    options.packageSource = preprocessPath(cmd[1], options.packagePath);
//                }
//            }
//        }
//    }
//}






template<typename Map, typename Func = std::function<std::string(const std::string&)>, typename... Args>
std::string getValueOrDefault(const Map& data, const std::string& key, const std::string& defaultValue, Func formatFunc = nullptr, Args... args) {
    auto it = data.find(key);
    if (it != data.end()) {
        return formatFunc ? formatFunc(it->second, args...) : it->second;
    }
    return defaultValue;
}


inline void clearMemory() {
    directoryCache.clear();
    hexSumCache.clear();
    selectedFooterDict.clear(); // Clears all data from the map, making it empty again
    selectedListItem.reset();
    lastSelectedListItem.reset();
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
 * @param keysHeld A bitset representing keys that are held down.
 * @param stillTouching Boolean indicating if the touchscreen is being interacted with.
 * @param lastSelectedListItem Reference to the UI element displaying the current status.
 * @param commandSuccess Reference to a boolean tracking the overall command success.
 * @return `true` if the operation needs to abort, `false` otherwise.
 */
bool handleRunningInterpreter(uint64_t& keysHeld) {
    static std::string lastSymbol;
    static int lastPercentage = -1;
    static bool inProgress = true;
    //static auto last_call = std::chrono::steady_clock::now();
    //auto now = std::chrono::steady_clock::now();
    bool shouldAbort = false;

    //if (now - last_call < std::chrono::milliseconds(20)) {
    //    return false;  // Exit if the minimum interval hasn't passed
    //}
    //last_call = now;  // Update last_call to the current time

    // Helper lambda to update the UI and manage completion state
    static auto updateUI = [&](std::atomic<int>& percentage, const std::string& symbol) {
        int currentPercentage = percentage.load(std::memory_order_acquire);
        if (currentPercentage != -1) {
            if (currentPercentage != lastPercentage) {
                lastSelectedListItem->setValue(symbol + " " + std::to_string(currentPercentage) + "%");
                lastPercentage = currentPercentage;
                lastSymbol = symbol;
            }
            if (currentPercentage == 100) {
                //inProgress = false;
                percentage.store(-1, std::memory_order_release);
            }
            
            return true;
        }// else if (lastPercentage > 0)
         //   lastSelectedListItem->setValue(lastSymbol + " 100%");
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

    if ((keysHeld & KEY_R) && !(keysHeld & ~KEY_R & ALL_KEYS_MASK) && !stillTouching) {
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
    std::string lastSelectedListItemFooter = "";
    bool rightAlignmentState;

    //void addToggleListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, bool state, const std::string& key) {
    //    auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(title, state, ON, OFF);
    //    toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get(), key](bool state) {
    //        tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
    //        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, key, state ? FALSE_STR : TRUE_STR);
    //        reinitializeWidgetVars();
    //        redrawWidget = true;
    //    });
    //    list->addItem(toggleListItem.release());
    //}

    // Helper function to add toggle list items
    //void addToggleItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, bool& stateVar, const std::string& iniKey, const std::function<void()>& onChangeCallback = nullptr) {
    //    auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(title, stateVar, ON, OFF);
    //    toggleListItem->setStateChangedListener([listItemRaw = toggleListItem.get(), &stateVar, iniKey, onChangeCallback](bool state) mutable {
    //        tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
    //        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, state ? TRUE_STR : FALSE_STR);
    //        if (stateVar != state) {
    //            stateVar = state;
    //            if (onChangeCallback) onChangeCallback();
    //        }
    //    });
    //    list->addItem(toggleListItem.release());
    //}

    void addListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, const std::string& value, const std::string& targetMenu) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(title);
        listItem->setValue(value);
        listItem->setClickListener([listItemRaw = listItem.get(), targetMenu](uint64_t keys) {
            if (runningInterpreter.load(std::memory_order_acquire))
                return false;

            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
            }
            if (keys & KEY_A) {

                if (targetMenu == "softwareUpdateMenu") {
                    //executeCommands({
                    //    {"download", LATEST_RELEASE_INFO_URL, SETTINGS_PATH}
                    //});
                    downloadFile(LATEST_RELEASE_INFO_URL, SETTINGS_PATH);
                    downloadPercentage.store(-1, std::memory_order_release);
                } else if (targetMenu == "themeMenu") {
                    if (!isFileOrDirectory(THEMES_PATH+"ultra.ini")) {
                        //executeCommands({
                        //    {"download", INCLUDED_THEME_FOLDER_URL+"ultra.ini", THEMES_PATH}
                        //});
                        downloadFile(INCLUDED_THEME_FOLDER_URL+"ultra.ini", THEMES_PATH);
                        downloadPercentage.store(-1, std::memory_order_release);
                    }
                    if (!isFileOrDirectory(THEMES_PATH+"classic.ini")) {
                        //executeCommands({
                        //    {"download", INCLUDED_THEME_FOLDER_URL+"classic.ini", THEMES_PATH}
                        //});
                        downloadFile(INCLUDED_THEME_FOLDER_URL+"classic.ini", THEMES_PATH);
                        downloadPercentage.store(-1, std::memory_order_release);
                    }
                }

                tsl::changeTo<UltrahandSettingsMenu>(targetMenu);
                selectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                simulatedSelectComplete = true;
                return true;
            }
            return false;
        });
        list->addItem(listItem.release());
    }

    void handleSelection(std::unique_ptr<tsl::elm::List>& list, const std::vector<std::string>& items, const std::string& defaultItem, const std::string& iniKey, const std::string& targetMenu) {
        std::unique_ptr<tsl::elm::ListItem> listItem;
        std::string mappedItem;
        for (const auto& item : items) {
            //auto mappedItem = convertComboToUnicode(item); // moved to ListItem class in libTesla
            //if (mappedItem.empty()) mappedItem = item;
            mappedItem = item;
            if (targetMenu == "keyComboMenu")
            	convertComboToUnicode(mappedItem);
    
            listItem = std::make_unique<tsl::elm::ListItem>(mappedItem);
            if (item == defaultItem) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }
            listItem->setClickListener([item, mappedItem, defaultItem, iniKey, targetMenu, listItemRaw = listItem.get()](uint64_t keys) {
                //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire))
                    return false;
                
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    
                    if (item != defaultItem) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, item);
                        if (targetMenu == "keyComboMenu")
                            setIniFileValue(TESLA_CONFIG_INI_PATH, TESLA_STR, iniKey, item);
                        reloadMenu = true;
                    }
                    lastSelectedListItem->setValue("");
                    selectedListItem->setValue(mappedItem);
                    listItemRaw->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                    shiftItemFocus(listItemRaw);
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
    				
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());
        }
        listItem.release();
    }


    void addUpdateButton(std::unique_ptr<tsl::elm::List>& list, const std::string& title, const std::string& downloadUrl, const std::string& targetPath, const std::string& movePath, const std::string& versionLabel) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(title);
        listItem->setValue(versionLabel, true);

        listItem->setClickListener([listItemRaw = listItem.get(), title, downloadUrl, targetPath, movePath](uint64_t keys) {
            static bool executingCommands = false;
            if (runningInterpreter.load(std::memory_order_acquire)) {
                return false;
            } else {
                if (executingCommands && commandSuccess && movePath != LANG_PATH) {
                    triggerMenuReload = true;
                }
                executingCommands = false;
            }

            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
            }
            std::vector<std::vector<std::string>> interpreterCommands;
            if (keys & KEY_A) {
                executingCommands = true;
                isDownloadCommand = true;

                if (title == UPDATE_ULTRAHAND) {
                	interpreterCommands = {
                	    {"try:"},
                	    //{"delete", THEMES_PATH+"ultra.ini"},
                	    //{"delete", THEMES_PATH+"classic.ini"},
                	    //{"delete", EXPANSION_PATH + "nx-ovlloader.zip"},
                	    //{"delete", EXPANSION_PATH + "nx-ovlloader+.zip"},
                	    {"delete", targetPath},
                	    {"download", INCLUDED_THEME_FOLDER_URL+"ultra.ini", THEMES_PATH},
                	    {"download", INCLUDED_THEME_FOLDER_URL+"classic.ini", THEMES_PATH},
                	    {"download", NX_OVLLOADER_ZIP_URL, EXPANSION_PATH},
                	    {"download", NX_OVLLOADER_PLUS_ZIP_URL, EXPANSION_PATH},
                	    {"download", downloadUrl, DOWNLOADS_PATH}
                	};
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
                    interpreterCommands.push_back({"unzip", EXPANSION_PATH + loaderTitle + ".zip", ROOT_PATH});
                }
                
                interpreterCommands.push_back({"delete", targetPath});


                runningInterpreter.store(true, std::memory_order_release);
                enqueueInterpreterCommands(std::move(interpreterCommands), "", "");
                startInterpreterThread();

                listItemRaw->setValue(INPROGRESS_SYMBOL);
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                shiftItemFocus(listItemRaw);
                lastRunningInterpreter = true;
                simulatedSelectComplete = true;
                lastSelectedListItem->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem.release());
    }

    // Helper function to create toggle list items
    auto createToggleListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& title, bool state, const std::string& iniKey, const bool invertLogic = false, const bool useReloadMenu2 = false) {
        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(title, invertLogic ? !state : state, ON, OFF);
        toggleListItem->setStateChangedListener([&, listItemRaw = toggleListItem.get(), iniKey, invertLogic, useReloadMenu2](bool newState) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, iniKey, newState ? (invertLogic ? FALSE_STR : TRUE_STR) : (invertLogic ? TRUE_STR : FALSE_STR));
            state = invertLogic ? !newState : newState;
    
            if (iniKey == "clean_version_labels") {
                versionLabel = APP_VERSION + std::string("   (") + loaderTitle + (state ? " v" : " ") + cleanVersionLabel(loaderInfo) + std::string(")");
                reinitializeVersionLabels();
            }
            else if (iniKey == "memory_expansion") {
                if (!isFileOrDirectory(EXPANSION_PATH + "nx-ovlloader.zip")) {
                    downloadFile(NX_OVLLOADER_ZIP_URL, EXPANSION_PATH);
                    downloadPercentage.store(-1, std::memory_order_release);
                }
                if (!isFileOrDirectory(EXPANSION_PATH + "nx-ovlloader+.zip")) {
                    downloadFile(NX_OVLLOADER_PLUS_ZIP_URL, EXPANSION_PATH);
                    downloadPercentage.store(-1, std::memory_order_release);
                }
                if (!isFileOrDirectory(EXPANSION_PATH + "nx-ovlloader.zip") || !isFileOrDirectory(EXPANSION_PATH + "nx-ovlloader+.zip")) {
                    listItemRaw->setState(loaderTitle == "nx-ovlloader+");
                } else {
                    executeCommands({
                        {"try:"},
                        {"del", EXPANSION_PATH + (state ? "nx-ovlloader+/" : "nx-ovlloader/")},
                        {"unzip", EXPANSION_PATH + (state ? "nx-ovlloader+.zip" : "nx-ovlloader.zip"),
                         EXPANSION_PATH + (state ? "nx-ovlloader+/" : "nx-ovlloader/")},
                        {"mv", EXPANSION_PATH + (state ? "nx-ovlloader+/" : "nx-ovlloader/"), "/"}
                    });
                }
            } else if (iniKey == "hide_clock" || iniKey == "hide_soc_temp" || iniKey == "hide_pcb_temp" || iniKey == "hide_battery") {
                reinitializeWidgetVars();
                redrawWidget = true;
            } else if (iniKey == "right_alignment") {
                triggerMenuReload = (rightAlignmentState != state);
            }
    
            reloadMenu = true;
            if (useReloadMenu2) reloadMenu2 = true;
        });
        list->addItem(toggleListItem.release());
    }
    

    std::vector<std::string> filesList;
    
public:
    UltrahandSettingsMenu(const std::string& selection = "") : dropdownSelection(selection) {}

    ~UltrahandSettingsMenu() {}

    virtual tsl::elm::Element* createUI() override {
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();
        
        const std::vector<std::string> defaultLanguagesRepresentation = {ENGLISH, SPANISH, FRENCH, GERMAN, JAPANESE, KOREAN, ITALIAN, DUTCH, PORTUGUESE, RUSSIAN, POLISH, SIMPLIFIED_CHINESE, TRADITIONAL_CHINESE};
        static const std::vector<std::string> defaultLanguages = {"en", "es", "fr", "de", "ja", "ko", "it", "nl", "pt", "ru", "pl", "zh-cn", "zh-tw"};
        static const std::vector<std::string> defaultCombos = {"ZL+ZR+DDOWN", "ZL+ZR+DRIGHT", "ZL+ZR+DUP", "ZL+ZR+DLEFT", "L+R+DDOWN", "L+R+DRIGHT", "L+R+DUP", "L+R+DLEFT", "L+DDOWN", "R+DDOWN", "ZL+ZR+PLUS", "L+R+PLUS", "ZL+PLUS", "ZR+PLUS", "MINUS+PLUS", "LS+RS", "L+DDOWN+RS"};
        
        auto list = std::make_unique<tsl::elm::List>();
        
        if (dropdownSelection.empty()) {
            addHeader(list, MAIN_SETTINGS);
            std::string defaultLang = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR);
            std::string keyCombo = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR);
            trim(keyCombo);
            defaultLang = defaultLang.empty() ? "en" : defaultLang;
            keyCombo = keyCombo.empty() ? defaultCombos[0] : keyCombo;

            convertComboToUnicode(keyCombo);
            addListItem(list, KEY_COMBO, keyCombo, "keyComboMenu");
            addListItem(list, LANGUAGE, defaultLang, "languageMenu");
            addListItem(list, SYSTEM, DROPDOWN_SYMBOL, "systemMenu");
            addListItem(list, SOFTWARE_UPDATE, DROPDOWN_SYMBOL, "softwareUpdateMenu");

            addHeader(list, UI_SETTINGS);

            std::string currentTheme = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme");
            currentTheme = (currentTheme.empty() || currentTheme == DEFAULT_STR) ? DEFAULT : currentTheme;
            addListItem(list, THEME, currentTheme, "themeMenu");

            if (expandedMemory) {
                std::string currentWallpaper = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper");
                currentWallpaper = (currentWallpaper.empty() || currentWallpaper == OPTION_SYMBOL) ? OPTION_SYMBOL : currentWallpaper;
                addListItem(list, WALLPAPER, currentWallpaper, "wallpaperMenu");
            }

            addListItem(list, WIDGET, DROPDOWN_SYMBOL, "widgetMenu");
            addListItem(list, MISCELLANEOUS, DROPDOWN_SYMBOL, "miscMenu");

        } else if (dropdownSelection == "keyComboMenu") {
            addHeader(list, KEY_COMBO);
            std::string defaultCombo = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR);
            trim(defaultCombo);
            handleSelection(list, defaultCombos, defaultCombo, KEY_COMBO_STR, "keyComboMenu");
        } else if (dropdownSelection == "languageMenu") {
            addHeader(list, LANGUAGE);
            std::string defaulLang = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR);
            size_t index = 0;
            std::string langFile;
            std::unique_ptr<tsl::elm::ListItem> listItem;

            for (const auto& defaultLangMode : defaultLanguages) {
                langFile = LANG_PATH + defaultLangMode + ".json";
                if (defaultLangMode != "en" && !isFileOrDirectory(langFile))  {index++; continue;}
                listItem = std::make_unique<tsl::elm::ListItem>(defaultLanguagesRepresentation[index]);
                listItem->setValue(defaultLangMode);
                if (defaultLangMode == defaulLang) {
                    lastSelectedListItemFooter = defaultLangMode;
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                listItem->setClickListener([this, skipLang = !isFileOrDirectory(langFile), defaultLangMode, defaulLang, langFile, listItemRaw = listItem.get()](uint64_t keys) {
                    //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                    if (runningInterpreter.load(std::memory_order_acquire)) return false;
                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR, defaultLangMode);
                        reloadMenu = reloadMenu2 = true;
                        parseLanguage(langFile);
                        if (skipLang && defaultLangMode == "en") reinitializeLangVars();
                        lastSelectedListItem->setValue(lastSelectedListItemFooter);
                        selectedListItem->setValue(defaultLangMode);
                        listItemRaw->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*){});
                        shiftItemFocus(listItemRaw);
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        lastSelectedListItemFooter = defaultLangMode;
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());
                index++;
            }
            listItem.release();
        } else if (dropdownSelection == "softwareUpdateMenu") {
            std::string versionLabel = cleanVersionLabel(parseValueFromIniSection((SETTINGS_PATH+"RELEASE.ini"), "Release Info", "latest_version"));

            addHeader(list, SOFTWARE_UPDATE);
            addUpdateButton(list, UPDATE_ULTRAHAND, ULTRAHAND_REPO_URL + "releases/latest/download/ovlmenu.ovl", "/config/ultrahand/downloads/ovlmenu.ovl", "/switch/.overlays/ovlmenu.ovl", versionLabel);
            addUpdateButton(list, UPDATE_LANGUAGES, ULTRAHAND_REPO_URL + "releases/latest/download/lang.zip", "/config/ultrahand/downloads/lang.zip", LANG_PATH, versionLabel);

            PackageHeader overlayHeader;
            overlayHeader.title = "Ultrahand Overlay";
            overlayHeader.version = APP_VERSION;
            overlayHeader.creator = "ppkantorski";
            overlayHeader.about = "Ultrahand Overlay is a versatile tool that enables you to create and share custom command-based packages.";
            overlayHeader.credits = "Special thanks to B3711, ComplexNarrative, Faker_dev, MasaGratoR, meha, WerWolv, HookedBehemoth and many others. ♥";
            addPackageInfo(list, overlayHeader, OVERLAY_STR);
            overlayHeader.clear();

        } else if (dropdownSelection == "systemMenu") {
            
            // Version info formatting with a reduced buffer
            char versionString[32];  // Reduced buffer size to 32
            snprintf(versionString, sizeof(versionString), "HOS %s│AMS %s", 
                     hosVersion, amsVersion);
            
            std::string hekateVersion = extractVersionFromBinary("sdmc:/bootloader/update.bin");
            
            addHeader(list, DEVICE_INFO);
            
            SetSysProductModel model = SetSysProductModel_Invalid;
            setsysGetProductModel(&model);
            
            const char* modelRev;
            switch (model) {
                case SetSysProductModel_Iowa: modelRev = "Iowa│Tegra X1+ (Mariko)"; break;
                case SetSysProductModel_Hoag: modelRev = "Hoag│Tegra X1+ (Mariko)"; break;
                case SetSysProductModel_Calcio: modelRev = "Calcio│Tegra X1+ (Mariko)"; break;
                case SetSysProductModel_Aula: modelRev = "Aula│Tegra X1+ (Mariko)"; break;
                case SetSysProductModel_Nx: modelRev = "Icosa│Tegra X1 (Erista)"; break;
                case SetSysProductModel_Copper: modelRev = "Copper│Tegra X1 (Erista)"; break;
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
            addTable(list, tableData, "", 163, 8, 3, 0, DEFAULT_STR, "section", RIGHT_STR, true);
            
            tableData.clear();
            tableData.resize(2);
            
            if (cpuSpeedo0 != 0 && cpuSpeedo2 != 0 && socSpeedo0 != 0 && cpuIDDQ != 0 && gpuIDDQ != 0 && socIDDQ != 0) {
                tableData[0] = {
                    "Speedo", "",
                    customAlign(cpuSpeedo0) + " │ " + customAlign(cpuSpeedo2) + " │ " + customAlign(socSpeedo0)
                };
                tableData[1] = {
                    "IDDQ", "",
                    customAlign(cpuIDDQ) + " │ " + customAlign(gpuIDDQ) + " │ " + customAlign(socIDDQ)
                };
            } else {
                tableData[0] = {"Speedo", "", "⋯    │    ⋯   │    ⋯  "};
                tableData[1] = {"IDDQ", "", "⋯    │    ⋯   │    ⋯  "};
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
            float freeRamMB = (static_cast<float>(RAM_Total_system_u - RAM_Used_system_u) / (1024.0f * 1024.0f)) - 8.0f;
            snprintf(ramString, sizeof(ramString), "%.2f MB %s", freeRamMB, FREE.c_str());
            
            // Reuse tableData with minimal reallocation
            tableData = {
                {NOTICE, "", UTILIZES + " 2 MB (" + ramString + ")"}
            };
            addTable(list, tableData, "", 163, 10, 7, 0, DEFAULT_STR, DEFAULT_STR, RIGHT_STR, true);
            // Memory expansion toggle
            useMemoryExpansion = (loaderTitle == "nx-ovlloader+" || 
                                  parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "memory_expansion") == TRUE_STR);
            createToggleListItem(list, MEMORY_EXPANSION, useMemoryExpansion, "memory_expansion", false, true);

            // Reboot required info
            tableData = {
                {"", "", REBOOT_REQUIRED}  // Direct reuse without reallocation
            };
            addTable(list, tableData, "", 163, 28, 0, 0, DEFAULT_STR, DEFAULT_STR, RIGHT_STR, true);
        
        } else if (dropdownSelection == "themeMenu") {
            addHeader(list, THEME);
            std::string currentTheme = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme");
            currentTheme = currentTheme.empty() ? DEFAULT_STR : currentTheme;
            auto listItem = std::make_unique<tsl::elm::ListItem>(DEFAULT);
            if (currentTheme == DEFAULT_STR) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }
            listItem->setClickListener([defaultTheme = THEMES_PATH + "default.ini", listItemRaw = listItem.get()](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire)) return false;
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", DEFAULT_STR);
                    deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                    if (isFileOrDirectory(defaultTheme)) {
                        copyFileOrDirectory(defaultTheme, THEME_CONFIG_INI_PATH);
                        copyPercentage.store(-1, std::memory_order_release);
                    }
                    else initializeTheme();
                    tsl::initializeThemeVars();
                    reloadMenu = reloadMenu2 = true;
                    lastSelectedListItem->setValue("");
                    selectedListItem->setValue(DEFAULT);
                    listItemRaw->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*){});
                    shiftItemFocus(listItemRaw);
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());

            filesList = getFilesListByWildcards(THEMES_PATH + "*.ini");
            std::sort(filesList.begin(), filesList.end());

            std::string themeName;
            for (const auto& themeFile : filesList) {
                themeName = getNameFromPath(themeFile);
                dropExtension(themeName);
                if (themeName == DEFAULT_STR) continue;
                listItem = std::make_unique<tsl::elm::ListItem>(themeName);
                if (themeName == currentTheme) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                listItem->setClickListener([themeName, themeFile, listItemRaw = listItem.get()](uint64_t keys) {
                    if (runningInterpreter.load(std::memory_order_acquire)) return false;
                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_theme", themeName);
                        //deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                        copyFileOrDirectory(themeFile, THEME_CONFIG_INI_PATH);
                        copyPercentage.store(-1, std::memory_order_release);
                        initializeTheme();
                        tsl::initializeThemeVars();
                        reloadMenu = reloadMenu2 = true;
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(themeName);
                        listItemRaw->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*){});
                        shiftItemFocus(listItemRaw);
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());
            }
        } else if (dropdownSelection == "wallpaperMenu") {
            addHeader(list, WALLPAPER);
            std::string currentWallpaper = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper");
            currentWallpaper = currentWallpaper.empty() ? OPTION_SYMBOL : currentWallpaper;

            auto listItem = std::make_unique<tsl::elm::ListItem>(OPTION_SYMBOL);
            if (currentWallpaper == OPTION_SYMBOL) {
                listItem->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
            }

            listItem->setClickListener([listItemRaw = listItem.get()](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire)) return false;
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper", "");
                    deleteFileOrDirectory(WALLPAPER_PATH);
                    reloadWallpaper();
                    //refreshWallpaper.store(true, std::memory_order_release);

                    //deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                    //if (isFileOrDirectory(defaultTheme)) copyFileOrDirectory(defaultTheme, THEME_CONFIG_INI_PATH);
                    //else initializeTheme();
                    //tsl::initializeThemeVars();
                    reloadMenu = reloadMenu2 = true;
                    lastSelectedListItem->setValue("");
                    selectedListItem->setValue(OPTION_SYMBOL);
                    listItemRaw->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*){});
                    shiftItemFocus(listItemRaw);
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
                    
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());

            filesList = getFilesListByWildcards(WALLPAPERS_PATH + "*.rgba");
            std::sort(filesList.begin(), filesList.end());

            std::string wallpaperName;
            for (const auto& wallpaperFile : filesList) {
                wallpaperName = getNameFromPath(wallpaperFile);
                dropExtension(wallpaperName);
                if (wallpaperName == DEFAULT_STR) continue;
                listItem = std::make_unique<tsl::elm::ListItem>(wallpaperName);
                if (wallpaperName == currentWallpaper) {
                    listItem->setValue(CHECKMARK_SYMBOL);
                    lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){});
                }
                listItem->setClickListener([wallpaperName, wallpaperFile, listItemRaw = listItem.get()](uint64_t keys) {
                    if (runningInterpreter.load(std::memory_order_acquire)) return false;
                    if (simulatedSelect && !simulatedSelectComplete) {
                        keys |= KEY_A;
                        simulatedSelect = false;
                    }
                    if (keys & KEY_A) {
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "current_wallpaper", wallpaperName);
                        //deleteFileOrDirectory(THEME_CONFIG_INI_PATH);
                        copyFileOrDirectory(wallpaperFile, WALLPAPER_PATH);
                        copyPercentage.store(-1, std::memory_order_release);
                        reloadWallpaper();
                        
                        //clearWallpaperData();
                        //initializeTheme();
                        //tsl::initializeThemeVars();
                        reloadMenu = reloadMenu2 = true;
                        lastSelectedListItem->setValue("");
                        selectedListItem->setValue(wallpaperName);
                        listItemRaw->setValue(CHECKMARK_SYMBOL);
                        lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*){});
                        shiftItemFocus(listItemRaw);
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem.release());
            }
        } else if (dropdownSelection == "widgetMenu") {
            addHeader(list, WIDGET);
            createToggleListItem(list, CLOCK, hideClock, "hide_clock", true);
            createToggleListItem(list, SOC_TEMPERATURE, hideSOCTemp, "hide_soc_temp", true);
            createToggleListItem(list, PCB_TEMPERATURE, hidePCBTemp, "hide_pcb_temp", true);
            createToggleListItem(list, BATTERY, hideBattery, "hide_battery", true);

        } else if (dropdownSelection == "miscMenu") {
            addHeader(list, MENU_ITEMS);
            
            hideUserGuide = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_user_guide") == TRUE_STR);
            createToggleListItem(list, USER_GUIDE, hideUserGuide, "hide_user_guide", true);
            
            cleanVersionLabels = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "clean_version_labels") == TRUE_STR);
            createToggleListItem(list, CLEAN_VERSIONS, cleanVersionLabels, "clean_version_labels", false, true);
            
            hideOverlayVersions = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_overlay_versions") == TRUE_STR);
            createToggleListItem(list, OVERLAY_VERSIONS, hideOverlayVersions, "hide_overlay_versions", true);
            
            hidePackageVersions = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_package_versions") == TRUE_STR);
            createToggleListItem(list, PACKAGE_VERSIONS, hidePackageVersions, "hide_package_versions", true);
            
            addHeader(list, EFFECTS);

            useSwipeToOpen = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "swipe_to_open") == TRUE_STR);
            createToggleListItem(list, SWIPE_TO_OPEN, useSwipeToOpen, "swipe_to_open");

            useRightAlignment = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "right_alignment") == TRUE_STR);
            rightAlignmentState = useRightAlignment;
            createToggleListItem(list, RIGHT_SIDE_MODE, useRightAlignment, "right_alignment");


            useOpaqueScreenshots = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "opaque_screenshots") == TRUE_STR);
            createToggleListItem(list, OPAQUE_SCREENSHOTS, useOpaqueScreenshots, "opaque_screenshots");
            
            //std::vector<std::vector<std::string>> commands = {
            //    {"set-ini-value", ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "overscan", "{value}"}
            //};
            //
            //list->addItem(new tsl::elm::TrackBarV2(TV_OVERSCAN, PACKAGE_PATH, 80, 100, "%", interpretAndExecuteCommands, getSourceReplacement, commands, "overscan", false, false, -1, false, false));

            //progressAnimation = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "progress_animation") == TRUE_STR);
            //createToggleListItem(list, PROGRESS_ANIMATION, progressAnimation, "progress_animation");
        
        } else {
            addBasicListItem(list, FAILED_TO_OPEN + ": " + settingsIniPath);
            //list->addItem(new tsl::elm::ListItem(FAILED_TO_OPEN + ": " + settingsIniPath));
        }

        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        rootFrame->setContent(list.release());
        return rootFrame.release();

        //return returnRootFrame(list, CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
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

        //if (refreshWallpaper.load(std::memory_order_acquire)) {
        //    reloadWallpaper();
        //    refreshWallpaper.store(false, std::memory_order_release);
        //}

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
                    allowSlide = unlockedSlide = false;
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
                allowSlide = unlockedSlide = false;
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
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
            tsl::Overlay::get()->close();
        }

        return false;
    }
};




class SettingsMenu : public tsl::Gui {
private:
    std::string entryName, entryMode, overlayName, packageName, dropdownSelection, settingsIniPath;
    bool isInSection, inQuotes, isFromMainMenu;
    int MAX_PRIORITY = 20;

public:
    SettingsMenu(const std::string& name, const std::string& mode, const std::string& overlayName = "", const std::string& packageName = "", const std::string& selection = "")
        : entryName(name), entryMode(mode), overlayName(overlayName), packageName(packageName), dropdownSelection(selection) {}

    ~SettingsMenu() {}

    void createAndAddToggleListItem(
        std::unique_ptr<tsl::elm::List>& list,
        const std::string& label,
        bool initialState,
        const std::string& iniKey,
        std::string currentValue,
        const std::string& settingsIniPath,
        const std::string& entryName,
        bool handleReload = false
    ) {
        if (currentValue.empty() && !initialState) currentValue = FALSE_STR;

        auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(label, initialState, ON, OFF);
        toggleListItem->setState(currentValue != FALSE_STR);
        toggleListItem->setStateChangedListener([this, iniKey, listItemRaw = toggleListItem.get(), handleReload](bool state) {
            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
            setIniFileValue(this->settingsIniPath, this->entryName, iniKey, state ? TRUE_STR : FALSE_STR);
            if (handleReload) {
                reloadMenu = state || (reloadMenu2 = !state);
            }
        });
        list->addItem(toggleListItem.release());
    }

    void createAndAddListItem(
        std::unique_ptr<tsl::elm::List>& list,
        const std::string& iStr,
        const std::string& priorityValue,
        const std::string& settingsIniPath,
        const std::string& entryName
    ) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(iStr);

        if (iStr == priorityValue) {
            listItem->setValue(CHECKMARK_SYMBOL);
            lastSelectedListItem.reset();
            lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*) {});
        }

        listItem->setClickListener([this, iStr, priorityValue, listItemRaw = listItem.get()](uint64_t keys) {
            if (runningInterpreter.load(std::memory_order_acquire)) return false;

            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
            }

            if (keys & KEY_A) {
                if (iStr != priorityValue)
                    reloadMenu = true; // Modify the global variable
    
                setIniFileValue(this->settingsIniPath, this->entryName, PRIORITY_STR, iStr);
                lastSelectedListItem->setValue("");
                selectedListItem->setValue(iStr);
                listItemRaw->setValue(CHECKMARK_SYMBOL);
                lastSelectedListItem.reset();
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*){});
                shiftItemFocus(listItemRaw);
                simulatedSelectComplete = true;
                lastSelectedListItem->triggerClickAnimation();
            }
            return false;
        });

        list->addItem(listItem.release());
    }

    virtual tsl::elm::Element* createUI() override {
        settingsIniPath = (entryMode == OVERLAY_STR) ? OVERLAYS_INI_FILEPATH : PACKAGES_INI_FILEPATH;
        std::string header = (entryMode == OVERLAY_STR) ? overlayName : packageName;
        inSettingsMenu = dropdownSelection.empty();
        inSubSettingsMenu = !dropdownSelection.empty();

        auto list = std::make_unique<tsl::elm::List>();

        if (inSettingsMenu) {
            addHeader(list, header + " " + SETTINGS);
            //std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, PRIORITY_STR);
            //std::string hideOption = parseValueFromIniSection(settingsIniPath, entryName, HIDE_STR);

            createAndAddToggleListItem(
                list,
                (entryMode == OVERLAY_STR) ? HIDE_OVERLAY : HIDE_PACKAGE,
                false,
                HIDE_STR,
                parseValueFromIniSection(settingsIniPath, entryName, HIDE_STR),
                settingsIniPath,
                entryName,
                true
            );

            auto listItem = std::make_unique<tsl::elm::ListItem>(SORT_PRIORITY);
            listItem->setValue(parseValueFromIniSection(settingsIniPath, entryName, PRIORITY_STR));
            listItem->setClickListener([this, listItemRaw = listItem.get()](uint64_t keys) {
                if (runningInterpreter.load(std::memory_order_acquire)) return false;
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    inMainMenu = false;
                    tsl::changeTo<SettingsMenu>(this->entryName, this->entryMode, this->overlayName, "", PRIORITY_STR);
                    selectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                    simulatedSelectComplete = true;
                    lastSelectedListItem->triggerClickAnimation();
                    return true;
                }
                return false;
            });
            list->addItem(listItem.release());

            if (entryMode == OVERLAY_STR) {
                createAndAddToggleListItem(
                    list,
                    LAUNCH_ARGUMENTS,
                    false,
                    USE_LAUNCH_ARGS_STR,
                    parseValueFromIniSection(settingsIniPath, entryName, USE_LAUNCH_ARGS_STR),
                    settingsIniPath,
                    entryName
                );
            } else if (entryMode == PACKAGE_STR) {
                createAndAddToggleListItem(
                    list,
                    BOOT_COMMANDS,
                    true,
                    USE_BOOT_PACKAGE_STR,
                    parseValueFromIniSection(settingsIniPath, entryName, USE_BOOT_PACKAGE_STR),
                    settingsIniPath,
                    entryName
                );
                createAndAddToggleListItem(
                    list,
                    EXIT_COMMANDS,
                    true,
                    USE_EXIT_PACKAGE_STR,
                    parseValueFromIniSection(settingsIniPath, entryName, USE_EXIT_PACKAGE_STR),
                    settingsIniPath,
                    entryName
                );
                createAndAddToggleListItem(
                    list,
                    ERROR_LOGGING,
                    false,
                    USE_LOGGING_STR,
                    parseValueFromIniSection(settingsIniPath, entryName, USE_LOGGING_STR),
                    settingsIniPath,
                    entryName
                );
            }
        } else if (dropdownSelection == PRIORITY_STR) {
            addHeader(list, SORT_PRIORITY);
            std::string priorityValue = parseValueFromIniSection(settingsIniPath, entryName, PRIORITY_STR);
            for (int i = 0; i <= MAX_PRIORITY; ++i) {
                createAndAddListItem(
                    list,
                    std::to_string(i),
                    priorityValue,
                    settingsIniPath,
                    entryName
                );
            }
        } else {
            addBasicListItem(list, FAILED_TO_OPEN + ": " + settingsIniPath);
        }

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
                    allowSlide = unlockedSlide = false;
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
                allowSlide = unlockedSlide = false;
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

//std::string lastPackageHeader;

bool overrideTitle = false, overrideVersion = false;





class ScriptOverlay : public tsl::Gui {
private:
    std::vector<std::vector<std::string>> commands;
    std::string filePath, specificKey;
    bool isInSection = false, inQuotes = false, isFromMainMenu = false, isFromPackage = false, isFromSelectionMenu = false;
    bool tableMode = false;

    std::string lastPackageHeader;

    void addListItem(std::unique_ptr<tsl::elm::List>& list, const std::string& line) {
        auto listItem = std::make_unique<tsl::elm::ListItem>(line);

        listItem->setClickListener([this, listItemRaw = listItem.get(), line](uint64_t keys) {
            if (runningInterpreter.load(std::memory_order_acquire)) return false;
            if (simulatedSelect && !simulatedSelectComplete) {
                keys |= KEY_A;
                simulatedSelect = false;
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
                        }
                    } else if (ch == ' ' && !inQuotes) {
                        if (!currentPart.empty()) {
                            commandParts.emplace_back(std::move(currentPart));
                            currentPart.clear();
                        }
                    } else {
                        currentPart += ch;
                    }
                }
                if (!currentPart.empty()) {
                    commandParts.emplace_back(std::move(currentPart));
                }

                commandVec.emplace_back(std::move(commandParts));

                //interpretAndExecuteCommands(std::move(commandVec), filePath, specificKey);
                //resetPercentages();
                //listItemRaw->setValue(commandSuccess ? CHECKMARK_SYMBOL : CROSSMARK_SYMBOL);

                enqueueInterpreterCommands(std::move(commandVec), filePath, specificKey);
                startInterpreterThread();
                listItemRaw->setValue(INPROGRESS_SYMBOL);

                lastSelectedListItem.reset();
                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
               
                lastRunningInterpreter = true;
                simulatedSelectComplete = true;
                listItemRaw->triggerClickAnimation();
                return true;
            }
            return false;
        });
        list->addItem(listItem.release());
    }

public:
    ScriptOverlay(std::vector<std::vector<std::string>> cmds, const std::string& file, const std::string& key = "", const std::string& fromMenu = "", bool tableMode = false, const std::string& _lastPackageHeader = "")
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
        auto list = std::make_unique<tsl::elm::List>();

        
		
		bool noClickableItems = false;
        if (!tableMode) {
        	size_t index = 0, tryCount = 0;
            // If not in table mode, loop through commands and display each command as a list item
            for (const auto& command : commands) {
            	if (index == 0 && command[0] != "try:" && command[0] != "on:" && command[0] != "off:") {
            		addHeader(list, specificKey);
            	}
				if (command[0] == "try:") {
            		tryCount++;
            		index++;
            		addHeader(list, specificKey+" (Try #"+std::to_string(tryCount)+")");
            		continue;
            	}
				if (command[0] == "on:") {
            		index++;
            		addHeader(list, specificKey+" ("+ON+")");
            		continue;
            	}
				if (command[0] == "off:") {
            		index++;
            		addHeader(list, specificKey+" ("+OFF+")");
            		continue;
            	}
                std::string combinedCommand = joinCommands(command); // Join commands into a single line for display
                addListItem(list, combinedCommand);
                index++;
            }
	    } else {
	    	addHeader(list, specificKey);

	    	noClickableItems = true;
	    	std::vector<std::string> sectionLines;  // Holds the sections (commands)
	    	std::vector<std::string> infoLines;     // Holds the info (empty in this case)
	        // Table mode: Collect command data for the table
	        std::string sectionLine;
	        for (const auto& command : commands) {
	            // Each command will be treated as a section with no corresponding info
	            sectionLine = joinCommands(command);  // Combine command parts into a section line
	            sectionLines.push_back(sectionLine);              // Add to section lines
	            infoLines.push_back("");                          // Empty info line
	        }
	
	        // Use default parameters for the table view
	        const size_t tableColumnOffset = 163;
	        const size_t tableStartGap = 19;
	        const size_t tableEndGap = 12;
	        const size_t tableSpacing = 10;
	        const std::string tableSectionTextColor = DEFAULT_STR;
	        const std::string tableInfoTextColor = DEFAULT_STR;
	        const std::string tableAlignment = LEFT_STR;
	        const bool hideTableBackground = false;
	        const bool useHeaderIndent = false;
	        const bool isScrollableTable = true;
	        const std::string wrappingMode = "char";
	        const bool useWrappedTextIndent = true;
	
	        //const bool usingTopPivot = true;
	        const bool usingBottomPivot = false;

            addDummyListItem(list);
	        // Draw the table using the sectionLines and empty infoLines
	        drawTable(list, sectionLines, infoLines, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
	                  tableSectionTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isScrollableTable, wrappingMode, useWrappedTextIndent);

            if (usingBottomPivot) {
                addDummyListItem(list);
                //lastItemIsScrollableTable = false;
            }
	    }

        std::string packageVersion = isFromMainMenu ? "" : packageRootLayerVersion;
        
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(packageName,
        	!lastPackageHeader.empty() ? lastPackageHeader + "(Ultrahand Script*)" : (packageVersion.empty() ? CAPITAL_ULTRAHAND_PROJECT_NAME + " Script" : packageVersion + "   (" + CAPITAL_ULTRAHAND_PROJECT_NAME + " Script)"),
        	noClickableItems);
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
                allowSlide = unlockedSlide = false;
                inScriptMenu = false;
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
                simulatedBackComplete = true;
                return true;
            }
        }

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
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
	    for (const auto& part : commandParts) {
	        std::string argument = part;

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




/**
 * @brief The `SelectionOverlay` class manages the selection overlay functionality.
 *
 * This class handles the selection overlay, allowing users to interact with and select various options.
 * It provides functions for creating the graphical user interface (GUI), handling user input, and executing commands.
 */
class SelectionOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, groupingName, lastGroupingName;
    std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
    std::string specifiedFooterKey;
    bool toggleState = false;
    std::string packageConfigIniPath;
    std::string commandSystem, commandMode, commandGrouping;
    
    std::string lastSelectedListItemFooter = "";

    std::string lastPackageHeader;

    // For handling on/off file_source toggle states
    std::unordered_map<int, std::string> currentSelectedItems;
    std::unordered_map<int, bool> isInitialized;

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

public:
    SelectionOverlay(const std::string& path, const std::string& key = "", const std::vector<std::vector<std::string>>& cmds = {}, const std::string& footerKey = "", const std::string& _lastPackageHeader = "")
        : filePath(path), specificKey(key), commands(std::move(cmds)), specifiedFooterKey(footerKey), lastPackageHeader(_lastPackageHeader) {
        //lastSelectedListItem.reset();
    }

    ~SelectionOverlay() {
        lastSelectedListItem.reset();
    }

    void processSelectionCommands() {
        //commands.erase(std::remove_if(commands.begin(), commands.end(),
        //    [](const std::vector<std::string>& vec) {
        //        return vec.empty();
        //    }),
        //    commands.end());
        removeEmptyCommands(commands);

        bool inEristaSection = false;
        bool inMarikoSection = false;
        std::string currentSection = GLOBAL_STR;
        std::string iniFilePath;

        std::string commandName;
        std::string filterEntry;
        std::vector<std::string> newFiles, newFilesOn, newFilesOff;

        for (auto& cmd : commands) {
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
                    if (!iniFilePath.empty()){
                        applyReplaceIniPlaceholder(cmd[1], INI_FILE_STR, iniFilePath);
                    }


                    if (commandName == "ini_file") {
                        iniFilePath = cmd[1];
                        preprocessPath(iniFilePath, filePath);
                        continue;
                    } else if (commandName == "filter") {
                        filterEntry = cmd[1];
                        removeQuotes(filterEntry);
                        if (sourceType == FILE_STR) {
                            preprocessPath(filterEntry, filePath);
                        }

                        if (currentSection == GLOBAL_STR)
                            filterList.push_back(std::move(filterEntry));
                        else if (currentSection == ON_STR)
                            filterListOn.push_back(std::move(filterEntry));
                        else if (currentSection == OFF_STR)
                            filterListOff.push_back(std::move(filterEntry));
                    } else if (commandName == "file_source") {
                        sourceType = FILE_STR;
                        if (currentSection == GLOBAL_STR) {
                            //logMessage("cmd[1]: "+cmd[1]);
                            pathPattern = cmd[1];
                            preprocessPath(pathPattern, filePath);
                            //logMessage("pathPattern: "+pathPattern);
                            newFiles = getFilesListByWildcards(pathPattern);
                            filesList.insert(filesList.end(), newFiles.begin(), newFiles.end()); // Append new files
                        } else if (currentSection == ON_STR) {
                            pathPatternOn = cmd[1];
                            preprocessPath(pathPatternOn, filePath);
                            newFilesOn = getFilesListByWildcards(pathPatternOn);
                            filesListOn.insert(filesListOn.end(), newFilesOn.begin(), newFilesOn.end()); // Append new files
                            sourceTypeOn = FILE_STR;
                        } else if (currentSection == OFF_STR) {
                            pathPatternOff = cmd[1];
                            preprocessPath(pathPatternOff, filePath);
                            newFilesOff = getFilesListByWildcards(pathPatternOff);
                            filesListOff.insert(filesListOff.end(), newFilesOff.begin(), newFilesOff.end()); // Append new files
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
            }
        }
    }

    virtual tsl::elm::Element* createUI() override {
        //filesList.clear();
        //filesListOn.clear();
        //filesListOff.clear();
        //filterList.clear();
        //filterListOn.clear();
        //filterListOff.clear();
        //currentSelectedItems.clear();
        //isInitialized.clear();
        //selectedItemsList.clear();


        inSelectionMenu = true;
        PackageHeader packageHeader = getPackageHeaderFromIni(filePath + PACKAGE_FILENAME);

        auto list = std::make_unique<tsl::elm::List>();
        packageConfigIniPath = filePath + CONFIG_FILENAME;

        commandSystem = commandSystems[0];
        commandMode = commandModes[0];
        commandGrouping = commandGroupings[0];

        processSelectionCommands();

        std::vector<std::string> selectedItemsListOn, selectedItemsListOff;

        std::string currentPackageHeader;

        if (commandMode == DEFAULT_STR || commandMode == OPTION_STR) {
            if (sourceType == FILE_STR)
                selectedItemsList = std::move(filesList);
            else if (sourceType == LIST_STR || sourceType == LIST_FILE_STR)
                selectedItemsList = (sourceType == LIST_STR) ? stringToList(listString) : readListFromFile(listPath);
            else if (sourceType == INI_FILE_STR)
                selectedItemsList = parseSectionsFromIni(iniPath);
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
            else if (sourceTypeOn == INI_FILE_STR)
                selectedItemsListOn = parseSectionsFromIni(iniPathOn);
            else if (sourceTypeOn == JSON_STR || sourceTypeOn == JSON_FILE_STR) {
                populateSelectedItemsList(sourceTypeOn, (sourceTypeOn == JSON_STR) ? jsonStringOn : jsonPathOn, jsonKeyOn, selectedItemsListOn);
                jsonPathOn.clear();
                jsonStringOn.clear();
            }

            if (sourceTypeOff == FILE_STR)
                selectedItemsListOff = std::move(filesListOff);
            else if (sourceTypeOff == LIST_STR || sourceTypeOff == LIST_FILE_STR)
                selectedItemsListOff = (sourceTypeOff == LIST_STR) ? stringToList(listStringOff) : readListFromFile(listPathOff);
            else if (sourceTypeOff == INI_FILE_STR)
                selectedItemsListOff = parseSectionsFromIni(iniPathOff);
            else if (sourceTypeOff == JSON_STR || sourceTypeOff == JSON_FILE_STR) {
                populateSelectedItemsList(sourceTypeOff, (sourceTypeOff == JSON_STR) ? jsonStringOff : jsonPathOff, jsonKeyOff, selectedItemsListOff);
                jsonPathOff.clear();
                jsonStringOff.clear();
            }

            if (sourceType == FILE_STR) {
                filterItemsList(filterListOn, selectedItemsListOn);
                filterListOn.clear();

                filterItemsList(filterListOff, selectedItemsListOff);
                filterListOff.clear();
            }


            selectedItemsList.reserve(selectedItemsListOn.size() + selectedItemsListOff.size());
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
            } else {
                std::sort(selectedItemsList.begin(), selectedItemsList.end(), [](const std::string& a, const std::string& b) {
                    return getNameFromPath(a) < getNameFromPath(b);
                });
            }
        }

        if (sourceType == FILE_STR) {
            filterItemsList(filterList, selectedItemsList);
            filterList.clear();
        }

        if (commandGrouping == DEFAULT_STR) {
            std::string cleanSpecificKey = specificKey.substr(1);
            removeTag(cleanSpecificKey);
            addHeader(list, cleanSpecificKey);
            currentPackageHeader = cleanSpecificKey;
        }

        std::unique_ptr<tsl::elm::ListItem> listItem;
        size_t pos;
        std::string parentDirName;
        std::string footer;
        std::string optionName;
        //bool toggleStateOn;

        if (selectedItemsList.empty()) {
            if (commandGrouping != DEFAULT_STR) {
                std::string cleanSpecificKey = specificKey.substr(1);
                removeTag(cleanSpecificKey);
                addHeader(list, cleanSpecificKey);
                currentPackageHeader = cleanSpecificKey;
            }
            listItem = std::make_unique<tsl::elm::ListItem>(EMPTY);
            list->addItem(listItem.release());
        }

        std::string tmpSelectedItem;

        for (size_t i = 0; i < selectedItemsList.size(); ++i) {
            const std::string& selectedItem = selectedItemsList[i];

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

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
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

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
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

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
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

                    if (lastGroupingName.empty() || (lastGroupingName != groupingName)) {
                        addHeader(list, groupingName);
                        currentPackageHeader = groupingName;
                        lastGroupingName = groupingName;
                    }
                }
            } else {
                if (commandMode == TOGGLE_STR) {
                    if (std::find(filterListOn.begin(), filterListOn.end(), itemName) != filterListOn.end() ||
                        std::find(filterListOff.begin(), filterListOff.end(), itemName) != filterListOff.end()) {
                        continue;
                    }
                } else {
                    if (std::find(filterList.begin(), filterList.end(), itemName) != filterList.end()) {
                        continue;
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
                    footer = getNameFromPath(selectedItem);
                    dropExtension(footer);
                }

                listItem = std::make_unique<tsl::elm::ListItem>(itemName);

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

                listItem->setClickListener([this, i, footer, listItemRaw = listItem.get(), _currentPackageHeader = currentPackageHeader](uint64_t keys) {
                    //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*) {})](uint64_t keys) {

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
                        //std::string selectedItemStr = std::string(selectedItem);

                        enqueueInterpreterCommands(getSourceReplacement(commands, selectedItemsList[i], i, filePath), filePath, specificKey);
                        startInterpreterThread(filePath);

                        listItemRaw->setValue(INPROGRESS_SYMBOL);

                        
                        if (commandMode == OPTION_STR) {
                            selectedFooterDict[specifiedFooterKey] = listItemRaw->getText();
                            if (lastSelectedListItem)
                                lastSelectedListItem->setValue(lastSelectedListItemFooter, true);
                            //std::string footerStr = std::string(footer);
                            lastSelectedListItemFooter = footer;
                        }

                        lastSelectedListItem.reset();
                        lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                        shiftItemFocus(listItemRaw);

                        lastRunningInterpreter = true;
                        simulatedSelectComplete = true;
                        lastSelectedListItem->triggerClickAnimation();
                        return true;
                    }

                    else if (keys & SCRIPT_KEY) {
                        //bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                        //if (inMainMenu) {
                        //    isFromMainMenu = true;
                        //    inMainMenu = false;
                        //}
                        //if (inPackageMenu)
                        //    inPackageMenu = false;
                        //if (inSubPackageMenu)
                        //    inSubPackageMenu = false;
                        inSelectionMenu = false;

                        auto modifiedCmds = getSourceReplacement(commands, selectedItemsList[i], i, filePath);
                        applyPlaceholderReplacementsToCommands(modifiedCmds);
                        //tsl::changeTo<ScriptOverlay>(modifiedCmds, filePath, specificKey+" - "+ selectedItemsList[i], "selection");
                        tsl::changeTo<ScriptOverlay>(modifiedCmds, filePath, getNameFromPath(selectedItemsList[i]), "selection", false, _currentPackageHeader);
                        return true;
                    }

                    return false;
                });
                list->addItem(listItem.release());

            } else if (commandMode == TOGGLE_STR) {
                auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(itemName, false, ON, OFF);

                bool toggleStateOn = std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end();
                toggleListItem->setState(toggleStateOn);

                toggleListItem->setStateChangedListener([this, i, listItemRaw = toggleListItem.get()](bool state) {
                    // Initialize currentSelectedItem for this index if it does not exist
                    if (isInitialized.find(i) == isInitialized.end() || !isInitialized[i]) {
                        currentSelectedItems[i] = selectedItemsList[i];
                        isInitialized[i] = true;
                    }
                    
                    tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                    
                    auto modifiedCmds = getSourceReplacement(!state ? commandsOn : commandsOff, currentSelectedItems[i], i, filePath);
                    //auto modifiedCmdsCopy = modifiedCmds;
                    //interpretAndExecuteCommands(std::move(modifiedCmds), filePath, specificKey);
                    
                    if (sourceType == FILE_STR) {
                        // Reset variables
                        std::string selectedFileName;
                        std::string updatedFileSource;
                        
                        // Extract the destination directory from the move command
                        for (const auto& cmd : modifiedCmds) {
                            if (cmd.size() > 1 && cmd[0] == "file_name") {
                                selectedFileName = cmd[1];
                                //logMessage("Selected file name: " + selectedFileName);
                                break; // Assuming there's only one move command at the end
                            }
                        }
                        
                        if (!selectedFileName.empty()) {
                            for (const auto& arg : !state ? commandsOff : commandsOn) {
                                if (arg.size() > 1 && arg[0] == "file_source") {
                                    updatedFileSource = arg[1];
                                    break;
                                }
                            }
                            
                            //logMessage("Original file source: " + updatedFileSource);
                            
                            // Replace the filename / folder name wildcard placeholder (rightmost) within updatedFileSource with the selectedFileName
                            size_t pos = updatedFileSource.rfind('*');
                            if (pos != std::string::npos) {
                                std::string prefix = updatedFileSource.substr(0, pos);
                                std::string suffix = updatedFileSource.substr(pos + 1);
                                
                                // Check if suffix contains a file extension
                                size_t extPos = selectedFileName.find('.');
                                if (extPos != std::string::npos) {
                                    // It's a file, include the file extension
                                    updatedFileSource = prefix + selectedFileName + suffix;
                                } else {
                                    // It's a folder, exclude the file extension if present in the suffix
                                    size_t extSuffixPos = suffix.find('.');
                                    if (extSuffixPos != std::string::npos) {
                                        updatedFileSource = prefix + selectedFileName + suffix.substr(extSuffixPos);
                                    } else {
                                        updatedFileSource = prefix + selectedFileName + suffix;
                                    }
                                }
                                currentSelectedItems[i] = updatedFileSource;
                                
                                // Debug logging
                                //logMessage("Updated file source for index " + std::to_string(i) + ": " + updatedFileSource);
                            } else {
                                //logMessage("Wildcard '*' not found in file source.");
                            }
                        } else {
                            //logMessage("Selected file name is empty.");
                        }
                    }
                    interpretAndExecuteCommands(std::move(modifiedCmds), filePath, specificKey);
                    resetPercentages();
                });
                
				// Set the script key listener (for SCRIPT_KEY)
				toggleListItem->setScriptKeyListener([this, i, _currentPackageHeader = currentPackageHeader](bool state) {
                    // Initialize currentSelectedItem for this index if it does not exist
                    if (isInitialized.find(i) == isInitialized.end() || !isInitialized[i]) {
                        currentSelectedItems[i] = selectedItemsList[i];
                        isInitialized[i] = true;
                    }

					inSelectionMenu = false;
				    // Custom logic for SCRIPT_KEY handling
				    auto modifiedCmds = getSourceReplacement(state ? commandsOn : commandsOff, currentSelectedItems[i], i, filePath);
				    applyPlaceholderReplacementsToCommands(modifiedCmds);
				    tsl::changeTo<ScriptOverlay>(modifiedCmds, filePath, getNameFromPath(selectedItemsList[i]), "selection", false, _currentPackageHeader);
				});

                
                
                list->addItem(toggleListItem.release());
            }
        }
        
        
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

        if (packageHeader.title.empty() || overrideTitle)
            packageHeader.title = packageRootLayerTitle;
        if (packageHeader.version.empty() || overrideVersion)
            packageHeader.version = packageRootLayerVersion;
        if (packageHeader.color.empty())
            packageHeader.color = packageRootLayerColor;
        

        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(
            (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(filePath)),
            !lastPackageHeader.empty() ? lastPackageHeader : (packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + "   (Ultrahand Package)" : "Ultrahand Package"),
            noClickableItems,
            "",
            packageHeader.color);

        if (filePath == PACKAGE_PATH)
            rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);

        rootFrame->setContent(list.release());

        return rootFrame.release();


        //if (filePath == PACKAGE_PATH)
        //    return returnRootFrame(list, CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel);
        //
        //return returnRootFrame(list,
        //    (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(filePath)),
        //    packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + "   (Ultrahand Package)" : "Ultrahand Package",
        //    "",
        //    packageHeader.color);
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

        if (refreshPage && !stillTouching) {
            tsl::goBack();
            tsl::changeTo<SelectionOverlay>(filePath, specificKey, commands, specifiedFooterKey);
            refreshPage = false;
        }

        if (refreshPackage && !stillTouching) {
            tsl::goBack();
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
                allowSlide = unlockedSlide = false;

                inSelectionMenu = false;

                if (filePath == PACKAGE_PATH) {
                    returningToMain = true;
                } else {
                    if (lastPackageMenu == "subPackageMenu") {
                        returningToSubPackage = true;
                    } else {
                        returningToPackage = true;
                    }
                }

                if (commandMode == OPTION_STR && isFileOrDirectory(packageConfigIniPath)) {
                    auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                    auto it = packageConfigData.find(specificKey);
                    if (it != packageConfigData.end()) {
                        auto& optionSection = it->second;
                        auto footerIt = optionSection.find(FOOTER_STR);
                        if (footerIt != optionSection.end() && (footerIt->second.find(NULL_STR) == std::string::npos)) {
                            selectedListItem->setValue(footerIt->second);
                        }
                    }
                }
                tsl::goBack();
                simulatedBackComplete = true;
                return true;
            }
        }

		if (returningToSelectionMenu && !(keysHeld & KEY_B)){
            returningToSelectionMenu = false;
            inSelectionMenu = true;
        }
        

        if (triggerExit.load(std::memory_order_acquire)) {
            triggerExit.store(false, std::memory_order_release);
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
    bool inRelevantSection = false;  // Tracks if we are within the desired section

    bool isFirstSection = true;
    for (const auto& nextOption : options) {
        // Check if this is the start of the relevant section
        if (nextOption.first == dropdownSection) {
            inRelevantSection = true;  // Start gathering commands
            continue;
        }

        // Stop capturing if we encounter a new section with no commands (empty section like [Commands])
        if (inRelevantSection && nextOption.second.empty()) {
            break;  // Stop when encountering an empty section
        }

        // Gather commands if we are in the relevant section
        if (inRelevantSection) {
            // Treat the current section name as a command if it's a section header (i.e., capture it with brackets)
            if (!nextOption.first.empty()) {

                if (!isFirstSection) {
					std::vector<std::string> fillerCommand = {"\u00A0"}; // whitespace
                	promptCommands.push_back(fillerCommand);  // Add the section header as a command
                }
				else if (isFirstSection) {
            		isFirstSection = false;
				}
                std::vector<std::string> sectionCommand = {"[" + nextOption.first + "]"};
                promptCommands.push_back(sectionCommand);  // Add the section header as a command
            }

            // Process and split each command by spaces
            for (const auto& cmd : nextOption.second) {
                std::vector<std::string> fullCmd;
                for (const auto& part : cmd) {
                    auto splitParts = splitString(part, " ");
                    fullCmd.insert(fullCmd.end(), splitParts.begin(), splitParts.end());  // Collect all parts of the command
                }
                promptCommands.push_back(fullCmd);  // Add the full command
            }
        }
    }

    // Return placeholder if no commands are found
    if (promptCommands.empty()) {
        //promptCommands = {{"No", "commands", "for", dropdownSection+"."}};
        promptCommands = {{UNAVAILABLE_SELECTION}};
    }

    return promptCommands;
}




class PackageMenu; // forwarding

// returns if there are or are not cickable items.
bool drawCommandsMenu(std::unique_ptr<tsl::elm::List>& list,
                    const std::string& packageIniPath,
                    const std::string& packageConfigIniPath,
                    const PackageHeader& packageHeader, std::string& pageLeftName, std::string& pageRightName,
                    const std::string& packagePath, const std::string& currentPage, const std::string& packageName, const std::string& dropdownSection, const size_t nestedLayer,
                    std::string& pathPattern, std::string& pathPatternOn, std::string& pathPatternOff, bool& usingPages, const bool packageMenuMode = true) {

    tsl::hlp::ini::IniData packageConfigData;
    std::unique_ptr<tsl::elm::ListItem> listItem;
    auto toggleListItem = std::make_unique<tsl::elm::ToggleListItem>("", true, "", "");
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
    std::string jsonPath, jsonPathOn, jsonPathOff;
    std::string jsonKey, jsonKeyOn, jsonKeyOff;
    
    std::string optionName;
    std::vector<std::vector<std::string>> commands, commandsOn, commandsOff;
    std::vector<std::vector<std::string>> tableData;
    
    std::string itemName, parentDirName;

    //std::vector<std::string> listData, listDataOn, listDataOff;
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

    //bool wasHeader = false;
    //bool lastItemWasHeader = false;
    bool isScrollableTable;
    bool usingTopPivot, usingBottomPivot;
    bool onlyTables = true;
    //bool lastItemIsScrollableTable = false;

    // Pack variables into structs

    // All empty values
    //std::string filepath, iniFilePath;
    //std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
    //std::string jsonString, jsonStringOn, jsonStringOff, listPath, listPathOn, listPathOff, listString, listStringOn, listStringOff, iniPath, iniPathOn, iniPathOff;
    //
    //
    //CommandOptions cmdOptions = {inEristaSection, inMarikoSection, usingErista, usingMariko, commandName, commandSystem,
    //                          commandMode, commandGrouping, currentSection, pathPattern, sourceType, pathPatternOn,
    //                          sourceTypeOn, pathPatternOff, sourceTypeOff, defaultToggleState, hideTableBackground,
    //                          tableStartGap, tableEndGap, tableColumnOffset, tableSpacing, tableSectionTextColor, tableInfoTextColor, tableAlignment,
    //                          minValue, maxValue, units, steps, unlockedTrackbar, onEveryTick, packageSource, packagePath,
    //                          filepath, iniFilePath, filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff,
    //                          jsonString, jsonStringOn, jsonStringOff, jsonKey, jsonKeyOn, jsonKeyOff, jsonPath, jsonPathOn, jsonPathOff, listPath, listPathOn, listPathOff, listString, listStringOn, listStringOff, iniPath, iniPathOn, iniPathOff};
    //
    //CommandData cmdData = {commands, commandsOn, commandsOff, tableData};
    std::vector<std::string> entryList;

    std::string commandNameLower;

    std::string cleanOptionName;

    std::string lastPackageHeader;

    for (size_t i = 0; i < options.size(); ++i) {
        auto& option = options[i];
        
        optionName = option.first;
        commands = std::move(option.second);
        
        footer = "";
        useSelection = false;
        // Table settings
        
        isScrollableTable = false;
        usingTopPivot = false;
        usingBottomPivot = false;
        hideTableBackground = false;
        useHeaderIndent = false;
        tableStartGap = 19;
        tableEndGap = 12;
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
        
        tableData.clear();
        commandsOn.clear();
        commandsOff.clear();
        
        //if (wasHeader)
        //    lastItemWasHeader = true;
        //else
        //    lastItemWasHeader = false;
        //wasHeader = false;

        
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
                        if (i == 0) {
                            // Add a section break with small text to indicate the "Commands" section
                            addHeader(list, COMMANDS);
                            lastPackageHeader = COMMANDS;
                            //wasHeader = true;
                            skipSection = false;
                            lastSection = "Commands";
                        }
                        commandFooter = parseValueFromIniSection(packageConfigIniPath, optionName, FOOTER_STR);
                        // override loading of the command footer
                        if (!commandFooter.empty() && commandFooter != NULL_STR){
                            footer = commandFooter;
                            cleanOptionName = optionName.substr(1);
                            removeTag(cleanOptionName);
                            listItem = std::make_unique<tsl::elm::ListItem>(cleanOptionName);
                            listItem->setValue(footer);
                        } else {
                            footer = DROPDOWN_SYMBOL;
                            cleanOptionName = optionName.substr(1);
                            removeTag(cleanOptionName);
                            // Create reference to PackageMenu with dropdownSection set to optionName
                            listItem = std::make_unique<tsl::elm::ListItem>(cleanOptionName, footer);
                        }
                        
                        if (packageMenuMode) {
                            listItem->setClickListener([packagePath, currentPage, packageName, i, optionName, options, _lastPackageHeader = lastPackageHeader](s64 keys) {
                                
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
                                    tsl::changeTo<PackageMenu>(packagePath, optionName, currentPage, packageName, 0, _lastPackageHeader);
                                    simulatedSelectComplete = true;
                                    
                                    return true;
                                
							    } else if (keys & SCRIPT_KEY) {
							        if (inPackageMenu)
							            inPackageMenu = false;
							        if (inSubPackageMenu)
							            inSubPackageMenu = false;
									
														
							        // Gather the prompt commands for the current dropdown section
							        std::vector<std::vector<std::string>> promptCommands = gatherPromptCommands(optionName, options);
									
														
							        // Pass all gathered commands to the ScriptOverlay
							        tsl::changeTo<ScriptOverlay>(promptCommands, packagePath, optionName, "package", true, _lastPackageHeader);
							        return true;
							    }
                                return false;
                            });
                        } else {
                            listItem->setClickListener([optionName, i, options, _lastPackageHeader = lastPackageHeader](s64 keys) {
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
                            	} else if (keys & SCRIPT_KEY) {
                            	    if (inMainMenu) {
                            	        inMainMenu = false;
                            	    }
                            	    
									
							        // Gather the prompt commands for the current dropdown section
							        std::vector<std::vector<std::string>> promptCommands = gatherPromptCommands(optionName, options);

                            	    tsl::changeTo<ScriptOverlay>(promptCommands, PACKAGE_PATH, optionName, "main", true, _lastPackageHeader);
                            	    return true;
                            	}
                                return false;
                            });
                        }
                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(listItem.release());
                        
                        
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
            //size_t delimiterPos;
            
            // Remove all empty command strings
            //commands.erase(std::remove_if(commands.begin(), commands.end(),
            //    [](const std::vector<std::string>& vec) {
            //        return vec.empty();
            //    }),
            //    commands.end());
            removeEmptyCommands(commands);
            
            // Initial processing of commands (DUPLICATE CODE)
            for (const auto& cmd : commands) {
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
                    } else if (commandName.find(SCROLLABLE_PATTERN) == 0) {
                        isScrollableTable = (commandName.substr(SCROLLABLE_PATTERN.length()) == TRUE_STR);
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
                        tableStartGap = std::stoi(commandName.substr(START_GAP_PATTERN.length()));
                        continue;
                    } else if (commandName.find(END_GAP_PATTERN) == 0) {
                        tableEndGap = std::stoi(commandName.substr(END_GAP_PATTERN.length()));
                        continue;
                    } else if (commandName.find(END_GAP_PATTERN_ALIAS) == 0) {
                        tableEndGap = std::stoi(commandName.substr(END_GAP_PATTERN_ALIAS.length()));
                        continue;
                    } else if (commandName.find(OFFSET_PATTERN) == 0) {
                        tableColumnOffset = std::stoi(commandName.substr(OFFSET_PATTERN.length()));
                        continue;
                    } else if (commandName.find(SPACING_PATTERN) == 0) {
                        tableSpacing = std::stoi(commandName.substr(SPACING_PATTERN.length()));
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
                        minValue = std::stoi(commandName.substr(MIN_VALUE_PATTERN.length()));
                        continue;
                    } else if (commandName.find(MAX_VALUE_PATTERN) == 0) {
                        maxValue = std::stoi(commandName.substr(MAX_VALUE_PATTERN.length()));
                        continue;
                    } else if (commandName.find(UNITS_PATTERN) == 0) {
                        units = commandName.substr(UNITS_PATTERN.length());
                        removeQuotes(units);
                        continue;
                    } else if (commandName.find(STEPS_PATTERN) == 0) {
                        steps = std::stoi(commandName.substr(STEPS_PATTERN.length()));
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

            
            if (isFileOrDirectory(packageConfigIniPath)) {
                packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                
                updateIniData(packageConfigData, packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                updateIniData(packageConfigData, packageConfigIniPath, optionName, MODE_STR, commandMode);
                updateIniData(packageConfigData, packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                updateIniData(packageConfigData, packageConfigIniPath, optionName, FOOTER_STR, commandFooter);
                
                packageConfigData.clear();
            } else { // write default data if settings are not loaded
                setIniFileValue(packageConfigIniPath, optionName, SYSTEM_STR, commandSystem);
                setIniFileValue(packageConfigIniPath, optionName, MODE_STR, commandMode);
                setIniFileValue(packageConfigIniPath, optionName, GROUPING_STR, commandGrouping);
                //setIniFileValue(packageConfigIniPath, optionName, FOOTER_STR, NULL_STR);
            }
            
            
            // Get Option name and footer
            if (optionName.front() == '*') { 
                useSelection = true;
                optionName = optionName.substr(1); // Strip the "*" character on the left
                footer = DROPDOWN_SYMBOL;
            } else {
                pos = optionName.find(" - ");
                if (pos != std::string::npos) {
                    footer = optionName.substr(pos + 2); // Assign the part after " - " as the footer
                    optionName = optionName.substr(0, pos); // Strip the " - " and everything after it
                }
            }
            
            if ((commandMode == OPTION_STR) || (commandMode == SLOT_STR) || (commandMode == TOGGLE_STR && !useSelection)) {
                // override loading of the command footer
                //if (!commandFooter.empty())
                //    footer = commandFooter;
                //else
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
                        tableStartGap = tableEndGap = 19; // for perfect alignment for header tables
                        isScrollableTable = false;
                        lastPackageHeader = getFirstSectionText(tableData, packagePath);
                    }
                    //if (isScrollableTable)
                    //    lastItemIsScrollableTable = true;
                    //else
                    //    lastItemIsScrollableTable = false;

                    if (usingTopPivot) {
                        if (list->getLastIndex() == 0)
                            onlyTables = false;

                        addDummyListItem(list);
                    }

                    addTable(list, tableData, packagePath, tableColumnOffset, tableStartGap, tableEndGap, tableSpacing,
                    	tableSectionTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isScrollableTable, tableWrappingMode, useWrappingIndent);

                    if (usingBottomPivot) {
                        addDummyListItem(list);
                        //lastItemIsScrollableTable = false;
                    }

                    continue;
                } else if (commandMode == TRACKBAR_STR) {
				    onlyTables = false;
				
				    // Create TrackBarV2 instance and configure it
				    auto trackBar = std::make_unique<tsl::elm::TrackBarV2>(optionName, packagePath, minValue, maxValue, units,
				        interpretAndExecuteCommands, getSourceReplacement, commands, option.first, false, false, -1, unlockedTrackbar, onEveryTick);
				
				    // Set the SCRIPT_KEY listener
				    trackBar->setScriptKeyListener([commands, keyName = option.first, packagePath, _lastPackageHeader = lastPackageHeader]() {
				        bool isFromMainMenu = (packagePath == PACKAGE_PATH);
						
				        std::string valueStr = parseValueFromIniSection(packagePath+"config.ini", keyName, "value");
				        std::string indexStr = parseValueFromIniSection(packagePath+"config.ini", keyName, "index");

				        // Handle the commands and placeholders for the trackbar
				        auto modifiedCmds = getSourceReplacement(commands, keyName, std::stoi(indexStr), packagePath);

                        //auto modifiedCmds = getSourceReplacement(commands, valueStr, m_index, m_packagePath);
                        
                        // Placeholder replacement
                        const std::string valuePlaceholder = "{value}";
                        const std::string indexPlaceholder = "{index}";
                        const size_t valuePlaceholderLength = valuePlaceholder.length();
                        const size_t indexPlaceholderLength = indexPlaceholder.length();

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

				        applyPlaceholderReplacementsToCommands(modifiedCmds);
				
				        // Switch to ScriptOverlay
				        tsl::changeTo<ScriptOverlay>(modifiedCmds, packagePath, keyName, isFromMainMenu ? "main" : "package", false, _lastPackageHeader);
				    });
				
				    // Add the TrackBarV2 to the list after setting the necessary listeners
				    list->addItem(trackBar.release());
				
                    continue;
                } else if (commandMode == STEP_TRACKBAR_STR) {
                    if (steps == 0) { // assign minimum steps
                        steps = std::abs(maxValue - minValue) +1;
                    }
                    onlyTables = false;
                    //lastItemIsScrollableTable = false;
                    //list->addItem(new tsl::elm::StepTrackBarV2(optionName, packagePath, steps, minValue, maxValue, units,
                    //	interpretAndExecuteCommands, getSourceReplacement, commands, option.first, false, unlockedTrackbar, onEveryTick));

					
					auto stepTrackBar = std::make_unique<tsl::elm::StepTrackBarV2>(optionName, packagePath, steps, minValue, maxValue, units,
					    interpretAndExecuteCommands, getSourceReplacement, commands, option.first, false, unlockedTrackbar, onEveryTick);
					
					// Set the SCRIPT_KEY listener
					stepTrackBar->setScriptKeyListener([commands, keyName = option.first, packagePath, _lastPackageHeader = lastPackageHeader]() {
					    bool isFromMainMenu = (packagePath == PACKAGE_PATH);
					    
					    // Parse the value and index from the INI file
					    std::string valueStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "value");
					    std::string indexStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "index");
						
						if (!isValidNumber(indexStr))
							indexStr = "0";
					    // Get and modify the commands with the appropriate replacements
					    auto modifiedCmds = getSourceReplacement(commands, keyName, std::stoi(indexStr), packagePath);
						
					    // Placeholder replacement for value and index
					    const std::string valuePlaceholder = "{value}";
					    const std::string indexPlaceholder = "{index}";
					    const size_t valuePlaceholderLength = valuePlaceholder.length();
					    const size_t indexPlaceholderLength = indexPlaceholder.length();
						
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
					    applyPlaceholderReplacementsToCommands(modifiedCmds);
					    tsl::changeTo<ScriptOverlay>(modifiedCmds, packagePath, keyName, isFromMainMenu ? "main" : "package", false, _lastPackageHeader);
					});
					
					// Add the StepTrackBarV2 to the list
					list->addItem(stepTrackBar.release());

                    continue;
                } else if (commandMode == NAMED_STEP_TRACKBAR_STR) {
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
                                populateSelectedItemsList(JSON_STR, jsonString, jsonKey, entryList);
                                break;
                            }
                            else if (cmd[0] == "json_file_source") {
                                std::string jsonPath = cmd[1];
                                preprocessPath(jsonPath, packagePath);
                                std::string jsonKey = cmd[2];
                                removeQuotes(jsonKey);
                                populateSelectedItemsList(JSON_FILE_STR, jsonPath, jsonKey, entryList);
                                break;
                            }
                        }
                        
                        ++it;
                    }
                    onlyTables = false;

                    //lastItemIsScrollableTable = false;
                    //list->addItem(new tsl::elm::NamedStepTrackBarV2(optionName, packagePath, entryList,
                    //	interpretAndExecuteCommands, getSourceReplacement, commands, option.first, unlockedTrackbar, onEveryTick));

					// Create NamedStepTrackBarV2 instance and configure it
					auto namedStepTrackBar = std::make_unique<tsl::elm::NamedStepTrackBarV2>(optionName, packagePath, entryList,
					    interpretAndExecuteCommands, getSourceReplacement, commands, option.first, unlockedTrackbar, onEveryTick);
					
					// Set the SCRIPT_KEY listener
					namedStepTrackBar->setScriptKeyListener([commands, keyName = option.first, packagePath, entryList, _lastPackageHeader = lastPackageHeader]() {
					    bool isFromMainMenu = (packagePath == PACKAGE_PATH);
					
					    // Parse the value and index from the INI file
					    std::string valueStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "value");
					    std::string indexStr = parseValueFromIniSection(packagePath + "config.ini", keyName, "index");
					
					    // Fallback if indexStr is not a valid number
					    if (!isValidNumber(indexStr))
					        indexStr = "0";
					
					    // Ensure the index is within the bounds of the entryList
					    size_t entryIndex = std::min(static_cast<size_t>(std::stoi(indexStr)), entryList.size() - 1);
					    valueStr = entryList[entryIndex];  // Update valueStr based on the current entry in the list
					
					    // Get and modify the commands with the appropriate replacements
					    auto modifiedCmds = getSourceReplacement(commands, keyName, entryIndex, packagePath);
					
					    // Placeholder replacement for value and index
					    const std::string valuePlaceholder = "{value}";
					    const std::string indexPlaceholder = "{index}";
					    const size_t valuePlaceholderLength = valuePlaceholder.length();
					    const size_t indexPlaceholderLength = indexPlaceholder.length();
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
					    applyPlaceholderReplacementsToCommands(modifiedCmds);
					    tsl::changeTo<ScriptOverlay>(modifiedCmds, packagePath, keyName, isFromMainMenu ? "main" : "package", false, _lastPackageHeader);
					});
					
					// Add the NamedStepTrackBarV2 to the list
					list->addItem(namedStepTrackBar.release());

                    continue;
                }
                if (useSelection) { // For wildcard commands (dropdown menus)

                    if ((footer == DROPDOWN_SYMBOL) || (footer.empty()) || footer == commandFooter) {
                        //if (!commandFooter.empty())
                        //    footer = commandFooter;
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        listItem = std::make_unique<tsl::elm::ListItem>(cleanOptionName, footer);
                    }
                    else {
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        listItem = std::make_unique<tsl::elm::ListItem>(cleanOptionName);

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
                        listItem->setClickListener([commands, keyName = option.first, dropdownSection, packagePath,
                        	forwarderPackagePath, forwarderPackageIniName, _lastPackageHeader = lastPackageHeader](s64 keys) mutable {
                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }
                            
                            if (keys & KEY_A) {
                                //auto commandsCopy = commands;
                                //interpretAndExecuteCommands(std::move(commandsCopy), packagePath, keyName); // Now correctly moved
                                //interpretAndExecuteCommands(getSourceReplacement(commands, keyName, i, packagePath), packagePath, keyName); // Now correctly moved
                                interpretAndExecuteCommands(std::move(std::vector<std::vector<std::string>>(commands)), packagePath, keyName);
                                resetPercentages();

                                nestedMenuCount++;
                                lastPackagePath = forwarderPackagePath;
                                lastPackageName = forwarderPackageIniName;
                                if (dropdownSection.empty())
                                    lastPackageMenu = "packageMenu";
                                else
                                    lastPackageMenu = "subPackageMenu";
                                allowSlide = unlockedSlide = false;
                                tsl::changeTo<PackageMenu>(forwarderPackagePath, "", LEFT_STR, forwarderPackageIniName, nestedMenuCount, _lastPackageHeader);
                                simulatedSelectComplete = true;
                                return true;
                            } else if (keys & SCRIPT_KEY) {
                                bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inMainMenu) {
                                //    isFromMainMenu = true;
                                //    inMainMenu = false;
                                //}
                                if (inPackageMenu) {
                                    inPackageMenu = false;
                                    lastMenu = "packageMenu";
                                }
                                if (inSubPackageMenu) {
                                    inSubPackageMenu = false;
                                    lastMenu = "subPackageMenu";
                                }

                                //auto modifiedCmds = commands;//getSourceReplacement(commands, keyName, i, packagePath);
                                //applyPlaceholderReplacementsToCommands(modifiedCmds);
                                std::string selectionItem = keyName;
                                removeTag(selectionItem);
                                // add lines ;mode=forwarder and package_source 'forwarderPackagePath' to front of modifiedCmds
                                tsl::changeTo<ScriptOverlay>(commands, packagePath, selectionItem, isFromMainMenu ? "main" : "package", true, _lastPackageHeader);
                                return true;
                            }
                            return false;
                        });
                    } else {
                        //if (!commandFooter.empty()) {
                        //    listItem->setValue(commandFooter, true);
                        //    //listItem = std::make_unique<tsl::elm::ListItem>(removeTag(optionName), footer);
                        //}
                        //listItem->setValue("TEST", true);
                        //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                        listItem->setClickListener([commands, keyName = option.first, dropdownSection, packagePath, packageName,
                        	footer, lastSection, listItemRaw = listItem.get(), _lastPackageHeader = lastPackageHeader](uint64_t keys) {
                            //listItemPtr = std::shared_ptr<tsl::elm::ListItem>(listItem.get(), [](auto*){})](uint64_t keys) {
                            
                            if (runningInterpreter.load(std::memory_order_acquire))
                                return false;
                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
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
                                    
                                    selectedListItem.reset();
                                    selectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                                    
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
                                    tsl::changeTo<SelectionOverlay>(packagePath, keyName, commands, newKey, _lastPackageHeader);
                                    lastKeyName = keyName;
                                }
                                simulatedSelectComplete = true;
                                return true;
                            } else if (keys & SCRIPT_KEY) {
                                bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inMainMenu) {
                                //    isFromMainMenu = true;
                                //    inMainMenu = false;
                                //}
                                if (inPackageMenu) {
                                    inPackageMenu = false;
                                    lastMenu = "packageMenu";
                                }
                                if (inSubPackageMenu) {
                                    inSubPackageMenu = false;
                                    lastMenu = "subPackageMenu";
                                }

                                //auto modifiedCmds = commands;//getSourceReplacement(commands, keyName, i, packagePath);
                                //applyPlaceholderReplacementsToCommands(modifiedCmds);
                                std::string selectionItem = keyName;
                                removeTag(selectionItem);
                                tsl::changeTo<ScriptOverlay>(commands, packagePath, selectionItem, isFromMainMenu ? "main" : "package", true, _lastPackageHeader);
                                return true;
                            }
                            return false;
                        });
                    }
                    onlyTables = false;
                    //lastItemIsScrollableTable = false;
                    list->addItem(listItem.release());
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
                        listItem = std::make_unique<tsl::elm::ListItem>(cleanOptionName);
                        if (commandMode == DEFAULT_STR)
                            listItem->setValue(footer, true);
                        else
                            listItem->setValue(footer);
                        
                        
                        listItem->setClickListener([i, commands, keyName = option.first, packagePath, packageName,
                        	selectedItem, listItemRaw = listItem.get(), _lastPackageHeader = lastPackageHeader](uint64_t keys) {
                            
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
                                startInterpreterThread(packagePath);
                                listItemRaw->setValue(INPROGRESS_SYMBOL);
                                
                                lastSelectedListItem.reset();
                                lastSelectedListItem = std::shared_ptr<tsl::elm::ListItem>(listItemRaw, [](auto*) {});
                                shiftItemFocus(listItemRaw);
                                
                                lastRunningInterpreter = true;
                                simulatedSelectComplete = true;
                                lastSelectedListItem->triggerClickAnimation();
                                return true;
                            }  else if (keys & SCRIPT_KEY) {
                                bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                                //if (inMainMenu) {
                                //    isFromMainMenu = true;
                                //    inMainMenu = false;
                                //}
                                if (inPackageMenu) {
                                    inPackageMenu = false;
                                    lastMenu = "packageMenu";
                                }
                                if (inSubPackageMenu) {
                                    inSubPackageMenu = false;
                                    lastMenu = "subPackageMenu";
                                }
                                auto modifiedCmds = getSourceReplacement(commands, selectedItem, i, packagePath);
                                applyPlaceholderReplacementsToCommands(modifiedCmds);
                                tsl::changeTo<ScriptOverlay>(modifiedCmds, packagePath, keyName, isFromMainMenu ? "main" : "package", false, _lastPackageHeader);
                                return true;
                            }
                            return false;
                        });
                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(listItem.release());
                    } else if (commandMode == TOGGLE_STR) {
                        cleanOptionName = optionName;
                        removeTag(cleanOptionName);
                        toggleListItem = std::make_unique<tsl::elm::ToggleListItem>(cleanOptionName, false, ON, OFF);
                        // Preprocess pathPatternOn and pathPatternOff separately
                        //preprocessPath(pathPatternOn, packagePath);
                        //preprocessPath(pathPatternOff, packagePath);

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
                        
                        toggleListItem->setStateChangedListener([i, commandsOn, commandsOff, keyName = option.first, packagePath,
                            pathPatternOn, pathPatternOff, listItemRaw = toggleListItem.get()](bool state) {
                            
                            tsl::Overlay::get()->getCurrentGui()->requestFocus(listItemRaw, tsl::FocusDirection::None);
                            
                            // Now pass the preprocessed paths to getSourceReplacement
                            interpretAndExecuteCommands(state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                                getSourceReplacement(commandsOff, pathPatternOff, i, packagePath), packagePath, keyName);
                            
                            resetPercentages();
                            // Set the ini file value after executing the command
                            setIniFileValue((packagePath + CONFIG_FILENAME), keyName, FOOTER_STR, state ? CAPITAL_ON_STR : CAPITAL_OFF_STR);
                            
                        });

						// Set the script key listener (for SCRIPT_KEY)
						toggleListItem->setScriptKeyListener([i, commandsOn, commandsOff, keyName = option.first, packagePath,
                            pathPatternOn, pathPatternOff, _lastPackageHeader = lastPackageHeader](bool state) {

                            bool isFromMainMenu = (packagePath == PACKAGE_PATH);
                            if (inPackageMenu)
                                inPackageMenu = false;
                            if (inSubPackageMenu)
                                inSubPackageMenu = false;

						    // Custom logic for SCRIPT_KEY handling
						    auto modifiedCmds = state ? getSourceReplacement(commandsOn, pathPatternOn, i, packagePath) :
                                getSourceReplacement(commandsOff, pathPatternOff, i, packagePath);

						    applyPlaceholderReplacementsToCommands(modifiedCmds);
						    tsl::changeTo<ScriptOverlay>(modifiedCmds, packagePath, keyName, isFromMainMenu ? "main" : "package", false, _lastPackageHeader);
						});


                        onlyTables = false;
                        //lastItemIsScrollableTable = false;
                        list->addItem(toggleListItem.release());
                    }
                }
            }
        }
    }
    if (onlyTables) {
        //auto dummyItem = new tsl::elm::DummyListItem();
        //list->addItem(dummyItem, 0, 1);
        addDummyListItem(list, 1);
    }

    //if (lastItemIsScrollableTable) {
    //    //auto dummyItem = new tsl::elm::DummyListItem();
    //    //list->addItem(dummyItem);
    //    addDummyListItem(list);
    //}

    listItem.release();
    toggleListItem.release();
    options.clear();
    commands.clear();
    commandsOn.clear();
    commandsOff.clear();
    tableData.clear();
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

public:
    /**
     * @brief Constructs a `PackageMenu` instance for a specific sub-menu path.
     *
     * Initializes a new instance of the `PackageMenu` class for the given sub-menu path.
     *
     * @param path The path to the sub-menu.
     */
    PackageMenu(const std::string& path, const std::string& sectionName = "", const std::string& page = LEFT_STR, const std::string& _packageName = PACKAGE_FILENAME, const size_t _nestedlayer = 0, const std::string& _pageHeader = "") :
        packagePath(path), dropdownSection(sectionName), currentPage(page), packageName(_packageName), nestedLayer(_nestedlayer), pageHeader(_pageHeader) {}
    /**
     * @brief Destroys the `PackageMenu` instance.
     *
     * Cleans up any resources associated with the `PackageMenu` instance.
     */
    ~PackageMenu() {
        if (returningToMain) {
            clearMemory();
            packageRootLayerTitle = "";
            packageRootLayerVersion = "";
            packageRootLayerColor = "";
            overrideTitle = false;
            overrideVersion = false;

            if (isFileOrDirectory(packagePath + EXIT_PACKAGE_FILENAME)) {
                bool useExitPackage = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_EXIT_PACKAGE_STR) == FALSE_STR);
                
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
        if (dropdownSection.empty()){
            inPackageMenu = true;
            lastMenu = "packageMenu";
        } else {
            inSubPackageMenu = true;
            lastMenu = "subPackageMenu";
        }
        
        auto list = std::make_unique<tsl::elm::List>();

        std::string packageIniPath = packagePath + packageName;
        std::string packageConfigIniPath = packagePath + CONFIG_FILENAME;

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
        
        std::unique_ptr<tsl::elm::OverlayFrame> rootFrame = std::make_unique<tsl::elm::OverlayFrame>(
            (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(packagePath)),
            (!pageHeader.empty() ? pageHeader: (packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + "   (Ultrahand Package)" : "Ultrahand Package")),
            noClickableItems,
            "",
            packageHeader.color,
            (usingPages && currentPage == RIGHT_STR) ? pageLeftName : "",
            (usingPages && currentPage == LEFT_STR) ? pageRightName : ""
        );
        
        rootFrame->setContent(list.release());
        
        return rootFrame.release();


        //return returnRootFrame(list,
        //    (!packageHeader.title.empty()) ? packageHeader.title : (!packageRootLayerTitle.empty() ? packageRootLayerTitle : getNameFromPath(packagePath)),
        //    packageHeader.version != "" ? (!packageRootLayerVersion.empty() ? packageRootLayerVersion : packageHeader.version) + "   (Ultrahand Package)" : "Ultrahand Package",
        //    "",
        //    packageHeader.color,
        //    (usingPages && currentPage == RIGHT_STR) ? pageLeftName : "",
        //    (usingPages && currentPage == LEFT_STR) ? pageRightName : ""
        //);
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
        
        if (!returningToPackage && !stillTouching) {
            if (refreshPage) {
                refreshPage = false;
                
                // Function to handle the transition and state resetting
                auto handleMenuTransition = [&] {
                    lastPackagePath = packagePath;
                    std::string lastDropdownSection = dropdownSection;
                    lastPage = currentPage;
                    std::string lastPackageName = packageName;
                    size_t lastNestedLayer = nestedLayer;
                    //lastPageHeader = pageHeader;
                    
                    inSubPackageMenu = false;
                    inPackageMenu = false;
                    tsl::goBack();
                
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::changeTo<PackageMenu>(lastPackagePath, lastDropdownSection, lastPage, lastPackageName, lastNestedLayer, pageHeader);
                };
                
                if (inPackageMenu) {
                    handleMenuTransition();
                    inPackageMenu = true;
                } 
                else if (inSubPackageMenu) {
                    handleMenuTransition();
                    inSubPackageMenu = true;
                }
            }
            if (refreshPackage) {
                if (nestedMenuCount == nestedLayer) {
                    lastPackagePath = packagePath;
                    lastPage = currentPage;
                    lastPackageName = PACKAGE_FILENAME;
                    
                    while (nestedMenuCount > 0) {
                        tsl::goBack();
                        nestedMenuCount--;
                    }

                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackagePath, "");
                    inPackageMenu = true;
                    inSubPackageMenu = false;
                    refreshPackage = false;

                }
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
                    //simulatedNextPage = false;
                }
                else if (currentPage == RIGHT_STR) {
                    keysHeld |= KEY_DLEFT;
                    //simulatedNextPage = false;
                }
                else {
                    //simulatedNextPage = false;
                    simulatedNextPageComplete = true;
                }
            }
            if (currentPage == LEFT_STR) {
                if ((keysHeld & KEY_RIGHT) && !(keysHeld & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK) && !stillTouching && (((!allowSlide && onTrackBar && !unlockedSlide) || (keysHeld & KEY_R)) || !onTrackBar || simulatedNextPage)) {
                    simulatedNextPage = false;
                    allowSlide = unlockedSlide = false;
                    lastPage = RIGHT_STR;
                    //lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackagePath, dropdownSection, RIGHT_STR, lastPackageName, nestedMenuCount, pageHeader);
                    simulatedNextPageComplete = true;
                    return true;
                }
            } else if (currentPage == RIGHT_STR) {
                if ((keysHeld & KEY_LEFT) && !(keysHeld & ~KEY_LEFT & ~KEY_R & ALL_KEYS_MASK) && !stillTouching && (((!allowSlide && onTrackBar && !unlockedSlide) || (keysHeld & KEY_R)) || !onTrackBar || simulatedNextPage)) {
                    simulatedNextPage = false;
                    allowSlide = unlockedSlide = false;
                    lastPage = LEFT_STR;
                    //lastPackage = packagePath;
                    selectedListItem.reset();
                    lastSelectedListItem.reset();
                    tsl::goBack();
                    tsl::changeTo<PackageMenu>(lastPackagePath, dropdownSection, LEFT_STR, lastPackageName, nestedMenuCount, pageHeader);
                    simulatedNextPageComplete = true;
                    return true;
                }
            } 
            simulatedNextPage = false;
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
                    allowSlide = unlockedSlide = false;
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
                    allowSlide = unlockedSlide = false;
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
                    allowSlide = unlockedSlide = false;
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
                    allowSlide = unlockedSlide = false;
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
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
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
    MainMenu(const std::string& hiddenMenuMode = "", const std::string& sectionName = "") : hiddenMenuMode(hiddenMenuMode), dropdownSection(sectionName) {
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
        menuMode = OVERLAYS_STR;

        if (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR) == TRUE_STR) {
            inMainMenu = false;
            inHiddenMode = true;
            hiddenMenuMode = OVERLAYS_STR;
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, FALSE_STR);
        }

        if (!inHiddenMode && dropdownSection.empty())
            inMainMenu = true;
        else
            inMainMenu = false;
        
        //tsl::hlp::ini::IniData settingsData, packageConfigData;
        std::string packagePath, pathReplace, pathReplaceOn, pathReplaceOff;
        std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
        std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
        
        bool noClickableItems = false;

        bool toPackages = false;
        //bool skipSystem = false;
        lastMenuMode = hiddenMenuMode;
        
        
        
        createDirectory(PACKAGE_PATH);
        createDirectory(SETTINGS_PATH);
        
        bool settingsLoaded = false;
        
        auto setDefaultValue = [](const auto& ultrahandSection, const std::string& section, const std::string& defaultValue, bool& settingFlag) {
            if (ultrahandSection.count(section) > 0) {
                settingFlag = (ultrahandSection.at(section) == TRUE_STR);
            } else {
                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, section, defaultValue);
                settingFlag = (defaultValue == TRUE_STR);
            }
        };
        
        auto setDefaultStrValue = [](const auto& ultrahandSection, const std::string& section, const std::string& defaultValue, std::string& settingValue) {
            if (ultrahandSection.count(section) > 0) {
                settingValue = ultrahandSection.at(section);
            } else {
                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, section, defaultValue);
            }
        };
        
        if (isFileOrDirectory(ULTRAHAND_CONFIG_INI_PATH)) {
            // Load key-value pairs from the "ULTRAHAND_PROJECT_NAME" section of the INI file
            auto ultrahandSection = getKeyValuePairsFromSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME);
            
            if (!ultrahandSection.empty()) {
                // Set default values for various settings
                setDefaultValue(ultrahandSection, "hide_user_guide", FALSE_STR, hideUserGuide);
                setDefaultValue(ultrahandSection, "clean_version_labels", FALSE_STR, cleanVersionLabels);
                setDefaultValue(ultrahandSection, "hide_overlay_versions", FALSE_STR, hideOverlayVersions);
                setDefaultValue(ultrahandSection, "hide_package_versions", FALSE_STR, hidePackageVersions);
                setDefaultValue(ultrahandSection, "memory_expansion", FALSE_STR, useMemoryExpansion);
                // setDefaultValue(ultrahandSection, "custom_wallpaper", FALSE_STR, useCustomWallpaper);
                setDefaultValue(ultrahandSection, "swipe_to_open", TRUE_STR, useSwipeToOpen);
                setDefaultValue(ultrahandSection, "right_alignment", FALSE_STR, useRightAlignment);
                setDefaultValue(ultrahandSection, "opaque_screenshots", TRUE_STR, useOpaqueScreenshots);
                //setDefaultValue(ultrahandSection, "progress_animation", FALSE_STR, progressAnimation);
                
                setDefaultStrValue(ultrahandSection, DEFAULT_LANG_STR, defaultLang, defaultLang);
            
                // Ensure certain settings are set in the INI file if they don't exist
                if (ultrahandSection.count("datetime_format") == 0) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "datetime_format", DEFAULT_DT_FORMAT);
                }
            
                if (ultrahandSection.count("hide_clock") == 0) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_clock", FALSE_STR);
                }
            
                if (ultrahandSection.count("hide_battery") == 0) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_battery", TRUE_STR);
                }
            
                if (ultrahandSection.count("hide_pcb_temp") == 0) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_pcb_temp", TRUE_STR);
                }
            
                if (ultrahandSection.count("hide_soc_temp") == 0) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "hide_soc_temp", TRUE_STR);
                }

                if (ultrahandSection.count("overscan") == 0) {
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "overscan", "100");
                }
            
                // Handle the 'to_packages' option if it exists
                if (ultrahandSection.count("to_packages") > 0) {
                    trim(ultrahandSection["to_packages"]);
                    toPackages = (ultrahandSection["to_packages"] == TRUE_STR);
                }
            
                // Mark settings as loaded if the "in_overlay" setting exists
                settingsLoaded = ultrahandSection.count(IN_OVERLAY_STR) > 0;
            }

        } else {
            updateMenuCombos = true;
        }

        
        if (!settingsLoaded) { // Write data if settings are not loaded
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR, defaultLang);
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, FALSE_STR);
            initializingSpawn = true;
        }
        
        
        std::string langFile = LANG_PATH+defaultLang+".json";
        if (isFileOrDirectory(langFile))
            parseLanguage(langFile);
        else {
        	if (defaultLang == "en")
            	reinitializeLangVars();
        }
        
        // write default theme
        initializeTheme();
        copyTeslaKeyComboToUltrahand();
        
        if (toPackages) {
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "to_packages", FALSE_STR); // this is handled within tesla.hpp
            currentMenu = PACKAGES_STR;
        }

        menuMode = currentMenu;
        
        versionLabel = std::string(APP_VERSION) + "   (" + loaderTitle + " " + (cleanVersionLabels ? "" : "v") + cleanVersionLabel(loaderInfo) + ")";
        //versionLabel = (cleanVersionLabels) ? std::string(APP_VERSION) : (std::string(APP_VERSION) + "   (" + extractTitle(loaderInfo) + " v" + cleanVersionLabel(loaderInfo) + ")");
        
        auto list = std::make_unique<tsl::elm::List>();
        //list = std::make_unique<tsl::elm::List>();

        std::unique_ptr<tsl::elm::ListItem> listItem;
        
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
            
            
            // Check if the overlays INI file exists
            std::ifstream overlaysIniFile(OVERLAYS_INI_FILEPATH);
            if (!overlaysIniFile.is_open()) {
                // The INI file doesn't exist, so create an empty one.
                std::ofstream createFile(OVERLAYS_INI_FILEPATH);
                //if (createFile.is_open()) {
                initializingSpawn = true;
                createFile.close(); // Close the file after creating it
                //}
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

                std::string assignedOverlayName, assignedOverlayVersion;

                auto it = overlaysIniData.end();
                // Assuming the existence of appropriate utility functions and types are defined elsewhere.
                for (const auto& overlayFile : overlayFiles) {
                    const std::string& overlayFileName = getNameFromPath(overlayFile);
                    
                    //if (overlayFileName == "ovlmenu.ovl" || overlayFileName.front() == '.') {
                    //    continue;
                    //}
                    
                    it = overlaysIniData.find(overlayFileName);
                    if (it == overlaysIniData.end()) {
                        // Initialization of new entries
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, PRIORITY_STR, "20");
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, STAR_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, HIDE_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, USE_LAUNCH_ARGS_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, LAUNCH_ARGS_STR, "");
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, "custom_name", "");
                        setIniFileValue(OVERLAYS_INI_FILEPATH, overlayFileName, "custom_version", "");
                        const auto& [result, overlayName, overlayVersion] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                        if (result != ResultSuccess) continue;

					    // Use retrieved overlay info
					    assignedOverlayName = overlayName;
					    assignedOverlayVersion = overlayVersion;
					
					    const std::string& baseOverlayInfo = "0020" + assignedOverlayName + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + overlayFileName;
					    overlayList.insert(baseOverlayInfo);
                        //overlayList.insert("0020"+(overlayName)+":" + overlayFileName);
                    } else {
                        const std::string& priority = getValueOrDefault(it->second, PRIORITY_STR, "20", formatPriorityString, 1);
                        const std::string& starred = getValueOrDefault(it->second, STAR_STR, FALSE_STR);
                        const std::string& hide = getValueOrDefault(it->second, HIDE_STR, FALSE_STR);
                        const std::string& useLaunchArgs = getValueOrDefault(it->second, USE_LAUNCH_ARGS_STR, FALSE_STR);
                        const std::string& launchArgs = getValueOrDefault(it->second, LAUNCH_ARGS_STR, "");
                        const std::string& customName = getValueOrDefault(it->second, "custom_name", "");
                        const std::string& customVersion = getValueOrDefault(it->second, "custom_version", "");
                        
                        

                        const auto& [result, overlayName, overlayVersion] = getOverlayInfo(OVERLAY_PATH + overlayFileName);
                        if (result != ResultSuccess) continue;

                        if (!customName.empty()){
                            assignedOverlayName = customName;
                        } else
                            assignedOverlayName = overlayName;

                        if (!customVersion.empty()){
                            assignedOverlayVersion = customVersion;
                        } else
                            assignedOverlayVersion = overlayVersion;
                        
                        const std::string& baseOverlayInfo = priority + (assignedOverlayName) + ":" + assignedOverlayName + ":" + assignedOverlayVersion + ":" + overlayFileName;
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
                    
                    newOverlayName = overlayName;
                    if (overlayStarred)
                        newOverlayName = STAR_SYMBOL+"  "+newOverlayName;
                    
                    
                    // Toggle the starred status
                    newStarred = !overlayStarred;
                    
                    
                    //logMessage(overlayFile);
                    if (isFileOrDirectory(overlayFile)) {
                        listItem = std::make_unique<tsl::elm::ListItem>(newOverlayName);
                        overlayVersion = getFirstLongEntry(overlayVersion);
                        if (cleanVersionLabels)
                            overlayVersion = cleanVersionLabel(overlayVersion);
                        if (!hideOverlayVersions)
                            listItem->setValue(overlayVersion, true);
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([this, overlayFile, newStarred, overlayFileName, overlayName](s64 keys) {
                            
                            if (runningInterpreter.load(std::memory_order_acquire))
                                return false;
                            

                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }

                            if (keys & KEY_A) {
                                
                                
                                std::string useOverlayLaunchArgs = parseValueFromIniSection(OVERLAYS_INI_FILEPATH, overlayFileName, USE_LAUNCH_ARGS_STR);
                                std::string overlayLaunchArgs = parseValueFromIniSection(OVERLAYS_INI_FILEPATH, overlayFileName, LAUNCH_ARGS_STR);
                                removeQuotes(overlayLaunchArgs);
                                
                                if (inHiddenMode) {
                                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, TRUE_STR);
                                }
                                
                                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
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
            if (!isFileOrDirectory(PACKAGE_PATH+PACKAGE_FILENAME)) {
                std::ofstream packageFileOut(PACKAGE_PATH+PACKAGE_FILENAME);
                if (packageFileOut) {
                    packageFileOut <<
                        "[*Reboot To]\n"
                        "[*Boot Entry]\n"
                        "ini_file_source /bootloader/hekate_ipl.ini\n"
                        "filter config\n"
                        "reboot boot '{ini_file_source(*)}'\n"
                        "[hekate]\n"
                        "reboot HEKATE\n"
                        "[hekate UMS]\n"
                        "reboot UMS\n"
                        "\n[Commands]\n"
                        "[Shutdown]\n"
                        "shutdown\n";
                    packageFileOut.close();
                }
            }
            inOverlaysPage = false;
            inPackagesPage = true;

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

                PackageHeader packageHeader;

                std::string assignedPackageName, assignedPackageVersion;

                auto packageIt = packagesIniData.end();
                for (const auto& packageName: subdirectories) {
                    packageIt = packagesIniData.find(packageName);
                    if (packageIt == packagesIniData.end()) {
                        // Initialize missing package data
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, PRIORITY_STR, "20");
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, STAR_STR, FALSE_STR);
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, HIDE_STR, FALSE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, packageName, USE_BOOT_PACKAGE_STR, TRUE_STR);
                        setIniFileValue(OVERLAYS_INI_FILEPATH, packageName, USE_EXIT_PACKAGE_STR, TRUE_STR);
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, "custom_name", "");
                        setIniFileValue(PACKAGES_INI_FILEPATH, packageName, "custom_version", "");

                        assignedPackageName = packageHeader.title;
                        assignedPackageVersion = packageHeader.version;

                        const std::string& basePackageInfo = priority + ":" + assignedPackageName + ":" + assignedPackageVersion + ":" + packageName;
                        packageList.insert(basePackageInfo);

                        //packageList.insert("0020" + (packageName) +":" + packageName);
                    } else {
                        // Process existing package data
                        priority = (packageIt->second.find(PRIORITY_STR) != packageIt->second.end()) ? 
                                    formatPriorityString(packageIt->second[PRIORITY_STR]) : "0020";
                        starred = (packageIt->second.find(STAR_STR) != packageIt->second.end()) ? 
                                  packageIt->second[STAR_STR] : FALSE_STR;
                        hide = (packageIt->second.find(HIDE_STR) != packageIt->second.end()) ? 
                               packageIt->second[HIDE_STR] : FALSE_STR;
                        
                        
                        const std::string& customName = getValueOrDefault(packageIt->second, "custom_name", "");
                        const std::string& customVersion = getValueOrDefault(packageIt->second, "custom_version", "");

                        packageHeader = getPackageHeaderFromIni(PACKAGE_PATH + packageName+ "/" +PACKAGE_FILENAME);
                        
                        if (cleanVersionLabels) {
                            packageHeader.version = cleanVersionLabel(packageHeader.version);
                            removeQuotes(packageHeader.version);
                        }
                        
                        //packageHeader.clear(); // free memory

                        assignedPackageName = assignedPackageVersion = "";

                        if (!customName.empty()){
                            assignedPackageName = customName;
                        } else {
                            if (packageHeader.title.empty())
                                assignedPackageName = packageName;
                            else
                                assignedPackageName = packageHeader.title;
                        }

                        if (!customVersion.empty()){
                            assignedPackageVersion = customVersion;
                        } else
                            assignedPackageVersion = packageHeader.version;

                        const std::string& basePackageInfo = priority + ":" + assignedPackageName + ":" + assignedPackageVersion + ":" + packageName;
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
                
                if (inHiddenMode) {
                    packageList = hiddenPackageList;
                    hiddenPackageList.clear();
                }
                
                std::string taintedPackageName;
                std::string packageName, packageVersion;
                bool packageStarred;
                std::string newPackageName;
                std::string packageFilePath;
                bool newStarred;
                
                
                size_t lastColonPos, secondLastColonPos, thirdLastColonPos;

                std::string tempPackageName;

                bool firstItem = true;
                for (const auto& taintedPackageName : packageList) {
                    if (firstItem) {
                        addHeader(list, !inHiddenMode ? PACKAGES : HIDDEN_PACKAGES);
                        firstItem = false;
                    }

                    
                    // packageName = taintedPackageName.c_str();
                    tempPackageName = taintedPackageName;

                    packageStarred = false;
                    // Check if the package is starred
                    if (tempPackageName.length() >= 3 && tempPackageName.substr(0, 3) == "-1:") {
                        packageStarred = true;
                        // Remove the "-1:" prefix
                        tempPackageName = tempPackageName.substr(3);
                    }
                    
                    // Find the position of the last colon
                    lastColonPos = tempPackageName.rfind(':');
                    if (lastColonPos != std::string::npos) {
                        // Extract the version part after the last colon
                        packageName = tempPackageName.substr(lastColonPos + 1);
                
                        // Remove the version part from tempPackageName
                        tempPackageName = tempPackageName.substr(0, lastColonPos);
                
                        // Now, find the position of the second-to-last colon
                        secondLastColonPos = tempPackageName.rfind(':');
                        if (secondLastColonPos != std::string::npos) {
                            // Extract the name part between the two colons
                            packageVersion = tempPackageName.substr(secondLastColonPos + 1);

                            // Remove the version part from tempPackageName
                            tempPackageName = tempPackageName.substr(0, secondLastColonPos);

                            // Now, find the position of the second-to-last colon
                            thirdLastColonPos = tempPackageName.rfind(':');
                            if (thirdLastColonPos != std::string::npos) {
                                newPackageName = tempPackageName.substr(thirdLastColonPos + 1);
                            }
                        }
                    }

                    
                    //packageName = packageName.substr(5);
                    
                    //newPackageName = (packageStarred) ? (STAR_SYMBOL + "  " + newPackageName) : newPackageName;
                    
                    packageFilePath = PACKAGE_PATH + packageName+ "/";
                    
                    // Toggle the starred status
                    newStarred = !packageStarred;
                    
                    
                    if (isFileOrDirectory(packageFilePath)) {
                        //packageHeader = getPackageHeaderFromIni(packageFilePath+PACKAGE_FILENAME);
                        
                        listItem = std::make_unique<tsl::elm::ListItem>((packageStarred) ? (STAR_SYMBOL + "  " + newPackageName) : newPackageName);
                        //if (cleanVersionLabels)
                        //    packageHeader.version = removeQuotes(cleanVersionLabel(packageHeader.version));
                        if (!hidePackageVersions)
                           listItem->setValue(packageVersion, true);
                        
                        //packageHeader.clear(); // free memory
                        
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([this, packageFilePath, newStarred, packageName, newPackageName, packageVersion](s64 keys) {
                            if (runningInterpreter.load(std::memory_order_acquire)) {
                                return false;
                            }
                            
                            if (simulatedSelect && !simulatedSelectComplete) {
                                keys |= KEY_A;
                                simulatedSelect = false;
                            }
                            
                            if (keys & KEY_A) {
                                inMainMenu = false;
                                

                                if (isFileOrDirectory(packageFilePath + BOOT_PACKAGE_FILENAME)) {
                                    bool useBootPackage = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, packageName, USE_BOOT_PACKAGE_STR) == FALSE_STR);

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
                                refreshPage = true;

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
                                
                                tsl::changeTo<SettingsMenu>(packageName, PACKAGE_STR, "", newPackageName);
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
                packagePath = PACKAGE_PATH;
                std::string packageName = "package.ini";
                std::string pageLeftName = "";
                std::string pageRightName = "";
                std::string currentPage = "left";
                size_t nestedLayer = 0;
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
        if (initializingSpawn) {
            
            initializingSpawn = false;
            list.reset();
            return createUI(); 
        }
        
        filesList.clear();

        //tsl::elm::OverlayFrame *rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel, menuMode+hiddenMenuMode+dropdownSection);
        auto rootFrame = std::make_unique<tsl::elm::OverlayFrame>(CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel, noClickableItems, menuMode+hiddenMenuMode+dropdownSection, "", "", "");
        
        rootFrame->setContent(list.release());
        
        return rootFrame.release();

        //return returnRootFrame(list, CAPITAL_ULTRAHAND_PROJECT_NAME, versionLabel, menuMode+hiddenMenuMode+dropdownSection);
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


        if (refreshPage && !stillTouching) {
            refreshPage = false;
            tsl::pop();
            tsl::changeTo<MainMenu>(hiddenMenuMode, dropdownSection);
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
                allowSlide = unlockedSlide = false;
                returningToMain = true;
                tsl::goBack();
                simulatedBackComplete = true;
                return true;
            }
        }
        
        if (inMainMenu && !inHiddenMode && dropdownSection.empty()){
            if (triggerMenuReload) { // for handling software updates
                triggerMenuReload = false;
                if (menuMode == PACKAGES_STR)
                    setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "to_packages", TRUE_STR);
                
                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR);
                tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl", "--skipCombo");
                
                tsl::Overlay::get()->close();
            }
            
            if (!freshSpawn && !returningToMain && !returningToHiddenMain) {
                
                if (simulatedNextPage && !simulatedNextPageComplete) {
                    if (menuMode != PACKAGES_STR) {
                        keysHeld |= KEY_DRIGHT;
                        //simulatedNextPage = false;
                    }
                    else if (menuMode != OVERLAYS_STR) {
                        keysHeld |= KEY_DLEFT;
                        //simulatedNextPage = false;
                    } else {
                        //simulatedNextPage = false;
                        simulatedNextPageComplete = true;
                    }
                }

                if ((keysHeld & KEY_RIGHT) && !(keysHeld & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK) && !stillTouching && (((!allowSlide && !unlockedSlide && onTrackBar) || (keysHeld & KEY_R)) || !onTrackBar || simulatedNextPage)) {
                    simulatedNextPage = false;
                    allowSlide = unlockedSlide = false;
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
                if ((keysHeld & KEY_LEFT) && !(keysHeld & ~KEY_LEFT & ~KEY_R & ALL_KEYS_MASK) && !stillTouching && (((!allowSlide && onTrackBar && !unlockedSlide) || (keysHeld & KEY_R)) || !onTrackBar || simulatedNextPage)) {
                    simulatedNextPage = false;
                    allowSlide = unlockedSlide = false;
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
                simulatedNextPage = false;

                if (simulatedBack && !simulatedBackComplete) {
                    keysHeld |= KEY_B;
                    simulatedBack = false;
                }

                if ((keysHeld & KEY_B) && !stillTouching) {
                    allowSlide = unlockedSlide = false;
                    tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
                    exitingUltrahand = true;
                    tsl::Overlay::get()->close();
                    simulatedBackComplete = true;
                    return true;
                }

                if (simulatedMenu && !simulatedMenuComplete) {
                    keysHeld |= SYSTEM_SETTINGS_KEY;
                    simulatedMenu = false;
                }

                if ((keysHeld & SYSTEM_SETTINGS_KEY) && !stillTouching) {
                    inMainMenu = false;
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
                    if (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR) == FALSE_STR) {
                        inMainMenu = true;
                        inHiddenMode = false;
                        hiddenMenuMode = "";
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_HIDDEN_OVERLAY_STR, "");
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
                    
                    allowSlide = unlockedSlide = false;
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
            tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
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
        //isLauncher = true;

        //tsl::initializeThemeVars();
        //tsl::initializeUltrahandSettings(); // unnecessary for Ultrahand's implementation
        //ASSERT_FATAL(smInitialize()); // might be unnecessary? needs investigating

    	ASSERT_FATAL(socketInitializeDefault());
        initializeCurl();

        // Load and execute "boot" commands if they exist
        //executeIniCommands(PACKAGE_PATH + BOOT_PACKAGE_FILENAME, "boot");

        // read commands from root package's boot_package.ini
        if (firstBoot) {
            // Load and execute "initial_boot" commands if they exist
            executeIniCommands(PACKAGE_PATH + BOOT_PACKAGE_FILENAME, "boot");
            
            bool disableFuseReload = (parseValueFromIniSection(FUSE_DATA_INI_PATH, FUSE_STR, "disable_reload") == TRUE_STR);
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

        //smExit();
        //closeInterpreterThread(); // shouldn't be running, but run close anyways
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
