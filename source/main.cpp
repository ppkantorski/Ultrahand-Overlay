#define NDEBUG
#define STBTT_STATIC
#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <utils.hpp>
#include <sys/stat.h>


// Overlay booleans
bool inSubMenu = false;
bool inConfigMenu = false;


// Config overlay 
class ConfigOverlay : public tsl::Gui {
private:
    std::string filePath;
    std::string specificKey;

public:
    ConfigOverlay(const std::string& file, const std::string& key = "") : filePath(file), specificKey(key) {}

    virtual tsl::elm::Element* createUI() override {
        inConfigMenu = true;
        
        auto rootFrame = new tsl::elm::OverlayFrame(getFolderName(filePath), "Ultrahand Config");
        auto list = new tsl::elm::List();

        std::string configFile = filePath + "/config.ini";

        std::string fileContent = readFileContent(configFile);
        if (!fileContent.empty()) {
            std::string line;
            std::istringstream iss(fileContent);
            std::string currentCategory;
            bool isInSection = false;
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
                } else {
                    if (isInSection) {
                        auto listItem = new tsl::elm::ListItem(line);
                        listItem->setClickListener([line, this](uint64_t keys) {
                            if (keys & KEY_A) {
                                std::istringstream iss(line);
                                std::vector<std::string> commandParts;
                                std::string part;
                                bool inQuotes = false;
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
                                std::string commandName = commandParts[0];
                                std::vector<std::vector<std::string>> commandVec;
                                commandVec.emplace_back(commandParts);
                                interpretAndExecuteCommand(commandVec);
                                return true;
                            }
                            return false;
                        });
                        list->addItem(listItem);
                    }
                }
            }
        } else {
            list->addItem(new tsl::elm::ListItem("Failed to open file: " + configFile));
        }

        rootFrame->setContent(list);
        return rootFrame;
    }

    virtual bool handleInput(u64 keysDown, u64 keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (keysHeld & KEY_B) {
            svcSleepThread(300'000'000);
            tsl::goBack();
            return true;
        }
        return false;
    }
};






// Sub menu
class SubMenu : public tsl::Gui {
private:
    std::string subPath;

public:
    SubMenu(const std::string& path) : subPath(path) {}

    virtual tsl::elm::Element* createUI() override {
        inSubMenu = true;
        
        auto rootFrame = new tsl::elm::OverlayFrame(getFolderName(subPath), "Ultrahand Package");
        auto list = new tsl::elm::List();

        // Add a section break with small text to indicate the "Packages" section
        list->addItem(new tsl::elm::CategoryHeader("Commands"));

        // Load options from INI file in the subdirectory
        std::string subConfigIniPath = subPath + "/config.ini";
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(subConfigIniPath);

        // Populate the sub menu with options
        for (const auto& option : options) {
            auto listItem = new tsl::elm::ListItem(option.first);

            listItem->setClickListener([command = option.second, keyName = option.first, subPath = this->subPath](uint64_t keys) {
                if (keys & KEY_A) {
                    // Interpret and execute the command
                    interpretAndExecuteCommand(command);
                    return true;
                } else if (keys & KEY_X) {
                    //inSubMenu = true; // Set boolean to true when entering a submenu
                    inSubMenu = true;
                    tsl::changeTo<ConfigOverlay>(subPath, keyName);
                    return true;
                }
                return false;
            });

            list->addItem(listItem);
        }

        rootFrame->setContent(list);

        return rootFrame;
    }

    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (!inConfigMenu & (keysHeld & KEY_B)) {
            svcSleepThread(300'000'000);
            tsl::goBack();
            return true;
        }
        
        return false;
    }
};


// Main menu
class MainMenu : public tsl::Gui {
private:
    std::string directoryPath = "sdmc:/config/ultrahand/";
    std::string configIniPath = directoryPath + "config.ini";
    std::string fullPath;
    //bool inSubMenu = false; // Added boolean to track submenu state
    //bool inTextMenu = false;
public:
    MainMenu() {}

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame("Ultrahand", APP_VERSION);
        auto list = new tsl::elm::List();

        // Add a section break with small text to indicate the "Packages" section
        list->addItem(new tsl::elm::CategoryHeader("Packages"));

