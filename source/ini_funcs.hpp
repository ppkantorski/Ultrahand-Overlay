/********************************************************************************
 * File: ini_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file provides functions for working with INI (Initialization) files
 *   in C++. It includes functions for reading, parsing, and editing INI files,
 *   as well as cleaning INI file formatting.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

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

/**
 * @brief Represents a package header structure.
 *
 * This structure holds information about a package header, including version,
 * creator, and description.
 */
struct PackageHeader {
    std::string version;
    std::string creator;
    std::string about;
};

/**
 * @brief Retrieves the package header information from an INI file.
 *
 * This function parses an INI file and extracts the package header information.
 *
 * @param filePath The path to the INI file.
 * @return The package header structure.
 */
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

/**
 * @brief Splits a string into a vector of substrings using a specified delimiter.
 *
 * This function splits a given string into multiple substrings based on the specified delimiter.
 *
 * @param str The input string to be split.
 * @param delim The delimiter character used for splitting (default is space ' ').
 * @return A vector of substrings obtained by splitting the input string.
 */
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

/**
 * @brief Parses an INI-formatted string into a map of sections and key-value pairs.
 *
 * This function parses an INI-formatted string and organizes the data into a map,
 * where sections are keys and key-value pairs are stored within each section.
 *
 * @param str The INI-formatted string to parse.
 * @return A map representing the parsed INI data.
 */
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

/**
 * @brief Parses an INI file and returns its content as a map of sections and key-value pairs.
 *
 * This function reads the contents of an INI file located at the specified path,
 * parses it into a map structure, where section names are keys and key-value pairs
 * are stored within each section.
 *
 * @param configIniPath The path to the INI file to be parsed.
 * @return A map representing the parsed INI data.
 */
std::map<std::string, std::map<std::string, std::string>> getParsedDataFromIniFile(const std::string& configIniPath) {
    std::map<std::string, std::map<std::string, std::string>> parsedData;
    std::string currentSection = ""; // Initialize the current section as empty

    FILE* configFileIn = fopen(configIniPath.c_str(), "rb");
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

    // Normalize line endings to \n
    fileDataString.erase(std::remove(fileDataString.begin(), fileDataString.end(), '\r'), fileDataString.end());

    // Split lines and parse
    std::istringstream fileStream(fileDataString);
    std::string line;
    while (std::getline(fileStream, line)) {
        // Remove leading and trailing whitespace
        line = trim(line);

        // Check if this line is a section
        if (line.size() > 2 && line.front() == '[' && line.back() == ']') {
            // Remove the brackets to get the section name
            currentSection = line.substr(1, line.size() - 2);
        } else {
            // If not a section, parse as key-value pair
            size_t delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(line.substr(0, delimiterPos));
                std::string value = trim(line.substr(delimiterPos + 1));

                // Store in the current section
                parsedData[currentSection][key] = value;
            }
        }
    }

    delete[] fileData;

    return parsedData;
}



/**
 * @brief Parses sections from an INI file and returns them as a list of strings.
 *
 * This function reads an INI file and extracts the section names from it.
 *
 * @param filePath The path to the INI file.
 * @return A vector of section names.
 */
std::vector<std::string> parseSectionsFromIni(const std::string& filePath) {
    std::vector<std::string> sections;

    FILE* file = fopen(filePath.c_str(), "r");
    if (file == nullptr) {
        return sections; // Return an empty list if the file cannot be opened
    }

    char line[4096];
    while (fgets(line, sizeof(line), file)) {
        std::string trimmedLine = trim(std::string(line));

        if (!trimmedLine.empty() && trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            // Extract section name and add it to the list
            std::string sectionName = trimmedLine.substr(1, trimmedLine.size() - 2);
            sections.push_back(sectionName);
        }
    }

    fclose(file);
    return sections;
}


/**
 * @brief Loads and parses options from an INI file.
 *
 * This function reads and parses options from an INI file, organizing them by section.
 *
 * @param configIniPath The path to the INI file.
 * @param makeConfig A flag indicating whether to create a config if it doesn't exist.
 * @return A vector containing pairs of section names and their associated key-value pairs.
 */
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


