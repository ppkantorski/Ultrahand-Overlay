#pragma once
#include <switch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
//#include <payload.hpp>

#define SpsmShutdownMode_Normal 0
#define SpsmShutdownMode_Reboot 1

// For loggging messages and debugging
//void logMessage(const std::string& message) {
//    std::time_t currentTime = std::time(nullptr);
//    std::string logEntry = std::asctime(std::localtime(&currentTime));
//    logEntry += message+"\n";
//
//    FILE* file = fopen("sdmc:/config/ultrahand/log.txt", "a");
//    if (file != nullptr) {
//        fputs(logEntry.c_str(), file);
//        fclose(file);
//    }
//}


// Function to read the content of a file
std::string readFileContent(const std::string& filePath) {
    std::string content;
    FILE* file = fopen(filePath.c_str(), "rb");
    if (file) {
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0 && fileInfo.st_size > 0) {
            content.resize(fileInfo.st_size);
            fread(&content[0], 1, fileInfo.st_size, file);
        }
        fclose(file);
    }
    return content;
}


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




bool moveFileOrDirectory(const std::string& sourcePath, const std::string& destinationPath) {
    struct stat sourceInfo;
    struct stat destinationInfo;
    
    if (stat(sourcePath.c_str(), &sourceInfo) == 0) {
        // Source file or directory exists
        
        // Check if the destination path exists
        bool destinationExists = (stat(destinationPath.c_str(), &destinationInfo) == 0);
        
        if (S_ISREG(sourceInfo.st_mode)) {
            // Source path is a regular file
            
            if (destinationExists && S_ISDIR(destinationInfo.st_mode)) {
                // Destination path is a directory, move the file into the directory
                
                // Extract the source file name from the path
                size_t lastSlashPos = sourcePath.find_last_of('/');
                std::string fileName = (lastSlashPos != std::string::npos)
                                           ? sourcePath.substr(lastSlashPos + 1)
                                           : sourcePath;
                                           
                // Create the destination file path
                std::string destinationFile = destinationPath + "/" + fileName;
                
                if (std::rename(sourcePath.c_str(), destinationFile.c_str()) == 0) {
                    // Rename successful
                    return true;
                }
            } else if (!destinationExists && std::rename(sourcePath.c_str(), destinationPath.c_str()) == 0) {
                // Destination path doesn't exist, rename the source file accordingly
                return true;
            }
        } else if (S_ISDIR(sourceInfo.st_mode)) {
            // Source path is a directory
            
            if (destinationExists && S_ISDIR(destinationInfo.st_mode)) {
                // Destination path is a directory, rename the source directory to the destination directory
                
                if (std::rename(sourcePath.c_str(), destinationPath.c_str()) == 0) {
                    // Rename successful
                    return true;
                }
            }
        }
    }
    
    return false;  // Rename unsuccessful or source file/directory doesn't exist
}


bool moveFilesByPattern(const std::string& sourcePathPattern, const std::string& destinationPath) {
    std::string sourceDirectory;
    std::string pattern;

    // Separate the directory path and pattern
    size_t lastSlashPos = sourcePathPattern.find_last_of('/');
    if (lastSlashPos != std::string::npos) {
        sourceDirectory = sourcePathPattern.substr(0, lastSlashPos);
        pattern = sourcePathPattern.substr(lastSlashPos + 1);
    } else {
        sourceDirectory = ".";
        pattern = sourcePathPattern;
    }

    // Open the source directory
    DIR* dir = opendir(sourceDirectory.c_str());
    if (dir == nullptr) {
        return false;  // Failed to open source directory
    }

    // Iterate through the directory entries
    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename != "." && filename != "..") {
            // Check if the filename matches the pattern
            if (fnmatch(pattern.c_str(), filename.c_str(), 0) == 0) {
                std::string sourceFile = sourceDirectory + "/" + filename;
                std::string destinationFile = destinationPath + "/" + filename;

                if (std::rename(sourceFile.c_str(), destinationFile.c_str()) != 0) {
                    closedir(dir);
                    return false;  // Failed to move file
                }
            }
        }
    }

    closedir(dir);
    return true;  // All files matching the pattern moved successfully
}






 // Perform copy action from "fromFile" to file or directory
void copyFile(const std::string& fromFile, const std::string& toFileOrDirectory);

