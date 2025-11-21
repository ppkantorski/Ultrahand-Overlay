/********************************************************************************
 * File: tsl_utils.cpp
 * Author: ppkantorski
 * Description: 
 *   'tsl_utils.cpp' provides the implementation of various utility functions
 *   defined in 'tsl_utils.hpp' for the Ultrahand Overlay project. This source file
 *   includes functionality for system checks, input handling, time-based interpolation,
 *   and other application-specific features essential for operating custom overlays
 *   on the Nintendo Switch.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository:
 *   GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay
 *
 *   Note: This notice is integral to the project's documentation and must not be 
 *   altered or removed.
 *
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2023-2025 ppkantorski
 ********************************************************************************/

#include <tsl_utils.hpp>

#include <cstdlib>
extern "C" { // assertion override
    void __assert_func(const char *_file, int _line, const char *_func, const char *_expr ) {
        abort();
    }
}

namespace ult {

    bool correctFrameSize; // for detecting the correct Overlay display size

    u16 DefaultFramebufferWidth = 448;            ///< Width of the framebuffer
    u16 DefaultFramebufferHeight = 720;           ///< Height of the framebuffer

    std::unordered_map<std::string, std::string> translationCache;
    
    std::unordered_map<u64, OverlayCombo> g_entryCombos;
    std::atomic<bool> launchingOverlay(false);
    std::atomic<bool> settingsInitialized(false);
    std::atomic<bool> currentForeground(false);
    //std::mutex simulatedNextPageMutex;