        // Create the directory if it doesn't exist
        createDirectory(directoryPath);

        // Load options from INI file
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(configIniPath);

        // Load subdirectories
        std::vector<std::string> subdirectories = getSubdirectories(directoryPath);
        std::sort(subdirectories.begin(), subdirectories.end()); // Sort subdirectories alphabetically
        for (const auto& subdirectory : subdirectories) {
            std::string subdirectoryIcon = "";//"\u2605 "; // Use a folder icon (replace with the actual font icon)
            std::string versionLabel = getVersionFromIni(directoryPath+subdirectory+"/config.ini");
            auto listItem = new tsl::elm::ListItem(subdirectoryIcon + subdirectory, versionLabel);

            listItem->setClickListener([this, subPath = directoryPath + subdirectory](uint64_t keys) {
                if (keys & KEY_A) {
                    //inSubMenu = true; // Set boolean to true when entering a submenu
                    tsl::changeTo<SubMenu>(subPath);
                    
                    
                    return true;
                }
                else if (keys & KEY_X) {
                    //inSubMenu = true; // Set boolean to true when entering a submenu
                    //inTextMenu = true; // Set boolean to true when entering a submenu
                    tsl::changeTo<ConfigOverlay>(subPath);
                    
                    
                    return true;
                }
                return false;
            });

            list->addItem(listItem);
        }
        

        // Add a section break with small text to indicate the "Packages" section
        list->addItem(new tsl::elm::CategoryHeader("Commands"));

        // Populate the menu with options
        for (const auto& option : options) {
            std::string optionName = option.first;
            std::string optionIcon;

            // Check if it's a subdirectory
            //struct stat entryStat;
            fullPath = directoryPath + optionName;
            //if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
            //    optionIcon = "+ "; // Use a folder icon (replace with the actual font icon)
            //} else {
            //    optionIcon = "";
            //    //optionIcon = "\uE001"; // Use a command icon (replace with the actual font icon)
            //}
            optionIcon = "";
            auto listItem = new tsl::elm::ListItem(optionIcon + " " + optionName);

            listItem->setClickListener([this, command = option.second, subPath = optionName](uint64_t keys) {
                if (keys & KEY_A) {
                    // Check if it's a subdirectory
                    struct stat entryStat;
                    std::string newPath = directoryPath + subPath;
                    if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
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
        }


        rootFrame->setContent(list);

        return rootFrame;
    }

    virtual bool handleInput(uint64_t keysDown, uint64_t keysHeld, touchPosition touchInput, JoystickPosition leftJoyStick, JoystickPosition rightJoyStick) override {
        if (!inSubMenu & (!inConfigMenu & (keysHeld & KEY_B))) {
            // Only go back if not in a submenu and B button is held

            //svcSleepThread(300'000'000);
            tsl::goBack();
            return true;
        }

        if (inSubMenu & !inConfigMenu & (keysHeld & KEY_B)) {
            // Reset the submenu boolean if in a submenu and A button is released
            svcSleepThread(300'000'000);
            inSubMenu = false;
        }
        
        if (inConfigMenu & (keysHeld & KEY_B)) {
            // Reset the submenu boolean if in a submenu and A button is released
            svcSleepThread(300'000'000);
            inConfigMenu = false;
        }
        
        return false;
    }
};



// Overlay
class Overlay : public tsl::Overlay {
public:
    virtual void initServices() override {
        // Initialize services
        //tsl::hlp::doWithSmSession([this]{});
        fsdevMountSdmc();
        splInitialize();
        spsmInitialize();
    }

    //virtual void closeThreads() override {
    //    // CloseThreads();
    //}
    
    virtual void exitServices() override {
        spsmExit();
        splExit();
        fsdevUnmountAll();
    }

    virtual void onShow() override {}    // Called before overlay wants to change from invisible to visible state
    virtual void onHide() override {}    // Called before overlay wants to change from visible to invisible state

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override {
        return initially<MainMenu>();  // Initial Gui to load. It's possible to pass arguments to its constructor like this
    }
};

int main(int argc, char* argv[]) {
    return tsl::loop<Overlay>(argc, argv);
}

