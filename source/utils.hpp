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
#include <ultra.hpp>
#include <tesla.hpp>
#include <switch.h>
#include <payload.hpp> // Studious Pancake
#include <util.hpp> // Studious Pancake

#include <fstream>
#include <fnmatch.h>
#include <numeric>
#include <queue>
#include <mutex>
#include <condition_variable>


static std::atomic<bool> abortCommand(false);
static std::atomic<bool> triggerExit(false);


bool isDownloadCommand = false;
bool commandSuccess = false;
bool refreshGui = false;
bool interpreterLogging = false;

bool usingErista = util::IsErista();
bool usingMariko = util::IsMariko();

/**
 * @brief Ultrahand-Overlay Configuration Paths
 *
 * This block of code defines string variables for various configuration and directory paths
 * used in the Ultrahand-Overlay project. These paths include:
 *
 * - `PACKAGE_FILENAME`: The name of the package file ("package.ini").
 * - `CONFIG_FILENAME`: The name of the configuration file ("config.ini").
 * - `SETTINGS_PATH`: The base path for Ultrahand settings ("sdmc:/config/ultrahand/").
 * - `ULTRAHAND_CONFIG_INI_PATH`: The full path to the Ultrahand settings configuration file.
 * - `PACKAGE_PATH`: The base directory for packages ("sdmc:/switch/.packages/").
 * - `OVERLAY_PATH`: The base directory for overlays ("sdmc:/switch/.overlays/").
 * - `TESLA_CONFIG_INI_PATH`: The full path to the Tesla settings configuration file.
 *
 * These paths are used within the Ultrahand-Overlay project to manage configuration files
 * and directories.
 */


/**
 * @brief Shuts off all connected controllers.
 *
 * This function disconnects all connected controllers by utilizing the Bluetooth manager (btm) service.
 * It checks the firmware version and uses the appropriate function to get the device condition and disconnects
 * the controllers.
 */
void powerOffAllControllers() {
    Result rc;
    static s32 g_connected_count = 0;
    static BtdrvAddress g_addresses[8] = {};
    
    // Initialize Bluetooth manager
    rc = btmInitialize();
    if (R_FAILED(rc)) {
        commandSuccess = false;
        //LogLine("Error btmInitialize: %u - %X\n", rc, rc);
        return;
    }
    
    if (hosversionAtLeast(13, 0, 0)) {
        BtmConnectedDeviceV13 connected_devices[8];
        rc = btmGetDeviceCondition(BtmProfile_None, connected_devices, 8, &g_connected_count);
        if (R_SUCCEEDED(rc)) {
            for (s32 i = 0; i != g_connected_count; ++i) {
                g_addresses[i] = connected_devices[i].address;
            }
        } else {
            commandSuccess = false;
            //LogLine("Error btmGetDeviceCondition: %u - %X\n", rc, rc);
        }
    } else {
        BtmDeviceCondition g_device_condition;
        rc = btmLegacyGetDeviceCondition(&g_device_condition);
        if (R_SUCCEEDED(rc)) {
            g_connected_count = g_device_condition.v900.connected_count;
            for (s32 i = 0; i != g_connected_count; ++i) {
                g_addresses[i] = g_device_condition.v900.devices[i].address;
            }
        } else {
            commandSuccess = false;
            //LogLine("Error btmLegacyGetDeviceCondition: %u - %X\n", rc, rc);
        }
    }
    
    if (R_SUCCEEDED(rc)) {
        //LogLine("Disconnecting controllers. Count: %u\n", g_connected_count);
        for (int i = 0; i != g_connected_count; ++i) {
            Result rc = btmHidDisconnect(g_addresses[i]);
            if (R_FAILED(rc)) {
                commandSuccess = false;
                //LogLine("Error btmHidDisconnect: %u - %X\n", rc, rc);
            } else {
                //LogLine("Disconnected Address: %u - %X\n", g_addresses[i], g_addresses[i]);
            }
        }
        //LogLine("All controllers disconnected.\n");
    } else {
        commandSuccess = false;
    }
    
    // Exit Bluetooth manager
    btmExit();
}

std::unordered_map<std::string, std::string> createButtonCharMap() {
    std::unordered_map<std::string, std::string> map;
    for (const auto& keyInfo : tsl::impl::KEYS_INFO) {
        map[keyInfo.name] = keyInfo.glyph;
    }
    return map;
}

std::unordered_map<std::string, std::string> buttonCharMap = createButtonCharMap();


std::string convertComboToUnicode(const std::string& combo) {

    std::istringstream iss(combo);
    std::string token;
    std::string unicodeCombo;

    while (std::getline(iss, token, '+')) {
        unicodeCombo += buttonCharMap[trim(token)] + "+";
    }

    if (!unicodeCombo.empty()) {
        unicodeCombo.pop_back();  // Remove the trailing '+'
    }

    return unicodeCombo;
}





void initializeTheme(std::string themeIniPath = THEME_CONFIG_INI_PATH) {
    tsl::hlp::ini::IniData themeData;
    bool initialize = false;

    if (isFileOrDirectory(themeIniPath)) {
        themeData = getParsedDataFromIniFile(themeIniPath);

        if (themeData.count(THEME_STR) > 0) {
            auto& themeSection = themeData[THEME_STR];

            // Iterate through each default setting and apply if not already set
            for (const auto& [key, value] : defaultThemeSettingsMap) {
                if (themeSection.count(key) == 0) {
                    setIniFileValue(themeIniPath, THEME_STR, key, value);
                }
            }
        } else {
            initialize = true;
        }
    } else {
        initialize = true;
    }

    // If the file does not exist or the theme section is missing, initialize with all default values
    if (initialize) {
        for (const auto& [key, value] : defaultThemeSettingsMap) {
            setIniFileValue(themeIniPath, THEME_STR, key, value);
        }
    }
}


/**
 * @brief Copy Tesla key combo to Ultrahand settings.
 *
 * This function retrieves the key combo from Tesla settings and copies it to Ultrahand settings.
 */
void copyTeslaKeyComboToUltrahand() {
    std::string keyCombo = ULTRAHAND_COMBO_STR;
    std::map<std::string, std::map<std::string, std::string>> parsedData;
    
    bool teslaConfigExists = isFileOrDirectory(TESLA_CONFIG_INI_PATH);
    bool ultrahandConfigExists = isFileOrDirectory(ULTRAHAND_CONFIG_INI_PATH);

    bool initializeTesla = false;
    std::string teslaKeyCombo = keyCombo;

    if (teslaConfigExists) {
        parsedData = getParsedDataFromIniFile(TESLA_CONFIG_INI_PATH);
        if (parsedData.count(TESLA_STR) > 0) {
            auto& teslaSection = parsedData[TESLA_STR];
            if (teslaSection.count(KEY_COMBO_STR) > 0) {
                teslaKeyCombo = teslaSection[KEY_COMBO_STR];
            } else {
                initializeTesla = true;
            }
        } else {
            initializeTesla = true;
        }
    } else {
        initializeTesla = true;
    }
    
    bool initializeUltrahand = false;
    if (ultrahandConfigExists) {
        parsedData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
        if (parsedData.count(ULTRAHAND_PROJECT_NAME) > 0) {
            auto& ultrahandSection = parsedData[ULTRAHAND_PROJECT_NAME];
            if (ultrahandSection.count(KEY_COMBO_STR) > 0) {
                keyCombo = ultrahandSection[KEY_COMBO_STR];
            } else {
                initializeUltrahand = true;
            }
        } else {
            initializeUltrahand = true;
        }
    } else {
        initializeUltrahand = true;
    }

    if (initializeTesla || (teslaKeyCombo != keyCombo)) {
        setIniFileValue(TESLA_CONFIG_INI_PATH, TESLA_STR, KEY_COMBO_STR, keyCombo);
    }

    if (initializeUltrahand) {
        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR, keyCombo);
    }

    tsl::impl::parseOverlaySettings();
}



// Constants for overlay module
constexpr int OverlayLoaderModuleId = 348;
constexpr Result ResultSuccess = MAKERESULT(0, 0);
constexpr Result ResultParseError = MAKERESULT(OverlayLoaderModuleId, 1);

/**
 * @brief Retrieves overlay module information from a given file.
 *
 * @param filePath The path to the overlay module file.
 * @return A tuple containing the result code, module name, and display version.
 */
