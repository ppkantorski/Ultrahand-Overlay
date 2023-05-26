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

void moveFile(const std::string& fromFile, const std::string& toDirectory) {
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
}

void renameFile(const std::string& fileToRename, const std::string& newFileName) {
    std::string directoryPath = fileToRename.substr(0, fileToRename.find_last_of('/'));
    std::string newFilePath = directoryPath + "/" + newFileName;

    if (std::rename(fileToRename.c_str(), newFilePath.c_str()) == 0) {
        // Rename successful
    } else {
        // Error renaming the file
    }
}

 // Perform copy action from "fromFile" to "toDirectory"
void copyFile(const std::string& fromFile, const std::string& toDirectory) {
    struct stat fileInfo;
    if (stat(fromFile.c_str(), &fileInfo) == 0 && S_ISREG(fileInfo.st_mode)) {
        // Source file exists and is a regular file

        // Extract the source file name from the file path
        size_t lastSlashPos = fromFile.find_last_of('/');
        std::string fileName = (lastSlashPos != std::string::npos)
                                   ? fromFile.substr(lastSlashPos + 1)
                                   : fromFile;

        // Create the destination directory if it doesn't exist
        createDirectory(toDirectory);

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
}



void deleteFile(const std::string& fileToDelete) {
    if (std::remove(fileToDelete.c_str()) == 0) {
        // Delete successful
    } else {
        // Error deleting the file
    }
}


// Trim leading and trailing whitespaces from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    if (first == std::string::npos || last == std::string::npos) {
        return "";
    }
    return str.substr(first, last - first + 1);
}

// Check if a string starts with a given prefix
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

std::string removeQuotes(const std::string& str) {
    std::size_t firstQuote = str.find_first_of('\'');
    std::size_t lastQuote = str.find_last_of('\'');
    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
        return str.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }
    return str;
}

void parseIniFile(const std::string& fileToParse, const std::string& desiredSection,
                  const std::string& desiredKey, const std::string& desiredValue) {
    FILE* configFile = fopen(fileToParse.c_str(), "r");
    if (!configFile) {
        //printf("Failed to open the INI file.\n");
        return;
    }
    std::string updatedDesiredSection = removeQuotes(desiredSection);
    
    char line[256];
    //std::string logPath = fileToParse + ".log";
    std::string tempPath = fileToParse + ".tmp";

    //FILE* logFile = fopen(logPath.c_str(), "w");
    
    FILE* tempFile = fopen(tempPath.c_str(), "w");
    
    
    //fprintf(logFile, "%s\n",updatedDesiredSection.c_str());
    //fprintf(logFile, "%s\n",desiredKey.c_str());
    //fprintf(logFile, "%s\n",desiredValue.c_str());
   
    
    if (tempFile) {
        std::string currentSection;

        while (fgets(line, sizeof(line), configFile)) {
            // Check if the line represents a section
            if (line[0] == '[' && line[strlen(line) - 2] == ']') {
                currentSection = removeQuotes(trim(std::string(line + 1, strlen(line) - 3)));
            }
            
            //fprintf(logFile, "test 1 %s\n", desiredKey.c_str());
            //fprintf(logFile, "test 2 %s\n", desiredValue.c_str());
            
            // Check if the line is in the desired section
            if (currentSection == updatedDesiredSection) {
                // Check if the line starts with the desired key
                if (startsWith(line, desiredKey + "=")) {
                    // Overwrite the value with the desired value
                    fprintf(tempFile, "%s=%s\n", desiredKey.c_str(), desiredValue.c_str());
                    continue;  // Skip writing the original line
                }
            }
    
            //if (!updatedDesiredSection.empty()) {
            //    fprintf(tempFile, "[%s]\n", updatedDesiredSection.c_str());
            //    updatedDesiredSection.clear(); // Only write the section name once
            //}
    
            fprintf(tempFile, "%s", line);
        }
        
        //fclose(logFile);
        fclose(configFile);
        fclose(tempFile);
        remove(fileToParse.c_str());  // Delete the old configuration file
        rename(tempPath.c_str(), fileToParse.c_str());  // Rename the temp file to the original name

        //printf("INI file updated successfully.\n");
    } else {
        //printf("Failed to create temporary file.\n");
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

        if (commandName == "make" || commandName == "mkdir") {
            std::string toDirectory = "sdmc:" + command[1];
            createDirectory(toDirectory);

            // Perform actions based on the command name
        } else if (commandName == "copy" || commandName == "cp") {
            // Copy command
            if (command.size() >= 3) {
                std::string fromPath = "sdmc:" + command[1];
                std::string toPath = "sdmc:" + command[2];
                copyFile(fromPath, toPath);
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "delete" || commandName == "del") {
            // Delete command
            if (command.size() >= 2) {
                std::string fileToDelete = "sdmc:" + command[1];
                deleteFile(fileToDelete);
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "move") {
            // Move command
            if (command.size() >= 3) {
                std::string fromFile = "sdmc:" + command[1];
                std::string toDirectory = "sdmc:" + command[2];
                moveFile(fromFile, toDirectory);
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "rename" || commandName == "mv") {
            // Rename command
            if (command.size() >= 3) {
                std::string fileToRename = "sdmc:" + command[1];
                std::string newFileName = "sdmc:" + command[2];
                renameFile(fileToRename, newFileName);
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "parse-ini") {
            // Parse command
            if (command.size() >= 5) {
                std::string fileToParse = "sdmc:" + command[1];

                std::string desiredSection;
                for (size_t i = 2; i < command.size() - 2; ++i) {
                    desiredSection += command[i];
                    if (i < command.size() - 3) {
                        desiredSection += " ";
                    }
                }

                std::string desiredKey = command[command.size() - 2];
                std::string desiredValue = command[command.size() - 1];
                
                parseIniFile(fileToParse.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
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
        fprintf(configFileOut, "[make directories]\n"
                               "mkdir /config/ultrahand/example1/\n"
                               "mkdir /config/ultrahand/example2/\n"
                               "[copy files]\n"
                               "copy /config/ultrahand/config.ini /config/ultrahand/example1/\n"
                               "copy /config/ultrahand/config.ini /config/ultrahand/example2/\n"
                               "[rename files]\n"
                               "rename /config/ultrahand/example1/config.ini /config/ultrahand/example1/configX.ini\n"
                               "rename /config/ultrahand/example2/config.ini /config/ultrahand/example2/configX.ini\n"
                               "[move directories]\n"
                               "move /config/ultrahand/example1/ /config/ultrahand/example3/\n"
                               "move /config/ultrahand/example2/ /config/ultrahand/example4/\n"
                               "[delete files]\n"
                               "delete /config/ultrahand/example1/config.ini\n"
                               "delete /config/ultrahand/example2/config.ini\n"
                               "[delete directories]\n"
                               "delete /config/ultrahand/example1/\n"
                               "delete /config/ultrahand/example2/\n"
                               "delete /config/ultrahand/example3/\n"
                               "delete /config/ultrahand/example4/\n"
                               "[parse ini file]\n"
                               "copy /bootloader/hekate_ipl.ini /config/ultrahand/\n"
                               "parse-ini /config/ultrahand/hekate_ipl.ini 'L4T Ubuntu Bionic' r2p_action working");
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
    std::string directoryPath = "sdmc:/config/ultrahand/";
    std::string configIniPath = directoryPath + "config.ini";

public:
    MainMenu() {}

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame("Ultrahand", APP_VERSION);
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