void copySingleFile(const std::string& fromFile, const std::string& toFile) {
    FILE* srcFile = fopen(fromFile.c_str(), "rb");
    FILE* destFile = fopen(toFile.c_str(), "wb");
    if (srcFile && destFile) {
        const size_t bufferSize = 131072; // Increase buffer size to 128 KB
        char buffer[bufferSize];
        size_t bytesRead;

        while ((bytesRead = fread(buffer, 1, bufferSize, srcFile)) > 0) {
            fwrite(buffer, 1, bytesRead, destFile);
        }

        fclose(srcFile);
        fclose(destFile);
    } else {
        // Error opening files or performing copy action.
        // Handle the error accordingly.
    }
}

void copyFile(const std::string& fromFile, const std::string& toFileOrDirectory) {
    struct stat fromFileInfo;
    if (stat(fromFile.c_str(), &fromFileInfo) == 0) {
        if (S_ISREG(fromFileInfo.st_mode)) {
            // Source is a regular file
            struct stat toFileInfo;
            if (stat(toFileOrDirectory.c_str(), &toFileInfo) == 0 && S_ISDIR(toFileInfo.st_mode)) {
                // Destination is a directory
                std::string fileName = fromFile.substr(fromFile.find_last_of('/') + 1);
                std::string toFilePath = toFileOrDirectory + "/" + fileName;

                // Check if the destination file exists and remove it
                if (stat(toFilePath.c_str(), &toFileInfo) == 0 && S_ISREG(toFileInfo.st_mode)) {
                    std::remove(toFilePath.c_str());
                }

                copySingleFile(fromFile, toFilePath);
            } else {
                // Destination is a file or doesn't exist
                // Check if the destination file exists and remove it
                if (stat(toFileOrDirectory.c_str(), &toFileInfo) == 0 && S_ISREG(toFileInfo.st_mode)) {
                    std::remove(toFileOrDirectory.c_str());
                }

                copySingleFile(fromFile, toFileOrDirectory);
            }
        } else if (S_ISDIR(fromFileInfo.st_mode)) {
            // Source is a directory
            struct stat toFileInfo;
            if (stat(toFileOrDirectory.c_str(), &toFileInfo) == 0 && S_ISDIR(toFileInfo.st_mode)) {
                // Destination is a directory
                std::string dirName = fromFile.substr(fromFile.find_last_of('/') + 1);
                std::string toDirPath = toFileOrDirectory + "/" + dirName;

                // Create the destination directory
                mkdir(toDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

                // Open the source directory
                DIR* dir = opendir(fromFile.c_str());
                if (dir != nullptr) {
                    dirent* entry;
                    while ((entry = readdir(dir)) != nullptr) {
                        std::string fileName = entry->d_name;
                        if (fileName != "." && fileName != "..") {
                            std::string fromFilePath = fromFile + "/" + fileName;
                            std::string toFilePath = toDirPath;
                            copyFile(fromFilePath, toFilePath);
                        }
                    }
                    closedir(dir);
                }
            }
        }
    }
}




bool isDirectory(const std::string& path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);
    }
    return false;
}

//bool deleteDirectory(const std::string& path) {
//    if (!isDirectory(path)) {
//        // Not a directory
//        return false;
//    }
//
//    FsFileSystem *fs = fsdevGetDeviceFileSystem("sdmc");
//    if (fs == nullptr) {
//        // Error getting file system
//        return false;
//    }
//
//    Result ret = fsFsDeleteDirectoryRecursively(fs, path.c_str());
//    if (R_FAILED(ret)) {
//        // Error deleting directory
//        return false;
//    }
//
//    return true;
//}
//
//
//
//void deleteFile(const std::string& pathToDelete) {
//    struct stat pathStat;
//    if (stat(pathToDelete.c_str(), &pathStat) == 0) {
//        if (S_ISREG(pathStat.st_mode)) {
//            if (std::remove(pathToDelete.c_str()) == 0) {
//                // Deletion successful
//            }
//        } else if (S_ISDIR(pathStat.st_mode)) {
//            if (deleteDirectory(pathToDelete)) {
//                // Deletion successful
//            }
//        }
//    }
//}
//


