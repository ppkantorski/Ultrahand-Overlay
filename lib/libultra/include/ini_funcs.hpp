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
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/

#pragma once
#include <fstream>
#include <cstring>  // For std::string, strlen(), etc.
#include <string>   // For std::string
#include <vector>   // For std::vector
#include <map>      // For std::map
#include <sstream>  // For std::istringstream
#include <algorithm> // For std::remove_if
#include <cctype>   // For ::isspace
#include "get_funcs.hpp"
#include "path_funcs.hpp"


/**
 * @brief Represents a package header structure.
 *
 * This structure holds information about a package header, including version,
 * creator, and description.
 */
struct PackageHeader {
    std::string title;
    std::string version;
    std::string creator;
    std::string about;
    std::string credits;
    std::string color;
    
    void clear() {
        title.clear();
        version.clear();
        creator.clear();
        about.clear();
        credits.clear();
        color.clear();
    }
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
    std::ifstream file(filePath);
    if (!file) {
        return packageHeader; // Return default-constructed PackageHeader if file opening fails
    }

    // Map to store references to the fields of the structure
    std::map<std::string, std::string*> fieldMap = {
        {";title=", &packageHeader.title},
        {";version=", &packageHeader.version},
        {";creator=", &packageHeader.creator},
        {";about=", &packageHeader.about},
        {";credits=", &packageHeader.credits},
        {";color=", &packageHeader.color}
    };

    std::string line;
    size_t startPos, endPos;

    while (getline(file, line)) {
        // Process each prefix in the map
        for (const auto& [prefix, field] : fieldMap) {
            startPos = line.find(prefix);
            if (startPos != std::string::npos) {
                startPos += prefix.length();
                endPos = line.find_first_of(";\r\n", startPos); // Assume ';' or newlines mark the end
                if (endPos == std::string::npos) {
                    endPos = line.length();
                }
                *field = removeQuotes(trim(line.substr(startPos, endPos - startPos)));
                break; // Break after processing the first match in a line
            }
        }
    }

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

    // Reserve an estimated amount of space to reduce reallocations
    out.reserve(10); // Assuming an average of 10 tokens per string

    std::string_view strv = str;
    size_t current, previous = 0;
    current = strv.find(delim);

    while (current != std::string::npos) {
        out.emplace_back(strv.substr(previous, current - previous));
        previous = current + 1;
        current = strv.find(delim, previous);
    }
    out.emplace_back(strv.substr(previous)); // No need to calculate the length

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

    std::string trimmedLine;

    size_t delimiterPos;
    //std::string key, value;

    for (auto& line : lines) {
        trimmedLine = trim(line);

        if (trimmedLine.empty() || trimmedLine.front() == '#') {
            // Ignore empty lines and comments
            continue;
        }

        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            lastHeader = trimmedLine.substr(1, trimmedLine.size() - 2);
            iniData[lastHeader]; // Ensures the section exists even if it remains empty
        }
        else {
            delimiterPos = trimmedLine.find('=');
            if (delimiterPos != std::string::npos) {
                //key = trim(trimmedLine.substr(0, delimiterPos));
                //value = trim(trimmedLine.substr(delimiterPos + 1));
                if (!lastHeader.empty()) {
                    //iniData[lastHeader][key] = value;
                    iniData[lastHeader][trim(trimmedLine.substr(0, delimiterPos))] = trim(trimmedLine.substr(delimiterPos + 1));
                }
            }
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
    std::ifstream configFile(configIniPath);
    if (!configFile) {
        logMessage("Failed to open the file: " + configIniPath);
        return parsedData;  // Return empty map if file cannot be opened
    }

    std::string line, currentSection;
    std::string trimmedLine;

    size_t delimiterPos;
    std::string key, value;

    while (getline(configFile, line)) {
        trimmedLine = trim(line);

        if (trimmedLine.empty()) continue;

        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            // Remove the brackets and set the current section
            currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
        } else {
            delimiterPos = trimmedLine.find('=');
            if (delimiterPos != std::string::npos) {
                key = trim(trimmedLine.substr(0, delimiterPos));
                value = trim(trimmedLine.substr(delimiterPos + 1));
                parsedData[currentSection][key] = value;
            }
        }
    }

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
    std::ifstream file(filePath);
    if (!file) {
        logMessage("Failed to open the file: " + filePath);
        return sections;  // Early return if the file cannot be opened
    }

