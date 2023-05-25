#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <sys/stat.h>

Result Hinted = 1;

// Function to create a directory if it doesn't exist
void createDirectory(const std::string& directoryPath) {
    struct stat st;
    if (stat(directoryPath.c_str(), &st) != 0) {
        mkdir(directoryPath.c_str(), 0777);
    }
}

void interpretAndExecuteCommand(const std::vector<std::vector<std::string>>& commands) {
    for (const auto& command : commands) {
        // Check the command and perform the appropriate action
        if (command.empty()) {
            // Empty command, do nothing
            continue;
        }

        // Get the command name (first part of the command)
        std::string commandName = command[0];

        if (commandName == "mkdir") {
            std::string toDirectory = "sdmc:" + command[1];
            createDirectory(toDirectory);

            // Perform actions based on the command name
        } else if (commandName == "copy") {
            // Copy command
            if (command.size() >= 3) {
                std::string fromFile = "sdmc:" + command[1];
                std::string toDirectory = "sdmc:" + command[2];
                // Perform copy action from "fromFile" to "toDirectory"

                struct stat fileInfo;
                if (stat(fromFile.c_str(), &fileInfo) == 0 && S_ISREG(fileInfo.st_mode)) {
                    // Source file exists and is a regular file

                    // Extract the source file name from the file path
                    size_t lastSlashPos = fromFile.find_last_of('/');
                    std::string fileName = (lastSlashPos != std::string::npos)
                                               ? fromFile.substr(lastSlashPos + 1)
                                               : fromFile;

                    // Create the destination file path
                    std::string toFile = toDirectory + "/" + fileName;

                    FILE* srcFile = fopen(fromFile.c_str(), "rb");
                    FILE* destFile = fopen(toFile.c_str(), "wb");
                    if (srcFile && destFile) {
                        char buffer[1024];
                        size_t bytesRead;
                        while ((bytesRead = fread(buffer, 1, sizeof(buffer), srcFile)) > 0) {
                            fwrite(buffer, 1, bytesRead, destFile);
                        }
                        fclose(srcFile);
                        fclose(destFile);
                        // Copy successful
                    } else {
                        // Error opening files or performing copy action
                    }
                } else {
                    // Source file doesn't exist or is not a regular file
                }
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "delete") {
            // Delete command
            if (command.size() >= 2) {
                std::string fileToDelete = "sdmc:" + command[1];
                // Perform delete action for "fileToDelete"
                if (std::remove(fileToDelete.c_str()) == 0) {
                    // Delete successful
                } else {
                    // Error deleting the file
                }
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "move") {
            // Move command
            if (command.size() >= 3) {
                std::string fromFile = "sdmc:" + command[1];
                std::string toDirectory = "sdmc:" + command[2];
                // Perform move action from "fromFile" to "toDirectory"

                struct stat fileInfo;
                if (stat(fromFile.c_str(), &fileInfo) == 0 && S_ISREG(fileInfo.st_mode)) {
                    // Source file exists and is a regular file

                    // Extract the source file name from the file path
                    size_t lastSlashPos = fromFile.find_last_of('/');
                    std::string fileName = (lastSlashPos != std::string::npos)
                                               ? fromFile.substr(lastSlashPos + 1)
                                               : fromFile;

                    // Create the destination file path
                    std::string toFile = toDirectory + "/" + fileName;

                    if (std::rename(fromFile.c_str(), toFile.c_str()) == 0) {
                        // Move successful
                    } else {
                        // Error moving the file
                    }
                } else {
                    // Source file doesn't exist or is not a regular file
                }
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else {
            // Unknown command, do nothing or display an error message
            // ...
        }
    }
}

std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& configIniPath) {
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;

    FILE* configFile = fopen(configIniPath.c_str(), "r");
    if (!configFile) {
        // Write the default INI file
        FILE* configFileOut = fopen(configIniPath.c_str(), "w");
        fprintf(configFileOut, "[make directory]\nmkdir /config/ultra-hand/test/\nmkdir /config/ultra-hand/test2/\n[copy function]\ncopy /config/ultra-hand/config.ini /config/\ncopy /config/ultra-hand/config.ini /\n[delete function]\ndelete /config/config.ini\ndelete /config.ini");
        fclose(configFileOut);
        configFile = fopen(configIniPath.c_str(), "r");
    }

    char line[256];
    std::string currentOption;
    std::vector<std::vector<std::string>> commands;

    while (fgets(line, sizeof(line), configFile)) {
        std::string trimmedLine = line;
        trimmedLine.erase(trimmedLine.find_last_not_of("\r\n") + 1);  // Remove trailing newline character

        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            // Skip empty lines and comment lines
            continue;
        } else if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            // New option section
            if (!currentOption.empty()) {
                // Store previous option and its commands
                options.emplace_back(std::move(currentOption), std::move(commands));
                commands.clear();
            }
            currentOption = trimmedLine.substr(1, trimmedLine.size() - 2);  // Extract option name
        } else {
            // Command line
            std::istringstream iss(trimmedLine);
            std::vector<std::string> commandParts;
            std::string part;
            while (iss >> part) {
                commandParts.push_back(part);
            }
            commands.push_back(std::move(commandParts));
        }
    }

    // Store the last option and its commands
    if (!currentOption.empty()) {
        options.emplace_back(std::move(currentOption), std::move(commands));
    }

    fclose(configFile);
    return options;
}

// Main menu
class MainMenu : public tsl::Gui {
private:
    std::string directoryPath = "sdmc:/config/ultra-hand/";
    std::string configIniPath = directoryPath + "config.ini";

public:
    MainMenu() {}

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame("Ultra Hand", APP_VERSION);
        auto list = new tsl::elm::List(6);

        // Create the directory if it doesn't exist
        createDirectory(directoryPath);

        // Load options from INI file
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options = loadOptionsFromIni(configIniPath);

        // Populate the menu with options
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
            tsl::goBack();
            return true;
        }
        return false;
    }
};

// Overlay
class Overlay : public tsl::Overlay {
public:
    virtual void initServices() override {
        //Initialize services
        tsl::hlp::doWithSmSession([this]{});
        Hinted = envIsSyscallHinted(0x6F);
    }

    virtual void exitServices() override {
        //CloseThreads();
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
