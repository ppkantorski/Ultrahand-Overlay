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
//#include <sstream>  // For std::istringstream
#include <algorithm> // For std::remove_if
//#include <cctype>   // For ::isspace
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
                std::string newLine = line.substr(startPos, endPos - startPos);
                trim(newLine);
                removeQuotes(newLine);
                *field = newLine;
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
    
    if (str.empty()) {
        return out;
    }
    
    // Reserve space assuming the worst case where every character is a delimiter
    out.reserve(std::count(str.begin(), str.end(), delim) + 1);
    
    size_t start = 0;
    size_t end = str.find(delim);
    
    while (end != std::string::npos) {
        out.emplace_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delim, start);
    }
    out.emplace_back(str.substr(start));
    
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
    
    //std::string trimmedLine;
    
    size_t delimiterPos;
    //std::string key, value;
    
    for (auto& line : lines) {
        trim(line);
        
        if (line.empty() || line.front() == '#') {
            // Ignore empty lines and comments
            continue;
        }
        
        if (line.front() == '[' && line.back() == ']') {
            lastHeader = line.substr(1, line.size() - 2);
            iniData[lastHeader]; // Ensures the section exists even if it remains empty
        }
        else {
            delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                //key = trim(trimmedLine.substr(0, delimiterPos));
                //value = trim(trimmedLine.substr(delimiterPos + 1));
                if (!lastHeader.empty()) {
                    //iniData[lastHeader][key] = value;
                    std::string newLine1 = line.substr(0, delimiterPos);
                    trim(newLine1);
                    std::string newLine2 = line.substr(delimiterPos + 1);
                    trim(newLine2);
                    iniData[lastHeader][newLine1] = newLine2;
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
        //logMessage("Failed to open the file: " + configIniPath);
        return parsedData;  // Return empty map if file cannot be opened
    }
    
    std::string line, currentSection;
    //std::string trimmedLine;
    
    size_t delimiterPos;
    std::string key, value;
    
    while (getline(configFile, line)) {
        trim(line);
        
        if (line.empty()) continue;
        
        if (line.front() == '[' && line.back() == ']') {
            // Remove the brackets and set the current section
            currentSection = line.substr(1, line.size() - 2);
        } else {
            delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                key = line.substr(0, delimiterPos);
                trim(key);

                value = line.substr(delimiterPos + 1);
                trim(value);

                parsedData[currentSection][key] = value;
            }
        }
    }
    
    return parsedData;
}


/**
 * @brief Parses an INI file and retrieves key-value pairs from a specific section.
 *
 * This function reads the contents of an INI file located at the specified path,
 * and returns the key-value pairs within a specific section.
 *
 * @param configIniPath The path to the INI file to be parsed.
 * @param sectionName The name of the section to retrieve key-value pairs from.
 * @return A map representing the key-value pairs in the specified section.
 */