    std::string line;

    while (std::getline(file, line)) {
        std::string trimmedLine = trim(line);
        
        // Check if the line contains a section header
        if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            std::string sectionName = trimmedLine.substr(1, trimmedLine.size() - 2);
            sections.push_back(sectionName);
        }
    }

    return sections;
}



/**
 * @brief Parses a specific value from a section and key in an INI file.
 *
 * @param filePath The path to the INI file.
 * @param sectionName The name of the section containing the desired key.
 * @param keyName The name of the key whose value is to be retrieved.
 * @return The value as a string, or an empty string if the key or section isn't found.
 */
std::string parseValueFromIniSection(const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
    std::string value = "";
    std::ifstream file(filePath);
    if (!file) {
        logMessage("Failed to open the file: " + filePath);
        return value;  // Return empty if the file cannot be opened
    }

    std::string line, currentSection;

    size_t delimiterPos;
    std::string currentKey;
    std::string trimmedLine;

    while (std::getline(file, line)) {
        trimmedLine = trim(line);

        if (trimmedLine.empty()) continue;

        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
            if (currentSection != sectionName) continue;  // Skip processing other sections
        } else if (currentSection == sectionName) {
            delimiterPos = trimmedLine.find('=');
            if (delimiterPos != std::string::npos) {
                currentKey = trim(trimmedLine.substr(0, delimiterPos));
                if (currentKey == keyName) {
                    value = trim(trimmedLine.substr(delimiterPos + 1));
                    break;  // Found the key, exit the loop
                }
            }
        }
    }

    return value;
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
    std::string tempPath = filePath + ".tmp";

    // Open the input file with ifstream
    std::ifstream inputFile(filePath);
    if (!inputFile) {
        logMessage("Failed to open the input file: " + filePath);
        return;
    }

    // Create a temporary file for output with ofstream
    std::ofstream outputFile(tempPath);
    if (!outputFile) {
        logMessage("Failed to create the output file: " + tempPath);
        return;  // inputFile will be closed by destructor
    }

    std::string line;
    bool isNewSection = false;

    std::string trimmedLine;

    while (std::getline(inputFile, line)) {
        trimmedLine = trim(line);

        // Add a newline before starting a new section, but not before the first section
        if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            if (isNewSection) {
                outputFile << '\n'; // Add an extra newline to separate sections
            }
            isNewSection = true;
        }

        if (!trimmedLine.empty()) {
            outputFile << trimmedLine << '\n';
        }
    }

    // Close both files
    inputFile.close();
    outputFile.close();

    // Replace the original file with the cleaned up version
    std::remove(filePath.c_str());
    std::rename(tempPath.c_str(), filePath.c_str());
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
void setIniFile(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& desiredNewKey = "", const std::string& comment = "") {
    std::ios::sync_with_stdio(false);  // Disable synchronization between C++ and C I/O.

    std::ifstream configFile(fileToEdit);
    std::stringstream buffer;  // Use stringstream to buffer the output.

    bool sectionFound = false;
    bool keyFound = false;
    bool firstSection = true;  // Flag to control new line before first section
    std::string line, currentSection;

    std::string trimmedLine;
    size_t delimiterPos;
    std::string key;

    while (getline(configFile, line)) {
        trimmedLine = trim(line);

        if (trimmedLine.empty()) {
            continue;  // Skip empty lines but do not add them to the buffer
        }

        if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            if (sectionFound && !keyFound) {
                buffer << desiredKey << "=" << desiredValue << '\n';  // Add missing key-value pair
                keyFound = true;
            }
            if (!firstSection) {
                buffer << '\n';  // Add a newline before the start of a new section
            }
            currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
            sectionFound = (currentSection == desiredSection);
            buffer << line << '\n';
            firstSection = false;
            continue;
        }

        if (sectionFound && !keyFound && trimmedLine.find('=') != std::string::npos) {
            delimiterPos = trimmedLine.find('=');
            key = trim(trimmedLine.substr(0, delimiterPos));
            if (key == desiredKey) {
                keyFound = true;
                trimmedLine = (desiredNewKey.empty() ? desiredKey : desiredNewKey) + "=" + desiredValue;
            }
        }

        buffer << trimmedLine << '\n';
    }

    if (!sectionFound && !keyFound) {
        if (!firstSection) buffer << '\n';  // Ensure newline before adding a new section, unless it's the first section
        buffer << '\n' << '[' << desiredSection << ']' << '\n';
        buffer << desiredKey << "=" << desiredValue << '\n';
    } else if (!keyFound) {
        buffer << desiredKey << "=" << desiredValue << '\n';
    }

    configFile.close();

    std::ofstream outFile(fileToEdit);
    outFile << buffer.str();
    outFile.close();
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
void setIniFileValue(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& comment="") {
    setIniFile(fileToEdit, desiredSection, desiredKey, desiredValue, "", comment);
    //cleanIniFormatting(fileToEdit);
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
void setIniFileKey(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredNewKey, const std::string& comment="") {
    setIniFile(fileToEdit, desiredSection, desiredKey, "", desiredNewKey, comment);
    //cleanIniFormatting(fileToEdit);
}




/**
 * @brief Adds a new section to an INI file.
 *
 * This function adds a new section with the specified name to the INI file located at the
 * specified path. If the section already exists, it does nothing.
 *
 * @param filePath The path to the INI file.
 * @param sectionName The name of the section to add.
 */
void addIniSection(const std::string& filePath, const std::string& sectionName) {
    // Open the input file to check if it exists and read from it
    std::ifstream inputFile(filePath);
    if (!inputFile) {
        logMessage("Error: Failed to open INI file for reading.");
        return;
    }
    
    // Create a temporary file
    std::string tempPath = filePath + ".tmp";
    std::ofstream tempFile(tempPath);
    if (!tempFile) {
        logMessage("Error: Failed to create a temporary file.");
        return;
    }

    std::string line;
    bool sectionExists = false;
    std::string fullSectionName = "[" + sectionName + "]";

    // Read through the file and copy lines to the temporary file
    while (std::getline(inputFile, line)) {
        // Check if the current line is the section to be added
        if (line.compare(0, fullSectionName.length(), fullSectionName) == 0) {
            sectionExists = true;
            break;  // Section already exists, no need to continue
        }
        tempFile << line << '\n';
    }

    // If the section does not exist, add it
    if (!sectionExists) {
        tempFile << fullSectionName << '\n';
        // Continue copying the rest of the file if needed
        while (std::getline(inputFile, line)) {
            tempFile << line << '\n';
        }
    } else {
        // If the section exists, finish copying the rest of the file
        do {
            tempFile << line << '\n';
        } while (std::getline(inputFile, line));
    }

    inputFile.close();
    tempFile.close();

    // Replace the original file with the temp file
    if (std::remove(filePath.c_str()) != 0) {
        logMessage("Failed to delete the original file.");
        return;
    }
    
    if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
        logMessage("Failed to rename the temporary file.");
    }
}




