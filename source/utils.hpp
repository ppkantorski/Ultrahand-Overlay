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
 *  Copyright (c) 2024 ppkantorski
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

#include <filesystem> // unused, but preserved for projects that might need it
#include <algorithm>
#include <cstring>
#include <cwctype>
#include <string>
#include <functional>
#include <type_traits>
#include <mutex>
#include <memory>
//#include <chrono>
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
struct GlyphInfo {
    u8* pointer;
    int width;
    int height;
};

struct KeyPairHash {
    std::size_t operator()(const std::pair<int, float>& key) const {
        // Combine hashes of both components
        union returnValue {
            char c[8];
            std::size_t s;
        } value;
        __builtin_memcpy(&value.c[0], &key.first, 4);
        __builtin_memcpy(&value.c[4], &key.second, 4);
        return value.s;
    }
};

// Custom equality comparison for int-float pairs
struct KeyPairEqual {
    bool operator()(const std::pair<int, float>& lhs, const std::pair<int, float>& rhs) const {
        const float epsilon = 0.00001f;
        return lhs.first == rhs.first && 
            std::abs(lhs.second - rhs.second) < epsilon;
    }
};

std::unordered_map<std::pair<s32, float>, GlyphInfo, KeyPairHash, KeyPairEqual> cache;

u8 TeslaFPS = 60;
u8 alphabackground = 0xD;
bool FullMode = true;
bool deactivateOriginalFooter = false;
bool fontCache = true;

#endif

#if USING_FPS_INDICATOR_DIRECTIVE
float fps = 0.0;
int frameCount = 0;
double elapsedTime;
#endif

//static bool jumpToListItem = false;
static bool jumpToTop = false;
static bool jumpToBottom = false;
static u32 offsetWidthVar = 112;


namespace tsl {

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
    static Color GradientColor(float temperature) {
        // Ensure temperature is within the range [0, 100]
        temperature = std::max(0.0f, std::min(100.0f, temperature)); // Celsius
        
        // this is where colors are at their full
        float blueStart = 35.0f;
        float greenStart = 45.0f;
        float yellowStart = 55.0f;
        float redStart = 65.0f;
        
        // Initialize RGB values
        uint8_t r, g, b, a = 0xFF;
        float t;

        if (temperature < blueStart) { // rgb 7, 7, 15 at blueStart
            r = 7;
            g = 7;
            b = 15;
        } else if (temperature >= blueStart && temperature < greenStart) {
            // Smooth color blending from (7 7 15) to (0 15 0)
            t = (temperature - blueStart) / (greenStart - blueStart);
            r = static_cast<uint8_t>(7 - 7 * t);
            g = static_cast<uint8_t>(7 + 8 * t);
            b = static_cast<uint8_t>(15 - 15 * t);
        } else if (temperature >= greenStart && temperature < yellowStart) {
            // Smooth color blending from (0 15 0) to (15 15 0)
            t = (temperature - greenStart) / (yellowStart - greenStart);
            r = static_cast<uint8_t>(15 * t);
            g = static_cast<uint8_t>(15);
            b = static_cast<uint8_t>(0);
        } else if (temperature >= yellowStart && temperature < redStart) {
            // Smooth color blending from (15 15 0) to (15 0 0)
            t = (temperature - yellowStart) / (redStart - yellowStart);
            r = static_cast<uint8_t>(15);
            g = static_cast<uint8_t>(15 - 15 * t);
            b = static_cast<uint8_t>(0);
        } else {
            // Full red
            r = 15;
            g = 0;
            b = 0;
        }
        
        return Color(r, g, b, a);
    }
    //#endif


    static Color RGB888(const std::string& hexColor, size_t alpha = 15, const std::string& defaultHexColor = ult::whiteColor) {
        std::string validHex = hexColor.empty() || hexColor[0] != '#' ? hexColor : hexColor.substr(1);
        
        if (!ult::isValidHexColor(validHex)) {
            validHex = defaultHexColor;
        }
        
        // Convert hex to RGB values
        uint8_t redValue = (ult::hexMap[static_cast<unsigned char>(validHex[0])] << 4) | ult::hexMap[static_cast<unsigned char>(validHex[1])];
        uint8_t greenValue = (ult::hexMap[static_cast<unsigned char>(validHex[2])] << 4) | ult::hexMap[static_cast<unsigned char>(validHex[3])];
        uint8_t blueValue = (ult::hexMap[static_cast<unsigned char>(validHex[4])] << 4) | ult::hexMap[static_cast<unsigned char>(validHex[5])];
        
        return Color(redValue >> 4, greenValue >> 4, blueValue >> 4, alpha);
    }
    
    
    namespace style {
        constexpr u32 ListItemDefaultHeight         = 70;       ///< Standard list item height
        constexpr u32 MiniListItemDefaultHeight     = 40;       ///< Mini list item height
        constexpr u32 TrackBarDefaultHeight         = 84;       ///< Standard track bar height
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

    static bool overrideBackButton = false; // for proprerly overriding the automatic "go back" functionality of KEY_B button presses

    // Theme color variable definitions
    static bool disableColorfulLogo = false;

    
    static Color logoColor1 = RGB888(ult::whiteColor);
    static Color logoColor2 = RGB888("F7253E");
    

    static size_t defaultBackgroundAlpha = 13;
    
    static Color defaultBackgroundColor = RGB888(ult::blackColor, defaultBackgroundAlpha);
    static Color defaultTextColor = RGB888(ult::whiteColor);
    static Color headerTextColor = RGB888(ult::whiteColor);
    static Color headerSeparatorColor = RGB888(ult::whiteColor);
    static Color starColor = RGB888(ult::whiteColor);
    static Color selectionStarColor = RGB888(ult::whiteColor);
    static Color buttonColor = RGB888(ult::whiteColor);
    static Color bottomTextColor = RGB888(ult::whiteColor);
    static Color botttomSeparatorColor = RGB888(ult::whiteColor);

    static Color defaultOverlayColor = RGB888(ult::whiteColor);
    static Color defaultPackageColor = RGB888(ult::whiteColor);//RGB888("#00FF00");
    static Color defaultScriptColor = RGB888("FF33FF");
    static Color clockColor = RGB888(ult::whiteColor);
    static Color batteryColor = RGB888("ffff45");
    static Color versionTextColor = RGB888("AAAAAA");
    static Color onTextColor = RGB888("00FFDD");
    static Color offTextColor = RGB888("AAAAAA");
    
    #if IS_LAUNCHER_DIRECTIVE
    static Color dynamicLogoRGB1 = RGB888("00E669");
    static Color dynamicLogoRGB2 = RGB888("8080EA");
    #endif

    static bool disableSelectionBG = false;
    static bool invertBGClickColor = false;

    static size_t selectionBGAlpha = 7;
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

    static Color selectedTextColor = RGB888(ult::whiteColor);
    static Color inprogressTextColor = RGB888(ult::whiteColor);
    static Color invalidTextColor = RGB888("FF0000");
    static Color clickTextColor = RGB888(ult::whiteColor);

    static size_t tableBGAlpha = 10;
    static Color tableBGColor = RGB888("303030", tableBGAlpha);
    static Color sectionTextColor = RGB888(ult::whiteColor);
    //static Color infoTextColor = RGB888("00FFDD");
    static Color infoTextColor =RGB888("85c4ff");
    static Color warningTextColor = RGB888("FF7777");

    static Color healthyRamTextColor = RGB888("00FF00");
    static Color neutralRamTextColor = RGB888("FFAA00");
    static Color badRamTextColor = RGB888("FF0000");

    static Color trackBarSliderColor = RGB888("606060");
    static Color trackBarSliderBorderColor = RGB888("505050");
    static Color trackBarSliderMalleableColor = RGB888("A0A0A0");
    static Color trackBarFullColor = RGB888("00FFDD");
    static Color trackBarEmptyColor = RGB888("404040");
    
    static void initializeThemeVars() { // NOTE: This needs to be called once in your application.
        // Fetch all theme settings at once from the INI file
        auto themeData = ult::getParsedDataFromIniFile(ult::THEME_CONFIG_INI_PATH);
        if (themeData.count(ult::THEME_STR) > 0) {
            auto& themeSection = themeData[ult::THEME_STR];
            
            // Fetch and process each theme setting using a helper to simplify fetching and fallback
            auto getValue = [&](const std::string& key) {
                return themeSection.count(key) ? themeSection[key] : ult::defaultThemeSettingsMap[key];
            };
            
            // Convert hex color to Color and manage default values and conversion
            auto getColor = [&](const std::string& key, size_t alpha = 15) {
                //std::string hexColor = getValue(key);
                return RGB888(getValue(key), alpha);
            };
            
            auto getAlpha = [&](const std::string& key) {
                std::string alphaStr = getValue(key);
                return !alphaStr.empty() ? ult::stoi(alphaStr) : ult::stoi(ult::defaultThemeSettingsMap[key]);
            };
            
            disableColorfulLogo = (getValue("disable_colorful_logo") == ult::TRUE_STR);
            
            #if IS_LAUNCHER_DIRECTIVE
            logoColor1 = getColor("logo_color_1");
            logoColor2 = getColor("logo_color_2");
            #endif

            defaultBackgroundAlpha = getAlpha("bg_alpha");
            defaultBackgroundColor = getColor("bg_color", defaultBackgroundAlpha);
            defaultTextColor = getColor("text_color");
            headerTextColor = getColor("header_text_color");
            headerSeparatorColor = getColor("header_separator_color");
            starColor = getColor("star_color");
            selectionStarColor = getColor("selection_star_color");
            buttonColor = getColor("bottom_button_color");
            bottomTextColor = getColor("bottom_text_color");
            botttomSeparatorColor = getColor("bottom_separator_color");
            defaultOverlayColor = getColor("default_overlay_color");
            defaultPackageColor = getColor("default_package_color");
            defaultScriptColor = getColor("default_script_color");

            clockColor = getColor("clock_color");
            batteryColor = getColor("battery_color");
            
            versionTextColor = getColor("version_text_color");
            onTextColor = getColor("on_text_color");
            offTextColor = getColor("off_text_color");
            
            #if IS_LAUNCHER_DIRECTIVE
            dynamicLogoRGB1 = getColor("dynamic_logo_color_1");
            dynamicLogoRGB2 = getColor("dynamic_logo_color_2");
            #endif

            disableSelectionBG = (getValue("disable_selection_bg") == ult::TRUE_STR);
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
            
            selectedTextColor = getColor("selection_text_color");
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
    }
    
    #if IS_LAUNCHER_DIRECTIVE
    #else
    static void initializeUltrahandSettings() { // only needed for regular overlays

        std::string defaultLang = ult::parseValueFromIniSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::DEFAULT_LANG_STR);
        defaultLang = defaultLang.empty() ? "en" : defaultLang;

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
        // Set Ultrahand Globals
        ult::useSwipeToOpen = (ult::parseValueFromIniSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, "swipe_to_open") == ult::TRUE_STR);
        ult::useOpaqueScreenshots = (ult::parseValueFromIniSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, "opaque_screenshots") == ult::TRUE_STR);

        std::string langFile = ult::LANG_PATH+defaultLang+".json";
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
        