std::tuple<Result, std::string, std::string> getOverlayInfo(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return {ResultParseError, "", ""};
    }
    
    NroHeader nroHeader;
    NroAssetHeader assetHeader;
    NacpStruct nacp;
    
    // Read NRO header
    file.seekg(sizeof(NroStart), std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&nroHeader), sizeof(NroHeader))) {
        return {ResultParseError, "", ""};
    }
    
    // Read asset header
    file.seekg(nroHeader.size, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&assetHeader), sizeof(NroAssetHeader))) {
        return {ResultParseError, "", ""};
    }
    
    // Read NACP struct
    file.seekg(nroHeader.size + assetHeader.nacp.offset, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&nacp), sizeof(NacpStruct))) {
        return {ResultParseError, "", ""};
    }
    
    // Assuming nacp.lang[0].name and nacp.display_version are null-terminated
    return {
        ResultSuccess,
        std::string(nacp.lang[0].name),
        std::string(nacp.display_version)
    };
}

void drawTable(std::unique_ptr<tsl::elm::List>& list, const std::vector<std::string>& sectionLines, const std::vector<std::string>& infoLines,
    const size_t& columnOffset = 120, const size_t& startGap = 20, const size_t& endGap = 3, const size_t& newlineGap = 0,
    const std::string& tableSectionTextColor = DEFAULT_STR, const std::string& tableInfoTextColor = DEFAULT_STR, const std::string& alignment = LEFT_STR, const bool& hideTableBackground = false) {

    size_t lineHeight = 16;
    size_t fontSize = 16;
    size_t xMax = tsl::cfg::FramebufferWidth - 95;

    auto sectionTextColor = tsl::gfx::Renderer::a(tsl::sectionTextColor);
    auto infoTextColor = tsl::gfx::Renderer::a(tsl::infoTextColor);
    auto alternateSectionTextColor = tsl::gfx::Renderer::a(tsl::warningTextColor);
    auto alternateInfoTextColor = tsl::gfx::Renderer::a(tsl::warningTextColor);

    if (tableSectionTextColor != DEFAULT_STR) {
        if (tableSectionTextColor == "warning") {
            alternateSectionTextColor = tsl::warningTextColor;
        } else {
            alternateSectionTextColor = tsl::RGB888(tableSectionTextColor);
        }
    }

    if (tableInfoTextColor != DEFAULT_STR) {
        if (tableInfoTextColor == "warning") {
            alternateInfoTextColor = tsl::warningTextColor;
        } else {
            alternateInfoTextColor = tsl::RGB888(tableInfoTextColor);
        }
    }

    size_t totalHeight = lineHeight * sectionLines.size() + newlineGap * (sectionLines.size() - 1) + endGap;

    // Precompute all y-offsets for sections and info lines
    std::vector<s32> yOffsets(sectionLines.size());
    for (size_t i = 0; i < sectionLines.size(); ++i) {
        yOffsets[i] = startGap + (i * (lineHeight + newlineGap));
    }

    // Precompute all x-offsets for info lines based on alignment
    std::vector<int> infoXOffsets(infoLines.size());
    std::vector<float> infoStringWidths(infoLines.size());

    // Precompute string widths using the provided renderer instance in the lambda
    for (size_t i = 0; i < infoLines.size(); ++i) {
        infoStringWidths[i] = 0.0f;  // Initialize with a default value
    }

    // Add the TableDrawer item
    list->addItem(new tsl::elm::TableDrawer([=](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) mutable {
        for (size_t i = 0; i < infoLines.size(); ++i) {
            if (infoStringWidths[i] == 0.0f) {  // Calculate only if not already calculated
                if (infoLines[i].find(NULL_STR) == std::string::npos)
                    infoStringWidths[i] = renderer->calculateStringWidth(infoLines[i], fontSize, false);
                else
                    infoStringWidths[i] = renderer->calculateStringWidth(UNAVAILABLE_SELECTION, fontSize, false);
            }

            if (alignment == LEFT_STR) {
                infoXOffsets[i] = columnOffset;
            } else if (alignment == RIGHT_STR) {
                infoXOffsets[i] = xMax - infoStringWidths[i];
            } else if (alignment == CENTER_STR) {
                infoXOffsets[i] = columnOffset + (xMax - infoStringWidths[i]) / 2;
            }
        }

        for (size_t i = 0; i < sectionLines.size(); ++i) {
            renderer->drawString(sectionLines[i].c_str(), false, x + 12, y + yOffsets[i], fontSize, renderer->a((tableSectionTextColor == DEFAULT_STR) ? sectionTextColor : alternateSectionTextColor));
            // Check if infoLines[i] is "null" and replace it with UNAVAILABLE_SELECTION if true
            std::string infoText = (infoLines[i].find(NULL_STR) != std::string::npos) ? UNAVAILABLE_SELECTION : infoLines[i];
            renderer->drawString(infoText.c_str(), false, x + infoXOffsets[i], y + yOffsets[i], fontSize, renderer->a((tableInfoTextColor == DEFAULT_STR) ? infoTextColor : alternateInfoTextColor));
        }
    }, hideTableBackground, endGap), totalHeight);
}




void applyPlaceholderReplacement(std::vector<std::string>& cmd, std::string hexPath, std::string iniPath, std::string listString, std::string listPath, std::string jsonString, std::string jsonPath);