/**
 * @brief Renames a section in an INI file.
 *
 * This function renames the section with the specified current name to the specified new name
 * in the INI file located at the specified path. If the current section does not exist, or if the
 * new section name already exists, it does nothing.
 *
 * @param filePath The path to the INI file.
 * @param currentSectionName The name of the section to rename.
 * @param newSectionName The new name for the section.
 */
void renameIniSection(const std::string& filePath, const std::string& currentSectionName, const std::string& newSectionName) {
    std::ifstream configFile(filePath);
    if (!configFile) {
        logMessage("Failed to open the input file: " + filePath);
        return;
    }

    std::string tempPath = filePath + ".tmp";
    std::ofstream tempFile(tempPath);
    if (!tempFile) {
        logMessage("Failed to create the temporary file: " + tempPath);
        return;
    }

    std::string line, trimmedLine, sectionName;

    while (getline(configFile, line)) {
        trimmedLine = trim(line);

        if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            sectionName = trimmedLine.substr(1, trimmedLine.length() - 2);

            if (sectionName == currentSectionName) {
                tempFile << "[" << newSectionName << "]\n";
            } else {
                tempFile << line << '\n';
            }
        } else {
            tempFile << line << '\n';
        }
    }

    configFile.close();
    tempFile.close();

    // Replace the original file with the modified temporary file
    if (remove(filePath.c_str()) != 0) {
        logMessage("Failed to delete the original file: " + filePath);
        return;
    }

    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        logMessage("Failed to rename the temporary file: " + tempPath);
    }
}




