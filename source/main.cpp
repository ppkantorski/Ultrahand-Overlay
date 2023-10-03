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
 *   Note: Please refer to the project documentation and README.md for detailed
 *   information on how to use and configure the Ultrahand Overlay.
 * 
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/


#define NDEBUG
#define STBTT_STATIC
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <utils.hpp>


// Overlay booleans
//static bool shouldCloseMenu = false;
static bool returningToMain = false;
static bool returningToSub = false;
static bool returningToSubSub = false;
static bool inMainMenu = false;
//static bool inOverlay = false;
static bool inSubMenu = false;
static bool inSubSubMenu = false;
static bool inConfigMenu = false;
static bool inSelectionMenu = false;
static bool defaultMenuLoaded = true;
static bool freshSpawn = true;

static tsl::elm::OverlayFrame *rootFrame = nullptr;
static tsl::elm::List *list = nullptr;

// Command mode globals
static std::vector<std::string> commandModes = {"default", "toggle", "option"};
static std::vector<std::string> commandGroupings = {"default", "split"};
static std::string modePattern = ";mode=";
static std::string groupingPattern = ";grouping=";


static std::string lastMenu = "";
static std::string lastKeyName = "";
static std::unordered_map<std::string, std::string> selectedFooterDict;
static auto selectedListItem = new tsl::elm::ListItem("");
static auto lastSelectedListItem = new tsl::elm::ListItem("");

// Pre-defined symbols (moved to libTesla)
//static std::string OPTION_SYMBOL = "\u22EF";
//static std::string DROPDOWN_SYMBOL = "\u25B6";
//static std::string CHECKMARK_SYMBOL = "\uE14B";
//static std::string STAR_SYMBOL = "\u2605";

/**
 * @brief The `ConfigOverlay` class handles configuration overlay functionality.
 *
 * This class manages the configuration overlay, allowing users to modify settings
 * in the INI file. It provides functions for creating, updating, and cleaning INI files
 * as well as handling user interactions related to configuration.
 */
class ConfigOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey;
    bool isInSection, inQuotes, isFromMainMenu;

