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
 *  Licensed under CC BY-NC-SA 4.0
 *  Copyright (c) 2023 ppkantorski
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
#include <get_funcs.hpp>
#include <path_funcs.hpp>


constexpr size_t BufferSize = 4096;//131072;

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
    
    FILE* file = fopen(filePath.c_str(), "r");
    if (file == nullptr) {
        packageHeader.title = "";
        packageHeader.version = "";
        packageHeader.creator = "";
        packageHeader.about = "";
        packageHeader.credits = "";
        packageHeader.color = "";
        return packageHeader;
    }
    
    //constexpr size_t BufferSize = 131072; // Choose a larger buffer size for reading lines
    char line[BufferSize];
    
    const std::string titlePrefix = ";title=";
    const std::string versionPrefix = ";version=";
    const std::string creatorPrefix = ";creator=";
    const std::string aboutPrefix = ";about=";
    const std::string creditsPrefix = ";credits=";
    const std::string colorPrefix = ";color=";
    
    size_t titlePos, versionPos, creatorPos, aboutPos, creditsPos, colorPos, startPos, endPos;
    std::string strLine;
    while (fgets(line, sizeof(line), file)) {
        strLine = line;
        
        titlePos = strLine.find(titlePrefix);
        if (titlePos != std::string::npos) {
            titlePos += titlePrefix.length();
            startPos = strLine.find("'", titlePos);
            endPos = strLine.find("'", startPos + 1);
            
            if (startPos != std::string::npos && endPos != std::string::npos) {
                // Value enclosed in single quotes
                packageHeader.title = strLine.substr(startPos + 1, endPos - startPos - 1);
            } else {
                // Value not enclosed in quotes
                packageHeader.title = strLine.substr(titlePos, endPos - titlePos);
            }
            
            // Remove trailing whitespace or newline characters
            packageHeader.title.erase(packageHeader.title.find_last_not_of(" \t\r\n") + 1);
        }
        
        versionPos = strLine.find(versionPrefix);
        if (versionPos != std::string::npos) {
            versionPos += versionPrefix.length();
            startPos = strLine.find("'", versionPos);
            endPos = strLine.find("'", startPos + 1);
            
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
        
        creatorPos = strLine.find(creatorPrefix);
        if (creatorPos != std::string::npos) {
            creatorPos += creatorPrefix.length();
            startPos = strLine.find("'", creatorPos);
            endPos = strLine.find("'", startPos + 1);
            
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
        
        aboutPos = strLine.find(aboutPrefix);
        if (aboutPos != std::string::npos) {
            aboutPos += aboutPrefix.length();
            startPos = strLine.find("'", aboutPos);
            endPos = strLine.find("'", startPos + 1);
            
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
        
        creditsPos = strLine.find(creditsPrefix);
        if (creditsPos != std::string::npos) {
            creditsPos += creditsPrefix.length();
            startPos = strLine.find("'", creditsPos);
            endPos = strLine.find("'", startPos + 1);
            
            if (startPos != std::string::npos && endPos != std::string::npos) {
                // Value enclosed in single quotes
                packageHeader.credits = strLine.substr(startPos + 1, endPos - startPos - 1);
            } else {
                // Value not enclosed in quotes
                packageHeader.credits = strLine.substr(creditsPos, endPos - creditsPos);
            }
            
            // Remove trailing whitespace or newline characters
            packageHeader.credits.erase(packageHeader.credits.find_last_not_of(" \t\r\n") + 1);
        }
        
        colorPos = strLine.find(colorPrefix);
        if (colorPos != std::string::npos) {
            colorPos += colorPrefix.length();
            startPos = strLine.find("'", colorPos);
            endPos = strLine.find("'", startPos + 1);
            
            if (startPos != std::string::npos && endPos != std::string::npos) {
                // Value enclosed in single quotes
                packageHeader.color = strLine.substr(startPos + 1, endPos - startPos - 1);
            } else {
                // Value not enclosed in quotes
                packageHeader.color = strLine.substr(colorPos, endPos - colorPos);
            }
            
            // Remove trailing whitespace or newline characters
            packageHeader.color.erase(packageHeader.color.find_last_not_of(" \t\r\n") + 1);
        }
        
        
        if (!packageHeader.version.empty() && !packageHeader.creator.empty() && !packageHeader.about.empty() && !packageHeader.credits.empty() && !packageHeader.color.empty()) {
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
    size_t delimiterPos;
    std::string key, value;
    while (std::getline(fileStream, line)) {
        // Remove leading and trailing whitespace
        line = trim(line);
        
        // Check if this line is a section
        if (line.size() > 2 && line.front() == '[' && line.back() == ']') {
            // Remove the brackets to get the section name
            currentSection = line.substr(1, line.size() - 2);
        } else {
            // If not a section, parse as key-value pair
            delimiterPos = line.find('=');
            if (delimiterPos != std::string::npos) {
                key = trim(line.substr(0, delimiterPos));
                value = trim(line.substr(delimiterPos + 1));
                
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
    
    //constexpr size_t BufferSize = 131072;
    char line[BufferSize];
    
    std::string trimmedLine;
    while (fgets(line, sizeof(line), file)) {
        trimmedLine = trim(std::string(line));
        
        if (!trimmedLine.empty() && trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            // Extract section name and add it to the list
            std::string sectionName = trimmedLine.substr(1, trimmedLine.size() - 2);
            sections.push_back(sectionName);
        }
    }
    
    fclose(file);
    return sections;
}



std::string parseValueFromIniSection(const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
    std::string value = "";
    
    FILE* file = fopen(filePath.c_str(), "r");
    if (file == nullptr) {
        return value; // Return an empty string if the file cannot be opened
    }
    
    std::string currentSection = "";
    char line[BufferSize];
    
    size_t delimiterPos;
    
    std::string trimmedLine;
    std::string currentKey;
    
    while (fgets(line, sizeof(line), file)) {
        trimmedLine = trim(std::string(line));
        
        if (!trimmedLine.empty()) {
            if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
                // This line is a section header
                currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
            } else if (currentSection == sectionName) {
                // Check if the line is within the desired section and contains the desired key
                delimiterPos = trimmedLine.find('=');
                if (delimiterPos != std::string::npos) {
                    currentKey = trim(trimmedLine.substr(0, delimiterPos));
                    if (currentKey == keyName) {
                        value = trim(trimmedLine.substr(delimiterPos + 1));
                        break; // Found the key, exit the loop
                    }
                }
            }
        }
    }
    
    fclose(file);
    
    return value;
}


std::string parseValueFromIniSectionF(FILE*& file, const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
    std::string value = "";
    
    //FILE* file = fopen(filePath.c_str(), "r");
    if (file == nullptr) {
        return value; // Return an empty string if the file cannot be opened
    }
    
    std::string currentSection = "";
    //constexpr size_t BufferSize = 131072;
    char line[BufferSize];
    
    std::string trimmedLine;
    size_t delimiterPos;
    
    while (fgets(line, sizeof(line), file)) {
        trimmedLine = trim(std::string(line));
        
        if (!trimmedLine.empty()) {
            if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
                // This line is a section header
                currentSection = trimmedLine.substr(1, trimmedLine.size() - 2);
            } else if (currentSection == sectionName) {
                // Check if the line is within the desired section and contains the desired key
                delimiterPos = trimmedLine.find('=');
                if (delimiterPos != std::string::npos) {
                    std::string currentKey = trim(trimmedLine.substr(0, delimiterPos));
                    if (currentKey == keyName) {
                        value = trim(trimmedLine.substr(delimiterPos + 1));
                        break; // Found the key, exit the loop
                    }
                }
            }
        }
    }
    
    //fclose(file);
    
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
    
    char line[BufferSize];
    
    std::string trimmedLine;
    while (fgets(line, sizeof(line), inputFile)) {
        trimmedLine = trim(std::string(line));
        
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
void setIniFile(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& desiredNewKey, const std::string& comment) {
    FILE* configFile = fopen(fileToEdit.c_str(), "r");
    if (!configFile) {
        createDirectory(removeFilename(fileToEdit));
        configFile = fopen(fileToEdit.c_str(), "w");
        if (!configFile) {
            // Handle the error accordingly
            return;
        }
        fprintf(configFile, (comment+std::string("[%s]\n%s = %s\n")).c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
        fclose(configFile);
        return;
    }
    
    // Create a buffer to store the updated content
    std::string updatedContent;
    std::string currentSection;
    std::string formattedDesiredValue = trim(desiredValue);
    char line[BufferSize];
    
    bool sectionFound = false;
    bool keyFound = false;
    bool addNewLine = false;
    
    std::string trimmedLine;
    size_t delimiterPos;
    std::string lineKey, originalValue;
    
    while (fgets(line, sizeof(line), configFile)) {
        trimmedLine = trim(line);
        
        if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            currentSection = removeQuotes(trimmedLine.substr(1, trimmedLine.length() - 2));
            if (sectionFound && !keyFound && desiredNewKey.empty()) {
                if (!updatedContent.empty() && updatedContent.substr(updatedContent.length() - 2) == "\n\n") {
                    updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
                    addNewLine = true;
                }
                updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
                
                if (addNewLine) { // if it ended with \n\n, add one more newline
                    updatedContent += "\n";
                    addNewLine = false;
                }
                keyFound = true;
            }
        }
        
        if (sectionFound && !keyFound && desiredNewKey.empty() && trim(currentSection) != trim(desiredSection)) {
            if (!updatedContent.empty() && updatedContent.substr(updatedContent.length() - 2) == "\n\n") {
                updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
                addNewLine = true;
            }
            updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
            // Add a newline character if the last part of updatedContent initially had "\n"
            if (addNewLine) {
                updatedContent += "\n";
                addNewLine = false;
            }
            keyFound = true;
        }
        
        if (trim(currentSection) == trim(desiredSection)) {
            sectionFound = true;
            delimiterPos = trimmedLine.find('=');
            
            if (delimiterPos != std::string::npos) {
                lineKey = trim(trimmedLine.substr(0, delimiterPos));
                
                if (lineKey == desiredKey) {
                    keyFound = true;
                    originalValue = getValueFromLine(trimmedLine);
                    if (!updatedContent.empty() && updatedContent.substr(updatedContent.length() - 2) == "\n\n") {
                        updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
                        addNewLine = true;
                    }
                    
                    if (!desiredNewKey.empty()) {
                        updatedContent += desiredNewKey + " = " + originalValue + "\n";
                    } else {
                        updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
                    }
                    
                    // Add a newline character if the last part of updatedContent initially had "\n"
                    if (addNewLine) {
                        updatedContent += "\n";
                        addNewLine = false;
                    }
                    continue;
                }
            }
        }
        
        updatedContent += line;
    }
    
    if (sectionFound && !keyFound && desiredNewKey.empty()) {
        if (!updatedContent.empty() && updatedContent.substr(updatedContent.length() - 2) == "\n\n") {
            updatedContent = updatedContent.substr(0, updatedContent.length() - 1);
            addNewLine = true;
        }
        updatedContent += desiredKey + " = " + formattedDesiredValue + "\n";
        // Add a newline character if the last part of updatedContent initially had "\n"
        if (addNewLine) {
            updatedContent += "\n";
            addNewLine = false;
        }
    }

    if (!sectionFound && !keyFound && desiredNewKey.empty()) {
        updatedContent += "\n[" + desiredSection + "]\n" + desiredKey + " = " + formattedDesiredValue + "\n";
    }

    fclose(configFile);

    // Reopen the original file for writing and overwrite its content
    configFile = fopen(fileToEdit.c_str(), "w");
    if (!configFile) {
        // Handle the error accordingly
        return;
    }
    fprintf(configFile, "%s", updatedContent.c_str());
    fclose(configFile);
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
void addIniSection(const char* filePath, const char* sectionName) {
    if (!isFileOrDirectory(filePath)) {
        // INI file doesn't exist, handle the error accordingly
        //std::cerr << "Error: INI file not found." << std::endl;
        return;
    }
    
    // Read the existing contents of the INI file
    FILE* inputFile = fopen(filePath, "r");
    if (!inputFile) {
        //std::cerr << "Error: Failed to open INI file for reading." << std::endl;
        return;
    }
    
    FILE* tempFile = fopen("temp.ini", "w");
    if (!tempFile) {
        //std::cerr << "Error: Failed to create a temporary file." << std::endl;
        fclose(inputFile);
        return;
    }
    
    //constexpr size_t BufferSize = 131072;
    char line[BufferSize];
    bool sectionExists = false;
    while (fgets(line, sizeof(line), inputFile)) {
        // Check if the line contains the section
        if (line[0] == '[' && strncmp(&line[1], sectionName, strlen(sectionName)) == 0) {
            sectionExists = true;
            break;  // Section already exists, no need to continue
        }
        fputs(line, tempFile);
    }
    
    if (!sectionExists) {
        // Section doesn't exist, add it
        fprintf(tempFile, "[%s]\n", sectionName);
    }
    
    // Copy the rest of the input file to the temp file
    while (fgets(line, sizeof(line), inputFile)) {
        fputs(line, tempFile);
    }
    
    fclose(inputFile);
    fclose(tempFile);
    
    // Replace the original file with the temp file
    remove(filePath);  // Delete the old configuration file
    rename("temp.ini", filePath);  // Rename the temp file to the original name
    
    //std::cout << "Section '" << sectionName << "' added to the INI file." << std::endl;
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
    FILE* configFile = fopen(filePath.c_str(), "r");
    if (!configFile) {
        // The INI file doesn't exist, handle the error accordingly
        return;
    }
    
    std::string tempPath = filePath + ".tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "w");
    if (!tempFile) {
        // Failed to create a temporary file, handle the error accordingly
        fclose(configFile);
        return;
    }
    
    std::string currentSection;
    bool renaming = false;
    //constexpr size_t BufferSize = 131072;
    char line[BufferSize];
    
    std::string currentLine, sectionName;
    while (fgets(line, sizeof(line), configFile)) {
        currentLine = trim(std::string(line));
        
        // Check if the line represents a section
        if (currentLine.length() > 2 && currentLine.front() == '[' && currentLine.back() == ']') {
            sectionName = currentLine.substr(1, currentLine.size() - 2);
            
            if (sectionName == currentSectionName) {
                // We found the section to rename
                fprintf(tempFile, "[%s]\n", newSectionName.c_str());
                renaming = true;
            } else {
                // Copy the line as is
                fprintf(tempFile, "%s", currentLine.c_str());
                renaming = false;
            }
        } else if (renaming) {
            // Rename the section in the following lines
            fprintf(tempFile, "[%s]\n", newSectionName.c_str());
            renaming = false;
        } else {
            // Copy the line as is
            fprintf(tempFile, "%s", currentLine.c_str());
        }
    }
    
    fclose(configFile);
    fclose(tempFile);
    
    // Replace the original file with the temp file
    if (remove(filePath.c_str()) != 0) {
        // Failed to delete the original file, handle the error accordingly
        return;
    }
    
    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        // Failed to rename the temp file, handle the error accordingly
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
    FILE* configFile = fopen(filePath.c_str(), "r");
    if (!configFile) {
        // The INI file doesn't exist, or there was an error opening it.
        // Handle the error accordingly or return.
        return;
    }
    
    std::string tempPath = filePath + ".tmp";
    FILE* tempFile = fopen(tempPath.c_str(), "w");
    if (!tempFile) {
        // Failed to create a temporary file, handle the error accordingly
        fclose(configFile);
        // Handle the error or return.
        return;
    }
    
    std::string currentSection;
    bool removing = false;
    //constexpr size_t BufferSize = 131072;
    char line[BufferSize];
    
    std::string currentLine, _section;
    
    while (fgets(line, sizeof(line), configFile)) {
        currentLine = trim(std::string(line));
        
        // Check if the line represents a section
        if (currentLine.length() > 2 && currentLine.front() == '[' && currentLine.back() == ']') {
            _section = currentLine.substr(1, currentLine.size() - 2);
            
            if (_section == sectionName) {
                // We found the section to remove, so skip it and associated key-value pairs
                removing = true;
            } else {
                // Keep other sections
                fprintf(tempFile, "%s\n", currentLine.c_str());
                removing = false;
            }
        } else if (!removing) {
            // Keep lines outside of the section
            fprintf(tempFile, "%s\n", currentLine.c_str());
        }
    }
    
    fclose(configFile);
    fclose(tempFile);
    
    // Replace the original file with the temp file
    if (remove(filePath.c_str()) != 0) {
        // Failed to delete the original file, handle the error accordingly
        return;
    }
    
    if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
        // Failed to rename the temp file, handle the error accordingly
    }
}


