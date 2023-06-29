#define NDEBUG
#define STBTT_STATIC
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <utils.hpp>
#include <sys/stat.h>


// Overlay booleans
bool shouldCloseMenu = false;
bool returningToMain = false;
bool returningToSub = false;
bool inMainMenu = false;
bool inOverlay = false;
bool inSubMenu = false;
bool inConfigMenu = false;
bool inSelectionMenu = false;

const std::string configFileName = "config.ini";



// Helper function to handle overlay menu input
//bool handleOverlayMenuInput(bool& inMenu, u64 keysHeld, u64 backKey, uint64_t sleepTime = 300'000'000) {
//    if (inMenu && (keysHeld & backKey)) {
//        svcSleepThread(sleepTime);
//        //tsl::Overlay::get()->close();
//        tsl::goBack();
//        
//        inMenu = false;
//        return true;
//    }
//    return false;
//}


// Config overlay 
class ConfigOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey;
    bool isInSection, inQuotes;

public:
    ConfigOverlay(const std::string& file, const std::string& key = "") : filePath(file), specificKey(key) {}

    virtual tsl::elm::Element* createUI() override {
        inConfigMenu = true;
        
        auto rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(filePath), "Ultrahand Config");
        auto list = new tsl::elm::List();

        std::string configFile = filePath + "/" + configFileName;

        std::string fileContent = readFileContent(configFile);
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
                            list->addItem(new tsl::elm::CategoryHeader(line.substr(1, line.size() - 2)));
                        } else {
                            currentCategory.clear();
                            isInSection = false;
                        }
                    } else {
                        currentCategory = line.substr(1, line.size() - 2);
                        isInSection = true;
                        list->addItem(new tsl::elm::CategoryHeader(line.substr(1, line.size() - 2)));
                    }
                } else if (isInSection) {
                    auto listItem = new tsl::elm::ListItem(line);
                    listItem->setClickListener([line, this](uint64_t keys) {
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
                            return true;
                        }
                        return false;
                    });
                    list->addItem(listItem);
                }
            }
        } else {
            list->addItem(new tsl::elm::ListItem("Failed to open file: " + configFile));
        }

        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (inConfigMenu && (keysHeld & KEY_B)) {
            //tsl::Overlay::get()->close();
            svcSleepThread(300'000'000);
            tsl::goBack();
            inConfigMenu = false;
            returningToSub = true;
            //tsl::Overlay::get()->close();
            return true;
        }
        return false;
        //return handleOverlayMenuInput(inConfigMenu, keysHeld, KEY_B);
    }
};



// Selection overlay
class SelectionOverlay : public tsl::Gui {
private:
    std::string filePath, specificKey, filterPath, filterOnPath, filterOffPath, pathPattern, pathPatternOn, pathPatternOff, itemName, parentDirName, lastParentDirName;
    std::vector<std::string> filesList, filesListOn, filesListOff;
    std::vector<std::vector<std::string>> commands;
    bool toggleState = false;


public:
    SelectionOverlay(const std::string& file, const std::string& key = "", const std::vector<std::vector<std::string>>& cmds = {}) 
        : filePath(file), specificKey(key), commands(cmds) {}

    virtual tsl::elm::Element* createUI() override {
        inSelectionMenu = true;

        auto rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(filePath), "Ultrahand Package");
        auto list = new tsl::elm::List();

        // Extract the path pattern from commands
        bool useToggle = false;
        bool useSplitHeader = false;
        
        for (const auto& cmd : commands) {
            if (cmd.size() > 1) {
                if (cmd[0] == "split") {
                    useSplitHeader = true;
                } else if (cmd[0] == "filter") {
                    filterPath = cmd[1];
                } else if (cmd[0] == "filter_on") {
                    filterOnPath = cmd[1];
                    useToggle = true;
                } else if (cmd[0] == "filter_off") {
                    filterOffPath = cmd[1];
                } else if (cmd[0] == "source") {
                    pathPattern = cmd[1];
                } else if (cmd[0] == "source_on") {
                    pathPatternOn = cmd[1];
                    useToggle = true;
                } else if (cmd[0] == "source_off") {
                    pathPatternOff = cmd[1];
                }
            } 
        }

