#pragma once

#include <string>
#include <vector>
#include <atomic>

namespace ult {
    extern const std::string CONFIG_FILENAME;

    extern const std::string ULTRAHAND_PROJECT_NAME;
    extern const std::string CAPITAL_ULTRAHAND_PROJECT_NAME;
    
    extern const std::string ROOT_PATH;
    extern const std::string SETTINGS_PATH;
    extern const std::string ULTRAHAND_CONFIG_INI_PATH;
    extern const std::string TESLA_CONFIG_INI_PATH;
    extern const std::string LANG_PATH;
    extern const std::string THEMES_PATH;
    extern const std::string WALLPAPERS_PATH;
    
    // Can be overriden with APPEARANCE_OVERRIDE_PATH directive
    extern std::string THEME_CONFIG_INI_PATH;
    extern std::string WALLPAPER_PATH;


    #if IS_LAUNCHER_DIRECTIVE
    extern const std::string SPLIT_PROJECT_NAME_1;
    extern const std::string SPLIT_PROJECT_NAME_2;
    extern const std::string BOOT_PACKAGE_FILENAME;
    extern const std::string EXIT_PACKAGE_FILENAME;
    extern const std::string PACKAGE_FILENAME;
    
    extern const std::string DOWNLOADS_PATH;
    extern const std::string EXPANSION_PATH;
    extern const std::string FUSE_DATA_INI_PATH;
    extern const std::string PACKAGE_PATH;
    extern const std::string OVERLAY_PATH;
    extern const std::string OVERLAYS_INI_FILEPATH;
    extern const std::string PACKAGES_INI_FILEPATH;
    extern const std::string ULTRAHAND_REPO_URL;
    extern const std::string INCLUDED_THEME_FOLDER_URL;
    extern const std::string LATEST_RELEASE_INFO_URL;
    extern const std::string NX_OVLLOADER_ZIP_URL;
    extern const std::string NX_OVLLOADER_PLUS_ZIP_URL;
    
    extern const std::string LAUNCH_ARGS_STR;
    extern const std::string USE_LAUNCH_ARGS_STR;
    extern const std::string USE_BOOT_PACKAGE_STR;
    extern const std::string USE_EXIT_PACKAGE_STR;
    extern const std::string USE_LOGGING_STR;

    #endif

    extern const std::string TESLA_COMBO_STR;
    extern const std::string ULTRAHAND_COMBO_STR;
    
    extern const std::string FUSE_STR;
    extern const std::string TESLA_STR;
    extern const std::string ERISTA_STR;
    extern const std::string MARIKO_STR;
    extern const std::string KEY_COMBO_STR;
    extern const std::string DEFAULT_LANG_STR;


    extern const std::string LIST_STR;
    extern const std::string LIST_FILE_STR;
    extern const std::string JSON_STR;
    extern const std::string JSON_FILE_STR;
    extern const std::string INI_FILE_STR;
    extern const std::string HEX_FILE_STR;
    extern const std::string PACKAGE_STR;
    extern const std::string PACKAGES_STR;
    extern const std::string OVERLAY_STR;
    extern const std::string OVERLAYS_STR;
    extern const std::string IN_OVERLAY_STR;
    extern const std::string IN_HIDDEN_OVERLAY_STR;
    extern const std::string FILE_STR;
    extern const std::string SYSTEM_STR;
    extern const std::string MODE_STR;
    extern const std::string GROUPING_STR;
    extern const std::string FOOTER_STR;
    extern const std::string TOGGLE_STR;
    extern const std::string LEFT_STR;
    extern const std::string RIGHT_STR;
    extern const std::string CENTER_STR;
    extern const std::string HIDE_STR;
    extern const std::string STAR_STR;
    extern const std::string PRIORITY_STR;
    extern const std::string ON_STR;
    extern const std::string OFF_STR;
    extern const std::string CAPITAL_ON_STR;
    extern const std::string CAPITAL_OFF_STR;
    extern const std::string TRUE_STR;
    extern const std::string FALSE_STR;
    extern const std::string GLOBAL_STR;
    extern const std::string DEFAULT_STR;
    extern const std::string SLOT_STR;
    extern const std::string OPTION_STR;
    extern const std::string FORWARDER_STR;
    extern const std::string TEXT_STR;
    extern const std::string TABLE_STR;
    extern const std::string TRACKBAR_STR;
    extern const std::string STEP_TRACKBAR_STR;
    extern const std::string NAMED_STEP_TRACKBAR_STR;
    extern const std::string NULL_STR;
    extern const std::string THEME_STR;
    extern const std::string NOT_AVAILABLE_STR;
    extern const std::string BUFFERS;

    // Pre-defined symbols
    extern const std::string OPTION_SYMBOL;
    extern const std::string DROPDOWN_SYMBOL;
    extern const std::string CHECKMARK_SYMBOL;
    extern const std::string CROSSMARK_SYMBOL;
    extern const std::string DOWNLOAD_SYMBOL;
    extern const std::string UNZIP_SYMBOL; 
    extern const std::string COPY_SYMBOL;
    extern const std::string INPROGRESS_SYMBOL;
    extern const std::string STAR_SYMBOL;

    extern const std::vector<std::string> THROBBER_SYMBOLS;

    extern std::atomic<int> downloadPercentage;
    extern std::atomic<int> unzipPercentage;
    extern std::atomic<int> copyPercentage;
    
    void resetPercentages();
}