void addTable(std::unique_ptr<tsl::elm::List>& list, std::vector<std::vector<std::string>>& tableData,
    const std::string& packagePath, const size_t& columnOffset=160, const size_t& tableStartGap=20, const size_t& tableEndGap=3, const size_t& tableSpacing=0, const std::string& tableSectionTextColor=DEFAULT_STR, const std::string& tableInfoTextColor=DEFAULT_STR, const std::string& tableAlignment=RIGHT_STR, const bool& hideTableBackground = false) {

    //std::string sectionString, infoString;
    std::vector<std::string> sectionLines, infoLines;

    std::string hexPath, iniPath, listString, listPath, jsonString, jsonPath;

    //std::string columnAlignment = tableAlignment;

    bool inEristaSection = false;
    bool inMarikoSection = false;
    //size_t tableSize = 0;
    //size_t newlineGap = 10;

    for (auto& commands : tableData) {

        auto& cmd = commands; // Get the first command for processing

        if (abortCommand.load(std::memory_order_acquire)) {
            abortCommand.store(false, std::memory_order_release);
            commandSuccess = false;
            return;
        }

        if (cmd.empty()) {
            //commands.erase(commands.begin()); // Remove empty command
            continue;
        }

        const std::string& commandName = cmd[0];

        if (commandName == "erista:") {
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

        if ((inEristaSection && !inMarikoSection && usingErista) || (!inEristaSection && inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {

            applyPlaceholderReplacement(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

            if (interpreterLogging) {
                std::string message = "Reading line:";
                for (const std::string& token : cmd)
                    message += " " + token;
                logMessage(message);
            }

            const size_t cmdSize = cmd.size();

            if (commandName == LIST_STR) {
                if (cmdSize >= 2) {
                    listString = removeQuotes(cmd[1]);
                }
            } else if (commandName == LIST_FILE_STR) {
                if (cmdSize >= 2) {
                    listPath = preprocessPath(cmd[1], packagePath);
                }
            } else if (commandName == JSON_STR) {
                if (cmdSize >= 2) {
                    jsonString = cmd[1];
                }
            } else if (commandName == JSON_FILE_STR) {
                if (cmdSize >= 2) {
                    jsonPath = preprocessPath(cmd[1], packagePath);
                }
            } else if (commandName == INI_FILE_STR) {
                if (cmdSize >= 2) {
                    iniPath = preprocessPath(cmd[1], packagePath);
                }
            } else if (commandName == HEX_FILE_STR) {
                if (cmdSize >= 2) {
                    hexPath = preprocessPath(cmd[1], packagePath);
                }
            } else {
                sectionLines.push_back(cmd[0]);
                infoLines.push_back(cmd[2]);
                //sectionString += cmd[0] + "\n";
                //infoString += cmd[2] + "\n";
                //tableSize++;
            }
        }
    }

    // seperate sectionString and info string.  the sections will be on the left side of the "=", the info will be on the right side of the "=" within the string.  the end of an entry will be met with a newline (except for the very last entry). 
    // sectionString and infoString will each have equal newlines (denoting )

    drawTable(list, sectionLines, infoLines, columnOffset, tableStartGap, tableEndGap, tableSpacing, tableSectionTextColor, tableInfoTextColor, tableAlignment, hideTableBackground);
}


void addHelpInfo(std::unique_ptr<tsl::elm::List>& list) {
    // Add a section break with small text to indicate the "Commands" section
    list->addItem(new tsl::elm::CategoryHeader(USER_GUIDE));

    // Adjust the horizontal offset as needed
    int xOffset = std::stoi(USERGUIDE_OFFSET);

    // Define the section lines and info lines directly
    const std::vector<std::string> sectionLines = {
        SETTINGS_MENU,
        SCRIPT_OVERLAY,
        STAR_FAVORITE,
        APP_SETTINGS
    };

    const std::vector<std::string> infoLines = {
        "\uE0B5 (" + ON_MAIN_MENU + ")",
        "\uE0B6 (" + ON_A_COMMAND + ")",
        "\uE0E2 (" + ON_OVERLAY_PACKAGE + ")",
        "\uE0E3 (" + ON_OVERLAY_PACKAGE + ")"
    };

    // Draw the table with the defined lines
    drawTable(list, sectionLines, infoLines, xOffset, 20, 12, 3);
}



void addPackageInfo(std::unique_ptr<tsl::elm::List>& list, auto& packageHeader, std::string type = PACKAGE_STR) {
    // Add a section break with small text to indicate the "Commands" section
    list->addItem(new tsl::elm::CategoryHeader(type == PACKAGE_STR ? PACKAGE_INFO : OVERLAY_INFO));

    int maxLineLength = 28;  // Adjust the maximum line length as needed
    int xOffset = 120;    // Adjust the horizontal offset as needed
    //int numEntries = 0;   // Count of the number of entries

    std::vector<std::string> sectionLines;
    std::vector<std::string> infoLines;

    // Helper function to add text with wrapping
    auto addWrappedText = [&](const std::string& header, const std::string& text) {
        sectionLines.push_back(header);
        std::string::size_type aboutHeaderLength = header.length();
        
        size_t startPos = 0;
        size_t spacePos = 0;

        size_t endPos;
        std::string line;

        while (startPos < text.length()) {
            endPos = std::min(startPos + maxLineLength, text.length());
            line = text.substr(startPos, endPos - startPos);
            
            // Check if the current line ends with a space; if not, find the last space in the line
            if (endPos < text.length() && text[endPos] != ' ') {
                spacePos = line.find_last_of(' ');
                if (spacePos != std::string::npos) {
                    endPos = startPos + spacePos;
                    line = text.substr(startPos, endPos - startPos);
                }
            }

            infoLines.push_back(line);
            startPos = endPos + 1;
            //numEntries++;

            // Add corresponding newline to the packageSectionString
            if (startPos < text.length())
                sectionLines.push_back(std::string(aboutHeaderLength, ' '));
        }
    };

    // Adding package header info
    if (!packageHeader.title.empty()) {
        sectionLines.push_back(TITLE);
        infoLines.push_back(packageHeader.title);
        //numEntries++;
    }

    if (!packageHeader.version.empty()) {
        sectionLines.push_back(VERSION);
        infoLines.push_back(packageHeader.version);
        //numEntries++;
    }

    if (!packageHeader.creator.empty()) {
        //sectionLines.push_back(CREATOR);
        //infoLines.push_back(packageHeader.creator);
        //numEntries++;
        addWrappedText(CREATOR, packageHeader.creator);
    }

    if (!packageHeader.about.empty()) {
        addWrappedText(ABOUT, packageHeader.about);
    }

    if (!packageHeader.credits.empty()) {
        addWrappedText(CREDITS, packageHeader.credits);
    }

    // Drawing the table with section lines and info lines
    drawTable(list, sectionLines, infoLines, xOffset, 20, 12, 3);
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
    static const std::vector<std::string> protectedFolders = {
        "sdmc:/Nintendo/",
        "sdmc:/emuMMC/",
        "sdmc:/emuMMC/RAW1/",
        "sdmc:/atmosphere/",
        "sdmc:/bootloader/",
        "sdmc:/switch/",
        "sdmc:/config/",
        ROOT_PATH
    };
    static const std::vector<std::string> ultraProtectedFolders = {
        "sdmc:/Nintendo/Contents/",
        "sdmc:/Nintendo/save/",
        "sdmc:/emuMMC/RAW1/Nintendo/Contents/",
        "sdmc:/emuMMC/RAW1/Nintendo/save/"
    };
    static const std::vector<std::string> dangerousCombinationPatterns = {
        "*",        // Wildcard in general
        "*/"        // Wildcard in general
    };
    static const std::vector<std::string> dangerousPatterns = {
        "..",       // Attempts to traverse to parent directories
        "~"         // Represents user's home directory, can be dangerous if misused
    };

    // Check ultra-protected folders
    for (const auto& folder : ultraProtectedFolders) {
        if (patternPath.find(folder) == 0) {
            return true; // Path is an ultra-protected folder
        }
    }

    // Check protected folders and dangerous patterns
    for (const auto& folder : protectedFolders) {
        if (patternPath == folder) {
            return true; // Path is a protected folder
        }
        if (patternPath.find(folder) == 0) {
            std::string relativePath = patternPath.substr(folder.size());

            // Check for dangerous patterns in the relative path
            for (const auto& pattern : dangerousPatterns) {
                if (relativePath.find(pattern) != std::string::npos) {
                    return true; // Relative path contains a dangerous pattern
                }
            }

            // Check for dangerous combination patterns in the relative path directly within the protected folder
            size_t slashPos = relativePath.find('/');
            bool isDirectlyWithinProtectedFolder = (slashPos == std::string::npos);
            if (isDirectlyWithinProtectedFolder) {
                for (const auto& combination : dangerousCombinationPatterns) {
                    if (relativePath.find(combination) != std::string::npos) {
                        return true; // Relative path contains a dangerous combination pattern
                    }
                }
            }

            // Check for wildcard patterns that could affect ultra-protected folders
            for (const auto& ultraFolder : ultraProtectedFolders) {
                if (patternPath.find(ultraFolder.substr(folder.size())) == 0) {
                    return true; // Path with wildcard could affect an ultra-protected folder
                }
            }
        }
    }

    // Check dangerous patterns in general
    for (const auto& pattern : dangerousPatterns) {
        if (patternPath.find(pattern) != std::string::npos) {
            return true; // Path contains a dangerous pattern
        }
    }

    // Check wildcard at root level
    if (patternPath.find(":/") != std::string::npos) {
        std::string rootPath = patternPath.substr(0, patternPath.find(":/") + 2);
        if (rootPath.find('*') != std::string::npos) {
            return true; // Root path contains a wildcard
        }
    }

    return false; // No dangerous combinations found
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
            configFileOut << "[Reboot]\nreboot\n\n[Shutdown]\nshutdown\n";
            //configFileOut << "[*Reboot]\n[HOS Reboot]\nreboot\n[Hekate Reboot]\nreboot HEKATE\n[UMS Reboot]\nreboot UMS\n\n[Commands]\n[Shutdown]\nshutdown";
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

        if (line.empty() || line.front() == '#') continue; // Skip empty or comment lines

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

    if (sourceType == JSON_STR) {
        jsonData.reset(stringToJson(jsonStringOrPath));
    } else if (sourceType == JSON_FILE_STR) {
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
inline std::string replacePlaceholder(const std::string& input, const std::string& placeholder, const std::string& replacement) {
    size_t pos = input.find(placeholder);
    if (pos == std::string::npos) {
        return input;  // Returns original string directly if no placeholder is found
    }
    std::string result = input;
    result.replace(pos, placeholder.length(), replacement);
    return result;
}




std::string replaceIniPlaceholder(const std::string& arg, const std::string& commandName, const std::string& iniPath) {

    const std::string searchString = "{" + commandName + "(";
    size_t startPos = arg.find(searchString);
    if (startPos == std::string::npos) {
        return arg;
    }

    size_t endPos = arg.find(")}", startPos);
    if (endPos == std::string::npos || endPos <= startPos) {
        return arg;
    }

    std::string replacement = arg;  // Copy arg because we need to modify it



    std::string placeholderContent = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
    placeholderContent = trim(placeholderContent);

    size_t commaPos = placeholderContent.find(',');
    if (commaPos != std::string::npos) {
        std::string iniSection = removeQuotes(trim(placeholderContent.substr(0, commaPos)));
        std::string iniKey = removeQuotes(trim(placeholderContent.substr(commaPos + 1)));

        std::string parsedResult = parseValueFromIniSection(iniPath, iniSection, iniKey);
        // Replace the placeholder with the parsed result and keep the remaining string intact
        replacement = replacement.substr(0, startPos) + parsedResult + replacement.substr(endPos + 2);
    } else {
        // Check if the content is an integer
        if (std::all_of(placeholderContent.begin(), placeholderContent.end(), ::isdigit)) {
            size_t entryIndex = std::stoi(placeholderContent);

            // Return list of section names and use entryIndex to get the specific entry
            std::vector<std::string> sectionNames = getIniSections(iniPath);
            if (entryIndex < sectionNames.size()) {
                std::string sectionName = sectionNames[entryIndex];
                replacement = replacement.substr(0, startPos) + sectionName + replacement.substr(endPos + 2);
            } else {
                // Handle the case where entryIndex is out of range
                replacement = replacement.substr(0, startPos) + NULL_STR + replacement.substr(endPos + 2);
            }
        } else {
            // Handle the case where the placeholder content is not a valid index
            replacement = replacement.substr(0, startPos) + NULL_STR + replacement.substr(endPos + 2);
        }
    }



    return replacement;
}



/**
 * @brief Replaces a JSON source placeholder with the actual JSON source.
 *
 * @param arg The input string containing the placeholder.
 * @param commandName The name of the JSON command (e.g., "json", "json_file").
 * @param jsonPathOrString The path to the JSON file or the JSON string itself.
 * @return std::string The input string with the placeholder replaced by the actual JSON source,
 *                   or the original input string if replacement failed or jsonDict is nullptr.
 */
// Replace JSON placeholders in the string
std::string replaceJsonPlaceholder(const std::string& arg, const std::string& commandName, const std::string& jsonPathOrString) {
    std::unique_ptr<json_t, JsonDeleter> jsonDict;
    if (commandName == "json" || commandName == "json_source") {
        jsonDict.reset(stringToJson(jsonPathOrString));
    } else if (commandName == "json_file" || commandName == "json_file_source") {
        jsonDict.reset(readJsonFromFile(jsonPathOrString));
    }

    if (!jsonDict) {
        return arg; // Return original string if JSON data couldn't be loaded
    }

    std::string replacement = arg;
    const std::string searchString = "{" + commandName + "(";
    size_t startPos = replacement.find(searchString);

    // Declare variables outside the loop to avoid reinitialization
    size_t endPos = 0;
    size_t nextPos = 0;
    size_t commaPos = 0;
    size_t index ;
    std::string key;
    bool validValue = false;
    
    while (startPos != std::string::npos) {
        endPos = replacement.find(")}", startPos);
        if (endPos == std::string::npos) {
            break; // Break if no closing tag is found
        }

        nextPos = startPos + searchString.length();
        json_t* value = jsonDict.get(); // Get the JSON root object
        validValue = true;

        while (nextPos < endPos && validValue) {
            commaPos = replacement.find(',', nextPos);
            if (commaPos == std::string::npos || commaPos > endPos) {
                commaPos = endPos; // Set to endPos if no comma is found or it's beyond endPos
            }

            key = replacement.substr(nextPos, commaPos - nextPos); // Extract the key
            if (json_is_object(value)) {
                value = json_object_get(value, key.c_str()); // Navigate through object
            } else if (json_is_array(value)) {
                index = std::stoul(key); // Convert key to index for arrays
                value = json_array_get(value, index);
            } else {
                validValue = false; // Set validValue to false if value is neither object nor array
            }
            nextPos = commaPos + 1; // Move next position past the comma
        }

        if (validValue && value && json_is_string(value)) {
            replacement.replace(startPos, endPos + 2 - startPos, json_string_value(value)); // Replace text
        }

        startPos = replacement.find(searchString, endPos + 2); // Find next occurrence
    }

    return replacement; // Return the modified string
}


// this will modify `commands`
std::vector<std::vector<std::string>> getSourceReplacement(const std::vector<std::vector<std::string>>& commands,
    const std::string& entry, size_t entryIndex, const std::string& packagePath = "") {
    
    bool inEristaSection = false;
    bool inMarikoSection = false;
    
    std::vector<std::vector<std::string>> modifiedCommands;
    //std::vector<std::string> listData;
    std::string listString, listPath;
    std::string jsonString, jsonPath;
    std::string iniPath;
    size_t startPos, endPos;
    
    std::vector<std::string> modifiedCmd;
    std::string modifiedArg, lastArg, replacement, commandName;

    std::string fileName = (isDirectory(entry) ? getNameFromPath(entry) : dropExtension(getNameFromPath(entry)));
    
    //if (isFileOrDirectory(entry)) {
    // Insert file_name command at the beginning (for handling sourced toggles)
    //}
    bool usingFileSource = false;

    for (const auto& cmd : commands) {
        if (cmd.empty())
            continue;
        
        modifiedCmd.clear();
        
        modifiedCmd.reserve(cmd.size()); // Reserve memory for efficiency

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
                if (commandName == "file_source") {
                    usingFileSource = true;
                }
                else if ((commandName == "list_source") && listString.empty())
                    listString = removeQuotes(cmd[1]);
                else if ((commandName == "list_file_source") && listPath.empty())
                    listPath = preprocessPath(cmd[1], packagePath);
                else if ((commandName == "ini_file_source") && iniPath.empty())
                    iniPath = preprocessPath(cmd[1], packagePath);
                else if ((commandName == "json_source") && jsonString.empty())
                    jsonString = cmd[1];
                else if ((commandName == "json_file_source") && jsonPath.empty())
                    jsonPath = preprocessPath(cmd[1], packagePath);
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
                    modifiedArg = replacePlaceholder(modifiedArg, "{file_name}", fileName);
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{folder_name}") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "{folder_name}", removeQuotes(getParentDirNameFromPath(entry)));
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
                            replacement = NULL_STR;
                            modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                            break;
                        }
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{list_file_source(") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{list_file_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = getEntryFromListFile(listPath, entryIndex);
                        if (replacement.empty()) {
                            replacement = NULL_STR;
                            modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                            break;
                        }
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                    if (modifiedArg == lastArg)
                        break;
                    lastArg = modifiedArg;
                }
                while (modifiedArg.find("{ini_file_source(") != std::string::npos) {
                    modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{ini_file_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        std::string placeholderContent = modifiedArg.substr(startPos + std::string("{ini_file_source(").length(), endPos - startPos - std::string("{ini_file_source(").length());
                        if (placeholderContent == "*") {
                            modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                        }

                        //replacement = getIniSections(iniPath)[entryIndex];
                        replacement = replaceIniPlaceholder(modifiedArg, "ini_file_source", iniPath);
                        if (replacement.empty()) {
                            replacement = NULL_STR;
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
                            replacement = NULL_STR;
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
                            replacement = NULL_STR;
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
        // Add the file_name command at the front if file_source was used
        if (usingFileSource) {
            modifiedCommands.insert(modifiedCommands.begin(), {"file_name", fileName});
        }
    }
    return modifiedCommands;
}


std::string getCurrentTimestamp(const std::string& format) {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), format.c_str());
    return ss.str();
}

// Define the replacePlaceholders function outside of applyPlaceholderReplacement
auto replacePlaceholders = [](std::string& arg, const std::string& placeholder, const std::function<std::string(const std::string&)>& replacer) {
    size_t startPos, endPos;
    std::string lastArg, replacement;

    while ((startPos = arg.find(placeholder)) != std::string::npos) {
        size_t nestedStartPos = startPos;
        while (true) {
            size_t nextStartPos = arg.find(placeholder, nestedStartPos + 1);
            size_t nextEndPos = arg.find(")}", nestedStartPos);
            if (nextStartPos != std::string::npos && nextStartPos < nextEndPos) {
                nestedStartPos = nextStartPos;
            } else {
                endPos = nextEndPos;
                break;
            }
        }

        if (endPos == std::string::npos || endPos <= startPos) break;

        replacement = replacer(arg.substr(startPos, endPos - startPos + 2));
        if (replacement.empty()) {
            replacement = NULL_STR;
        }
        arg.replace(startPos, endPos - startPos + 2, replacement);
        if (arg == lastArg) {
            if (interpreterLogging) {
                logMessage("failed replacement arg: " + arg);
            }
            replacement = NULL_STR;
            arg.replace(startPos, endPos - startPos + 2, replacement);
            break;
        }
        lastArg = arg;
    }
};

void applyPlaceholderReplacement(std::vector<std::string>& cmd, std::string hexPath, std::string iniPath, std::string listString, std::string listPath, std::string jsonString, std::string jsonPath) {
    std::vector<std::pair<std::string, std::function<std::string(const std::string&)>>> placeholders = {
        {"{hex_file(", [&](const std::string& placeholder) { return replaceHexPlaceholder(placeholder, hexPath); }},
        {"{ini_file(", [&](const std::string& placeholder) { return replaceIniPlaceholder(placeholder, INI_FILE_STR, iniPath); }},
        {"{list(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find('(') + 1;
            size_t endPos = placeholder.find(')');
            size_t listIndex = std::stoi(placeholder.substr(startPos, endPos - startPos));
            return stringToList(listString)[listIndex];
        }},
        {"{list_file(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find('(') + 1;
            size_t endPos = placeholder.find(')');
            size_t listIndex = std::stoi(placeholder.substr(startPos, endPos - startPos));
            return getEntryFromListFile(listPath, listIndex);
        }},
        {"{json(", [&](const std::string& placeholder) { return replaceJsonPlaceholder(placeholder, JSON_STR, jsonString); }},
        {"{json_file(", [&](const std::string& placeholder) { return replaceJsonPlaceholder(placeholder, JSON_FILE_STR, jsonPath); }},
        {"{timestamp(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find("(") + 1;
            size_t endPos = placeholder.find(")");
            std::string format = (endPos != std::string::npos) ? placeholder.substr(startPos, endPos - startPos) : "%Y-%m-%d %H:%M:%S";
            return getCurrentTimestamp(removeQuotes(format));
        }},
        {"{decimal_to_hex(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find("(") + 1;
            size_t endPos = placeholder.find(")");
            std::string decimalValue = placeholder.substr(startPos, endPos - startPos);
            return decimalToHex(decimalValue);
        }},
        {"{ascii_to_hex(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find("(") + 1;
            size_t endPos = placeholder.find(")");
            std::string asciiValue = placeholder.substr(startPos, endPos - startPos);
            return asciiToHex(asciiValue);
        }},
        {"{hex_to_rhex(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find("(") + 1;
            size_t endPos = placeholder.find(")");
            std::string hexValue = placeholder.substr(startPos, endPos - startPos);
            return hexToReversedHex(hexValue);
        }},
        {"{hex_to_decimal(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find("(") + 1;
            size_t endPos = placeholder.find(")");
            std::string hexValue = placeholder.substr(startPos, endPos - startPos);
            return hexToDecimal(hexValue);
        }},
        {"{slice(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find('(') + 1;
            size_t endPos = placeholder.find(')');
            std::string parameters = placeholder.substr(startPos, endPos - startPos);
            size_t commaPos = parameters.find(',');

            if (commaPos != std::string::npos) {
                std::string str = parameters.substr(0, commaPos);
                size_t sliceStart = std::stoi(parameters.substr(commaPos + 1, parameters.find(',', commaPos + 1) - (commaPos + 1)));
                size_t sliceEnd = std::stoi(parameters.substr(parameters.find_last_of(',') + 1));
                return sliceString(str, sliceStart, sliceEnd);
            }
            return placeholder;
        }}
    };

    for (auto& arg : cmd) {
        for (const auto& [placeholder, replacer] : placeholders) {
            replacePlaceholders(arg, placeholder, replacer);
        }

        // Failed replacement cleanup
        //if (arg == NULL_STR) arg = UNAVAILABLE_SELECTION;
    }
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

    auto settingsData = getParsedDataFromIniFile(ULTRAHAND_CONFIG_INI_PATH);
    if (settingsData.count(ULTRAHAND_PROJECT_NAME) > 0) {
        auto& ultrahandSection = settingsData[ULTRAHAND_PROJECT_NAME];
        if (settingsData.count(ULTRAHAND_PROJECT_NAME) > 0) {
            // Directly update buffer sizes without a map
            std::string section = "copy_buffer_size";
            if (ultrahandSection.count(section) > 0) {
                COPY_BUFFER_SIZE = std::stoi(ultrahandSection[section]);
            }
            section = "unzip_buffer_size";
            if (ultrahandSection.count(section) > 0) {
                UNZIP_BUFFER_SIZE = std::stoi(ultrahandSection[section]);
            }
            section = "download_buffer_size";
            if (ultrahandSection.count(section) > 0) {
                DOWNLOAD_BUFFER_SIZE = std::stoi(ultrahandSection[section]);
            }
            section = "hex_buffer_size";
            if (ultrahandSection.count(section) > 0) {
                HEX_BUFFER_SIZE = std::stoi(ultrahandSection[section]);
            }
        }
    }
    settingsData.clear();


    bool inEristaSection = false;
    bool inMarikoSection = false;
    bool inTrySection = false;
    std::string listString, listPath, jsonString, jsonPath, hexPath, iniPath, lastArg;

    //size_t startPos, endPos, listIndex;
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
            if (inTrySection && commandSuccess) break;
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

                applyPlaceholderReplacement(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

                if (interpreterLogging) {
                    std::string message = "Executing command: ";
                    for (const std::string& token : cmd)
                        message += token + " ";
                    logMessage(message);
                }

                const size_t cmdSize = cmd.size();

                if (commandName == LIST_STR) {
                    if (cmdSize >= 2) {
                        listString = removeQuotes(cmd[1]);
                    }
                } else if (commandName == LIST_FILE_STR) {
                    if (cmdSize >= 2) {
                        listPath = preprocessPath(cmd[1], packagePath);
                    }
                } else if (commandName == JSON_STR) {
                    if (cmdSize >= 2) {
                        jsonString = cmd[1];
                    }
                } else if (commandName == JSON_FILE_STR) {
                    if (cmdSize >= 2) {
                        jsonPath = preprocessPath(cmd[1], packagePath);
                    }
                } else if (commandName == INI_FILE_STR) {
                    if (cmdSize >= 2) {
                        iniPath = preprocessPath(cmd[1], packagePath);
                    }
                } else if (commandName == HEX_FILE_STR) {
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


// Helper function to parse command arguments
void parseCommandArguments(const std::vector<std::string>& cmd, const std::string& packagePath, std::string& sourceListPath, std::string& destinationListPath, std::string& logSource, std::string& logDestination, std::string& sourcePath, std::string& destinationPath) {
    for (size_t i = 1; i < cmd.size(); ++i) {
        if (cmd[i] == "-src" && i + 1 < cmd.size()) {
            sourceListPath = preprocessPath(cmd[++i], packagePath);
        } else if (cmd[i] == "-dest" && i + 1 < cmd.size()) {
            destinationListPath = preprocessPath(cmd[++i], packagePath);
        } else if (cmd[i] == "-log_src" && i + 1 < cmd.size()) {
            logSource = preprocessPath(cmd[++i], packagePath);
        } else if (cmd[i] == "-log_dest" && i + 1 < cmd.size()) {
            logDestination = preprocessPath(cmd[++i], packagePath);
        } else if (sourcePath.empty()) {
            sourcePath = preprocessPath(cmd[i], packagePath);
        } else if (destinationPath.empty()) {
            destinationPath = preprocessPath(cmd[i], packagePath);
        }
    }
}


// Helper function for commands involving source and destination paths
void handleSourceDestinationPaths(const std::string& sourcePath, const std::string& destinationPath, const std::string& logSource, const std::string& logDestination, std::function<void(const std::string&, const std::string&, const std::string&, const std::string&)> operation) {
    if (sourcePath.empty() || destinationPath.empty()) {
        logMessage("Source and destination paths must be specified.");
    } else if (!isFileOrDirectory(sourcePath)) {
        logMessage("Source file or directory doesn't exist: " + sourcePath);
    } else if (sourcePath.find('*') != std::string::npos) {
        operation(sourcePath, destinationPath, logSource, logDestination); // Operation by pattern
    } else {
        operation(sourcePath, destinationPath, logSource, logDestination); // Single operation
    }
}

// Helper function for commands involving source paths
void handleSourcePath(const std::string& sourcePath, const std::string& logSource, std::function<void(const std::string&, const std::string&)> operation) {
    if (sourcePath.empty()) {
        logMessage("Source path must be specified.");
    } else if (!isDangerousCombination(sourcePath)) {
        if (sourcePath.find('*') != std::string::npos) {
            operation(sourcePath, logSource); // Operation by pattern
        } else {
            operation(sourcePath, logSource); // Single operation
        }
    }
}

void handleMakeDirCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd.size() >= 2) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        createDirectory(sourcePath);
    }
}

void handleCopyCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath);

    if (!sourceListPath.empty() && !destinationListPath.empty()) {
        auto sourceFilesList = readListFromFile(sourceListPath);
        auto destinationFilesList = readListFromFile(destinationListPath);
        for (size_t i = 0; i < sourceFilesList.size(); ++i) {
            sourcePath = preprocessPath(sourceFilesList[i]);
            destinationPath = preprocessPath(destinationFilesList[i]);
            long long totalBytesCopied = 0;
            long long totalSize = getTotalSize(sourcePath);
            copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
        }
    } else {
        handleSourceDestinationPaths(sourcePath, destinationPath, logSource, logDestination, [](const std::string& src, const std::string& dest, const std::string& logSrc, const std::string& logDest) {
            long long totalBytesCopied = 0;
            long long totalSize = getTotalSize(src);
            copyFileOrDirectory(src, dest, &totalBytesCopied, totalSize, logSrc, logDest);
        });
    }
}

void handleDeleteCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    std::string sourceListPath, logSource, sourcePath, destinationListPath, logDestination, destinationPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath);

    if (!sourceListPath.empty()) {
        auto sourceFilesList = readListFromFile(sourceListPath);
        for (const auto& filePath : sourceFilesList) {
            sourcePath = preprocessPath(filePath);
            deleteFileOrDirectory(sourcePath);
        }
    } else {
        handleSourcePath(sourcePath, logSource, [](const std::string& src, const std::string& logSrc) {
            deleteFileOrDirectory(src, logSrc);
        });
    }
}


void handleMirrorCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd.size() >= 2) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string destinationPath = cmd.size() >= 3 ? preprocessPath(cmd[2], packagePath) : ROOT_PATH;
        std::string operation = (cmd[0] == "mirror_copy" || cmd[0] == "mirror_cp") ? "copy" : "delete";

        if (sourcePath.find('*') == std::string::npos) {
            mirrorFiles(sourcePath, destinationPath, operation);
        } else {
            auto fileList = getFilesListByWildcards(sourcePath);
            for (const auto& sourceDirectory : fileList) {
                mirrorFiles(sourceDirectory, destinationPath, operation);
            }
        }
    }
}

void handleMoveCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath);

    if (!sourceListPath.empty() && !destinationListPath.empty()) {
        auto sourceFilesList = readListFromFile(sourceListPath);
        auto destinationFilesList = readListFromFile(destinationListPath);
        if (sourceFilesList.size() != destinationFilesList.size()) {
            logMessage("Source and destination lists must have the same number of entries.");
        } else {
            for (size_t i = 0; i < sourceFilesList.size(); ++i) {
                sourcePath = preprocessPath(sourceFilesList[i]);
                destinationPath = preprocessPath(destinationFilesList[i]);
                moveFileOrDirectory(sourcePath, destinationPath);
            }
        }
    } else {
        handleSourceDestinationPaths(sourcePath, destinationPath, logSource, logDestination, [](const std::string& src, const std::string& dest, const std::string& logSrc, const std::string& logDest) {
            moveFileOrDirectory(src, dest, logSrc, logDest);
        });
    }
}

