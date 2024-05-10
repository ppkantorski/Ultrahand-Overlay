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
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/

#pragma once
#include <switch.h>
#include <fstream>
#include <fnmatch.h>

#include <payload.hpp> // Studious Pancake
#include <util.hpp> // Studious Pancake
#include <tesla.hpp>

#include <numeric>
#include <queue>
#include <mutex>
#include <condition_variable>


static std::atomic<bool> abortCommand(false);
static std::atomic<bool> triggerExit(false);


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
static bool interpreterLogging = false;

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
            if (themedSection.count("inprogress_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "inprogress_text_color", "#FFFF45");
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
            if (themedSection.count("highlight_color_3") == 0)
                setIniFileValue(themeIniPath, "theme", "highlight_color_3", "#FFFF45");
            if (themedSection.count("highlight_color_4") == 0)
                setIniFileValue(themeIniPath, "theme", "highlight_color_4", "#F7253E");
            if (themedSection.count("click_text_color") == 0)
                setIniFileValue(themeIniPath, "theme", "click_text_color", "#FFFFFF");
            if (themedSection.count("click_alpha") == 0)
                setIniFileValue(themeIniPath, "theme", "click_alpha", "7");
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
        setIniFileValue(themeIniPath, "theme", "inprogress_text_color", "#FFFF45");
        setIniFileValue(themeIniPath, "theme", "selection_text_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "selection_bg_color", "#000000");
        setIniFileValue(themeIniPath, "theme", "trackbar_color", "#555555");
        setIniFileValue(themeIniPath, "theme", "highlight_color_1", "#2288CC");
        setIniFileValue(themeIniPath, "theme", "highlight_color_2", "#88FFFF");
        setIniFileValue(themeIniPath, "theme", "highlight_color_3", "#FFFF45");
        setIniFileValue(themeIniPath, "theme", "highlight_color_4", "#F7253E");
        setIniFileValue(themeIniPath, "theme", "click_text_color", "#FFFFFF");
        setIniFileValue(themeIniPath, "theme", "click_alpha", "7");
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
        }), fontSize * numEntries +2);
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
        }), fontSize * numEntries +2);
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
    std::ifstream configFile(configIniPath);
    if (!configFile && makeConfig) {
        std::ofstream configFileOut(configIniPath);
        if (configFileOut) {
            configFileOut << "[REBOOT]\nreboot\n\n[SHUTDOWN]\nshutdown\n";
            configFileOut.close();
        }
        configFile.open(configIniPath);  // Reopen the newly created file
    }

    if (!configFile) {
        return {}; // If file still cannot be opened, return empty vector
    }

    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;
    std::string line, currentSection, part, arg;
    std::vector<std::vector<std::string>> sectionCommands;
    std::vector<std::string> commandParts;
    bool isFirstEntry = true, inQuotes = false;
    std::istringstream iss, argIss; // Declare outside the loop

    while (getline(configFile, line)) {
        // Properly remove carriage returns and newlines
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

        if (line.empty() || line[0] == '#') continue; // Skip empty or comment lines

        if (line.front() == '[' && line.back() == ']') { // Section headers
            if (!currentSection.empty()) {
                options.emplace_back(std::move(currentSection), std::move(sectionCommands));
                sectionCommands.clear();
            }
            currentSection = line.substr(1, line.size() - 2);
            isFirstEntry = false;
        } else if (!isFirstEntry) { // Command lines within sections
            commandParts.clear();
            iss.clear(); // Clear any error state
            iss.str(line); // Set the new line to parse
            inQuotes = false;
            while (std::getline(iss, part, '\'')) { // Split on single quotes
                if (inQuotes) {
                    commandParts.push_back(part); // Inside quotes, treat as a whole argument
                } else {
                    argIss.clear(); // Clear any error state
                    argIss.str(part); // Set part to parse
                    while (argIss >> arg) {
                        commandParts.push_back(arg); // Split part outside quotes by spaces
                    }
                }
                inQuotes = !inQuotes; // Toggle the inQuotes flag
            }
            sectionCommands.push_back(std::move(commandParts));
        }
    }

    if (!currentSection.empty()) {
        options.emplace_back(std::move(currentSection), std::move(sectionCommands));
    }

    return options;
}



