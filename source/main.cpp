#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include <sys/stat.h>
#include <dirent.h>
#include <ctime>  // Add this include for time functions

// C-Library commands
//extern "C" {
//    Result spsmInitialize(void);
//    Result spsmShutdown(bool reboot);
//}


// For loggging messages and debugging
void logMessage(const std::string& message) {
    std::time_t currentTime = std::time(nullptr);
    std::string logEntry = std::asctime(std::localtime(&currentTime));
    logEntry += message;

    FILE* file = fopen("sdmc:/config/ultrahand/log.txt", "a");
    if (file != nullptr) {
        fputs(logEntry.c_str(), file);
        fclose(file);
    }
}


void reboot() {
    logMessage("Rebooting...\n");
    // Implement the reboot functionality here
    spsmInitialize();
    spsmShutdown(true);
    logMessage("Reboot failed..\n");
}

void shutdown() {
    logMessage("Shutting down...\n");
    // Implement the shutdown functionality here
    spsmInitialize();
    spsmShutdown(false);
    logMessage("Shutdown failed..\n");
}




//Result Hinted = 1;

std::vector<std::string> getSubdirectories(const std::string& directoryPath) {
    std::vector<std::string> subdirectories;

    DIR* dir = opendir(directoryPath.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string entryName = entry->d_name;

            // Exclude current directory (.) and parent directory (..)
            if (entryName != "." && entryName != "..") {
                struct stat entryStat;
                std::string fullPath = directoryPath + "/" + entryName;

                if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
                    subdirectories.push_back(entryName);
                }
            }
        }

        closedir(dir);
    }

    return subdirectories;
}

// Function to create a directory if it doesn't exist
void createDirectory(const std::string& directoryPath) {
    struct stat st;
    if (stat(directoryPath.c_str(), &st) != 0) {
        mkdir(directoryPath.c_str(), 0777);
    }
}

void renameFileOrDirectory(const std::string& sourcePath, const std::string& destinationPath) {
    struct stat fileInfo;
    if (stat(sourcePath.c_str(), &fileInfo) == 0) {
        // Source file or directory exists

        if (S_ISREG(fileInfo.st_mode)) {
            // Source path is a regular file

            // Extract the source file name from the path
            size_t lastSlashPos = sourcePath.find_last_of('/');
            std::string fileName = (lastSlashPos != std::string::npos)
                                       ? sourcePath.substr(lastSlashPos + 1)
                                       : sourcePath;

            // Create the destination file path
            std::string destinationFile = destinationPath + "/" + fileName;

            if (std::rename(sourcePath.c_str(), destinationFile.c_str()) == 0) {
                // Rename successful
            } else {
                // Error renaming the file
            }
        } else if (S_ISDIR(fileInfo.st_mode)) {
            // Source path is a directory

            if (std::rename(sourcePath.c_str(), destinationPath.c_str()) == 0) {
                // Rename successful
            } else {
                // Error renaming the directory
            }
        } else {
            // Source path is neither a file nor a directory
        }
    } else {
        // Source path doesn't exist
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

        // Check if the destination file name is already present in the toDirectory
        size_t fileNamePos = toDirectory.find(fileName);
        if (fileNamePos != std::string::npos) {
            // Destination file name is already present in the toDirectory
            // Perform renaming by adding a suffix to the file name
            std::string baseFileName = fileName.substr(0, fileName.find_last_of('.'));
            std::string fileExtension = fileName.substr(fileName.find_last_of('.'));
            std::string newFileName = baseFileName + "_copy" + fileExtension;

            // Create the destination file path using the new file name
            std::string toFile = toDirectory + "/" + newFileName;

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
                // Copy successful with renaming
            } else {
                // Error opening files or performing copy action
            }
        } else {
            // Destination file name not found in the toDirectory
            // Create the destination file path using the original file name
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
                // Copy successful without renaming
            } else {
                // Error opening files or performing copy action
            }
        }
    } else {
        // Source file doesn't exist or is not a regular file
    }
}




bool isDirectory(const std::string& path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);
    }
    return false;
}

bool deleteDirectory(const std::string& path) {
    if (!isDirectory(path)) {
        // Not a directory
        return false;
    }

    DIR* dir = opendir(path.c_str());
    if (dir == nullptr) {
        // Error opening directory
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string entryName = entry->d_name;
        if (entryName != "." && entryName != "..") {
            std::string entryPath = path + "/" + entryName;
            if (isDirectory(entryPath)) {
                if (!deleteDirectory(entryPath)) {
                    // Error deleting subdirectory
                    closedir(dir);
                    return false;
                }
            } else {
                if (std::remove(entryPath.c_str()) != 0) {
                    // Error deleting file
                    closedir(dir);
                    return false;
                }
            }
        }
    }

    closedir(dir);

    if (rmdir(path.c_str()) == 0) {
        // Deletion successful
        return true;
    }

    return false;
}

