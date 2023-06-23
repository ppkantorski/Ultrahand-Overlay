#pragma once
#include <switch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>

#define SpsmShutdownMode_Normal 0
#define SpsmShutdownMode_Reboot 1

// For loggging messages and debugging
//#include <ctime>
//void logMessage(const std::string& message) {
//    std::time_t currentTime = std::time(nullptr);
//    std::string logEntry = std::asctime(std::localtime(&currentTime));
//    // Find the last non-newline character
//    std::size_t lastNonNewline = logEntry.find_last_not_of("\r\n");
//
//    // Remove everything after the last non-newline character
//    if (lastNonNewline != std::string::npos) {
//        logEntry.erase(lastNonNewline + 1);
//    }
//    logEntry = "["+logEntry+"] ";
//    logEntry += message+"\n";
//
//    FILE* file = fopen("sdmc:/config/ultrahand/log.txt", "a");
//    if (file != nullptr) {
//        fputs(logEntry.c_str(), file);
//        fclose(file);
//    }
//}


// String functions
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

std::string replaceMultipleSlashes(const std::string& input) {
    std::string output;
    bool previousSlash = false;

    for (char c : input) {
        if (c == '/') {
            if (!previousSlash) {
                output.push_back(c);
            }
            previousSlash = true;
        } else {
            output.push_back(c);
            previousSlash = false;
        }
    }

    return output;
}


std::string dropExtension(const std::string& filename) {
    size_t lastDotPos = filename.find_last_of(".");
    if (lastDotPos != std::string::npos) {
        return filename.substr(0, lastDotPos);
    }
    return filename;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
}




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


// Directory functions
bool isDirectory(const std::string& path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);
    }
    return false;
}


// Function to create a directory if it doesn't exist
void createDirectory(const std::string& directoryPath) {
    struct stat st;
    if (stat(directoryPath.c_str(), &st) != 0) {
        mkdir(directoryPath.c_str(), 0777);
    }
}


// Get functions
struct PackageHeader {
    std::string version;
    std::string creator;
};
PackageHeader getPackageHeaderFromIni(const std::string& filePath) {
    PackageHeader packageHeader;
    std::string version = "";
    std::string creator = "";
    
    FILE* file = fopen(filePath.c_str(), "r");
    if (file == nullptr) {
        packageHeader.version = version;
        packageHeader.creator = creator;
        return packageHeader;
    }

    constexpr size_t BufferSize = 131072; // Choose a larger buffer size for reading lines
    char line[BufferSize];

    const std::string versionPrefix = ";version=";
    const std::string creatorPrefix = ";creator=";
    const size_t versionPrefixLength = versionPrefix.length();
    const size_t creatorPrefixLength = creatorPrefix.length();

    while (fgets(line, sizeof(line), file)) {
        std::string strLine(line);
        size_t versionPos = strLine.find(versionPrefix);
        if (versionPos != std::string::npos) {
            versionPos += versionPrefixLength;
            size_t endPos = strLine.find_first_of(" \t\r\n", versionPos);
            version = strLine.substr(versionPos, endPos - versionPos);
        }
        size_t creatorPos = strLine.find(creatorPrefix);
        if (creatorPos != std::string::npos) {
            creatorPos += creatorPrefixLength;
            size_t endPos = strLine.find_first_of(" \t\r\n", creatorPos);
            creator = strLine.substr(creatorPos, endPos - creatorPos);
        }

        if (!version.empty() && !creator.empty()) {
            break; // Both version and creator found, exit the loop
        }
    }

    fclose(file);
    
    packageHeader.version = version;
    packageHeader.creator = creator;
    return packageHeader;
}

std::string getValueFromLine(const std::string& line) {
    std::size_t equalsPos = line.find('=');
    if (equalsPos != std::string::npos) {
        std::string value = line.substr(equalsPos + 1);
        return trim(value);
    }
    return "";
}


std::string getNameFromPath(const std::string& path) {
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos) {
        std::string name = path.substr(lastSlash + 1);
        if (name.empty()) {
            // The path ends with a slash, indicating a directory
            std::string strippedPath = path.substr(0, lastSlash);
            lastSlash = strippedPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                name = strippedPath.substr(lastSlash + 1);
            }
        }
        return name;
    }
    return path;
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