void deleteFile(const std::string& pathToDelete) {
    struct stat pathStat;
    if (stat(pathToDelete.c_str(), &pathStat) == 0) {
        if (S_ISREG(pathStat.st_mode)) {
            if (std::remove(pathToDelete.c_str()) == 0) {
                // Deletion successful
            }
        } else if (S_ISDIR(pathStat.st_mode)) {
            // Delete all files in the directory
            DIR* directory = opendir(pathToDelete.c_str());
            if (directory != nullptr) {
                dirent* entry;
                while ((entry = readdir(directory)) != nullptr) {
                    std::string fileName = entry->d_name;
                    if (fileName != "." && fileName != "..") {
                        std::string filePath = pathToDelete + "/" + fileName;
                        deleteFile(filePath);
                    }
                }
                closedir(directory);
            }

            // Remove the directory itself
            if (rmdir(pathToDelete.c_str()) == 0) {
                // Deletion successful
            }
        }
    }
}


// Check if a string starts with a given prefix
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
}


// Trim leading and trailing whitespaces from a string
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}


std::string removeQuotes(const std::string& str) {
    std::size_t firstQuote = str.find_first_of('\'');
    std::size_t lastQuote = str.find_last_of('\'');
    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
        return str.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }
    return str;
}

std::string getValueFromLine(const std::string& line) {
    std::size_t equalsPos = line.find('=');
    if (equalsPos != std::string::npos) {
        std::string value = line.substr(equalsPos + 1);
        return trim(value);
    }
    return "";
}


void setIniFile(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& desiredNewKey) {
    FILE* configFile = fopen(fileToEdit.c_str(), "r");
    if (!configFile) {
        printf("Failed to open the INI file.\n");
        return;
    }

    std::string trimmedLine;
    std::string tempPath = fileToEdit + ".tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "w");

    if (tempFile) {
        std::string currentSection;
        char line[256];
        while (fgets(line, sizeof(line), configFile)) {
            trimmedLine = trim(std::string(line));

            // Check if the line represents a section
            if (trimmedLine[0] == '[' && trimmedLine[trimmedLine.length() - 1] == ']') {
                currentSection = removeQuotes(trim(std::string(trimmedLine.c_str() + 1, trimmedLine.length() - 2)));
            }

            // Check if the line is in the desired section
            if (trim(currentSection) == trim(desiredSection)) {
                // Tokenize the line based on "=" delimiter
                std::string::size_type delimiterPos = trimmedLine.find('=');
                if (delimiterPos != std::string::npos) {
                    std::string lineKey = trim(trimmedLine.substr(0, delimiterPos));

                    // Check if the line key matches the desired key
                    if (lineKey == desiredKey) {
                        std::string formattedDesiredValue = removeQuotes(desiredValue);
                        std::string originalValue = getValueFromLine(trimmedLine); // Extract the original value

                        // Write the modified line with the desired key and value
                        if (!desiredNewKey.empty()) {
                            fprintf(tempFile, "%s = %s\n", desiredNewKey.c_str(), originalValue.c_str());
                        } else {
                            fprintf(tempFile, "%s = %s\n", desiredKey.c_str(), formattedDesiredValue.c_str());
                        }
                        continue; // Skip writing the original line
                    }
                }
            }

            fprintf(tempFile, "%s", line);
        }

        fclose(configFile);
        fclose(tempFile);
        remove(fileToEdit.c_str()); // Delete the old configuration file
        rename(tempPath.c_str(), fileToEdit.c_str()); // Rename the temp file to the original name

        // printf("INI file updated successfully.\n");
    } else {
        // printf("Failed to create temporary file.\n");
    }
}


void setIniFileValue(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue) {
    setIniFile(fileToEdit, desiredSection, desiredKey, desiredValue, "");
}
void setIniFileKey(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredNewKey) {
    setIniFile(fileToEdit, desiredSection, desiredKey, "", desiredNewKey);
}



