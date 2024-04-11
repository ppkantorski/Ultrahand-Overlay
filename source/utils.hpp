/********************************************************************************
 * File: utils.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains utility functions and macros used in the
 *   Ultrahand Overlay project. These functions and macros include definitions for
 *   various button keys, path variables, and command interpretation and execution.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 *
 *  Licensed under GPLv2
 *  Copyright (c) 2023 ppkantorski
 ********************************************************************************/

#pragma once
#include <switch.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <path_funcs.hpp>
#include <hex_funcs.hpp>
#include <download_funcs.hpp>
#include <list_funcs.hpp>
#include <mod_funcs.hpp>

#include <payload.hpp> // Studious Pancake
#include <util.hpp> // Studious Pancake
#include <tesla.hpp>


/**
 * @brief Ultrahand-Overlay Configuration Paths
 *
 * This block of code defines string variables for various configuration and directory paths
 * used in the Ultrahand-Overlay project. These paths include:
 *
 * - `packageFileName`: The name of the package file ("package.ini").
 * - `configFileName`: The name of the configuration file ("config.ini").
 * - `settingsPath`: The base path for Ultrahand settings ("sdmc:/config/ultrahand/").
 * - `settingsConfigIniPath`: The full path to the Ultrahand settings configuration file.
 * - `packageDirectory`: The base directory for packages ("sdmc:/switch/.packages/").
 * - `overlayDirectory`: The base directory for overlays ("sdmc:/switch/.overlays/").
 * - `teslaSettingsConfigIniPath`: The full path to the Tesla settings configuration file.
 *
 * These paths are used within the Ultrahand-Overlay project to manage configuration files
 * and directories.
 */
static const std::string bootPackageFileName = "boot_package.ini";
static const std::string packageFileName = "package.ini";
static const std::string configFileName = "config.ini";
static const std::string themeFileName = "theme.ini";
static const std::string settingsPath = "sdmc:/config/ultrahand/";
static const std::string settingsConfigIniPath = settingsPath + configFileName;
static const std::string langPath = settingsPath+"lang/";
static const std::string themeConfigIniPath = settingsPath + themeFileName;
static const std::string themesPath = settingsPath+"themes/";
static const std::string downloadsPath = settingsPath+"downloads/";
static const std::string packageDirectory = "sdmc:/switch/.packages/";
static const std::string overlayDirectory = "sdmc:/switch/.overlays/";
static const std::string teslaSettingsConfigIniPath = "sdmc:/config/tesla/"+configFileName;
static const std::string overlaysIniFilePath = settingsPath + "overlays.ini";
static const std::string packagesIniFilePath = settingsPath + "packages.ini";
static const std::string ultrahandRepo = "https://github.com/ppkantorski/Ultrahand-Overlay/";

static bool isDownloadCommand = false;
static bool commandSuccess = false;
static bool refreshGui = false;
static bool usingErista = util::IsErista();
static bool usingMariko = util::IsMariko();


