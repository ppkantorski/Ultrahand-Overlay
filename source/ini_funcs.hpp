#pragma once
#include <sys/stat.h>
#include <cstdio>   // For FILE*, fopen(), fclose(), fprintf(), etc.
#include <cstring>  // For std::string, strlen(), etc.
#include <string>   // For std::string
#include <vector>   // For std::vector
#include <map>      // For std::map
#include <sstream>  // For std::istringstream
#include <algorithm> // For std::remove_if
#include <cctype>   // For ::isspace

// Ini Functions

struct PackageHeader {
    std::string version;
    std::string creator;
    std::string about;
};
PackageHeader getPackageHeaderFromIni(const std::string& filePath) {
    PackageHeader packageHeader;
    
    FILE* file = fopen(filePath.c_str(), "r");
    if (file == nullptr) {
        packageHeader.version = "";
        packageHeader.creator = "";
        packageHeader.about = "";
        return packageHeader;
    }

    constexpr size_t BufferSize = 131072; // Choose a larger buffer size for reading lines
    char line[BufferSize];

    const std::string versionPrefix = ";version=";
    const std::string creatorPrefix = ";creator=";
    const std::string aboutPrefix = ";about=";

    while (fgets(line, sizeof(line), file)) {
        std::string strLine(line);
        size_t versionPos = strLine.find(versionPrefix);
        if (versionPos != std::string::npos) {
            versionPos += versionPrefix.length();
            size_t startPos = strLine.find("'", versionPos);
            size_t endPos = strLine.find("'", startPos + 1);

            if (startPos != std::string::npos && endPos != std::string::npos) {
                // Value enclosed in single quotes
                packageHeader.version = strLine.substr(startPos + 1, endPos - startPos - 1);
            } else {
                // Value not enclosed in quotes
                packageHeader.version = strLine.substr(versionPos, endPos - versionPos);
            }

            // Remove trailing whitespace or newline characters
            packageHeader.version.erase(packageHeader.version.find_last_not_of(" \t\r\n") + 1);
        }

        size_t creatorPos = strLine.find(creatorPrefix);
        if (creatorPos != std::string::npos) {
            creatorPos += creatorPrefix.length();
            size_t startPos = strLine.find("'", creatorPos);
            size_t endPos = strLine.find("'", startPos + 1);

            if (startPos != std::string::npos && endPos != std::string::npos) {
                // Value enclosed in single quotes
                packageHeader.creator = strLine.substr(startPos + 1, endPos - startPos - 1);
            } else {
                // Value not enclosed in quotes
                packageHeader.creator = strLine.substr(creatorPos, endPos - creatorPos);
            }

            // Remove trailing whitespace or newline characters
            packageHeader.creator.erase(packageHeader.creator.find_last_not_of(" \t\r\n") + 1);
        }

        size_t aboutPos = strLine.find(aboutPrefix);
        if (aboutPos != std::string::npos) {
            aboutPos += aboutPrefix.length();
            size_t startPos = strLine.find("'", aboutPos);
            size_t endPos = strLine.find("'", startPos + 1);

            if (startPos != std::string::npos && endPos != std::string::npos) {
                // Value enclosed in single quotes
                packageHeader.about = strLine.substr(startPos + 1, endPos - startPos - 1);
            } else {
                // Value not enclosed in quotes
                packageHeader.about = strLine.substr(aboutPos, endPos - aboutPos);
            }

            // Remove trailing whitespace or newline characters
            packageHeader.about.erase(packageHeader.about.find_last_not_of(" \t\r\n") + 1);
        }

        if (!packageHeader.version.empty() && !packageHeader.creator.empty() && !packageHeader.about.empty()) {
            break; // Both version and creator found, exit the loop
        }
    }



    fclose(file);
    
    return packageHeader;
}

static std::vector<std::string> split(const std::string& str, char delim = ' ') {
    std::vector<std::string> out;

    std::size_t current, previous = 0;
    current = str.find(delim);
    while (current != std::string::npos) {
        out.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find(delim, previous);
    }
    out.push_back(str.substr(previous, current - previous));

    return out;
}