public:
    /**
     * @brief Constructs a `ConfigOverlay` instance.
     *
     * Initializes a new instance of the `ConfigOverlay` class with the provided parameters.
     *
     * @param file The file path associated with the overlay.
     * @param key The specific key related to the overlay (optional).
     */
    ConfigOverlay(const std::string& file, const std::string& key = "", const bool& fromMainMenu=false) : filePath(file), specificKey(key), isFromMainMenu(fromMainMenu) {}
    
    /**
     * @brief Destroys the `ConfigOverlay` instance.
     *
     * Cleans up any resources associated with the `ConfigOverlay` instance.
     */
    ~ConfigOverlay() {}
    
    /**
     * @brief Creates the graphical user interface (GUI) for the configuration overlay.
     *
     * This function initializes and sets up the GUI elements for the configuration overlay,
     * allowing users to modify settings in the INI file.
     *
     * @return A pointer to the GUI element representing the configuration overlay.
     */
    virtual tsl::elm::Element* createUI() override {
        inConfigMenu = true;
        std::string packageName = getNameFromPath(filePath);
        if (packageName == ".packages") {
            packageName = "Root Package";
        }
        rootFrame = new tsl::elm::OverlayFrame(packageName, "Ultrahand Config");
        list = new tsl::elm::List();
        
        std::string packageFile = filePath + packageFileName;
        std::string fileContent = getFileContents(packageFile);
        
        if (!fileContent.empty()) {
            std::string line;
            std::istringstream iss(fileContent);
            std::string currentCategory;
            isInSection = false;
            while (std::getline(iss, line)) {
                if (line.empty() || line.find_first_not_of('\n') == std::string::npos) {
                    continue;
                }
                
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
                                    } else {
                                        commandParts.emplace_back(part);
                                    }
                                }
                                inQuotes = !inQuotes;
                            }
                            
                            commandVec.emplace_back(std::move(commandParts));
                            interpretAndExecuteCommand(commandVec);
                            listItem->setValue(CHECKMARK_SYMBOL);
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                }
            }
        } else {
            list->addItem(new tsl::elm::ListItem("Failed to open file: " + packageFile));
        }
        
        
        
        
        
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
        if (inConfigMenu) {
            if (keysHeld & KEY_B) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inConfigMenu = false;
                if (isFromMainMenu == false){
                    returningToSub = true;
                } else {
                    returningToMain = true;
                }
                tsl::goBack();
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        if (keysHeld & KEY_B) {
            return false;
        }
        return false;
        //return handleOverlayMenuInput(inConfigMenu, keysHeld, KEY_B);
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
    SelectionOverlay(const std::string& path, const std::string& key = "", const std::vector<std::vector<std::string>>& cmds = {})
        : filePath(path), specificKey(key), commands(cmds) {
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
        
        rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(filePath), "Ultrahand Package");
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
        std::vector<std::string> listData, listDataOn, listDataOff;
        json_t* jsonData = nullptr;
        json_t* jsonDataOn = nullptr;
        json_t* jsonDataOff = nullptr;
        
        
        // initial processing of commands
        for (const auto& cmd : commands) {
            
            if (!cmd.empty()) { // Isolate command settings
                // Extract the command mode
                if (cmd[0].find(modePattern) == 0) {
                    commandMode = cmd[0].substr(modePattern.length());
                    if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end()) {
                        commandMode = commandModes[0]; // reset to default if commandMode is unknown
                    }
                } else if (cmd[0].find(groupingPattern) == 0) {// Extract the command grouping
                    commandGrouping = cmd[0].substr(groupingPattern.length());
                    if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end()) {
                        commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                    }
                }
                
                // Extract the command grouping
                if (commandMode == "toggle") {
                    if (cmd[0].find("on:") == 0) {
                        currentSection = "on";
                    } else if (cmd[0].find("off:") == 0) {
                        currentSection = "off";
                    }
                    
                    // Seperation of command chuncks
                    if (currentSection == "global") {
                        commandsOn.push_back(cmd);
                        commandsOff.push_back(cmd);
                    } else if (currentSection == "on") {
                        commandsOn.push_back(cmd);
                    } else if (currentSection == "off") {
                        commandsOff.push_back(cmd);
                    }
                } else if (commandMode == "option") {
                    // 
                    
                }
                
            }
            if (cmd.size() > 1) { // Pre-process advanced commands
                if (cmd[0] == "filter") {
                    if (currentSection == "global") {
                        filterList.push_back(cmd[1]);
                    } else if (currentSection == "on") {
                        filterListOn.push_back(cmd[1]);
                    } else if (currentSection == "off") {
                        filterListOff.push_back(cmd[1]);
                    }
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
                        jsonData = readJsonFromFile(jsonPath);
                        sourceType = "json_file";
                        if (cmd.size() > 2) {
                            jsonKey = cmd[2]; //json display key
                        }
                    } else if (currentSection == "on") {
                        jsonPathOn = preprocessPath(cmd[1]);
                        jsonDataOn = readJsonFromFile(jsonPathOn);
                        sourceTypeOn = "json_file";
                        if (cmd.size() > 2) {
                            jsonKeyOn = cmd[2]; //json display key
                        }
                    } else if (currentSection == "off") {
                        jsonPathOff = preprocessPath(cmd[1]);
                        jsonDataOff = readJsonFromFile(jsonPathOff);
                        sourceTypeOff = "json_file";
                        if (cmd.size() > 2) {
                            jsonKeyOff = cmd[2]; //json display key
                        }
                    }
                } else if (cmd[0] == "list_source") {
                    if (currentSection == "global") {
                        listData = stringToList(removeQuotes(cmd[1]));
                        sourceType = "list";
                    } else if (currentSection == "on") {
                        listDataOn = stringToList(removeQuotes(cmd[1]));
                        sourceTypeOn = "list";
                    } else if (currentSection == "off") {
                        listDataOff = stringToList(removeQuotes(cmd[1]));
                        sourceTypeOff = "list";
                    }
                } else if (cmd[0] == "json_source") {
                    if (currentSection == "global") {
                        jsonData = stringToJson(cmd[1]); // convert string to jsonData
                        sourceType = "json";
                        
                        if (cmd.size() > 2) {
                            jsonKey = cmd[2]; //json display key
                        }
                    } else if (currentSection == "on") {
                        jsonDataOn = stringToJson(cmd[1]); // convert string to jsonData
                        sourceTypeOn = "json";
                        
                        if (cmd.size() > 2) {
                            jsonKeyOn = cmd[2]; //json display key
                        }
                        
                    } else if (currentSection == "off") {
                        jsonDataOff = stringToJson(cmd[1]); // convert string to jsonData
                        sourceTypeOff = "json";
                        
                        if (cmd.size() > 2) {
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
            if (sourceType == "file"){
                selectedItemsList = filesList;
            } else if (sourceType == "list"){
                selectedItemsList = listData;
            } else if ((sourceType == "json") || (sourceType == "json_file")) {
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
            }
        } else if (commandMode == "toggle") {
            if (sourceTypeOn == "file") {
                selectedItemsListOn = filesListOn;
            } else if (sourceTypeOn == "list") {
                selectedItemsListOn = listDataOn;
            } else if ((sourceTypeOn == "json") || (sourceTypeOn == "json_file")) {
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
            }
            
            if (sourceTypeOff == "file") {
                selectedItemsListOff = filesListOff;
            } else if (sourceTypeOff == "list") {
                selectedItemsListOff = listDataOff;
            } else if ((sourceTypeOff == "json") || (sourceTypeOff == "json_file")) {
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
                    if (parentDirA != parentDirB) {
                        return parentDirA < parentDirB;
                    } else {
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
        
        
        if (commandGrouping == "default") {
            list->addItem(new tsl::elm::CategoryHeader(specificKey.substr(1))); // remove * from key
        }
        
        
        
        
        // Add each file as a menu item
        for (size_t i = 0; i < selectedItemsList.size(); ++i) {
            const std::string& selectedItem = selectedItemsList[i];
            
            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, selectedItem);
            //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, std::to_string(i));
            
            // For entries that are paths
            itemName = getNameFromPath(selectedItem);
            if (!isDirectory(preprocessPath(selectedItem))) {
                itemName = dropExtension(itemName);
            }
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
                    if (selectedFooterDict[specificKey] == selectedItem) {
                        lastSelectedListItem = listItem;
                        listItem->setValue(CHECKMARK_SYMBOL);
                    } else {
                        listItem->setValue(footer);
                    }
                } else {
                    listItem->setValue(footer, true);
                }
                
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
                    listItem->setClickListener([this, optionName, cmds=commands, footer, selectedItem, i, listItem](uint64_t keys) { // Add 'command' to the capture list
                        if (keys & KEY_A) {
                            if (commandMode == "option") {
                                selectedFooterDict[specificKey] = selectedItem;
                                lastSelectedListItem->setValue(footer, true);
                            }
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem, i); // replace source
                            //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                            interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                            
                            listItem->setValue(CHECKMARK_SYMBOL);
                            
                            if (commandMode == "option") {
                                lastSelectedListItem = listItem;
                            }
                            
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                } else {
                    listItem->setClickListener([this, optionName, cmds=commands, footer, selectedItem, i, listItem](uint64_t keys) { // Add 'command' to the capture list
                        if (keys & KEY_A) {
                            if (commandMode == "option") {
                                selectedFooterDict[specificKey] = selectedItem;
                                lastSelectedListItem->setValue(footer, true);
                            }
                            
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem, i); // replace source
                            //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                            interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                            
                            listItem->setValue(CHECKMARK_SYMBOL);
                            
                            if (commandMode == "option") {
                                lastSelectedListItem = listItem;
                            }
                            
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                }
            } else if (commandMode == "toggle") {
                auto toggleListItem = new tsl::elm::ToggleListItem(itemName, false, "On", "Off");
                
                // Set the initial state of the toggle item
                bool toggleStateOn = std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end();
                toggleListItem->setState(toggleStateOn);
                
                toggleListItem->setStateChangedListener([this, cmdsOn=commandsOn, cmdsOff=commandsOff, selectedItem, i, selectedItemsListOn, selectedItemsListOff, toggleListItem](bool state) {
                    if (!state) {
                        if (std::find(selectedItemsListOn.begin(), selectedItemsListOn.end(), selectedItem) != selectedItemsListOn.end()) {
                            // Toggle switched to On
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOn, selectedItem, i); // replace source
                            interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                        } else {
                            toggleListItem->setState(!state);
                        }
                    } else {
                        if (std::find(selectedItemsListOff.begin(), selectedItemsListOff.end(), selectedItem) != selectedItemsListOff.end()) {
                            // Toggle switched to Off
                            std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOff, selectedItem, i); // replace source
                            interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                        } else {
                            toggleListItem->setState(!state);
                        }
                    }
                });
                list->addItem(toggleListItem);
            }
            //count++;
        }
        
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
        if (inSelectionMenu) {
            if (keysHeld & KEY_B) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inSelectionMenu = false;
                
                if (lastMenu == "subMenu") {
                    returningToSub = true;
                } else if (lastMenu == "subSubMenu") {
                    returningToSubSub = true;
                }
                
                
                if (commandMode == "option") {
                    if (isFileOrDirectory(packageConfigIniPath)) {
                        auto packageConfigData = getParsedDataFromIniFile(packageConfigIniPath);
                        if (packageConfigData.count(specificKey) > 0) {
                            auto& optionSection = packageConfigData[specificKey];
                            if (optionSection.count("footer") > 0) {
                                auto& commandFooter = optionSection["footer"];
                                if (commandFooter != "null") {
                                    selectedListItem->setValue(commandFooter);
                                }
                            }
                        }
                    }
                }
                
                //lastSelectedListItem = new tsl::elm::ListItem("");
                tsl::goBack();
                //tsl::Overlay::get()->close();
                return true;
            }
        } 
        if (keysHeld & KEY_B) {
            return false;
        }
        
        return false;
        
        //return handleOverlayMenuInput(inSelectionMenu, keysHeld, KEY_B);
    }
};