void handleIniCommands(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd[0] == "add-ini-section" && cmd.size() >= 2) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string desiredSection = removeQuotes(cmd[2]);
        addIniSection(sourcePath.c_str(), desiredSection.c_str());
    } else if (cmd[0] == "rename-ini-section" && cmd.size() >= 3) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string desiredSection = removeQuotes(cmd[2]);
        std::string desiredNewSection = removeQuotes(cmd[3]);
        renameIniSection(sourcePath.c_str(), desiredSection.c_str(), desiredNewSection.c_str());
    } else if (cmd[0] == "remove-ini-section" && cmd.size() >= 2) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string desiredSection = removeQuotes(cmd[2]);
        removeIniSection(sourcePath.c_str(), desiredSection.c_str());
    } else if (cmd[0] == "remove-ini-key" && cmd.size() >= 3) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string desiredSection = removeQuotes(cmd[2]);
        std::string desiredKey = removeQuotes(cmd[3]);
        removeIniKey(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str());
    } else if ((cmd[0] == "set-ini-val" || cmd[0] == "set-ini-value") && cmd.size() >= 5) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string desiredSection = removeQuotes(cmd[2]);
        std::string desiredKey = removeQuotes(cmd[3]);
        std::string desiredValue = std::accumulate(cmd.begin() + 4, cmd.end(), std::string(""), [](const std::string& a, const std::string& b) -> std::string {
            return a.empty() ? b : a + " " + b;
        });
        setIniFileValue(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
    } else if (cmd[0] == "set-ini-key" && cmd.size() >= 5) {
        std::string sourcePath = preprocessPath(cmd[1], packagePath);
        std::string desiredSection = removeQuotes(cmd[2]);
        std::string desiredKey = removeQuotes(cmd[3]);
        std::string desiredNewKey = std::accumulate(cmd.begin() + 4, cmd.end(), std::string(""), [](const std::string& a, const std::string& b) -> std::string {
            return a.empty() ? b : a + " " + b;
        });
        setIniFileKey(sourcePath.c_str(), desiredSection.c_str(), desiredKey.c_str(), desiredNewKey.c_str());
    }
}

