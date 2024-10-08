/********************************************************************************
 * File: tsl_utils.hpp
 * Author: ppkantorski
 * Description: 
 *   'tsl_utils.hpp' is a central utility header for the Ultrahand Overlay project,
 *   containing a variety of functions and definitions related to system status,
 *   input handling, and application-specific behavior on the Nintendo Switch.
 *   This header provides essential utilities for interacting with the system,
 *   managing key input, and enhancing overlay functionality.
 *
 *   The utilities defined here are designed to operate independently, facilitating
 *   robust system interaction capabilities required for custom overlays.
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


#pragma once
#ifndef TSL_UTILS_HPP
#define TSL_UTILS_HPP

#if NO_FSTREAM_DIRECTIVE // For not using fstream (needs implementing)
#include <stdio.h>
#else
#include <fstream>
#endif

#include <ultra.hpp>
#include <switch.h>
#include <arm_neon.h>

#include <stdlib.h>
#include <strings.h>
#include <math.h>

#include <algorithm>
#include <cstring>
#include <cwctype>
#include <string>
#include <functional>
#include <type_traits>
#include <mutex>
#include <memory>
#include <chrono>
#include <list>
#include <stack>
#include <map>
#include <barrier>



namespace ult {
    extern std::unordered_map<std::string, std::string> translationCache;
    bool loadTranslationsFromJSON(const std::string& filePath);

    extern u16 activeHeaderHeight;

    bool consoleIsDocked();
    
    std::string getTitleIdAsString();
    
    //extern bool isLauncher;
    extern bool internalTouchReleased;
    extern u32 layerEdge;
    extern bool useRightAlignment;
    extern bool useSwipeToOpen;
    extern bool noClickableItems;


    
    // Define the duration boundaries (for smooth scrolling)
    extern const std::chrono::milliseconds initialInterval;  // Example initial interval
    extern const std::chrono::milliseconds shortInterval;    // Short interval after long hold
    extern const std::chrono::milliseconds transitionPoint; // Point at which the shortest interval is reached
    
    // Function to interpolate between two durations
    std::chrono::milliseconds interpolateDuration(std::chrono::milliseconds start, std::chrono::milliseconds end, float t);
    
    
    
    //#include <filesystem> // Comment out filesystem
    
    // CUSTOM SECTION START
    extern float backWidth, selectWidth, nextPageWidth;
    extern bool inMainMenu;
    extern bool inOverlaysPage;
    extern bool inPackagesPage;
    
    extern bool firstBoot; // for detecting first boot
    
    //static std::unordered_map<std::string, std::string> hexSumCache;
    
    // Define an atomic bool for interpreter completion
    extern std::atomic<bool> threadFailure;
    extern std::atomic<bool> runningInterpreter;
    extern std::atomic<bool> shakingProgress;
    
    extern std::atomic<bool> isHidden;
    
    //bool progressAnimation = false;
    extern bool disableTransparency;
    //bool useCustomWallpaper = false;
    extern bool useMemoryExpansion;
    extern bool useOpaqueScreenshots;
    
    extern bool onTrackBar;
    extern bool allowSlide;
    extern bool unlockedSlide;
    
    /**
     * @brief Shutdown modes for the Ultrahand-Overlay project.
     *
     * These macros define the shutdown modes used in the Ultrahand-Overlay project:
     * - `SpsmShutdownMode_Normal`: Normal shutdown mode.
     * - `SpsmShutdownMode_Reboot`: Reboot mode.
     */
    #define SpsmShutdownMode_Normal 0
    #define SpsmShutdownMode_Reboot 1
    
    /**
     * @brief Key mapping macros for button keys.
     *
     * These macros define button keys for the Ultrahand-Overlay project to simplify key mappings.
     * For example, `KEY_A` represents the `HidNpadButton_A` key.
     */
    #define KEY_A HidNpadButton_A
    #define KEY_B HidNpadButton_B
    #define KEY_X HidNpadButton_X
    #define KEY_Y HidNpadButton_Y
    #define KEY_L HidNpadButton_L
    #define KEY_R HidNpadButton_R
    #define KEY_ZL HidNpadButton_ZL
    #define KEY_ZR HidNpadButton_ZR
    #define KEY_PLUS HidNpadButton_Plus
    #define KEY_MINUS HidNpadButton_Minus
    #define KEY_DUP HidNpadButton_Up
    #define KEY_DDOWN HidNpadButton_Down
    #define KEY_DLEFT HidNpadButton_Left
    #define KEY_DRIGHT HidNpadButton_Right
    #define KEY_SL HidNpadButton_AnySL
    #define KEY_SR HidNpadButton_AnySR
    #define KEY_LSTICK HidNpadButton_StickL
    #define KEY_RSTICK HidNpadButton_StickR
    #define KEY_UP HidNpadButton_AnyUp
    #define KEY_DOWN HidNpadButton_AnyDown
    #define KEY_LEFT HidNpadButton_AnyLeft
    #define KEY_RIGHT HidNpadButton_AnyRight
    
    #define SCRIPT_KEY HidNpadButton_Minus
    #define SYSTEM_SETTINGS_KEY HidNpadButton_Plus
    #define SETTINGS_KEY HidNpadButton_Y
    #define STAR_KEY HidNpadButton_X

    
    // Define a mask with all possible key flags
    #define ALL_KEYS_MASK (KEY_A | KEY_B | KEY_X | KEY_Y | KEY_DUP | KEY_DDOWN | KEY_DLEFT | KEY_DRIGHT | KEY_L | KEY_R | KEY_ZL | KEY_ZR | KEY_SL | KEY_SR | KEY_LSTICK | KEY_RSTICK | KEY_PLUS | KEY_MINUS)
    
    
    extern bool updateMenuCombos;
    
    /**
     * @brief Ultrahand-Overlay Input Macros
     *
     * This block of code defines macros for handling input in the Ultrahand-Overlay project.
     * These macros simplify the mapping of input events to corresponding button keys and
     * provide aliases for touch and joystick positions.
     *
     * The macros included in this block are:
     *
     * - `touchPosition`: An alias for a constant `HidTouchState` pointer.
     * - `touchInput`: An alias for `&touchPos`, representing touch input.
     * - `JoystickPosition`: An alias for `HidAnalogStickState`, representing joystick input.
     *
     * These macros are utilized within the Ultrahand-Overlay project to manage and interpret
     * user input, including touch and joystick events.
     */
    #define touchPosition const HidTouchState
    #define touchInput &touchPos
    #define JoystickPosition HidAnalogStickState
    
    //void convertComboToUnicode(std::string& combo);

    /**
     * @brief Combo key mapping
     */
    struct KeyInfo {
        u64 key;
        const char* name;
        const char* glyph;
    };

    /**
     * @brief Combo key mappings
     *
     * Ordered as they should be displayed
     */
    extern std::array<KeyInfo, 18> KEYS_INFO;

    std::unordered_map<std::string, std::string> createButtonCharMap();
    
    extern std::unordered_map<std::string, std::string> buttonCharMap;
    
    
    void convertComboToUnicode(std::string& combo);
    
    
    // English string definitions
    
    extern const std::string whiteColor;
    extern const std::string blackColor;
    
    constexpr float _M_PI = 3.14159265358979323846;
    constexpr float RAD_TO_DEG = 180.0f / _M_PI;
    
    #if IS_LAUNCHER_DIRECTIVE
    extern std::string ENGLISH;
    extern std::string SPANISH;
    extern std::string FRENCH;
    extern std::string GERMAN;
    extern std::string JAPANESE;
    extern std::string KOREAN;
    extern std::string ITALIAN;
    extern std::string DUTCH;
    extern std::string PORTUGUESE;
    extern std::string RUSSIAN;
    extern std::string POLISH;
    extern std::string SIMPLIFIED_CHINESE;
    extern std::string TRADITIONAL_CHINESE;

    extern std::string OVERLAYS; //defined in libTesla now
    extern std::string OVERLAY;
    extern std::string HIDDEN_OVERLAYS;
    extern std::string PACKAGES; //defined in libTesla now
    extern std::string PACKAGE;
    extern std::string HIDDEN_PACKAGES;
    extern std::string HIDDEN;
    extern std::string HIDE_OVERLAY;
    extern std::string HIDE_PACKAGE;
    extern std::string LAUNCH_ARGUMENTS;
    extern std::string BOOT_COMMANDS;
    extern std::string EXIT_COMMANDS;
    extern std::string ERROR_LOGGING;
    extern std::string COMMANDS;
    extern std::string SETTINGS;
    extern std::string MAIN_SETTINGS;
    extern std::string UI_SETTINGS;

    extern std::string WIDGET;
    extern std::string CLOCK;
    extern std::string BATTERY;
    extern std::string SOC_TEMPERATURE;
    extern std::string PCB_TEMPERATURE;
    extern std::string MISCELLANEOUS;
    extern std::string MENU_ITEMS;
    extern std::string USER_GUIDE;
    extern std::string VERSION_LABELS;
    extern std::string KEY_COMBO;
    extern std::string LANGUAGE;
    extern std::string OVERLAY_INFO;
    extern std::string SOFTWARE_UPDATE;
    extern std::string UPDATE_ULTRAHAND;
    extern std::string UPDATE_LANGUAGES;
    extern std::string SYSTEM;
    extern std::string DEVICE_INFO;
    extern std::string FIRMWARE;
    extern std::string BOOTLOADER;
    extern std::string HARDWARE;
    extern std::string MEMORY;
    extern std::string VENDOR;
    extern std::string MODEL;
    extern std::string STORAGE;
    extern std::string NOTICE;
    extern std::string UTILIZES;
    extern std::string FREE;
    extern std::string MEMORY_EXPANSION;
    extern std::string REBOOT_REQUIRED;
    extern std::string LOCAL_IP;
    extern std::string WALLPAPER;
    extern std::string THEME;
    extern std::string DEFAULT;
    extern std::string ROOT_PACKAGE;
    extern std::string SORT_PRIORITY;
    extern std::string FAILED_TO_OPEN;
    extern std::string CLEAN_VERSIONS;
    extern std::string OVERLAY_VERSIONS;
    extern std::string PACKAGE_VERSIONS;
    extern std::string OPAQUE_SCREENSHOTS;

    extern std::string PACKAGE_INFO;
    extern std::string _TITLE;
    extern std::string _VERSION;
    extern std::string _CREATOR;
    extern std::string _ABOUT;
    extern std::string _CREDITS;

    extern std::string USERGUIDE_OFFSET;
    extern std::string SETTINGS_MENU;
    extern std::string SCRIPT_OVERLAY;
    extern std::string STAR_FAVORITE;
    extern std::string APP_SETTINGS;
    extern std::string ON_MAIN_MENU;
    extern std::string ON_A_COMMAND;
    extern std::string ON_OVERLAY_PACKAGE;
    extern std::string EFFECTS;
    extern std::string SWIPE_TO_OPEN;
    extern std::string RIGHT_SIDE_MODE;
    extern std::string PROGRESS_ANIMATION;

    extern std::string REBOOT_TO;
    extern std::string REBOOT;
    extern std::string SHUTDOWN;
    extern std::string BOOT_ENTRY;
    #endif

    extern std::string DEFAULT_CHAR_WIDTH;
    extern std::string UNAVAILABLE_SELECTION;

    extern std::string ON;
    extern std::string OFF;

    extern std::string OK;
    extern std::string BACK;

    extern std::string GAP_1;
    extern std::string GAP_2;
    

    extern std::string EMPTY;
    
    #if USING_WIDGET_DIRECTIVE
    extern std::string SUNDAY;
    extern std::string MONDAY;
    extern std::string TUESDAY;
    extern std::string WEDNESDAY;
    extern std::string THURSDAY;
    extern std::string FRIDAY;
    extern std::string SATURDAY;
    
    extern std::string JANUARY;
    extern std::string FEBRUARY;
    extern std::string MARCH;
    extern std::string APRIL;
    extern std::string MAY;
    extern std::string JUNE;
    extern std::string JULY;
    extern std::string AUGUST;
    extern std::string SEPTEMBER;
    extern std::string OCTOBER;
    extern std::string NOVEMBER;
    extern std::string DECEMBER;
    
    extern std::string SUN;
    extern std::string MON;
    extern std::string TUE;
    extern std::string WED;
    extern std::string THU;
    extern std::string FRI;
    extern std::string SAT;
    
    extern std::string JAN;
    extern std::string FEB;
    extern std::string MAR;
    extern std::string APR;
    extern std::string MAY_ABBR;
    extern std::string JUN;
    extern std::string JUL;
    extern std::string AUG;
    extern std::string SEP;
    extern std::string OCT;
    extern std::string NOV;
    extern std::string DEC;
    #endif
    
    #if IS_LAUNCHER_DIRECTIVE
    // Constant string definitions (English)
    void reinitializeLangVars();
    #endif
    
    
    // Define the updateIfNotEmpty function
    void updateIfNotEmpty(std::string& constant, const char* jsonKey, const json_t* jsonData);
    
    void parseLanguage(const std::string langFile);
    
    #if USING_WIDGET_DIRECTIVE
    void localizeTimeStr(char* timeStr);
    #endif

    // Unified function to apply replacements
    void applyLangReplacements(std::string& text, bool isValue = false);
    
    
    
    //// Map of character widths (pre-calibrated)
    //extern std::unordered_map<wchar_t, float> characterWidths;
    
    //extern float defaultNumericCharWidth;
    
    
    
    // Predefined hexMap
    extern const std::array<int, 256> hexMap;
    
    
    // Prepare a map of default settings
    extern std::map<std::string, std::string> defaultThemeSettingsMap;
    
    bool isNumericCharacter(char c);
    
    bool isValidHexColor(const std::string& hexColor);
    
    
    
    float calculateAmplitude(float x, float peakDurationFactor = 0.25f);
            
    
    
    extern std::atomic<bool> refreshWallpaper;
    extern std::vector<u8> wallpaperData;
    extern std::atomic<bool> inPlot;
    
    extern std::mutex wallpaperMutex;
    extern std::condition_variable cv;
    
    
    
    // Function to load the RGBA file into memory and modify wallpaperData directly
    void loadWallpaperFile(const std::string& filePath, s32 width = 448, s32 height = 720);
    void reloadWallpaper();
    
    // Global variables for FPS calculation
    //double lastTimeCount = 0.0;
    //int frameCount = 0;
    //float fps = 0.0f;
    //double elapsedTime = 0.0;
    
    
    extern bool themeIsInitialized;

    // Variables for touch commands
    extern bool touchingBack;
    extern bool touchingSelect;
    extern bool touchingNextPage;
    extern bool touchingMenu;
    extern bool simulatedBack;
    extern bool simulatedBackComplete;
    extern bool simulatedSelect;
    extern bool simulatedSelectComplete;
    extern bool simulatedNextPage;
    extern bool simulatedNextPageComplete;
    extern bool simulatedMenu;
    extern bool simulatedMenuComplete;
    extern bool stillTouching;
    extern bool interruptedTouch;
    extern bool touchInBounds;
    
    
    #if USING_WIDGET_DIRECTIVE
    // Battery implementation
    extern bool powerInitialized;
    extern bool powerCacheInitialized;
    extern uint32_t powerCacheCharge;
    extern bool powerCacheIsCharging;
    extern PsmSession powerSession;
    
    // Define variables to store previous battery charge and time
    extern uint32_t prevBatteryCharge;
    extern s64 timeOut;
    
    
    extern uint32_t batteryCharge;
    extern bool isCharging;
    
    constexpr std::chrono::seconds min_delay = std::chrono::seconds(3); // Minimum delay between checks
    
    bool powerGetDetails(uint32_t *batteryCharge, bool *isCharging);
    
    void powerInit(void);
    
    void powerExit(void);
    #endif
    
    // Temperature Implementation
    extern float PCB_temperature, SOC_temperature;
    
    /*
    I2cReadRegHandler was taken from Switch-OC-Suite source code made by KazushiMe
    Original repository link (Deleted, last checked 15.04.2023): https://github.com/KazushiMe/Switch-OC-Suite
    */
    
    Result I2cReadRegHandler(u8 reg, I2cDevice dev, u16 *out);
    
    
    #define TMP451_SOC_TEMP_REG 0x01  // Register for SOC temperature integer part
    #define TMP451_SOC_TMP_DEC_REG 0x10  // Register for SOC temperature decimal part
    #define TMP451_PCB_TEMP_REG 0x00  // Register for PCB temperature integer part
    #define TMP451_PCB_TMP_DEC_REG 0x15  // Register for PCB temperature decimal part
    
    // Common helper function to read temperature (integer and fractional parts)
    Result ReadTemperature(float *temperature, u8 integerReg, u8 fractionalReg, bool integerOnly);
    
    // Function to get the SOC temperature
    Result ReadSocTemperature(float *temperature, bool integerOnly = true);
    
    // Function to get the PCB temperature
    Result ReadPcbTemperature(float *temperature, bool integerOnly = true);
    
    
    
    // Time implementation
    
    extern const std::string DEFAULT_DT_FORMAT;
    extern std::string datetimeFormat;
    
    
    // Widget settings
    //static std::string hideClock, hideBattery, hidePCBTemp, hideSOCTemp;
    extern bool hideClock, hideBattery, hidePCBTemp, hideSOCTemp;
    
    void reinitializeWidgetVars();
    
    extern bool cleanVersionLabels, hideOverlayVersions, hidePackageVersions;
    
    extern std::string loaderInfo;
    extern std::string loaderTitle;
    extern bool expandedMemory;
    
    extern std::string versionLabel;
    
    #if IS_LAUNCHER_DIRECTIVE
    void reinitializeVersionLabels();
    #endif
    
    
    // Number of renderer threads to use
    extern const unsigned numThreads;
    extern std::vector<std::thread> threads;
    extern s32 bmpChunkSize;
    extern std::atomic<s32> currentRow;
    
    
    
    inline std::barrier inPlotBarrier(numThreads, [](){
        inPlot.store(false, std::memory_order_release);
    });
    

    void initializeThemeVars();
    
    void initializeUltrahandSettings();


}

#endif