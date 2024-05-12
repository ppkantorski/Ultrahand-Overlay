/********************************************************************************
 * File: ultra.hpp
 * Author: ppkantorski
 * Description: 
 *   'ultra.hpp' serves as a central include header for the Ultrahand Overlay project,
 *   bringing together a comprehensive suite of utility functions essential for the
 *   development and operation of custom overlays on the Nintendo Switch. This header
 *   provides consolidated access to functions facilitating debugging, string processing,
 *   file management, JSON manipulation, and more, enhancing the modularity and 
 *   reusability of code within the project.
 *
 *   These utilities are designed to operate independently, providing robust tools to
 *   support complex overlay functionalities and interactions.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository:
 *   GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay
 *
 *   Note: This notice is integral to the project's documentation and must not be 
 *   altered or removed.
 *
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/

#ifndef ULTRA_HPP
#define ULTRA_HPP
#include <string>

const std::string bootPackageFileName = "boot_package.ini";
const std::string packageFileName = "package.ini";
const std::string configFileName = "config.ini";
const std::string themeFileName = "theme.ini";
const std::string projectName = "ultrahand";
const std::string upperProjectName = "Ultrahand";
const std::string rootPath = "sdmc:/";
const std::string settingsPath = "sdmc:/config/ultrahand/";
const std::string settingsConfigIniPath = "sdmc:/config/ultrahand/config.ini";
const std::string langPath = "sdmc:/config/ultrahand/lang/";
const std::string themeConfigIniPath = "sdmc:/config/ultrahand/theme.ini";
const std::string themesPath = "sdmc:/config/ultrahand/themes/";
const std::string downloadsPath = "sdmc:/config/ultrahand/downloads/";
const std::string packageDirectory = "sdmc:/switch/.packages/";
const std::string overlayDirectory = "sdmc:/switch/.overlays/";
const std::string teslaSettingsConfigIniPath = "sdmc:/config/tesla/config.ini";
const std::string overlaysIniFilePath = "sdmc:/config/ultrahand/overlays.ini";
const std::string packagesIniFilePath = "sdmc:/config/ultrahand/packages.ini";
const std::string ultrahandRepo = "https://github.com/ppkantorski/Ultrahand-Overlay/";

const std::string teslaStr = "tesla";
const std::string eristaStr = "erista";
const std::string marikoStr = "mariko";
const std::string keyComboStr = "key_combo";
const std::string defaultLangStr = "default_lang";
const std::string launchArgsStr = "launch_args";
const std::string useLaunchArgsStr = "use_launch_args";
const std::string _listStr = "list";
const std::string _jsonStr = "json";
const std::string jsonFileStr = "json_file";
const std::string packageStr = "package";
const std::string packagesStr = "packages";
const std::string overlayStr = "overlay";
const std::string overlaysStr = "overlays";
const std::string inOverlayStr = "in_overlay";
const std::string inHiddenOverlayStr = "in_hidden_overlay";
const std::string fileStr = "file";
const std::string systemStr = "system";
const std::string modeStr = "mode";
const std::string groupingStr = "grouping";
const std::string footerStr = "footer";
const std::string toggleStr = "toggle";
const std::string leftStr = "left";
const std::string rightStr = "right";
const std::string hideStr = "hide";
const std::string starStr = "star";
const std::string priorityStr = "priority";
const std::string lowerOnStr = "on";
const std::string lowerOffStr = "off";
const std::string onStr = "On";
const std::string offStr = "Off";
const std::string trueStr = "true";
const std::string falseStr = "false";
const std::string globalStr = "global";
const std::string defaultStr = "default";
const std::string optionStr = "option";
const std::string forwarderStr = "forwarder";
const std::string nullStr = "null";
const std::string themeSection = "theme";
const std::string englishNotAvailable = "Not available";

// Include all functional headers used in the libUltra library
#include "debug_funcs.hpp"
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include "path_funcs.hpp"
#include "list_funcs.hpp"
#include "json_funcs.hpp"
#include "ini_funcs.hpp"
#include "hex_funcs.hpp"
#include "download_funcs.hpp"
#include "mod_funcs.hpp"

#endif // ULTRA_HPP