void handleHexEdit(const std::string& sourcePath, const std::string& secondArg, const std::string& thirdArg, const std::string& commandName, const std::vector<std::string>& cmd) {
    if (commandName == "hex-by-offset") {
        hexEditByOffset(sourcePath.c_str(), secondArg.c_str(), thirdArg.c_str());
    } else if (commandName == "hex-by-swap") {
        if (cmd.size() >= 5) {
            size_t occurrence = std::stoul(removeQuotes(cmd[4]));
            hexEditFindReplace(sourcePath, secondArg, thirdArg, occurrence);
        } else {
            hexEditFindReplace(sourcePath, secondArg, thirdArg);
        }
    } else if (commandName == "hex-by-string") {
        std::string hexDataToReplace = asciiToHex(secondArg);
        std::string hexDataReplacement = asciiToHex(thirdArg);
        if (hexDataReplacement.length() < hexDataToReplace.length()) {
            hexDataReplacement += std::string(hexDataToReplace.length() - hexDataReplacement.length(), '\0');
        } else if (hexDataReplacement.length() > hexDataToReplace.length()) {
            hexDataToReplace += std::string(hexDataReplacement.length() - hexDataToReplace.length(), '\0');
        }
        if (cmd.size() >= 5) {
            size_t occurrence = std::stoul(removeQuotes(cmd[4]));
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    } else if (commandName == "hex-by-decimal") {
        std::string hexDataToReplace = decimalToHex(secondArg);
        std::string hexDataReplacement = decimalToHex(thirdArg);
        if (cmd.size() >= 5) {
            size_t occurrence = std::stoul(removeQuotes(cmd[4]));
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    } else if (commandName == "hex-by-rdecimal") {
        std::string hexDataToReplace = decimalToReversedHex(secondArg);
        std::string hexDataReplacement = decimalToReversedHex(thirdArg);
        if (cmd.size() >= 5) {
            size_t occurrence = std::stoul(removeQuotes(cmd[4]));
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    }
}

void handleHexByCustom(const std::string& sourcePath, const std::string& customPattern, const std::string& offset, std::string hexDataReplacement, const std::string& commandName) {
    if (hexDataReplacement != NULL_STR) {
        if (commandName == "hex-by-custom-decimal-offset") {
            hexDataReplacement = decimalToHex(hexDataReplacement);
        } else if (commandName == "hex-by-custom-rdecimal-offset") {
            hexDataReplacement = decimalToReversedHex(hexDataReplacement);
        }
        hexEditByCustomOffset(sourcePath.c_str(), customPattern.c_str(), offset.c_str(), hexDataReplacement.c_str());
    }
}


void rebootToHekateConfig(Payload::HekateConfigList& configList, const std::string& option, bool isIni) {
    int rebootIndex = -1;  // Initialize rebootIndex to -1, indicating no match found
    auto configIterator = configList.begin();

    if (std::all_of(option.begin(), option.end(), ::isdigit)) {
        rebootIndex = std::stoi(option);
        std::advance(configIterator, rebootIndex);
    } else {
        for (auto it = configList.begin(); it != configList.end(); ++it) {
            if (it->name == option) {
                rebootIndex = std::distance(configList.begin(), it);
                configIterator = it;  // Update the iterator to the matching element
                break;
            }
        }
    }

    if (rebootIndex != -1) {
        Payload::RebootToHekateConfig(*configIterator, isIni);
    }
}

// Main processCommand function
void processCommand(const std::vector<std::string>& cmd, const std::string& packagePath = "", const std::string& selectedCommand = "") {
    const std::string& commandName = cmd[0];

    if (commandName == "mkdir" || commandName == "make") {
        handleMakeDirCommand(cmd, packagePath);
    } else if (commandName == "cp" || commandName == "copy") {
        handleCopyCommand(cmd, packagePath);
    } else if (commandName == "del" || commandName == "delete") {
        handleDeleteCommand(cmd, packagePath);
    } else if (commandName.substr(0, 7) == "mirror_") {
        handleMirrorCommand(cmd, packagePath);
    } else if (commandName == "mv" || commandName == "move" || commandName == "rename") {
        handleMoveCommand(cmd, packagePath);
    } else if (commandName == "add-ini-section" || commandName == "rename-ini-section" || commandName == "remove-ini-section" || commandName == "remove-ini-key" || commandName == "set-ini-val" || commandName == "set-ini-value" || commandName == "set-ini-key") {
        handleIniCommands(cmd, packagePath);
    } else if (commandName == "set-footer") {
        if (cmd.size() >= 2) {
            std::string desiredValue = removeQuotes(cmd[1]);
            setIniFileValue((packagePath + CONFIG_FILENAME).c_str(), selectedCommand.c_str(), FOOTER_STR, desiredValue.c_str());
        }
    } else if (commandName.substr(0, 7) == "hex-by-") {
        if (cmd.size() >= 4) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            const std::string& secondArg = removeQuotes(cmd[2]);
            const std::string& thirdArg = removeQuotes(cmd[3]);

            if (commandName == "hex-by-custom-offset" || commandName == "hex-by-custom-decimal-offset" || commandName == "hex-by-custom-rdecimal-offset") {
                if (cmd.size() >= 5) {
                    std::string customPattern = removeQuotes(cmd[2]);
                    std::string offset = removeQuotes(cmd[3]);
                    std::string hexDataReplacement = removeQuotes(cmd[4]);
                    handleHexByCustom(sourcePath, customPattern, offset, hexDataReplacement, commandName);
                }
            } else {
                handleHexEdit(sourcePath, secondArg, thirdArg, commandName, cmd);
            }
        }
    } else if (commandName == "download") {
        if (cmd.size() >= 3) {
            std::string fileUrl = preprocessUrl(cmd[1]);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            bool downloadSuccess = false;
            for (size_t i = 0; i < 3; ++i) {
                downloadSuccess = downloadFile(fileUrl, destinationPath);
                if (abortDownload.load(std::memory_order_acquire)) {
                    downloadSuccess = false;
                    break;
                }
                if (downloadSuccess) break;
            }
            commandSuccess = downloadSuccess && commandSuccess;
        }
    } else if (commandName == "unzip") {
        if (cmd.size() >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            commandSuccess = unzipFile(sourcePath, destinationPath) && commandSuccess;
        }
    } else if (commandName == "pchtxt2ips") {
        if (cmd.size() >= 3) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            std::string destinationPath = preprocessPath(cmd[2], packagePath);
            commandSuccess = pchtxt2ips(sourcePath, destinationPath) && commandSuccess;
        }
    } else if (commandName == "pchtxt2cheat") {
        if (cmd.size() >= 2) {
            std::string sourcePath = preprocessPath(cmd[1], packagePath);
            commandSuccess = pchtxt2cheat(sourcePath) && commandSuccess;
        }
    } else if (commandName == "exec") {
        if (cmd.size() >= 2) {
            std::string bootCommandName = removeQuotes(cmd[1]);
            if (isFileOrDirectory(packagePath + BOOT_PACKAGE_FILENAME)) {
                auto bootOptions = loadOptionsFromIni(packagePath + BOOT_PACKAGE_FILENAME, true);
                std::string bootOptionName;
                bool resetCommandSuccess;
                for (auto& bootOption : bootOptions) {
                    bootOptionName = bootOption.first;
                    auto& bootCommands = bootOption.second;
                    if (bootOptionName == bootCommandName) {
                        resetCommandSuccess = false;
                        if (!commandSuccess) resetCommandSuccess = true;
                        interpretAndExecuteCommands(std::move(bootCommands), packagePath, bootOptionName);
                        if (resetCommandSuccess) {
                            commandSuccess = false;
                            resetCommandSuccess = false;
                        }
                        break;
                    }
                }
                bootOptions.clear();
            }
        }
    } else if (commandName == "reboot") { // credits to Studious Pancake for the Payload and utils methods
        if (util::IsErista() || util::SupportsMarikoRebootToConfig()) {
            std::string rebootOption;
            if (cmd.size() >= 2) {
                rebootOption = removeQuotes(cmd[1]);
                if (cmd.size() >= 3) {
                    std::string option = removeQuotes(cmd[2]);
                    if (rebootOption == "boot") {
                        Payload::HekateConfigList bootConfigList = Payload::LoadHekateConfigList();
                        rebootToHekateConfig(bootConfigList, option, false);
                    } else if (rebootOption == "ini") {
                        Payload::HekateConfigList iniConfigList = Payload::LoadIniConfigList();
                        rebootToHekateConfig(iniConfigList, option, true);
                    }
                }
                if (rebootOption == "UMS") {
                    Payload::RebootToHekateUMS(Payload::UmsTarget_Sd);
                } else if (rebootOption == "HEKATE" || rebootOption == "hekate") {
                    Payload::RebootToHekateMenu();
                } else if (isFileOrDirectory(rebootOption)) {
                    std::string fileName = getNameFromPath(rebootOption);
                    if (util::IsErista()) {
                        Payload::PayloadConfig reboot_payload = {fileName, rebootOption};
                        Payload::RebootToPayload(reboot_payload);
                    } else {
                        setIniFileValue("/bootloader/ini/" + fileName + ".ini", fileName, "payload", rebootOption);
                        Payload::HekateConfigList iniConfigList = Payload::LoadIniConfigList();
                        rebootToHekateConfig(iniConfigList, fileName, true);
                    }
                }
            }
            if (rebootOption.empty()) {
                Payload::RebootToHekate();
            }
        }

        i2cExit();
        splExit();
        fsdevUnmountAll();
        spsmShutdown(SpsmShutdownMode_Reboot);
    } else if (commandName == "shutdown") {
        if (cmd.size() >= 2) {
            std::string selection = removeQuotes(cmd[1]);
            if (selection == "controllers") {
                powerOffAllControllers();
            }
        } else {
            splExit();
            fsdevUnmountAll();
            spsmShutdown(SpsmShutdownMode_Normal);
        }
    } else if (commandName == "exit") {
        //triggerExit.store(true, std::memory_order_release);
        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
        tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
        tsl::Overlay::get()->close();
        return;
    } else if (commandName == "backlight") {
        if (cmd.size() >= 2) {
            std::string togglePattern = removeQuotes(cmd[1]);
            lblInitialize();
            if (togglePattern == ON_STR)
                lblSwitchBacklightOn(0);
            else if (togglePattern == OFF_STR)
                lblSwitchBacklightOff(0);
            else if (isValidNumber(togglePattern)) {
                lblSetCurrentBrightnessSetting(std::stof(togglePattern) / 100.0f);
            }
            lblExit();
        }
    } else if (commandName == "refresh") {
        if (cmd.size() == 1)
            refreshGui = true;
        else if (cmd.size() > 1) {
            std::string refreshPattern = removeQuotes(cmd[1]);
            if (refreshPattern == "theme")
                tsl::initializeThemeVars();
        }
    } else if (commandName == "logging") {
        interpreterLogging = !interpreterLogging;
    } else if (commandName == "clear") {
        if (cmd.size() >= 2) {
            std::string clearOption = removeQuotes(cmd[1]);
            if (clearOption == "log") deleteFileOrDirectory(logFilePath);
            else if (clearOption == "hex_sum_cache") hexSumCache.clear();
        }
    }
}





// Thread information structure
Thread interpreterThread;
std::queue<std::tuple<std::vector<std::vector<std::string>>, std::string, std::string>> interpreterQueue;
std::mutex queueMutex;
std::condition_variable queueCondition;
std::atomic<bool> interpreterThreadExit{false};


inline void clearInterpreterFlags(bool state = false) {
    abortDownload.store(state, std::memory_order_release);
    abortUnzip.store(state, std::memory_order_release);
    abortFileOp.store(state, std::memory_order_release);
    abortCommand.store(state, std::memory_order_release);
}

inline void resetPercentages() {
    downloadPercentage.store(-1, std::memory_order_release);
    unzipPercentage.store(-1, std::memory_order_release);
    copyPercentage.store(-1, std::memory_order_release);
}


void backgroundInterpreter(void*) {
    std::tuple<std::vector<std::vector<std::string>>, std::string, std::string> args;
    while (!interpreterThreadExit.load(std::memory_order_acquire)) {
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [] { return !interpreterQueue.empty() || interpreterThreadExit.load(std::memory_order_acquire); });
            if (interpreterThreadExit.load(std::memory_order_acquire)) {
                logMessage("Exiting Thread...");
                break;
            }
            if (!interpreterQueue.empty()) {
                args = std::move(interpreterQueue.front());
                interpreterQueue.pop();
            }
            //svcSleepThread(10'000'000);
        } // Release the lock before processing the command

        if (!std::get<0>(args).empty()) {
            //logMessage("Start of interpreter");
            // Clear flags and perform any cleanup if necessary
            clearInterpreterFlags();
            resetPercentages();
            threadFailure.store(false, std::memory_order_release);
            
            runningInterpreter.store(true, std::memory_order_release);
            interpretAndExecuteCommands(std::move(std::get<0>(args)), std::move(std::get<1>(args)), std::move(std::get<2>(args)));

            // Clear flags and perform any cleanup if necessary
            clearInterpreterFlags();
            resetPercentages();

            runningInterpreter.store(false, std::memory_order_release);
            interpreterThreadExit.store(true, std::memory_order_release);
            //logMessage("End of interpreter");
            //break;
        }
        //logMessage("looping...");
    }
}