        // Get the list of files matching the pattern
        if (!useToggle) {
            filesList = getFilesListByWildcards(pathPattern);
        } else {
            filesListOn = getFilesListByWildcards(pathPatternOn);
            filesListOff = getFilesListByWildcards(pathPatternOff);
            
            if (!filterOnPath.empty()){
                removeEntryFromList(filterOnPath, filesListOn);
            }
            if (!filterOffPath.empty()){
                removeEntryFromList(filterOffPath, filesListOff);
            }
            // remove filterOnPath from filesListOn
            // remove filterOffPath from filesListOff
            
            
            filesList.reserve(filesListOn.size() + filesListOff.size());
            filesList.insert(filesList.end(), filesListOn.begin(), filesListOn.end());
            filesList.insert(filesList.end(), filesListOff.begin(), filesListOff.end());
            if (useSplitHeader) {
                std::sort(filesList.begin(), filesList.end(), [](const std::string& a, const std::string& b) {
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
                std::sort(filesList.begin(), filesList.end(), [](const std::string& a, const std::string& b) {
                    return getNameFromPath(a) < getNameFromPath(b);
                });
            }

            
        }
        
        if (!filterPath.empty()){
            // remove filterPath from filesList
            removeEntryFromList(filterPath, filesList);
        }
        
        
        if (!useSplitHeader){
            list->addItem(new tsl::elm::CategoryHeader(specificKey.substr(1)));
        }
        
        // Add each file as a menu item
        for (const std::string& file : filesList) {
            //if (file.compare(0, filterPath.length(), filterPath) != 0){
            itemName = getNameFromPath(file);
            if (!isDirectory(preprocessPath(file))) {
                itemName = dropExtension(itemName);
            }
            parentDirName = getParentDirNameFromPath(file);
            if (useSplitHeader && (lastParentDirName.empty() || (lastParentDirName != parentDirName))){
                list->addItem(new tsl::elm::CategoryHeader(removeQuotes(parentDirName)));
                lastParentDirName = parentDirName.c_str();
            }
            
            if (!useToggle) {
                auto listItem = new tsl::elm::ListItem(itemName);
                listItem->setClickListener([file, this](uint64_t keys) { // Add 'command' to the capture list
                    if (keys & KEY_A) {
                        // Replace "{source}" with file in commands, then execute
                        std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, file);
                        interpretAndExecuteCommand(modifiedCommands);
                        return true;
                    }
                    return false;
                });
                list->addItem(listItem);
            } else { // for handiling toggles
                auto toggleListItem = new tsl::elm::ToggleListItem(itemName, false, "On", "Off");

                // Set the initial state of the toggle item
                bool toggleStateOn = std::find(filesListOn.begin(), filesListOn.end(), file) != filesListOn.end();
                toggleListItem->setState(toggleStateOn);

                toggleListItem->setStateChangedListener([toggleListItem, file, toggleStateOn, this](bool state) {
                    if (!state) {
                        // Toggle switched to On
                        if (toggleStateOn) {
                            std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, file, true);
                            interpretAndExecuteCommand(modifiedCommands);
                        } else {
                            // Handle the case where the command should only run in the source_on section
                            // Add your specific code here
                        }
                    } else {
                        // Toggle switched to Off
                        if (!toggleStateOn) {
                            std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(commands, file, true, false);
                            interpretAndExecuteCommand(modifiedCommands);
                        } else {
                            // Handle the case where the command should only run in the source_off section
                            // Add your specific code here
                        }
                    }
                });

                list->addItem(toggleListItem);
            } 

        }

        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (inSelectionMenu && (keysHeld & KEY_B)) {
            //tsl::Overlay::get()->close();
            svcSleepThread(300'000'000);
            tsl::goBack();
            inSelectionMenu = false;
            returningToSub = true;
            //tsl::Overlay::get()->close();
            return true;
        }
        return false;
        
        //return handleOverlayMenuInput(inSelectionMenu, keysHeld, KEY_B);
    }
};



// Sub menu
class SubMenu : public tsl::Gui {
private:
    std::string subPath, pathReplace, pathReplaceOn, pathReplaceOff;

public:
    SubMenu(const std::string& path) : subPath(path) {}