    // Helper function to read file content into a string
    bool readFileContent(const std::string& filePath, std::string& content) {
        #if !USING_FSTREAM_DIRECTIVE
            FILE* file = fopen(filePath.c_str(), "r");
            if (!file) {
                #if USING_LOGGING_DIRECTIVE
                logMessage("Failed to open JSON file: " + filePath);
                #endif
                return false;
            }
            char buffer[256];
            while (fgets(buffer, sizeof(buffer), file) != nullptr) {
                content += buffer;
            }
            fclose(file);
        #else
            std::ifstream file(filePath);
            if (!file.is_open()) {
                #if USING_LOGGING_DIRECTIVE
                logMessage("Failed to open JSON file: " + filePath);
                #endif
                return false;
            }
            content.assign(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
            file.close();
        #endif
    
        return true;
    }
    
    // Helper function to parse JSON-like content into a map
    void parseJsonContent(const std::string& content, std::unordered_map<std::string, std::string>& result) {
        size_t pos = 0;
        size_t keyStart, keyEnd, colonPos, valueStart, valueEnd;
        std::string key, value;
    
        auto normalizeNewlines = [](std::string &s) {
            size_t n = 0;
            while ((n = s.find("\\n", n)) != std::string::npos) {
                s.replace(n, 2, "\n");
                n += 1;
            }
        };
    
        while ((pos = content.find('"', pos)) != std::string::npos) {
            keyStart = pos + 1;
            keyEnd = content.find('"', keyStart);
            if (keyEnd == std::string::npos) break;
    
            key = content.substr(keyStart, keyEnd - keyStart);
            colonPos = content.find(':', keyEnd);
            if (colonPos == std::string::npos) break;
    
            valueStart = content.find('"', colonPos);
            valueEnd = content.find('"', valueStart + 1);
            if (valueStart == std::string::npos || valueEnd == std::string::npos) break;
    
            value = content.substr(valueStart + 1, valueEnd - valueStart - 1);
    
            // 🔹 Convert escaped newlines (\\n) into real ones
            normalizeNewlines(key);
            normalizeNewlines(value);
    
            result[key] = value;
    
            key.clear();
            value.clear();
            pos = valueEnd + 1; // Move to next pair
        }
    }
    
    // Function to parse JSON key-value pairs into a map
    bool parseJsonToMap(const std::string& filePath, std::unordered_map<std::string, std::string>& result) {
        std::string content;
        if (!readFileContent(filePath, content)) {
            return false;
        }
        
        parseJsonContent(content, result);
        return true;
    }
    
    // Function to load translations from a JSON-like file into the translation cache
    bool loadTranslationsFromJSON(const std::string& filePath) {
        return parseJsonToMap(filePath, translationCache);
    }
    
    
    u16 activeHeaderHeight = 97;

    bool consoleIsDocked() {
        Result rc = apmInitialize();
        if (R_FAILED(rc)) return false;
        
        ApmPerformanceMode perfMode = ApmPerformanceMode_Invalid;
        rc = apmGetPerformanceMode(&perfMode);
        apmExit();
        
        return R_SUCCEEDED(rc) && (perfMode == ApmPerformanceMode_Boost);
    }
    
    //static bool pminfoInitialized = false;
    //static u64 lastPid = 0;
    //static u64 lastTid = 0;
    //
    //std::string getTitleIdAsString() {
    //    Result rc;
    //    u64 pid = 0;
    //    u64 tid = 0;
    //
    //    // Get the current application PID
    //    rc = pmdmntGetApplicationProcessId(&pid);
    //    if (R_FAILED(rc) || pid == 0) {
    //        return NULL_STR;
    //    }
    //
    //    // If it's the same PID as last time, return cached TID
    //    if (pid == lastPid && lastTid != 0) {
    //        char cachedTidStr[17];
    //        snprintf(cachedTidStr, sizeof(cachedTidStr), "%016lX", lastTid);
    //        return std::string(cachedTidStr);
    //    }
    //
    //    // Initialize pminfo if not already
    //    if (!pminfoInitialized) {
    //        rc = pminfoInitialize();
    //        if (R_FAILED(rc)) {
    //            return NULL_STR;
    //        }
    //        pminfoInitialized = true;
    //    }
    //
    //    // Retrieve the TID (Program ID)
    //    rc = pminfoGetProgramId(&tid, pid);
    //    if (R_FAILED(rc)) {
    //        return NULL_STR;
    //    }
    //
    //    lastPid = pid;
    //    lastTid = tid;
    //
    //    char titleIdStr[17];
    //    snprintf(titleIdStr, sizeof(titleIdStr), "%016lX", tid);
    //    return std::string(titleIdStr);
    //}
    
    //std::string getProcessIdAsString() {
    //    u64 pid = 0;
    //    if (R_FAILED(pmdmntGetApplicationProcessId(&pid)))
    //        return NULL_STR;
    //    
    //    char pidStr[21];  // Max u64 is 20 digits + null terminator
    //    snprintf(pidStr, sizeof(pidStr), "%llu", pid);
    //    return std::string(pidStr);
    //}

    std::string getBuildIdAsString() {
        u64 pid = 0;
        if (R_FAILED(pmdmntGetApplicationProcessId(&pid)))
            return NULL_STR;
        
        Service srv;
        if (R_FAILED(smGetService(&srv, "dmnt:cht")))
            return NULL_STR;
        
        if (R_FAILED(serviceDispatch(&srv, 65003))) {
            serviceClose(&srv);
            return NULL_STR;
        }
        
        struct {
            u64 process_id;
            u64 title_id;
            struct { u64 base; u64 size; } main_nso_extents;
            struct { u64 base; u64 size; } heap_extents;
            struct { u64 base; u64 size; } alias_extents;
            struct { u64 base; u64 size; } address_space_extents;
            u8 main_nso_build_id[0x20];
        } metadata;
        
        Result rc = serviceDispatchOut(&srv, 65002, metadata);
        serviceClose(&srv);
        
        if (R_FAILED(rc))
            return NULL_STR;
        
        u64 buildid;
        std::memcpy(&buildid, metadata.main_nso_build_id, sizeof(u64));
        
        char buildIdStr[17];
        snprintf(buildIdStr, sizeof(buildIdStr), "%016lX", __builtin_bswap64(buildid));
        return std::string(buildIdStr);
    }
    

    std::string getTitleIdAsString() {
        u64 pid = 0, tid = 0;
        if (R_FAILED(pmdmntGetApplicationProcessId(&pid)) ||
            R_FAILED(pmdmntGetProgramId(&tid, pid)))
            return NULL_STR;
    
        char tidStr[17];
        snprintf(tidStr, sizeof(tidStr), "%016lX", tid);
        return std::string(tidStr);
    }

    
    std::string lastTitleID;
    std::atomic<bool> resetForegroundCheck(false); // initialize as true


    

    std::atomic<bool> internalTouchReleased(true);
    u32 layerEdge = 0;
    bool useRightAlignment = false;
    bool useSwipeToOpen = true;
    bool useLaunchCombos = true;
    bool useNotifications = true;
    bool useSoundEffects = true;
    bool useHapticFeedback = false;
    bool usePageSwap = false;
    bool useDynamicLogo = true;
    bool useSelectionBG = true;
    bool useSelectionText = true;
    bool useSelectionValue = false;

    std::atomic<bool> noClickableItems{false};
    
    #if IS_LAUNCHER_DIRECTIVE
    std::atomic<bool> overlayLaunchRequested{false};
    std::string requestedOverlayPath;
    std::string requestedOverlayArgs;
    std::mutex overlayLaunchMutex;
    #endif

    // Define the duration boundaries (for smooth scrolling)
    //const std::chrono::milliseconds initialInterval = std::chrono::milliseconds(67);  // Example initial interval
    //const std::chrono::milliseconds shortInterval = std::chrono::milliseconds(10);    // Short interval after long hold
    //const std::chrono::milliseconds transitionPoint = std::chrono::milliseconds(2000); // Point at which the shortest interval is reached
    
    // Function to interpolate between two durations
    //std::chrono::milliseconds interpolateDuration(std::chrono::milliseconds start, std::chrono::milliseconds end, float t) {
    //    using namespace std::chrono;
    //    auto interpolated = start.count() + static_cast<long long>((end.count() - start.count()) * t);
    //    return milliseconds(interpolated);
    //}
    
    
    
    //#include <filesystem> // Comment out filesystem
    
    // CUSTOM SECTION START
    //float backWidth, selectWidth, nextPageWidth;
    std::atomic<float> backWidth;
    std::atomic<float> selectWidth;
    std::atomic<float> nextPageWidth;
    std::atomic<bool> inMainMenu{false};
    std::atomic<bool> inHiddenMode{false};
    std::atomic<bool> inSettingsMenu{false};
    std::atomic<bool> inSubSettingsMenu{false};
    std::atomic<bool> inOverlaysPage{false};
    std::atomic<bool> inPackagesPage{false};
    
    bool firstBoot = true; // for detecting first boot
    
    //std::unordered_map<std::string, std::string> hexSumCache;
    
    // Define an atomic bool for interpreter completion
    std::atomic<bool> threadFailure(false);
    std::atomic<bool> runningInterpreter(false);
    std::atomic<bool> shakingProgress(true);
    
    std::atomic<bool> isHidden(false);
    std::atomic<bool> externalAbortCommands(false);
    
    //bool progressAnimation = false;
    bool disableTransparency = false;
    //bool useCustomWallpaper = false;
    //bool useMemoryExpansion = false;
    bool useOpaqueScreenshots = false;
    
    std::atomic<bool> onTrackBar(false);
    std::atomic<bool> allowSlide(false);
    std::atomic<bool> unlockedSlide(false);
    
    

    void atomicToggle(std::atomic<bool>& b) {
        bool expected = b.load(std::memory_order_relaxed);
        for (;;) {
            const bool desired = !expected;
            if (b.compare_exchange_weak(expected, desired,
                                        std::memory_order_acq_rel,
                                        std::memory_order_relaxed)) {
                break; // success
            }
            // expected has been updated with the current value on failure; loop continues
        }
    }

    
    bool updateMenuCombos = false;
    
    
    //void convertComboToUnicode(std::string& combo);


    std::array<KeyInfo, 18> KEYS_INFO = {{
        { HidNpadButton_L, "L", "\uE0E4" }, { HidNpadButton_R, "R", "\uE0E5" },
        { HidNpadButton_ZL, "ZL", "\uE0E6" }, { HidNpadButton_ZR, "ZR", "\uE0E7" },
        { HidNpadButton_AnySL, "SL", "\uE0E8" }, { HidNpadButton_AnySR, "SR", "\uE0E9" },
        { HidNpadButton_Left, "DLEFT", "\uE0ED" }, { HidNpadButton_Up, "DUP", "\uE0EB" },
        { HidNpadButton_Right, "DRIGHT", "\uE0EE" }, { HidNpadButton_Down, "DDOWN", "\uE0EC" },
        { HidNpadButton_A, "A", "\uE0E0" }, { HidNpadButton_B, "B", "\uE0E1" },
        { HidNpadButton_X, "X", "\uE0E2" }, { HidNpadButton_Y, "Y", "\uE0E3" },
        { HidNpadButton_StickL, "LS", "\uE08A" }, { HidNpadButton_StickR, "RS", "\uE08B" },
        { HidNpadButton_Minus, "MINUS", "\uE0B6" }, { HidNpadButton_Plus, "PLUS", "\uE0B5" }
    }};

    std::unordered_map<std::string, std::string> createButtonCharMap() {
        std::unordered_map<std::string, std::string> map;
        for (const auto& keyInfo : KEYS_INFO) {
            map[keyInfo.name] = keyInfo.glyph;
        }
        return map;
    }
    
    std::unordered_map<std::string, std::string> buttonCharMap = createButtonCharMap();
    
    
    void convertComboToUnicode(std::string& combo) {
        // Quick check to see if the string contains a '+'
        if (combo.find('+') == std::string::npos) {
            return;  // No '+' found, nothing to modify
        }

        // Exit early if the combo contains any spaces
        if (combo.find(' ') != std::string::npos) {
            return;  // Spaces found, return without modifying
        }
    
        std::string unicodeCombo;
        bool modified = false;
        size_t start = 0;
        const size_t length = combo.length();
        size_t end = 0;  // Moved outside the loop
        std::string token;  // Moved outside the loop
        auto it = buttonCharMap.end();  // Initialize iterator once outside the loop
    
        // Iterate through the combo string and split by '+'
        for (size_t i = 0; i <= length; ++i) {
            if (i == length || combo[i] == '+') {
                // Get the current token (trimmed)
                end = i;  // Reuse the end variable
                while (start < end && std::isspace(combo[start])) start++;  // Trim leading spaces
                while (end > start && std::isspace(combo[end - 1])) end--;  // Trim trailing spaces
    
                token = combo.substr(start, end - start);  // Reuse the token variable
                it = buttonCharMap.find(token);  // Reuse the iterator
    
                if (it != buttonCharMap.end()) {
                    unicodeCombo += it->second;  // Append the mapped Unicode value
                    modified = true;
                } else {
                    unicodeCombo += token;  // Append the original token if not found
                }
    
                if (i != length) {
                    unicodeCombo += "+";  // Only append '+' if we're not at the end
                }
    
                start = i + 1;  // Move to the next token
            }
        }
    
        // If a modification was made, update the original combo
        if (modified) {
            combo = unicodeCombo;
        }
    }
    
    
    CONSTEXPR_STRING std::string whiteColor = "FFFFFF";
    CONSTEXPR_STRING std::string blackColor = "000000";
    CONSTEXPR_STRING std::string greyColor = "AAAAAA";
    
    std::atomic<bool> languageWasChanged{false};

    #if IS_LAUNCHER_DIRECTIVE
    std::string ENGLISH = "English";
    std::string SPANISH = "Spanish";
    std::string FRENCH = "French";
    std::string GERMAN = "German";
    std::string JAPANESE = "Japanese";
    std::string KOREAN = "Korean";
    std::string ITALIAN = "Italian";
    std::string DUTCH = "Dutch";
    std::string PORTUGUESE = "Portuguese";
    std::string RUSSIAN = "Russian";
    std::string UKRAINIAN = "Ukrainian";
    std::string POLISH = "Polish";
    std::string SIMPLIFIED_CHINESE = "Simplified Chinese";
    std::string TRADITIONAL_CHINESE = "Traditional Chinese";
    std::string OVERLAYS = "Overlays"; //defined in libTesla now
    std::string OVERLAYS_ABBR = "Overlays";
    std::string OVERLAY = "Overlay";
    std::string HIDDEN_OVERLAYS = "Hidden Overlays";
    std::string PACKAGES = "Packages"; //defined in libTesla now
    std::string PACKAGE = "Package";
    std::string HIDDEN_PACKAGES = "Hidden Packages";
    std::string HIDDEN = "Hidden";
    std::string HIDE_OVERLAY = "Hide Overlay";
    std::string HIDE_PACKAGE = "Hide Package";
    std::string LAUNCH_ARGUMENTS = "Launch Arguments";
    std::string FORCE_LNY2_SUPPORT = "Force LNY2 Support";
    std::string QUICK_LAUNCH = "Quick Launch";
    std::string BOOT_COMMANDS = "Boot Commands";
    std::string EXIT_COMMANDS = "Exit Commands";
    std::string ERROR_LOGGING = "Error Logging";
    std::string COMMANDS = "Commands";
    std::string SETTINGS = "Settings";
    std::string FAVORITE = "Favorite";
    std::string MAIN_SETTINGS = "Main Settings";
    std::string UI_SETTINGS = "UI Settings";

    std::string WIDGET = "Widget";
    std::string WIDGET_ITEMS = "Widget Items";
    std::string WIDGET_SETTINGS = "Widget Settings";
    std::string CLOCK = "Clock";
    std::string BATTERY = "Battery";
    std::string SOC_TEMPERATURE = "SOC Temperature";
    std::string PCB_TEMPERATURE = "PCB Temperature";
    std::string BACKDROP = "Backdrop";
    std::string DYNAMIC_COLORS = "Dynamic Colors";
    std::string CENTER_ALIGNMENT = "Center Alignment";
    std::string EXTENDED_BACKDROP = "Extended Backdrop";
    std::string MISCELLANEOUS = "Miscellaneous";
    //std::string MENU_ITEMS = "Menu Items";
    std::string MENU_SETTINGS = "Menu Settings";
    std::string USER_GUIDE = "User Guide";
    std::string SHOW_HIDDEN = "Show Hidden";
    std::string SHOW_DELETE = "Show Delete";
    std::string SHOW_UNSUPPORTED = "Show Unsupported";
    std::string PAGE_SWAP = "Page Swap";
    std::string RIGHT_SIDE_MODE = "Right-side Mode";
    std::string OVERLAY_VERSIONS = "Overlay Versions";
    std::string PACKAGE_VERSIONS = "Package Versions";
    std::string CLEAN_VERSIONS = "Clean Versions";
    //std::string VERSION_LABELS = "Version Labels";
    std::string KEY_COMBO = "Key Combo";
    std::string MODE = "Mode";
    std::string LAUNCH_MODES = "Launch Modes";
    std::string LANGUAGE = "Language";
    std::string OVERLAY_INFO = "Overlay Info";
    std::string SOFTWARE_UPDATE = "Software Update";
    std::string UPDATE_ULTRAHAND = "Update Ultrahand";
    std::string UPDATE_LANGUAGES = "Update Languages";
    std::string SYSTEM = "System";
    std::string DEVICE_INFO = "Device Info";
    std::string FIRMWARE = "Firmware";
    std::string BOOTLOADER = "Bootloader";
    std::string HARDWARE = "Hardware";
    std::string MEMORY = "Memory";
    std::string VENDOR = "Vendor";
    std::string MODEL = "Model";
    std::string STORAGE = "Storage";
    std::string NOTICE = "Notice";
    std::string UTILIZES = "Utilizes";

    std::string MEMORY_EXPANSION = "Memory Expansion";
    std::string REBOOT_REQUIRED = "*Reboot required.";
    std::string LOCAL_IP = "Local IP";
    std::string WALLPAPER = "Wallpaper";
    std::string THEME = "Theme";
    std::string DEFAULT = "default";
    std::string ROOT_PACKAGE = "Root Package";
    std::string SORT_PRIORITY = "Sort Priority";
    std::string OPTIONS = "Options";
    std::string FAILED_TO_OPEN = "Failed to open file";
    std::string LAUNCH_COMBOS = "Launch Combos";
    std::string NOTIFICATIONS = "Notifications";
    std::string SOUND_EFFECTS = "Sound Effects";
    std::string HAPTIC_FEEDBACK = "Haptic Feedback";
    std::string OPAQUE_SCREENSHOTS = "Opaque Screenshots";

    std::string PACKAGE_INFO = "Package Info";
    std::string _TITLE = "Title";
    std::string _VERSION= "Version";
    std::string _CREATOR = "Creator(s)";
    std::string _ABOUT = "About";
    std::string _CREDITS = "Credits";

    std::string USERGUIDE_OFFSET = "177";
    std::string SETTINGS_MENU = "Settings Menu";
    std::string SCRIPT_OVERLAY = "Script Overlay";
    std::string STAR_FAVORITE = "Star/Favorite";
    std::string APP_SETTINGS = "App Settings";
    std::string ON_MAIN_MENU = "on Main Menu";
    std::string ON_A_COMMAND = "on a command";
    std::string ON_OVERLAY_PACKAGE = "on overlay/package";
    std::string FEATURES = "Features";
    std::string SWIPE_TO_OPEN = "Swipe to Open";

    std::string THEME_SETTINGS = "Theme Settings";
    std::string DYNAMIC_LOGO = "Dynamic Logo";
    std::string SELECTION_BACKGROUND = "Selection Background";
    std::string SELECTION_TEXT = "Selection Text";
    std::string SELECTION_VALUE = "Selection Value";
    std::string LIBULTRAHAND_TITLES = "libultrahand Titles";
    std::string LIBULTRAHAND_VERSIONS = "libultrahand Versions";
    std::string PACKAGE_TITLES = "Package Titles";

    std::string ULTRAHAND_HAS_STARTED = "Ultrahand has started.";
    std::string NEW_UPDATE_IS_AVAILABLE = "New update is available!";
    //std::string REBOOT_IS_REQUIRED = "Reboot is required.";
    //std::string HOLD_A_TO_DELETE = "Hold \uE0E0 to Delete";
    std::string SELECTION_IS_EMPTY = "Selection is empty!";
    std::string FORCED_SUPPORT_WARNING = "Forcing support can be dangerous.";


    //std::string PACKAGE_VERSIONS = "Package Versions";

    //std::string PROGRESS_ANIMATION = "Progress Animation";

    std::string REBOOT_TO = "Reboot To";
    std::string REBOOT = "Reboot";
    std::string SHUTDOWN = "Shutdown";
    std::string BOOT_ENTRY = "Boot Entry";
    #endif

    std::string FREE = "free";

    std::string DEFAULT_CHAR_WIDTH = "0.33";
    std::string UNAVAILABLE_SELECTION = "Not available";


    std::string ON = "On";
    std::string OFF = "Off";

    std::string OK = "OK";
    std::string BACK = "Back";
    std::string HIDE = "Hide";
    std::string CANCEL = "Cancel";

    std::string GAP_1 = "     ";
    std::string GAP_2 = "  ";

    std::atomic<float> halfGap = 0.0f;
    

    //std::string EMPTY = "Empty";
    
    #if USING_WIDGET_DIRECTIVE
    std::string SUNDAY = "Sunday";
    std::string MONDAY = "Monday";
    std::string TUESDAY = "Tuesday";
    std::string WEDNESDAY = "Wednesday";
    std::string THURSDAY = "Thursday";
    std::string FRIDAY = "Friday";
    std::string SATURDAY = "Saturday";
    
    std::string JANUARY = "January";
    std::string FEBRUARY = "February";
    std::string MARCH = "March";
    std::string APRIL = "April";
    std::string MAY = "May";
    std::string JUNE = "June";
    std::string JULY = "July";
    std::string AUGUST = "August";
    std::string SEPTEMBER = "September";
    std::string OCTOBER = "October";
    std::string NOVEMBER = "November";
    std::string DECEMBER = "December";
    
    std::string SUN = "Sun";
    std::string MON = "Mon";
    std::string TUE = "Tue";
    std::string WED = "Wed";
    std::string THU = "Thu";
    std::string FRI = "Fri";
    std::string SAT = "Sat";
    
    std::string JAN = "Jan";
    std::string FEB = "Feb";
    std::string MAR = "Mar";
    std::string APR = "Apr";
    std::string MAY_ABBR = "May";
    std::string JUN = "Jun";
    std::string JUL = "Jul";
    std::string AUG = "Aug";
    std::string SEP = "Sep";
    std::string OCT = "Oct";
    std::string NOV = "Nov";
    std::string DEC = "Dec";
    #endif

    
    #if IS_LAUNCHER_DIRECTIVE
    // Constant string definitions (English)
    void reinitializeLangVars() {
        ENGLISH = "English";
        SPANISH = "Spanish";
        FRENCH = "French";
        GERMAN = "German";
        JAPANESE = "Japanese";
        KOREAN = "Korean";
        ITALIAN = "Italian";
        DUTCH = "Dutch";
        PORTUGUESE = "Portuguese";
        RUSSIAN = "Russian";
        UKRAINIAN = "Ukrainian";
        POLISH = "Polish";
        SIMPLIFIED_CHINESE = "Simplified Chinese";
        TRADITIONAL_CHINESE = "Traditional Chinese";
        DEFAULT_CHAR_WIDTH = "0.33";
        UNAVAILABLE_SELECTION = "Not available";
        OVERLAYS = "Overlays"; //defined in libTesla now
        OVERLAYS_ABBR = "Overlays";
        OVERLAY = "Overlay";
        HIDDEN_OVERLAYS = "Hidden Overlays";
        PACKAGES = "Packages"; //defined in libTesla now
        PACKAGE = "Package";
        HIDDEN_PACKAGES = "Hidden Packages";
        HIDDEN = "Hidden";
        HIDE_OVERLAY = "Hide Overlay";
        HIDE_PACKAGE = "Hide Package";
        LAUNCH_ARGUMENTS = "Launch Arguments";
        FORCE_LNY2_SUPPORT = "Force LNY2 Support";
        QUICK_LAUNCH = "Quick Launch";
        BOOT_COMMANDS = "Boot Commands";
        EXIT_COMMANDS = "Exit Commands";
        ERROR_LOGGING = "Error Logging";
        COMMANDS = "Commands";
        SETTINGS = "Settings";
        FAVORITE = "Favorite";
        MAIN_SETTINGS = "Main Settings";
        UI_SETTINGS = "UI Settings";
        WIDGET = "Widget";
        WIDGET_ITEMS = "Widget Items";
        WIDGET_SETTINGS = "Widget Settings";
        CLOCK = "Clock";
        BATTERY = "Battery";
        SOC_TEMPERATURE = "SOC Temperature";
        PCB_TEMPERATURE = "PCB Temperature";
        BACKDROP = "Backdrop";
        DYNAMIC_COLORS = "Dynamic Colors";
        CENTER_ALIGNMENT = "Center Alignment";
        EXTENDED_BACKDROP = "Extended Backdrop";
        MISCELLANEOUS = "Miscellaneous";
        //MENU_ITEMS = "Menu Items";
        MENU_SETTINGS = "Menu Settings";
        USER_GUIDE = "User Guide";
        SHOW_HIDDEN = "Show Hidden";
        SHOW_DELETE = "Show Delete";
        SHOW_UNSUPPORTED = "Show Unsupported";
        PAGE_SWAP = "Page Swap";
        RIGHT_SIDE_MODE = "Right-side Mode";
        OVERLAY_VERSIONS = "Overlay Versions";
        PACKAGE_VERSIONS = "Package Versions";
        CLEAN_VERSIONS = "Clean Versions";
        //VERSION_LABELS = "Version Labels";
        KEY_COMBO = "Key Combo";
        MODE = "Mode";
        LAUNCH_MODES = "Launch Modes";
        LANGUAGE = "Language";
        OVERLAY_INFO = "Overlay Info";
        SOFTWARE_UPDATE = "Software Update";
        UPDATE_ULTRAHAND = "Update Ultrahand";
        UPDATE_LANGUAGES = "Update Languages";
        SYSTEM = "System";
        DEVICE_INFO = "Device Info";
        FIRMWARE = "Firmware";
        BOOTLOADER = "Bootloader";
        HARDWARE = "Hardware";
        MEMORY = "Memory";
        VENDOR = "Vendor";
        MODEL = "Model";
        STORAGE = "Storage";
        NOTICE = "Notice";
        UTILIZES = "Utilizes";
        FREE = "free";
        MEMORY_EXPANSION = "Memory Expansion";
        REBOOT_REQUIRED = "*Reboot required.";
        LOCAL_IP = "Local IP";
        WALLPAPER = "Wallpaper";
        THEME = "Theme";
        DEFAULT = "default";
        ROOT_PACKAGE = "Root Package";
        SORT_PRIORITY = "Sort Priority";
        OPTIONS = "Options";
        FAILED_TO_OPEN = "Failed to open file";

        LAUNCH_COMBOS = "Launch Combos";
        NOTIFICATIONS = "Notifications";
        SOUND_EFFECTS = "Sound Effects";
        HAPTIC_FEEDBACK = "Haptic Feedback";
        OPAQUE_SCREENSHOTS = "Opaque Screenshots";
        ON = "On";
        OFF = "Off";
        PACKAGE_INFO = "Package Info";
        _TITLE = "Title";
        _VERSION= "Version";
        _CREATOR = "Creator(s)";
        _ABOUT = "About";
        _CREDITS = "Credits";
        OK = "OK";
        BACK = "Back";
        HIDE = "Hide";
        CANCEL = "Cancel";

        REBOOT_TO = "Reboot To";
        REBOOT = "Reboot";
        SHUTDOWN = "Shutdown";
        BOOT_ENTRY = "Boot Entry";
        GAP_1 = "     ";
        GAP_2 = "  ";

        USERGUIDE_OFFSET = "177";
        SETTINGS_MENU = "Settings Menu";
        SCRIPT_OVERLAY = "Script Overlay";
        STAR_FAVORITE = "Star/Favorite";
        APP_SETTINGS = "App Settings";
        ON_MAIN_MENU = "on Main Menu";
        ON_A_COMMAND = "on a command";
        ON_OVERLAY_PACKAGE = "on overlay/package";
        FEATURES = "Features";
        SWIPE_TO_OPEN = "Swipe to Open";
        //PROGRESS_ANIMATION = "Progress Animation";

        THEME_SETTINGS = "Theme Settings";
        DYNAMIC_LOGO = "Dynamic Logo";
        SELECTION_BACKGROUND = "Selection Background";
        SELECTION_TEXT = "Selection Text";
        SELECTION_VALUE = "Selection Value";
        LIBULTRAHAND_TITLES = "libultrahand Titles";
        LIBULTRAHAND_VERSIONS = "libultrahand Versions";
        PACKAGE_TITLES = "Package Titles";
        //PACKAGE_VERSIONS = "Package Versions";

        ULTRAHAND_HAS_STARTED = "Ultrahand has started.";
        NEW_UPDATE_IS_AVAILABLE = "New update is available!";
        //REBOOT_IS_REQUIRED = "Reboot is required.";
        //HOLD_A_TO_DELETE = "Hold  to Delete";
        SELECTION_IS_EMPTY = "Selection is empty!";
        FORCED_SUPPORT_WARNING = "Forcing support can be dangerous.";

        //EMPTY = "Empty";
    
        SUNDAY = "Sunday";
        MONDAY = "Monday";
        TUESDAY = "Tuesday";
        WEDNESDAY = "Wednesday";
        THURSDAY = "Thursday";
        FRIDAY = "Friday";
        SATURDAY = "Saturday";
        
        JANUARY = "January";
        FEBRUARY = "February";
        MARCH = "March";
        APRIL = "April";
        MAY = "May";
        JUNE = "June";
        JULY = "July";
        AUGUST = "August";
        SEPTEMBER = "September";
        OCTOBER = "October";
        NOVEMBER = "November";
        DECEMBER = "December";
        
        SUN = "Sun";
        MON = "Mon";
        TUE = "Tue";
        WED = "Wed";
        THU = "Thu";
        FRI = "Fri";
        SAT = "Sat";
        
        JAN = "Jan";
        FEB = "Feb";
        MAR = "Mar";
        APR = "Apr";
        MAY_ABBR = "May";
        JUN = "Jun";
        JUL = "Jul";
        AUG = "Aug";
        SEP = "Sep";
        OCT = "Oct";
        NOV = "Nov";
        DEC = "Dec";
    }
    #endif
    
    
    
    // Function to update a constant if the new value from JSON is not empty
    void updateIfNotEmpty(std::string& constant, const std::string& newValue) {
        if (!newValue.empty()) {
            constant = newValue;
        }
    }

    void parseLanguage(const std::string& langFile) {
        // Map to store parsed JSON data
        std::unordered_map<std::string, std::string> jsonMap;
        if (!parseJsonToMap(langFile, jsonMap)) {
            #if USING_LOGGING_DIRECTIVE
            logMessage("Failed to parse language file: " + langFile);
            #endif
            return;
        }

        
        std::unordered_map<std::string, std::string*> configMap = {
            #if IS_LAUNCHER_DIRECTIVE
            {"ENGLISH", &ENGLISH},
            {"SPANISH", &SPANISH},
            {"FRENCH", &FRENCH},
            {"GERMAN", &GERMAN},
            {"JAPANESE", &JAPANESE},
            {"KOREAN", &KOREAN},
            {"ITALIAN", &ITALIAN},
            {"DUTCH", &DUTCH},
            {"PORTUGUESE", &PORTUGUESE},
            {"RUSSIAN", &RUSSIAN},
            {"UKRAINIAN", &UKRAINIAN},
            {"POLISH", &POLISH},
            {"SIMPLIFIED_CHINESE", &SIMPLIFIED_CHINESE},
            {"TRADITIONAL_CHINESE", &TRADITIONAL_CHINESE},
            {"OVERLAYS", &OVERLAYS},
            {"OVERLAYS_ABBR", &OVERLAYS_ABBR},
            {"OVERLAY", &OVERLAY},
            {"HIDDEN_OVERLAYS", &HIDDEN_OVERLAYS},
            {"PACKAGES", &PACKAGES},
            {"PACKAGE", &PACKAGE},
            {"HIDDEN_PACKAGES", &HIDDEN_PACKAGES},
            {"HIDDEN", &HIDDEN},
            {"HIDE_PACKAGE", &HIDE_PACKAGE},
            {"HIDE_OVERLAY", &HIDE_OVERLAY},
            {"LAUNCH_ARGUMENTS", &LAUNCH_ARGUMENTS},
            {"FORCE_LNY2_SUPPORT", &FORCE_LNY2_SUPPORT},
            {"QUICK_LAUNCH", &QUICK_LAUNCH},
            {"BOOT_COMMANDS", &BOOT_COMMANDS},
            {"EXIT_COMMANDS", &EXIT_COMMANDS},
            {"ERROR_LOGGING", &ERROR_LOGGING},
            {"COMMANDS", &COMMANDS},
            {"SETTINGS", &SETTINGS},
            {"FAVORITE", &FAVORITE},
            {"MAIN_SETTINGS", &MAIN_SETTINGS},
            {"UI_SETTINGS", &UI_SETTINGS},

            {"WIDGET", &WIDGET},
            {"WIDGET_ITEMS", &WIDGET_ITEMS},
            {"WIDGET_SETTINGS", &WIDGET_SETTINGS},
            {"CLOCK", &CLOCK},
            {"BATTERY", &BATTERY},
            {"SOC_TEMPERATURE", &SOC_TEMPERATURE},
            {"PCB_TEMPERATURE", &PCB_TEMPERATURE},
            {"BACKDROP", &BACKDROP},
            {"DYNAMIC_COLORS", &DYNAMIC_COLORS},
            {"CENTER_ALIGNMENT", &CENTER_ALIGNMENT},
            {"EXTENDED_BACKDROP", &EXTENDED_BACKDROP},
            {"MISCELLANEOUS", &MISCELLANEOUS},
            //{"MENU_ITEMS", &MENU_ITEMS},
            {"MENU_SETTINGS", &MENU_SETTINGS},
            {"USER_GUIDE", &USER_GUIDE},
            {"SHOW_HIDDEN", &SHOW_HIDDEN},
            {"SHOW_DELETE", &SHOW_DELETE},
            {"SHOW_UNSUPPORTED", &SHOW_UNSUPPORTED},
            {"PAGE_SWAP", &PAGE_SWAP},
            {"RIGHT_SIDE_MODE", &RIGHT_SIDE_MODE},
            {"OVERLAY_VERSIONS", &OVERLAY_VERSIONS},
            {"PACKAGE_VERSIONS", &PACKAGE_VERSIONS},
            {"CLEAN_VERSIONS", &CLEAN_VERSIONS},
            //{"VERSION_LABELS", &VERSION_LABELS},
            {"KEY_COMBO", &KEY_COMBO},
            {"MODE", &MODE},
            {"LAUNCH_MODES", &LAUNCH_MODES},
            {"LANGUAGE", &LANGUAGE},
            {"OVERLAY_INFO", &OVERLAY_INFO},
            {"SOFTWARE_UPDATE", &SOFTWARE_UPDATE},
            {"UPDATE_ULTRAHAND", &UPDATE_ULTRAHAND},
            {"UPDATE_LANGUAGES", &UPDATE_LANGUAGES},
            {"SYSTEM", &SYSTEM},
            {"DEVICE_INFO", &DEVICE_INFO},
            {"FIRMWARE", &FIRMWARE},
            {"BOOTLOADER", &BOOTLOADER},
            {"HARDWARE", &HARDWARE},
            {"MEMORY", &MEMORY},
            {"VENDOR", &VENDOR},
            {"MODEL", &MODEL},
            {"STORAGE", &STORAGE},
            {"NOTICE", &NOTICE},
            {"UTILIZES", &UTILIZES},

            {"MEMORY_EXPANSION", &MEMORY_EXPANSION},
            {"REBOOT_REQUIRED", &REBOOT_REQUIRED},
            {"LOCAL_IP", &LOCAL_IP},
            {"WALLPAPER", &WALLPAPER},
            {"THEME", &THEME},
            {"DEFAULT", &DEFAULT},
            {"ROOT_PACKAGE", &ROOT_PACKAGE},
            {"SORT_PRIORITY", &SORT_PRIORITY},
            {"OPTIONS", &OPTIONS},
            {"FAILED_TO_OPEN", &FAILED_TO_OPEN},

            {"LAUNCH_COMBOS", &LAUNCH_COMBOS},
            {"NOTIFICATIONS", &NOTIFICATIONS},
            {"SOUND_EFFECTS", &SOUND_EFFECTS},
            {"HAPTIC_FEEDBACK", &HAPTIC_FEEDBACK},
            {"OPAQUE_SCREENSHOTS", &OPAQUE_SCREENSHOTS},

            {"PACKAGE_INFO", &PACKAGE_INFO},
            {"TITLE", &_TITLE},
            {"VERSION", &_VERSION},
            {"CREATOR", &_CREATOR},
            {"ABOUT", &_ABOUT},
            {"CREDITS", &_CREDITS},

            {"USERGUIDE_OFFSET", &USERGUIDE_OFFSET},
            {"SETTINGS_MENU", &SETTINGS_MENU},
            {"SCRIPT_OVERLAY", &SCRIPT_OVERLAY},
            {"STAR_FAVORITE", &STAR_FAVORITE},
            {"APP_SETTINGS", &APP_SETTINGS},
            {"ON_MAIN_MENU", &ON_MAIN_MENU},
            {"ON_A_COMMAND", &ON_A_COMMAND},
            {"ON_OVERLAY_PACKAGE", &ON_OVERLAY_PACKAGE},
            {"FEATURES", &FEATURES},
            {"SWIPE_TO_OPEN", &SWIPE_TO_OPEN},

            {"THEME_SETTINGS", &THEME_SETTINGS},
            {"DYNAMIC_LOGO", &DYNAMIC_LOGO},
            {"SELECTION_BACKGROUND", &SELECTION_BACKGROUND},
            {"SELECTION_TEXT", &SELECTION_TEXT},
            {"SELECTION_VALUE", &SELECTION_VALUE},
            {"LIBULTRAHAND_TITLES", &LIBULTRAHAND_TITLES},
            {"LIBULTRAHAND_VERSIONS", &LIBULTRAHAND_VERSIONS},
            {"PACKAGE_TITLES", &PACKAGE_TITLES},

            {"ULTRAHAND_HAS_STARTED", &ULTRAHAND_HAS_STARTED},
            {"NEW_UPDATE_IS_AVAILABLE", &NEW_UPDATE_IS_AVAILABLE},
            //{"REBOOT_IS_REQUIRED", &REBOOT_IS_REQUIRED},
            //{"HOLD_A_TO_DELETE", &HOLD_A_TO_DELETE},
            {"SELECTION_IS_EMPTY", &SELECTION_IS_EMPTY},
            {"FORCED_SUPPORT_WARNING", &FORCED_SUPPORT_WARNING},

            //{"PACKAGE_VERSIONS", &PACKAGE_VERSIONS},
            //{"PROGRESS_ANIMATION", &PROGRESS_ANIMATION},

            {"REBOOT_TO", &REBOOT_TO},
            {"REBOOT", &REBOOT},
            {"SHUTDOWN", &SHUTDOWN},
            {"BOOT_ENTRY", &BOOT_ENTRY},
            #endif

            {"FREE", &FREE},
            
            {"DEFAULT_CHAR_WIDTH", &DEFAULT_CHAR_WIDTH},
            {"UNAVAILABLE_SELECTION", &UNAVAILABLE_SELECTION},

            {"ON", &ON},
            {"OFF", &OFF},

            {"OK", &OK},
            {"BACK", &BACK},
            {"HIDE", &HIDE},
            {"CANCEL", &CANCEL},

            {"GAP_1", &GAP_1},
            {"GAP_2", &GAP_2},

            //{"EMPTY", &EMPTY},

            #if USING_WIDGET_DIRECTIVE
            {"SUNDAY", &SUNDAY},
            {"MONDAY", &MONDAY},
            {"TUESDAY", &TUESDAY},
            {"WEDNESDAY", &WEDNESDAY},
            {"THURSDAY", &THURSDAY},
            {"FRIDAY", &FRIDAY},
            {"SATURDAY", &SATURDAY},
            {"JANUARY", &JANUARY},
            {"FEBRUARY", &FEBRUARY},
            {"MARCH", &MARCH},
            {"APRIL", &APRIL},
            {"MAY", &MAY},
            {"JUNE", &JUNE},
            {"JULY", &JULY},
            {"AUGUST", &AUGUST},
            {"SEPTEMBER", &SEPTEMBER},
            {"OCTOBER", &OCTOBER},
            {"NOVEMBER", &NOVEMBER},
            {"DECEMBER", &DECEMBER},
            {"SUN", &SUN},
            {"MON", &MON},
            {"TUE", &TUE},
            {"WED", &WED},
            {"THU", &THU},
            {"FRI", &FRI},
            {"SAT", &SAT},
            {"JAN", &JAN},
            {"FEB", &FEB},
            {"MAR", &MAR},
            {"APR", &APR},
            {"MAY_ABBR", &MAY_ABBR},
            {"JUN", &JUN},
            {"JUL", &JUL},
            {"AUG", &AUG},
            {"SEP", &SEP},
            {"OCT", &OCT},
            {"NOV", &NOV},
            {"DEC", &DEC}
            #endif
        };
    
        // Iterate over the map to update global variables
        for (auto& kv : configMap) {
            auto it = jsonMap.find(kv.first);
            if (it != jsonMap.end()) {
                updateIfNotEmpty(*kv.second, it->second);
            }
        }
    }
    
    
    // Helper function to apply replacements
    //void applyTimeStrReplacements(std::string& str, const std::unordered_map<std::string, std::string>& mappings) {
    //    size_t pos;
    //    for (const auto& mapping : mappings) {
    //        pos = str.find(mapping.first);
    //        while (pos != std::string::npos) {
    //            str.replace(pos, mapping.first.length(), mapping.second);
    //            pos = str.find(mapping.first, pos + mapping.second.length());
    //        }
    //    }
    //}
    
    #if USING_WIDGET_DIRECTIVE
    void localizeTimeStr(char* timeStr) {
        // Define static unordered_map for day and month mappings
        static std::unordered_map<std::string, std::string*> mappings = {
            {"Sun", &SUN},
            {"Mon", &MON},
            {"Tue", &TUE},
            {"Wed", &WED},
            {"Thu", &THU},
            {"Fri", &FRI},
            {"Sat", &SAT},
            {"Sunday", &SUNDAY},
            {"Monday", &MONDAY},
            {"Tuesday", &TUESDAY},
            {"Wednesday", &WEDNESDAY},
            {"Thursday", &THURSDAY},
            {"Friday", &FRIDAY},
            {"Saturday", &SATURDAY},
            {"Jan", &JAN},
            {"Feb", &FEB},
            {"Mar", &MAR},
            {"Apr", &APR},
            {"May", &MAY_ABBR},
            {"Jun", &JUN},
            {"Jul", &JUL},
            {"Aug", &AUG},
            {"Sep", &SEP},
            {"Oct", &OCT},
            {"Nov", &NOV},
            {"Dec", &DEC},
            {"January", &JANUARY},
            {"February", &FEBRUARY},
            {"March", &MARCH},
            {"April", &APRIL},
            {"May", &MAY},
            {"June", &JUNE},
            {"July", &JULY},
            {"August", &AUGUST},
            {"September", &SEPTEMBER},
            {"October", &OCTOBER},
            {"November", &NOVEMBER},
            {"December", &DECEMBER}
        };
    
        std::string timeStrCopy = timeStr; // Convert the char array to a string for processing
    
        // Apply day and month replacements
        size_t pos;
        for (const auto& mapping : mappings) {
            pos = timeStrCopy.find(mapping.first);
            while (pos != std::string::npos) {
                timeStrCopy.replace(pos, mapping.first.length(), *(mapping.second));
                pos = timeStrCopy.find(mapping.first, pos + mapping.second->length());
            }
        }
    
        // Copy the modified string back to the character array
        strcpy(timeStr, timeStrCopy.c_str());
    }
    #endif

    // Unified function to apply replacements
    void applyLangReplacements(std::string& text, bool isValue) {
        if (isValue) {
            // Direct comparison for value replacements
            if (text.length() == 2) {
                if (text[0] == 'O') {
                    if (text[1] == 'n') {
                        text = ON;
                        return;
                    } else if (text[1] == 'f' && text == "Off") {
                        text = OFF;
                        return;
                    }
                }
            }
        }
        #if IS_LAUNCHER_DIRECTIVE
        else {
            // Direct comparison for launcher replacements
            switch (text.length()) {
                case 6:
                    if (text == "Reboot") {
                        text = REBOOT;
                    }
                    break;
                case 8:
                    if (text == "Shutdown") {
                        text = SHUTDOWN;
                    }
                    break;
                case 9:
                    if (text == "Reboot To") {
                        text = REBOOT_TO;
                    }
                    break;
                case 10:
                    if (text == "Boot Entry") {
                        text = BOOT_ENTRY;
                    }
                    break;
            }
        }
        #endif
    }
    
    
    
    // Predefined hexMap
    //const std::array<int, 256> hexMap = [] {
    //    std::array<int, 256> map = {0};
    //    map['0'] = 0; map['1'] = 1; map['2'] = 2; map['3'] = 3; map['4'] = 4;
    //    map['5'] = 5; map['6'] = 6; map['7'] = 7; map['8'] = 8; map['9'] = 9;
    //    map['A'] = 10; map['B'] = 11; map['C'] = 12; map['D'] = 13; map['E'] = 14; map['F'] = 15;
    //    map['a'] = 10; map['b'] = 11; map['c'] = 12; map['d'] = 13; map['e'] = 14; map['f'] = 15;
    //    return map;
    //}();
    
    
    // Prepare a map of default settings
    std::map<const std::string, std::string> defaultThemeSettingsMap = {
        {"default_overlay_color", whiteColor},
        {"default_package_color", whiteColor},
        {"default_script_color", "FF33FF"},
        {"clock_color", whiteColor},
        {"temperature_color", whiteColor},
        {"battery_color", "ffff45"},
        {"battery_charging_color", "00FF00"},
        {"battery_low_color", "FF0000"},
        {"widget_backdrop_alpha", "15"},
        {"widget_backdrop_color", blackColor},
        {"bg_alpha", "13"},
        {"bg_color", blackColor},
        {"separator_alpha", "15"},
        {"separator_color", "404040"},
        {"text_separator_color", "404040"},
        {"text_color", whiteColor},
        {"notification_text_color", whiteColor},
        {"header_text_color", whiteColor},
        {"header_separator_color", whiteColor},
        {"star_color", whiteColor},
        {"selection_star_color", whiteColor},
        {"bottom_button_color", whiteColor},
        {"bottom_text_color", whiteColor},
        {"bottom_separator_color", whiteColor},
        {"top_separator_color", "404040"},
        {"table_bg_color", "2C2C2C"},
        {"table_bg_alpha", "14"},
        {"table_section_text_color", whiteColor},
        //{"table_info_text_color", "00FFDD"},
        {"table_info_text_color", "9ed0ff"},
        {"warning_text_color", "FF7777"},
        {"healthy_ram_text_color", "00FF00"},
        {"neutral_ram_text_color", "FFAA00"},
        {"bad_ram_text_color", "FF0000"},
        {"trackbar_slider_color", "606060"},
        {"trackbar_slider_border_color", "505050"},
        {"trackbar_slider_malleable_color", "A0A0A0"},
        {"trackbar_full_color", "00FFDD"},
        {"trackbar_empty_color", "404040"},
        {"overlay_text_color", whiteColor},
        {"ult_overlay_text_color", "9ed0ff"},
        {"package_text_color", whiteColor},
        {"ult_package_text_color", "9ed0ff"},
        {"banner_version_text_color", greyColor},
        {"overlay_version_text_color", greyColor},
        {"ult_overlay_version_text_color", "00FFDD"},
        {"package_version_text_color", greyColor},
        {"ult_package_version_text_color", "00FFDD"},
        {"on_text_color", "00FFDD"},
        {"off_text_color", greyColor},
        {"invalid_text_color", "FF0000"},
        {"inprogress_text_color", "FFFF45"},
        {"selection_text_color", "9ed0ff"},
        {"selection_value_text_color", "FF7777"},
        {"selection_bg_color", blackColor},
        {"selection_bg_alpha", "11"},
        {"trackbar_color", "555555"},
        {"highlight_color_1", "2288CC"},
        {"highlight_color_2", "88FFFF"},
        {"highlight_color_3", "FFFF45"},
        {"highlight_color_4", "F7253E"},
        {"click_text_color", whiteColor},
        {"click_alpha", "7"},
        {"click_color", "3E25F7"},
        {"progress_alpha", "7"},
        {"progress_color", "253EF7"},
        {"invert_bg_click_color", FALSE_STR},
        //{"disable_selection_bg", FALSE_STR},
        //{"disable_selection_value_color", FALSE_STR},
        //{"disable_colorful_logo", FALSE_STR},
        {"logo_color_1", whiteColor},
        {"logo_color_2", "FF0000"},
        {"dynamic_logo_color_1", "00E669"},
        {"dynamic_logo_color_2", "8080EA"}
    };
    
    bool isNumericCharacter(char c) {
        return std::isdigit(c);
    }
    
    bool isValidHexColor(const std::string& hexColor) {
        // Check if the string is a valid hexadecimal color of the format "#RRGGBB"
        if (hexColor.size() != 6) {
            return false; // Must be exactly 6 characters long
        }
        
        for (char c : hexColor) {
            if (!isxdigit(c)) {
                return false; // Must contain only hexadecimal digits (0-9, A-F, a-f)
            }
        }
        
        return true;
    }
    
    
    
    //float calculateAmplitude(float x, float peakDurationFactor) {
    //    //const float phasePeriod = 360.0f * peakDurationFactor;  // One full phase period
    //
    //    // Convert x from radians to degrees and calculate phase within the period
    //    const int phase = static_cast<int>(x * RAD_TO_DEG) % static_cast<int>(360.0f * peakDurationFactor);
    //
    //    // Check if the phase is odd using bitwise operation
    //    if (phase & 1) {
    //        return 1.0f;  // Flat amplitude (maximum positive)
    //    } else {
    //        // Calculate the sinusoidal amplitude for the remaining period
    //        return (APPROXIMATE_cos(x) + 1.0f) / 2.0f;  // Cosine function expects radians
    //    }
    //}
            
    
    std::atomic<bool> refreshWallpaperNow(false);
    std::atomic<bool> refreshWallpaper(false);
    std::vector<u8> wallpaperData; 
    std::atomic<bool> inPlot(false);
    
    std::mutex wallpaperMutex;
    std::condition_variable cv;
    
    
    // Function to load the RGBA file into memory and modify wallpaperData directly
    //void loadWallpaperFile(const std::string& filePath, s32 width, s32 height) {
    //    const size_t originalDataSize = width * height * 4; // Original size in bytes (4 bytes per pixel)
    //    const size_t compressedDataSize = originalDataSize / 2; // RGBA4444 uses half the space
    //    
    //    wallpaperData.resize(compressedDataSize);
    //    
    //    if (!isFileOrDirectory(filePath)) {
    //        wallpaperData.clear();
    //        return;
    //    }
    //    
    //    #if !USING_FSTREAM_DIRECTIVE
    //        FILE* file = fopen(filePath.c_str(), "rb");
    //        if (!file) {
    //            wallpaperData.clear();
    //            return;
    //        }
    //        
    //        std::vector<uint8_t> buffer;
    //        //if (reducedMemory) {
    //        //    // Reuse wallpaperData to avoid double allocation
    //        //    buffer.swap(wallpaperData);
    //        //    buffer.resize(originalDataSize);
    //        //} else {
    //        buffer.resize(originalDataSize);
    //        //}
    //        
    //        const size_t bytesRead = fread(buffer.data(), 1, originalDataSize, file);
    //        fclose(file);
    //        
    //        if (bytesRead != originalDataSize) {
    //            wallpaperData.clear();
    //            return;
    //        }
    //        
    //    #else
    //        std::ifstream file(filePath, std::ios::binary);
    //        if (!file) {
    //            wallpaperData.clear();
    //            return;
    //        }
    //        
    //        std::vector<uint8_t> buffer;
    //        //if (reducedMemory) {
    //        //    buffer.swap(wallpaperData);
    //        //    buffer.resize(originalDataSize);
    //        //} else {
    //        buffer.resize(originalDataSize);
    //        //}
    //        
    //        file.read(reinterpret_cast<char*>(buffer.data()), originalDataSize);
    //        if (!file) {
    //            wallpaperData.clear();
    //            return;
    //        }
    //    #endif
    //    
    //    // Compress RGBA8888 to RGBA4444
    //    //if (reducedMemory) {
    //    //    // In-place compression to save memory
    //    //    size_t writeIndex = 0;
    //    //    for (size_t i = 0; i < originalDataSize; i += 8, writeIndex += 4) {
    //    //        uint8_t r1 = buffer[i] >> 4;
    //    //        uint8_t g1 = buffer[i + 1] >> 4;
    //    //        uint8_t b1 = buffer[i + 2] >> 4;
    //    //        uint8_t a1 = buffer[i + 3] >> 4;
    //    //
    //    //        uint8_t r2 = buffer[i + 4] >> 4;
    //    //        uint8_t g2 = buffer[i + 5] >> 4;
    //    //        uint8_t b2 = buffer[i + 6] >> 4;
    //    //        uint8_t a2 = buffer[i + 7] >> 4;
    //    //
    //    //        buffer[writeIndex]     = (r1 << 4) | g1;
    //    //        buffer[writeIndex + 1] = (b1 << 4) | a1;
    //    //        buffer[writeIndex + 2] = (r2 << 4) | g2;
    //    //        buffer[writeIndex + 3] = (b2 << 4) | a2;
    //    //    }
    //    //    buffer.resize(compressedDataSize);
    //    //    wallpaperData.swap(buffer);
    //    //} else {
    //    uint8_t* input = buffer.data();
    //    uint8_t* output = wallpaperData.data();
    //    //uint8_t r1, g1, b1, a1;
    //    //uint8_t r2, g2, b2, a2;
    //    
    //    //for (size_t i = 0, j = 0; i < originalDataSize; i += 8, j += 4) {
    //    //    // Read 2 RGBA pixels (8 bytes)
    //    //    const uint8_t r1 = input[i] >> 4;
    //    //    const uint8_t g1 = input[i + 1] >> 4;
    //    //    const uint8_t b1 = input[i + 2] >> 4;
    //    //    const uint8_t a1 = input[i + 3] >> 4;
    //    //    
    //    //    const uint8_t r2 = input[i + 4] >> 4;
    //    //    const uint8_t g2 = input[i + 5] >> 4;
    //    //    const uint8_t b2 = input[i + 6] >> 4;
    //    //    const uint8_t a2 = input[i + 7] >> 4;
    //    //    
    //    //    // Pack them into 4 bytes (2 bytes per pixel)
    //    //    output[j]     = (r1 << 4) | g1;
    //    //    output[j + 1] = (b1 << 4) | a1;
    //    //    output[j + 2] = (r2 << 4) | g2;
    //    //    output[j + 3] = (b2 << 4) | a2;
    //    //}
    //    
    //    for (size_t i = 0, j = 0; i < originalDataSize; i += 16, j += 8) {
    //        output[j]     = ((input[i]     >> 4) << 4) | (input[i + 1] >> 4);
    //        output[j + 1] = ((input[i + 2] >> 4) << 4) | (input[i + 3] >> 4);
    //        output[j + 2] = ((input[i + 4] >> 4) << 4) | (input[i + 5] >> 4);
    //        output[j + 3] = ((input[i + 6] >> 4) << 4) | (input[i + 7] >> 4);
    //        output[j + 4] = ((input[i + 8] >> 4) << 4) | (input[i + 9] >> 4);
    //        output[j + 5] = ((input[i + 10] >> 4) << 4) | (input[i + 11] >> 4);
    //        output[j + 6] = ((input[i + 12] >> 4) << 4) | (input[i + 13] >> 4);
    //        output[j + 7] = ((input[i + 14] >> 4) << 4) | (input[i + 15] >> 4);
    //    }
    //    //}
    //}


    
        
    void loadWallpaperFile(const std::string& filePath, s32 width, s32 height) {
        const size_t originalDataSize = width * height * 4;
        const size_t compressedDataSize = originalDataSize / 2;
        
        wallpaperData.resize(compressedDataSize);
        
        if (!isFileOrDirectory(filePath)) {
            wallpaperData.clear();
            return;
        }
        
        FILE* file = fopen(filePath.c_str(), "rb");
        if (!file) {
            wallpaperData.clear();
            return;
        }

        setvbuf(file, nullptr, _IOFBF, 256 * 1024);
        
        constexpr size_t chunkBytes = 128 * 1024;
        uint8_t chunkBuffer[chunkBytes];
        
        size_t totalRead = 0;
        uint8_t* dst = wallpaperData.data();
        const uint8x8_t mask = vdup_n_u8(0xF0);
        
        while (totalRead < originalDataSize) {
            const size_t remaining = originalDataSize - totalRead;
            const size_t toRead = remaining < chunkBytes ? remaining : chunkBytes;
            
            const size_t bytesRead = fread(chunkBuffer, 1, toRead, file);
            if (bytesRead == 0) {
                fclose(file);
                wallpaperData.clear();
                return;
            }
            
            const uint8_t* src = chunkBuffer;
            size_t i = 0;
            
            // NEON: Process 16 bytes -> 8 bytes
            for (; i + 16 <= bytesRead; i += 16) {
                uint8x16_t data = vld1q_u8(src + i);
                uint8x8x2_t sep = vuzp_u8(vget_low_u8(data), vget_high_u8(data));
                vst1_u8(dst, vorr_u8(vand_u8(sep.val[0], mask), vshr_n_u8(sep.val[1], 4)));
                dst += 8;
            }
            
            // Scalar fallback
            for (; i + 1 < bytesRead; i += 2) {
                *dst++ = (src[i] & 0xF0) | (src[i + 1] >> 4);
            }
            
            totalRead += bytesRead;
        }
        
        fclose(file);
    }
    

    void loadWallpaperFileWhenSafe() {
        if (expandedMemory && !inPlot.load(std::memory_order_acquire) && !refreshWallpaper.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(wallpaperMutex);
            cv.wait(lock, [] { return !inPlot.load(std::memory_order_acquire) && !refreshWallpaper.load(std::memory_order_acquire); });
            if (wallpaperData.empty() && isFileOrDirectory(WALLPAPER_PATH)) {
                loadWallpaperFile(WALLPAPER_PATH);
            }
        }
    }


    void reloadWallpaper() {
        // Signal that wallpaper is being refreshed
        refreshWallpaper.store(true, std::memory_order_release);
    
        // Lock the mutex for condition waiting
        std::unique_lock<std::mutex> lock(wallpaperMutex);
    
        // Wait for inPlot to be false before reloading the wallpaper
        cv.wait(lock, [] { return !inPlot.load(std::memory_order_acquire); });
    
        // Clear the current wallpaper data
        wallpaperData.clear();
    
        // Reload the wallpaper file
        if (isFileOrDirectory(WALLPAPER_PATH)) {
            loadWallpaperFile(WALLPAPER_PATH);
        }
    
        // Signal that wallpaper has finished refreshing
        refreshWallpaper.store(false, std::memory_order_release);
        
        // Notify any waiting threads
        cv.notify_all();
    }

    
    
    // Global variables for FPS calculation
    //double lastTimeCount = 0.0;
    //int frameCount = 0;
    //float fps = 0.0f;
    //double elapsedTime = 0.0;
    
    std::atomic<bool> themeIsInitialized(false); // for loading the theme once in OverlayFrame / HeaderOverlayFrame
    
    // Variables for touch commands
    std::atomic<bool> touchingBack(false);
    std::atomic<bool> touchingSelect(false);
    std::atomic<bool> touchingNextPage(false);
    std::atomic<bool> touchingMenu(false);
    std::atomic<bool> shortTouchAndRelease(false);
    std::atomic<bool> longTouchAndRelease(false);
    std::atomic<bool> simulatedBack(false);
    //bool simulatedBackComplete = true;
    std::atomic<bool> simulatedSelect(false);
    //bool simulatedSelectComplete = true;
    std::atomic<bool> simulatedNextPage(false);
    //std::atomic<bool> simulatedNextPageComplete(true);
    std::atomic<bool> simulatedMenu(false);
    //bool simulatedMenuComplete = true;
    std::atomic<bool> stillTouching(false);
    std::atomic<bool> interruptedTouch(false);
    std::atomic<bool> touchInBounds(false);
    
    
#if USING_WIDGET_DIRECTIVE
    // Battery implementation
    bool powerInitialized = false;
    bool powerCacheInitialized;
    uint32_t powerCacheCharge;
    //float powerConsumption;
    bool powerCacheIsCharging;
    PsmSession powerSession;
    
    // Define variables to store previous battery charge and time
    //uint32_t prevBatteryCharge = 0;
    //s64 timeOut = 0;
    
    
    std::atomic<uint32_t> batteryCharge{0};
    std::atomic<bool> isCharging{false};
    //bool validPower;
    
    
    
    bool powerGetDetails(uint32_t *_batteryCharge, bool *_isCharging) {
        static uint64_t last_call_ns = 0;
        
        // Ensure power system is initialized
        if (!powerInitialized) {
            return false;
        }
        
        // Get the current time in nanoseconds
        const uint64_t now_ns = armTicksToNs(armGetSystemTick());
        
        // 3 seconds in nanoseconds
        static constexpr uint64_t min_delay_ns = 3000000000ULL;
        
        // Check if enough time has elapsed or if cache is not initialized
        const bool useCache = (now_ns - last_call_ns <= min_delay_ns) && powerCacheInitialized;
        if (!useCache) {
            PsmChargerType charger = PsmChargerType_Unconnected;
            Result rc = psmGetBatteryChargePercentage(_batteryCharge);
            bool hwReadsSucceeded = R_SUCCEEDED(rc);
    
            if (hwReadsSucceeded) {
                rc = psmGetChargerType(&charger);
                hwReadsSucceeded &= R_SUCCEEDED(rc);
                *_isCharging = (charger != PsmChargerType_Unconnected);
    
                if (hwReadsSucceeded) {
                    // Update cache
                    powerCacheCharge = *_batteryCharge;
                    powerCacheIsCharging = *_isCharging;
                    powerCacheInitialized = true;
                    last_call_ns = now_ns; // Update last call time after successful hardware read
                    return true;
                }
            }
    
            // Use cached values if the hardware read fails
            if (powerCacheInitialized) {
                *_batteryCharge = powerCacheCharge;
                *_isCharging = powerCacheIsCharging;
                return hwReadsSucceeded; // Return false if hardware read failed but cache is valid
            }
    
            // Return false if cache is not initialized and hardware read failed
            return false;
        }
    
        // Use cached values if not enough time has passed
        *_batteryCharge = powerCacheCharge;
        *_isCharging = powerCacheIsCharging;
        return true; // Return true as cache is used
    }
    
    
    void powerInit(void) {
        uint32_t charge = 0;
        bool charging = false;
    
        powerCacheInitialized = false;
        powerCacheCharge = 0;
        powerCacheIsCharging = false;
    
        if (!powerInitialized) {
            Result rc = psmInitialize();
            if (R_SUCCEEDED(rc)) {
                rc = psmBindStateChangeEvent(&powerSession, 1, 1, 1);
    
                if (R_FAILED(rc))
                    psmExit();
    
                if (R_SUCCEEDED(rc)) {
                    powerInitialized = true;
                    ult::powerGetDetails(&charge, &charging);
                    ult::batteryCharge.store(charge, std::memory_order_release);
                    ult::isCharging.store(charging, std::memory_order_release);
                    //prevBatteryCharge = charge; // if needed
                }
            }
        }
    }
    
    void powerExit(void) {
        if (powerInitialized) {
            psmUnbindStateChangeEvent(&powerSession);
            psmExit();
            powerInitialized = false;
            powerCacheInitialized = false;
        }
    }
#endif
    
    
    // Temperature Implementation
    std::atomic<float> PCB_temperature{0.0f};
    std::atomic<float> SOC_temperature{0.0f};
    
    /*
    I2cReadRegHandler was taken from Switch-OC-Suite source code made by KazushiMe
    Original repository link (Deleted, last checked 15.04.2023): https://github.com/KazushiMe/Switch-OC-Suite
    */
    
    Result I2cReadRegHandler(u8 reg, I2cDevice dev, u16 *out) {
        struct readReg {
            u8 send;
            u8 sendLength;
            u8 sendData;
            u8 receive;
            u8 receiveLength;
        };
    
        I2cSession _session;
    
        Result res = i2cOpenSession(&_session, dev);
        if (res)
            return res;
    
        u16 val;
    
        struct readReg readRegister = {
            .send = 0 | (I2cTransactionOption_Start << 6),
            .sendLength = sizeof(reg),
            .sendData = reg,
            .receive = 1 | (I2cTransactionOption_All << 6),
            .receiveLength = sizeof(val),
        };
    
        res = i2csessionExecuteCommandList(&_session, &val, sizeof(val), &readRegister, sizeof(readRegister));
        if (res) {
            i2csessionClose(&_session);
            return res;
        }
    
        *out = val;
        i2csessionClose(&_session);
        return 0;
    }
    
    
    // Common helper function to read temperature (integer and fractional parts)
    Result ReadTemperature(float *temperature, u8 integerReg, u8 fractionalReg, bool integerOnly) {
        u16 rawValue;
        u8 val;
        s32 integerPart = 0;
        float fractionalPart = 0.0f;  // Change this to a float to retain fractional precision
    
        // Read the integer part of the temperature
        Result res = I2cReadRegHandler(integerReg, I2cDevice_Tmp451, &rawValue);
        if (R_FAILED(res)) {
            return res;  // Error during I2C read
        }
    
        val = (u8)rawValue;  // Cast the value to an 8-bit unsigned integer
        integerPart = val;    // Integer part of temperature in Celsius
    
        if (integerOnly)
        {
            *temperature = static_cast<float>(integerPart);  // Ensure it's treated as a float
            return 0;  // Return only integer part if requested
        }
    
        // Read the fractional part of the temperature
        res = I2cReadRegHandler(fractionalReg, I2cDevice_Tmp451, &rawValue);
        if (R_FAILED(res)) {
            return res;  // Error during I2C read
        }
    
        val = (u8)rawValue;  // Cast the value to an 8-bit unsigned integer
        fractionalPart = static_cast<float>(val >> 4) * 0.0625f;  // Convert upper 4 bits into fractional part
    
        // Combine integer and fractional parts
        *temperature = static_cast<float>(integerPart) + fractionalPart;
        
        return 0;
    }
    
    // Function to get the SOC temperature
    Result ReadSocTemperature(float *temperature, bool integerOnly) {
        return ReadTemperature(temperature, TMP451_SOC_TEMP_REG, TMP451_SOC_TMP_DEC_REG, integerOnly);
    }
    
    // Function to get the PCB temperature
    Result ReadPcbTemperature(float *temperature, bool integerOnly) {
        return ReadTemperature(temperature, TMP451_PCB_TEMP_REG, TMP451_PCB_TMP_DEC_REG, integerOnly);
    }
    
    
    // Time implementation
    CONSTEXPR_STRING std::string DEFAULT_DT_FORMAT = "'%a %T'";
    std::string datetimeFormat = "%a %T";
    
    
    // Widget settings
    //std::string hideClock, hideBattery, hidePCBTemp, hideSOCTemp;
    bool hideClock, hideBattery, hidePCBTemp, hideSOCTemp, dynamicWidgetColors;
    bool hideWidgetBackdrop, centerWidgetAlignment, extendedWidgetBackdrop;

    #if IS_LAUNCHER_DIRECTIVE
    void reinitializeWidgetVars() {
        // Load INI data once instead of 8 separate file reads
        auto ultrahandSection = getKeyValuePairsFromSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME);
        
        // Helper lambda to safely get boolean values with proper defaults
        auto getBoolValue = [&](const std::string& key, bool defaultValue = false) -> bool {
            if (ultrahandSection.count(key) > 0) {
                return (ultrahandSection.at(key) != FALSE_STR);
            }
            return defaultValue;
        };
        
        // Set all values from the loaded section with correct defaults (matching initialization)
        hideClock = getBoolValue("hide_clock", false);                           // FALSE_STR default
        hideBattery = getBoolValue("hide_battery", true);                        // TRUE_STR default
        hideSOCTemp = getBoolValue("hide_soc_temp", true);                       // TRUE_STR default  
        hidePCBTemp = getBoolValue("hide_pcb_temp", true);                       // TRUE_STR default
        dynamicWidgetColors = getBoolValue("dynamic_widget_colors", true);       // TRUE_STR default
        hideWidgetBackdrop = getBoolValue("hide_widget_backdrop", false);        // FALSE_STR default
        centerWidgetAlignment = getBoolValue("center_widget_alignment", true);   // TRUE_STR default
        extendedWidgetBackdrop = getBoolValue("extended_widget_backdrop", false); // FALSE_STR default
    }
    #endif
    