std::map<std::string, std::string> getKeyValuePairsFromSection(const std::string& configIniPath, const std::string& sectionName) {
    std::map<std::string, std::string> sectionData;
    std::ifstream configFile(configIniPath);
    if (!configFile) {
        //logMessage("Failed to open the file: " + configIniPath);
        return sectionData;  // Return empty map if file cannot be opened
    }
    
    std::string line, currentSection;
    //std::string trimmedLine;
    
    size_t delimiterPos;
    std::string key, value;
    bool inTargetSection = false;  // To track if we're in the desired section
    
    while (getline(configFile, line)) {
        trim(line);
        
        if (line.empty()) continue; // Skip empty lines
        
        if (line.front() == '[' && line.back() == ']') {
            // Remove the brackets to get the section name
            currentSection = line.substr(1, line.size() - 2);
            // Check if this is the section we're interested in
            inTargetSection = (currentSection == sectionName);
        } else if (inTargetSection) {
            // Look for key-value pairs within the target section
            delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                key = line.substr(0, delimiterPos);
                trim(key);
                value = line.substr(delimiterPos + 1);
                trim(value);
                sectionData[key] = value;  // Store the key-value pair
            }
        }
    }

    return sectionData;
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
        //logMessage("Failed to open the file: " + filePath);
        return sections;  // Early return if the file cannot be opened
    }
    
    std::string line;
    //std::string trimmedLine;

    while (std::getline(file, line)) {
        trim(line);
        
        // Check if the line contains a section header
        if (!line.empty() && line.front() == '[' && line.back() == ']') {
            std::string sectionName = line.substr(1, line.size() - 2);
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
        //logMessage("Failed to open the file: " + filePath);
        return value;  // Return empty if the file cannot be opened
    }
    
    std::string line, currentSection;
    
    size_t delimiterPos;
    std::string currentKey;
    //std::string trimmedLine;
    
    while (std::getline(file, line)) {
        trim(line);
        
        if (line.empty()) continue;
        
        if (line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.size() - 2);
            if (currentSection != sectionName) continue;  // Skip processing other sections
        } else if (currentSection == sectionName) {
            delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                currentKey = line.substr(0, delimiterPos);
                trim(currentKey);
                if (currentKey == keyName) {
                    value = line.substr(delimiterPos + 1);
                    trim(value);
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
    
    //std::string trimmedLine;
    
    while (std::getline(inputFile, line)) {
        trim(line);
        
        // Add a newline before starting a new section, but not before the first section
        if (!line.empty() && line.front() == '[' && line.back() == ']') {
            if (isNewSection) {
                outputFile << '\n'; // Add an extra newline to separate sections
            }
            isNewSection = true;
        }
        
        if (!line.empty()) {
            outputFile << line << '\n';
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
    
    if (!isFile(fileToEdit)) {
        createDirectory(getParentDirFromPath(fileToEdit));
    }
    
    std::ifstream configFile(fileToEdit);
    std::stringstream buffer;  // Use stringstream to buffer the output.
    
    bool sectionFound = false;
    bool keyFound = false;
    bool firstSection = true;  // Flag to control new line before first section
    std::string line, currentSection;
    
    //std::string trimmedLine;
    size_t delimiterPos;
    std::string key;
    
    while (getline(configFile, line)) {
        trim(line);
        
        if (line.empty()) {
            continue;  // Skip empty lines but do not add them to the buffer
        }
        
        if (line[0] == '[' && line.back() == ']') {
            if (sectionFound && !keyFound) {
                buffer << desiredKey << "=" << desiredValue << '\n';  // Add missing key-value pair
                keyFound = true;
            }
            if (!firstSection) {
                buffer << '\n';  // Add a newline before the start of a new section
            }
            currentSection = line.substr(1, line.size() - 2);
            sectionFound = (currentSection == desiredSection);
            buffer << line << '\n';
            firstSection = false;
            continue;
        }
        
        if (sectionFound && !keyFound && line.find('=') != std::string::npos) {
            delimiterPos = line.find('=');
            key = line.substr(0, delimiterPos);
            trim(key);
            if (key == desiredKey) {
                keyFound = true;
                line = (desiredNewKey.empty() ? desiredKey : desiredNewKey) + "=" + desiredValue;
            }
        }
        
        buffer << line << '\n';
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

    std::string line, sectionName;

    while (getline(configFile, line)) {
        trim(line);

        if (!line.empty() && line.front() == '[' && line.back() == ']') {
            sectionName = line.substr(1, line.length() - 2);

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

    //std::string trimmedLine;

    while (getline(configFile, line)) {
        trim(line);

        // Check if the line represents a section
        if (!line.empty() && line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);

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
    //std::string trimmedLine;

    while (getline(configFile, line)) {
        trim(line);

        // Check if the line represents a section
        if (!line.empty() && line.front() == '[' && line.back() == ']') {
            currentSection = line.substr(1, line.length() - 2);

            if (currentSection == sectionName) {
                // We are in the target section
                inTargetSection = true;
            } else {
                // We've left the target section
                inTargetSection = false;
            }
            tempFile << line << '\n'; // Always write section headers
        } else if (inTargetSection && line.find(keyName + "=") == 0) {
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


//void saveIniFileData(const std::string& filePath, const std::map<std::string, std::map<std::string, std::string>>& data) {
//    std::ofstream file(filePath);
//    if (!file.is_open()) {
//        // Handle error: could not open file
//        return;
//    }
//
//    for (const auto& section : data) {
//        file << "[" << section.first << "]\n";
//        for (const auto& kv : section.second) {
//            file << kv.first << "=" << kv.second << "\n";
//        }
//        file << "\n"; // Separate sections with a newline
//    }
//
//    file.close();
//}


void updateIniData(const std::map<std::string, std::map<std::string, std::string>>& packageConfigData,
                   const std::string& packageConfigIniPath,
                   const std::string& optionName,
                   const std::string& key,
                   std::string& value) {
    auto optionIt = packageConfigData.find(optionName);
    if (optionIt != packageConfigData.end()) {
        auto it = optionIt->second.find(key);
        if (it != optionIt->second.end()) {
            value = it->second;  // Update value only if the key exists
        } else {
            setIniFileValue(packageConfigIniPath, optionName, key, value); // Set INI file value if key not found
        }
    }
}


/**
 * @brief Parses a command line into individual parts, handling quoted strings.
 *
 * @param line The command line to parse.
 * @return A vector of strings containing the parsed command parts.
 */
std::vector<std::string> parseCommandLine(const std::string& line) {
    std::vector<std::string> commandParts;
    bool inQuotes = false;
    std::string part;

    std::istringstream iss(line);
    while (std::getline(iss, part, '\'')) { // Handle single quotes
        if (inQuotes) {
            commandParts.push_back(part); // Inside quotes, treat as a whole argument
        } else {
            std::istringstream argIss(part);
            std::string arg;
            while (argIss >> arg) {
                commandParts.push_back(arg); // Split part outside quotes by spaces
            }
        }
        inQuotes = !inQuotes; // Toggle the inQuotes flag
    }

    return commandParts;
}



/**
 * @brief Loads and parses options from an INI file.
 *
 * This function reads and parses options from an INI file, organizing them by section.
 *
 * @param packageIniPath The path to the INI file.
 * @return A vector containing pairs of section names and their associated key-value pairs.
 */
std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& packageIniPath) {
    std::ifstream packageFile(packageIniPath);
    
    if (!packageFile) return {}; // Return empty vector if file can't be opened

    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;
    std::string line, currentSection;
    std::vector<std::vector<std::string>> sectionCommands;

    while (std::getline(packageFile, line)) {
        // Remove carriage returns and newlines
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        if (line.empty() || line.front() == '#') continue; // Skip empty or comment lines

        if (line.front() == '[' && line.back() == ']') { // Section headers
            if (!currentSection.empty()) {
                options.emplace_back(std::move(currentSection), std::move(sectionCommands));
                sectionCommands.clear();
            }
            currentSection = line.substr(1, line.size() - 2);
        } else if (!currentSection.empty()) { // Command lines within sections
            sectionCommands.push_back(parseCommandLine(line)); // Use helper to parse command line
        }
    }

    if (!currentSection.empty()) {
        options.emplace_back(std::move(currentSection), std::move(sectionCommands));
    }
    packageFile.close();

    return options;
}

/**
 * @brief Loads a specific section from an INI file.
 *
 * This function reads and parses a specific section from an INI file.
 *
 * @param packageIniPath The path to the INI file.
 * @param sectionName The name of the section to load.
 * @return A vector of commands within the specified section.
 */
std::vector<std::vector<std::string>> loadSpecificSectionFromIni(const std::string& packageIniPath, const std::string& sectionName) {
    std::ifstream packageFile(packageIniPath);

    if (!packageFile) return {}; // Return empty vector if file can't be opened

    std::string line, currentSection;
    std::vector<std::vector<std::string>> sectionCommands;
    bool inTargetSection = false;

    while (std::getline(packageFile, line)) {
        // Remove carriage returns and newlines
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

        if (line.empty() || line.front() == '#') continue; // Skip empty or comment lines

        if (line.front() == '[' && line.back() == ']') { // Section headers
            currentSection = line.substr(1, line.size() - 2);
            inTargetSection = (currentSection == sectionName); // Check if this is the target section
        } else if (inTargetSection) { // Only parse commands within the target section
            sectionCommands.push_back(parseCommandLine(line)); // Use helper to parse command line
        }
    }

    packageFile.close();
    return sectionCommands; // Return only the commands from the target section
}







// Helper functions
//std::vector<std::string> getIniSections(const std::string& iniPath) {
//    std::ifstream file(iniPath);
//    std::vector<std::string> sections;
//    std::string line;
//    while (std::getline(file, line)) {
//        line = trim(line);
//        if (!line.empty() && line[0] == '[' && line.back() == ']') {
//            sections.push_back(line.substr(1, line.size() - 2));  // Extract section name without brackets
//        }
//    }
//    return sections;
//}