    virtual tsl::elm::Element* createUI() override {
        inSubMenu = true;
        
        auto rootFrame = new tsl::elm::OverlayFrame(getNameFromPath(subPath), "Ultrahand Package");
        auto list = new tsl::elm::List();

        // Add a section break with small text to indicate the "Commands" section
        list->addItem(new tsl::elm::CategoryHeader("Commands"));

        // Load options from INI file in the subdirectory
        std::string subConfigIniPath = subPath + "/" + configFileName;
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(subConfigIniPath);
        
        // Populate the sub menu with options
        for (const auto& option : options) {
            std::string optionName = option.first;
            std::string footer; 
            bool usePattern = false;
            if (optionName[0] == '*') { 
                usePattern = true;
                optionName = optionName.substr(1); // Strip the "*" character on the left
                footer = ">";
            }
            
            // Extract the path pattern from commands
            bool useToggle = false;
            for (const auto& cmd : option.second) {
                if (cmd.size() > 1) {
                    if (cmd[0] == "source") {
                        pathReplace = cmd[1];
                        break;
                    } else if (cmd[0] == "source_on") {
                        pathReplaceOn = cmd[1];
                        useToggle = true;
                    } else if (cmd[0] == "source_off") {
                        pathReplaceOff = cmd[1];
                        break;
                    }
                } 
            }
            
            if (usePattern || !useToggle){
                auto listItem = new tsl::elm::ListItem(optionName, footer);
            
                listItem->setClickListener([command = option.second, keyName = option.first, subPath = this->subPath, usePattern](uint64_t keys) {
                    if (keys & KEY_A) {
                        if (usePattern) {
                            inSubMenu = false;
                            tsl::changeTo<SelectionOverlay>(subPath, keyName, command);
                        } else {
                            // Interpret and execute the command
                            interpretAndExecuteCommand(command);
                        }
                        return true;
                    } else if (keys & KEY_X) {
                        inSubMenu = false; // Set boolean to true when entering a submenu
                        tsl::changeTo<ConfigOverlay>(subPath, keyName);
                        return true;
                    }
                    return false;
                });

                list->addItem(listItem);
            } else {
                auto toggleListItem = new tsl::elm::ToggleListItem(optionName, false, "On", "Off");
                // Set the initial state of the toggle item
                bool toggleStateOn = isFileOrDirectory(preprocessPath(pathReplaceOn));
                
                toggleListItem->setState(toggleStateOn);

                toggleListItem->setStateChangedListener([toggleStateOn, command = option.second, this](bool state) {
                    if (!state) {
                        // Toggle switched to On
                        if (toggleStateOn) {
                            std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(command, pathReplaceOn, true);
                            interpretAndExecuteCommand(modifiedCommands);
                        } else {
                            // Handle the case where the command should only run in the source_on section
                            // Add your specific code here
                        }
                    } else {
                        // Toggle switched to Off
                        if (!toggleStateOn) {
                            std::vector<std::vector<std::string>> modifiedCommands = getModifyCommands(command, pathReplaceOff, true, false);
                            interpretAndExecuteCommand(modifiedCommands);
                        } else {
                            // Handle the case where the command should only run in the source_off section
                            // Add your specific code here
                        }
                    }
                });

                list->addItem(toggleListItem);
            }

        }

        // Package Info
        PackageHeader packageHeader = getPackageHeaderFromIni(subConfigIniPath);
        list->addItem(new tsl::elm::CategoryHeader("Package Info"));

        constexpr int lineHeight = 20;  // Adjust the line height as needed
        constexpr int xOffset = 120;    // Adjust the horizontal offset as needed
        constexpr int fontSize = 16;    // Adjust the font size as needed
        constexpr int numEntries = 1;   // Adjust the number of entries as needed

        list->addItem(new tsl::elm::CustomDrawer([lineHeight, xOffset, fontSize, packageHeader](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString("Creator\nVersion", false, x, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
            renderer->drawString((packageHeader.creator+"\n"+packageHeader.version).c_str(), false, x + xOffset, y + lineHeight, fontSize, a(tsl::style::color::ColorText));
        }), fontSize * numEntries + lineHeight);
        
        rootFrame->setContent(list);
        
        return rootFrame;
    }

    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (inSubMenu && (keysHeld & KEY_B)) {
            //tsl::Overlay::get()->close();
            svcSleepThread(300'000'000);
            tsl::goBack();
            inSubMenu = false;
            returningToMain = true;
            //tsl::Overlay::get()->close();
            return true;
        }
        
        if (returningToSub){
            returningToSub = false;
            inSubMenu = true;
        }
        
        return false;
        
        //return handleOverlayMenuInput(inSubMenu, keysHeld, KEY_B);
    }
};



// Main menu
class MainMenu : public tsl::Gui {
private:
    std::string directoryPath = "sdmc:/config/ultrahand/";
    std::string overlayDirectory = "sdmc:/switch/.overlays/";
    std::string configIniPath = directoryPath + configFileName;
    std::string fullPath, optionName;
    //bool inSubMenu = false; // Added boolean to track submenu state
    //bool inTextMenu = false;
public:
    MainMenu() {}

