/********************************************************************************
 * Custom Fork Information
 * 
 * File: tesla.hpp
 * Author: ppkantorski
 * Description: 
 *   This file serves as the core logic for the Ultrahand Overlay project's custom fork
 *   of libtesla, an overlay executor. Within this file, you will find a collection of
 *   functions, menu structures, and interaction logic designed to facilitate the
 *   smooth execution and flexible customization of overlays within the project.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 *
 *  Copyright (c) 2023-2025 ppkantorski
 ********************************************************************************/

/**
 * Copyright (C) 2020 werwolv
 *
 * This file is part of libtesla.
 *
 * libtesla is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * libtesla is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libtesla.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once


#include <ultra.hpp>
#include <switch.h>
#include <arm_neon.h>

#include <strings.h>
#include <math.h>

#if !IS_LAUNCHER_DIRECTIVE
#include <filesystem> // unused, but preserved for projects that might need it
#endif

#include <algorithm>
#include <cstring>
#include <cwctype>
#include <string>
#include <functional>
#include <type_traits>
#include <mutex>
#include <shared_mutex>
#include <memory>
//#include <chrono> // despite being commented out, it must still be being imported via other libs
#include <list>
#include <stack>
#include <map>
//#include <barrier>


// Define this makro before including tesla.hpp in your main file. If you intend
// to use the tesla.hpp header in more than one source file, only define it once!
// #define TESLA_INIT_IMPL

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

#ifdef TESLA_INIT_IMPL
    #define STB_TRUETYPE_IMPLEMENTATION
#endif
#include "stb_truetype.h"

#pragma GCC diagnostic pop

#define ELEMENT_BOUNDS(elem) elem->getX(), elem->getY(), elem->getWidth(), elem->getHeight()

#define ASSERT_EXIT(x) if (R_FAILED(x)) std::exit(1)
#define ASSERT_FATAL(x) if (Result res = x; R_FAILED(res)) fatalThrow(res)

#define PACKED __attribute__((packed))
#define ALWAYS_INLINE inline __attribute__((always_inline))

/// Evaluates an expression that returns a result, and returns the result if it would fail.
#define TSL_R_TRY(resultExpr)           \
    ({                                  \
        const auto result = resultExpr; \
        if (R_FAILED(result)) {         \
            return result;              \
        }                               \
    })

using namespace std::literals::string_literals;
using namespace std::literals::chrono_literals; // potentially unused, restored for softare compatibility

#if IS_STATUS_MONITOR_DIRECTIVE
//struct GlyphInfo {
//    u8* pointer;
//    int width;
//    int height;
//};

struct KeyPairHash {
    std::size_t operator()(const std::pair<int, float>& key) const {
        // Combine hashes of both components
        union returnValue {
            char c[8];
            std::size_t s;
        } value;
        memcpy(&value.c[0], &key.first, 4);
        memcpy(&value.c[4], &key.second, 4);
        return value.s;
    }
};

// Custom equality comparison for int-float pairs
struct KeyPairEqual {
    bool operator()(const std::pair<int, float>& lhs, const std::pair<int, float>& rhs) const {
        //static constexpr float epsilon = 0.00001f;
        return lhs.first == rhs.first && 
            std::abs(lhs.second - rhs.second) < 0.00001f;
    }
};

//std::unordered_map<std::pair<s32, float>, GlyphInfo, KeyPairHash, KeyPairEqual> cache;

u8 TeslaFPS = 60;
//u8 alphabackground = 0xD;
std::atomic<bool> triggerExitNow{false};
std::atomic<bool> isRendering{false};
std::atomic<bool> delayUpdate{false};
std::atomic<bool> pendingExit{false};
std::atomic<bool> wasRendering{false};

LEvent renderingStopEvent;
bool FullMode = true;
bool deactivateOriginalFooter = false;
//bool fontCache = true;
bool disableJumpTo = false;

// Check for mini/micro mode flags
//bool isMiniOrMicroMode = false;
inline std::string lastMode;
inline std::set<std::string> overlayModes = {"full", "mini", "micro", "fps_graph", "fps_counter", "game_resolutions"};

bool isValidOverlayMode() {
    return overlayModes.count(lastMode) > 0;
}

#endif

#if USING_FPS_INDICATOR_DIRECTIVE
float fps = 0.0;
int frameCount = 0;
double elapsedTime;
#endif


// Custom variables
//static bool jumpToListItem = false;
inline std::atomic<bool> jumpToTop{false};
inline std::atomic<bool> jumpToBottom{false};
inline std::atomic<bool> skipUp{false};
inline std::atomic<bool> skipDown{false};
inline u32 offsetWidthVar = 112;
//inline std::string g_overlayFilename;;
inline std::string lastOverlayFilename;
inline std::string lastOverlayMode;

inline std::mutex jumpItemMutex;
inline std::string jumpItemName;
inline std::string jumpItemValue;
inline std::atomic<bool> jumpItemExactMatch{true};

inline std::atomic<bool> s_onLeftPage{false};
inline std::atomic<bool> s_onRightPage{false};
inline std::atomic<bool> screenshotsAreDisabled{false};
inline std::atomic<bool> screenshotsAreForceDisabled{false};

//#if IS_LAUNCHER_DIRECTIVE
inline bool hideHidden = false;
//#endif

//inline std::atomic<bool> isLaunchingNextOverlay{false};
inline std::atomic<bool> mainComboHasTriggered{false};
inline std::atomic<bool> launchComboHasTriggered{false};



// Sound triggering variables
inline std::atomic<bool> triggerNavigationSound{false};
inline std::atomic<bool> triggerEnterSound{false};
inline std::atomic<bool> triggerExitSound{false};
inline std::atomic<bool> triggerWallSound{false};
inline std::atomic<bool> triggerOnSound{false};
inline std::atomic<bool> triggerOffSound{false};
inline std::atomic<bool> triggerSettingsSound{false};
inline std::atomic<bool> triggerMoveSound{false};
inline std::atomic<bool> disableSound{false};
//inline std::atomic<bool> clearSoundCacheNow{false};
inline std::atomic<bool> reloadSoundCacheNow{false};

// Haptic triggering variables
inline std::atomic<bool> triggerRumbleClick{false};
inline std::atomic<bool> triggerRumbleDoubleClick{false};


static inline void triggerNavigationFeedback() {
    triggerRumbleClick.store(true, std::memory_order_release);
    triggerNavigationSound.store(true, std::memory_order_release);
}

static inline void triggerEnterFeedback() {
    triggerRumbleClick.store(true, std::memory_order_release);
    triggerEnterSound.store(true, std::memory_order_release);
}

static inline void triggerExitFeedback() {
    triggerRumbleDoubleClick.store(true, std::memory_order_release);
    triggerExitSound.store(true, std::memory_order_release);
}


/**
 * @brief Checks if an NRO file uses new libnx (has LNY2 tag).
 *
 * @param filePath The path to the NRO file.
 * @return true if the file uses new libnx (LNY2 present), false otherwise.
 */
static inline bool usingLNY2(const std::string& filePath) {
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file)
        return false;
    
    // --- Get file size ---
    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    if (fileSize < (long)(sizeof(NroStart) + sizeof(NroHeader))) {
        fclose(file);
        return false;
    }
    const size_t fileSz = (size_t)fileSize;
    fseek(file, 0, SEEK_SET);
    
    // --- Read front chunk (header + MOD0 area) ---
    constexpr size_t FRONT_READ_SIZE = 8192;
    const size_t frontReadSize = (fileSz < FRONT_READ_SIZE) ? fileSz : FRONT_READ_SIZE;
    uint8_t* frontBuf = (uint8_t*)malloc(frontReadSize);
    if (!frontBuf) {
        fclose(file);
        return false;
    }
    
    if (fread(frontBuf, 1, frontReadSize, file) != frontReadSize) {
        free(frontBuf);
        fclose(file);
        return false;
    }
    
    // --- Extract offsets directly (no NroHeader copy needed) ---
    const uint32_t mod0_rel   = *reinterpret_cast<const uint32_t*>(frontBuf + 0x4);
    const uint32_t text_offset = *reinterpret_cast<const uint32_t*>(frontBuf + 0x20);
    
    bool isNew = false;
    
    // --- MOD0 detection ---
    if (text_offset < fileSz && mod0_rel != 0 && text_offset <= fileSz - mod0_rel) {
        const uint32_t mod0_offset = text_offset + mod0_rel;
        
        // --- MOD0 is inside front buffer ---
        if (mod0_offset <= frontReadSize - 60) {
            const uint8_t* mod0_ptr = frontBuf + mod0_offset;
            if (memcmp(mod0_ptr, "MOD0", 4) == 0 &&
                memcmp(mod0_ptr + 52, "LNY2", 4) == 0)
            {
                const uint32_t libnx = *reinterpret_cast<const uint32_t*>(mod0_ptr + 56);
                isNew = (libnx >= 1);
            }
        }
        // --- MOD0 must be read separately ---
        else if (mod0_offset <= fileSz - 60) {
            uint8_t mod0Buf[60];
            fseek(file, mod0_offset, SEEK_SET);
            if (fread(mod0Buf, 1, 60, file) == 60) {
                if (memcmp(mod0Buf, "MOD0", 4) == 0 &&
                    memcmp(mod0Buf + 52, "LNY2", 4) == 0)
                {
                    const uint32_t libnx = *reinterpret_cast<const uint32_t*>(mod0Buf + 56);
                    isNew = (libnx >= 1);
                }
            }
        }
    }
    
    free(frontBuf);
    fclose(file);
    return isNew;
}

/**
 * @brief Checks if the current AMS version is at least the specified version.
 *
 * @param major Minimum major version required
 * @param minor Minimum minor version required  
 * @param patch Minimum patch version required
 * @return true if current AMS version >= specified version, false otherwise
 */
static inline bool amsVersionAtLeast(uint8_t major, uint8_t minor, uint8_t patch) {
    u64 packed_version;
    if (R_FAILED(splGetConfig((SplConfigItem)65000, &packed_version))) {
        return false;
    }
    
    return ((packed_version >> 40) & 0xFFFFFF) >= static_cast<u32>((major << 16) | (minor << 8) | patch);
}

static bool requiresLNY2 = false;







namespace tsl {

    // Booleans
    inline std::atomic<bool> clearGlyphCacheNow(false);

    // Constants
    
    namespace cfg {
        
        constexpr u32 ScreenWidth = 1920;       ///< Width of the Screen
        constexpr u32 ScreenHeight = 1080;      ///< Height of the Screen
        constexpr u32 LayerMaxWidth = 1280;
        constexpr u32 LayerMaxHeight = 720;
        
        extern u16 LayerWidth;                  ///< Width of the Tesla layer
        extern u16 LayerHeight;                 ///< Height of the Tesla layer
        extern u16 LayerPosX;                   ///< X position of the Tesla layer
        extern u16 LayerPosY;                   ///< Y position of the Tesla layer
        extern u16 FramebufferWidth;            ///< Width of the framebuffer
        extern u16 FramebufferHeight;           ///< Height of the framebuffer
        extern u64 launchCombo;                 ///< Overlay activation key combo
        extern u64 launchCombo2;                 ///< Overlay activation key combo
        
    }
    
    /**
     * @brief RGBA4444 Color structure
     */
    struct Color {
        
        union {
            struct {
                u16 r: 4, g: 4, b: 4, a: 4;
            } PACKED;
            u16 rgba;
        };
        
        constexpr inline Color(u16 raw): rgba(raw) {}
        constexpr inline Color(u8 r, u8 g, u8 b, u8 a): r(r), g(g), b(b), a(a) {}
        
    };
    
    //#if USING_WIDGET_DIRECTIVE
    // Ultra-fast version - zero variables, optimized calculations
    inline constexpr Color GradientColor(float temperature) {
        if (temperature <= 35.0f) return Color(7, 7, 15, 0xFF);
        if (temperature >= 65.0f) return Color(15, 0, 0, 0xFF);
        
        if (temperature < 45.0f) {
            // Single calculation, avoid repetition
            const float factor = (temperature - 35.0f) * 0.1f;
            return Color(7 - 7 * factor, 7 + 8 * factor, 15 - 15 * factor, 0xFF);
        }
        
        if (temperature < 55.0f) {
            return Color(15 * (temperature - 45.0f) * 0.1f, 15, 0, 0xFF);
        }
        
        return Color(15, 15 - 15 * (temperature - 55.0f) * 0.1f, 0, 0xFF);
    }
    //#endif


    // Ultra-fast version - single variable, minimal branching
    inline Color RGB888(const std::string& hexColor, size_t alpha = 15, const std::string& defaultHexColor = ult::whiteColor) {
        const char* h = hexColor.size() == 6 ? hexColor.data() :
                        hexColor.size() == 7 && hexColor[0] == '#' ? hexColor.data() + 1 :
                        defaultHexColor.data();
        
        return Color(
            (ult::hexMap[h[0]] << 4 | ult::hexMap[h[1]]) >> 4,
            (ult::hexMap[h[2]] << 4 | ult::hexMap[h[3]]) >> 4,
            (ult::hexMap[h[4]] << 4 | ult::hexMap[h[5]]) >> 4,
            alpha
        );
    }
    
    
    namespace style {
        constexpr u32 ListItemDefaultHeight         = 70;       ///< Standard list item height
        constexpr u32 MiniListItemDefaultHeight     = 40;       ///< Mini list item height
        constexpr u32 TrackBarDefaultHeight         = 83;       ///< Standard track bar height
        constexpr u8  ListItemHighlightSaturation   = 7;        ///< Maximum saturation of Listitem highlights
        constexpr u8  ListItemHighlightLength       = 22;       ///< Maximum length of Listitem highlights
        
        namespace color {
            constexpr Color ColorFrameBackground  = { 0x0, 0x0, 0x0, 0xD };   ///< Overlay frame background color
            constexpr Color ColorTransparent      = { 0x0, 0x0, 0x0, 0x0 };   ///< Transparent color
            constexpr Color ColorHighlight        = { 0x0, 0xF, 0xD, 0xF };   ///< Greenish highlight color
            constexpr Color ColorFrame            = { 0x7, 0x7, 0x7, 0x7 };   ///< Outer boarder color // CUSTOM MODIFICATION
            constexpr Color ColorHandle           = { 0x5, 0x5, 0x5, 0xF };   ///< Track bar handle color
            constexpr Color ColorText             = { 0xF, 0xF, 0xF, 0xF };   ///< Standard text color
            constexpr Color ColorDescription      = { 0xA, 0xA, 0xA, 0xF };   ///< Description text color
            constexpr Color ColorHeaderBar        = { 0xC, 0xC, 0xC, 0xF };   ///< Category header rectangle color
            constexpr Color ColorClickAnimation   = { 0x0, 0x2, 0x2, 0xF };   ///< Element click animation color
        }
    }

    static bool overrideBackButton = false; // for properly overriding the automatic "go back" functionality of KEY_B button presses

    // Theme color variable definitions
    //static bool disableColorfulLogo = false;

    
    static Color logoColor1 = RGB888(ult::whiteColor);
    static Color logoColor2 = RGB888("F7253E");
    

    static size_t defaultBackgroundAlpha = 13;
    
    static Color defaultBackgroundColor = RGB888(ult::blackColor, defaultBackgroundAlpha);
    static Color defaultTextColor = RGB888(ult::whiteColor);
    static Color notificationTextColor = RGB888(ult::whiteColor);
    static Color headerTextColor = RGB888(ult::whiteColor);
    static Color headerSeparatorColor = RGB888(ult::whiteColor);
    static Color starColor = RGB888(ult::whiteColor);
    static Color selectionStarColor = RGB888(ult::whiteColor);
    static Color buttonColor = RGB888(ult::whiteColor);
    static Color bottomTextColor = RGB888(ult::whiteColor);
    static Color bottomSeparatorColor = RGB888(ult::whiteColor);
    static Color topSeparatorColor = RGB888("404040");

    static Color defaultOverlayColor = RGB888(ult::whiteColor);
    static Color defaultPackageColor = RGB888(ult::whiteColor);//RGB888("#00FF00");
    static Color defaultScriptColor = RGB888("FF33FF");
    static Color clockColor = RGB888(ult::whiteColor);
    static Color temperatureColor = RGB888(ult::whiteColor);
    static Color batteryColor = RGB888("ffff45");
    static Color batteryChargingColor = RGB888("00FF00");
    static Color batteryLowColor = RGB888("FF0000");
    static size_t widgetBackdropAlpha = 15;
    static Color widgetBackdropColor = RGB888(ult::blackColor, widgetBackdropAlpha);

    static Color overlayTextColor = RGB888(ult::whiteColor);
    static Color ultOverlayTextColor = RGB888("9ed0ff");
    static Color packageTextColor = RGB888(ult::whiteColor);
    static Color ultPackageTextColor = RGB888("9ed0ff");

    static Color bannerVersionTextColor = RGB888(ult::greyColor);
    static Color overlayVersionTextColor = RGB888(ult::greyColor);
    static Color ultOverlayVersionTextColor = RGB888("00FFDD");
    static Color packageVersionTextColor = RGB888(ult::greyColor);
    static Color ultPackageVersionTextColor = RGB888("00FFDD");
    static Color onTextColor = RGB888("00FFDD");
    static Color offTextColor = RGB888(ult::greyColor);
    
    #if IS_LAUNCHER_DIRECTIVE
    static Color dynamicLogoRGB1 = RGB888("00E669");
    static Color dynamicLogoRGB2 = RGB888("8080EA");
    #endif

    //static bool disableSelectionBG = false;
    //static bool disableSelectionValueColor = false;
    static bool invertBGClickColor = false;

    static size_t selectionBGAlpha = 11;
    static Color selectionBGColor = RGB888(ult::blackColor, selectionBGAlpha);

    static Color highlightColor1 = RGB888("2288CC");
    static Color highlightColor2 = RGB888("88FFFF");
    static Color highlightColor3 = RGB888("FFFF45");
    static Color highlightColor4 = RGB888("F7253E");

    static Color highlightColor = tsl::style::color::ColorHighlight;
    
    static size_t clickAlpha = 7;
    static Color clickColor = RGB888("3E25F7", clickAlpha);

    static size_t progressAlpha = 7;
    static Color progressColor = RGB888("253EF7", progressAlpha);

    static Color trackBarColor = RGB888("555555");

    static size_t separatorAlpha = 15;
    static Color separatorColor = RGB888("404040", separatorAlpha);
    static Color edgeSeparatorColor = RGB888("303030");

    static Color textSeparatorColor = RGB888("404040");

    static Color selectedTextColor = RGB888("9ed0ff");
    static Color selectedValueTextColor = RGB888("FF7777");
    static Color inprogressTextColor = RGB888(ult::whiteColor);
    static Color invalidTextColor = RGB888("FF0000");
    static Color clickTextColor = RGB888(ult::whiteColor);

    static size_t tableBGAlpha = 14;
    static Color tableBGColor = RGB888("2C2C2C", tableBGAlpha); //RGB888("303030", tableBGAlpha);
    static Color sectionTextColor = RGB888(ult::whiteColor);
    //static Color infoTextColor = RGB888("00FFDD");
    static Color infoTextColor =RGB888("9ed0ff");
    static Color warningTextColor = RGB888("FF7777");

    static Color healthyRamTextColor = RGB888("00FF00");
    static Color neutralRamTextColor = RGB888("FFAA00");
    static Color badRamTextColor = RGB888("FF0000");

    static Color trackBarSliderColor = RGB888("606060");
    static Color trackBarSliderBorderColor = RGB888("505050");
    static Color trackBarSliderMalleableColor = RGB888("A0A0A0");
    static Color trackBarFullColor = RGB888("00FFDD");
    static Color trackBarEmptyColor = RGB888("404040");
    
    static void initializeThemeVars() {
        auto themeData = ult::getParsedDataFromIniFile(ult::THEME_CONFIG_INI_PATH);
        if (themeData.count(ult::THEME_STR) == 0) return;
        
        auto& themeSection = themeData[ult::THEME_STR];
        
        auto getValue = [&](const char* key) -> const std::string& {
            auto it = themeSection.find(key);
            return it != themeSection.end() ? it->second : ult::defaultThemeSettingsMap[key];
        };
        
        auto getColor = [&](const char* key, size_t alpha = 15) {
            return RGB888(getValue(key), alpha);
        };
        
        auto getAlpha = [&](const char* key) {
            const auto& alphaStr = getValue(key);
            return ult::stoi(alphaStr);
        };
        
        #if IS_LAUNCHER_DIRECTIVE
        logoColor1 = getColor("logo_color_1");
        logoColor2 = getColor("logo_color_2");
        dynamicLogoRGB1 = getColor("dynamic_logo_color_1");
        dynamicLogoRGB2 = getColor("dynamic_logo_color_2");
        #endif
    
        defaultBackgroundAlpha = getAlpha("bg_alpha");
        defaultBackgroundColor = getColor("bg_color", defaultBackgroundAlpha);
        defaultTextColor = getColor("text_color");
        notificationTextColor = getColor("notification_text_color");
        headerTextColor = getColor("header_text_color");
        headerSeparatorColor = getColor("header_separator_color");
        starColor = getColor("star_color");
        selectionStarColor = getColor("selection_star_color");
        buttonColor = getColor("bottom_button_color");
        bottomTextColor = getColor("bottom_text_color");
        bottomSeparatorColor = getColor("bottom_separator_color");
        topSeparatorColor = getColor("top_separator_color");
        defaultOverlayColor = getColor("default_overlay_color");
        defaultPackageColor = getColor("default_package_color");
        defaultScriptColor = getColor("default_script_color");
        clockColor = getColor("clock_color");
        temperatureColor = getColor("temperature_color");
        batteryColor = getColor("battery_color");
        batteryChargingColor = getColor("battery_charging_color");
        batteryLowColor = getColor("battery_low_color");
        widgetBackdropAlpha = getAlpha("widget_backdrop_alpha");
        widgetBackdropColor = getColor("widget_backdrop_color", widgetBackdropAlpha);
        overlayTextColor = getColor("overlay_text_color");
        ultOverlayTextColor = getColor("ult_overlay_text_color");
        packageTextColor = getColor("package_text_color");
        ultPackageTextColor = getColor("ult_package_text_color");
        bannerVersionTextColor = getColor("banner_version_text_color");
        overlayVersionTextColor = getColor("overlay_version_text_color");
        ultOverlayVersionTextColor = getColor("ult_overlay_version_text_color");
        packageVersionTextColor = getColor("package_version_text_color");
        ultPackageVersionTextColor = getColor("ult_package_version_text_color");
        onTextColor = getColor("on_text_color");
        offTextColor = getColor("off_text_color");
        invertBGClickColor = (getValue("invert_bg_click_color") == ult::TRUE_STR);
        selectionBGAlpha = getAlpha("selection_bg_alpha");
        selectionBGColor = getColor("selection_bg_color", selectionBGAlpha);
        highlightColor1 = getColor("highlight_color_1");
        highlightColor2 = getColor("highlight_color_2");
        highlightColor3 = getColor("highlight_color_3");
        highlightColor4 = getColor("highlight_color_4");
        clickAlpha = getAlpha("click_alpha");
        clickColor = getColor("click_color", clickAlpha);
        progressAlpha = getAlpha("progress_alpha");
        progressColor = getColor("progress_color", progressAlpha);
        trackBarColor = getColor("trackbar_color");
        separatorAlpha = getAlpha("separator_alpha");
        separatorColor = getColor("separator_color", separatorAlpha);
        textSeparatorColor = getColor("text_separator_color");
        selectedTextColor = getColor("selection_text_color");
        selectedValueTextColor = getColor("selection_value_text_color");
        inprogressTextColor = getColor("inprogress_text_color");
        invalidTextColor = getColor("invalid_text_color");
        clickTextColor = getColor("click_text_color");
        tableBGAlpha = getAlpha("table_bg_alpha");
        tableBGColor = getColor("table_bg_color", tableBGAlpha);
        sectionTextColor = getColor("table_section_text_color");
        infoTextColor = getColor("table_info_text_color");
        warningTextColor = getColor("warning_text_color");
        healthyRamTextColor = getColor("healthy_ram_text_color");
        neutralRamTextColor = getColor("neutral_ram_text_color");
        badRamTextColor = getColor("bad_ram_text_color");
        trackBarSliderColor = getColor("trackbar_slider_color");
        trackBarSliderBorderColor = getColor("trackbar_slider_border_color");
        trackBarSliderMalleableColor = getColor("trackbar_slider_malleable_color");
        trackBarFullColor = getColor("trackbar_full_color");
        trackBarEmptyColor = getColor("trackbar_empty_color");
    }
    
    #if !IS_LAUNCHER_DIRECTIVE
    static void initializeUltrahandSettings() { // only needed for regular overlays
        // Load INI data once instead of 4 separate file reads
        auto ultrahandSection = ult::getKeyValuePairsFromSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME);
        
        // Helper lambda to safely get string values
        auto getStringValue = [&](const std::string& key, const std::string& defaultValue = "") -> std::string {
            if (ultrahandSection.count(key) > 0) {
                return ultrahandSection.at(key);
            }
            return defaultValue;
        };
        
        // Helper lambda to safely get boolean values
        auto getBoolValue = [&](const std::string& key, bool defaultValue = false) -> bool {
            if (ultrahandSection.count(key) > 0) {
                return (ultrahandSection.at(key) == ult::TRUE_STR);
            }
            return defaultValue;
        };
        
        // Get default language with fallback
        std::string defaultLang = getStringValue(ult::DEFAULT_LANG_STR, "en");
        if (defaultLang.empty()) {
            defaultLang = "en";
        }
        
        #ifdef UI_OVERRIDE_PATH
        
        std::string UI_PATH = UI_OVERRIDE_PATH;
        ult::preprocessPath(UI_PATH);
        const std::string NEW_THEME_CONFIG_INI_PATH = UI_PATH+"theme.ini";
        const std::string NEW_WALLPAPER_PATH = UI_PATH+"wallpaper.rgba";
                  
        const std::string TRANSLATION_JSON_PATH = UI_PATH+"lang/"+defaultLang+".json";
        if (ult::isFileOrDirectory(NEW_THEME_CONFIG_INI_PATH))
            ult::THEME_CONFIG_INI_PATH = NEW_THEME_CONFIG_INI_PATH; // Override theme path (optional)
        if (ult::isFileOrDirectory(NEW_WALLPAPER_PATH))
            ult::WALLPAPER_PATH = NEW_WALLPAPER_PATH; // Override wallpaper path (optional)
        if (ult::isFileOrDirectory(TRANSLATION_JSON_PATH))
            ult::loadTranslationsFromJSON(TRANSLATION_JSON_PATH); // load translations (optional)
        #endif
        
        // Set Ultrahand Globals using loaded section (defaults match initialization function)
        ult::useLaunchCombos = getBoolValue("launch_combos", true);       // TRUE_STR default
        ult::useNotifications = getBoolValue("notifications", true);       // TRUE_STR default
        if (ult::useNotifications && !ult::isFile(ult::NOTIFICATIONS_FLAG_FILEPATH)) {
            FILE* file = std::fopen((ult::NOTIFICATIONS_FLAG_FILEPATH).c_str(), "w");
            if (file) {
                std::fclose(file);
            }
        } else {
            ult::deleteFileOrDirectory(ult::NOTIFICATIONS_FLAG_FILEPATH);
        }
        ult::useSoundEffects = getBoolValue("sound_effects", false);
        ult::useHapticFeedback = getBoolValue("haptic_feedback", false);

        ult::useSwipeToOpen = getBoolValue("swipe_to_open", true);        // TRUE_STR default
        ult::useOpaqueScreenshots = getBoolValue("opaque_screenshots", true); // TRUE_STR default
        
        ultrahandSection.clear();

        const std::string langFile = ult::LANG_PATH+defaultLang+".json";
        if (ult::isFileOrDirectory(langFile))
            ult::parseLanguage(langFile);
    }
    #endif
    
    
    // Declarations
    
    /**
     * @brief Direction in which focus moved before landing on
     *        the currently focused element
     */
    enum class FocusDirection {
        None,                       ///< Focus was placed on the element programatically without user input
        Up,                         ///< Focus moved upwards
        Down,                       ///< Focus moved downwards
        Left,                       ///< Focus moved from left to rigth
        Right                       ///< Focus moved from right to left
    };
    
    /**
     * @brief Current input controll mode
     *
     */
    enum class InputMode {
        Controller,                 ///< Input from controller
        Touch,                      ///< Touch input
        TouchScroll                 ///< Moving/scrolling touch input
    };
    
    class Overlay;
    namespace elm { class Element; }
    
    namespace impl {
        
        /**
         * @brief Overlay launch parameters
         */
        enum class LaunchFlags : u8 {
            None = 0,                       ///< Do nothing special at launch
            CloseOnExit        = BIT(0)     ///< Close the overlay the last Gui gets poped from the stack
        };
        
        static constexpr LaunchFlags operator|(LaunchFlags lhs, LaunchFlags rhs) {
            return static_cast<LaunchFlags>(u8(lhs) | u8(rhs));
        }
        
        
        
    }
    
    static void goBack(u32 count = 1);

    static void pop(u32 count = 1);
    
    static void setNextOverlay(const std::string& ovlPath, std::string args = "");
    
    template<typename TOverlay, impl::LaunchFlags launchFlags = impl::LaunchFlags::CloseOnExit>
    int loop(int argc, char** argv);
    
    // Helpers
    
    namespace hlp {

        /**
         * @brief Wrapper for service initialization
         *
         * @param f wrapped function
         */
        template<typename F>
        static inline void doWithSmSession(F f) {
            smInitialize();
            f();
            smExit();
        }
        
        /**
         * @brief Wrapper for sd card access using stdio
         * @note Consider using raw fs calls instead as they are faster and need less space
         *
         * @param f wrapped function
         */
        template<typename F>
        static inline void doWithSDCardHandle(F f) {
            fsdevMountSdmc();
            f();
            fsdevUnmountDevice("sdmc");
        }
        
        /**
         * @brief Guard that will execute a passed function at the end of the current scope
         *
         * @param f wrapped function
         */
        template<typename F>
        class ScopeGuard {
            ScopeGuard(const ScopeGuard&) = delete;
            ScopeGuard& operator=(const ScopeGuard&) = delete;
            private:
                F f;
                bool canceled = false;
            public:
                ALWAYS_INLINE ScopeGuard(F f) : f(std::move(f)) { }
                ALWAYS_INLINE ~ScopeGuard() { if (!canceled) { f(); } }
                void dismiss() { canceled = true; }
        };
        
        /**
         * @brief libnx hid:sys shim that gives or takes away frocus to or from the process with the given aruid
         *
         * @param enable Give focus or take focus
         * @param aruid Aruid of the process to focus/unfocus
         * @return Result Result
         */
        static Result hidsysEnableAppletToGetInput(bool enable, u64 aruid) {
            const struct {
                u8 permitInput;
                u64 appletResourceUserId;
            } in = { enable != 0, aruid };
            
            return serviceDispatchIn(hidsysGetServiceSession(), 503, in);
        }
        
        static Result viAddToLayerStack(ViLayer *layer, ViLayerStack stack) {
            const struct {
                u32 stack;
                u64 layerId;
            } in = { stack, layer->layer_id };
            
            return serviceDispatchIn(viGetSession_IManagerDisplayService(), 6000, in);
        }

        /**
         * @brief Remove layer from layer stack
         */
        static Result viRemoveFromLayerStack(ViLayer *layer, ViLayerStack stack) {
            const struct {
                u32 stack;
                u64 layerId;
            } in = { stack, layer->layer_id };
            
            // Service command 6001 is commonly used for remove operations
            // If this doesn't work, try 6002, 6010, or other nearby values
            return serviceDispatchIn(viGetSession_IManagerDisplayService(), 6001, in);
        }
        
        /**
         * @brief Toggles focus between the Tesla overlay and the rest of the system
         *
         * @param enabled Focus Tesla?
         */
        static void requestForeground(bool enabled, bool updateGlobalFlag = true) {
            if (updateGlobalFlag)
                ult::currentForeground.store(enabled, std::memory_order_release);

            u64 applicationAruid = 0, appletAruid = 0;
            
            for (u64 programId = 0x0100000000001000UL; programId < 0x0100000000001020UL; programId++) {
                pmdmntGetProcessId(&appletAruid, programId);
                
                if (appletAruid != 0)
                    hidsysEnableAppletToGetInput(!enabled, appletAruid);
            }
            

            pmdmntGetApplicationProcessId(&applicationAruid);
            hidsysEnableAppletToGetInput(!enabled, applicationAruid);
            
            hidsysEnableAppletToGetInput(true, 0);
        }
        

        
        namespace ini {
            
            /**
             * @brief Ini file type
             */
            using IniData = std::map<std::string, std::map<std::string, std::string>>;
            
            /**
             * @brief Parses a ini string
             * 
             * @param str String to parse
             * @return Parsed data
             * // Modified to be "const std" instead of just "std"
             */
            static IniData parseIni(const std::string &str) {
                //IniData iniData;
                //
                //auto lines = split(str, '\n');
                //
                //std::string lastHeader = "";
                //for (auto& line : lines) {
                //    line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
                //    
                //    if (line[0] == '[' && line[line.size() - 1] == ']') {
                //        lastHeader = line.substr(1, line.size() - 2);
                //        iniData.emplace(lastHeader, std::map<std::string, std::string>{});
                //    }
                //    else if (auto keyValuePair = split(line, '='); keyValuePair.size() == 2) {
                //        iniData[lastHeader].emplace(keyValuePair[0], keyValuePair[1]);
                //    }
                //}
                
                return ult::parseIni(str);
            }
            
            /**
             * @brief Unparses ini data into a string
             *
             * @param iniData Ini data
             * @return Ini string
             */
            static std::string unparseIni(const IniData &iniData) {
                std::string result;
                bool addSectionGap = false;
            
                for (const auto &section : iniData) {
                    if (addSectionGap) {
                        result += '\n';
                    }
                    result += '[' + section.first + "]\n";
                    for (const auto &keyValue : section.second) {
                        result += keyValue.first + '=' + keyValue.second + '\n';
                    }
                    addSectionGap = true;
                }
            
                return result;
            }

            
            /**
             * @brief Read Tesla settings file
             *
             * @return Settings data
             */
            static IniData readOverlaySettings(auto& CONFIG_FILE) {
                /* Open Sd card filesystem. */
                FsFileSystem fsSdmc;
                if (R_FAILED(fsOpenSdCardFileSystem(&fsSdmc)))
                    return {};
                hlp::ScopeGuard fsGuard([&] { fsFsClose(&fsSdmc); });
                
                /* Open config file. */
                FsFile fileConfig;
                if (R_FAILED(fsFsOpenFile(&fsSdmc, CONFIG_FILE, FsOpenMode_Read, &fileConfig)))
                    return {};
                hlp::ScopeGuard fileGuard([&] { fsFileClose(&fileConfig); });
                
                /* Get config file size. */
                s64 configFileSize;
                if (R_FAILED(fsFileGetSize(&fileConfig, &configFileSize)))
                    return {};
                
                /* Read and parse config file. */
                std::string configFileData(configFileSize, '\0');
                u64 readSize;
                Result rc = fsFileRead(&fileConfig, 0, configFileData.data(), configFileSize, FsReadOption_None, &readSize);
                if (R_FAILED(rc) || readSize != static_cast<u64>(configFileSize))
                    return {};
                
                return ult::parseIni(configFileData);
            }
            
            /**
             * @brief Replace Tesla settings file with new data
             *
             * @param iniData new data
             */
            static void writeOverlaySettings(IniData const &iniData, auto& CONFIG_FILE) {
                /* Open Sd card filesystem. */
                FsFileSystem fsSdmc;
                if (R_FAILED(fsOpenSdCardFileSystem(&fsSdmc)))
                    return;
                hlp::ScopeGuard fsGuard([&] { fsFsClose(&fsSdmc); });
                
                /* Open config file. */
                FsFile fileConfig;
                if (R_FAILED(fsFsOpenFile(&fsSdmc, CONFIG_FILE, FsOpenMode_Write, &fileConfig)))
                    return;
                hlp::ScopeGuard fileGuard([&] { fsFileClose(&fileConfig); });
                
                const std::string iniString = unparseIni(iniData);
                
                fsFileWrite(&fileConfig, 0, iniString.c_str(), iniString.length(), FsWriteOption_Flush);
            }
            
            /**
             * @brief Merge and save changes into Tesla settings file
             *
             * @param changes setting values to add or update
             */
            static void updateOverlaySettings(IniData const &changes, auto& CONFIG_FILE) {
                hlp::ini::IniData iniData = hlp::ini::readOverlaySettings(CONFIG_FILE);
                for (auto &section : changes) {
                    for (auto &keyValue : section.second) {
                        iniData[section.first][keyValue.first] = keyValue.second;
                    }
                }
                writeOverlaySettings(iniData, CONFIG_FILE);
            }
            
        }
        
        /**
         * @brief Decodes a key string into it's key code
         *
         * @param value Key string
         * @return Key code
         */
        static u64 stringToKeyCode(const std::string& value) {
            for (const auto& keyInfo : ult::KEYS_INFO) {
                if (strcasecmp(value.c_str(), keyInfo.name) == 0)
                    return keyInfo.key;
            }
            return 0;
        }
        
        
        /**
         * @brief Decodes a combo string into key codes
         *
         * @param value Combo string
         * @return Key codes
         */
        static u64 comboStringToKeys(const std::string &value) {
            u64 keyCombo = 0x00;
            for (std::string key : ult::split(ult::removeWhiteSpaces(value), '+')) { // CUSTOM MODIFICATION (bug fix)
                keyCombo |= hlp::stringToKeyCode(key);
            }
            return keyCombo;
        }
        
        /**
         * @brief Encodes key codes into a combo string
         *
         * @param keys Key codes
         * @return Combo string
         */
        static std::string keysToComboString(u64 keys) {
            if (keys == 0) return "";  // Early return for empty input
        
            std::string result;
            bool first = true;
        
            for (const auto &keyInfo : ult::KEYS_INFO) {
                if (keys & keyInfo.key) {
                    if (!first) {
                        result += "+";
                    }
                    result += keyInfo.name;
                    first = false;
                }
            }
        
            return result;
        }

        inline static std::mutex comboMutex;

        // Function to load key combo mappings from both overlays.ini and packages.ini
        static void loadEntryKeyCombos() {
            std::lock_guard<std::mutex> lock(comboMutex);
            ult::g_entryCombos.clear();
        
            // Load overlay combos from overlays.ini
            auto overlayData = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
            std::string fullPath;
            u64 keys;

            std::vector<std::string> modeList, comboList;
            for (auto& [fileName, settings] : overlayData) {
                fullPath = ult::OVERLAY_PATH + fileName;
        
                // 1) main key_combo
                if (auto it = settings.find(ult::KEY_COMBO_STR); it != settings.end() && !it->second.empty()) {
                    keys = hlp::comboStringToKeys(it->second);
                    if (keys) ult::g_entryCombos[keys] = { fullPath, "" };
                }
        
                // 2) per-mode combos
                auto modesIt = settings.find("mode_args");
                auto argsIt  = settings.find("mode_combos");
                if (modesIt != settings.end()) {
                    modeList  = ult::splitIniList(modesIt->second);
                    comboList = (argsIt != settings.end())
                                   ? ult::splitIniList(argsIt->second)
                                   : std::vector<std::string>();
                    if (comboList.size() < modeList.size())
                        comboList.resize(modeList.size());
        
                    for (size_t i = 0; i < modeList.size(); ++i) {
                        const std::string& comboStr = comboList[i];
                        if (comboStr.empty()) continue;
                        keys = hlp::comboStringToKeys(comboStr);
                        if (!keys) continue;
                        // launchArg is the *mode* (i.e. modeList[i])
                        ult::g_entryCombos[keys] = { fullPath, modeList[i] };
                    }
                }
            }
        
            // Load package combos from packages.ini
            auto packageData = ult::getParsedDataFromIniFile(ult::PACKAGES_INI_FILEPATH);
            for (auto& [packageName, settings] : packageData) {
                // Only handle main key_combo for packages (no modes for packages)
                if (auto it = settings.find(ult::KEY_COMBO_STR); it != settings.end() && !it->second.empty()) {
                    keys = hlp::comboStringToKeys(it->second);
                    //std::string tmpPackageName = packageName;
                    //ult::removeQuotes(packageName);
                    if (keys) ult::g_entryCombos[keys] = { ult::OVERLAY_PATH + "ovlmenu.ovl", "--package " + packageName};
                }
            }
        }
        
        // Function to check if a key combination matches any overlay key combo
        static OverlayCombo getEntryForKeyCombo(u64 keys) {
            std::lock_guard<std::mutex> lock(comboMutex);
            if (auto it = ult::g_entryCombos.find(keys); it != ult::g_entryCombos.end())
                return it->second;
            return { "", "" };
        }

    }
    


    // Renderer
    
    namespace gfx {
        
        extern "C" u64 __nx_vi_layer_id;
        

        struct ScissoringConfig {
            u32 x, y, w, h, x_max, y_max;
        };
        

        // Forward declarations
        class Renderer;
        

        #ifdef UI_OVERRIDE_PATH
        inline static std::shared_mutex s_translationCacheMutex;
        #endif
        class FontManager {
        public:
            struct Glyph {
                stbtt_fontinfo *currFont;
                float currFontSize;
                int bounds[4];
                int xAdvance;
                u8 *glyphBmp;
                int width, height;
                
                // Add destructor to ensure cleanup
                ~Glyph() {
                    if (glyphBmp) {
                        stbtt_FreeBitmap(glyphBmp, nullptr);
                        glyphBmp = nullptr;
                    }
                }
                
                // Prevent copying to avoid double-free
                Glyph(const Glyph&) = delete;
                Glyph& operator=(const Glyph&) = delete;
                
                // Allow moving
                Glyph(Glyph&& other) noexcept 
                    : currFont(other.currFont), currFontSize(other.currFontSize)
                    , xAdvance(other.xAdvance), glyphBmp(other.glyphBmp)
                    , width(other.width), height(other.height) {
                    memcpy(bounds, other.bounds, sizeof(bounds));
                    other.glyphBmp = nullptr; // Prevent double-free
                }
                
                Glyph& operator=(Glyph&& other) noexcept {
                    if (this != &other) {
                        if (glyphBmp) {
                            stbtt_FreeBitmap(glyphBmp, nullptr);
                        }
                        currFont = other.currFont;
                        currFontSize = other.currFontSize;
                        xAdvance = other.xAdvance;
                        glyphBmp = other.glyphBmp;
                        width = other.width;
                        height = other.height;
                        memcpy(bounds, other.bounds, sizeof(bounds));
                        other.glyphBmp = nullptr;
                    }
                    return *this;
                }
                
                Glyph() : currFont(nullptr), currFontSize(0.0f), xAdvance(0), 
                          glyphBmp(nullptr), width(0), height(0) {
                    std::memset(bounds, 0, sizeof(bounds));
                }
            };

            struct FontMetrics {
                int ascent, descent, lineGap;
                int lineHeight; // ascent - descent + lineGap
                stbtt_fontinfo* font;
                float fontSize;
                
                FontMetrics() : ascent(0), descent(0), lineGap(0), lineHeight(0), font(nullptr), fontSize(0.0f) {}
                
                FontMetrics(stbtt_fontinfo* f, float size) : font(f), fontSize(size) {
                    if (font) {
                        stbtt_GetFontVMetrics(font, &ascent, &descent, &lineGap);
                        const float scale = stbtt_ScaleForPixelHeight(font, fontSize);
                        ascent = static_cast<int>(ascent * scale);
                        descent = static_cast<int>(descent * scale);
                        lineGap = static_cast<int>(lineGap * scale);
                        lineHeight = ascent - descent + lineGap;
                    } else {
                        ascent = descent = lineGap = lineHeight = 0;
                    }
                }
            };

            enum class CacheType {
                Regular,
                Notification,
                Persistent
            };
            
        private:
            inline static std::shared_mutex s_cacheMutex;
            inline static std::mutex s_initMutex;
            
            // Existing caches
            inline static std::unordered_map<u64, std::shared_ptr<Glyph>> s_sharedGlyphCache;
            //inline static std::unordered_map<u64, std::shared_ptr<Glyph>> s_persistentGlyphCache;
            
            // NEW: Notification-specific cache
            inline static std::unordered_map<u64, std::shared_ptr<Glyph>> s_notificationGlyphCache;
            
            // Font metrics cache
            inline static std::unordered_map<u64, FontMetrics> s_fontMetricsCache;
            
            // Add cache size limits
            static constexpr size_t MAX_CACHE_SIZE = 600;
            static constexpr size_t CLEANUP_THRESHOLD = 500;
            static constexpr size_t MAX_NOTIFICATION_CACHE_SIZE = 200; // Separate limit for notifications
            
            // font handles & state
            inline static stbtt_fontinfo* s_stdFont     = nullptr;
            inline static stbtt_fontinfo* s_localFont   = nullptr;
            inline static stbtt_fontinfo* s_extFont     = nullptr;
            inline static bool             s_hasLocalFont = false;
            inline static bool             s_initialized  = false;
            
            // Fix cache key generation to prevent collisions
            static u64 generateCacheKey(u32 character, bool monospace, u32 fontSize) {
                // Use more bits for fontSize and separate monospace bit
                u64 key = static_cast<u64>(character);
                key = (key << 32) | static_cast<u64>(fontSize);
                if (monospace) {
                    key |= (1ULL << 63); // Use the highest bit for monospace
                }
                return key;
            }

            // Generate cache key for font metrics
            static u64 generateFontMetricsCacheKey(stbtt_fontinfo* font, u32 fontSize) {
                // Use pointer address as font identifier and fontSize
                const u64 fontKey = reinterpret_cast<uintptr_t>(font);
                return (fontKey << 32) | static_cast<u64>(fontSize);
            }
            
            // Cleanup old entries when cache gets too large
            static void cleanupOldEntries() {
                if (s_sharedGlyphCache.size() <= CLEANUP_THRESHOLD) return;
                
                // Simple cleanup: remove oldest entries
                // In a real implementation, you might want LRU or other strategies
                const size_t toRemove = s_sharedGlyphCache.size() - CLEANUP_THRESHOLD;
                auto it = s_sharedGlyphCache.begin();
                for (size_t i = 0; i < toRemove && it != s_sharedGlyphCache.end(); ++i) {
                    it = s_sharedGlyphCache.erase(it);
                }
            }

            // NEW: Cleanup notification cache when it gets too large
            static void cleanupNotificationCache() {
                if (s_notificationGlyphCache.size() <= MAX_NOTIFICATION_CACHE_SIZE) return;
                
                const size_t toRemove = s_notificationGlyphCache.size() - (MAX_NOTIFICATION_CACHE_SIZE / 2);
                auto it = s_notificationGlyphCache.begin();
                for (size_t i = 0; i < toRemove && it != s_notificationGlyphCache.end(); ++i) {
                    it = s_notificationGlyphCache.erase(it);
                }
            }

            // NEW: Internal unified glyph creation method
            static std::shared_ptr<Glyph> getOrCreateGlyphInternal(u32 character, bool monospace, u32 fontSize, CacheType cacheType) {
                const u64 key = generateCacheKey(character, monospace, fontSize);
                
                // Select target cache based on type
                std::unordered_map<u64, std::shared_ptr<Glyph>>* targetCache;
                switch (cacheType) {
                    case CacheType::Notification:
                        targetCache = &s_notificationGlyphCache;
                        break;
                    //case CacheType::Persistent:
                    //    targetCache = &s_persistentGlyphCache;
                    //    break;
                    default:
                        targetCache = &s_sharedGlyphCache;
                        break;
                }
                
                // First, try to find in target cache with shared lock
                {
                    std::shared_lock<std::shared_mutex> readLock(s_cacheMutex);
                    
                    if (!s_initialized) return nullptr;
                    
                    // Check target cache first
                    auto it = targetCache->find(key);
                    if (it != targetCache->end()) {
                        return it->second;
                    }
                    
                    // For notification cache, also check persistent cache (but not regular cache)
                    // For regular cache, also check persistent cache (existing behavior)
                    //if (cacheType != CacheType::Persistent) {
                    //    auto persistentIt = s_persistentGlyphCache.find(key);
                    //    if (persistentIt != s_persistentGlyphCache.end()) {
                    //        return persistentIt->second;
                    //    }
                    //}
                }
                
                // Glyph not found, need to create it with exclusive lock
                std::unique_lock<std::shared_mutex> writeLock(s_cacheMutex);
                
                if (!s_initialized) return nullptr;
                
                // Double-check pattern for target cache
                auto it = targetCache->find(key);
                if (it != targetCache->end()) {
                    return it->second;
                }
                
                // Double-check persistent cache
                //if (cacheType != CacheType::Persistent) {
                //    auto persistentIt = s_persistentGlyphCache.find(key);
                //    if (persistentIt != s_persistentGlyphCache.end()) {
                //        return persistentIt->second;
                //    }
                //}
                
                // Check cache size and cleanup if needed
                if (cacheType == CacheType::Regular && s_sharedGlyphCache.size() >= MAX_CACHE_SIZE) {
                    cleanupOldEntries();
                } else if (cacheType == CacheType::Notification && s_notificationGlyphCache.size() >= MAX_NOTIFICATION_CACHE_SIZE) {
                    cleanupNotificationCache();
                }
                
                // Create new glyph
                auto glyph = std::make_shared<Glyph>();
                glyph->currFont = selectFontForCharacterUnsafe(character);
                if (!glyph->currFont) {
                    return nullptr;
                }
                
                glyph->currFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
                
                stbtt_GetCodepointBitmapBoxSubpixel(glyph->currFont, character, 
                    glyph->currFontSize, glyph->currFontSize, 0, 0, 
                    &glyph->bounds[0], &glyph->bounds[1], &glyph->bounds[2], &glyph->bounds[3]);
                
                s32 yAdvance = 0;
                stbtt_GetCodepointHMetrics(glyph->currFont, monospace ? 'W' : character, 
                                          &glyph->xAdvance, &yAdvance);
                
                glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont, 
                    glyph->currFontSize, glyph->currFontSize, character, 
                    &glyph->width, &glyph->height, nullptr, nullptr);
                
                // Store in target cache
                (*targetCache)[key] = glyph;
                
                return glyph;
            }
            
        public:
            // NEW: Preload and persist specific characters
            //static void preloadPersistentGlyphs(const std::string& characters, u32 fontSize, bool monospace = false) {
            //    std::unique_lock<std::shared_mutex> writeLock(s_cacheMutex);
            //    
            //    if (!s_initialized) return;
            //    
            //    // Convert UTF-8 string to UTF-32 codepoints
            //    #pragma GCC diagnostic push
            //    #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
            //    
            //    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
            //    const std::u32string codepoints = converter.from_bytes(characters);
            //    
            //    #pragma GCC diagnostic pop
            //    
            //    s32 yAdvance;
            //    for (char32_t character : codepoints) {
            //        const u64 key = generateCacheKey(character, monospace, fontSize);
            //
            //        if (s_persistentGlyphCache.find(key) != s_persistentGlyphCache.end()) {
            //            continue;
            //        }
            //
            //        auto glyph = std::make_shared<Glyph>();
            //        glyph->currFont = selectFontForCharacterUnsafe(character);
            //        if (!glyph->currFont) continue;
            //
            //        glyph->currFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
            //
            //        stbtt_GetCodepointBitmapBoxSubpixel(glyph->currFont, character,
            //                                            glyph->currFontSize, glyph->currFontSize, 0, 0,
            //                                            &glyph->bounds[0], &glyph->bounds[1], &glyph->bounds[2], &glyph->bounds[3]);
            //
            //        yAdvance = 0;
            //        stbtt_GetCodepointHMetrics(glyph->currFont, monospace ? 'W' : character,
            //                                  &glyph->xAdvance, &yAdvance);
            //
            //        glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont,
            //                                                   glyph->currFontSize, glyph->currFontSize, character,
            //                                                   &glyph->width, &glyph->height, nullptr, nullptr);
            //
            //        s_persistentGlyphCache[key] = glyph;
            //    }
            //}
        

            static void initializeFonts(stbtt_fontinfo* stdFont, stbtt_fontinfo* localFont, 
                                      stbtt_fontinfo* extFont, bool hasLocalFont) {
                std::lock_guard<std::mutex> initLock(s_initMutex);
                std::unique_lock<std::shared_mutex> cacheLock(s_cacheMutex);
                
                s_stdFont = stdFont;
                s_localFont = localFont;
                s_extFont = extFont;
                s_hasLocalFont = hasLocalFont;
                s_initialized = true;
            }
            
            static stbtt_fontinfo* selectFontForCharacter(u32 character) {
                std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
                
                if (!s_initialized) return nullptr;
                
                if (stbtt_FindGlyphIndex(s_extFont, character)) {
                    return s_extFont;
                } else if (s_hasLocalFont && stbtt_FindGlyphIndex(s_localFont, character) != 0) {
                    return s_localFont;
                }
                return s_stdFont;
            }

            // Get font metrics with caching
            static FontMetrics getFontMetrics(stbtt_fontinfo* font, u32 fontSize) {
                if (!font) return FontMetrics();

                const u64 key = generateFontMetricsCacheKey(font, fontSize);
                
                // First, try to find existing metrics with shared lock
                {
                    std::shared_lock<std::shared_mutex> readLock(s_cacheMutex);
                    auto it = s_fontMetricsCache.find(key);
                    if (it != s_fontMetricsCache.end()) {
                        return it->second;
                    }
                }
                
                // Metrics not found, need to create them with exclusive lock
                std::unique_lock<std::shared_mutex> writeLock(s_cacheMutex);
                
                // Double-check pattern
                auto it = s_fontMetricsCache.find(key);
                if (it != s_fontMetricsCache.end()) {
                    return it->second;
                }
                
                // Create new font metrics
                FontMetrics metrics(font, static_cast<float>(fontSize));
                s_fontMetricsCache[key] = metrics;
                
                return metrics;
            }

            // Convenience method to get font metrics for a character (selects appropriate font)
            static FontMetrics getFontMetricsForCharacter(u32 character, u32 fontSize) {
                stbtt_fontinfo* font = selectFontForCharacter(character);
                return getFontMetrics(font, fontSize);
            }
            
            // UPDATED: Regular glyph method - now uses internal method
            static std::shared_ptr<Glyph> getOrCreateGlyph(u32 character, bool monospace, u32 fontSize) {
                return getOrCreateGlyphInternal(character, monospace, fontSize, CacheType::Regular);
            }

            // NEW: Notification-specific glyph method
            static std::shared_ptr<Glyph> getOrCreateNotificationGlyph(u32 character, bool monospace, u32 fontSize) {
                return getOrCreateGlyphInternal(character, monospace, fontSize, CacheType::Notification);
            }

            // NEW: Clear only the notification cache
            static void clearNotificationCache() {
                std::unique_lock<std::shared_mutex> cacheLock(s_cacheMutex);
                s_notificationGlyphCache.clear();
                s_notificationGlyphCache.rehash(0);
            }
            
            static void clearCache() {
                // Note: This is now safe because any code holding a shared_ptr
                // will keep the Glyph alive even after the cache is cleared
                std::unique_lock<std::shared_mutex> cacheLock(s_cacheMutex);
                s_sharedGlyphCache.clear();
                s_sharedGlyphCache.rehash(0);
                s_fontMetricsCache.clear(); // Also clear font metrics cache
                s_fontMetricsCache.rehash(0);
            }

            static void clearAllCaches() {
                std::unique_lock<std::shared_mutex> cacheLock(s_cacheMutex);
                s_sharedGlyphCache.clear();
                s_sharedGlyphCache.rehash(0);
                //s_persistentGlyphCache.clear();
                //s_persistentGlyphCache.rehash(0);
                s_notificationGlyphCache.clear();
                s_notificationGlyphCache.rehash(0);
                s_fontMetricsCache.clear();
                s_fontMetricsCache.rehash(0);
            }
            
            static void cleanup() {
                std::lock_guard<std::mutex> initLock(s_initMutex);
                std::unique_lock<std::shared_mutex> cacheLock(s_cacheMutex);
                
                s_sharedGlyphCache.clear();
                s_sharedGlyphCache.rehash(0);
                //s_persistentGlyphCache.clear();
                //s_persistentGlyphCache.rehash(0);
                s_notificationGlyphCache.clear();
                s_notificationGlyphCache.rehash(0);
                s_fontMetricsCache.clear();
                s_initialized = false;
                s_stdFont = nullptr;
                s_localFont = nullptr;
                s_extFont = nullptr;
                s_hasLocalFont = false;
            }
            
            static size_t getCacheSize() {
                std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
                return s_sharedGlyphCache.size();
            }

            static size_t getFontMetricsCacheSize() {
                std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
                return s_fontMetricsCache.size();
            }
            
            static bool isInitialized() {
                std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
                return s_initialized;
            }

            //static size_t getPersistentCacheSize() {
            //    std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
            //    return s_persistentGlyphCache.size();
            //}

            // NEW: Get notification cache size
            static size_t getNotificationCacheSize() {
                std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
                return s_notificationGlyphCache.size();
            }
            
            // Add memory usage monitoring
            static size_t getMemoryUsage() {
                std::shared_lock<std::shared_mutex> lock(s_cacheMutex);
                size_t totalMemory = 0;
                
                // Regular cache
                for (const auto& pair : s_sharedGlyphCache) {
                    const auto& glyph = pair.second;
                    if (glyph && glyph->glyphBmp) {
                        totalMemory += glyph->width * glyph->height;
                    }
                }
                
                // Persistent cache
                //for (const auto& pair : s_persistentGlyphCache) {
                //    const auto& glyph = pair.second;
                //    if (glyph && glyph->glyphBmp) {
                //        totalMemory += glyph->width * glyph->height;
                //    }
                //}

                // Notification cache
                for (const auto& pair : s_notificationGlyphCache) {
                    const auto& glyph = pair.second;
                    if (glyph && glyph->glyphBmp) {
                        totalMemory += glyph->width * glyph->height;
                    }
                }
                
                return totalMemory;
            }
            
        private:
            static stbtt_fontinfo* selectFontForCharacterUnsafe(u32 character) {
                if (!s_initialized) return nullptr;
                
                if (stbtt_FindGlyphIndex(s_extFont, character)) {
                    return s_extFont;
                } else if (s_hasLocalFont && stbtt_FindGlyphIndex(s_localFont, character) != 0) {
                    return s_localFont;
                }
                return s_stdFont;
            }
        };

        
        // Static member definitions
        //std::shared_mutex FontManager::s_cacheMutex;
        //std::mutex FontManager::s_initMutex;
        //std::unordered_map<u64, std::unique_ptr<FontManager::Glyph>> FontManager::s_sharedGlyphCache;
        //stbtt_fontinfo* FontManager::s_stdFont = nullptr;
        //stbtt_fontinfo* FontManager::s_localFont = nullptr;
        //stbtt_fontinfo* FontManager::s_extFont = nullptr;
        //bool FontManager::s_hasLocalFont = false;
        //bool FontManager::s_initialized = false;
        
        // Updated thread-safe calculateStringWidth function
        static float calculateStringWidth(const std::string& originalString, const float fontSize, const bool monospace = false) {
            if (originalString.empty() || !FontManager::isInitialized()) {
                return 0.0f;
            }
            
            // Thread-safe translation cache access
            std::string text;
            #ifdef UI_OVERRIDE_PATH
            {
                std::shared_lock<std::shared_mutex> readLock(s_translationCacheMutex);
                auto translatedIt = ult::translationCache.find(originalString);
                if (translatedIt != ult::translationCache.end()) {
                    text = translatedIt->second;
                } else {
                    // Don't insert anything, just fallback to original string
                    text = originalString;
                }
            }
            #else
            text = originalString;
            #endif
            
            // CRITICAL: Use the same data types as drawString
            s32 maxWidth = 0;
            s32 currentLineWidth = 0;
            ssize_t codepointWidth;
            u32 currCharacter = 0;
            
            // Convert fontSize to u32 to match drawString behavior
            const u32 fontSizeInt = static_cast<u32>(fontSize);
            
            auto itStrEnd = text.cend();
            auto itStr = text.cbegin();
            
            // Fast ASCII check
            bool isAsciiOnly = true;
            for (unsigned char c : text) {
                if (c > 127) {
                    isAsciiOnly = false;
                    break;
                }
            }
            
            while (itStr != itStrEnd) {
                // Decode UTF-8 codepoint
                if (isAsciiOnly) {
                    currCharacter = static_cast<u32>(*itStr);
                    codepointWidth = 1;
                } else {
                    codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                    if (codepointWidth <= 0) break;
                }
                
                itStr += codepointWidth;
                
                // Handle newlines
                if (currCharacter == '\n') {
                    maxWidth = std::max(currentLineWidth, maxWidth);
                    currentLineWidth = 0;
                    continue;
                }
                
                // Use u32 fontSize to match drawString - now thread-safe
                std::shared_ptr<FontManager::Glyph> glyph = FontManager::getOrCreateGlyph(currCharacter, monospace, fontSizeInt);
                if (!glyph) continue;
                
                // CRITICAL: Use the same calculation as drawString
                currentLineWidth += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
            }
            
            // Final width calculation
            maxWidth = std::max(currentLineWidth, maxWidth);
            return static_cast<float>(maxWidth);
        }

        static std::pair<int, int> getUnderscanPixels();

        /**
         * @brief Manages the Tesla layer and draws raw data to the screen
         */
        class Renderer final {
        public:

            using Glyph = FontManager::Glyph;

            Renderer& operator=(Renderer&) = delete;
            
            friend class tsl::Overlay;
            
            /**
             * @brief Gets the renderer instance
             *
             * @return Renderer
             */
            inline static Renderer& get() {
                static Renderer renderer;
                
                return renderer;
            }
            
            stbtt_fontinfo m_stdFont, m_localFont, m_extFont;
            bool m_hasLocalFont = false;

            /**
             * @brief Handles opacity of drawn colors for fadeout. Pass all colors through this function in order to apply opacity properly
             *
             * @param c Original color
             * @return Color with applied opacity
             */
            static inline Color a(const Color& c) {
                const u8 opacity_limit = static_cast<u8>(0xF * Renderer::s_opacity);
                return (c.rgba & 0x0FFF) | (static_cast<u16>(
                    ult::disableTransparency
                        ? (ult::useOpaqueScreenshots
                               ? 0xF                       // fully opaque when both flags on
                               : (c.a > 0xE ? c.a : 0xE)) // clamp to 14, keep lower values
                        : (c.a < opacity_limit ? c.a : opacity_limit) // normal fade logic
                ) << 12);
            }

            static inline Color aWithOpacity(const Color& c) {
                const u8 opacity_limit = static_cast<u8>(0xF * Renderer::s_opacity);
                return (c.rgba & 0x0FFF) | (static_cast<u16>(
                    ult::disableTransparency
                        ? 0xF                       // fully opaque when both flags on
                        : (c.a < opacity_limit ? c.a : opacity_limit) // normal fade logic
                ) << 12);
            }

            static inline Color a2(const Color& c) {
                const u8 opacity_limit = static_cast<u8>(0xF);
                return (c.rgba & 0x0FFF) | (static_cast<u16>(
                    ult::disableTransparency
                        ? (ult::useOpaqueScreenshots
                               ? 0xF                       // fully opaque when both flags on
                               : (c.a > 0xE ? c.a : 0xE)) // clamp to 14, keep lower values
                        : (c.a < opacity_limit ? c.a : opacity_limit) // normal fade logic
                ) << 12);
            }
            
            /**
             * @brief Enables scissoring, discarding of any draw outside the given boundaries
             *
             * @param x x pos
             * @param y y pos
             * @param w Width
             * @param h Height
             */
            inline void enableScissoring(const u32 x, const u32 y, const u32 w, const u32 h) {
                this->m_scissoringStack.emplace(x, y, w, h, x+w, y+h);
            }
            
            /**
             * @brief Disables scissoring
             */
            inline void disableScissoring() {
                this->m_scissoringStack.pop();
            }
            
            
            // Drawing functions
            
            /**
             * @brief Draw a single pixel onto the screen
             *
             * @param x X pos
             * @param y Y pos
             * @param color Color
             */
            inline void setPixel(const u32 x, const u32 y, const Color& color) {
                const u32 offset = this->getPixelOffset(x, y);
                if (offset != UINT32_MAX) [[likely]] {
                    Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                    framebuffer[offset] = color;
                }
            }

            inline void setPixelAtOffset(const u32 offset, const Color& color) {
                Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                framebuffer[offset] = color;
            }


            
            /**
             * @brief Blends two colors
             *
             * @param src Source color
             * @param dst Destination color
             * @param alpha Opacity
             * @return Blended color
             */
            static constexpr u8 inv_alpha_table[16] = {15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
            
            inline u8 __attribute__((always_inline)) blendColor(const u8 src, const u8 dst, const u8 alpha) {
                return ((src * inv_alpha_table[alpha]) + (dst * alpha)) >> 4;
            }
            
            /**
             * @brief Draws a single source blended pixel onto the screen
             *
             * @param x X pos
             * @param y Y pos
             * @param color Color
             */
            inline void setPixelBlendSrc(const u32 x, const u32 y, const Color& color) {
                const u32 offset = this->getPixelOffset(x, y);
                if (offset == UINT32_MAX) [[unlikely]]
                    return;
                
                Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                const Color src = framebuffer[offset];
                
                // Direct write instead of calling setPixel
                framebuffer[offset] = Color(
                    blendColor(src.r, color.r, color.a),
                    blendColor(src.g, color.g, color.a),
                    blendColor(src.b, color.b, color.a),
                    src.a
                );
            }
            

            // Compromise version - keep framebuffer lookup but inline the rest
            inline void setPixelBlendDst(const u32 x, const u32 y, const Color& color) {
                const u32 offset = this->getPixelOffset(x, y);
                if (offset == UINT32_MAX) [[unlikely]]
                    return;
                
                Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                const Color src = framebuffer[offset];
                
                // Direct write instead of calling setPixel
                framebuffer[offset] = Color(
                    blendColor(src.r, color.r, color.a),
                    blendColor(src.g, color.g, color.a),
                    blendColor(src.b, color.b, color.a),
                    (color.a + (src.a * (0xF - color.a) >> 4))
                );
            }

            // Batch version for setPixelBlendDst
            inline void setPixelBlendDstBatch(const u32 baseX, const u32 baseY, 
                                              const u8 red[16], const u8 green[16], 
                                              const u8 blue[16], const u8 alpha[16], 
                                              const s32 count) {
                Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                
                for (s32 i = 0; i < count; ++i) {
                    // Early exit for transparent pixels
                    const u8 currentAlpha = alpha[i];
                    if (currentAlpha == 0) [[unlikely]]
                        continue;
                    
                    const u32 offset = this->getPixelOffset(baseX + i, baseY);
                    if (offset == UINT32_MAX) [[unlikely]]
                        continue;
                    
                    // Direct framebuffer read
                    const Color src = framebuffer[offset];
                    const u8 invAlpha = 0xF - currentAlpha;
                    
                    // Direct framebuffer write - skip setPixelAtOffset call
                    framebuffer[offset] = Color(
                        blendColor(src.r, red[i], currentAlpha),
                        blendColor(src.g, green[i], currentAlpha),
                        blendColor(src.b, blue[i], currentAlpha),
                        currentAlpha + ((src.a * invAlpha) >> 4)
                    );
                }
            }


            /**
             * @brief Draws a rectangle of given sizes
             *
             * @param x X pos
             * @param y Y pos
             * @param w Width
             * @param h Height
             * @param color Color
             */
            inline void drawRect(const s32 x, const s32 y, const s32 w, const s32 h, const Color& color) {
                // Early exit for invalid dimensions
                //if (w <= 0 || h <= 0) return;
                
                // Calculate clipped bounds
                const s32 x_start = x < 0 ? 0 : x;
                const s32 y_start = y < 0 ? 0 : y;
                const s32 x_end = (x + w > cfg::FramebufferWidth) ? cfg::FramebufferWidth : x + w;
                const s32 y_end = (y + h > cfg::FramebufferHeight) ? cfg::FramebufferHeight : y + h;
                
                // Early exit if completely outside bounds
                if (x_start >= x_end || y_start >= y_end) [[unlikely]] return;
                

                // Draw row by row for better cache locality
                for (s32 yi = y_start; yi < y_end; ++yi) {
                    for (s32 xi = x_start; xi < x_end; ++xi) {
                        this->setPixelBlendDst(xi, yi, color);
                    }
                }
            }

            /**
             * @brief Worker function for multithreaded rectangle drawing
             * @param x_start Start X coordinate
             * @param x_end End X coordinate  
             * @param y_start Start Y coordinate for this thread
             * @param y_end End Y coordinate for this thread
             * @param color Color to draw
             */
            inline void processRectChunk(const s32 x_start, const s32 x_end, const s32 y_start, const s32 y_end, const Color& color) {
                for (s32 yi = y_start; yi < y_end; ++yi) {
                    for (s32 xi = x_start; xi < x_end; ++xi) {
                        this->setPixelBlendDst(xi, yi, color);
                    }
                }
            }
        

            /**
             * @brief Draws a rectangle of given sizes (Multi-threaded)
             *
             * @param x X pos
             * @param y Y pos
             * @param w Width
             * @param h Height
             * @param color Color
             */
            inline void drawRectMultiThreaded(const s32 x, const s32 y, const s32 w, const s32 h, const Color& color) {
                // Early exit for invalid dimensions
                if (w <= 0 || h <= 0) return;
                
                // Calculate clipped bounds
                const s32 x_start = x < 0 ? 0 : x;
                const s32 y_start = y < 0 ? 0 : y;
                const s32 x_end = (x + w > cfg::FramebufferWidth) ? cfg::FramebufferWidth : x + w;
                const s32 y_end = (y + h > cfg::FramebufferHeight) ? cfg::FramebufferHeight : y + h;
                
                // Early exit if completely outside bounds
                if (x_start >= x_end || y_start >= y_end) return;
                
                // Calculate visible dimensions
                const s32 visibleHeight = y_end - y_start;
                
                // Calculate chunk size - divide rows among threads
                const s32 chunkSize = std::max(1, visibleHeight / static_cast<s32>(ult::numThreads));
                
                // Launch threads using ult::renderThreads array
                for (unsigned i = 0; i < static_cast<unsigned>(ult::numThreads); ++i) {
                    const s32 startRow = y_start + (i * chunkSize);
                    const s32 endRow = (i == static_cast<unsigned>(ult::numThreads) - 1) ? 
                                      y_end : 
                                      std::min(startRow + chunkSize, y_end);
                    
                    // Skip threads that have no work
                    if (startRow >= endRow) {
                        ult::renderThreads[i] = std::thread([](){}); // Empty thread (still needed for joining)
                        continue;
                    }
                    
                    // Use member function instead of lambda - much faster
                    ult::renderThreads[i] = std::thread(&Renderer::processRectChunk, this, 
                                                       x_start, x_end, startRow, endRow, color);
                }
                
                // Join all ult::renderThreads
                for (auto& t : ult::renderThreads) {
                    t.join();
                }
            }


            /**
             * @brief Draws a rectangle of given sizes with empty filling
             * 
             * @param x X pos 
             * @param y Y pos
             * @param w Width
             * @param h Height
             * @param color Color
             */
            inline void drawEmptyRect(s32 x, s32 y, s32 w, s32 h, Color color) {
                // Only precompute values that are actually reused
                const s32 x_end = x + w - 1;
                const s32 y_end = y + h - 1;
                
                // Early exit for completely out-of-bounds rectangles
                if (x_end < 0 || y_end < 0 || x >= cfg::FramebufferWidth || y >= cfg::FramebufferHeight) [[unlikely]] {
                    return;
                }
                
                // Early exit for degenerate rectangles
                //if (w <= 0 || h <= 0) {
                //    return;
                //}
                
                // These are reused for both horizontal lines
                const s32 line_x_start = x < 0 ? 0 : x;
                const s32 line_x_end = x_end >= cfg::FramebufferWidth ? cfg::FramebufferWidth - 1 : x_end;
                
                // Draw top horizontal line
                if (y >= 0 && y < cfg::FramebufferHeight) {
                    for (s32 xi = line_x_start; xi <= line_x_end; ++xi) {
                        this->setPixelBlendDst(xi, y, color);
                    }
                }
                
                // Draw bottom horizontal line (only if different from top)
                if (h > 1 && y_end >= 0 && y_end < cfg::FramebufferHeight) {
                    for (s32 xi = line_x_start; xi <= line_x_end; ++xi) {
                        this->setPixelBlendDst(xi, y_end, color);
                    }
                }
                
                // Draw vertical lines only if there's space between horizontal lines
                if (h > 2) {
                    // These are reused for both vertical lines
                    const s32 line_y_start = (y + 1) < 0 ? 0 : (y + 1);
                    const s32 line_y_end = (y_end - 1) >= cfg::FramebufferHeight ? cfg::FramebufferHeight - 1 : (y_end - 1);
                    
                    // Only proceed if there are actually vertical pixels to draw
                    if (line_y_start <= line_y_end) {
                        // Left vertical line
                        if (x >= 0 && x < cfg::FramebufferWidth) {
                            for (s32 yi = line_y_start; yi <= line_y_end; ++yi) {
                                this->setPixelBlendDst(x, yi, color);
                            }
                        }
                        
                        // Right vertical line (only if different from left)
                        if (w > 1 && x_end >= 0 && x_end < cfg::FramebufferWidth) {
                            for (s32 yi = line_y_start; yi <= line_y_end; ++yi) {
                                this->setPixelBlendDst(x_end, yi, color);
                            }
                        }
                    }
                }
            }

            /**
             * @brief Draws a line
             * 
             * @param x0 Start X pos 
             * @param y0 Start Y pos
             * @param x1 End X pos
             * @param y1 End Y pos
             * @param color Color
             */
            inline void drawLine(s32 x0, s32 y0, s32 x1, s32 y1, Color color) {
                // Early exit for single point
                if (x0 == x1 && y0 == y1) {
                    if (x0 >= 0 && y0 >= 0 && x0 < cfg::FramebufferWidth && y0 < cfg::FramebufferHeight) {
                        this->setPixelBlendDst(x0, y0, color);
                    }
                    return;
                }
                
                // Calculate deltas
                const s32 dx = x1 - x0;
                const s32 dy = y1 - y0;
                
                // Calculate absolute deltas and steps
                const s32 abs_dx = dx < 0 ? -dx : dx;
                const s32 abs_dy = dy < 0 ? -dy : dy;
                const s32 step_x = dx < 0 ? -1 : 1;
                const s32 step_y = dy < 0 ? -1 : 1;
                
                // Bresenham's algorithm
                s32 x = x0, y = y0;
                s32 error = abs_dx - abs_dy;
                s32 error2;

                while (true) {
                    // Bounds check and draw pixel
                    if (x >= 0 && y >= 0 && x < cfg::FramebufferWidth && y < cfg::FramebufferHeight) {
                        this->setPixelBlendDst(x, y, color);
                    }
                    
                    // Check if we've reached the end point
                    if (x == x1 && y == y1) break;
                    
                    // Calculate error and step
                    error2 = error << 1;  // error * 2
                    
                    if (error2 > -abs_dy) {
                        error -= abs_dy;
                        x += step_x;
                    }
                    if (error2 < abs_dx) {
                        error += abs_dx;
                        y += step_y;
                    }
                }
            }

            /**
             * @brief Draws a dashed line
             * 
             * @param x0 Start X pos 
             * @param y0 Start Y pos
             * @param x1 End X pos
             * @param y1 End Y pos
             * @param line_width How long one line can be
             * @param color Color
             */
            inline void drawDashedLine(s32 x0, s32 y0, s32 x1, s32 y1, s32 line_width, Color color) {
                // Source of formula: https://www.cc.gatech.edu/grads/m/Aaron.E.McClennen/Bresenham/code.html

                const s32 x_min = std::min(x0, x1);
                const s32 x_max = std::max(x0, x1);
                const s32 y_min = std::min(y0, y1);
                const s32 y_max = std::max(y0, y1);

                if (x_min < 0 || y_min < 0 || x_min >= cfg::FramebufferWidth || y_min >= cfg::FramebufferHeight)
                    return;

                const s32 dx = x_max - x_min;
                const s32 dy = y_max - y_min;
                s32 d = 2 * dy - dx;

                const s32 incrE = 2*dy;
                const s32 incrNE = 2*(dy - dx);

                this->setPixelBlendDst(x_min, y_min, color);

                s32 x = x_min;
                s32 y = y_min;
                s32 rendered = 0;

                while(x < x1) {
                    if (d <= 0) {
                        d += incrE;
                        x++;
                    }
                    else {
                        d += incrNE;
                        x++;
                        y++;
                    }
                    rendered++;
                    if (x < 0 || y < 0 || x >= cfg::FramebufferWidth || y >= cfg::FramebufferHeight)
                        continue;
                    if (x <= x_max && y <= y_max) {
                        if (rendered > 0 && rendered < line_width) {
                            this->setPixelBlendDst(x, y, color);
                        }
                        else if (rendered > 0 && rendered >= line_width) {
                            rendered *= -1;
                        }
                    }
                } 
                    
            }
            
            inline void drawCircle(const s32 centerX, const s32 centerY, const u16 radius, const bool filled, const Color& color) {
                const float r_f = static_cast<float>(radius);
                const float r2 = r_f * r_f;
                const u8 base_a = color.a;
                const bool full_opacity = (base_a == 0xFF);
                
                const s32 bound = radius + 2;
                const s32 clip_left = std::max(0, centerX - bound);
                const s32 clip_right = std::min(static_cast<s32>(cfg::FramebufferWidth), centerX + bound);
                const s32 clip_top = std::max(0, centerY - bound);
                const s32 clip_bottom = std::min(static_cast<s32>(cfg::FramebufferHeight), centerY + bound);
                
                // 8-sample pattern (better than 4-sample, captures diagonals better)
                const float offset = 0.353553f; // sqrt(2)/4, keeps samples equidistant
                const float samples[8][2] = {
                    {-offset, -offset}, {offset, -offset},  // Diagonal
                    {-offset, offset},  {offset, offset},   // Diagonal
                    {-0.5f, 0.0f},     {0.5f, 0.0f},       // Horizontal edge
                    {0.0f, -0.5f},     {0.0f, 0.5f}        // Vertical edge
                };
                
                for (s32 yc = clip_top; yc < clip_bottom; ++yc) {
                    const float py = static_cast<float>(yc - centerY) + 0.5f;
                    const float py_sq = py * py;
                    
                    for (s32 xc = clip_left; xc < clip_right; ++xc) {
                        const float px = static_cast<float>(xc - centerX) + 0.5f;
                        const float px_sq = px * px;
                        const float center_d2 = px_sq + py_sq;
                        
                        // Quick reject for pixels far from edge
                        if (filled) {
                            if (center_d2 <= r2 - r_f) {
                                // Definitely inside
                                const u32 off = this->getPixelOffset(xc, yc);
                                if (off != UINT32_MAX) {
                                    if (full_opacity) this->setPixelAtOffset(off, color);
                                    else this->setPixelBlendDst(xc, yc, color);
                                }
                                continue;
                            } else if (center_d2 > r2 + r_f) {
                                // Definitely outside
                                continue;
                            }
                            
                            // On the edge - use 8-sample supersampling
                            u32 inside_count = 0;
                            for (u32 s = 0; s < 8; ++s) {
                                const float sx = px + samples[s][0];
                                const float sy = py + samples[s][1];
                                if (sx*sx + sy*sy <= r2) {
                                    inside_count++;
                                }
                            }
                            
                            if (inside_count > 0) {
                                const u32 off = this->getPixelOffset(xc, yc);
                                if (off != UINT32_MAX) {
                                    Color c = color;
                                    c.a = static_cast<u8>((base_a * inside_count + 4) / 8); // +4 for rounding
                                    this->setPixelBlendDst(xc, yc, c);
                                }
                            }
                        } else {
                            // Outline
                            const float inner_r2 = (r_f - 1.0f) * (r_f - 1.0f);
                            
                            if (center_d2 >= inner_r2 + r_f && center_d2 <= r2 - r_f) {
                                // Definitely in ring
                                const u32 off = this->getPixelOffset(xc, yc);
                                if (off != UINT32_MAX) {
                                    if (full_opacity) this->setPixelAtOffset(off, color);
                                    else this->setPixelBlendDst(xc, yc, color);
                                }
                                continue;
                            } else if (center_d2 < inner_r2 - r_f || center_d2 > r2 + r_f) {
                                // Definitely outside ring
                                continue;
                            }
                            
                            // On edge - use 8-sample supersampling
                            u32 inside_count = 0;
                            for (u32 s = 0; s < 8; ++s) {
                                const float sx = px + samples[s][0];
                                const float sy = py + samples[s][1];
                                const float sd2 = sx*sx + sy*sy;
                                if (sd2 >= inner_r2 && sd2 <= r2) {
                                    inside_count++;
                                }
                            }
                            
                            if (inside_count > 0) {
                                const u32 off = this->getPixelOffset(xc, yc);
                                if (off != UINT32_MAX) {
                                    Color c = color;
                                    c.a = static_cast<u8>((base_a * inside_count + 4) / 8);
                                    this->setPixelBlendDst(xc, yc, c);
                                }
                            }
                        }
                    }
                }
            }
            
            inline void drawBorderedRoundedRect(const s32 x, const s32 y, const s32 width, const s32 height, const s32 thickness, const s32 radius, const Color& highlightColor) {
                const s32 startX = x + 4;
                const s32 startY = y;
                const s32 adjustedWidth = width - 12;
                const s32 adjustedHeight = height + 1;
                
                // Pre-calculate corner positions
                const s32 leftCornerX = startX;
                const s32 rightCornerX = x + width - 9;
                const s32 topCornerY = startY;
                const s32 bottomCornerY = startY + height;
                
                // Draw borders
                this->drawRect(startX, startY - thickness, adjustedWidth, thickness, highlightColor);
                this->drawRect(startX, startY + adjustedHeight, adjustedWidth, thickness, highlightColor);
                this->drawRect(startX - thickness, startY, thickness, adjustedHeight, highlightColor);
                this->drawRect(startX + adjustedWidth, startY, thickness, adjustedHeight, highlightColor);
                
                // Pre-calculate AA colors once
                const Color aaColor1 = {highlightColor.r, highlightColor.g, highlightColor.b, static_cast<u8>(highlightColor.a >> 1)};  // 50%
                const Color aaColor2 = {highlightColor.r, highlightColor.g, highlightColor.b, static_cast<u8>(highlightColor.a >> 2)};  // 25%
                
                // Circle drawing with AA - optimized Bresenham
                s32 cx = radius;
                s32 cy = 0;
                s32 radiusError = 0;
                const s32 diameter = radius << 1;
                s32 xChange = 1 - diameter;
                s32 yChange = 0;
                s32 lastCx = cx;
                
                while (cx >= cy) {
                    // Pre-calculate Y coordinates (hoist invariants)
                    const s32 topY1 = topCornerY - cy;
                    const s32 topY2 = topCornerY - cx;
                    const s32 bottomY1 = bottomCornerY + cy;
                    const s32 bottomY2 = bottomCornerY + cx;
                    
                    // Pre-calculate X bounds
                    const s32 leftX1Start = leftCornerX - cx;
                    const s32 leftX2Start = leftCornerX - cy;
                    const s32 rightX1Start = rightCornerX + 1;
                    const s32 rightX1End = rightCornerX + cx;
                    const s32 rightX2End = rightCornerX + cy;
                    
                    // Draw filled spans - NOW PERFECTLY MIRRORED
                    // Upper-left corner (exclusive)
                    for (s32 i = leftX1Start; i < leftCornerX; i++) {
                        this->setPixelBlendDst(i, topY1, highlightColor);
                    }
                    for (s32 i = leftX2Start; i < leftCornerX; i++) {
                        this->setPixelBlendDst(i, topY2, highlightColor);
                    }
                    
                    // Lower-left corner (NOW exclusive like top)
                    for (s32 i = leftX1Start; i < leftCornerX; i++) {
                        this->setPixelBlendDst(i, bottomY1, highlightColor);
                    }
                    for (s32 i = leftX2Start; i < leftCornerX; i++) {
                        this->setPixelBlendDst(i, bottomY2, highlightColor);
                    }
                    
                    // Upper-right corner (starts at +1)
                    for (s32 i = rightX1Start; i <= rightX1End; i++) {
                        this->setPixelBlendDst(i, topY1, highlightColor);
                    }
                    for (s32 i = rightX1Start; i <= rightX2End; i++) {
                        this->setPixelBlendDst(i, topY2, highlightColor);
                    }
                    
                    // Lower-right corner (NOW starts at +1 like top)
                    for (s32 i = rightX1Start; i <= rightX1End; i++) {
                        this->setPixelBlendDst(i, bottomY1, highlightColor);
                    }
                    for (s32 i = rightX1Start; i <= rightX2End; i++) {
                        this->setPixelBlendDst(i, bottomY2, highlightColor);
                    }
                    
                    // Add AA at step transitions
                    if (__builtin_expect(cx != lastCx && cy > 0, 0)) {
                        // Pre-calculate AA pixel positions
                        const s32 cxAA = cx + 1;
                        
                        // Upper-left AA
                        this->setPixelBlendDst(leftCornerX - cxAA, topY1, aaColor1);
                        this->setPixelBlendDst(leftCornerX - cxAA, topY1 + 1, aaColor2);
                        this->setPixelBlendDst(leftX2Start, topY2 - 1, aaColor1);
                        this->setPixelBlendDst(leftX2Start + 1, topY2 - 1, aaColor2);
                        
                        // Upper-right AA
                        this->setPixelBlendDst(rightCornerX + cxAA, topY1, aaColor1);
                        this->setPixelBlendDst(rightCornerX + cxAA, topY1 + 1, aaColor2);
                        this->setPixelBlendDst(rightX2End, topY2 - 1, aaColor1);
                        this->setPixelBlendDst(rightX2End - 1, topY2 - 1, aaColor2);
                        
                        // Lower-left AA
                        this->setPixelBlendDst(leftCornerX - cxAA, bottomY1, aaColor1);
                        this->setPixelBlendDst(leftCornerX - cxAA, bottomY1 - 1, aaColor2);
                        this->setPixelBlendDst(leftX2Start, bottomY2 + 1, aaColor1);
                        this->setPixelBlendDst(leftX2Start + 1, bottomY2 + 1, aaColor2);
                        
                        // Lower-right AA
                        this->setPixelBlendDst(rightCornerX + cxAA, bottomY1, aaColor1);
                        this->setPixelBlendDst(rightCornerX + cxAA, bottomY1 - 1, aaColor2);
                        this->setPixelBlendDst(rightX2End, bottomY2 + 1, aaColor1);
                        this->setPixelBlendDst(rightX2End - 1, bottomY2 + 1, aaColor2);
                    }
                    
                    lastCx = cx;
                    
                    // Bresenham iteration - optimized
                    cy++;
                    radiusError += yChange;
                    yChange += 2;
                    
                    if (__builtin_expect(((radiusError << 1) + xChange) > 0, 0)) {
                        cx--;
                        radiusError += xChange;
                        xChange += 2;
                    }
                }
            }
            
            // Pre-compute all horizontal spans for the entire shape
            struct HorizontalSpan {
                s32 start_x, end_x;
            };
            
            // Helper function - defined outside, compiler will inline
            static inline void sampleAndBlendArcPixel(Renderer* self, s32 xp, s32 yc, 
                                                      int px2, int cx2, int sx, int py2, int cy2, int sy,
                                                      long long r2_scaled, const Color& color, u8 base_a)
            {
                int hits = 0;
                long long dx1 = px2 + sx - cx2;
                long long dx2 = px2 - sx - cx2;
                long long dy1 = py2 + sy - cy2;
                long long dy2 = py2 - sy - cy2;
                
                if (dx1*dx1 + dy1*dy1 <= r2_scaled) ++hits;
                if (dx1*dx1 + dy2*dy2 <= r2_scaled) ++hits;
                if (dx2*dx2 + dy1*dy1 <= r2_scaled) ++hits;
                if (dx2*dx2 + dy2*dy2 <= r2_scaled) ++hits;
                
                if (hits == 4) {
                    self->setPixelBlendDst(xp, yc, color);
                } else if (hits > 0) {
                    u8 a = (base_a * hits + 2) >> 2;
                    if (a) {
                        Color c = color;
                        c.a = a;
                        self->setPixelBlendDst(xp, yc, c);
                    }
                }
            }
            
            static void processRoundedRectChunk(Renderer* self, const s32 x, const s32 y, const s32 w, const s32 h,
                                                const s32 radius, const Color& color,
                                                const s32 startRow, const s32 endRow)
            {
                if (radius <= 0) return;
            
                const s32 x_end = x + w;
                const s32 y_end = y + h;
            
                const s32 clip_x     = std::max(0, x);
                const s32 clip_x_end = std::min<s32>(cfg::FramebufferWidth, x_end);
            
                const s32 left_arc_end    = x + radius - 1;
                const s32 right_arc_start = x_end - radius;
                const s32 top_arc_end     = y + radius - 1;
                const s32 bottom_arc_start = y_end - radius;
            
                const int cx2_left  = 2 * (x + radius);
                const int cx2_right = 2 * (x_end - radius);
                const int cy2_top    = 2 * (y + radius);
                const int cy2_bottom = 2 * (y_end - radius);
            
                const long long r2_scaled = 4LL * radius * radius;
                const long long reject_threshold = (2LL*radius + 2)*(2LL*radius + 2);
            
                const u8 base_a = color.a;
            
                // Pre-compute sample offsets (constant per corner)
                const int sx_left   = ((x + radius)     & 1) ? -1 : 1;
                const int sx_right  = ((x_end - radius) & 1) ? -1 : 1;
                const int sy_top    = ((y + radius)     & 1) ? -1 : 1;
                const int sy_bottom = ((y_end - radius) & 1) ? -1 : 1;
            
                alignas(64) u8 redArray[512], greenArray[512], blueArray[512], alphaArray[512];
                const uint8x16_t rv = vdupq_n_u8(color.r);
                const uint8x16_t gv = vdupq_n_u8(color.g);
                const uint8x16_t bv = vdupq_n_u8(color.b);
                const uint8x16_t av = vdupq_n_u8(color.a);
                for (int i = 0; i < 512; i += 16) {
                    vst1q_u8(redArray + i, rv);
                    vst1q_u8(greenArray + i, gv);
                    vst1q_u8(blueArray + i, bv);
                    vst1q_u8(alphaArray + i, av);
                }
            
                for (s32 yc = startRow; yc < endRow; ++yc) {
                    if (yc < y || yc >= y_end) continue;
            
                    const bool is_top = (yc <= top_arc_end);
                    const bool in_arc_rows = is_top || (yc >= bottom_arc_start);
                    
                    if (!in_arc_rows) {
                        s32 xs = std::max(clip_x, x);
                        s32 xe = std::min(clip_x_end, x_end);
                        for (s32 xp = xs; xp < xe; xp += 512)
                            self->setPixelBlendDstBatch(xp, yc, redArray, greenArray, blueArray, alphaArray,
                                                        std::min(512, xe - xp));
                        continue;
                    }
            
                    const int cy2 = is_top ? cy2_top : cy2_bottom;
                    const int py2 = 2 * yc + 1;
                    const int sy = is_top ? sy_top : sy_bottom;
            
                    // Quick row reject
                    const long long dy = py2 - cy2;
                    if (dy * dy > reject_threshold) continue;
            
                    const s32 xe = std::min(clip_x_end, x_end);
                    s32 xp = std::max(clip_x, x);
            
                    // Left arc
                    for (; xp <= left_arc_end && xp < xe; ++xp) {
                        sampleAndBlendArcPixel(self, xp, yc, 2*xp + 1, cx2_left, sx_left, 
                                               py2, cy2, sy, r2_scaled, color, base_a);
                    }
            
                    // Middle flat
                    s32 mid_start = std::max(xp, left_arc_end + 1);
                    s32 mid_end   = std::min(xe, right_arc_start);
                    if (mid_start < mid_end) {
                        for (s32 bx = mid_start; bx < mid_end; bx += 512)
                            self->setPixelBlendDstBatch(bx, yc, redArray, greenArray, blueArray, alphaArray,
                                                        std::min(512, mid_end - bx));
                    }
            
                    // Right arc
                    xp = std::max(xp, right_arc_start);
                    for (; xp < xe; ++xp) {
                        sampleAndBlendArcPixel(self, xp, yc, 2*xp + 1, cx2_right, sx_right,
                                               py2, cy2, sy, r2_scaled, color, base_a);
                    }
                }
            }


            /**
             * @brief Draws a rounded rectangle of given sizes and corner radius (Multi-threaded)
             *
             * @param x X pos
             * @param y Y pos
             * @param w Width
             * @param h Height
             * @param radius Corner radius
             * @param color Color
             */
            inline void drawRoundedRectMultiThreaded(const s32 x, const s32 y, const s32 w, const s32 h, const s32 radius, const Color& color) {
                if (w <= 0 || h <= 0) return;
                
                // Get framebuffer bounds for early exit check
                //const s32 fb_width = static_cast<s32>(cfg::FramebufferWidth);
                //const s32 fb_height = static_cast<s32>(cfg::FramebufferHeight);
                
                // Calculate clipped bounds for early exit check
                const s32 clampedX = std::max(0, x);
                const s32 clampedY = std::max(0, y);
                const s32 clampedXEnd = std::min(static_cast<s32>(cfg::FramebufferWidth), x + w);
                const s32 clampedYEnd = std::min(static_cast<s32>(cfg::FramebufferHeight), y + h);
                
                // Early exit if nothing to draw after clamping
                if (clampedX >= clampedXEnd || clampedY >= clampedYEnd) return;
                
                // Calculate visible dimensions
                const s32 visibleHeight = clampedYEnd - clampedY;
                
                // Dynamic chunk size based on visible rectangle height
                const s32 chunkSize = std::max(1, visibleHeight / (static_cast<s32>(ult::numThreads) * 2));
                std::atomic<s32> currentRow(clampedY);
                
                auto threadTask = [&]() {
                    s32 startRow, endRow;
                    while ((startRow = currentRow.fetch_add(chunkSize)) < clampedYEnd) {
                        endRow = std::min(startRow + chunkSize, clampedYEnd);
                        processRoundedRectChunk(this, x, y, w, h, radius, color, startRow, endRow);
                    }
                };
                
                // Launch threads using ult::renderThreads array
                for (unsigned i = 0; i < static_cast<unsigned>(ult::numThreads); ++i) {
                    ult::renderThreads[i] = std::thread(threadTask);
                }
                
                // Join all ult::renderThreads
                for (auto& t : ult::renderThreads) {
                    t.join();
                }
            }
            
            /**
             * @brief Draws a rounded rectangle of given sizes and corner radius (Single-threaded)
             *
             * @param x X pos
             * @param y Y pos
             * @param w Width
             * @param h Height
             * @param radius Corner radius
             * @param color Color
             */
            inline void drawRoundedRectSingleThreaded(s32 x, s32 y, s32 w, s32 h, s32 radius, const Color& color) {
                if (w <= 0 || h <= 0) return;
            
                const s32 clampedY = std::max(0, y);
                const s32 clampedYEnd = std::min(static_cast<s32>(cfg::FramebufferHeight), y + h);
                //const s32 clampedXEnd = std::min(static_cast<s32>(cfg::FramebufferWidth), x + w);
            
                // Early exit if nothing to draw after clamping
                if (x + w <= 0 || x >= static_cast<s32>(cfg::FramebufferWidth) || clampedY >= clampedYEnd)
                    return;
            
                processRoundedRectChunk(this, x, y, w, h, radius, color, clampedY, clampedYEnd);
            }
            
            std::function<void(s32, s32, s32, s32, s32, Color)> drawRoundedRect;
            inline void updateDrawFunction() {
                if (ult::expandedMemory) {
                    drawRoundedRect = [this](s32 x, s32 y, s32 w, s32 h, s32 radius, Color color) {
                        drawRoundedRectMultiThreaded(x, y, w, h, radius, color);
                    };
                } else {
                    drawRoundedRect = [this](s32 x, s32 y, s32 w, s32 h, s32 radius, Color color) {
                        drawRoundedRectSingleThreaded(x, y, w, h, radius, color);
                    };
                }
            }
            
                                                
            inline void drawUniformRoundedRect(const s32 x, const s32 y, const s32 w, const s32 h, const Color& color) {
                const s32 radius = h >> 1;
                const s32 clip_left = std::max(0, x);
                const s32 clip_top = std::max(0, y);
                const s32 clip_right = std::min(static_cast<s32>(cfg::FramebufferWidth), x + w);
                const s32 clip_bottom = std::min(static_cast<s32>(cfg::FramebufferHeight), y + h);
                
                if (clip_left >= clip_right || clip_top >= clip_bottom) return;
                
                const s32 x_end = x + w;
                const s32 y_end = y + h;
                const s32 corner_x_left = x + radius;
                const s32 corner_x_right = x_end - radius - 1;
                const s32 corner_y_top = y + radius;
                const s32 corner_y_bottom = y_end - radius - 1;
                const float r_f = static_cast<float>(radius);
                const float r2 = r_f * r_f;
                const float aa_thresh = r2 + 2.0f * r_f + 1.0f;
                const u8 base_a = color.a;
                const bool full_opacity = (base_a == 0xF);
                
                for (s32 yc = clip_top; yc < clip_bottom; ++yc) {
                    if (yc < y || yc >= y_end) continue;
                    
                    const bool in_corners = yc < corner_y_top || yc > corner_y_bottom;
                    
                    if (!in_corners) {
                        const s32 span_start = std::max(x, clip_left);
                        const s32 span_end = std::min(x_end, clip_right);
                        
                        for (s32 xc = span_start; xc < span_end; ++xc) {
                            const u32 off = this->getPixelOffset(xc, yc);
                            if (off != UINT32_MAX) {
                                if (full_opacity) {
                                    this->setPixelAtOffset(off, color);
                                } else {
                                    this->setPixelBlendDst(xc, yc, color);
                                }
                            }
                        }
                    } else {
                        const float dy = (yc < corner_y_top) ? static_cast<float>(corner_y_top - yc) : 
                                                                 static_cast<float>(yc - corner_y_bottom);
                        const float dy_sq = dy * dy;
                        
                        if (dy_sq > aa_thresh) continue;
                        
                        const float dy_half = dy - 0.5f;
                        const float dy_half_sq = dy_half * dy_half;
                        
                        const s32 span_start = std::max(x, clip_left);
                        const s32 span_end = std::min(x_end, clip_right);
                        s32 xc = span_start;
                        
                        // Left corner/edge
                        const s32 left_end = std::min(corner_x_left + 1, span_end);
                        for (; xc < left_end; ++xc) {
                            const float dx = static_cast<float>(corner_x_left - xc);
                            const float dx_sq = dx * dx;
                            const float d2 = dx_sq + dy_sq;
                            
                            if (d2 <= r2) {
                                const u32 off = this->getPixelOffset(xc, yc);
                                if (off != UINT32_MAX) {
                                    if (full_opacity) this->setPixelAtOffset(off, color);
                                    else this->setPixelBlendDst(xc, yc, color);
                                }
                            } else if (d2 <= aa_thresh) {
                                const float dx_half = dx - 0.5f;
                                float cov = 0.0f;
                                if (dx_sq + dy_sq <= r2) cov += 0.25f;
                                if (dx_half*dx_half + dy_sq <= r2) cov += 0.25f;
                                if (dx_sq + dy_half_sq <= r2) cov += 0.25f;
                                if (dx_half*dx_half + dy_half_sq <= r2) cov += 0.25f;
                                
                                if (cov > 0.0f) {
                                    const u32 off = this->getPixelOffset(xc, yc);
                                    if (off != UINT32_MAX) {
                                        Color c = color;
                                        c.a = static_cast<u8>((base_a * static_cast<u8>(cov * 15.0f + 0.5f)) / 15);
                                        this->setPixelBlendDst(xc, yc, c);
                                    }
                                }
                            }
                        }
                        
                        // Middle section
                        const s32 mid_end = std::min(corner_x_right, span_end);
                        for (; xc < mid_end; ++xc) {
                            const u32 off = this->getPixelOffset(xc, yc);
                            if (off != UINT32_MAX) {
                                if (full_opacity) this->setPixelAtOffset(off, color);
                                else this->setPixelBlendDst(xc, yc, color);
                            }
                        }
                        
                        // Right corner/edge
                        for (; xc < span_end; ++xc) {
                            const float dx = static_cast<float>(xc - corner_x_right);
                            const float dx_sq = dx * dx;
                            const float d2 = dx_sq + dy_sq;
                            
                            if (d2 <= r2) {
                                const u32 off = this->getPixelOffset(xc, yc);
                                if (off != UINT32_MAX) {
                                    if (full_opacity) this->setPixelAtOffset(off, color);
                                    else this->setPixelBlendDst(xc, yc, color);
                                }
                            } else if (d2 <= aa_thresh) {
                                const float dx_half = dx - 0.5f;
                                float cov = 0.0f;
                                if (dx_sq + dy_sq <= r2) cov += 0.25f;
                                if (dx_half*dx_half + dy_sq <= r2) cov += 0.25f;
                                if (dx_sq + dy_half_sq <= r2) cov += 0.25f;
                                if (dx_half*dx_half + dy_half_sq <= r2) cov += 0.25f;
                                
                                if (cov > 0.0f) {
                                    const u32 off = this->getPixelOffset(xc, yc);
                                    if (off != UINT32_MAX) {
                                        Color c = color;
                                        c.a = static_cast<u8>((base_a * static_cast<u8>(cov * 15.0f + 0.5f)) / 15);
                                        this->setPixelBlendDst(xc, yc, c);
                                    }
                                }
                            }
                        }
                    }
                }
            }
                        
            // Struct for batch pixel processing with better alignment
            //struct alignas(64) PixelBatch {
            //    s32 baseX, baseY;
            //    u8 red[32], green[32], blue[32], alpha[32];  // Doubled for 32-pixel batches
            //    s32 count;
            //};
            //
            //// Batch pixel setter - process multiple pixels at once if available
            //inline void setPixelBatchBlendSrc(const s32 baseX, const s32 baseY, const PixelBatch& batch) {
            //    // If your graphics system supports batch operations, use them here
            //    // Otherwise fall back to individual calls
            //    for (s32 i = 0; i < batch.count; ++i) {
            //        setPixelBlendSrc(baseX + i, baseY, {
            //            batch.red[i], batch.green[i], batch.blue[i], batch.alpha[i]
            //        });
            //    }
            //}

            // RGBA4444 processing - no expansion needed
            const uint8x16_t mask_low = vdupq_n_u8(0x0F);
            
            inline void processBMPChunk(const s32 x, const s32 y, const s32 screenW, const u8 *preprocessedData, 
                                       const s32 startRow, const s32 endRow, const u8 globalAlphaLimit) {
                static constexpr s32 bytesPerRow = 448 * 2;
                static constexpr s32 endX16 = 448 & ~15;
                const uint8x16_t alpha_limit_vec = vdupq_n_u8(globalAlphaLimit);
                
                // Get framebuffer once for entire chunk
                Color* const framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                
                for (s32 y1 = startRow; y1 < endRow; ++y1) {
                    const u8 *rowPtr = preprocessedData + (y1 * bytesPerRow);
                    const s32 baseY = y + y1;
                    
                    s32 x1 = 0;
                    
                    // SIMD processing for 16 pixels at once
                    for (; x1 < endX16; x1 += 16) {
                        const u8* ptr = rowPtr + (x1 << 1);
                        
                        // Load and unpack RGBA4444 data - keep as 4-bit
                        uint8x16x2_t packed = vld2q_u8(ptr);
                        uint8x16_t high1 = vshrq_n_u8(packed.val[0], 4);
                        uint8x16_t low1  = vandq_u8(packed.val[0], mask_low);
                        uint8x16_t high2 = vshrq_n_u8(packed.val[1], 4);
                        uint8x16_t low2  = vminq_u8(vandq_u8(packed.val[1], mask_low), alpha_limit_vec);
                        
                        // Store results directly
                        alignas(16) u8 red_vals[16], green_vals[16], blue_vals[16], alpha_vals[16];
                        vst1q_u8(red_vals, high1);
                        vst1q_u8(green_vals, low1); 
                        vst1q_u8(blue_vals, high2);
                        vst1q_u8(alpha_vals, low2);
                        
                        const s32 baseX = x + x1;
                        
                        // Optimized pixel loop with direct framebuffer access
                        for (int i = 0; i < 16; ++i) {
                            const u8 a = alpha_vals[i];
                            if (a == 0) continue;
                            
                            const u32 offset = this->getPixelOffset(baseX + i, baseY);
                            if (offset == UINT32_MAX) continue;
                            
                            const Color src = framebuffer[offset];
                            
                            framebuffer[offset] = {
                                blendColor(src.r, red_vals[i], a),
                                blendColor(src.g, green_vals[i], a),
                                blendColor(src.b, blue_vals[i], a),
                                src.a
                            };
                        }
                    }
                    
                    // Handle remaining pixels
                    for (; x1 < screenW; ++x1) {
                        const u8 p1 = rowPtr[x1 << 1];
                        const u8 p2 = rowPtr[(x1 << 1) + 1];
                        const u8 alpha = std::min(static_cast<u8>(p2 & 0x0F), globalAlphaLimit);
                        
                        setPixelBlendSrc(x + x1, baseY, {
                            static_cast<u8>(p1 >> 4),
                            static_cast<u8>(p1 & 0x0F),
                            static_cast<u8>(p2 >> 4),
                            alpha
                        });
                    }
                }
                
                ult::inPlotBarrier.arrive_and_wait();
            }
            

            /**
             * @brief Draws a scaled RGBA8888 bitmap from memory
             *
             * @param x X start position
             * @param y Y start position
             * @param w Bitmap width (original width of the bitmap)
             * @param h Bitmap height (original height of the bitmap)
             * @param bmp Pointer to bitmap data
             * @param screenW Target screen width
             * @param screenH Target screen height
             */

            //inline void drawBitmapRGBA4444(const s32 x, const s32 y, const s32 screenW, const s32 screenH, const u8 *preprocessedData) {
            //    s32 startRow;
            //    
            //    // Divide rows among ult::renderThreads
            //    //s32 chunkSize = (screenH + ult::numThreads - 1) / ult::numThreads;
            //    for (unsigned i = 0; i < ult::numThreads; ++i) {
            //        startRow = i * ult::bmpChunkSize;
            //        //s32 endRow = std::min(startRow + ult::bmpChunkSize, screenH);
            //        
            //        // Bind the member function and create the thread
            //        ult::renderThreads[i] = std::thread(std::bind(&tsl::gfx::Renderer::processBMPChunk, this, x, y, screenW, preprocessedData, startRow, std::min(startRow + ult::bmpChunkSize, screenH)));
            //    }
            //    
            //    // Join all ult::renderThreads
            //    for (auto& t : ult::renderThreads) {
            //        t.join();
            //    }
            //}

            inline void drawBitmapRGBA4444(const s32 x, const s32 y, const s32 screenW, const s32 screenH, 
                                           const u8 *preprocessedData, float opacity = 1.0f) {
                // Pre-compute alpha limit once
                const u8 globalAlphaLimit = static_cast<u8>(0xF * opacity);
                
                s32 startRow;
                
                for (unsigned i = 0; i < ult::numThreads; ++i) {
                    startRow = i * ult::bmpChunkSize;
                    
                    // Pass the alpha limit to each thread
                    ult::renderThreads[i] = std::thread(std::bind(&tsl::gfx::Renderer::processBMPChunk, 
                        this, x, y, screenW, preprocessedData, startRow, 
                        std::min(startRow + ult::bmpChunkSize, screenH), globalAlphaLimit));
                }
            
                // Join all threads
                for (auto& t : ult::renderThreads) {
                    t.join();
                }
            }



            //inline void drawWallpaper() {
            //    if (!ult::expandedMemory || ult::refreshWallpaper.load(std::memory_order_acquire)) {
            //        return;
            //    }
            //    
            //    ult::inPlot.store(true, std::memory_order_release);
            //    
            //    if (!ult::wallpaperData.empty() && 
            //        !ult::refreshWallpaper.load(std::memory_order_acquire) && 
            //        ult::correctFrameSize) {
            //        drawBitmapRGBA4444(0, 0, cfg::FramebufferWidth, cfg::FramebufferHeight, ult::wallpaperData.data());
            //    }
            //    
            //    ult::inPlot.store(false, std::memory_order_release);
            //}

            inline void drawWallpaper() {
                if (!ult::expandedMemory || ult::refreshWallpaper.load(std::memory_order_acquire)) {
                    return;
                }
                
                ult::inPlot.store(true, std::memory_order_release);
                
                if (!ult::wallpaperData.empty() && 
                    !ult::refreshWallpaper.load(std::memory_order_acquire) && 
                    ult::correctFrameSize) {
                    // Use the renderer's opacity directly
                    drawBitmapRGBA4444(0, 0, cfg::FramebufferWidth, cfg::FramebufferHeight, 
                                      ult::wallpaperData.data(), Renderer::s_opacity);
                }
                
                ult::inPlot.store(false, std::memory_order_release);
            }


            /**
             * @brief Draws a RGBA8888 bitmap from memory
             *
             * @param x X start position
             * @param y Y start position
             * @param w Bitmap width
             * @param h Bitmap height
             * @param bmp Pointer to bitmap data
             */
            inline void drawBitmap(s32 x, s32 y, s32 w, s32 h, const u8 *bmp) {
                if (w <= 0 || h <= 0) [[unlikely]] return;
                
                const u8* __restrict__ src = bmp;
                
                // Pre-compute alpha limit once using global opacity
                const u8 alphaLimit = static_cast<u8>(0xF * Renderer::s_opacity);
                
                // Completely unroll small bitmaps for maximum speed
                if (w <= 8 && h <= 8) [[likely]] {
                    s32 px;
                    // Specialized path for small bitmaps (icons, etc.)
                    for (s32 py = 0; py < h; ++py) {
                        const s32 rowY = y + py;
                        px = x;
                        
                        // Unroll inner loop completely for small widths
                        switch(w) {
                            case 8: goto pixel8;
                            case 7: goto pixel7;
                            case 6: goto pixel6;
                            case 5: goto pixel5;
                            case 4: goto pixel4;
                            case 3: goto pixel3;
                            case 2: goto pixel2;
                            case 1: goto pixel1;
                            default: break;
                        }
                        
                        pixel8: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel7: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel6: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel5: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel4: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel3: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel2: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            px++; src += 4;
                        }
                        pixel1: {
                            u8 alpha = src[3] >> 4;
                            if (alpha > 0) {
                                alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                                const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                               static_cast<u8>(src[2] >> 4), alpha};
                                setPixelBlendSrc(px, rowY, a(c));
                            }
                            src += 4;
                        }
                    }
                    return;
                }
                
                // Optimized scalar path for larger bitmaps
                for (s32 py = 0; py < h; ++py) {
                    const s32 rowY = y + py;
                    s32 px = x;
                    const u8* rowEnd = src + (w * 4);
                    
                    // Prefetch first cache line
                    __builtin_prefetch(src, 0, 3);
                    
                    // Process all pixels in the row
                    while (src < rowEnd) {
                        // Prefetch ahead every 16 pixels (64 bytes)
                        if (((uintptr_t)src & 63) == 0) [[unlikely]] {
                            __builtin_prefetch(src + 64, 0, 3);
                        }
                        
                        u8 alpha = src[3] >> 4;
                        if (alpha > 0) {
                            alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), alpha};
                            setPixelBlendSrc(px, rowY, c);
                        }
                        px++;
                        src += 4;
                    }
                }
            }
            
            /**
             * @brief Fills the entire layer with a given color
             *
             * @param color Color
             */
            inline void fillScreen(const Color& color) {
                std::fill_n(static_cast<Color*>(this->getCurrentFramebuffer()), this->getFramebufferSize() / sizeof(Color), color);
            }
            
            /**
             * @brief Clears the layer (With transparency)
             *
             */
            inline void clearScreen() {
                this->fillScreen(Color(0x0, 0x0, 0x0, 0x0)); // Fully transparent
            }
            
            const stbtt_fontinfo& getStandardFont() const {
                return m_stdFont;
            }
                    
            
            // Optimized unified drawString method with thread safety
            inline std::pair<s32, s32> drawString(const std::string& originalString, bool monospace, 
                                                  const s32 x, const s32 y, const u32 fontSize, 
                                                  const Color& defaultColor, const ssize_t maxWidth = 0, 
                                                  bool draw = true,
                                                  const Color* highlightColor = nullptr,
                                                  const std::vector<std::string>* specialSymbols = nullptr,
                                                  const u32 highlightStartChar = 0,
                                                  const u32 highlightEndChar = 0,
                                                  const bool useNotificationCache = false) {
                
                // Thread-safe translation cache access
                const std::string* text = &originalString;
                std::string translatedText;
                
                #if defined(UI_OVERRIDE_PATH)
                {
                    std::shared_lock<std::shared_mutex> readLock(s_translationCacheMutex);
                    auto translatedIt = ult::translationCache.find(originalString);
                    if (translatedIt != ult::translationCache.end()) {
                        translatedText = translatedIt->second;
                        text = &translatedText;
                    }
                }
                #endif
                
                if (text->empty() || fontSize == 0) return {0, 0};
                
                const float maxWidthLimit = maxWidth > 0 ? x + maxWidth : std::numeric_limits<float>::max();
                
                // Check if highlighting is enabled
                const bool highlightingEnabled = highlightColor && highlightStartChar != 0 && highlightEndChar != 0;
                
                // Get font metrics once
                const auto fontMetrics = FontManager::getFontMetricsForCharacter('A', fontSize);
                const s32 lineHeight = static_cast<s32>(fontMetrics.lineHeight);
                
                // Fast ASCII check with early exit
                bool isAsciiOnly = true;
                const char* textPtr = text->data();
                const char* textEnd = textPtr + text->size();
                
                for (const char* p = textPtr; p < textEnd; ++p) {
                    if (static_cast<unsigned char>(*p) > 127) {
                        isAsciiOnly = false;
                        break;
                    }
                }
                
                s32 maxX = x, currX = x, currY = y;
                s32 maxY = y + lineHeight;
                bool inHighlight = false;
                const Color* currentColor = &defaultColor;
                
                // Main processing loop
                if (isAsciiOnly && !specialSymbols) {
                    // Fast ASCII-only path
                    for (const char* p = textPtr; p < textEnd && currX < maxWidthLimit; ++p) {
                        u32 currCharacter = static_cast<u32>(*p);
                        
                        // Handle highlighting
                        if (highlightingEnabled) {
                            if (currCharacter == highlightStartChar) {
                                inHighlight = true;
                                currentColor = &defaultColor;
                            } else if (currCharacter == highlightEndChar) {
                                inHighlight = false;
                                currentColor = &defaultColor;
                            } else {
                                currentColor = inHighlight ? highlightColor : &defaultColor;
                            }
                        }
                        
                        // Handle newline
                        if (currCharacter == '\n') {
                            maxX = std::max(currX, maxX);
                            currX = x;
                            currY += lineHeight;
                            maxY = std::max(maxY, currY + lineHeight);
                            continue;
                        }
                        
                        // Get glyph
                        std::shared_ptr<FontManager::Glyph> glyph = useNotificationCache ?
                            FontManager::getOrCreateNotificationGlyph(currCharacter, monospace, fontSize) :
                            FontManager::getOrCreateGlyph(currCharacter, monospace, fontSize);
                        
                        if (!glyph) continue;
                        
                        maxY = std::max(maxY, currY + lineHeight);
                        
                        // Render if needed
                        if (draw && glyph->glyphBmp && currCharacter > 32) {
                            renderGlyph(glyph, currX, currY, *currentColor, useNotificationCache);
                        }
                        
                        currX += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                    }
                } else {
                    // UTF-8 path with special symbols support
                    auto itStr = text->cbegin();
                    const auto itStrEnd = text->cend();
                    
                    while (itStr != itStrEnd && currX < maxWidthLimit) {
                        // Check for special symbols first
                        bool symbolProcessed = false;
                        
                        if (specialSymbols) {
                            const size_t remainingLength = itStrEnd - itStr;
                            
                            for (const auto& symbol : *specialSymbols) {
                                if (remainingLength >= symbol.length() &&
                                    std::equal(symbol.begin(), symbol.end(), itStr)) {
                                    
                                    // Process special symbol
                                    for (size_t i = 0; i < symbol.length(); ) {
                                        u32 symChar;
                                        const ssize_t symWidth = decode_utf8(&symChar, 
                                            reinterpret_cast<const u8*>(&symbol[i]));
                                        if (symWidth <= 0) break;
                                        
                                        if (symChar == '\n') {
                                            maxX = std::max(currX, maxX);
                                            currX = x;
                                            currY += lineHeight;
                                            maxY = std::max(maxY, currY + lineHeight);
                                        } else {
                                            auto glyph = FontManager::getOrCreateGlyph(symChar, monospace, fontSize);
                                            if (glyph) {
                                                maxY = std::max(maxY, currY + lineHeight);
                                                
                                                if (draw && glyph->glyphBmp && symChar > 32) {
                                                    renderGlyph(glyph, currX, currY, *highlightColor, useNotificationCache);
                                                }
                                                currX += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                                            }
                                        }
                                        i += symWidth;
                                    }
                                    itStr += symbol.length();
                                    symbolProcessed = true;
                                    break;
                                }
                            }
                        }
                        
                        if (symbolProcessed) continue;
                        
                        // Decode character
                        u32 currCharacter;
                        ssize_t codepointWidth;
                        
                        if (isAsciiOnly) {
                            currCharacter = static_cast<u32>(*itStr);
                            codepointWidth = 1;
                        } else {
                            codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                            if (codepointWidth <= 0) break;
                        }
                        
                        itStr += codepointWidth;
                        
                        // Handle highlighting
                        if (highlightingEnabled) {
                            if (currCharacter == highlightStartChar) {
                                inHighlight = true;
                                currentColor = &defaultColor;
                            } else if (currCharacter == highlightEndChar) {
                                inHighlight = false;
                                currentColor = &defaultColor;
                            } else {
                                currentColor = inHighlight ? highlightColor : &defaultColor;
                            }
                        }
                        
                        // Handle newline
                        if (currCharacter == '\n') {
                            maxX = std::max(currX, maxX);
                            currX = x;
                            currY += lineHeight;
                            maxY = std::max(maxY, currY + lineHeight);
                            continue;
                        }
                        
                        // Get glyph
                        auto glyph = FontManager::getOrCreateGlyph(currCharacter, monospace, fontSize);
                        if (!glyph) continue;
                        
                        maxY = std::max(maxY, currY + lineHeight);
                        
                        // Render if needed
                        if (draw && glyph->glyphBmp && currCharacter > 32) {
                            renderGlyph(glyph, currX, currY, *currentColor, useNotificationCache);
                        }
                        
                        currX += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                    }
                }
                
                maxX = std::max(currX, maxX);
                return {maxX - x, maxY - y};
            }

            inline std::pair<s32, s32> drawNotificationString(const std::string& text, bool monospace, 
                                                              const s32 x, const s32 y, const u32 fontSize, 
                                                              const Color& defaultColor, const ssize_t maxWidth = 0, 
                                                              bool draw = true,
                                                              const Color* highlightColor = nullptr,
                                                              const std::vector<std::string>* specialSymbols = nullptr,
                                                              const u32 highlightStartChar = 0,
                                                              const u32 highlightEndChar = 0) {
                return drawString(text, monospace, x, y, fontSize, defaultColor, maxWidth, draw, 
                                 highlightColor, specialSymbols, highlightStartChar, highlightEndChar, true);
            }
            
            // Convenience wrappers for backward compatibility
            inline std::pair<s32, s32> drawStringWithHighlight(const std::string& text, bool monospace, 
                                                              s32 x, s32 y, const u32 fontSize, 
                                                              const Color& defaultColor, 
                                                              const Color& specialColor, 
                                                              const ssize_t maxWidth = 0,
                                                              const u32 startChar = '(',
                                                              const u32 endChar = ')') {
                return drawString(text, monospace, x, y, fontSize, defaultColor, maxWidth, true, &specialColor, nullptr, startChar, endChar);
            }
            
            inline std::pair<s32, s32> drawStringWithColoredSections(const std::string& text, bool monospace,
                                                    const std::vector<std::string>& specialSymbols, 
                                                    s32 x, const s32 y, const u32 fontSize, 
                                                    const Color& defaultColor, 
                                                    const Color& specialColor) {
                return drawString(text, monospace, x, y, fontSize, defaultColor, 0, true, &specialColor, &specialSymbols);
            }
            
            // Calculate string dimensions without drawing
            inline std::pair<s32, s32> getTextDimensions(const std::string& text, bool monospace, 
                                                         const u32 fontSize, const ssize_t maxWidth = 0) {
                return drawString(text, monospace, 0, 0, fontSize, Color{0,0,0,0}, maxWidth, false);
            }

            inline std::pair<s32, s32> getNotificationTextDimensions(const std::string& text, bool monospace, 
                                                                     const u32 fontSize, const ssize_t maxWidth = 0) {
                return drawString(text, monospace, 0, 0, fontSize, Color{0,0,0,0}, maxWidth, false, 
                                 nullptr, nullptr, 0, 0, true);
            }
            
            // Thread-safe limitStringLength using the unified cache
            inline std::string limitStringLength(const std::string& originalString, const bool monospace, 
                                               const u32 fontSize, const s32 maxLength) {  // Changed fontSize to u32
                
                // Thread-safe translation cache access
                std::string text;
                #ifdef UI_OVERRIDE_PATH
                {
                    std::shared_lock<std::shared_mutex> readLock(s_translationCacheMutex);
                    auto translatedIt = ult::translationCache.find(originalString);
                    if (translatedIt != ult::translationCache.end()) {
                        text = translatedIt->second;
                    } else {
                        // Don't insert anything, just fallback to original string
                        text = originalString;
                    }
                }
                #else
                text = originalString;
                #endif
                
                if (text.size() < 2) return text;
                
                // Get ellipsis width using shared cache (now thread-safe)
                static constexpr u32 ellipsisChar = 0x2026;
                std::shared_ptr<FontManager::Glyph> ellipsisGlyph = FontManager::getOrCreateGlyph(ellipsisChar, monospace, fontSize);
                if (!ellipsisGlyph) return text;
                
                // Fixed: Use consistent s32 calculation like other functions
                const s32 ellipsisWidth = static_cast<s32>(ellipsisGlyph->xAdvance * ellipsisGlyph->currFontSize);
                const s32 maxWidthWithoutEllipsis = maxLength - ellipsisWidth;
                
                if (maxWidthWithoutEllipsis <= 0) {
                    return "…"; // If there's no room for text, just return ellipsis
                }
                
                // Calculate width incrementally
                s32 currX = 0;
                auto itStr = text.cbegin();
                const auto itStrEnd = text.cend();
                auto lastValidPos = itStr;
                
                // Fast ASCII check
                bool isAsciiOnly = true;
                for (unsigned char c : text) {
                    if (c > 127) {
                        isAsciiOnly = false;
                        break;
                    }
                }
                
                // Move variable declarations outside the loop
                u32 currCharacter;
                ssize_t codepointWidth;
                s32 charWidth;
                size_t bytePos;

                while (itStr != itStrEnd) {
                    // Decode UTF-8 codepoint
                    if (isAsciiOnly) {
                        currCharacter = static_cast<u32>(*itStr);
                        codepointWidth = 1;
                    } else {
                        codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                        if (codepointWidth <= 0) break;
                    }
                    
                    // FontManager::getOrCreateGlyph is now thread-safe
                    std::shared_ptr<FontManager::Glyph> glyph = FontManager::getOrCreateGlyph(currCharacter, monospace, fontSize);
                    if (!glyph) {
                        itStr += codepointWidth;
                        continue;
                    }
                    
                    // Fixed: Use consistent s32 calculation
                    charWidth = static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                    
                    if (currX + charWidth > maxWidthWithoutEllipsis) {
                        // Calculate the byte position for substring
                        bytePos = std::distance(text.cbegin(), lastValidPos);
                        return text.substr(0, bytePos) + "…";
                    }
                    
                    currX += charWidth;
                    itStr += codepointWidth;
                    lastValidPos = itStr;
                }
                
                return text;
            }

            inline void setLayerPos(u32 x, u32 y) {
                //const float ratio = 1.5;
                //u32 maxX = cfg::ScreenWidth - (int)(ratio * cfg::FramebufferWidth);
                //u32 maxY = cfg::ScreenHeight - (int)(ratio * cfg::FramebufferHeight);
                if (x > cfg::ScreenWidth - (int)(1.5 * cfg::FramebufferWidth) || y > cfg::ScreenHeight - (int)(1.5 * cfg::FramebufferHeight)) {
                    return;
                }
                setLayerPosImpl(x, y);
            }

            void updateLayerSize() {
                const auto [horizontalUnderscanPixels, verticalUnderscanPixels] = getUnderscanPixels();
                
                // Recalculate layer dimensions with new underscan values
                cfg::LayerWidth  = cfg::ScreenWidth * (float(cfg::FramebufferWidth) / float(cfg::LayerMaxWidth));
                cfg::LayerHeight = cfg::ScreenHeight * (float(cfg::FramebufferHeight) / float(cfg::LayerMaxHeight));
                
                // Apply underscan adjustments
                if (ult::DefaultFramebufferWidth == 1280 && ult::DefaultFramebufferHeight == 28) {
                    cfg::LayerHeight += cfg::ScreenHeight/720. * verticalUnderscanPixels;
                } else if (ult::correctFrameSize) {
                    cfg::LayerWidth += horizontalUnderscanPixels;
                }
                
                // Update position if using right alignment
                if (ult::useRightAlignment && ult::correctFrameSize) {
                    ult::layerEdge = (1280 - 448);
                }
                
                // Update the existing layer with new dimensions
                viSetLayerSize(&this->m_layer, cfg::LayerWidth, cfg::LayerHeight);
                
                // Update position if using right alignment
                if (ult::useRightAlignment && ult::correctFrameSize) {
                    viSetLayerPosition(&this->m_layer, 1280-32 - horizontalUnderscanPixels, 0);
                    viSetLayerSize(&this->m_layer, cfg::LayerWidth, cfg::LayerHeight);
                    viSetLayerPosition(&this->m_layer, 1280-32 - horizontalUnderscanPixels, 0);
                }
                // ADD THIS: Update position for micro mode bottom positioning
                else if (ult::DefaultFramebufferWidth == 1280 && ult::DefaultFramebufferHeight == 28 && cfg::LayerPosY > 500) {
                    // Only adjust if already positioned at bottom (LayerPosY > 500 indicates bottom positioning)
                    const u32 targetY = !verticalUnderscanPixels ? 1038 : 1038- (cfg::ScreenHeight/720. * verticalUnderscanPixels) +0.5;
                    viSetLayerPosition(&this->m_layer, 0, targetY);
                    viSetLayerSize(&this->m_layer, cfg::LayerWidth, cfg::LayerHeight);
                    viSetLayerPosition(&this->m_layer, 0, targetY);
                }
            }

            static Renderer& getRenderer() {
                return get();
            }

            inline void setLayerPosImpl(u32 x, u32 y) {
                // Get the underscan pixel values for both horizontal and vertical borders
                //const auto [horizontalUnderscanPixels, verticalUnderscanPixels] = getUnderscanPixels();
                
                // Simply set the position to what was requested - no automatic right alignment
                cfg::LayerPosX = x;
                cfg::LayerPosY = y;
                
                ASSERT_FATAL(viSetLayerPosition(&this->m_layer, cfg::LayerPosX, cfg::LayerPosY));
            }


        #if USING_WIDGET_DIRECTIVE
            // Method to draw clock, temperatures, and battery percentage
            inline void drawWidget() {
                static time_t lastTimeUpdate = 0;
                static char timeStr[20];
                static char PCB_temperatureStr[10];
                static char SOC_temperatureStr[10];
                static char chargeString[6];
                static time_t lastSensorUpdate = 0;
                
                const bool showAnyWidget = !(ult::hideBattery && ult::hidePCBTemp && ult::hideSOCTemp && ult::hideClock);
                
                // Draw separator and backdrop if showing any widget
                if (showAnyWidget) {
                    drawRect(239, 15 + 2 - 2, 1, 64 + 2, aWithOpacity(topSeparatorColor));
                    if (!ult::hideWidgetBackdrop) {
                        drawUniformRoundedRect(
                            247, 15 + 2 - 2,
                            (ult::extendedWidgetBackdrop
                                ? tsl::cfg::FramebufferWidth - 255
                                : tsl::cfg::FramebufferWidth - 215),
                            64 + 2, a(widgetBackdropColor)
                        );
                    }
                }
                
                // Calculate base Y offset
                size_t y_offset = ((ult::hideBattery && ult::hidePCBTemp && ult::hideSOCTemp) || ult::hideClock)
                                  ? (55 + 2 - 1)
                                  : (44 + 2 - 1);
                
                // Constants for centering calculations
                const int backdropCenterX = 247 + ((tsl::cfg::FramebufferWidth - 255) >> 1);
                
                time_t currentTime = time(nullptr);
                
                // Draw clock
                if (!ult::hideClock) {
                    if (currentTime != lastTimeUpdate || ult::languageWasChanged.load(std::memory_order_acquire)) {
                        strftime(timeStr, sizeof(timeStr), ult::datetimeFormat.c_str(), localtime(&currentTime));
                        ult::localizeTimeStr(timeStr);
                        lastTimeUpdate = currentTime;
                    }
                    
                    const int timeWidth = getTextDimensions(timeStr, false, 20).first;
                    
                    if (ult::centerWidgetAlignment) {
                        // Centered alignment
                        drawString(timeStr, false, backdropCenterX - (timeWidth >> 1), y_offset, 20, clockColor);
                    } else {
                        // Right alignment
                        drawString(timeStr, false, tsl::cfg::FramebufferWidth - timeWidth - 25, y_offset, 20, clockColor);
                    }
                    
                    y_offset += 22;
                }
                
                // Update sensor data every second
                if ((currentTime - lastSensorUpdate) >= 1) {
                    if (!ult::hideSOCTemp) {
                        float socTemp = 0.0f;
                        ult::ReadSocTemperature(&socTemp);
                        ult::SOC_temperature.store(socTemp, std::memory_order_release);
                        snprintf(
                            SOC_temperatureStr, sizeof(SOC_temperatureStr),
                            "%d°C",
                            static_cast<int>(round(ult::SOC_temperature.load(std::memory_order_acquire)))
                        );
                    }
                    
                    if (!ult::hidePCBTemp) {
                        float pcbTemp = 0.0f;
                        ult::ReadPcbTemperature(&pcbTemp);
                        ult::PCB_temperature.store(pcbTemp, std::memory_order_release);
                        snprintf(
                            PCB_temperatureStr, sizeof(PCB_temperatureStr),
                            "%d°C",
                            static_cast<int>(round(ult::PCB_temperature.load(std::memory_order_acquire)))
                        );
                    }
                    
                    if (!ult::hideBattery) {
                        uint32_t bc = 0;
                        bool charging = false;
                        ult::powerGetDetails(&bc, &charging);
                        bc = std::min(bc, 100U);
                        ult::batteryCharge.store(bc, std::memory_order_release);
                        ult::isCharging.store(charging, std::memory_order_release);
                        snprintf(chargeString, sizeof(chargeString), "%u%%", bc);
                    }
                    
                    lastSensorUpdate = currentTime;
                }
                
                if (ult::centerWidgetAlignment) {
                    // CENTERED ALIGNMENT
                    int totalWidth = 0;
                    int socWidth = 0, pcbWidth = 0, chargeWidth = 0;
                    bool hasMultiple = false;
                    
                    const float socTemp = ult::SOC_temperature.load(std::memory_order_acquire);
                    const float pcbTemp = ult::PCB_temperature.load(std::memory_order_acquire);
                    const uint32_t batteryCharge = ult::batteryCharge.load(std::memory_order_acquire);
                    const bool charging = ult::isCharging.load(std::memory_order_acquire);
                    
                    if (!ult::hideSOCTemp && socTemp > 0.0f) {
                        socWidth = getTextDimensions(SOC_temperatureStr, false, 20).first;
                        totalWidth += socWidth;
                        hasMultiple = true;
                    }
                    if (!ult::hidePCBTemp && pcbTemp > 0.0f) {
                        pcbWidth = getTextDimensions(PCB_temperatureStr, false, 20).first;
                        if (hasMultiple) totalWidth += 5;
                        totalWidth += pcbWidth;
                        hasMultiple = true;
                    }
                    if (!ult::hideBattery && batteryCharge > 0) {
                        chargeWidth = getTextDimensions(chargeString, false, 20).first;
                        if (hasMultiple) totalWidth += 5;
                        totalWidth += chargeWidth;
                    }
                    
                    int currentX = backdropCenterX - (totalWidth >> 1);
                    if (socWidth > 0) {
                        drawString(
                            SOC_temperatureStr, false, currentX, y_offset, 20,
                            ult::dynamicWidgetColors
                                ? tsl::GradientColor(socTemp)
                                : temperatureColor
                        );
                        currentX += socWidth + 5;
                    }
                    if (pcbWidth > 0) {
                        drawString(
                            PCB_temperatureStr, false, currentX, y_offset, 20,
                            ult::dynamicWidgetColors
                                ? tsl::GradientColor(pcbTemp)
                                : temperatureColor
                        );
                        currentX += pcbWidth + 5;
                    }
                    if (chargeWidth > 0) {
                        const Color batteryColorToUse = charging
                            ? batteryChargingColor
                            : (batteryCharge < 20 ? batteryLowColor : batteryColor);
                        drawString(chargeString, false, currentX, y_offset, 20, batteryColorToUse);
                    }
                    
                } else {
                    // RIGHT ALIGNMENT
                    int chargeWidth = 0, pcbWidth = 0, socWidth = 0;
                    const float pcbTemp = ult::PCB_temperature.load(std::memory_order_acquire);
                    const float socTemp = ult::SOC_temperature.load(std::memory_order_acquire);
                    const uint32_t batteryCharge = ult::batteryCharge.load(std::memory_order_acquire);
                    const bool charging = ult::isCharging.load(std::memory_order_acquire);
                    
                    if (!ult::hideBattery && batteryCharge > 0) {
                        const Color batteryColorToUse = charging
                            ? batteryChargingColor
                            : (batteryCharge < 20 ? batteryLowColor : batteryColor);
                        chargeWidth = getTextDimensions(chargeString, false, 20).first;
                        drawString(
                            chargeString, false,
                            tsl::cfg::FramebufferWidth - chargeWidth - 25,
                            y_offset, 20, batteryColorToUse
                        );
                    }
                    
                    int offset = 0;
                    if (!ult::hidePCBTemp && pcbTemp > 0.0f) {
                        if (!ult::hideBattery) offset -= 5;
                        pcbWidth = getTextDimensions(PCB_temperatureStr, false, 20).first;
                        drawString(
                            PCB_temperatureStr, false,
                            tsl::cfg::FramebufferWidth + offset - pcbWidth - chargeWidth - 25,
                            y_offset, 20,
                            ult::dynamicWidgetColors
                                ? tsl::GradientColor(pcbTemp)
                                : defaultTextColor
                        );
                    }
                    if (!ult::hideSOCTemp && socTemp > 0.0f) {
                        if (!ult::hidePCBTemp || !ult::hideBattery) offset -= 5;
                        socWidth = getTextDimensions(SOC_temperatureStr, false, 20).first;
                        drawString(
                            SOC_temperatureStr, false,
                            tsl::cfg::FramebufferWidth + offset - socWidth - pcbWidth - chargeWidth - 25,
                            y_offset, 20,
                            ult::dynamicWidgetColors
                                ? tsl::GradientColor(socTemp)
                                : defaultTextColor
                        );
                    }
                }
            }
        #endif

            // Single unified glyph cache for all text operations
            //inline static std::unordered_map<u64, Glyph> s_unifiedGlyphCache;
            
            // Helper to select appropriate font for a character
            inline std::shared_ptr<FontManager::Glyph> getOrCreateGlyph(u32 character, bool monospace, u32 fontSize) {
                return FontManager::getOrCreateGlyph(character, monospace, fontSize);
            }
            
            inline stbtt_fontinfo* selectFontForCharacter(u32 character) {
                return FontManager::selectFontForCharacter(character);
            }
            
            // Optimized glyph rendering
            inline void renderGlyph(std::shared_ptr<FontManager::Glyph> glyph, float x, float y, const Color& color, bool skipAlphaLimit = false) {
                if (!glyph->glyphBmp || color.a == 0) [[unlikely]] return;
                
                const s32 xPos = static_cast<s32>(x + glyph->bounds[0]);
                const s32 yPos = static_cast<s32>(y + glyph->bounds[1]);
                
                if (xPos >= cfg::FramebufferWidth || yPos >= cfg::FramebufferHeight ||
                    xPos + glyph->width <= 0 || yPos + glyph->height <= 0) [[unlikely]] return;
                
                const s32 startX = std::max(0, -xPos);
                const s32 startY = std::max(0, -yPos);
                const s32 endX = std::min(glyph->width, static_cast<s32>(cfg::FramebufferWidth) - xPos);
                const s32 endY = std::min(glyph->height, static_cast<s32>(cfg::FramebufferHeight) - yPos);
                const u8 alphaLimit = skipAlphaLimit ? 0xF : static_cast<u8>(0xF * Renderer::s_opacity);
                const uint8_t* bmpPtr = glyph->glyphBmp + startY * glyph->width;
                
                for (s32 bmpY = startY; bmpY < endY; ++bmpY, bmpPtr += glyph->width) {
                    const s32 pixelY = yPos + bmpY;
                    
                    for (s32 bmpX = startX; bmpX < endX; ++bmpX) {
                        u8 alpha = bmpPtr[bmpX] >> 4;
                        if (alpha == 0) [[unlikely]] continue;
                        
                        alpha = (alpha < alphaLimit) ? alpha : alphaLimit;
                        const s32 pixelX = xPos + bmpX;
                        
                        if (alpha == 0xF) [[likely]] {
                            this->setPixel(pixelX, pixelY, color);
                        } else {
                            this->setPixelBlendDst(pixelX, pixelY, Color(color.r, color.g, color.b, alpha));
                        }
                    }
                }
            }
            

            /**
             * @brief Adds the layer from screenshot and recording stacks
             */
            inline void addScreenshotStacks(bool forceDisable = true) {
                tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Screenshot);
                tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Recording);
                screenshotsAreDisabled.store(false, std::memory_order_release);
                if (forceDisable)
                    screenshotsAreForceDisabled.store(false, std::memory_order_release);
            }

            /**
             * @brief Removes the layer from screenshot and recording stacks
             */
            inline void removeScreenshotStacks(bool forceDisable = true) {
                tsl::hlp::viRemoveFromLayerStack(&this->m_layer, ViLayerStack_Screenshot);
                tsl::hlp::viRemoveFromLayerStack(&this->m_layer, ViLayerStack_Recording);
                screenshotsAreDisabled.store(true, std::memory_order_release);
                if (forceDisable)
                    screenshotsAreForceDisabled.store(true, std::memory_order_release);
            }
            
        private:
            Renderer() {
                updateDrawFunction();
            }
            
            /**
             * @brief Sets the opacity of the layer
             *
             * @param opacity Opacity
             */
            inline static void setOpacity(float opacity) {
                opacity = std::clamp(opacity, 0.0F, 1.0F);
                
                Renderer::s_opacity = opacity;
            }
            
            bool m_initialized = false;
            ViDisplay m_display;
            ViLayer m_layer;
            Event m_vsyncEvent;
            
            NWindow m_window;
            Framebuffer m_framebuffer;
            void *m_currentFramebuffer = nullptr;
            
            std::stack<ScissoringConfig> m_scissoringStack;
            

            static inline float s_opacity = 1.0F;
            
            
            /**
             * @brief Get the current framebuffer address
             *
             * @return Framebuffer address
             */
            inline void* getCurrentFramebuffer() {
                return this->m_currentFramebuffer;
            }
            
            /**
             * @brief Get the next framebuffer address
             *
             * @return Next framebuffer address
             */
            inline void* getNextFramebuffer() {
                return static_cast<u8*>(this->m_framebuffer.buf) + this->getNextFramebufferSlot() * this->getFramebufferSize();
            }
            
            /**
             * @brief Get the framebuffer size
             *
             * @return Framebuffer size
             */
            inline size_t getFramebufferSize() {
                return this->m_framebuffer.fb_size;
            }
            
            /**
             * @brief Get the number of framebuffers in use
             *
             * @return Number of framebuffers
             */
            inline size_t getFramebufferCount() {
                return this->m_framebuffer.num_fbs;
            }
            
            /**
             * @brief Get the currently used framebuffer's slot
             *
             * @return Slot
             */
            inline u8 getCurrentFramebufferSlot() {
                return this->m_window.cur_slot;
            }
            
            /**
             * @brief Get the next framebuffer's slot
             *
             * @return Next slot
             */
            inline u8 getNextFramebufferSlot() {
                return (this->getCurrentFramebufferSlot() + 1) % this->getFramebufferCount();
            }
            
            /**
             * @brief Waits for the vsync event
             *
             */
            inline void waitForVSync() {
                eventWait(&this->m_vsyncEvent, UINT64_MAX);
            }
            
            /**
             * @brief Decodes a x and y coordinate into a offset into the swizzled framebuffer
             *
             * @param x X pos
             * @param y Y Pos
             * @return Offset
             */

            inline u32 __attribute__((always_inline)) getPixelOffset(const u32 x, const u32 y) {
                // Check for scissoring boundaries
                if (!this->m_scissoringStack.empty()) {
                    const auto& currScissorConfig = this->m_scissoringStack.top();
                    if (x < currScissorConfig.x || y < currScissorConfig.y || 
                        x >= currScissorConfig.x_max || 
                        y >= currScissorConfig.y_max) {
                        return UINT32_MAX;
                    }
                }
                
                return ((((y & 127) >> 4) + ((x >> 5) << 3) + ((y >> 7) * offsetWidthVar)) << 9) +
                       ((y & 8) << 5) + ((x & 16) << 3) + ((y & 6) << 4) + 
                       ((x & 8) << 1) + ((y & 1) << 3) + (x & 7);
            }

            
            /**
             * @brief Initializes the renderer and layers
             *
             */
            void init() {
                // Get the underscan pixel values for both horizontal and vertical borders
                const auto [horizontalUnderscanPixels, verticalUnderscanPixels] = getUnderscanPixels();
                //int horizontalUnderscanPixels = 0;

                
                ult::useRightAlignment = (ult::parseValueFromIniSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, "right_alignment") == ult::TRUE_STR);

                //cfg::LayerPosX = 1280-32;
                cfg::LayerPosX = 0;
                cfg::LayerPosY = 0;
                cfg::FramebufferWidth  = ult::DefaultFramebufferWidth;
                cfg::FramebufferHeight = ult::DefaultFramebufferHeight;

                offsetWidthVar = (((cfg::FramebufferWidth / 2) >> 4) << 3);

                ult::correctFrameSize = (cfg::FramebufferWidth == 448 && cfg::FramebufferHeight == 720); // for detecting the correct Overlay display size
                if (ult::useRightAlignment && ult::correctFrameSize) {
                    cfg::LayerPosX = 1280-32 - horizontalUnderscanPixels;
                    ult::layerEdge = (1280-448);
                }

                cfg::LayerWidth  = cfg::ScreenWidth * (float(cfg::FramebufferWidth) / float(cfg::LayerMaxWidth));
                cfg::LayerHeight = cfg::ScreenHeight * (float(cfg::FramebufferHeight) / float(cfg::LayerMaxHeight));

                // Apply underscanning offset
                if (ult::DefaultFramebufferWidth == 1280 && ult::DefaultFramebufferHeight == 28) // for status monitor micro mode
                    cfg::LayerHeight += cfg::ScreenHeight/720. *verticalUnderscanPixels;
                else if (ult::correctFrameSize)
                    cfg::LayerWidth += horizontalUnderscanPixels;

                // NEW: Scale down to 1/4 size (0.5x in each dimension)
                //static constexpr float scaleFactor = 0.5f;
                //cfg::LayerWidth *= scaleFactor;
                //cfg::LayerHeight *= scaleFactor;
                
                if (this->m_initialized)
                    return;

                //s32 layerZ = 0;
                
                tsl::hlp::doWithSmSession([this, horizontalUnderscanPixels]{

                    ASSERT_FATAL(viInitialize(ViServiceType_Manager));
                    ASSERT_FATAL(viOpenDefaultDisplay(&this->m_display));
                    ASSERT_FATAL(viGetDisplayVsyncEvent(&this->m_display, &this->m_vsyncEvent));
                    ASSERT_FATAL(viCreateManagedLayer(&this->m_display, static_cast<ViLayerFlags>(0), 0, &__nx_vi_layer_id));
                    ASSERT_FATAL(viCreateLayer(&this->m_display, &this->m_layer));
                    ASSERT_FATAL(viSetLayerScalingMode(&this->m_layer, ViScalingMode_FitToLayer));
                    
                    //if (s32 layerZ = 0; R_SUCCEEDED(viGetZOrderCountMax(&this->m_display, &layerZ)) && layerZ > 0)
                    //    ASSERT_FATAL(viSetLayerZ(&this->m_layer, layerZ));

                    if (horizontalUnderscanPixels == 0) {
                        s32 layerZ = 0;
                        if (R_SUCCEEDED(viGetZOrderCountMax(&this->m_display, &layerZ)) && layerZ > 0) {
                            ASSERT_FATAL(viSetLayerZ(&this->m_layer, layerZ));
                        }
                        else {
                            ASSERT_FATAL(viSetLayerZ(&this->m_layer, 255)); // max value 255 as fallback
                        }
                    } else {
                        ASSERT_FATAL(viSetLayerZ(&this->m_layer, 34)); // 34 is the edge for underscanning
                    }

                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Default));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Screenshot));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Recording));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Arbitrary));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_LastFrame));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Null));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_ApplicationForDebug));
                    ASSERT_FATAL(tsl::hlp::viAddToLayerStack(&this->m_layer, ViLayerStack_Lcd));
                    
                    ASSERT_FATAL(viSetLayerSize(&this->m_layer, cfg::LayerWidth, cfg::LayerHeight));
                    ASSERT_FATAL(viSetLayerPosition(&this->m_layer, cfg::LayerPosX, cfg::LayerPosY));
                    ASSERT_FATAL(nwindowCreateFromLayer(&this->m_window, &this->m_layer));
                    ASSERT_FATAL(framebufferCreate(&this->m_framebuffer, &this->m_window, cfg::FramebufferWidth, cfg::FramebufferHeight, PIXEL_FORMAT_RGBA_4444, 2));
                    ASSERT_FATAL(setInitialize());
                    ASSERT_FATAL(this->initFonts());
                    setExit();
                });
                
                this->m_initialized = true;
            }
            
            /**
             * @brief Exits the renderer and layer
             *
             */
            void exit() {
                if (!this->m_initialized)
                    return;
                
                // Cleanup shared font manager
                FontManager::cleanup();

                framebufferClose(&this->m_framebuffer);
                nwindowClose(&this->m_window);
                viDestroyManagedLayer(&this->m_layer);
                viCloseDisplay(&this->m_display);
                eventClose(&this->m_vsyncEvent);
                viExit();
            }
            
            /**
             * @brief Initializes Nintendo's shared fonts. Default and Extended
             *
             * @return Result
             */
            Result initFonts() {
                PlFontData stdFontData, localFontData, extFontData;
                
                // Nintendo's default font
                TSL_R_TRY(plGetSharedFontByType(&stdFontData, PlSharedFontType_Standard));
                
                u8 *fontBuffer = reinterpret_cast<u8*>(stdFontData.address);
                stbtt_InitFont(&this->m_stdFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));
                
                u64 languageCode;
                if (R_SUCCEEDED(setGetSystemLanguage(&languageCode))) {
                    // Check if need localization font
                    SetLanguage setLanguage;
                    TSL_R_TRY(setMakeLanguage(languageCode, &setLanguage));
                    this->m_hasLocalFont = true;
                    switch (setLanguage) {
                    case SetLanguage_ZHCN:
                    case SetLanguage_ZHHANS:
                    case SetLanguage_ZHTW:
                    case SetLanguage_ZHHANT:
                        TSL_R_TRY(plGetSharedFontByType(&localFontData, PlSharedFontType_ChineseSimplified));
                        break;
                    case SetLanguage_KO:
                        TSL_R_TRY(plGetSharedFontByType(&localFontData, PlSharedFontType_KO));
                        break;
                    default:
                        this->m_hasLocalFont = false;
                        break;
                    }
                    
                    if (this->m_hasLocalFont) {
                        fontBuffer = reinterpret_cast<u8*>(localFontData.address);
                        stbtt_InitFont(&this->m_localFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));
                    }
                }
                
                // Nintendo's extended font containing a bunch of icons
                TSL_R_TRY(plGetSharedFontByType(&extFontData, PlSharedFontType_NintendoExt));
                
                fontBuffer = reinterpret_cast<u8*>(extFontData.address);
                stbtt_InitFont(&this->m_extFont, fontBuffer, stbtt_GetFontOffsetForIndex(fontBuffer, 0));
                
                // Initialize the shared font manager
                FontManager::initializeFonts(&this->m_stdFont, &this->m_localFont, 
                                           &this->m_extFont, this->m_hasLocalFont);
                
                return 0;
            }
            

            /**
             * @brief Start a new frame
             * @warning Don't call this more than once before calling \ref endFrame
             */
            inline void startFrame() {
                this->m_currentFramebuffer = framebufferBegin(&this->m_framebuffer, nullptr);
            }
            
            /**
             * @brief End the current frame
             * @warning Don't call this before calling \ref startFrame once
             */
            //inline void endFrame() {
            //#if IS_STATUS_MONITOR_DIRECTIVE
            //    if (isRendering) {
            //        static u32 lastFPS = 0;
            //        static u64 cachedIntervalNs = 1000000000ULL / 60; // Default to 60 FPS
            //
            //        u32 fps = TeslaFPS;
            //        if (__builtin_expect(fps != lastFPS, 0)) {
            //            cachedIntervalNs = (fps > 0) ? (1000000000ULL / fps) : cachedIntervalNs;
            //            lastFPS = fps;
            //        }
            //
            //        // Frame pacing before VSync
            //        leventWait(&renderingStopEvent, cachedIntervalNs);
            //    }
            //#endif
            //
            //    // Then hardware sync
            //    this->waitForVSync();
            //    framebufferEnd(&this->m_framebuffer);
            //    this->m_currentFramebuffer = nullptr;
            //
            //    if (tsl::clearGlyphCacheNow.exchange(false)) {
            //        tsl::gfx::FontManager::clearCache();
            //    }
            //}

            inline void endFrame() {
            #if IS_STATUS_MONITOR_DIRECTIVE
                if (isRendering) {
                    static u32 lastFPS = 0;
                    static u64 cachedIntervalNs = 1000000000ULL / 60;
                    
                    u32 fps = TeslaFPS;
                    if (__builtin_expect(fps != lastFPS, 0)) {
                        cachedIntervalNs = (fps > 0) ? (1000000000ULL / fps) : cachedIntervalNs;
                        lastFPS = fps;
                    }
                    
                    // Just wait - touch thread will signal if needed
                    leventWait(&renderingStopEvent, cachedIntervalNs);
                }
            #endif
            
                this->waitForVSync();
                framebufferEnd(&this->m_framebuffer);
                this->m_currentFramebuffer = nullptr;
            
                if (tsl::clearGlyphCacheNow.exchange(false, std::memory_order_acq_rel)) {
                    tsl::gfx::FontManager::clearCache();
                }
            }
            

        };

        static std::pair<int, int> getUnderscanPixels() {
            if (!ult::consoleIsDocked()) {
                return {0, 0};
            }
            
            // Retrieve the TV settings
            SetSysTvSettings tvSettings;
            Result res = setsysGetTvSettings(&tvSettings);
            if (R_FAILED(res)) {
                // Handle error: return default underscan or log error
                return {0, 0};
            }
            
            // The underscan value might not be a percentage, we need to interpret it correctly
            const u32 underscanValue = tvSettings.underscan;
            
            // Convert the underscan value to a fraction. Assuming 0 means no underscan and larger values represent
            // greater underscan. Adjust this formula based on actual observed behavior or documentation.
            const float underscanPercentage = 1.0f - (underscanValue / 100.0f);
            
            // Original dimensions of the full 720p image (1280x720)
            const float originalWidth = 1280;
            const float originalHeight = 720;
            
            // Adjust the width and height based on the underscan percentage
            const float adjustedWidth = (originalWidth * underscanPercentage);
            const float adjustedHeight = (originalHeight * underscanPercentage);
            
            // Calculate the underscan in pixels (left/right and top/bottom)
            const int horizontalUnderscanPixels = (originalWidth - adjustedWidth);
            const int verticalUnderscanPixels = (originalHeight - adjustedHeight);
            
            return {horizontalUnderscanPixels, verticalUnderscanPixels};
        }

    }
    
    
    // Elements
    
    namespace elm {
        
        enum class TouchEvent {
            Touch,
            Hold,
            Scroll,
            Release,
            None
        };
        
        /**
         * @brief The top level Element of the libtesla UI library
         * @note When creating your own elements, extend from this or one of it's sub classes
         */
        class Element {
        public:
            
            Element() {}
            virtual ~Element() {
                m_clickListener = {};   // frees captures immediately
            }
            
            bool m_isTable = false;  // Default to false for non-table elements
            bool m_isItem = true;
            

            u64 t_ns;  // Changed from chrono::duration to nanoseconds
            u8 saturation;
            float progress;
            
            s32 x, y;
            s32 amplitude;
            u64 m_animationStartTime; // Changed from chrono::time_point to nanoseconds
            
            virtual bool isTable() const {
                return m_isTable;
            }

            virtual bool isItem() const {
                return m_isItem;
            }

            /**
             * @brief Handles focus requesting
             * @note This function should return the element to focus.
             *       When this element should be focused, return `this`.
             *       When one of it's child should be focused, return `this->child->requestFocus(oldFocus, direction)`
             *       When this element is not focusable, return `nullptr`
             *
             * @param oldFocus Previously focused element
             * @param direction Direction in which focus moved. \ref FocusDirection::None is passed for the initial load
             * @return Element to focus
             */
            virtual inline Element* requestFocus(Element *oldFocus, FocusDirection direction) {
                return nullptr;
            }
            
            /**
             * @brief Function called when a joycon button got pressed
             *
             * @param keys Keys pressed in the last frame
             * @return true when button press has been consumed
             * @return false when button press should be passed on to the parent
             */
            virtual inline bool onClick(u64 keys) {
                return m_clickListener(keys);
            }
            
            /**
             * @brief Called once per frame with the latest HID inputs
             *
             * @param keysDown Buttons pressed in the last frame
             * @param keysHeld Buttons held down longer than one frame
             * @param touchInput Last touch position
             * @param leftJoyStick Left joystick position
             * @param rightJoyStick Right joystick position
             * @return Weather or not the input has been consumed
             */
            virtual inline bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
                return false;
            }
            
            /**
             * @brief Function called when the element got touched
             * @todo Not yet implemented
             *
             * @param x X pos
             * @param y Y pos
             * @return true when touch input has been consumed
             * @return false when touch input should be passed on to the parent
             */
            virtual inline bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) {
                return false;
            }
            
            /**
             * @brief Called once per frame to draw the element
             * @warning Do not call this yourself. Use \ref Element::frame(gfx::Renderer *renderer)
             *
             * @param renderer Renderer
             */
            virtual void draw(gfx::Renderer *renderer) = 0;
            
            /**
             * @brief Called when the underlying Gui gets created and after calling \ref Gui::invalidate() to calculate positions and boundaries of the element
             * @warning Do not call this yourself. Use \ref Element::invalidate()
             *
             * @param parentX Parent X pos
             * @param parentY Parent Y pos
             * @param parentWidth Parent Width
             * @param parentHeight Parent Height
             */
            virtual inline void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) = 0;
            
            /**
             * @brief Draws highlighting and the element itself
             * @note When drawing children of a element in \ref Element::draw(gfx::Renderer *renderer), use `this->child->frame(renderer)` instead of calling draw directly
             *
             * @param renderer
             */
            void inline frame(gfx::Renderer *renderer) {
                
                if (this->m_focused) {
                    renderer->enableScissoring(0, ult::activeHeaderHeight, tsl::cfg::FramebufferWidth, tsl::cfg::FramebufferHeight-73-ult::activeHeaderHeight);
                    this->drawFocusBackground(renderer);
                    this->drawHighlight(renderer);
                    renderer->disableScissoring();
                }
                
                this->draw(renderer);
            }
            
            /**
             * @brief Forces a layout recreation of a element
             *
             */
            void inline invalidate() {
                const auto& parent = this->getParent();
                
                if (parent == nullptr)
                    this->layout(0, 0, cfg::FramebufferWidth, cfg::FramebufferHeight);
                else
                    this->layout(ELEMENT_BOUNDS(parent));
            }
            
            /**
             * @brief Shake the highlight in the given direction to signal that the focus cannot move there
             *
             * @param direction Direction to shake highlight in
             */
            void inline shakeHighlight(FocusDirection direction) {
                this->m_highlightShaking = true;
                this->m_highlightShakingDirection = direction;
                this->m_highlightShakingStartTime = armTicksToNs(armGetSystemTick()); // Changed
                if (direction != FocusDirection::None && m_isItem) {
                    triggerRumbleClick.store(true, std::memory_order_release);
                    triggerWallSound.store(true, std::memory_order_release);
                }
            }
            
            /**
             * @brief Triggers the blue click animation to signal a element has been clicked on
             *
             */
            void inline triggerClickAnimation() {
                this->m_clickAnimationProgress = tsl::style::ListItemHighlightLength;
                this->m_animationStartTime = armTicksToNs(armGetSystemTick()); // Changed
            }


            
            /**
             * @brief Resets the click animation progress, canceling the animation
             */
            void inline resetClickAnimation() {
                this->m_clickAnimationProgress = 0;
            }
            
            /**
             * @brief Draws the blue highlight animation when clicking on a button
             * @note Override this if you have a element that e.g requires a non-rectangular animation or a different color
             *
             * @param renderer Renderer
             */
            virtual void drawClickAnimation(gfx::Renderer *renderer) {
                if (!m_isItem)
                    return;
                if (ult::useSelectionBG) {
                    if (ult::expandedMemory)
                        renderer->drawRectMultiThreaded(this->getX() + x + 4, this->getY() + y, this->getWidth() - 8, this->getHeight(), aWithOpacity(selectionBGColor)); // CUSTOM MODIFICATION 
                    else
                        renderer->drawRect(this->getX() + x + 4, this->getY() + y, this->getWidth() - 8, this->getHeight(), aWithOpacity(selectionBGColor));
                }
            
                saturation = tsl::style::ListItemHighlightSaturation * (float(this->m_clickAnimationProgress) / float(tsl::style::ListItemHighlightLength));
            
                Color animColor = {0xF,0xF,0xF,0xF};
                if (invertBGClickColor) {
                    const u8 inverted = 15-saturation;
                    animColor = {inverted, inverted, inverted, selectionBGColor.a};
                } else {
                    animColor = {saturation, saturation, saturation, selectionBGColor.a};
                }
                if (ult::expandedMemory)
                    renderer->drawRectMultiThreaded(ELEMENT_BOUNDS(this), aWithOpacity(animColor));
                else
                    renderer->drawRect(ELEMENT_BOUNDS(this), aWithOpacity(animColor));
            
                // Cache time calculation - only compute once
                static u64 lastTimeUpdate = 0;
                static double cachedProgress = 0.0;
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                
                // Only recalculate progress if enough time has passed (reduce computation frequency)
                if (currentTime_ns - lastTimeUpdate > 16666666) { // ~60 FPS update rate
                    //double time_seconds = currentTime_ns / 1000000000.0;
                    cachedProgress = (ult::cos(2.0 * ult::M_PI * std::fmod(currentTime_ns / 1000000000.0 - 0.25, 1.0)) + 1.0) / 2.0;
                    lastTimeUpdate = currentTime_ns;
                }
                progress = cachedProgress;
                
                Color clickColor1 = highlightColor1;
                Color clickColor2 = clickColor;
                
                if (progress >= 0.5) {
                    clickColor1 = clickColor;
                    clickColor2 = highlightColor2;
                }
                
                // Combine color interpolation into single calculation
                highlightColor = {
                    static_cast<u8>((clickColor1.r - clickColor2.r) * progress + clickColor2.r),
                    static_cast<u8>((clickColor1.g - clickColor2.g) * progress + clickColor2.g),
                    static_cast<u8>((clickColor1.b - clickColor2.b) * progress + clickColor2.b),
                    0xF
                };
                
                x = 0;
                y = 0;
                if (this->m_highlightShaking) {
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime;
                    const double t_ms = t_ns / 1000000.0;
                    
                    static constexpr double SHAKE_DURATION_MS = 200.0;
                    
                    if (t_ms >= SHAKE_DURATION_MS)
                        this->m_highlightShaking = false;
                    else {
                        // Generate random amplitude only once per shake using the start time as seed
                        const double amplitude = 6.0 + ((this->m_highlightShakingStartTime / 1000000) % 5);
                        const double progress = t_ms / SHAKE_DURATION_MS; // 0 to 1
                        
                        // Lighter damping so both bounces are visible
                        const double damping = 1.0 / (1.0 + 2.5 * progress * (1.0 + 1.3 * progress));
                        
                        // 2 full oscillations = 2 clear bounces
                        const double oscillation = ult::cos(ult::M_PI * 4.0 * progress);
                        const double displacement = amplitude * oscillation * damping;
                        const int offset = static_cast<int>(displacement);
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:    y = -offset; break;
                            case FocusDirection::Down:  y = offset; break;
                            case FocusDirection::Left:  x = -offset; break;
                            case FocusDirection::Right: x = offset; break;
                            default: break;
                        }
                    }
                }
                
                renderer->drawBorderedRoundedRect(this->getX() + x, this->getY() + y, this->getWidth() +4, this->getHeight(), 5, 5, a(highlightColor));
            }
            
            /**
             * @brief Draws the back background when a element is highlighted
             * @note Override this if you have a element that e.g requires a non-rectangular focus
             *
             * @param renderer Renderer
             */
            virtual void drawFocusBackground(gfx::Renderer *renderer) {
                if (this->m_clickAnimationProgress > 0) {
                    this->drawClickAnimation(renderer);
            
                    // Single time calculation and direct millisecond conversion
                    //const double elapsed_ms = (armTicksToNs(armGetSystemTick()) - this->m_animationStartTime) * 0.000001; // Direct conversion
            
                    // Direct calculation without intermediate multiplication
                    this->m_clickAnimationProgress = tsl::style::ListItemHighlightLength * (1.0f - ((armTicksToNs(armGetSystemTick()) - this->m_animationStartTime) * 0.000001) * 0.002f); // 0.002f = 1/500
            
                    // Clamp to 0 in one operation
                    if (this->m_clickAnimationProgress < 0) {
                        this->m_clickAnimationProgress = 0;
                    }
                }
            }
            
            /**
             * @brief Draws the blue boarder when a element is highlighted
             * @note Override this if you have a element that e.g requires a non-rectangular focus
             *
             * @param renderer Renderer
             */
            virtual void drawHighlight(gfx::Renderer *renderer) { // CUSTOM MODIFICATION start
                if (!m_isItem)
                    return;
                
                // Use cached time calculation from drawClickAnimation if possible
                static u64 lastHighlightUpdate = 0;
                static double cachedHighlightProgress = 0.0;
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                
                // Update progress at 60 FPS rate with high-precision calculation
                if (currentTime_ns - lastHighlightUpdate > 16666666) {
                    // High precision time calculation - matches original timing exactly
                    //double time_seconds = currentTime_ns * 0.000000001; // Direct conversion like original
                    
                    // Match original calculation exactly but with higher precision
                    cachedHighlightProgress = (ult::cos(2.0 * ult::M_PI * std::fmod(currentTime_ns * 0.000000001 - 0.25, 1.0)) + 1.0) * 0.5;
                    
                    lastHighlightUpdate = currentTime_ns;
                }
                progress = cachedHighlightProgress;
            
                // Cache the interpreter state check result to avoid atomic load overhead
                static bool lastInterpreterState = false;
                static u64 lastInterpreterCheck = 0;
                if (currentTime_ns - lastInterpreterCheck > 50000000) { // Check every 50ms
                    lastInterpreterState = ult::runningInterpreter.load(std::memory_order_acquire);
                    lastInterpreterCheck = currentTime_ns;
                }
            
                if (lastInterpreterState) {
                    // High precision floating point color interpolation for interpreter colors
                    highlightColor = {
                        static_cast<u8>(highlightColor4.r + (highlightColor3.r - highlightColor4.r) * progress + 0.5),
                        static_cast<u8>(highlightColor4.g + (highlightColor3.g - highlightColor4.g) * progress + 0.5),
                        static_cast<u8>(highlightColor4.b + (highlightColor3.b - highlightColor4.b) * progress + 0.5),
                        0xF
                    };
                } else {
                    // High precision floating point color interpolation for normal colors
                    highlightColor = {
                        static_cast<u8>(highlightColor2.r + (highlightColor1.r - highlightColor2.r) * progress + 0.5),
                        static_cast<u8>(highlightColor2.g + (highlightColor1.g - highlightColor2.g) * progress + 0.5),
                        static_cast<u8>(highlightColor2.b + (highlightColor1.b - highlightColor2.b) * progress + 0.5),
                        0xF
                    };
                }
                
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime;
                    const double t_ms = t_ns / 1000000.0;
                    
                    static constexpr double SHAKE_DURATION_MS = 200.0;
                    
                    if (t_ms >= SHAKE_DURATION_MS)
                        this->m_highlightShaking = false;
                    else {
                        // Generate random amplitude only once per shake using the start time as seed
                        const double amplitude = 6.0 + ((this->m_highlightShakingStartTime / 1000000) % 5);
                        const double progress = t_ms / SHAKE_DURATION_MS; // 0 to 1
                        
                        // Lighter damping so both bounces are visible
                        const double damping = 1.0 / (1.0 + 2.5 * progress * (1.0 + 1.3 * progress));
                        
                        // 2 full oscillations = 2 clear bounces
                        const double oscillation = ult::cos(ult::M_PI * 4.0 * progress);
                        const double displacement = amplitude * oscillation * damping;
                        const int offset = static_cast<int>(displacement);
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:    y = -offset; break;
                            case FocusDirection::Down:  y = offset; break;
                            case FocusDirection::Left:  x = -offset; break;
                            case FocusDirection::Right: x = offset; break;
                            default: break;
                        }
                    }
                }
                
                if (this->m_clickAnimationProgress == 0) {
                    if (ult::useSelectionBG) {
                        if (ult::expandedMemory)
                            renderer->drawRectMultiThreaded(this->getX() + x + 4, this->getY() + y, this->getWidth() - 12 +4, this->getHeight(), aWithOpacity(selectionBGColor)); // CUSTOM MODIFICATION 
                        else
                            renderer->drawRect(this->getX() + x + 4, this->getY() + y, this->getWidth() - 12 +4, this->getHeight(), aWithOpacity(selectionBGColor));
                    }
            
                    #if IS_LAUNCHER_DIRECTIVE
                    // Determine the active percentage to use
                    const float activePercentage = ult::displayPercentage.load(std::memory_order_acquire);
                    if (activePercentage > 0){
                        if (ult::expandedMemory)
                            renderer->drawRectMultiThreaded(this->getX() + x + 4, this->getY() + y, (this->getWidth()- 12 +4)*(activePercentage * 0.01f), this->getHeight(), aWithOpacity(progressColor)); // Direct percentage conversion
                        else
                            renderer->drawRect(this->getX() + x + 4, this->getY() + y, (this->getWidth()- 12 +4)*(activePercentage * 0.01f), this->getHeight(), aWithOpacity(progressColor)); // Direct percentage conversion
                    }
                    #endif
            
                    renderer->drawBorderedRoundedRect(this->getX() + x, this->getY() + y, this->getWidth() +4, this->getHeight(), 5, 5, a(highlightColor));
                }
                
                ult::onTrackBar.store(false, std::memory_order_release);
            }
            
            
            
            /**
             * @brief Sets the boundaries of this view
             *
             * @param x Start X pos
             * @param y Start Y pos
             * @param width Width
             * @param height Height
             */
            inline void setBoundaries(s32 x, s32 y, s32 width, s32 height) {
                this->m_x = x;
                this->m_y = y;
                this->m_width = width;
                this->m_height = height;
            }
            
            /**
             * @brief Adds a click listener to the element
             *
             * @param clickListener Click listener called with keys that were pressed last frame. Callback should return true if keys got consumed
             */
            virtual inline void setClickListener(std::function<bool(u64 keys)> clickListener) {
                this->m_clickListener = clickListener;
            }
            
            /**
             * @brief Gets the element's X position
             *
             * @return X position
             */
            inline s32 getX() { return this->m_x; }
            /**
             * @brief Gets the element's Y position
             *
             * @return Y position
             */
            inline s32 getY() { return this->m_y; }
            /**
             * @brief Gets the element's Width
             *
             * @return Width
             */
            inline s32 getWidth() { return this->m_width;  }
            /**
             * @brief Gets the element's Height
             *
             * @return Height
             */
            inline s32 getHeight() { return this->m_height; }
            
            inline s32 getTopBound() { return this->getY(); }
            inline s32 getLeftBound() { return this->getX(); }
            inline s32 getRightBound() { return this->getX() + this->getWidth(); }
            inline s32 getBottomBound() { return this->getY() + this->getHeight(); }
            
            /**
             * @brief Check if the coordinates are in the elements bounds
             *
             * @return true if coordinates are in bounds, false otherwise
             */
            bool inBounds(s32 touchX, s32 touchY) {
                //static u32 ult::layerEdge = cfg::LayerPosX == 0 ? 0 : (1280-448);
                return touchX >= this->getLeftBound() + int(ult::layerEdge) && touchX <= this->getRightBound() + int(ult::layerEdge) && touchY >= this->getTopBound() && touchY <= this->getBottomBound();
            }
            
            /**
             * @brief Sets the element's parent
             * @note This is required to handle focus and button downpassing properly
             *
             * @param parent Parent
             */
            inline void setParent(Element *parent) { this->m_parent = parent; }
            
            /**
             * @brief Get the element's parent
             *
             * @return Parent
             */
            inline Element* getParent() { return this->m_parent; }
            

            virtual inline std::vector<Element*> getChildren() const {
                return {}; // Return empty vector for simplicity
            }

            /**
             * @brief Marks this element as focused or unfocused to draw the highlight
             *
             * @param focused Focused
             */
            virtual inline void setFocused(bool focused) {
                this->m_focused = focused;
                this->m_clickAnimationProgress = 0;
            }

            virtual bool matchesJumpCriteria(const std::string& jumpText, const std::string& jumpValue, bool contains) const {
                return false; // Default implementation for non-ListItem elements
            }
            
            
            static InputMode getInputMode() { return Element::s_inputMode; }
            
            static void setInputMode(InputMode mode) { Element::s_inputMode = mode; }
            
        protected:
            constexpr static inline auto a = &gfx::Renderer::a;
            constexpr static inline auto aWithOpacity = &gfx::Renderer::aWithOpacity;
            bool m_focused = false;
            u8 m_clickAnimationProgress = 0;
            
            // Highlight shake animation
            bool m_highlightShaking = false;
            u64 m_highlightShakingStartTime; // Changed from chrono::time_point to nanoseconds
            FocusDirection m_highlightShakingDirection;
            
            static inline InputMode s_inputMode;
            
            /**
             * @brief Shake animation calculation based on a damped sine wave
             *
             * @param t_ns Passed time in nanoseconds
             * @param a Amplitude
             * @return Damped sine wave output
             */
           //inline int shakeAnimation(u64 t_ns, float a) {
           //    //float w = 0.2F;
           //    //float tau = 0.05F;
           //    
           //    // Convert nanoseconds to microseconds for the calculation
           //    const int t_us = t_ns / 1000;
           //    
           //    return roundf(a * exp(-(0.05F * t_us) * sin(0.2F * t_us)));
           //}
            
        private:
            friend class Gui;
            
            s32 m_x = 0, m_y = 0, m_width = 0, m_height = 0;
            Element *m_parent = nullptr;
            std::vector<Element*> m_children;
            std::function<bool(u64 keys)> m_clickListener = [](u64) { return false; };
        };

        /**
         * @brief A Element that exposes the renderer directly to draw custom views easily
         */
        class CustomDrawer : public Element {
        public:
            /**
             * @brief Constructor
             * @note This element should only be used to draw static things the user cannot interact with e.g info text, images, etc.
             *
             * @param renderFunc Callback that will be called once every frame to draw this view
             */
            CustomDrawer(std::function<void(gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h)> renderFunc) : Element(), m_renderFunc(renderFunc) {
                m_isItem = false;
                m_isTable = true;
            }

            virtual ~CustomDrawer() {}
            
            virtual void draw(gfx::Renderer* renderer) override {
                //renderer->enableScissoring(ELEMENT_BOUNDS(this));
                this->m_renderFunc(renderer, ELEMENT_BOUNDS(this));
                //renderer->disableScissoring();
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                
            }
            
        private:
            std::function<void(gfx::Renderer*, s32 x, s32 y, s32 w, s32 h)> m_renderFunc;
        };
    //#endif

        /**
         * @brief A Element that exposes the renderer directly to draw custom views easily
         */
        class TableDrawer : public Element {
        public:
            TableDrawer(std::function<void(gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h)> renderFunc, bool _hideTableBackground, size_t _endGap, bool _isScrollable = false)
                : Element(), m_renderFunc(renderFunc), hideTableBackground(_hideTableBackground), endGap(_endGap), isScrollable(_isScrollable) {
                    m_isTable = isScrollable;  // Mark this element as a table
                    m_isItem = false;
                }
            
            virtual ~TableDrawer() {}

            virtual void draw(gfx::Renderer* renderer) override {

                renderer->enableScissoring(0, 88, tsl::cfg::FramebufferWidth, tsl::cfg::FramebufferHeight - 73 - 97 +2+5);
                
                if (!hideTableBackground)
                    renderer->drawRoundedRect(this->getX() + 4+2, this->getY()-4-1, this->getWidth() +2 + 1, this->getHeight() + 20 - endGap+2, 12.0, aWithOpacity(tableBGColor));
                
                m_renderFunc(renderer, this->getX() + 4, this->getY(), this->getWidth() + 4, this->getHeight());
                
                renderer->disableScissoring();
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {}


            virtual bool onClick(u64 keys) {
                return false;
            }
            
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return nullptr;
            }
        
        private:
            std::function<void(gfx::Renderer*, s32 x, s32 y, s32 w, s32 h)> m_renderFunc;
            bool hideTableBackground = false;
            size_t endGap = 3;
            bool isScrollable = false;
        };


        #if IS_LAUNCHER_DIRECTIVE
        // Simple utility function to draw the dynamic "Ultra" part of the logo
        static s32 drawDynamicUltraText(gfx::Renderer* renderer, s32 startX, s32 y, u32 fontSize, 
                                       const tsl::Color& staticColor, bool useNotificationMethod = false) {
            static constexpr double cycleDuration = 1.6;
            s32 currentX = startX;
            
            if (ult::useDynamicLogo) {
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const double currentTimeCount = static_cast<double>(currentTime_ns) / 1000000000.0;
                const double timeBase = std::fmod(currentTimeCount, cycleDuration);
                const double waveScale = 2.0 * ult::M_PI / cycleDuration;
                static constexpr double phaseShift = ult::M_PI / 2.0;
                
                float countOffset = 0;
                for (const char letter : ult::SPLIT_PROJECT_NAME_1) {
                    const double wavePhase = waveScale * (timeBase + static_cast<double>(countOffset));
                    const double rawProgress = ult::cos(wavePhase - phaseShift);
                    
                    const double normalizedProgress = (rawProgress + 1.0) * 0.5;
                    const double smoothedProgress = normalizedProgress * normalizedProgress * (3.0 - 2.0 * normalizedProgress);
                    const double ultraSmoothProgress = smoothedProgress * smoothedProgress * (3.0 - 2.0 * smoothedProgress);
                    
                    const double blend = std::max(0.0, std::min(1.0, ultraSmoothProgress));
                    
                    const tsl::Color highlightColor = {
                        static_cast<u8>(dynamicLogoRGB1.r + (dynamicLogoRGB2.r - dynamicLogoRGB1.r) * blend + 0.5),
                        static_cast<u8>(dynamicLogoRGB1.g + (dynamicLogoRGB2.g - dynamicLogoRGB1.g) * blend + 0.5),
                        static_cast<u8>(dynamicLogoRGB1.b + (dynamicLogoRGB2.b - dynamicLogoRGB1.b) * blend + 0.5),
                        15
                    };
                    
                    const std::string letterStr(1, letter);
                    if (useNotificationMethod) {
                        //const auto [letterWidth, letterHeight] = renderer->drawNotificationString(letterStr, false, currentX, y, fontSize, highlightColor);
                        currentX += renderer->drawNotificationString(letterStr, false, currentX, y, fontSize, highlightColor).first;
                    } else {
                        currentX += renderer->drawString(letterStr, false, currentX, y, fontSize, highlightColor).first;
                    }
                    countOffset -= static_cast<float>(cycleDuration / 8.0);
                }
            } else {
                // Static rendering
                for (const char letter : ult::SPLIT_PROJECT_NAME_1) {
                    const std::string letterStr(1, letter);
                    if (useNotificationMethod) {
                        //const auto [letterWidth, letterHeight] = renderer->drawNotificationString(letterStr, false, currentX, y, fontSize, staticColor);
                        currentX += renderer->drawNotificationString(letterStr, false, currentX, y, fontSize, staticColor).first;
                    } else {
                        currentX += renderer->drawString(letterStr, false, currentX, y, fontSize, staticColor).first;
                    }
                }
            }
            
            return currentX;
        }
        
        // Utility function to calculate width of the Ultra text (for notification centering)
        static s32 calculateUltraTextWidth(gfx::Renderer* renderer, u32 fontSize, bool useNotificationMethod = false) {
            s32 totalWidth = 0;
            
            if (ult::useDynamicLogo) {
                // Calculate width by measuring each character for dynamic rendering
                for (const char letter : ult::SPLIT_PROJECT_NAME_1) {
                    const std::string letterStr(1, letter);
                    if (useNotificationMethod) {
                        //const auto [lw, lh] = renderer->getNotificationTextDimensions(letterStr, false, fontSize);
                        totalWidth += renderer->getNotificationTextDimensions(letterStr, false, fontSize).first;
                    } else {
                        //const auto [lw, lh] = renderer->getTextDimensions(letterStr, false, fontSize);
                        totalWidth += renderer->getTextDimensions(letterStr, false, fontSize).first;
                    }
                }
            } else {
                // Static rendering - measure the whole string at once
                if (useNotificationMethod) {
                    //const auto [uw, uh] = renderer->getNotificationTextDimensions(ult::SPLIT_PROJECT_NAME_1, false, fontSize);
                    totalWidth = renderer->getNotificationTextDimensions(ult::SPLIT_PROJECT_NAME_1, false, fontSize).first;
                } else {
                    //const auto [uw, uh] = renderer->getTextDimensions(ult::SPLIT_PROJECT_NAME_1, false, fontSize);
                    totalWidth = renderer->getTextDimensions(ult::SPLIT_PROJECT_NAME_1, false, fontSize).first;
                }
            }
            
            return totalWidth;
        }

        #endif

        struct TopCache {
            std::string title;
            std::string subtitle;
            tsl::Color titleColor{0xF, 0xF, 0xF, 0xF}; // white by default
            bool widgetDrawn = false;
            bool useDynamicLogo = false;
            bool disabled = false;
        };
        
        struct BottomCache {
            std::string bottomText;
            float backWidth = 0.0f;
            float selectWidth = 0.0f;
            float nextPageWidth = 0.0f;
            bool disabled = false;
        };
        
        // Global or namespace-level variable
        inline TopCache g_cachedTop;
        inline BottomCache g_cachedBottom;

        inline std::atomic<bool> g_disableMenuCacheOnReturn = false;

        /**
         * @brief The base frame which can contain another view
         *
         */
        class OverlayFrame : public Element {
        public:
            /**
             * @brief Constructor
             *
             * @param title Name of the Overlay drawn bolt at the top
             * @param subtitle Subtitle drawn bellow the title e.g version number
             */
            std::string m_title;
            std::string m_subtitle;
        
            bool m_noClickableItems;
        
        #if IS_LAUNCHER_DIRECTIVE
            std::string m_menuMode; // CUSTOM MODIFICATION
            std::string m_colorSelection; // CUSTOM MODIFICATION
            std::string m_pageLeftName; // CUSTOM MODIFICATION
            std::string m_pageRightName; // CUSTOM MODIFICATION
            
            
            tsl::Color titleColor = {0xF,0xF,0xF,0xF};
            float letterWidth;
        #endif

        #if USING_WIDGET_DIRECTIVE
            bool m_showWidget = false;
        #endif
        
            float x, y;
            int offset, y_offset;
            int fontSize;

        #if IS_LAUNCHER_DIRECTIVE
            OverlayFrame(const std::string& title, const std::string& subtitle, const bool& _noClickableItems=false, const std::string& menuMode = "", const std::string& colorSelection = "", const std::string& pageLeftName = "", const std::string& pageRightName = "")
                : Element(), m_title(title), m_subtitle(subtitle), m_noClickableItems(_noClickableItems), m_menuMode(menuMode), m_colorSelection(colorSelection), m_pageLeftName(pageLeftName), m_pageRightName(pageRightName) {
        #else
            OverlayFrame(const std::string& title, const std::string& subtitle, const bool& _noClickableItems=false)
                : Element(), m_title(title), m_subtitle(subtitle), m_noClickableItems(_noClickableItems) {
        #endif
                    ult::activeHeaderHeight = 97;
                    ult::loadWallpaperFileWhenSafe();
                    m_isItem = false;
                    disableSound.store(false, std::memory_order_release);
                }
        
            ~OverlayFrame() {
                delete m_contentElement;
                
                // Check if returning from a list that disabled caching
                if (g_disableMenuCacheOnReturn.exchange(false, std::memory_order_acq_rel)) {
                    g_cachedTop.disabled = true;
                    g_cachedBottom.disabled = true;
                }
            }
        
        #if USING_FPS_INDICATOR_DIRECTIVE
            // Function to calculate FPS
            inline float updateFPS(double currentTimeCount) {
                static double lastUpdateTime = currentTimeCount;
                static int frameCount = 0;
                static float fps = 0.0f;
            
                ++frameCount;
                const double elapsedTime = currentTimeCount - lastUpdateTime;
            
                if (elapsedTime >= 1.0) { // Update FPS every second
                    fps = frameCount / static_cast<float>(elapsedTime);
                    lastUpdateTime = currentTimeCount;
                    frameCount = 0;
                }
                return fps;
            }
        #endif
            
            // CUSTOM SECTION START
            void draw(gfx::Renderer *renderer) override {
                if (!ult::themeIsInitialized.exchange(true, std::memory_order_acq_rel)) {
                    tsl::initializeThemeVars();
                }
                
            
                renderer->fillScreen(a(defaultBackgroundColor));
                renderer->drawWallpaper();
                
                y = 50;
                offset = 0;
                
            #if IS_LAUNCHER_DIRECTIVE
                // Current interpreter state (atomic<bool>)
                const bool interpreterIsRunningNow = ult::runningInterpreter.load(std::memory_order_acquire) && (ult::downloadPercentage.load(std::memory_order_acquire) != -1 || ult::unzipPercentage.load(std::memory_order_acquire) != -1 || ult::copyPercentage.load(std::memory_order_acquire) != -1);
                
                if (m_noClickableItems != ult::noClickableItems.load(std::memory_order_acquire)) {
                    ult::noClickableItems.store(m_noClickableItems, std::memory_order_release);
                }
            
                const bool isUltrahandMenu = (m_title == ult::CAPITAL_ULTRAHAND_PROJECT_NAME && 
                                        m_subtitle.find("Ultrahand Package") == std::string::npos && 
                                        m_subtitle.find("Ultrahand Script") == std::string::npos);
                
                // Determine if we should use cached data (first frame of new overlay)
                const bool useCachedTop = !g_cachedTop.disabled && 
                                          !g_cachedTop.title.empty() && 
                                          (g_cachedTop.title != m_title || g_cachedTop.subtitle != m_subtitle);
                
                // Use cached or current data for rendering
                const std::string& renderTitle = useCachedTop ? g_cachedTop.title : m_title;
                const std::string& renderSubtitle = useCachedTop ? g_cachedTop.subtitle : m_subtitle;
                const tsl::Color& renderTitleColor = useCachedTop ? g_cachedTop.titleColor : titleColor;
                const bool renderUseDynamicLogo = useCachedTop ? g_cachedTop.useDynamicLogo : ult::useDynamicLogo;
                
                const bool renderIsUltrahandMenu = (renderTitle == ult::CAPITAL_ULTRAHAND_PROJECT_NAME && 
                                                     renderSubtitle.find("Ultrahand Package") == std::string::npos && 
                                                     renderSubtitle.find("Ultrahand Script") == std::string::npos);
                
                if (renderIsUltrahandMenu) {
            #if USING_WIDGET_DIRECTIVE
                    if (useCachedTop) {
                        if (g_cachedTop.widgetDrawn) {
                            renderer->drawWidget();
                        }
                    } else {
                        renderer->drawWidget();
                    }
            #endif
            
                    if (ult::touchingMenu.load(std::memory_order_acquire) && (ult::inMainMenu.load(std::memory_order_acquire) ||
                        (ult::inHiddenMode.load(std::memory_order_acquire) && !ult::inSettingsMenu.load(std::memory_order_acquire) && !ult::inSubSettingsMenu.load(std::memory_order_acquire)))) {
                        renderer->drawRoundedRect(0.0f + 7, 12.0f, 245.0f - 13, 73.0f, 10.0f, a(clickColor));
                    }
                    
                    x = 20;
                    fontSize = 42;
                    offset = 6;
                    
                    if (renderUseDynamicLogo) {
                        x = drawDynamicUltraText(renderer, x, y + offset, fontSize, logoColor1, false);
                    } else {
                        for (const char letter : ult::SPLIT_PROJECT_NAME_1) {
                            const std::string letterStr(1, letter);
                            x += renderer->drawString(letterStr, false, x, y + offset, fontSize, logoColor1).first;
                        }
                    }
                    
                    renderer->drawString(ult::SPLIT_PROJECT_NAME_2, false, x, y + offset, fontSize, logoColor2);
                    
                } else {
                    if (useCachedTop) {
                        if (g_cachedTop.widgetDrawn) {
                            renderer->drawWidget();
                        }
                    } else {
                        if (m_showWidget) {
                            renderer->drawWidget();
                        }
                    }
            
                    x = 20;
                    y = 52 - 2;
                    fontSize = 32;
            
                    if (renderSubtitle.find("Ultrahand Script") != std::string::npos) {
                        renderer->drawString(renderTitle, false, x, y, fontSize, defaultScriptColor);
                    } else {
                        tsl::Color drawColor = defaultPackageColor; // Default to green
                        
                        if (!useCachedTop) {
                            // Calculate color only if not using cache
                            if (!m_colorSelection.empty()) {
                                const char firstChar = m_colorSelection[0];
                                const size_t len = m_colorSelection.length();
                                
                                // Fast path: check first char + length for unique combinations
                                switch (firstChar) {
                                    case 'g': // green
                                        if (len == 5 && m_colorSelection.compare("green") == 0) {
                                            drawColor = {0x0, 0xF, 0x0, 0xF};
                                        }
                                        break;
                                    case 'r': // red
                                        if (len == 3 && m_colorSelection.compare("red") == 0) {
                                            //drawColor = RGB888("#F7253E");
                                            drawColor = {0xF, 0x2, 0x4, 0xF};
                                        }
                                        break;
                                    case 'b': // blue
                                        if (len == 4 && m_colorSelection.compare("blue") == 0) {
                                            drawColor = {0x7, 0x7, 0xF, 0xF};
                                        }
                                        break;
                                    case 'y': // yellow
                                        if (len == 6 && m_colorSelection.compare("yellow") == 0) {
                                            drawColor = {0xF, 0xF, 0x0, 0xF};
                                        }
                                        break;
                                    case 'o': // orange
                                        if (len == 6 && m_colorSelection.compare("orange") == 0) {
                                            //drawColor = {0xFF, 0xA5, 0x00, 0xFF};
                                            drawColor = {0xF, 0xA, 0x0, 0xF};
                                        }
                                        break;
                                    case 'p': // pink or purple
                                        if (len == 4 && m_colorSelection.compare("pink") == 0) {
                                            //drawColor = {0xFF, 0x69, 0xB4, 0xFF};
                                            drawColor = {0xF, 0x6, 0xB, 0xF};
                                        } else if (len == 6 && m_colorSelection.compare("purple") == 0) {
                                            //drawColor = {0x80, 0x00, 0x80, 0xFF};
                                            drawColor = {0x8, 0x0, 0x8, 0xF};
                                        }
                                        break;
                                    case 'w': // white
                                        if (len == 5 && m_colorSelection.compare("white") == 0) {
                                            drawColor = {0xF, 0xF, 0xF, 0xF};
                                        }
                                        break;
                                    case '#': // hex color
                                        if (len == 7 && ult::isValidHexColor(m_colorSelection.substr(1))) {
                                            drawColor = RGB888(m_colorSelection.substr(1));
                                        }
                                        break;
                                }
                            }
                            titleColor = drawColor;
                        } else {
                            drawColor = renderTitleColor;
                        }
                        
                        renderer->drawString(renderTitle, false, x, y, fontSize, drawColor);
                        y += 2;
                    }
                }
                
                static const std::vector<std::string> specialChars2 = {""};
                if (renderTitle == ult::CAPITAL_ULTRAHAND_PROJECT_NAME) {
                    renderer->drawStringWithColoredSections(ult::versionLabel, false, specialChars2, 20, y+25, 15, bannerVersionTextColor, textSeparatorColor);
                } else {
                    std::string subtitle = renderSubtitle;
                    const size_t pos = subtitle.find("?Ultrahand Script");
                    if (pos != std::string::npos) {
                        subtitle.erase(pos, 17); // "?Ultrahand Script".length() = 17
                    }
                    renderer->drawStringWithColoredSections(subtitle, false, specialChars2, 20, y+23, 15, bannerVersionTextColor, textSeparatorColor);
                }
                
                // Update top cache after rendering for next frame
                g_cachedTop.title = m_title;
                g_cachedTop.subtitle = m_subtitle;
                g_cachedTop.titleColor = titleColor;
                g_cachedTop.useDynamicLogo = ult::useDynamicLogo;
                // Store whether widget was ACTUALLY drawn this frame
                if (isUltrahandMenu) {
                    g_cachedTop.widgetDrawn = true;  // Ultrahand menu always shows widget
                } else {
                    g_cachedTop.widgetDrawn = m_showWidget;  // Other menus use m_showWidget
                }
                g_cachedTop.disabled = false;
            
            #else
                // NON-LAUNCHER PATH WITH CACHE SUPPORT
                if (m_noClickableItems != ult::noClickableItems.load(std::memory_order_acquire)) {
                    ult::noClickableItems.store(m_noClickableItems, std::memory_order_release);
                }
                
                // Determine if we should use cached data (first frame of new overlay)
                const bool useCachedTop = !g_cachedTop.disabled && 
                                          !g_cachedTop.title.empty() && 
                                          (g_cachedTop.title != m_title || g_cachedTop.subtitle != m_subtitle);
                
                // Use cached or current data for rendering
                const std::string& renderTitle = useCachedTop ? g_cachedTop.title : m_title;
                const std::string& renderSubtitle = useCachedTop ? g_cachedTop.subtitle : m_subtitle;
                
            #if USING_WIDGET_DIRECTIVE
                if (useCachedTop) {
                    if (g_cachedTop.widgetDrawn) {
                        renderer->drawWidget();
                    }
                } else {
                    if (m_showWidget)
                        renderer->drawWidget();
                }
            #endif
                
                renderer->drawString(renderTitle, false, 20, 52-2, 32, defaultOverlayColor);
                renderer->drawString(renderSubtitle, false, 20, y+2+23, 15, bannerVersionTextColor);
                
                // Update top cache after rendering for next frame
                g_cachedTop.title = m_title;
                g_cachedTop.subtitle = m_subtitle;
                g_cachedTop.titleColor = {0xF, 0xF, 0xF, 0xF};
            #if USING_WIDGET_DIRECTIVE
                g_cachedTop.widgetDrawn = m_showWidget;
            #else
                g_cachedTop.widgetDrawn = false;
            #endif
                g_cachedTop.useDynamicLogo = false;
                g_cachedTop.disabled = false;
            #endif
            
                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(bottomSeparatorColor));
            
                // Compute gap width once from GAP_1 and derive halfGap
                const float gapWidth = renderer->getTextDimensions(ult::GAP_1, false, 23).first;
                
                // Calculate text widths for buttons depending on launch mode and interpreter state
            #if IS_LAUNCHER_DIRECTIVE
                const float backTextWidth = renderer->getTextDimensions(
                    "\uE0E1" + ult::GAP_2 + (!interpreterIsRunningNow ? ult::BACK : ult::HIDE), false, 23).first;
                const float selectTextWidth = renderer->getTextDimensions(
                    "\uE0E0" + ult::GAP_2 + (!interpreterIsRunningNow ? ult::OK : ult::CANCEL), false, 23).first;
            #else
                const float backTextWidth = renderer->getTextDimensions(
                    "\uE0E1" + ult::GAP_2 + ult::BACK, false, 23).first;
                const float selectTextWidth = renderer->getTextDimensions(
                    "\uE0E0" + ult::GAP_2 + ult::OK, false, 23).first;
            #endif
                
                const float _halfGap = gapWidth / 2.0f;
                if (_halfGap != ult::halfGap.load(std::memory_order_acquire))
                    ult::halfGap.store(_halfGap, std::memory_order_release);
            
                // Total button widths include half-gap padding on both sides
                const float _backWidth = backTextWidth + gapWidth;
                if (_backWidth != ult::backWidth.load(std::memory_order_acquire))
                    ult::backWidth.store(_backWidth, std::memory_order_release);
                const float _selectWidth = selectTextWidth + gapWidth;
                if (_selectWidth != ult::selectWidth.load(std::memory_order_acquire))
                    ult::selectWidth.store(_selectWidth, std::memory_order_release);
                
                // Set initial button position
                static constexpr float buttonStartX = 30;
                const float buttonY = static_cast<float>(cfg::FramebufferHeight - 73 + 1);
                
                // Draw back button if touched
                if (ult::touchingBack) {
                    renderer->drawRoundedRect(buttonStartX+2 - _halfGap, buttonY, _backWidth-1, 73.0f, 10.0f, a(clickColor));
                }
                
                // Draw select button (to the right of back) if touched
                if (ult::touchingSelect.load(std::memory_order_acquire) && !m_noClickableItems) {
                    renderer->drawRoundedRect(buttonStartX+2 - _halfGap + _backWidth+1, buttonY,
                                              _selectWidth-2, 73.0f, 10.0f, a(clickColor));
                }
                
            #if IS_LAUNCHER_DIRECTIVE
                // Handle optional next page button when in launcher mode and appropriate conditions are met
                if (!interpreterIsRunningNow && (ult::inMainMenu.load(std::memory_order_acquire) ||
                                                 !m_pageLeftName.empty() || !m_pageRightName.empty())) {
                    // Construct next-page label inline without creating temporary strings
                    const float _nextPageWidth = renderer->getTextDimensions(
                            !m_pageLeftName.empty() ? ("\uE0ED" + ult::GAP_2 + m_pageLeftName) :
                            !m_pageRightName.empty() ? ("\uE0EE" + ult::GAP_2 + m_pageRightName) :
                            (ult::inMainMenu.load(std::memory_order_acquire) ?
                                (((m_menuMode.compare("packages") == 0) ?
                                    (ult::usePageSwap ? "\uE0EE" : "\uE0ED") :
                                    (ult::usePageSwap ? "\uE0ED" : "\uE0EE")) +
                                ult::GAP_2 + (ult::inOverlaysPage.load(std::memory_order_acquire) ?
                                    ult::PACKAGES : ult::OVERLAYS_ABBR)) :
                                ""),
                            false, 23).first + gapWidth;
            
                    if (_nextPageWidth != ult::nextPageWidth.load(std::memory_order_acquire))
                        ult::nextPageWidth.store(_nextPageWidth, std::memory_order_release);
                
                    // Draw next-page button if touched
                    if (ult::touchingNextPage.load(std::memory_order_acquire)) {
                        float nextX = buttonStartX+2 - _halfGap + _backWidth +1;
                        if (!m_noClickableItems)
                            nextX += _selectWidth;
                
                        renderer->drawRoundedRect(nextX, buttonY,
                                                  _nextPageWidth-2,
                                                  73.0f, 10.0f, a(clickColor));
                    }
                }
            #endif
                
            #if IS_LAUNCHER_DIRECTIVE
                const std::string currentBottomLine =
                    "\uE0E1" + ult::GAP_2 +
                    (interpreterIsRunningNow ? ult::HIDE : ult::BACK) + ult::GAP_1 +
                    (!m_noClickableItems && !interpreterIsRunningNow
                        ? "\uE0E0" + ult::GAP_2 + ult::OK + ult::GAP_1
                        : "") +
                    (interpreterIsRunningNow
                        ? "\uE0E5" + ult::GAP_2 + ult::CANCEL + ult::GAP_1
                        : "") +
                    (!interpreterIsRunningNow
                        ? (!ult::usePageSwap
                            ? ((m_menuMode.compare("packages") == 0)
                                ? "\uE0ED" + ult::GAP_2 + ult::OVERLAYS_ABBR
                                : (m_menuMode.compare("overlays") == 0)
                                    ? "\uE0EE" + ult::GAP_2 + ult::PACKAGES
                                    : "")
                            : ((m_menuMode.compare("packages") == 0)
                                ? "\uE0EE" + ult::GAP_2 + ult::OVERLAYS_ABBR
                                : (m_menuMode.compare("overlays") == 0)
                                    ? "\uE0ED" + ult::GAP_2 + ult::PACKAGES
                                    : ""))
                        : "") +
                    (!interpreterIsRunningNow && !m_pageLeftName.empty()
                        ? "\uE0ED" + ult::GAP_2 + m_pageLeftName
                        : !interpreterIsRunningNow && !m_pageRightName.empty()
                            ? "\uE0EE" + ult::GAP_2 + m_pageRightName
                            : "");
            #else
                const std::string currentBottomLine =
                    "\uE0E1" + ult::GAP_2 + ult::BACK + ult::GAP_1 +
                    (!m_noClickableItems
                        ? "\uE0E0" + ult::GAP_2 + ult::OK + ult::GAP_1
                        : "");
            #endif
                
                // Determine if we should use cached bottom text (first frame of new overlay)
                const bool useCachedBottom = !g_cachedBottom.disabled && 
                                              !g_cachedBottom.bottomText.empty() && 
                                              g_cachedBottom.bottomText != currentBottomLine;
                
                const std::string& menuBottomLine = useCachedBottom ? g_cachedBottom.bottomText : currentBottomLine;
                
                // Render the text - it starts halfGap inside the first button, so edgePadding + halfGap
                static const std::vector<std::string> specialChars = {"\uE0E1","\uE0E0","\uE0ED","\uE0EE","\uE0E5"};
                renderer->drawStringWithColoredSections(menuBottomLine, false, specialChars, 
                                                        buttonStartX, 693, 23, 
                                                        (bottomTextColor), (buttonColor));
                
                // Update bottom cache after rendering for next frame
                g_cachedBottom.bottomText = currentBottomLine;
                g_cachedBottom.backWidth = _backWidth;
                g_cachedBottom.selectWidth = _selectWidth;
            #if IS_LAUNCHER_DIRECTIVE
                g_cachedBottom.nextPageWidth = ult::nextPageWidth.load(std::memory_order_acquire);
            #else
                g_cachedBottom.nextPageWidth = 0.0f;
            #endif
                g_cachedBottom.disabled = false;
            
            #if USING_FPS_INDICATOR_DIRECTIVE
                // Update and display FPS
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const double currentTime_seconds = currentTime_ns / 1000000000.0;
                const float currentFps = updateFPS(currentTime_seconds);
            
                static char fpsBuffer[32];
                static float lastFps = -1.0f;
                
                // Only update string if FPS changed significantly
                if (std::abs(currentFps - lastFps) > 0.1f) {
                    snprintf(fpsBuffer, sizeof(fpsBuffer), "FPS: %.2f", currentFps);
                    lastFps = currentFps;
                }
                static constexpr auto whiteColor = tsl::Color(0xF,0xF,0xF,0xF);
                renderer->drawString(fpsBuffer, false, 20, tsl::cfg::FramebufferHeight - 60, 20, whiteColor);
            #endif
            
                if (m_contentElement != nullptr)
                    m_contentElement->frame(renderer);
            
                if (!ult::useRightAlignment)
                    renderer->drawRect(447, 0, 448, 720, a(edgeSeparatorColor));
                else
                    renderer->drawRect(0, 0, 1, 720, a(edgeSeparatorColor));
            
            }
            // CUSTOM SECTION END
        
            inline void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                setBoundaries(parentX, parentY, parentWidth, parentHeight);
                
                if (m_contentElement != nullptr) {
                    m_contentElement->setBoundaries(parentX + 35, parentY + 97, parentWidth - 85, parentHeight - 73 - 105);
                    m_contentElement->invalidate();
                }
            }
            
            inline Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return m_contentElement ? m_contentElement->requestFocus(oldFocus, direction) : nullptr;
            }
            
            inline bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) {
                // Discard touches outside bounds
                if (!m_contentElement || !m_contentElement->inBounds(currX, currY))
                    return false;
                
                return m_contentElement->onTouch(event, currX, currY, prevX, prevY, initialX, initialY);
            }
            
            /**
             * @brief Sets the content of the frame
             *
             * @param content Element
             */
            inline void setContent(Element *content) {
                delete m_contentElement;
                m_contentElement = content;
                
                if (content != nullptr) {
                    m_contentElement->setParent(this);
                    invalidate();
                }
            }
            
            /**
             * @brief Changes the title of the menu
             *
             * @param title Title to change to
             */
            inline void setTitle(const std::string &title) {
                m_title = title;
            }
            
            /**
             * @brief Changes the subtitle of the menu
             *
             * @param title Subtitle to change to
             */
            inline void setSubtitle(const std::string &subtitle) {
                m_subtitle = subtitle;
            }
            
        protected:
            Element *m_contentElement = nullptr;
        };
        
    #if IS_STATUS_MONITOR_DIRECTIVE

        /**
         * @brief The base frame which can contain another view
         *
         */
        class HeaderOverlayFrame : public Element {
        public:
            /**
             * @brief Constructor
             *
             * @param title Name of the Overlay drawn bolt at the top
             * @param subtitle Subtitle drawn bellow the title e.g version number
             */
            std::string m_title;
            std::string m_subtitle;
            bool m_noClickableItems;

            float x, y;
            int offset, y_offset;
            int fontSize;
            
        HeaderOverlayFrame(const std::string& title, const std::string& subtitle, const bool& _noClickableItems=false)
            : Element(), m_title(title), m_subtitle(subtitle), m_noClickableItems(_noClickableItems) {
                ult::activeHeaderHeight = 97;

                if (FullMode)
                    ult::loadWallpaperFileWhenSafe();
                else
                    svcSleepThread(250'000); // sleep thread for initial values to auto-load

                m_isItem = false;
            }

            virtual ~HeaderOverlayFrame() {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
                
                // Check if returning from a list that disabled caching
                if (g_disableMenuCacheOnReturn.exchange(false, std::memory_order_acq_rel)) {
                    g_cachedTop.disabled = true;
                    g_cachedBottom.disabled = true;
                }
            }

            
            virtual void draw(gfx::Renderer *renderer) override {
                if (!ult::themeIsInitialized.load(std::memory_order_acquire) && FullMode) {
                    ult::themeIsInitialized.store(true, std::memory_order_release);
                    tsl::initializeThemeVars();
                }

                if (m_noClickableItems != ult::noClickableItems.load(std::memory_order_acquire)) {
                    ult::noClickableItems.store(m_noClickableItems, std::memory_order_release);
                }
                
                
                if (FullMode == true) {
                    renderer->fillScreen(a(defaultBackgroundColor));
                    if (lastMode.empty() || (lastMode.compare("returning") == 0))
                        renderer->drawWallpaper();
                } else {
                    renderer->fillScreen({ 0x0, 0x0, 0x0, 0x0});
                }
                
                y = 50;
                offset = 0;
                
                // Determine if we should use cached data (first frame of new overlay)
                const bool useCachedTop = !g_cachedTop.disabled && 
                                          !g_cachedTop.title.empty() && 
                                          (g_cachedTop.title != m_title || g_cachedTop.subtitle != m_subtitle);
                
                // Use cached or current data for rendering
                const std::string& renderTitle = useCachedTop ? g_cachedTop.title : m_title;
                const std::string& renderSubtitle = useCachedTop ? g_cachedTop.subtitle : m_subtitle;
                
                renderer->drawString(renderTitle, false, 20, 50, 32, defaultOverlayColor);
                renderer->drawString(renderSubtitle, false, 20, y+2+23, 15, bannerVersionTextColor);
                
                if (FullMode == true)
                    renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(bottomSeparatorColor));
                
                // Set initial button position
                static constexpr float buttonStartX = 30;
                
                if (FullMode && !deactivateOriginalFooter) {
                    // Get the exact gap width from ult::GAP_1
                    const auto gapWidth = renderer->getTextDimensions(ult::GAP_1, false, 23).first;
                    const float _halfGap = gapWidth / 2.0f;
                    if (_halfGap != ult::halfGap.load(std::memory_order_acquire))
                        ult::halfGap.store(_halfGap, std::memory_order_release);
                
                    // Calculate text dimensions for buttons without gaps
                    const auto backTextWidth = renderer->getTextDimensions("\uE0E1" + ult::GAP_2 + ult::BACK, false, 23).first;
                    const auto selectTextWidth = renderer->getTextDimensions("\uE0E0" + ult::GAP_2 + ult::OK, false, 23).first;
                
                    // Update widths to include the half-gap padding on each side
                    const float _backWidth = backTextWidth + gapWidth;
                    if (_backWidth != ult::backWidth.load(std::memory_order_acquire))
                        ult::backWidth.store(_backWidth, std::memory_order_release);
                    const float _selectWidth = selectTextWidth + gapWidth;
                    if (_selectWidth != ult::selectWidth.load(std::memory_order_acquire))
                        ult::selectWidth.store(_selectWidth, std::memory_order_release);
                
                    const float buttonY = static_cast<float>(cfg::FramebufferHeight - 73 + 1);
                
                    // Draw back button rectangle
                    if (ult::touchingBack.load(std::memory_order_acquire)) {
                        renderer->drawRoundedRect(buttonStartX+2 - _halfGap, buttonY, _backWidth-1, 73.0f, 10.0f, a(clickColor));
                    }
                
                    // Draw select button rectangle (starts right after back button)
                    if (ult::touchingSelect.load(std::memory_order_acquire) && !m_noClickableItems) {
                        renderer->drawRoundedRect(buttonStartX+2 - _halfGap + _backWidth+1, buttonY,
                                                  _selectWidth-2, 73.0f, 10.0f, a(clickColor));
                    }
                }
                
                // Build current bottom line
                const std::string currentBottomLine = 
                    "\uE0E1" + ult::GAP_2 + ult::BACK + ult::GAP_1 +
                    (!m_noClickableItems 
                        ? "\uE0E0" + ult::GAP_2 + ult::OK + ult::GAP_1
                        : "");
                
                // Determine if we should use cached bottom text (first frame of new overlay)
                const bool useCachedBottom = !g_cachedBottom.disabled && 
                                              !g_cachedBottom.bottomText.empty() && 
                                              g_cachedBottom.bottomText != currentBottomLine;
                
                const std::string& menuBottomLine = useCachedBottom ? g_cachedBottom.bottomText : currentBottomLine;
                
                // Render the text with special character handling
                if (!deactivateOriginalFooter)  {
                    static const std::vector<std::string> specialChars = {"\uE0E1","\uE0E0","\uE0ED","\uE0EE","\uE0E5"};
                    renderer->drawStringWithColoredSections(menuBottomLine, false, specialChars, buttonStartX, 693, 23, bottomTextColor, buttonColor);
                }
                
                if (this->m_contentElement != nullptr)
                    this->m_contentElement->frame(renderer);

                if (FullMode) {
                    if (!ult::useRightAlignment)
                        renderer->drawRect(447, 0, 448, 720, a(edgeSeparatorColor));
                    else
                        renderer->drawRect(0, 0, 1, 720, a(edgeSeparatorColor));
                }
                
                // Update top cache after rendering for next frame
                g_cachedTop.title = m_title;
                g_cachedTop.subtitle = m_subtitle;
                g_cachedTop.titleColor = {0xF, 0xF, 0xF, 0xF}; // HeaderOverlayFrame uses default white
                g_cachedTop.widgetDrawn = false;  // HeaderOverlayFrame doesn't use widgets
                g_cachedTop.useDynamicLogo = false; // HeaderOverlayFrame doesn't use dynamic logo
                g_cachedTop.disabled = false;
                
                // Update bottom cache after rendering for next frame
                g_cachedBottom.bottomText = currentBottomLine;
                g_cachedBottom.backWidth = ult::backWidth.load(std::memory_order_acquire);
                g_cachedBottom.selectWidth = ult::selectWidth.load(std::memory_order_acquire);
                g_cachedBottom.nextPageWidth = 0.0f;  // HeaderOverlayFrame doesn't use next page
                g_cachedBottom.disabled = false;
            }
            

            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(parentX, parentY, parentWidth, parentHeight);
        
                if (this->m_contentElement != nullptr) {
                    //this->m_contentElement->setBoundaries(parentX + 35, parentY + 140, parentWidth - 85, parentHeight - 73 - 105); // CUSTOM MODIFICATION
                    this->m_contentElement->setBoundaries(parentX + 35, parentY + ult::activeHeaderHeight, parentWidth - 85, parentHeight - 73 - 105);
                    this->m_contentElement->invalidate();
                }
            }
            virtual inline Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                if (this->m_contentElement != nullptr)
                    return this->m_contentElement->requestFocus(oldFocus, direction);
                else
                    return nullptr;
            }
            
            virtual inline bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) {
                // Discard touches outside bounds
                if (!this->m_contentElement->inBounds(currX, currY))
                    return false;
                
                if (this->m_contentElement != nullptr)
                    return this->m_contentElement->onTouch(event, currX, currY, prevX, prevY, initialX, initialY);
                else return false;
            }
            
            /**
             * @brief Sets the content of the frame
             *
             * @param content Element
             */
            inline void setContent(Element *content) {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
                
                this->m_contentElement = content;
                
                if (content != nullptr) {
                    this->m_contentElement->setParent(this);
                    this->invalidate();
                }
            }
            
            /**
             * @brief Changes the title of the menu
             *
             * @param title Title to change to
             */
            inline void setTitle(const std::string &title) {
                this->m_title = title;
            }
            
            /**
             * @brief Changes the subtitle of the menu
             *
             * @param title Subtitle to change to
             */
            inline void setSubtitle(const std::string &subtitle) {
                this->m_subtitle = subtitle;
            }
            
        protected:
            Element *m_contentElement = nullptr;
            
            //std::string m_title, m_subtitle;
        };
    #else
        /**
         * @brief The base frame which can contain another view with a customizable header
         *
         */
        class HeaderOverlayFrame : public Element {
        public:
        #if USING_WIDGET_DIRECTIVE
            bool m_showWidget = false;
        #endif

            HeaderOverlayFrame(u16 headerHeight = 175) : Element(), m_headerHeight(headerHeight) {
                ult::activeHeaderHeight = headerHeight;
                // Load the bitmap file into memory
                ult::loadWallpaperFileWhenSafe();
                m_isItem = false;

            }
            virtual ~HeaderOverlayFrame() {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
                
                if (this->m_header != nullptr)
                    delete this->m_header;
            }
            
            virtual void draw(gfx::Renderer *renderer) override {
                if (!ult::themeIsInitialized.exchange(true, std::memory_order_acq_rel)) {
                    tsl::initializeThemeVars();
                }
            
                renderer->fillScreen(a(defaultBackgroundColor));
                renderer->drawWallpaper();
                //renderer->drawRect(tsl::cfg::FramebufferWidth - 1, 0, 1, tsl::cfg::FramebufferHeight, a(0xF222));
                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(bottomSeparatorColor));
                
                #if USING_WIDGET_DIRECTIVE
                if (m_showWidget)
                    renderer->drawWidget();
                #endif

                // Get the exact gap width from ult::GAP_1
                const float gapWidth = renderer->getTextDimensions(ult::GAP_1, false, 23).first;
                const float _halfGap = gapWidth / 2.0f;
                if (_halfGap != ult::halfGap.load(std::memory_order_acquire))
                    ult::halfGap.store(_halfGap, std::memory_order_release);
            
                // Calculate text dimensions for buttons without gaps
                const float backTextWidth = renderer->getTextDimensions("\uE0E1" + ult::GAP_2 + ult::BACK, false, 23).first;
                const float selectTextWidth = renderer->getTextDimensions("\uE0E0" + ult::GAP_2 + ult::OK, false, 23).first;
            
                // Store final widths with gap padding included
                const float _backWidth = backTextWidth + gapWidth;
                if (_backWidth != ult::backWidth.load(std::memory_order_acquire))
                    ult::backWidth.store(_backWidth, std::memory_order_release);
                const float _selectWidth = selectTextWidth + gapWidth;
                if (_selectWidth != ult::selectWidth.load(std::memory_order_acquire))
                    ult::selectWidth.store(_selectWidth, std::memory_order_release);
            
                // Set initial button position
                static constexpr float buttonStartX = 30;
                const float buttonY = static_cast<float>(cfg::FramebufferHeight - 73 + 1);
            
                // Draw back button rectangle
                if (ult::touchingBack.load(std::memory_order_acquire)) {
                    renderer->drawRoundedRect(buttonStartX+2 - _halfGap, buttonY, _backWidth-1, 73.0f, 10.0f, a(clickColor));
                }
            
                // Draw select button rectangle
                if (ult::touchingSelect.load(std::memory_order_acquire)) {
                    renderer->drawRoundedRect(buttonStartX+2 - _halfGap + _backWidth+1, buttonY,
                                              _selectWidth-2, 73.0f, 10.0f, a(clickColor));
                }
            
                // Draw bottom text
                const std::string menuBottomLine = "\uE0E1" + ult::GAP_2 + ult::BACK + ult::GAP_1 +
                                                   "\uE0E0" + ult::GAP_2 + ult::OK + ult::GAP_1;
                renderer->drawStringWithColoredSections(menuBottomLine, false,
                                                        {"\uE0E1", "\uE0E0", "\uE0ED", "\uE0EE"},
                                                        buttonStartX, 693, 23,
                                                        bottomTextColor, buttonColor);
            
                if (this->m_header != nullptr)
                    this->m_header->frame(renderer);
            
                if (this->m_contentElement != nullptr)
                    this->m_contentElement->frame(renderer);

                if (!ult::useRightAlignment)
                    renderer->drawRect(447, 0, 448, 720, a(edgeSeparatorColor));
                else
                    renderer->drawRect(0, 0, 1, 720, a(edgeSeparatorColor));
            }
            
            virtual inline void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(parentX, parentY, parentWidth, parentHeight);
                
                if (this->m_contentElement != nullptr) {
                    this->m_contentElement->setBoundaries(parentX + 35, parentY + this->m_headerHeight, parentWidth - 85, parentHeight - 73 - this->m_headerHeight -8);
                    this->m_contentElement->invalidate();
                }
                
                if (this->m_header != nullptr) {
                    this->m_header->setBoundaries(parentX, parentY, parentWidth, this->m_headerHeight);
                    this->m_header->invalidate();
                }
            }
            
            virtual inline bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) {
                // Discard touches outside bounds
                if (!this->m_contentElement->inBounds(currX, currY))
                    return false;
                
                if (this->m_contentElement != nullptr)
                    return this->m_contentElement->onTouch(event, currX, currY, prevX, prevY, initialX, initialY);
                else return false;
            }
            
            virtual inline Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                if (this->m_contentElement != nullptr)
                    return this->m_contentElement->requestFocus(oldFocus, direction);
                else
                    return nullptr;
            }
            
            /**
             * @brief Sets the content of the frame
             *
             * @param content Element
             */
            inline void setContent(Element *content) {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
                
                this->m_contentElement = content;
                
                if (content != nullptr) {
                    this->m_contentElement->setParent(this);
                    this->invalidate();
                }
            }
            
            /**
             * @brief Sets the header of the frame
             *
             * @param header Header custom drawer
             */
            inline void setHeader(CustomDrawer *header) {
                if (this->m_header != nullptr)
                    delete this->m_header;
                
                this->m_header = header;
                
                if (header != nullptr) {
                    this->m_header->setParent(this);
                    this->invalidate();
                }
            }
            
        protected:
            Element *m_contentElement = nullptr;
            CustomDrawer *m_header = nullptr;
            
            u16 m_headerHeight;
        };
    #endif
        
        /**
         * @brief Single color rectangle element mainly used for debugging to visualize boundaries
         *
         */
        class DebugRectangle : public Element {
        public:
            /**
             * @brief Constructor
             *
             * @param color Color of the rectangle
             */
            DebugRectangle(Color color) : Element(), m_color(color) {
                m_isItem = false;
            }
            virtual ~DebugRectangle() {}
            
            virtual void draw(gfx::Renderer *renderer) override {
                renderer->drawRect(ELEMENT_BOUNDS(this), a(this->m_color));
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {}
            
        private:
            Color m_color;
        };


        class ListItem; // forward declaration

        static std::mutex s_lastFrameItemsMutex;
        static std::vector<Element*> s_lastFrameItems;
        static std::atomic<bool> s_isForwardCache(false); // NEW VARIABLE FOR FORWARD CACHING
        static std::atomic<bool> s_hasValidFrame(false);
        static std::atomic<s32> s_cachedTopBound{0};
        static std::atomic<s32> s_cachedBottomBound{0};
        static std::atomic<s32> s_cachedHeight{0};
        static std::atomic<s32> s_cachedListHeight{0};
        static std::atomic<s32> s_cachedActualContentBottom{0};
        static std::atomic<bool> s_shouldDrawScrollbar(false);
        static std::atomic<u32> s_cachedScrollbarHeight{0};
        static std::atomic<u32> s_cachedScrollbarOffset{0};
        static std::atomic<u32> s_cachedScrollbarX{0};
        static std::atomic<u32> s_cachedScrollbarY{0};
        static std::atomic<float> s_currentScrollVelocity{0};

        static std::atomic<bool> s_directionalKeyReleased{false};
        static std::atomic<bool> s_cacheForwardFrameOnce(true);
        static std::atomic<bool> lastInternalTouchRelease(true);
        static std::atomic<bool> s_hasClearedCache(false);

        //static std::atomic<bool> s_skipCaching(false);

        static std::mutex s_safeToSwapMutex;
        //static std::mutex s_safeTransitionMutex;
        static std::atomic<bool> s_safeToSwap{false};

        static std::atomic<bool> fullDeconstruction{false};
        static std::atomic<bool> skipDeconstruction{false};
        static std::atomic<bool> skipOnce{false};

        static std::atomic<bool> isTableScrolling{false};

        class List : public Element {
        
        public:
            List() : Element() {
                if (fullDeconstruction.load(std::memory_order_acquire)) {
                    return;
                }

                s_safeToSwap.store(false, std::memory_order_release);
                //s_directionalKeyReleased.store(false, std::memory_order_release);
                //std::lock_guard<std::mutex> lock(s_safeTransitionMutex);
                //s_safeToSwap.store(false, std::memory_order_release);
                
                // Initialize instance state
                m_hasForwardCached = false;
                m_pendingJump = false;
                m_cachingDisabled = false;
                m_clearList = false;
                m_focusedIndex = 0;
                m_offset = 0;
                m_nextOffset = 0;
                m_listHeight = 0;
                actualItemCount = 0;
                m_isItem = false;

                {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                    
                    s_hasClearedCache.store(false, std::memory_order_release);
                    
                    if (skipDeconstruction.load(std::memory_order_acquire)) {
                        purgePendingItems();
                    } else {
                        s_cacheForwardFrameOnce.store(true, std::memory_order_release);
                        skipOnce.store(false, std::memory_order_release);
                    }
                }
            }
            
            virtual ~List() {
                if (fullDeconstruction.load(std::memory_order_acquire)) {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);  // Add this
                    
                    purgePendingItems();
                    
                    if (s_isForwardCache.load(std::memory_order_acquire)) {
                        clearStaticCacheUnsafe(true);
                        s_isForwardCache.store(false, std::memory_order_release);
                    } else {
                        clearStaticCacheUnsafe();
                    }
                    clearItems();
                    
                    return;
                }

                s_safeToSwap.store(false, std::memory_order_release);
                //s_directionalKeyReleased.store(false, std::memory_order_release);
                //std::lock_guard<std::mutex> lock(s_safeTransitionMutex);
                //s_safeToSwap.store(false, std::memory_order_release);
            
                // NOW take mutex for shared static variable operations
                {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);

                    if (!skipDeconstruction.load(std::memory_order_acquire)) {
                        purgePendingItems();
                        
                        if (!s_isForwardCache.load(std::memory_order_acquire)) {
                            clearStaticCacheUnsafe();
                            clearItems();
                        }
            
                        s_isForwardCache.store(false, std::memory_order_release);
                        s_cacheForwardFrameOnce.store(true, std::memory_order_release);
                    }
            
                    if (m_cachingDisabled || (skipOnce.load(std::memory_order_acquire) && skipDeconstruction.load(std::memory_order_acquire))) {
                        purgePendingItems();
                        clearItems();
                    } else if (skipDeconstruction.load(std::memory_order_acquire)) {
                        skipOnce.store(true, std::memory_order_release);
                    }
                }
            }
            
            
            virtual void draw(gfx::Renderer* renderer) override {
                if (fullDeconstruction.load(std::memory_order_acquire)) {
                    return;
                }

                s_safeToSwap.store(false, std::memory_order_release);
                std::lock_guard<std::mutex> lock(s_safeToSwapMutex);
                //s_safeToSwap.store(false, std::memory_order_release);
                
                // Early exit optimizations
                if (m_clearList) {
                    if (!s_isForwardCache.load(std::memory_order_acquire)) {
                        clearStaticCacheUnsafe();
                        
                    } else {
                        clearStaticCacheUnsafe(true);
                    }
                    clearItems();
                    s_isForwardCache.store(false, std::memory_order_release);
                    s_cacheForwardFrameOnce.store(true, std::memory_order_release);
                    return;
                }
                {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                    // Process pending operations in batch
                    if (!m_itemsToAdd.empty()) addPendingItems();
                    if (!m_itemsToRemove.empty()) removePendingItems();
                }
                
                // Only lock when checking s_lastFrameItems.empty()
                bool shouldResetCache = false;
                {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                    if (!s_hasValidFrame.load(std::memory_order_acquire) && s_lastFrameItems.empty() && 
                        !s_cacheForwardFrameOnce.load(std::memory_order_acquire)) {
                        shouldResetCache = true;
                    }
                }
                
                if (shouldResetCache) {
                    s_cacheForwardFrameOnce.store(true, std::memory_order_release);
                }
                
                // This part is for fixing returning to Ultrahand without rendering that first frame skip
                static bool checkOnce = true;
                if (checkOnce && m_pendingJump && !s_hasValidFrame.load(std::memory_order_acquire) && 
                    !s_isForwardCache.load(std::memory_order_acquire)) {
                    checkOnce = false;
                    return;
                } else {
                    static bool checkOnce2 = true;
                    if (checkOnce2) {
                        checkOnce = true;
                        checkOnce2 = false;
                    }
                }
                
                // Check if we should render cached frame
                if ((m_pendingJump || !m_hasForwardCached) && 
                    (s_hasValidFrame.load(std::memory_order_acquire) || s_isForwardCache.load(std::memory_order_acquire))) {
                    {
                        std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);                    
                        // Render using cached frame state if available
                        renderCachedFrame(renderer);  // This method handles its own locking
                        
                        // Clear cache after rendering
                        if (s_isForwardCache.load(std::memory_order_acquire))
                            clearStaticCacheUnsafe(true);  // This method handles its own locking
                        else
                            clearStaticCacheUnsafe();      // This method handles its own locking
                    }
                    
                    return;
                }
                
                // Cache bounds for hot loop
                const s32 topBound = getTopBound();
                const s32 bottomBound = getBottomBound();
                const s32 height = getHeight();
                
                renderer->enableScissoring(getLeftBound(), topBound-8, getWidth() + 8, height + 14);
                
                {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                    // Optimized visibility culling
                    for (Element* entry : m_items) {
                        if (entry->getBottomBound() > topBound && entry->getTopBound() < bottomBound) {
                            entry->frame(renderer);
                        }
                    }
                }

                renderer->disableScissoring();
                
                // Draw scrollbar only when needed
                if (m_listHeight > height) {
                    drawScrollbar(renderer, height);
                    updateScrollAnimation();
                }
                
                // Handle caching operations - lock only for the critical section
                {
                    std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                    
                    if (!s_isForwardCache.load(std::memory_order_acquire) && s_hasValidFrame.load(std::memory_order_acquire)) {
                        // Clear cache after rendering (this is called within the lock)
                        clearStaticCacheUnsafe(); // New unsafe version for use within lock
                        s_hasValidFrame.store(false, std::memory_order_release);
                        s_cacheForwardFrameOnce.store(true, std::memory_order_release);
                    }
                    
                    if (!m_cachingDisabled) {
                        if (s_cacheForwardFrameOnce.load(std::memory_order_acquire) && 
                            !s_hasValidFrame.load(std::memory_order_acquire)) {
                            // Cache current frame (this is called within the lock)
                            cacheCurrentFrameUnsafe(true); // New unsafe version for use within lock
                            s_cacheForwardFrameOnce.store(false, std::memory_order_release);
                            s_isForwardCache.store(true, std::memory_order_release);
                            s_hasValidFrame.store(true, std::memory_order_release);
                            m_hasForwardCached = true;
                        }
                        cacheCurrentScrollbar();
                    }

                    //if (m_cachingDisabled ||(s_hasValidFrame.load(std::memory_order_acquire) && s_isForwardCache.load(std::memory_order_acquire)))
                    //    s_safeToSwap.store(true, std::memory_order_release);
                }
                s_safeToSwap.store(true, std::memory_order_release);
            }

            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                s32 y = getY() - m_offset;
                
                // Position all items first (don't calculate m_listHeight here)
                for (Element* entry : m_items) {
                    entry->setBoundaries(getX(), y, getWidth(), entry->getHeight());
                    entry->invalidate();
                    y += entry->getHeight();
                }

                
                // Calculate total height AFTER all invalidations are done
                m_listHeight = BOTTOM_PADDING;
                for (Element* entry : m_items) {
                    m_listHeight += entry->getHeight();
                }
            }
                                                
            // Fixed onTouch method - prevents controller state corruption
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                // Quick bounds check
                if (!inBounds(currX, currY)) return false;
                
                // Forward to children first
                for (Element* item : m_items) {
                    if (item->onTouch(event, currX, currY, prevX, prevY, initialX, initialY)) {
                        return true;
                    }
                }
                
                // Handle scrolling
                if (event != TouchEvent::Release && Element::getInputMode() == InputMode::TouchScroll) {
                    if (prevX && prevY) {
                        m_nextOffset += (prevY - currY);
                        m_nextOffset = std::clamp(m_nextOffset, 0.0f, static_cast<float>(m_listHeight - getHeight()));
                        
                        // Track that we're touch scrolling
                        m_touchScrollActive = true;
                    }
                    return true;
                }
                
                return false;
            }
            

            inline void addItem(Element* element, u16 height = 0, ssize_t index = -1) {
                if (!element) return;
                
                // First item optimization
                if (actualItemCount == 0 && element->m_isItem) {
                    auto* customDrawer = new tsl::elm::CustomDrawer([](gfx::Renderer*, s32, s32, s32, s32) {});
                    customDrawer->setBoundaries(getX(), getY(), getWidth(), 29+4);
                    customDrawer->setParent(this);
                    customDrawer->invalidate();
                    m_itemsToAdd.emplace_back(-1, customDrawer);
                }
        
                if (height) {
                    element->setBoundaries(getX(), getY(), getWidth(), height);
                }
        
                element->setParent(this);
                element->invalidate();
                m_itemsToAdd.emplace_back(index, element);
                ++actualItemCount;
            }
        
            virtual void removeItem(Element *element) {
                if (element) m_itemsToRemove.push_back(element);
            }
            
            virtual void removeIndex(size_t index) {
                if (index < m_items.size()) removeItem(m_items[index]);
            }
            
            inline void clear() {
                m_clearList = true;
            }
        
            virtual Element* requestFocus(Element* oldFocus, FocusDirection direction) override {
                if (m_clearList || !m_itemsToAdd.empty()) return nullptr;
                
                static bool delayedHandle = false;

                // NEW: Handle pending jump to specific item
                if (m_pendingJump && !delayedHandle) {
                    delayedHandle = true;
                    return handleJumpToItem(oldFocus);
                } else if (m_pendingJump) {
                    m_pendingJump = false;
                    delayedHandle = false;
                    return handleJumpToItem(oldFocus); // needs to be handled 2x for proper rendering
                }
                
                if (jumpToBottom.exchange(false, std::memory_order_acq_rel))
                    return handleJumpToBottom(oldFocus);
                
                if (jumpToTop.exchange(false, std::memory_order_acq_rel))
                    return handleJumpToTop(oldFocus);
                
                if (skipDown.exchange(false, std::memory_order_acq_rel))
                    return handleSkipDown(oldFocus);
                
                if (skipUp.exchange(false, std::memory_order_acq_rel))
                    return handleSkipUp(oldFocus);

            
                if (direction == FocusDirection::None) {
                    return handleInitialFocus(oldFocus);
                }
                else if (direction == FocusDirection::Down) {
                    return handleDownFocus(oldFocus);
                }
                else if (direction == FocusDirection::Up) {
                    return handleUpFocus(oldFocus);
                }
            
                return oldFocus;
            }

            inline void jumpToItem(const std::string& text = "", const std::string& value = "", bool exactMatch=true) {

                if (!text.empty() || !value.empty()) {
                    m_pendingJump = true;
                    m_jumpToText = text;
                    m_jumpToValue = value;
                    m_jumpToExactMatch = exactMatch;
                }

            }
                        
            virtual Element* getItemAtIndex(u32 index) {
                return (m_items.size() <= index) ? nullptr : m_items[index];
            }
            
            virtual s32 getIndexInList(Element *element) {
                auto it = std::find(m_items.begin(), m_items.end(), element);
                return (it == m_items.end()) ? -1 : static_cast<s32>(it - m_items.begin());
            }
        
            virtual s32 getLastIndex() {
                return static_cast<s32>(m_items.size()) - 1;
            }
            
            virtual void setFocusedIndex(u32 index) {
                if (m_items.size() > index) {
                    m_focusedIndex = index;
                    updateScrollOffset();
                }
            }
            
            inline void onDirectionalKeyReleased() {
                m_hasWrappedInCurrentSequence = false;
                m_lastNavigationResult = NavigationResult::None;
                m_isHolding = false;
                m_stoppedAtBoundary = false;
                m_lastNavigationTime = 0;
                m_lastScrollTime = 0;
            }

            inline void disableCaching() {
                m_cachingDisabled = true;
                //clearFrameCache();
                g_disableMenuCacheOnReturn.store(true, std::memory_order_release);
            }
        
        protected:

            std::vector<Element*> m_items;
            u16 m_focusedIndex = 0;
            
            float m_offset = 0, m_nextOffset = 0;
            s32 m_listHeight = 0;
            
            bool m_clearList = false;
            std::vector<Element*> m_itemsToRemove;
            std::vector<std::pair<ssize_t, Element*>> m_itemsToAdd;
            std::vector<float> prefixSums;
            
            // Instance identification
            //const size_t m_instanceId;

            // Enhanced navigation state tracking
            bool m_justWrapped = false;
            bool m_isHolding = false;
            bool m_stoppedAtBoundary = false;
            u64 m_lastNavigationTime = 0;
            static constexpr u64 HOLD_THRESHOLD_NS = 100000000ULL;  // 100ms
        
            size_t actualItemCount = 0;

            // Jump to navigation variables
            std::string m_jumpToText;
            std::string m_jumpToValue;
            bool m_jumpToExactMatch = false;
            bool m_pendingJump = false;
            bool m_hasForwardCached = false;
            bool m_cachingDisabled = false;  // New flag to disable caching
            
            //bool m_hasRenderedCache = false;

            // Stack variables for hot path - reused to avoid allocations
            u32 scrollbarHeight;
            u32 scrollbarOffset;
            u32 prevOffset;
            static constexpr float SCROLLBAR_X_OFFSET = 21.0f;
            static constexpr float SCROLLBAR_Y_OFFSET = 3.0f;
            static constexpr float SCROLLBAR_HEIGHT_TRIM = 6.0f;
            
            //static constexpr float smoothingFactor = 0.15f;
            //static constexpr float dampingFactor = 0.3f;
            static constexpr float TABLE_SCROLL_STEP_SIZE = 10;
            static constexpr float TABLE_SCROLL_STEP_SIZE_CLICK = 22;
            static constexpr float BOTTOM_PADDING = 7.0f;
            static constexpr float VIEW_CENTER_OFFSET = 7.0f;

            u64 m_lastScrollTime = 0;

            float m_scrollVelocity = 0.0f;
            
            bool m_touchScrollActive = false;

            enum class NavigationResult {
                None,
                Success,
                HitBoundary,
                Wrapped
            };
            
            bool m_hasWrappedInCurrentSequence = false;
            NavigationResult m_lastNavigationResult = NavigationResult::None;
        
        private:

            // Thread-safe versions (handle their own locking)
            static void clearStaticCache(bool preservePointers = false) {
                std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                clearStaticCacheUnsafe(preservePointers);
            }
            
            void cacheCurrentFrame(bool preservePointers = false) {
                std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                cacheCurrentFrameUnsafe(preservePointers);
            }

        
            static void clearStaticCacheUnsafe(bool preservePointers = false) {
                //std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                if (!preservePointers) {
                    // Normal case: delete elements and clear
                    for (Element* el : s_lastFrameItems) {
                        delete el;
                    }
                }
            
                s_lastFrameItems.clear();
                //s_lastFrameItems.shrink_to_fit();
            
                // CRITICAL: Always reset these, even for forward cache!
                s_hasValidFrame.store(false, std::memory_order_release);  // This MUST be false after clearing
                s_isForwardCache.store(false, std::memory_order_release);
                
                s_cachedTopBound.store(0, std::memory_order_release);
                s_cachedBottomBound.store(0, std::memory_order_release);
                s_cachedHeight.store(0, std::memory_order_release);
                s_cachedListHeight.store(0, std::memory_order_release);
                s_cachedActualContentBottom.store(0, std::memory_order_release);
            
                s_shouldDrawScrollbar.store(false, std::memory_order_release);
                s_cachedScrollbarHeight.store(0, std::memory_order_release);
                s_cachedScrollbarOffset.store(0, std::memory_order_release);
                s_cachedScrollbarX.store(0, std::memory_order_release);
                s_cachedScrollbarY.store(0, std::memory_order_release);
            }
                    
            void cacheCurrentFrameUnsafe(bool preservePointers = false) {
                //std::lock_guard<std::mutex> lock(s_lastFrameItemsMutex);
                if (!preservePointers) {
                    for (Element* el : s_lastFrameItems) delete el;
                }
            
                s_lastFrameItems = m_items;
            
                // Store new cache values using atomic stores
                s_cachedTopBound.store(getTopBound(), std::memory_order_release);
                s_cachedBottomBound.store(getBottomBound(), std::memory_order_release);
                s_cachedHeight.store(getHeight(), std::memory_order_release);
                s_cachedListHeight.store(m_listHeight, std::memory_order_release);
            
                if (preservePointers)
                    s_isForwardCache.store(true, std::memory_order_release);
            
                s_hasValidFrame.store(true, std::memory_order_release);
            }
            
            void cacheCurrentScrollbar() {
                const s32 cachedHeight = s_cachedHeight.load(std::memory_order_acquire);
                const s32 cachedListHeight = s_cachedListHeight.load(std::memory_order_acquire);
            
                s_shouldDrawScrollbar.store((cachedListHeight > cachedHeight), std::memory_order_release);
            
                if (s_shouldDrawScrollbar.load(std::memory_order_acquire)) {
                    const float viewHeight  = static_cast<float>(cachedHeight);
                    const float totalHeight = static_cast<float>(cachedListHeight);
                    const u32   maxScroll   = std::max(static_cast<u32>(totalHeight - viewHeight), 1u);
            
                    u32 scrollbarHeight = std::min(
                        static_cast<u32>((viewHeight * viewHeight) / totalHeight),
                        static_cast<u32>(viewHeight)
                    );
            
                    u32 scrollbarOffset = std::min(
                        static_cast<u32>((m_offset / maxScroll) * (viewHeight - scrollbarHeight)),
                        static_cast<u32>(viewHeight - scrollbarHeight) // corrected potential bug
                    );
            
                    scrollbarHeight -= SCROLLBAR_HEIGHT_TRIM;
            
                    s_cachedScrollbarHeight.store(scrollbarHeight, std::memory_order_release);
                    s_cachedScrollbarOffset.store(scrollbarOffset, std::memory_order_release);
                    s_cachedScrollbarX.store(getRightBound() + SCROLLBAR_X_OFFSET, std::memory_order_release);
                    s_cachedScrollbarY.store(getY() + scrollbarOffset + SCROLLBAR_Y_OFFSET, std::memory_order_release);
                }
            }
                                                
            void renderCachedFrame(gfx::Renderer* renderer) {
                const s32 cachedTopBound    = s_cachedTopBound.load(std::memory_order_acquire);
                const s32 cachedBottomBound = s_cachedBottomBound.load(std::memory_order_acquire);
                const s32 cachedHeight      = s_cachedHeight.load(std::memory_order_acquire);
            
                renderer->enableScissoring(getLeftBound(), cachedTopBound - 8, getWidth() + 8, cachedHeight + 14);
            
                for (Element* entry : s_lastFrameItems) {
                    if (entry &&
                        entry->getBottomBound() > cachedTopBound &&
                        entry->getTopBound() < cachedBottomBound) {
                        entry->frame(renderer);
                    }
                }
            
                renderer->disableScissoring();
            
                if (s_shouldDrawScrollbar.load(std::memory_order_acquire)) {
                    const u32 scrollbarX      = s_cachedScrollbarX.load(std::memory_order_acquire);
                    const u32 scrollbarY      = s_cachedScrollbarY.load(std::memory_order_acquire);
                    const u32 scrollbarHeight = s_cachedScrollbarHeight.load(std::memory_order_acquire);
            
                    renderer->drawRect(scrollbarX, scrollbarY, 5, scrollbarHeight, a(trackBarColor));
                    renderer->drawCircle(scrollbarX + 2, scrollbarY, 2, true, a(trackBarColor));
                    renderer->drawCircle(scrollbarX + 2, scrollbarY + scrollbarHeight, 2, true, a(trackBarColor));
                }
            }
            

            void clearItems() {

                for (Element* item : m_items) delete item;
                m_items = {};
                //m_items.clear();
                //m_items.shrink_to_fit();
                m_offset = 0;
                m_focusedIndex = 0;
                invalidate();
                m_clearList = false;
                actualItemCount = 0;
            }
            
            void addPendingItems() {
                for (auto [index, element] : m_itemsToAdd) {
                    element->invalidate();
                    if (index >= 0 && static_cast<size_t>(index) < m_items.size()) {
                        m_items.insert(m_items.begin() + index, element);
                    } else {
                        m_items.push_back(element);
                    }
                }
                m_itemsToAdd = {};
                //m_itemsToAdd.clear();
                //m_itemsToAdd.shrink_to_fit();
                invalidate();
                updateScrollOffset();
            }
            
            void removePendingItems() {
                //size_t index;
                for (Element* element : m_itemsToRemove) {
                    auto it = std::find(m_items.begin(), m_items.end(), element);
                    if (it != m_items.end()) {
                        const size_t index = static_cast<size_t>(it - m_items.begin());
                        m_items.erase(it);
                        if (m_focusedIndex >= index && m_focusedIndex > 0) {
                            --m_focusedIndex;
                        }
                        delete element;
                    }
                }
                m_itemsToRemove = {};
                //m_itemsToRemove.clear();
                //m_itemsToRemove.shrink_to_fit();
                invalidate();
                updateScrollOffset();
            }

            void purgePendingItems() {
                for (auto& [_, element] : m_itemsToAdd) {
                    if (element) { element->invalidate(); delete element; }
                }
                m_itemsToAdd = {};
                //m_itemsToAdd.clear();
                //m_itemsToAdd.shrink_to_fit();
                
                //size_t index;
                for (Element* element : m_itemsToRemove) {
                    auto it = std::find(m_items.begin(), m_items.end(), element);
                    if (it != m_items.end()) {
                        //index = static_cast<std::size_t>(it - m_items.begin());
                        const u16 index16 = static_cast<u16>(static_cast<std::size_t>(it - m_items.begin()));
                        element->invalidate();
                        delete element;
                        m_items.erase(it);
            
                        constexpr u16 noFocus = static_cast<u16>(0xFFFF);
                        if (m_focusedIndex == index16)
                            m_focusedIndex = noFocus;
                        else if (m_focusedIndex != noFocus && m_focusedIndex > index16)
                            --m_focusedIndex;
                    }
                }
                m_itemsToRemove = {};
               //m_itemsToRemove.clear();
               //m_itemsToRemove.shrink_to_fit();
            
                invalidate();
                updateScrollOffset();
            }

            
            void drawScrollbar(gfx::Renderer* renderer, s32 height) {
                const float viewHeight = static_cast<float>(height);
                const float totalHeight = static_cast<float>(m_listHeight);
                const u32 maxScrollableHeight = std::max(static_cast<u32>(totalHeight - viewHeight), 1u);
                
                scrollbarHeight = std::min(static_cast<u32>((viewHeight * viewHeight) / totalHeight), 
                                         static_cast<u32>(viewHeight));
                
                scrollbarOffset = std::min(static_cast<u32>((m_offset / maxScrollableHeight) * (viewHeight - scrollbarHeight)), 
                                         static_cast<u32>(viewHeight - scrollbarHeight));
        
                const u32 scrollbarX = getRightBound() + SCROLLBAR_X_OFFSET;
                const u32 scrollbarY = getY() + scrollbarOffset+SCROLLBAR_Y_OFFSET;

                scrollbarHeight -= SCROLLBAR_HEIGHT_TRIM; // shorten very slightly
        
                renderer->drawRect(scrollbarX, scrollbarY, 5, scrollbarHeight, a(trackBarColor));
                renderer->drawCircle(scrollbarX + 2, scrollbarY, 2, true, a(trackBarColor));
                renderer->drawCircle(scrollbarX + 2, scrollbarY + scrollbarHeight, 2, true, a(trackBarColor));
            }

            
            inline void updateScrollAnimation() {
                if (Element::getInputMode() == InputMode::Controller) {
                    // Clear touch flag when in controller mode
                    m_touchScrollActive = false;
                    
                    // Calculate distance to target
                    const float diff = m_nextOffset - m_offset;
                    const float distance = std::abs(diff);
                    
                    // ENHANCED BOUNDARY SNAPPING: More aggressive snapping for boundaries
                    if (distance < 1.0f) {  // Increased threshold from 0.5f
                        m_offset = m_nextOffset;
                        m_scrollVelocity = 0.0f;
                        s_currentScrollVelocity.store(m_scrollVelocity, std::memory_order_release);
                        
                        if (prevOffset != m_offset) {
                            invalidate();
                            prevOffset = m_offset;
                        }
                        return;
                    }
                    
                    // SPECIAL CASE: If target is exactly 0 or max, be more aggressive
                    const float maxOffset = static_cast<float>(m_listHeight - getHeight());
                    if (m_nextOffset == 0.0f || m_nextOffset == maxOffset) {
                        if (distance < 3.0f) {  // Larger snap zone for boundaries
                            m_offset = m_nextOffset;
                            m_scrollVelocity = 0.0f;
                            s_currentScrollVelocity.store(m_scrollVelocity, std::memory_order_release);
                            
                            if (prevOffset != m_offset) {
                                invalidate();
                                prevOffset = m_offset;
                            }

                            return;
                        }
                    }
                    
                    // Emergency correction if item is going out of bounds
                    if (m_focusedIndex < m_items.size()) {
                        float itemTop = 0.0f;
                        for (size_t i = 0; i < m_focusedIndex; ++i) {
                            itemTop += m_items[i]->getHeight();
                        }
                        const float itemBottom = itemTop + m_items[m_focusedIndex]->getHeight();
                        
                        //float viewTop = m_offset;
                        const float viewBottom = m_offset + getHeight();
                        
                        if (itemTop < m_offset || itemBottom > viewBottom) {
                            const float emergencySpeed = (itemBottom < m_offset || itemTop > viewBottom) ? 0.9f : 0.6f;
                            
                            m_offset += diff * emergencySpeed;
                            m_scrollVelocity = diff * 0.3f;
                            s_currentScrollVelocity.store(m_scrollVelocity, std::memory_order_release);
                            
                            if (prevOffset != m_offset) {
                                invalidate();
                                prevOffset = m_offset;
                            }
                            return;
                        }
                    }
                    
                    // Rest of your existing smooth scrolling logic...
                    const bool isLargeJump = distance > getHeight() * 1.5f;
                    const bool isFromRest = std::abs(m_scrollVelocity) < 2.0f;
                    
                    if (isLargeJump && isFromRest) {
                        static constexpr float gentleAcceleration = 0.08f;
                        static constexpr float gentleDamping = 0.85f;
                        
                        const float targetVelocity = diff * gentleAcceleration;
                        m_scrollVelocity += (targetVelocity - m_scrollVelocity) * gentleDamping;
                    } else {
                        const float urgency = std::min(distance / getHeight(), 1.0f);
                        const float accelerationFactor = 0.18f + (0.24f * urgency);
                        const float dampingFactor = 0.48f - (0.18f * urgency);
                        
                        const float targetVelocity = diff * accelerationFactor;
                        m_scrollVelocity += (targetVelocity - m_scrollVelocity) * dampingFactor;
                    }
                    
                    // Apply velocity
                    m_offset += m_scrollVelocity;
                    
                    // ENHANCED overshoot prevention with better boundary handling
                    if ((m_scrollVelocity > 0 && m_offset > m_nextOffset) ||
                        (m_scrollVelocity < 0 && m_offset < m_nextOffset)) {
                        m_offset = m_nextOffset;
                        m_scrollVelocity = 0.0f;
                    }
                    
                    // ADDITIONAL: Force exact boundary values
                    if (m_nextOffset == 0.0f && m_offset < 1.0f) {
                        m_offset = 0.0f;
                        m_scrollVelocity = 0.0f;
                    } else if (m_nextOffset == maxOffset && m_offset > maxOffset - 1.0f) {
                        m_offset = maxOffset;
                        m_scrollVelocity = 0.0f;
                    }

                    s_currentScrollVelocity.store(m_scrollVelocity, std::memory_order_release);
                
                } else if (Element::getInputMode() == InputMode::TouchScroll) {
                    // Your existing touch scroll logic...
                    m_offset = m_nextOffset;
                    m_scrollVelocity = 0.0f;
                    
                    if (m_touchScrollActive) {
                        const float viewCenter = m_offset + (getHeight() / 2.0f);
                        float accumHeight = 0.0f;
                        
                        //float itemHeight, itemCenter;
                        for (size_t i = 0; i < m_items.size(); ++i) {
                            const float itemHeight = m_items[i]->getHeight();
                            const float itemCenter = accumHeight + (itemHeight / 2.0f);
                            
                            if (itemCenter >= viewCenter) {
                                m_focusedIndex = i;
                                break;
                            }
                            
                            accumHeight += itemHeight;
                        }
                    }
                }
                
                if (prevOffset != m_offset) {
                    invalidate();
                    prevOffset = m_offset;
                }
            }
                                                        
            Element* handleInitialFocus(Element* oldFocus) {
                const size_t itemCount = m_items.size();
                if (itemCount == 0) return nullptr;
                
                size_t startIndex = 0;
                
                // Calculate starting index based on current scroll position
                if (!oldFocus && m_offset > 0) {
                    float elementHeight = 0.0f;
                    const size_t maxIndex = itemCount - 1;
                    while (elementHeight < m_offset && startIndex < maxIndex) {
                        elementHeight += m_items[startIndex]->getHeight();
                        ++startIndex;
                    }
                }
                
                //resetNavigationState();
                
                // Save current offset to prevent scroll jumping
                const float savedOffset = m_offset;
                const float savedNextOffset = m_nextOffset;
                
                // Single loop with wraparound logic - visits each item exactly once
                for (size_t count = 0; count < itemCount; ++count) {
                    const size_t i = (startIndex + count) % itemCount;
                    
                    if (!m_items[i]->isTable()) {
                        Element* const newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::None);
                        if (newFocus && newFocus != oldFocus) {
                            m_focusedIndex = i;
                            m_offset = savedOffset;
                            m_nextOffset = savedNextOffset;
                            return newFocus;
                        }
                    }
                }
                
                return nullptr;
            }
            
                                                                                                                                            
            inline Element* handleDownFocus(Element* oldFocus) {
                static bool triggerShakeOnce = true;
                const bool atBottom = isAtBottom();
                updateHoldState();
                
                // Check if the next item is non-focusable BEFORE we do anything else
                if (m_focusedIndex + 1 < int(m_items.size())) {
                    Element* nextItem = m_items[m_focusedIndex + 1];
                    if (!nextItem->m_isItem) {
                        isTableScrolling.store(true, std::memory_order_release);
                    }
                }
                
                // If holding and at boundary, try to scroll first
                if (m_isHolding && m_stoppedAtBoundary && !atBottom) {
                    scrollDown();
                    m_stoppedAtBoundary = false;
                    return oldFocus;
                }
                
                Element* result = navigateDown(oldFocus);
                
                if (result != oldFocus) {
                    m_lastNavigationResult = NavigationResult::Success;
                    m_stoppedAtBoundary = false;
                    triggerShakeOnce = true;  // This resets it for THIS function
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerNavigationSound.store(true, std::memory_order_release);
                    triggerNavigationFeedback();
                    return result;
                }
                
                // Check if we can still scroll down
                if (!atBottom) {
                    scrollDown();
                    triggerShakeOnce = true;  // ADDED: Reset when scrolling away from boundary
                    return oldFocus;
                }
                
                // At absolute bottom - check for wrapping (single tap)
                if (!m_isHolding && !m_hasWrappedInCurrentSequence && atBottom) {
                    s_directionalKeyReleased.store(false, std::memory_order_release);
                    m_hasWrappedInCurrentSequence = true;
                    m_lastNavigationResult = NavigationResult::Wrapped;
                    
                    //if (result->m_isItem) {
                    triggerShakeOnce = true;  // Reset when wrapping
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerNavigationSound.store(true, std::memory_order_release);
                    //}
                    return handleJumpToTop(oldFocus);
                }
                
                // Set boundary flag (for holding)
                if (m_isHolding && atBottom) {
                    m_stoppedAtBoundary = true;
                    if (triggerShakeOnce) {
                        if (result->m_isItem) {
                            triggerRumbleClick.store(true, std::memory_order_release);
                            triggerWallSound.store(true, std::memory_order_release);
                            
                            for (ssize_t i = static_cast<ssize_t>(m_focusedIndex); i >= 0; --i) {
                                if (m_items[i]->m_isItem) {
                                    m_items[i]->shakeHighlight(FocusDirection::Down);
                                    break;
                                }
                            }
                        } else {
                            triggerRumbleClick.store(true, std::memory_order_release);
                            triggerWallSound.store(true, std::memory_order_release);
                        }
                        triggerShakeOnce = false;
                    }
                } else if (!m_isHolding) {
                    triggerShakeOnce = true;
                }
            
                m_lastNavigationResult = NavigationResult::HitBoundary;
                return oldFocus;
            }
            
            inline Element* handleUpFocus(Element* oldFocus) {
                static bool triggerShakeOnce = true;
                const bool atTop = isAtTop();
                updateHoldState();
                
                // Check if the previous item is non-focusable BEFORE we do anything else
                if (m_focusedIndex > 0) {
                    Element* prevItem = m_items[m_focusedIndex - 1];
                    if (prevItem->isTable()) {
                        isTableScrolling.store(true, std::memory_order_release);
                    }
                }
                
                // If holding and at boundary, try to scroll first
                if (m_isHolding && m_stoppedAtBoundary && !atTop) {
                    scrollUp();
                    m_stoppedAtBoundary = false;
                    return oldFocus;
                }
                
                Element* result = navigateUp(oldFocus);
                
                if (result != oldFocus) {
                    m_lastNavigationResult = NavigationResult::Success;
                    m_stoppedAtBoundary = false;
                    triggerShakeOnce = true;  // This resets it for THIS function
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerNavigationSound.store(true, std::memory_order_release);
                    triggerNavigationFeedback();
                    return result;
                }
                
                // Check if we can still scroll up
                if (!atTop) {
                    scrollUp();
                    triggerShakeOnce = true;  // ADDED: Reset when scrolling away from boundary
                    return oldFocus;
                }
                
                // At absolute top - check for wrapping (single tap)
                if (!m_isHolding && !m_hasWrappedInCurrentSequence && atTop) {
                    s_directionalKeyReleased.store(false, std::memory_order_release);
                    m_hasWrappedInCurrentSequence = true;
                    m_lastNavigationResult = NavigationResult::Wrapped;
                    
                    //if (result->m_isItem) {
                    triggerShakeOnce = true;  // Reset when wrapping
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerNavigationSound.store(true, std::memory_order_release);
                    //}
                    return handleJumpToBottom(oldFocus);
                }
                
                // Set boundary flag (for holding)
                if (m_isHolding && atTop) {
                    m_stoppedAtBoundary = true;
                    if (triggerShakeOnce) {
                        if (result->m_isItem) {
                            triggerRumbleClick.store(true, std::memory_order_release);
                            triggerWallSound.store(true, std::memory_order_release);
                            
                            for (size_t i = m_focusedIndex; i < m_items.size(); ++i) {
                                if (m_items[i]->m_isItem) {
                                    m_items[i]->shakeHighlight(FocusDirection::Up);
                                    break;
                                }
                            }
                        } else {
                            triggerRumbleClick.store(true, std::memory_order_release);
                            triggerWallSound.store(true, std::memory_order_release);
                        }
                        triggerShakeOnce = false;
                    }
                } else if (!m_isHolding) {
                    triggerShakeOnce = true;
                }
                
                m_lastNavigationResult = NavigationResult::HitBoundary;
                return oldFocus;
            }
            
            
            inline bool isAtTop() {
                if (m_items.empty()) return true;
                
                // Check if we're at scroll position 0
                if (m_offset != 0.0f) return false;
                
                // Even at offset 0, check if the first item is actually visible
                // This handles cases where the first item might be partially above viewport
                if (!m_items.empty()) {
                    Element* firstItem = m_items[0];
                    return firstItem->getTopBound() >= getTopBound();
                }
                
                return true;
            }
            
            inline bool isAtBottom() {
                if (m_items.empty()) return true;
                
                // First check: are we at the maximum scroll offset?
                //float maxOffset = static_cast<float>(m_listHeight - getHeight());
                const bool atMaxOffset = (m_offset >= static_cast<float>(m_listHeight - getHeight()));
                
                // If list is shorter than viewport, we're always at bottom
                if (m_listHeight <= getHeight()) return true;
                
                // If we're not at max offset, we're definitely not at bottom
                if (!atMaxOffset) return false;
                
                // At max offset - now check if the last item is actually fully visible
                // This prevents wrap-around when there's still content below viewport
                if (!m_items.empty()) {
                    Element* lastItem = m_items.back();
                    //s32 lastItemBottom = lastItem->getBottomBound();
                    //s32 viewportBottom = getBottomBound();
                    
                    // We're truly at bottom only if:
                    // 1. We're at max scroll offset AND
                    // 2. The last item's bottom is at or above the viewport bottom
                    return lastItem->getBottomBound() <= getBottomBound();
                }
                
                return atMaxOffset;
            }

            // Helper to check if there are any focusable items
            inline bool hasAnyFocusableItems() {
                for (size_t i = 0; i < m_items.size(); ++i) {
                    //Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    //
                    //if (test) return true;
                    if (m_items[i]->m_isItem) return true;
                }
                return false;
            }

            
            inline void updateHoldState() {
                const u64 currentTime = armTicksToNs(armGetSystemTick());
                if ((m_lastNavigationTime != 0 && (currentTime - m_lastNavigationTime) < HOLD_THRESHOLD_NS)) {
                    m_isHolding = true;
                } else {
                    m_isHolding = false;
                    m_stoppedAtBoundary = false;
                    m_hasWrappedInCurrentSequence = false;
                }
                m_lastNavigationTime = currentTime;
            }
        
            inline void resetNavigationState() {
                m_hasWrappedInCurrentSequence = false;
                m_lastNavigationResult = NavigationResult::None;
                m_isHolding = false;
                m_stoppedAtBoundary = false;
                m_lastNavigationTime = 0;
            }

            inline Element* handleJumpToItem(Element* oldFocus) {
                resetNavigationState();
                invalidate();
                
                const bool needsScroll = m_listHeight > getHeight();
                const float viewHeight = static_cast<float>(getHeight());
                const float maxOffset = needsScroll ? m_listHeight - viewHeight : 0.0f;
                
                float h = 0.0f;
                
                //float itemHeight, itemCenterPos, viewportCenter, idealOffset;

                for (size_t i = 0; i < m_items.size(); ++i) {
                    m_focusedIndex = i;
                    
                    Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::Down);
                    if (newFocus && newFocus != oldFocus && m_items[i]->matchesJumpCriteria(m_jumpToText, m_jumpToValue, m_jumpToExactMatch)) {
                        // CHANGED: Calculate center of the item and center it in viewport
                        const float itemHeight = m_items[i]->getHeight();
                        // For middle items, use centering logic
                        const float itemCenterPos = h + (itemHeight / 2.0f);  // FIXED: Use center, not bottom
                        const float viewportCenter = viewHeight / 2.0f + VIEW_CENTER_OFFSET + 0.5f; // Same offset as updateScrollOffset
                        //float idealOffset = itemCenterPos - viewportCenter;
                        
                        // Clamp to valid bounds (same as updateScrollOffset)
                        const float idealOffset = std::max(0.0f, std::min(itemCenterPos - viewportCenter, maxOffset));
                        
                        // Set both current and target offset
                        m_offset = m_nextOffset = idealOffset;
                        
                        return newFocus;
                    }
                    
                    h += m_items[i]->getHeight();
                }
                
                // No match found
                return handleInitialFocus(oldFocus);
            }
        
            // Core navigation logic
            // Optimized version with variable definitions pulled outside the loop
            inline Element* navigateDown(Element* oldFocus) {
                size_t searchIndex = m_focusedIndex + 1;
                
                // If currently on a table that needs more scrolling
                if (m_focusedIndex < m_items.size() && m_items[m_focusedIndex]->isTable()) {
                    Element* currentTable = m_items[m_focusedIndex];
                    if (currentTable->getBottomBound() > getBottomBound()) {
                        isTableScrolling.store(true, std::memory_order_release);
                        scrollDown();
                        return oldFocus;
                    }
                }
                
                // Cache invariant values (legitimate optimization)
                const s32 viewBottom = getBottomBound();
                const float containerHeight = getHeight();
                const float offsetPlusHeight = m_offset + containerHeight;
                
                while (searchIndex < m_items.size()) {
                    Element* item = m_items[searchIndex];
                    m_focusedIndex = searchIndex;
                    
                    if (item->isTable()) {
                        // Table needs scrolling
                        const s32 tableBottom = item->getBottomBound();
                        if (tableBottom > viewBottom) {
                            isTableScrolling.store(true, std::memory_order_release);
                            scrollDown();
                            return oldFocus;
                        }
                        searchIndex++;
                        continue;
                    }
                    
                    // Try to focus this item
                    Element* newFocus = item->requestFocus(oldFocus, FocusDirection::Down);
                    if (newFocus && newFocus != oldFocus) {
                        // ONLY reset when we successfully focus something
                        isTableScrolling.store(false, std::memory_order_release);
                        updateScrollOffset();
                        return newFocus;
                    } else {
                        // Non-focusable item (gap/header)
                        const float itemBottom = calculateItemPosition(searchIndex) + item->getHeight();
                        if (itemBottom > offsetPlusHeight) {
                            isTableScrolling.store(true, std::memory_order_release);  // Treat gaps/headers like tables
                            scrollDown();
                            return oldFocus;
                        }
                        searchIndex++;
                    }
                }
                
                return oldFocus;
            }
            
            inline Element* navigateUp(Element* oldFocus) {
                if (m_focusedIndex == 0) return oldFocus;
                ssize_t searchIndex = static_cast<ssize_t>(m_focusedIndex) - 1;
                
                // If currently on a table that needs more scrolling
                if (m_focusedIndex < m_items.size() && m_items[m_focusedIndex]->isTable()) {
                    Element* currentTable = m_items[m_focusedIndex];
                    if (currentTable->getTopBound() < getTopBound()) {
                        isTableScrolling.store(true, std::memory_order_release);
                        scrollUp();
                        return oldFocus;
                    }
                }
                
                // Cache invariant values (legitimate optimization)
                const s32 viewTop = getTopBound();
                const float offset = m_offset;  // Cache in case m_offset is volatile or has accessor overhead
                
                while (searchIndex >= 0) {
                    Element* item = m_items[searchIndex];
                    m_focusedIndex = static_cast<size_t>(searchIndex);
                    
                    if (item->isTable()) {
                        // Table needs scrolling
                        const s32 tableTop = item->getTopBound();
                        if (tableTop < viewTop) {
                            isTableScrolling.store(true, std::memory_order_release);
                            scrollUp();
                            return oldFocus;
                        }
                        searchIndex--;
                        continue;
                    }
                    
                    // Try to focus this item
                    Element* newFocus = item->requestFocus(oldFocus, FocusDirection::Up);
                    if (newFocus && newFocus != oldFocus) {
                        // ONLY reset when we successfully focus something
                        isTableScrolling.store(false, std::memory_order_release);
                        updateScrollOffset();
                        return newFocus;
                    } else {
                        // Non-focusable item (gap/header)
                        const float itemTop = calculateItemPosition(static_cast<size_t>(searchIndex));
                        if (itemTop < offset) {
                            isTableScrolling.store(true, std::memory_order_release);  // Treat gaps/headers like tables
                            scrollUp();
                            return oldFocus;
                        }
                        searchIndex--;
                    }
                }
                
                return oldFocus;
            }
            
            // Helper method to calculate an item's position in the list
            inline float calculateItemPosition(size_t index) {
                float position = 0.0f;
                for (size_t i = 0; i < index && i < m_items.size(); ++i) {
                    position += m_items[i]->getHeight();
                }
                return position;
            }

            // Enhanced scroll methods that ensure we always reach boundaries
            //inline bool canScrollDown() {
            //    if (m_listHeight <= getHeight()) return false;
            //    float maxOffset = static_cast<float>(m_listHeight - getHeight());
            //    return (m_nextOffset < maxOffset - 0.1f) && (m_offset < maxOffset - 0.1f);
            //}
            //
            //inline bool canScrollUp() {
            //    return (m_nextOffset > 0.1f) || (m_offset > 0.1f);
            //}
            
            
            //u64 m_lastScrollNavigationTime = 0;
            //bool m_isHoldingOnTable = false;

            // Enhanced scroll methods that snap to exact boundaries
            inline void scrollDown() {
                const u64 currentTime = armTicksToNs(armGetSystemTick());
                
                // Calculate frame time
                float frameTimeMs = 0.0f;
                if (m_lastScrollTime != 0) {
                    frameTimeMs = static_cast<float>(currentTime - m_lastScrollTime) / 1000000.0f;
                }
                m_lastScrollTime = currentTime;
                
                // Use original frame-based amounts
                float scrollAmount = m_isHolding ? TABLE_SCROLL_STEP_SIZE : TABLE_SCROLL_STEP_SIZE_CLICK;
                
                // If frame took longer than ~33ms (slower than 30fps), scale up the scroll amount
                if (frameTimeMs > 33.0f) {
                    const float scaleFactor = frameTimeMs / 16.67f;  // 16.67ms = 60fps baseline
                    scrollAmount *= std::min(scaleFactor, 3.0f);  // Cap at 3x for very slow frames
                }
                
                m_nextOffset = std::min(m_nextOffset + scrollAmount, 
                                       static_cast<float>(m_listHeight - getHeight()));
            }
            
            inline void scrollUp() {
                const u64 currentTime = armTicksToNs(armGetSystemTick());
                
                // Calculate frame time
                float frameTimeMs = 0.0f;
                if (m_lastScrollTime != 0) {
                    frameTimeMs = static_cast<float>(currentTime - m_lastScrollTime) / 1000000.0f;
                }
                m_lastScrollTime = currentTime;
                
                // Use original frame-based amounts
                float scrollAmount = m_isHolding ? TABLE_SCROLL_STEP_SIZE : TABLE_SCROLL_STEP_SIZE_CLICK;
                
                // If frame took longer than ~33ms (slower than 30fps), scale up the scroll amount
                if (frameTimeMs > 33.0f) {
                    const float scaleFactor = frameTimeMs / 16.67f;  // 16.67ms = 60fps baseline
                    scrollAmount *= std::min(scaleFactor, 3.0f);  // Cap at 3x for very slow frames
                }
                
                m_nextOffset = std::max(m_nextOffset - scrollAmount, 0.0f);
            }

            // Jump to Bottom (original behavior + fixed trigger condition)
            Element* handleJumpToBottom(Element* oldFocus) {
                if (m_items.empty()) return oldFocus;
                
                invalidate();
                resetNavigationState();
                jumpToBottom.store(false, std::memory_order_release);
                
                const float targetOffset = (m_listHeight > getHeight()) ?
                                           static_cast<float>(m_listHeight - getHeight()) : 0.0f;
                static constexpr float tolerance = 5.0f;
            
                // Find last focusable item (search backward)
                size_t lastFocusableIndex = m_items.size();
                for (ssize_t i = static_cast<ssize_t>(m_items.size()) - 1; i >= 0; --i) {
                    Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    if (test) {
                        lastFocusableIndex = static_cast<size_t>(i);
                        break;
                    }
                }
            
                if (lastFocusableIndex == m_items.size())
                    return oldFocus; // no focusable items
            
                const bool alreadyAtBottom = (m_focusedIndex == lastFocusableIndex) &&
                                       (std::abs(m_nextOffset - targetOffset) <= tolerance);
                if (alreadyAtBottom)
                    return oldFocus;
            
                const float oldOffset = m_nextOffset;
                m_focusedIndex = lastFocusableIndex;
                m_nextOffset = targetOffset;
            
                Element* newFocus = m_items[lastFocusableIndex]->requestFocus(oldFocus, FocusDirection::None);
            
                // Trigger feedback if offset or focus changed
                if ((newFocus && newFocus != oldFocus) ||
                    (std::abs(m_nextOffset - oldOffset) > tolerance)) {
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerNavigationSound.store(true, std::memory_order_release);
                    triggerNavigationFeedback();
                }
            
                return newFocus ? newFocus : oldFocus;
            }
            
            
            // Jump to Top (original behavior + fixed trigger condition)
            Element* handleJumpToTop(Element* oldFocus) {
                if (m_items.empty()) return oldFocus;
            
                invalidate();
                resetNavigationState();
                jumpToTop.store(false, std::memory_order_release);
            
                static constexpr float targetOffset = 0.0f;
                static constexpr float tolerance = 5.0f;
            
                // Find first focusable item (search forward)
                size_t firstFocusableIndex = m_items.size();
                for (size_t i = 0; i < m_items.size(); ++i) {
                    Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    if (test) {
                        firstFocusableIndex = i;
                        break;
                    }
                }
            
                if (firstFocusableIndex == m_items.size())
                    return oldFocus; // no focusable items
            
                const bool alreadyAtTop = (m_focusedIndex == firstFocusableIndex) &&
                                    (std::abs(m_nextOffset - targetOffset) <= tolerance);
                if (alreadyAtTop)
                    return oldFocus;
            
                const float oldOffset = m_nextOffset;
                m_focusedIndex = firstFocusableIndex;
                m_nextOffset = targetOffset;
            
                Element* newFocus = m_items[firstFocusableIndex]->requestFocus(oldFocus, FocusDirection::None);
            
                // Trigger feedback if offset or focus changed
                if ((newFocus && newFocus != oldFocus) ||
                    (std::abs(m_nextOffset - oldOffset) > tolerance)) {
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerNavigationSound.store(true, std::memory_order_release);
                    triggerNavigationFeedback();
                }
            
                return newFocus ? newFocus : oldFocus;
            }
            
            Element* handleSkipDown(Element* oldFocus) {
                if (m_items.empty()) return oldFocus;
            
                invalidate();
                resetNavigationState();
            
                const float targetOffset = (m_listHeight > getHeight()) ?
                                           static_cast<float>(m_listHeight - getHeight()) : 0.0f;
                static constexpr float tolerance = 0.0f;
            
                // Find last focusable item
                size_t lastFocusableIndex = m_items.size();
                for (ssize_t i = static_cast<ssize_t>(m_items.size()) - 1; i >= 0; --i) {
                    Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    if (test) {
                        lastFocusableIndex = static_cast<size_t>(i);
                        break;
                    }
                }
            
                const bool alreadyAtBottom = (lastFocusableIndex < m_items.size()) &&
                                       (m_focusedIndex == lastFocusableIndex) &&
                                       (std::abs(m_nextOffset - targetOffset) <= tolerance);
            
                if (alreadyAtBottom) return oldFocus;
            
                const float viewHeight = static_cast<float>(getHeight());
                const float maxOffset = (m_listHeight > viewHeight) ? static_cast<float>(m_listHeight - viewHeight) : 0.0f;
                const float targetViewportTop = std::min(m_offset + viewHeight, maxOffset);
            
                const float actualTravelDistance = targetViewportTop - m_offset;
                const bool traveledFullViewport = (actualTravelDistance >= viewHeight - tolerance);
                const float targetViewportCenter = targetViewportTop + (viewHeight / 2.0f + VIEW_CENTER_OFFSET);
            
                float itemTop = 0.0f;
                size_t targetIndex = 0;
                bool foundFocusable = false;
                float bestDistance = std::numeric_limits<float>::max();
            
                for (size_t i = 0; i < m_items.size(); ++i) {
                    const float itemHeight = m_items[i]->getHeight();
                    const float itemCenter = itemTop + (itemHeight / 2.0f);
                    const float distanceFromCenter = std::abs(itemCenter - targetViewportCenter);
            
                    Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    if (test && test->m_isItem && distanceFromCenter < bestDistance) {
                        targetIndex = i;
                        bestDistance = distanceFromCenter;
                        foundFocusable = true;
                    }
            
                    itemTop += itemHeight;
                }
            
                const float oldOffset = m_nextOffset;
            
                if (foundFocusable) {
                    bool nearBottom = true;
                    if (targetIndex > m_focusedIndex && traveledFullViewport) {
                        m_focusedIndex = targetIndex;
                        nearBottom = false;
                    }
                    isTableScrolling.store(false, std::memory_order_release);
                    updateScrollOffset();
            
                    Element* newFocus = m_items[targetIndex]->requestFocus(oldFocus, FocusDirection::None);
            
                    if (newFocus && newFocus != oldFocus && !nearBottom && traveledFullViewport) {
                        //triggerRumbleClick.store(true, std::memory_order_release);
                        //triggerNavigationSound.store(true, std::memory_order_release);
                        triggerNavigationFeedback();
                        return newFocus;
                    } else {
                        return handleJumpToBottom(oldFocus);
                    }
                } else {
                    // Scroll viewport even if no focusable items
                    isTableScrolling.store(true, std::memory_order_release);
                    m_nextOffset = targetViewportTop;
            
                    if (std::abs(m_nextOffset - oldOffset) > 0.0f) {
                        //triggerRumbleClick.store(true, std::memory_order_release);
                        //triggerNavigationSound.store(true, std::memory_order_release);
                        triggerNavigationFeedback();
                    }
            
                    // Focus last visible focusable item
                    float searchItemTop = 0.0f;
                    size_t lastVisibleFocusable = m_focusedIndex;
            
                    for (size_t i = 0; i < m_items.size(); ++i) {
                        const float itemHeight = m_items[i]->getHeight();
                        const float itemBottom = searchItemTop + itemHeight;
            
                        if (searchItemTop >= targetViewportTop + viewHeight) break;
            
                        if (itemBottom > targetViewportTop) {
                            Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                            if (test && test->m_isItem) lastVisibleFocusable = i;
                        }
            
                        searchItemTop += itemHeight;
                    }
            
                    if (lastVisibleFocusable != m_focusedIndex) {
                        m_focusedIndex = lastVisibleFocusable;
                        Element* newFocus = m_items[m_focusedIndex]->requestFocus(oldFocus, FocusDirection::None);
                        if (newFocus && newFocus != oldFocus) {
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                            return newFocus;
                        }
                    }
                }
            
                return oldFocus;
            }
            
            Element* handleSkipUp(Element* oldFocus) {
                if (m_items.empty()) return oldFocus;
            
                invalidate();
                resetNavigationState();
            
                static constexpr float targetOffset = 0.0f;
                static constexpr float tolerance = 0.0f;
            
                // Find first focusable item
                size_t firstFocusableIndex = m_items.size();
                for (size_t i = 0; i < m_items.size(); ++i) {
                    Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    if (test) {
                        firstFocusableIndex = i;
                        break;
                    }
                }
            
                const bool alreadyAtTop = (firstFocusableIndex < m_items.size()) &&
                                    (m_focusedIndex == firstFocusableIndex) &&
                                    (std::abs(m_nextOffset - targetOffset) <= tolerance);
            
                if (alreadyAtTop) return oldFocus;
            
                const float viewHeight = static_cast<float>(getHeight());
                const float targetViewportTop = std::max(0.0f, m_offset - viewHeight);
            
                const float actualTravelDistance = m_offset - targetViewportTop;
                const bool traveledFullViewport = (actualTravelDistance >= viewHeight - tolerance);
                const float targetViewportCenter = targetViewportTop + (viewHeight / 2.0f + VIEW_CENTER_OFFSET);
            
                float itemTop = 0.0f;
                size_t targetIndex = 0;
                bool foundFocusable = false;
                float bestDistance = std::numeric_limits<float>::max();
            
                for (size_t i = 0; i < m_items.size(); ++i) {
                    const float itemHeight = m_items[i]->getHeight();
                    const float itemCenter = itemTop + (itemHeight / 2.0f);
                    const float distanceFromCenter = std::abs(itemCenter - targetViewportCenter);
            
                    Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                    if (test && test->m_isItem && distanceFromCenter < bestDistance) {
                        targetIndex = i;
                        bestDistance = distanceFromCenter;
                        foundFocusable = true;
                    }
            
                    itemTop += itemHeight;
                }
            
                const float oldOffset = m_nextOffset;
            
                if (foundFocusable) {
                    bool nearTop = true;
                    if (targetIndex < m_focusedIndex && traveledFullViewport) {
                        m_focusedIndex = targetIndex;
                        nearTop = false;
                    }
                    isTableScrolling.store(false, std::memory_order_release);
                    updateScrollOffset();
            
                    Element* newFocus = m_items[targetIndex]->requestFocus(oldFocus, FocusDirection::None);
            
                    if (newFocus && newFocus != oldFocus && !nearTop && traveledFullViewport) {
                        //triggerRumbleClick.store(true, std::memory_order_release);
                        //triggerNavigationSound.store(true, std::memory_order_release);
                        triggerNavigationFeedback();
                        return newFocus;
                    } else {
                        return handleJumpToTop(oldFocus);
                    }
                } else {
                    // Scroll viewport even if no focusable items
                    isTableScrolling.store(true, std::memory_order_release);
                    m_nextOffset = targetViewportTop;
            
                    if (std::abs(m_nextOffset - oldOffset) > 0.0f) {
                        //triggerRumbleClick.store(true, std::memory_order_release);
                        //triggerNavigationSound.store(true, std::memory_order_release);
                        triggerNavigationFeedback();
                    }
            
                    // Focus first visible focusable item
                    float searchItemTop = 0.0f;
                    size_t firstVisibleFocusable = m_focusedIndex;
            
                    for (size_t i = 0; i < m_items.size(); ++i) {
                        const float itemHeight = m_items[i]->getHeight();
                        const float itemBottom = searchItemTop + itemHeight;
            
                        if (itemBottom > targetViewportTop && searchItemTop < targetViewportTop + viewHeight) {
                            Element* test = m_items[i]->requestFocus(nullptr, FocusDirection::None);
                            if (test && test->m_isItem) {
                                firstVisibleFocusable = i;
                                break;
                            }
                        }
            
                        searchItemTop += itemHeight;
                    }
            
                    if (firstVisibleFocusable != m_focusedIndex) {
                        m_focusedIndex = firstVisibleFocusable;
                        Element* newFocus = m_items[m_focusedIndex]->requestFocus(oldFocus, FocusDirection::None);
                        if (newFocus && newFocus != oldFocus) {
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                            return newFocus;
                        }
                    }
                }
            
                return oldFocus;
            }
            
                        
            inline void initializePrefixSums() {
                prefixSums.clear();
                prefixSums.resize(m_items.size() + 1, 0.0f);
                
                for (size_t i = 1; i < prefixSums.size(); ++i) {
                    prefixSums[i] = prefixSums[i - 1] + m_items[i - 1]->getHeight();
                }
            }
            
            
            // Keep your EXACT original updateScrollOffset() method unchanged:
            virtual void updateScrollOffset() {
                if (Element::getInputMode() != InputMode::Controller) return;
                
                if (m_listHeight <= getHeight()) {
                    m_nextOffset = m_offset = 0;
                    return;
                }
                
                // Calculate position of focused item
                float itemPos = 0.0f;
                for (size_t i = 0; i < m_focusedIndex && i < m_items.size(); ++i) {
                    itemPos += m_items[i]->getHeight();
                }
                
                // Get the focused item's height
                const float itemHeight = (m_focusedIndex < m_items.size()) ? m_items[m_focusedIndex]->getHeight() : 0.0f;
                
                // Calculate viewport height
                const float viewHeight = static_cast<float>(getHeight());
            
                // FIXED: Special handling for the first focusable item
                //if (m_focusedIndex == 0 || itemPos <= viewHeight * 0.3f) {
                //    // For items at the very top or very close to top, snap to absolute zero
                //    m_nextOffset = 0.0f;
                //    return;
                //}
                
                // FIXED: Special handling for items near the bottom
                const float maxOffset = static_cast<float>(m_listHeight - getHeight());
                //const float itemBottom = itemPos + itemHeight;
                //if (itemBottom >= m_listHeight - (viewHeight * 0.3f)) {
                //    // For items near the bottom, snap to max offset
                //    m_nextOffset = maxOffset;
                //    return;
                //}
                
                // For middle items, use centering logic
                const float itemCenterPos = itemPos + (itemHeight / 2.0f);
                const float viewportCenter = viewHeight / 2.0f + VIEW_CENTER_OFFSET + 0.5f; // add slight offset
                //float idealOffset = itemCenterPos - viewportCenter;
                
                // Clamp to valid scroll bounds
                const float idealOffset = std::max(0.0f, std::min(itemCenterPos - viewportCenter, maxOffset));
                
                // Set target for smooth animation
                m_nextOffset = idealOffset;

                //m_nextOffset = std::max(0.0f, std::min(itemPos + itemHeight * 0.5f - (viewHeight * 0.5f + 7.0f), maxOffset));
            }
            
        };


        

        /**
         * @brief A item that goes into a list
         *
         */
        class ListItem : public Element {
        public:
            u32 width, height;
            u64 m_touchStartTime_ns;
            bool isLocked = false;
            bool m_shortThresholdCrossed = false;
            #if IS_LAUNCHER_DIRECTIVE
            bool m_longThresholdCrossed = false;
            #endif

        #if IS_LAUNCHER_DIRECTIVE
            ListItem(const std::string& text, const std::string& value = "", bool isMini = false, bool useScriptKey = true)
                : Element(), m_text(text), m_value(value), m_listItemHeight(isMini ? tsl::style::MiniListItemDefaultHeight : tsl::style::ListItemDefaultHeight) {
                m_isItem = true;
                m_flags.m_useScriptKey = useScriptKey;
                m_flags.m_useClickAnimation = true;
                m_text_clean = m_text;
                ult::removeTag(m_text_clean);
                applyInitialTranslations();
                if (!value.empty()) applyInitialTranslations(true);
            }
        #else
            ListItem(const std::string& text, const std::string& value = "", bool isMini = false)
                : Element(), m_text(text), m_value(value), m_listItemHeight(isMini ? tsl::style::MiniListItemDefaultHeight : tsl::style::ListItemDefaultHeight) {
                m_isItem = true;
                m_flags.m_useClickAnimation = true;
                m_text_clean = m_text;
                ult::removeTag(m_text_clean);
                applyInitialTranslations();
                if (!value.empty()) applyInitialTranslations(true);
            }
        #endif
        
            virtual ~ListItem() = default;
        
            virtual void draw(gfx::Renderer *renderer) override {
                const bool useClickTextColor = m_flags.m_touched && Element::getInputMode() == InputMode::Touch && ult::touchInBounds;
                
                if (useClickTextColor) [[unlikely]] {
                    auto drawFunc = ult::expandedMemory ? &gfx::Renderer::drawRectMultiThreaded : &gfx::Renderer::drawRect;
                    (renderer->*drawFunc)(this->getX() + 4, this->getY(), this->getWidth() - 8, this->getHeight(), aWithOpacity(clickColor));
                }
        
                const s16 yOffset = ((tsl::style::ListItemDefaultHeight - m_listItemHeight) >> 1) + 1;
        
                if (!m_maxWidth) [[unlikely]] {
                    calculateWidths(renderer);
                }
        
                // Optimized separator drawing
                const float topBound = this->getTopBound();
                const float bottomBound = this->getBottomBound();
                static float lastBottomBound = 0.0f;
                
                if (lastBottomBound != topBound) [[unlikely]] {
                    renderer->drawRect(this->getX() + 4, topBound, this->getWidth() + 10, 1, a(separatorColor));
                }
                renderer->drawRect(this->getX() + 4, bottomBound, this->getWidth() + 10, 1, a(separatorColor));
                lastBottomBound = bottomBound;
            
            #if IS_LAUNCHER_DIRECTIVE
                static const std::vector<std::string> specialChars = {ult::STAR_SYMBOL};
            #else
                static const std::vector<std::string> specialChars = {ult::DIVIDER_SYMBOL};
            #endif
                // Fast path for non-truncated text
                if (!m_flags.m_truncated) [[likely]] {
                    const Color textColor = m_focused
                        ? (!ult::useSelectionText
                            ? (m_flags.m_hasCustomTextColor ? m_customTextColor : defaultTextColor)
                            : (useClickTextColor
                                ? clickTextColor
                                : selectedTextColor))
                        : (m_flags.m_hasCustomTextColor
                            ? m_customTextColor
                            : (useClickTextColor
                                ? clickTextColor
                                : defaultTextColor));
                #if IS_LAUNCHER_DIRECTIVE
                    renderer->drawStringWithColoredSections(m_text_clean, false, specialChars, this->getX() + 19, this->getY() + 45 - yOffset, 23,
                        textColor, m_focused ? starColor : selectionStarColor);
                #else
                    renderer->drawStringWithColoredSections(m_text_clean, false, specialChars, this->getX() + 19, this->getY() + 45 - yOffset, 23,
                        textColor, textSeparatorColor);
                #endif
                } else {
                    drawTruncatedText(renderer, yOffset, useClickTextColor, specialChars);
                }
        
                if (!m_value.empty()) [[likely]] {
                    drawValue(renderer, yOffset, useClickTextColor);
                }
            }
        
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX() + 3, this->getY(), this->getWidth() + 9, m_listItemHeight);
            }
        
            virtual bool onClick(u64 keys) override {
                if (keys & KEY_A) [[likely]] {
                    triggerRumbleClick.store(true, std::memory_order_release);

                    if (isLocked)
                        triggerWallSound.store(true, std::memory_order_release);
                    else if (m_value.find(ult::CAPITAL_ON_STR) != std::string::npos)
                        triggerOffSound.store(true, std::memory_order_release);
                    else if (m_value.find(ult::CAPITAL_OFF_STR) != std::string::npos)
                        triggerOnSound.store(true, std::memory_order_release);
                    else
                        triggerEnterSound.store(true, std::memory_order_release);
                    
                    if (m_flags.m_useClickAnimation)
                        triggerClickAnimation();
                } else if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) [[unlikely]] {
                    m_clickAnimationProgress = 0;
                }
                //if (keys & KEY_B) {
                //    triggerRumbleDoubleClick.store(true, std::memory_order_release);
                //    triggerExitSound.store(true, std::memory_order_release);
                //    
                //}
                return Element::onClick(keys);
            }
        
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                if (event == TouchEvent::Touch) [[likely]] {
                    if ((m_flags.m_touched = inBounds(currX, currY))) [[likely]] {
                        m_touchStartTime_ns = armTicksToNs(armGetSystemTick());
                        m_shortThresholdCrossed = false;
                        #if IS_LAUNCHER_DIRECTIVE
                        m_longThresholdCrossed = false;
                        #endif
                    }
                    return false;
                }
            
                if (event == TouchEvent::Hold && m_flags.m_touched) [[likely]] {
                    const u64 touchDuration_ns = armTicksToNs(armGetSystemTick()) - m_touchStartTime_ns;
                    const float touchDurationInSeconds = static_cast<float>(touchDuration_ns) * 1e-9f;
                    
                    #if IS_LAUNCHER_DIRECTIVE
                    if (!m_longThresholdCrossed && touchDurationInSeconds >= 1.0f) [[unlikely]] {
                        m_longThresholdCrossed = true;
                        triggerRumbleClick.store(true, std::memory_order_release);
                    } else
                    #endif
                    if (!m_shortThresholdCrossed && touchDurationInSeconds >= 0.5f) [[unlikely]] {
                        m_shortThresholdCrossed = true;
                        triggerRumbleClick.store(true, std::memory_order_release);
                    }
                    return false;
                }
            
                if (event == TouchEvent::Release && m_flags.m_touched) [[likely]] {
                    m_flags.m_touched = false;
                    if (Element::getInputMode() == InputMode::Touch) [[likely]] {
            #if IS_LAUNCHER_DIRECTIVE
                        const s64 keyToUse = determineKeyOnTouchRelease(m_flags.m_useScriptKey);
            #else
                        const s64 keyToUse = determineKeyOnTouchRelease(false);
            #endif
                        const bool handled = onClick(keyToUse);
                        m_clickAnimationProgress = 0;
                        return handled;
                    }
                }
                return false;
            }
            
            virtual void setFocused(bool state) override {
                if (state != m_focused) [[likely]] {
                    m_flags.m_scroll = false;
                    m_scrollOffset = 0;
                    timeIn_ns = armTicksToNs(armGetSystemTick());
                    Element::setFocused(state);
                }
            }
        
            virtual inline Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return this;
            }
        
            inline void setText(const std::string& text) {
                if (m_text != text) [[likely]] {
                    m_text = text;
                    m_text_clean = m_text;
                    ult::removeTag(m_text_clean);
                    resetTextProperties();
                    applyInitialTranslations();
                }
            }
        
            inline void setValue(const std::string& value, bool faint = false) {
                if (m_value != value || m_flags.m_faint != faint) [[likely]] {
                    m_value = value;
                    m_flags.m_faint = faint;
                    m_maxWidth = 0;
                    if (!value.empty()) applyInitialTranslations(true);
                }
            }
            
            inline void setTextColor(Color color) {
                m_customTextColor = color;
                m_flags.m_hasCustomTextColor = true;
            }
            
            inline void setValueColor(Color color) {
                m_customValueColor = color;
                m_flags.m_hasCustomValueColor = true;
            }
            
            inline void clearTextColor() {
                m_flags.m_hasCustomTextColor = false;
            }
            
            inline void clearValueColor() {
                m_flags.m_hasCustomValueColor = false;
            }

            inline void disableClickAnimation() {
                m_flags.m_useClickAnimation = false;
            }

            inline void enableClickAnimation() {
                m_flags.m_useClickAnimation = true;
            }
            
            inline const std::string& getText() const noexcept {
                return m_text;
            }
        
            inline const std::string& getValue() const noexcept {
                return m_value;
            }

            virtual bool matchesJumpCriteria(const std::string& jumpText, const std::string& jumpValue, bool exactMatch=true) const {
                if (jumpText.empty() && jumpValue.empty()) return false;
                
                bool textMatches, valueMatches;
                if (exactMatch) {
                    textMatches = (m_text == jumpText);
                    valueMatches = (m_value == jumpValue);
                } else { // contains check
                    textMatches = (m_text.find(jumpText) != std::string::npos);
                    valueMatches = (m_value.find(jumpValue) != std::string::npos);
                }
                
                if (jumpText.empty() && !jumpValue.empty())
                    return valueMatches;
                else if (!jumpText.empty() && jumpValue.empty())
                    return textMatches;

                return (textMatches && valueMatches);
            }
        
        protected:
            u64 timeIn_ns;
            std::string m_text;
            std::string m_text_clean;
            std::string m_value;
            std::string m_scrollText;
            std::string m_ellipsisText;
            u16 m_listItemHeight;  // Changed from u32 to u16

            
            // Bitfield for boolean flags - saves ~7 bytes per instance
            struct {
                bool m_scroll : 1;
                bool m_truncated : 1;
                bool m_faint : 1;
                bool m_touched : 1;
                bool m_hasCustomTextColor : 1;
                bool m_hasCustomValueColor : 1;
                bool m_useClickAnimation : 1;
            #if IS_LAUNCHER_DIRECTIVE
                bool m_useScriptKey : 1;
            #endif
            } m_flags = {};
        
            Color m_customTextColor = {0};
            Color m_customValueColor = {0};
        
            float m_scrollOffset = 0.0f;
            u16 m_maxWidth = 0;     // Changed from u32 to u16
            u16 m_textWidth = 0;     // Changed from u32 to u16
        
        private:
            // Consolidated scroll constants struct
            struct ScrollConstants {
                double totalCycleDuration;
                double delayDuration;
                double scrollDuration;
                double accelTime;
                double constantVelocityTime;
                double maxVelocity;
                double accelDistance;
                double constantVelocityDistance;
                double minScrollDistance;
                double invAccelTime;
                double invDecelTime;
                double invBillion;
                bool initialized = false;
            };
            
            void applyInitialTranslations(bool isValue = false) {
                std::string& target = isValue ? m_value : m_text_clean;
                ult::applyLangReplacements(target, isValue);
                ult::convertComboToUnicode(target);
                
                #ifdef UI_OVERRIDE_PATH
                {
                    const std::string originalKey = target;
                    
                    std::shared_lock<std::shared_mutex> readLock(tsl::gfx::s_translationCacheMutex);
                    auto translatedIt = ult::translationCache.find(originalKey);
                    if (translatedIt != ult::translationCache.end()) {
                        target = translatedIt->second;
                    } else {
                        readLock.unlock();
                        std::unique_lock<std::shared_mutex> writeLock(tsl::gfx::s_translationCacheMutex);
                        
                        translatedIt = ult::translationCache.find(originalKey);
                        if (translatedIt != ult::translationCache.end()) {
                            target = translatedIt->second;
                        } else {
                            ult::translationCache[originalKey] = originalKey;
                        }
                    }
                }
                #endif
            }
        
            void calculateWidths(gfx::Renderer* renderer) {
                if (m_value.empty()) {
                    m_maxWidth = getWidth() - 62;
                } else {
                    m_maxWidth = getWidth() - renderer->getTextDimensions(m_value, false, 20).first - 66;
                }
            
                const u16 width = renderer->getTextDimensions(m_text_clean, false, 23).first;
                m_flags.m_truncated = width > m_maxWidth + 20;
            
                if (m_flags.m_truncated) [[unlikely]] {
                    m_scrollText.clear();
                    m_scrollText.reserve(m_text_clean.size() * 2 + 8);
                    
                    m_scrollText.append(m_text_clean).append("        ");
                    m_textWidth = renderer->getTextDimensions(m_scrollText, false, 23).first;
                    m_scrollText.append(m_text_clean);
                    
                    m_ellipsisText = renderer->limitStringLength(m_text_clean, false, 23, m_maxWidth);
                } else {
                    m_textWidth = width;
                }
            }
        
            void drawTruncatedText(gfx::Renderer* renderer, s32 yOffset, bool useClickTextColor, const std::vector<std::string>& specialSymbols = {}) {
                if (m_focused) {
                    renderer->enableScissoring(getX() + 6, 97, m_maxWidth + (m_value.empty() ? 49 : 27), tsl::cfg::FramebufferHeight - 170);
                #if IS_LAUNCHER_DIRECTIVE
                    renderer->drawStringWithColoredSections(m_scrollText, false, specialSymbols, getX() + 19 - static_cast<s32>(m_scrollOffset), getY() + 45 - yOffset, 23,
                        !ult::useSelectionText ? defaultTextColor: (useClickTextColor ? clickTextColor : selectedTextColor), starColor);
                #else
                    renderer->drawStringWithColoredSections(m_scrollText, false, specialSymbols, getX() + 19 - static_cast<s32>(m_scrollOffset), getY() + 45 - yOffset, 23,
                        !ult::useSelectionText ? defaultTextColor: (useClickTextColor ? clickTextColor : selectedTextColor), textSeparatorColor);
                #endif
                    renderer->disableScissoring();
                    handleScrolling();
                } else {
                #if IS_LAUNCHER_DIRECTIVE
                    renderer->drawStringWithColoredSections(m_ellipsisText, false, specialSymbols, getX() + 19, getY() + 45 - yOffset, 23,
                        m_flags.m_hasCustomTextColor ? m_customTextColor : (useClickTextColor ? clickTextColor : defaultTextColor), starColor);
                #else
                    renderer->drawStringWithColoredSections(m_ellipsisText, false, specialSymbols, getX() + 19, getY() + 45 - yOffset, 23,
                        m_flags.m_hasCustomTextColor ? m_customTextColor : (useClickTextColor ? clickTextColor : defaultTextColor), textSeparatorColor);
                #endif
                }
            }
                    
            void handleScrolling() {
                static ScrollConstants sc;
                static u64 lastUpdateTime = 0;
                static float cachedScrollOffset = 0.0f;
                
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const u64 elapsed_ns = currentTime_ns - timeIn_ns;
                
                if (!sc.initialized || sc.minScrollDistance != static_cast<double>(m_textWidth)) {
                    sc.delayDuration = 2.0;
                    static constexpr double pauseDuration = 1.0;
                    sc.maxVelocity = 166.0;
                    sc.accelTime = 0.5;
                    static constexpr double decelTime = 0.5;
                    
                    sc.minScrollDistance = static_cast<double>(m_textWidth);
                    sc.accelDistance = 0.5 * sc.maxVelocity * sc.accelTime;
                    const double decelDistance = 0.5 * sc.maxVelocity * decelTime;
                    sc.constantVelocityDistance = std::max(0.0, sc.minScrollDistance - sc.accelDistance - decelDistance);
                    sc.constantVelocityTime = sc.constantVelocityDistance / sc.maxVelocity;
                    sc.scrollDuration = sc.accelTime + sc.constantVelocityTime + decelTime;
                    sc.totalCycleDuration = sc.delayDuration + sc.scrollDuration + pauseDuration;
                    
                    sc.invAccelTime = 1.0 / sc.accelTime;
                    sc.invDecelTime = 1.0 / decelTime;
                    sc.invBillion = 1.0 / 1000000000.0;
                    
                    sc.initialized = true;
                }
                
                const double elapsed_seconds = static_cast<double>(elapsed_ns) * sc.invBillion;
                
                if (currentTime_ns - lastUpdateTime >= 8333333ULL) {
                    const double cyclePosition = std::fmod(elapsed_seconds, sc.totalCycleDuration);
                    
                    if (cyclePosition < sc.delayDuration) [[likely]] {
                        cachedScrollOffset = 0.0f;
                    } else if (cyclePosition < sc.delayDuration + sc.scrollDuration) [[likely]] {
                        const double scrollTime = cyclePosition - sc.delayDuration;
                        double distance;
                        
                        if (scrollTime <= sc.accelTime) {
                            const double t = scrollTime * sc.invAccelTime;
                            const double smoothT = t * t;
                            distance = smoothT * sc.accelDistance;
                        } else if (scrollTime <= sc.accelTime + sc.constantVelocityTime) {
                            const double constantTime = scrollTime - sc.accelTime;
                            distance = sc.accelDistance + (constantTime * sc.maxVelocity);
                        } else {
                            const double decelStartTime = sc.accelTime + sc.constantVelocityTime;
                            const double t = (scrollTime - decelStartTime) * sc.invDecelTime;
                            const double oneMinusT = 1.0 - t;
                            const double smoothT = 1.0 - oneMinusT * oneMinusT;
                            distance = sc.accelDistance + sc.constantVelocityDistance + (smoothT * (sc.minScrollDistance - sc.accelDistance - sc.constantVelocityDistance));
                        }
                        
                        cachedScrollOffset = static_cast<float>(distance < sc.minScrollDistance ? distance : sc.minScrollDistance);
                    } else [[unlikely]] {
                        cachedScrollOffset = static_cast<float>(m_textWidth);
                    }
                    
                    lastUpdateTime = currentTime_ns;
                }
                
                m_scrollOffset = cachedScrollOffset;
                
                if (elapsed_seconds >= sc.totalCycleDuration) [[unlikely]] {
                    timeIn_ns = currentTime_ns;
                }
            }
                    
            void drawValue(gfx::Renderer* renderer, s32 yOffset, bool useClickTextColor) {
                const s32 xPosition = getX() + m_maxWidth + 47;
                const s32 yPosition = getY() + 45 - yOffset-1;
                static constexpr s32 fontSize = 20;
        
                static bool lastRunningInterpreter = false;
                const auto textColor = determineValueTextColor(useClickTextColor, lastRunningInterpreter);
        
                if (m_value != ult::INPROGRESS_SYMBOL) [[likely]] {
                    static const std::vector<std::string> specialChars = {ult::DIVIDER_SYMBOL};
                    renderer->drawStringWithColoredSections(m_value, false, specialChars, xPosition, yPosition, fontSize, textColor, textSeparatorColor);
                } else {
                    drawThrobber(renderer, xPosition, yPosition, fontSize, textColor);
                }
                lastRunningInterpreter = ult::runningInterpreter.load(std::memory_order_acquire);
            }
                    
            Color determineValueTextColor(bool useClickTextColor, bool lastRunningInterpreter) const {
                if (m_focused && ult::useSelectionValue) {
                    if (m_value == ult::DROPDOWN_SYMBOL || m_value == ult::OPTION_SYMBOL) {
                        return useClickTextColor ? (clickTextColor) :
                               (m_flags.m_faint ? offTextColor : (useClickTextColor ? clickTextColor : (ult::useSelectionText ? selectedTextColor : defaultTextColor)));
                    }
                    
                    const bool isRunning = ult::runningInterpreter.load(std::memory_order_acquire) || lastRunningInterpreter;
                    if (isRunning && (m_value.find(ult::DOWNLOAD_SYMBOL) != std::string::npos ||
                                     m_value.find(ult::UNZIP_SYMBOL) != std::string::npos ||
                                     m_value.find(ult::COPY_SYMBOL) != std::string::npos)) {
                        return m_flags.m_faint ? offTextColor : (inprogressTextColor);
                    }
                    
                    if (m_value == ult::INPROGRESS_SYMBOL) {
                        return m_flags.m_faint ? offTextColor : (inprogressTextColor);
                    }
                    
                    if (m_value == ult::CROSSMARK_SYMBOL) {
                        return m_flags.m_faint ? offTextColor : (invalidTextColor);
                    }
                    
                    return useClickTextColor ? clickTextColor : selectedValueTextColor;
                }
                
                if (m_flags.m_hasCustomValueColor) {
                    return m_customValueColor;
                }
                
                if (m_value == ult::DROPDOWN_SYMBOL || m_value == ult::OPTION_SYMBOL) {
                    return (m_focused ? (useClickTextColor ? clickTextColor : (m_flags.m_faint ? offTextColor : (ult::useSelectionText ? selectedTextColor : defaultTextColor))) :
                           (useClickTextColor ? clickTextColor : (m_flags.m_faint ? offTextColor : defaultTextColor)));
                }
                
                const bool isRunning = ult::runningInterpreter.load(std::memory_order_acquire) || lastRunningInterpreter;
                if (isRunning && (m_value.find(ult::DOWNLOAD_SYMBOL) != std::string::npos ||
                                 m_value.find(ult::UNZIP_SYMBOL) != std::string::npos ||
                                 m_value.find(ult::COPY_SYMBOL) != std::string::npos)) {
                    return m_flags.m_faint ? offTextColor : (inprogressTextColor);
                }
                
                if (m_value == ult::INPROGRESS_SYMBOL) {
                    return m_flags.m_faint ? offTextColor : (inprogressTextColor);
                }
                
                if (m_value == ult::CROSSMARK_SYMBOL) {
                    return m_flags.m_faint ? offTextColor : (invalidTextColor);
                }
                
                return (m_flags.m_faint ? offTextColor : (onTextColor));
            }

            void drawThrobber(gfx::Renderer* renderer, s32 xPosition, s32 yPosition, s32 fontSize, Color textColor) {
                static size_t throbberCounter = 0;
                const auto& throbberSymbol = ult::THROBBER_SYMBOLS[(throbberCounter / 10) % ult::THROBBER_SYMBOLS.size()];
                throbberCounter = (throbberCounter + 1) % (10 * ult::THROBBER_SYMBOLS.size());
                renderer->drawString(throbberSymbol, false, xPosition, yPosition, fontSize, textColor);
            }
            
            s64 determineKeyOnTouchRelease(bool useScriptKey) const {
                const u64 touchDuration_ns = armTicksToNs(armGetSystemTick()) - m_touchStartTime_ns;
                const float touchDurationInSeconds = static_cast<float>(touchDuration_ns) * 1e-9f;
                
                #if IS_LAUNCHER_DIRECTIVE
                if (touchDurationInSeconds >= 1.0f) [[unlikely]] {
                    ult::longTouchAndRelease.store(true, std::memory_order_release);
                    return useScriptKey ? SCRIPT_KEY : STAR_KEY;
                }
                #endif
                if (touchDurationInSeconds >= 0.5f) [[unlikely]] {
                    ult::shortTouchAndRelease.store(true, std::memory_order_release);
                    return useScriptKey ? SCRIPT_KEY : SETTINGS_KEY;
                }
                return KEY_A;
            }
        
            void resetTextProperties() {
                m_scrollText.clear();
                m_ellipsisText.clear();
                m_maxWidth = 0;
            }
        };
        
        class MiniListItem : public ListItem {
        public:
        #if IS_LAUNCHER_DIRECTIVE
            // Constructor for MiniListItem, with no `isMini` boolean.
            MiniListItem(const std::string& text, const std::string& value = "", bool useScriptKey = false) 
                : ListItem(text, value, true, useScriptKey) { // Call the parent constructor with `isMini = true`
        #else
            MiniListItem(const std::string& text, const std::string& value = "")
                : ListItem(text, value, true) {  // Call the parent constructor with `isMini = true`
        #endif
            
                // Additional MiniListItem-specific initialization can go here, if necessary.
            }
            
            // Destructor if needed (inherits default behavior from ListItem)
            virtual ~MiniListItem() {}
        };

        /**
         * @brief A item that goes into a list (this version uses value and faint color sourcing)
         *
         */
        class ListItemV2 : public Element {
        public:
            u32 width, height;
            u64 m_touchStartTime_ns;  // Track the time when touch starts
        
            /**
             * @brief Constructor
             *
             * @param text Initial description text
             */
            ListItemV2(const std::string& text, const std::string& value = "", Color valueColor = onTextColor, Color faintColor = offTextColor)
                : Element(), m_text(text), m_value(value), m_valueColor{valueColor}, m_faintColor{faintColor} {
            }
            virtual ~ListItemV2() {}
        
                    
            virtual void draw(gfx::Renderer *renderer) override {
                static float lastBottomBound;
                bool useClickTextColor = false;
                if (this->m_touched && Element::getInputMode() == InputMode::Touch) {
                    if (ult::touchInBounds) {
                        //renderer->drawRect(ELEMENT_BOUNDS(this), a(clickColor));
                        renderer->drawRect( this->getX()+4, this->getY(), this->getWidth()-8, this->getHeight(), a(clickColor));
                        useClickTextColor = true;
                    }
                    //renderer->drawRect(ELEMENT_BOUNDS(this), tsl::style::color::ColorClickAnimation);
                }
            
                // Calculate vertical offset to center the text
                const s32 yOffset = (tsl::style::ListItemDefaultHeight - this->m_listItemHeight) / 2;
            
                if (this->m_maxWidth == 0) {
                    if (this->m_value.length() > 0) {
                        //std::tie(width, height) = renderer->drawString(this->m_value, false, 0, 0, 20, a(tsl::style::color::ColorTransparent));
                        //auto valueWidth = renderer->getTextDimensions(this->m_value, false, 20).first;
                        width = renderer->getTextDimensions(this->m_value, false, 20).first;
                        this->m_maxWidth = this->getWidth() - width - 70 +4;
                    } else {
                        this->m_maxWidth = this->getWidth() - 40 -10 -12;
                    }
                    
                    //std::tie(width, height) = renderer->drawString(this->m_text, false, 0, 0, 23, a(tsl::style::color::ColorTransparent));
                    //auto textWidth = renderer->getTextDimensions(this->m_text, false, 23).first;
                    width = renderer->getTextDimensions(this->m_text, false, 23).first;
                    this->m_trunctuated = width > this->m_maxWidth+20;
                    
                    if (this->m_trunctuated) {
                        this->m_scrollText = this->m_text + "        ";
                        //std::tie(width, height) = renderer->drawString(this->m_scrollText, false, 0, 0, 23, a(tsl::style::color::ColorTransparent));
                        //auto scrollWidth = renderer->getTextDimensions(this->m_scrollText, false, 23).first;
                        width = renderer->getTextDimensions(this->m_scrollText, false, 23).first;
                        this->m_scrollText += this->m_text;
                        this->m_textWidth = width;
                        
                        this->m_ellipsisText = renderer->limitStringLength(this->m_text, false, 23, this->m_maxWidth);
                    } else {
                        this->m_textWidth = width;
                    }
                }
                
                if (lastBottomBound !=  this->getTopBound())
                    renderer->drawRect(this->getX()+4, this->getTopBound(), this->getWidth()+6 +4, 1, a(separatorColor));
                renderer->drawRect(this->getX()+4, this->getBottomBound(), this->getWidth()+6 +4, 1, a(separatorColor));
            
                lastBottomBound = this->getBottomBound();
                
            
                if (this->m_trunctuated) {
                    if (this->m_focused) {
                        if (this->m_value.length() > 0)
                            renderer->enableScissoring(this->getX()+6, 97, this->m_maxWidth + 30 -3, tsl::cfg::FramebufferHeight-73-97);
                        else
                            renderer->enableScissoring(this->getX()+6, 97, this->m_maxWidth + 40 +9, tsl::cfg::FramebufferHeight-73-97);
                        renderer->drawString(this->m_scrollText, false, this->getX() + 20-1 - this->m_scrollOffset, this->getY() + 45 - yOffset, 23, selectedTextColor);
                        renderer->disableScissoring();
                        
                        // Handle scrolling with frame rate compensation
                        const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                        const u64 elapsed_ns = currentTime_ns - this->timeIn_ns;
                        
                        // Frame rate compensation - cache calculations to reduce stutter
                        static u64 lastUpdateTime = 0;
                        static float cachedScrollOffset = 0.0f;
                        
                        // Pre-compute constants as statics to avoid recalculation
                        static bool constantsInitialized = false;
                        static double totalCycleDuration;
                        static double delayDuration;
                        static double scrollDuration;
                        static double accelTime;
                        static double constantVelocityTime;
                        static double maxVelocity;
                        static double accelDistance;
                        static double constantVelocityDistance;
                        static double minScrollDistance;
                        static double invAccelTime;  // 1/accelTime for multiplication instead of division
                        static double invDecelTime;  // 1/decelTime for multiplication instead of division
                        static double invBillion;    // 1/1000000000.0 for ns to seconds conversion
                        
                        if (!constantsInitialized || minScrollDistance != static_cast<double>(this->m_textWidth)) {
                            // Constants for velocity-based scrolling
                            delayDuration = 2.0;
                            static constexpr double pauseDuration = 1.0;
                            maxVelocity = 166.0;
                            accelTime = 0.5;
                            static constexpr double decelTime = 0.5;
                            
                            // Pre-calculate derived constants
                            minScrollDistance = static_cast<double>(this->m_textWidth);
                            accelDistance = 0.5 * maxVelocity * accelTime;
                            const double decelDistance = 0.5 * maxVelocity * decelTime;
                            constantVelocityDistance = std::max(0.0, minScrollDistance - accelDistance - decelDistance);
                            constantVelocityTime = constantVelocityDistance / maxVelocity;
                            scrollDuration = accelTime + constantVelocityTime + decelTime;
                            totalCycleDuration = delayDuration + scrollDuration + pauseDuration;
                            
                            // Pre-calculate reciprocals for faster division
                            invAccelTime = 1.0 / accelTime;
                            invDecelTime = 1.0 / decelTime;
                            invBillion = 1.0 / 1000000000.0;
                            
                            constantsInitialized = true;
                        }
                        
                        // Fast ns to seconds conversion
                        const double elapsed_seconds = static_cast<double>(elapsed_ns) * invBillion;
                        
                        // Update at consistent intervals regardless of frame rate
                        if (currentTime_ns - lastUpdateTime >= 8333333ULL) { // ~120 FPS update rate
                            // Use std::fmod for modulo - it's optimized and faster than loops
                            const double cyclePosition = std::fmod(elapsed_seconds, totalCycleDuration);
                            
                            if (cyclePosition < delayDuration) {
                                // Delay phase - no scrolling
                                cachedScrollOffset = 0.0f;
                            } else if (cyclePosition < delayDuration + scrollDuration) {
                                // Scrolling phase - velocity-based movement
                                const double scrollTime = cyclePosition - delayDuration;
                                double distance;
                                
                                if (scrollTime <= accelTime) {
                                    // Acceleration phase - quadratic ease-in
                                    const double t = scrollTime * invAccelTime;  // Multiply instead of divide
                                    const double smoothT = t * t;
                                    distance = smoothT * accelDistance;
                                } else if (scrollTime <= accelTime + constantVelocityTime) {
                                    // Constant velocity phase
                                    const double constantTime = scrollTime - accelTime;
                                    distance = accelDistance + (constantTime * maxVelocity);
                                } else {
                                    // Deceleration phase - quadratic ease-out
                                    const double decelStartTime = accelTime + constantVelocityTime;
                                    const double t = (scrollTime - decelStartTime) * invDecelTime;  // Multiply instead of divide
                                    const double oneMinusT = 1.0 - t;
                                    const double smoothT = 1.0 - oneMinusT * oneMinusT;  // Avoid repeated calculation
                                    distance = accelDistance + constantVelocityDistance + (smoothT * (minScrollDistance - accelDistance - constantVelocityDistance));
                                }
                                
                                // Use branchless min with conditional move behavior
                                cachedScrollOffset = static_cast<float>(distance < minScrollDistance ? distance : minScrollDistance);
                            } else {
                                // Pause phase - stay at end
                                cachedScrollOffset = static_cast<float>(this->m_textWidth);
                            }
                            
                            lastUpdateTime = currentTime_ns;
                        }
                        
                        // Use cached value for consistent display
                        this->m_scrollOffset = cachedScrollOffset;
                        
                        // Reset timer when cycle completes
                        if (elapsed_seconds >= totalCycleDuration) {
                            this->timeIn_ns = currentTime_ns;
                        }
                    } else {
                        renderer->drawString(this->m_ellipsisText, false, this->getX() + 20-1, this->getY() + 45 - yOffset, 23, !useClickTextColor ? defaultTextColor : clickTextColor);
                    }
                } else {
                    // Render the text with special character handling
                #if IS_LAUNCHER_DIRECTIVE
                    static const std::vector<std::string> specialChars = {ult::STAR_SYMBOL};
                #else
                    static const std::vector<std::string> specialChars = {};
                #endif
                    renderer->drawStringWithColoredSections(this->m_text, false, specialChars, this->getX() + 20-1, this->getY() + 45 - yOffset, 23,
                        (this->m_focused ? (!useClickTextColor ? selectedTextColor : clickTextColor) : (!useClickTextColor ? defaultTextColor : clickTextColor)),
                        (this->m_focused ? starColor : selectionStarColor)
                    );
                }
                
                
                // CUSTOM SECTION START (modification for submenu footer color)
                const s32 xPosition = this->getX() + this->m_maxWidth + 44 + 3;
                const s32 yPosition = this->getY() + 45 - yOffset;
                static constexpr s32 fontSize = 20;
            
            
                //static bool lastRunningInterpreter = ult::runningInterpreter.load(std::memory_order_acquire);
            
                // Determine text color
                const auto textColor = this->m_faint ? a(m_faintColor) : a(m_valueColor);
                
                if (this->m_value != ult::INPROGRESS_SYMBOL) {
                    // Draw the string with the determined text color
                    renderer->drawString(this->m_value, false, xPosition, yPosition, fontSize, textColor);
                } else {
                    static size_t throbberCounter = 0;

                    
                    // Reset counter to prevent overflow (every full cycle)
                    if (throbberCounter >= 10 * ult::THROBBER_SYMBOLS.size()) {
                        throbberCounter = 0;
                    }
                    
                    // Get current throbber symbol (changes every 10 frames)
                    const size_t symbolIndex = (throbberCounter / 10) % ult::THROBBER_SYMBOLS.size();
                    const std::string& currentSymbol = ult::THROBBER_SYMBOLS[symbolIndex];
                    
                    // Instance-specific counter for independent throbber animation
                    ++throbberCounter;

                    renderer->drawString(currentSymbol, false, xPosition, yPosition, fontSize, textColor);
                }
                //lastRunningInterpreter = ult::runningInterpreter.load(std::memory_order_acquire);
            }
        
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX()+2+1, this->getY(), this->getWidth()+8+1, m_listItemHeight);
            }
        
            virtual bool onClick(u64 keys) override {
                if (keys & KEY_A) {
                    this->triggerClickAnimation();
                }
                else if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
                    this->m_clickAnimationProgress = 0;
        
                return Element::onClick(keys);
            }
        
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                if (event == TouchEvent::Touch)
                    this->m_touched = this->inBounds(currX, currY);
        
                if (event == TouchEvent::Release && this->m_touched) {
                    this->m_touched = false;
        
                    if (Element::getInputMode() == InputMode::Touch) {
                        const bool handled = this->onClick(KEY_A);
        
                        this->m_clickAnimationProgress = 0;
                        return handled;
                    }
                }
                
                
                return false;
            }
        
        
            virtual void setFocused(bool state) override {
                this->m_scroll = false;
                this->m_scrollOffset = 0;
                this->timeIn_ns = armTicksToNs(armGetSystemTick());
                Element::setFocused(state);
            }
        
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return this;
            }
        
            /**
             * @brief Sets the left hand description text of the list item
             *
             * @param text Text
             */
            inline void setText(const std::string& text) {
                this->m_text = text;
                this->m_scrollText = "";
                this->m_ellipsisText = "";
                this->m_maxWidth = 0;
            }
        
            /**
             * @brief Sets the right hand value text of the list item
             *
             * @param value Text
             * @param faint Should the text be drawn in a glowing green or a faint gray
             */
            inline void setValue(const std::string& value, bool faint = false) {
                this->m_value = value;
                this->m_faint = faint;
                this->m_maxWidth = 0;
            }
        
            /**
             * @brief Sets the value color
             *
             * @param value_color color of the value
             */
            inline void setValueColor(Color value_color) {
                this->m_valueColor = value_color;
            }
        
            /**
             * @brief Sets the faint color
             *
             * @param faint_color color of the faint
             */
            inline void setFaintColor(Color faint_color) {
                this->m_faintColor = faint_color;
            }
        
            /**
             * @brief Gets the left hand description text of the list item
             *
             * @return Text
             */
            inline const std::string& getText() const {
                return this->m_text;
            }
        
            /**
             * @brief Gets the right hand value text of the list item
             *
             * @return Value
             */
            inline const std::string& getValue() {
                return this->m_value;
            }
        
        protected:
            u64 timeIn_ns;
        
            std::string m_text;
            std::string m_value;
            std::string m_scrollText;
            std::string m_ellipsisText;
            u32 m_listItemHeight = tsl::style::ListItemDefaultHeight;
        
        #if IS_LAUNCHER_DIRECTIVE
            bool m_useScriptKey = false;
        #endif
            Color m_valueColor;
            Color m_faintColor;
        
            bool m_scroll = false;
            bool m_trunctuated = false;
            bool m_faint = false;
        
            bool m_touched = false;
        
            u16 m_maxScroll = 0;
            u16 m_scrollOffset = 0;
            u32 m_maxWidth = 0;
            u32 m_textWidth = 0;
            u16 m_scrollAnimationCounter = 0;
        };

        /**
         * @brief A toggleable list item that changes the state from On to Off when the A button gets pressed
         *
         */
        class ToggleListItem : public ListItem {
        public:
            /**
             * @brief Constructor
             *
             * @param text Initial description text
             * @param initialState Is the toggle set to On or Off initially
             * @param onValue Value drawn if the toggle is on
             * @param offValue Value drawn if the toggle is off
             */
            ToggleListItem(const std::string& text, bool initialState, const std::string& onValue = ult::ON, const std::string& offValue = ult::OFF, bool isMini = false, bool delayedHandle=false)
                : ListItem(text, "", isMini), m_state(initialState), m_onValue(onValue), m_offValue(offValue), m_delayedHandle(delayedHandle) {
                this->setState(this->m_state);
            }
            
            virtual ~ToggleListItem() {}
            
            virtual bool onClick(u64 keys) override {

                #if IS_LAUNCHER_DIRECTIVE
                if (ult::runningInterpreter.load(std::memory_order_acquire))
                    return false;
                #endif

                // Handle KEY_A for toggling
                if (keys & KEY_A) {
                    triggerRumbleClick.store(true, std::memory_order_release);
                    if (!this->m_state)
                        triggerOnSound.store(true, std::memory_order_release);
                    else
                        triggerOffSound.store(true, std::memory_order_release);
                    
                    
                    this->m_state = !this->m_state;
                    
                    if (!m_delayedHandle)
                        this->setState(this->m_state);
                    
                    this->m_stateChangedListener(this->m_state);
                    this->triggerClickAnimation();
                    
                    return true;
                }
                //if (keys & KEY_B) {
                //    triggerRumbleDoubleClick.store(true, std::memory_order_release);
                //    triggerExitSound.store(true, std::memory_order_release);
                //    
                //}

                #if IS_LAUNCHER_DIRECTIVE
                // Handle SCRIPT_KEY for executing script logic
                else if (keys & SCRIPT_KEY) {
                    // Trigger the script key listener
                    if (this->m_scriptKeyListener) {
                        this->m_scriptKeyListener(this->m_state);  // Pass the current state to the script key listener
                    }
                    return ListItem::onClick(keys);
                }
                #endif
                return false;
            }
            
            /**
             * @brief Gets the current state of the toggle
             *
             * @return State
             */
            virtual inline bool getState() {
                return this->m_state;
            }
            
            /**
             * @brief Sets the current state of the toggle. Updates the Value
             *
             * @param state State
             */
            virtual inline void setState(bool state) {
                #if IS_LAUNCHER_DIRECTIVE
                if (ult::runningInterpreter.load(std::memory_order_acquire))
                    return;
                #endif

                this->m_state = state;
                this->setValue(state ? this->m_onValue : this->m_offValue, !state);
            }
            
            /**
             * @brief Adds a listener that gets called whenever the state of the toggle changes
             *
             * @param stateChangedListener Listener with the current state passed in as parameter
             */
            void setStateChangedListener(std::function<void(bool)> stateChangedListener) {
                this->m_stateChangedListener = stateChangedListener;
            }

            #if IS_LAUNCHER_DIRECTIVE
            // Attach the script key listener for SCRIPT_KEY handling
            void setScriptKeyListener(std::function<void(bool)> scriptKeyListener) {
                this->m_scriptKeyListener = scriptKeyListener;
            }
            #endif
            

        protected:
            bool m_state = true;

            std::string m_onValue, m_offValue;
            bool m_delayedHandle = false;
            
            std::function<void(bool)> m_stateChangedListener = [](bool){};

            #if IS_LAUNCHER_DIRECTIVE
            std::function<void(bool)> m_scriptKeyListener = nullptr;     // Script key listener (with state)
            #endif
        };
        
        class MiniToggleListItem : public ToggleListItem {
        public:
            // Constructor for MiniToggleListItem, with no `isMini` boolean.
            MiniToggleListItem(const std::string& text, bool initialState, const std::string& onValue = ult::ON, const std::string& offValue = ult::OFF)
                : ToggleListItem(text, initialState, onValue, offValue, true) {
            }
            
            // Destructor if needed (inherits default behavior from ListItem)
            virtual ~MiniToggleListItem() {}
        };


        class DummyListItem : public ListItem {
        public:
            DummyListItem()
                : ListItem("") { // Use an empty string for the base class constructor
                // Set the properties to indicate it's a dummy item
                this->m_text = "";
                this->m_value = "";
                this->m_maxWidth = 0;
                this->width = 0;
                this->height = 0;
                m_isItem = false;
                isLocked = true;
            }
            
            virtual ~DummyListItem() {}
            
            // Override the draw method to do nothing
            virtual void draw(gfx::Renderer* renderer) override {
                // Intentionally left blank
            }
            
            // Override the layout method to set the dimensions to zero
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                //this->setBoundaries(parentX, parentY, 0, 0); // Zero size
                this->setBoundaries(this->getX(), this->getY(), 0, 0);
            }
            
            // Override the requestFocus method to allow this item to be focusable
            virtual inline Element* requestFocus(Element* oldFocus, FocusDirection direction) override {
                return this; // Allow this item to be focusable
            }
            
            //// Optionally override onClick and onTouch to handle interactions
            //virtual bool onClick(u64 keys) override {
            //    return true; // Consume the click event
            //}
            //
            //virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
            //    return true; // Consume the touch event
            //}
        };


        class CategoryHeader : public Element {
        public:
            CategoryHeader(const std::string &title, bool hasSeparator = true) 
                : m_text(title), m_hasSeparator(hasSeparator), timeIn_ns(0),
                  m_scroll(false), m_truncated(false), m_scrollOffset(0.0f), 
                  m_maxWidth(0), m_textWidth(0) {
                ult::applyLangReplacements(m_text);
                ult::convertComboToUnicode(m_text);
                m_isItem = false;
            }
            
            virtual ~CategoryHeader() {}
            
            virtual void draw(gfx::Renderer *renderer) override {
                static const std::vector<std::string> specialChars = {""};
                
                // Calculate widths if not done yet
                if (!m_maxWidth) {
                    calculateWidths(renderer);
                }
                
                // Draw separator if needed
                if (this->m_hasSeparator) {
                    renderer->drawRect(this->getX()+1+1, this->getBottomBound() - 29-4, 4, 22, aWithOpacity(headerSeparatorColor));
                }
                
                // Determine text position
                const int textX = m_hasSeparator ? (this->getX() + 15+1) : this->getX();
                const int textY = this->getBottomBound() - 12-4;
                
                // Handle scrolling text if truncated
                if (m_truncated) {
                    if (!m_scroll) {
                        m_scroll = true;
                        timeIn_ns = armTicksToNs(armGetSystemTick());
                    }
                    
                    // Calculate scissoring bounds that respect parent clipping
                    const int scissorX = textX;
                    const int scissorY = textY - 16;
                    const int scissorWidth = m_maxWidth;
                    const int scissorHeight = 24;
                    
                    // Get parent bounds (you'll need to implement this based on your parent system)
                    // This assumes your parent has some way to get its visible bounds
                    if (Element* parent = this->getParent()) {
                        const int parentTop = parent->getY()-8; // or whatever method gets the top bound
                        const int parentBottom = parent->getBottomBound(); // or equivalent
                        const int parentLeft = parent->getX();
                        const int parentRight = parent->getX() + parent->getWidth();
                        
                        // Clip scissor rectangle to parent bounds
                        const int clipLeft = std::max(scissorX, parentLeft);
                        const int clipRight = std::min(scissorX + scissorWidth, parentRight);
                        const int clipTop = std::max(scissorY, parentTop);
                        const int clipBottom = std::min(scissorY + scissorHeight, parentBottom);
                        
                        // Only enable scissoring if there's a visible area
                        if (clipLeft < clipRight && clipTop < clipBottom) {
                            renderer->enableScissoring(clipLeft, clipTop, 
                                                     clipRight - clipLeft, 
                                                     clipBottom - clipTop);
                            
                            renderer->drawStringWithColoredSections(m_scrollText, false, specialChars, 
                                textX - static_cast<s32>(m_scrollOffset), textY, 16, 
                                headerTextColor, textSeparatorColor);
                            
                            renderer->disableScissoring();
                        } else {
                            // Draw normal or ellipsis text
                            //const std::string& displayText = m_truncated ? m_ellipsisText : m_text;
                            renderer->drawStringWithColoredSections(m_text, false, specialChars, 
                                textX, textY, 16, headerTextColor, textSeparatorColor);
                        }
                        // If completely clipped, don't draw anything
                    } else {
                        // Draw normal or ellipsis text
                        //const std::string& displayText = m_truncated ? m_ellipsisText : m_text;
                        renderer->drawStringWithColoredSections(m_text, false, specialChars, 
                            textX, textY, 16, headerTextColor, textSeparatorColor);
                    }
                    
                    handleScrolling();
                } else {
                    // Draw normal or ellipsis text
                    //const std::string& displayText = m_truncated ? m_ellipsisText : m_text;
                    renderer->drawStringWithColoredSections(m_text, false, specialChars, 
                        textX, textY, 16, headerTextColor, textSeparatorColor);
                }
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                // Check if the CategoryHeader is part of a list and if it's the first entry in it, half it's height
                if (List *list = static_cast<List*>(this->getParent()); list != nullptr) {
                    if (list->getIndexInList(this) == 0) {
                        this->setBoundaries(this->getX(), this->getY(), this->getWidth(), 29+4);
                        return;
                    }
                }
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), tsl::style::ListItemDefaultHeight *0.90);
            }
            
            virtual bool onClick(u64 keys) {
                return false;
            }
            
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return nullptr;
            }
            
            virtual void setFocused(bool state) override {}
            
            inline void setText(const std::string &text) {
                if (this->m_text != text) {
                    this->m_text = text;
                    ult::applyLangReplacements(m_text);
                    ult::convertComboToUnicode(m_text);
                    //resetTextProperties();
                }
            }
            
            inline const std::string& getText() const {
                return this->m_text;
            }
        
        private:
            std::string m_text;
            bool m_hasSeparator;
            
            // Scrolling properties (matching ListItem)
            u64 timeIn_ns;
            std::string m_scrollText;
            //std::string m_ellipsisText;
            bool m_scroll;
            bool m_truncated;
            float m_scrollOffset;
            u32 m_maxWidth;
            u32 m_textWidth;
            

            // Frame rate compensation - cache calculations to reduce stutter
            u64 lastUpdateTime = 0;
            float cachedScrollOffset = 0.0f;
            
            // Pre-compute constants as statics to avoid recalculation
            bool constantsInitialized = false;
            double totalCycleDuration;
            double delayDuration;
            double scrollDuration;
            double accelTime;
            double constantVelocityTime;
            double maxVelocity;
            double accelDistance;
            double constantVelocityDistance;
            double minScrollDistance;
            double invAccelTime;
            double invDecelTime;
            double invBillion;

            void calculateWidths(gfx::Renderer* renderer) {
                // Available width (accounting for separator and margins)
                m_maxWidth = getWidth() - (m_hasSeparator ? 20-3 : 4);
                
                // Get actual text width
                const u32 width = renderer->getTextDimensions(m_text, false, 16).first;
                m_truncated = width > m_maxWidth;
                
                if (m_truncated) {
                    // Build scroll text: "text        text"
                    m_scrollText.clear();
                    m_scrollText.reserve(m_text.size() * 2 + 8);
                    m_scrollText.append(m_text).append("        ");
                    m_textWidth = renderer->getTextDimensions(m_scrollText, false, 16).first;
                    m_scrollText.append(m_text);
                    
                    // Create ellipsis text
                    //m_ellipsisText = renderer->limitStringLength(m_text, false, 16, m_maxWidth);
                } else {
                    m_textWidth = width;
                }
            }
            
            void handleScrolling() {
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const u64 elapsed_ns = currentTime_ns - timeIn_ns;
                
                
                if (!constantsInitialized || minScrollDistance != static_cast<double>(m_textWidth)) {
                    // Constants for velocity-based scrolling (3 second pauses as requested)
                    delayDuration = 3.0;  // 3 second pause at start
                    static constexpr double pauseDuration = 2.0;  // 3 second pause at end
                    maxVelocity = 100.0;  // Adjust for desired scroll speed
                    accelTime = 0.5;
                    static constexpr double decelTime = 0.5;
                    
                    // Pre-calculate derived constants
                    minScrollDistance = static_cast<double>(m_textWidth);
                    accelDistance = 0.5 * maxVelocity * accelTime;
                    const double decelDistance = 0.5 * maxVelocity * decelTime;
                    constantVelocityDistance = std::max(0.0, minScrollDistance - accelDistance - decelDistance);
                    constantVelocityTime = constantVelocityDistance / maxVelocity;
                    scrollDuration = accelTime + constantVelocityTime + decelTime;
                    totalCycleDuration = delayDuration + scrollDuration + pauseDuration;
                    
                    // Pre-calculate reciprocals for faster division
                    invAccelTime = 1.0 / accelTime;
                    invDecelTime = 1.0 / decelTime;
                    invBillion = 1.0 / 1000000000.0;
                    
                    constantsInitialized = true;
                }
                
                // Fast ns to seconds conversion
                const double elapsed_seconds = static_cast<double>(elapsed_ns) * invBillion;
                
                // Update at consistent intervals regardless of frame rate
                if (currentTime_ns - lastUpdateTime >= 8333333ULL) { // ~120 FPS update rate
                    // Use std::fmod for modulo - it's optimized and faster than loops
                    const double cyclePosition = std::fmod(elapsed_seconds, totalCycleDuration);
                    
                    if (cyclePosition < delayDuration) {
                        // Delay phase - no scrolling (3 second pause)
                        cachedScrollOffset = 0.0f;
                    } else if (cyclePosition < delayDuration + scrollDuration) {
                        // Scrolling phase - velocity-based movement
                        const double scrollTime = cyclePosition - delayDuration;
                        double distance;
                        
                        if (scrollTime <= accelTime) {
                            // Acceleration phase - quadratic ease-in
                            const double t = scrollTime * invAccelTime;
                            const double smoothT = t * t;
                            distance = smoothT * accelDistance;
                        } else if (scrollTime <= accelTime + constantVelocityTime) {
                            // Constant velocity phase
                            const double constantTime = scrollTime - accelTime;
                            distance = accelDistance + (constantTime * maxVelocity);
                        } else {
                            // Deceleration phase - quadratic ease-out
                            const double decelStartTime = accelTime + constantVelocityTime;
                            const double t = (scrollTime - decelStartTime) * invDecelTime;
                            const double oneMinusT = 1.0 - t;
                            const double smoothT = 1.0 - oneMinusT * oneMinusT;
                            distance = accelDistance + constantVelocityDistance + (smoothT * (minScrollDistance - accelDistance - constantVelocityDistance));
                        }
                        
                        // Use branchless min
                        cachedScrollOffset = static_cast<float>(distance < minScrollDistance ? distance : minScrollDistance);
                    } else {
                        // Pause phase - stay at end (3 second pause)
                        cachedScrollOffset = static_cast<float>(m_textWidth);
                    }
                    
                    lastUpdateTime = currentTime_ns;
                }
                
                // Use cached value for consistent display
                m_scrollOffset = cachedScrollOffset;
                
                // Reset timer when cycle completes
                if (elapsed_seconds >= totalCycleDuration) {
                    timeIn_ns = currentTime_ns;
                }
            }
            
            //void resetTextProperties() {
            //    m_scrollText.clear();
            //    m_ellipsisText.clear();
            //    m_maxWidth = 0;
            //}
        };
        

        /**
         * @brief A customizable analog trackbar going from 0% to 100% (like the brightness slider)
         *
         */
        class TrackBar : public Element {
        public:
            /**
             * @brief Constructor
             *
             * @param icon Icon shown next to the track bar
             * @param usingStepTrackbar Whether this is a step trackbar
             * @param usingNamedStepTrackbar Whether this is a named step trackbar
             * @param useV2Style Whether to use V2 visual style (label + value instead of icon)
             * @param label Label text for V2 style
             * @param units Units text for V2 style
             */
            TrackBar(const char icon[3], bool usingStepTrackbar=false, bool usingNamedStepTrackbar = false, 
                    bool useV2Style = false, const std::string& label = "", const std::string& units = "") 
                : m_icon(icon), m_usingStepTrackbar(usingStepTrackbar), m_usingNamedStepTrackbar(usingNamedStepTrackbar),
                  m_useV2Style(useV2Style), m_label(label), m_units(units) {
                m_isItem = true;
            }

            virtual ~TrackBar() {}

            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) {
                return this;
            }

            virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                static s16 lastHapticSegment = -1;
                
                if (keysHeld & KEY_LEFT && keysHeld & KEY_RIGHT)
                    return true;
                
                if (keysHeld & KEY_LEFT) {
                    if (this->m_value > 0) {
                        this->m_value--;
                        this->m_valueChangedListener(this->m_value);
                        
                        // Calculate current segment (0-10 for 11 segments)
                        const s16 currentSegment = (this->m_value * 10) / 100;
                        if (this->m_value == 0 || currentSegment != lastHapticSegment) {
                            lastHapticSegment = currentSegment;
                            triggerNavigationFeedback();
                        }
                        
                        return true;
                    }
                }
                
                if (keysHeld & KEY_RIGHT) {
                    if (this->m_value < 100) {
                        this->m_value++;
                        this->m_valueChangedListener(this->m_value);
                        
                        // Calculate current segment (0-10 for 11 segments)
                        const s16 currentSegment = (this->m_value * 10) / 100;
                        if (this->m_value == 0 || currentSegment != lastHapticSegment) {
                            lastHapticSegment = currentSegment;
                            triggerNavigationFeedback();
                        }
                        
                        return true;
                    }
                }
                
                return false;
            }
            
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                const u16 trackBarWidth = this->getWidth() - 95;
                const u16 handlePos = (trackBarWidth * (this->m_value - 0)) / (100 - 0);
                const s32 circleCenterX = this->getX() + 59 + handlePos;
                const s32 circleCenterY = this->getY() + 40 + 16 - 1;
                static constexpr s32 circleRadius = 16;
                static bool triggerOnce = true;
                static s16 lastHapticSegment = -1;
                const bool touchInCircle = (std::abs(initialX - circleCenterX) <= circleRadius) && (std::abs(initialY - circleCenterY) <= circleRadius);
                
                if (event == TouchEvent::Release) {
                    triggerOnce = true;
                    lastHapticSegment = -1; // Reset for next touch
                    triggerRumbleDoubleClick.store(true, std::memory_order_release);
                    triggerOffSound.store(true, std::memory_order_release);
                    touchInSliderBounds = false;
                    return false;
                }
            
                if (touchInCircle || touchInSliderBounds) {
                    if (triggerOnce){
                        triggerOnce = false;
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerOnSound.store(true, std::memory_order_release);
                    }
                    touchInSliderBounds = true;
                    
                    s16 newValue = (static_cast<float>(currX - (this->getX() + 60)) / static_cast<float>(this->getWidth() - 95)) * 100;
                    
                    if (newValue < 0) {
                        newValue = 0;
                    } else if (newValue > 100) {
                        newValue = 100;
                    }
            
                    if (newValue != this->m_value) {
                        this->m_value = newValue;
                        this->m_valueChangedListener(this->getProgress());
                        
                        // Calculate which 10% segment we're in (0-10 for 11 segments)
                        const s16 currentSegment = (newValue * 10) / 100;
                        
                        // Trigger haptics when crossing into a new 10% segment OR at value 0
                        if (newValue == 0 || currentSegment != lastHapticSegment) {
                            lastHapticSegment = currentSegment;
                            triggerNavigationFeedback();
                        }
                    }
            
                    return true;
                }
            
                return false;
            }

            // Define drawBar function outside the draw method
            void drawBar(gfx::Renderer *renderer, s32 x, s32 y, u16 width, Color& color, bool isRounded = true) {
                if (isRounded) {
                    renderer->drawUniformRoundedRect(x, y, width, 7, a(color));
                } else {
                    renderer->drawRect(x, y, width, 7, a(color));
                }
            }

            virtual void draw(gfx::Renderer *renderer) override {
                //static float lastBottomBound;
                
                if (touchInSliderBounds) {
                    m_drawFrameless = true;
                    drawHighlight(renderer);
                } else {
                    m_drawFrameless = false;
                }


                s32 xPos = this->getX() + 59;
                s32 yPos = this->getY() + 40 + 16 - 1;
                s32 width = this->getWidth() - 95;
                u16 handlePos = width * (this->m_value) / (100);

                if (!m_usingNamedStepTrackbar) {
                    yPos -= 11;
                }

                s32 iconOffset = 0;

                if (!m_useV2Style && m_icon[0] != '\0') {
                    s32 iconWidth = 23;//tsl::gfx::calculateStringWidth(m_icon, 23);
                    iconOffset = 14 + iconWidth;
                    xPos += iconOffset;
                    width -= iconOffset;
                    handlePos = (width) * (this->m_value) / (100);
                }

                // Draw step tick marks if this is a step trackbar
                if (m_usingStepTrackbar || m_usingNamedStepTrackbar) {
                    const u8 numSteps = m_numSteps;
                    const u16 baseX = xPos;
                    const u16 baseY = this->getY() + 44;
                    const u8 halfNumSteps = (numSteps - 1) / 2;
                    const u16 lastStepX = baseX + width - 1;
                    const float stepSpacing = static_cast<float>(width) / (numSteps - 1);
                    const auto stepColor = a(trackBarEmptyColor);
                    
                    u16 stepX;
                    for (u8 i = 0; i < numSteps; i++) {
                        if (i == numSteps - 1) {
                            stepX = lastStepX;
                        } else {
                            stepX = baseX + static_cast<u16>(std::round(i * stepSpacing));
                            if (i > halfNumSteps) {
                                stepX -= 1;
                            }
                        }
                        renderer->drawRect(stepX, baseY, 1, 8, stepColor);
                    }
                }

                // Draw track bar background
                drawBar(renderer, xPos, yPos-3, width, trackBarEmptyColor, !m_usingNamedStepTrackbar);

                if (!this->m_focused) {
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + handlePos, yPos, 16, true, a(m_drawFrameless ? highlightColor : trackBarSliderBorderColor));
                    renderer->drawCircle(xPos + handlePos, yPos, 13, true, a((m_unlockedTrackbar || touchInSliderBounds) ? trackBarSliderMalleableColor : trackBarSliderColor));
                } else {
                    touchInSliderBounds = false;
                    if (m_unlockedTrackbar != ult::unlockedSlide.load(std::memory_order_acquire))
                        ult::unlockedSlide.store(m_unlockedTrackbar, std::memory_order_release);
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 16, true, a(highlightColor));
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 12, true, a((ult::allowSlide.load(std::memory_order_acquire) || m_unlockedTrackbar) ? trackBarSliderMalleableColor : trackBarSliderColor));
                }

                // Draw icon (original style) or label + value (V2 style)
                if (m_useV2Style) {
                    // V2 Style: Draw label and value
                    std::string labelPart = this->m_label;
                    ult::removeTag(labelPart);
                
                    std::string valuePart;
                    if (!m_usingNamedStepTrackbar) {
                        valuePart = (m_units.compare("%") == 0 || m_units.compare("°C") == 0 || m_units.compare("°F") == 0)
                                    ? ult::to_string(m_value) + m_units
                                    : ult::to_string(m_value) + (m_units.empty() ? "" : " ") + m_units;
                    } else {
                        valuePart = this->m_selection;
                    }
                
                    const auto valueWidth = renderer->getTextDimensions(valuePart, false, 16).first;
                
                    renderer->drawString(labelPart, false, this->getX() + 59, this->getY() + 14 + 16, 16, 
                                       ((!this->m_focused || !ult::useSelectionText) ? defaultTextColor : selectedTextColor));

                    renderer->drawString(valuePart, false, this->getWidth() -17 - valueWidth, this->getY() + 14 + 16, 16, (this->m_focused && ult::useSelectionValue) ? selectedValueTextColor : onTextColor);
                } else {
                    // Original Style: Draw icon
                    if (m_icon[0] != '\0')
                        renderer->drawString(this->m_icon, false, this->getX()+42, this->getY() + 50+2, 23, tsl::style::color::ColorText);
                }


                if (m_lastBottomBound != this->getTopBound())
                    renderer->drawRect(this->getX() + 4+20-1, this->getTopBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                renderer->drawRect(this->getX() + 4+20-1, this->getBottomBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                m_lastBottomBound = this->getBottomBound();
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX() - 16 , this->getY(), this->getWidth()+20+4, tsl::style::TrackBarDefaultHeight );
            }

            virtual void drawFocusBackground(gfx::Renderer *renderer) {
                // No background drawn here in HOS
            }

            virtual void drawHighlight(gfx::Renderer *renderer) override {
                
                // Get current time using ARM system tick for animation timing
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                
                // High precision time calculation - matches standard cosine wave timing
                const double time_seconds = static_cast<double>(currentTime_ns) / 1000000000.0;
                
                // Standard cosine wave calculation with high precision
                progress = (ult::cos(2.0 * ult::M_PI * std::fmod(time_seconds, 1.0) - ult::M_PI / 2) + 1.0) / 2.0;
            
                // High precision floating point color interpolation
                highlightColor = {
                    static_cast<u8>(highlightColor2.r + (highlightColor1.r - highlightColor2.r) * progress + 0.5),
                    static_cast<u8>(highlightColor2.g + (highlightColor1.g - highlightColor2.g) * progress + 0.5),
                    static_cast<u8>(highlightColor2.b + (highlightColor1.b - highlightColor2.b) * progress + 0.5),
                    0xF
                };
                
                // Initialize position offsets
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime;
                    const double t_ms = t_ns / 1000000.0;
                    
                    static constexpr double SHAKE_DURATION_MS = 200.0;
                    
                    if (t_ms >= SHAKE_DURATION_MS)
                        this->m_highlightShaking = false;
                    else {
                        // Generate random amplitude only once per shake using the start time as seed
                        const double amplitude = 6.0 + ((this->m_highlightShakingStartTime / 1000000) % 5);
                        const double progress = t_ms / SHAKE_DURATION_MS; // 0 to 1
                        
                        // Lighter damping so both bounces are visible
                        const double damping = 1.0 / (1.0 + 2.5 * progress * (1.0 + 1.3 * progress));
                        
                        // 2 full oscillations = 2 clear bounces
                        const double oscillation = ult::cos(ult::M_PI * 4.0 * progress);
                        const double displacement = amplitude * oscillation * damping;
                        const int offset = static_cast<int>(displacement);
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:    y = -offset; break;
                            case FocusDirection::Down:  y = offset; break;
                            case FocusDirection::Left:  x = -offset; break;
                            case FocusDirection::Right: x = offset; break;
                            default: break;
                        }
                    }
                }
                
                if (!m_drawFrameless) {
                    if (ult::useSelectionBG) {
                        if (ult::expandedMemory)
                            renderer->drawRectMultiThreaded(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(selectionBGColor)); // CUSTOM MODIFICATION 
                        else
                            renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(selectionBGColor)); // CUSTOM MODIFICATION 


                        //renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), a(selectionBGColor)); // CUSTOM MODIFICATION 
                    }
                    
                    renderer->drawBorderedRoundedRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11, this->getHeight(), 5, 5, a(highlightColor));
                } else {
                    if (ult::useSelectionBG) {
                        if (ult::expandedMemory)
                            renderer->drawRectMultiThreaded(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(clickColor)); // CUSTOM MODIFICATION 
                        else
                            renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(clickColor)); // CUSTOM MODIFICATION 
                    }
                }


                ult::onTrackBar.exchange(true, std::memory_order_acq_rel);
            }

            /**
             * @brief Gets the current value of the trackbar
             *
             * @return State
             */
            virtual inline u8 getProgress() {
                return this->m_value;
            }

            /**
             * @brief Sets the current state of the toggle. Updates the Value
             *
             * @param state State
             */
            virtual void setProgress(u8 value) {
                this->m_value = value;
            }

            /**
             * @brief Adds a listener that gets called whenever the state of the toggle changes
             *
             * @param stateChangedListener Listener with the current state passed in as parameter
             */
            void setValueChangedListener(std::function<void(u8)> valueChangedListener) {
                this->m_valueChangedListener = valueChangedListener;
            }

        protected:
            const char *m_icon = nullptr;
            s16 m_value = 0;
            bool m_interactionLocked = false;

            std::function<void(u8)> m_valueChangedListener = [](u8){};

            bool m_usingStepTrackbar = false;
            bool m_usingNamedStepTrackbar = false;
            bool m_unlockedTrackbar = true;
            bool touchInSliderBounds = false;
            
            u8 m_numSteps = 101;
            // V2 Style properties
            bool m_useV2Style = false;
            std::string m_label;
            std::string m_units;
            std::string m_selection; // Used for named step trackbars
            bool m_drawFrameless = false;

            float m_lastBottomBound;
        };


        /**
         * @brief A customizable analog trackbar going from 0% to 100% but using discrete steps (Like the volume slider)
         *
         */
        class StepTrackBar : public TrackBar {
        public:
            /**
             * @brief Constructor
             *
             * @param icon Icon shown next to the track bar
             * @param numSteps Number of steps the track bar has
             * @param usingNamedStepTrackbar Whether this is a named step trackbar
             * @param useV2Style Whether to use V2 visual style (label + value instead of icon)
             * @param label Label text for V2 style
             * @param units Units text for V2 style
             */
            StepTrackBar(const char icon[3], size_t numSteps, bool usingNamedStepTrackbar = false,
                        bool useV2Style = false, const std::string& label = "", const std::string& units = "")
                : TrackBar(icon, true, usingNamedStepTrackbar, useV2Style, label, units), m_numSteps(numSteps) { }

            virtual ~StepTrackBar() {}

            virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                static u32 tick = 0;

                if (keysHeld & KEY_LEFT && keysHeld & KEY_RIGHT) {
                    tick = 0;
                    return true;
                }

                if (keysHeld & (KEY_LEFT | KEY_RIGHT)) {
                    if ((tick == 0 || tick > 20) && (tick % 3) == 0) {
                        if (keysHeld & KEY_LEFT && this->m_value > 0) {
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                            this->m_value = std::max(this->m_value - (100 / (this->m_numSteps - 1)), 0);
                        } else if (keysHeld & KEY_RIGHT && this->m_value < 100) {
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerNavigationSound.store(true, std::memory_order_release);
                            triggerNavigationFeedback();
                            this->m_value = std::min(this->m_value + (100 / (this->m_numSteps - 1)), 100);
                        } else {
                            return false;
                        }
                        this->m_valueChangedListener(this->getProgress());
                    }
                    tick++;
                    return true;
                } else {
                    tick = 0;
                }

                return false;
            }

            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                const u16 trackBarWidth = this->getWidth() - 95;
                const u16 handlePos = (trackBarWidth * this->m_value) / 100;
                const s32 circleCenterX = this->getX() + 59 + handlePos;
                const s32 circleCenterY = this->getY() + 40 + 16 - 1;
                static constexpr s32 circleRadius = 16;
                static bool triggerOnce = true;
                
                const bool touchInCircle = (std::abs(initialX - circleCenterX) <= circleRadius) && (std::abs(initialY - circleCenterY) <= circleRadius);
                
                if (event == TouchEvent::Release) {
                    triggerOnce = true;
                    triggerRumbleDoubleClick.store(true, std::memory_order_release);
                    triggerOffSound.store(true, std::memory_order_release);
                    touchInSliderBounds = false;
                    return false;
                }
            
                if (touchInCircle || touchInSliderBounds) {
                    if (triggerOnce){
                        triggerOnce = false;
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerOnSound.store(true, std::memory_order_release);
                    }
        
                    touchInSliderBounds = true;
                    
                    // Add 0.5 for rounding to nearest step instead of truncating
                    float rawValue = (static_cast<float>(currX - (this->getX() + 60)) / static_cast<float>(this->getWidth() - 95)) * 100;
                    s16 newValue;
        
                    if (rawValue < 0) {
                        newValue = 0;
                    } else if (rawValue > 100) {
                        newValue = 100;
                    } else {
                        // Round to nearest step with 0.5 offset for proper snapping
                        newValue = std::round((rawValue + 0.5f) / (100.0F / (this->m_numSteps - 1))) * (100.0F / (this->m_numSteps - 1));
                        // Clamp after rounding
                        newValue = std::min(std::max(newValue, s16(0)), s16(100));
                    }
        
                    if (newValue != this->m_value) {
                        triggerNavigationFeedback();
                        this->m_value = newValue;
                        this->m_valueChangedListener(this->getProgress());
                    }
        
                    return true;
                }
            
                return false;
            }

            /**
             * @brief Gets the current value of the trackbar
             *
             * @return State
             */
            virtual inline u8 getProgress() override {
                return this->m_value / (100 / (this->m_numSteps - 1));
            }

            /**
             * @brief Sets the current state of the toggle. Updates the Value
             *
             * @param state State
             */
            virtual void setProgress(u8 value) override {
                value = std::min(value, u8(this->m_numSteps - 1));
                this->m_value = value * (100 / (this->m_numSteps - 1));
            }

        protected:
            u8 m_numSteps = 1;
        };


        /**
         * @brief A customizable trackbar with multiple discrete steps with specific names. Name gets displayed above the bar
         *
         */
        class NamedStepTrackBar : public StepTrackBar {
        public:
            /**
             * @brief Constructor
             *
             * @param icon Icon shown next to the track bar
             * @param stepDescriptions Step names displayed above the track bar
             * @param useV2Style Whether to use V2 visual style (label + value instead of icon)
             * @param label Label text for V2 style
             */
            NamedStepTrackBar(const char icon[3], std::initializer_list<std::string> stepDescriptions,
                             bool useV2Style = false, const std::string& label = "")
                : StepTrackBar(icon, stepDescriptions.size(), true, useV2Style, label, ""), 
                  m_stepDescriptions(stepDescriptions.begin(), stepDescriptions.end()) {
                this->m_usingNamedStepTrackbar = true;
                // Initialize selection with first step
                if (!m_stepDescriptions.empty()) {
                    this->m_selection = m_stepDescriptions[0];
                }
                m_numSteps = m_stepDescriptions.size();
            }

            virtual ~NamedStepTrackBar() {}

            virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                // Store previous value to update selection
                const u8 prevProgress = this->getProgress();
                
                // Call parent input handling
                const bool result = StepTrackBar::handleInput(keysDown, keysHeld, touchPos, leftJoyStick, rightJoyStick);
                
                // Update selection if progress changed
                if (result && this->getProgress() != prevProgress) {
                    const u8 currentIndex = this->getProgress();
                    if (currentIndex < m_stepDescriptions.size()) {
                        this->m_selection = m_stepDescriptions[currentIndex];
                    }
                }
                
                return result;
            }

            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                // Store previous value to update selection
                const u8 prevProgress = this->getProgress();
                
                // Call parent touch handling
                const bool result = StepTrackBar::onTouch(event, currX, currY, prevX, prevY, initialX, initialY);
                
                // Update selection if progress changed
                if (result && this->getProgress() != prevProgress) {
                    const u8 currentIndex = this->getProgress();
                    if (currentIndex < m_stepDescriptions.size()) {
                        this->m_selection = m_stepDescriptions[currentIndex];
                    }
                }
                
                return result;
            }

            virtual void setProgress(u8 value) override {
                StepTrackBar::setProgress(value);
                
                // Update selection when progress is set programmatically
                const u8 currentIndex = this->getProgress();
                if (currentIndex < m_stepDescriptions.size()) {
                    this->m_selection = m_stepDescriptions[currentIndex];
                }
            }

            virtual void draw(gfx::Renderer *renderer) override {
                if (touchInSliderBounds) {
                    m_drawFrameless = true;
                    drawHighlight(renderer);
                } else {
                    m_drawFrameless = false;
                }
            
                s32 xPos = this->getX() + 59;
                s32 yPos = this->getY() + 40 + 16 - 1;
                s32 width = this->getWidth() - 95;
                u16 handlePos = width * (this->m_value) / (100);
            
                if (!m_usingNamedStepTrackbar) {
                    yPos -= 11;
                }
            
                s32 iconOffset = 0;
            
                if (!m_useV2Style && m_icon[0] != '\0') {
                    s32 iconWidth = 23;
                    iconOffset = 14 + iconWidth;
                    xPos += iconOffset;
                    width -= iconOffset;
                    handlePos = (width) * (this->m_value) / (100);
                }
            
                // Draw step tick marks if this is a step trackbar
                {
                    const u8 numSteps = m_numSteps;
                    const u16 baseX = xPos;
                    const u16 baseY = this->getY() + 44;
                    const u8 halfNumSteps = (numSteps - 1) / 2;
                    const u16 lastStepX = baseX + width - 1;
                    const float stepSpacing = static_cast<float>(width) / (numSteps - 1);
                    const auto stepColor = a(trackBarEmptyColor);
                    
                    u16 stepX;
                    for (u8 i = 0; i < numSteps; i++) {
                        if (i == numSteps - 1) {
                            stepX = lastStepX;
                        } else {
                            stepX = baseX + static_cast<u16>(std::round(i * stepSpacing));
                            if (i > halfNumSteps) {
                                stepX -= 1;
                            }
                        }
                        renderer->drawRect(stepX, baseY, 1, 8, stepColor);
                    }
                }

                // Draw track bar background
                drawBar(renderer, xPos, yPos-3, width, trackBarEmptyColor, !m_usingNamedStepTrackbar);
            
                if (!this->m_focused) {
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + handlePos, yPos, 16, true, a(m_drawFrameless ? highlightColor : trackBarSliderBorderColor));
                    renderer->drawCircle(xPos + handlePos, yPos, 13, true, a((m_unlockedTrackbar || touchInSliderBounds) ? trackBarSliderMalleableColor : trackBarSliderColor));
                } else {
                    touchInSliderBounds = false;
                    if (m_unlockedTrackbar != ult::unlockedSlide.load(std::memory_order_acquire))
                        ult::unlockedSlide.store(m_unlockedTrackbar, std::memory_order_release);
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 16, true, a(highlightColor));
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 12, true, a((ult::allowSlide.load(std::memory_order_acquire) || m_unlockedTrackbar) ? trackBarSliderMalleableColor : trackBarSliderColor));
                }
            
                // Draw icon (original style) or label + value (V2 style)
                if (m_useV2Style) {
                    // V2 Style: Draw label and value
                    std::string labelPart = this->m_label;
                    ult::removeTag(labelPart);
                
                    std::string valuePart;
                    if (!m_usingNamedStepTrackbar) {
                        valuePart = (m_units.compare("%") == 0 || m_units.compare("°C") == 0 || m_units.compare("°F") == 0)
                                    ? ult::to_string(m_value) + m_units
                                    : ult::to_string(m_value) + (m_units.empty() ? "" : " ") + m_units;
                    } else {
                        valuePart = this->m_selection;
                    }
                
                    const auto valueWidth = renderer->getTextDimensions(valuePart, false, 16).first;
                
                    renderer->drawString(labelPart, false, this->getX() + 59, this->getY() + 14 + 16, 16, 
                                       ((!this->m_focused || !ult::useSelectionText) ? defaultTextColor : selectedTextColor));
            
                    renderer->drawString(valuePart, false, this->getWidth() -17 - valueWidth, this->getY() + 14 + 16, 16, (this->m_focused && ult::useSelectionValue) ? selectedValueTextColor : onTextColor);
                } else {
                    // Original Style: Draw icon
                    if (m_icon[0] != '\0')
                        renderer->drawString(this->m_icon, false, this->getX()+42, this->getY() + 50+2, 23, tsl::style::color::ColorText);
                }
            
                if (m_lastBottomBound != this->getTopBound())
                    renderer->drawRect(this->getX() + 4+20-1, this->getTopBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                renderer->drawRect(this->getX() + 4+20-1, this->getBottomBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                m_lastBottomBound = this->getBottomBound();
            }

        protected:
            std::vector<std::string> m_stepDescriptions;

        };



        /**
         * @brief A customizable analog trackbar going from minValue to maxValue
         *
         */
        class TrackBarV2 : public Element {
        public:
            using SimpleValueChangeCallback = std::function<void(s16 value, s16 index)>;

            u64 lastUpdate_ns;
        
            Color highlightColor = {0xf, 0xf, 0xf, 0xf};
            float progress;
            float counter = 0.0;
            s32 x, y;
            s32 amplitude;
            u32 descWidth, descHeight;
            
            void setScriptKeyListener(std::function<void()> listener) {
                m_scriptKeyListener = std::move(listener);
            }
        
            TrackBarV2(std::string label, std::string packagePath = "", s16 minValue = 0, s16 maxValue = 100, std::string units = "",
                     std::function<bool(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
                     std::function<std::vector<std::vector<std::string>>(const std::vector<std::vector<std::string>>&, const std::string&, size_t, const std::string&)> sourceReplacementFunc = nullptr,
                     std::vector<std::vector<std::string>> cmd = {}, const std::string& selCmd = "", bool usingStepTrackbar = false, bool usingNamedStepTrackbar = false, s16 numSteps = -1, bool unlockedTrackbar = false, bool executeOnEveryTick = false)
                : m_label(label), m_packagePath(packagePath), m_minValue(minValue), m_maxValue(maxValue), m_units(units),
                  interpretAndExecuteCommands(executeCommands), getSourceReplacement(sourceReplacementFunc), commands(std::move(cmd)), selectedCommand(selCmd),
                  m_usingStepTrackbar(usingStepTrackbar), m_usingNamedStepTrackbar(usingNamedStepTrackbar), m_numSteps(numSteps), m_unlockedTrackbar(unlockedTrackbar), m_executeOnEveryTick(executeOnEveryTick) {
                
                m_isItem = true;
            
                if (maxValue < minValue) {
                    std::swap(minValue, maxValue);
                    m_minValue = minValue;
                    m_maxValue = maxValue;
                }
            
                if ((!usingStepTrackbar && !usingNamedStepTrackbar) || numSteps == -1) {
                    m_numSteps = (maxValue - minValue) + 1;
                }
                
                if (m_numSteps < 2) {
                    m_numSteps = 2;
                }
            
                bool loadedValue = false;
                
                if (!m_packagePath.empty()) {
                    auto configIniData = ult::getParsedDataFromIniFile(m_packagePath + "config.ini");
                    auto sectionIt = configIniData.find(m_label);
                    
                    if (sectionIt != configIniData.end()) {
                        auto indexIt = sectionIt->second.find("index");
                        if (indexIt != sectionIt->second.end() && !indexIt->second.empty()) {
                            m_index = static_cast<s16>(ult::stoi(indexIt->second));
                        }
                        
                        if (!m_usingNamedStepTrackbar) {
                            auto valueIt = sectionIt->second.find("value");
                            if (valueIt != sectionIt->second.end() && !valueIt->second.empty()) {
                                m_value = static_cast<s16>(ult::stoi(valueIt->second));
                                loadedValue = true;
                            }
                        }
                    }
                }
            
                if (m_index >= m_numSteps) m_index = m_numSteps - 1;
                if (m_index < 0) m_index = 0;
            
                if (!loadedValue) {
                    if (m_numSteps > 1) {
                        m_value = minValue + m_index * (static_cast<float>(maxValue - minValue) / (m_numSteps - 1));
                    } else {
                        m_value = minValue;
                    }
                }
                
                if (m_value > maxValue) m_value = maxValue;
                if (m_value < minValue) m_value = minValue;
            
                lastUpdate_ns = armTicksToNs(armGetSystemTick());
            }
            
            virtual ~TrackBarV2() {}
            
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) {
                return this;
            }
        
            inline void updateAndExecute(bool updateIni = true) {
                if (m_simpleCallback) {
                    m_simpleCallback(m_value, m_index);
                    return;
                }


                if (m_packagePath.empty()) {
                    return;
                }
            
                const std::string indexStr = ult::to_string(m_index);
                const std::string valueStr = m_usingNamedStepTrackbar ? m_selection : ult::to_string(m_value);
            
                if (updateIni) {
                    const std::string configPath = m_packagePath + "config.ini";
                    ult::setIniFileValue(configPath, m_label, "index", indexStr);
                    ult::setIniFileValue(configPath, m_label, "value", valueStr);
                }
                bool success = false;
            
                static const std::string valuePlaceholder = "{value}";
                static const std::string indexPlaceholder = "{index}";
                static const size_t valuePlaceholderLen = valuePlaceholder.length();
                static const size_t indexPlaceholderLen = indexPlaceholder.length();
                const size_t valueStrLen = valueStr.length();
                const size_t indexStrLen = indexStr.length();
                
                size_t tryCount = 0;
                while (!success) {
                    if (interpretAndExecuteCommands) {
                        if (tryCount > 3)
                            break;
                        auto modifiedCmds = getSourceReplacement(commands, valueStr, m_index, m_packagePath);
                        
                        for (auto& cmd : modifiedCmds) {
                            for (auto& arg : cmd) {
                                for (size_t pos = 0; (pos = arg.find(valuePlaceholder, pos)) != std::string::npos; pos += valueStrLen) {
                                    arg.replace(pos, valuePlaceholderLen, valueStr);
                                }
                                
                                if (m_usingNamedStepTrackbar) {
                                    for (size_t pos = 0; (pos = arg.find(indexPlaceholder, pos)) != std::string::npos; pos += indexStrLen) {
                                        arg.replace(pos, indexPlaceholderLen, indexStr);
                                    }
                                }
                            }
                        }
                        
                        success = interpretAndExecuteCommands(std::move(modifiedCmds), m_packagePath, selectedCommand);
                        ult::resetPercentages();
            
                        if (success)
                            break;
                        tryCount++;
                    }
                }
            }
            
            virtual inline bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                const u64 keysReleased = m_prevKeysHeld & ~keysHeld;
                m_prevKeysHeld = keysHeld;
                
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const u64 elapsed_ns = currentTime_ns - lastUpdate_ns;
            
                m_keyRHeld = (keysHeld & KEY_R) != 0;
            
                if ((keysHeld & KEY_R)) {
                    if (keysDown & KEY_UP && !(keysHeld & ~KEY_UP & ~KEY_R & ALL_KEYS_MASK))
                        this->shakeHighlight(FocusDirection::Up);
                    else if (keysDown & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ~KEY_R & ALL_KEYS_MASK))
                        this->shakeHighlight(FocusDirection::Down);
                    else if (keysDown & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ~KEY_R & ALL_KEYS_MASK)){
                        this->shakeHighlight(FocusDirection::Left);
                    }
                    else if (keysDown & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK)) {
                        this->shakeHighlight(FocusDirection::Right);
                    }
                    return true;
                }
            
                if ((keysDown & KEY_A) && !(keysHeld & ~KEY_A & ALL_KEYS_MASK)) {
                    triggerEnterFeedback();
                    
                    if (!m_unlockedTrackbar) {
                        ult::atomicToggle(ult::allowSlide);
                        m_holding = false;
                    }
                    if (m_unlockedTrackbar || (!m_unlockedTrackbar && !ult::allowSlide.load(std::memory_order_acquire))) {
                        updateAndExecute();
                        triggerClick = true;
                    }
                    return true;
                }
            
                if ((keysDown & SCRIPT_KEY) && !(keysHeld & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                    if (m_scriptKeyListener) {
                        m_scriptKeyListener();
                    }
                    return true;
                }
            
                if (ult::allowSlide.load(std::memory_order_acquire) || m_unlockedTrackbar) {
                    static s16 lastHapticSegment = -1;
                    
                    // Handle key release
                    if (((keysReleased & KEY_LEFT) || (keysReleased & KEY_RIGHT))) {
                        lastHapticSegment = -1; // Reset for next interaction
                        
                        // If we were holding and repeating, just stop
                        if (m_wasLastHeld) {
                            m_wasLastHeld = false;
                            m_holding = false;
                            updateAndExecute();
                            lastUpdate_ns = armTicksToNs(armGetSystemTick());
                            return true;
                        }
                        // If it was a quick tap (no repeat happened), handle the single tick
                        else if (m_holding) {
                            m_holding = false;
                            updateAndExecute();
                            lastUpdate_ns = armTicksToNs(armGetSystemTick());
                            return true;
                        }
                    }
                    
                    // Ignore simultaneous left+right
                    if (keysDown & KEY_LEFT && keysDown & KEY_RIGHT)
                        return true;
                    if (keysHeld & KEY_LEFT && keysHeld & KEY_RIGHT)
                        return true;
            
                    // Handle initial key press
                    if (keysDown & KEY_LEFT || keysDown & KEY_RIGHT) {
                        triggerRumbleClick.store(true, std::memory_order_release);
                        
                        // Start tracking the hold
                        m_holding = true;
                        m_wasLastHeld = false;
                        m_holdStartTime_ns = armTicksToNs(armGetSystemTick());
                        lastUpdate_ns = currentTime_ns;
                        
                        // Perform the initial single tick
                        if (keysDown & KEY_LEFT && this->m_value > m_minValue) {
                            this->m_index--;
                            this->m_value--;
                            this->m_valueChangedListener(this->m_value);
                            updateAndExecute(false);
                            
                            // Calculate and store initial segment (0-10 for 11 segments)
                            const s16 currentSegment = (this->m_index * 10) / (m_numSteps - 1);
                            if (this->m_index == 0 || currentSegment != lastHapticSegment) {
                                lastHapticSegment = currentSegment;
                                triggerNavigationFeedback();
                            }
                        } else if (keysDown & KEY_RIGHT && this->m_value < m_maxValue) {
                            this->m_index++;
                            this->m_value++;
                            this->m_valueChangedListener(this->m_value);
                            updateAndExecute(false);
                            
                            // Calculate and store initial segment (0-10 for 11 segments)
                            const s16 currentSegment = (this->m_index * 10) / (m_numSteps - 1);
                            if (this->m_index == 0 || currentSegment != lastHapticSegment) {
                                lastHapticSegment = currentSegment;
                                triggerNavigationFeedback();
                            }
                        }
                        return true;
                    }
                    
                    // Handle continued holding (after initial press)
                    if (m_holding && ((keysHeld & KEY_LEFT) || (keysHeld & KEY_RIGHT))) {
                        const u64 holdDuration_ns = currentTime_ns - m_holdStartTime_ns;
            
                        // Initial delay before repeating starts (e.g., 300ms)
                        static constexpr u64 initialDelay_ns = 300000000ULL;
                        // Calculate interval with acceleration
                        static constexpr u64 initialInterval_ns = 67000000ULL;  // ~67ms
                        static constexpr u64 shortInterval_ns = 10000000ULL;    // ~10ms
                        static constexpr u64 transitionPoint_ns = 1000000000ULL; // 1 second
            
                        // If we haven't passed the initial delay, don't repeat yet
                        if (holdDuration_ns < initialDelay_ns) {
                            return true;
                        }
                        
                        const u64 holdDurationAfterDelay_ns = holdDuration_ns - initialDelay_ns;
                        const float t = std::min(1.0f, static_cast<float>(holdDurationAfterDelay_ns) / static_cast<float>(transitionPoint_ns));
                        const u64 currentInterval_ns = static_cast<u64>((initialInterval_ns - shortInterval_ns) * (1.0f - t) + shortInterval_ns);
                        
                        if (elapsed_ns >= currentInterval_ns) {
                            if (keysHeld & KEY_LEFT && this->m_value > m_minValue) {
                                this->m_index--;
                                this->m_value--;
                                this->m_valueChangedListener(this->m_value);
                                if (m_executeOnEveryTick) {
                                    updateAndExecute(false);
                                }
                                
                                // Calculate current segment (0-10 for 11 segments) and trigger haptics on segment change
                                const s16 currentSegment = (this->m_index * 10) / (m_numSteps - 1);
                                if (this->m_index == 0 || currentSegment != lastHapticSegment) {
                                    lastHapticSegment = currentSegment;
                                    triggerNavigationFeedback();
                                }
                                
                                lastUpdate_ns = currentTime_ns;
                                m_wasLastHeld = true;
                                return true;
                            }
                            
                            if (keysHeld & KEY_RIGHT && this->m_value < m_maxValue) {
                                this->m_index++;
                                this->m_value++;
                                this->m_valueChangedListener(this->m_value);
                                if (m_executeOnEveryTick) {
                                    updateAndExecute(false);
                                }
                                
                                // Calculate current segment (0-10 for 11 segments) and trigger haptics on segment change
                                const s16 currentSegment = (this->m_index * 10) / (m_numSteps - 1);
                                if (this->m_index == 0 || currentSegment != lastHapticSegment) {
                                    lastHapticSegment = currentSegment;
                                    triggerNavigationFeedback();
                                }
                                
                                lastUpdate_ns = currentTime_ns;
                                m_wasLastHeld = true;
                                return true;
                            }
                        }
                    } else {
                        m_holding = false;
                    }
                }
                
                return false;
            }
                        
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                const u16 trackBarWidth = this->getWidth() - 95;
                const u16 handlePos = (trackBarWidth * (this->m_value - m_minValue)) / (m_maxValue - m_minValue);
                const s32 circleCenterX = this->getX() + 59 + handlePos;
                const s32 circleCenterY = this->getY() + 40 + 16 - 1;
                static constexpr s32 circleRadius = 16;
                static bool triggerOnce = true;
                static s16 lastHapticSegment = -1;
                static bool wasOriginallyLocked = false;
                
                const bool touchInCircle = (std::abs(initialX - circleCenterX) <= circleRadius) && (std::abs(initialY - circleCenterY) <= circleRadius);
                
                // Handle touch start
                if (event == TouchEvent::Touch && touchInCircle) {
                    // Remember if it was locked before we touched it
                    wasOriginallyLocked = !m_unlockedTrackbar && !ult::allowSlide.load(std::memory_order_acquire);
                    
                    // Temporarily unlock if it was locked
                    if (wasOriginallyLocked) {
                        ult::allowSlide.store(true, std::memory_order_release);
                    }
                }
                
                // Handle release
                if (event == TouchEvent::Release) {
                    triggerOnce = true;
                    lastHapticSegment = -1;
                    
                    // Re-lock if it was originally locked
                    if (wasOriginallyLocked) {
                        ult::allowSlide.store(false, std::memory_order_release);
                        wasOriginallyLocked = false;
                    }
                    
                    if (touchInSliderBounds) {
                        updateAndExecute();
                        touchInSliderBounds = false;
                        triggerRumbleDoubleClick.store(true, std::memory_order_release);
                        triggerOffSound.store(true, std::memory_order_release);
                    }
                    return false;
                }
                
                const bool isUnlocked = m_unlockedTrackbar || ult::allowSlide.load(std::memory_order_acquire);
                
                if ((touchInCircle || touchInSliderBounds) && isUnlocked) {
                    touchInSliderBounds = true;
                    if (triggerOnce) {
                        triggerOnce = false;
                        triggerRumbleClick.store(true, std::memory_order_release);
                        triggerOnSound.store(true, std::memory_order_release);
                    }
                    
                    // Add 0.5 to round to nearest step instead of truncating
                    const s16 newIndex = std::max(static_cast<s16>(0), std::min(static_cast<s16>((currX - (this->getX() + 59)) / static_cast<float>(this->getWidth() - 95) * (m_numSteps - 1) + 0.5f), static_cast<s16>(m_numSteps - 1)));
                    const s16 newValue = m_minValue + newIndex * (static_cast<float>(m_maxValue - m_minValue) / (m_numSteps - 1));
                    
                    if (newValue != this->m_value || newIndex != this->m_index) {
                        this->m_value = newValue;
                        this->m_index = newIndex;
                        this->m_valueChangedListener(this->getProgress());
                        if (m_executeOnEveryTick) {
                            updateAndExecute(false);
                        }
                        
                        // Calculate which 10% segment we're in (0-10 for 11 segments)
                        const s16 currentSegment = (newIndex * 10) / (m_numSteps - 1);
                        
                        // Trigger haptics when crossing into a new 10% segment OR at index 0
                        if (newIndex == 0 || currentSegment != lastHapticSegment) {
                            lastHapticSegment = currentSegment;
                            triggerNavigationFeedback();
                        }
                    }
            
                    return true;
                }
                
                return false;
            }
                        
            void drawBar(gfx::Renderer *renderer, s32 x, s32 y, u16 width, Color& color, bool isRounded = true) {
                if (isRounded) {
                    renderer->drawUniformRoundedRect(x, y, width, 7, a(color));
                } else {
                    renderer->drawRect(x, y, width, 7, a(color));
                }
            }
        
            virtual void draw(gfx::Renderer *renderer) override {
                const u16 handlePos = (this->getWidth() - 95) * (this->m_value - m_minValue) / (m_maxValue - m_minValue);
                const s32 xPos = this->getX() + 59;
                const s32 yPos = this->getY() + 40 + 16 - 1;
                const s32 width = this->getWidth() - 95;
        
                const bool shouldAppearLocked = m_unlockedTrackbar && m_keyRHeld;
                const bool visuallyUnlocked = (m_unlockedTrackbar && !m_keyRHeld) || touchInSliderBounds;

                if (visuallyUnlocked && touchInSliderBounds) {
                    m_drawFrameless = true;
                    drawHighlight(renderer);
                } else {
                    m_drawFrameless = false;
                }

                drawBar(renderer, xPos, yPos-3, width, trackBarEmptyColor, !m_usingNamedStepTrackbar);
                
                
                if (!this->m_focused) {
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + handlePos, yPos, 16, true, a(!m_drawFrameless ? trackBarSliderBorderColor : highlightColor));
                    renderer->drawCircle(xPos + handlePos, yPos, 13, true, a(visuallyUnlocked ? trackBarSliderMalleableColor : trackBarSliderColor));
                } else {
                    touchInSliderBounds = false;
                    if (m_unlockedTrackbar != ult::unlockedSlide.load(std::memory_order_acquire))
                        ult::unlockedSlide.store(m_unlockedTrackbar, std::memory_order_release);
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 16, true, a(highlightColor));
                    const bool focusedVisuallyUnlocked = (ult::allowSlide.load(std::memory_order_acquire) || m_unlockedTrackbar) && !shouldAppearLocked;
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 12, true, a(focusedVisuallyUnlocked ? trackBarSliderMalleableColor : trackBarSliderColor));
                }
                 
                std::string labelPart = this->m_label;
                ult::removeTag(labelPart);
            
                if (!m_usingNamedStepTrackbar) {
                    m_valuePart = (this->m_units.compare("%") == 0 || this->m_units.compare("°C") == 0 || this->m_units.compare("°F") == 0) 
                                  ? ult::to_string(this->m_value) + this->m_units 
                                  : ult::to_string(this->m_value) + (this->m_units.empty() ? "" : " ") + this->m_units;
                } else
                    m_valuePart = this->m_selection;
            
                const auto valueWidth = renderer->getTextDimensions(m_valuePart, false, 16).first;
            
                renderer->drawString(labelPart, false, xPos, this->getY() + 14 + 16, 16, (!this->m_focused || !ult::useSelectionText) ? defaultTextColor : selectedTextColor);
                renderer->drawString(m_valuePart, false, this->getWidth() -17 - valueWidth, this->getY() + 14 + 16, 16, 
                    (this->m_focused && ult::useSelectionValue) ? selectedValueTextColor : onTextColor);
            
                if (m_lastBottomBound != this->getTopBound())
                    renderer->drawRect(this->getX() + 4+20-1, this->getTopBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                renderer->drawRect(this->getX() + 4+20-1, this->getBottomBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                m_lastBottomBound = this->getBottomBound();
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX() - 16 , this->getY(), this->getWidth()+20+4, tsl::style::TrackBarDefaultHeight );
            }
            
            virtual void drawFocusBackground(gfx::Renderer *renderer) {
            }
            
            virtual void drawHighlight(gfx::Renderer *renderer) override {
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const double timeInSeconds = static_cast<double>(currentTime_ns) / 1000000000.0;
                progress = ((ult::cos(2.0 * ult::M_PI * std::fmod(timeInSeconds, 1.0) - ult::M_PI / 2) + 1.0) / 2.0);
                
                Color clickColor1 = highlightColor1;
                Color clickColor2 = clickColor;
                
                if (triggerClick && !m_clickActive) {
                    m_clickStartTime_ns = currentTime_ns;
                    m_clickActive = true;
                    if (progress >= 0.5) {
                        clickColor1 = clickColor;
                        clickColor2 = highlightColor2;
                    }
                }
            
                if (m_lastLabel != m_label) {
                    m_clickActive = false;
                    triggerClick = false;
                }
                m_lastLabel = m_label;
            
                if (m_clickActive) {
                    const u64 elapsedTime_ns = currentTime_ns - m_clickStartTime_ns;
                    if (elapsedTime_ns < 500000000ULL) {
                        highlightColor = {
                            static_cast<u8>((clickColor1.r - clickColor2.r) * progress + clickColor2.r + 0.5),
                            static_cast<u8>((clickColor1.g - clickColor2.g) * progress + clickColor2.g + 0.5),
                            static_cast<u8>((clickColor1.b - clickColor2.b) * progress + clickColor2.b + 0.5),
                            0xF
                        };
                    } else {
                        m_clickActive = false;
                        triggerClick = false;
                    }
                } else {
                    const bool shouldAppearLocked = m_unlockedTrackbar && m_keyRHeld;
                    
                    if ((ult::allowSlide.load(std::memory_order_acquire) || m_unlockedTrackbar) && !shouldAppearLocked) {
                        highlightColor = {
                            static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress + highlightColor2.r + 0.5),
                            static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress + highlightColor2.g + 0.5),
                            static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress + highlightColor2.b + 0.5),
                            0xF
                        };
                    } else {
                        highlightColor = {
                            static_cast<u8>((highlightColor3.r - highlightColor4.r) * progress + highlightColor4.r + 0.5),
                            static_cast<u8>((highlightColor3.g - highlightColor4.g) * progress + highlightColor4.g + 0.5),
                            static_cast<u8>((highlightColor3.b - highlightColor4.b) * progress + highlightColor4.b + 0.5),
                            0xF
                        };
                    }
                }
                            
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime;
                    const double t_ms = t_ns / 1000000.0;
                    
                    static constexpr double SHAKE_DURATION_MS = 200.0;
                    
                    if (t_ms >= SHAKE_DURATION_MS)
                        this->m_highlightShaking = false;
                    else {
                        // Generate random amplitude only once per shake using the start time as seed
                        const double amplitude = 6.0 + ((this->m_highlightShakingStartTime / 1000000) % 5);
                        const double progress = t_ms / SHAKE_DURATION_MS; // 0 to 1
                        
                        // Lighter damping so both bounces are visible
                        const double damping = 1.0 / (1.0 + 2.5 * progress * (1.0 + 1.3 * progress));
                        
                        // 2 full oscillations = 2 clear bounces
                        const double oscillation = ult::cos(ult::M_PI * 4.0 * progress);
                        const double displacement = amplitude * oscillation * damping;
                        const int offset = static_cast<int>(displacement);
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:    y = -offset; break;
                            case FocusDirection::Down:  y = offset; break;
                            case FocusDirection::Left:  x = -offset; break;
                            case FocusDirection::Right: x = offset; break;
                            default: break;
                        }
                    }
                }
            
                
                if (!m_drawFrameless) {
                    if (ult::useSelectionBG) {
                        if (ult::expandedMemory)
                            renderer->drawRectMultiThreaded(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(selectionBGColor));
                        else
                            renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(selectionBGColor));
                    }

                    renderer->drawBorderedRoundedRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11, this->getHeight(), 5, 5, a(highlightColor));
                } else {
                    if (ult::useSelectionBG) {
                        if (ult::expandedMemory)
                            renderer->drawRectMultiThreaded(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(clickColor));
                        else
                            renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), aWithOpacity(clickColor));
                    }

                }
                
                ult::onTrackBar.store(true, std::memory_order_release);
            
                if (m_clickActive && m_useClickAnimation) {
                    const u64 elapsedTime_ns = currentTime_ns - m_clickStartTime_ns;
            
                    auto clickAnimationProgress = tsl::style::ListItemHighlightLength * (1.0f - (static_cast<float>(elapsedTime_ns) / 500000000.0f));
                    
                    if (clickAnimationProgress < 0.0f) {
                        clickAnimationProgress = 0.0f;
                    }
                
                    if (clickAnimationProgress > 0.0f) {
                        const u8 saturation = tsl::style::ListItemHighlightSaturation * (float(clickAnimationProgress) / float(tsl::style::ListItemHighlightLength));
                
                        Color animColor = {0xF, 0xF, 0xF, 0xF};
                        if (invertBGClickColor) {
                            animColor.r = 15 - saturation;
                            animColor.g = 15 - saturation;
                            animColor.b = 15 - saturation;
                        } else {
                            animColor.r = saturation;
                            animColor.g = saturation;
                            animColor.b = saturation;
                        }
                        animColor.a = selectionBGColor.a;
                        renderer->drawRect(this->getX() +22, this->getY(), this->getWidth() -22, this->getHeight(), aWithOpacity(animColor));
                    }
                }
            }
            
            virtual inline u8 getProgress() {
                return this->m_value;
            }
            
            virtual void setProgress(u8 value) {
                this->m_value = value;
            }
            
            void setValueChangedListener(std::function<void(u8)> valueChangedListener) {
                this->m_valueChangedListener = valueChangedListener;
            }

            void setSimpleCallback(SimpleValueChangeCallback callback) {
                m_simpleCallback = std::move(callback);
            }
        
            inline void disableClickAnimation() {
                m_useClickAnimation = false;
            }

        protected:
            std::string m_label;
            std::string m_packagePath;
            std::string m_selection;
            s16 m_value = 0;
            s16 m_minValue = 0;
            s16 m_maxValue = 100;
            std::string m_units;
            bool m_interactionLocked = false;
            bool m_keyRHeld = false;
            
            std::function<void(u8)> m_valueChangedListener = [](u8) {};
        
            std::function<bool(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> interpretAndExecuteCommands;
            std::function<std::vector<std::vector<std::string>>(const std::vector<std::vector<std::string>>&, const std::string&, size_t, const std::string&)> getSourceReplacement;
            std::vector<std::vector<std::string>> commands;
            std::string selectedCommand;
        
            bool m_usingStepTrackbar = false;
            bool m_usingNamedStepTrackbar = false;
            s16 m_numSteps = 2;
            s16 m_index = 0;
            bool m_unlockedTrackbar = false;
            bool m_executeOnEveryTick = false;
            bool touchInSliderBounds = false;
            bool triggerClick = false;
            std::function<void()> m_scriptKeyListener;
            
            // Instance variables replacing static ones
            float m_lastBottomBound = 0.0f;
            std::string m_valuePart = "";
            u64 m_clickStartTime_ns = 0;
            bool m_clickActive = false;
            std::string m_lastLabel = "";
            bool m_holding = false;
            u64 m_holdStartTime_ns = 0;
            u64 m_prevKeysHeld = 0;
            bool m_wasLastHeld = false;
            bool m_drawFrameless = false;

            bool m_useClickAnimation = true;

            SimpleValueChangeCallback m_simpleCallback = nullptr;
        };
        
        
        /**
         * @brief A customizable analog trackbar going from 0% to 100% but using discrete steps (Like the volume slider)
         *
         */
        class StepTrackBarV2 : public TrackBarV2 {
        public:

            /**
             * @brief Constructor
             *
             * @param icon Icon shown next to the track bar
             * @param numSteps Number of steps the track bar has
             */
            StepTrackBarV2(std::string label, std::string packagePath, size_t numSteps, s16 minValue, s16 maxValue, std::string units,
                std::function<bool(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
                std::function<std::vector<std::vector<std::string>>(const std::vector<std::vector<std::string>>&, const std::string&, size_t, const std::string&)> sourceReplacementFunc = nullptr,
                std::vector<std::vector<std::string>> cmd = {}, const std::string& selCmd = "", bool usingNamedStepTrackbar = false, bool unlockedTrackbar = false, bool executeOnEveryTick = false)
                : TrackBarV2(label, packagePath, minValue, maxValue, units, executeCommands, sourceReplacementFunc, cmd, selCmd, !usingNamedStepTrackbar, usingNamedStepTrackbar, numSteps, unlockedTrackbar, executeOnEveryTick) {}
            
            virtual ~StepTrackBarV2() {}
            
            virtual inline bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                static u32 tick = 0;
                static bool holding = false;
                static u64 prevKeysHeld = 0;
                const u64 keysReleased = prevKeysHeld & ~keysHeld;
                prevKeysHeld = keysHeld;
            
                static bool wasLastHeld = false;
            
                // ADD THIS LINE: Update KEY_R state for visual appearance
                m_keyRHeld = (keysHeld & KEY_R) != 0;
            
                if ((keysHeld & KEY_R)) {
                    //auto currentFocus = currentGui->getFocusedElement();
                    if (keysDown & KEY_UP && !(keysHeld & ~KEY_UP & ~KEY_R & ALL_KEYS_MASK))
                        this->shakeHighlight(FocusDirection::Up);
                    else if (keysDown & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ~KEY_R & ALL_KEYS_MASK))
                        this->shakeHighlight(FocusDirection::Down);
                    else if (keysDown & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ~KEY_R & ALL_KEYS_MASK)){
                        this->shakeHighlight(FocusDirection::Left);
                    }
                    else if (keysDown & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ~KEY_R & ALL_KEYS_MASK)) {
                        this->shakeHighlight(FocusDirection::Right);
                    }
                    return true;
                }
            
                // Check if KEY_A is pressed to toggle ult::allowSlide
                if ((keysDown & KEY_A) && !(keysHeld & ~KEY_A & ALL_KEYS_MASK)) {
                    //triggerRumbleClick.store(true, std::memory_order_release);
                    //triggerEnterSound.store(true, std::memory_order_release);
                    triggerEnterFeedback();
                    

                    if (!m_unlockedTrackbar) {
                        ult::atomicToggle(ult::allowSlide);
                        holding = false; // Reset holding state when KEY_A is pressed
                    }
                    if (m_unlockedTrackbar || (!m_unlockedTrackbar && !ult::allowSlide.load(std::memory_order_acquire))) {
                        updateAndExecute();
                        triggerClick = true;
                    }
                    return true;
                }
            
                //if (keysDown & KEY_B && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)) {
                //    triggerRumbleDoubleClick.store(true, std::memory_order_release);
                //    triggerExitSound.store(true, std::memory_order_release);
                //}

                // Handle SCRIPT_KEY press
                if ((keysDown & SCRIPT_KEY) && !(keysHeld & ~SCRIPT_KEY & ALL_KEYS_MASK)) {
                    if (m_scriptKeyListener) {
                        m_scriptKeyListener();
                    }
                    return true;
                }
            
                if (ult::allowSlide.load(std::memory_order_acquire) || m_unlockedTrackbar) {
                    if (((keysReleased & KEY_LEFT) || (keysReleased & KEY_RIGHT)) ||
                        (wasLastHeld && !(keysHeld & (KEY_LEFT | KEY_RIGHT)))) {
                        updateAndExecute();
                        holding = false;
                        wasLastHeld = false;
                        tick = 0;
                        return true;
                    }
                    
                    if (keysHeld & KEY_LEFT && keysHeld & KEY_RIGHT) {
                        tick = 0;
                        return true;
                    }
                    
                    if (keysHeld & (KEY_LEFT | KEY_RIGHT)) {
                        if (!holding) {
                            holding = true;
                            tick = 0;
                        }
                        
                        if ((tick == 0 || tick > 20) && (tick % 3) == 0) {
                            const float stepSize = static_cast<float>(m_maxValue - m_minValue) / (this->m_numSteps - 1);
                            if (keysHeld & KEY_LEFT && this->m_index > 0) {
                                //triggerRumbleClick.store(true, std::memory_order_release);
                                //triggerNavigationSound.store(true, std::memory_order_release);
                                triggerNavigationFeedback();
                                
                                this->m_index--;
                                this->m_value = static_cast<s16>(std::round(m_minValue + m_index * stepSize));
                            } else if (keysHeld & KEY_RIGHT && this->m_index < this->m_numSteps-1) {
                                //triggerRumbleClick.store(true, std::memory_order_release);
                                //triggerNavigationSound.store(true, std::memory_order_release);
                                triggerNavigationFeedback();
                                
                                this->m_index++;
                                this->m_value = static_cast<s16>(std::round(m_minValue + m_index * stepSize));
                            } else {
                                return false;
                            }
                            this->m_valueChangedListener(this->getProgress());
                            if (m_executeOnEveryTick)
                                updateAndExecute(false);
                            wasLastHeld = true;
                        }
                        tick++;
                        return true;
                    } else {
                        holding = false;
                        tick = 0;
                    }
                }
                
                return false;
            }
            
            
            /**
             * @brief Gets the current value of the trackbar
             *
             * @return State
             */
            virtual inline u8 getProgress() override {
                return this->m_value / (100 / (this->m_numSteps - 1));
            }
            
            /**
             * @brief Sets the current state of the toggle. Updates the Value
             *
             * @param state State
             */
            virtual void setProgress(u8 value) override {
                value = std::min(value, u8(this->m_numSteps - 1));
                this->m_index = value;
                
                // If using simple callback (modern API), use minValue/maxValue range
                // Otherwise use legacy 0-100 range for config.ini compatibility
                if (m_simpleCallback) {
                    const float stepSize = static_cast<float>(m_maxValue - m_minValue) / (this->m_numSteps - 1);
                    this->m_value = static_cast<s16>(std::round(m_minValue + m_index * stepSize));
                } else {
                    // Legacy behavior for command system
                    this->m_value = value * (100 / (this->m_numSteps - 1));
                }
            }
            
        //protected:
            //u8 m_numSteps = 1;
            
        };
        
        
        /**
         * @brief A customizable trackbar with multiple discrete steps with specific names. Name gets displayed above the bar
         *
         */
        class NamedStepTrackBarV2 : public StepTrackBarV2 {
        public:
            u16 trackBarWidth, stepWidth, currentDescIndex;
            u32 descWidth, descHeight;
            
            /**
             * @brief Constructor
             *
             * @param icon Icon shown next to the track bar
             * @param stepDescriptions Step names displayed above the track bar
             */
            NamedStepTrackBarV2(std::string label, std::string packagePath, std::vector<std::string>& stepDescriptions,
                std::function<bool(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
                std::function<std::vector<std::vector<std::string>>(const std::vector<std::vector<std::string>>&, const std::string&, size_t, const std::string&)> sourceReplacementFunc = nullptr,
                std::vector<std::vector<std::string>> cmd = {}, const std::string& selCmd = "", bool unlockedTrackbar = false, bool executeOnEveryTick = false)
                : StepTrackBarV2(label, packagePath, stepDescriptions.size(), 0, (stepDescriptions.size()-1), "", executeCommands, sourceReplacementFunc, cmd, selCmd, true, unlockedTrackbar, executeOnEveryTick), m_stepDescriptions(stepDescriptions) {
                    //usingNamedStepTrackbar = true;
                    //logMessage("on initialization");

                    // Initialize the selection with the current index
                    if (!m_stepDescriptions.empty() && m_index >= 0 && m_index < static_cast<s16>(m_stepDescriptions.size())) {
                        this->m_selection = m_stepDescriptions[m_index];
                        currentDescIndex = m_index;
                    }
                }
            
            virtual ~NamedStepTrackBarV2() {}
                        
            virtual void draw(gfx::Renderer *renderer) override {
                // Cache frequently used values
                const u16 trackBarWidth = this->getWidth() - 95;
                const u16 baseX = this->getX() + 59;
                const u16 baseY = this->getY() + 44; // 50 - 3
                const u8 numSteps = this->m_numSteps;
                const u8 halfNumSteps = (numSteps - 1) / 2;
                const u16 lastStepX = baseX + trackBarWidth - 1;
                
                // Pre-calculate step spacing
                const float stepSpacing = static_cast<float>(trackBarWidth) / (numSteps - 1);
                
                // Cache color for multiple drawRect calls
                const auto stepColor = a(trackBarEmptyColor);
                
                // Draw step rectangles - optimized loop
                u16 stepX;
                for (u8 i = 0; i < numSteps; i++) {
                    
                    if (i == numSteps - 1) {
                        // Last step - avoid overshooting
                        stepX = lastStepX;
                    } else {
                        stepX = baseX + static_cast<u16>(std::round(i * stepSpacing));
                        // Adjust for steps on right side of center
                        if (i > halfNumSteps) {
                            stepX -= 1;
                        }
                    }
                    
                    renderer->drawRect(stepX, baseY, 1, 8, stepColor);
                }
                
                // Update selection (only if index changed - optional optimization)
                if (currentDescIndex != this->m_index) {
                    currentDescIndex = this->m_index;
                    this->m_selection = this->m_stepDescriptions[currentDescIndex];
                }
                
                // Draw the parent trackbar
                StepTrackBarV2::draw(renderer);
            }

            
        protected:
            std::vector<std::string> m_stepDescriptions;
            
        };
        
    }
    
    // Global state and event system
    static inline Event notificationEvent;
    static inline std::mutex notificationJsonMutex;
    static inline std::atomic<uint32_t> notificationGeneration{0};
    
    struct NotificationFile {
        std::string filename;
        std::string fullPath;
        time_t creationTime;
        int priority;
    };

    class NotificationPrompt {
    public:
        NotificationPrompt()
            : enabled_(true),
              is_active_(false),
              //pending_event_fire_(false),
              generation_(notificationGeneration.load(std::memory_order_acquire))
        {}
    
        ~NotificationPrompt() {
            shutdown(); // safe cleanup
        }
    
        enum class PromptState {
            Inactive,
            SlidingIn,
            Visible,
            SlidingOut
        };
    
        struct NotificationData {
            std::string text;
            std::string fileName;
            size_t fontSize = 28;
            s32 promptWidth = 448;
            s32 promptHeight = 88;
            u32 durationMs = 2500;
            u32 priority = 20;
            u64 arrivalNs = 0;
    
            NotificationData() = default;
    
            NotificationData(const std::string& t, const std::string& f = "",
                             size_t fs = 28, s32 w = 448, s32 h = 88,
                             u32 dur = 2500, u32 prio = 20)
                : text(t), fileName(f), fontSize(fs), promptWidth(w), promptHeight(h),
                  durationMs(dur), priority(prio), arrivalNs(0) {}
        };
    
        struct NotificationCompare {
            bool operator()(const NotificationData& a, const NotificationData& b) const {
                if (a.priority == b.priority) {
                    return a.arrivalNs > b.arrivalNs; // FIFO
                }
                return a.priority > b.priority; // Max-heap
            }
        };
    
        struct NotificationState {
            std::string activeText;
            std::string fileName;
            size_t fontSize = 28;
            s32 promptWidth = 448;
            s32 promptHeight = 88;
            PromptState state = PromptState::Inactive;
            u64 expireNs = 0;
            u64 stateStartNs = 0;
    
            NotificationState() = default;
            bool isTextEmpty() const { return activeText.empty(); }
        };
    
        // ---------------- Public Methods ----------------
    
        void show(const std::string& msg, size_t fontSize = 26, u32 priority = 20,
                  const std::string& fileName = "", u32 durationMs = 2500,
                  s32 promptWidth = 448, s32 promptHeight = 88)
        {
            if (msg.empty()) return;
    
            // Quick reject using atomics (fast-path)
            if (!enabled_.load(std::memory_order_acquire)) return;
            if (!ult::useNotifications) return;
            if (generation_ != notificationGeneration.load(std::memory_order_acquire)) return;
    
            NotificationData data;
            data.text = msg;
            data.fileName = fileName;
            data.fontSize = std::clamp(fontSize, size_t(8), size_t(48));
            data.promptWidth = std::clamp(promptWidth, s32(100), s32(1280));
            data.promptHeight = std::clamp(promptHeight, s32(50), s32(720));
            data.durationMs = std::clamp(durationMs, 500u, 30000u);
            data.priority = priority;
            data.arrivalNs = armTicksToNs(armGetSystemTick());
    
            std::lock_guard<std::mutex> lg(state_mutex_);
    
            // Re-check under lock to avoid TOCTOU
            if (!enabled_.load(std::memory_order_acquire)) return;
            if (generation_ != notificationGeneration.load(std::memory_order_acquire)) return;
            if (pending_queue_.size() >= MAX_NOTIFS) return;
    
            pending_queue_.push(data);
    
            if (!is_active_) {
                startNext_NoLock();
                //pending_event_fire_.store(true, std::memory_order_release);
                eventFire(&notificationEvent);

                #if IS_STATUS_MONITOR_DIRECTIVE
                if (isRendering) {
                    isRendering = false;
                    wasRendering = true;
                    
                    leventSignal(&renderingStopEvent);
                }
                #endif
            }
        }
    

        void draw(gfx::Renderer* renderer, bool promptOnly = false) {
            if (ult::launchingOverlay.load(std::memory_order_acquire) ||
                generation_ != notificationGeneration.load(std::memory_order_acquire)) return;
            if (!enabled_.load(std::memory_order_acquire)) return;
        
            NotificationState copy;
            {
                std::lock_guard<std::mutex> lg(state_mutex_);
                if (current_state_.state == PromptState::Inactive || current_state_.activeText.empty()) return;
                copy = current_state_;
            }
            
            const u64 now = armTicksToNs(armGetSystemTick());
            const u64 elapsedMs = (now - copy.stateStartNs) / 1'000'000ULL;
        
            s32 x = 0, y = 0;
            switch (copy.state) {
                case PromptState::SlidingIn: {
                    const float t = std::min(1.0f, float(elapsedMs) / SLIDE_DURATION_MS);
                    x = ult::useRightAlignment ?
                        (tsl::cfg::FramebufferWidth - copy.promptWidth + static_cast<s32>((1.0f - t) * copy.promptWidth)) :
                        static_cast<s32>(-copy.promptWidth + t * copy.promptWidth);
                    break;
                }
                case PromptState::Visible:
                    x = ult::useRightAlignment ? (tsl::cfg::FramebufferWidth - copy.promptWidth) : 0;
                    break;
                case PromptState::SlidingOut: {
                    const float t = std::min(1.0f, float(elapsedMs) / SLIDE_DURATION_MS);
                    x = ult::useRightAlignment ?
                        (tsl::cfg::FramebufferWidth - copy.promptWidth + static_cast<s32>(t * copy.promptWidth)) :
                        static_cast<s32>(-t * copy.promptWidth);
                    break;
                }
                default: return;
            }
        
            const s32 scissorX = std::max(0, x);
            const s32 scissorW = std::min(copy.promptWidth, tsl::cfg::FramebufferWidth - scissorX);
        
            if (scissorX >= 0 && scissorW > 0 && copy.promptHeight > 0) {
                renderer->enableScissoring(scissorX, y, scissorW, copy.promptHeight);

            #if IS_STATUS_MONITOR_DIRECTIVE
                renderer->drawRect(x, y, copy.promptWidth, copy.promptHeight, defaultBackgroundColor);
            #else
                if (!promptOnly && ult::expandedMemory)
                    renderer->drawRectMultiThreaded(x, y, copy.promptWidth, copy.promptHeight, defaultBackgroundColor);
                else
                    renderer->drawRect(x, y, copy.promptWidth, copy.promptHeight, defaultBackgroundColor);
            #endif
            
                if (!copy.activeText.empty()) {
                    std::vector<std::string> lines;
                    const std::string& text = copy.activeText;
            
                    size_t start = 0;
                    while (start < text.size() && lines.size() < 8) {
                        // Look for escaped "\n"
                        const size_t pos = text.find("\n", start);
            
                        if (pos == std::string::npos) {
                            // No more "\n", take the rest
                            lines.emplace_back(text.substr(start));
                            break;
                        } else {
                            // Extract line up to the escape sequence
                            lines.emplace_back(text.substr(start, pos - start));
                            start = pos + 1; // Skip past "\n"
                        }
                    }
            
                    const auto fm = tsl::gfx::FontManager::getFontMetricsForCharacter('A', copy.fontSize);
                    const s32 startY = y + (copy.promptHeight - (static_cast<int>(lines.size()) * fm.lineHeight)) / 2 + fm.ascent;
            
                    for (size_t i = 0; i < lines.size(); ++i) {
                        const std::string& line = lines[i];
                        
                        #if IS_LAUNCHER_DIRECTIVE
                        // Check if line contains "Ultrahand" (case insensitive)
                        const bool hasUltrahand = (line.find(ult::CAPITAL_ULTRAHAND_PROJECT_NAME) != std::string::npos);
                        
                        if (hasUltrahand) {
                            // Draw line with dynamic Ultrahand effect
                            drawUltrahandLine(renderer, line, x, startY + static_cast<int>(i) * fm.lineHeight, 
                                            copy.fontSize, copy.promptWidth);
                        } else {
                            // Draw normal line
                            const auto [lw, lh] = renderer->getNotificationTextDimensions(line, false, copy.fontSize);
                            renderer->drawNotificationString(
                                line, false,
                                x + (copy.promptWidth - lw) / 2,
                                startY + static_cast<int>(i) * fm.lineHeight,
                                copy.fontSize, notificationTextColor
                            );
                        }
                        #else
                        // Draw normal line
                        const auto [lw, lh] = renderer->getNotificationTextDimensions(line, false, copy.fontSize);
                        renderer->drawNotificationString(
                            line, false,
                            x + (copy.promptWidth - lw) / 2,
                            startY + static_cast<int>(i) * fm.lineHeight,
                            copy.fontSize, notificationTextColor
                        );
                        #endif
                    }
                }
            
                if (!ult::useRightAlignment) {
                    renderer->drawRect(x + copy.promptWidth - 1, y, 1, copy.promptHeight, edgeSeparatorColor);
                    renderer->drawRect(x, y + copy.promptHeight - 1, copy.promptWidth, 1, edgeSeparatorColor);
                } else {
                    renderer->drawRect(x, y, 1, copy.promptHeight, edgeSeparatorColor);
                    renderer->drawRect(x, y + copy.promptHeight - 1, copy.promptWidth, 1, edgeSeparatorColor);
                }
            
                renderer->disableScissoring();
            }
        }

        #if IS_LAUNCHER_DIRECTIVE
        void drawUltrahandLine(gfx::Renderer* renderer, const std::string& line, s32 x, s32 y, 
                              u32 fontSize, s32 promptWidth) {
            // Find position of "Ultrahand" in the line (case insensitive)
            size_t ultrahandPos = std::string::npos;
            std::string ultrahandToReplace;
            
            // Check for "Ultrahand" first
            ultrahandPos = line.find(ult::CAPITAL_ULTRAHAND_PROJECT_NAME);
            if (ultrahandPos != std::string::npos) {
                ultrahandToReplace = ult::CAPITAL_ULTRAHAND_PROJECT_NAME;
            }
            
            if (ultrahandPos == std::string::npos) {
                // Fallback to normal drawing if not found
                const auto [lw, lh] = renderer->getNotificationTextDimensions(line, false, fontSize);
                renderer->drawNotificationString(line, false, x + (promptWidth - lw) / 2, y, fontSize, notificationTextColor);
                return;
            }
            
            // Split the line into parts
            const std::string before = line.substr(0, ultrahandPos);
            const std::string hand = ult::SPLIT_PROJECT_NAME_2;
            const std::string after = line.substr(ultrahandPos + ultrahandToReplace.length());
            
            // Calculate individual part widths to get accurate total width
            s32 beforeWidth = 0, handWidth = 0, afterWidth = 0;
            
            if (!before.empty()) {
                const auto [bw, bh] = renderer->getNotificationTextDimensions(before, false, fontSize);
                beforeWidth = bw;
            }
            
            if (!after.empty()) {
                const auto [aw, ah] = renderer->getNotificationTextDimensions(after, false, fontSize);
                afterWidth = aw;
            }
            
            const auto [hw, hh] = renderer->getNotificationTextDimensions(hand, false, fontSize);
            handWidth = hw;
            
            // Use shared utility to calculate Ultra width
            const s32 ultraWidth = tsl::elm::calculateUltraTextWidth(renderer, fontSize, true);
            
            // Calculate total width and starting position for centering
            const s32 totalWidth = beforeWidth + ultraWidth + handWidth + afterWidth;
            s32 currentX = x + (promptWidth - totalWidth) / 2;
            
            // Draw each part in sequence
            
            // Draw "before" part
            if (!before.empty()) {
                renderer->drawNotificationString(before, false, currentX, y, fontSize, notificationTextColor);
                currentX += beforeWidth;
            }
            
            // Draw dynamic "Ultra" part using shared utility
            currentX = tsl::elm::drawDynamicUltraText(renderer, currentX, y, fontSize, logoColor1, true);
            
            // Draw static "hand" part
            renderer->drawNotificationString(hand, false, currentX, y, fontSize, logoColor2);
            currentX += handWidth;
            
            // Draw "after" part
            if (!after.empty()) {
                renderer->drawNotificationString(after, false, currentX, y, fontSize, notificationTextColor);
            }
        }
        #endif

        void update() {
            if (!isActive()) {
                return;
            }
            std::lock_guard<std::mutex> lg(state_mutex_);
            // Optional extra safety: skip if already inactive and queue empty
            if (ult::launchingOverlay.load(std::memory_order_acquire) ||
                (!is_active_ && current_state_.activeText.empty() && pending_queue_.empty())) {
                return;
            }
    
            //std::lock_guard<std::mutex> lg(state_mutex_);
            if (generation_ != notificationGeneration.load(std::memory_order_acquire) ||
                !enabled_.load(std::memory_order_acquire))
            {
                current_state_ = NotificationState{};
                is_active_ = false;
                return;
            }
    
            const u64 now = armTicksToNs(armGetSystemTick());
            const u64 elapsedMs = (current_state_.stateStartNs == 0) ? 0 : (now - current_state_.stateStartNs) / 1'000'000ULL;
    
            switch (current_state_.state) {
                case PromptState::SlidingIn:
                    if (elapsedMs >= SLIDE_DURATION_MS) {
                        current_state_.state = PromptState::Visible;
                        current_state_.stateStartNs = now;
                    }
                    break;
                case PromptState::Visible:
                    if (now >= current_state_.expireNs) {
                        current_state_.state = PromptState::SlidingOut;
                        current_state_.stateStartNs = now;
                    }
                    break;
                case PromptState::SlidingOut:
                    if (elapsedMs >= SLIDE_DURATION_MS) {
                        const std::string fileToDelete = current_state_.fileName;
                        // Delete the JSON file safely
                        if (!fileToDelete.empty()) {
                            std::lock_guard<std::mutex> lg(notificationJsonMutex);
                            const std::string fullPath = ult::NOTIFICATIONS_PATH + fileToDelete;
                            remove(fullPath.c_str());  // ignore errors for now
                        }

                        current_state_ = NotificationState{};
                        const bool hadNext = startNext_NoLock();
                        if (!hadNext) {
                            is_active_ = false;
                        }
                    }
                    break;
                default: break;
            }
        }
    
        bool isActive() const {
            if (!ult::useNotifications) return false;
            if (generation_ != notificationGeneration.load(std::memory_order_acquire)) return false;
            std::lock_guard<std::mutex> lg(state_mutex_);
            if (is_active_) return true;
            if (!pending_queue_.empty()) return true;
            //if (pending_event_fire_.load(std::memory_order_acquire)) return true;
            if (!current_state_.activeText.empty() && current_state_.state != PromptState::Inactive) return true;
            return false;
        }
    
        void shutdown() {
            enabled_.store(false, std::memory_order_release);
            notificationGeneration.fetch_add(1, std::memory_order_acq_rel);
            generation_ = notificationGeneration.load(std::memory_order_acquire);
    
            std::lock_guard<std::mutex> lg(state_mutex_);
            while (!pending_queue_.empty()) pending_queue_.pop();
            current_state_ = NotificationState{};
            is_active_ = false;
            //pending_event_fire_.store(false, std::memory_order_release);
        }
    
        void forceShutdown() {
            enabled_.store(false, std::memory_order_release);
            //pending_event_fire_.store(false, std::memory_order_release);
        }
    
    
    private:
        static constexpr size_t MAX_NOTIFS = 30;
        static constexpr u32 SLIDE_DURATION_MS = 200;
        //static constexpr double cycleDuration = 1.6;
    
        mutable std::mutex state_mutex_;
        NotificationState current_state_;
        std::priority_queue<NotificationData, std::vector<NotificationData>, NotificationCompare> pending_queue_;

        std::atomic<bool> enabled_{true};
        bool is_active_{false}; // protected by mutex
        //std::atomic<bool> pending_event_fire_{false};
        uint32_t generation_{0};
    
        bool startNext_NoLock() {
            if (pending_queue_.empty()) return false;
            const NotificationData next = pending_queue_.top();
            pending_queue_.pop();
    
            const u64 now = armTicksToNs(armGetSystemTick());
            current_state_.activeText = next.text;
            current_state_.fileName = next.fileName;
            current_state_.fontSize = next.fontSize;
            current_state_.promptWidth = next.promptWidth;
            current_state_.promptHeight = next.promptHeight;
            current_state_.state = PromptState::SlidingIn;
            current_state_.stateStartNs = now;
            current_state_.expireNs = now + static_cast<u64>(SLIDE_DURATION_MS) * 1'000'000ULL
                                        + static_cast<u64>(next.durationMs) * 1'000'000ULL;
            is_active_ = true;
            return true;
        }
    };
    
    // Optional: pointer to global notification
    static inline NotificationPrompt* notification = nullptr;
    

    // GUI
    
    /**
     * @brief The top level Gui class
     * @note The main menu and every sub menu are a separate Gui. Create your own Gui class that extends from this one to create your own menus
     *
     */
    class Gui {
    public:
        Gui() {
            #if IS_LAUNCHER_DIRECTIVE
            #else
            {
                #if INITIALIZE_IN_GUI_DIRECTIVE // for different project structures
                tsl::initializeThemeVars();
                
                // Load the bitmap file into memory
                ult::loadWallpaperFileWhenSafe();
                #endif
            }
            #endif
        }
        
        virtual ~Gui() {
            if (this->m_topElement != nullptr)
                delete this->m_topElement;

            if (this->m_bottomElement != nullptr)
                delete this->m_bottomElement;
        }
        
        /**
         * @brief Creates all elements present in this Gui
         * @note Implement this function and let it return a heap allocated element used as the top level element. This is usually some kind of frame e.g \ref OverlayFrame
         *
         * @return Top level element
         */
        virtual elm::Element* createUI() = 0;
        
        /**
         * @brief Called once per frame to update values
         *
         */
        virtual void update() {}
        
        /**
         * @brief Called once per frame with the latest HID inputs
         *
         * @param keysDown Buttons pressed in the last frame
         * @param keysHeld Buttons held down longer than one frame
         * @param touchInput Last touch position
         * @param leftJoyStick Left joystick position
         * @param rightJoyStick Right joystick position
         * @return Weather or not the input has been consumed
         */
        virtual inline bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) {
            return false;
        }
        
        /**
         * @brief Gets the top level element
         *
         * @return Top level element
         */
        elm::Element* getTopElement() {
            return this->m_topElement;
        }
        
        /**
         * @brief Gets the bottom level element
         *
         * @return Bottom level element
         */
        elm::Element* getBottomElement() {
            return this->m_bottomElement;
        }

        /**
         * @brief Get the currently focused element
         *
         * @return Focused element
         */
        elm::Element* getFocusedElement() {
            return this->m_focusedElement;
        }
        
        /**
         * @brief Requests focus to a element
         * @note Use this function when focusing a element outside of a element's requestFocus function
         *
         * @param element Element to focus
         * @param direction Focus direction
         */
        inline void requestFocus(elm::Element *element, FocusDirection direction, bool shake = true) {
            elm::Element *oldFocus = this->m_focusedElement;
            
            if (element != nullptr) {
                this->m_focusedElement = element->requestFocus(oldFocus, direction);
                
                if (oldFocus != nullptr)
                    oldFocus->setFocused(false);
                
                if (this->m_focusedElement != nullptr) {
                    this->m_focusedElement->setFocused(true);
                }
            }
            
            if (shake && oldFocus == this->m_focusedElement && this->m_focusedElement != nullptr)
                this->m_focusedElement->shakeHighlight(direction);
        }
        
        /**
         * @brief Removes focus from a element
         *
         * @param element Element to remove focus from. Pass nullptr to remove the focus unconditionally
         */
        inline void removeFocus(elm::Element* element = nullptr) {
            if (element == nullptr || element == this->m_focusedElement) {
                if (this->m_focusedElement != nullptr) {
                    this->m_focusedElement->setFocused(false);
                    this->m_focusedElement = nullptr;
                }
            }
        }
        
        inline void restoreFocus() {
            this->m_initialFocusSet = false;
        }
        
    protected:
        constexpr static inline auto a = &gfx::Renderer::a;
        constexpr static inline auto aWithOpacity = &gfx::Renderer::aWithOpacity;
        
    private:
        elm::Element *m_focusedElement = nullptr;
        elm::Element *m_topElement = nullptr;
        elm::Element *m_bottomElement = nullptr;

        bool m_initialFocusSet = false;
        
        friend class Overlay;
        friend class gfx::Renderer;
        
        //// Function to recursively find the bottom element
        //void findBottomElement(elm::Element* currentElement) {
        //    // Base case: if the current element has no children, it is the bottom element
        //    if (currentElement->getChildren().empty()) {
        //        m_bottomElement = currentElement;
        //        return;
        //    }
        //
        //    // Recursive case: traverse through all children elements
        //    for (elm::Element* child : currentElement->getChildren()) {
        //        findBottomElement(child);
        //    }
        //}

        /**
         * @brief Draws the Gui
         *
         * @param renderer
         */
        void draw(gfx::Renderer *renderer) {
            if (this->m_topElement != nullptr)
                this->m_topElement->draw(renderer);
        }
        
        inline bool initialFocusSet() {
            return this->m_initialFocusSet;
        }
        
        inline void markInitialFocusSet() {
            this->m_initialFocusSet = true;
        }
        
    };
    

    // Overlay
    
    /**
     * @brief The top level Overlay class
     * @note Every Tesla overlay should have exactly one Overlay class initializing services and loading the default Gui
     */
    class Overlay {
    protected:
        /**
         * @brief Constructor
         * @note Called once when the Overlay gets loaded
         */
        Overlay() {}
    public:

        /**
         * @brief Deconstructor
         * @note Called once when the Overlay exits
         *
         */
        virtual ~Overlay() {}


        /**
         * @brief Initializes services
         * @note Called once at the start to initializes services. You have a sm session available during this call, no need to initialize sm yourself
         */
        virtual void initServices() {}
        
        /**
         * @brief Exits services
         * @note Make sure to exit all services you initialized in \ref Overlay::initServices() here to prevent leaking handles
         */
        virtual void exitServices() {}
        
        /**
         * @brief Called before overlay changes from invisible to visible state
         *
         */
        virtual void onShow() {}
        
        /**
         * @brief Called before overlay changes from visible to invisible state
         *
         */
        virtual void onHide() {}
        
        /**
         * @brief Loads the default Gui
         * @note This function should return the initial Gui to load using the \ref Gui::initially<T>(Args.. args) function
         *       e.g `return initially<GuiMain>();`
         *
         * @return Default Gui
         */
        virtual std::unique_ptr<tsl::Gui> loadInitialGui() = 0;
        
        /**
         * @brief Gets a reference to the current Gui on top of the Gui stack
         *
         * @return Current Gui reference
         */
        std::unique_ptr<tsl::Gui>& getCurrentGui() {
            return this->m_guiStack.top();
        }
        
        /**
         * @brief Shows the Gui
         *
         */
        void show() {
            

            if (this->m_disableNextAnimation) {
                this->m_animationCounter = MAX_ANIMATION_COUNTER;
                this->m_disableNextAnimation = false;
            }
            else {
                this->m_fadeInAnimationPlaying = true;
                this->m_animationCounter = 0;
            }

            ult::isHidden.store(false);
            this->onShow();
            triggerRumbleClick.store(true, std::memory_order_release);

            // reinitialize audio for changes from handheld to docked and vise versa
            if (ult::expandedMemory && ult::useSoundEffects)
                ult::AudioPlayer::reloadIfDockedChanged();
            
            //if (auto& currGui = this->getCurrentGui(); currGui != nullptr) // TESTING DISABLED (EFFECTS NEED TO BE VERIFIED)
            //    currGui->restoreFocus();
        }
        
        /**
         * @brief Hides the Gui
         *
         */
        void hide(bool useNoFade = false) {


            if (useNoFade) {
                // Immediately hide overlay
                ult::isHidden.store(true);
                this->m_shouldHide = true;
                return;
            }
            

        #if IS_STATUS_MONITOR_DIRECTIVE
            if (FullMode && !deactivateOriginalFooter) {
                
                if (this->m_disableNextAnimation) {
                    this->m_animationCounter = 0;
                    this->m_disableNextAnimation = false;
                }
                else {
                    this->m_fadeOutAnimationPlaying = true;
                    this->m_animationCounter = MAX_ANIMATION_COUNTER;
                }
                ult::isHidden.store(true);
                this->onHide();
            }
        #else

            if (this->m_disableNextAnimation) {
                this->m_animationCounter = 0;
                this->m_disableNextAnimation = false;
            }
            else {
                this->m_fadeOutAnimationPlaying = true;
                this->m_animationCounter = MAX_ANIMATION_COUNTER;
            }
            ult::isHidden.store(true);
            this->onHide();
        #endif
            triggerRumbleClick.store(true, std::memory_order_release);
        }
        
        /**
         * @brief Returns whether fade animation is playing
         *
         * @return whether fade animation is playing
         */
        bool fadeAnimationPlaying() {
            return this->m_fadeInAnimationPlaying || this->m_fadeOutAnimationPlaying;
        }
        
        /**
         * @brief Closes the Gui
         * @note This makes the Tesla overlay exit and return back to the Tesla-Menu
         *
         */
        void close(bool forceClose = false) {
            if (!forceClose && notification && notification->isActive()) {
                this->closeAfter();
                this->hide(true);
                return;
            }
        
            this->m_shouldClose = true;
        }

        /**
         * @brief Closes the Gui
         * @note This makes the Tesla overlay exit and return back to the Tesla-Menu
         *
         */
        void closeAfter() {
            this->m_shouldCloseAfter = true;

        }
        
        /**
         * @brief Gets the Overlay instance
         *
         * @return Overlay instance
         */
        static inline Overlay* const get() {
            return Overlay::s_overlayInstance;
        }
        
        /**
         * @brief Creates the initial Gui of an Overlay and moves the object to the Gui stack
         *
         * @tparam T
         * @tparam Args
         * @param args
         * @return constexpr std::unique_ptr<T>
         */
        template<typename T, typename ... Args>
        constexpr inline std::unique_ptr<T> initially(Args&&... args) {
            return std::make_unique<T>(args...);
        }
        
    private:
        using GuiPtr = std::unique_ptr<tsl::Gui>;
        std::stack<GuiPtr, std::list<GuiPtr>> m_guiStack;
        static inline Overlay *s_overlayInstance = nullptr;
        
        bool m_fadeInAnimationPlaying = false, m_fadeOutAnimationPlaying = false;
        u8 m_animationCounter = 0;
        static constexpr int MAX_ANIMATION_COUNTER = 5; // Define the maximum animation counter value

        bool m_shouldHide = false;
        bool m_shouldClose = false;
        bool m_shouldCloseAfter = false;
        
        bool m_disableNextAnimation = false;
        
        bool m_closeOnExit;
        
        static inline std::atomic<bool> isNavigatingBackwards{false};
        bool justNavigated = false;

        /**
         * @brief Initializes the Renderer
         *
         */
        void initScreen() {
            gfx::Renderer::get().init();
        }
        
        /**
         * @brief Exits the Renderer
         *
         */
        void exitScreen() {
            gfx::Renderer::get().exit();
        }
        
        /**
         * @brief Weather or not the Gui should get hidden
         *
         * @return should hide
         */
        bool shouldHide() {
            return this->m_shouldHide;
        }
        
        /**
         * @brief Weather or not hte Gui should get closed
         *
         * @return should close
         */
        bool shouldClose() {
            return this->m_shouldClose;
        }
        
        /**
         * @brief Weather or not hte Gui should get closed after
         *
         * @return should close after
         */
        bool shouldCloseAfter() {
            return this->m_shouldCloseAfter;
        }
        

        /**
         * @brief Quadratic ease-in-out function
         *
         * @param t Normalized time (0 to 1)
         * @return Eased value
         */
        float calculateEaseInOut(float t) {
            if (t < 0.5) {
                return 2 * t * t;
            } else {
                return -1 + (4 - 2 * t) * t;
            }
        }

        /**
         * @brief Handles fade in and fade out animations of the Overlay
         *
         */
        void animationLoop() {
            
        
            if (this->m_fadeInAnimationPlaying) {
                if (this->m_animationCounter < MAX_ANIMATION_COUNTER) {
                    this->m_animationCounter++;
                }
                
                if (this->m_animationCounter >= MAX_ANIMATION_COUNTER) {
                    this->m_fadeInAnimationPlaying = false;
                }
            }
            
            if (this->m_fadeOutAnimationPlaying) {
                if (this->m_animationCounter > 0) {
                    this->m_animationCounter--;
                }
                
                if (this->m_animationCounter == 0) {
                    this->m_fadeOutAnimationPlaying = false;
                    this->m_shouldHide = true;
                }
            }
        
            // Calculate and set the opacity using an easing function
            //float opacity = calculateEaseInOut(static_cast<float>(this->m_animationCounter) / MAX_ANIMATION_COUNTER);
            gfx::Renderer::setOpacity(calculateEaseInOut(static_cast<float>(this->m_animationCounter) / MAX_ANIMATION_COUNTER));
        }


        
        /**
         * @brief Overlay Main loop
         *
         */
        void loop(bool promptOnly = false) {
            // Early exit check - avoid all work if shutting down
            if (ult::launchingOverlay.load(std::memory_order_acquire)) {
                return;
            }
            
            // CRITICAL: Initialize to TRUE because stacks are added in init()!
            static std::atomic<bool> screenshotStacksAdded{true};
            static std::atomic<bool> notificationCacheNeedsClearing{false};

            auto& renderer = gfx::Renderer::get();
            renderer.startFrame();
        
            // Handle main UI rendering
            if (!promptOnly) {

                // In normal mode, ensure screenshots are enabled
                // Only re-add if they were removed AND force-disable is not set
                if (!screenshotStacksAdded.load(std::memory_order_acquire) &&
                    !screenshotsAreForceDisabled.load(std::memory_order_acquire)) {
                    if (!screenshotStacksAdded.exchange(true, std::memory_order_acq_rel)) {
                        renderer.addScreenshotStacks(false);
                    }
                }
                
                this->animationLoop();
                this->getCurrentGui()->update();
                this->getCurrentGui()->draw(&renderer);
                
                //notificationCacheNeedsClearing.store(true, std::memory_order_release);
            } else {
                // Prompt-only mode - temporarily remove screenshots
                if (screenshotStacksAdded.load(std::memory_order_acquire) &&
                    !screenshotsAreDisabled.load(std::memory_order_acquire) &&
                    !screenshotsAreForceDisabled.load(std::memory_order_acquire)) {

                    if (screenshotStacksAdded.exchange(false, std::memory_order_acq_rel)) {
                        renderer.removeScreenshotStacks(false);
                    }
                }
                renderer.clearScreen();
            }
        
            // Notification handling — safe, consistent, and null-guarded
            {
                if (notification && notification->isActive()) {
                    notification->update();
                    notification->draw(&renderer, promptOnly);

                    // Only set flag if it's not already set
                    notificationCacheNeedsClearing.exchange(true, std::memory_order_acq_rel);

                } else if (notificationCacheNeedsClearing.exchange(false, std::memory_order_acq_rel)) {
                    tsl::gfx::FontManager::clearNotificationCache();
                    #if IS_STATUS_MONITOR_DIRECTIVE
                    if (wasRendering) {
                        wasRendering = false;
                        isRendering = true;
                        leventClear(&renderingStopEvent);
                    }
                    #endif
                }
            }
        
            renderer.endFrame();
        }
        
        // Calculate transition using ease-in-out curve instead of linear
        float easeInOutCubic(float t) {
            return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
        }
        
        

        void handleInput(u64 keysDown, u64 keysHeld, bool touchDetected, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
            if (!ult::internalTouchReleased.load(std::memory_order_acquire) || ult::launchingOverlay.load(std::memory_order_acquire))
                return;

            // Static variables to maintain state between function calls
            static HidTouchState initialTouchPos = { 0 };
            static HidTouchState oldTouchPos = { 0 };
            static bool oldTouchDetected = false;
            static elm::TouchEvent touchEvent, oldTouchEvent;
        
            static u64 buttonPressTime_ns = 0, lastKeyEventTime_ns = 0, keyEventInterval_ns = 67000000ULL;
            static bool singlePressHandled = false;
            static constexpr u64 CLICK_THRESHOLD_NS = 340000000ULL; // 340ms in nanoseconds
            

            static bool hasScrolled = false;
            static void* lastGuiPtr = nullptr;  // Use void* instead
        
            auto& currentGui = this->getCurrentGui();

            // Return early if current GUI is not available or internal touch is not released
            if (!currentGui) {

                elm::Element::setInputMode(InputMode::Controller);
                
                oldTouchPos = { 0 };
                initialTouchPos = { 0 };
                touchEvent = elm::TouchEvent::None;
                ult::stillTouching.store(false, std::memory_order_release);
                ult::interruptedTouch.store(false, std::memory_order_release);
                return;
            }

            // Retrieve current focus and top/bottom elements of the GUI
            auto currentFocus = currentGui->getFocusedElement();
            

            const bool interpreterIsRunning = ult::runningInterpreter.load(std::memory_order_acquire);
        #if !IS_STATUS_MONITOR_DIRECTIVE
            if (interpreterIsRunning) {
                if (keysDown & KEY_UP && !(keysHeld & ~KEY_UP & ALL_KEYS_MASK)) {
                    currentFocus->shakeHighlight(FocusDirection::Up);
                    return;
                }
                else if (keysDown & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ALL_KEYS_MASK)) {
                    currentFocus->shakeHighlight(FocusDirection::Down);
                    return;
                }
                else if (keysDown & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK)) {
                    currentFocus->shakeHighlight(FocusDirection::Left);
                    return;
                }
                else if (keysDown & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK)) {
                    currentFocus->shakeHighlight(FocusDirection::Right);
                    return;
                }
            }
        #endif
        
        #if IS_STATUS_MONITOR_DIRECTIVE
            if (FullMode && !deactivateOriginalFooter) {
                if (ult::simulatedSelect.exchange(false, std::memory_order_acq_rel))
                    keysDown |= KEY_A;
                
                if (ult::simulatedBack.exchange(false, std::memory_order_acq_rel))
                    keysDown |= KEY_B;

                if (!overrideBackButton) {
                    if (keysDown & KEY_B && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)) {
                        if (!currentGui->handleInput(KEY_B,0,{},{},{})) {
                            this->goBack();
                            if (this->m_guiStack.size() >= 1) {
                                //triggerRumbleDoubleClick.store(true, std::memory_order_release);
                                //triggerExitSound.store(true, std::memory_order_release);
                                triggerExitFeedback();
                            }
                            //ult::simulatedBackComplete = true;
                        }
                        return;
                    }
                }
            } else {
                ult::simulatedSelect.exchange(false, std::memory_order_acq_rel);
                ult::simulatedBack.exchange(false, std::memory_order_acq_rel);
            }
        #else
            if (ult::simulatedSelect.exchange(false, std::memory_order_acq_rel))
                keysDown |= KEY_A;
            
            if (ult::simulatedBack.exchange(false, std::memory_order_acq_rel))
                keysDown |= KEY_B;

            if (!overrideBackButton) {
                if (keysDown & KEY_B && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)) {
                    if (!currentGui->handleInput(KEY_B,0,{},{},{})) {
                        this->goBack();
                        if (this->m_guiStack.size() >= 1 && !interpreterIsRunning) {
                            //triggerRumbleDoubleClick.store(true, std::memory_order_release);
                            //triggerExitSound.store(true, std::memory_order_release);
                            triggerExitFeedback();
                        }
                        //ult::simulatedBackComplete = true;
                    }
                    return;
                }
            } else {
                if (keysDown & KEY_B && !(keysHeld & ~KEY_B & ALL_KEYS_MASK)) {
                    if (this->m_guiStack.size() >= 1 && !interpreterIsRunning) {
                        //triggerRumbleDoubleClick.store(true, std::memory_order_release);
                        //triggerExitSound.store(true, std::memory_order_release);
                        triggerExitFeedback();
                    }
                }
            }
        #endif
        
            // Reset touch state when GUI changes
            if (currentGui.get() != lastGuiPtr) {  // or just currentGui != lastGuiPtr if it's not a smart pointer
                hasScrolled = false;
                oldTouchEvent = elm::TouchEvent::None;
                oldTouchDetected = false;
                oldTouchPos = { 0 };
                initialTouchPos = { 0 };
                lastGuiPtr = currentGui.get();  // or just currentGui
            }
            
            auto topElement = currentGui->getTopElement();

            const u64 currentTime_ns = armTicksToNs(armGetSystemTick());

            if (!currentFocus && !ult::simulatedBack.load(std::memory_order_acquire) && !ult::stillTouching.load(std::memory_order_acquire) && !oldTouchDetected && !interpreterIsRunning) {
                if (!topElement) return;
                
                if (!currentGui->initialFocusSet() || keysDown & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
                    currentGui->requestFocus(topElement, FocusDirection::None);
                    currentGui->markInitialFocusSet();
                }
            }
            if (isNavigatingBackwards.load(std::memory_order_acquire) && !currentFocus && topElement && keysDown & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) {
                currentGui->requestFocus(topElement, FocusDirection::None);
                currentGui->markInitialFocusSet();
                isNavigatingBackwards.store(false, std::memory_order_release);
                
                // Reset navigation timing to prevent fast scrolling
                buttonPressTime_ns = currentTime_ns;
                lastKeyEventTime_ns = buttonPressTime_ns;
                singlePressHandled = false;
            }
            

        
            if (!currentFocus && !touchDetected && (!oldTouchDetected || oldTouchEvent == elm::TouchEvent::Scroll)) {
                if (!isNavigatingBackwards.load(std::memory_order_acquire) &&
                    !ult::shortTouchAndRelease.load(std::memory_order_acquire) &&
                    !ult::longTouchAndRelease.load(std::memory_order_acquire) &&
                    !ult::simulatedSelect.load(std::memory_order_acquire) &&
                    !ult::simulatedBack.load(std::memory_order_acquire) &&
                    !ult::simulatedNextPage.load(std::memory_order_acquire)
                    && topElement) {

                    if (oldTouchEvent == elm::TouchEvent::Scroll) {
                        hasScrolled = true;
                    }
                    if (!hasScrolled) {
                        currentGui->removeFocus();
                        currentGui->requestFocus(topElement, FocusDirection::None);
                    }
                } else if (ult::longTouchAndRelease.exchange(false, std::memory_order_acq_rel)) {
                    hasScrolled = true;
                } else if (ult::shortTouchAndRelease.exchange(false, std::memory_order_acq_rel)) {
                    hasScrolled = true;
                }
            }
            
            bool handled = false;
            elm::Element* parentElement = currentFocus;
            
            while (!handled && parentElement) {
                handled = parentElement->onClick(keysDown) || parentElement->handleInput(keysDown, keysHeld, touchPos, joyStickPosLeft, joyStickPosRight);
                parentElement = parentElement->getParent();
            }
            
            if (currentGui != this->getCurrentGui()) return;
            
            handled |= currentGui->handleInput(keysDown, keysHeld, touchPos, joyStickPosLeft, joyStickPosRight);
            

            // Navigational boundary cases for handling wrapping
            static bool lastDirectionPressed = true;
            const bool directionPressed = ((keysHeld & KEY_UP) || (keysHeld & KEY_DOWN) || (keysHeld & KEY_LEFT) || (keysHeld & KEY_RIGHT));

            if (!directionPressed && lastDirectionPressed)
                tsl::elm::s_directionalKeyReleased.store(true, std::memory_order_release);
            else if (directionPressed && lastDirectionPressed)
                tsl::elm::s_directionalKeyReleased.store(false, std::memory_order_release);

            lastDirectionPressed = directionPressed;

            const float currentScrollVelocity = tsl::elm::s_currentScrollVelocity.load(std::memory_order_acquire);

            if (hasScrolled) {
                const bool singleArrowKeyPress = ((keysHeld & KEY_UP) != 0) + ((keysHeld & KEY_DOWN) != 0) + ((keysHeld & KEY_LEFT) != 0) + ((keysHeld & KEY_RIGHT) != 0) == 1 && !(keysHeld & ~((currentScrollVelocity != 0.0f ? KEY_A | KEY_UP : KEY_UP) | KEY_DOWN | KEY_LEFT | KEY_RIGHT) & ALL_KEYS_MASK);
                
                if (singleArrowKeyPress) {
                   // const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    buttonPressTime_ns = currentTime_ns;
                    lastKeyEventTime_ns = currentTime_ns;
                    hasScrolled = false;
                    isNavigatingBackwards.store(false, std::memory_order_release);
                }
            } else {
                if (!touchDetected && !oldTouchDetected && !handled && currentFocus && !ult::stillTouching.load(std::memory_order_acquire) && !interpreterIsRunning) {
                    static bool shouldShake = true;
                    const bool singleArrowKeyPress = ((keysHeld & KEY_UP) != 0) + ((keysHeld & KEY_DOWN) != 0) + ((keysHeld & KEY_LEFT) != 0) + ((keysHeld & KEY_RIGHT) != 0) == 1 && !(keysHeld & ~((currentScrollVelocity != 0.0f ? KEY_A | KEY_UP: KEY_UP) | KEY_DOWN | KEY_LEFT | KEY_RIGHT) & ALL_KEYS_MASK);
                    
                    if (singleArrowKeyPress) {
                        //const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                        
                        if (keysDown) {
                            buttonPressTime_ns = currentTime_ns;
                            lastKeyEventTime_ns = currentTime_ns;
                            singlePressHandled = false;
                            // Immediate single press action
                            if (keysHeld & KEY_UP && !(keysHeld & ~KEY_UP & ALL_KEYS_MASK))
                                currentGui->requestFocus(topElement, FocusDirection::Up, shouldShake);
                            else if (keysHeld & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ALL_KEYS_MASK)) {
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Down, shouldShake);
                                //isTopElement = false;
                            }
                            else if (keysHeld & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Left, shouldShake);
                            else if (keysHeld & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Right, shouldShake);
                        }
                        
                        if (keysHeld & ~KEY_DOWN & ~KEY_UP & ~KEY_LEFT & ~KEY_RIGHT & ALL_KEYS_MASK) // reset
                            buttonPressTime_ns = currentTime_ns;
                        
                        const u64 durationSincePress_ns = currentTime_ns - buttonPressTime_ns;
                        const u64 durationSinceLastEvent_ns = currentTime_ns - lastKeyEventTime_ns;
                        
                        if (!singlePressHandled && durationSincePress_ns >= CLICK_THRESHOLD_NS) {
                            singlePressHandled = true;
                        }
                        
                        if (!tsl::elm::isTableScrolling.load(std::memory_order_acquire)) {
                            // Calculate transition factor (t) from 0 to 1 based on how far we are from the transition point
                            static constexpr u64 transitionPoint_ns = 2000000000ULL; // 2000ms in nanoseconds
                            static constexpr u64 initialInterval_ns = 67000000ULL;   // 67ms in nanoseconds
                            static constexpr u64 shortInterval_ns = 10000000ULL;     // 10ms in nanoseconds
                            
                            const float t = (durationSincePress_ns >= transitionPoint_ns) ? 1.0f : 
                                     (float)durationSincePress_ns / (float)transitionPoint_ns;
                            // Smooth transition between intervals using linear interpolation
                            keyEventInterval_ns = ((1.0f - t) * initialInterval_ns + t * shortInterval_ns);
                        } else {
                            // Table scrolling - faster timing
                            static constexpr u64 transitionPoint_ns = 200000000ULL; // 300ms (faster transition)
                            static constexpr u64 initialInterval_ns = 33000000ULL;   // 33ms (faster initial)
                            static constexpr u64 shortInterval_ns = 5000000ULL;      // 5ms (faster sustained)
                            
                            const float t = (durationSincePress_ns >= transitionPoint_ns) ? 1.0f : 
                                     (float)durationSincePress_ns / (float)transitionPoint_ns;
                            // Smooth transition between intervals using linear interpolation
                            keyEventInterval_ns = ((1.0f - t) * initialInterval_ns + t * shortInterval_ns);
                        }
                        
        
                        
                        if (singlePressHandled && durationSinceLastEvent_ns >= keyEventInterval_ns) {
                            lastKeyEventTime_ns = currentTime_ns;
                            if (keysHeld & KEY_UP && !(keysHeld & ~((currentScrollVelocity != 0.0f ? KEY_A | KEY_UP: KEY_UP)) & ALL_KEYS_MASK))
                                currentGui->requestFocus(topElement, FocusDirection::Up, false);
                            else if (keysHeld & KEY_DOWN && !(keysHeld & ~((currentScrollVelocity != 0.0f ? KEY_A | KEY_DOWN: KEY_DOWN)) & ALL_KEYS_MASK)) {
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Down, false);
                                //isTopElement = false;
                            }
                            else if (keysHeld & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Left, false);
                            else if (keysHeld & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Right, false);
                        }
        #if !IS_STATUS_MONITOR_DIRECTIVE
                    } else {
                        buttonPressTime_ns = lastKeyEventTime_ns = currentTime_ns;
        #endif
                    }
                }
            }
        
        #if !IS_STATUS_MONITOR_DIRECTIVE
            if (!touchDetected && !interpreterIsRunning && topElement) {
        #else
            if (!disableJumpTo && !touchDetected && !interpreterIsRunning && topElement) {
        #endif
                // Shared constants used by ZL/ZR buttons
                static constexpr u64 INITIAL_HOLD_THRESHOLD_NS = 400000000ULL;
                static constexpr u64 HOLD_THRESHOLD_NS = 300000000ULL;         // 300ms to start continuous
                static constexpr u64 RAPID_CLICK_WINDOW_NS = 500000000ULL;     // 500ms window for rapid clicking
                static constexpr u64 RAPID_MODE_TIMEOUT_NS = 1000000000ULL;    // 1s timeout to exit rapid mode
                // Acceleration timing constants
                static constexpr u64 ACCELERATION_POINT_NS = 1500000000ULL;    // 1.5s transition point
                static constexpr u64 INITIAL_INTERVAL_NS = 67000000ULL;        // 67ms initial interval
                static constexpr u64 FAST_INTERVAL_NS = 10000000ULL;           // 10ms fast interval
                
                //const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                                
                // Detect PHYSICAL key states (whether key is actually pressed)
                const bool lKeyPressed = (keysHeld & KEY_L);
                const bool rKeyPressed = (keysHeld & KEY_R);
                const bool zlKeyPressed = (keysHeld & KEY_ZL);
                const bool zrKeyPressed = (keysHeld & KEY_ZR);
                
                // Detect if other keys are pressed (for preventing timer resets)
                const bool notlKeyPressed = (keysHeld & ~KEY_L & ALL_KEYS_MASK);
                const bool notrKeyPressed = (keysHeld & ~KEY_R & ALL_KEYS_MASK);
                const bool notzlKeyPressed = (keysHeld & ~KEY_ZL & ALL_KEYS_MASK);
                const bool notzrKeyPressed = (keysHeld & ~KEY_ZR & ALL_KEYS_MASK);
                
                // Handle L button (simple jump to top on release, but not if held too long)
                {
                    static bool lKeyWasPressed = false;
                    static bool lWasIsolated = false;  // Track if L was isolated when first pressed
                    static u64 lButtonPressStart_ns = 0;
                    
                    if (lKeyPressed) {
                        
                        if (!lKeyWasPressed) {
                            // L key physically pressed for the first time (start timer)
                            lButtonPressStart_ns = currentTime_ns;
                            lWasIsolated = !notlKeyPressed;  // Remember if it started isolated
                        }
                        // Don't reset timer if other keys are pressed after L was already held
                        lKeyWasPressed = true;
                    } else {
                        if (lKeyWasPressed) {
                            // L key physically released - only jump to top if was isolated when first pressed and not held too long
                            if (lWasIsolated && !(keysHeld & ~KEY_L & ALL_KEYS_MASK)) {  // Was isolated initially and no other keys held at release
                                const u64 holdDuration = currentTime_ns - lButtonPressStart_ns;
                                if (holdDuration < INITIAL_HOLD_THRESHOLD_NS) {
                                    jumpToTop.store(true, std::memory_order_release);
                                    currentGui->requestFocus(topElement, FocusDirection::None);
                                }
                            }
                        }
                        lKeyWasPressed = false;
                        lWasIsolated = false;
                    }
                }
                
                // Handle R button (simple jump to bottom on release, but not if held too long)
                {
                    static bool rKeyWasPressed = false;
                    static bool rWasIsolated = false;  // Track if R was isolated when first pressed
                    static u64 rButtonPressStart_ns = 0;
                    
                    if (rKeyPressed) {
                        if (!rKeyWasPressed) {
                            // R key physically pressed for the first time (start timer)
                            rButtonPressStart_ns = currentTime_ns;
                            rWasIsolated = !notrKeyPressed;  // Remember if it started isolated
                        }
                        // Don't reset timer if other keys are pressed after R was already held
                        rKeyWasPressed = true;
                    } else {
                        if (rKeyWasPressed) {
                            // R key physically released - only jump to bottom if was isolated when first pressed and not held too long
                            if (rWasIsolated && !(keysHeld & ~KEY_R & ALL_KEYS_MASK)) {  // Was isolated initially and no other keys held at release
                                const u64 holdDuration = currentTime_ns - rButtonPressStart_ns;
                                if (holdDuration < INITIAL_HOLD_THRESHOLD_NS) {
                                    jumpToBottom.store(true, std::memory_order_release);
                                    currentGui->requestFocus(topElement, FocusDirection::None);
                                }
                            }
                        }
                        rKeyWasPressed = false;
                        rWasIsolated = false;
                    }
                }
                
                // Handle ZL button (skip up with hold)
                {
                    static u64 zlLastClickTime_ns = 0;
                    static bool zlKeyWasPressed = false;
                    static bool zlWasIsolated = false;  // Track if ZL was isolated when first pressed
                    static bool zlInRapidClickMode = false;
                    static u64 zlFirstClickPressStart_ns = 0;  // Track timing for first clicks only
                    
                    // Check if we should exit rapid click mode due to timeout
                    if (zlInRapidClickMode && (currentTime_ns - zlLastClickTime_ns) > RAPID_MODE_TIMEOUT_NS) {
                        zlInRapidClickMode = false;
                    }
                    
                    if (zlKeyPressed) {
                        if (!zlKeyWasPressed) {
                            // ZL key physically pressed for the first time
                            const u64 timeSinceLastClick = currentTime_ns - zlLastClickTime_ns;
                            
                            zlWasIsolated = !notzlKeyPressed;  // Remember if it started isolated
                            
                            // Track press start time for first clicks (when not in rapid mode)
                            if (!zlInRapidClickMode) {
                                zlFirstClickPressStart_ns = currentTime_ns;
                            }
                            
                            // Enter rapid click mode if clicking within window
                            if (timeSinceLastClick <= RAPID_CLICK_WINDOW_NS) {
                                zlInRapidClickMode = true;
                            }
                            
                            // Only trigger immediately if in rapid click mode AND was isolated initially
                            if (zlInRapidClickMode && zlWasIsolated) {
                                skipUp.store(true, std::memory_order_release);
                                currentGui->requestFocus(topElement, FocusDirection::None);
                                zlLastClickTime_ns = currentTime_ns;
                            }
                        }
                        
                        // Check for hold behavior - ONLY if in rapid click mode AND was isolated initially
                        if (zlInRapidClickMode && zlWasIsolated) {
                            static u64 zlButtonPressStart_ns = 0;
                            static u64 zlLastHoldTrigger_ns = 0;
                            static bool zlHoldTriggered = false;
                            
                            // Initialize on new press
                            if (!zlKeyWasPressed) {
                                zlButtonPressStart_ns = currentTime_ns;
                                zlLastHoldTrigger_ns = currentTime_ns;
                                zlHoldTriggered = false;
                            }
                            
                            const u64 holdDuration = currentTime_ns - zlButtonPressStart_ns;
                            
                            if (holdDuration >= HOLD_THRESHOLD_NS) {
                                // Calculate dynamic interval based on hold duration (accelerating)
                                const float t = (holdDuration >= ACCELERATION_POINT_NS) ? 1.0f : 
                                               (float)holdDuration / (float)ACCELERATION_POINT_NS;
                                const u64 currentInterval = ((1.0f - t) * INITIAL_INTERVAL_NS + t * FAST_INTERVAL_NS);
                                
                                const u64 timeSinceLastHoldTrigger = currentTime_ns - zlLastHoldTrigger_ns;
                                
                                if (!zlHoldTriggered || timeSinceLastHoldTrigger >= currentInterval) {
                                    // Trigger skip
                                    skipUp.store(true, std::memory_order_release);
                                    currentGui->requestFocus(topElement, FocusDirection::None);
                                    zlHoldTriggered = true;
                                    zlLastHoldTrigger_ns = currentTime_ns;
                                    zlLastClickTime_ns = currentTime_ns;  // Keep rapid mode active
                                }
                            }
                        }
                        
                        zlKeyWasPressed = true;
                    } else {
                        if (zlKeyWasPressed) {
                            // ZL key physically released - only trigger if was isolated initially and no other keys held at release
                            if (!zlInRapidClickMode && zlWasIsolated && !(keysHeld & ~KEY_ZL & ALL_KEYS_MASK)) {
                                const u64 holdDuration = currentTime_ns - zlFirstClickPressStart_ns;
                                
                                // Only trigger if not held too long
                                if (holdDuration < INITIAL_HOLD_THRESHOLD_NS) {
                                    skipUp.store(true, std::memory_order_release);
                                    currentGui->requestFocus(topElement, FocusDirection::None);
                                    zlLastClickTime_ns = currentTime_ns;
                                    zlInRapidClickMode = true;  // Enter rapid mode after first release
                                }
                            }
                        }
                        zlKeyWasPressed = false;
                        zlWasIsolated = false;
                    }
                }
                
                // Handle ZR button (skip down with hold)
                {
                    static u64 zrLastClickTime_ns = 0;
                    static bool zrKeyWasPressed = false;
                    static bool zrWasIsolated = false;  // Track if ZR was isolated when first pressed
                    static bool zrInRapidClickMode = false;
                    static u64 zrFirstClickPressStart_ns = 0;  // Track timing for first clicks only
                    
                    // Check if we should exit rapid click mode due to timeout
                    if (zrInRapidClickMode && (currentTime_ns - zrLastClickTime_ns) > RAPID_MODE_TIMEOUT_NS) {
                        zrInRapidClickMode = false;
                    }
                    
                    if (zrKeyPressed) {
                        if (!zrKeyWasPressed) {
                            // ZR key physically pressed for the first time
                            const u64 timeSinceLastClick = currentTime_ns - zrLastClickTime_ns;
                            
                            zrWasIsolated = !notzrKeyPressed;  // Remember if it started isolated
                            
                            // Track press start time for first clicks (when not in rapid mode)
                            if (!zrInRapidClickMode) {
                                zrFirstClickPressStart_ns = currentTime_ns;
                            }
                            
                            // Enter rapid click mode if clicking within window
                            if (timeSinceLastClick <= RAPID_CLICK_WINDOW_NS) {
                                zrInRapidClickMode = true;
                            }
                            
                            // Only trigger immediately if in rapid click mode AND was isolated initially
                            if (zrInRapidClickMode && zrWasIsolated) {
                                skipDown.store(true, std::memory_order_release);
                                currentGui->requestFocus(topElement, FocusDirection::None);
                                zrLastClickTime_ns = currentTime_ns;
                            }
                        }
                        
                        // Check for hold behavior - ONLY if in rapid click mode AND was isolated initially
                        if (zrInRapidClickMode && zrWasIsolated) {
                            static u64 zrButtonPressStart_ns = 0;
                            static u64 zrLastHoldTrigger_ns = 0;
                            static bool zrHoldTriggered = false;
                            
                            // Initialize on new press
                            if (!zrKeyWasPressed) {
                                zrButtonPressStart_ns = currentTime_ns;
                                zrLastHoldTrigger_ns = currentTime_ns;
                                zrHoldTriggered = false;
                            }
                            
                            const u64 holdDuration = currentTime_ns - zrButtonPressStart_ns;
                            
                            if (holdDuration >= HOLD_THRESHOLD_NS) {
                                // Calculate dynamic interval based on hold duration (accelerating)
                                const float t = (holdDuration >= ACCELERATION_POINT_NS) ? 1.0f : 
                                               (float)holdDuration / (float)ACCELERATION_POINT_NS;
                                const u64 currentInterval = ((1.0f - t) * INITIAL_INTERVAL_NS + t * FAST_INTERVAL_NS);
                                
                                const u64 timeSinceLastHoldTrigger = currentTime_ns - zrLastHoldTrigger_ns;
                                
                                if (!zrHoldTriggered || timeSinceLastHoldTrigger >= currentInterval) {
                                    // Trigger skip
                                    skipDown.store(true, std::memory_order_release);
                                    currentGui->requestFocus(topElement, FocusDirection::None);
                                    zrHoldTriggered = true;
                                    zrLastHoldTrigger_ns = currentTime_ns;
                                    zrLastClickTime_ns = currentTime_ns;  // Keep rapid mode active
                                }
                            }
                        }
                        
                        zrKeyWasPressed = true;
                    } else {
                        if (zrKeyWasPressed) {
                            // ZR key physically released - only trigger if was isolated initially and no other keys held at release
                            if (!zrInRapidClickMode && zrWasIsolated && !(keysHeld & ~KEY_ZR & ALL_KEYS_MASK)) {
                                const u64 holdDuration = currentTime_ns - zrFirstClickPressStart_ns;
                                
                                // Only trigger if not held too long
                                if (holdDuration < INITIAL_HOLD_THRESHOLD_NS) {
                                    skipDown.store(true, std::memory_order_release);
                                    currentGui->requestFocus(topElement, FocusDirection::None);
                                    zrLastClickTime_ns = currentTime_ns;
                                    zrInRapidClickMode = true;  // Enter rapid mode after first release
                                }
                            }
                        }
                        zrKeyWasPressed = false;
                        zrWasIsolated = false;
                    }
                }
            }
            
            //if (keysDown & KEY_ZL) {
            //    //while (tsl::notification && tsl::notification->isActive()) {
            //    //    tsl::notification->update(true, true); // No file ops, allow state transitions
            //    //    svcSleepThread(10'000'000); // 1ms sleep
            //    //}
            //    if (notification)
            //        notification->forceShutdown();
            //}

            
            if (!touchDetected && oldTouchDetected && currentGui && topElement) {
                topElement->onTouch(elm::TouchEvent::Release, oldTouchPos.x, oldTouchPos.y, oldTouchPos.x, oldTouchPos.y, initialTouchPos.x, initialTouchPos.y);
            }

            // Cache common calculations
            // Use consistent edge padding equal to halfGap (matching drawing code)
            const float edgePadding = ult::halfGap.load(std::memory_order_acquire) - 5;
            const float buttonStartX = edgePadding;
            
            // Calculate button positions matching the drawing code
            const float backLeftEdge = buttonStartX + ult::layerEdge;
            const float backRightEdge = backLeftEdge + ult::backWidth.load(std::memory_order_acquire);
            const float selectLeftEdge = backRightEdge;
            const float selectRightEdge = selectLeftEdge + ult::selectWidth.load(std::memory_order_acquire);
            const float nextPageLeftEdge = ult::noClickableItems.load(std::memory_order_acquire) ? backRightEdge : selectRightEdge;
            const float nextPageRightEdge = nextPageLeftEdge + ult::nextPageWidth.load(std::memory_order_acquire);
            
            const float menuRightEdge = 245.0f + ult::layerEdge - 13;
            const u32 footerY = cfg::FramebufferHeight - 73U + 1;
            static std::vector<bool> lastSimulatedTouch = {false, false, false, false};
            
            // Touch region calculations
            const bool backTouched = (touchPos.x >= backLeftEdge && touchPos.x < backRightEdge && touchPos.y > footerY) &&
                                     (initialTouchPos.x >= backLeftEdge && initialTouchPos.x < backRightEdge && initialTouchPos.y > footerY);
            
            const bool selectTouched = !ult::noClickableItems.load(std::memory_order_acquire) &&
                                       (touchPos.x >= selectLeftEdge && touchPos.x < selectRightEdge && touchPos.y > footerY) &&
                                       (initialTouchPos.x >= selectLeftEdge && initialTouchPos.x < selectRightEdge && initialTouchPos.y > footerY);
            
            const bool nextPageTouched = (touchPos.x >= nextPageLeftEdge && touchPos.x < nextPageRightEdge && touchPos.y > footerY) &&
                                          (initialTouchPos.x >= nextPageLeftEdge && initialTouchPos.x < nextPageRightEdge && initialTouchPos.y > footerY);
            
            const bool menuTouched = (touchPos.x > ult::layerEdge+7U && touchPos.x <= menuRightEdge && touchPos.y > 10U && touchPos.y <= 83U) &&
                                     (initialTouchPos.x > ult::layerEdge+7U && initialTouchPos.x <= menuRightEdge && initialTouchPos.y > 10U && initialTouchPos.y <= 83U);
            
            //ult::touchingBack.store(backTouched, std::memory_order_release);
            //ult::touchingSelect.store(selectTouched, std::memory_order_release);
            //ult::touchingNextPage.store(nextPageTouched, std::memory_order_release);
            //ult::touchingMenu.store(menuTouched, std::memory_order_release);

            // Only update and trigger rumble on state changes
            bool shouldTriggerRumble = false;
            
            if (backTouched != ult::touchingBack.exchange(backTouched, std::memory_order_acq_rel)) {
                if (backTouched) shouldTriggerRumble = true;
            }
            
            if (selectTouched != ult::touchingSelect.exchange(selectTouched, std::memory_order_acq_rel)) {
                if (selectTouched) shouldTriggerRumble = true;
            }
            
            if (nextPageTouched != ult::touchingNextPage.exchange(nextPageTouched, std::memory_order_acq_rel)) {
                if (nextPageTouched) shouldTriggerRumble = true;
            }
            
            if (menuTouched != ult::touchingMenu.exchange(menuTouched, std::memory_order_acq_rel)) {
                if (menuTouched && ult::inMainMenu.load(std::memory_order_acquire)) shouldTriggerRumble = true;
            }
            
            if (shouldTriggerRumble) {
                triggerRumbleClick.store(true, std::memory_order_release);
            }

            
            if (touchDetected) {
                // Update lastSimulatedTouch with current touch states
                lastSimulatedTouch = {
                    backTouched, 
                    selectTouched, 
                    nextPageTouched, 
                    menuTouched
                };

                ult::interruptedTouch.store(((keysHeld & ALL_KEYS_MASK) != 0), std::memory_order_release);
            
                const u32 xDistance = std::abs(static_cast<s32>(initialTouchPos.x) - static_cast<s32>(touchPos.x));
                const u32 yDistance = std::abs(static_cast<s32>(initialTouchPos.y) - static_cast<s32>(touchPos.y));
                
                const bool isScroll = (xDistance * xDistance + yDistance * yDistance) > 1000;
                if (isScroll) {
                    elm::Element::setInputMode(InputMode::TouchScroll);
                    touchEvent = elm::TouchEvent::Scroll;
                } else {
                    if (touchEvent != elm::TouchEvent::Scroll) {
                        touchEvent = elm::TouchEvent::Hold;
                    }
                }
                
                if (!oldTouchDetected) {
                    initialTouchPos = touchPos;
                    elm::Element::setInputMode(InputMode::Touch);
                    if (!interpreterIsRunning) {
                        ult::touchInBounds = (initialTouchPos.y <= footerY && initialTouchPos.y > 73U && 
                                            initialTouchPos.x <= ult::layerEdge + cfg::FramebufferWidth - 30U && 
                                            initialTouchPos.x > 40U + ult::layerEdge);
                        if (ult::touchInBounds) {
                            triggerRumbleClick.store(true, std::memory_order_release);
                            currentGui->removeFocus();
                        }
                    }
                    touchEvent = elm::TouchEvent::Touch;
                }
                
                if (currentGui && topElement && !interpreterIsRunning) {
                    topElement->onTouch(touchEvent, touchPos.x, touchPos.y, oldTouchPos.x, oldTouchPos.y, initialTouchPos.x, initialTouchPos.y);
                    if (touchPos.x > 40U + ult::layerEdge && touchPos.x <= cfg::FramebufferWidth - 30U + ult::layerEdge && 
                        touchPos.y > 73U && touchPos.y <= footerY) {
                        currentGui->removeFocus();
                    }
                }
                
                oldTouchPos = touchPos;
                if ((touchPos.x < ult::layerEdge || touchPos.x > cfg::FramebufferWidth + ult::layerEdge) && tsl::elm::Element::getInputMode() == tsl::InputMode::Touch) {
                    oldTouchPos = { 0 };
                    initialTouchPos = { 0 };
            #if IS_STATUS_MONITOR_DIRECTIVE
                    if (FullMode && !deactivateOriginalFooter) {
                        this->hide();
                    }
            #else
                    this->hide();
            #endif
                }
                ult::stillTouching.store(true, std::memory_order_release);
            } else {
                // Process touch release using stored touch states - no need to recalculate boundaries
                for (int i = 0; i < 4; ++i) {
                    if (lastSimulatedTouch[i]) {
                        if (!ult::interruptedTouch.load(std::memory_order_acquire) && !interpreterIsRunning) {
                            switch (i) {
                                case 0: // Back button
                                    ult::simulatedBack.store(true, std::memory_order_release);
                                    break;
                                case 1: // Select button
                                    ult::simulatedSelect.store(true, std::memory_order_release);
                                    break;
                                case 2: // Next page button
                                    ult::simulatedNextPage.store(true, std::memory_order_release);
                                    break;
                                case 3: // Menu button
                                    ult::simulatedMenu.store(true, std::memory_order_release);
                                    break;
                            }
                        } else if (interpreterIsRunning) {
                            switch (i) {
                                case 0: // Back button when interpreter is running
                                    this->hide();
                                    break;
                                case 1: // Select button when interpreter is running
                                    ult::externalAbortCommands.store(true, std::memory_order_release);
                                    break;
                                // cases 2 and 3 don't have interpreter running logic in original code
                            }
                        }
                    }
                }
                
                // Update lastSimulatedTouch with current touch states
                lastSimulatedTouch = {
                    false, 
                    false, 
                    false, 
                    false
                };

                elm::Element::setInputMode(InputMode::Controller);
                
                oldTouchPos = { 0 };
                initialTouchPos = { 0 };
                touchEvent = elm::TouchEvent::None;
                ult::stillTouching.store(false, std::memory_order_release);
                ult::interruptedTouch.store(false, std::memory_order_release);
            }
            
            oldTouchDetected = touchDetected;
            oldTouchEvent = touchEvent;

        }
        

        /**
         * @brief Clears the screen
         *
         */
        void clearScreen() {
            auto& renderer = gfx::Renderer::get();
            
            renderer.startFrame();
            renderer.clearScreen();
            renderer.endFrame();
        }
        
        /**
         * @brief Reset hide and close flags that were previously set by \ref Overlay::close() or \ref Overlay::hide()
         *
         */
        void resetFlags() {
            this->m_shouldHide = false;
            this->m_shouldClose = false;
            this->m_shouldCloseAfter = false;
        }
        
        /**
         * @brief Disables the next animation that would play
         *
         */
        void disableNextAnimation() {
            this->m_disableNextAnimation = true;
        }
        

        /**
         * @brief Changes to a different Gui
         *
         * @param gui Gui to change to
         * @return Reference to the Gui
         */
        std::unique_ptr<tsl::Gui>& changeTo(std::unique_ptr<tsl::Gui>&& gui, bool clearGlyphCache = false) {
            if (this->m_guiStack.top() != nullptr && this->m_guiStack.top()->m_focusedElement != nullptr)
                this->m_guiStack.top()->m_focusedElement->resetClickAnimation();
            
            isNavigatingBackwards.store(false, std::memory_order_release);

            // cache frame for forward rendering using external list method (to be implemented)

            // Create the top element of the new Gui
            gui->m_topElement = gui->createUI();

            
            // Push the new Gui onto the stack
            this->m_guiStack.push(std::move(gui));
            //if (clearGlyphCache)
            //    tsl::gfx::FontManager::clearCache();
            return this->m_guiStack.top();
        }

        
        /**
         * @brief Creates a new Gui and changes to it
         *
         * @tparam G Gui to create
         * @tparam Args Arguments to pass to the Gui
         * @param args Arguments to pass to the Gui
         * @return Reference to the newly created Gui
         */
        // Template version without clearGlyphCache (for backward compatibility)
        template<typename G, typename ...Args>
        std::unique_ptr<tsl::Gui>& changeTo(Args&&... args) {
            return this->changeTo(std::make_unique<G>(std::forward<Args>(args)...), false);
        }
        

        /**
         * @brief Swaps to a different Gui
         *
         * @param gui Gui to change to
         * @return Reference to the Gui
         */
        std::unique_ptr<tsl::Gui>& swapTo(std::unique_ptr<tsl::Gui>&& gui, u32 count = 1) {
            //isNavigatingBackwards = true;
            
            isNavigatingBackwards.store(true, std::memory_order_release);
            
            // Clamp count to available stack size to prevent underflow
            const u32 actualCount = std::min(count, static_cast<u32>(this->m_guiStack.size()));
            
            if (actualCount > 1) {
                tsl::elm::skipDeconstruction.store(true, std::memory_order_release);
                // Pop the specified number of GUIs
                for (u32 i = 0; i < actualCount; ++i) {
                    this->m_guiStack.pop();
                }
                tsl::elm::skipDeconstruction.store(false, std::memory_order_release);
            } else {
                this->m_guiStack.pop();
            }



            if (this->m_guiStack.top() != nullptr && this->m_guiStack.top()->m_focusedElement != nullptr)
                this->m_guiStack.top()->m_focusedElement->resetClickAnimation();
            
            isNavigatingBackwards.store(false, std::memory_order_release);

            // cache frame for forward rendering using external list method (to be implemented)

            // Create the top element of the new Gui
            gui->m_topElement = gui->createUI();

            
            // Push the new Gui onto the stack
            this->m_guiStack.push(std::move(gui));
            //if (clearGlyphCache)
            //    tsl::gfx::FontManager::clearCache();
            return this->m_guiStack.top();
        }

        /**
         * @brief Creates a new Gui and changes to it
         *
         * @tparam G Gui to create
         * @tparam Args Arguments to pass to the Gui
         * @param args Arguments to pass to the Gui
         * @return Reference to the newly created Gui
         */
        // Template version without clearGlyphCache (for backward compatibility)
        template<typename G, typename ...Args>
        std::unique_ptr<tsl::Gui>& swapTo(SwapDepth depth, Args&&... args) {
            return this->swapTo(std::make_unique<G>(std::forward<Args>(args)...), depth.value);
        }
        
        template<typename G, typename ...Args>
        std::unique_ptr<tsl::Gui>& swapTo(Args&&... args) {
            return this->swapTo(std::make_unique<G>(std::forward<Args>(args)...), 1);
        }
        
        /**
         * @brief Pops the top Gui(s) from the stack and goes back count number of times
         * @param count Number of Guis to pop from the stack (default: 1)
         * @note The Overlay gets closed once there are no more Guis on the stack
         */
        void goBack(u32 count = 1) {
            tsl::elm::g_disableMenuCacheOnReturn.store(true, std::memory_order_release);

            // If there is exactly one GUI and an active notification, handle that first
            if (this->m_guiStack.size() == 1 && notification && notification->isActive()) {
                this->close(); 
                return;
            }
        
            isNavigatingBackwards.store(true, std::memory_order_release);
        
            // Clamp count to available stack size to prevent underflow
            const u32 actualCount = std::min(count, static_cast<u32>(this->m_guiStack.size()));
        
            // Special case: if we don't close on exit and popping everything would leave us with 0 or 1 GUI
            if (!this->m_closeOnExit && this->m_guiStack.size() <= actualCount) {
                this->hide();
                return;
            }
        
            if (actualCount > 1)
                tsl::elm::skipDeconstruction.store(true, std::memory_order_release);
        
            // Pop the specified number of GUIs
            for (u32 i = 0; i < actualCount && !this->m_guiStack.empty(); ++i) {
                this->m_guiStack.pop();
            }
        
            tsl::elm::skipDeconstruction.exchange(false, std::memory_order_acq_rel);
            
            // Close overlay if stack is empty
            if (this->m_guiStack.empty()) {
                this->close();
            } else {
                //triggerRumbleDoubleClick.store(true, std::memory_order_release);
                //triggerExitSound.store(true, std::memory_order_release);
            }

        }

        void pop(u32 count = 1) {
            isNavigatingBackwards.store(true, std::memory_order_release);
            
            // Clamp count to available stack size to prevent underflow
            const u32 actualCount = std::min(count, static_cast<u32>(this->m_guiStack.size()));

            if (actualCount > 1) {
                tsl::elm::skipDeconstruction.store(true, std::memory_order_release);
                // Pop the specified number of GUIs
                for (u32 i = 0; i < actualCount; ++i) {
                    this->m_guiStack.pop();
                }
                tsl::elm::skipDeconstruction.store(false, std::memory_order_release);
            } else {
                this->m_guiStack.pop();
            }
        }


        
        template<typename G, typename ...Args>
        friend std::unique_ptr<tsl::Gui>& changeTo(Args&&... args);
        template<typename G, typename ...Args>
        friend std::unique_ptr<tsl::Gui>& swapTo(Args&&... args);
        
        template<typename G, typename ...Args>
        friend std::unique_ptr<tsl::Gui>& swapTo(SwapDepth depth, Args&&... args);
        
        friend void goBack(u32 count);
        friend void pop(u32 count);
        
        template<typename, tsl::impl::LaunchFlags>
        friend int loop(int argc, char** argv);
        
        friend class tsl::Gui;
    };
    
    
    namespace impl {
        static constexpr const char* TESLA_CONFIG_FILE = "/config/tesla/config.ini";
        static constexpr const char* ULTRAHAND_CONFIG_FILE = "/config/ultrahand/config.ini";
        
        /**
         * @brief Data shared between the different ult::renderThreads
         *
         */
        struct SharedThreadData {
            std::atomic<bool> running = false;
            
            Event comboEvent = { 0 };
            
            std::atomic<bool> overlayOpen = false;
            
            std::mutex dataMutex;
            u64 keysDown = 0;
            u64 keysDownPending = 0;
            u64 keysHeld = 0;
            HidTouchScreenState touchState = { 0 };
            HidAnalogStickState joyStickPosLeft = { 0 }, joyStickPosRight = { 0 };
        };
        
        
        /**
         * @brief Extract values from Tesla settings file
         *
         */
        static void parseOverlaySettings() {
            hlp::ini::IniData parsedConfig = hlp::ini::readOverlaySettings(ULTRAHAND_CONFIG_FILE);
            
            u64 decodedKeys = hlp::comboStringToKeys(parsedConfig[ult::ULTRAHAND_PROJECT_NAME][ult::KEY_COMBO_STR]); // CUSTOM MODIFICATION
            if (decodedKeys)
                tsl::cfg::launchCombo = decodedKeys;
            else {
                parsedConfig = hlp::ini::readOverlaySettings(TESLA_CONFIG_FILE);
                decodedKeys = hlp::comboStringToKeys(parsedConfig["tesla"][ult::KEY_COMBO_STR]);
                if (decodedKeys)
                    tsl::cfg::launchCombo = decodedKeys;
            }
            
            //#if USING_WIDGET_DIRECTIVE
            ult::datetimeFormat = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["datetime_format"]; // read datetime_format
            ult::removeQuotes(ult::datetimeFormat);
            if (ult::datetimeFormat.empty()) {
                ult::datetimeFormat = ult::DEFAULT_DT_FORMAT;
                ult::removeQuotes(ult::datetimeFormat);
            }


            std::string tempStr;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_clock"];
            ult::removeQuotes(tempStr);
            ult::hideClock = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_battery"];
            ult::removeQuotes(tempStr);
            ult::hideBattery = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_pcb_temp"];
            ult::removeQuotes(tempStr);
            ult::hidePCBTemp = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_soc_temp"];
            ult::removeQuotes(tempStr);
            ult::hideSOCTemp = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["dynamic_widget_colors"];
            ult::removeQuotes(tempStr);
            ult::dynamicWidgetColors = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_widget_backdrop"];
            ult::removeQuotes(tempStr);
            ult::hideWidgetBackdrop = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["center_widget_alignment"];
            ult::removeQuotes(tempStr);
            ult::centerWidgetAlignment = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["extended_widget_backdrop"];
            ult::removeQuotes(tempStr);
            ult::extendedWidgetBackdrop = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["dynamic_logo"];
            ult::removeQuotes(tempStr);
            ult::useDynamicLogo = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["selection_bg"];
            ult::removeQuotes(tempStr);
            ult::useSelectionBG = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["selection_text"];
            ult::removeQuotes(tempStr);
            ult::useSelectionText = tempStr != ult::FALSE_STR;
            
            tempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["selection_value"];
            ult::removeQuotes(tempStr);
            ult::useSelectionValue = tempStr != ult::FALSE_STR;

            //#endif
            
        }

        /**
         * @brief Update and save launch combo keys
         *
         * @param keys the new combo keys
         */
        [[maybe_unused]] static void updateCombo(u64 keys) {
            tsl::cfg::launchCombo = keys;
            hlp::ini::updateOverlaySettings({
                { ult::TESLA_STR, { // CUSTOM MODIFICATION
                    { ult::KEY_COMBO_STR , tsl::hlp::keysToComboString(keys) }
                }}
            }, TESLA_CONFIG_FILE);
            hlp::ini::updateOverlaySettings({
                { ult::ULTRAHAND_PROJECT_NAME, { // CUSTOM MODIFICATION
                    { ult::KEY_COMBO_STR , tsl::hlp::keysToComboString(keys) }
                }}
            }, ULTRAHAND_CONFIG_FILE);
        }
        
        static auto currentUnderscanPixels = std::make_pair(0, 0);

        /**
         * @brief Background event polling loop thread
         *
         * @param args Used to pass in a pointer to a \ref SharedThreadData struct
         */
        static void backgroundEventPoller(void *args) {
            requiresLNY2 = amsVersionAtLeast(1,10,0);     // Detect if using HOS 21+

            // Initialize the audio service
            if (ult::useSoundEffects && ult::expandedMemory) {
                ult::AudioPlayer::initialize();
            }

            tsl::hlp::loadEntryKeyCombos();
            ult::launchingOverlay.store(false, std::memory_order_release);
        
            SharedThreadData *shData = static_cast<SharedThreadData*>(args);
            
            // To prevent focus glitchout, close the overlay immediately when the home button gets pressed
            Event homeButtonPressEvent = {};
            hidsysAcquireHomeButtonEventHandle(&homeButtonPressEvent, false);
            eventClear(&homeButtonPressEvent);
            tsl::hlp::ScopeGuard homeButtonEventGuard([&] { eventClose(&homeButtonPressEvent); });
            
            // To prevent focus glitchout, close the overlay immediately when the power button gets pressed
            Event powerButtonPressEvent = {};
            hidsysAcquireSleepButtonEventHandle(&powerButtonPressEvent, false);
            eventClear(&powerButtonPressEvent);
            tsl::hlp::ScopeGuard powerButtonEventGuard([&] { eventClose(&powerButtonPressEvent); });
            
        
            // For handling screenshots color alpha
            Event captureButtonPressEvent = {};
            hidsysAcquireCaptureButtonEventHandle(&captureButtonPressEvent, false);
            eventClear(&captureButtonPressEvent);
            hidsysAcquireCaptureButtonEventHandle(&captureButtonPressEvent, false);
            eventClear(&captureButtonPressEvent);
            tsl::hlp::ScopeGuard captureButtonEventGuard([&] { eventClose(&captureButtonPressEvent); });
        
            // Parse Tesla settings
            impl::parseOverlaySettings();
            
        
            // Allow only Player 1 and handheld mode
            HidNpadIdType id_list[2] = { HidNpadIdType_No1, HidNpadIdType_Handheld };
            
            // Configure HID system to only listen to these IDs
            hidSetSupportedNpadIdType(id_list, 2);
            
            // Configure input for up to 2 supported controllers (P1 + Handheld)
            padConfigureInput(2, HidNpadStyleSet_NpadStandard | HidNpadStyleTag_NpadSystemExt);
            
            // Initialize separate pad states for both controllers
            PadState pad_p1;
            PadState pad_handheld;
            padInitialize(&pad_p1, HidNpadIdType_No1);
            padInitialize(&pad_handheld, HidNpadIdType_Handheld);
            
            // Touch screen init
            hidInitializeTouchScreen();
            
            // Clear any stale input from both controllers
            padUpdate(&pad_p1);
            padUpdate(&pad_handheld);

            //ult::initRumble(); // initialize rumble
            
            enum WaiterObject {
                WaiterObject_HomeButton,
                WaiterObject_PowerButton,
                WaiterObject_CaptureButton,
                WaiterObject_Count
            };
            
            // Construct waiter
            Waiter objects[3] = {
                [WaiterObject_HomeButton] = waiterForEvent(&homeButtonPressEvent),
                [WaiterObject_PowerButton] = waiterForEvent(&powerButtonPressEvent),
                [WaiterObject_CaptureButton] = waiterForEvent(&captureButtonPressEvent),
            };
            
            u64 currentTouchTick = 0;
            auto lastTouchX = 0;
            auto lastTouchY = 0;
        
            // Preset touch boundaries
            constexpr int SWIPE_RIGHT_BOUND = 16;  // 16 + 80
            constexpr int SWIPE_LEFT_BOUND = (1280 - 16);
            constexpr u64 TOUCH_THRESHOLD_NS = 150'000'000ULL; // 150ms in nanoseconds
            constexpr u64 FAST_SWAP_THRESHOLD_NS = 150'000'000ULL;
        
            // Global underscan monitoring - run at most once every 300ms
            auto lastUnderscanPixels = std::make_pair(0, 0);
            bool firstUnderscanCheck = true;
            u64 lastUnderscanCheckNs = 0;  // store last execution in nanoseconds
            constexpr u64 UNDERSCAN_INTERVAL_NS = 300'000'000ULL; // 300ms in ns

            s32 idx;
            Result rc;
            
            std::string currentTitleID;
        
            u64 lastPollTick = 0;
            u64 resetStartTick = armGetSystemTick();
            const u64 startNs = armTicksToNs(resetStartTick);

            ult::lastTitleID = ult::getTitleIdAsString();
        
            //u64 elapsedTime_ns;

            // Notification variables
            u64 lastNotifCheck = 0;
            std::vector<std::string> shownFiles;
            std::string text;
            int fontSize;
            int priority;
            time_t creationTime;

            
            
            while (shData->running.load(std::memory_order_acquire)) {

                const u64 nowTick = armGetSystemTick();
                const u64 nowNs = armTicksToNs(nowTick);
                
                
                
                // Scan for input changes from both controllers
                padUpdate(&pad_p1);
                padUpdate(&pad_handheld);
                
                // Read in HID values
                {

                    // Poll Title ID every 1 seconds
                    if (!ult::resetForegroundCheck.load(std::memory_order_acquire)) {
                        const u64 elapsedNs = armTicksToNs(nowTick - lastPollTick);
                        if (elapsedNs >= 1'000'000'000ULL) {
                            lastPollTick = nowTick;
                            
                            currentTitleID = ult::getTitleIdAsString();
                            if (currentTitleID != ult::lastTitleID) {
                                ult::lastTitleID = currentTitleID;
                                ult::resetForegroundCheck.store(true, std::memory_order_release);
                                resetStartTick = nowTick;
                            }
                        }
                    }

                    // If a reset is scheduled, trigger after 3.5s delay
                    if (ult::resetForegroundCheck.load(std::memory_order_acquire)) {
                        const u64 resetElapsedNs = armTicksToNs(nowTick - resetStartTick);
                        if (resetElapsedNs >= 3'500'000'000ULL) {
                            if (shData->overlayOpen && ult::currentForeground.load(std::memory_order_acquire)) {
                                #if IS_STATUS_MONITOR_DIRECTIVE
                                if (!isValidOverlayMode())
                                    hlp::requestForeground(true, false);
                                #else
                                hlp::requestForeground(true, false);
                                #endif
                            }
                            ult::resetForegroundCheck.store(false, std::memory_order_release);
                        }
                    }
                    
                    if (firstUnderscanCheck || (nowNs - lastUnderscanCheckNs) >= UNDERSCAN_INTERVAL_NS) {
                        currentUnderscanPixels = tsl::gfx::getUnderscanPixels();
                    
                        if (firstUnderscanCheck || currentUnderscanPixels != lastUnderscanPixels) {
                            // Update layer dimensions without destroying state
                            tsl::gfx::Renderer::get().updateLayerSize();
                            
                            lastUnderscanPixels = currentUnderscanPixels;
                            firstUnderscanCheck = false;
                        }
                    
                        lastUnderscanCheckNs = nowNs;
                    }
    
                    //bool expected = true;
                    //if (fireNotificationEvent.compare_exchange_strong(expected, false, std::memory_order_acq_rel)) {
                    //    if (ult::launchingOverlay.load(std::memory_order_acquire))
                    //        return;
                    //    eventFire(&shData->notificationEvent);  // wake the loop
                    //}
    
                    // Process notification files every 300ms
                    {
                        std::lock_guard<std::mutex> jsonLock(notificationJsonMutex);
                        
                        if (armTicksToNs(nowTick - lastNotifCheck) >= 300'000'000ULL) {
                            lastNotifCheck = nowTick;
                            
                            DIR* dir = opendir(ult::NOTIFICATIONS_PATH.c_str());
                            if (dir) {
                                
                                if (ult::useNotifications) {
                                    const std::string& notifPath = ult::NOTIFICATIONS_PATH;
                                    
                                    // --- Prune missing files from shownFiles ---
                                    for (auto it = shownFiles.begin(); it != shownFiles.end();) {
                                        const std::string fullPath = notifPath + *it;
                                        if (access(fullPath.c_str(), F_OK) != 0) {
                                            it = shownFiles.erase(it);
                                        } else {
                                            ++it;
                                        }
                                    }
                                    
                                    // Reuse existing variables - track best file as we scan
                                    static std::string bestFilename;
                                    static std::string bestFullPath;
                                    static time_t bestCreationTime;
                                    static int bestPriority;
                                    
                                    bestFilename.clear();
                                    bestFullPath.clear();
                                    bestPriority = -1;
                                    bestCreationTime = 0;
                                    bool foundAny = false;
                                    
                                    struct dirent* entry;
                                    
                                    // --- Find the best notification file in one pass ---
                                    while ((entry = readdir(dir)) != nullptr) {
                                        if (entry->d_type != DT_REG) continue;
                                        
                                        const char* fname = entry->d_name;
                                        const size_t filenameLen = strlen(fname);
                                        
                                        // Must end with ".notify"
                                        if (filenameLen <= 7 || strcmp(fname + filenameLen - 7, ".notify") != 0)
                                            continue;
                                        
                                        // Skip if already shown
                                        if (std::find(shownFiles.begin(), shownFiles.end(), fname) != shownFiles.end())
                                            continue;
                                        
                                        // --- Build path ---
                                        static std::string fullPath;
                                        fullPath = notifPath;
                                        fullPath += fname;
                                        
                                        // --- Get file creation/modification time ---
                                        struct stat fileStat;
                                        creationTime = 0;
                                        if (stat(fullPath.c_str(), &fileStat) == 0) {
                                            creationTime = fileStat.st_mtime;
                                        }
                                        
                                        // --- Read priority from JSON ---
                                        priority = 20; // default (reuse existing variable)
                                        std::unique_ptr<ult::json_t, ult::JsonDeleter> root(
                                            ult::readJsonFromFile(fullPath), ult::JsonDeleter());
                                        if (root) {
                                            cJSON* croot = reinterpret_cast<cJSON*>(root.get());
                                            cJSON* priorityObj = cJSON_GetObjectItemCaseSensitive(croot, "priority");
                                            if (priorityObj && cJSON_IsNumber(priorityObj)) {
                                                priority = static_cast<int>(priorityObj->valuedouble);
                                            }
                                        }
                                        
                                        // --- Is this better than current best? ---
                                        const bool isBetter = !foundAny || 
                                                       (priority > bestPriority) ||
                                                       (priority == bestPriority && creationTime < bestCreationTime);
                                        
                                        if (isBetter) {
                                            bestFilename = fname;
                                            bestFullPath = fullPath;
                                            bestCreationTime = creationTime;
                                            bestPriority = priority;
                                            foundAny = true;
                                        }
                                    }
                                    
                                    closedir(dir);
                                    
                                    // --- Process the best file ---
                                    if (foundAny) {
                                        text = ult::getStringFromJsonFile(bestFullPath, "text");
                                        if (!text.empty()) {
                                            fontSize = 28; // default (reuse existing variable)
                                            
                                            std::unique_ptr<ult::json_t, ult::JsonDeleter> root(
                                                ult::readJsonFromFile(bestFullPath), ult::JsonDeleter());
                                            if (root) {
                                                cJSON* croot = reinterpret_cast<cJSON*>(root.get());
                                                cJSON* fontSizeObj = cJSON_GetObjectItemCaseSensitive(croot, "font_size");
                                                if (fontSizeObj && cJSON_IsNumber(fontSizeObj)) {
                                                    fontSize = std::clamp(static_cast<int>(fontSizeObj->valuedouble), 1, 34);
                                                }
                                            }
                                            
                                            // --- Show notification safely ---
                                            if (notification) {
                                                notification->show(text, fontSize, bestPriority, bestFilename);
                                            }
                                            
                                            // Mark file as shown
                                            shownFiles.push_back(bestFilename);
                                        }
                                    }
                                    
                                } else {
                                    // --- Notifications disabled: delete all files ---
                                    struct dirent* entry;
                                    static std::string fullPath;
                                    
                                    while ((entry = readdir(dir)) != nullptr) {
                                        if (entry->d_type != DT_REG) continue;
                                        
                                        const char* fname = entry->d_name;
                                        const size_t len = strlen(fname);
                                        
                                        if (len > 7 && strcmp(fname + len - 7, ".notify") == 0) {
                                            fullPath.clear();
                                            fullPath = ult::NOTIFICATIONS_PATH;
                                            fullPath.append(fname, len);
                                            
                                            remove(fullPath.c_str());
                                        }
                                    }
                                    closedir(dir);
                                }
                            }
                        }
                    }


                    if (ult::launchingOverlay.load(std::memory_order_acquire))
                        break;
                    std::scoped_lock lock(shData->dataMutex);
                    if (ult::launchingOverlay.load(std::memory_order_acquire))
                        break;

                    // Flush any pending rumble triggers when feedback is off
                    if (!ult::useHapticFeedback) {
                        triggerRumbleClick.exchange(false, std::memory_order_acq_rel);
                        triggerRumbleDoubleClick.exchange(false, std::memory_order_acq_rel);
                    } else {
                        ult::checkAndReinitRumble();
                        
                        if (triggerRumbleDoubleClick.exchange(false)) {
                            if (!ult::doubleClickActive.load(std::memory_order_acquire)) {
                                ult::rumbleDoubleClick();
                            }
                            triggerRumbleClick.exchange(false);
                        } else if (triggerRumbleClick.exchange(false)) {
                            ult::rumbleClick();
                        }
                        
                        //const u64 _nowNs = armTicksToNs(armGetSystemTick());

                        ult::processRumbleStop(nowNs);
                        ult::processRumbleDoubleClick(nowNs);
                    }
                    
                    // Flush any pending sound triggers when effects are off

                    if (ult::expandedMemory) {
                        if (!ult::useSoundEffects || disableSound.load(std::memory_order_acquire)) {
                            triggerNavigationSound.exchange(false, std::memory_order_acq_rel);
                            triggerEnterSound.exchange(false, std::memory_order_acq_rel);
                            triggerExitSound.exchange(false, std::memory_order_acq_rel);
                            triggerWallSound.exchange(false, std::memory_order_acq_rel);
                            triggerOnSound.exchange(false, std::memory_order_acq_rel);
                            triggerOffSound.exchange(false, std::memory_order_acq_rel);
                            triggerSettingsSound.exchange(false, std::memory_order_acq_rel);
                            triggerMoveSound.exchange(false, std::memory_order_acq_rel);
                        } else {
                            if (reloadSoundCacheNow.exchange(false, std::memory_order_acq_rel)) {
                                ult::AudioPlayer::reloadAllSounds();
                            }

                            if (triggerNavigationSound.exchange(false)) {
                                ult::AudioPlayer::playNavigateSound();
                            } else if (triggerEnterSound.exchange(false)) {
                                ult::AudioPlayer::playEnterSound();
                            } else if (triggerExitSound.exchange(false)) {
                                ult::AudioPlayer::playExitSound();
                            } else if (triggerWallSound.exchange(false)) {
                                ult::AudioPlayer::playWallSound();
                            } else if (triggerOnSound.exchange(false)) {
                                ult::AudioPlayer::playOnSound();
                            } else if (triggerOffSound.exchange(false)) {
                                ult::AudioPlayer::playOffSound();
                            } else if (triggerSettingsSound.exchange(false)) {
                                ult::AudioPlayer::playSettingsSound();
                            } else if (triggerMoveSound.exchange(false)) {
                                ult::AudioPlayer::playMoveSound();
                            }

                            //if (clearSoundCacheNow.exchange(false, std::memory_order_acq_rel)) {
                            //    //ult::AudioPlayer::unloadAllSounds(ult::AudioPlayer::SoundType::Wall);
                            //    ult::AudioPlayer::unloadAllSounds({ult::AudioPlayer::SoundType::Wall});
                            //    //ult::AudioPlayer::unloadAllSounds();
                            //    //clearSoundCacheNow.store(false, std::memory_order_release);
                            //    clearSoundCacheNow.notify_all();
                            //}
                        }
                    }

                    //else if (triggerNavigationSound.exchange(false)) {
                    //    ult::AudioPlayer::playSlideSound();
                    //}
                    
                    // Combine inputs from both controllers
                    const u64 kDown_p1 = padGetButtonsDown(&pad_p1);
                    const u64 kHeld_p1 = padGetButtons(&pad_p1);
                    const u64 kDown_handheld = padGetButtonsDown(&pad_handheld);
                    const u64 kHeld_handheld = padGetButtons(&pad_handheld);
                    
                    shData->keysDown = kDown_p1 | kDown_handheld;
                    shData->keysHeld = kHeld_p1 | kHeld_handheld;
                    
                    // For joysticks, prioritize handheld if available, otherwise use P1
                    const HidAnalogStickState leftStick_handheld = padGetStickPos(&pad_handheld, 0);
                    const HidAnalogStickState rightStick_handheld = padGetStickPos(&pad_handheld, 1);
                    
                    // Check if handheld has any stick input (not at center position)
                    const bool handheldHasInput = (leftStick_handheld.x != 0 || leftStick_handheld.y != 0 || 
                                                  rightStick_handheld.x != 0 || rightStick_handheld.y != 0);
                    
                    if (handheldHasInput) {
                        shData->joyStickPosLeft = leftStick_handheld;
                        shData->joyStickPosRight = rightStick_handheld;
                    } else {
                        shData->joyStickPosLeft = padGetStickPos(&pad_p1, 0);
                        shData->joyStickPosRight = padGetStickPos(&pad_p1, 1);
                    }
                    
                    
                    // Read in touch positions
                    if (hidGetTouchScreenStates(&shData->touchState, 1) > 0) { // Check if any touch event is present
                        if (!shData->overlayOpen) {
                            //ult::internalTouchReleased = false;
                            ult::internalTouchReleased.store(false, std::memory_order_release);
                        }
        
                        const HidTouchState& currentTouch = shData->touchState.touches[0];  // Correct type is HidTouchPoint
                        
                        
                        const u64 elapsedTime_ns = armTicksToNs(nowTick - currentTouchTick);
                        
                        // Check if the touch is within bounds for left-to-right swipe within the time window
                        if (ult::useSwipeToOpen && elapsedTime_ns <= TOUCH_THRESHOLD_NS) {
                            if ((lastTouchX != 0 && lastTouchY != 0) && (currentTouch.x != 0 || currentTouch.y != 0)) {
                                if (ult::layerEdge == 0 && currentTouch.x > SWIPE_RIGHT_BOUND + 84 && lastTouchX <= SWIPE_RIGHT_BOUND) {
                                    eventFire(&shData->comboEvent);
                                    mainComboHasTriggered.store(true, std::memory_order_release);
                                }
                                // Check if the touch is within bounds for right-to-left swipe within the time window
                                else if (ult::layerEdge > 0 && currentTouch.x < SWIPE_LEFT_BOUND - 84 && lastTouchX >= SWIPE_LEFT_BOUND) {
                                    eventFire(&shData->comboEvent);
                                    mainComboHasTriggered.store(true, std::memory_order_release);
                                }
                            }
                        }
                    
                        // Handle touch release state
                        if (currentTouch.x == 0 && currentTouch.y == 0) {
                            ult::internalTouchReleased.store(true, std::memory_order_release);
                            //ult::internalTouchReleased = true;  // Indicate that the touch has been released
                            //ult::internalTouchReleased.store(true, std::memory_order_release);
                            lastTouchX = 0;
                            lastTouchY = 0;
                        }
        
                        // If this is the first touch of a gesture, store lastTouchX
                        else if ((lastTouchX == 0 && lastTouchY == 0) && (currentTouch.x != 0 || currentTouch.y != 0)) {
                            currentTouchTick = nowTick;
                            lastTouchX = currentTouch.x;
                            lastTouchY = currentTouch.y;
                        }
        
                    } else {
                        // Reset touch state if no touch is present
                        shData->touchState = { 0 };
                        //ult::internalTouchReleased = true;
                        ult::internalTouchReleased.store(true, std::memory_order_release);
                        //ult::internalTouchReleased.store(true, std::memory_order_release);
                    
                        // Reset touch history to invalid state
                        lastTouchX = 0;
                        lastTouchY = 0;
                    
                        // Reset time tracking
                        //currentTouchTick = nowTick;
                    }

                    #if IS_STATUS_MONITOR_DIRECTIVE
                    if (triggerExitNow) {
                        ult::launchingOverlay.store(true, std::memory_order_release);
                        ult::setIniFileValue(
                            ult::ULTRAHAND_CONFIG_INI_PATH,
                            ult::ULTRAHAND_PROJECT_NAME,
                            ult::IN_OVERLAY_STR,
                            ult::FALSE_STR
                        );
                        tsl::setNextOverlay(
                            ult::OVERLAY_PATH + "ovlmenu.ovl"
                        );
                        tsl::Overlay::get()->close();
                        //tsl::goBack();
                        //triggerExitNow = false;
                        break;
                    }
                    #endif
                    //if ((shData->keysDown & KEY_ZL && shData->keysHeld & KEY_L) || (shData->keysDown & KEY_L && shData->keysHeld & KEY_ZL)) {
                    //    notification->show("Hello world! ¯\\_(ツ)_/¯");
                    //    eventFire(&shData->notificationEvent);  // wake the loop
                    //}
                    
                    
                    // Check main launch combo first (highest priority)
                    if ((((shData->keysHeld & tsl::cfg::launchCombo) == tsl::cfg::launchCombo) && shData->keysDown & tsl::cfg::launchCombo)) {
                    #if IS_LAUNCHER_DIRECTIVE
                        if (ult::updateMenuCombos) {
                            ult::setIniFileValue(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::KEY_COMBO_STR , ult::ULTRAHAND_COMBO_STR);
                            ult::setIniFileValue(ult::TESLA_CONFIG_INI_PATH, ult::TESLA_STR, ult::KEY_COMBO_STR , ult::ULTRAHAND_COMBO_STR);
                            ult::updateMenuCombos = false;
                        }
                    #endif
                        
                        #if IS_STATUS_MONITOR_DIRECTIVE
                        isRendering = false;
                        leventSignal(&renderingStopEvent);
                        #endif
                        
                        if (shData->overlayOpen) {
                            tsl::Overlay::get()->hide();
                            shData->overlayOpen = false;
                        }
                        else {
                            eventFire(&shData->comboEvent);
                            mainComboHasTriggered.store(true, std::memory_order_release);
                        }
                    }
                #if IS_LAUNCHER_DIRECTIVE
                    else if (ult::updateMenuCombos && (((shData->keysHeld & tsl::cfg::launchCombo2) == tsl::cfg::launchCombo2) && shData->keysDown & tsl::cfg::launchCombo2)) {
                        std::swap(tsl::cfg::launchCombo, tsl::cfg::launchCombo2); // Swap the two launch combos
                        ult::setIniFileValue(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::KEY_COMBO_STR , ult::TESLA_COMBO_STR);
                        ult::setIniFileValue(ult::TESLA_CONFIG_INI_PATH, ult::TESLA_STR, ult::KEY_COMBO_STR , ult::TESLA_COMBO_STR);
                        eventFire(&shData->comboEvent);
                        mainComboHasTriggered.store(true, std::memory_order_release);
                        ult::updateMenuCombos = false;
                    }
                    else if (ult::overlayLaunchRequested.load(std::memory_order_acquire) && !ult::runningInterpreter.load(std::memory_order_acquire) && ult::settingsInitialized.load(std::memory_order_acquire) && (nowNs - startNs) >= FAST_SWAP_THRESHOLD_NS) {
                        std::string requestedPath, requestedArgs;
                        
                        // Get the request data safely
                        {
                            std::lock_guard<std::mutex> lock(ult::overlayLaunchMutex);
                            requestedPath = ult::requestedOverlayPath;
                            requestedArgs = ult::requestedOverlayArgs;
                            ult::overlayLaunchRequested.store(false, std::memory_order_release);
                        }
                        
                        if (!requestedPath.empty()) {

                            const std::string overlayFileName = ult::getNameFromPath(requestedPath);
                            
                            // Set overlay state for ovlmenu.ovl
        
                            // OPTIMIZED: Batch INI file writes
                            {
                                auto iniData = ult::getParsedDataFromIniFile(ult::ULTRAHAND_CONFIG_INI_PATH);
                                auto& section = iniData[ult::ULTRAHAND_PROJECT_NAME];
                                section[ult::IN_OVERLAY_STR] = ult::TRUE_STR;
                                section["to_packages"] = ult::TRUE_STR;
                                ult::saveIniFileData(ult::ULTRAHAND_CONFIG_INI_PATH, iniData);
                            }
        
                            // Reset navigation state variables (these control slide navigation)
                            ult::allowSlide.store(false, std::memory_order_release);
                            ult::unlockedSlide.store(false, std::memory_order_release);
                            
                            eventClose(&homeButtonPressEvent);
                            eventClose(&powerButtonPressEvent);
                            eventClose(&captureButtonPressEvent);

                            //hidExit();
                            // Launch the overlay using the same mechanism as key combos
                            //shData->overlayOpen = false;
                            ult::launchingOverlay.store(true, std::memory_order_release);
                            tsl::setNextOverlay(requestedPath, requestedArgs+" --direct");
                            tsl::Overlay::get()->close();
                            eventFire(&shData->comboEvent);

                            launchComboHasTriggered.store(true, std::memory_order_release);
                            return;
                        }
                    }
                #endif
                    // Check overlay key combos (only when overlay is not open, keys are pressed, and not conflicting with main combos)
                    //else if (!shData->overlayOpen && shData->keysDown != 0) {
                    else if (shData->keysDown != 0 && ult::useLaunchCombos) {
                        if (shData->keysHeld != tsl::cfg::launchCombo) {
                            // Lookup both path and optional mode launch args
                            const auto comboInfo = tsl::hlp::getEntryForKeyCombo(shData->keysHeld);
                            const std::string& overlayPath = comboInfo.path;
                            
                    
                    #if IS_LAUNCHER_DIRECTIVE
                            if (!overlayPath.empty() && (shData->keysHeld) && !ult::runningInterpreter.load(std::memory_order_acquire) && ult::settingsInitialized.load(std::memory_order_acquire) && (armTicksToNs(nowTick) - startNs) >= FAST_SWAP_THRESHOLD_NS) {
                    #else
                            if (!overlayPath.empty() && (shData->keysHeld) && (nowNs - startNs) >= FAST_SWAP_THRESHOLD_NS) {
                    #endif

                                const std::string& modeArg = comboInfo.launchArg;
                                const std::string overlayFileName = ult::getNameFromPath(overlayPath);
                    
                                // Check HOS21 support before doing anything
                                if (requiresLNY2 && !usingLNY2(overlayPath)) {
                                    // Skip launch if not supported
                                    const auto forceSupportStatus = ult::parseValueFromIniSection(
                                        ult::OVERLAYS_INI_FILEPATH, overlayFileName, "force_support");
                                    if (forceSupportStatus != ult::TRUE_STR) {
                                        continue;
                                    }
                                    continue;
                                }

                                // hideHidden check
                                if (hideHidden) {
                                    const auto hideStatus = ult::parseValueFromIniSection(
                                        ult::OVERLAYS_INI_FILEPATH, overlayFileName, ult::HIDE_STR);
                                    if (hideStatus == ult::TRUE_STR) {
                                        shData->keysDownPending |= shData->keysDown;
                                        continue;
                                    }
                                }
        
                                #if IS_STATUS_MONITOR_DIRECTIVE
                                isRendering = false;
                                leventSignal(&renderingStopEvent);
                                #endif
                    
                    #if !IS_LAUNCHER_DIRECTIVE
                                if (lastOverlayFilename == overlayFileName && lastOverlayMode == modeArg) {
                    #else
                                if (lastOverlayFilename == overlayFileName  && lastOverlayMode == modeArg && lastOverlayMode.find("--package") != std::string::npos) {
                    #endif
                                    ult::setIniFileValue(
                                        ult::ULTRAHAND_CONFIG_INI_PATH,
                                        ult::ULTRAHAND_PROJECT_NAME,
                                        ult::IN_OVERLAY_STR,
                                        ult::TRUE_STR
                                    );
                                
                                    //shData->overlayOpen = false;
                                    //hidExit();

                                    eventClose(&homeButtonPressEvent);
                                    eventClose(&powerButtonPressEvent);
                                    eventClose(&captureButtonPressEvent);

                                    ult::launchingOverlay.store(true, std::memory_order_release);
                                    tsl::setNextOverlay(ult::OVERLAY_PATH + "ovlmenu.ovl", "--direct --comboReturn");
                                    tsl::Overlay::get()->close();
                                    eventFire(&shData->comboEvent);
                                    launchComboHasTriggered.store(true, std::memory_order_release);
                                    return;
                                }
                                
                                // Compose launch args
                                std::string finalArgs;
                                if (!modeArg.empty()) {
                                    finalArgs = modeArg;
                                } else {
                                    // Only check overlay-specific launch args for non-ovlmenu entries
                                    if (overlayFileName.compare("ovlmenu.ovl") != 0) {
                                        // OPTIMIZED: Single INI read for both values
                                        auto overlaysIniData = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
                                        std::string useArgs = "";
                                        std::string launchArgs = "";
        
                                        auto sectionIt = overlaysIniData.find(overlayFileName);
                                        if (sectionIt != overlaysIniData.end()) {
                                            auto useArgsIt = sectionIt->second.find(ult::USE_LAUNCH_ARGS_STR);
                                            if (useArgsIt != sectionIt->second.end()) {
                                                useArgs = useArgsIt->second;
                                            }
                                            
                                            auto argsIt = sectionIt->second.find(ult::LAUNCH_ARGS_STR);
                                            if (argsIt != sectionIt->second.end()) {
                                                launchArgs = argsIt->second;
                                            }
                                        }
                                        
                                        if (useArgs == ult::TRUE_STR) {
                                            finalArgs = launchArgs;
                                            ult::removeQuotes(finalArgs);
                                        }
                                    }
                                }
                                if (finalArgs.empty()) {
                                    finalArgs = "--direct";
                                } else {
                                    finalArgs += " --direct";
                                }
        
                                if (overlayFileName.compare("ovlmenu.ovl") == 0) {
                                    finalArgs += " --comboReturn";
                                    ult::setIniFileValue(
                                        ult::ULTRAHAND_CONFIG_INI_PATH,
                                        ult::ULTRAHAND_PROJECT_NAME,
                                        ult::IN_OVERLAY_STR,
                                        ult::TRUE_STR
                                    );
                                }
                    
                                //shData->overlayOpen = false;
                                //hidExit();
                                eventClose(&homeButtonPressEvent);
                                eventClose(&powerButtonPressEvent);
                                eventClose(&captureButtonPressEvent);

                                ult::launchingOverlay.store(true, std::memory_order_release);
                                tsl::setNextOverlay(overlayPath, finalArgs);
                                tsl::Overlay::get()->close();
                                eventFire(&shData->comboEvent);
                                launchComboHasTriggered.store(true, std::memory_order_release);
                                return;
                            }
                        }
                    }
                //#endif
                    
                    shData->keysDownPending |= shData->keysDown;
                }

                //20 ms
                //s32 idx = 0;
                rc = waitObjects(&idx, objects, WaiterObject_Count, 20'000'000ul);
                if (R_SUCCEEDED(rc)) {
        
        #if IS_STATUS_MONITOR_DIRECTIVE
                    if (idx == WaiterObject_HomeButton || idx == WaiterObject_PowerButton) { // Changed condition to exclude capture button
                        if (shData->overlayOpen && !isValidOverlayMode()) {
                            tsl::Overlay::get()->hide();
                            shData->overlayOpen = false;
                        }
                    }
        #else
                    if (idx == WaiterObject_HomeButton || idx == WaiterObject_PowerButton) { // Changed condition to exclude capture button
                        if (shData->overlayOpen) {
                            tsl::Overlay::get()->hide();
                            shData->overlayOpen = false;
                        }
                    }
        #endif
                    
                    switch (idx) {
                        case WaiterObject_HomeButton:
                            eventClear(&homeButtonPressEvent);
                            break;
                        case WaiterObject_PowerButton:
                            eventClear(&powerButtonPressEvent);
        
                            // Perform any necessary cleanup
                            hidExit();
        
                            // Reinitialize resources
                            ASSERT_FATAL(hidInitialize()); // Reinitialize HID to reset states
                            
                            // Reinitialize both controllers
                            padInitialize(&pad_p1, HidNpadIdType_No1);
                            padInitialize(&pad_handheld, HidNpadIdType_Handheld);
                            hidInitializeTouchScreen();
                            
                            // Update both controllers
                            padUpdate(&pad_p1);
                            padUpdate(&pad_handheld);
                            break;
                            
                            
                        case WaiterObject_CaptureButton:
                            if (screenshotsAreDisabled) {
                                eventClear(&captureButtonPressEvent);
                                break;
                            }

                            #if IS_STATUS_MONITOR_DIRECTIVE
                            bool inOverlayMode = isValidOverlayMode();
                            if (inOverlayMode) {
                                delayUpdate = true;
                                isRendering = false;
                                leventSignal(&renderingStopEvent);
                            }
                            #endif
        
                            ult::disableTransparency = true;
                            eventClear(&captureButtonPressEvent);
                            svcSleepThread(1'500'000'000);
                            ult::disableTransparency = false;
        
                            #if IS_STATUS_MONITOR_DIRECTIVE
                            if (inOverlayMode) {
                                isRendering = true;
                                leventClear(&renderingStopEvent);
                                delayUpdate = false;
                            }
                            #endif
        
                            break;
                    }
                } else if (rc != KERNELRESULT(TimedOut)) {
                    ASSERT_FATAL(rc);
                }
            }
            //hidExit();

            eventClose(&homeButtonPressEvent);
            eventClose(&powerButtonPressEvent);
            eventClose(&captureButtonPressEvent);

        }
    }
    
    /**
     * @brief Creates a new Gui and changes to it
     *
     * @tparam G Gui to create
     * @tparam Args Arguments to pass to the Gui
     * @param args Arguments to pass to the Gui
     * @return Reference to the newly created Gui
     */
    template<typename G, typename ...Args>
    std::unique_ptr<tsl::Gui>& changeTo(Args&&... args) {
        return Overlay::get()->changeTo<G, Args...>(std::forward<Args>(args)...);
    }

    template<typename G, typename ...Args>
    std::unique_ptr<tsl::Gui>& swapTo(Args&&... args) {
        return Overlay::get()->swapTo<G, Args...>(std::forward<Args>(args)...);
    }

    template<typename G, typename ...Args>
    std::unique_ptr<tsl::Gui>& swapTo(SwapDepth depth, Args&&... args) {
        return Overlay::get()->swapTo<G, Args...>(depth, std::forward<Args>(args)...);
    }
    

    /**
     * @brief Pops the top Gui from the stack and goes back to the last one
     * @note The Overlay gets closed once there are no more Guis on the stack
     */
    void goBack(u32 count) {
        Overlay::get()->goBack(count);
    }
    
    void pop(u32 count) {
        Overlay::get()->pop(count);
    }
        
    
    static inline std::mutex setNextOverlayMutex;

    static inline std::string nextOverlayName;
    static void setNextOverlay(const std::string& ovlPath, std::string origArgs) {
        std::lock_guard lk(setNextOverlayMutex);
        char buffer[512];
        char* p = buffer;
        char* bufferEnd = buffer + sizeof(buffer) - 1; // Leave room for null terminator
        
        // Store filename and copy it
        const std::string filenameStr = ult::getNameFromPath(ovlPath);
        nextOverlayName = filenameStr;

        const char* filename = filenameStr.c_str();
        while (*filename && p < bufferEnd) *p++ = *filename++;
        if (p < bufferEnd) *p++ = ' ';
        
        // Single-pass argument filtering
        const char* src = origArgs.c_str();
        const char* end = src + origArgs.length();
        bool hasSkipCombo = false;
        
        while (src < end && p < bufferEnd) {
            // Skip whitespace
            while (src < end && *src == ' ' && p < bufferEnd) {
                *p++ = *src++;
            }
            
            if (src >= end || p >= bufferEnd) break;
            
            // Check for flags to filter/detect
            if (src[0] == '-' && src[1] == '-') {
                
                // Check what flag this is
                if (strncmp(src, "--skipCombo", 11) == 0 && (src[11] == ' ' || src[11] == '\0')) {
                    hasSkipCombo = true;
                    // Copy this flag
                    while (src < end && *src != ' ' && p < bufferEnd) *p++ = *src++;
                }
                else if (strncmp(src, "--foregroundFix", 15) == 0) {
                    // Skip this flag and its value
                    src += 15;
                    while (src < end && *src == ' ') src++; // Skip spaces
                    if (src < end && (*src == '0' || *src == '1')) src++; // Skip value
                }
                else if (strncmp(src, "--lastTitleID", 13) == 0) {
                    // Skip this flag and its value
                    src += 13;
                    while (src < end && *src == ' ') src++; // Skip spaces
                    while (src < end && *src != ' ' && *src != '\0') src++; // Skip title ID
                }
                else {
                    // Copy unknown flag
                    while (src < end && *src != ' ' && p < bufferEnd) *p++ = *src++;
                }
            }
            else {
                // Copy regular argument
                while (src < end && *src != ' ' && p < bufferEnd) *p++ = *src++;
            }
        }
        
        // Add required flags with bounds checking
        if (!hasSkipCombo && (p + 12) < bufferEnd) {
            memcpy(p, " --skipCombo", 12);
            p += 12;
        }
        
        // Add foreground flag with bounds checking
        if ((p + 17) < bufferEnd) {
            memcpy(p, " --foregroundFix ", 17);
            p += 17;
            if (p < bufferEnd) {
                *p++ = (ult::resetForegroundCheck.load(std::memory_order_acquire) || ult::lastTitleID != ult::getTitleIdAsString()) ? '1' : '0';
            }
        }
        
        // Add last title ID with bounds checking
        if ((p + 15 + ult::lastTitleID.length()) < bufferEnd) {
            memcpy(p, " --lastTitleID ", 15);
            p += 15;
            const char* titleId = ult::lastTitleID.c_str();
            while (*titleId && p < bufferEnd) *p++ = *titleId++;
        }
        
        // Safety check - if we're at the end, we might have truncated
        if (p >= bufferEnd) {
            p = bufferEnd;
        }
        
        *p = '\0';
        
        //isLaunchingNextOverlay.store(true, std::memory_order_release);
        envSetNextLoad(ovlPath.c_str(), buffer);
    }
    
    

    struct option_entry {
        const char* name;
        u8 len;
        u8 action;
    };
    
    static constexpr struct option_entry options[] = {
        {"direct", 6, 1},
        {"skipCombo", 9, 2},
        {"lastTitleID", 11, 3}, 
        {"foregroundFix", 13, 4},
        {"package", 7, 5},
        {"lastSelectedItem", 16, 6},
        {"comboReturn", 11, 7} // new option
    };


    /**
     * @brief libtesla's main function
     * @note Call it directly from main passing in argc and argv and returning it e.g `return tsl::loop<OverlayTest>(argc, argv);`
     *
     * @tparam TOverlay Your overlay class
     * @tparam launchFlags \ref LaunchFlags
     * @param argc argc
     * @param argv argv
     * @return int result
     */
    template<typename TOverlay, impl::LaunchFlags launchFlags>
    static inline int loop(int argc, char** argv) {
        static_assert(std::is_base_of_v<tsl::Overlay, TOverlay>, "tsl::loop expects a type derived from tsl::Overlay");

    #if IS_STATUS_MONITOR_DIRECTIVE
        leventClear(&renderingStopEvent);
        // Status monitor will load heap settings directly in main, so bypass here in loop
    #else

        ult::currentHeapSize = ult::getCurrentHeapSize();
        ult::expandedMemory = ult::currentHeapSize >= ult::OverlayHeapSize::Size_8MB;
        ult::limitedMemory = ult::currentHeapSize == ult::OverlayHeapSize::Size_4MB;


        // Initialize buffer sizes based on expanded memory setting
        if (ult::expandedMemory) {
            ult::furtherExpandedMemory = ult::currentHeapSize >= ult::OverlayHeapSize::Size_10MB;
            
            ult::loaderTitle += !ult::furtherExpandedMemory ? "+" : "×";
            ult::COPY_BUFFER_SIZE = 262144;
            ult::HEX_BUFFER_SIZE = 8192;
            ult::UNZIP_READ_BUFFER = 262144;
            ult::UNZIP_WRITE_BUFFER = 131072;
            ult::DOWNLOAD_READ_BUFFER = 262144/2;
            ult::DOWNLOAD_WRITE_BUFFER = 131072;
        } else if (ult::limitedMemory) {
            ult::loaderTitle += "-";
        }
    #endif
    
        if (argc > 0) {
            //g_overlayFilename = ult::getNameFromPath(argv[0]);
            lastOverlayFilename = ult::getNameFromPath(argv[0]);
    
            lastOverlayMode.clear();
            bool skip;
            for (u8 arg = 1; arg < argc; arg++) {
                const char* s = argv[arg];

                skip = false;
    
                if (arg > 1) {
                    const char* prev = argv[arg - 1];
                    if (prev[0] == '-' && prev[1] == '-') {
                        if (strcmp(prev, "--lastTitleID") == 0 || strcmp(prev, "--foregroundFix") == 0) {
                            skip = true;
                        }
                    }
                }
    
                if (!skip && s[0] == '-' && s[1] == '-') {
                    if (strcmp(s, "--direct") == 0 ||
                        strcmp(s, "--skipCombo") == 0 ||
                        strcmp(s, "--lastTitleID") == 0 ||
                        strcmp(s, "--foregroundFix") == 0) {
                        skip = true;
                    }
                }
    
                if (!skip) {
                    if (strcmp(s, "--package") == 0) {
                        lastOverlayMode = "--package";
                        arg++;
                        if (arg < argc) {
                            lastOverlayMode += " ";
                            lastOverlayMode += argv[arg];
                            arg++;
                            while (arg < argc && argv[arg][0] != '-') {
                                lastOverlayMode += " ";
                                lastOverlayMode += argv[arg];
                                arg++;
                            }
                        }
                    } else {
                        lastOverlayMode = s;
                    }
                    break;
                }
            }
        }
    
        bool skipCombo = false;
    #if IS_LAUNCHER_DIRECTIVE
        bool comboReturn = false;
        bool directMode = true;
    #else
        bool directMode = false;
    #endif
        bool usingPackageLauncher = false;
        
    
        for (u8 arg = 0; arg < argc; arg++) {
            const char* s = argv[arg];
            if (s[0] != '-' || s[1] != '-') continue;
            const char* opt = s + 2;
    
            for (u8 i = 0; i < 7; i++) { // now 6 instead of 5
                if (memcmp(opt, options[i].name, options[i].len) == 0 && opt[options[i].len] == '\0') {
                    switch (options[i].action) {
                        case 1: // direct
                            directMode = true;
                            //g_overlayFilename = "";
                            jumpItemName = "";
                            jumpItemValue = "";
                            jumpItemExactMatch.store(true, std::memory_order_release);
                            break;
                        case 2: // skipCombo
                            skipCombo = true;
                            ult::firstBoot = false;
                            break;
                        case 3: // lastTitleID
                            if (++arg < argc) {
                                const char* providedID = argv[arg];
                                if (ult::getTitleIdAsString() != providedID) {
                                    ult::resetForegroundCheck.store(true, std::memory_order_release);
                                }
                            }
                            break;
                        case 4: // foregroundFix
                            if (++arg < argc) {
                                ult::resetForegroundCheck.store(
                                    ult::resetForegroundCheck.load(std::memory_order_acquire) ||
                                    (argv[arg][0] == '1'), std::memory_order_release);
                            }
                            break;
                        case 5: // package
                            usingPackageLauncher = true;
                            break;
                        case 6: // lastSelectedItem
                            #if IS_STATUS_MONITOR_DIRECTIVE
                            lastMode = "returning";
                            #endif
                            break;
                        case 7: // comboReturn
                            #if IS_LAUNCHER_DIRECTIVE
                            comboReturn = true;
                            #endif
                            break;
                    }
                }
            }
        }
    
        impl::SharedThreadData shData;
        shData.running.store(true, std::memory_order_release);
    
        Thread backgroundThread;
        threadCreate(&backgroundThread, impl::backgroundEventPoller, &shData, nullptr, 0x2000, 0x2c, -2);
        threadStart(&backgroundThread);
    
        eventCreate(&shData.comboEvent, false);
    
        auto& overlay = tsl::Overlay::s_overlayInstance;
        overlay = new TOverlay();
        overlay->m_closeOnExit = (u8(launchFlags) & u8(impl::LaunchFlags::CloseOnExit)) == u8(impl::LaunchFlags::CloseOnExit);
    
        tsl::hlp::doWithSmSession([&overlay]{
            overlay->initServices();
        });

    #if !IS_LAUNCHER_DIRECTIVE
        tsl::initializeUltrahandSettings();
    #endif

        overlay->initScreen();
        overlay->changeTo(overlay->loadInitialGui());
        

        bool shouldFireEvent = false;

    #if IS_LAUNCHER_DIRECTIVE
       
        {
            auto configData = ult::getParsedDataFromIniFile(ult::ULTRAHAND_CONFIG_INI_PATH);
            bool needsUpdate = false;
        
            // Get reference to project section (create if missing)
            auto& project = configData[ult::ULTRAHAND_PROJECT_NAME];
        
            // Determine current overlay state
            bool inOverlay = true;
            auto it = project.find(ult::IN_OVERLAY_STR);
            if (it != project.end()) {
                inOverlay = (it->second != ult::FALSE_STR);
            }
        
            // Only update the overlay key once, for either firstBoot or skipCombo
            if (ult::firstBoot || (inOverlay && skipCombo)) {
                project[ult::IN_OVERLAY_STR] = ult::FALSE_STR;
                needsUpdate = true;
                if (inOverlay && skipCombo) {
                    shouldFireEvent = true;
                }
            }
        
            // Write INI only if we changed something
            if (needsUpdate) {
                ult::saveIniFileData(ult::ULTRAHAND_CONFIG_INI_PATH, configData);
            }
        
            // Fire event if needed
            if (shouldFireEvent) {
                eventFire(&shData.comboEvent);
            }
        }
    #else
        {
            auto configData = ult::getParsedDataFromIniFile(ult::ULTRAHAND_CONFIG_INI_PATH);
        
            auto projectIt = configData.find(ult::ULTRAHAND_PROJECT_NAME);
            if (projectIt != configData.end()) {
                auto& project = projectIt->second;
        
                auto overlayIt = project.find(ult::IN_OVERLAY_STR);
                const bool inOverlay = (overlayIt == project.end() ||
                                        overlayIt->second != ult::FALSE_STR);
        
                if (inOverlay && directMode) {
                    project[ult::IN_OVERLAY_STR] = ult::FALSE_STR;
                    ult::saveIniFileData(ult::ULTRAHAND_CONFIG_INI_PATH, configData);
                }
            }
        }

        if (skipCombo) {
            eventFire(&shData.comboEvent);
            shouldFireEvent = true;
        }
    #endif
    
        overlay->disableNextAnimation();
    
        {
            const Handle handles[2] = { shData.comboEvent.revent, notificationEvent.revent };
            s32 index = -1;
            
            bool exitAfterPrompt = false;
            bool comboBreakout = false;
            bool firstLoop = !ult::firstBoot;
    
            while (shData.running.load(std::memory_order_acquire)) {
                // Early exit if launching new overlay
                if (ult::launchingOverlay.load(std::memory_order_acquire)) {
                    //std::scoped_lock lock(shData.dataMutex);
                    shData.running.store(false, std::memory_order_release);
                    shData.overlayOpen.store(false, std::memory_order_release);
                    break;
                }
    
                // Wait for events only if no active notification
                if (!(notification && notification->isActive())) {
                    svcWaitSynchronization(&index, handles, 2, UINT64_MAX);
                    //eventClear(&notificationEvent);
                    //eventClear(&shData.comboEvent);
                }
                eventClear(&notificationEvent);
                eventClear(&shData.comboEvent);
    
                if ((notification && notification->isActive() && !firstLoop) || index == 1) {
                    comboBreakout = false;
    
                    while (shData.running.load(std::memory_order_acquire)) {
                        {
                            //std::scoped_lock lock(shData.dataMutex);
                            if (ult::launchingOverlay.load(std::memory_order_acquire)) {
                                shData.running.store(false, std::memory_order_release);
                                shData.overlayOpen.store(false, std::memory_order_release);
                                break;
                            }
                            overlay->loop(true); // Draw prompts while hidden
                        }
    
                        if (mainComboHasTriggered.exchange(false, std::memory_order_acq_rel)) {
                            comboBreakout = true;
                            exitAfterPrompt = false;
                            break;
                        }
    
                        if (launchComboHasTriggered.load(std::memory_order_acquire)) {
                            exitAfterPrompt = true;
                            usingPackageLauncher = false;
                            directMode = false;
                            break;
                        }
    
                        if (!(notification && notification->isActive())) {
                            break;
                        }
                    }
    
                    if (!comboBreakout || !shData.running.load(std::memory_order_acquire)) {
                        {
                            //std::scoped_lock lock(shData.dataMutex);
                            if (!ult::launchingOverlay.load(std::memory_order_acquire)) {
                                overlay->clearScreen();
                            }
                        }
                        if (exitAfterPrompt) {
                            std::scoped_lock lock(shData.dataMutex);
                            exitAfterPrompt = false;
                            shData.running.store(false, std::memory_order_release);
                            shData.overlayOpen.store(false, std::memory_order_release);
                            ult::launchingOverlay.store(true, std::memory_order_release);
                            launchComboHasTriggered.store(true, std::memory_order_release); // for isolating sound effect
    
                            if (usingPackageLauncher || directMode) {
                                tsl::setNextOverlay(ult::OVERLAY_PATH + "ovlmenu.ovl");
                            }
                            
                            hlp::requestForeground(false);
                            break;
                        }
                        continue;
                    }
                }
    
                {
                    //std::scoped_lock lock(shData.dataMutex);
                    if (ult::launchingOverlay.load(std::memory_order_acquire)) {
                        shData.running.store(false, std::memory_order_release);
                        shData.overlayOpen.store(false, std::memory_order_release);
                        break;
                    }
                    firstLoop = false;
                    shData.overlayOpen.store(true, std::memory_order_release);
    
    #if IS_STATUS_MONITOR_DIRECTIVE
                    if (!isValidOverlayMode())
                        hlp::requestForeground(true);
    #else
                    hlp::requestForeground(true);
    #endif
    
                    overlay->show();
                    if (!comboBreakout && !(notification && notification->isActive()))
                        overlay->clearScreen();

                    {
                        std::scoped_lock lock(shData.dataMutex);
                        // Clear derived states that the overlay loop will use
                        shData.keysDown = 0;
                        shData.keysHeld = 0;
                        // Clear any queued pending keys so nothing gets processed one frame late
                        shData.keysDownPending = 0;
                    }
                }
                
                while (shData.running.load(std::memory_order_acquire)) {
                    {
                        
                        if (ult::launchingOverlay.load(std::memory_order_acquire)) {
                            shData.running.store(false, std::memory_order_release);
                            shData.overlayOpen.store(false, std::memory_order_release);
                            break;
                        }
                        overlay->loop();
                        {
                            std::scoped_lock lock(shData.dataMutex);
                            if (!overlay->fadeAnimationPlaying()) {
                                overlay->handleInput(shData.keysDownPending, shData.keysHeld, shData.touchState.count, shData.touchState.touches[0], shData.joyStickPosLeft, shData.joyStickPosRight);
                            }
                            shData.keysDownPending = 0;
                        }
                        #if IS_LAUNCHER_DIRECTIVE
                        if (shouldFireEvent) {
                            shouldFireEvent = false;
                            
                            if (!comboReturn) {
                                //triggerRumbleDoubleClick.store(true, std::memory_order_release);
                                //triggerExitSound.store(true, std::memory_order_release);
                                triggerExitFeedback();
                            //} else {
                            //    triggerRumbleClick.store(true, std::memory_order_release);
                            }
                        }
                        #else
                        if (!directMode && shouldFireEvent) {
                            shouldFireEvent = false;
                            #if IS_STATUS_MONITOR_DIRECTIVE
                            if (lastMode.compare("returning") == 0) {
                                //triggerRumbleDoubleClick.store(true, std::memory_order_release);
                                //triggerExitSound.store(true, std::memory_order_release);
                                triggerExitFeedback();
                            } else {
                                //triggerRumbleClick.store(true, std::memory_order_release);
                                triggerEnterSound.store(true, std::memory_order_release);
                            }
                            #else
                            //triggerRumbleClick.store(true, std::memory_order_release);
                            //triggerEnterSound.store(true, std::memory_order_release);
                            triggerEnterFeedback();
                            #endif
                        }
                        #endif
                    }
    
    #if IS_STATUS_MONITOR_DIRECTIVE
                    if (pendingExit && wasRendering) {
                        pendingExit = false;
                        wasRendering = false;
                        isRendering = true;
                        leventClear(&renderingStopEvent);
                    }
    #endif
    
                    if (overlay->shouldHide()) {
                        if (overlay->shouldCloseAfter()) {
                            if (!directMode) {
                                //std::scoped_lock lock(shData.dataMutex);
                                shData.running.store(false, std::memory_order_release);
                                shData.overlayOpen.store(false, std::memory_order_release);
                                break;
                            } else {
                                exitAfterPrompt = true;
    #if IS_STATUS_MONITOR_DIRECTIVE
                                pendingExit = true;
    #endif
                            }
                        }
                        break;
                    }
    
                    if (overlay->shouldClose()) {
                        //std::scoped_lock lock(shData.dataMutex);
                        shData.running.store(false, std::memory_order_release);
                        shData.overlayOpen.store(false, std::memory_order_release);

                        break;
                    }
                }
    
                if (shData.running.load(std::memory_order_acquire)) {
                    //std::scoped_lock lock(shData.dataMutex);
                    if (!(notification && notification->isActive()))
                        overlay->clearScreen();
                    overlay->resetFlags();
                    hlp::requestForeground(false);
                    shData.overlayOpen.store(false, std::memory_order_release);
                    mainComboHasTriggered.store(false, std::memory_order_acquire);
                    launchComboHasTriggered.store(false, std::memory_order_acquire);
                    eventClear(&shData.comboEvent);
                }
            }

            // Ensure background thread is fully stopped before overlay cleanup
            shData.running.store(false, std::memory_order_release);
            threadWaitForExit(&backgroundThread);
            threadClose(&backgroundThread);

            
            // Cleanup overlay resources
            tsl::elm::fullDeconstruction.store(true, std::memory_order_release);
            hlp::requestForeground(false);
            overlay->exitScreen();
            overlay->exitServices();
            delete overlay;
            
            
            eventClose(&shData.comboEvent);


            if (directMode && !launchComboHasTriggered.load(std::memory_order_acquire)) {
                if (!disableSound.load(std::memory_order_acquire) && ult::useSoundEffects)
                    ult::AudioPlayer::playExitSound();
                if (ult::useHapticFeedback) {
                    ult::rumbleDoubleClickStandalone();
                }
            }

            
            // Brief delay to ensure thread quiescence before nx-ovlloader transition
            //svcSleepThread(100'000'000); // 100ms
    
            return 0;
        }
    }

}


#ifdef TESLA_INIT_IMPL

namespace tsl::cfg {
    
    u16 LayerWidth  = 0;
    u16 LayerHeight = 0;
    u16 LayerPosX   = 0;
    u16 LayerPosY   = 0;
    u16 FramebufferWidth  = 0;
    u16 FramebufferHeight = 0;
    u64 launchCombo = KEY_ZL | KEY_ZR | KEY_DDOWN;
    u64 launchCombo2 = KEY_L | KEY_DDOWN | KEY_RSTICK;
}
extern "C" void __libnx_init_time(void);

extern "C" {
    
    u32 __nx_applet_type = AppletType_None;
    u32 __nx_fs_num_sessions = 1;
    u32  __nx_nv_transfermem_size = 0x15000;
    ViLayerFlags __nx_vi_stray_layer_flags = (ViLayerFlags)0;
    
    /**
     * @brief libtesla service initializing function to override libnx's
     *
     */
    void __appInit(void) {
        ASSERT_FATAL(smInitialize()); // needed to prevent issues with powering device into sleep

        //tsl::hlp::doWithSmSession([]{
        
        ASSERT_FATAL(fsInitialize());
        ASSERT_FATAL(hidInitialize());                          // Controller inputs and Touch
        if (hosversionAtLeast(16,0,0)) {
            ASSERT_FATAL(plInitialize(PlServiceType_User));     // Font data. Use pl:u for 16.0.0+
        } else {
            ASSERT_FATAL(plInitialize(PlServiceType_System));   // Use pl:s for 15.0.1 and below to prevent qlaunch/overlaydisp session exhaustion
        }
        ASSERT_FATAL(pmdmntInitialize());                       // PID querying
        ASSERT_FATAL(hidsysInitialize());                       // Focus control
        ASSERT_FATAL(setsysInitialize());                       // Settings querying
        
        // Time initializations
        if R_SUCCEEDED(timeInitialize()) {
            __libnx_init_time();
            timeExit();
        }

        #if USING_WIDGET_DIRECTIVE
        ult::powerInit();
        i2cInitialize();
        #endif

        fsdevMountSdmc();
        splInitialize();
        spsmInitialize();
        //i2cInitialize();
        //ASSERT_FATAL(socketInitializeDefault());
        //ASSERT_FATAL(nifmInitialize(NifmServiceType_User));

        //});

        //requiresLNY2 = amsVersionAtLeast(1,9,0);     // Detect if using HOS 21+
        //ult::currentHeapSize = ult::getCurrentHeapSize();
        //ult::expandedMemory = ult::currentHeapSize >= ult::OverlayHeapSize::Size_8MB;

        #if IS_STATUS_MONITOR_DIRECTIVE
        Service *plSrv = plGetServiceSession();
        Service plClone;
        ASSERT_FATAL(serviceClone(plSrv, &plClone));
        serviceClose(plSrv);
        *plSrv = plClone;
        #endif

        eventCreate(&tsl::notificationEvent, false);
        tsl::notification = new tsl::NotificationPrompt();
        //tsl::notification = nullptr;
    }
    
    /**
     * @brief libtesla service exiting function to override libnx's
     *
     */
    void __appExit(void) {
        delete tsl::notification;
        eventClose(&tsl::notificationEvent);

        //deinitRumble();
        if (ult::expandedMemory)
            ult::AudioPlayer::exit();

        
        //socketExit();
        //nifmExit();
        spsmExit();
        splExit();
        fsdevUnmountAll();
        
        #if USING_WIDGET_DIRECTIVE
        i2cExit();
        ult::powerExit(); // CUSTOM MODIFICATION
        #endif
        
        fsExit();
        hidExit();
        plExit();
        pmdmntExit();
        hidsysExit();
        setsysExit();
        smExit();

        // Final cleanup
        tsl::gfx::FontManager::cleanup();
    }

}

#endif