static std::map<std::string, std::map<std::string, std::string>> parseIni(const std::string &str) {
    std::map<std::string, std::map<std::string, std::string>> iniData;

    auto lines = split(str, '\n');

    std::string lastHeader = "";
    for (auto& line : lines) {
        line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());

        if (line[0] == '[' && line[line.size() - 1] == ']') {
            lastHeader = line.substr(1, line.size() - 2);
            iniData.emplace(lastHeader, std::map<std::string, std::string>{});
        }
        else if (auto keyValuePair = split(line, '='); keyValuePair.size() == 2) {
            iniData[lastHeader].emplace(keyValuePair[0], keyValuePair[1]);
        }
    }

    return iniData;
}

// Custom utility function for parsing an ini file
std::map<std::string, std::map<std::string, std::string>> getParsedDataFromIniFile(const std::string& configIniPath) {
    std::map<std::string, std::map<std::string, std::string>> parsedData;
    
    // Open the INI file
    FILE* configFileIn = fopen(configIniPath.c_str(), "r");
    if (!configFileIn) {
        return parsedData;
    }

    // Determine the size of the INI file
    fseek(configFileIn, 0, SEEK_END);
    long fileSize = ftell(configFileIn);
    rewind(configFileIn);

    // Read the contents of the INI file
    char* fileData = new char[fileSize + 1];
    fread(fileData, sizeof(char), fileSize, configFileIn);
    fileData[fileSize] = '\0';  // Add null-terminator to create a C-string
    fclose(configFileIn);

    // Parse the INI data
    std::string fileDataString(fileData, fileSize);
    parsedData = parseIni(fileDataString);
    
    delete[] fileData;
    
    return parsedData;
}

std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& configIniPath, bool makeConfig = false) {
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;

    FILE* configFile = fopen(configIniPath.c_str(), "r");
    if (!configFile ) {
        // Write the default INI file
        FILE* configFileOut = fopen(configIniPath.c_str(), "w");
        std::string commands;
        if (makeConfig) {
            commands = "[HOS Reboot]\n"
                       "reboot\n"
                       "[Shutdown]\n"
                       "shutdown\n";
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
        // The INI file doesn't exist, create a new file and add the section and key-value pair
        configFile = fopen(fileToEdit.c_str(), "w");
        if (!configFile) {
            // Failed to create the file
            // Handle the error accordingly
            return;
        }
        fprintf(configFile, "[%s]\n", desiredSection.c_str());
        fprintf(configFile, "%s = %s\n", desiredKey.c_str(), desiredValue.c_str());
        fclose(configFile);
        // printf("INI file created successfully.\n");
        return;
    }

    std::string trimmedLine;
    std::string tempPath = fileToEdit + ".tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "w");

    if (tempFile) {
        std::string currentSection;
        std::string formattedDesiredValue = removeQuotes(desiredValue);
        constexpr size_t BufferSize = 4096;
        char line[BufferSize];
        bool sectionFound = false;
        //bool sectionOutOfBounds = false;
        bool keyFound = false;
        while (fgets(line, sizeof(line), configFile)) {
            trimmedLine = trim(std::string(line));

            // Check if the line represents a section
            if (trimmedLine[0] == '[' && trimmedLine[trimmedLine.length() - 1] == ']') {
                currentSection = removeQuotes(trim(std::string(trimmedLine.c_str() + 1, trimmedLine.length() - 2)));
                
                if (sectionFound && (desiredNewKey.empty())) {
                    // Write the modified line with the desired key and value
                    formattedDesiredValue = removeQuotes(desiredValue);
                    fprintf(tempFile, "%s = %s\n", desiredKey.c_str(), formattedDesiredValue.c_str());
                    keyFound = true;
                }
                
            }

            if (sectionFound && !keyFound && desiredNewKey.empty()) {
                if (trim(currentSection) != trim(desiredSection)) {
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
        
        if (sectionFound && !keyFound && (desiredNewKey.empty())) {
            // Write the modified line with the desired key and value
            fprintf(tempFile, "%s = %s\n", desiredKey.c_str(), formattedDesiredValue.c_str());
        }
        
        if (!sectionFound && !keyFound && desiredNewKey.empty()) {
            // The desired section doesn't exist, so create it and add the key-value pair
            fprintf(tempFile, "[%s]\n", desiredSection.c_str());
            fprintf(tempFile, "%s = %s\n", desiredKey.c_str(), formattedDesiredValue.c_str());
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