// Forward declaration of the MainMenu class.
class MainMenu;

/**
 * @brief The `SubMenu` class handles sub-menu overlay functionality.
 *
 * This class manages sub-menu overlays, allowing users to interact with specific menu options.
 * It provides functions for creating, updating, and navigating sub-menus, as well as handling user interactions related to sub-menu items.
 */
class SubMenu : public tsl::Gui {
private:
    tsl::hlp::ini::IniData packageConfigData;
    std::string subPath, dropdownSection, pathReplace, pathReplaceOn, pathReplaceOff;
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
    std::string lastSection = "";
public:
    /**
     * @brief Constructs a `SubMenu` instance for a specific sub-menu path.
     *
     * Initializes a new instance of the `SubMenu` class for the given sub-menu path.
     *
     * @param path The path to the sub-menu.
     */
    SubMenu(const std::string& path, const std::string& sectionName = "") : subPath(path), dropdownSection(sectionName) {}
    /**
     * @brief Destroys the `SubMenu` instance.
     *
     * Cleans up any resources associated with the `SubMenu` instance.
     */
    ~SubMenu() {
        if (inSubMenu) {
            selectedFooterDict.clear(); // Clears all data from the map, making it empty again
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
            inSubMenu = true;
            lastMenu = "subMenu";
        } else {
            inSubSubMenu = true;
            lastMenu = "subSubMenu";
        }
        
        
        rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(subPath), "Ultrahand Package");
        list = new tsl::elm::List();
        
        auto listItem = static_cast<tsl::elm::ListItem*>(nullptr);
        