void interpretAndExecuteCommand(const std::vector<std::vector<std::string>>& commands) {
    for (const auto& command : commands) {
        
        // Check the command and perform the appropriate action
        if (command.empty()) {
            // Empty command, do nothing
            continue;
        }


        //std::vector<std::string> arguments;
        //for (std::size_t i = 1; i < command.size(); ++i) {
        //    std::string arg = command[i];
        //
        //    // Check if the argument is enclosed within quotes
        //    if (!arg.empty() && arg.front() == '"' && arg.back() == '"') {
        //        // Remove the quotes and add the argument
        //        arg = arg.substr(1, arg.size() - 2);
        //    }
        //
        //    arguments.push_back(arg);
        //}


        // Get the command name (first part of the command)
        std::string commandName = command[0];
        //logMessage(commandName);
        //logMessage(command[1]);

        if (commandName == "make" || commandName == "mkdir") {
            std::string toDirectory = "sdmc:" + removeQuotes(command[1]);
            createDirectory(toDirectory);

            // Perform actions based on the command name
        } else if (commandName == "copy" || commandName == "cp") {
            // Copy command
            if (command.size() >= 3) {
                std::string fromPath = "sdmc:" + removeQuotes(command[1]);
                std::string toPath = "sdmc:" + removeQuotes(command[2]);
                copyFile(fromPath, toPath);
            }
        } else if (commandName == "delete" || commandName == "del") {
            // Delete command
            if (command.size() >= 2) {
                std::string fileToDelete = "sdmc:" + removeQuotes(command[1]);
                deleteFile(fileToDelete);
            }
        } else if (commandName == "rename" || commandName == "move" || commandName == "mv") {
            // Rename command
            if (command.size() >= 3) {

                std::string sourcePath = "sdmc:" + removeQuotes(command[1]);
                std::string destinationPath = "sdmc:" + removeQuotes(command[2]);

                if (command[1].back() == '/') {
                    // Remove trailing '/' from source path
                    sourcePath.pop_back();
                }

                if (command[2].back() == '/') {
                    // Remove trailing '/' from destination path
                    destinationPath.pop_back();
                }

                if (sourcePath.find('*') != std::string::npos) {
                    // Move files by pattern
                    if (moveFilesByPattern(sourcePath, destinationPath)) {
                        //std::cout << "Files moved successfully." << std::endl;
                    } else {
                        //std::cout << "Failed to move files." << std::endl;
                    }
                } else {
                    // Move single file or directory
                    if (moveFileOrDirectory(sourcePath, destinationPath)) {
                        //std::cout << "File or directory moved successfully." << std::endl;
                    } else {
                        //std::cout << "Failed to move file or directory." << std::endl;
                    }
                }
            } else {
                //std::cout << "Invalid move command. Usage: move <source_path> <destination_path>" << std::endl;
            }

        } else if (commandName == "set-ini-val" || commandName == "set-ini-value") {
            // Edit command
            if (command.size() >= 5) {
                std::string fileToEdit = "sdmc:" + removeQuotes(command[1]);

                std::string desiredSection = removeQuotes(command[2]);
                std::string desiredKey = removeQuotes(command[3]);

                std::string desiredValue;
                for (size_t i = 4; i < command.size(); ++i) {
                    desiredValue += command[i];
                    if (i < command.size() - 1) {
                        desiredValue += " ";
                    }
                }

                setIniFileValue(fileToEdit.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
            }
        } else if (commandName == "set-ini-key") {
            // Edit command
            if (command.size() >= 5) {
                std::string fileToEdit = "sdmc:" + removeQuotes(command[1]);

                std::string desiredSection = removeQuotes(command[2]);
                std::string desiredKey = removeQuotes(command[3]);

                std::string desiredNewKey;
                for (size_t i = 4; i < command.size(); ++i) {
                    desiredNewKey += command[i];
                    if (i < command.size() - 1) {
                        desiredNewKey += " ";
                    }
                }

                setIniFileKey(fileToEdit.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredNewKey.c_str());
            }
        } else if (commandName == "reboot") {
            // Reboot command
            spsmShutdown(SpsmShutdownMode_Reboot);
        } else if (commandName == "shutdown") {
            // Reboot command
            spsmShutdown(SpsmShutdownMode_Normal);
        }
    }
}


std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& configIniPath) {
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;

    FILE* configFile = fopen(configIniPath.c_str(), "r");
    if (!configFile) {
        // Write the default INI file
        FILE* configFileOut = fopen(configIniPath.c_str(), "w");
        fprintf(configFileOut, "[make directories]\n");
        fclose(configFileOut);
        configFile = fopen(configIniPath.c_str(), "r");
    }

    constexpr size_t BufferSize = 131072; // Choose a larger buffer size for reading lines
    char line[BufferSize];
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
            bool inQuotes = false;
            while (std::getline(iss, part, '\'')) {
                if (!part.empty()) {
                    if (!inQuotes) {
                        // Outside quotes, split on spaces
                        std::istringstream argIss(part);
                        std::string arg;
                        while (argIss >> arg) {
                            commandParts.push_back(arg);
                        }
                    } else {
                        // Inside quotes, treat as a whole argument
                        commandParts.push_back(part);
                    }
                }
                inQuotes = !inQuotes;
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