    bool cleanVersionLabels, hideOverlayVersions, hidePackageVersions, useLibultrahandTitles, useLibultrahandVersions, usePackageTitles, usePackageVersions;
    


    
    // Implementation
    OverlayHeapSize getCurrentHeapSize() {
        // Fast path: return cached value if already loaded
        if (heapSizeCache.initialized) {
            return heapSizeCache.cachedSize;
        }
        
        // Slow path: read from file (only happens once)
        FILE* f = fopen(ult::OVL_HEAP_CONFIG_PATH.c_str(), "rb");
        if (!f) {
            heapSizeCache.cachedSize = OverlayHeapSize::Size_6MB;
            heapSizeCache.initialized = true;
            return heapSizeCache.cachedSize;
        }
        
        u64 size;
        if (fread(&size, sizeof(size), 1, f) == 1) {
            if (size == 0x400000 || size == 0x600000 || size == 0x800000 || size == 0xA00000) {
                heapSizeCache.cachedSize = static_cast<OverlayHeapSize>(size);
            }
        }
        
        fclose(f);
        heapSizeCache.initialized = true;
        return heapSizeCache.cachedSize;
    }
    
    //OverlayHeapSize currentHeapSize = getCurrentHeapSize();
    OverlayHeapSize currentHeapSize = OverlayHeapSize::Size_6MB;
    