void closeInterpreterThread() {
   {
       std::lock_guard<std::mutex> lock(queueMutex);
       interpreterThreadExit.store(true, std::memory_order_release);
       queueCondition.notify_one();
   }
   threadWaitForExit(&interpreterThread);
   threadClose(&interpreterThread);
   // Reset flags
   clearInterpreterFlags();
}



void startInterpreterThread(int stackSize = 0x8000) {

    std::string interpreterHeap = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "interpreter_heap");
    if (!interpreterHeap.empty())
        stackSize = std::stoi(interpreterHeap, nullptr, 16);  // Convert from base 16

    interpreterThreadExit.store(false, std::memory_order_release);

    int result = threadCreate(&interpreterThread, backgroundInterpreter, nullptr, nullptr, stackSize, 0x2B, -2);
    if (result != 0) {
        commandSuccess = false;
        clearInterpreterFlags();
        runningInterpreter.store(false, std::memory_order_release);
        interpreterThreadExit.store(true, std::memory_order_release);
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
    queueCondition.notify_one();
}

//void playClickVibration() {
//    // Initialize HID services
//    if (R_FAILED(hidInitialize())) {
//        logMessage("Failed to initialize HID services");
//        return;
//    }
//
//    // Example vibration pattern for a quick click feedback
//    HidVibrationValue vibrationValue = {
//        .amp_low = 0.5,
//        .freq_low = 160.0,
//        .amp_high = 0.5,
//        .freq_high = 320.0
//    };
//
//    // Use the correct controller ID
//    HidVibrationDeviceHandle vibrationDevice;
//    Result rc = hidGetVibrationDeviceInfo(&vibrationDevice, CONTROLLER_P1_AUTO);
//    if (R_FAILED(rc)) {
//        logMessage("Failed to get vibration device info");
//        hidExit();
//        return;
//    }
//
//    rc = hidSendVibrationValues(&vibrationDevice, &vibrationValue, 1);
//    if (R_FAILED(rc)) {
//        logMessage("Failed to send vibration values");
//    }
//
//    hidExit();
//}
//


