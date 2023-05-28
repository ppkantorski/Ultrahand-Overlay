#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <utils.hpp>


// Sub menu
class SubMenu : public tsl::Gui {
private:
    std::string subPath;

public:
    SubMenu(const std::string& path) : subPath(path) {}

    virtual tsl::elm::Element* createUI() override {
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

            listItem->setClickListener([command = option.second](uint64_t keys) {
                if (keys & KEY_A) {
                    // Interpret and execute the command
                    interpretAndExecuteCommand(command);
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
        if (keysHeld & KEY_B) {
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
    bool inSubMenu = false; // Added boolean to track submenu state

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
            std::string subdirectoryIcon = "\u2605"; // Use a folder icon (replace with the actual font icon)
            auto listItem = new tsl::elm::ListItem(subdirectoryIcon + " " + subdirectory);

            listItem->setClickListener([this, subPath = directoryPath + subdirectory](uint64_t keys) {
                if (keys & KEY_A) {
                    tsl::changeTo<SubMenu>(subPath);
                    
                    inSubMenu = true; // Set boolean to true when entering a submenu
                    
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
            struct stat entryStat;
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
                    fullPath = directoryPath + subPath;
                    if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
                        tsl::changeTo<SubMenu>(fullPath);
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
        if (!inSubMenu && (keysHeld & KEY_B)) {
            // Only go back if not in a submenu and B button is held

            //svcSleepThread(900'000'000);
            tsl::goBack();
            return true;
        }

        if (inSubMenu && (keysHeld & KEY_B)) {
            // Reset the submenu boolean if in a submenu and A button is released
            svcSleepThread(300'000'000);
            inSubMenu = false;
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
        //Hinted = envIsSyscallHinted(0x6F);
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

