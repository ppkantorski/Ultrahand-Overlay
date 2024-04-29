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
#include <get_funcs.hpp>
#include <path_funcs.hpp>

//constexpr size_t BufferSize = 4096*3;//131072;

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
    if (!file.is_open()) {
        logMessage("Failed to open the file: " +filePath);
        return packageHeader; // Return empty header if file cannot be opened
    }

    std::string line;
    const std::map<std::string, std::string*> headerFields = {
        {";title=", &packageHeader.title},
        {";version=", &packageHeader.version},
        {";creator=", &packageHeader.creator},
        {";about=", &packageHeader.about},
        {";credits=", &packageHeader.credits},
        {";color=", &packageHeader.color}
    };

    while (getline(file, line)) {
        for (const auto& field : headerFields) {
            size_t pos = line.find(field.first);
            if (pos != std::string::npos) {
                pos += field.first.length();
                size_t endPos = line.find('\n', pos);
                std::string value = trim(line.substr(pos, endPos - pos));
                size_t quotePos = value.find("'");
                if (quotePos != std::string::npos) {
                    size_t endQuote = value.find("'", quotePos + 1);
                    *field.second = value.substr(quotePos + 1, endQuote - quotePos - 1);
                } else {
                    *field.second = value;
                }
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
    
    size_t current, previous = 0;
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
    std::ifstream configFile(configIniPath);
    if (!configFile.is_open()) {
        logMessage("Failed to open the file: " + configIniPath);
        return parsedData; // Return empty map if file cannot be opened
    }

    std::string line;
    std::string currentSection;

    while (getline(configFile, line)) {
        std::string trimmedLine = trim(line);
        if (trimmedLine.empty()) {
            continue;
        }

        // Check if the line contains a section header
        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
        } else {
            size_t delimiterPos = trimmedLine.find('=');
            if (delimiterPos != std::string::npos) {
                std::string key = trim(trimmedLine.substr(0, delimiterPos));
                std::string value = trim(trimmedLine.substr(delimiterPos + 1));
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
    if (!file.is_open()) {
        logMessage("Failed to open the input file: ");
        return sections; // Early return if the file cannot be opened
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

    // file will automatically close when it goes out of scope
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
    std::ifstream file(filePath);
    if (!file.is_open()) {
        logMessage("Failed to open the file: " + filePath);
        return ""; // Early return if the file cannot be opened
    }

    std::string line;
    std::string currentSection = "";

    while (getline(file, line)) {
        std::string trimmedLine = trim(line);

        if (trimmedLine.empty()) {
            continue;
        }

        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
            if (currentSection != sectionName) {
                continue; // Skip processing other sections
            }
        } else if (currentSection == sectionName) {
            size_t delimiterPos = trimmedLine.find('=');
            if (delimiterPos != std::string::npos) {
                std::string currentKey = trim(trimmedLine.substr(0, delimiterPos));
                if (currentKey == keyName) {
                    return trim(trimmedLine.substr(delimiterPos + 1)); // Found the key, return the value
                }
            }
        }
    }

    return ""; // Return an empty string if the key or section isn't found
}


//std::string parseValueFromIniSectionF(FILE*& file, const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
//    std::string value = "";
//    
//    //FILE* file = fopen(filePath.c_str(), "r");
//    if (file == nullptr) {
//        return value; // Return an empty string if the file cannot be opened
//    }
//    
//    std::string currentSection = "";
//    //constexpr size_t BufferSize = 4096;
//    char line[BufferSize];
//    
//    std::string trimmedLine;
//    size_t delimiterPos;
//    
//    while (fgets(line, sizeof(line), file)) {
//        trimmedLine = trim(std::string(line));
//        
//        if (!trimmedLine.empty()) {
//            if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
//                // This line is a section header
//                currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
//            } else if (currentSection == sectionName) {
//                // Check if the line is within the desired section and contains the desired key
//                delimiterPos = trimmedLine.find('=');
//                if (delimiterPos != std::string::npos) {
//                    std::string currentKey = trim(trimmedLine.substr(0, delimiterPos));
//                    if (currentKey == keyName) {
//                        value = trim(trimmedLine.substr(delimiterPos + 1));
//                        break; // Found the key, exit the loop
//                    }
//                }
//            }
//        }
//    }
//    
//    //fclose(file);
//    
//    return value;
//}





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
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        logMessage("Failed to open the input file: " + filePath);
        return;
    }

    std::ofstream outputFile(tempPath);
    if (!outputFile.is_open()) {
        logMessage("Failed to create the output file: " + tempPath);
        return; // inputFile will automatically close
    }

    std::string line;
    //bool isNewSection = false;
    bool firstSectionFound = false;

    while (getline(inputFile, line)) {
        std::string trimmedLine = trim(line);

        if (!trimmedLine.empty()) {
            // Add a newline before starting a new section, but not before the first section
            if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
                if (firstSectionFound) {
                    outputFile << "\n"; // Add a newline before the section
                } else {
                    firstSectionFound = true; // Mark the first section as found
                }
            }
            outputFile << trimmedLine << "\n";
        }
    }

    // inputFile and outputFile will automatically close when they go out of scope

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
    std::ifstream configFile(fileToEdit);
    std::ostringstream updatedContent;
    bool sectionFound = false;
    bool keyFound = false;

    if (!configFile && !configFile.is_open()) {
        // File does not exist, so we create a new one with the desired content
        std::ofstream newFile(fileToEdit);
        newFile << comment;
        if (!comment.empty()) newFile << "\n";
        newFile << "[" << desiredSection << "]\n" << desiredKey << " = " << desiredValue << "\n";
        newFile.close();
        return;
    }

    std::string line;
    std::string currentSection;

    while (getline(configFile, line)) {
        std::string trimmedLine = trim(line);

        if (trimmedLine.empty()) continue;

        if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
            if (currentSection == desiredSection) {
                sectionFound = true;
                updatedContent << line << "\n";
                continue;
            }
        }

        if (sectionFound && !keyFound && trimmedLine.find('=') != std::string::npos) {
            auto delimiterPos = trimmedLine.find('=');
            std::string key = trim(trimmedLine.substr(0, delimiterPos));

            if (key == desiredKey) {
                keyFound = true;
                if (desiredNewKey.empty()) {
                    updatedContent << desiredKey << " = " << desiredValue << "\n";
                } else {
                    updatedContent << desiredNewKey << " = " << desiredValue << "\n";
                }
                continue;
            }
        }

        updatedContent << line << "\n";
    }

    if (!sectionFound) {
        updatedContent << "\n[" << desiredSection << "]\n";
    }
    if (!keyFound) {
        updatedContent << desiredKey << " = " << desiredValue << "\n";
    }

    configFile.close();

    // Rewrite the file
    std::ofstream outFile(fileToEdit);
    if (outFile.is_open()) {
        outFile << updatedContent.str();
        outFile.close();
    } else {
        // Handle file opening error
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
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        logMessage("Error: INI file not found or failed to open.");
        return;
    }

    std::ofstream tempFile("temp.ini");
    if (!tempFile.is_open()) {
        logMessage("Error: Failed to create a temporary file.");
        return;
    }

    std::string line;
    bool sectionExists = false;
    std::string sectionHeader = "[" + sectionName + "]";

    while (getline(inputFile, line)) {
        // Check if the current line contains the section
        if (line.find(sectionHeader) == 0) {
            sectionExists = true;
            // Break since the section already exists; no need to add it again
            break;
        }
        tempFile << line << "\n";
    }

    // Only add the section if it does not exist
    if (!sectionExists) {
        tempFile << sectionHeader << "\n";
    }

    // Copy the rest of the input file to the temp file if not done
    while (getline(inputFile, line)) {
        tempFile << line << "\n";
    }

    inputFile.close();
    tempFile.close();

    // Replace the original file with the temp file
    std::remove(filePath.c_str());  // Delete the old configuration file
    std::rename("temp.ini", filePath.c_str());  // Rename the temp file to the original name

    logMessage("Section '" + sectionName + "' added to the INI file.");
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
    if (!configFile.is_open()) {
        logMessage("Error: Failed to open INI file.");
        return;
    }

    std::string tempPath = filePath + ".tmp";
    std::ofstream tempFile(tempPath);
    if (!tempFile.is_open()) {
        logMessage("Error: Failed to create a temporary file.");
        configFile.close();
        return;
    }

    std::string line;
    bool sectionExists = false;
    bool newNameExists = false;

    while (getline(configFile, line)) {
        std::string trimmedLine = trim(line);

        if (trimmedLine.size() > 2 && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            std::string sectionHeader = trimmedLine.substr(1, trimmedLine.size() - 2);
            
            if (sectionHeader == currentSectionName) {
                tempFile << "[" << newSectionName << "]\n";
                sectionExists = true;
                continue;
            } else if (sectionHeader == newSectionName) {
                newNameExists = true;
                break; // Exit if the new section name already exists
            }
        }

        tempFile << line << "\n";
    }

    configFile.close();
    tempFile.close();

    if (sectionExists && !newNameExists) {
        std::remove(filePath.c_str());
        std::rename(tempPath.c_str(), filePath.c_str());
    } else {
        // If the section doesn't exist or the new section name already exists, delete the temp file
        std::remove(tempPath.c_str());
        if (!sectionExists) {
            logMessage("Error: The section to rename does not exist.");
        }
        if (newNameExists) {
            logMessage("Error: The new section name already exists.");
        }
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
    if (!configFile.is_open()) {
        logMessage("Error opening INI file");
        return;
    }

    std::string tempPath = filePath + ".tmp";
    std::ofstream tempFile(tempPath);
    if (!tempFile.is_open()) {
        logMessage("Error creating temporary file");
        configFile.close();
        return;
    }

    std::string line;
    bool removing = false;
    std::string sectionHeader = "[" + sectionName + "]";

    while (getline(configFile, line)) {
        std::string trimmedLine = trim(line);

        if (trimmedLine.empty()) continue;

        if (trimmedLine.front() == '[' && trimmedLine.back() == ']') {
            removing = (trimmedLine == sectionHeader);
        }

        if (!removing) {
            tempFile << line << '\n';
        }
    }

    configFile.close();
    tempFile.close();

    if (remove(filePath.c_str()) != 0) {
        logMessage("Error deleting original file");
        return;
    }

    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        logMessage("Error renaming temporary file");
    }
}