//bool load_wav(const std::string &file_path, int &sample_rate, int &num_channels, std::vector<uint8_t> &audio_data) {
//    std::ifstream file(file_path, std::ios::binary);
//    if (!file) {
//        logMessage("Could not open WAV file: " + file_path);
//        return false;
//    }
//
//    // Read the WAV header
//    char buffer[44];
//    file.read(buffer, 44);
//
//    // Parse WAV header (simplified)
//    sample_rate = *reinterpret_cast<int*>(buffer + 24);
//    num_channels = *reinterpret_cast<short*>(buffer + 22);
//
//    // Check if the format is PCM
//    if (buffer[20] != 1 || buffer[21] != 0) {
//        logMessage("Unsupported WAV format");
//        return false;
//    }
//
//    // Read the audio data
//    file.seekg(0, std::ios::end);
//    size_t file_size = file.tellg();
//    file.seekg(44, std::ios::beg);
//    size_t data_size = file_size - 44;
//
//    audio_data.resize(data_size);
//    file.read(reinterpret_cast<char*>(audio_data.data()), data_size);
//
//    return true;
//}
//
//Result try_open_audio_out(const char* device_name, u32 sample_rate, u32 num_channels, u32& sample_rate_out, u32& channel_count_out, PcmFormat& format, AudioOutState& state) {
//    Result rc = audoutOpenAudioOut(device_name, nullptr, sample_rate, num_channels, &sample_rate_out, &channel_count_out, &format, &state);
//    if (R_FAILED(rc)) {
//        logMessage("Failed to open audio out with result code: " + std::to_string(rc));
//    }
//    return rc;
//}
//
//int play_audio(const std::string &file_path) {
//    int sample_rate, num_channels;
//    std::vector<uint8_t> audio_data;
//
//    if (!load_wav(file_path, sample_rate, num_channels, audio_data)) {
//        logMessage("Failed to load WAV file");
//        return 1;
//    }
//
//    logMessage("WAV file loaded successfully");
//    logMessage("Sample rate: " + std::to_string(sample_rate));
//    logMessage("Number of channels: " + std::to_string(num_channels));
//
//    // Initialize the audio output service
//    Result rc = audoutInitialize();
//    if (R_FAILED(rc)) {
//        logMessage("Failed to initialize audio output, result code: " + std::to_string(rc));
//        return 1;
//    }
//
//    // List audio outputs to get the device name
//    char device_names[0x100 * 8] = {0};  // Allow space for up to 8 device names
//    u32 device_names_count = 0;
//    rc = audoutListAudioOuts(device_names, 8, &device_names_count);
//
//    if (R_FAILED(rc) || device_names_count == 0) {
//        logMessage("Failed to list audio outputs or no outputs available, result code: " + std::to_string(rc));
//        audoutExit();
//        return 1;
//    }
//
//    logMessage("Audio outputs listed successfully, count: " + std::to_string(device_names_count));
//    logMessage("Device name: " + std::string(device_names));
//
//    // Align buffer size to 0x1000 bytes
//    size_t aligned_buffer_size = (audio_data.size() + 0xFFF) & ~0xFFF;
//    std::vector<uint8_t> aligned_audio_data(aligned_buffer_size);
//    memcpy(aligned_audio_data.data(), audio_data.data(), audio_data.size());
//
//    AudioOutBuffer source = {};
//    source.next = nullptr;
//    source.buffer = aligned_audio_data.data();
//    source.buffer_size = aligned_buffer_size;
//    source.data_size = audio_data.size();
//    source.data_offset = 0;
//
//    u32 sample_rate_out;
//    u32 channel_count_out;
//    AudioOutState state;
//
//    // Try different PCM formats
//    PcmFormat formats[] = {PcmFormat_Int16, PcmFormat_Int32};
//    bool success = false;
//    for (PcmFormat format : formats) {
//        logMessage("Trying format: " + std::to_string(format));
//        rc = try_open_audio_out(device_names, sample_rate, num_channels, sample_rate_out, channel_count_out, format, state);
//        if (R_SUCCEEDED(rc)) {
//            success = true;
//            break;
//        }
//    }
//
//    if (!success) {
//        logMessage("Failed to open audio out with any supported format.");
//        audoutExit();
//        return 1;
//    }
//
//    logMessage("Audio out opened successfully");
//
//    rc = audoutStartAudioOut();
//    if (R_FAILED(rc)) {
//        logMessage("Failed to start audio out, result code: " + std::to_string(rc));
//        audoutExit();
//        return 1;
//    }
//
//    rc = audoutAppendAudioOutBuffer(&source);
//    if (R_FAILED(rc)) {
//        logMessage("Failed to play audio buffer, result code: " + std::to_string(rc));
//        audoutStopAudioOut();
//        audoutExit();
//        return 1;
//    }
//
//    AudioOutBuffer* released_buffer = nullptr;
//    u32 released_count;
//
//    rc = audoutWaitPlayFinish(&released_buffer, &released_count, UINT64_MAX);
//    if (R_FAILED(rc)) {
//        logMessage("Failed to wait for audio playback, result code: " + std::to_string(rc));
//    }
//
//    audoutStopAudioOut();
//    audoutExit();
//
//    logMessage("Audio playback completed successfully");
//
//    return 0;
//}//