    virtual tsl::elm::Element* createUI() override {
        inMainMenu = true;
        
        auto rootFrame = new tsl::elm::OverlayFrame("Ultrahand", APP_VERSION);
        auto list = new tsl::elm::List();

        //loadOverlayFiles(list);
        
        int count = 0;

        // Load overlay files
        std::vector<std::string> overlayFiles;
        std::vector<std::string> files = getFilesListByWildcard(overlayDirectory+"*.ovl");
        for (const auto& file : files) {
            // Check if the file is an overlay file (*.ovl)
            if (file.substr(file.length() - 4) == ".ovl" && getNameFromPath(file) != "ovlmenu.ovl") {
                overlayFiles.push_back(file);
            }
        }
        std::sort(overlayFiles.begin(), overlayFiles.end()); // Sort overlay files alphabetically

        if (!overlayFiles.empty()) {
            
            for (const auto& overlayFile : overlayFiles) {
                if (getNameFromPath(overlayFile) == "ovlmenu.ovl")
                    continue;

                // Get the path of the overlay file
                //std::string overlayPath = overlayDirectory + "/" + overlayFile;

                // Get the name and version of the overlay file
                auto [result, overlayName, overlayVersion] = getOverlayInfo(overlayFile);
                if (result != ResultSuccess)
                    continue;

                // Create a new list item with the overlay name and version
                auto* listItem = new tsl::elm::ListItem(overlayName);
                listItem->setValue(overlayVersion, true);

                // Add a click listener to load the overlay when clicked upon
                listItem->setClickListener([overlayFile](s64 key) {
                    if (key & KEY_A) {
                        // Load the overlay here
                        inMainMenu = false;
                        inOverlay = true;
                        tsl::setNextOverlay(overlayFile);
                        //envSetNextLoad(overlayPath, "");
                        tsl::Overlay::get()->close();
                        //inMainMenu = true;
                        return true;
                    }
                    return false;
                });

                if (count == 0) {
                    list->addItem(new tsl::elm::CategoryHeader("Overlays"));
                }
                list->addItem(listItem);
                count++;
            }
        }
        
        
        // Create the directory if it doesn't exist
        createDirectory(directoryPath);

        // Load options from INI file
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(configIniPath, true);
        
        count = 0;
        // Load subdirectories
        std::vector<std::string> subdirectories = getSubdirectories(directoryPath);
        std::sort(subdirectories.begin(), subdirectories.end()); // Sort subdirectories alphabetically
        for (const auto& subdirectory : subdirectories) {
            std::string subdirectoryIcon = "";//"\u2605 "; // Use a folder icon (replace with the actual font icon)
            PackageHeader packageHeader = getPackageHeaderFromIni(directoryPath + subdirectory + "/"+ configFileName);
            
            if (count == 0) {
                // Add a section break with small text to indicate the "Packages" section
                list->addItem(new tsl::elm::CategoryHeader("Packages"));
            }
            
            auto listItem = new tsl::elm::ListItem(subdirectoryIcon + subdirectory);
            listItem->setValue(packageHeader.version, true);
            
            listItem->setClickListener([this, subPath = directoryPath + subdirectory](uint64_t keys) {
                if (keys & KEY_A) {
                    inMainMenu = false;
                    tsl::changeTo<SubMenu>(subPath);
                    
                    return true;
                }
                return false;
            });

            list->addItem(listItem);
            count++;
        }

        
        count = 0;
        //std::string optionName;
        // Populate the menu with options
        for (const auto& option : options) {
            optionName = option.first;
            
            // Check if it's a subdirectory
            fullPath = directoryPath + optionName;
            if (count == 0) {
                // Add a section break with small text to indicate the "Packages" section
                list->addItem(new tsl::elm::CategoryHeader("Commands"));
            }
            
            auto listItem = new tsl::elm::ListItem(optionName);

            listItem->setClickListener([this, command = option.second, subPath = optionName](uint64_t keys) {
                if (keys & KEY_A) {
                    // Check if it's a subdirectory
                    struct stat entryStat;
                    std::string newPath = directoryPath + subPath;
                    if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
                        inMainMenu = false;
                        tsl::changeTo<SubMenu>(newPath);
                    } else {
                        // Interpret and execute the command
                        interpretAndExecuteCommand(command);
                    }

                    return true;
                }
                return false;
            });

            list->addItem(listItem);
            count++;
        }


        rootFrame->setContent(list);

        return rootFrame;
    }

    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        
        if (inMainMenu && (keysHeld & KEY_B)) {
            inMainMenu = false;
            tsl::Overlay::get()->close();
            return true;
        }
        if (returningToMain){
            returningToMain = false;
            inMainMenu = true;
        }
        return false;
    }
};


// Overlay
class Overlay : public tsl::Overlay {
public:
    
    virtual void initServices() override {
        fsdevMountSdmc();
        splInitialize();
        spsmInitialize();
    }

    virtual void exitServices() override {
        spsmExit();
        splExit();
        fsdevUnmountAll();
    }

    virtual void onShow() override {}   // Called before overlay wants to change from invisible to visible state
    virtual void onHide() override {}   // Called before overlay wants to change from visible to invisible state

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainMenu>();  // Initial Gui to load. It's possible to pass arguments to its constructor like this
    }
};

int main(int argc, char* argv[]) {
    return tsl::loop<Overlay, tsl::impl::LaunchFlags::None>(argc, argv);
}