    bool setOverlayHeapSize(OverlayHeapSize heapSize) {
        ult::createDirectory(ult::NX_OVLLOADER_PATH);
        
        FILE* f = fopen(ult::OVL_HEAP_CONFIG_PATH.c_str(), "wb");
        if (!f) return false;
        
        const u64 size = static_cast<u64>(heapSize);
        const bool success = (fwrite(&size, sizeof(size), 1, f) == 1);
        fclose(f);
        
        // Update cache on successful write
        if (success) {
            heapSizeCache.cachedSize = heapSize;
            heapSizeCache.initialized = true;
        }
        
        return success;
    }
    
    
    // Implementation
    bool requestOverlayExit() {
        ult::createDirectory(ult::NX_OVLLOADER_PATH);
        
        FILE* f = fopen(ult::OVL_EXIT_FLAG_PATH.c_str(), "wb");
        if (!f) return false;
        
        // Write a single byte (flag file just needs to exist)
        u8 flag = 1;
        bool success = (fwrite(&flag, 1, 1, f) == 1);
        fclose(f);
        
        return success;
    }


    const std::string loaderInfo = envGetLoaderInfo();
    std::string loaderTitle = extractTitle(loaderInfo);

    bool expandedMemory = false;
    bool furtherExpandedMemory = false;
    bool limitedMemory = false;
    