void initializeTheme(std::string themeIniPath = themeConfigIniPath) {
    tsl::hlp::ini::IniData themesData;
    bool initialize = false;
    
    // write default theme
    if (isFileOrDirectory(themeIniPath)) {
        themesData = getParsedDataFromIniFile(themeIniPath);
        if (themesData.count("theme") > 0) {
            auto& themedSection = themesData["theme"];
            
            if (themedSection.count("clock_color") == 0)
                setIniFileValue(themeIniPath, "theme", "clock_color", "#FFFFFF");
            
            if (themedSection.count("bg_alpha") == 0)
                setIniFileValue(themeIniPath, "theme", "bg_alpha", "13");
            
            if (themedSection.count("bg_color") == 0)
                setIniFileValue(themeIniPath, "theme", "bg_color", "#000000");
            
            if (themedSection.count("seperator_alpha") == 0)
                setIniFileValue(themeIniPath, "theme", "seperator_alpha", "7");
            
            if (themedSection.count("seperator_color") == 0)
                setIniFileValue(themeIniPath, "theme", "seperator_color", "#777777");
            
            if (themedSection.count("battery_color") == 0)
                setIniFileValue(themeIniPath, "theme", "battery_color", "#FFFFFF");
            
            if (themedSection.count("text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "text_color", "#FFFFFF");
            
            if (themedSection.count("info_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "info_text_color", "#FFFFFF");
            
            if (themedSection.count("version_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "version_text_color", "#AAAAAA");
            
            if (themedSection.count("on_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "on_text_color", "#00FFDD");
            
            if (themedSection.count("off_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "off_text_color", "#AAAAAA");
            
            if (themedSection.count("invalid_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "invalid_text_color", "#FF0000");
            
            if (themedSection.count("selection_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "selection_text_color", "#FFFFFF");
            
            if (themedSection.count("selection_bg_color") == 0)
                setIniFileValue(themeIniPath, "theme", "selection_bg_color", "#000000");
            
            if (themedSection.count("trackbar_color") == 0)
                setIniFileValue(themeIniPath, "theme", "trackbar_color", "#555555");
            
            if (themedSection.count("highlight_color_1") == 0)
                setIniFileValue(themeIniPath, "theme", "highlight_color_1", "#2288CC");
            
            if (themedSection.count("highlight_color_2") == 0)
                setIniFileValue(themeIniPath, "theme", "highlight_color_2", "#88FFFF");
            
            if (themedSection.count("click_color") == 0)
                setIniFileValue(themeIniPath, "theme", "click_color", "#F7253E");
            
            if (themedSection.count("invert_bg_click_color") == 0)
                setIniFileValue(themeIniPath, "theme", "invert_bg_click_color", "false");
            
            if (themedSection.count("disable_selection_bg") == 0)
                setIniFileValue(themeIniPath, "theme", "disable_selection_bg", "true");
            
            // For disabling colorful logo
            if (themedSection.count("disable_colorful_logo") == 0)
                setIniFileValue(themeIniPath, "theme", "disable_colorful_logo", "false");
            
            if (themedSection.count("logo_color_1") == 0)
                setIniFileValue(themeIniPath, "theme", "logo_color_1", "#FFFFFF");
            
            if (themedSection.count("logo_color_2") == 0)
                setIniFileValue(themeIniPath, "theme", "logo_color_2", "#FF0000");
            
            if (themedSection.count("dynamic_logo_color_1") == 0)
                setIniFileValue(themeIniPath, "theme", "dynamic_logo_color_1", "#00E669");
            
            if (themedSection.count("dynamic_logo_color_2") == 0)
                setIniFileValue(themeIniPath, "theme", "dynamic_logo_color_2", "#8080EA");
            
        } else
            initialize = true;
    } else
        initialize = true;
    
    if (initialize) {
        setIniFileValue(themeIniPath, "theme", "clock_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "battery_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "bg_alpha", "13");
        setIniFileValue(themeIniPath, "theme", "bg_color", "#000000");
        setIniFileValue(themeIniPath, "theme", "seperator_alpha", "7");
        setIniFileValue(themeIniPath, "theme", "seperator_color", "#777777");
        setIniFileValue(themeIniPath, "theme", "text_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "info_text_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "version_text_color", "#AAAAAA");
        setIniFileValue(themeIniPath, "theme", "on_text_color", "#00FFDD");
        setIniFileValue(themeIniPath, "theme", "off_text_color", "#AAAAAA");
        setIniFileValue(themeIniPath, "theme", "invalid_text_color", "#FF0000");
        setIniFileValue(themeIniPath, "theme", "selection_text_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "selection_bg_color", "#000000");
        setIniFileValue(themeIniPath, "theme", "trackbar_color", "#555555");
        setIniFileValue(themeIniPath, "theme", "highlight_color_1", "#2288CC");
        setIniFileValue(themeIniPath, "theme", "highlight_color_2", "#88FFFF");
        setIniFileValue(themeIniPath, "theme", "click_color", "#F7253E");
        setIniFileValue(themeIniPath, "theme", "invert_bg_click_color", "false");
        setIniFileValue(themeIniPath, "theme", "disable_selection_bg", "true");
        setIniFileValue(themeIniPath, "theme", "disable_colorful_logo", "false");
        setIniFileValue(themeIniPath, "theme", "logo_color_1", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "logo_color_2", "#F7253E");
        setIniFileValue(themeIniPath, "theme", "dynamic_logo_color_1", "#00E669");
        setIniFileValue(themeIniPath, "theme", "dynamic_logo_color_2", "#8080EA");
    }
}



/**
 * @brief Copy Tesla key combo to Ultrahand settings.
 *
 * This function retrieves the key combo from Tesla settings and copies it to Ultrahand settings.
 */
void copyTeslaKeyComboToUltrahand() {
    std::string keyCombo = "ZL+ZR+DDOWN";
    std::map<std::string, std::map<std::string, std::string>> parsedData;
    std::string legacyComment = "; Auto-generated by Ultrahand for legacy compatibility.\n; See '/config/ultrahand/config.ini' for Ultrahand settings.\n\n";
    
    bool initializeTesla = false;
    if (isFileOrDirectory(teslaSettingsConfigIniPath)) {
        parsedData = getParsedDataFromIniFile(teslaSettingsConfigIniPath);
        if (parsedData.count("tesla") > 0) {
            auto& teslaSection = parsedData["tesla"];
            if (teslaSection.count("key_combo") > 0) {
                keyCombo = teslaSection["key_combo"];
            } else
                initializeTesla = true;
        } else
            initializeTesla = true;
    } else
        initializeTesla = true;
    
    if (initializeTesla)
        setIniFileValue(teslaSettingsConfigIniPath, "tesla", "key_combo", keyCombo, legacyComment);
    
    
    if (isFileOrDirectory(settingsConfigIniPath)) {
        parsedData = getParsedDataFromIniFile(settingsConfigIniPath);
        if (parsedData.count("ultrahand") > 0) {
            auto& ultrahandSection = parsedData["ultrahand"];
            if (ultrahandSection.count("key_combo") == 0) { // no entry present
                // Write the key combo to the destination file
                setIniFileValue(settingsConfigIniPath, "ultrahand", "key_combo", keyCombo);
                setIniFileValue(teslaSettingsConfigIniPath, "tesla", "key_combo", keyCombo, legacyComment);
            }
        }
    } else {
        // Write the key combo to the destination file
        setIniFileValue(settingsConfigIniPath, "ultrahand", "key_combo", keyCombo);
        setIniFileValue(teslaSettingsConfigIniPath, "tesla", "key_combo", keyCombo, legacyComment);
    }
    tsl::impl::parseOverlaySettings();
}



void addHelpInfo(auto& list) {
    tsl::Color infoTextColor = tsl::RGB888(parseValueFromIniSection(themeConfigIniPath, "theme", "info_text_color"), "#FFFFFF");
    tsl::Color onTextColor = tsl::RGB888(parseValueFromIniSection(themeConfigIniPath, "theme", "on_text_color"), "#00FFDD");
    
    // Add a section break with small text to indicate the "Commands" section
    list->addItem(new tsl::elm::CategoryHeader(USER_GUIDE));
    
    //constexpr int maxLineLength = 28;  // Adjust the maximum line length as needed
    constexpr int lineHeight = 20;  // Adjust the line height as needed
    int xOffset = std::stoi(USERGUIDE_OFFSET);    // Adjust the horizontal offset as needed
    constexpr int fontSize = 16;    // Adjust the font size as needed
    int numEntries = 4;   // Adjust the number of entries as needed
    
    //std::string::size_type startPos;
    //std::string::size_type spacePos;
    
    std::string sectionString = "";
    std::string infoString = "";
    
    sectionString += SETTINGS_MENU+"\n";
    infoString += "\uE0B5 ("+ON_MAIN_MENU+")\n";
    
    sectionString += SCRIPT_OVERLAY+"\n";
    infoString += "\uE0B6 ("+ON_A_COMMAND+")\n";
    
    sectionString += STAR_FAVORITE+"\n";
    infoString += "\uE0E2 ("+ON_OVERLAY_PACKAGE+")\n";
    
    sectionString += APP_SETTINGS+"\n";
    infoString += "\uE0E3 ("+ON_OVERLAY_PACKAGE+")\n";
    
    // Remove trailing newline character
    if ((sectionString != "") && (sectionString.back() == '\n'))
        sectionString = sectionString.substr(0, sectionString.size() - 1);
    if ((infoString != "") && (infoString.back() == '\n'))
        infoString = infoString.substr(0, infoString.size() - 1);
    
    
    if ((sectionString != "") && (infoString != "")) {
        list->addItem(new tsl::elm::CustomDrawer([lineHeight, xOffset, fontSize, sectionString, infoString, infoTextColor, onTextColor](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString(sectionString.c_str(), false, x + 12, y + lineHeight, fontSize, infoTextColor);
            renderer->drawString(infoString.c_str(), false, x + xOffset+ 12, y + lineHeight, fontSize, onTextColor);
        }), fontSize * numEntries + lineHeight);
    }
}


void addAppInfo(auto& list, auto& packageHeader, std::string type = "package") {
    // Add a section break with small text to indicate the "Commands" section
    if (type == "package")
        list->addItem(new tsl::elm::CategoryHeader(PACKAGE_INFO));
    else
        list->addItem(new tsl::elm::CategoryHeader(OVERLAY_INFO));
    
    tsl::Color infoTextColor = tsl::RGB888(parseValueFromIniSection(themeConfigIniPath, "theme", "info_text_color"), "#FFFFFF");
    
    constexpr int maxLineLength = 28;  // Adjust the maximum line length as needed
    constexpr int lineHeight = 20;  // Adjust the line height as needed
    constexpr int xOffset = 120;    // Adjust the horizontal offset as needed
    constexpr int fontSize = 16;    // Adjust the font size as needed
    int numEntries = 0;   // Adjust the number of entries as needed
    
    size_t startPos, endPos, spacePos;
    std::string line;
    
    std::string packageSectionString = "";
    std::string packageInfoString = "";
    if (packageHeader.title != "") {
        packageSectionString += TITLE+"\n";
        packageInfoString += (packageHeader.title+"\n").c_str();
        numEntries++;
    }
    if (packageHeader.version != "") {
        packageSectionString += VERSION+"\n";
        packageInfoString += (packageHeader.version+"\n").c_str();
        numEntries++;
    }
    if (packageHeader.creator != "") {
        packageSectionString += CREATOR+"\n";
        packageInfoString += (packageHeader.creator+"\n").c_str();
        numEntries++;
    }
    if (packageHeader.about != "") {
        std::string aboutHeaderText = ABOUT+"\n";
        std::string::size_type aboutHeaderLength = aboutHeaderText.length();
        std::string aboutText = packageHeader.about;
        
        packageSectionString += aboutHeaderText;
        
        // Split the about text into multiple lines with proper word wrapping
        startPos = 0;
        spacePos = 0;
        
        
        while (startPos < aboutText.length()) {
            endPos = std::min(startPos + maxLineLength, aboutText.length());
            line = aboutText.substr(startPos, endPos - startPos);
            
            // Check if the current line ends with a space; if not, find the last space in the line
            if (endPos < aboutText.length() && aboutText[endPos] != ' ') {
                spacePos = line.find_last_of(' ');
                if (spacePos != std::string::npos) {
                    endPos = startPos + spacePos;
                    line = aboutText.substr(startPos, endPos - startPos);
                }
            }
            
            packageInfoString += line + '\n';
            startPos = endPos + 1;
            numEntries++;
            
            // Add corresponding newline to the packageSectionString
            if (startPos < aboutText.length())
                packageSectionString += std::string(aboutHeaderLength, ' ') + '\n';
        }
    }
    if (packageHeader.credits != "") {
        std::string creditsHeaderText = CREDITS+"\n";
        std::string::size_type creditsHeaderLength = creditsHeaderText.length();
        std::string creditsText = packageHeader.credits;
        
        packageSectionString += creditsHeaderText;
        
        // Split the credits text into multiple lines with proper word wrapping
        startPos = 0;
        spacePos = 0;
        
        while (startPos < creditsText.length()) {
            endPos = std::min(startPos + maxLineLength, creditsText.length());
            line = creditsText.substr(startPos, endPos - startPos);
            
            // Check if the current line ends with a space; if not, find the last space in the line
            if (endPos < creditsText.length() && creditsText[endPos] != ' ') {
                spacePos = line.find_last_of(' ');
                if (spacePos != std::string::npos) {
                    endPos = startPos + spacePos;
                    line = creditsText.substr(startPos, endPos - startPos);
                }
            }
            
            packageInfoString += line + '\n';
            startPos = endPos + 1;
            numEntries++;
            
            // Add corresponding newline to the packageSectionString
            if (startPos < creditsText.length())
                packageSectionString += std::string(creditsHeaderLength, ' ') + '\n';
        }
    }
    
    
    // Remove trailing newline character
    if ((packageSectionString != "") && (packageSectionString.back() == '\n'))
        packageSectionString = packageSectionString.substr(0, packageSectionString.size() - 1);
    if ((packageInfoString != "") && (packageInfoString.back() == '\n'))
        packageInfoString = packageInfoString.substr(0, packageInfoString.size() - 1);
    
    
    if ((packageSectionString != "") && (packageInfoString != "")) {
        list->addItem(new tsl::elm::CustomDrawer([lineHeight, xOffset, fontSize, packageSectionString, packageInfoString, infoTextColor](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
            renderer->drawString(packageSectionString.c_str(), false, x + 12, y + lineHeight, fontSize, infoTextColor);
            renderer->drawString(packageInfoString.c_str(), false, x + xOffset, y + lineHeight, fontSize, infoTextColor);
        }), fontSize * numEntries + lineHeight);
    }
}



/**
 * @brief Ultrahand-Overlay Protected Folders
 *
 * This block of code defines two vectors containing paths to protected folders used in the
 * Ultrahand-Overlay project. These folders are designated as protected to prevent certain
 * operations that may pose security risks.
 *
 * The two vectors include:
 *
 * - `protectedFolders`: Paths to standard protected folders.
 * - `ultraProtectedFolders`: Paths to ultra protected folders with stricter security.
 *
 * These protected folder paths are used within the Ultrahand-Overlay project to enforce
 * safety conditions and ensure that certain operations are not performed on sensitive
 * directories.
 */


/**
 * @brief Check if a path contains dangerous combinations.
 *
 * This function checks if a given path contains patterns that may pose security risks.
 *
 * @param patternPath The path to check.
 * @return True if the path contains dangerous combinations, otherwise false.
 */
bool isDangerousCombination(const std::string& patternPath) {
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
        if (patternPath.find(ultraProtectedFolder) == 0)
            return true; // Pattern path is an ultra protected folder
    }
    
    // Check if the patternPath is a protected folder
    std::string relativePath, pathSegment;
    std::vector<std::string> pathSegments;
    
    for (const std::string& protectedFolder : protectedFolders) {
        if (patternPath == protectedFolder)
            return true; // Pattern path is a protected folder
        
        // Check if the patternPath starts with a protected folder and includes a dangerous pattern
        if (patternPath.find(protectedFolder) == 0) {
            relativePath = patternPath.substr(protectedFolder.size());
            
            // Split the relativePath by '/' to handle multiple levels of wildcards
            pathSegments.clear();
            pathSegment = "";
            
            for (char c : relativePath) {
                if (c == '/') {
                    if (!pathSegment.empty()) {
                        pathSegments.push_back(pathSegment);
                        pathSegment.clear();
                    }
                } else
                    pathSegment += c;
            }
            
            if (!pathSegment.empty())
                pathSegments.push_back(pathSegment);
            
            for (const std::string& pathSegment : pathSegments) {
                // Check if the pathSegment includes a dangerous pattern
                for (const std::string& dangerousPattern : dangerousPatterns) {
                    if (pathSegment.find(dangerousPattern) != std::string::npos)
                        return true; // Pattern path includes a dangerous pattern
                }
            }
            pathSegments.clear();
        }
        
        // Check if the patternPath is a combination of a protected folder and a dangerous pattern
        for (const std::string& dangerousPattern : dangerousCombinationPatterns) {
            if (patternPath == protectedFolder + dangerousPattern)
                return true; // Pattern path is a protected folder combined with a dangerous pattern
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
            } else
                pathSegment += c;
        }
        
        if (!pathSegment.empty())
            pathSegments.push_back(pathSegment);
        
        for (const std::string& pathSegment : pathSegments) {
            // Check if the pathSegment includes a dangerous pattern
            for (const std::string& dangerousPattern : dangerousPatterns) {
                if (pathSegment == dangerousPattern)
                    return true; // Pattern path is a dangerous pattern
            }
        }
        pathSegments.clear();
    }
    
    // Check if the patternPath includes a wildcard at the root level
    if (patternPath.find(":/") != std::string::npos) {
        std::string rootPath = patternPath.substr(0, patternPath.find(":/") + 2);
        if (rootPath.find('*') != std::string::npos)
            return true; // Pattern path includes a wildcard at the root level
    }
    
    // Check if the provided path matches any dangerous patterns
    for (const std::string& pattern : dangerousPatterns) {
        if (patternPath.find(pattern) != std::string::npos)
            return true; // Path contains a dangerous pattern
    }
    
    return false; // Pattern path is not a protected folder, a dangerous pattern, or includes a wildcard at the root level
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
            commands = "["+REBOOT+"]\n"
                       "reboot\n"
                       "["+SHUTDOWN+"]\n"
                       "shutdown\n";
        } else
            commands = "";
        fprintf(configFileOut, "%s", commands.c_str());
        
        
        fclose(configFileOut);
        configFile = fopen(configIniPath.c_str(), "r");
    }
    
    //constexpr size_t BufferSize = 131072; // Choose a larger buffer size for reading lines
    char line[BufferSize];
    std::string currentOption;
    std::vector<std::vector<std::string>> commands;
    
    bool isFirstEntry = true;
    std::string trimmedLine;
    std::string part, arg;
    bool inQuotes;
    
    std::vector<std::string> commandParts;
    std::istringstream iss, argIss; // Move this line outside the loop
    
    while (fgets(line, sizeof(line), configFile)) {
        trimmedLine = line;
        trimmedLine.erase(trimmedLine.find_last_not_of("\r\n") + 1);  // Remove trailing newline character
        
        if (trimmedLine.empty() || trimmedLine[0] == '#')
            continue;// Skip empty lines and comment lines
        else if (trimmedLine[0] == '[' && trimmedLine.back() == ']') {
            if (isFirstEntry) { // for preventing header comments from being loaded within the first command section
                commands.clear();
                isFirstEntry = false;
            }
            
            // New option section
            if (!currentOption.empty()) {
                // Store previous option and its commands
                options.emplace_back(std::move(currentOption), std::move(commands));
                commands.clear();
            }
            currentOption = trimmedLine.substr(1, trimmedLine.size() - 2);  // Extract option name
        } else {
            // Command line
            //std::istringstream iss(trimmedLine);
            iss.clear(); // Reset stream state
            iss.str(trimmedLine); // Set new content
            
            commandParts.clear();
            
            part = "";
            inQuotes = false;
            while (std::getline(iss, part, '\'')) {
                if (!part.empty()) {
                    if (!inQuotes) {
                        // Outside quotes, split on spaces
                        argIss.clear();
                        argIss.str(part);
                        //std::istringstream argIss(part);
                        arg = "";
                        while (argIss >> arg)
                            commandParts.push_back(arg);
                    } else
                        commandParts.push_back(part); // Inside quotes, treat as a whole argument
                }
                inQuotes = !inQuotes;
            }
            commands.push_back(std::move(commandParts));
        }
    }
    
    // Store the last option and its commands
    if (!currentOption.empty())
        options.emplace_back(std::move(currentOption), std::move(commands));
    
    fclose(configFile);
    return options;
}




// Function to populate selectedItemsListOff from a JSON array based on a key
void populateSelectedItemsList(const std::string& sourceType, const std::string& jsonStringOrPath, const std::string& jsonKey, std::vector<std::string>& selectedItemsList) {
    json_t* jsonData = nullptr;
    
    if (sourceType == "json")
        jsonData = stringToJson(jsonStringOrPath);
    else if (sourceType == "json_file")
        jsonData = readJsonFromFile(jsonStringOrPath);
    
    if (jsonData && json_is_array(jsonData)) {
        
        size_t arraySize = json_array_size(jsonData);
        selectedItemsList.reserve(arraySize); // Preallocate memory for efficiency
        
        for (size_t i = 0; i < arraySize; ++i) {
            json_t* item = json_array_get(jsonData, i);
            if (item && json_is_object(item)) {
                json_t* keyValue = json_object_get(item, jsonKey.c_str());
                if (keyValue && json_is_string(keyValue)) {
                    const char* value = json_string_value(keyValue);
                    if (value) {
                        selectedItemsList.emplace_back(value);
                    }
                }
            }
        }
    }
    
    // Free jsonDataOn
    if (jsonData != nullptr) {
        json_decref(jsonData);
        jsonData = nullptr;
    }
}



/**
 * @brief Replaces a placeholder with a replacement string in the input.
 *
 * This function replaces all occurrences of a specified placeholder with the
 * provided replacement string in the input string.
 *
 * @param input The input string.
 * @param placeholder The placeholder to replace.
 * @param replacement The string to replace the placeholder with.
 * @return The input string with placeholders replaced by the replacement string.
 */
std::string replacePlaceholder(const std::string& input, const std::string& placeholder, const std::string& replacement) {
    std::string result = input;
    size_t pos = result.find(placeholder);
    if (pos != std::string::npos)
        result.replace(pos, placeholder.length(), replacement);
    return result;
}



// `{hex_file(customAsciiPattern, offsetStr, length)}`
std::string replaceIniPlaceholder(const std::string& arg, const std::string& iniPath) {
    std::string replacement = arg;
    std::string searchString = "{ini_file(";
    
    size_t startPos = replacement.find(searchString);
    size_t endPos = replacement.find(")}");
    
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        std::string placeholderContent = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
        
        // Split the placeholder content into its components (customAsciiPattern, offsetStr, length)
        std::vector<std::string> components;
        std::istringstream componentStream(placeholderContent);
        std::string component;
        
        while (std::getline(componentStream, component, ','))
            components.push_back(trim(component));
        
        if (components.size() == 2) {
            // Extract individual components
            std::string iniSection = removeQuotes(components[0]);
            std::string iniKey = removeQuotes(components[1]);
            components.clear();
            
            // Call the parsing function and replace the placeholder
            std::string parsedResult = parseValueFromIniSection(iniPath, iniSection, iniKey);
            
            // Replace the entire placeholder with the parsed result
            replacement.replace(startPos, endPos - startPos + searchString.length() + 2, parsedResult);
        }
    }
    
    return replacement;
}


// this will modify `commands`
std::vector<std::vector<std::string>> getSourceReplacement(const std::vector<std::vector<std::string>>& commands, const std::string& entry, size_t entryIndex) {
    
    bool inEristaSection = false;
    bool inMarikoSection = false;
    
    std::vector<std::vector<std::string>> modifiedCommands;
    //std::vector<std::string> listData;
    std::string listString;
    std::string jsonPath, jsonString;
    size_t startPos, endPos;
    
    std::vector<std::string> modifiedCmd;
    std::string modifiedArg, lastArg, replacement, commandName;
    
    for (const auto& cmd : commands) {
        if (cmd.empty())
            continue;
        
        modifiedCmd.clear();
        
        //modifiedCmd.reserve(cmd.size()); // Reserve memory for efficiency
        commandName = cmd[0];

        if (commandName == "download")
            isDownloadCommand = true;

        if (commandName == "erista:" || commandName == "Erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            continue;
        } else if (commandName == "mariko:" || commandName == "Mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            continue;
        }
        
        if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
            
            if (cmd.size() > 1) {
                if ((commandName == "list_source") && listString.empty())
                    listString = removeQuotes(cmd[1]);
                else if ((commandName == "json_file_source") && jsonPath.empty())
                    jsonPath = preprocessPath(cmd[1]);
                else if ((commandName == "json_source") && jsonString.empty())
                    jsonString = cmd[1];
            }
            
            
            for (const auto& arg : cmd) {
                modifiedArg = arg; // Working with a copy for modifications
                lastArg = ""; // Initialize lastArg for each argument
                
                while (modifiedArg.find("{file_source}") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "{file_source}", entry);
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{file_name}") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "{file_name}", getNameFromPath(entry));
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{folder_name}") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "{folder_name}", getParentDirNameFromPath(entry));
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{list_source(") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{list_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = stringToList(listString)[entryIndex];
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{json_source(") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{json_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = replaceJsonPlaceholder(modifiedArg.substr(startPos, endPos - startPos + 2), "json_source", jsonString);
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{json_file_source(") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{json_file_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = replaceJsonPlaceholder(modifiedArg.substr(startPos, endPos - startPos + 2), "json_file_source", jsonPath);
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                
                modifiedCmd.push_back(std::move(modifiedArg)); // Move modified arg to the modified command vector
            }
            
            modifiedCommands.emplace_back(std::move(modifiedCmd)); // Move modified command to the result vector
        }
    }
    return modifiedCommands;
}



/**
 * @brief Interpret and execute a list of commands.
 *
 * This function interprets and executes a list of commands based on their names and arguments.
 *
 * @param commands A list of commands, where each command is represented as a vector of strings.
 */
void interpretAndExecuteCommand(const std::vector<std::vector<std::string>>& commands, const std::string& packagePath="", const std::string& selectedCommand="") {
    
    bool logging = false;
    
    bool inEristaSection = false;
    bool inMarikoSection = false;
    
    bool downloadSuccess;
    
    std::string bootCommandName, sourcePath, destinationPath, \
        desiredSection, desiredNewSection, desiredKey, desiredNewKey, desiredValue, \
        offset, customPattern, hexDataToReplace, hexDataReplacement, fileUrl, clearOption;
    
    size_t cmdSize;
    size_t occurrence;
    size_t tryCounter = 0;
    size_t startPos, endPos;
    size_t listIndex;
    
    // Overwrite globals
    commandSuccess = true;
    refreshGui = false;
    
    std::string listString, jsonString, jsonPath, hexPath, iniPath, lastArg;
    
    //std::vector<std::string> command;
    std::string replacement;
    std::vector<std::string> modifiedCmd;
    
    std::string message;
    
    for (const auto& cmd : commands) {
        
        // Check the command and perform the appropriate action
        if (cmd.empty())
            continue; // Empty command, do nothing
        
        // Get the command name (first part of the command)
        const std::string& commandName = cmd[0];
        
        // Try implementation
        if (commandName == "try:") {
            tryCounter++;
            if (commandSuccess && tryCounter > 1)
                break;
            commandSuccess = true;
            if (logging)
                logMessage("Try #"+std::to_string(tryCounter));
            continue;
        } else if (commandName == "erista:" || commandName == "Erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            continue;
        } else if (commandName == "mariko:" || commandName == "Mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            continue;
        }
        
        if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
            if (tryCounter == 0 || (commandSuccess && tryCounter != 0)) {
                // Create a modified command vector to store changes
                //std::vector<std::string> modifiedCmd = cmd;
                modifiedCmd = std::move(cmd);
                
                for (auto& arg : modifiedCmd) {
                    lastArg = "";
                    while ((!hexPath.empty() && (arg.find("{hex_file(") != std::string::npos))) {
                        startPos = arg.find("{hex_file(");
                        endPos = arg.find(")}");
                        if (endPos != std::string::npos && endPos > startPos) {
                            replacement = replaceHexPlaceholder(arg.substr(startPos, endPos - startPos + 2), hexPath);
                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                arg.replace(startPos, endPos - startPos + 2, "null"); // fall back replacement value of null
                                commandSuccess = false;
                                break;
                            }
                        } else
                            break;
                        lastArg = arg;
                    }
                    while ((!iniPath.empty() && (arg.find("{ini_file(") != std::string::npos))) {
                        startPos = arg.find("{ini_file(");
                        endPos = arg.find(")}");
                        if (endPos != std::string::npos && endPos > startPos) {
                            replacement = replaceIniPlaceholder(arg.substr(startPos, endPos - startPos + 2), iniPath);
                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                arg.replace(startPos, endPos - startPos + 2, "null"); // fall back replacement value of null
                                commandSuccess = false;
                                break;
                            }
                        } else
                            break;
                        lastArg = arg;
                    }
                    while ((!listString.empty() && (arg.find("{list(") != std::string::npos))) {
                        startPos = arg.find("{list(");
                        endPos = arg.find(")}");
                        if (endPos != std::string::npos && endPos > startPos) {
                            listIndex = std::stoi(arg.substr(startPos, endPos - startPos + 2));
                            replacement = stringToList(listString)[listIndex];
                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                arg.replace(startPos, endPos - startPos + 2, "null"); // fall back replacement value of null
                                commandSuccess = false;
                                break;
                            }
                        } else
                            break;
                        lastArg = arg;
                    }
                    while ((!jsonString.empty() && (arg.find("{json(") != std::string::npos))) {
                        startPos = arg.find("{json(");
                        endPos = arg.find(")}");
                        if (endPos != std::string::npos && endPos > startPos) {
                            replacement = replaceJsonPlaceholder(arg.substr(startPos, endPos - startPos + 2), "json", jsonString);
                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                arg.replace(startPos, endPos - startPos + 2, UNAVAILABLE_SELECTION); // fall back replacement value of `UNAVAILABLE_SELECTION`
                                commandSuccess = false;
                                break;
                            }
                        } else
                            break;
                        lastArg = arg;
                    }
                    while ((!jsonPath.empty() && (arg.find("{json_file(") != std::string::npos))) {
                        startPos = arg.find("{json_file(");
                        endPos = arg.find(")}");
                        if (endPos != std::string::npos && endPos > startPos) {
                            replacement = replaceJsonPlaceholder(arg.substr(startPos, endPos - startPos + 2), "json_file", jsonPath);
                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                arg.replace(startPos, endPos - startPos + 2, UNAVAILABLE_SELECTION); // fall back replacement value of `UNAVAILABLE_SELECTION`
                                commandSuccess = false;
                                break;
                            }
                        } else
                            break;
                        lastArg = arg;
                    }
                }
                //command = std::move(modifiedCmd); // update command
                
                cmdSize = modifiedCmd.size();
                
                // Variable replacement definitions
                if (commandName == "list") {
                    if (cmdSize >= 2) {
                        listString = removeQuotes(modifiedCmd[1]);
                    }
                } else if (commandName == "json") {
                    if (cmdSize >= 2) {
                        jsonString = modifiedCmd[1];
                    }
                } else if (commandName == "json_file") {
                    if (cmdSize >= 2) {
                        jsonPath = preprocessPath(modifiedCmd[1]);
                    }
                } else if (commandName == "ini_file") {
                    if (cmdSize >= 2) {
                        iniPath = preprocessPath(modifiedCmd[1]);
                    }
                } else if (commandName == "hex_file") {
                    if (cmdSize >= 2) {
                        hexPath = preprocessPath(modifiedCmd[1]);
                    }
                } else if (commandName == "make" || commandName == "mkdir") { // Make command
                    if (cmdSize >= 2) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        createDirectory(sourcePath);
                    }
                } else if (commandName == "copy" || commandName == "cp") { // Copy command
                    if (cmdSize >= 3) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        destinationPath = preprocessPath(modifiedCmd[2]);
                        
                        if (sourcePath.find('*') != std::string::npos)
                            copyFileOrDirectoryByPattern(sourcePath, destinationPath); // Delete files or directories by pattern
                        else
                            copyFileOrDirectory(sourcePath, destinationPath);
                    }
                } else if (commandName == "delete" || commandName == "del") { // Delete command
                    if (cmdSize >= 2) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        if (!isDangerousCombination(sourcePath)) {
                            if (sourcePath.find('*') != std::string::npos)
                                deleteFileOrDirectoryByPattern(sourcePath); // Delete files or directories by pattern
                            else
                                deleteFileOrDirectory(sourcePath);
                        }
                    }
                } else if (commandName.compare(0, 7, "mirror_") == 0) {
                    
                    if (cmdSize >= 2) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        
                        if (cmdSize >= 3) {
                            destinationPath = preprocessPath(modifiedCmd[2]);
                        } else {
                            destinationPath = "sdmc:/";
                        }
                        
                        //std::string action = (commandName == "mirror_copy" || commandName == "mirror_cp") ? "copy" : "delete";
                        mirrorFiles(sourcePath, destinationPath, (commandName == "mirror_copy" || commandName == "mirror_cp") ? "copy" : "delete");
                    }
                } else if (commandName == "rename" || commandName == "move" || commandName == "mv") { // Rename command
                    if (cmdSize >= 3) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        destinationPath = preprocessPath(modifiedCmd[2]);
                        if (!isDangerousCombination(sourcePath)) {
                            if (sourcePath.find('*') != std::string::npos)
                                moveFilesOrDirectoriesByPattern(sourcePath, destinationPath); // Move files by pattern
                            else
                                moveFileOrDirectory(sourcePath, destinationPath); // Move single file or directory
                        }
                    }
                } else if (commandName == "add-ini-section") {
                    if (cmdSize >= 2) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        desiredSection = removeQuotes(modifiedCmd[2]);
                        addIniSection(sourcePath.c_str(), desiredSection.c_str());
                    }
                } else if (commandName == "rename-ini-section") {
                    if (cmdSize >= 3) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        desiredSection = removeQuotes(modifiedCmd[2]);
                        desiredNewSection = removeQuotes(modifiedCmd[3]);
                        renameIniSection(sourcePath.c_str(), desiredSection.c_str(), desiredNewSection.c_str());
                    }
                } else if (commandName == "remove-ini-section") {
                    if (cmdSize >= 2) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        desiredSection = removeQuotes(modifiedCmd[2]);
                        removeIniSection(sourcePath.c_str(), desiredSection.c_str());
                    }
                } else if (commandName == "set-ini-val" || commandName == "set-ini-value") {
                    if (cmdSize >= 5) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        desiredSection = removeQuotes(modifiedCmd[2]);
                        desiredKey = removeQuotes(modifiedCmd[3]);
                        desiredValue = "";
                        for (size_t i = 4; i < cmdSize; ++i) {
                            desiredValue += modifiedCmd[i];
                            if (i < cmdSize - 1)
                                desiredValue += " ";
                        }
                        setIniFileValue(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
                    }
                } else if (commandName == "set-ini-key") {
                    if (cmdSize >= 5) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        desiredSection = removeQuotes(modifiedCmd[2]);
                        desiredKey = removeQuotes(modifiedCmd[3]);
                        for (size_t i = 4; i < cmdSize; ++i) {
                            desiredNewKey += modifiedCmd[i];
                            if (i < cmdSize - 1)
                                desiredNewKey += " ";
                        }
                        setIniFileKey(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredNewKey.c_str());
                    }
                } else if (commandName == "set-footer") {
                    if (cmdSize >= 2) {
                        desiredValue = removeQuotes(modifiedCmd[1]);
                        setIniFileValue((packagePath+configFileName).c_str(), selectedCommand.c_str(), "footer", desiredValue.c_str());
                    }
                } else if (commandName.compare(0, 7, "hex-by-") == 0) {
                    if (cmdSize >= 4) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        const std::string& secondArg = removeQuotes(modifiedCmd[2]);
                        const std::string& thirdArg = removeQuotes(modifiedCmd[3]);
                        
                        if (commandName == "hex-by-offset") {
                            hexEditByOffset(sourcePath.c_str(), secondArg.c_str(), thirdArg.c_str());
                        } else if (commandName == "hex-by-swap") {
                            if (cmdSize >= 5) {
                                occurrence = std::stoul(removeQuotes(modifiedCmd[4]));
                                hexEditFindReplace(sourcePath, secondArg, thirdArg, occurrence);
                            } else {
                                hexEditFindReplace(sourcePath, secondArg, thirdArg);
                            }
                        } else if (commandName == "hex-by-string") {
                            hexDataToReplace = asciiToHex(secondArg);
                            hexDataReplacement = asciiToHex(thirdArg);
                            
                            // Fix miss-matched string sizes
                            if (hexDataReplacement.length() < hexDataToReplace.length()) {
                                hexDataReplacement += std::string(hexDataToReplace.length() - hexDataReplacement.length(), '\0');
                            } else if (hexDataReplacement.length() > hexDataToReplace.length()) {
                                hexDataToReplace += std::string(hexDataReplacement.length() - hexDataToReplace.length(), '\0');
                            }
                            
                            if (cmdSize >= 5) {
                                occurrence = std::stoul(removeQuotes(modifiedCmd[4]));
                                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
                            } else {
                                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
                            }
                        } else if (commandName == "hex-by-decimal") {
                            hexDataToReplace = decimalToHex(secondArg);
                            hexDataReplacement = decimalToHex(thirdArg);
                            
                            if (cmdSize >= 5) {
                                occurrence = std::stoul(removeQuotes(modifiedCmd[4]));
                                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
                            } else {
                                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
                            }
                        } else if (commandName == "hex-by-rdecimal") {
                            hexDataToReplace = decimalToReversedHex(secondArg);
                            hexDataReplacement = decimalToReversedHex(thirdArg);
                            
                            if (cmdSize >= 5) {
                                occurrence = std::stoul(removeQuotes(modifiedCmd[4]));
                                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
                            } else {
                                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
                            }
                        } else if (commandName == "hex-by-custom-offset" ||
                                   commandName == "hex-by-custom-decimal-offset" ||
                                   commandName == "hex-by-custom-rdecimal-offset") {
                            if (cmdSize >= 5) {
                                customPattern = removeQuotes(modifiedCmd[2]);
                                offset = removeQuotes(modifiedCmd[3]);
                                hexDataReplacement = removeQuotes(modifiedCmd[4]);
                                
                                if (commandName == "hex-by-custom-decimal-offset") {
                                    hexDataReplacement = decimalToHex(hexDataReplacement);
                                } else if (commandName == "hex-by-custom-rdecimal-offset") {
                                    hexDataReplacement = decimalToReversedHex(hexDataReplacement);
                                }
                                
                                hexEditByCustomOffset(sourcePath.c_str(), customPattern.c_str(), offset.c_str(), hexDataReplacement.c_str());
                            }
                        }
                    }
                } else if (commandName == "download") {
                    if (cmdSize >= 3) {
                        fileUrl = preprocessUrl(modifiedCmd[1]);
                        destinationPath = preprocessPath(modifiedCmd[2]);
                        downloadSuccess = false;
                        
                        //setIniFileValue((packagePath+configFileName).c_str(), selectedCommand.c_str(), "footer", "downloading");
                        for (size_t i = 0; i < 3; ++i) { // Try 3 times.
                            downloadSuccess = downloadFile(fileUrl, destinationPath);
                            if (downloadSuccess)
                                break;
                        }
                        commandSuccess = (downloadSuccess && commandSuccess);
                    }
                } else if (commandName == "unzip") {
                    if (cmdSize >= 3) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        destinationPath = preprocessPath(modifiedCmd[2]);
                        commandSuccess = unzipFile(sourcePath, destinationPath) && commandSuccess;
                    }
                } else if (commandName == "pchtxt2ips") {
                    if (cmdSize >= 3) {
                        sourcePath = preprocessPath(modifiedCmd[1]);
                        destinationPath = preprocessPath(modifiedCmd[2]);
                        commandSuccess = pchtxt2ips(sourcePath, destinationPath) && commandSuccess;
                    }
                } else if (commandName == "exec") {
                    if (cmdSize >= 2) {
                        bootCommandName = removeQuotes(modifiedCmd[1]);
                        if (isFileOrDirectory(packagePath+bootPackageFileName)) {
                            auto bootOptions = loadOptionsFromIni(packagePath+bootPackageFileName, true);
                            std::string bootOptionName;
                            
                            bool resetCommandSuccess;
                            for (const auto& bootOption:bootOptions) {
                                bootOptionName = bootOption.first;
                                auto& bootCommands = bootOption.second;
                                if (bootOptionName == bootCommandName) {
                                    resetCommandSuccess = false;
                                    if (!commandSuccess)
                                        resetCommandSuccess = true;
                                    interpretAndExecuteCommand(bootCommands, packagePath+bootPackageFileName, bootOptionName); // Execute modified 
                                    if (resetCommandSuccess) {
                                        commandSuccess = false;
                                        resetCommandSuccess = false;
                                    }
                                    //bootCommands.clear();
                                    break;
                                }
                                //bootCommands.clear();
                            }
                            //if (bootOptions.size() > 0)
                            //    auto bootOption = bootOptions[0];
                            bootOptions.clear();
                        }
                    }
                } else if (commandName == "reboot") { // credits to Studious Pancake for the Payload and utils methods
                    std::string rebootOption;
                    int rebootIndex = 0;
                    
                    if (util::IsErista() || util::SupportsMarikoRebootToConfig()) {
                        if (cmdSize >= 2) {
                            rebootOption = removeQuotes(modifiedCmd[1]);
                            
                            if (cmdSize >= 3) {
                                std::string option;
                                if (rebootOption == "boot") {
                                    option = removeQuotes(modifiedCmd[2]);
                                    Payload::HekateConfigList bootConfigList = Payload::LoadHekateConfigList();
                                    auto bootConfigIterator = bootConfigList.begin();  // Define the iterator here
                                    if (std::all_of(option.begin(), option.end(), ::isdigit)) {
                                        rebootIndex = std::stoi(option);
                                        
                                        std::advance(bootConfigIterator, rebootIndex);
                                        Payload::RebootToHekateConfig(*bootConfigIterator, false);
                                    
                                    } else { 
                                        std::string& entryName = option;
                                        rebootIndex = -1;  // Initialize rebootIndex to -1, indicating no match found
                                        
                                        for (auto it = bootConfigList.begin(); it != bootConfigList.end(); ++it) {
                                            if (it->name == entryName) {
                                                // Match found, store the index and break the loop
                                                rebootIndex = std::distance(bootConfigList.begin(), it);
                                                bootConfigIterator = it;  // Update the iterator to the matching element
                                                break;
                                            }
                                        }
                                        if (rebootIndex != -1)
                                            Payload::RebootToHekateConfig(*bootConfigIterator, false);
                                    }
                                } else if (rebootOption == "ini") {
                                    option = removeQuotes(modifiedCmd[2]);
                                    Payload::HekateConfigList iniConfigList = Payload::LoadIniConfigList();
                                    auto iniConfigIterator = iniConfigList.begin();
                                    if (std::all_of(option.begin(), option.end(), ::isdigit)) {
                                        rebootIndex = std::stoi(option);
                                        
                                        std::advance(iniConfigIterator, rebootIndex);
                                        Payload::RebootToHekateConfig(*iniConfigIterator, true);
                                    
                                    } else { 
                                        std::string& entryName = option;
                                        rebootIndex = -1;  // Initialize rebootIndex to -1, indicating no match found
                                        
                                        for (auto it = iniConfigList.begin(); it != iniConfigList.end(); ++it) {
                                            if (it->name == entryName) {
                                                // Match found, store the index and break the loop
                                                rebootIndex = std::distance(iniConfigList.begin(), it);
                                                iniConfigIterator = it;  // Update the iterator to the matching element
                                                break;
                                            }
                                        }
                                        if (rebootIndex != -1)
                                            Payload::RebootToHekateConfig(*iniConfigIterator, true);
                                    }
                                }
                            }
                            
                            if (rebootOption == "UMS")
                                Payload::RebootToHekateUMS(Payload::UmsTarget_Sd);
                            else if (rebootOption == "HEKATE" || rebootOption == "hekate")
                                Payload::RebootToHekateMenu();
                            else if (isFileOrDirectory(rebootOption)) {
                                std::string fileName = getNameFromPath(rebootOption);
                                if (util::IsErista()) {
                                    Payload::PayloadConfig reboot_payload = {fileName, rebootOption};
                                    Payload::RebootToPayload(reboot_payload);
                                } else {
                                    setIniFileValue("/bootloader/ini/" + fileName + ".ini", fileName, "payload", rebootOption); // generate entry
                                    Payload::HekateConfigList iniConfigList = Payload::LoadIniConfigList();
                                    
                                    rebootIndex = -1;  // Initialize rebootIndex to -1, indicating no match found
                                    auto iniConfigIterator = iniConfigList.begin();  // Define the iterator here
                                    
                                    for (auto it = iniConfigList.begin(); it != iniConfigList.end(); ++it) {
                                        if (it->name == fileName) {
                                            // Match found, store the index and break the loop
                                            rebootIndex = std::distance(iniConfigList.begin(), it);
                                            iniConfigIterator = it;  // Update the iterator to the matching element
                                            break;
                                        }
                                    }
                                    
                                    if (rebootIndex != -1)
                                        Payload::RebootToHekateConfig(*iniConfigIterator, true);
                                }
                            }
                        }
                        
                        if (rebootOption.empty())
                            Payload::RebootToHekate();
                    }
                    
                    // Fall back reboot command
                    i2cExit();
                    splExit();
                    fsdevUnmountAll();
                    spsmShutdown(SpsmShutdownMode_Reboot);
                    
                } else if (commandName == "shutdown") {
                    // Reboot command
                    splExit();
                    fsdevUnmountAll();
                    spsmShutdown(SpsmShutdownMode_Normal);
                } else if (commandName == "backlight") {
                    lblInitialize();
                    LblBacklightSwitchStatus lblstatus = LblBacklightSwitchStatus_Disabled;
                    lblGetBacklightSwitchStatus(&lblstatus);
                    lblstatus ? lblSwitchBacklightOff(0) : lblSwitchBacklightOn(0);
                    lblExit();
                } else if (commandName == "refresh") {
                    refreshGui = true;
                } else if (commandName == "logging") {
                    logging = !logging;
                } else if (commandName == "clear") {
                    if (cmdSize >= 2) {
                        clearOption = removeQuotes(modifiedCmd[1]);
                        if (clearOption == "log")
                            deleteFileOrDirectory(logFilePath);
                        else if (clearOption == "hex_sum_cache")
                            hexSumCache.clear();
                    }
                }
                
                // Log the command using logMessage
                if (logging) {
                    message = "Executing command: ";
                    for (const std::string& token : modifiedCmd)
                        message += token + " ";
                    logMessage(message);
                }
                modifiedCmd.clear();
            }
        }
    }
}