// Function to populate selectedItemsListOff from a JSON array based on a key
void populateSelectedItemsList(const std::string& sourceType, const std::string& jsonStringOrPath, const std::string& jsonKey, std::vector<std::string>& selectedItemsList) {
    std::unique_ptr<json_t, void(*)(json_t*)> jsonData(nullptr, json_decref);  // Proper deleter for JSON objects

    if (sourceType == "json") {
        jsonData.reset(stringToJson(jsonStringOrPath));
    } else if (sourceType == "json_file") {
        jsonData.reset(readJsonFromFile(jsonStringOrPath));
    }

    if (!jsonData || !json_is_array(jsonData.get())) {
        return; // Early return if jsonData is null or not an array
    }

    json_t* jsonArray = jsonData.get();
    size_t arraySize = json_array_size(jsonArray);
    selectedItemsList.reserve(arraySize); // Preallocate memory for efficiency

    for (size_t i = 0; i < arraySize; ++i) {
        json_t* item = json_array_get(jsonArray, i);
        if (json_is_object(item)) {
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
    size_t pos = input.find(placeholder);
    if (pos == std::string::npos) {
        return input;  // Returns original string directly if no placeholder is found
    }
    std::string result = input;
    result.replace(pos, placeholder.length(), replacement);
    return result;
}




// `{hex_file(customAsciiPattern, offsetStr, length)}`
std::string replaceIniPlaceholder(const std::string& arg, const std::string& iniPath) {
    const std::string searchString = "{ini_file(";
    size_t startPos = arg.find(searchString);
    if (startPos == std::string::npos) {
        return arg;
    }

    size_t endPos = arg.find(")}", startPos);
    if (endPos == std::string::npos || endPos <= startPos) {
        return arg;
    }

    std::string replacement = arg;  // Now we copy arg because we need to modify it

    std::string placeholderContent = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
    size_t commaPos = placeholderContent.find(',');
    if (commaPos != std::string::npos) {
        std::string iniSection = removeQuotes(trim(placeholderContent.substr(0, commaPos)));
        std::string iniKey = removeQuotes(trim(placeholderContent.substr(commaPos + 1)));

        std::string parsedResult = parseValueFromIniSection(iniPath, iniSection, iniKey);
        replacement.replace(startPos, endPos - startPos + searchString.length() + 2, parsedResult);
    }

    return replacement;
}


// this will modify `commands`
std::vector<std::vector<std::string>> getSourceReplacement(const std::vector<std::vector<std::string>>& commands, const std::string& entry, size_t entryIndex, const std::string& packagePath = "") {
    
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

        if (stringToLowercase(commandName) == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            continue;
        } else if (stringToLowercase(commandName) == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            continue;
        }
        
        if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
            
            if (cmd.size() > 1) {
                if ((commandName == "list_source") && listString.empty())
                    listString = removeQuotes(cmd[1]);
                else if ((commandName == "json_file_source") && jsonPath.empty())
                    jsonPath = preprocessPath(cmd[1], packagePath);
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
                        if (replacement.empty()) {
                            replacement = "null";
                            modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                            break;
                        }
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
                        if (replacement.empty()) {
                            replacement = "null";
                            modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                            break;
                        }
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
                        if (replacement.empty()) {
                            replacement = "null";
                            modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                            break;
                        }
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


// forward declarartion
void processCommand(const std::vector<std::string>& cmd, const std::string& packagePath, const std::string& selectedCommand);


/**
 * @brief Interpret and execute a list of commands.
 *
 * This function interprets and executes a list of commands based on their names and arguments.
 *
 * @param commands A list of commands, where each command is represented as a vector of strings.
 */
void interpretAndExecuteCommands(std::vector<std::vector<std::string>>&& commands, const std::string& packagePath="", const std::string& selectedCommand="") {
    bool inEristaSection = false;
    bool inMarikoSection = false;
    bool inTrySection = false;
    std::string listString, jsonString, jsonPath, hexPath, iniPath, lastArg;

    size_t startPos, endPos, listIndex;
    std::string replacement;

    // Overwrite globals
    commandSuccess = true;
    refreshGui = false;
    interpreterLogging = false;

    while (!commands.empty()) {

        auto& cmd = commands.front(); // Get the first command for processing

        if (abortCommand.load(std::memory_order_acquire)) {
            abortCommand.store(false, std::memory_order_release);
            commandSuccess = false;
            return;
        }

        if (cmd.empty()) {
            commands.erase(commands.begin()); // Remove empty command
            continue;
        }

        const std::string& commandName = cmd[0];

        if (commandName == "try:") {
            if (inTrySection && commandSuccess)
                break;
            commandSuccess = true;

            inTrySection = true;
            commands.erase(commands.begin()); // Remove processed command
            continue;
        } else if (commandName == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            commands.erase(commands.begin()); // Remove processed command
            continue;
        } else if (commandName == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            commands.erase(commands.begin()); // Remove processed command
            continue;
        }

        if (!commandSuccess && inTrySection){
            commands.erase(commands.begin()); // Remove processed command
            continue;
        }

        if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
            if (!inTrySection || (commandSuccess && inTrySection)) {
                for (auto& arg : cmd) {
                    lastArg = "";
                    while ((!hexPath.empty() && (arg.find("{hex_file(") != std::string::npos))) {
                        startPos = arg.find("{hex_file(");
                        endPos = arg.find(")}");
                        if (endPos != std::string::npos && endPos > startPos) {
                            replacement = replaceHexPlaceholder(arg.substr(startPos, endPos - startPos + 2), hexPath);
                            if (replacement.empty()) {replacement = "null"; arg.replace(startPos, endPos - startPos + 2, replacement); break;}

                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                if (interpreterLogging)
                                    logMessage("failed replacement ard: "+arg);
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

                            if (replacement.empty()) {replacement = "null"; arg.replace(startPos, endPos - startPos + 2, replacement); break;}

                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            

                            if (arg == lastArg) {
                                if (interpreterLogging)
                                    logMessage("failed replacement ard: "+arg);
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
                            if (replacement.empty()) {replacement = "null"; arg.replace(startPos, endPos - startPos + 2, replacement); break;}

                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                if (interpreterLogging)
                                    logMessage("failed replacement ard: "+arg);
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
                            if (replacement.empty()) {replacement = UNAVAILABLE_SELECTION; arg.replace(startPos, endPos - startPos + 2, replacement); break;}

                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                if (interpreterLogging)
                                    logMessage("failed replacement ard: "+arg);
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
                            if (replacement.empty()) {replacement = UNAVAILABLE_SELECTION; arg.replace(startPos, endPos - startPos + 2, replacement); break;}

                            arg.replace(startPos, endPos - startPos + 2, replacement);
                            if (arg == lastArg) {
                                if (interpreterLogging)
                                    logMessage("failed replacement ard: "+arg);
                                arg.replace(startPos, endPos - startPos + 2, UNAVAILABLE_SELECTION); // fall back replacement value of `UNAVAILABLE_SELECTION`
                                commandSuccess = false;

                                break;
                            }
                        } else
                            break;
                        lastArg = arg;
                    }
                }

                if (interpreterLogging) {
                    std::string message = "Executing command: ";
                    for (const std::string& token : cmd)
                        message += token + " ";
                    logMessage(message);
                }

                const size_t cmdSize = cmd.size();

                if (commandName == "list") {
                    if (cmdSize >= 2) {
                        listString = removeQuotes(cmd[1]);
                    }
                } else if (commandName == "json") {
                    if (cmdSize >= 2) {
                        jsonString = cmd[1];
                    }
                } else if (commandName == "json_file") {
                    if (cmdSize >= 2) {
                        jsonPath = preprocessPath(cmd[1], packagePath);
                    }
                } else if (commandName == "ini_file") {
                    if (cmdSize >= 2) {
                        iniPath = preprocessPath(cmd[1], packagePath);
                    }
                } else if (commandName == "hex_file") {
                    if (cmdSize >= 2) {
                        hexPath = preprocessPath(cmd[1], packagePath);
                    }
                } else {
                    processCommand(cmd, packagePath, selectedCommand);
                }
            }
        }

        commands.erase(commands.begin()); // Remove processed command
    }
}



void processCommand(const std::vector<std::string>& cmd, const std::string& packagePath="", const std::string& selectedCommand="") {

    const std::string& commandName = cmd[0];
    size_t cmdSize = cmd.size();

    if (commandName == "make" || commandName == "mkdir") { // Make command
        if (cmdSize >= 2) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            createDirectory(sourcePath);
        }
    } else if (commandName == "cp" || commandName == "copy") { // Copy command
        if (cmdSize >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            
            if (sourcePath.find('*') != std::string::npos)
                copyFileOrDirectoryByPattern(sourcePath, destinationPath); // Delete files or directories by pattern
            else {
                long long totalBytesCopied = 0;
                long long totalSize = getTotalSize(sourcePath);  // Ensure this is calculated if needed.
                
                copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
            }
        }
    } else if (commandName == "del" || commandName == "delete") { // Delete command
        if (cmdSize >= 2) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            if (!isDangerousCombination(sourcePath)) {
                if (sourcePath.find('*') != std::string::npos)
                    deleteFileOrDirectoryByPattern(sourcePath); // Delete files or directories by pattern
                else
                    deleteFileOrDirectory(sourcePath);
            }
        }
    } else if (commandName.compare(0, 7, "mirror_") == 0) {
        
        if (cmdSize >= 2) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            
            std::string destinationPath;
            if (cmdSize >= 3) {
                destinationPath = preprocessPath(cmd[2], packagePath);
            } else {
                destinationPath = "sdmc:/";
            }
            
            //std::string action = (commandName == "mirror_copy" || commandName == "mirror_cp") ? "copy" : "delete";
            mirrorFiles(sourcePath, destinationPath, (commandName == "mirror_copy" || commandName == "mirror_cp") ? "copy" : "delete");
        }
    } else if (commandName == "mv" || commandName == "move" || commandName == "rename" ) { // Rename command
        if (cmdSize >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            if (!isDangerousCombination(sourcePath)) {
                if (sourcePath.find('*') != std::string::npos)
                    moveFilesOrDirectoriesByPattern(sourcePath, destinationPath); // Move files by pattern
                else
                    moveFileOrDirectory(sourcePath, destinationPath); // Move single file or directory
            }
        }
    } else if (commandName == "add-ini-section") {
        if (cmdSize >= 2) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string desiredSection = removeQuotes(cmd[2]);
            addIniSection(sourcePath.c_str(), desiredSection.c_str());
        }
    } else if (commandName == "rename-ini-section") {
        if (cmdSize >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string desiredSection = removeQuotes(cmd[2]);
            std::string desiredNewSection = removeQuotes(cmd[3]);
            renameIniSection(sourcePath.c_str(), desiredSection.c_str(), desiredNewSection.c_str());
        }
    } else if (commandName == "remove-ini-section") {
        if (cmdSize >= 2) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string desiredSection = removeQuotes(cmd[2]);
            removeIniSection(sourcePath.c_str(), desiredSection.c_str());
        }
    } else if (commandName == "set-ini-val" || commandName == "set-ini-value") {
        if (cmdSize >= 5) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string desiredSection = removeQuotes(cmd[2]);
            std::string desiredKey = removeQuotes(cmd[3]);
            std::string desiredValue = std::accumulate(cmd.begin() + 4, cmd.end(), std::string(""),
                [](const std::string& a, const std::string& b) -> std::string {
                    return a.empty() ? b : a + " " + b;
                });
            
            setIniFileValue(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
        }
    } else if (commandName == "set-ini-key") {
        if (cmdSize >= 5) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string desiredSection = removeQuotes(cmd[2]);
            std::string desiredKey = removeQuotes(cmd[3]);
            std::string desiredNewKey = std::accumulate(cmd.begin() + 4, cmd.end(), std::string(),
                [](const std::string& a, const std::string& b) -> std::string {
                    return a.empty() ? b : a + " " + b;
                });
            
            setIniFileKey(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredNewKey.c_str());
        }
    }else if (commandName == "set-footer") {
        if (cmdSize >= 2) {
            std::string desiredValue = removeQuotes(cmd[1]);
            setIniFileValue((packagePath+configFileName).c_str(), selectedCommand.c_str(), "footer", desiredValue.c_str());
        }
    } else if (commandName.compare(0, 7, "hex-by-") == 0) {
        if (cmdSize >= 4) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            const std::string& secondArg = removeQuotes(cmd[2]);
            const std::string& thirdArg = removeQuotes(cmd[3]);
            std::string actionType = commandName.substr(7);  // Extracts the specific action type part of the command name
            
            if (actionType == "offset") {
                hexEditByOffset(sourcePath.c_str(), secondArg.c_str(), thirdArg.c_str());
            } else if (actionType == "swap") {
                size_t occurrence = (cmdSize >= 5) ? std::stoul(removeQuotes(cmd[4])) : 1;
                hexEditFindReplace(sourcePath, secondArg, thirdArg, occurrence);
            } else if (actionType == "string") {
                std::string hexDataToReplace = asciiToHex(secondArg);
                std::string hexDataReplacement = padToEqualLength(asciiToHex(thirdArg), hexDataToReplace.length());
                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, cmdSize >= 5 ? std::stoul(removeQuotes(cmd[4])) : 1);
            } else if (actionType == "decimal") {
                std::string hexDataToReplace = decimalToHex(secondArg);
                std::string hexDataReplacement = padToEqualLength(decimalToHex(thirdArg), hexDataToReplace.length());
                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, cmdSize >= 5 ? std::stoul(removeQuotes(cmd[4])) : 1);
            } else if (actionType == "rdecimal") {
                std::string hexDataToReplace = decimalToReversedHex(secondArg);
                std::string hexDataReplacement = padToEqualLength(decimalToReversedHex(thirdArg), hexDataToReplace.length());
                hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, cmdSize >= 5 ? std::stoul(removeQuotes(cmd[4])) : 1);
            } else if (actionType.starts_with("custom-offset") || actionType.starts_with("custom-decimal-offset") || actionType.starts_with("custom-rdecimal-offset")) {
                if (cmdSize >= 5) {
                    std::string customPattern = removeQuotes(cmd[2]);
                    std::string offset = removeQuotes(cmd[3]);
                    std::string hexDataReplacement = removeQuotes(cmd[4]);
                    
                    if (actionType == "custom-decimal-offset") {
                        hexDataReplacement = decimalToHex(hexDataReplacement);
                    } else if (actionType == "custom-rdecimal-offset") {
                        hexDataReplacement = decimalToReversedHex(hexDataReplacement);
                    }
                    
                    hexEditByCustomOffset(sourcePath.c_str(), customPattern.c_str(), offset.c_str(), hexDataReplacement.c_str());
                }
            }
        }
    } else if (commandName == "download") {
        if (cmdSize >= 3) {
            std::string fileUrl = preprocessUrl(cmd[1]);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            bool downloadSuccess = false;
            
            //setIniFileValue((packagePath+configFileName).c_str(), selectedCommand.c_str(), "footer", "downloading");
            for (size_t i = 0; i < 3; ++i) { // Try 3 times.
                downloadSuccess = downloadFile(fileUrl, destinationPath);
                if (abortDownload.load(std::memory_order_acquire)) {
                    downloadSuccess = false;
                    break;
                }
                if (downloadSuccess)
                    break;
            }
            //downloadSuccess = enqueueDownloadFile(fileUrl, destinationPath);
            //downloadSuccess = downloadFile(fileUrl, destinationPath);
            commandSuccess = (downloadSuccess && commandSuccess);
        }
    } else if (commandName == "unzip") {
        if (cmdSize >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            commandSuccess = unzipFile(sourcePath, destinationPath) && commandSuccess;
            //commandSuccess = enqueueUnzipFile(sourcePath, destinationPath) && commandSuccess;
        }
    } else if (commandName == "pchtxt2ips") {
        if (cmdSize >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            commandSuccess = pchtxt2ips(sourcePath, destinationPath) && commandSuccess;
        }
    } else if (commandName == "exec") {
        if (cmdSize >= 2) {
            std::string bootCommandName = removeQuotes(cmd[1]);
            if (isFileOrDirectory(packagePath+bootPackageFileName)) {
                auto bootOptions = loadOptionsFromIni(packagePath+bootPackageFileName, true);
                std::string bootOptionName;
                
                bool resetCommandSuccess;
                for (auto& bootOption:bootOptions) {
                    bootOptionName = bootOption.first;
                    auto& bootCommands = bootOption.second;
                    if (bootOptionName == bootCommandName) {
                        resetCommandSuccess = false;
                        if (!commandSuccess)
                            resetCommandSuccess = true;
                        interpretAndExecuteCommands(std::move(bootCommands), packagePath, bootOptionName); // Execute modified 
                        //enqueueInterpreterCommands(std::move(bootCommands), packagePath+bootPackageFileName, bootOptionName);
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
                rebootOption = removeQuotes(cmd[1]);
                
                if (cmdSize >= 3) {
                    std::string option;
                    if (rebootOption == "boot") {
                        option = removeQuotes(cmd[2]);
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
                        option = removeQuotes(cmd[2]);
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
    } else if (commandName == "exit") {
        triggerExit.store(true, std::memory_order_release);
        return;
    } else if (commandName == "backlight") {
        if (cmdSize >= 2) {
            std::string togglePattern = removeQuotes(cmd[1]);
            lblInitialize();
            if (togglePattern == "on")
                lblSwitchBacklightOn(0);
            else if (togglePattern == "off")
                lblSwitchBacklightOff(0);
            lblExit();
        }
        
    } else if (commandName == "refresh") {
        refreshGui = true;
    } else if (commandName == "logging") {
        interpreterLogging = !interpreterLogging;
    } else if (commandName == "clear") {
        if (cmdSize >= 2) {
            std::string clearOption = removeQuotes(cmd[1]);
            if (clearOption == "log")
                deleteFileOrDirectory(logFilePath);
            else if (clearOption == "hex_sum_cache")
                hexSumCache.clear();
        }
    }
}



// Thread information structure
Thread interpreterThread;
std::queue<std::tuple<std::vector<std::vector<std::string>>, std::string, std::string>> interpreterQueue;
std::mutex queueMutex;
std::condition_variable queueCondition;
std::atomic<bool> interpreterThreadExit{false};
//static bool lastRunningInterpreter = false;

void clearInterpreterFlags(bool state = false) {
    abortDownload.store(state, std::memory_order_release);
    abortUnzip.store(state, std::memory_order_release);
    abortFileOp.store(state, std::memory_order_release);
    abortCommand.store(state, std::memory_order_release);
}



void backgroundInterpreter(void*) {
    while (!interpreterThreadExit.load()) {
        std::tuple<std::vector<std::vector<std::string>>, std::string, std::string> args;

        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [] { return !interpreterQueue.empty() || interpreterThreadExit.load(); });
            if (interpreterThreadExit.load()) {
                logMessage("Exiting Thread...");
                break;
            }
            if (!interpreterQueue.empty()) {
                args = std::move(interpreterQueue.front());
                interpreterQueue.pop();
            }
        } // Release the lock before processing the command

        if (!std::get<0>(args).empty()) {
            logMessage("Start of interpreter");
            // Clear flags and perform any cleanup if necessary
            clearInterpreterFlags();
            threadFailure.store(false, std::memory_order_release);
            
            runningInterpreter.store(true, std::memory_order_release);
            interpretAndExecuteCommands(std::move(std::get<0>(args)), std::move(std::get<1>(args)), std::move(std::get<2>(args)));

            runningInterpreter.store(false, std::memory_order_release);
            // Clear flags and perform any cleanup if necessary
            clearInterpreterFlags();
            //interpreterThreadExit.store(true, std::memory_order_release);
            logMessage("End of interpreter");
            //break;
        }
        //logMessage("looping...");
    }
}

void closeInterpreterThread() {
   //logMessage("Closing interpreter...");
   {
       std::lock_guard<std::mutex> lock(queueMutex);
       interpreterThreadExit.store(true, std::memory_order_release);
       queueCondition.notify_one();
   }
   threadWaitForExit(&interpreterThread);
   threadClose(&interpreterThread);
   // Reset flags
   clearInterpreterFlags();
   //logMessage("Interpreter has been closed.");
}



void startInterpreterThread(int stackSize = 0x8000) {
    //if (isDownloadCommand)
    //    stackSize = 0x8000;
    interpreterThreadExit.store(false, std::memory_order_release);

    int result = threadCreate(&interpreterThread, backgroundInterpreter, nullptr, nullptr, stackSize, 0x2B, -2);
    if (result != 0) {
        commandSuccess = false;
        logMessage("Failed to create interpreter thread.");
        return;
    }
    threadStart(&interpreterThread);
}




void enqueueInterpreterCommands(std::vector<std::vector<std::string>>&& commands, const std::string& packagePath, const std::string& selectedCommand) {
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        interpreterQueue.emplace(std::move(commands), packagePath, selectedCommand);
    }

    // Start a new interpreter thread
    //if (isDownloadCommand) {
    //    startInterpreterThread(0x8000);
    //} else {
    //    startInterpreterThread();
    //}
    queueCondition.notify_one();
}