// get files list for file patterns and folders list for folder patterns
std::vector<std::string> getFilesListByWildcards(const std::string& pathPattern) {
    std::string dirPath = "";
    std::string wildcard = "";

    std::size_t wildcardPos = pathPattern.find('*');
    if (wildcardPos != std::string::npos) {
        std::size_t slashPos = pathPattern.rfind('/', wildcardPos);

        if (slashPos != std::string::npos) {
            dirPath = pathPattern.substr(0, slashPos + 1);
            wildcard = pathPattern.substr(slashPos + 1);
        } else {
            dirPath = "";
            wildcard = pathPattern;
        }
    } else {
        dirPath = pathPattern + "/";
    }

    //logMessage("dirPath: " + dirPath);
    //logMessage("wildcard: " + wildcard);

    std::vector<std::string> fileList;

    bool isFolderWildcard = wildcard.back() == '/';
    if (isFolderWildcard) {
        wildcard = wildcard.substr(0, wildcard.size() - 1);  // Remove the trailing slash
    }

    //logMessage("isFolderWildcard: " + std::to_string(isFolderWildcard));

    DIR* dir = opendir(dirPath.c_str());
    if (dir != nullptr) {
        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string entryName = entry->d_name;
            std::string entryPath = dirPath + entryName;

            bool isEntryDirectory = isDirectory(entryPath);

            //logMessage("entryName: " + entryName);
            //logMessage("entryPath: " + entryPath);
            //logMessage("isFolderWildcard: " + std::to_string(isFolderWildcard));
            //logMessage("isEntryDirectory: " + std::to_string(isEntryDirectory));

            if (isFolderWildcard && isEntryDirectory && fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0) {
                if (entryName != "." && entryName != "..") {
                    fileList.push_back(entryPath+"/");
                }
            } else if (!isFolderWildcard && !isEntryDirectory) {
                std::size_t wildcardPos = wildcard.find('*');
                if (wildcardPos != std::string::npos) {
                    std::string prefix = wildcard.substr(0, wildcardPos);
                    if (entryName.find(prefix) == 0) {
                        std::string suffix = wildcard.substr(wildcardPos + 1);
                        if (entryName.size() >= suffix.size() && entryName.compare(entryName.size() - suffix.size(), suffix.size(), suffix) == 0) {
                            fileList.push_back(entryPath);
                        }
                    }
                } else if (fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0) {
                    fileList.push_back(entryPath);
                }
            }
        }
        closedir(dir);
    }

    std::string fileListAsString;
    for (const std::string& filePath : fileList) {
        fileListAsString += filePath + "\n";
    }
    //logMessage("File List:\n" + fileListAsString);

    return fileList;
}