        // Load options from INI file in the subdirectory
        std::string packageIniPath = subPath + packageFileName;
        std::string packageConfigIniPath = subPath + configFileName;
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath);
        
        bool skipSection = false;
        // Populate the sub menu with options
        //for (const auto& option : options) {
        
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
            
            // Custom header implementation
            if (!dropdownSection.empty()) {
                if (i == 0) {
                    // Add a section break with small text to indicate the "Commands" section
                    //if (dropdownSection[0] == '*') {
                    //    dropdownSection = dropdownSection.substr(1);
                    //}
                    list->addItem(new tsl::elm::CategoryHeader(dropdownSection.substr(1)));
                    skipSection = true;
                    lastSection = dropdownSection;
                }
                if (commands.size() == 0) {
                    if (optionName == dropdownSection) {
                        skipSection = false;
                    } else {
                        skipSection = true;
                    }
                    continue;
                }
            } else {
                if (commands.size() == 0) {
                    if (optionName[0] != '*') {
                        if (optionName != lastSection) {
                            // Add a section break with small text to indicate the "Commands" section
                            list->addItem(new tsl::elm::CategoryHeader(optionName));
                            lastSection = optionName;
                        }
                        skipSection = false;
                    } else {
                        // Create reference to SubMenu with dropdownSection set to optionName
                        listItem = new tsl::elm::ListItem(optionName.substr(1), DROPDOWN_SYMBOL);
                        
                        listItem->setClickListener([this, optionName](s64 key) {
                            if (key & KEY_A) {
                                inSubMenu = false;
                                tsl::changeTo<SubMenu>(subPath, optionName);
                                return true;
                            }
                            return false;
                        });
                        list->addItem(listItem);
                        
                        
                        skipSection = true;
                    }
                    
                    continue;
                } else if (i == 0) {
                    // Add a section break with small text to indicate the "Commands" section
                    list->addItem(new tsl::elm::CategoryHeader("Commands"));
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
                        if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end()) {
                            commandMode = commandModes[0]; // reset to default if commandMode is unknown
                        }
                    } else if (cmd[0].find(groupingPattern) == 0) {// Extract the command grouping
                        commandGrouping = cmd[0].substr(groupingPattern.length());
                        if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end()) {
                            commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                        }
                    }
                    
                    // Extract the command grouping
                    if (commandMode == "toggle") {
                        if (cmd[0].find("on:") == 0) {
                            currentSection = "on";
                        } else if (cmd[0].find("off:") == 0) {
                            currentSection = "off";
                        }
                        
                        // Seperation of command chuncks
                        if (currentSection == "global") {
                            commandsOn.push_back(cmd);
                            commandsOff.push_back(cmd);
                        } else if (currentSection == "on") {
                            commandsOn.push_back(cmd);
                        } else if (currentSection == "off") {
                            commandsOff.push_back(cmd);
                        }
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
                    if (optionSection.count("mode") > 0) {
                        commandMode = optionSection["mode"];
                    } else {
                        setIniFileValue(packageConfigIniPath, optionName, "mode", commandMode);
                    }
                    if (optionSection.count("grouping") > 0) {
                        commandGrouping = optionSection["grouping"];
                    } else {
                        setIniFileValue(packageConfigIniPath, optionName, "grouping", commandGrouping);
                    }
                    
                    if (optionSection.count("footer") > 0) {
                        commandFooter = optionSection["footer"];
                    } else {
                        setIniFileValue(packageConfigIniPath, optionName, "footer", commandFooter);
                    }
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
                if (commandFooter != "null") {
                    footer = commandFooter;
                } else {
                    footer = OPTION_SYMBOL;
                }
            }
            
            if (skipSection == false) { // for skipping the drawing of sections
                if (useSelection) { // For wildcard commands (dropdown menus)
                    
                    if ((footer == DROPDOWN_SYMBOL) || (footer.empty())) {
                        listItem = new tsl::elm::ListItem(optionName, footer);
                    } else {
                        listItem = new tsl::elm::ListItem(optionName);
                        if (commandMode == "option") {
                            listItem->setValue(footer);
                        } else {
                            listItem->setValue(footer, true);
                        }
                    
                    }
                    
                    //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                    listItem->setClickListener([cmds = commands, keyName = option.first, this, subPath = this->subPath, footer, listItem](uint64_t keys) {
                        if (keys & KEY_A) {
                            if (inSubMenu) {
                                inSubMenu = false;
                            }
                            if (inSubSubMenu) {
                                inSubSubMenu = false;
                            }
                            
                            selectedListItem = listItem;
                            
                            if (inSubMenu) {
                                if (selectedFooterDict.find(lastSection+keyName) == selectedFooterDict.end()) {
                                    selectedFooterDict[lastSection+keyName] = footer;
                                }
                            } else {
                                if (selectedFooterDict.find("sub_"+lastSection+keyName) == selectedFooterDict.end()) {
                                    selectedFooterDict["sub_"+lastSection+keyName] = footer;
                                }
                            }
                            tsl::changeTo<SelectionOverlay>(subPath, keyName, cmds);
                            lastKeyName = keyName;
                            
                            return true;
                        } else if (keys & KEY_X) {
                            if (inSubMenu) {
                                inSubMenu = false;
                            }
                            if (inSubSubMenu) {
                                inSubSubMenu = false;
                            }
                            tsl::changeTo<ConfigOverlay>(subPath, keyName);
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem);
                } else { // For everything else
                    
                    const std::string& selectedItem = optionName;
                    
                    // For entries that are paths
                    itemName = getNameFromPath(selectedItem);
                    if (!isDirectory(preprocessPath(selectedItem))) {
                        itemName = dropExtension(itemName);
                    }
                    parentDirName = getParentDirNameFromPath(selectedItem);
                    
                    
                    if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                        auto listItem = new tsl::elm::ListItem(optionName);
                        if (commandMode == "default") {
                            listItem->setValue(footer, true);
                        } else {
                            listItem->setValue(footer);
                        }
                        
                        
                        if (sourceType == "json") { // For JSON wildcards
                            listItem->setClickListener([this, cmds=commands, subPath = this->subPath, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                if (keys & KEY_A) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                                
                                    listItem->setValue(CHECKMARK_SYMBOL);
                                    return true;
                                }  else if (keys & KEY_X) {
                                    if (inSubMenu) {
                                        inSubMenu = false;
                                    }
                                    if (inSubSubMenu) {
                                        inSubSubMenu = false;
                                    }
                                    tsl::changeTo<ConfigOverlay>(subPath, keyName);
                                    return true;
                                }
                            
                                return false;
                            });
                            list->addItem(listItem);
                        } else {
                            listItem->setClickListener([this, cmds=commands, subPath = this->subPath, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                if (keys & KEY_A) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                                
                                    listItem->setValue(CHECKMARK_SYMBOL);
                                    return true;
                                }  else if (keys & KEY_X) {
                                    if (inSubMenu) {
                                        inSubMenu = false;
                                    }
                                    if (inSubSubMenu) {
                                        inSubSubMenu = false;
                                    }
                                    tsl::changeTo<ConfigOverlay>(subPath, keyName);
                                    return true;
                                }
                                return false;
                            });
                            list->addItem(listItem);
                        }
                    } else if (commandMode == "toggle") {
                    
                    
                        auto toggleListItem = new tsl::elm::ToggleListItem(optionName, false, "On", "Off");
                        // Set the initial state of the toggle item
                        bool toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn));
                    
                        toggleListItem->setState(toggleStateOn);
                    
                        toggleListItem->setStateChangedListener([this, cmdsOn=commandsOn, cmdsOff=commandsOff, toggleStateOn](bool state) {
                            if (!state) {
                                // Toggle switched to On
                                if (toggleStateOn) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOn, preprocessPath(pathPatternOn)); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                                } else {
                                    // Handle the case where the command should only run in the source_on section
                                    // Add your specific code here
                                }
                            } else {
                                // Toggle switched to Off
                                if (!toggleStateOn) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOff, preprocessPath(pathPatternOff)); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
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
        
        if (dropdownSection.empty()) {
            // Package Info
            PackageHeader packageHeader = getPackageHeaderFromIni(packageIniPath);
            
            constexpr int lineHeight = 20;  // Adjust the line height as needed
            constexpr int xOffset = 120;    // Adjust the horizontal offset as needed
            constexpr int fontSize = 16;    // Adjust the font size as needed
            int numEntries = 0;   // Adjust the number of entries as needed
            
            std::string packageSectionString = "";
            std::string packageInfoString = "";
            if (packageHeader.version != "") {
                packageSectionString += "Version\n";
                packageInfoString += (packageHeader.version+"\n").c_str();
                numEntries++;
            }
            if (packageHeader.creator != "") {
                packageSectionString += "Creator(s)\n";
                packageInfoString += (packageHeader.creator+"\n").c_str();
                numEntries++;
            }
            if (packageHeader.about != "") {
                std::string aboutHeaderText = "About\n";
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
                    if (startPos < aboutText.length()) {
                        packageSectionString += std::string(aboutHeaderLength, ' ') + '\n';
                    }
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
                list->addItem(new tsl::elm::CategoryHeader("Package Info"));
                list->addItem(new tsl::elm::CustomDrawer([lineHeight, xOffset, fontSize, packageSectionString, packageInfoString](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
                    renderer->drawString(packageSectionString.c_str(), false, x, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
                    renderer->drawString(packageInfoString.c_str(), false, x + xOffset, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
                }), fontSize * numEntries + lineHeight);
            }
        }
        
        
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
        
        if (!returningToSub && inSubMenu) {
            if ((keysHeld & KEY_B)) {
                //tsl::Overlay::get()->close();
                //svcSleepThread(300'000'000);
                //tsl::goBack();
                inSubMenu = false;
                returningToMain = true;
                tsl::changeTo<MainMenu>();
                
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        
        if (!returningToSubSub && inSubSubMenu) {
            if ((keysHeld & KEY_B)) {
                inSubSubMenu = false;
                returningToSub = true;
                tsl::goBack();
                
                //tsl::Overlay::get()->close();
                return true;
            }
        }
        
        
        if (keysHeld & KEY_B) {
            return false;
        }
        
        if (returningToSub && !(keysHeld & KEY_B)){
            returningToSub = false;
            inSubMenu = true;
        }
        
        if (returningToSubSub && !(keysHeld & KEY_B)){
            returningToSubSub = false;
            inSubSubMenu = true;
        }
        
        return false;
        
        //return handleOverlayMenuInput(inSubMenu, keysHeld, KEY_B);
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
    tsl::hlp::ini::IniData settingsData, packageConfigData;
    std::string packageIniPath = packageDirectory + packageFileName;
    std::string packageConfigIniPath = packageDirectory + configFileName;
    std::string menuMode, defaultMenuMode, inOverlayString, fullPath, optionName, hideOverlayVersions, hidePackageVersions, priority, starred;
    bool useDefaultMenu = false;
    
    
    std::string subPath, pathReplace, pathReplaceOn, pathReplaceOff;
    std::string filePath, specificKey, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    std::vector<std::string> filesList, filesListOn, filesListOff, filterList, filterListOn, filterListOff;
public:
    /**
     * @brief Constructs a `MainMenu` instance.
     *
     * Initializes a new instance of the `MainMenu` class with the necessary parameters.
     */
    MainMenu() {}
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
        inMainMenu = true;
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
                
                // For hiding the versions of overlays/packages
                if (ultrahandSection.count("hide_overlay_versions") > 0) {
                    hideOverlayVersions = ultrahandSection["hide_overlay_versions"];
                } else {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_overlay_versions", "false");
                    hideOverlayVersions = "false";
                }
                if (ultrahandSection.count("hide_package_versions") > 0) {
                    hidePackageVersions = ultrahandSection["hide_package_versions"];
                } else {
                    setIniFileValue(settingsConfigIniPath, "ultrahand", "hide_package_versions", "false");
                    hidePackageVersions = "false";
                }
                
                if (ultrahandSection.count("last_menu") > 0) {
                    menuMode = ultrahandSection["last_menu"];
                    if (ultrahandSection.count("default_menu") > 0) {
                        defaultMenuMode = ultrahandSection["default_menu"];
                        if (ultrahandSection.count("in_overlay") > 0) {
                            settingsLoaded = true;
                        }
                    }
                }
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
            setIniFileValue(settingsConfigIniPath, "ultrahand", "default_menu", defaultMenuMode);
            setIniFileValue(settingsConfigIniPath, "ultrahand", "last_menu", menuMode);
            setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "false");
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
        
        std::string versionLabel = APP_VERSION+std::string("   (")+envGetLoaderInfo()+std::string(")");
        rootFrame = new tsl::elm::OverlayFrame("Ultrahand", versionLabel, menuMode);
        list = new tsl::elm::List();
        
        
        
        // Overlays menu
        if (menuMode == "overlays") {
            list->addItem(new tsl::elm::CategoryHeader("Overlays"));
            
            // Load overlay files
            std::vector<std::string> overlayFiles = getFilesListByWildcard(overlayDirectory+"*.ovl");
            //std::sort(overlayFiles.begin(), overlayFiles.end()); // Sort overlay files alphabetically
            
            
            FILE* overlaysIniFile = fopen(overlaysIniFilePath.c_str(), "r");
            if (!overlaysIniFile) {
                // The INI file doesn't exist, so create an empty one.
                fclose(fopen(overlaysIniFilePath.c_str(), "w"));
            } else {
                // The file exists, so close it.
                fclose(overlaysIniFile);
            }
            
            // load overlayList from overlaysIniFilePath.  this will be the overlayFilenames
            std::vector<std::string> overlayList;
            
            
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
                        overlayList.push_back("1000_"+overlayFileName);
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "priority", "1000");
                        setIniFileValue(overlaysIniFilePath, overlayFileName, "star", "false");
                        
                    } else {
                        // Read priority and starred status from ini
                        priority = "1000";
                        starred = "false";
                        
                        // Check if the "priority" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("priority") != overlaysIniData[overlayFileName].end()) {
                            priority = formatPriorityString(overlaysIniData[overlayFileName]["priority"]);
                        }
                        // Check if the "star" key exists in overlaysIniData for overlayFileName
                        if (overlaysIniData.find(overlayFileName) != overlaysIniData.end() &&
                            overlaysIniData[overlayFileName].find("star") != overlaysIniData[overlayFileName].end()) {
                            starred = overlaysIniData[overlayFileName]["star"];
                        }
                        
                        if (starred == "true") {
                            overlayList.push_back("-1_"+priority+"_"+overlayFileName);
                        } else {
                            overlayList.push_back(priority+"_"+overlayFileName);
                        }
                    }
                }
                
                std::sort(overlayList.begin(), overlayList.end());
                
                for (const auto& taintedOverlayFileName : overlayList) {
                    
                    //logMessage(taintedOverlayFileName);
                    
                    std::string overlayFileName = taintedOverlayFileName;
                    std::string overlayStarred = "false";
                    
                    if ((overlayFileName.length() >= 2) && (overlayFileName.substr(0, 3) == "-1_")) {
                        // strip first two characters
                        overlayFileName = overlayFileName.substr(3);
                        overlayStarred = "true";
                    }
                    
                    overlayFileName = overlayFileName.substr(5);
                    
                    
                    //logMessage(overlayFileName);
                    
                    std::string overlayFile = overlayDirectory+overlayFileName;
                    //logMessage(overlayFile);
                    
                    // Get the name and version of the overlay file
                    auto [result, overlayName, overlayVersion] = getOverlayInfo(overlayFile);
                    if (result != ResultSuccess)
                        continue;
                    
                    //logMessage(overlayName);
                    
                    std::string newOverlayName = overlayName.c_str();
                    if (overlayStarred == "true") {
                        newOverlayName = STAR_SYMBOL+" "+newOverlayName;
                    }
                    
                    
                    // Toggle the starred status
                    std::string newStarred = (overlayStarred == "true") ? "false" : "true";
                    
                    tsl::elm::ListItem* listItem = nullptr;
                    
                    //logMessage(overlayFile);
                    if (isFileOrDirectory(overlayFile)) {
                        listItem = new tsl::elm::ListItem(newOverlayName);
                        if (hideOverlayVersions != "true") {
                            listItem->setValue(overlayVersion, true);
                        }
                   
                        // Add a click listener to load the overlay when clicked upon
                        listItem->setClickListener([overlayFile, newStarred, overlayFileName](s64 key) {
                            if (key & KEY_A) {
                                // Load the overlay here
                                //inMainMenu = false;
                                //inOverlay = true;
                                setIniFileValue(settingsConfigIniPath, "ultrahand", "in_overlay", "true"); // this is handled within tesla.hpp
                                tsl::setNextOverlay(overlayFile);
                                //envSetNextLoad(overlayPath, "");
                                tsl::Overlay::get()->close();
                                //inMainMenu = true;
                                return true;
                            } else if (key & KEY_PLUS) {
                                if (!overlayFile.empty()) {
                                
                                    // Update the INI file with the new value
                                    setIniFileValue(overlaysIniFilePath, overlayFileName, "star", newStarred);
                                    // Now, you can use the newStarred value for further processing if needed
                                }
                                tsl::changeTo<MainMenu>();
                                return true;
                            }
                            return false;
                        });
                    }
                    if (listItem != nullptr) {
                        list->addItem(listItem);
                    }
                }
            }
        }
        
        // Packages menu
        if (menuMode == "packages" ) {
            
            // Create the directory if it doesn't exist
            createDirectory(packageDirectory);
            
            // Load options from INI file
            std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(packageIniPath, true);
            
            
            FILE* packagesIniFile = fopen(packagesIniFilePath.c_str(), "r");
            if (!packagesIniFile) {
                // The INI file doesn't exist, so create an empty one.
                fclose(fopen(packagesIniFilePath.c_str(), "w"));
            } else {
                // The file exists, so close it.
                fclose(packagesIniFile);
            }
            
            std::vector<std::string> packageList;
            
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
                    packageList.push_back("1000_"+packageName);
                    setIniFileValue(packagesIniFilePath, packageName, "priority", "1000");
                    setIniFileValue(packagesIniFilePath, packageName, "star", "false");
                } else {
                    // Read priority and starred status from ini
                    priority = "1000";
                    starred = "false";
                    
                    // Check if the "priority" key exists in overlaysIniData for overlayFileName
                    if (packagesIniData.find(packageName) != packagesIniData.end() &&
                        packagesIniData[packageName].find("priority") != packagesIniData[packageName].end()) {
                        priority = formatPriorityString(packagesIniData[packageName]["priority"]);
                    }
                    // Check if the "star" key exists in overlaysIniData for overlayFileName
                    if (packagesIniData.find(packageName) != packagesIniData.end() &&
                        packagesIniData[packageName].find("star") != packagesIniData[packageName].end()) {
                        starred = packagesIniData[packageName]["star"];
                    }
                    
                    if (starred == "true") {
                        packageList.push_back("-1_"+priority+"_"+packageName);
                    } else {
                        packageList.push_back(priority+"_"+packageName);
                    }
                }
            }
            std::sort(packageList.begin(), packageList.end());
            
            
            //count = 0;
            for (size_t i = 0; i < packageList.size(); ++i) {
                auto taintePackageName = packageList[i];
                if (i == 0) {
                    list->addItem(new tsl::elm::CategoryHeader("Packages"));
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
                if (packageStarred == "true") {
                    newPackageName = STAR_SYMBOL+" "+newPackageName;
                }
                
                std::string packageFilePath = packageDirectory + packageName+ "/";
            
                // Toggle the starred status
                std::string newStarred = (packageStarred == "true") ? "false" : "true";
                
                tsl::elm::ListItem* listItem = nullptr;
                if (isFileOrDirectory(packageFilePath)) {
                    PackageHeader packageHeader = getPackageHeaderFromIni(packageFilePath+packageFileName);
                    //if (count == 0) {
                    //    // Add a section break with small text to indicate the "Packages" section
                    //    list->addItem(new tsl::elm::CategoryHeader("Packages"));
                    //}
                    
                    listItem = new tsl::elm::ListItem(newPackageName);
                    if (hidePackageVersions != "true") {
                       listItem->setValue(packageHeader.version, true);
                    }
                    
                    
                    // Add a click listener to load the overlay when clicked upon
                    listItem->setClickListener([packageFilePath, newStarred, packageName](s64 key) {
                        if (key & KEY_A) {
                            inMainMenu = false;
                            tsl::changeTo<SubMenu>(packageFilePath);
                            
                            return true;
                        } else if (key & KEY_PLUS) {
                            if (!packageName.empty()) {
                            
                                // Update the INI file with the new value
                                setIniFileValue(packagesIniFilePath, packageName, "star", newStarred);
                            }
                            tsl::changeTo<MainMenu>();
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                    //count++;
                }
            }
            
            
            
            
            // ********* THIS PART ALWAYS NEEDS TO MIRROR WHAT IS WITHIN SUBMENU (perhaps create a new method?)*********
            
           
            
            
            
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
                    list->addItem(new tsl::elm::CategoryHeader(optionName));
                    continue;
                } else if (i == 0) {
                    // Add a section break with small text to indicate the "Commands" section
                    list->addItem(new tsl::elm::CategoryHeader("Commands"));
                }
                
                
                
                
                // items can be paths, commands, or variables depending on source
                //std::vector<std::string> selectedItemsList, selectedItemsListOn, selectedItemsListOff;
                
                // initial processing of commands
                for (const auto& cmd : commands) {
                    
                    
                    if (!cmd.empty()) { // Isolate command settings
                        // Extract the command mode
                        if (cmd[0].find(modePattern) == 0) {
                            commandMode = cmd[0].substr(modePattern.length());
                            if (std::find(commandModes.begin(), commandModes.end(), commandMode) == commandModes.end()) {
                                commandMode = commandModes[0]; // reset to default if commandMode is unknown
                            }
                        } else if (cmd[0].find(groupingPattern) == 0) {// Extract the command grouping
                            commandGrouping = cmd[0].substr(groupingPattern.length());
                            if (std::find(commandGroupings.begin(), commandGroupings.end(), commandGrouping) == commandGroupings.end()) {
                                commandGrouping = commandGroupings[0]; // reset to default if commandMode is unknown
                            }
                        }
                    
                        // Extract the command grouping
                        if (commandMode == "toggle") {
                            if (cmd[0].find("on:") == 0) {
                                currentSection = "on";
                            } else if (cmd[0].find("off:") == 0) {
                                currentSection = "off";
                            }
                            
                            // Seperation of command chuncks
                            if (currentSection == "global") {
                                commandsOn.push_back(cmd);
                                commandsOff.push_back(cmd);
                            } else if (currentSection == "on") {
                                commandsOn.push_back(cmd);
                            } else if (currentSection == "off") {
                                commandsOff.push_back(cmd);
                            }
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
                        if (optionSection.count("mode") > 0) {
                            commandMode = optionSection["mode"];
                        } else {
                            setIniFileValue(packageConfigIniPath, optionName, "mode", commandMode);
                        }
                        if (optionSection.count("grouping") > 0) {
                            commandGrouping = optionSection["grouping"];
                        } else {
                            setIniFileValue(packageConfigIniPath, optionName, "grouping", commandGrouping);
                        }
                        
                        if (optionSection.count("footer") > 0) {
                            commandFooter = optionSection["footer"];
                        } else {
                            setIniFileValue(packageConfigIniPath, optionName, "footer", commandFooter);
                        }
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
                if (commandFooter != "null") {
                    footer = commandFooter;
                }
                
                
                if (useSelection) { // For wildcard commands (dropdown menus)
                    auto listItem = static_cast<tsl::elm::ListItem*>(nullptr);
                    if ((footer == DROPDOWN_SYMBOL) || (footer.empty())) {
                        listItem = new tsl::elm::ListItem(optionName, footer);
                    } else {
                        listItem = new tsl::elm::ListItem(optionName);
                        listItem->setValue(footer, true);
                    }
                    
                    //std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(option.second, pathReplace);
                    listItem->setClickListener([this, cmds = commands, keyName = option.first, subPath = packageDirectory, listItem](uint64_t keys) {
                        if (keys & KEY_A) {
                            inMainMenu = false;
                            tsl::changeTo<SelectionOverlay>(subPath, keyName, cmds);
                            return true;
                        } else if (keys & KEY_X) {
                            inMainMenu = false; // Set boolean to true when entering a submenu
                            tsl::changeTo<ConfigOverlay>(subPath, keyName, true);
                            return true;
                        }
                        return false;
                    });
                    
                    list->addItem(listItem);
                } else { // For everything else
                    
                    const std::string& selectedItem = optionName;
                    
                    // For entries that are paths
                    itemName = getNameFromPath(selectedItem);
                    if (!isDirectory(preprocessPath(selectedItem))) {
                        itemName = dropExtension(itemName);
                    }
                    parentDirName = getParentDirNameFromPath(selectedItem);
                    
                    
                    if (commandMode == "default" || commandMode == "option") { // for handiling toggles
                        auto listItem = new tsl::elm::ListItem(optionName);
                        listItem->setValue(footer, true);
                        
                        if (sourceType == "json") { // For JSON wildcards
                            listItem->setClickListener([this, cmds=commands, subPath = packageDirectory, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                if (keys & KEY_A) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                                    
                                    listItem->setValue(CHECKMARK_SYMBOL);
                                    return true;
                                }  else if (keys & KEY_X) {
                                    inMainMenu = false; // Set boolean to true when entering a submenu
                                    tsl::changeTo<ConfigOverlay>(subPath, keyName, true);
                                    return true;
                                }
                                
                                return false;
                            });
                            list->addItem(listItem);
                        } else {
                            listItem->setClickListener([this, cmds=commands, subPath = packageDirectory, keyName = option.first, selectedItem, listItem](uint64_t keys) { // Add 'command' to the capture list
                                if (keys & KEY_A) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmds, selectedItem); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                                    
                                    listItem->setValue(CHECKMARK_SYMBOL);
                                    return true;
                                }  else if (keys & KEY_X) {
                                    inMainMenu = false; // Set boolean to true when entering a submenu
                                    tsl::changeTo<ConfigOverlay>(subPath, keyName, true);
                                    return true;
                                }
                                return false;
                            });
                            list->addItem(listItem);
                        }
                    } else if (commandMode == "toggle") {
                        
                        
                        auto toggleListItem = new tsl::elm::ToggleListItem(optionName, false, "On", "Off");
                        // Set the initial state of the toggle item
                        bool toggleStateOn = isFileOrDirectory(preprocessPath(pathPatternOn));
                        
                        toggleListItem->setState(toggleStateOn);
                        
                        toggleListItem->setStateChangedListener([this, cmdsOn=commandsOn, cmdsOff=commandsOff, toggleStateOn](bool state) {
                            if (!state) {
                                // Toggle switched to On
                                if (toggleStateOn) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOn, preprocessPath(pathPatternOn)); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
                                } else {
                                    // Handle the case where the command should only run in the source_on section
                                    // Add your specific code here
                                }
                            } else {
                                // Toggle switched to Off
                                if (!toggleStateOn) {
                                    std::vector<std::vector<std::string>> modifiedCmds = getSourceReplacement(cmdsOff, preprocessPath(pathPatternOff)); // replace source
                                    //modifiedCmds = getSecondaryReplacement(modifiedCmds); // replace list and json
                                    interpretAndExecuteCommand(modifiedCmds); // Execute modified 
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
        
        if (inMainMenu){
            if (!freshSpawn && !returningToMain) {
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
                        tsl::changeTo<MainMenu>();
                        return true;
                    }
                }
                if (keysHeld & KEY_B) {
                    //inMainMenu = false;
                    tsl::Overlay::get()->close();
                    return true;
                }
            }
        }
        if (keysHeld & KEY_B) {
            return false;
        }
        
        if (freshSpawn && !(keysHeld & KEY_B)){
            freshSpawn = false;
        }
        if (returningToMain && !(keysHeld & KEY_B)){
            returningToMain = false;
            inMainMenu = true;
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
        fsdevMountSdmc();
        splInitialize();
        spsmInitialize();
        ASSERT_FATAL(socketInitializeDefault());
        ASSERT_FATAL(nifmInitialize(NifmServiceType_User));
        ASSERT_FATAL(timeInitialize());
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
        timeExit();
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
        //    tsl::Overlay::get()->getCurrentGui()->removeFocus();
        //    rootFrame->invalidate();
        //    tsl::Overlay::get()->getCurrentGui()->requestFocus(rootFrame, tsl::FocusDirection::None);
        //}
    } 
    
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