        [[maybe_unused]] static constexpr LaunchFlags operator|(LaunchFlags lhs, LaunchFlags rhs) {
            return static_cast<LaunchFlags>(u8(lhs) | u8(rhs));
        }
        
        
        
    }
    
    [[maybe_unused]] static void goBack();

    [[maybe_unused]] static void pop();
    
    [[maybe_unused]] static void setNextOverlay(const std::string& ovlPath, std::string args = "");
    
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
         * @brief Toggles focus between the Tesla overlay and the rest of the system
         *
         * @param enabled Focus Tesla?
         */
        static void requestForeground(bool enabled, bool updateGlobalFlag = true) {
            if (updateGlobalFlag)
                ult::currentForeground = enabled;

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
                
                std::string iniString = unparseIni(iniData);
                
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

    #if IS_LAUNCHER_DIRECTIVE
        // Function to load key combo mappings from overlays.ini
        static void loadOverlayKeyCombos() {
            ult::overlayKeyCombos.clear();
            auto overlaysIniData = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
            u64 keys;

            for (const auto& [overlayFileName, settings] : overlaysIniData) {
                auto keyComboIt = settings.find("key_combo");
                if (keyComboIt != settings.end() && !keyComboIt->second.empty()) {
                    keys = hlp::comboStringToKeys(keyComboIt->second);
                    if (keys != 0) {
                        ult::overlayKeyCombos[keys] = ult::OVERLAY_PATH + overlayFileName;
                    }
                }
            }
        }
        
        // Function to check if a key combination matches any overlay key combo
        static std::string getOverlayForKeyCombo(u64 keys) {
            auto it = ult::overlayKeyCombos.find(keys);
            return (it != ult::overlayKeyCombos.end()) ? it->second : "";
        }
    #endif

    }
    


    // Renderer
    
    namespace gfx {
        
        extern "C" u64 __nx_vi_layer_id;
        

        struct ScissoringConfig {
            u32 x, y, w, h;
        };
        

        static float calculateStringWidth(const std::string& originalString, const float fontSize, const bool fixedWidthNumbers); // forward declaration

        static std::pair<int, int> getUnderscanPixels();

        /**
         * @brief Manages the Tesla layer and draws raw data to the screen
         */
        class Renderer final {
        public:

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
            static Color a(const Color& c) {
                u8 alpha = (ult::disableTransparency && ult::useOpaqueScreenshots) ? 0xF : static_cast<u8>(std::min(static_cast<u8>(c.a), static_cast<u8>(0xF * Renderer::s_opacity)));
                return (c.rgba & 0x0FFF) | (alpha << 12);
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
                this->m_scissoringStack.emplace(x, y, w, h);
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
            inline void setPixel(const u32 x, const u32 y, const Color& color, const u32 offset) {
                if (offset != UINT32_MAX) [[likely]] {
                    Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                    framebuffer[offset] = color;
                }
            }


            
            /**
             * @brief Blends two colors
             *
             * @param src Source color
             * @param dst Destination color
             * @param alpha Opacity
             * @return Blended color
             */
            inline u8 blendColor(const u8 src, const u8 dst, const u8 alpha) {
                //const u8 inv_alpha = 15 - alpha;
                return (dst * alpha + src * (alpha ^ 15)) >> 4;
            }
            
            /**
             * @brief Draws a single source blended pixel onto the screen
             *
             * @param x X pos
             * @param y Y pos
             * @param color Color
             */
            inline void setPixelBlendSrc(const u32 x, const u32 y, const Color& color) {
                // Early exit for fully transparent pixels
                //if (color.a == 0)
                //    return;
                const u32 offset = this->getPixelOffset(x, y);
                if (offset == UINT32_MAX) [[unlikely]]
                    return;
                
                //const u16* framebuffer = static_cast<const u16*>(this->getCurrentFramebuffer());
                const Color src(static_cast<const u16*>(this->getCurrentFramebuffer())[offset]);
                
                // Inline the blending and Color construction
                //const Color end = {
                //    blendColor(src.r, color.r, color.a),
                //    blendColor(src.g, color.g, color.a), 
                //    blendColor(src.b, color.b, color.a),
                //    src.a
                //};
            
                this->setPixel(x, y, {blendColor(src.r, color.r, color.a), blendColor(src.g, color.g, color.a),  blendColor(src.b, color.b, color.a), src.a}, offset);
            }

            // Alternative batch version for processing multiple pixels at once
            inline void setPixelBlendSrcBatch(const u32 baseX, const u32 baseY, 
                                              const u8 red[16], const u8 green[16], 
                                              const u8 blue[16], const u8 alpha[16], 
                                              const s32 count) {
                // All variables moved outside the loop
                const u16* framebuffer = static_cast<const u16*>(this->getCurrentFramebuffer());
                u32 offset;
                u8 currentAlpha;
                Color src = {0}, end = {0};
                u32 currentX;
                
                for (s32 i = 0; i < count; ++i) {
                    // Early exit for transparent pixels
                    currentAlpha = alpha[i];
                    if (currentAlpha == 0)
                        continue;
                    
                    currentX = baseX + i;
                    offset = this->getPixelOffset(currentX, baseY);
                    if (offset == UINT32_MAX) [[unlikely]]
                        continue;
                    
                    // Direct framebuffer access and color construction
                    src = Color(framebuffer[offset]);
                    
                    // Direct member assignment instead of constructor
                    end.r = blendColor(src.r, red[i], currentAlpha);
                    end.g = blendColor(src.g, green[i], currentAlpha);
                    end.b = blendColor(src.b, blue[i], currentAlpha);
                    end.a = src.a;
                    
                    this->setPixel(currentX, baseY, end, offset);
                }
            }

            
            /**
             * @brief Draws a single destination blended pixel onto the screen
             *
             * @param x X pos
             * @param y Y pos
             * @param color Color
             */
            inline void setPixelBlendDst(const u32 x, const u32 y, const Color& color) {
                const u32 offset = this->getPixelOffset(x, y);
                if (offset == UINT32_MAX) [[unlikely]]
                    return;
                
                // Early exit for fully transparent pixels
                //if (color.a == 0)
                //    return;
                
                const u16* framebuffer = static_cast<const u16*>(this->getCurrentFramebuffer());
                const Color src(framebuffer[offset]);
                
                // Optimized alpha blending calculation
                const u8 invColorAlpha = 0xF - color.a;
                const Color end = {
                    blendColor(src.r, color.r, color.a),
                    blendColor(src.g, color.g, color.a),
                    blendColor(src.b, color.b, color.a),
                    static_cast<u8>(color.a + (src.a * invColorAlpha >> 4))  // Optimized alpha blend
                };
            
                this->setPixel(x, y, end, offset);
            }
            
            // Batch version for setPixelBlendDst
            inline void setPixelBlendDstBatch(const u32 baseX, const u32 baseY, 
                                              const u8 red[16], const u8 green[16], 
                                              const u8 blue[16], const u8 alpha[16], 
                                              const s32 count) {
                // All variables moved outside the loop
                const u16* framebuffer = static_cast<const u16*>(this->getCurrentFramebuffer());
                u32 offset;
                u8 currentAlpha;
                u8 invAlpha;
                Color src = {0}, end = {0};
                u32 currentX;
                
                for (s32 i = 0; i < count; ++i) {
                    // Early exit for transparent pixels
                    currentAlpha = alpha[i];
                    if (currentAlpha == 0)
                        continue;
                    
                    currentX = baseX + i;
                    offset = this->getPixelOffset(currentX, baseY);
                    if (offset == UINT32_MAX) [[unlikely]]
                        continue;
                    
                    // Direct framebuffer access and color construction
                    src = Color(framebuffer[offset]);
                    invAlpha = 0xF - currentAlpha;
                    
                    // Direct member assignment instead of constructor
                    end.r = blendColor(src.r, red[i], currentAlpha);
                    end.g = blendColor(src.g, green[i], currentAlpha);
                    end.b = blendColor(src.b, blue[i], currentAlpha);
                    end.a = static_cast<u8>(currentAlpha + (src.a * invAlpha >> 4));
                    
                    this->setPixel(currentX, baseY, end, offset);
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
                s32 dx = x1 - x0;
                s32 dy = y1 - y0;
                
                // Calculate absolute deltas and steps
                s32 abs_dx = dx < 0 ? -dx : dx;
                s32 abs_dy = dy < 0 ? -dy : dy;
                s32 step_x = dx < 0 ? -1 : 1;
                s32 step_y = dy < 0 ? -1 : 1;
                
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

                s32 dx = x_max - x_min;
                s32 dy = y_max - y_min;
                s32 d = 2 * dy - dx;
                s32 incrE = 2*dy;
                s32 incrNE = 2*(dy - dx);

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
                s32 x = radius;
                s32 y = 0;
                s32 radiusError = 0;
                s32 xChange = 1 - (radius << 1);
                s32 yChange = 0;
                
                while (x >= y) {
                    if (filled) {
                        for (s32 i = centerX - x; i <= centerX + x; i++) {
                            this->setPixelBlendDst(i, centerY + y, color);
                            this->setPixelBlendDst(i, centerY - y, color);
                        }
                        
                        for (s32 i = centerX - y; i <= centerX + y; i++) {
                            this->setPixelBlendDst(i, centerY + x, color);
                            this->setPixelBlendDst(i, centerY - x, color);
                        }
                    } else {
                        this->setPixelBlendDst(centerX + x, centerY + y, color);
                        this->setPixelBlendDst(centerX + y, centerY + x, color);
                        this->setPixelBlendDst(centerX - y, centerY + x, color);
                        this->setPixelBlendDst(centerX - x, centerY + y, color);
                        this->setPixelBlendDst(centerX - x, centerY - y, color);
                        this->setPixelBlendDst(centerX - y, centerY - x, color);
                        this->setPixelBlendDst(centerX + y, centerY - x, color);
                        this->setPixelBlendDst(centerX + x, centerY - y, color);
                    }
                    
                    y++;
                    radiusError += yChange;
                    yChange += 2;
                    
                    if (((radiusError << 1) + xChange) > 0) {
                        x--;
                        radiusError += xChange;
                        xChange += 2;
                    }
                }
            }

            //inline void drawQuarterCircle(s32 centerX, s32 centerY, u16 radius, bool filled, const Color& color, int quadrant) {
            //    s32 x = radius;
            //    s32 y = 0;
            //    s32 radiusError = 0;
            //    s32 xChange = 1 - (radius << 1);
            //    s32 yChange = 0;
            //    
            //    while (x >= y) {
            //        if (filled) {
            //            switch (quadrant) {
            //                case 1: // Top-right
            //                    for (s32 i = centerX; i <= centerX + x; i++) {
            //                        this->setPixelBlendDst(i, centerY - y, color);
            //                    }
            //                    for (s32 i = centerX; i <= centerX + y; i++) {
            //                        this->setPixelBlendDst(i, centerY - x, color);
            //                    }
            //                    break;
            //                case 2: // Top-left
            //                    for (s32 i = centerX - x; i <= centerX; i++) {
            //                        this->setPixelBlendDst(i, centerY - y, color);
            //                    }
            //                    for (s32 i = centerX - y; i <= centerX; i++) {
            //                        this->setPixelBlendDst(i, centerY - x, color);
            //                    }
            //                    break;
            //                case 3: // Bottom-left
            //                    for (s32 i = centerX - x; i <= centerX; i++) {
            //                        this->setPixelBlendDst(i, centerY + y, color);
            //                    }
            //                    for (s32 i = centerX - y; i <= centerX; i++) {
            //                        this->setPixelBlendDst(i, centerY + x, color);
            //                    }
            //                    break;
            //                case 4: // Bottom-right
            //                    for (s32 i = centerX; i <= centerX + x; i++) {
            //                        this->setPixelBlendDst(i, centerY + y, color);
            //                    }
            //                    for (s32 i = centerX; i <= centerX + y; i++) {
            //                        this->setPixelBlendDst(i, centerY + x, color);
            //                    }
            //                    break;
            //            }
            //        } else {
            //            switch (quadrant) {
            //                case 1: // Top-right
            //                    this->setPixelBlendDst(centerX + x, centerY - y, color);
            //                    this->setPixelBlendDst(centerX + y, centerY - x, color);
            //                    break;
            //                case 2: // Top-left
            //                    this->setPixelBlendDst(centerX - x, centerY - y, color);
            //                    this->setPixelBlendDst(centerX - y, centerY - x, color);
            //                    break;
            //                case 3: // Bottom-left
            //                    this->setPixelBlendDst(centerX - x, centerY + y, color);
            //                    this->setPixelBlendDst(centerX - y, centerY + x, color);
            //                    break;
            //                case 4: // Bottom-right
            //                    this->setPixelBlendDst(centerX + x, centerY + y, color);
            //                    this->setPixelBlendDst(centerX + y, centerY + x, color);
            //                    break;
            //            }
            //        }
            //        
            //        y++;
            //        radiusError += yChange;
            //        yChange += 2;
            //        
            //        if (((radiusError << 1) + xChange) > 0) {
            //            x--;
            //            radiusError += xChange;
            //            xChange += 2;
            //        }
            //    }
            //}
            
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
                
                // Draw borders (unchanged for exact visual match)
                this->drawRect(startX, startY - thickness, adjustedWidth, thickness, highlightColor); // Top border
                this->drawRect(startX, startY + adjustedHeight, adjustedWidth, thickness, highlightColor); // Bottom border
                this->drawRect(startX - thickness, startY, thickness, adjustedHeight, highlightColor); // Left border
                this->drawRect(startX + adjustedWidth, startY, thickness, adjustedHeight, highlightColor); // Right border
                
                // Optimized filled quarter circle drawing - all 4 corners in one pass
                s32 cx = radius;
                s32 cy = 0;
                s32 radiusError = 0;
                s32 xChange = 1 - (radius << 1);
                s32 yChange = 0;
                
                while (cx >= cy) {
                    // Draw horizontal spans for all 4 corners simultaneously
                    // Upper-left corner (quadrant 2) - two horizontal lines
                    for (s32 i = leftCornerX - cx; i <= leftCornerX; i++) {
                        this->setPixelBlendDst(i, topCornerY - cy, highlightColor);
                    }
                    for (s32 i = leftCornerX - cy; i <= leftCornerX; i++) {
                        this->setPixelBlendDst(i, topCornerY - cx, highlightColor);
                    }
                    
                    // Lower-left corner (quadrant 3) - two horizontal lines
                    for (s32 i = leftCornerX - cx; i <= leftCornerX; i++) {
                        this->setPixelBlendDst(i, bottomCornerY + cy, highlightColor);
                    }
                    for (s32 i = leftCornerX - cy; i <= leftCornerX; i++) {
                        this->setPixelBlendDst(i, bottomCornerY + cx, highlightColor);
                    }
                    
                    // Upper-right corner (quadrant 1) - two horizontal lines
                    for (s32 i = rightCornerX; i <= rightCornerX + cx; i++) {
                        this->setPixelBlendDst(i, topCornerY - cy, highlightColor);
                    }
                    for (s32 i = rightCornerX; i <= rightCornerX + cy; i++) {
                        this->setPixelBlendDst(i, topCornerY - cx, highlightColor);
                    }
                    
                    // Lower-right corner (quadrant 4) - two horizontal lines
                    for (s32 i = rightCornerX; i <= rightCornerX + cx; i++) {
                        this->setPixelBlendDst(i, bottomCornerY + cy, highlightColor);
                    }
                    for (s32 i = rightCornerX; i <= rightCornerX + cy; i++) {
                        this->setPixelBlendDst(i, bottomCornerY + cx, highlightColor);
                    }
                    
                    // Bresenham circle algorithm step
                    cy++;
                    radiusError += yChange;
                    yChange += 2;
                    
                    if (((radiusError << 1) + xChange) > 0) {
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

            // Define processChunk as a static member function
            static void processRoundedRectChunk(Renderer* self, const s32 x, const s32 y, const s32 x_end, const s32 y_end, const s32 r2, const s32 radius, const Color& color, const s32 startRow, const s32 endRow) {
                // All constant calculations moved outside loops
                const s32 x_left = x + radius;
                const s32 x_right = x_end - radius;
                const s32 y_top = y + radius;
                const s32 y_bottom = y_end - radius;
                const s32 total_height = y_end - y;
                
                // Use stack allocation for small arrays to avoid heap allocation
                alignas(64) u8 redArray[512], greenArray[512], blueArray[512], alphaArray[512];
                const u8 red = color.r;
                const u8 green = color.g;
                const u8 blue = color.b;
                const u8 alpha = color.a;
                
                // SIMD-optimized color array initialization (if available)
                #ifdef __AVX2__
                const __m256i color_vec = _mm256_set1_epi32((alpha << 24) | (blue << 16) | (green << 8) | red);
                for (s32 i = 0; i < 512; i += 8) {
                    _mm256_store_si256((__m256i*)(redArray + i), _mm256_and_si256(color_vec, _mm256_set1_epi32(0xFF)));
                    _mm256_store_si256((__m256i*)(greenArray + i), _mm256_and_si256(_mm256_srli_epi32(color_vec, 8), _mm256_set1_epi32(0xFF)));
                    _mm256_store_si256((__m256i*)(blueArray + i), _mm256_and_si256(_mm256_srli_epi32(color_vec, 16), _mm256_set1_epi32(0xFF)));
                    _mm256_store_si256((__m256i*)(alphaArray + i), _mm256_and_si256(_mm256_srli_epi32(color_vec, 24), _mm256_set1_epi32(0xFF)));
                }
                #else
                // Fallback: unroll the loop for better performance
                for (s32 i = 0; i < 512; i += 8) {
                    redArray[i] = redArray[i+1] = redArray[i+2] = redArray[i+3] = 
                    redArray[i+4] = redArray[i+5] = redArray[i+6] = redArray[i+7] = red;
                    greenArray[i] = greenArray[i+1] = greenArray[i+2] = greenArray[i+3] = 
                    greenArray[i+4] = greenArray[i+5] = greenArray[i+6] = greenArray[i+7] = green;
                    blueArray[i] = blueArray[i+1] = blueArray[i+2] = blueArray[i+3] = 
                    blueArray[i+4] = blueArray[i+5] = blueArray[i+6] = blueArray[i+7] = blue;
                    alphaArray[i] = alphaArray[i+1] = alphaArray[i+2] = alphaArray[i+3] = 
                    alphaArray[i+4] = alphaArray[i+5] = alphaArray[i+6] = alphaArray[i+7] = alpha;
                }
                #endif
                
                // Only compute spans for rows we actually need
                const s32 first_row_idx = std::max(0, startRow - y);
                const s32 last_row_idx = std::min(total_height - 1, endRow - y - 1);
                
                // Use stack allocation for small span arrays
                HorizontalSpan spans[512]; // Assuming reasonable max height
                const bool use_heap = (last_row_idx - first_row_idx + 1) > 512;
                std::vector<HorizontalSpan> heap_spans;
                
                HorizontalSpan* span_ptr;
                if (use_heap) {
                    heap_spans.resize(last_row_idx - first_row_idx + 1);
                    span_ptr = heap_spans.data();
                } else {
                    span_ptr = spans;
                }
                
                // Pre-compute spans only for needed rows
                s32 span_idx = 0;
                for (s32 row_idx = first_row_idx; row_idx <= last_row_idx; ++row_idx) {
                    const s32 y1 = y + row_idx;
                    
                    if (y1 >= y_top && y1 < y_bottom) {
                        // Middle section - direct assignment
                        span_ptr[span_idx].start_x = x;
                        span_ptr[span_idx].end_x = x_end;
                    } else {
                        // Corner section - optimize with lookup table for small radii
                        const bool isTopSection = (y1 < y_top);
                        const s32 corner_y = isTopSection ? y_top : y_bottom;
                        const s32 dy = abs(y1 - corner_y);
                        const s32 dy2 = dy * dy;
                        
                        if (dy2 > r2) {
                            span_ptr[span_idx].start_x = 0;
                            span_ptr[span_idx].end_x = 0; // Empty span
                        } else {
                            // Use fast integer square root for better performance
                            const s32 max_dx = static_cast<s32>(std::sqrt(r2 - dy2));
                            span_ptr[span_idx].start_x = std::max(x_left - max_dx, x);
                            span_ptr[span_idx].end_x = std::min(x_right + max_dx, x_end);
                        }
                    }
                    ++span_idx;
                }
                
                // Render using pre-computed spans with larger batch sizes
                span_idx = 0;
                for (s32 y_current = startRow; y_current < endRow; ++y_current) {
                    const s32 row_idx = y_current - y;
                    if (row_idx < first_row_idx || row_idx > last_row_idx) continue;
                    
                    const HorizontalSpan* span = &span_ptr[span_idx++];
                    if (span->start_x >= span->end_x) continue;
                    
                    // Draw with larger batch size for better cache utilization
                    s32 x_pos = span->start_x;
                    while (x_pos < span->end_x) {
                        const s32 remaining = span->end_x - x_pos;
                        const s32 batch_size = std::min(remaining, 512);
                        
                        self->setPixelBlendDstBatch(x_pos, y_current, redArray, greenArray, blueArray, alphaArray, batch_size);
                        x_pos += batch_size;
                    }
                }
            }



            /**
             * @brief Draws a rounded rectangle of given sizes and corner radius
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
                
                // For small rectangles, use single-threaded version
                if (w * h < 1000) {
                    drawRoundedRectSingleThreaded(x, y, w, h, radius, color);
                    return;
                }
                
                // Use the existing multi-threaded approach but with better chunk sizes
                s32 x_end = x + w;
                s32 y_end = y + h;
                s32 r2 = radius * radius;
                
                // Dynamic chunk size based on rectangle size
                s32 chunkSize = std::max((s32)1, (s32)(h / (ult::numThreads * 2)));
                std::atomic<s32> currentRow(y);
                
                auto threadTask = [&]() {
                    s32 startRow, endRow;
                    while ((startRow = currentRow.fetch_add(chunkSize)) < y_end) {
                        endRow = std::min(startRow + chunkSize, y_end);
                        processRoundedRectChunk(this, x, y, x_end, y_end, r2, radius, color, startRow, endRow);
                    }
                };
                
                // Launch threads
                std::vector<std::thread> threads;
                threads.reserve(ult::numThreads);
                
                for (unsigned i = 0; i < ult::numThreads; ++i) {
                    threads.emplace_back(threadTask);
                }
                
                for (auto& t : threads) {
                    t.join();
                }
            }


            inline void drawRoundedRectSingleThreaded(const s32 x, const s32 y, const s32 w, const s32 h, const s32 radius, const Color& color) {
                //const s32 x_end = x + w;
                const s32 y_end = y + h;
                //const s32 r2 = radius * radius;
                
                // Call the processRoundedRectChunk function directly for the entire rectangle
                processRoundedRectChunk(this, x, y, x + w, y_end, radius * radius, radius, color, y, y_end);
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
                // Early exit for degenerate cases
                //if (w <= 0 || h <= 0) return;
                
                // Radius is half of height to create perfect half circles on each side
                const s32 radius = h / 2;
                //if (radius <= 0) return;
                
                const s32 x_start = x + radius;
                const s32 x_end = x + w - radius;
                const s32 radius_sq = radius * radius;
                const s32 center_y = y + radius;
                
                // Pre-declare all loop variables outside loops
                s32 y1, x1, x_offset;
                s32 dy, dy_sq, dist_sq;
                s32 max_x_offset_for_row;
                s32 left_x, right_x;
                
                // Draw the central rectangle (unchanged - this is already optimal)
                for (y1 = y; y1 < y + h; ++y1) {
                    for (x1 = x_start; x1 < x_end; ++x1) {
                        this->setPixelBlendDst(x1, y1, color);
                    }
                }
                
                // Optimized semicircle drawing with precomputed values
                for (y1 = y; y1 < y + h; ++y1) {
                    dy = y1 - center_y;
                    dy_sq = dy * dy;
                    
                    // Calculate the exact maximum x_offset for this row to avoid unnecessary iterations
                    // max_x_offset^2 + dy^2 = radius^2, so max_x_offset = sqrt(radius^2 - dy^2)
                    if (dy_sq >= radius_sq) continue; // Skip rows completely outside the circle
                    
                    // Use integer square root approximation or simple bound
                    max_x_offset_for_row = radius; // Conservative bound, could be optimized further
                    
                    // Precompute the x coordinates
                    left_x = x + radius;
                    right_x = x + w - radius;
                    
                    for (x_offset = 0; x_offset < max_x_offset_for_row; ++x_offset) {
                        dist_sq = x_offset * x_offset + dy_sq;
                        
                        if (dist_sq <= radius_sq) {
                            // Left semicircle
                            this->setPixelBlendDst(left_x - x_offset, y1, color);
                            // Right semicircle (only if x_offset > 0 to avoid double-drawing center)
                            if (x_offset > 0) {
                                this->setPixelBlendDst(right_x + x_offset, y1, color);
                            }
                        } else {
                            // Once we're outside the circle, no need to check further x values
                            break;
                        }
                    }
                }
            }
                        
            // Struct for batch pixel processing with better alignment
            struct alignas(64) PixelBatch {
                s32 baseX, baseY;
                u8 red[32], green[32], blue[32], alpha[32];  // Doubled for 32-pixel batches
                s32 count;
            };
            
            // Batch pixel setter - process multiple pixels at once if available
            inline void setPixelBatchBlendSrc(const s32 baseX, const s32 baseY, const PixelBatch& batch) {
                // If your graphics system supports batch operations, use them here
                // Otherwise fall back to individual calls
                for (s32 i = 0; i < batch.count; ++i) {
                    setPixelBlendSrc(baseX + i, baseY, {
                        batch.red[i], batch.green[i], batch.blue[i], batch.alpha[i]
                    });
                }
            }

            // Fixed compilation errors - simplified SIMD version
            const uint8x16_t lut = {0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255};
            const uint8x16_t mask_low = vdupq_n_u8(0x0F);
            // Pre-computed lookup table for 4-bit to 8-bit conversion
            const u8 expand4to8[16] = {
                0, 17, 34, 51, 68, 85, 102, 119, 136, 153, 170, 187, 204, 221, 238, 255
            };
            
            inline void processBMPChunk(const s32 x, const s32 y, const s32 screenW, const u8 *preprocessedData, 
                                       const s32 startRow, const s32 endRow) {
                const s32 bytesPerRow = screenW * 2;
                const s32 endX16 = screenW & ~15;
                
                // Pre-declare all variables outside loops to avoid repeated allocations
                const u8 *rowPtr;
                s32 baseY;
                s32 x1;
                const u8* ptr;
                uint8x16x2_t packed;
                uint8x16_t high1, low1, high2, low2;
                uint8x16_t red, green, blue, alpha;
                alignas(16) u8 red_vals[16], green_vals[16], blue_vals[16], alpha_vals[16];
                s32 baseX;
                s32 pixelX;
                u32 offset;
                Color color = {0}, src = {0}, end = {0};
                const u16* framebuffer;
                u8 p1, p2;
                
                for (s32 y1 = startRow; y1 < endRow; ++y1) {
                    rowPtr = preprocessedData + (y1 * bytesPerRow);
                    baseY = y + y1;
                    
                    x1 = 0;
                    
                    // SIMD processing for 16 pixels at once
                    for (; x1 < endX16; x1 += 16) {
                        ptr = rowPtr + (x1 << 1);
                        packed = vld2q_u8(ptr);
                        
                        // Expand 4-bit to 8-bit values
                        high1 = vshrq_n_u8(packed.val[0], 4);
                        low1  = vandq_u8(packed.val[0], mask_low);
                        high2 = vshrq_n_u8(packed.val[1], 4);
                        low2  = vandq_u8(packed.val[1], mask_low);
                        
                        red   = vqtbl1q_u8(lut, high1);
                        green = vqtbl1q_u8(lut, low1);
                        blue  = vqtbl1q_u8(lut, high2);
                        alpha = vqtbl1q_u8(lut, low2);
                        
                        // Store to arrays and process individually
                        vst1q_u8(red_vals, red);
                        vst1q_u8(green_vals, green); 
                        vst1q_u8(blue_vals, blue);
                        vst1q_u8(alpha_vals, alpha);
                        
                        baseX = x + x1;
                        
                        // Process 16 pixels with minimal function call overhead
                        for (int i = 0; i < 16; ++i) {
                            // Skip transparent pixels
                            if (alpha_vals[i] == 0) continue;
                            
                            pixelX = baseX + i;
                            offset = this->getPixelOffset(pixelX, baseY);
                            
                            if (offset != UINT32_MAX) {
                                color = {red_vals[i], green_vals[i], blue_vals[i], alpha_vals[i]};
                                
                                framebuffer = static_cast<const u16*>(this->getCurrentFramebuffer());
                                src = Color(framebuffer[offset]);
                                
                                end = {
                                    blendColor(src.r, color.r, color.a),
                                    blendColor(src.g, color.g, color.a), 
                                    blendColor(src.b, color.b, color.a),
                                    src.a
                                };
                            
                                this->setPixel(pixelX, baseY, end, offset);
                            }
                        }
                    }
                    
                    // Handle remaining pixels (less than 16)
                    for (; x1 < screenW; ++x1) {
                        p1 = rowPtr[x1 << 1];
                        p2 = rowPtr[(x1 << 1) + 1];
                        
                        setPixelBlendSrc(x + x1, baseY, {
                            expand4to8[p1 >> 4], expand4to8[p1 & 0x0F],
                            expand4to8[p2 >> 4], expand4to8[p2 & 0x0F]
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

            inline void drawBitmapRGBA4444(const s32 x, const s32 y, const s32 screenW, const s32 screenH, const u8 *preprocessedData) {
                s32 startRow;

                // Divide rows among ult::threads
                //s32 chunkSize = (screenH + ult::numThreads - 1) / ult::numThreads;
                for (unsigned i = 0; i < ult::numThreads; ++i) {
                    startRow = i * ult::bmpChunkSize;
                    //s32 endRow = std::min(startRow + ult::bmpChunkSize, screenH);
                    
                    // Bind the member function and create the thread
                    ult::threads[i] = std::thread(std::bind(&tsl::gfx::Renderer::processBMPChunk, this, x, y, screenW, preprocessedData, startRow, std::min(startRow + ult::bmpChunkSize, screenH)));
                }
            
                // Join all ult::threads
                for (auto& t : ult::threads) {
                    t.join();
                }
            }


            inline void drawWallpaper() {
                if (ult::expandedMemory && !ult::refreshWallpaper.load(std::memory_order_acquire)) {
                    //ult::inPlot = true;
                    ult::inPlot.store(true, std::memory_order_release);
                    //std::lock_guard<std::mutex> lock(wallpaperMutex);
                    if (!ult::wallpaperData.empty()) {
                        // Draw the bitmap at position (0, 0) on the screen
                        //static bool ult::correctFrameSize = (cfg::FramebufferWidth == 448 && cfg::FramebufferHeight == 720);
                        if (!ult::refreshWallpaper.load(std::memory_order_acquire) && ult::correctFrameSize) { // hard coded width and height for consistency
                            drawBitmapRGBA4444(0, 0, cfg::FramebufferWidth, cfg::FramebufferHeight, ult::wallpaperData.data());
                        } else
                            ult::inPlot.store(false, std::memory_order_release);
                    } else {
                        ult::inPlot.store(false, std::memory_order_release);
                    }
                    //ult::inPlot = false;
                }
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
                
                s32 px;

                // Completely unroll small bitmaps for maximum speed
                if (w <= 8 && h <= 8) [[likely]] {
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
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel7: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel6: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel5: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel4: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel3: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel2: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c)); src += 4;
                        }
                        pixel1: {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px, rowY, a(c)); src += 4;
                        }
                    }
                    return;
                }
                
                // Fallback to vectorized version for larger bitmaps
                const s32 vectorWidth = w & ~7; // Process 8 pixels at a time
                const s32 remainder = w & 7;
                
                for (s32 py = 0; py < h; ++py) {
                    const s32 rowY = y + py;
                    px = x;
                    
                    // Process 8 pixels at once (cache-friendly)
                    for (s32 i = 0; i < vectorWidth; i += 8) {
                        // Prefetch next cache line
                        __builtin_prefetch(src + 64, 0, 3);
                        
                        // Process 8 pixels with minimal overhead
                        for (int j = 0; j < 8; ++j) {
                            const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                           static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                            setPixelBlendSrc(px++, rowY, a(c));
                            src += 4;
                        }
                    }
                    
                    // Handle remainder
                    for (s32 i = 0; i < remainder; ++i) {
                        const Color c = {static_cast<u8>(src[0] >> 4), static_cast<u8>(src[1] >> 4), 
                                       static_cast<u8>(src[2] >> 4), static_cast<u8>(src[3] >> 4)};
                        setPixelBlendSrc(px++, rowY, a(c));
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
            
            struct Glyph {
                stbtt_fontinfo *currFont;
                float currFontSize;
                int bounds[4];
                int xAdvance;
                u8 *glyphBmp;
                int width, height;
            };

            const stbtt_fontinfo& getStandardFont() const {
                return m_stdFont;
            }
            
            // Ultra-optimized renderTextSegment with extreme performance enhancements
            // Fixed renderTextSegment that exactly matches original behavior
            inline void renderTextSegment(const char* textPtr, size_t length, float& currX, float& currY, float& maxX,
                                          const u32 fontSize, const Color& color, const u64 keyBase,
                                          std::unordered_map<u64, Glyph>& glyphCache, 
                                          bool monospace = false, const ssize_t maxWidth = 0, const float startX = 0) {
                
                if (length == 0 || color.a == 0x0) [[unlikely]] return;
                
                const char* strPtr = textPtr;
                const char* const strEnd = textPtr + length;
                
                // Pre-declare all variables for maximum register usage
                u32 currCharacter;
                ssize_t codepointWidth;
                u64 key;
                Glyph* glyph;
                const uint8_t* bmpPtr;
                float xPos, yPos;
                s32 width, height;
                uint8_t alpha;
                
                // Ultra-fast ASCII detection using 64-bit word scanning
                bool isAsciiOnly = true;
                if (length >= 8) {
                    const uint64_t* wordPtr = reinterpret_cast<const uint64_t*>(textPtr);
                    const uint64_t* wordEnd = reinterpret_cast<const uint64_t*>(textPtr + (length & ~7));
                    
                    // Check 8 bytes at once for non-ASCII
                    while (wordPtr < wordEnd) {
                        if (*wordPtr & 0x8080808080808080ULL) {
                            isAsciiOnly = false;
                            break;
                        }
                        ++wordPtr;
                    }
                    
                    // Check remaining bytes if still ASCII
                    if (isAsciiOnly) {
                        for (size_t i = (length & ~7); i < length; ++i) {
                            if (static_cast<unsigned char>(textPtr[i]) > 127) {
                                isAsciiOnly = false;
                                break;
                            }
                        }
                    }
                } else {
                    // For small strings, simple loop
                    for (size_t i = 0; i < length; ++i) {
                        if (static_cast<unsigned char>(textPtr[i]) > 127) {
                            isAsciiOnly = false;
                            break;
                        }
                    }
                }
                
                // Main character processing loop - EXACTLY like original
                while (strPtr < strEnd) {
                    // EXACT same maxWidth check as original
                    if (maxWidth > 0 && (currX - startX) >= maxWidth)
                        break;
                    
                    // Dual-path character decoding
                    if (isAsciiOnly) [[likely]] {
                        currCharacter = static_cast<u32>(*strPtr);
                        ++strPtr;
                    } else {
                        codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(strPtr));
                        if (codepointWidth <= 0) break;
                        strPtr += codepointWidth;
                    }
                    
                    // EXACT same newline handling as original
                    if (currCharacter == '\n') {
                        maxX = std::max(currX, maxX);
                        currX = startX;
                        currY += fontSize;
                        continue;
                    }
                    
                    // Optimized glyph key calculation
                    key = (static_cast<u64>(currCharacter) << 32) | keyBase;
                    
                    // Cache lookup
                    auto it = glyphCache.find(key);
                    if (it == glyphCache.end()) {
                        auto [insertIt, inserted] = glyphCache.emplace(key, Glyph{});
                        glyph = &insertIt->second;
                        
                        // Font selection - same as original
                        if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter)) {
                            glyph->currFont = &this->m_extFont;
                        } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, currCharacter) == 0) {
                            glyph->currFont = &this->m_localFont;
                        } else {
                            glyph->currFont = &this->m_stdFont;
                        }
            
                        const float scaledFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
                        glyph->currFontSize = scaledFontSize;
            
                        stbtt_GetCodepointBitmapBoxSubpixel(glyph->currFont, currCharacter, scaledFontSize, scaledFontSize,
                                                            0, 0, &glyph->bounds[0], &glyph->bounds[1], &glyph->bounds[2], &glyph->bounds[3]);
            
                        s32 yAdvance = 0;
                        stbtt_GetCodepointHMetrics(glyph->currFont, monospace ? 'W' : currCharacter, &glyph->xAdvance, &yAdvance);
            
                        glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont, scaledFontSize, scaledFontSize, 
                                                                   currCharacter, &glyph->width, &glyph->height, nullptr, nullptr);
                    } else {
                        glyph = &it->second;
                    }
                    
                    // EXACT same rendering condition as original
                    if (glyph->glyphBmp != nullptr && !std::iswspace(currCharacter) && fontSize > 0 && color.a != 0x0) {
                        xPos = currX + glyph->bounds[0];
                        yPos = currY + glyph->bounds[1];
                        bmpPtr = glyph->glyphBmp;
                        width = glyph->width;
                        height = glyph->height;
                        
                        s32 col;

                        // Optimized pixel blitting
                        for (s32 row = 0; row < height; ++row) {
                            const float currentY = yPos + row;
                            const uint8_t* rowPtr = bmpPtr + (row * width);
                            
                            col = 0;
                            const s32 simdWidth = width & ~7;
                            
                            // Process 8 pixels at once
                            for (; col < simdWidth; col += 8) {
                                const uint8_t a0 = rowPtr[col] >> 4;
                                const uint8_t a1 = rowPtr[col + 1] >> 4;
                                const uint8_t a2 = rowPtr[col + 2] >> 4;
                                const uint8_t a3 = rowPtr[col + 3] >> 4;
                                const uint8_t a4 = rowPtr[col + 4] >> 4;
                                const uint8_t a5 = rowPtr[col + 5] >> 4;
                                const uint8_t a6 = rowPtr[col + 6] >> 4;
                                const uint8_t a7 = rowPtr[col + 7] >> 4;
                                
                                const float baseX = xPos + col;
                                
                                if (a0 == 0xF) {
                                    this->setPixel(baseX, currentY, color, this->getPixelOffset(baseX, currentY));
                                } else if (a0 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a0;
                                    this->setPixelBlendDst(baseX, currentY, tmpColor);
                                }
                                
                                if (a1 == 0xF) {
                                    this->setPixel(baseX + 1, currentY, color, this->getPixelOffset(baseX + 1, currentY));
                                } else if (a1 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a1;
                                    this->setPixelBlendDst(baseX + 1, currentY, tmpColor);
                                }
                                
                                if (a2 == 0xF) {
                                    this->setPixel(baseX + 2, currentY, color, this->getPixelOffset(baseX + 2, currentY));
                                } else if (a2 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a2;
                                    this->setPixelBlendDst(baseX + 2, currentY, tmpColor);
                                }
                                
                                if (a3 == 0xF) {
                                    this->setPixel(baseX + 3, currentY, color, this->getPixelOffset(baseX + 3, currentY));
                                } else if (a3 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a3;
                                    this->setPixelBlendDst(baseX + 3, currentY, tmpColor);
                                }
                                
                                if (a4 == 0xF) {
                                    this->setPixel(baseX + 4, currentY, color, this->getPixelOffset(baseX + 4, currentY));
                                } else if (a4 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a4;
                                    this->setPixelBlendDst(baseX + 4, currentY, tmpColor);
                                }
                                
                                if (a5 == 0xF) {
                                    this->setPixel(baseX + 5, currentY, color, this->getPixelOffset(baseX + 5, currentY));
                                } else if (a5 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a5;
                                    this->setPixelBlendDst(baseX + 5, currentY, tmpColor);
                                }
                                
                                if (a6 == 0xF) {
                                    this->setPixel(baseX + 6, currentY, color, this->getPixelOffset(baseX + 6, currentY));
                                } else if (a6 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a6;
                                    this->setPixelBlendDst(baseX + 6, currentY, tmpColor);
                                }
                                
                                if (a7 == 0xF) {
                                    this->setPixel(baseX + 7, currentY, color, this->getPixelOffset(baseX + 7, currentY));
                                } else if (a7 != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = a7;
                                    this->setPixelBlendDst(baseX + 7, currentY, tmpColor);
                                }
                            }
                            
                            // Handle remaining pixels
                            for (; col < width; ++col) {
                                alpha = rowPtr[col] >> 4;
                                if (alpha == 0xF) {
                                    this->setPixel(xPos + col, currentY, color, this->getPixelOffset(xPos + col, currentY));
                                } else if (alpha != 0x0) {
                                    Color tmpColor = color;
                                    tmpColor.a = alpha;
                                    this->setPixelBlendDst(xPos + col, currentY, tmpColor);
                                }
                            }
                        }
                    }
                    
                    // Advance cursor - EXACT same as original
                    currX += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                }
            }
            
            // Optimized drawString that returns EXACTLY the same values as original
            inline std::pair<s32, s32> drawString(const std::string& originalString, bool monospace, const s32 x, const s32 y, const u32 fontSize, const Color& color, const ssize_t maxWidth = 0) {
                
                #ifdef UI_OVERRIDE_PATH
                // Check for translation in the cache
                auto translatedIt = ult::translationCache.find(originalString);
                std::string translatedString = (translatedIt != ult::translationCache.end()) ? translatedIt->second : originalString;
            
                // Cache the translation if it wasn't already present
                if (translatedIt == ult::translationCache.end()) {
                    ult::translationCache[originalString] = translatedString;
                }
                const std::string* stringPtr = &translatedString;
                #else
                const std::string* stringPtr = &originalString;
                #endif
            
                // Early exit optimizations
                //if (stringPtr->empty() || fontSize == 0 || color.a == 0x0) {
                //    return {0, 0};
                //}
            
                // Variable declarations moved outside loops
                float maxX = x;
                float currX = x;
                float currY = y;
                
                const auto itStrEnd = stringPtr->cend();
                auto itStr = stringPtr->cbegin();
                
                u32 currCharacter = 0;
                ssize_t codepointWidth = 0;
                u64 key = 0;
                Glyph* glyph = nullptr;
                
                float xPos = 0;
                float yPos = 0;
                u32 rowOffset = 0;
                Color tmpColor(0);
                
                // Pre-calculate constants
                const s32 framebufferWidth = static_cast<s32>(cfg::FramebufferWidth);
                const s32 framebufferHeight = static_cast<s32>(cfg::FramebufferHeight);
                const s32 maxWidthLimit = x + maxWidth;
                const bool hasMaxWidth = maxWidth > 0;
                
                // Pixel processing variables
                s32 glyphLeft, glyphTop, glyphRight, glyphBottom;
                s32 clipLeft, clipTop, clipRight, clipBottom;
                s32 startX, startY, endX, endY;
                s32 bmpX, bmpY;
                s32 simdEnd, vectorEnd;
                uint8_t alpha;
                float pixelX, pixelY, baseX;
                u32 offset;
                const uint8_t* rowPtr;
                
                // Alpha values for SIMD processing
                uint8_t a0, a1, a2, a3, a4, a5, a6, a7;
                u32 offset0, offset1, offset2, offset3, offset4, offset5, offset6, offset7;
                
                float scaledFontSize;
                s32 yAdvance = 0;
                
                // Static glyph cache
                static std::unordered_map<u64, Glyph> s_glyphCache;
                auto cacheIt = s_glyphCache.end();
            
                // Ultra-fast ASCII detection for the entire string
                const size_t strLen = stringPtr->size();
                bool isAsciiOnly = true;
                
                if (strLen >= 8) {
                    const uint64_t* wordPtr = reinterpret_cast<const uint64_t*>(stringPtr->data());
                    const uint64_t* wordEnd = reinterpret_cast<const uint64_t*>(stringPtr->data() + (strLen & ~7));
                    
                    while (wordPtr < wordEnd) {
                        if (*wordPtr & 0x8080808080808080ULL) {
                            isAsciiOnly = false;
                            break;
                        }
                        ++wordPtr;
                    }
                    
                    if (isAsciiOnly) {
                        for (size_t i = (strLen & ~7); i < strLen; ++i) {
                            if (static_cast<unsigned char>((*stringPtr)[i]) > 127) {
                                isAsciiOnly = false;
                                break;
                            }
                        }
                    }
                } else {
                    for (size_t i = 0; i < strLen; ++i) {
                        if (static_cast<unsigned char>((*stringPtr)[i]) > 127) {
                            isAsciiOnly = false;
                            break;
                        }
                    }
                }
            
                // Main character processing loop
                while (itStr != itStrEnd) {
                    // Early maxWidth check with pre-calculated limit
                    if (hasMaxWidth && currX >= maxWidthLimit)
                        break;
            
                    // Optimized UTF-8 decoding
                    if (isAsciiOnly) {
                        currCharacter = static_cast<u32>(*itStr);
                        codepointWidth = 1;
                        ++itStr;
                    } else {
                        codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                        if (codepointWidth <= 0)
                            break;
                        itStr += codepointWidth;
                    }
            
                    // Newline handling
                    if (currCharacter == '\n') {
                        maxX = std::max(currX, maxX);
                        currX = x;
                        currY += fontSize;
                        continue;
                    }
            
                    // Calculate glyph key
                    key = (static_cast<u64>(currCharacter) << 32) | (static_cast<u64>(monospace) << 31) | static_cast<u64>(fontSize);
            
                    // Check cache for the glyph
                    cacheIt = s_glyphCache.find(key);
            
                    // If glyph not found, create and cache it
                    if (cacheIt == s_glyphCache.end()) {
                        glyph = &s_glyphCache.emplace(key, Glyph()).first->second;
                        
                        // Determine the appropriate font for the character
                        if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter)) {
                            glyph->currFont = &this->m_extFont;
                        } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, currCharacter) == 0) {
                            glyph->currFont = &this->m_localFont;
                        } else {
                            glyph->currFont = &this->m_stdFont;
                        }
            
                        scaledFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
                        glyph->currFontSize = scaledFontSize;
            
                        // Get glyph bitmap and metrics
                        stbtt_GetCodepointBitmapBoxSubpixel(glyph->currFont, currCharacter, scaledFontSize, scaledFontSize,
                                                            0, 0, &glyph->bounds[0], &glyph->bounds[1], &glyph->bounds[2], &glyph->bounds[3]);
            
                        stbtt_GetCodepointHMetrics(glyph->currFont, monospace ? 'W' : currCharacter, &glyph->xAdvance, &yAdvance);
            
                        glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont, scaledFontSize, scaledFontSize, currCharacter, &glyph->width, &glyph->height, nullptr, nullptr);
                    } else {
                        glyph = &cacheIt->second;
                    }
            
                    // Rendering with glyph-level bounds checking
                    if (glyph->glyphBmp != nullptr && !std::iswspace(currCharacter)) {
                        xPos = currX + glyph->bounds[0];
                        yPos = currY + glyph->bounds[1];
            
                        // Calculate glyph bounds for clipping
                        glyphLeft = static_cast<s32>(xPos);
                        glyphTop = static_cast<s32>(yPos);
                        glyphRight = glyphLeft + glyph->width;
                        glyphBottom = glyphTop + glyph->height;
                        
                        // Early rejection if glyph is completely outside framebuffer
                        if (glyphRight > 0 && glyphBottom > 0 && glyphLeft < framebufferWidth && glyphTop < framebufferHeight) {
                            // Calculate clipping bounds
                            clipLeft = std::max(glyphLeft, 0);
                            clipTop = std::max(glyphTop, 0);
                            clipRight = std::min(glyphRight, framebufferWidth);
                            clipBottom = std::min(glyphBottom, framebufferHeight);
                            
                            // Convert back to glyph-relative coordinates
                            startX = clipLeft - glyphLeft;
                            startY = clipTop - glyphTop;
                            endX = clipRight - glyphLeft;
                            endY = clipBottom - glyphTop;
            
                            // Ultra-optimized pixel processing with bounds-safe rendering
                            for (bmpY = startY; bmpY < endY; ++bmpY) {
                                rowOffset = bmpY * glyph->width;
                                rowPtr = glyph->glyphBmp + rowOffset;
                                pixelY = yPos + bmpY;
                                
                                // Process 8 pixels at once for maximum performance
                                bmpX = startX;
                                simdEnd = std::min(endX, (startX + 7) & ~7);
                                
                                // Handle initial unaligned pixels
                                for (; bmpX < simdEnd; ++bmpX) {
                                    alpha = rowPtr[bmpX] >> 4;
                                    if (alpha != 0x0) {
                                        pixelX = xPos + bmpX;
                                        if (alpha == 0xF) {
                                            offset = this->getPixelOffset(pixelX, pixelY);
                                            this->setPixel(pixelX, pixelY, color, offset);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = alpha;
                                            this->setPixelBlendDst(pixelX, pixelY, tmpColor);
                                        }
                                    }
                                }
                                
                                // Process 8-pixel chunks with unrolled loop
                                vectorEnd = endX & ~7;
                                for (; bmpX < vectorEnd; bmpX += 8) {
                                    // Load 8 alpha values
                                    a0 = rowPtr[bmpX] >> 4;
                                    a1 = rowPtr[bmpX + 1] >> 4;
                                    a2 = rowPtr[bmpX + 2] >> 4;
                                    a3 = rowPtr[bmpX + 3] >> 4;
                                    a4 = rowPtr[bmpX + 4] >> 4;
                                    a5 = rowPtr[bmpX + 5] >> 4;
                                    a6 = rowPtr[bmpX + 6] >> 4;
                                    a7 = rowPtr[bmpX + 7] >> 4;
                                    
                                    baseX = xPos + bmpX;
                                    
                                    // Unrolled pixel processing
                                    if (a0 != 0x0) {
                                        if (a0 == 0xF) {
                                            offset0 = this->getPixelOffset(baseX, pixelY);
                                            this->setPixel(baseX, pixelY, color, offset0);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a0;
                                            this->setPixelBlendDst(baseX, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a1 != 0x0) {
                                        if (a1 == 0xF) {
                                            offset1 = this->getPixelOffset(baseX + 1, pixelY);
                                            this->setPixel(baseX + 1, pixelY, color, offset1);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a1;
                                            this->setPixelBlendDst(baseX + 1, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a2 != 0x0) {
                                        if (a2 == 0xF) {
                                            offset2 = this->getPixelOffset(baseX + 2, pixelY);
                                            this->setPixel(baseX + 2, pixelY, color, offset2);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a2;
                                            this->setPixelBlendDst(baseX + 2, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a3 != 0x0) {
                                        if (a3 == 0xF) {
                                            offset3 = this->getPixelOffset(baseX + 3, pixelY);
                                            this->setPixel(baseX + 3, pixelY, color, offset3);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a3;
                                            this->setPixelBlendDst(baseX + 3, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a4 != 0x0) {
                                        if (a4 == 0xF) {
                                            offset4 = this->getPixelOffset(baseX + 4, pixelY);
                                            this->setPixel(baseX + 4, pixelY, color, offset4);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a4;
                                            this->setPixelBlendDst(baseX + 4, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a5 != 0x0) {
                                        if (a5 == 0xF) {
                                            offset5 = this->getPixelOffset(baseX + 5, pixelY);
                                            this->setPixel(baseX + 5, pixelY, color, offset5);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a5;
                                            this->setPixelBlendDst(baseX + 5, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a6 != 0x0) {
                                        if (a6 == 0xF) {
                                            offset6 = this->getPixelOffset(baseX + 6, pixelY);
                                            this->setPixel(baseX + 6, pixelY, color, offset6);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a6;
                                            this->setPixelBlendDst(baseX + 6, pixelY, tmpColor);
                                        }
                                    }
                                    
                                    if (a7 != 0x0) {
                                        if (a7 == 0xF) {
                                            offset7 = this->getPixelOffset(baseX + 7, pixelY);
                                            this->setPixel(baseX + 7, pixelY, color, offset7);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = a7;
                                            this->setPixelBlendDst(baseX + 7, pixelY, tmpColor);
                                        }
                                    }
                                }
                                
                                // Handle remaining pixels
                                for (; bmpX < endX; ++bmpX) {
                                    alpha = rowPtr[bmpX] >> 4;
                                    if (alpha != 0x0) {
                                        pixelX = xPos + bmpX;
                                        if (alpha == 0xF) {
                                            offset = this->getPixelOffset(pixelX, pixelY);
                                            this->setPixel(pixelX, pixelY, color, offset);
                                        } else {
                                            tmpColor = color;
                                            tmpColor.a = alpha;
                                            this->setPixelBlendDst(pixelX, pixelY, tmpColor);
                                        }
                                    }
                                }
                            }
                        }
                    }
            
                    // Advance the cursor for the next glyph
                    currX += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                }
            
                // Final calculation
                maxX = std::max(currX, maxX);
                return {maxX - x, currY - y};
            }


            inline std::pair<s32, s32> drawStringWithHighlight(
                const std::string& text, bool monospace, s32 x, s32 y,
                const u32 fontSize, const Color& defaultColor, const Color& specialColor,
                const ssize_t maxWidth = 0
            ) {
                // Early exits
                //if (text.empty() || fontSize <= 0) [[unlikely]] {
                //    return { 0, 0 };
                //}
            
                // Static glyph cache (shared across all calls)
                static std::unordered_map<u64, Glyph> s_glyphCache;
                
                // Pre-declare ALL variables outside loops for maximum optimization
                const size_t strLen = text.size();
                float maxX = x;
                float currX = x;
                float currY = y;
                bool inHighlight = false;
                
                // Iterator variables
                auto itStrEnd = text.cend();
                auto itStr = text.cbegin();
                
                // Character processing variables
                u32 currCharacter;
                ssize_t codepointWidth;
                u64 key;
                Glyph* glyph;
                std::unordered_map<u64, Glyph>::iterator it;
                std::pair<std::unordered_map<u64, Glyph>::iterator, bool> insertResult;
                
                // Font and glyph creation variables
                float scaledFontSize;
                s32 yAdvance;
                
                // Rendering position variables
                float xPos, yPos;
                s32 glyphLeft, glyphTop, glyphRight, glyphBottom;
                s32 clipLeft, clipTop, clipRight, clipBottom;
                s32 startX, startY, endX, endY;
                
                // Pixel processing variables
                s32 bmpX, bmpY;
                u32 rowOffset;
                const uint8_t* rowPtr;
                uint8_t bmpColor;
                Color tmpColor = {0};
                float pixelX, pixelY;
                u32 pixelOffset;
                
                // Color selection variables
                const Color* currentColor;
                
                // Cursor advance variables
                float xAdvanceScaled;
                
                // Bounds checking variables
                const s32 fbWidth = static_cast<s32>(cfg::FramebufferWidth);
                const s32 fbHeight = static_cast<s32>(cfg::FramebufferHeight);
                const float maxWidthLimit = maxWidth > 0 ? x + maxWidth : std::numeric_limits<float>::max();
                
                // Monospace key component
                const u64 monospaceKey = static_cast<u64>(monospace) << 31;
                const u64 fontSizeKey = static_cast<u64>(std::bit_cast<u32>(fontSize));
                const u64 baseKey = monospaceKey | fontSizeKey;
                
                // Ultra-fast ASCII detection for the entire string
                bool isAsciiOnly = true;
                if (strLen >= 8) {
                    const uint64_t* wordPtr = reinterpret_cast<const uint64_t*>(text.data());
                    const uint64_t* wordEnd = reinterpret_cast<const uint64_t*>(text.data() + (strLen & ~7));
                    
                    while (wordPtr < wordEnd) {
                        if (*wordPtr & 0x8080808080808080ULL) {
                            isAsciiOnly = false;
                            break;
                        }
                        ++wordPtr;
                    }
                    
                    if (isAsciiOnly) {
                        const unsigned char* bytePtr = reinterpret_cast<const unsigned char*>(text.data() + (strLen & ~7));
                        const unsigned char* byteEnd = reinterpret_cast<const unsigned char*>(text.data() + strLen);
                        while (bytePtr < byteEnd) {
                            if (*bytePtr > 127) {
                                isAsciiOnly = false;
                                break;
                            }
                            ++bytePtr;
                        }
                    }
                } else {
                    const unsigned char* bytePtr = reinterpret_cast<const unsigned char*>(text.data());
                    const unsigned char* byteEnd = bytePtr + strLen;
                    while (bytePtr < byteEnd) {
                        if (*bytePtr > 127) {
                            isAsciiOnly = false;
                            break;
                        }
                        ++bytePtr;
                    }
                }
            
                // Main character processing loop with all variables pre-declared
                while (itStr != itStrEnd) {
                    // Width boundary check with pre-computed limit
                    if (currX >= maxWidthLimit) [[unlikely]] {
                        break;
                    }
            
                    // Optimized UTF-8 decoding
                    if (isAsciiOnly && *itStr > 0) [[likely]] {
                        currCharacter = static_cast<u32>(*itStr);
                        ++itStr;
                    } else {
                        codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                        if (codepointWidth <= 0) [[unlikely]]
                            break;
                        itStr += codepointWidth;
                    }
            
                    // Handle parentheses for highlight state
                    if (currCharacter == '(') [[unlikely]] {
                        inHighlight = true;
                    } else if (currCharacter == ')') [[unlikely]] {
                        inHighlight = false;
                    }
            
                    // Handle newlines  
                    if (currCharacter == '\n') [[unlikely]] {
                        maxX = std::max(currX, maxX);
                        currX = x;
                        currY += fontSize;
                        continue;
                    }
            
                    // Calculate glyph key with pre-computed components
                    key = (static_cast<u64>(currCharacter) << 32) | baseKey;
            
                    // Check cache for the glyph
                    it = s_glyphCache.find(key);
            
                    // If glyph not found, create and cache it
                    if (it == s_glyphCache.end()) [[unlikely]] {
                        insertResult = s_glyphCache.emplace(key, Glyph());
                        glyph = &insertResult.first->second;
                        
                        // Determine the appropriate font for the character
                        if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter)) {
                            glyph->currFont = &this->m_extFont;
                        } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, currCharacter) == 0) {
                            glyph->currFont = &this->m_localFont;
                        } else {
                            glyph->currFont = &this->m_stdFont;
                        }
            
                        scaledFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
                        glyph->currFontSize = scaledFontSize;
            
                        // Get glyph bitmap and metrics
                        stbtt_GetCodepointBitmapBoxSubpixel(glyph->currFont, currCharacter, scaledFontSize, scaledFontSize,
                                                            0, 0, &glyph->bounds[0], &glyph->bounds[1], &glyph->bounds[2], &glyph->bounds[3]);
            
                        yAdvance = 0;
                        stbtt_GetCodepointHMetrics(glyph->currFont, monospace ? 'W' : currCharacter, &glyph->xAdvance, &yAdvance);
            
                        glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont, scaledFontSize, scaledFontSize, currCharacter, &glyph->width, &glyph->height, nullptr, nullptr);
                    } else [[likely]] {
                        glyph = &it->second;
                    }
            
                    // Pre-compute cursor advance
                    xAdvanceScaled = glyph->xAdvance * glyph->currFontSize;
            
                    // Determine color based on highlight state and parentheses
                    currentColor = (currCharacter == '(' || currCharacter == ')') ? &defaultColor : 
                                   (inHighlight ? &specialColor : &defaultColor);
            
                    // Render the glyph if it's not whitespace with optimized bounds checking
                    if (glyph->glyphBmp != nullptr && !std::iswspace(currCharacter) && currentColor->a != 0x0) [[likely]] {
                        xPos = currX + glyph->bounds[0];
                        yPos = currY + glyph->bounds[1];
            
                        // Calculate glyph bounds for clipping with pre-declared variables
                        glyphLeft = static_cast<s32>(xPos);
                        glyphTop = static_cast<s32>(yPos);
                        glyphRight = glyphLeft + glyph->width;
                        glyphBottom = glyphTop + glyph->height;
                        
                        // Early rejection if glyph is completely outside framebuffer
                        if (glyphRight > 0 && glyphBottom > 0 && glyphLeft < fbWidth && glyphTop < fbHeight) [[likely]] {
                            
                            // Calculate clipping bounds with pre-declared variables
                            clipLeft = std::max(glyphLeft, 0);
                            clipTop = std::max(glyphTop, 0);
                            clipRight = std::min(glyphRight, fbWidth);
                            clipBottom = std::min(glyphBottom, fbHeight);
                            
                            // Convert back to glyph-relative coordinates
                            startX = clipLeft - glyphLeft;
                            startY = clipTop - glyphTop;
                            endX = clipRight - glyphLeft;
                            endY = clipBottom - glyphTop;
            
                            // Render clipped pixels with optimized inner loops
                            for (bmpY = startY; bmpY < endY; ++bmpY) {
                                rowOffset = bmpY * glyph->width;
                                rowPtr = glyph->glyphBmp + rowOffset;
                                pixelY = yPos + bmpY;
                                
                                for (bmpX = startX; bmpX < endX; ++bmpX) {
                                    bmpColor = rowPtr[bmpX] >> 4;
                                    if (bmpColor != 0x0) {
                                        pixelX = xPos + bmpX;
                                        pixelOffset = this->getPixelOffset(pixelX, pixelY);
                                        if (bmpColor == 0xF) {
                                            this->setPixel(pixelX, pixelY, *currentColor, pixelOffset);
                                        } else {
                                            tmpColor = *currentColor;
                                            tmpColor.a = bmpColor;
                                            this->setPixelBlendDst(pixelX, pixelY, tmpColor);
                                        }
                                    }
                                }
                            }
                        }
                    }
            
                    // Advance the cursor for the next glyph
                    currX += xAdvanceScaled;
                }
            
                // Calculate final dimensions
                maxX = std::max(currX, maxX);
                return {static_cast<s32>(maxX - x), static_cast<s32>(currY - y + fontSize)};
            }

                                    
            inline void drawStringWithColoredSections(const std::string& text, const std::vector<std::string>& specialSymbols, s32 x, const s32 y, const u32 fontSize, const Color& defaultColor, const Color& specialColor) {
                // Early exits
                //if (text.empty() || fontSize <= 0) [[unlikely]] {
                //    return;
                //}
                
                // Fast path: no special symbols
                //if (specialSymbols.empty()) [[likely]] {
                //    drawString(text, false, x, y, fontSize, defaultColor);
                //    return;
                //}
                
                // Build efficient pattern matcher using Aho-Corasick-like approach
                // For simplicity, using optimized linear search with memoization
                
                // Create sorted symbol list for greedy matching
                //std::vector<std::string_view> sortedSymbols;
                //for (const auto& symbol : specialSymbols) {
                //    if (!symbol.empty()) {
                //        sortedSymbols.emplace_back(symbol);
                //    }
                //}
                
                // Sort by length (descending) for longest-first matching
                //std::sort(sortedSymbols.begin(), sortedSymbols.end(),
                //          [](const auto& a, const auto& b) { return a.length() > b.length(); });
                
                // Initialize rendering state
                float currX = x;
                float currY = y;
                
                // Pre-calculate constants
                const u64 keyBase = (static_cast<u64>(false) << 31) | (static_cast<u64>(std::bit_cast<u32>(fontSize)));
                
                // Static glyph cache (shared)
                static std::unordered_map<u64, Glyph> s_glyphCache;
                static bool s_cacheInitialized = false;
                if (!s_cacheInitialized) [[unlikely]] {
                    s_glyphCache.reserve(512);
                    s_cacheInitialized = true;
                }
                
                // String processing setup
                const char* strPtr = text.data();
                const char* const strEnd = strPtr + text.length();
                const char* currentStart = strPtr;
                
                bool foundMatch;
                size_t matchLength;
                size_t symLen;

                // Main processing loop
                while (strPtr < strEnd) {
                    // Check for special symbol matches at current position
                    foundMatch = false;
                    matchLength = 0;
                    
                    for (const auto& symbol : specialSymbols) {
                        //const size_t symLen = symbol.length();
                        symLen = symbol.length();
                        
                        // Check if symbol fits in remaining text
                        if (strPtr + symLen > strEnd) continue;
                        
                        // Fast memory comparison
                        if (std::memcmp(strPtr, symbol.data(), symLen) == 0) {
                            foundMatch = true;
                            matchLength = symLen;
                            break;
                        }
                    }
                    
                    if (foundMatch) {
                        // Render any accumulated normal text
                        if (strPtr > currentStart) {
                            renderTextSegment(currentStart, strPtr - currentStart, currX, currY, fontSize, defaultColor, keyBase, s_glyphCache);
                        }
                        
                        // Render special symbol
                        renderTextSegment(strPtr, matchLength, currX, currY, fontSize, specialColor, keyBase, s_glyphCache);
                        
                        // Advance past the special symbol
                        strPtr += matchLength;
                        currentStart = strPtr;
                    } else {
                        // Move to next character
                        ++strPtr;
                    }
                }
                
                // Render any remaining normal text
                if (strPtr > currentStart) {
                    renderTextSegment(currentStart, strPtr - currentStart, currX, currY, fontSize, defaultColor, keyBase, s_glyphCache);
                }
            }
                        
             // Ultra-optimized text segment rendering with all variables hoisted outside loops
            inline void renderTextSegment(const char* textPtr, size_t length, float& currX, float& currY, 
                                         const u32 fontSize, const Color& color, const u64 keyBase,
                                         std::unordered_map<u64, Glyph>& glyphCache) {
                
                if (length == 0 || color.a == 0x0) return;
                
                // Hoist ALL variable declarations outside loops
                const char* strPtr = textPtr;
                const char* const strEnd = textPtr + length;
                
                // Character processing variables
                u32 currCharacter;
                ssize_t codepointWidth;
                u64 key;
                Glyph* glyph;
                std::unordered_map<u64, Glyph>::iterator it;
                std::pair<std::unordered_map<u64, Glyph>::iterator, bool> insertResult;
                
                // Font and glyph creation variables
                float scaledFontSize;
                s32 yAdvance;
                
                // Rendering variables
                s32 xPos, yPos;
                const uint8_t* bmpPtr;
                s32 width, height;
                s32 currentY, currentX;
                s32 col, row;
                const s32* bounds;
                
                // SIMD processing variables
                s32 simdWidth;
                s32 x0, x1, x2, x3;
                uint8_t c0, c1, c2, c3;
                uint8_t bmpColor;
                Color tmpColor = {0};
                
                // Cursor advance variable
                s32 xAdvanceScaled;
                
                // Pre-compute ASCII optimization check
                bool isAsciiOnly = true;
                const unsigned char* asciiCheckPtr = reinterpret_cast<const unsigned char*>(textPtr);
                for (size_t i = 0; i < length; ++i) {
                    if (asciiCheckPtr[i] > 127) {
                        isAsciiOnly = false;
                        break;
                    }
                }
                
                // Pre-compute framebuffer bounds for faster comparisons
                const s32 fbWidth = static_cast<s32>(cfg::FramebufferWidth);
                const s32 fbHeight = static_cast<s32>(cfg::FramebufferHeight);
                
                // Character processing loop with all variables pre-declared
                while (strPtr < strEnd) {
                    // Fast UTF-8 decode with branch prediction hints
                    if (isAsciiOnly && *strPtr > 0) [[likely]] {
                        currCharacter = static_cast<u32>(*strPtr);
                        ++strPtr;
                    } else {
                        codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(strPtr));
                        if (codepointWidth <= 0) [[unlikely]]
                            break;
                        strPtr += codepointWidth;
                    }
                    
                    // Handle newlines with early continue
                    if (currCharacter == '\n') [[unlikely]] {
                        currX = 0;
                        currY += fontSize;
                        continue;
                    }
                    
                    // Glyph lookup with pre-computed key
                    key = (static_cast<u64>(currCharacter) << 32) | keyBase;
                    it = glyphCache.find(key);
                    
                    if (it == glyphCache.end()) [[unlikely]] {
                        // Create new glyph with optimized insertion
                        insertResult = glyphCache.emplace(key, Glyph{});
                        glyph = &insertResult.first->second;
                        
                        // Font selection (unchanged logic but using pre-declared variables)
                        if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter)) {
                            glyph->currFont = &this->m_extFont;
                        } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, currCharacter) == 0) {
                            glyph->currFont = &this->m_localFont;
                        } else {
                            glyph->currFont = &this->m_stdFont;
                        }
            
                        scaledFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
                        glyph->currFontSize = scaledFontSize;
            
                        stbtt_GetCodepointBitmapBoxSubpixel(glyph->currFont, currCharacter, scaledFontSize, scaledFontSize,
                                                            0, 0, &glyph->bounds[0], &glyph->bounds[1], &glyph->bounds[2], &glyph->bounds[3]);
            
                        yAdvance = 0;
                        stbtt_GetCodepointHMetrics(glyph->currFont, currCharacter, &glyph->xAdvance, &yAdvance);
            
                        glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont, scaledFontSize, scaledFontSize, currCharacter, &glyph->width, &glyph->height, nullptr, nullptr);
                    } else [[likely]] {
                        glyph = &it->second;
                    }
                    
                    // Pre-compute advance for cursor movement
                    xAdvanceScaled = static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                    
                    // Skip whitespace rendering but advance cursor
                    if (std::iswspace(currCharacter)) [[unlikely]] {
                        currX += xAdvanceScaled;
                        continue;
                    }
                    
                    // Render glyph with all variables pre-declared
                    if (glyph->glyphBmp != nullptr) [[likely]] {
                        bounds = glyph->bounds;
                        xPos = static_cast<s32>(currX + bounds[0]);
                        yPos = static_cast<s32>(currY + bounds[1]);
                        
                        // Quick reject with pre-computed framebuffer bounds
                        if (xPos >= fbWidth || yPos >= fbHeight || xPos + glyph->width <= 0 || yPos + glyph->height <= 0) [[unlikely]] {
                            currX += xAdvanceScaled;
                            continue;
                        }
                        
                        bmpPtr = glyph->glyphBmp;
                        width = glyph->width;
                        height = glyph->height;
                        simdWidth = width & ~3;
                        
                        // Optimized pixel rendering loop with pre-declared variables
                        for (row = 0; row < height; ++row) {
                            currentY = yPos + row;
                            
                            // Skip off-screen rows
                            if (currentY < 0 || currentY >= fbHeight) {
                                bmpPtr += width;
                                continue;
                            }
                            
                            // Process 4 pixels at once with pre-declared variables
                            for (col = 0; col < simdWidth; col += 4) {
                                x0 = xPos + col;
                                x1 = x0 + 1;
                                x2 = x0 + 2;
                                x3 = x0 + 3;
                                
                                c0 = bmpPtr[col] >> 4;
                                c1 = bmpPtr[col + 1] >> 4;
                                c2 = bmpPtr[col + 2] >> 4;
                                c3 = bmpPtr[col + 3] >> 4;
                                
                                // Unrolled pixel processing with bounds checking
                                if (c0 != 0 && x0 >= 0 && x0 < fbWidth) {
                                    if (c0 == 0xF) {
                                        this->setPixel(x0, currentY, color, this->getPixelOffset(x0, currentY));
                                    } else {
                                        tmpColor = color;
                                        tmpColor.a = c0;
                                        this->setPixelBlendDst(x0, currentY, tmpColor);
                                    }
                                }
                                
                                if (c1 != 0 && x1 >= 0 && x1 < fbWidth) {
                                    if (c1 == 0xF) {
                                        this->setPixel(x1, currentY, color, this->getPixelOffset(x1, currentY));
                                    } else {
                                        tmpColor = color;
                                        tmpColor.a = c1;
                                        this->setPixelBlendDst(x1, currentY, tmpColor);
                                    }
                                }
                                
                                if (c2 != 0 && x2 >= 0 && x2 < fbWidth) {
                                    if (c2 == 0xF) {
                                        this->setPixel(x2, currentY, color, this->getPixelOffset(x2, currentY));
                                    } else {
                                        tmpColor = color;
                                        tmpColor.a = c2;
                                        this->setPixelBlendDst(x2, currentY, tmpColor);
                                    }
                                }
                                
                                if (c3 != 0 && x3 >= 0 && x3 < fbWidth) {
                                    if (c3 == 0xF) {
                                        this->setPixel(x3, currentY, color, this->getPixelOffset(x3, currentY));
                                    } else {
                                        tmpColor = color;
                                        tmpColor.a = c3;
                                        this->setPixelBlendDst(x3, currentY, tmpColor);
                                    }
                                }
                            }
                            
                            // Handle remaining pixels with pre-declared variables
                            for (; col < width; ++col) {
                                currentX = xPos + col;
                                if (currentX >= 0 && currentX < fbWidth) {
                                    bmpColor = bmpPtr[col] >> 4;
                                    if (bmpColor == 0xF) {
                                        this->setPixel(currentX, currentY, color, this->getPixelOffset(currentX, currentY));
                                    } else if (bmpColor != 0x0) {
                                        tmpColor = color;
                                        tmpColor.a = bmpColor;
                                        this->setPixelBlendDst(currentX, currentY, tmpColor);
                                    }
                                }
                            }
                            
                            bmpPtr += width;
                        }
                    }
                    
                    currX += xAdvanceScaled;
                }
            }

                                    
            /**
             * @brief Limit a string's length and end it with "…" - Actually optimized version
             *
             * @param string String to truncate
             * @param monospace Whether the font is monospace
             * @param fontSize Size of the font
             * @param maxLength Maximum length of the string in terms of width
             */
            inline std::string limitStringLength(const std::string& originalString, const bool monospace, const s32 fontSize, const s32 maxLength) {
                #ifdef UI_OVERRIDE_PATH
                // Check for translation in the cache
                auto translatedIt = ult::translationCache.find(originalString);
                std::string translatedString = (translatedIt != ult::translationCache.end()) ? translatedIt->second : originalString;
                
                // Cache the translation if it wasn't already present
                if (translatedIt == ult::translationCache.end()) {
                    ult::translationCache[originalString] = translatedString;
                }
                const std::string* stringPtr = &translatedString;
                #else
                const std::string* stringPtr = &originalString;
                #endif
                
                if (stringPtr->size() < 2) {
                    return *stringPtr;
                }
                
                // Pre-calculate ellipsis width once
                constexpr u32 ellipsisCharacter = 0x2026;  // Unicode code point for '…'
                stbtt_fontinfo* ellipsisFont = &this->m_stdFont;
                if (stbtt_FindGlyphIndex(&this->m_extFont, ellipsisCharacter)) {
                    ellipsisFont = &this->m_extFont;
                } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, ellipsisCharacter) == 0) {
                    ellipsisFont = &this->m_localFont;
                }
                
                const float ellipsisFontSize = stbtt_ScaleForPixelHeight(ellipsisFont, fontSize);
                int ellipsisXAdvance = 0;
                stbtt_GetCodepointHMetrics(ellipsisFont, ellipsisCharacter, &ellipsisXAdvance, nullptr);
                const s32 ellipsisWidth = static_cast<s32>(ellipsisXAdvance * ellipsisFontSize);
                
                // THE KEY OPTIMIZATION: Incremental width calculation instead of recalculating entire substring
                s32 currX = 0;
                const s32 maxWidthWithoutEllipsis = maxLength - ellipsisWidth;
                const char* strPtr = stringPtr->data();
                const char* const strEnd = strPtr + stringPtr->size();
                const char* lastValidPos = strPtr;
                
                // MOVED OUTSIDE LOOP: Pre-declare variables used in loop
                u32 currCharacter;
                ssize_t codepointWidth;
                stbtt_fontinfo* font;
                float fontScale;
                int xAdvance;
                s32 charWidth;
                
                // Pre-calculate monospace character width if needed
                s32 monospaceCharWidth = 0;
                if (monospace) {
                    stbtt_fontinfo* monoFont = &this->m_stdFont;
                    if (stbtt_FindGlyphIndex(&this->m_extFont, 'W')) {
                        monoFont = &this->m_extFont;
                    } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, 'W') == 0) {
                        monoFont = &this->m_localFont;
                    }
                    //const float monoFontScale = stbtt_ScaleForPixelHeight(monoFont, fontSize);
                    int monoXAdvance = 0;
                    stbtt_GetCodepointHMetrics(monoFont, 'W', &monoXAdvance, nullptr);
                    monospaceCharWidth = static_cast<s32>(monoXAdvance * stbtt_ScaleForPixelHeight(monoFont, fontSize));
                }
                
                while (strPtr < strEnd) {
                    codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(strPtr));
                    if (codepointWidth <= 0) break;
                    
                    if (monospace) {
                        // Use pre-calculated monospace width
                        charWidth = monospaceCharWidth;
                    } else {
                        // Calculate width of just this character
                        font = &this->m_stdFont;
                        if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter)) {
                            font = &this->m_extFont;
                        } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, currCharacter) == 0) {
                            font = &this->m_localFont;
                        }
                        
                        fontScale = stbtt_ScaleForPixelHeight(font, fontSize);
                        xAdvance = 0;
                        stbtt_GetCodepointHMetrics(font, currCharacter, &xAdvance, nullptr);
                        charWidth = static_cast<s32>(xAdvance * fontScale);
                    }
                    
                    // Check if adding this character would exceed the limit
                    if (currX + charWidth > maxWidthWithoutEllipsis) {
                        // Return truncated string with ellipsis
                        //const size_t truncateLength = lastValidPos - stringPtr->data();
                        return stringPtr->substr(0, lastValidPos - stringPtr->data()) + "…";
                    }
                    
                    // Add this character's width and continue
                    currX += charWidth;
                    strPtr += codepointWidth;
                    lastValidPos = strPtr;
                }
                
                // String fits entirely
                return *stringPtr;
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

            static Renderer& getRenderer() {
                return get();
            }

            inline void setLayerPosImpl(u32 x, u32 y) {
                // Get the underscan pixel values for both horizontal and vertical borders
                auto [horizontalUnderscanPixels, verticalUnderscanPixels] = getUnderscanPixels();
                //int horizontalUnderscanPixels = 0;

                //ult::useRightAlignment = (ult::parseValueFromIniSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, "right_alignment") == ult::TRUE_STR);
                //cfg::LayerPosX = 1280-32;
                cfg::LayerPosX = 0;
                cfg::LayerPosY = 0;
                cfg::FramebufferWidth  = ult::DefaultFramebufferWidth;
                cfg::FramebufferHeight = ult::DefaultFramebufferHeight;

                //ult::correctFrameSize = (cfg::FramebufferWidth == 448 && cfg::FramebufferHeight == 720); // for detecting the correct Overlay display size
                if (ult::useRightAlignment && ult::correctFrameSize) {
                    cfg::LayerPosX = 1280-32 - horizontalUnderscanPixels;
                    ult::layerEdge = (1280-448);
                }

                cfg::LayerPosX += x;
                cfg::LayerPosY += y;
                ASSERT_FATAL(viSetLayerPosition(&this->m_layer, cfg::LayerPosX, cfg::LayerPosY));
            }


            #if USING_WIDGET_DIRECTIVE
            // Method to draw clock, temperatures, and battery percentage
            inline void drawWidget() {
                // Draw clock if it's not hidden
                static time_t lastTimeUpdate = 0;
                static char timeStr[20]; // Allocate a buffer to store the time string
                size_t y_offset = 45;
        
                if (!(ult::hideBattery && ult::hidePCBTemp && ult::hideSOCTemp && ult::hideClock)) {
                    drawRect(245, 23, 1, 49, a(separatorColor));
                }

                if ((ult::hideBattery && ult::hidePCBTemp && ult::hideSOCTemp) || ult::hideClock) {
                    y_offset += 10;
                }

                // Use simpler time() function instead of clock_gettime for seconds precision
                time_t currentTime = time(nullptr);
                if (!ult::hideClock) {
                    // Only update time string if a second has passed
                    if (currentTime != lastTimeUpdate) {
                        strftime(timeStr, sizeof(timeStr), ult::datetimeFormat.c_str(), localtime(&currentTime));
                        ult::localizeTimeStr(timeStr);
                        lastTimeUpdate = currentTime;
                    }
                    drawString(timeStr, false, tsl::cfg::FramebufferWidth - calculateStringWidth(timeStr, 20, true) - 20, y_offset, 20, a(clockColor));
                    y_offset += 22;
                }
        
                // Draw temperatures and battery percentage
                static char PCB_temperatureStr[10];
                static char SOC_temperatureStr[10];
                static char chargeString[6];
        
                size_t statusChange = size_t(ult::hideSOCTemp) + size_t(ult::hidePCBTemp) + size_t(ult::hideBattery);
                static size_t lastStatusChange = 0;
                static time_t lastSensorUpdate = 0;
                
                // Update sensors every second or when visibility settings change
                if ((currentTime - lastSensorUpdate) >= 1 || statusChange != lastStatusChange) {
                    if (!ult::hideSOCTemp) {
                        ult::ReadSocTemperature(&ult::SOC_temperature);
                        snprintf(SOC_temperatureStr, sizeof(SOC_temperatureStr) - 1, "%d°C", static_cast<int>(round(ult::SOC_temperature)));
                    } else {
                        strcpy(SOC_temperatureStr, "");
                        ult::SOC_temperature = 0;
                    }
                    
                    if (!ult::hidePCBTemp) {
                        ult::ReadPcbTemperature(&ult::PCB_temperature);
                        snprintf(PCB_temperatureStr, sizeof(PCB_temperatureStr) - 1, "%d°C", static_cast<int>(round(ult::PCB_temperature)));
                    } else {
                        strcpy(PCB_temperatureStr, "");
                        ult::PCB_temperature = 0;
                    }
                    
                    if (!ult::hideBattery) {
                        ult::powerGetDetails(&ult::batteryCharge, &ult::isCharging);
                        ult::batteryCharge = std::min(ult::batteryCharge, 100U);
                        sprintf(chargeString, "%d%%", ult::batteryCharge);
                    } else {
                        strcpy(chargeString, "");
                        ult::batteryCharge = 0;
                    }
                    
                    lastSensorUpdate = currentTime;
                    lastStatusChange = statusChange;
                }
                
                // Draw battery percentage
                if (!ult::hideBattery && ult::batteryCharge > 0) {
                    Color batteryColorToUse = ult::isCharging ? tsl::Color(0x0, 0xF, 0x0, 0xF) : 
                                            (ult::batteryCharge < 20 ? tsl::Color(0xF, 0x0, 0x0, 0xF) : batteryColor);
                    drawString(chargeString, false, tsl::cfg::FramebufferWidth - calculateStringWidth(chargeString, 20, true) - 22, y_offset, 20, a(batteryColorToUse));
                }
        
                // Draw PCB and SOC temperatures
                int offset = 0;
                if (!ult::hidePCBTemp && ult::PCB_temperature > 0) {
                    if (!ult::hideBattery)
                        offset -= 5;
                    drawString(PCB_temperatureStr, false, tsl::cfg::FramebufferWidth + offset - calculateStringWidth(PCB_temperatureStr, 20, true) - calculateStringWidth(chargeString, 20, true) - 22, y_offset, 20, a(tsl::GradientColor(ult::PCB_temperature)));
                }
        
                if (!ult::hideSOCTemp && ult::SOC_temperature > 0) {
                    if (!ult::hidePCBTemp || !ult::hideBattery)
                        offset -= 5;
                    drawString(SOC_temperatureStr, false, tsl::cfg::FramebufferWidth + offset - calculateStringWidth(SOC_temperatureStr, 20, true) - calculateStringWidth(PCB_temperatureStr, 20, true) - calculateStringWidth(chargeString, 20, true) - 22, y_offset, 20, a(tsl::GradientColor(ult::SOC_temperature)));
                }
            }
            #endif

            
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
            
            //u32 tmpPos;
            
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

            inline u32 getPixelOffset(const u32 x, const u32 y) {
                // Check for scissoring boundaries
                if (!this->m_scissoringStack.empty()) {
                    const auto& currScissorConfig = this->m_scissoringStack.top();
                    if (x < currScissorConfig.x || y < currScissorConfig.y || 
                        x >= currScissorConfig.x + currScissorConfig.w || 
                        y >= currScissorConfig.y + currScissorConfig.h) {
                        return UINT32_MAX;
                    }
                }
                
                // Replace divisions and modulos with bit operations - EXACT same logic
                //return ((((y & 127) >> 4) + ((x >> 5) << 3) + ((y >> 7) * 112)) << 9) +  // *512 = <<9
                //       (((y & 15) >> 3) << 8) +     // ((y % 16) / 8) * 256
                //       (((x & 31) >> 4) << 7) +     // ((x % 32) / 16) * 128
                //       (((y & 7) >> 1) << 5) +      // ((y % 8) / 2) * 32
                //       (((x & 15) >> 3) << 4) +     // ((x % 16) / 8) * 16
                //       ((y & 1) << 3) +             // (y % 2) * 8
                //       (x & 7);                     // x % 8

                //return ((((y & 127) >> 4) + ((x >> 5) << 3) + ((y >> 7) * (((cfg::FramebufferWidth / 2) >> 4) << 3))) << 9) +  // *512 = <<9
                //       (((y & 15) >> 3) << 8) +     // ((y % 16) / 8) * 256
                //       (((x & 31) >> 4) << 7) +     // ((x % 32) / 16) * 128
                //       (((y & 7) >> 1) << 5) +      // ((y % 8) / 2) * 32
                //       (((x & 15) >> 3) << 4) +     // ((x % 16) / 8) * 16
                //       ((y & 1) << 3) +             // (y % 2) * 8
                //       (x & 7);                     // x % 8

                //return ((((y & 127) >> 4) + ((x >> 5) << 3) + ((y >> 7) * offsetWidthVar)) << 9) +  // *512 = <<9
                //       (((y & 15) >> 3) << 8) +     // ((y % 16) / 8) * 256
                //       (((x & 31) >> 4) << 7) +     // ((x % 32) / 16) * 128
                //       (((y & 7) >> 1) << 5) +      // ((y % 8) / 2) * 32
                //       (((x & 15) >> 3) << 4) +     // ((x % 16) / 8) * 16
                //       ((y & 1) << 3) +             // (y % 2) * 8
                //       (x & 7);                     // x % 8

                return ((((y & 127) >> 4) + ((x >> 5) << 3) + ((y >> 7) * offsetWidthVar)) << 9) +
                       ((y & 8) << 5) + ((x & 16) << 3) + ((y & 6) << 4) + 
                       ((x & 8) << 1) + ((y & 1) << 3) + (x & 7);

                //const u32 y_hi = y >> 7;
                //const u32 y_mid = (y >> 4) & 7;    // bits 4-6 of y
                //const u32 y_lo = y & 15;           // bits 0-3 of y
                //
                //const u32 x_hi = x >> 5;           // bits 5+ of x  
                //const u32 x_lo = x & 31;          // bits 0-4 of x
                //
                //return ((y_mid + (x_hi << 3) + (y_hi * offsetWidthVar)) << 9) +
                //       ((y_lo >> 3) << 8) + ((x_lo >> 4) << 7) + 
                //       (((y_lo & 7) >> 1) << 5) + (((x_lo & 15) >> 3) << 4) +
                //       ((y_lo & 1) << 3) + (x_lo & 7);
            }

            
            /**
             * @brief Initializes the renderer and layers
             *
             */
            void init() {
                // Get the underscan pixel values for both horizontal and vertical borders
                auto [horizontalUnderscanPixels, verticalUnderscanPixels] = getUnderscanPixels();
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
                    cfg::LayerHeight += 1.99*verticalUnderscanPixels;
                else
                    cfg::LayerWidth += horizontalUnderscanPixels;

                
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
                static PlFontData stdFontData, localFontData, extFontData;
                
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
                        TSL_R_TRY(plGetSharedFontByType(&localFontData, PlSharedFontType_ChineseSimplified));
                        break;
                    case SetLanguage_KO:
                        TSL_R_TRY(plGetSharedFontByType(&localFontData, PlSharedFontType_KO));
                        break;
                    case SetLanguage_ZHTW:
                    case SetLanguage_ZHHANT:
                        TSL_R_TRY(plGetSharedFontByType(&localFontData, PlSharedFontType_ChineseTraditional));
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
            inline void endFrame() {
                #if IS_STATUS_MONITOR_DIRECTIVE
                if (!FullMode || deactivateOriginalFooter) {
                    __builtin_memcpy(this->getNextFramebuffer(), this->getCurrentFramebuffer(), this->getFramebufferSize());
                    svcSleepThread(1000*1000*1000 / TeslaFPS);
                }
                #endif

                this->waitForVSync();
                framebufferEnd(&this->m_framebuffer);
                
                this->m_currentFramebuffer = nullptr;
            }

        #if IS_STATUS_MONITOR_DIRECTIVE
            /**
             * @brief Draws a single font glyph
             * 
             * @param codepoint Unicode codepoint to draw
             * @param x X pos
             * @param y Y pos
             * @param color Color
             * @param font STB Font to use
             * @param fontSize Font size
             */

            inline void drawGlyph(s32 codepoint, s32 x, s32 y, Color color, stbtt_fontinfo *font, float fontSize) {
                int width = 10, height = 10;
                u8* glyphBmp = nullptr;
                if (fontCache) {
                    auto pair = std::make_pair(codepoint, fontSize);
                    auto found = cache.find(pair);
                    if (found != cache.end()) {
                        glyphBmp = found->second.pointer;
                        width = found->second.width;
                        height = found->second.height;
                    }
                    else {
                        glyphBmp = stbtt_GetCodepointBitmap(font, fontSize, fontSize, codepoint, &width, &height, nullptr, nullptr);
                        if (glyphBmp) cache[pair] = GlyphInfo{glyphBmp, width, height};
                    }
                }
                else {
                    glyphBmp = stbtt_GetCodepointBitmap(font, fontSize, fontSize, codepoint, &width, &height, nullptr, nullptr);
                }
                
                if (glyphBmp == nullptr)
                    return;
                
                // Pre-calculate constants outside loops
                const float colorAFloat = float(color.a) / 15.0f;  // Divide by 15, not 0xF
                const u8* bmpPtr = glyphBmp;  // Cache pointer for faster access
                
                // Single loop with pointer arithmetic
                for (s32 bmpY = 0; bmpY < height; bmpY++) {
                    const s32 pixelY = y + bmpY;
                    for (s32 bmpX = 0; bmpX < width; bmpX++) {
                        const u8 alpha = *bmpPtr++;  // Direct pointer increment
                        if (alpha) {  // Skip transparent pixels
                            Color tmpColor = color;
                            tmpColor.a = (alpha >> 4) * colorAFloat;
                            this->setPixelBlendSrc(x + bmpX, pixelY, tmpColor);
                        }
                    }
                }
                if (!fontCache) std::free(glyphBmp);
            }
        #endif
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
            u32 underscanValue = tvSettings.underscan;
            
            // Convert the underscan value to a fraction. Assuming 0 means no underscan and larger values represent
            // greater underscan. Adjust this formula based on actual observed behavior or documentation.
            float underscanPercentage = 1.0f - (underscanValue / 100.0f);
            
            // Original dimensions of the full 720p image (1280x720)
            float originalWidth = cfg::ScreenWidth;
            float originalHeight = cfg::ScreenHeight;
            
            // Adjust the width and height based on the underscan percentage
            float adjustedWidth = (originalWidth * underscanPercentage);
            float adjustedHeight = (originalHeight * underscanPercentage);
            
            // Calculate the underscan in pixels (left/right and top/bottom)
            int horizontalUnderscanPixels = ((originalWidth - adjustedWidth) / 2.);
            int verticalUnderscanPixels = ((originalHeight - adjustedHeight) / 2.);
            
            return {horizontalUnderscanPixels, verticalUnderscanPixels};
        }


        // Helper function to calculate string width
        static float calculateStringWidth(const std::string& originalString, const float fontSize, const bool fixedWidthNumbers = false) {
            if (originalString.empty()) {
                return 0.0f;
            }
        
            static const stbtt_fontinfo& font = tsl::gfx::Renderer::get().getStandardFont();
            float maxWidth = 0.0f;
            float currentLineWidth = 0.0f;
            ssize_t codepointWidth;
            u32 prevCharacter = 0;
            u32 currCharacter = 0;
        
            static std::unordered_map<u64, Renderer::Glyph> s_glyphCache;
            auto itStrEnd = originalString.cend();
            auto itStr = originalString.cbegin();
        
            u64 key = 0;
            Renderer::Glyph* glyph = nullptr;
            auto it = s_glyphCache.end();
        
            while (itStr != itStrEnd) {
                // Decode UTF-8 codepoint
                codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                if (codepointWidth <= 0) {
                    break;
                }
        
                // Move the iterator forward by the width of the current codepoint
                itStr += codepointWidth;
        
                // If newline, finalize the current line's width and start a new line
                if (currCharacter == '\n') {
                    if (currentLineWidth > maxWidth) {
                        maxWidth = currentLineWidth;
                    }
                    currentLineWidth = 0.0f; // Reset for the new line
                    prevCharacter = 0;
                    continue;
                }
        
                // Calculate glyph key
                key = (static_cast<u64>(currCharacter) << 32) | (static_cast<u64>(fixedWidthNumbers) << 31) | (static_cast<u64>(std::bit_cast<u32>(fontSize)));
        
                // Check cache for the glyph
                it = s_glyphCache.find(key);
        
                // If glyph not found, create and cache it
                if (it == s_glyphCache.end()) {
                    glyph = &s_glyphCache.emplace(key, Renderer::Glyph()).first->second;
        
                    // Use const_cast to handle const font
                    glyph->currFont = const_cast<stbtt_fontinfo*>(&font);
                    glyph->currFontSize = stbtt_ScaleForPixelHeight(glyph->currFont, fontSize);
                    stbtt_GetCodepointHMetrics(glyph->currFont, currCharacter, &glyph->xAdvance, nullptr);
                } else {
                    glyph = &it->second;
                }
        
                if (prevCharacter) {
                    float kernAdvance = stbtt_GetCodepointKernAdvance(glyph->currFont, prevCharacter, currCharacter);
                    currentLineWidth += kernAdvance * glyph->currFontSize;
                }
        
                currentLineWidth += int(glyph->xAdvance * glyph->currFontSize);
                prevCharacter = currCharacter;
            }
        
            // Final comparison for the last line (if no trailing newline)
            if (currentLineWidth > maxWidth) {
                maxWidth = currentLineWidth;
            }
        
            return maxWidth;
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
            virtual ~Element() { }
            
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
                if (!disableSelectionBG)
                    renderer->drawRect(this->getX() + x + 4, this->getY() + y, this->getWidth() - 8, this->getHeight(), a(selectionBGColor)); // CUSTOM MODIFICATION 

                saturation = tsl::style::ListItemHighlightSaturation * (float(this->m_clickAnimationProgress) / float(tsl::style::ListItemHighlightLength));

                Color animColor = {0xF,0xF,0xF,0xF};
                if (invertBGClickColor) {
                    animColor.r = 15-saturation;
                    animColor.g = 15-saturation;
                    animColor.b = 15-saturation;
                    animColor.a = 15-saturation;
                } else {
                    animColor.r = saturation;
                    animColor.g = saturation;
                    animColor.b = saturation;
                    animColor.a = saturation;
                }
                renderer->drawRect(ELEMENT_BOUNDS(this), a(animColor));

                Color clickColor1 = highlightColor1;
                Color clickColor2 = clickColor;
                
                // Changed timing calculation to use armTicksToNs
                u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                double time_seconds = currentTime_ns / 1000000000.0; // Convert nanoseconds to seconds
                progress = (std::cos(2.0 * ult::_M_PI * std::fmod(time_seconds - 0.25, 1.0)) + 1.0) / 2.0;
                
                if (progress >= 0.5) {
                    clickColor1 = clickColor;
                    clickColor2 = highlightColor2;
                }
                
                highlightColor = {
                    static_cast<u8>((clickColor1.r - clickColor2.r) * progress + clickColor2.r),
                    static_cast<u8>((clickColor1.g - clickColor2.g) * progress + clickColor2.g),
                    static_cast<u8>((clickColor1.b - clickColor2.b) * progress + clickColor2.b),
                    0xF
                };
                
                x = 0;
                y = 0;
                if (this->m_highlightShaking) {
                    u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime; // Changed
                    if (t_ns >= 100000000) // 100ms in nanoseconds
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                    switch (this->m_highlightShakingDirection) {
                        case FocusDirection::Up:
                            y -= shakeAnimation(t_ns, amplitude); // Changed parameter
                            break;
                        case FocusDirection::Down:
                            y += shakeAnimation(t_ns, amplitude); // Changed parameter
                            break;
                        case FocusDirection::Left:
                            x -= shakeAnimation(t_ns, amplitude); // Changed parameter
                            break;
                        case FocusDirection::Right:
                            x += shakeAnimation(t_ns, amplitude); // Changed parameter
                            break;
                        default:
                            break;
                    }
                        
                        x = std::clamp(x, -amplitude, amplitude);
                        y = std::clamp(y, -amplitude, amplitude);
                    }
                }
                
                
                //renderer->drawRect(this->getX() + x - 4, this->getY() + y - 4, this->getWidth() + 8, 4, highlightColor);
                //renderer->drawRect(this->getX() + x - 4, this->getY() + y + this->getHeight(), this->getWidth() + 8, 4, highlightColor);
                //renderer->drawRect(this->getX() + x - 4, this->getY() + y, 4, this->getHeight(), highlightColor);
                //renderer->drawRect(this->getX() + x + this->getWidth(), this->getY() + y, 4, this->getHeight(), highlightColor);
                
                
                renderer->drawBorderedRoundedRect(this->getX() + x, this->getY() + y, this->getWidth() +4, this->getHeight(), 5, 5, a(highlightColor));
                    
                //}
            }
            
            /**
             * @brief Draws the back background when a element is highlighted
             * @note Override this if you have a element that e.g requires a non-rectangular focus
             *
             * @param renderer Renderer
             */
            virtual void drawFocusBackground(gfx::Renderer *renderer) {
                //if (!disableSelectionBG)
                //    renderer->drawRect(ELEMENT_BOUNDS(this), a(selectionBGColor)); // CUSTOM MODIFICATION 
                
                if (this->m_clickAnimationProgress > 0) {
                    this->drawClickAnimation(renderer);
        
                    // Calculate time elapsed since the animation started using armTicksToNs
                    u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    u64 elapsed_ns = currentTime_ns - this->m_animationStartTime;
                    double elapsed_ms = elapsed_ns / 1000000.0; // Convert to milliseconds
        
                    // Decrease progress based on the elapsed time (1 second = 1000ms)
                    this->m_clickAnimationProgress = tsl::style::ListItemHighlightLength * (1.0f - (elapsed_ms / 500.0f));
        
                    // Ensure progress does not go below 0
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
                
                
                // Changed timing calculation to use armTicksToNs
                u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                double time_seconds = currentTime_ns / 1000000000.0; // Convert nanoseconds to seconds
                progress = ((std::cos(2.0 * ult::_M_PI * std::fmod(time_seconds - 0.25, 1.0)) + 1.0) / 2.0);

                if (ult::runningInterpreter.load(std::memory_order_acquire)) {
                    highlightColor = {
                        static_cast<u8>((highlightColor3.r - highlightColor4.r) * progress + highlightColor4.r),
                        static_cast<u8>((highlightColor3.g - highlightColor4.g) * progress + highlightColor4.g),
                        static_cast<u8>((highlightColor3.b - highlightColor4.b) * progress + highlightColor4.b),
                        0xF
                    };
                } else {
                    highlightColor = {
                        static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress + highlightColor2.r),
                        static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress + highlightColor2.g),
                        static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress + highlightColor2.b),
                        0xF
                    };
                }
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime; // Changed
                    if (t_ns >= 100000000) // 100ms in nanoseconds
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            default:
                                break;
                        }
                        
                        x = std::clamp(x, -amplitude, amplitude);
                        y = std::clamp(y, -amplitude, amplitude);
                    }
                }
                //if ((disableSelectionBG && this->m_clickAnimationProgress == 0) || !disableSelectionBG) {
                if (this->m_clickAnimationProgress == 0) {
                    if (!disableSelectionBG)
                        renderer->drawRect(this->getX() + x + 4, this->getY() + y, this->getWidth() - 12 +4, this->getHeight(), a(selectionBGColor)); // CUSTOM MODIFICATION 

                    #if IS_LAUNCHER_DIRECTIVE
                    // Determine the active percentage to use
                    float activePercentage = 0.0f;
                    if (ult::downloadPercentage > 0) {
                        activePercentage = ult::downloadPercentage;
                    } else if (ult::unzipPercentage > 0) {
                        activePercentage = ult::unzipPercentage;
                    } else if (ult::copyPercentage > 0) {
                        activePercentage = ult::copyPercentage;
                    }
                    if (activePercentage > 0){
                        renderer->drawRect(this->getX() + x + 4, this->getY() + y, (this->getWidth()- 12 +4)*(activePercentage/100.0f), this->getHeight(), a(progressColor));
                        //if (ult::copyPercentage == 100.0f) {
                        //    ult::copyPercentage = -1;
                        //}
                    }
                    #endif

                    renderer->drawBorderedRoundedRect(this->getX() + x, this->getY() + y, this->getWidth() +4, this->getHeight(), 5, 5, a(highlightColor));
                }
                //renderer->drawRect(ELEMENT_BOUNDS(this), a(0xF000)); // This has been moved here (needs to be toggleable)
                ult::onTrackBar = false;
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

            virtual bool matchesJumpCriteria(const std::string& jumpText, const std::string& jumpValue) const {
                return false; // Default implementation for non-ListItem elements
            }
            
            
            static InputMode getInputMode() { return Element::s_inputMode; }
            
            static void setInputMode(InputMode mode) { Element::s_inputMode = mode; }
            
        protected:
            constexpr static inline auto a = &gfx::Renderer::a;
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
            inline int shakeAnimation(u64 t_ns, float a) {
                float w = 0.2F;
                float tau = 0.05F;
                
                // Convert nanoseconds to microseconds for the calculation
                int t_us = t_ns / 1000;
                
                return roundf(a * exp(-(tau * t_us) * sin(w * t_us)));
            }
            
        private:
            friend class Gui;
            
            s32 m_x = 0, m_y = 0, m_width = 0, m_height = 0;
            Element *m_parent = nullptr;
            std::vector<Element*> m_children;
            std::function<bool(u64 keys)> m_clickListener = [](u64) { return false; };
        };

        //static std::vector<Element*> m_lastFrameItems; // for smooth handling of jumpToItem navigation
        //static bool m_hasValidFrame = false;
        //static float m_lastFrameOffset = 0.0f;
        // Static cache with instance validation
        
    #if IS_STATUS_MONITOR_DIRECTIVE
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
                CustomDrawer(std::function<void(gfx::Renderer*, u16 x, u16 y, u16 w, u16 h)> renderFunc) : Element(), m_renderFunc(renderFunc) {}
                virtual ~CustomDrawer() {
                    m_isTable = true;
                }

                virtual void draw(gfx::Renderer* renderer) override {
                    this->m_renderFunc(renderer, this->getX(), this->getY(), this->getWidth(), this->getHeight());
                }

                virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {}

            private:
                std::function<void(gfx::Renderer*, u16 x, u16 y, u16 w, u16 h)> m_renderFunc;
        };
    #else
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
    #endif

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

                renderer->enableScissoring(0, 97, tsl::cfg::FramebufferWidth, tsl::cfg::FramebufferHeight - 73 - 97 - 4);
                
                if (!hideTableBackground)
                    renderer->drawRoundedRect(this->getX() + 4+2, this->getY()-6, this->getWidth() +2, this->getHeight() + 20 - endGap+2, 10.0, a(tableBGColor));
                
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

    #if IS_STATUS_MONITOR_DIRECTIVE

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

            float x, y;
            int offset, y_offset;
            int fontSize;

            std::string menuBottomLine;
            
        OverlayFrame(const std::string& title, const std::string& subtitle, const bool& _noClickableItems=false)
            : Element(), m_title(title), m_subtitle(subtitle), m_noClickableItems(_noClickableItems) {
                ult::activeHeaderHeight = 97;
                ult::loadWallpaperFileWhenSafe();

                m_isItem = false;
            }

            virtual ~OverlayFrame() {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
            }

            
            virtual void draw(gfx::Renderer *renderer) override {
                if (!ult::themeIsInitialized) {
                    tsl::initializeThemeVars(); // Initialize variables for ultrahand themes
                    ult::themeIsInitialized = true;
                }

                if (m_noClickableItems != ult::noClickableItems)
                    ult::noClickableItems = m_noClickableItems;
                
                
                if (FullMode == true) {
                    renderer->fillScreen(a(defaultBackgroundColor));
                    renderer->drawWallpaper();
                } else {
                    renderer->fillScreen({ 0x0, 0x0, 0x0, alphabackground});
                }
                
                y = 50;
                offset = 0;
                
                renderer->drawString(this->m_title, false, 20, 50+2, 32, a(defaultOverlayColor));
                renderer->drawString(this->m_subtitle, false, 20, y+23, 15, a(versionTextColor));
                
                if (FullMode == true)
                    renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(botttomSeparatorColor));
                
                if (FullMode && !deactivateOriginalFooter) {
                    ult::backWidth = tsl::gfx::calculateStringWidth(ult::BACK, 23);
                    if (ult::touchingBack) {
                        renderer->drawRoundedRect(18.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                                  ult::backWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                    }

                    ult::selectWidth = tsl::gfx::calculateStringWidth(ult::OK, 23);
                    if (ult::touchingSelect && !m_noClickableItems) {
                        renderer->drawRoundedRect(18.0f + ult::backWidth+68.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                                  ult::selectWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                    }
                }

                if (m_noClickableItems)
                    menuBottomLine = "\uE0E1"+ult::GAP_2+ult::BACK+ult::GAP_1;
                else
                    menuBottomLine = "\uE0E1"+ult::GAP_2+ult::BACK+ult::GAP_1+"\uE0E0"+ult::GAP_2+ult::OK+ult::GAP_1;
                
                //renderer->drawString(menuBottomLine.c_str(), false, 30, 693, 23, a(defaultTextColor));
                // Render the text with special character handling
                if (!deactivateOriginalFooter) renderer->drawStringWithColoredSections(menuBottomLine, {"\uE0E1","\uE0E0","\uE0ED","\uE0EE"}, 30, 693, 23, a(bottomTextColor), a(buttonColor));
                
                
                if (this->m_contentElement != nullptr)
                    this->m_contentElement->frame(renderer);

            }
            

            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(parentX, parentY, parentWidth, parentHeight);
        
                if (this->m_contentElement != nullptr) {
                    this->m_contentElement->setBoundaries(parentX + 35, parentY + 140, parentWidth - 85, parentHeight - 73 - 105); // CUSTOM MODIFICATION
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
            static constexpr double cycleDuration = 1.5;
            float counter = 0;
            float countOffset;
            float progress;
            float letterWidth;
        #endif
        
            float x, y;
            int offset, y_offset;
            int fontSize;
        
            std::string menuBottomLine;
            
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
                }
        
            ~OverlayFrame() {
                delete m_contentElement;
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
                if (!ult::themeIsInitialized) {
                    tsl::initializeThemeVars(); // Initialize variables for ultrahand themes
                    ult::themeIsInitialized = true;
                }
        
                if (m_noClickableItems != ult::noClickableItems)
                    ult::noClickableItems = m_noClickableItems;
                
                renderer->fillScreen(a(defaultBackgroundColor));
                renderer->drawWallpaper();
                
                y = 50;
                offset = 0;
                
            #if IS_LAUNCHER_DIRECTIVE
                const bool isUltrahand = (m_title == ult::CAPITAL_ULTRAHAND_PROJECT_NAME && 
                                        m_subtitle.find("Ultrahand Package") == std::string::npos && 
                                        m_subtitle.find("Ultrahand Script") == std::string::npos);
        
                if (isUltrahand) {
                #if USING_WIDGET_DIRECTIVE
                    renderer->drawWidget();
                #endif
        
                    if (ult::touchingMenu && ult::inMainMenu) {
                        renderer->drawRoundedRect(0.0f, 12.0f, 245.0f, 73.0f, 6.0f, a(clickColor));
                    }
                    
                    x = 20;
                    fontSize = 42;
                    offset = 6;
                    countOffset = 0;
        
                    if (!disableColorfulLogo && ult::useDynamicLogo) {
                        const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                        const double currentTimeCount = currentTime_ns / 1000000000.0;
        
                        for (const char letter : ult::SPLIT_PROJECT_NAME_1) {
                            counter = (2 * ult::_M_PI * (std::fmod(currentTimeCount, cycleDuration) + countOffset) / 1.5);
                            progress = std::cos(counter - ult::_M_PI / 2.0);
                            
                            const tsl::Color highlightColor = {
                                static_cast<u8>((dynamicLogoRGB2.r - dynamicLogoRGB1.r) * (progress + 1.0) * 0.5 + dynamicLogoRGB1.r),
                                static_cast<u8>((dynamicLogoRGB2.g - dynamicLogoRGB1.g) * (progress + 1.0) * 0.5 + dynamicLogoRGB1.g),
                                static_cast<u8>((dynamicLogoRGB2.b - dynamicLogoRGB1.b) * (progress + 1.0) * 0.5 + dynamicLogoRGB1.b),
                                15
                            };
                            
                            const std::string letterStr(1, letter);
                            renderer->drawString(letterStr, false, x, y + offset, fontSize, a(highlightColor));
                            x += tsl::gfx::calculateStringWidth(letterStr, fontSize);
                            countOffset -= 0.2F;
                        }
                    } else {
                        for (const char letter : ult::SPLIT_PROJECT_NAME_1) {
                            const std::string letterStr(1, letter);
                            renderer->drawString(letterStr, false, x, y + offset, fontSize, a(logoColor1));
                            x += tsl::gfx::calculateStringWidth(letterStr, fontSize);
                            countOffset -= 0.2F;
                        }
                    }
                    
                    renderer->drawString(ult::SPLIT_PROJECT_NAME_2, false, x, y + offset, fontSize, a(logoColor2));
                    
                } else {
                    x = 20;
                    y = 52;
                    fontSize = 32;
        
                    if (m_subtitle.find("Ultrahand Script") != std::string::npos) {
                        renderer->drawString(m_title, false, x, y, fontSize, a(defaultScriptColor));
                    } else {
                        titleColor = defaultPackageColor; // Default to green
                        
                        // Ultra-fast color selection using first character optimization
                        if (!m_colorSelection.empty()) {
                            const char firstChar = m_colorSelection[0];
                            const size_t len = m_colorSelection.length();
                            
                            // Fast path: check first char + length for unique combinations
                            switch (firstChar) {
                                case 'g': // green
                                    if (len == 5 && m_colorSelection == "green") {
                                        titleColor = {0x0, 0xF, 0x0, 0xF};
                                    }
                                    break;
                                case 'r': // red
                                    if (len == 3 && m_colorSelection == "red") {
                                        titleColor = RGB888("#F7253E");
                                    }
                                    break;
                                case 'b': // blue
                                    if (len == 4 && m_colorSelection == "blue") {
                                        titleColor = {0x7, 0x7, 0xF, 0xF};
                                    }
                                    break;
                                case 'y': // yellow
                                    if (len == 6 && m_colorSelection == "yellow") {
                                        titleColor = {0xF, 0xF, 0x0, 0xF};
                                    }
                                    break;
                                case 'o': // orange
                                    if (len == 6 && m_colorSelection == "orange") {
                                        titleColor = {0xFF, 0xA5, 0x00, 0xFF};
                                    }
                                    break;
                                case 'p': // pink or purple
                                    if (len == 4 && m_colorSelection == "pink") {
                                        titleColor = {0xFF, 0x69, 0xB4, 0xFF};
                                    } else if (len == 6 && m_colorSelection == "purple") {
                                        titleColor = {0x80, 0x00, 0x80, 0xFF};
                                    }
                                    break;
                                case 'w': // white
                                    if (len == 5 && m_colorSelection == "white") {
                                        titleColor = {0xF, 0xF, 0xF, 0xF};
                                    }
                                    break;
                                case 'u': // ultra
                                    if (len == 5 && m_colorSelection == "ultra") {
                                        for (const char letter : m_title) {
                                            progress = ult::calculateAmplitude(counter - x * 0.0001F);
                                            
                                            const tsl::Color highlightColor = {
                                                static_cast<u8>((0xA - 0xF) * (3 - 1.5 * progress) + 0xF),
                                                static_cast<u8>((0xA - 0xF) * 1.5 * progress + 0xF),
                                                static_cast<u8>((0xA - 0xF) * (1.25 - progress) + 0xF),
                                                0xF
                                            };
                                            
                                            const std::string letterStr(1, letter);
                                            renderer->drawString(letterStr.c_str(), false, x, y, fontSize, a(highlightColor));
                                            letterWidth = tsl::gfx::calculateStringWidth(letterStr, fontSize);
                                            x += letterWidth;
                                            counter -= 0.00004F;
                                        }
                                        goto skip_normal_draw;
                                    }
                                    break;
                                case '#': // hex color
                                    if (len == 7 && ult::isValidHexColor(m_colorSelection.substr(1))) {
                                        titleColor = RGB888(m_colorSelection.substr(1));
                                    }
                                    break;
                            }
                        }
                        
                        renderer->drawString(m_title, false, x, y, fontSize, a(titleColor));
                        skip_normal_draw:;
                    }
                }
                
                if (m_title == ult::CAPITAL_ULTRAHAND_PROJECT_NAME) {
                    renderer->drawString(ult::versionLabel, false, 20, y+25, 15, a(versionTextColor));
                } else {
                    std::string subtitle = m_subtitle;
                    const size_t pos = subtitle.find("?Ultrahand Script");
                    if (pos != std::string::npos) {
                        subtitle.erase(pos, 17); // "?Ultrahand Script".length() = 17
                    }
                    renderer->drawString(subtitle, false, 20, y+23, 15, a(versionTextColor));
                }
        
            #else
                {
                #if USING_WIDGET_DIRECTIVE
                    renderer->drawWidget();
                #endif
                }
                renderer->drawString(m_title, false, 20, 52, 32, a(defaultOverlayColor));
                renderer->drawString(m_subtitle, false, 20, y+23, 15, a(versionTextColor));
            #endif
        
                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(botttomSeparatorColor));
                
                ult::backWidth = tsl::gfx::calculateStringWidth(ult::BACK, 23);
                if (ult::touchingBack) {
                    renderer->drawRoundedRect(18.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              ult::backWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }
        
                ult::selectWidth = tsl::gfx::calculateStringWidth(ult::OK, 23);
                if (ult::touchingSelect && !m_noClickableItems) {
                    renderer->drawRoundedRect(18.0f + ult::backWidth+68.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              ult::selectWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }
                
            #if IS_LAUNCHER_DIRECTIVE
                if (!m_pageLeftName.empty())
                    ult::nextPageWidth = tsl::gfx::calculateStringWidth(m_pageLeftName, 23);
                else if (!m_pageRightName.empty())
                    ult::nextPageWidth = tsl::gfx::calculateStringWidth(m_pageRightName, 23);
                else if (ult::inMainMenu) {
                    ult::nextPageWidth = tsl::gfx::calculateStringWidth(
                        ult::inOverlaysPage ? ult::PACKAGES : ult::OVERLAYS, 23);
                }
        
                if (ult::inMainMenu || !m_pageLeftName.empty() || !m_pageRightName.empty()) {
                    if (ult::touchingNextPage) {
                        renderer->drawRoundedRect(18.0f + ult::backWidth+68.0f + ((!m_noClickableItems) ? ult::selectWidth+68.0f : 0), 
                                                  static_cast<float>(cfg::FramebufferHeight - 73), 
                                                  ult::nextPageWidth+70.0f, 73.0f, 6.0f, a(clickColor));
                    }
                }
            #endif
        
                // Pre-build menu bottom line efficiently
                menuBottomLine.clear();
                menuBottomLine.reserve(128); // Reserve space to avoid reallocations
                
                menuBottomLine += "\uE0E1" + ult::GAP_2 + ult::BACK + ult::GAP_1;
                if (!m_noClickableItems) {
                    menuBottomLine += "\uE0E0" + ult::GAP_2 + ult::OK + ult::GAP_1;
                }
        
            #if IS_LAUNCHER_DIRECTIVE
                if (!ult::usePageSwap) {
                    if (m_menuMode == "packages") {
                        menuBottomLine += "\uE0ED" + ult::GAP_2 + ult::OVERLAYS;
                    } else if (m_menuMode == "overlays") {
                        menuBottomLine += "\uE0EE" + ult::GAP_2 + ult::PACKAGES;
                    }
                } else {
                    if (m_menuMode == "packages") {
                        menuBottomLine += "\uE0EE" + ult::GAP_2 + ult::OVERLAYS;
                    } else if (m_menuMode == "overlays") {
                        menuBottomLine += "\uE0ED" + ult::GAP_2 + ult::PACKAGES;
                    }
                }
                
                if (!m_pageLeftName.empty()) {
                    menuBottomLine += "\uE0ED" + ult::GAP_2 + m_pageLeftName;
                } else if (!m_pageRightName.empty()) {
                    menuBottomLine += "\uE0EE" + ult::GAP_2 + m_pageRightName;
                }
            #endif
                
                // Render the text with special character handling
                static const std::vector<std::string> specialChars = {"\uE0E1","\uE0E0","\uE0ED","\uE0EE"};
                renderer->drawStringWithColoredSections(menuBottomLine, specialChars, 30, 693, 23, a(bottomTextColor), a(buttonColor));
        
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
                
                renderer->drawString(fpsBuffer, false, 20, tsl::cfg::FramebufferHeight - 60, 20, a(tsl::Color(0xFF, 0xFF, 0xFF, 0xFF)));
            #endif
        
                if (m_contentElement != nullptr)
                    m_contentElement->frame(renderer);
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
                if (!m_contentElement || !m_contentElement->inBounds(currX, currY) || !ult::internalTouchReleased)
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
    #endif
        
        /**
         * @brief The base frame which can contain another view with a customizable header
         *
         */
        class HeaderOverlayFrame : public Element {
        public:
            

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
                if (!ult::themeIsInitialized) {
                    tsl::initializeThemeVars(); // Initialize variables for ultrahand themes
                    ult::themeIsInitialized = true;
                }

                renderer->fillScreen(a(defaultBackgroundColor));

                renderer->drawWallpaper();

                //renderer->fillScreen(tsl::style::color::ColorFrameBackground);
                renderer->drawRect(tsl::cfg::FramebufferWidth - 1, 0, 1, tsl::cfg::FramebufferHeight, a(0xF222));
                
                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(botttomSeparatorColor));
                
                //renderer->drawString(("\uE0E1  "+ult::BACK+"     \uE0E0  "+ult::OK), false, 30, 693, 23, a(defaultTextColor)); // CUSTOM MODIFICATION
                
                ult::backWidth = tsl::gfx::calculateStringWidth(ult::BACK, 23);
                if (ult::touchingBack) {
                    renderer->drawRoundedRect(18.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              ult::backWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }

                ult::selectWidth = tsl::gfx::calculateStringWidth(ult::OK, 23);
                if (ult::touchingSelect) {
                    renderer->drawRoundedRect(18.0f + ult::backWidth+68.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              ult::selectWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }

                std::string menuBottomLine = "\uE0E1"+ult::GAP_2+ult::BACK+ult::GAP_1+"\uE0E0"+ult::GAP_2+ult::OK+ult::GAP_1;
                renderer->drawStringWithColoredSections(menuBottomLine, {"\uE0E1","\uE0E0","\uE0ED","\uE0EE"}, 30, 693, 23, a(bottomTextColor), a(buttonColor));

                if (this->m_header != nullptr)
                    this->m_header->frame(renderer);
                
                if (this->m_contentElement != nullptr)
                    this->m_contentElement->frame(renderer);
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

        static std::vector<Element*> s_lastFrameItems;
        static bool s_hasValidFrame = false;
        static float s_lastFrameOffset = 0.0f;
        static size_t s_cachedInstanceId = 0;
        static size_t s_nextInstanceId = 1; 

        class List : public Element {
        
        public:
            List() : Element(), m_instanceId(generateInstanceId()) {
                m_isItem = false;
            }
            virtual ~List() {
                clearItems();
                if (s_cachedInstanceId == m_instanceId) {
                    clearStaticCache();
                }
            }
            
            
            virtual void draw(gfx::Renderer* renderer) override {
                // Early exit optimizations
                if (m_clearList) {
                    clearItems();
                    return;
                }
                
                // Process pending operations in batch
                if (!m_itemsToAdd.empty()) addPendingItems();
                if (!m_itemsToRemove.empty()) removePendingItems();

                
                if (m_pendingJump && s_hasValidFrame) {
                    // Render using cached frame state if available
                    renderCachedFrame(renderer);
                    s_hasValidFrame = false;
                    return;
                }

                cacheCurrentFrame();

                // Cache bounds for hot loop
                const s32 topBound = getTopBound();
                const s32 bottomBound = getBottomBound();
                const s32 height = getHeight();
                
                renderer->enableScissoring(getLeftBound(), topBound, getWidth() + 8, height + 4);
        
                // Optimized visibility culling
                for (Element* entry : m_items) {
                    //const s32 entryBottom = entry->getBottomBound();
                    if (entry->getBottomBound() > topBound && entry->getTopBound() < bottomBound) {
                        entry->frame(renderer);
                    }
                }
                
                renderer->disableScissoring();

                // FIXED: Check if content actually extends beyond viewport bounds
                // Calculate the actual bottom position of the last item
                s32 actualContentBottom = 0;
                if (!m_items.empty()) {
                    Element* lastItem = m_items.back();
                    actualContentBottom = lastItem->getBottomBound() - getTopBound();
                }
        
                // Draw scrollbar only when needed
                if (m_listHeight > height || actualContentBottom > height) {
                    drawScrollbar(renderer, height);
                    updateScrollAnimation();
                }
                
            }
        
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                y = getY() - m_offset;
                
                // Calculate total height in single pass
                m_listHeight = 9;
                for (Element* entry : m_items) {
                    m_listHeight += entry->getHeight();
                    entry->setBoundaries(getX(), y, getWidth(), entry->getHeight());
                    entry->invalidate();
                    y += entry->getHeight();
                }
                y -= 0;
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
                        m_nextOffset = std::clamp(m_nextOffset, 0.0f, 
                            static_cast<float>(m_listHeight - getHeight()));
                        
                        // FIXED: Always detect table state based on current scroll position
                        // This ensures we're in the correct table context regardless of where we scroll
                        detectAndEnterTableAtOffset();
                    }
                    return true;
                }
                
                return false;
            }
            
            // Fixed syncTableStateFromOffset - only called during controller navigation
            inline void syncTableStateFromOffset() {
                // FIXED: Only sync during controller mode to prevent conflicts
                if (Element::getInputMode() != InputMode::Controller) return;
                if (!isInTable || tableIndex >= m_items.size()) return;
                
                float tableStartPos = calculateTableStartPosition(tableIndex);
                float relativeOffset = m_offset - tableStartPos;
                
                Element* table = m_items[tableIndex];
                float maxTableScroll = static_cast<float>(table->getHeight() - getHeight());
                tableScrollOffset = std::clamp(relativeOffset, 0.0f, maxTableScroll);
            }
            
            // Fixed detectAndEnterTableAtOffset - only called during controller navigation
            inline void detectAndEnterTableAtOffset() {
                // Always reset first, then detect based on current position
                //bool wasInTable = isInTable;
                //size_t oldTableIndex = tableIndex;
                resetTableState();
                
                float currentPos = 0.0f;
                float itemHeight;
                
                for (size_t i = 0; i < m_items.size(); ++i) {
                    itemHeight = static_cast<float>(m_items[i]->getHeight());
                    
                    // Check if current offset falls within this item
                    if (m_offset >= currentPos && m_offset < currentPos + itemHeight) {
                        m_focusedIndex = i;
                        
                        // If this item is a table that can be entered
                        if (canEnterTable(i)) {
                            isInTable = true;
                            tableIndex = i;
                            tableScrollOffset = m_offset - currentPos;
                            
                            // Ensure table scroll offset is within bounds
                            Element* table = m_items[i];
                            float maxTableScroll = static_cast<float>(table->getHeight() - getHeight());
                            tableScrollOffset = std::clamp(tableScrollOffset, 0.0f, maxTableScroll);
                        }
                        break;
                    }
                    currentPos += itemHeight;
                }
            }

            inline void addItem(Element* element, u16 height = 0, ssize_t index = -1) {
                if (!element) return;
                
                // First item optimization
                if (actualItemCount == 0 && element->m_isItem) {
                    auto* customDrawer = new tsl::elm::CustomDrawer([](gfx::Renderer*, s32, s32, s32, s32) {});
                    customDrawer->setBoundaries(getX(), getY(), getWidth(), tsl::style::ListItemDefaultHeight / 2);
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
                    return handleJumpToItem(handleJumpToItem(oldFocus));
                } else if (m_pendingJump) {
                    m_pendingJump = false;
                    delayedHandle = false;
                    return handleJumpToItem(oldFocus); // needs to be handled 2x for proper rendering
                }
                
                if (jumpToBottom) {
                    jumpToBottom = false;
                    return handleJumpToBottom(oldFocus);
                }
            
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

            inline void jumpToItem(const std::string& text = "", const std::string& value = "") {
                m_pendingJump = true;
                m_jumpToText = text;
                m_jumpToValue = value;
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
            const size_t m_instanceId;

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
            bool m_pendingJump = false;

            // Stack variables for hot path - reused to avoid allocations
            u32 scrollbarHeight;
            u32 scrollbarOffset;
            u32 prevOffset;
        
            static constexpr float smoothingFactor = 0.15f;
            static constexpr float dampingFactor = 0.3f;
            static constexpr float TABLE_SCROLL_STEP_SIZE = 40.0f;
        
            // Simplified table state - just track which table and its scroll position
            bool isInTable = false;
            size_t tableIndex = 0;
            float tableScrollOffset = 0.0f;  // Offset within the table itself
            
            enum class NavigationResult {
                None,
                Success,
                HitBoundary,
                Wrapped
            };
            
            bool m_hasWrappedInCurrentSequence = false;
            NavigationResult m_lastNavigationResult = NavigationResult::None;
        
        private:
            // Method to explicitly preserve cache when navigating away
            void preserveCacheForReturn() {
                if (m_instanceId == s_cachedInstanceId && s_hasValidFrame) {
                    // Cache is already preserved for this instance
                    return;
                }
                cacheCurrentFrame();
            }
        
            // Method to check if this instance has a valid cached frame
            bool hasCachedFrame() const {
                return s_hasValidFrame && s_cachedInstanceId == m_instanceId;
            }

            static size_t generateInstanceId() {
                return s_nextInstanceId++;
            }
        
            static void clearStaticCache() {
                s_lastFrameItems.clear();
                s_hasValidFrame = false;
                s_lastFrameOffset = 0.0f;
                s_cachedInstanceId = 0;
            }
        
            void cacheCurrentFrame() {
                s_lastFrameItems = m_items; // Cache this instance's items
                s_lastFrameOffset = m_offset;
                s_cachedInstanceId = m_instanceId; // Mark which instance this cache belongs to
                s_hasValidFrame = true;
            }
            
            void renderCachedFrame(gfx::Renderer* renderer) {
                const s32 topBound = getTopBound();
                const s32 bottomBound = getBottomBound();
                const s32 height = getHeight();
                
                renderer->enableScissoring(getLeftBound(), topBound, getWidth() + 8, height + 4);
            
                // Use cached offset for positioning
                for (Element* entry : s_lastFrameItems) {
                    // Adjust positioning based on cached offset
                    if (entry->getBottomBound() > topBound && entry->getTopBound() < bottomBound) {
                        entry->frame(renderer);
                    }
                }
                
                renderer->disableScissoring();
                
                if (m_listHeight > height) {
                    drawScrollbar(renderer, height);
                }
            }


            inline void clearItems() {
                // Clear static cache if it belongs to this instance
                if (s_cachedInstanceId == m_instanceId) {
                    clearStaticCache();
                }

                for (Element* item : m_items) delete item;
                m_items.clear();
                m_offset = 0;
                m_focusedIndex = 0;
                invalidate();
                m_clearList = false;
                actualItemCount = 0;
                resetTableState();
            }
            
            inline void addPendingItems() {
                for (auto [index, element] : m_itemsToAdd) {
                    element->invalidate();
                    if (index >= 0 && static_cast<size_t>(index) < m_items.size()) {
                        m_items.insert(m_items.begin() + index, element);
                    } else {
                        m_items.push_back(element);
                    }
                }
                m_itemsToAdd.clear();
                invalidate();
                updateScrollOffset();
            }
            
            inline void removePendingItems() {
                size_t index;
                for (Element* element : m_itemsToRemove) {
                    auto it = std::find(m_items.begin(), m_items.end(), element);
                    if (it != m_items.end()) {
                        index = static_cast<size_t>(it - m_items.begin());
                        m_items.erase(it);
                        if (m_focusedIndex >= index && m_focusedIndex > 0) {
                            --m_focusedIndex;
                        }
                        delete element;
                    }
                }
                m_itemsToRemove.clear();
                invalidate();
                updateScrollOffset();
            }
            
            inline void drawScrollbar(gfx::Renderer* renderer, s32 height) {
                const float viewHeight = static_cast<float>(height - 10);
                const float totalHeight = static_cast<float>(m_listHeight-22);
                const u32 maxScrollableHeight = std::max(static_cast<u32>(totalHeight - viewHeight), 1u);
                
                scrollbarHeight = std::min(static_cast<u32>((viewHeight * viewHeight) / totalHeight), 
                                         static_cast<u32>(viewHeight));
                
                scrollbarOffset = std::min(static_cast<u32>((m_offset / maxScrollableHeight) * (viewHeight - scrollbarHeight)), 
                                         static_cast<u32>(viewHeight - scrollbarHeight)) + 4;
        
                const u32 scrollbarX = getRightBound() + 20;
                const u32 scrollbarY = getY() + scrollbarOffset+2;
        
                renderer->drawRect(scrollbarX, scrollbarY, 5, scrollbarHeight, a(trackBarColor));
                renderer->drawCircle(scrollbarX + 2, scrollbarY, 2, true, a(trackBarColor));
                renderer->drawCircle(scrollbarX + 2, scrollbarY + scrollbarHeight, 2, true, a(trackBarColor));
            }
        
            inline void updateScrollAnimation() {
                if (Element::getInputMode() == InputMode::Controller) {
                    static float velocity = 0.0f;
                    velocity = velocity * dampingFactor + (m_nextOffset - m_offset) * smoothingFactor;
                    
                    if (std::abs(velocity) < 0.2f) {
                        m_offset = m_nextOffset;
                        velocity = 0.0f;
                    } else {
                        m_offset += velocity;
                    }
                } else if (Element::getInputMode() == InputMode::TouchScroll) {
                    m_offset += (m_nextOffset - m_offset);
                }
                
                if (prevOffset != m_offset) {
                    invalidate();
                    prevOffset = m_offset;
                }
            }
        
            Element* handleInitialFocus(Element* oldFocus) {
                // FIXED: Always detect table state when controller mode starts
                // This ensures we're in the right state regardless of how we got to this position
                detectAndEnterTableAtOffset();
                
                resetNavigationState();
                
                // Start from the focused index (which was set by detectAndEnterTableAtOffset)
                for (size_t i = m_focusedIndex; i < m_items.size(); ++i) {
                    Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::None);
                    if (newFocus && newFocus != oldFocus) {
                        m_focusedIndex = i;
                        // Don't call updateScrollOffset() - we want to maintain current position
                        return newFocus;
                    }
                }
                
                // If nothing from focused index onwards, try from the beginning
                for (size_t i = 0; i < m_focusedIndex && i < m_items.size(); ++i) {
                    Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::None);
                    if (newFocus && newFocus != oldFocus) {
                        m_focusedIndex = i;
                        return newFocus;
                    }
                }
                
                return nullptr;
            }
                                                                                            
            inline Element* handleDownFocus(Element* oldFocus) {
                updateHoldState();
                
                // FIXED: Only stop at boundary for non-table navigation
                if (m_isHolding && m_stoppedAtBoundary && !isInTable) {
                    return oldFocus;
                }
                
                Element* result = navigateDown(oldFocus);
                
                if (result != oldFocus) {
                    m_lastNavigationResult = NavigationResult::Success;
                    m_stoppedAtBoundary = false;
                    return result;
                }
                
                // At boundary - check for wrapping
                if (!m_isHolding && !m_hasWrappedInCurrentSequence && isAtAbsoluteBottom()) {
                    m_hasWrappedInCurrentSequence = true;
                    m_lastNavigationResult = NavigationResult::Wrapped;
                    return wrapToTop(oldFocus);
                }
                
                // FIXED: Only set boundary stop for non-table items
                if (m_isHolding && !isInTable) {
                    m_stoppedAtBoundary = true;
                }
                m_lastNavigationResult = NavigationResult::HitBoundary;
                return oldFocus;
            }
        
            inline Element* handleUpFocus(Element* oldFocus) {
                updateHoldState();
                
                // FIXED: Only stop at boundary for non-table navigation
                if (m_isHolding && m_stoppedAtBoundary && !isInTable) {
                    return oldFocus;
                }
                
                Element* result = navigateUp(oldFocus);
                
                if (result != oldFocus) {
                    m_lastNavigationResult = NavigationResult::Success;
                    m_stoppedAtBoundary = false;
                    return result;
                }
                
                // At boundary - check for wrapping
                if (!m_isHolding && !m_hasWrappedInCurrentSequence && isAtAbsoluteTop()) {
                    m_hasWrappedInCurrentSequence = true;
                    m_lastNavigationResult = NavigationResult::Wrapped;
                    return wrapToBottom(oldFocus);
                }
                
                // FIXED: Only set boundary stop for non-table items
                if (m_isHolding && !isInTable) {
                    m_stoppedAtBoundary = true;
                }
                m_lastNavigationResult = NavigationResult::HitBoundary;
                return oldFocus;
            }
        
            inline void updateHoldState() {
                u64 currentTime = armTicksToNs(armGetSystemTick());
                if (m_lastNavigationTime != 0 && (currentTime - m_lastNavigationTime) < HOLD_THRESHOLD_NS) {
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
        
            inline void resetTableState() {
                isInTable = false;
                tableIndex = 0;
                tableScrollOffset = 0.0f;
            }

            inline Element* handleJumpToItem(Element* oldFocus) {
                resetTableState();
                resetNavigationState();
                invalidate();
            
                const bool needsScroll = m_listHeight > getHeight();
                const float viewportThird = needsScroll ? getHeight() / 3.0f : 0.0f;
                const float maxOffset = needsScroll ? m_listHeight - getHeight() : 0.0f;
                
                float h = 0.0f;
                
                for (size_t i = 0; i < m_items.size(); ++i) {
                    m_focusedIndex = i;
                    
                    Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::Down);
                    if (newFocus && newFocus != oldFocus && m_items[i]->matchesJumpCriteria(m_jumpToText, m_jumpToValue)) {
                        m_offset = m_nextOffset = needsScroll && i ? std::clamp(h - viewportThird, 0.0f, maxOffset) : 0.0f;
                        return newFocus;
                    }
                    
                    h += m_items[i]->getHeight();
                }
                
                // No match found
                return handleInitialFocus(oldFocus);
            }
        
            // Core navigation logic
            inline Element* navigateDown(Element* oldFocus) {
                if (isInTable) {
                    return handleTableNavigationDown(oldFocus);
                }
                
                // FIXED: Check if we just wrapped AND we're still at a boundary
                if (m_justWrapped) {
                    m_justWrapped = false;
                    // Only stay put if we're actually at the first item and there's a table after us
                    if (m_focusedIndex == 0 || (m_focusedIndex < m_items.size() - 1 && canEnterTable(m_focusedIndex + 1))) {
                        return oldFocus; // Stay on current item for one navigation cycle
                    }
                }
                
                // Look for next focusable item
                for (size_t i = m_focusedIndex + 1; i < m_items.size(); ++i) {
                    if (canEnterTable(i)) {
                        return enterTable(oldFocus, i, true); // Enter from top
                    } else if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        updateScrollOffset();
                        return m_items[i]->requestFocus(oldFocus, FocusDirection::Down);
                    }
                }
                
                return oldFocus;
            }

                    
            inline Element* navigateUp(Element* oldFocus) {
                if (isInTable) {
                    return handleTableNavigationUp(oldFocus);
                }
                
                // FIXED: If we just wrapped, don't immediately enter a table
                if (m_justWrapped) {
                    m_justWrapped = false;
                    return oldFocus; // Stay on current item for one navigation cycle
                }
                
                // Look for previous focusable item
                for (ssize_t i = static_cast<ssize_t>(m_focusedIndex) - 1; i >= 0; --i) {
                    //size_t idx = static_cast<size_t>(i);
                    if (canEnterTable(i)) {
                        return enterTable(oldFocus, i, false); // Enter from bottom
                    } else if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        updateScrollOffset();
                        return m_items[i]->requestFocus(oldFocus, FocusDirection::Up);
                    }
                }
                
                return oldFocus;
            }
        
            // Table navigation
            inline Element* handleTableNavigationDown(Element* oldFocus) {
                if (!canScrollTableDown()) {
                    // Try to exit table downward
                    return exitTableDown(oldFocus);
                }
                
                // FIXED: Don't stop at boundary when holding - allow continuous scrolling
                scrollTableDown();
                return oldFocus;
            }

            inline Element* handleTableNavigationUp(Element* oldFocus) {
                if (!canScrollTableUp()) {
                    // Try to exit table upward
                    return exitTableUp(oldFocus);
                }
                
                // FIXED: Don't stop at boundary when holding - allow continuous scrolling
                scrollTableUp();
                return oldFocus;
            }
        
            inline bool canScrollTableDown() {
                if (tableIndex >= m_items.size()) return false;
                
                Element* table = m_items[tableIndex];
                // FIXED: Add buffer to ensure we can see the very bottom content
                float maxScroll = static_cast<float>(table->getHeight() - getHeight());
                return tableScrollOffset < maxScroll && maxScroll > 0;
            }
        
            inline bool canScrollTableUp() {
                return tableScrollOffset > 0.0f;
            }
        
            inline void scrollTableDown() {
                if (tableIndex >= m_items.size()) return;
                
                Element* table = m_items[tableIndex];
                // FIXED: Same buffer calculation as canScrollTableDown
                float maxScroll = static_cast<float>(table->getHeight() - getHeight());
                
                tableScrollOffset = std::min(tableScrollOffset + TABLE_SCROLL_STEP_SIZE, maxScroll);
                
                // Update global scroll position
                float tableStartPos = calculateTableStartPosition(tableIndex);
                m_nextOffset = tableStartPos + tableScrollOffset;
            }

            inline void scrollTableUp() {
                tableScrollOffset = std::max(tableScrollOffset - TABLE_SCROLL_STEP_SIZE, 0.0f);
                
                // Update global scroll position
                float tableStartPos = calculateTableStartPosition(tableIndex);
                m_nextOffset = tableStartPos + tableScrollOffset;
            }
        
            inline float calculateTableStartPosition(size_t tableIdx) {
                float pos = 0.0f;
                for (size_t i = 0; i < tableIdx && i < m_items.size(); ++i) {
                    pos += m_items[i]->getHeight();
                }
                return pos;
            }
            
            Element* enterTable(Element* oldFocus, size_t tableIdx, bool fromTop) {
                isInTable = true;
                tableIndex = tableIdx;
                m_focusedIndex = tableIdx;
                
                float tableStartPos = calculateTableStartPosition(tableIdx);
                
                if (fromTop) {
                    tableScrollOffset = 0.0f;
                    // FIXED: Use smooth animation when entering table from top
                    m_nextOffset = 0;
                } else {
                    // Enter from bottom - scroll to show the bottom of the table
                    Element* table = m_items[tableIdx];
                    tableScrollOffset = static_cast<float>(table->getHeight() - getHeight());
                    if (tableScrollOffset < 0) tableScrollOffset = 0.0f;
                    
                    // FIXED: Use smooth animation when entering table from bottom
                    m_nextOffset = tableStartPos + tableScrollOffset;
                }
                
                // Don't set m_offset directly - let the animation system handle it
                return oldFocus;
            }
        
            Element* exitTableDown(Element* oldFocus) {
                size_t currentTableIndex = tableIndex;
                resetTableState();
                m_focusedIndex = currentTableIndex;
                
                // Look for next focusable item after current table
                for (size_t i = currentTableIndex + 1; i < m_items.size(); ++i) {
                    if (canEnterTable(i)) {
                        return enterTable(oldFocus, i, true);
                    } else if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        updateScrollOffset();
                        return m_items[i]->requestFocus(oldFocus, FocusDirection::Down);
                    }
                }
                
                return oldFocus;
            }
        
            Element* exitTableUp(Element* oldFocus) {
                size_t currentTableIndex = tableIndex;
                resetTableState();
                m_focusedIndex = currentTableIndex;
                
                // Look for previous focusable item before current table
                for (ssize_t i = static_cast<ssize_t>(currentTableIndex) - 1; i >= 0; --i) {
                    //ize_t idx = static_cast<size_t>(i);
                    if (canEnterTable(i)) {
                        return enterTable(oldFocus, i, false);
                    } else if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        updateScrollOffset();
                        return m_items[i]->requestFocus(oldFocus, FocusDirection::Up);
                    }
                }
                
                return oldFocus;
            }
        
            // Boundary detection
            inline bool isAtAbsoluteTop() {
                if (m_items.empty()) return true;
                
                if (isInTable) {
                    // Must be at top of table AND no focusable items above
                    bool atTableTop = (tableScrollOffset <= 1.0f); // Small epsilon
                    return atTableTop && !hasFocusableItemsBefore(tableIndex);
                }
                
                // Not in table - check if at very first focusable item
                return !hasFocusableItemsBefore(m_focusedIndex);
            }
                                
            inline bool isAtAbsoluteBottom() {
                if (m_items.empty()) return true;
                
                if (isInTable) {
                    // Must be at bottom of table AND no focusable items below
                    if (tableIndex >= m_items.size()) return true;
                    
                    Element* table = m_items[tableIndex];
                    float maxScroll = static_cast<float>(table->getHeight() - getHeight());
                    
                    // Use a small epsilon for floating point comparison
                    bool atTableBottom = (tableScrollOffset >= maxScroll - 1.0f);
                    return atTableBottom && !hasFocusableItemsAfter(tableIndex);
                }
                
                // Not in table - check if at very last focusable item
                return !hasFocusableItemsAfter(m_focusedIndex);
            }
                    
            inline bool hasFocusableItemsBefore(size_t index) {
                for (ssize_t i = static_cast<ssize_t>(index) - 1; i >= 0; --i) {
                    //size_t idx = static_cast<size_t>(i);
                    if (canFocusRegularItem(i) || canEnterTable(i)) {
                        return true;
                    }
                }
                return false;
            }
        
            inline bool hasFocusableItemsAfter(size_t index) {
                for (size_t i = index + 1; i < m_items.size(); ++i) {
                    if (canFocusRegularItem(i) || canEnterTable(i)) {
                        return true;
                    }
                }
                return false;
            }
        
            // Helper functions
            inline bool canEnterTable(size_t index) {
                if (index >= m_items.size() || !m_items[index]) return false;
                return m_items[index]->isTable() && (m_items[index]->getHeight() > getHeight());
            }
        
            inline bool canFocusRegularItem(size_t index) {
                if (index >= m_items.size() || !m_items[index]) return false;
                if (m_items[index]->isTable()) return false;
                
                // FIXED: More robust focus testing - try both directions
                Element* testFocus = m_items[index]->requestFocus(nullptr, FocusDirection::None);
                if (testFocus != nullptr) return true;
                
                // Try with directional focus as backup
                testFocus = m_items[index]->requestFocus(nullptr, FocusDirection::Down);
                return (testFocus != nullptr);
            }
        
            // Wrapping
            // Fix for the wrapToTop function
            Element* wrapToTop(Element* oldFocus) {
                resetTableState();
                
                for (size_t i = 0; i < m_items.size(); ++i) {
                    if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::Down);
                        if (newFocus && newFocus != oldFocus) {
                            // FIXED: Only set target offset, let animation system handle the smooth transition
                            m_nextOffset = 0.0f;
                            // Don't set m_offset - let updateScrollAnimation() handle the smooth transition
                            return newFocus;
                        }
                    } else if (canEnterTable(i)) {
                        return enterTable(oldFocus, i, true);
                    }
                }
                
                return oldFocus;
            }

                        
            // Also fix wrapToBottom for consistency
            Element* wrapToBottom(Element* oldFocus) {
                resetTableState();
                invalidate();
                // REMOVED: m_justWrapped = true; // This was causing the issue
                
                //size_t idx;
                // More thorough search for the last focusable item
                for (ssize_t i = static_cast<ssize_t>(m_items.size()) - 1; i >= 0; --i) {
                    //idx = static_cast<size_t>(i);
                    if (canEnterTable(i)) {
                        return enterTable(oldFocus, i, false);
                    } else if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::Up);
                        if (newFocus && newFocus != oldFocus) {
                            // FIXED: Only set target offset for smooth animation to bottom
                            if (m_listHeight > getHeight()) {
                                
                                m_nextOffset = static_cast<float>(m_listHeight - getHeight());
                                // Don't set m_offset - let updateScrollAnimation() handle the smooth transition
                            }
                            return newFocus;
                        }
                    }
                }
                
                return oldFocus;
            }
            
            
            // Add these methods to handle jumps with smooth scrolling
            Element* handleJumpToTop(Element* oldFocus) {
                if (m_items.empty()) return oldFocus;
                
                // Check if already at the top-most focusable item
                size_t firstFocusableIndex = SIZE_MAX;
                for (size_t i = 0; i < m_items.size(); ++i) {
                    if (canFocusRegularItem(i) || canEnterTable(i)) {
                        firstFocusableIndex = i;
                        break;
                    }
                }
                
                // If we're already at the first focusable item and not in a table, do nothing
                if (firstFocusableIndex != SIZE_MAX && 
                    m_focusedIndex == firstFocusableIndex && 
                    !isInTable) {
                    return oldFocus;
                }
                
                // If we're in a table at the first position and already at top of table, do nothing
                if (isInTable && tableIndex == firstFocusableIndex && tableScrollOffset <= 1.0f) {
                    return oldFocus;
                }
                
                resetTableState();
                resetNavigationState();
                
                // Find the first focusable item
                for (size_t i = 0; i < m_items.size(); ++i) {
                    if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        m_nextOffset = 0.0f; // Set target for smooth scroll to top
                        // Don't set m_offset - let updateScrollAnimation() handle smooth transition
                        Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::None);
                        if (newFocus && newFocus != oldFocus) {
                            invalidate();
                            return newFocus;
                        }
                    } else if (canEnterTable(i)) {
                        m_nextOffset = 0.0f; // Set target for smooth scroll to top
                        // Don't set m_offset - let updateScrollAnimation() handle smooth transition
                        Element* newFocus = enterTable(oldFocus, i, true);
                        invalidate();
                        return newFocus ? newFocus : oldFocus;
                    }
                }
                
                return oldFocus;
            }
            
            Element* handleJumpToBottom(Element* oldFocus) {
                if (m_items.empty()) return oldFocus;
                
                // Check if already at the bottom-most focusable item
                size_t lastFocusableIndex = SIZE_MAX;
                //size_t idx;
                for (ssize_t i = static_cast<ssize_t>(m_items.size()) - 1; i >= 0; --i) {
                    //idx = static_cast<size_t>(i);
                    if (canFocusRegularItem(i) || canEnterTable(i)) {
                        lastFocusableIndex = i;
                        break;
                    }
                }
                
                // If we're already at the last focusable item and not in a table, do nothing
                if (lastFocusableIndex != SIZE_MAX && 
                    m_focusedIndex == lastFocusableIndex && 
                    !isInTable) {
                    return oldFocus;
                }
                
                float maxScroll;
                // If we're in a table at the last position and already at bottom of table, do nothing
                if (isInTable && tableIndex == lastFocusableIndex) {
                    if (tableIndex < m_items.size()) {
                        Element* table = m_items[tableIndex];
                        maxScroll = static_cast<float>(table->getHeight() - getHeight());
                        if (tableScrollOffset >= maxScroll - 1.0f) {
                            return oldFocus;
                        }
                    }
                }
                
                resetTableState();
                resetNavigationState();
                invalidate();
                
                // Find the last focusable item
                for (ssize_t i = static_cast<ssize_t>(m_items.size()) - 1; i >= 0; --i) {
                    //idx = static_cast<size_t>(i);
                    if (canEnterTable(i)) {
                        // Set target for smooth scroll to bottom position
                        if (m_listHeight > getHeight()) {
                            m_nextOffset = static_cast<float>(m_listHeight - getHeight());
                        }
                        // Don't set m_offset - let updateScrollAnimation() handle smooth transition
                        Element* newFocus = enterTable(oldFocus, i, false);
                        invalidate();
                        return newFocus ? newFocus : oldFocus;
                    } else if (canFocusRegularItem(i)) {
                        m_focusedIndex = i;
                        // Set target for smooth scroll to bottom position
                        if (m_listHeight > getHeight()) {
                            m_nextOffset = static_cast<float>(m_listHeight - getHeight());
                        }
                        // Don't set m_offset - let updateScrollAnimation() handle smooth transition
                        Element* newFocus = m_items[i]->requestFocus(oldFocus, FocusDirection::None);
                        if (newFocus && newFocus != oldFocus) {
                            invalidate();
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
            
            virtual inline void updateScrollOffset() {

                if (Element::getInputMode() != InputMode::Controller) return;
                
                if (m_listHeight <= getHeight()) {
                    m_nextOffset = m_offset = 0;
                    return;
                }
                
                // If in table, don't override table scrolling
                if (isInTable) return;
                
                if (prefixSums.size() != m_items.size() + 1) {
                    initializePrefixSums();
                }
                
                // FIXED: If focused on first item, keep at absolute top
                if (m_focusedIndex == 0) {
                    m_nextOffset = 0.0f;
                    return;
                }
                
                m_nextOffset = std::clamp(prefixSums[m_focusedIndex] - (getHeight() / 3), 
                                        0.0f, 
                                        static_cast<float>(m_listHeight - getHeight()));
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
        
        #if IS_LAUNCHER_DIRECTIVE
            ListItem(const std::string& text, const std::string& value = "", bool isMini = false, bool useScriptKey = false)
                : Element(), m_text(text), m_value(value), m_listItemHeight(isMini ? tsl::style::MiniListItemDefaultHeight : tsl::style::ListItemDefaultHeight)
        #if IS_LAUNCHER_DIRECTIVE
                , m_useScriptKey(useScriptKey)
        #endif
            {
                m_isItem = true;
                applyInitialTranslations();
                if (!value.empty()) applyInitialTranslations(true);
            }
        #else
            ListItem(const std::string& text, const std::string& value = "", bool isMini = false)
                : Element(), m_text(text), m_value(value), m_listItemHeight(isMini ? tsl::style::MiniListItemDefaultHeight : tsl::style::ListItemDefaultHeight) {
                m_isItem = true;
                applyInitialTranslations();
                if (!value.empty()) applyInitialTranslations(true);
            }
        #endif
        
            virtual ~ListItem() = default;
        
            virtual void draw(gfx::Renderer *renderer) override {
                const bool useClickTextColor = m_touched && Element::getInputMode() == InputMode::Touch && ult::touchInBounds;
                
                if (useClickTextColor) [[unlikely]] {
                    renderer->drawRect(this->getX() + 4, this->getY(), this->getWidth() - 8, this->getHeight(), a(clickColor));
                }
        
                const s32 yOffset = (tsl::style::ListItemDefaultHeight - m_listItemHeight) >> 1; // Bit shift for division by 2
        
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
        
                // Fast path for non-truncated text
                if (!m_truncated) [[likely]] {
                    renderer->drawStringWithColoredSections(m_text, {ult::STAR_SYMBOL + "  "}, this->getX() + 19, this->getY() + 45 - yOffset, 23,
                        a(m_focused ? (useClickTextColor ? clickTextColor : selectedTextColor) : (useClickTextColor ? clickTextColor : defaultTextColor)),
                        a(m_focused ? starColor : selectionStarColor));
                } else {
                    drawTruncatedText(renderer, yOffset, useClickTextColor);
                }
        
                if (!m_value.empty()) [[likely]] {
                    drawValue(renderer, yOffset, useClickTextColor);
                }
            }
        
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX() + 3, this->getY(), this->getWidth() + 9, m_listItemHeight);
            }
        
            virtual bool onClick(u64 keys) override {
                if (ult::simulatedSelect && !ult::simulatedSelectComplete) [[unlikely]] {
                    keys |= KEY_A;
                    ult::simulatedSelect = false;
                }
                
                if (keys & KEY_A) [[likely]] {
                    triggerClickAnimation();
                    ult::simulatedSelectComplete = true;
                } else if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) [[unlikely]] {
                    m_clickAnimationProgress = 0;
                }
                return Element::onClick(keys);
            }
        
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                if (event == TouchEvent::Touch) [[likely]] {
                    if ((m_touched = inBounds(currX, currY))) [[likely]] {
                        m_touchStartTime_ns = armTicksToNs(armGetSystemTick());
                    }
                    return false;
                }
        
                if (event == TouchEvent::Release && m_touched) [[likely]] {
                    m_touched = false;
                    if (Element::getInputMode() == InputMode::Touch) [[likely]] {
        #if IS_LAUNCHER_DIRECTIVE
                        const s64 keyToUse = determineKeyOnTouchRelease(m_useScriptKey);
                        const bool handled = onClick(keyToUse);
        #else
                        const bool handled = onClick(KEY_A);
        #endif
                        m_clickAnimationProgress = 0;
                        return handled;
                    }
                }
                return false;
            }
            
            virtual void setFocused(bool state) override {
                if (state != m_focused) [[likely]] {
                    m_scroll = false;
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
                    resetTextProperties();
                    applyInitialTranslations();
                }
            }
        
            inline void setValue(const std::string& value, bool faint = false) {
                if (m_value != value || m_faint != faint) [[likely]] {
                    m_value = value;
                    m_faint = faint;
                    m_maxWidth = 0;
                    if (!value.empty()) applyInitialTranslations(true);
                }
            }
        
            inline const std::string& getText() const noexcept {
                return m_text;
            }
        
            inline const std::string& getValue() const noexcept {
                return m_value;
            }

            virtual bool matchesJumpCriteria(const std::string& jumpText, const std::string& jumpValue) const {
                if (jumpText.empty() && jumpValue.empty()) return false;
                
                bool textMatches = (m_text == jumpText);
                bool valueMatches = (m_value == jumpValue);
                
                return textMatches || valueMatches;
            }
        
        protected:
            u64 timeIn_ns;
            std::string m_text;
            std::string m_value;
            std::string m_scrollText;
            std::string m_ellipsisText;
            u32 m_listItemHeight;
        
        #if IS_LAUNCHER_DIRECTIVE
            bool m_useScriptKey = false;
        #endif
        
            bool m_scroll = false;
            bool m_truncated = false;
            bool m_faint = false;
            bool m_touched = false;
        
            float m_scrollOffset = 0.0f;
            u32 m_maxWidth = 0;
            u32 m_textWidth = 0;
        
        private:
            void applyInitialTranslations(bool isValue = false) {
                std::string& target = isValue ? m_value : m_text;
                ult::applyLangReplacements(target, isValue);
                ult::convertComboToUnicode(target);
            }
        
            void calculateWidths(gfx::Renderer* renderer) {
                m_maxWidth = m_value.empty() ? 
                    getWidth() - 62 : 
                    getWidth() - tsl::gfx::calculateStringWidth(m_value, 20) - 66;
        
                const u32 width = tsl::gfx::calculateStringWidth(m_text, 23);
                m_truncated = width > m_maxWidth + 20;
        
                if (m_truncated) [[unlikely]] {
                    m_scrollText.reserve(m_text.size() * 2 + 8); // Pre-allocate
                    m_scrollText = m_text + "        ";
                    m_textWidth = tsl::gfx::calculateStringWidth(m_scrollText, 23);
                    m_scrollText += m_text;
                    m_ellipsisText = renderer->limitStringLength(m_text, false, 23, m_maxWidth);
                } else {
                    m_textWidth = width;
                }
            }
        
            void drawTruncatedText(gfx::Renderer* renderer, s32 yOffset, bool useClickTextColor) {
                if (m_focused) [[likely]] {
                    renderer->enableScissoring(getX() + 6, 97, m_maxWidth + (m_value.empty() ? 49 : 27), tsl::cfg::FramebufferHeight - 170);
                    renderer->drawString(m_scrollText, false, getX() + 19 - static_cast<s32>(m_scrollOffset), getY() + 45 - yOffset, 23, a(selectedTextColor));
                    renderer->disableScissoring();
                    handleScrolling();
                } else {
                    renderer->drawString(m_ellipsisText, false, getX() + 19, getY() + 45 - yOffset, 23,
                        a(useClickTextColor ? clickTextColor : defaultTextColor));
                }
            }
        
            void handleScrolling() {
                const u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                const u64 elapsed_ns = currentTime_ns - timeIn_ns;
                
                if (elapsed_ns >= 2000000000ULL) [[likely]] {
                    if (m_scrollOffset >= m_textWidth) [[unlikely]] {
                        m_scrollOffset = 0;
                        timeIn_ns = currentTime_ns;
                    } else {
                        const u64 scroll_elapsed_ms = (elapsed_ns - 2000000000ULL) / 1000000ULL;
                        m_scrollOffset = 0.1f * scroll_elapsed_ms;
                    }
                }
            }
        
            void drawValue(gfx::Renderer* renderer, s32 yOffset, bool useClickTextColor) {
                const s32 xPosition = getX() + m_maxWidth + 47;
                const s32 yPosition = getY() + 45 - yOffset;
                constexpr s32 fontSize = 20;
        
                static bool lastRunningInterpreter = false;
                const auto textColor = determineValueTextColor(useClickTextColor, lastRunningInterpreter);
        
                if (m_value != ult::INPROGRESS_SYMBOL) [[likely]] {
                    renderer->drawString(m_value, false, xPosition, yPosition, fontSize, textColor);
                } else {
                    drawThrobber(renderer, xPosition, yPosition, fontSize, textColor);
                }
                lastRunningInterpreter = ult::runningInterpreter.load(std::memory_order_relaxed); // Relaxed ordering is sufficient
            }
        
            Color determineValueTextColor(bool useClickTextColor, bool lastRunningInterpreter) const {
                // Fast path for most common cases
                if (m_value == ult::DROPDOWN_SYMBOL || m_value == ult::OPTION_SYMBOL) [[unlikely]] {
                    return a(m_focused ? (useClickTextColor ? clickTextColor : (m_faint ? offTextColor : selectedTextColor)) :
                        (useClickTextColor ? clickTextColor : (m_faint ? offTextColor : defaultTextColor)));
                }
                
                const bool isRunning = ult::runningInterpreter.load(std::memory_order_relaxed) || lastRunningInterpreter;
                if (isRunning && (m_value.find(ult::DOWNLOAD_SYMBOL) != std::string::npos ||
                                 m_value.find(ult::UNZIP_SYMBOL) != std::string::npos ||
                                 m_value.find(ult::COPY_SYMBOL) != std::string::npos)) [[unlikely]] {
                    return m_faint ? offTextColor : a(inprogressTextColor);
                }
                
                if (m_value == ult::INPROGRESS_SYMBOL) [[unlikely]] {
                    return m_faint ? offTextColor : a(inprogressTextColor);
                }
                
                if (m_value == ult::CROSSMARK_SYMBOL) [[unlikely]] {
                    return m_faint ? offTextColor : a(invalidTextColor);
                }
                
                return m_faint ? offTextColor : a(onTextColor);
            }
        
            void drawThrobber(gfx::Renderer* renderer, s32 xPosition, s32 yPosition, s32 fontSize, Color textColor) {
                static size_t throbberCounter = 0;
                const auto& throbberSymbol = ult::THROBBER_SYMBOLS[(throbberCounter / 10) % ult::THROBBER_SYMBOLS.size()];
                throbberCounter = (throbberCounter + 1) % (10 * ult::THROBBER_SYMBOLS.size());
                renderer->drawString(throbberSymbol, false, xPosition, yPosition, fontSize, textColor);
            }
        
        #if IS_LAUNCHER_DIRECTIVE
            s64 determineKeyOnTouchRelease(bool useScriptKey) const {
                const u64 touchDuration_ns = armTicksToNs(armGetSystemTick()) - m_touchStartTime_ns;
                const float touchDurationInSeconds = static_cast<float>(touchDuration_ns) * 1e-9f; // More efficient than division
                
                if (touchDurationInSeconds >= 1.0f) [[unlikely]] {
                    return useScriptKey ? SCRIPT_KEY : STAR_KEY;
                }
                if (touchDurationInSeconds >= 0.3f) [[unlikely]] {
                    return useScriptKey ? SCRIPT_KEY : SETTINGS_KEY;
                }
                return KEY_A;
            }
        #endif
        
            void resetTextProperties() {
                m_scrollText.clear();
                m_ellipsisText.clear();
                m_maxWidth = 0;
            }
        };
        
        
        class MiniListItem : public ListItem {
        public:
            // Constructor for MiniListItem, with no `isMini` boolean.
            MiniListItem(const std::string& text, const std::string& value = "", bool useScriptKey = false)
                : ListItem(text, value, useScriptKey)  // Call the parent constructor with `isMini = true`
            {
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
                s32 yOffset = (tsl::style::ListItemDefaultHeight - this->m_listItemHeight) / 2;
        
                if (this->m_maxWidth == 0) {
                    if (this->m_value.length() > 0) {
                        //std::tie(width, height) = renderer->drawString(this->m_value, false, 0, 0, 20, a(tsl::style::color::ColorTransparent));
                        width = tsl::gfx::calculateStringWidth(this->m_value, 20);
                        this->m_maxWidth = this->getWidth() - width - 70 +4;
                    } else {
                        this->m_maxWidth = this->getWidth() - 40 -10 -12;
                    }
                    
                    //std::tie(width, height) = renderer->drawString(this->m_text, false, 0, 0, 23, a(tsl::style::color::ColorTransparent));
                    width = tsl::gfx::calculateStringWidth(this->m_text, 23);
                    this->m_trunctuated = width > this->m_maxWidth+20;
                    
                    if (this->m_trunctuated) {
                        this->m_scrollText = this->m_text + "        ";
                        //std::tie(width, height) = renderer->drawString(this->m_scrollText, false, 0, 0, 23, a(tsl::style::color::ColorTransparent));
                        width = tsl::gfx::calculateStringWidth(this->m_scrollText, 23);
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
                        renderer->drawString(this->m_scrollText, false, this->getX() + 20-1 - this->m_scrollOffset, this->getY() + 45 - yOffset, 23, a(selectedTextColor));
                        renderer->disableScissoring();
                        
                        // Handle scrolling with nanosecond timing
                        u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                        u64 elapsed_ns = currentTime_ns - this->timeIn_ns;
                        
                        // 2000ms = 2,000,000,000 nanoseconds
                        if (elapsed_ns >= 2000000000ULL) {
                            if (this->m_scrollOffset >= this->m_textWidth) {
                                this->m_scrollOffset = 0;
                                this->timeIn_ns = currentTime_ns;
                            } else {
                                // Calculate the increment based on the desired scroll rate
                                u64 scroll_elapsed_ns = elapsed_ns - 2000000000ULL;
                                u64 scroll_elapsed_ms = scroll_elapsed_ns / 1000000ULL;
                                this->m_scrollOffset = 0.1f * scroll_elapsed_ms;
                            }
                        }
                    } else {
                        renderer->drawString(this->m_ellipsisText, false, this->getX() + 20-1, this->getY() + 45 - yOffset, 23, a(!useClickTextColor ? defaultTextColor : clickTextColor));
                    }
                } else {
                    // Render the text with special character handling
                    renderer->drawStringWithColoredSections(this->m_text, {ult::STAR_SYMBOL+"  "}, this->getX() + 20-1, this->getY() + 45 - yOffset, 23,
                        a(this->m_focused ? (!useClickTextColor ? selectedTextColor : clickTextColor) : (!useClickTextColor ? defaultTextColor : clickTextColor)),
                        a(this->m_focused ? starColor : selectionStarColor)
                    );
                }
                
                
                // CUSTOM SECTION START (modification for submenu footer color)
                //const std::string& value = this->m_value;
                s32 xPosition = this->getX() + this->m_maxWidth + 44 + 3;
                s32 yPosition = this->getY() + 45 - yOffset;
                s32 fontSize = 20;
                //bool isFaint = ;
                //bool isFocused = this->m_focused;
        
        
                //static bool lastRunningInterpreter = ult::runningInterpreter.load(std::memory_order_acquire);
        
                // Determine text color
                auto textColor = this->m_faint ? a(m_faintColor) : a(m_valueColor);
                
        
                if (this->m_value != ult::INPROGRESS_SYMBOL) {
                    // Draw the string with the determined text color
                    renderer->drawString(this->m_value, false, xPosition, yPosition, fontSize, textColor);
                    // CUSTOM SECTION END 
                } else {
                    // Static counter for the throbber symbols
                    static size_t throbberCounter = 0;
                    // Avoid copying the original string
                    const std::string* stringPtr = &this->m_value;
        
                    //size_t index = (throbberCounter / 10) % ult::THROBBER_SYMBOLS.size();  // Change index every 10 counts
                    stringPtr = &ult::THROBBER_SYMBOLS[(throbberCounter / 10) % ult::THROBBER_SYMBOLS.size()];  // Point to the new string without copying
                    
                    throbberCounter++;
        
                    // Reset counter to prevent overflow after many cycles (ensures it never grows indefinitely)
                    if (throbberCounter >= 10 * ult::THROBBER_SYMBOLS.size()) {
                        throbberCounter = 0;
                    }
                    renderer->drawString(*stringPtr, false, xPosition, yPosition, fontSize, textColor);
                }
                //lastRunningInterpreter = ult::runningInterpreter.load(std::memory_order_acquire);
            }
        
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX()+2+1, this->getY(), this->getWidth()+8+1, m_listItemHeight);
            }
        
            virtual bool onClick(u64 keys) override {
                if (ult::simulatedSelect && !ult::simulatedSelectComplete) {
                    keys |= KEY_A;
                    ult::simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    this->triggerClickAnimation();
                    ult::simulatedSelectComplete = true;
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
                        bool handled = this->onClick(HidNpadButton_A);
        
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
            std::string m_value = "";
            std::string m_scrollText = "";
            std::string m_ellipsisText = "";
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
            ToggleListItem(const std::string& text, bool initialState, const std::string& onValue = ult::ON, const std::string& offValue = ult::OFF, bool isMini = false)
                : ListItem(text, "", isMini), m_state(initialState), m_onValue(onValue), m_offValue(offValue) {
                this->setState(this->m_state);
            }
            
            virtual ~ToggleListItem() {}
            
            virtual bool onClick(u64 keys) override {
                if (ult::simulatedSelect && !ult::simulatedSelectComplete) {
                    keys |= KEY_A;
                    ult::simulatedSelect = false;
                }
                // Handle KEY_A for toggling
                if (keys & HidNpadButton_A) {
                    this->m_state = !this->m_state;
                    
                    this->setState(this->m_state);
                    this->m_stateChangedListener(this->m_state);
                    
                    ult::simulatedSelectComplete = true;
                    return ListItem::onClick(keys);
                }
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
            
            
            CategoryHeader(const std::string &title, bool hasSeparator = true) : m_text(title), m_hasSeparator(hasSeparator) {
                ult::applyLangReplacements(m_text);
                ult::convertComboToUnicode(m_text);
                m_isItem = false;
                m_isTable = true;
            }
            virtual ~CategoryHeader() {}
            
            virtual void draw(gfx::Renderer *renderer) override {
                if (this->m_hasSeparator) {
                    renderer->drawRect(this->getX()+1+1, this->getBottomBound() - 29, 4, 22, a(headerSeparatorColor));
                    renderer->drawString(this->m_text, false, this->getX() + 15+1, this->getBottomBound() - 12, 16, a(headerTextColor));
                } else {
                    renderer->drawString(this->m_text, false, this->getX(), this->getBottomBound() - 12, 16, a(headerTextColor));
                }
                //if (this->m_hasSeparator)
                //    renderer->drawRect(this->getX(), this->getBottomBound(), this->getWidth(), 1, tsl::style::color::ColorFrame); // CUSTOM MODIFICATION
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                // Check if the CategoryHeader is part of a list and if it's the first entry in it, half it's height
                if (List *list = static_cast<List*>(this->getParent()); list != nullptr) {
                    if (list->getIndexInList(this) == 0) {
                        this->setBoundaries(this->getX(), this->getY()-4, this->getWidth(), tsl::style::ListItemDefaultHeight / 2);
                        return;
                    }
                }
                this->setBoundaries(this->getX(), this->getY()-4, this->getWidth(), tsl::style::ListItemDefaultHeight *0.90);
                //if (m_hasSeparator) { // CUSTOM MODIFICATION
                //    this->setBoundaries(this->getX(), this->getY()-4, this->getWidth(), tsl::style::ListItemDefaultHeight *0.90); // CUSTOM MODIFICATION
                //} else {
                //    this->setBoundaries(this->getX(), this->getY()-4, this->getWidth(), tsl::style::ListItemDefaultHeight / 2); // CUSTOM MODIFICATION
                //}
            }
            
            virtual bool onClick(u64 keys) {
                return false;
            }
            
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
                return nullptr;
            }
            
            inline void setText(const std::string &text) {
                this->m_text = text;
                ult::applyLangReplacements(m_text);
            }
            
            inline const std::string& getText() const {
                return this->m_text;
            }
            
        private:
            std::string m_text;
            bool m_hasSeparator;
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
             */
            TrackBar(const char icon[3], bool usingStepTrackbar=false, bool usingNamedStepTrackbar = false) : m_icon(icon), m_usingStepTrackbar(usingStepTrackbar), m_usingNamedStepTrackbar(usingNamedStepTrackbar) {
                m_isItem = true;
            }

            virtual ~TrackBar() {}

            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) {
                return this;
            }

            virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                if (keysHeld & HidNpadButton_AnyLeft && keysHeld & HidNpadButton_AnyRight)
                    return true;

                if (keysHeld & HidNpadButton_AnyLeft) {
                    if (this->m_value > 0) {
                        this->m_value--;
                        this->m_valueChangedListener(this->m_value);
                        return true;
                    }
                }

                if (keysHeld & HidNpadButton_AnyRight) {
                    if (this->m_value < 100) {
                        this->m_value++;
                        this->m_valueChangedListener(this->m_value);
                        return true;
                    }
                }

                return false;
            }

            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                u16 trackBarWidth = this->getWidth() - 95;
                u16 handlePos = (trackBarWidth * (this->m_value - 0)) / (100 - 0);
                s32 circleCenterX = this->getX() + 59 + handlePos;
                s32 circleCenterY = this->getY() + 40 + 16 - 1;
                s32 circleRadius = 16;
                
                bool touchInCircle = (std::abs(initialX - circleCenterX) <= circleRadius) && (std::abs(initialY - circleCenterY) <= circleRadius);
                

                if (event == TouchEvent::Release) {
                    this->m_interactionLocked = false;
                    touchInSliderBounds = false;
                    return false;
                }

                if (!this->m_interactionLocked && (touchInCircle || touchInSliderBounds)) {
                    touchInSliderBounds = true;
                    if (currX > this->getLeftBound() + 50 && currX < this->getRightBound() && currY > this->getTopBound() && currY < this->getBottomBound()) {
                        s16 newValue = (static_cast<float>(currX - (this->getX() + 60)) / static_cast<float>(this->getWidth() - 95)) * 100;

                        if (newValue < 0) {
                            newValue = 0;
                        } else if (newValue > 100) {
                            newValue = 100;
                        }

                        if (newValue != this->m_value) {
                            this->m_value = newValue;
                            this->m_valueChangedListener(this->getProgress());
                        }

                        return true;
                    }
                }
                else
                    this->m_interactionLocked = true;

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
                static float lastBottomBound;
                
                s32 xPos = this->getX() + 59;
                s32 yPos = this->getY() + 40 + 16 - 1;
                s32 width = this->getWidth() - 95;
                u16 handlePos = width * (this->m_value) / (100);

                if (!m_usingNamedStepTrackbar) {
                    yPos -= 11;
                }

                s32 iconOffset;

                if (m_icon[0] != '\0') {
                    s32 iconWidth = 23;//tsl::gfx::calculateStringWidth(m_icon, 23);
                    iconOffset = 14 + iconWidth;
                    xPos += iconOffset;
                    width -= iconOffset;
                    handlePos = (width) * (this->m_value) / (100);
                }

                // Draw track bar background
                drawBar(renderer, xPos, yPos-3, width, trackBarEmptyColor, !m_usingNamedStepTrackbar);

                if (!this->m_focused) {
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + handlePos, yPos, 16, true, a(trackBarSliderBorderColor));
                    renderer->drawCircle(xPos + handlePos, yPos, 13, true, a((m_unlockedTrackbar || touchInSliderBounds) ? trackBarSliderMalleableColor : trackBarSliderColor));
                } else {
                    touchInSliderBounds = false;
                    ult::unlockedSlide = m_unlockedTrackbar;
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 16, true, a(highlightColor));
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 12, true, a((ult::allowSlide || m_unlockedTrackbar) ? trackBarSliderMalleableColor : trackBarSliderColor));
                }


                //renderer->drawRect(this->getX(), this->getY(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));
                //renderer->drawRect(this->getX(), this->getBottomBound(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));
                if (m_icon[0] != '\0')
                    renderer->drawString(this->m_icon, false, this->getX()+42, this->getY() + 50+2, 23, a(tsl::style::color::ColorText));

                //u16 handlePos = (this->getWidth() - 95) * static_cast<float>(this->m_value) / 100;
                //renderer->drawCircle(this->getX() + 60, this->getY() + 42, 2, true, a(tsl::style::color::ColorHighlight));
                //renderer->drawCircle(this->getX() + 60 + this->getWidth() - 95, this->getY() + 42, 2, true, a(tsl::style::color::ColorFrame));
                //renderer->drawRect(this->getX() + 60 + handlePos, this->getY() + 40, this->getWidth() - 95 - handlePos, 5, a(tsl::style::color::ColorFrame));
                //renderer->drawRect(this->getX() + 60, this->getY() + 40, handlePos, 5, a(tsl::style::color::ColorHighlight));
                //
                //renderer->drawCircle(this->getX() + 62 + handlePos, this->getY() + 42, 18, true, a(tsl::style::color::ColorHandle));
                //renderer->drawCircle(this->getX() + 62 + handlePos, this->getY() + 42, 18, false, a(tsl::style::color::ColorFrame));


                if (lastBottomBound != this->getTopBound())
                    renderer->drawRect(this->getX() + 4+20-1, this->getTopBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                renderer->drawRect(this->getX() + 4+20-1, this->getBottomBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                lastBottomBound = this->getBottomBound();
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX() - 16 , this->getY(), this->getWidth()+20+4, tsl::style::TrackBarDefaultHeight );
            }

            virtual void drawFocusBackground(gfx::Renderer *renderer) {
                // No background drawn here in HOS
            }

            virtual void drawHighlight(gfx::Renderer *renderer) override {
                
                //progress = (std::cos(2.0 * ult::_M_PI * std::fmod(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(), 1.0) - ult::_M_PI / 2) + 1.0) / 2.0;
                // Get current time using ARM system tick for animation timing
                u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                double timeInSeconds = static_cast<double>(currentTime_ns) / 1000000000.0; // Convert nanoseconds to seconds
                
                progress = (std::cos(2.0 * ult::_M_PI * std::fmod(timeInSeconds, 1.0) - ult::_M_PI / 2) + 1.0) / 2.0;

                //if (ult::allowSlide || m_unlockedTrackbar) {
                //    highlightColor = {
                //        static_cast<u8>((highlightColor3.r - highlightColor4.r) * progress + highlightColor4.r),
                //        static_cast<u8>((highlightColor3.g - highlightColor4.g) * progress + highlightColor4.g),
                //        static_cast<u8>((highlightColor3.b - highlightColor4.b) * progress + highlightColor4.b),
                //        0xF
                //    };
                //} else {
                //    highlightColor = {
                //        static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress + highlightColor2.r),
                //        static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress + highlightColor2.g),
                //        static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress + highlightColor2.b),
                //        0xF
                //    };
                //}
                highlightColor = {
                    static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress + highlightColor2.r),
                    static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress + highlightColor2.g),
                    static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress + highlightColor2.b),
                    0xF
                };
                
                //u16 handlePos = (this->getWidth() - 95) * (this->m_value - m_minValue) / (m_maxValue - m_minValue);
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime; // Changed
                    if (t_ns >= 100000000) // 100ms in nanoseconds
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t_ns, amplitude); // Changed parameter
                                break;
                            default:
                                break;
                        }
                        
                        x = std::clamp(x, -amplitude, amplitude);
                        y = std::clamp(y, -amplitude, amplitude);
                    }
                }

                if (!disableSelectionBG)
                    renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), a(selectionBGColor)); // CUSTOM MODIFICATION 

                renderer->drawBorderedRoundedRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11, this->getHeight(), 5, 5, a(highlightColor));

                //if (this->m_clickAnimationProgress == 0) {
                //    renderer->drawRect(this->getX() + 60, this->getY() + 40 + 16+2, handlePos, 5, a(tsl::style::color::ColorHighlight));
                //    renderer->drawCircle(this->getX() + 62 + x + handlePos, this->getY() + 42 + y + 16+2, 16, true, a(highlightColor));
                //    renderer->drawCircle(this->getX() + 62 + x + handlePos, this->getY() + 42 + y + 16+2, 11, true, a(trackBarColor));
                //}

                ult::onTrackBar = true;
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
             */
            StepTrackBar(const char icon[3], size_t numSteps, bool usingNamedStepTrackbar = false)
                : TrackBar(icon, true, usingNamedStepTrackbar), m_numSteps(numSteps) { }

            virtual ~StepTrackBar() {}

            virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                static u32 tick = 0;

                if (keysHeld & HidNpadButton_AnyLeft && keysHeld & HidNpadButton_AnyRight) {
                    tick = 0;
                    return true;
                }

                if (keysHeld & (HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
                    if ((tick == 0 || tick > 20) && (tick % 3) == 0) {
                        if (keysHeld & HidNpadButton_AnyLeft && this->m_value > 0) {
                            this->m_value = std::max(this->m_value - (100 / (this->m_numSteps - 1)), 0);
                        } else if (keysHeld & HidNpadButton_AnyRight && this->m_value < 100) {
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
                if (this->inBounds(initialX, initialY)) {
                    if (currY > this->getTopBound() && currY < this->getBottomBound()) {
                        s16 newValue = (static_cast<float>(currX - (this->getX() + 60)) / static_cast<float>(this->getWidth() - 95)) * 100;

                        if (newValue < 0) {
                            newValue = 0;
                        } else if (newValue > 100) {
                            newValue = 100;
                        } else {
                            newValue = std::round(newValue / (100.0F / (this->m_numSteps - 1))) * (100.0F / (this->m_numSteps - 1));
                        }

                        if (newValue != this->m_value) {
                            this->m_value = newValue;
                            this->m_valueChangedListener(this->getProgress());
                        }

                        return true;
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
             */
            NamedStepTrackBar(const char icon[3], std::initializer_list<std::string> stepDescriptions)
                : StepTrackBar(icon, stepDescriptions.size(), true), m_stepDescriptions(stepDescriptions.begin(), stepDescriptions.end()) {
                    this->m_usingNamedStepTrackbar = true;
                }

            virtual ~NamedStepTrackBar() {}

            virtual void draw(gfx::Renderer *renderer) override {
                // TrackBar width excluding the handle areas
                u16 trackBarWidth = this->getWidth() - 95;
                
                // Base X and Y coordinates
                u16 baseX = this->getX() + 59;
                u16 baseY = this->getY() + 44; // 50 - 3
                
                s32 iconOffset;
                
                if (m_icon[0] != '\0') {
                    s32 iconWidth = 23;//tsl::gfx::calculateStringWidth(m_icon, 23);
                    iconOffset = 14 + iconWidth;
                    baseX += iconOffset;
                    trackBarWidth -= iconOffset;
                }



                // Calculate the spacing between each step
                float stepSpacing = static_cast<float>(trackBarWidth) / (this->m_numSteps - 1);
                
                // Calculate the halfway point index
                u8 halfNumSteps = (this->m_numSteps - 1) / 2;

                // Draw step rectangles
                u16 stepX;
                for (u8 i = 0; i < this->m_numSteps; i++) {
                    stepX = baseX + std::round(i * stepSpacing);
                    
                    // Subtract 1 from the X position for steps on the right side of the center
                    if (i > halfNumSteps) {
                        stepX -= 1;
                    }

                    // Adjust the last step to avoid overshooting
                    if (i == this->m_numSteps - 1) {
                        stepX = baseX + trackBarWidth -1;
                    }
            
                    renderer->drawRect(stepX, baseY, 1, 8, a(trackBarEmptyColor));
                }
                
                u8 currentDescIndex = std::clamp(this->m_value / (100 / (this->m_numSteps - 1)), 0, this->m_numSteps - 1);

                auto descWidth = tsl::gfx::calculateStringWidth(this->m_stepDescriptions[currentDescIndex].c_str(), 15);
                renderer->drawString(this->m_stepDescriptions[currentDescIndex].c_str(), false, ((baseX +1) + (trackBarWidth) / 2) - (descWidth / 2), this->getY() + 20 + 6, 15, a(tsl::style::color::ColorDescription));
                
                // Draw the parent trackbar
                StepTrackBar::draw(renderer);





                //u16 trackBarWidth = this->getWidth() - 95;
                //u16 stepWidth = trackBarWidth / (this->m_numSteps - 1);
                //
                //for (u8 i = 0; i < this->m_numSteps; i++) {
                //    renderer->drawRect(this->getX() + 60 + stepWidth * i, this->getY() + 50, 1, 10, a(tsl::style::color::ColorFrame));
                //}
                //
                //u8 currentDescIndex = std::clamp(this->m_value / (100 / (this->m_numSteps - 1)), 0, this->m_numSteps - 1);
                //
                //auto [descWidth, descHeight] = renderer->drawString(this->m_stepDescriptions[currentDescIndex].c_str(), false, 0, 0, 15, tsl::style::color::ColorTransparent);
                //renderer->drawString(this->m_stepDescriptions[currentDescIndex].c_str(), false, ((this->getX() + 60) + (this->getWidth() - 95) / 2) - (descWidth / 2), this->getY() + 20, 15, a(tsl::style::color::ColorDescription));
                //
                //StepTrackBar::draw(renderer);
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
            u64 lastUpdate_ns;
        
            Color highlightColor = {0xf, 0xf, 0xf, 0xf};
            float progress;
            float counter = 0.0;
            s32 x, y;
            s32 amplitude;
            u32 descWidth, descHeight;
            
            // Add setScriptKeyListener function
            void setScriptKeyListener(std::function<void()> listener) {
                m_scriptKeyListener = std::move(listener);
            }
        
            
            // Ensure the order of initialization matches the order of declaration
            TrackBarV2(std::string label, std::string packagePath = "", s16 minValue = 0, s16 maxValue = 100, std::string units = "",
                     std::function<bool(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
                     std::function<std::vector<std::vector<std::string>>(const std::vector<std::vector<std::string>>&, const std::string&, size_t, const std::string&)> sourceReplacementFunc = nullptr,
                     std::vector<std::vector<std::string>> cmd = {}, const std::string& selCmd = "", bool usingStepTrackbar = false, bool usingNamedStepTrackbar = false, s16 numSteps = -1, bool unlockedTrackbar = false, bool executeOnEveryTick = false)
                : m_label(label), m_packagePath(packagePath), m_minValue(minValue), m_maxValue(maxValue), m_units(units),
                  interpretAndExecuteCommands(executeCommands), getSourceReplacement(sourceReplacementFunc), commands(std::move(cmd)), selectedCommand(selCmd), m_usingStepTrackbar(usingStepTrackbar), m_usingNamedStepTrackbar(usingNamedStepTrackbar), m_numSteps(numSteps), m_unlockedTrackbar(unlockedTrackbar), m_executeOnEveryTick(executeOnEveryTick) {
                m_isItem = true;
        
                if ((!usingStepTrackbar && !usingNamedStepTrackbar) || numSteps == -1) {
                    m_numSteps = (maxValue - minValue)+1;
                }
        
                bool loadedValue = false;
                if (!m_packagePath.empty()) {
                    std::string initialIndex = ult::parseValueFromIniSection(m_packagePath + "config.ini", m_label, "index");
        
                    if (!initialIndex.empty()) {
                        m_index = static_cast<s16>(ult::stoi(initialIndex)); // convert initializedValue to s16
                    }
                    if (!m_usingNamedStepTrackbar) {
                        std::string initialValue = ult::parseValueFromIniSection(m_packagePath + "config.ini", m_label, "value");
                        
                        if (!initialValue.empty()) {
                            m_value = static_cast<s16>(ult::stoi(initialValue)); // convert initializedValue to s16
                            loadedValue = true;
                        }
                    }
                }
        
                if (m_index > m_numSteps -1) m_index = m_numSteps - 1;
                else if (m_index < 0) m_index = 0;
        
                if (!loadedValue)
                    m_value = minValue + m_index * (static_cast<float>(maxValue - minValue) / (m_numSteps - 1));
        
                if (m_value > maxValue) m_value = maxValue;
                else if (m_value < minValue) m_value = minValue;
        
                lastUpdate_ns = armTicksToNs(armGetSystemTick());
            }
            
            virtual ~TrackBarV2() {}
            
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) {
                return this;
            }
        
        
            inline void updateAndExecute(bool updateIni = true) {
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
        
                size_t pos;
                size_t tryCount = 0;
                while (!success) {
                    if (interpretAndExecuteCommands) {
                        if (tryCount > 3)
                            break;
                        auto modifiedCmds = getSourceReplacement(commands, valueStr, m_index, m_packagePath);
                        
                        // Placeholder replacement
                        const std::string valuePlaceholder = "{value}";
                        const std::string indexPlaceholder = "{index}";
                        const size_t valuePlaceholderLength = valuePlaceholder.length();
                        const size_t indexPlaceholderLength = indexPlaceholder.length();
                        
                        
                        for (auto& cmd : modifiedCmds) {
                            for (auto& arg : cmd) {
                                pos = 0;
                                while ((pos = arg.find(valuePlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, valuePlaceholderLength, valueStr);
                                    pos += valueStr.length();
                                }
                                
                                if (m_usingNamedStepTrackbar) {
                                    pos = 0;
                                    while ((pos = arg.find(indexPlaceholder, pos)) != std::string::npos) {
                                        arg.replace(pos, indexPlaceholderLength, indexStr);
                                        pos += indexStr.length();
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
                static bool holding = false;
                static u64 holdStartTime_ns;
                static u64 prevKeysHeld = 0;
                
                u64 keysReleased = prevKeysHeld & ~keysHeld;
                prevKeysHeld = keysHeld;
                
                u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                u64 elapsed_ns = currentTime_ns - lastUpdate_ns;
                
                static bool wasLastHeld = false;
        
                if ((keysHeld & KEY_R)) {
                    return true;
                }
        
                if (ult::simulatedSelect && !ult::simulatedSelectComplete) {
                    keysDown |= KEY_A;
                    ult::simulatedSelect = false;
                }
        
                // Check if KEY_A is pressed to toggle ult::allowSlide
                if (keysDown & KEY_A) {
                    if (!m_unlockedTrackbar) {
                        ult::allowSlide = !ult::allowSlide;
                        holding = false; // Reset holding state when KEY_A is pressed
                    }
                    if (m_unlockedTrackbar || (!m_unlockedTrackbar && !ult::allowSlide)) {
                        updateAndExecute();
                        triggerClick = true;
                    }
                    ult::simulatedSelectComplete = true;
                    return true;
                }
        
                // Handle SCRIPT_KEY press
                if (keysDown & SCRIPT_KEY) {
                    if (m_scriptKeyListener) {
                        m_scriptKeyListener();
                    }
                    return true;
                }
                
        
                // Allow sliding only if KEY_A has been pressed
                if (ult::allowSlide || m_unlockedTrackbar) {
        
                    if (((keysReleased & HidNpadButton_AnyLeft) || (keysReleased & HidNpadButton_AnyRight)) ||
                        (wasLastHeld && !((keysHeld & HidNpadButton_AnyLeft) || (keysHeld & HidNpadButton_AnyRight)))) {
        
                        wasLastHeld = false;
                        updateAndExecute();
                        lastUpdate_ns = armTicksToNs(armGetSystemTick()); 
        
                        holding = false;
                        return true;
                    }
                    
                    if (keysDown & HidNpadButton_AnyLeft && keysDown & HidNpadButton_AnyRight)
                        return true;
        
                    if (keysDown & HidNpadButton_AnyLeft) {
                        if (this->m_value > m_minValue) {
                            this->m_index--;
                            this->m_value--;
                            this->m_valueChangedListener(this->m_value);
                            updateAndExecute(false);
                            lastUpdate_ns = armTicksToNs(armGetSystemTick());
                            return true;
                        }
                    }
                    
                    if (keysDown & HidNpadButton_AnyRight) {
                        if (this->m_value < m_maxValue) {
                            this->m_index++;
                            this->m_value++;
                            this->m_valueChangedListener(this->m_value);
                            updateAndExecute(false);
                            lastUpdate_ns = armTicksToNs(armGetSystemTick());
                            return true;
                        }
                    }
        
        
        
                    if (keysHeld & HidNpadButton_AnyLeft && keysHeld & HidNpadButton_AnyRight)
                        return true;
                    
                    // Check if the button is being held down
                    if (((keysHeld & HidNpadButton_AnyLeft) || (keysHeld & HidNpadButton_AnyRight))) {
                        
                        if (!holding) {
                            holding = true;
                            holdStartTime_ns = armTicksToNs(armGetSystemTick());
                        }
                        
                        u64 holdDuration_ns = currentTime_ns - holdStartTime_ns;
        
                        // Define the duration boundaries in nanoseconds
                        const u64 initialInterval_ns = 67000000ULL;    // 67ms in nanoseconds
                        const u64 shortInterval_ns = 10000000ULL;      // 10ms in nanoseconds
                        const u64 transitionPoint_ns = 2000000000ULL;  // 2000ms in nanoseconds
                        
                        // Calculate transition factor (t) from 0 to 1 based on how far we are from the transition point
                        float t = std::min(1.0f, static_cast<float>(holdDuration_ns) / static_cast<float>(transitionPoint_ns));
                        
                        // Smooth transition between intervals
                        u64 currentInterval_ns = static_cast<u64>((initialInterval_ns - shortInterval_ns) * (1.0f - t) + shortInterval_ns);
                        
                        if (elapsed_ns >= currentInterval_ns) {
                            if (keysHeld & HidNpadButton_AnyLeft) {
                                if (this->m_value > m_minValue) {
                                    this->m_index--;
                                    this->m_value--;
                                    this->m_valueChangedListener(this->m_value);
                                    if (m_executeOnEveryTick) {
                                        updateAndExecute(false);
                                    }
                                    lastUpdate_ns = armTicksToNs(armGetSystemTick());
                                    wasLastHeld = true;
                                    return true;
                                }
                            }
                            
                            if (keysHeld & HidNpadButton_AnyRight) {
                                if (this->m_value < m_maxValue) {
                                    this->m_index++;
                                    this->m_value++;
                                    this->m_valueChangedListener(this->m_value);
                                    if (m_executeOnEveryTick) {
                                        updateAndExecute(false);
                                    }
                                    lastUpdate_ns = armTicksToNs(armGetSystemTick());
                                    wasLastHeld = true;
                                    return true;
                                }
                            }
                        }
                    } else {
                        holding = false; // Reset holding state if no relevant key is held
                    }
                }
                
                return false;
            }
            
            
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                u16 trackBarWidth = this->getWidth() - 95;
                u16 handlePos = (trackBarWidth * (this->m_value - m_minValue)) / (m_maxValue - m_minValue);
                s32 circleCenterX = this->getX() + 59 + handlePos;
                s32 circleCenterY = this->getY() + 40 + 16 - 1;
                s32 circleRadius = 16;
                
                bool touchInCircle = (std::abs(initialX - circleCenterX) <= circleRadius) && (std::abs(initialY - circleCenterY) <= circleRadius);
                
                if (!m_unlockedTrackbar && !ult::allowSlide)
                    return false;
        
                if ((touchInCircle || touchInSliderBounds)) {
        
                    touchInSliderBounds = true;
                    
                    s16 newIndex = static_cast<s16>((currX - (this->getX() + 59)) / static_cast<float>(this->getWidth() - 95) * (m_numSteps - 1));
                    
                    // Clamp the index within valid range
                    newIndex = std::max(static_cast<s16>(0), std::min(newIndex, static_cast<s16>(m_numSteps - 1)));
                    
                    s16 newValue = m_minValue + newIndex * (static_cast<float>(m_maxValue - m_minValue) / (m_numSteps - 1));
                    
                    if (newValue != this->m_value || newIndex != this->m_index) {
                        this->m_value = newValue;
                        this->m_index = newIndex;
                        this->m_valueChangedListener(this->getProgress());
                        if (m_executeOnEveryTick) {
                            updateAndExecute(false);
                        }
                    } else {
                        if (event == TouchEvent::Release) {
                            updateAndExecute();
                            if (event == TouchEvent::Release)
                                touchInSliderBounds = false;
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
                static float lastBottomBound;
                u16 handlePos = (this->getWidth() - 95) * (this->m_value - m_minValue) / (m_maxValue - m_minValue);
                s32 xPos = this->getX() + 59;
                s32 yPos = this->getY() + 40 + 16 - 1;
                s32 width = this->getWidth() - 95;
        
        
                // Draw track bar background
                drawBar(renderer, xPos, yPos-3, width, trackBarEmptyColor, !m_usingNamedStepTrackbar);
                
                
                if (!this->m_focused) {
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + handlePos, yPos, 16, true, a(trackBarSliderBorderColor));
                    renderer->drawCircle(xPos + handlePos, yPos, 13, true, a((m_unlockedTrackbar || touchInSliderBounds) ? trackBarSliderMalleableColor : trackBarSliderColor));
                } else {
                    touchInSliderBounds = false;
                    ult::unlockedSlide = m_unlockedTrackbar;
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 16, true, a(highlightColor));
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 12, true, a((ult::allowSlide || m_unlockedTrackbar) ? trackBarSliderMalleableColor : trackBarSliderColor));
                }
                 
                std::string labelPart = this->m_label;
                ult::removeTag(labelPart);
        
                std::string valuePart;
                if (!m_usingNamedStepTrackbar)
                    valuePart = (this->m_units == "%" || this->m_units == "°C" || this->m_units == "°F") ? ult::to_string(this->m_value) + this->m_units : ult::to_string(this->m_value) + (this->m_units.empty() ? "" : " ") + this->m_units;
                else
                    valuePart = this->m_selection;
        
                size_t valueWidth = tsl::gfx::calculateStringWidth(valuePart, 16);
        
                renderer->drawString(labelPart, false, xPos, this->getY() + 14 + 16, 16, (!this->m_focused ? a(defaultTextColor) : a(selectedTextColor)));
                renderer->drawString(valuePart, false, this->getWidth() -17 - valueWidth, this->getY() + 14 + 16, 16, a(onTextColor));
        
        
                if (lastBottomBound != this->getTopBound())
                    renderer->drawRect(this->getX() + 4+20-1, this->getTopBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                renderer->drawRect(this->getX() + 4+20-1, this->getBottomBound(), this->getWidth() + 6 + 10+20 +4, 1, a(separatorColor));
                lastBottomBound = this->getBottomBound();
            }
        
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX() - 16 , this->getY(), this->getWidth()+20+4, tsl::style::TrackBarDefaultHeight );
            }
            
            virtual void drawFocusBackground(gfx::Renderer *renderer) {
                // No background drawn here in HOS
            }
            
            virtual void drawHighlight(gfx::Renderer *renderer) override {
                
                // Calculate progress using ARM ticks instead of chrono
                u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                double timeInSeconds = static_cast<double>(currentTime_ns) / 1000000000.0;
                progress = ((std::cos(2.0 * ult::_M_PI * std::fmod(timeInSeconds, 1.0) - ult::_M_PI / 2) + 1.0) / 2.0);
                
                static u64 clickStartTime_ns;
                static bool clickActive = false;
                
                Color clickColor1 = highlightColor1;
                Color clickColor2 = clickColor;
        
                // Activate `clickStartTime_ns` when `triggerClick` is set to true
                if (triggerClick && !clickActive) {
                    clickStartTime_ns = armTicksToNs(armGetSystemTick());
                    clickActive = true;
                    // Within the cycle, perform the highlight effect
                    if (progress >= 0.5) {
                        clickColor1 = clickColor;
                        clickColor2 = highlightColor2;
                    }
                }
                static auto lastLabel = m_label;
        
                if (lastLabel != m_label) {
                    clickActive = false;
                    triggerClick = false;
                }
                lastLabel = m_label;
        
                if (clickActive) {
                    u64 elapsedTime_ns = currentTime_ns - clickStartTime_ns;
                    if (elapsedTime_ns < 500000000ULL) { // 500ms in nanoseconds
                        highlightColor = {
                            static_cast<u8>((clickColor1.r - clickColor2.r) * progress + clickColor2.r),
                            static_cast<u8>((clickColor1.g - clickColor2.g) * progress + clickColor2.g),
                            static_cast<u8>((clickColor1.b - clickColor2.b) * progress + clickColor2.b),
                            0xF
                        };
                    } else {
                        // End the effect after one cycle
                        clickActive = false;
                        triggerClick = false;
                    }
        
                } else {
        
                    if (ult::allowSlide || m_unlockedTrackbar) {
                        highlightColor = {
                            static_cast<u8>((highlightColor1.r - highlightColor2.r) * progress + highlightColor2.r),
                            static_cast<u8>((highlightColor1.g - highlightColor2.g) * progress + highlightColor2.g),
                            static_cast<u8>((highlightColor1.b - highlightColor2.b) * progress + highlightColor2.b),
                            0xF
                        };
                    } else {
                        highlightColor = {
                            static_cast<u8>((highlightColor3.r - highlightColor4.r) * progress + highlightColor4.r),
                            static_cast<u8>((highlightColor3.g - highlightColor4.g) * progress + highlightColor4.g),
                            static_cast<u8>((highlightColor3.b - highlightColor4.b) * progress + highlightColor4.b),
                            0xF
                        };
                    }
                }
                
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    t_ns = currentTime_ns - this->m_highlightShakingStartTime;
                    if (t_ns >= 100000000ULL) // 100ms in nanoseconds
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t_ns, amplitude);
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t_ns, amplitude);
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t_ns, amplitude);
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t_ns, amplitude);
                                break;
                            default:
                                break;
                        }
                        
                        x = std::clamp(x, -amplitude, amplitude);
                        y = std::clamp(y, -amplitude, amplitude);
                    }
                }
        
                if (!disableSelectionBG)
                    renderer->drawRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11-4, this->getHeight(), a(selectionBGColor)); // CUSTOM MODIFICATION 
        
                renderer->drawBorderedRoundedRect(this->getX() + x +19, this->getY() + y, this->getWidth()-11, this->getHeight(), 5, 5, a(highlightColor));
        
                ult::onTrackBar = true;
        
                if (clickActive) {
                    u64 elapsedTime_ns = currentTime_ns - clickStartTime_ns;
        
                    // Handle click animation progress and animColor rendering
                    auto clickAnimationProgress = tsl::style::ListItemHighlightLength * (1.0f - (static_cast<float>(elapsedTime_ns) / 500000000.0f)); // 500ms in nanoseconds
                    
                    // Ensure progress does not go below 0
                    if (clickAnimationProgress < 0.0f) {
                        clickAnimationProgress = 0.0f;
                    }
                
                    if (clickAnimationProgress > 0.0f) {
                        u8 saturation = tsl::style::ListItemHighlightSaturation * (float(clickAnimationProgress) / float(tsl::style::ListItemHighlightLength));
                
                        Color animColor = {0xF, 0xF, 0xF, 0xF};
                        if (invertBGClickColor) {
                            animColor.r = 15 - saturation;
                            animColor.g = 15 - saturation;
                            animColor.b = 15 - saturation;
                            animColor.a = 15 - saturation;
                        } else {
                            animColor.r = saturation;
                            animColor.g = saturation;
                            animColor.b = saturation;
                            animColor.a = saturation;
                        }
                        renderer->drawRect(this->getX() +22, this->getY(), this->getWidth() -22, this->getHeight(), a(animColor));
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
        
        protected:
            std::string m_label;
            std::string m_packagePath;
            std::string m_selection;
            s16 m_value = 0;
            s16 m_minValue;
            s16 m_maxValue;
            std::string m_units;
            bool m_interactionLocked = false;
            
            std::function<void(u8)> m_valueChangedListener = [](u8) {};
        
            // New member variables to store the function and its parameters
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
                u64 keysReleased = prevKeysHeld & ~keysHeld;
                prevKeysHeld = keysHeld;
                //static bool usingUnlockedTrackbar = m_unlockedTrackbar;

                static bool wasLastHeld = false;

                if ((keysHeld & KEY_R)) {
                    return true;
                }

                if (ult::simulatedSelect && !ult::simulatedSelectComplete) {
                    keysDown |= KEY_A;
                    ult::simulatedSelect = false;
                }

                // Check if KEY_A is pressed to toggle ult::allowSlide
                if ((keysDown & KEY_A)) {
                    if (!m_unlockedTrackbar) {
                        ult::allowSlide = !ult::allowSlide;
                        holding = false; // Reset holding state when KEY_A is pressed
                    //} else {
                    //    m_unlockedTrackbar = !m_unlockedTrackbar;
                    //    holding = false; // Reset holding state when KEY_A is pressed
                    }
                    if (m_unlockedTrackbar || (!m_unlockedTrackbar && !ult::allowSlide)) {
                        updateAndExecute();
                        triggerClick = true;
                    }
                    ult::simulatedSelectComplete = true;
                    return true;
                }

                // Handle SCRIPT_KEY press
                if (keysDown & SCRIPT_KEY) {
                    if (m_scriptKeyListener) {
                        m_scriptKeyListener();
                    }
                    return true;
                }

                if (ult::allowSlide || m_unlockedTrackbar) {
                    if (((keysReleased & HidNpadButton_AnyLeft) || (keysReleased & HidNpadButton_AnyRight)) ||
                        (wasLastHeld && !(keysHeld & (HidNpadButton_AnyLeft | HidNpadButton_AnyRight)))) {
                        //if (!m_executeOnEveryTick)
                        updateAndExecute();
                        holding = false;
                        wasLastHeld = false;
                        tick = 0;
                        return true;
                    }

                    //if ((keysDown & HidNpadButton_AnyLeft) || (keysDown & HidNpadButton_AnyRight)) {
                    //    tick = 0;
                    //}
                    
                    if (keysHeld & HidNpadButton_AnyLeft && keysHeld & HidNpadButton_AnyRight) {
                        tick = 0;
                        return true;
                    }
                    
                    if (keysHeld & (HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
                        if (!holding) {
                            holding = true;
                            tick = 0;
                        }
                        
                        if ((tick == 0 || tick > 20) && (tick % 3) == 0) {
                            float stepSize = static_cast<float>(m_maxValue - m_minValue) / (this->m_numSteps - 1);
                            if (keysHeld & HidNpadButton_AnyLeft && this->m_index > 0) {
                                this->m_index--;
                                this->m_value = static_cast<s16>(std::round(m_minValue + m_index * stepSize));
                                //this->m_value = static_cast<s16>(std::max(std::round(this->m_value - stepSize), static_cast<float>(m_minValue)));
                            } else if (keysHeld & HidNpadButton_AnyRight && this->m_index < this->m_numSteps-1) {
                                this->m_index++;
                                this->m_value = static_cast<s16>(std::round(m_minValue + m_index * stepSize));
                                //this->m_value = static_cast<s16>(std::min(std::round(this->m_value + stepSize), static_cast<float>(m_maxValue)));
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
                this->m_value = value * (100 / (this->m_numSteps - 1));
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
                }
            
            virtual ~NamedStepTrackBarV2() {}
                        
            virtual void draw(gfx::Renderer *renderer) override {
                // TrackBarV2 width excluding the handle areas
                u16 trackBarWidth = this->getWidth() - 95;
            
                // Base X and Y coordinates
                u16 baseX = this->getX() + 59;
                u16 baseY = this->getY() + 44; // 50 - 3
            
                // Calculate the spacing between each step
                float stepSpacing = static_cast<float>(trackBarWidth) / (this->m_numSteps - 1);
                
                // Calculate the halfway point index
                u8 halfNumSteps = (this->m_numSteps - 1) / 2;

                // Draw step rectangles
                u16 stepX;
                for (u8 i = 0; i < this->m_numSteps; i++) {
                    stepX = baseX + std::round(i * stepSpacing);
                    
                    // Subtract 1 from the X position for steps on the right side of the center
                    if (i > halfNumSteps) {
                        stepX -= 1;
                    }

                    // Adjust the last step to avoid overshooting
                    if (i == this->m_numSteps - 1) {
                        stepX = baseX + trackBarWidth -1;
                    }
            
                    renderer->drawRect(stepX, baseY, 1, 8, a(trackBarEmptyColor));
                }
            
                // Draw the current step description
                currentDescIndex = this->m_index;
                this->m_selection = this->m_stepDescriptions[currentDescIndex];
            
                // Draw the parent trackbar
                StepTrackBarV2::draw(renderer);
            }

            
        protected:
            std::vector<std::string> m_stepDescriptions;
            
        };
        
    }
    
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
            
            if (auto& currGui = this->getCurrentGui(); currGui != nullptr) // TESTING DISABLED (EFFECTS NEED TO BE VERIFIED)
                currGui->restoreFocus();
        }
        
        /**
         * @brief Hides the Gui
         *
         */
        void hide() {
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
        void close() {
            this->m_shouldClose = true;
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
        const int MAX_ANIMATION_COUNTER = 5; // Define the maximum animation counter value

        bool m_shouldHide = false;
        bool m_shouldClose = false;
        
        bool m_disableNextAnimation = false;
        
        bool m_closeOnExit;
        
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
            float opacity = calculateEaseInOut(static_cast<float>(this->m_animationCounter) / MAX_ANIMATION_COUNTER);
            gfx::Renderer::setOpacity(opacity);
        }


        
        /**
         * @brief Main loop
         *
         */
        void loop() {

            auto& renderer = gfx::Renderer::get();
            
        #if IS_LAUNCHER_DIRECTIVE
            if (ult::launchingOverlay)
                return;
        #endif
            renderer.startFrame();
            
            this->animationLoop();
            this->getCurrentGui()->update();
            this->getCurrentGui()->draw(&renderer);
            
            renderer.endFrame();
        }
        


                
        void handleInput(u64 keysDown, u64 keysHeld, bool touchDetected, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
            // Static variables to maintain state between function calls
            static HidTouchState initialTouchPos = { 0 };
            static HidTouchState oldTouchPos = { 0 };
            static bool oldTouchDetected = false;
            static elm::TouchEvent touchEvent, oldTouchEvent;
        
            static u64 buttonPressTime_ns = 0, lastKeyEventTime_ns = 0;
            static bool singlePressHandled = false;
            static const u64 clickThreshold_ns = 340000000ULL; // 340ms in nanoseconds
            static u64 keyEventInterval_ns = 67000000ULL; // 67ms in nanoseconds
            
            auto& currentGui = this->getCurrentGui();
            //static bool isTopElement = true;
        
            // Return early if current GUI is not available
        #if !IS_STATUS_MONITOR_DIRECTIVE
            if (!currentGui) return;
            if (!ult::internalTouchReleased) return;
        #endif
            
            // Retrieve current focus and top/bottom elements of the GUI
            auto currentFocus = currentGui->getFocusedElement();
            auto topElement = currentGui->getTopElement();
        
        #if !IS_STATUS_MONITOR_DIRECTIVE
            if (ult::runningInterpreter.load()) {
                if (keysDown & KEY_UP && !(keysDown & ~KEY_UP & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Up);
                else if (keysDown & KEY_DOWN && !(keysDown & ~KEY_DOWN & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Down);
                else if (keysDown & KEY_LEFT && !(keysDown & ~KEY_LEFT & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Left);
                else if (keysDown & KEY_RIGHT && !(keysDown & ~KEY_RIGHT & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Right);
            }
        #endif
        
        #if IS_STATUS_MONITOR_DIRECTIVE
            if (FullMode && !deactivateOriginalFooter) {
                if (ult::simulatedBack) {
                    ult::simulatedBack = false;
                    ult::simulatedBackComplete = true;
                    ult::stillTouching = false;
                    this->goBack();
                    return;
                }
            } else {
                ult::simulatedBack = false;
                ult::simulatedBackComplete = true;
            }
        #else
            if (!overrideBackButton) {
                if (ult::simulatedBack) {
                    keysDown |= KEY_B;
                    ult::simulatedBack = false;
                    ult::simulatedBackComplete = true;
                }
                if (keysDown & KEY_B) {
                    if (!currentGui->handleInput(KEY_B,0,{},{},{}))
                        this->goBack();
                    return;
                }
            }
        #endif
            
            if (!currentFocus && !ult::simulatedBack && ult::simulatedBackComplete && !ult::stillTouching && !ult::runningInterpreter.load(std::memory_order_acquire)) {
                if (!topElement) return;
                
                if (!currentGui->initialFocusSet() || keysDown & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
                    currentGui->requestFocus(topElement, FocusDirection::None);
                    currentGui->markInitialFocusSet();
                    //isTopElement = true;
                }
            }
            
            static bool hasScrolled = false;
        
            if (!currentFocus && !touchDetected && (!oldTouchDetected || oldTouchEvent == elm::TouchEvent::Scroll)) {
                if (!ult::simulatedBack && ult::simulatedBackComplete && topElement) {
                    if (oldTouchEvent == elm::TouchEvent::Scroll) {
                        hasScrolled = true;
                    }
                    if (!hasScrolled) {
                        currentGui->removeFocus();
                        currentGui->requestFocus(topElement, FocusDirection::None);
                    }
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
        
            if (hasScrolled) {
                bool singleArrowKeyPress = ((keysHeld & KEY_UP) != 0) + ((keysHeld & KEY_DOWN) != 0) + ((keysHeld & KEY_LEFT) != 0) + ((keysHeld & KEY_RIGHT) != 0) == 1;
                
                if (singleArrowKeyPress) {
                    u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                    buttonPressTime_ns = currentTime_ns;
                    lastKeyEventTime_ns = currentTime_ns;
                    hasScrolled = false;
                }
            } else {
                if (!touchDetected && !oldTouchDetected && !handled && currentFocus && !ult::stillTouching && !ult::runningInterpreter.load(std::memory_order_acquire)) {
                    static bool shouldShake = true;
                    bool singleArrowKeyPress = ((keysHeld & KEY_UP) != 0) + ((keysHeld & KEY_DOWN) != 0) + ((keysHeld & KEY_LEFT) != 0) + ((keysHeld & KEY_RIGHT) != 0) == 1;
                    
                    if (singleArrowKeyPress) {
                        u64 currentTime_ns = armTicksToNs(armGetSystemTick());
                        
                        if (keysDown) {
                            buttonPressTime_ns = currentTime_ns;
                            lastKeyEventTime_ns = currentTime_ns;
                            singlePressHandled = false;
                            // Immediate single press action
                            if (keysHeld & KEY_UP && !(keysHeld & ~KEY_UP & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentGui->getTopElement(), FocusDirection::Up, shouldShake);
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
                        
                        u64 durationSincePress_ns = currentTime_ns - buttonPressTime_ns;
                        u64 durationSinceLastEvent_ns = currentTime_ns - lastKeyEventTime_ns;
                        
                        if (!singlePressHandled && durationSincePress_ns >= clickThreshold_ns) {
                            singlePressHandled = true;
                        }
        
                        // Calculate transition factor (t) from 0 to 1 based on how far we are from the transition point
                        const u64 transitionPoint_ns = 2000000000ULL; // 2000ms in nanoseconds
                        const u64 initialInterval_ns = 67000000ULL;   // 67ms in nanoseconds
                        const u64 shortInterval_ns = 10000000ULL;     // 10ms in nanoseconds
                        
                        float t = (durationSincePress_ns >= transitionPoint_ns) ? 1.0f : 
                                 (float)durationSincePress_ns / (float)transitionPoint_ns;
                        
                        // Smooth transition between intervals using linear interpolation
                        keyEventInterval_ns = (u64)((1.0f - t) * initialInterval_ns + t * shortInterval_ns);
                        
                        if (singlePressHandled && durationSinceLastEvent_ns >= keyEventInterval_ns) {
                            lastKeyEventTime_ns = currentTime_ns;
                            if (keysHeld & KEY_UP && !(keysHeld & ~KEY_UP & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentGui->getTopElement(), FocusDirection::Up, false);
                            else if (keysHeld & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ALL_KEYS_MASK)) {
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
                        // Handle the rest of the input
                        if (ult::simulatedBack) {
                            keysDown |= KEY_B;
                            ult::simulatedBack = false;
                        }
        
                        if (keysDown & KEY_B)
                            this->goBack();
                        singlePressHandled = false;
        #endif
                    }
                }
            }
            
            if (!touchDetected && (keysDown & KEY_L) && !(keysHeld & ~KEY_L & ALL_KEYS_MASK) && !ult::runningInterpreter.load(std::memory_order_acquire)) {
                //if (!isTopElement)
                //    currentGui->requestFocus(topElement, FocusDirection::None);
                //isTopElement = true;
                jumpToTop = true;
                currentGui->requestFocus(topElement, FocusDirection::None);
            }
            if (!touchDetected && (keysDown & KEY_R) && !(keysHeld & ~KEY_R & ALL_KEYS_MASK) && !ult::runningInterpreter.load(std::memory_order_acquire)) {
                //if (!isTopElement)
                //    currentGui->requestFocus(topElement, FocusDirection::None);
                //isTopElement = true;
                jumpToBottom = true;
                currentGui->requestFocus(topElement, FocusDirection::None);
            }
            
            if (!touchDetected && oldTouchDetected && currentGui && topElement) {
                topElement->onTouch(elm::TouchEvent::Release, oldTouchPos.x, oldTouchPos.y, oldTouchPos.x, oldTouchPos.y, initialTouchPos.x, initialTouchPos.y);
            }
        
            // Touch region calculations
            ult::touchingBack = (touchPos.x >= 20.0f + ult::layerEdge && touchPos.x < ult::backWidth+86.0f + ult::layerEdge && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= 20.0f + ult::layerEdge && initialTouchPos.x < ult::backWidth+86.0f + ult::layerEdge && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            ult::touchingSelect = !ult::noClickableItems && (touchPos.x >= ult::backWidth+86.0f + ult::layerEdge && touchPos.x < (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >=  ult::backWidth+86.0f + ult::layerEdge && initialTouchPos.x < (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            if (!ult::noClickableItems)
                ult::touchingNextPage = (touchPos.x >= (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && (touchPos.x <= ult::backWidth+86.0f + ult::selectWidth+68.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && (initialTouchPos.x <= ult::backWidth+86.0f + ult::selectWidth+68.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            else
                ult::touchingNextPage = (touchPos.x >= (ult::backWidth+86.0f + ult::layerEdge) && (touchPos.x <= ult::backWidth+86.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (ult::backWidth+86.0f + ult::layerEdge) && (initialTouchPos.x <= ult::backWidth+86.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            ult::touchingMenu = (touchPos.x > ult::layerEdge && touchPos.x <= 245+ult::layerEdge && touchPos.y > 10U && touchPos.y <= 83U) && (initialTouchPos.x > ult::layerEdge && initialTouchPos.x <= 245 + ult::layerEdge && initialTouchPos.y > 10U && initialTouchPos.y <= 83U);
            
        
            if (touchDetected) {
                if (!ult::interruptedTouch) ult::interruptedTouch = (keysHeld & ALL_KEYS_MASK) != 0;
                
                u32 xDistance = std::abs(static_cast<s32>(initialTouchPos.x) - static_cast<s32>(touchPos.x));
                u32 yDistance = std::abs(static_cast<s32>(initialTouchPos.y) - static_cast<s32>(touchPos.y));
                
                bool isScroll = (xDistance * xDistance + yDistance * yDistance) > 1000;
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
                    if (!ult::runningInterpreter.load(std::memory_order_acquire)) {
                        ult::touchInBounds = (initialTouchPos.y <= cfg::FramebufferHeight - 73U && initialTouchPos.y > 73U && initialTouchPos.x <= ult::layerEdge + cfg::FramebufferWidth-30U && initialTouchPos.x > 40U + ult::layerEdge);
                        if (ult::touchInBounds) currentGui->removeFocus();
                    }
                    touchEvent = elm::TouchEvent::Touch;
                }
                
                if (currentGui && topElement && !ult::runningInterpreter.load(std::memory_order_acquire)) {
                    topElement->onTouch(touchEvent, touchPos.x, touchPos.y, oldTouchPos.x, oldTouchPos.y, initialTouchPos.x, initialTouchPos.y);
                    if (touchPos.x > 40U + ult::layerEdge && touchPos.x <= cfg::FramebufferWidth-30U + ult::layerEdge && touchPos.y > 73U && touchPos.y <= cfg::FramebufferHeight - 73U) {
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
                ult::stillTouching = true;
            } else {
                if (!ult::interruptedTouch && !ult::runningInterpreter.load(std::memory_order_acquire)) {
                    if ((oldTouchPos.x >= 20.0f + ult::layerEdge && oldTouchPos.x < ult::backWidth+86.0f + ult::layerEdge && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= 20.0f + ult::layerEdge && initialTouchPos.x < ult::backWidth+86.0f + ult::layerEdge && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        ult::simulatedBackComplete = false;
                        ult::simulatedBack = true;
                    } else if (!ult::noClickableItems && (oldTouchPos.x >= ult::backWidth+86.0f + ult::layerEdge && oldTouchPos.x < (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >=  ult::backWidth+86.0f + ult::layerEdge && initialTouchPos.x < (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        ult::simulatedSelectComplete = false;
                        ult::simulatedSelect = true;
                    } else if (!ult::noClickableItems && (oldTouchPos.x >= (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && (oldTouchPos.x <= ult::backWidth+86.0f + ult::selectWidth+68.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (ult::backWidth+86.0f + ult::selectWidth+68.0f + ult::layerEdge) && (initialTouchPos.x <= ult::backWidth+86.0f + ult::selectWidth+68.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        ult::simulatedNextPageComplete = false;
                        ult::simulatedNextPage = true;
                    } else if (ult::noClickableItems && (oldTouchPos.x >= (ult::backWidth+86.0f + ult::layerEdge) && (oldTouchPos.x <= ult::backWidth+86.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (ult::backWidth+86.0f + ult::layerEdge) && (initialTouchPos.x <= ult::backWidth+86.0f +ult::nextPageWidth+70.0f + ult::layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        ult::simulatedNextPageComplete = false;
                        ult::simulatedNextPage = true;
                    } else if ((oldTouchPos.x > ult::layerEdge && oldTouchPos.x <= ult::layerEdge + 245 && oldTouchPos.y > 10U && oldTouchPos.y <= 83U) && (initialTouchPos.x > ult::layerEdge && initialTouchPos.x <= ult::layerEdge + 245 && initialTouchPos.y > 10U && initialTouchPos.y <= 83U)) {
                        ult::simulatedMenuComplete = false;
                        ult::simulatedMenu = true;
                    }
                }
                
                elm::Element::setInputMode(InputMode::Controller);
                
                oldTouchPos = { 0 };
                initialTouchPos = { 0 };
                touchEvent = elm::TouchEvent::None;
                ult::stillTouching = false;
                ult::interruptedTouch = false;
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
        std::unique_ptr<tsl::Gui>& changeTo(std::unique_ptr<tsl::Gui>&& gui) {
            if (this->m_guiStack.top() != nullptr && this->m_guiStack.top()->m_focusedElement != nullptr)
                this->m_guiStack.top()->m_focusedElement->resetClickAnimation();
            
            
            // Create the top element of the new Gui
            gui->m_topElement = gui->createUI();

            
            // Push the new Gui onto the stack
            this->m_guiStack.push(std::move(gui));
            
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
        template<typename G, typename ...Args>
        std::unique_ptr<tsl::Gui>& changeTo(Args&&... args) {
            return this->changeTo(std::make_unique<G>(std::forward<Args>(args)...));
        }
        
        /**
         * @brief Pops the top Gui from the stack and goes back to the last one
         * @note The Overlay gets closes once there are no more Guis on the stack
         */
        void goBack() {
            if (!this->m_closeOnExit && this->m_guiStack.size() == 1) {
                this->hide();
                return;
            }
            
            if (!this->m_guiStack.empty())
                this->m_guiStack.pop();
            
            if (this->m_guiStack.empty())
                this->close();
        }

        void pop() {
            if (!this->m_guiStack.empty())
                this->m_guiStack.pop();
        }
        
        template<typename G, typename ...Args>
        friend std::unique_ptr<tsl::Gui>& changeTo(Args&&... args);
        
        friend void goBack();
        friend void pop();
        
        template<typename, tsl::impl::LaunchFlags>
        friend int loop(int argc, char** argv);
        
        friend class tsl::Gui;
    };
    
    
    namespace impl {
        static const char* TESLA_CONFIG_FILE = "/config/tesla/config.ini"; // CUSTOM MODIFICATION
        static const char* ULTRAHAND_CONFIG_FILE = "/config/ultrahand/config.ini"; // CUSTOM MODIFICATION
        
        /**
         * @brief Data shared between the different ult::threads
         *
         */
        struct SharedThreadData {
            bool running = false;
            
            Event comboEvent = { 0 };
            
            bool overlayOpen = false;
            
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
            
            #if USING_WIDGET_DIRECTIVE
            ult::datetimeFormat = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["datetime_format"]; // read datetime_format
            ult::removeQuotes(ult::datetimeFormat);
            if (ult::datetimeFormat.empty()) {
                ult::datetimeFormat = ult::DEFAULT_DT_FORMAT;
                ult::removeQuotes(ult::datetimeFormat);
            }
            std::string hideClockStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_clock"];
            ult::removeQuotes(hideClockStr);
            ult::hideClock = hideClockStr != ult::FALSE_STR;
            
            std::string hideBatteryStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_battery"];
            ult::removeQuotes(hideBatteryStr);
            ult::hideBattery = hideBatteryStr != ult::FALSE_STR;
            
            std::string hidePCBTempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_pcb_temp"];
            ult::removeQuotes(hidePCBTempStr);
            ult::hidePCBTemp = hidePCBTempStr != ult::FALSE_STR;
            
            std::string hideSOCTempStr = parsedConfig[ult::ULTRAHAND_PROJECT_NAME]["hide_soc_temp"];
            ult::removeQuotes(hideSOCTempStr);
            ult::hideSOCTemp = hideSOCTempStr != ult::FALSE_STR;
            #endif
            
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
        

        /**
         * @brief Background event polling loop thread
         *
         * @param args Used to pass in a pointer to a \ref SharedThreadData struct
         */
        static void backgroundEventPoller(void *args) {

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
            
        #if IS_LAUNCHER_DIRECTIVE
            // Load overlay key combos
            tsl::hlp::loadOverlayKeyCombos();
        #endif
            
            // Configure input to take all controllers and up to 8
            padConfigureInput(8, HidNpadStyleSet_NpadStandard | HidNpadStyleTag_NpadSystemExt);
            
            // Initialize pad
            PadState pad;
            padInitializeAny(&pad);
            
            // Initialize touch screen
            hidInitializeTouchScreen();
            
            // Drop all inputs from the previous overlay
            padUpdate(&pad);
            
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
            
            static u64 currentTouchTick = 0;
            static auto lastTouchX = 0;

            // Preset touch boundaries
            static const int SWIPE_RIGHT_BOUND = 16;  // 16 + 80
            static const int SWIPE_LEFT_BOUND = (1280 - 16);
            static const u64 TOUCH_THRESHOLD_NS = 150'000'000ULL; // 150ms in nanoseconds

            s32 idx;
            Result rc;

        #if IS_LAUNCHER_DIRECTIVE
            ult::launchingOverlay = false;
        #endif
            std::string currentTitleID;
            u64 nowTick, resetElapsedNs;
            u64 elapsedNs;

            static u64 lastPollTick = 0;
            static u64 resetStartTick = armGetSystemTick();
            static bool runOnce = true;

            if (runOnce) {
                ult::lastTitleID = ult::getTitleIdAsString();
                runOnce = false;
            }

            while (shData->running) {
            
                nowTick = armGetSystemTick();
                elapsedNs = armTicksToNs(nowTick - lastPollTick);

                // Poll Title ID every 1 seconds
                if (!ult::resetForegroundCheck && elapsedNs >= 1'000'000'000ULL) {
                    lastPollTick = nowTick;
                
                    currentTitleID = ult::getTitleIdAsString();
                    if (currentTitleID != ult::lastTitleID) {
                        ult::lastTitleID = currentTitleID;
                        ult::resetForegroundCheck = true;
                        resetStartTick = nowTick;
                    }
                }

                //currentTitleID = ult::getTitleIdAsString();
                //if (currentTitleID != ult::lastTitleID) {
                //    ult::lastTitleID = currentTitleID;
                //    ult::resetForegroundCheck = true;
                //    resetStartTick = nowTick;
                //}
            
                // If a reset is scheduled, trigger after 3.5s delay
                if (ult::resetForegroundCheck) {
                    resetElapsedNs = armTicksToNs(nowTick - resetStartTick);
                    if (resetElapsedNs >= 3'500'000'000ULL) {
                        if (shData->overlayOpen && ult::currentForeground) {
                            hlp::requestForeground(true, false);
                        }
                        ult::resetForegroundCheck = false;
                    }
                }


                // Scan for input changes
                padUpdate(&pad);
                
                // Read in HID values
                {
                    std::scoped_lock lock(shData->dataMutex);
                    
                    shData->keysDown = padGetButtonsDown(&pad);
                    shData->keysHeld = padGetButtons(&pad);
                    shData->joyStickPosLeft  = padGetStickPos(&pad, 0);
                    shData->joyStickPosRight = padGetStickPos(&pad, 1);
                    
                    
                    // Read in touch positions
                    if (hidGetTouchScreenStates(&shData->touchState, 1) > 0) { // Check if any touch event is present
                        HidTouchState& currentTouch = shData->touchState.touches[0];  // Correct type is HidTouchPoint
                        
                    
                        if (!shData->overlayOpen) {
                            ult::internalTouchReleased = false;
                        }
                        
                        u64 elapsedTime_ns = armTicksToNs(nowTick - currentTouchTick);
                        
                        // Check if the touch is within bounds for left-to-right swipe within the time window
                        if (ult::useSwipeToOpen && elapsedTime_ns <= TOUCH_THRESHOLD_NS) {
                            if (lastTouchX != 0 && currentTouch.x != 0) {
                                if (ult::layerEdge == 0 && currentTouch.x > SWIPE_RIGHT_BOUND + 84 && lastTouchX <= SWIPE_RIGHT_BOUND) {
                                    eventFire(&shData->comboEvent);
                                }
                                // Check if the touch is within bounds for right-to-left swipe within the time window
                                else if (ult::layerEdge > 0 && currentTouch.x < SWIPE_LEFT_BOUND - 84 && lastTouchX >= SWIPE_LEFT_BOUND) {
                                    eventFire(&shData->comboEvent);
                                }
                            }
                        }
                    
                        // Handle touch release state
                        if (currentTouch.x == 0 && currentTouch.y == 0) {
                            ult::internalTouchReleased = true;  // Indicate that the touch has been released
                            lastTouchX = currentTouch.x;
                        }

                        // If this is the first touch of a gesture, store lastTouchX
                        if (lastTouchX == 0 && currentTouch.x != 0) {
                            lastTouchX = currentTouch.x;
                            currentTouchTick = nowTick;
                        }
                    
                    } else {
                        // Reset touch state if no touch is present
                        shData->touchState = { 0 };
                        ult::internalTouchReleased = true;
                    
                        // Reset touch history to invalid state
                        lastTouchX = 0;
                    
                        // Reset time tracking
                        currentTouchTick = nowTick;
                    }
                    

                    // Check main launch combo first (highest priority)
                    if ((((shData->keysHeld & tsl::cfg::launchCombo) == tsl::cfg::launchCombo) && shData->keysDown & tsl::cfg::launchCombo)) {
                    #if IS_LAUNCHER_DIRECTIVE
                        if (ult::updateMenuCombos) {
                            ult::setIniFileValue(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::KEY_COMBO_STR , ult::ULTRAHAND_COMBO_STR);
                            ult::setIniFileValue(ult::TESLA_CONFIG_INI_PATH, ult::TESLA_STR, ult::KEY_COMBO_STR , ult::ULTRAHAND_COMBO_STR);
                            ult::updateMenuCombos = false;
                        }
                    #endif
                        
                        if (shData->overlayOpen) {
                            tsl::Overlay::get()->hide();
                            shData->overlayOpen = false;
                        }
                        else {
                            eventFire(&shData->comboEvent);
                        }
                    }
                #if IS_LAUNCHER_DIRECTIVE
                    else if (ult::updateMenuCombos && (((shData->keysHeld & tsl::cfg::launchCombo2) == tsl::cfg::launchCombo2) && shData->keysDown & tsl::cfg::launchCombo2)) {
                        std::swap(tsl::cfg::launchCombo, tsl::cfg::launchCombo2); // Swap the two launch combos
                        ult::setIniFileValue(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::KEY_COMBO_STR , ult::TESLA_COMBO_STR);
                        ult::setIniFileValue(ult::TESLA_CONFIG_INI_PATH, ult::TESLA_STR, ult::KEY_COMBO_STR , ult::TESLA_COMBO_STR);
                        eventFire(&shData->comboEvent);
                        ult::updateMenuCombos = false;
                    }
                    // Check overlay key combos (only when overlay is not open, keys are pressed, and not conflicting with main combos)
                    else if (!shData->overlayOpen && shData->keysDown != 0) {
                        // Make sure this isn't a subset of the main launch combos
                        bool isMainComboMatch = shData->keysHeld == tsl::cfg::launchCombo;
                        //bool isMainCombo2Match = shData->keysHeld == tsl::cfg::launchCombo2;

                        if (!isMainComboMatch) {
                            std::string overlayPath = tsl::hlp::getOverlayForKeyCombo(shData->keysHeld);
                            if (!overlayPath.empty() && (shData->keysHeld)) {
                                // Validate overlay file exists
                                if (ult::isFileOrDirectory(overlayPath)) {
                                    ult::launchingOverlay = true;
                                    //svcSleepThread(500'000'000); // 50ms delay
                                    // Get overlay settings
                                    std::string overlayFileName = ult::getNameFromPath(overlayPath);
                                    std::string useOverlayLaunchArgs = ult::parseValueFromIniSection(ult::OVERLAYS_INI_FILEPATH, 
                                        overlayFileName, ult::USE_LAUNCH_ARGS_STR);
                                    std::string overlayLaunchArgs = ult::parseValueFromIniSection(ult::OVERLAYS_INI_FILEPATH, 
                                        overlayFileName, ult::LAUNCH_ARGS_STR);
                                    ult::removeQuotes(overlayLaunchArgs);
                                    
                                    // Set the next overlay directly
                                    if (useOverlayLaunchArgs == ult::TRUE_STR)
                                        tsl::setNextOverlay(overlayPath, overlayLaunchArgs);
                                    else
                                        tsl::setNextOverlay(overlayPath);
                                    
                                    // Properly close the overlay to trigger the launch
                                    tsl::Overlay::get()->close();
                                    eventFire(&shData->comboEvent);
                                    break;
                                    // DON'T set shData->running = false here!
                                }
                            }
                        }
                        //bool isMainCombo2Match = false;
                    }
                #endif
                    
                    shData->keysDownPending |= shData->keysDown;
                }
                
                //20 ms
                //s32 idx = 0;
                rc = waitObjects(&idx, objects, WaiterObject_Count, 20'000'000ul);
                if (R_SUCCEEDED(rc)) {
                    if (idx == WaiterObject_HomeButton || idx == WaiterObject_PowerButton) { // Changed condition to exclude capture button
                        if (shData->overlayOpen) {
                            tsl::Overlay::get()->hide();
                            shData->overlayOpen = false;
                        }
                    }
                    
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
                            padInitializeAny(&pad);
                            hidInitializeTouchScreen();
                            padUpdate(&pad);
                            
                            break;
                        case WaiterObject_CaptureButton:
                            ult::disableTransparency = true;
                            eventClear(&captureButtonPressEvent);
                            svcSleepThread(500'000'000);
                            ult::disableTransparency = false;
                            break;
                    }
                } else if (rc != KERNELRESULT(TimedOut)) {
                    ASSERT_FATAL(rc);
                }
            }
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
    
    /**
     * @brief Pops the top Gui from the stack and goes back to the last one
     * @note The Overlay gets closed once there are no more Guis on the stack
     */
    static void goBack() {
        Overlay::get()->goBack();
    }
    
    static void pop() {
        Overlay::get()->pop();
    }

    
    static void setNextOverlay(const std::string& ovlPath, std::string origArgs) {
        bool hasSkipCombo = origArgs.find("--skipCombo") != std::string::npos;
        
        char buffer[1024]; // Adjust size as needed
        char* p = buffer;
        
        // Store the filename in a string to keep it alive
        std::string filenameStr = ult::getNameFromPath(ovlPath);
        const char* filename = filenameStr.c_str();
        
        // Copy filename
        while (*filename) *p++ = *filename++;
        *p++ = ' ';
        
        // Copy origArgs while filtering --foregroundFix and --lastTitleID
        const char* src = origArgs.c_str();
        const char* end = src + origArgs.length();
        
        while (src < end) {
            const char* fgPos = strstr(src, "--foregroundFix");
            const char* titlePos = strstr(src, "--lastTitleID");
            
            // Find the earliest flag to remove
            const char* nextFlag = nullptr;
            if (fgPos && titlePos) {
                nextFlag = (fgPos < titlePos) ? fgPos : titlePos;
            } else if (fgPos) {
                nextFlag = fgPos;
            } else if (titlePos) {
                nextFlag = titlePos;
            }
            
            if (nextFlag) {
                // Copy before flag
                while (src < nextFlag) *p++ = *src++;
                
                if (nextFlag == fgPos) {
                    // Skip "--foregroundFix X"
                    src = nextFlag + 15; // length of "--foregroundFix"
                    while (src < end && *src == ' ') src++; // Skip spaces
                    if (src < end && (*src == '0' || *src == '1')) src++; // Skip single digit
                } else {
                    // Skip "--lastTitleID XXXXXXXXXXXXXXXX"
                    src = nextFlag + 13; // length of "--lastTitleID"
                    while (src < end && *src == ' ') src++; // Skip spaces
                    while (src < end && *src != ' ') src++; // Skip title ID value
                }
            } else {
                // No more flags to filter, copy rest
                while (src < end) *p++ = *src++;
                break;
            }
        }
        
        // Add flags
        if (!hasSkipCombo) {
            __builtin_memcpy(p, " --skipCombo", 12);
            p += 12;
        }
        
        // Add foreground flag
        __builtin_memcpy(p, " --foregroundFix ", 17);
        p += 17;
        *p++ = (ult::resetForegroundCheck || ult::lastTitleID != ult::getTitleIdAsString()) ? '1' : '0';
        
        // Add last title ID
        __builtin_memcpy(p, " --lastTitleID ", 15);
        p += 15;
        const char* titleId = ult::lastTitleID.c_str();
        while (*titleId) *p++ = *titleId++;
        
        *p = '\0';
        
        envSetNextLoad(ovlPath.c_str(), buffer);
    }
    
    
    
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

        // CUSTOM SECTION START
        // Argument parsing
    #if IS_LAUNCHER_DIRECTIVE
        const std::string settings = ult::inputExists(ult::SETTINGS_PATH);
    #endif

        bool skipCombo = false;
        for (u8 arg = 0; arg < argc; arg++) {
            const char* s = argv[arg];
        
            if (s[0] == '-' && s[1] == '-') {
                if (s[2] == 's' && !strcmp(s, "--skipCombo")) {
                    skipCombo = true;
                    ult::firstBoot = false;
                }
                else if (s[2] == 'f' && !strcmp(s, "--foregroundFix") && arg + 1 < argc) {
                    ult::resetForegroundCheck = ult::resetForegroundCheck || (argv[++arg][0] == '1'); // Just check for '1'
                }
                else if (s[2] == 'l' && !strcmp(s, "--lastTitleID") && arg + 1 < argc) {
                    const char* providedID = argv[++arg];
                    if (ult::getTitleIdAsString() != providedID) {
                        ult::resetForegroundCheck = true;
                    }
                }
            }
        }

        impl::SharedThreadData shData;
        
        shData.running = true;
        
        Thread backgroundThread;
        threadCreate(&backgroundThread, impl::backgroundEventPoller, &shData, nullptr, 0x1000, 0x2c, -2);
        threadStart(&backgroundThread);
        
        eventCreate(&shData.comboEvent, false);
        
        auto& overlay = tsl::Overlay::s_overlayInstance;
        overlay = new TOverlay();
        overlay->m_closeOnExit = (u8(launchFlags) & u8(impl::LaunchFlags::CloseOnExit)) == u8(impl::LaunchFlags::CloseOnExit);
        
        
        tsl::hlp::doWithSmSession([&overlay]{
            overlay->initServices();
        });
    #if IS_LAUNCHER_DIRECTIVE
    #else
        tsl::initializeUltrahandSettings(); // for initializing settings
    #endif
        overlay->initScreen();
        overlay->changeTo(overlay->loadInitialGui());


    #if IS_LAUNCHER_DIRECTIVE
        bool inOverlay;
        if (ult::inputExists(settings)
            != "}nwmD9myxpsq9\x7fv~|krkxn9"
        ) {inOverlay = true; return 0;}
        else {
            if (ult::firstBoot)
                ult::setIniFileValue(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::IN_OVERLAY_STR, ult::FALSE_STR);
            inOverlay = (ult::parseValueFromIniSection(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::IN_OVERLAY_STR) != ult::FALSE_STR);
        }

    #else
        bool inOverlay = true;
    #endif

        if (inOverlay && skipCombo) {
            #if IS_LAUNCHER_DIRECTIVE
            ult::setIniFileValue(ult::ULTRAHAND_CONFIG_INI_PATH, ult::ULTRAHAND_PROJECT_NAME, ult::IN_OVERLAY_STR, ult::FALSE_STR);
            #endif
            eventFire(&shData.comboEvent);
        }

        overlay->disableNextAnimation();


        while (shData.running) {
            eventWait(&shData.comboEvent, UINT64_MAX);
            eventClear(&shData.comboEvent);
            shData.overlayOpen = true;
            
            
            hlp::requestForeground(true);
            
            overlay->show();
            overlay->clearScreen();
            
            while (shData.running) {
                overlay->loop();
                {
                    std::scoped_lock lock(shData.dataMutex);
                    if (!overlay->fadeAnimationPlaying()) {
                        overlay->handleInput(shData.keysDownPending, shData.keysHeld, shData.touchState.count, shData.touchState.touches[0], shData.joyStickPosLeft, shData.joyStickPosRight);
                    }
                    shData.keysDownPending = 0;
                }
                
                if (overlay->shouldHide()) {
                    break;
                }
                
                if (overlay->shouldClose())
                    shData.running = false;
            }
            
            overlay->clearScreen();
            overlay->resetFlags();
            
            hlp::requestForeground(false);
            
            shData.overlayOpen = false;

            eventClear(&shData.comboEvent);
        }
        
        eventClose(&shData.comboEvent);
        
        threadWaitForExit(&backgroundThread);
        threadClose(&backgroundThread);
        
        overlay->exitScreen();
        overlay->exitServices();
        
        delete overlay;
        
        return 0;
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

        tsl::hlp::doWithSmSession([]{
            
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
            
            #if USING_WIDGET_DIRECTIVE
            ASSERT_FATAL(timeInitialize()); // CUSTOM MODIFICATION
            __libnx_init_time();            // CUSTOM MODIFICATION
            timeExit(); // CUSTOM MODIFICATION

            ult::powerInit();
            i2cInitialize();
            #endif

            fsdevMountSdmc();
            splInitialize();
            spsmInitialize();
            //i2cInitialize();
            //ASSERT_FATAL(socketInitializeDefault());
            //ASSERT_FATAL(nifmInitialize(NifmServiceType_User));
        });
        ASSERT_FATAL(smInitialize()); // needed to prevent issues with powering device into sleep

        #if IS_STATUS_MONITOR_DIRECTIVE
        Service *plSrv = plGetServiceSession();
        Service plClone;
        ASSERT_FATAL(serviceClone(plSrv, &plClone));
        serviceClose(plSrv);
        *plSrv = plClone;
        #endif
    }
    
    /**
     * @brief libtesla service exiting function to override libnx's
     *
     */
    void __appExit(void) {
        
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
    }

}

#endif