    std::string versionLabel;
    
    #if IS_LAUNCHER_DIRECTIVE
    void reinitializeVersionLabels() {
        // Load INI data once instead of 6 separate file reads
        auto ultrahandSection = getKeyValuePairsFromSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME);
        
        // Helper lambda to safely get boolean values with proper defaults
        auto getBoolValue = [&](const std::string& key, bool defaultValue = false) -> bool {
            if (ultrahandSection.count(key) > 0) {
                return (ultrahandSection.at(key) != FALSE_STR);
            }
            return defaultValue;
        };
        
        // Set all values from the loaded section with correct defaults (matching initialization)
        cleanVersionLabels = getBoolValue("clean_version_labels", false);        // FALSE_STR default
        hideOverlayVersions = getBoolValue("hide_overlay_versions", false);      // FALSE_STR default  
        hidePackageVersions = getBoolValue("hide_package_versions", false);      // FALSE_STR default
        //libultrahandTitles = getBoolValue("libultrahand_titles", false);               // FALSE_STR default
        //useLibultrahandVersions = getBoolValue("libultrahand_versions", true);            // TRUE_STR default
        //matchPackages = getBoolValue("match_packages", true);            // TRUE_STR default
        
        //#ifdef APP_VERSION
        //versionLabel = cleanVersionLabel(APP_VERSION) + "  (" + loaderTitle + " " + cleanVersionLabel(loaderInfo) + ")";
        //#endif
        //versionLabel = (cleanVersionLabels) ? std::string(APP_VERSION) : (std::string(APP_VERSION) + "   (" + extractTitle(loaderInfo) + " v" + cleanVersionLabel(loaderInfo) + ")");
    }
    #endif
    
    
    // Number of renderer threads to use
    const unsigned numThreads = 4;//expandedMemory ? 4 : 0;
    std::vector<std::thread> renderThreads(numThreads);

    const s32 bmpChunkSize = ((720 + numThreads - 1) / numThreads);
    std::atomic<s32> currentRow;
    
    //std::atomic<unsigned int> barrierCounter{0};
    //std::mutex barrierMutex;
    //std::condition_variable barrierCV;
    //
    //void barrierWait() {
    //    std::unique_lock<std::mutex> lock(barrierMutex);
    //    if (++barrierCounter == numThreads) {
    //        barrierCounter = 0; // Reset for the next round
    //        barrierCV.notify_all();
    //    } else {
    //        barrierCV.wait(lock, [] { return barrierCounter == 0; });
    //    }
    //}
}