// Delete functions
void deleteFileOrDirectory(const std::string& pathToDelete) {
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
                        deleteFileOrDirectory(filePath);
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

void deleteFileOrDirectoryByPattern(const std::string& pathPattern) {
    //logMessage("pathPattern: "+pathPattern);
    std::vector<std::string> fileList = getFilesListByWildcards(pathPattern);

    for (const auto& path : fileList) {
        //logMessage("path: "+path);
        deleteFileOrDirectory(path);
    }
}


// Move functions
bool moveFileOrDirectory(const std::string& sourcePath, const std::string& destinationPath) {
    struct stat sourceInfo;
    struct stat destinationInfo;
    
    //logMessage("sourcePath: "+sourcePath);
    //logMessage("destinationPath: "+destinationPath);
    
    if (stat(sourcePath.c_str(), &sourceInfo) == 0) {
        // Source file or directory exists

        // Check if the destination path exists
        bool destinationExists = (stat(destinationPath.c_str(), &destinationInfo) == 0);

        if (S_ISDIR(sourceInfo.st_mode)) {
            // Source path is a directory

            if (!destinationExists) {
                // Create the destination directory
                createDirectory(destinationPath);
            }

            DIR* dir = opendir(sourcePath.c_str());
            if (!dir) {
                //logMessage("Failed to open source directory: "+sourcePath);
                //printf("Failed to open source directory: %s\n", sourcePath.c_str());
                return false;
            }

            struct dirent* entry;
            while ((entry = readdir(dir)) != NULL) {
                const std::string fileOrFolderName = entry->d_name;

                if (fileOrFolderName != "." && fileOrFolderName != "..") {
                    std::string sourceFilePath = sourcePath + fileOrFolderName;
                    std::string destinationFilePath = destinationPath + fileOrFolderName;

                    if (entry->d_type == DT_DIR) {
                        // Append trailing slash to destination path for folders
                        destinationFilePath += "/";
                        sourceFilePath += "/";
                    }

                    if (!moveFileOrDirectory(sourceFilePath, destinationFilePath)) {
                        closedir(dir);
                        return false;
                    }
                }
            }

            closedir(dir);

            // Delete the source directory
            deleteFileOrDirectory(sourcePath);

            return true;
        } else {
            // Source path is a regular file
            std::string filename = getNameFromPath(sourcePath.c_str());

            std::string destinationFilePath = destinationPath;

            if (destinationPath[destinationPath.length() - 1] == '/') {
                destinationFilePath += filename;
            }
            
            
            //logMessage("sourcePath: "+sourcePath);
            //logMessage("destinationFilePath: "+destinationFilePath);
            
            if (rename(sourcePath.c_str(), destinationFilePath.c_str()) == -1) {
                //printf("Failed to move file: %s\n", sourcePath.c_str());
                //logMessage("Failed to move file: "+sourcePath);
                return false;
            }

            return true;
        }
    }

    return false;  // Move unsuccessful or source file/directory doesn't exist
}

bool moveFilesOrDirectoriesByPattern(const std::string& sourcePathPattern, const std::string& destinationPath) {
    std::vector<std::string> fileList = getFilesListByWildcards(sourcePathPattern);
    
    std::string fileListAsString;
    for (const std::string& filePath : fileList) {
        fileListAsString += filePath + "\n";
    }
    //logMessage("File List:\n" + fileListAsString);
    
    
    bool success = false;
    //logMessage("pre loop");
    // Iterate through the file list
    for (const std::string& sourceFileOrDirectory : fileList) {
        //logMessage("sourceFileOrDirectory: "+sourceFileOrDirectory);
        // if sourceFile is a file (Needs condition handling)
        if (!isDirectory(sourceFileOrDirectory)) {
            //logMessage("destinationPath: "+destinationPath);
            success = moveFileOrDirectory(sourceFileOrDirectory.c_str(), destinationPath.c_str());
        } else if (isDirectory(sourceFileOrDirectory)) {
            // if sourceFile is a directory (needs conditoin handling)
            std::string folderName = getNameFromPath(sourceFileOrDirectory);
            std::string fixedDestinationPath = destinationPath + folderName + "/";
        
            //logMessage("fixedDestinationPath: "+fixedDestinationPath);
        
            success = moveFileOrDirectory(sourceFileOrDirectory.c_str(), fixedDestinationPath.c_str());
        }

    }
    //logMessage("post loop");
    return success;  // All files matching the pattern moved successfully
}


// Copy functions
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

void copyFileOrDirectory(const std::string& fromFileOrDirectory, const std::string& toFileOrDirectory) {
    struct stat fromFileOrDirectoryInfo;
    if (stat(fromFileOrDirectory.c_str(), &fromFileOrDirectoryInfo) == 0) {
        if (S_ISREG(fromFileOrDirectoryInfo.st_mode)) {
            // Source is a regular file
            std::string fromFile = fromFileOrDirectory;
            
            struct stat toFileOrDirectoryInfo;
            
            if (stat(toFileOrDirectory.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISDIR(toFileOrDirectoryInfo.st_mode)) {
                // Destination is a directory
                std::string toDirectory = toFileOrDirectory;
                std::string fileName = fromFile.substr(fromFile.find_last_of('/') + 1);
                std::string toFilePath = toDirectory + fileName;

                // Check if the destination file exists and remove it
                if (stat(toFilePath.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISREG(toFileOrDirectoryInfo.st_mode)) {
                    std::remove(toFilePath.c_str());
                }

                copySingleFile(fromFile, toFilePath);
            } else {
                std::string toFile = toFileOrDirectory;
                // Destination is a file or doesn't exist
                // Check if the destination file exists and remove it
                if (stat(toFile.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISREG(toFileOrDirectoryInfo.st_mode)) {
                    std::remove(toFile.c_str());
                }

                copySingleFile(fromFile, toFile);
            }
        } else if (S_ISDIR(fromFileOrDirectoryInfo.st_mode)) {
            // Source is a directory
            std::string fromDirectory = fromFileOrDirectory;
            //logMessage("fromDirectory: "+fromDirectory);
            
            struct stat toFileOrDirectoryInfo;
            if (stat(toFileOrDirectory.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISDIR(toFileOrDirectoryInfo.st_mode)) {
                // Destination is a directory
                std::string toDirectory = toFileOrDirectory;
                std::string dirName = getNameFromPath(fromDirectory);
                if (dirName != "") {
                    std::string toDirPath = toDirectory + dirName +"/";
                    //logMessage("toDirectory: "+toDirectory);
                    //logMessage("dirName: "+dirName);
                    //logMessage("toDirPath: "+toDirPath);

                    // Create the destination directory
                    mkdir(toDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

                    // Open the source directory
                    DIR* dir = opendir(fromDirectory.c_str());
                    if (dir != nullptr) {
                        dirent* entry;
                        while ((entry = readdir(dir)) != nullptr) {
                            std::string fileOrFolderName = entry->d_name;
                            
                            // handle cade for files
                            if (fileOrFolderName != "." && fileOrFolderName != "..") {
                                std::string fromFilePath = fromDirectory + fileOrFolderName;
                                copyFileOrDirectory(fromFilePath, toDirPath);
                            }
                            // handle case for subfolders within the from file path
                            if (entry->d_type == DT_DIR && fileOrFolderName != "." && fileOrFolderName != "..") {
                                std::string subFolderPath = fromDirectory + fileOrFolderName + "/";
                                
                                
                                copyFileOrDirectory(subFolderPath, toDirPath);
                            }
                            
                        }
                        closedir(dir);
                    }
                }
            }
        }
    }
}

void copyFileOrDirectoryByPattern(const std::string& sourcePathPattern, const std::string& toDirectory) {
    std::vector<std::string> fileList = getFilesListByWildcards(sourcePathPattern);

    for (const std::string& sourcePath : fileList) {
        //logMessage("sourcePath: "+sourcePath);
        //logMessage("toDirectory: "+toDirectory);
        if (sourcePath != toDirectory){
            copyFileOrDirectory(sourcePath, toDirectory);
        }
        
    }
}


// Ini Functions
std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& configIniPath, bool makeConfig = false) {
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;

    FILE* configFile = fopen(configIniPath.c_str(), "r");
    if (!configFile ) {
        // Write the default INI file
        FILE* configFileOut = fopen(configIniPath.c_str(), "w");
        std::string commands;
        if (makeConfig) {
            commands = "[make directories]\n"
                       "mkdir /config/ultrahand/example1/\n"
                       "mkdir /config/ultrahand/example2/\n"
                       "[copy files]\n"
                       "copy /config/ultrahand/config.ini /config/ultrahand/example1/\n"
                       "copy /config/ultrahand/config.ini /config/ultrahand/example2/\n"
                       "[rename files]\n"
                       "move /config/ultrahand/example1/config.ini /config/ultrahand/example1/configRenamed.ini\n"
                       "move /config/ultrahand/example2/config.ini /config/ultrahand/example2/configRenamed.ini\n"
                       "[move directories]\n"
                       "move /config/ultrahand/example1/ /config/ultrahand/example3/\n"
                       "move /config/ultrahand/example2/ /config/ultrahand/example4/\n"
                       "[delete files]\n"
                       "delete /config/ultrahand/example1/config.ini\n"
                       "delete /config/ultrahand/example2/config.ini\n"
                       "[delete directories]\n"
                       "delete /config/ultrahand/example*/\n"
                       "[modify ini file]\n"
                       "copy /bootloader/hekate_ipl.ini /config/ultrahand/\n"
                       "set-ini-val /config/ultrahand/hekate_ipl.ini 'Atmosphere' fss0 gonnawritesomethingelse\n"
                       "new-ini-entry /config/ultrahand/hekate_ipl.ini 'Atmosphere' booty true\n";
        } else {
            commands = "";
        }
        fprintf(configFileOut, "%s", commands.c_str());
        
        
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

void setIniFile(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& desiredNewKey) {
    FILE* configFile = fopen(fileToEdit.c_str(), "r");
    if (!configFile) {
        //printf("Failed to open the INI file.\n");
        return;
    }

    std::string trimmedLine;
    std::string tempPath = fileToEdit + ".tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "w");

    if (tempFile) {
        std::string currentSection;
        std::string formattedDesiredValue;
        constexpr size_t BufferSize = 4096;
        char line[BufferSize];
        bool sectionFound = false;
        bool keyFound = false;
        while (fgets(line, sizeof(line), configFile)) {
            trimmedLine = trim(std::string(line));

            // Check if the line represents a section
            if (trimmedLine[0] == '[' && trimmedLine[trimmedLine.length() - 1] == ']') {
                currentSection = removeQuotes(trim(std::string(trimmedLine.c_str() + 1, trimmedLine.length() - 2)));
            }

            if (sectionFound && !keyFound && desiredNewKey.empty()) {
                if (trim(currentSection) != trim(desiredSection)) {
                    formattedDesiredValue = removeQuotes(desiredValue);
                    fprintf(tempFile, "%s = %s\n", desiredKey.c_str(), formattedDesiredValue.c_str());
                    keyFound = true;
                }
            }

            // Check if the line is in the desired section
            if (trim(currentSection) == trim(desiredSection)) {
                sectionFound = true;
                // Tokenize the line based on "=" delimiter
                std::string::size_type delimiterPos = trimmedLine.find('=');
                if (delimiterPos != std::string::npos) {
                    std::string lineKey = trim(trimmedLine.substr(0, delimiterPos));

                    // Check if the line key matches the desired key
                    if (lineKey == desiredKey) {
                        keyFound = true;
                        std::string originalValue = getValueFromLine(trimmedLine); // Extract the original value
                        formattedDesiredValue = removeQuotes(desiredValue);

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
        
        if (!sectionFound && !keyFound && desiredNewKey.empty()) {
            // The desired section doesn't exist, so create it and add the key-value pair
            fprintf(tempFile, "\n[%s]\n", desiredSection.c_str());
            fprintf(tempFile, "%s = %s\n", desiredKey.c_str(), desiredValue.c_str());
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


// Hex-editing commands
std::string asciiToHex(const std::string& asciiStr) {
    std::string hexStr;
    hexStr.reserve(asciiStr.length() * 2); // Reserve space for the hexadecimal string

    for (char c : asciiStr) {
        unsigned char uc = static_cast<unsigned char>(c); // Convert char to unsigned char
        char hexChar[3]; // Buffer to store the hexadecimal representation (2 characters + null terminator)

        // Format the unsigned char as a hexadecimal string and append it to the result
        std::snprintf(hexChar, sizeof(hexChar), "%02X", uc);
        hexStr += hexChar;
    }
    
    if (hexStr.length() % 2 != 0) {
        hexStr = '0' + hexStr;
    }

    return hexStr;
}

std::string decimalToHex(const std::string& decimalStr) {
    // Convert decimal string to integer
    int decimalValue = std::stoi(decimalStr);

    // Convert decimal to hexadecimal
    std::string hexadecimal;
    while (decimalValue > 0) {
        int remainder = decimalValue % 16;
        char hexChar = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
        hexadecimal += hexChar;
        decimalValue /= 16;
    }

    // Reverse the hexadecimal string
    std::reverse(hexadecimal.begin(), hexadecimal.end());
    
    // If the length is odd, add a trailing '0'
    if (hexadecimal.length() % 2 != 0) {
        hexadecimal = '0' + hexadecimal;
    }

    return hexadecimal;
}

std::string decimalToReversedHex(const std::string& decimalStr, int order=2) {
    std::string hexadecimal = decimalToHex(decimalStr);
    
    // Reverse the hexadecimal string in groups of order
    std::string reversedHex;
    for (int i = hexadecimal.length() - order; i >= 0; i -= order) {
        reversedHex += hexadecimal.substr(i, order);
    }

    return reversedHex;
}

std::vector<std::string> findHexDataOffsets(const std::string& filePath, const std::string& hexData) {
    std::vector<std::string> offsets;

    // Open the file for reading in binary mode
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        //std::cerr << "Failed to open the file." << std::endl;
        return offsets;
    }

    // Check the file size
    struct stat fileStatus;
    if (stat(filePath.c_str(), &fileStatus) != 0) {
        //std::cerr << "Failed to retrieve file size." << std::endl;
        fclose(file);
        return offsets;
    }
    std::size_t fileSize = fileStatus.st_size;

    // Convert the hex data string to binary data
    std::vector<char> binaryData;
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        binaryData.push_back(byte);
    }

    // Read the file in chunks to find the offsets where the hex data is located
    const std::size_t bufferSize = 1024;
    std::vector<char> buffer(bufferSize);
    std::streampos offset = 0;
    std::streampos bytesRead = 0;
    while ((bytesRead = fread(buffer.data(), sizeof(char), bufferSize, file)) > 0) {
        for (std::size_t i = 0; i < bytesRead; i++) {
            if (std::memcmp(buffer.data() + i, binaryData.data(), binaryData.size()) == 0) {
                std::streampos currentOffset = offset + i;
                offsets.push_back(std::to_string(currentOffset));
            }
        }
        offset += bytesRead;
    }

    fclose(file);
    return offsets;
}

void hexEditByOffset(const std::string& filePath, const std::string& offsetStr, const std::string& hexData) {
    // Convert the offset string to std::streampos
    std::streampos offset = std::stoll(offsetStr);

    // Open the file for reading and writing in binary mode
    FILE* file = fopen(filePath.c_str(), "rb+");
    if (!file) {
        //logMessage("Failed to open the file.");
        return;
    }

    // Move the file pointer to the specified offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        //logMessage("Failed to move the file pointer.");
        fclose(file);
        return;
    }

    // Convert the hex data string to binary data
    std::vector<char> binaryData;
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        binaryData.push_back(byte);
    }

    // Calculate the number of bytes to be replaced
    std::size_t bytesToReplace = binaryData.size();

    // Read the existing data from the file
    std::vector<char> existingData(bytesToReplace);
    if (fread(existingData.data(), sizeof(char), bytesToReplace, file) != bytesToReplace) {
        //logMessage("Failed to read existing data from the file.");
        fclose(file);
        return;
    }

    // Move the file pointer back to the offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        //logMessage("Failed to move the file pointer.");
        fclose(file);
        return;
    }

    // Write the replacement binary data to the file
    if (fwrite(binaryData.data(), sizeof(char), bytesToReplace, file) != bytesToReplace) {
        //logMessage("Failed to write data to the file.");
        fclose(file);
        return;
    }

    fclose(file);
    //logMessage("Hex editing completed.");
}



void hexEditFindReplace(const std::string& filePath, const std::string& hexDataToReplace, const std::string& hexDataReplacement, const std::string& occurrence="0") {
    std::vector<std::string> offsetStrs = findHexDataOffsets(filePath, hexDataToReplace);
    if (!offsetStrs.empty()) {
        if (occurrence == "0") {
            // Replace all occurrences
            for (const std::string& offsetStr : offsetStrs) {
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            }
        } else {
            // Convert the occurrence string to an integer
            int index = std::stoi(occurrence);
            if (index > 0 && index <= offsetStrs.size()) {
                // Replace the specified occurrence/index
                std::string offsetStr = offsetStrs[index - 1];
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            } else {
                // Invalid occurrence/index specified
                //std::cout << "Invalid occurrence/index specified." << std::endl;
            }
        }
        //std::cout << "Hex data replaced successfully." << std::endl;
    } else {
        //std::cout << "Hex data to replace not found." << std::endl;
    }
}


// Safety conditions
// List of protected folders
const std::vector<std::string> protectedFolders = {
    "sdmc:/Nintendo/",
    "sdmc:/emuMMC/",
    "sdmc:/atmosphere/",
    "sdmc:/bootloader/",
    "sdmc:/switch/",
    "sdmc:/config/",
    "sdmc:/"
};
const std::vector<std::string> ultraProtectedFolders = {
    "sdmc:/Nintendo/",
    "sdmc:/emuMMC/"
};
bool isDangerousCombination(const std::string& patternPath) {
    // List of obviously dangerous patterns
    const std::vector<std::string> dangerousCombinationPatterns = {
        "*",         // Deletes all files/directories in the current directory
        "*/"         // Deletes all files/directories in the current directory
    };

    // List of obviously dangerous patterns
    const std::vector<std::string> dangerousPatterns = {
        "..",     // Attempts to traverse to parent directories
        "~"       // Represents user's home directory, can be dangerous if misused
    };

    // Check if the patternPath is an ultra protected folder
    for (const std::string& ultraProtectedFolder : ultraProtectedFolders) {
        if (patternPath.find(ultraProtectedFolder) == 0) {
            return true; // Pattern path is an ultra protected folder
        }
    }

    // Check if the patternPath is a protected folder
    for (const std::string& protectedFolder : protectedFolders) {
        if (patternPath == protectedFolder) {
            return true; // Pattern path is a protected folder
        }

        // Check if the patternPath starts with a protected folder and includes a dangerous pattern
        if (patternPath.find(protectedFolder) == 0) {
            std::string relativePath = patternPath.substr(protectedFolder.size());

            // Split the relativePath by '/' to handle multiple levels of wildcards
            std::vector<std::string> pathSegments;
            std::string pathSegment;

            for (char c : relativePath) {
                if (c == '/') {
                    if (!pathSegment.empty()) {
                        pathSegments.push_back(pathSegment);
                        pathSegment.clear();
                    }
                } else {
                    pathSegment += c;
                }
            }

            if (!pathSegment.empty()) {
                pathSegments.push_back(pathSegment);
            }

            for (const std::string& pathSegment : pathSegments) {
                // Check if the pathSegment includes a dangerous pattern
                for (const std::string& dangerousPattern : dangerousPatterns) {
                    if (pathSegment.find(dangerousPattern) != std::string::npos) {
                        return true; // Pattern path includes a dangerous pattern
                    }
                }
            }
        }

        // Check if the patternPath is a combination of a protected folder and a dangerous pattern
        for (const std::string& dangerousPattern : dangerousCombinationPatterns) {
            if (patternPath == protectedFolder + dangerousPattern) {
                return true; // Pattern path is a protected folder combined with a dangerous pattern
            }
        }
    }

    // Check if the patternPath is a dangerous pattern
    if (patternPath.find("sdmc:/") == 0) {
        std::string relativePath = patternPath.substr(6); // Remove "sdmc:/"

        // Split the relativePath by '/' to handle multiple levels of wildcards
        std::vector<std::string> pathSegments;
        std::string pathSegment;

        for (char c : relativePath) {
            if (c == '/') {
                if (!pathSegment.empty()) {
                    pathSegments.push_back(pathSegment);
                    pathSegment.clear();
                }
            } else {
                pathSegment += c;
            }
        }

        if (!pathSegment.empty()) {
            pathSegments.push_back(pathSegment);
        }

        for (const std::string& pathSegment : pathSegments) {
            // Check if the pathSegment includes a dangerous pattern
            for (const std::string& dangerousPattern : dangerousPatterns) {
                if (pathSegment == dangerousPattern) {
                    return true; // Pattern path is a dangerous pattern
                }
            }
        }
    }

    // Check if the patternPath includes a wildcard at the root level
    if (patternPath.find(":/") != std::string::npos) {
        std::string rootPath = patternPath.substr(0, patternPath.find(":/") + 2);
        if (rootPath.find('*') != std::string::npos) {
            return true; // Pattern path includes a wildcard at the root level
        }
    }

    // Check if the provided path matches any dangerous patterns
    for (const std::string& pattern : dangerousPatterns) {
        if (patternPath.find(pattern) != std::string::npos) {
            return true; // Path contains a dangerous pattern
        }
    }

    return false; // Pattern path is not a protected folder, a dangerous pattern, or includes a wildcard at the root level
}



// Main interpreter
void interpretAndExecuteCommand(const std::vector<std::vector<std::string>>& commands) {
    for (const auto& command : commands) {
        
        // Check the command and perform the appropriate action
        if (command.empty()) {
            // Empty command, do nothing
            continue;
        }


        // Get the command name (first part of the command)
        std::string commandName = command[0];
        //logMessage(commandName);
        //logMessage(command[1]);

        if (commandName == "make" || commandName == "mkdir") {
            std::string toDirectory = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));
            createDirectory(toDirectory);

            // Perform actions based on the command name
        } else if (commandName == "copy" || commandName == "cp") {
            // Copy command
            if (command.size() >= 3) {
                std::string fromPath = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));
                std::string toPath = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[2]));
                
                
                if (fromPath.find('*') != std::string::npos) {
                    // Delete files or directories by pattern
                    copyFileOrDirectoryByPattern(fromPath, toPath);
                } else {
                    copyFileOrDirectory(fromPath, toPath);
                }
                
            }
        } else if (commandName == "delete" || commandName == "del") {
            // Delete command
            if (command.size() >= 2) {
                std::string fileOrPathToDelete = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));
                
                if (!isDangerousCombination(fileOrPathToDelete)) {
                    if (fileOrPathToDelete.find('*') != std::string::npos) {
                        // Delete files or directories by pattern
                        deleteFileOrDirectoryByPattern(fileOrPathToDelete);
                    } else {
                        deleteFileOrDirectory(fileOrPathToDelete);
                    }
                }
            }
        } else if (commandName == "rename" || commandName == "move" || commandName == "mv") {
            // Rename command
            if (command.size() >= 3) {

                std::string sourcePath = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));
                std::string destinationPath = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[2]));
                //logMessage("sourcePath: "+sourcePath);
                //logMessage("destinationPath: "+destinationPath);
                
                if (!isDangerousCombination(sourcePath)) {
                    if (sourcePath.find('*') != std::string::npos) {
                        // Move files by pattern
                        if (moveFilesOrDirectoriesByPattern(sourcePath, destinationPath)) {
                            //std::cout << "Files moved successfully." << std::endl;
                            //logMessage( "Files moved successfully.");
                        } else {
                            //logMessage( "Failed to move files.");
                            //std::cout << "Failed to move files." << std::endl;
                        }
                    } else {
                        // Move single file or directory
                        if (moveFileOrDirectory(sourcePath, destinationPath)) {
                            //logMessage( "File or directory moved successfully.");
                            //std::cout << "File or directory moved successfully." << std::endl;
                        } else {
                            //logMessage( "Failed to move file or directory.");
                            //std::cout << "Failed to move file or directory." << std::endl;
                        }
                    }
                }
                else {
                    //logMessage( "Dangerous combo.");
                }
            } else {
                //logMessage( "Invalid move command.");
                //std::cout << "Invalid move command. Usage: move <source_path> <destination_path>" << std::endl;
            }

        } else if (commandName == "set-ini-val" || commandName == "set-ini-value") {
            // Edit command
            if (command.size() >= 5) {
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

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
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

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
        } else if (commandName == "hex-by-offset") {
            // Edit command
            if (command.size() >= 4) {
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

                std::string offset = removeQuotes(command[2]);
                std::string hexDataReplacement = removeQuotes(command[3]);

                hexEditByOffset(fileToEdit.c_str(), offset.c_str(), hexDataReplacement.c_str());
            }
        } else if (commandName == "hex-by-swap") {
            // Edit command - Hex data replacement with occurrence
            if (command.size() >= 4) {
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

                std::string hexDataToReplace = removeQuotes(command[2]);
                std::string hexDataReplacement = removeQuotes(command[3]);

                if (command.size() >= 5) {
                    std::string occurrence = removeQuotes(command[4]);
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement, occurrence);
                } else {
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement);
                }
            }
        } else if (commandName == "hex-by-string") {
            // Edit command - Hex data replacement with occurrence
            if (command.size() >= 4) {
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

                std::string hexDataToReplace = asciiToHex(removeQuotes(command[2]));
                std::string hexDataReplacement = asciiToHex(removeQuotes(command[3]));
                //logMessage("hexDataToReplace: "+hexDataToReplace);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                
                // Fix miss-matched string sizes
                if (hexDataReplacement.length() < hexDataToReplace.length()) {
                    // Pad with spaces at the end
                    hexDataReplacement += std::string(hexDataToReplace.length() - hexDataReplacement.length(), '00');
                } else if (hexDataReplacement.length() > hexDataToReplace.length()) {
                    // Add spaces to hexDataToReplace at the far right end
                    hexDataToReplace += std::string(hexDataReplacement.length() - hexDataToReplace.length(), '00');
                }
                
                if (command.size() >= 5) {
                    std::string occurrence = removeQuotes(command[4]);
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement, occurrence);
                } else {
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement);
                }
            }
        } else if (commandName == "hex-by-decimal") {
            // Edit command - Hex data replacement with occurrence
            if (command.size() >= 4) {
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

                std::string hexDataToReplace = decimalToHex(removeQuotes(command[2]));
                std::string hexDataReplacement = decimalToHex(removeQuotes(command[3]));
                //logMessage("hexDataToReplace: "+hexDataToReplace);
                //logMessage("hexDataReplacement: "+hexDataReplacement);

                if (command.size() >= 5) {
                    std::string occurrence = removeQuotes(command[4]);
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement, occurrence);
                } else {
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement);
                }
            }
        } else if (commandName == "hex-by-rdecimal") {
            // Edit command - Hex data replacement with occurrence
            if (command.size() >= 4) {
                std::string fileToEdit = "sdmc:" + replaceMultipleSlashes(removeQuotes(command[1]));

                std::string hexDataToReplace = decimalToReversedHex(removeQuotes(command[2]));
                std::string hexDataReplacement = decimalToReversedHex(removeQuotes(command[3]));
                //logMessage("hexDataToReplace: "+hexDataToReplace);
                //logMessage("hexDataReplacement: "+hexDataReplacement);

                if (command.size() >= 5) {
                    std::string occurrence = removeQuotes(command[4]);
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement, occurrence);
                } else {
                    hexEditFindReplace(fileToEdit, hexDataToReplace, hexDataReplacement);
                }
            }
        } else if (commandName == "reboot") {
            // Reboot command
            splExit();
            fsdevUnmountAll();
            spsmShutdown(SpsmShutdownMode_Reboot);
        } else if (commandName == "shutdown") {
            // Reboot command
            splExit();
            fsdevUnmountAll();
            spsmShutdown(SpsmShutdownMode_Normal);
        }
    }
}