void deleteFile(const std::string& pathToDelete) {
    struct stat pathStat;
    if (stat(pathToDelete.c_str(), &pathStat) == 0) {
        if (S_ISREG(pathStat.st_mode)) {
            if (std::remove(pathToDelete.c_str()) == 0) {
                // Deletion successful
            } else {
                // Error deleting the file
            }
        } else if (S_ISDIR(pathStat.st_mode)) {
            if (deleteDirectory(pathToDelete)) {
                // Deletion successful
            } else {
                // Error deleting the directory
            }
        } else {
            // Unsupported file type or invalid path
        }
    } else {
        // Error accessing the file/directory
    }
}


// Trim leading and trailing whitespaces from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

// Check if a string starts with a given prefix
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
}

std::string removeQuotes(const std::string& str) {
    std::size_t firstQuote = str.find_first_of('\'');
    std::size_t lastQuote = str.find_last_of('\'');
    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
        return str.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }
    return str;
}

void editIniFile(const std::string& fileToParse, const std::string& desiredSection,
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
        } else if (commandName == "rename" || commandName == "move" || commandName == "mv") {
            // Rename command
            if (command.size() >= 3) {
                std::string pathToRename = "sdmc:" + command[1];
                std::string newPathName = "sdmc:" + command[2];
                renameFileOrDirectory(pathToRename, newPathName);
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "edit-ini") {
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
                
                editIniFile(fileToParse.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
            } else {
                // Invalid command format, display an error message or handle it accordingly
                // ...
            }
        } else if (commandName == "reboot") {
            // Reboot command
            reboot();
        } else if (commandName == "shutdown") {
            // Reboot command
            shutdown();
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
                               "mv /config/ultrahand/example1/config.ini /config/ultrahand/example1/configX.ini\n"
                               "mv /config/ultrahand/example2/config.ini /config/ultrahand/example2/configX.ini\n"
                               "[rename directories]\n"
                               "mv /config/ultrahand/example1/ /config/ultrahand/example3/\n"
                               "mv /config/ultrahand/example2/ /config/ultrahand/example4/\n"
                               "[delete files]\n"
                               "delete /config/ultrahand/example1/config.ini\n"
                               "delete /config/ultrahand/example2/config.ini\n"
                               "[delete directories]\n"
                               "delete /config/ultrahand/example1/\n"
                               "delete /config/ultrahand/example2/\n"
                               "delete /config/ultrahand/example3/\n"
                               "delete /config/ultrahand/example4/\n"
                               "[edit ini file]\n"
                               "copy /bootloader/hekate_ipl.ini /config/ultrahand/\n"
                               "edit-ini /config/ultrahand/hekate_ipl.ini 'L4T Ubuntu Bionic' r2p_action working\n"
                               "[shutdown]\n"
                               "shutdown\n"
                               "[reboot]\n"
                               "reboot");
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

std::string getFolderName(const std::string& path) {
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos && lastSlash + 1 < path.length()) {
        return path.substr(lastSlash + 1);
    }
    return path;
}

// Sub menu
class SubMenu : public tsl::Gui {
private:
    std::string subPath;

public:
    SubMenu(const std::string& path) : subPath(path) {}

    virtual tsl::elm::Element* createUI() override {
        auto rootFrame = new tsl::elm::OverlayFrame(getFolderName(subPath), "Ultrahand Package");
        auto list = new tsl::elm::List();

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
    bool inSubmenu = false; // Added boolean to track submenu state

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
                    
                    inSubmenu = true; // Set boolean to true when entering a submenu
                    
                    return true;
                }
                return false;
            });

            list->addItem(listItem);
        }
        
        // create a section break on the end  list that is drawn in with smmall text stating "Commands"
        
        // create a section break on the list that is drawn in with smmall text stating "Commands"

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
        if (!inSubmenu && (keysHeld & KEY_B)) {
            // Only go back if not in a submenu and B button is held

            //svcSleepThread(900'000'000);
            tsl::goBack();
            return true;
        }

        if (inSubmenu && (keysHeld & KEY_B)) {
            // Reset the submenu boolean if in a submenu and A button is released
            svcSleepThread(300'000'000);
            inSubmenu = false;
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