/**
 * @brief Removes a section from an INI file.
 *
 * This function removes the section with the specified name, including all its associated key-value
 * pairs, from the INI file located at the specified path. If the section does not exist in the file,
 * it does nothing.
 *
 * @param filePath The path to the INI file.
 * @param sectionName The name of the section to remove.
 */
void removeIniSection(const std::string& filePath, const std::string& sectionName) {
    std::ifstream configFile(filePath);
    if (!configFile) {
        logMessage("Failed to open the input file: " + filePath);
        return; // Handle the error accordingly
    }

    std::string tempPath = filePath + ".tmp";
    std::ofstream tempFile(tempPath);
    if (!tempFile) {
        logMessage("Failed to create the temporary file: " + tempPath);
        return; // Handle the error accordingly
    }

    std::string line, currentSection;
    bool inSectionToRemove = false;

    std::string trimmedLine;

    while (getline(configFile, line)) {
        trimmedLine = trim(line);

        // Check if the line represents a section
        if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            currentSection = trimmedLine.substr(1, trimmedLine.length() - 2);

            if (currentSection == sectionName) {
                // Mark that we are in the section to remove
                inSectionToRemove = true;
            } else {
                // We've reached a new section, so stop removing
                inSectionToRemove = false;
                tempFile << line << '\n'; // Write this new section header
            }
        } else if (!inSectionToRemove) {
            // Write lines that are not part of the section to remove
            tempFile << line << '\n';
        }
    }

    configFile.close();
    tempFile.close();

    // Replace the original file with the temp file
    if (remove(filePath.c_str()) != 0) {
        logMessage("Failed to delete the original file: " + filePath);
        return; // Handle the error accordingly
    }
    
    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        logMessage("Failed to rename the temporary file: "+ tempPath);
        // Handle the error accordingly
    }
}


// Removes a key-value pair from an ini accordingly.
void removeIniKey(const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
    std::ifstream configFile(filePath);
    if (!configFile) {
        logMessage("Failed to open the input file: " + filePath);
        return; // Handle the error accordingly
    }

    std::string tempPath = filePath + ".tmp";
    std::ofstream tempFile(tempPath);
    if (!tempFile) {
        logMessage("Failed to create the temporary file: " + tempPath);
        return; // Handle the error accordingly
    }

    std::string line, currentSection;
    bool inTargetSection = false;
    std::string trimmedLine;

    while (getline(configFile, line)) {
        trimmedLine = trim(line);

        // Check if the line represents a section
        if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            currentSection = trimmedLine.substr(1, trimmedLine.length() - 2);

            if (currentSection == sectionName) {
                // We are in the target section
                inTargetSection = true;
            } else {
                // We've left the target section
                inTargetSection = false;
            }
            tempFile << line << '\n'; // Always write section headers
        } else if (inTargetSection && trimmedLine.find(keyName + "=") == 0) {
            // If we're in the target section and the line starts with the key, skip it
            continue;
        } else {
            // Write lines that are not part of the key to remove
            tempFile << line << '\n';
        }
    }

    configFile.close();
    tempFile.close();

    // Replace the original file with the temp file
    if (remove(filePath.c_str()) != 0) {
        logMessage("Failed to delete the original file: " + filePath);
        return; // Handle the error accordingly
    }
    
    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        logMessage("Failed to rename the temporary file: "+ tempPath);
        // Handle the error accordingly
    }
}