/**
 * @brief Cleans the formatting of an INI file by removing empty lines and standardizing section formatting.
 *
 * This function takes an INI file located at the specified path, removes empty lines,
 * and standardizes the formatting of sections by ensuring that there is a newline
 * between each section's closing ']' and the next section's opening '['.
 *
 * @param filePath The path to the INI file to be cleaned.
 */
void cleanIniFormatting(const std::string& filePath) {
    FILE* inputFile = fopen(filePath.c_str(), "r");
    if (!inputFile) {
        // Failed to open the input file
        // Handle the error accordingly
        return;
    }

    std::string tempPath = filePath + ".tmp";
    FILE* outputFile = fopen(tempPath.c_str(), "w");
    if (!outputFile) {
        // Failed to create the output file
        // Handle the error accordingly
        fclose(inputFile);
        return;
    }

    bool isNewSection = false;

    char line[4096];
    while (fgets(line, sizeof(line), inputFile)) {
        std::string trimmedLine = trim(std::string(line));

        if (!trimmedLine.empty()) {
            if (trimmedLine[0] == '[' && trimmedLine[trimmedLine.length() - 1] == ']') {
                if (isNewSection) {
                    fprintf(outputFile, "\n");
                }
                isNewSection = true;
            }

            fprintf(outputFile, "%s\n", trimmedLine.c_str());
        }
    }

    fclose(inputFile);
    fclose(outputFile);

    // Remove the original file and rename the temp file
    remove(filePath.c_str());
    rename(tempPath.c_str(), filePath.c_str());
}


/**
 * @brief Modifies or creates an INI file by adding or updating key-value pairs in the specified section.
 *
 * This function attempts to open the specified INI file for reading. If the file doesn't exist,
 * it creates a new file and adds the specified section and key-value pair. If the file exists,
 * it reads its contents, modifies or adds the key-value pair in the specified section, and saves
 * the changes back to the original file.
 *
 * @param fileToEdit      The path to the INI file to be modified or created.
 * @param desiredSection  The name of the section in which the key-value pair should be added or updated.
 * @param desiredKey      The key for the key-value pair to be added or updated.
 * @param desiredValue    The new value for the key-value pair.
 * @param desiredNewKey   (Optional) If provided, the function will rename the key while preserving the original value.
 */
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
                
                if (sectionFound && !keyFound && (desiredNewKey.empty())) {
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

/**
 * @brief Sets the value of a key in an INI file within the specified section and cleans the formatting.
 *
 * This function sets the value of the specified key within the given section of the INI file.
 * If the key or section does not exist, it creates them. After updating the INI file,
 * it cleans the formatting to ensure proper INI file structure.
 *
 * @param fileToEdit      The path to the INI file to be modified or created.
 * @param desiredSection  The name of the section in which the key-value pair should be added or updated.
 * @param desiredKey      The key for the key-value pair to be added or updated.
 * @param desiredValue    The new value for the key-value pair.
 */
void setIniFileValue(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue) {
    setIniFile(fileToEdit, desiredSection, desiredKey, desiredValue, "");
    cleanIniFormatting(fileToEdit);
}

/**
 * @brief Sets the key name to a new name in an INI file within the specified section and cleans the formatting.
 *
 * This function sets the key name to a new name within the given section of the INI file.
 * If the key or section does not exist, it creates them. After updating the INI file,
 * it cleans the formatting to ensure proper INI file structure.
 *
 * @param fileToEdit      The path to the INI file to be modified or created.
 * @param desiredSection  The name of the section in which the key-name change should occur.
 * @param desiredKey      The key name to be changed.
 * @param desiredNewKey   The new key name to replace the original key name.
 */
void setIniFileKey(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredNewKey) {
    setIniFile(fileToEdit, desiredSection, desiredKey, "", desiredNewKey);
    cleanIniFormatting(fileToEdit);
}
