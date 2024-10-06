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

//static bool debugFPS = true;

//uint64_t RAM_Used_system_u = 0;
//uint64_t RAM_Total_system_u = 0;

using namespace ult;


// CUSTOM SECTION END

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
#define ELEMENT_BOUNDS_2(elem) elem->getX()+4, elem->getY()+4, elem->getWidth(), elem->getHeight()+10

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
using namespace std::literals::chrono_literals;



namespace tsl {

    // Constants
    
    namespace cfg {
        
        constexpr u32 ScreenWidth = 1920;       ///< Width of the Screen
        constexpr u32 ScreenHeight = 1080;      ///< Height of the Screen
        
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
    
    inline Color GradientColor(float temperature) {
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


    inline Color RGB888(const std::string& hexColor, size_t alpha = 15, const std::string& defaultHexColor = whiteColor) {
        std::string validHex = hexColor.empty() || hexColor[0] != '#' ? hexColor : hexColor.substr(1);
        
        if (!isValidHexColor(validHex)) {
            validHex = defaultHexColor;
        }
        
        // Convert hex to RGB values
        uint8_t redValue = (hexMap[static_cast<unsigned char>(validHex[0])] << 4) | hexMap[static_cast<unsigned char>(validHex[1])];
        uint8_t greenValue = (hexMap[static_cast<unsigned char>(validHex[2])] << 4) | hexMap[static_cast<unsigned char>(validHex[3])];
        uint8_t blueValue = (hexMap[static_cast<unsigned char>(validHex[4])] << 4) | hexMap[static_cast<unsigned char>(validHex[5])];
        
        return Color(redValue >> 4, greenValue >> 4, blueValue >> 4, alpha);
    }


    inline std::tuple<uint8_t, uint8_t, uint8_t> hexToRGB444Floats(const std::string& hexColor, const std::string& defaultHexColor = "#FFFFFF") {
        const char* validHex = hexColor.c_str();
        if (validHex[0] == '#') validHex++;
        
        if (!isValidHexColor(validHex)) {
            validHex = defaultHexColor.c_str();
            if (validHex[0] == '#') validHex++;
        }
        
        // Manually parse the hex string to an integer value
        unsigned int hexValue = (hexMap[static_cast<unsigned char>(validHex[0])] << 20) |
                                (hexMap[static_cast<unsigned char>(validHex[1])] << 16) |
                                (hexMap[static_cast<unsigned char>(validHex[2])] << 12) |
                                (hexMap[static_cast<unsigned char>(validHex[3])] << 8)  |
                                (hexMap[static_cast<unsigned char>(validHex[4])] << 4)  |
                                hexMap[static_cast<unsigned char>(validHex[5])];
        
        // Extract and scale the RGB components from 8-bit (0-255) to 4-bit float scale (0-15)
        uint8_t red = ((hexValue >> 16) & 0xFF) / 255.0f * 15.0f;
        uint8_t green = ((hexValue >> 8) & 0xFF) / 255.0f * 15.0f;
        uint8_t blue = (hexValue & 0xFF) / 255.0f * 15.0f;
        
        return std::make_tuple(red, green, blue);
    }

    
    
    namespace style {
        constexpr u32 ListItemDefaultHeight         = 70;       ///< Standard list item height
        constexpr u32 MiniListItemDefaultHeight     = 40;       ///< Standard list item height
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


    // Theme color variable definitions
    static bool disableColorfulLogo = false;
    static Color logoColor1 = RGB888(whiteColor);
    static Color logoColor2 = RGB888("#F7253E");
    static size_t defaultBackgroundAlpha = 13;
    
    static Color defaultBackgroundColor = RGB888(blackColor, defaultBackgroundAlpha);
    static Color defaultTextColor = RGB888(whiteColor);
    static Color headerTextColor = RGB888(whiteColor);
    static Color headerSeparatorColor = RGB888(whiteColor);
    static Color starColor = RGB888(whiteColor);
    static Color selectionStarColor = RGB888(whiteColor);
    static Color buttonColor = RGB888(whiteColor);
    static Color bottomTextColor = RGB888(whiteColor);
    static Color botttomSeparatorColor = RGB888(whiteColor);

    static Color defaultPackageColor = RGB888("#00FF00");
    static Color clockColor = RGB888(whiteColor);
    static Color batteryColor = RGB888("#ffff45");
    static Color versionTextColor = RGB888("#AAAAAA");
    static Color onTextColor = RGB888("#00FFDD");
    static Color offTextColor = RGB888("#AAAAAA");
    
    static std::tuple<float,float,float> dynamicLogoRGB1 = hexToRGB444Floats("#00E669");
    static std::tuple<float,float,float> dynamicLogoRGB2 = hexToRGB444Floats("#8080EA");

    static bool disableSelectionBG = false;
    static bool invertBGClickColor = false;

    static size_t selectionBGAlpha = 7;
    static Color selectionBGColor = RGB888(blackColor, selectionBGAlpha);

    static Color highlightColor1 = RGB888("#2288CC");
    static Color highlightColor2 = RGB888("#88FFFF");
    static Color highlightColor3 = RGB888("#FFFF45");
    static Color highlightColor4 = RGB888("#F7253E");

    static Color highlightColor = tsl::style::color::ColorHighlight;
    
    static size_t clickAlpha = 7;
    static Color clickColor = RGB888("#3E25F7", clickAlpha);

    static size_t progressAlpha = 7;
    static Color progressColor = RGB888("#253EF7", progressAlpha);

    static Color trackBarColor = RGB888("#555555");

    static size_t separatorAlpha = 15;
    static Color separatorColor = RGB888("#404040", separatorAlpha);

    static Color selectedTextColor = RGB888(whiteColor);
    static Color inprogressTextColor = RGB888(whiteColor);
    static Color invalidTextColor = RGB888("#FF0000");
    static Color clickTextColor = RGB888(whiteColor);

    static size_t tableBGAlpha = 10;
    static Color tableBGColor = RGB888("#303030", tableBGAlpha);
    static Color sectionTextColor = RGB888("#e9ff40");
    static Color infoTextColor = RGB888(whiteColor);
    static Color warningTextColor = RGB888("#FF7777");


    static Color trackBarSliderColor = RGB888("#606060");
    static Color trackBarSliderBorderColor = RGB888("#505050");
    static Color trackBarSliderMalleableColor = RGB888("#A0A0A0");
    static Color trackBarFullColor = RGB888("#00FFDD");
    static Color trackBarEmptyColor = RGB888("#404040");
    
    static void initializeThemeVars() { // NOTE: This needs to be called once in your application.
        // Fetch all theme settings at once from the INI file
        auto themeData = getParsedDataFromIniFile(THEME_CONFIG_INI_PATH);
        if (themeData.count(THEME_STR) > 0) {
            auto& themeSection = themeData[THEME_STR];
            
            // Fetch and process each theme setting using a helper to simplify fetching and fallback
            auto getValue = [&](const std::string& key) {
                return themeSection.count(key) ? themeSection[key] : defaultThemeSettingsMap[key];
            };
            
            // Convert hex color to Color and manage default values and conversion
            auto getColor = [&](const std::string& key, size_t alpha = 15) {
                //std::string hexColor = getValue(key);
                return RGB888(getValue(key), alpha);
            };
            
            auto getAlpha = [&](const std::string& key) {
                std::string alphaStr = getValue(key);
                return !alphaStr.empty() ? std::stoi(alphaStr) : std::stoi(defaultThemeSettingsMap[key]);
            };
            
            disableColorfulLogo = (getValue("disable_colorful_logo") == TRUE_STR);
            
            logoColor1 = getColor("logo_color_1");
            logoColor2 = getColor("logo_color_2");
            
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
            defaultPackageColor = getColor("default_package_color");

            clockColor = getColor("clock_color");
            batteryColor = getColor("battery_color");
            
            versionTextColor = getColor("version_text_color");
            onTextColor = getColor("on_text_color");
            offTextColor = getColor("off_text_color");
            
            dynamicLogoRGB1 = hexToRGB444Floats(getValue("dynamic_logo_color_1"));
            dynamicLogoRGB2 = hexToRGB444Floats(getValue("dynamic_logo_color_2"));
            
            disableSelectionBG = (getValue("disable_selection_bg") == TRUE_STR);
            invertBGClickColor = (getValue("invert_bg_click_color") == TRUE_STR);

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


            trackBarSliderColor = getColor("trackbar_slider_color");
            trackBarSliderBorderColor = getColor("trackbar_slider_border_color");
            trackBarSliderMalleableColor = getColor("trackbar_slider_malleable_color");
            trackBarFullColor = getColor("trackbar_full_color");
            trackBarEmptyColor = getColor("trackbar_empty_color");
        }
    }
    
    #if IS_LAUNCHER_DIRECTIVE
    #else
    static void initializeUltrahandSettings() {
        // Set Ultrahand Globals
        useSwipeToOpen = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "swipe_to_open") == TRUE_STR);
        useOpaqueScreenshots = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "opaque_screenshots") == TRUE_STR);

        std::string defaultLang = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, DEFAULT_LANG_STR);
        defaultLang = defaultLang.empty() ? "en" : defaultLang;

        std::string langFile = LANG_PATH+defaultLang+".json";
        if (isFileOrDirectory(langFile))
            parseLanguage(langFile);
        else
            reinitializeLangVars();
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
        static void requestForeground(bool enabled) {
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
                
                return parseIni(configFileData);
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
            for (const auto& keyInfo : KEYS_INFO) {
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
            for (std::string key : split(removeWhiteSpaces(value), '+')) { // CUSTOM MODIFICATION (bug fix)
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
        
            for (const auto &keyInfo : KEYS_INFO) {
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
        
    }
    
    // Renderer
    
    namespace gfx {
        
        extern "C" u64 __nx_vi_layer_id;
        
        //Color src(0);
        //Color end(0);

        struct ScissoringConfig {
            s32 x, y, w, h;
        };
        
        /**
         * @brief Manages the Tesla layer and draws raw data to the screen
         */
        class Renderer final {
        public:

            Renderer& operator=(Renderer&) = delete;
            
            friend class tsl::Overlay;
            
            /**
             * @brief Handles opacity of drawn colors for fadeout. Pass all colors through this function in order to apply opacity properly
             *
             * @param c Original color
             * @return Color with applied opacity
             */
            inline static Color a(const Color& c) {
                u8 alpha = (disableTransparency && useOpaqueScreenshots) ? 0xF : static_cast<u8>(std::min(static_cast<u8>(c.a), static_cast<u8>(0xF * Renderer::s_opacity)));
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
            inline void enableScissoring(const s32 x, const s32 y, const s32 w, const s32 h) {
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
            inline void setPixel(const s32 x, const s32 y, const Color& color, const u32 offset) {
                if (x < cfg::FramebufferWidth && y < cfg::FramebufferHeight) {
                    //u32 offset = this->getPixelOffset(x, y);
                    if (offset != UINT32_MAX) {
                        Color* framebuffer = static_cast<Color*>(this->getCurrentFramebuffer());
                        framebuffer[offset] = color;
                    }
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
                return (dst * alpha + src * (0x0F - alpha)) >> 4;
            }
            
            /**
             * @brief Draws a single source blended pixel onto the screen
             *
             * @param x X pos
             * @param y Y pos
             * @param color Color
             */
            inline void setPixelBlendSrc(const s32 x, const s32 y, const Color& color) {
                //if (x < 0 || y < 0 || x >= cfg::FramebufferWidth || y >= cfg::FramebufferHeight)
                //    return;
                
                u32 offset = this->getPixelOffset(x, y);
                if (offset == UINT32_MAX)
                    return;
                
                u16* framebuffer = static_cast<u16*>(this->getCurrentFramebuffer());
                //src=framebuffer[offset];
                Color src(framebuffer[offset]);
                
                Color end(0);
                end.r = blendColor(src.r, color.r, color.a);
                end.g = blendColor(src.g, color.g, color.a);
                end.b = blendColor(src.b, color.b, color.a);
                end.a = src.a;
            
                this->setPixel(x, y, end, offset);
            }
            
            /**
             * @brief Draws a single destination blended pixel onto the screen
             *
             * @param x X pos
             * @param y Y pos
             * @param color Color
             */
            inline void setPixelBlendDst(const s32 x, const s32 y, const Color& color) {
                //if (x < 0 || y < 0 || x >= cfg::FramebufferWidth || y >= cfg::FramebufferHeight)
                //    return;
                
                u32 offset = this->getPixelOffset(x, y);
                if (offset == UINT32_MAX)
                    return;
                
                u16* framebuffer = static_cast<u16*>(this->getCurrentFramebuffer());
                //src=framebuffer[offset];
                Color src(framebuffer[offset]);
                
                Color end(0);
                end.r = blendColor(src.r, color.r, color.a);
                end.g = blendColor(src.g, color.g, color.a);
                end.b = blendColor(src.b, color.b, color.a);
                end.a = color.a + (src.a * (0xF - color.a) / 0xF);
            
                this->setPixel(x, y, end, offset);
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
                s32 x_end = x + w;
                s32 y_end = y + h;
            
                for (s32 yi = y; yi < y_end; ++yi) {
                    for (s32 xi = x; xi < x_end; ++xi) {
                        this->setPixelBlendDst(xi, yi, color);
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

            inline void drawQuarterCircle(s32 centerX, s32 centerY, u16 radius, bool filled, const Color& color, int quadrant) {
                s32 x = radius;
                s32 y = 0;
                s32 radiusError = 0;
                s32 xChange = 1 - (radius << 1);
                s32 yChange = 0;
                
                while (x >= y) {
                    if (filled) {
                        switch (quadrant) {
                            case 1: // Top-right
                                for (s32 i = centerX; i <= centerX + x; i++) {
                                    this->setPixelBlendDst(i, centerY - y, color);
                                }
                                for (s32 i = centerX; i <= centerX + y; i++) {
                                    this->setPixelBlendDst(i, centerY - x, color);
                                }
                                break;
                            case 2: // Top-left
                                for (s32 i = centerX - x; i <= centerX; i++) {
                                    this->setPixelBlendDst(i, centerY - y, color);
                                }
                                for (s32 i = centerX - y; i <= centerX; i++) {
                                    this->setPixelBlendDst(i, centerY - x, color);
                                }
                                break;
                            case 3: // Bottom-left
                                for (s32 i = centerX - x; i <= centerX; i++) {
                                    this->setPixelBlendDst(i, centerY + y, color);
                                }
                                for (s32 i = centerX - y; i <= centerX; i++) {
                                    this->setPixelBlendDst(i, centerY + x, color);
                                }
                                break;
                            case 4: // Bottom-right
                                for (s32 i = centerX; i <= centerX + x; i++) {
                                    this->setPixelBlendDst(i, centerY + y, color);
                                }
                                for (s32 i = centerX; i <= centerX + y; i++) {
                                    this->setPixelBlendDst(i, centerY + x, color);
                                }
                                break;
                        }
                    } else {
                        switch (quadrant) {
                            case 1: // Top-right
                                this->setPixelBlendDst(centerX + x, centerY - y, color);
                                this->setPixelBlendDst(centerX + y, centerY - x, color);
                                break;
                            case 2: // Top-left
                                this->setPixelBlendDst(centerX - x, centerY - y, color);
                                this->setPixelBlendDst(centerX - y, centerY - x, color);
                                break;
                            case 3: // Bottom-left
                                this->setPixelBlendDst(centerX - x, centerY + y, color);
                                this->setPixelBlendDst(centerX - y, centerY + x, color);
                                break;
                            case 4: // Bottom-right
                                this->setPixelBlendDst(centerX + x, centerY + y, color);
                                this->setPixelBlendDst(centerX + y, centerY + x, color);
                                break;
                        }
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

            inline void drawBorderedRoundedRect(const s32 x, const s32 y, const s32 width, const s32 height, const s32 thickness, const s32 radius, const Color& highlightColor) {
                s32 startX = x + 4;
                s32 startY = y;
                s32 adjustedWidth = width - 12;
                s32 adjustedHeight = height + 1;
                
                // Draw borders
                this->drawRect(startX, startY - thickness, adjustedWidth, thickness, highlightColor); // Top border
                this->drawRect(startX, startY + adjustedHeight, adjustedWidth, thickness, highlightColor); // Bottom border
                this->drawRect(startX - thickness, startY, thickness, adjustedHeight, highlightColor); // Left border
                this->drawRect(startX + adjustedWidth, startY, thickness, adjustedHeight, highlightColor); // Right border
                
                // Draw corners
                this->drawQuarterCircle(startX, startY, radius, true, highlightColor, 2); // Upper-left
                this->drawQuarterCircle(startX, startY + height, radius, true, highlightColor, 3); // Lower-left
                this->drawQuarterCircle(x + width - 9, startY, radius, true, highlightColor, 1); // Upper-right
                this->drawQuarterCircle(x + width - 9, startY + height, radius, true, highlightColor, 4); // Lower-right
            }


            // Define processChunk as a static member function
            static void processRoundedRectChunk(Renderer* self, const s32 x, const s32 y, const s32 x_end, const s32 y_end, const s32 r2, const s32 radius, const Color& color, const s32 startRow, const s32 endRow) {
                s32 x_left = x + radius;
                s32 x_right = x_end - radius;
                s32 y_top = y + radius;
                s32 y_bottom = y_end - radius;
                
                // Process the rectangle in chunks
                for (s32 y1 = startRow; y1 < endRow; ++y1) {
                    for (s32 x1 = x; x1 < x_end; ++x1) {
                        //bool isInCorner = false;
            
                        if (x1 < x_left) {
                            // Left side
                            if (y1 < y_top) {
                                // Top-left corner
                                s32 dx = x_left - x1;
                                s32 dy = y_top - y1;
                                if (dx * dx + dy * dy <= r2) {
                                    self->setPixelBlendDst(x1, y1, color);
                                }
                            } else if (y1 >= y_bottom) {
                                // Bottom-left corner
                                s32 dx = x_left - x1;
                                s32 dy = y1 - y_bottom;
                                if (dx * dx + dy * dy <= r2) {
                                    self->setPixelBlendDst(x1, y1, color);
                                }
                            } else {
                                // Left side of the rectangle
                                self->setPixelBlendDst(x1, y1, color);
                            }
                        } else if (x1 >= x_right) {
                            // Right side
                            if (y1 < y_top) {
                                // Top-right corner
                                s32 dx = x1 - x_right;
                                s32 dy = y_top - y1;
                                if (dx * dx + dy * dy <= r2) {
                                    self->setPixelBlendDst(x1, y1, color);
                                }
                            } else if (y1 >= y_bottom) {
                                // Bottom-right corner
                                s32 dx = x1 - x_right;
                                s32 dy = y1 - y_bottom;
                                if (dx * dx + dy * dy <= r2) {
                                    self->setPixelBlendDst(x1, y1, color);
                                }
                            } else {
                                // Right side of the rectangle
                                self->setPixelBlendDst(x1, y1, color);
                            }
                        } else {
                            self->setPixelBlendDst(x1, y1, color);
                        }
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
                s32 x_end = x + w;
                s32 y_end = y + h;
                s32 r2 = radius * radius;
            
                // Reuse threads and implement work-stealing
                currentRow = y;
            
                auto threadTask = [this, x, y, x_end, y_end, r2, radius, color]() {
                    s32 startRow;
                    while (true) {
                        startRow = currentRow.fetch_add(4);
                        if (startRow >= y_end) break;
                        //s32 endRow = std::min(startRow + 4,y_end); // Process one row at a time
                        processRoundedRectChunk(this, x, y, x_end, y_end, r2, radius, color, startRow, std::min(startRow + 4,y_end));
                    }
                };
            
                // Launch threads
                for (unsigned i = 0; i < numThreads; ++i) {
                    threads[i] = std::thread(threadTask);
                }
            
                // Join threads
                for (auto& t : threads) {
                    if (t.joinable()) t.join();
                }
            }


            inline void drawRoundedRectSingleThreaded(const s32 x, const s32 y, const s32 w, const s32 h, const s32 radius, const Color& color) {
                s32 x_end = x + w;
                s32 y_end = y + h;
                s32 r2 = radius * radius;
                
                // Call the processRoundedRectChunk function directly for the entire rectangle
                processRoundedRectChunk(this, x, y, x_end, y_end, r2, radius, color, y, y_end);
            }

            std::function<void(s32, s32, s32, s32, s32, Color)> drawRoundedRect;
            inline void updateDrawFunction() {
                if (expandedMemory) {
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
                // Radius is half of height to create perfect half circles on each side
                s32 radius = h / 2;
                s32 x_start = x + radius;
                s32 x_end = x + w - radius;
            
                // Draw the central rectangle excluding the corners
                for (s32 y1 = y; y1 < y + h; ++y1) {
                    for (s32 x1 = x_start; x1 < x_end; ++x1) {
                        this->setPixelBlendDst(x1, y1, color);
                    }
                }
            
                // Draw the rounded corners using trigonometric functions for smoothness
                for (s32 x1 = 0; x1 < radius; ++x1) {
                    for (s32 y1 = 0; y1 < h; ++y1) {
                        s32 dy = y1 - radius; // Offset from center of the circle
                        if (x1 * x1 + dy * dy <= radius * radius) {
                            // Left half-circle
                            this->setPixelBlendDst(x + radius - x1, y + y1, color);
                            // Right half-circle
                            this->setPixelBlendDst(x + w - radius + x1, y + y1, color);
                        }
                    }
                }
            }


            
            inline void processBMPChunk(const s32 x, const s32 y, const s32 screenW, const u8 *preprocessedData, const s32 startRow, const s32 endRow) {
                s32 bytesPerRow = screenW * 4;
                const s32 endX = screenW - 16;
                
                for (s32 y1 = startRow; y1 < endRow; ++y1) {
                    const u8 *rowPtr = preprocessedData + (y1 * bytesPerRow);
                    s32 x1 = 0;
                    
                    // Process in chunks of 16 pixels using SIMD
                    for (; x1 <= endX; x1 += 16) {
                        uint8x16x4_t pixelData = vld4q_u8(rowPtr + (x1 * 4));
                        
                        // Unroll the loop to reduce the number of operations
                        for (s32 i = 0; i < 16; ++i) {
                            setPixelBlendSrc(x + x1 + i, y + y1, {
                                pixelData.val[0][i],
                                pixelData.val[1][i],
                                pixelData.val[2][i],
                                pixelData.val[3][i]
                            });
                        }
                    }
            
                    // Handle the remaining pixels (less than 16)
                    for (s32 xRem = x1; xRem < screenW; ++xRem) {
                        const u8 *p = rowPtr + (xRem * 4);
                        setPixelBlendSrc(x + xRem, y + y1, {p[0], p[1], p[2], p[3]});
                    }
                }
                //inPlot.store(false, std::memory_order_release);
                inPlotBarrier.arrive_and_wait(); // Wait for all threads to reach this point
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
                // Number of threads to use
                //const unsigned numThreads = 3;
                //std::vector<std::thread> threads(numThreads);

                // Divide rows among threads
                //s32 chunkSize = (screenH + numThreads - 1) / numThreads;
                for (unsigned i = 0; i < numThreads; ++i) {
                    s32 startRow = i * bmpChunkSize;
                    s32 endRow = std::min(startRow + bmpChunkSize, screenH);
                    
                    // Bind the member function and create the thread
                    threads[i] = std::thread(std::bind(&tsl::gfx::Renderer::processBMPChunk, this, x, y, screenW, preprocessedData, startRow, endRow));
                }
            
                // Join all threads
                for (auto& t : threads) {
                    t.join();
                }
            }


            inline void drawWallpaper() {
                if (expandedMemory && !refreshWallpaper.load(std::memory_order_acquire)) {
                    //inPlot = true;
                    inPlot.store(true, std::memory_order_release);
                    //std::lock_guard<std::mutex> lock(wallpaperMutex);
                    if (!wallpaperData.empty()) {
                        // Draw the bitmap at position (0, 0) on the screen
                        if (!refreshWallpaper.load(std::memory_order_acquire))
                            drawBitmapRGBA4444(0, 0, 448, 720, wallpaperData.data());
                        else
                            inPlot.store(false, std::memory_order_release);
                    } else {
                        inPlot.store(false, std::memory_order_release);
                    }
                    //inPlot = false;
                }
            }

            // Method to draw clock, temperatures, and battery percentage
            void drawWidget() {
                // Draw clock if it's not hidden
                static timespec currentTime;
                static char timeStr[20]; // Allocate a buffer to store the time string
                size_t y_offset = 45;
        
                if (!(hideBattery && hidePCBTemp && hideSOCTemp && hideClock)) {
                    drawRect(245, 23, 1, 49, a(separatorColor));
                }

                if ((hideBattery && hidePCBTemp && hideSOCTemp) || hideClock) {
                    y_offset += 10;
                }

                clock_gettime(CLOCK_REALTIME, &currentTime);
                if (!hideClock) {
                    strftime(timeStr, sizeof(timeStr), datetimeFormat.c_str(), localtime(&currentTime.tv_sec));
                    localizeTimeStr(timeStr);
                    drawString(timeStr, false, tsl::cfg::FramebufferWidth - calculateStringWidth(timeStr, 20, true) - 20, y_offset, 20, a(clockColor));
                    y_offset += 22;
                }
        
                // Draw temperatures and battery percentage
                static char PCB_temperatureStr[10];
                static char SOC_temperatureStr[10];
                static char chargeString[6];
        
                size_t statusChange = size_t(hideSOCTemp) + size_t(hidePCBTemp) + size_t(hideBattery);
                static size_t lastStatusChange = 0;
                
                if ((currentTime.tv_sec - timeOut) >= 1 || statusChange != lastStatusChange) {
                    if (!hideSOCTemp) {
                        ReadSocTemperature(&SOC_temperature);
                        snprintf(SOC_temperatureStr, sizeof(SOC_temperatureStr) - 1, "%d°C", static_cast<int>(round(SOC_temperature)));
                    } else {
                        strcpy(SOC_temperatureStr, "");
                        SOC_temperature=0;
                    }
                    if (!hidePCBTemp) {
                        ReadPcbTemperature(&PCB_temperature);
                        snprintf(PCB_temperatureStr, sizeof(PCB_temperatureStr) - 1, "%d°C", static_cast<int>(round(PCB_temperature)));
                    } else {
                        strcpy(PCB_temperatureStr, "");
                        PCB_temperature=0;
                    }
                    if (!hideBattery) {
                        powerGetDetails(&batteryCharge, &isCharging);
                        batteryCharge = std::min(batteryCharge, 100U);
                        sprintf(chargeString, "%d%%", batteryCharge);
                    } else {
                        strcpy(chargeString, "");
                        batteryCharge=0;
                    }
                    timeOut = int(currentTime.tv_sec);
                }
                
                lastStatusChange = statusChange;
                
                // Draw battery percentage
                if (!hideBattery && batteryCharge > 0) {
                    Color batteryColorToUse = isCharging ? tsl::Color(0x0, 0xF, 0x0, 0xF) : 
                                            (batteryCharge < 20 ? tsl::Color(0xF, 0x0, 0x0, 0xF) : batteryColor);
                    drawString(chargeString, false, tsl::cfg::FramebufferWidth - calculateStringWidth(chargeString, 20, true) - 22, y_offset, 20, a(batteryColorToUse));
                }
        
                // Draw PCB and SOC temperatures
                int offset = 0;
                if (!hidePCBTemp && PCB_temperature > 0) {
                    if (!hideBattery)
                        offset -= 5;
                    drawString(PCB_temperatureStr, false, tsl::cfg::FramebufferWidth + offset - calculateStringWidth(PCB_temperatureStr, 20, true) - calculateStringWidth(chargeString, 20, true) - 22, y_offset, 20, a(tsl::GradientColor(PCB_temperature)));
                }
        
                if (!hideSOCTemp && SOC_temperature > 0) {
                    if (!hidePCBTemp || !hideBattery)
                        offset -= 5;
                    drawString(SOC_temperatureStr, false, tsl::cfg::FramebufferWidth + offset - calculateStringWidth(SOC_temperatureStr, 20, true) - calculateStringWidth(PCB_temperatureStr, 20, true) - calculateStringWidth(chargeString, 20, true) - 22, y_offset, 20, a(tsl::GradientColor(SOC_temperature)));
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
            void drawBitmap(s32 x, s32 y, s32 w, s32 h, const u8 *bmp) {
                for (s32 y1 = 0; y1 < h; y1++) {
                    for (s32 x1 = 0; x1 < w; x1++) {
                        const Color color = { static_cast<u8>(bmp[0] >> 4), static_cast<u8>(bmp[1] >> 4), static_cast<u8>(bmp[2] >> 4), static_cast<u8>(bmp[3] >> 4) };
                        setPixelBlendSrc(x + x1, y + y1, a(color));
                        bmp += 4;
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
                this->fillScreen({ 0x00, 0x00, 0x00, 0x00 });
            }
            
            struct Glyph {
                stbtt_fontinfo *currFont;
                float currFontSize;
                int bounds[4];
                int xAdvance;
                u8 *glyphBmp;
                int width, height;
            };
            
            inline float calculateStringWidth(const std::string& str, const s32 fontSize, const bool fixedWidthNumbers = false) {
                if (str.empty()) {
                    return 0.0f;
                }
            
                float totalWidth = 0.0f;
                std::string::size_type strPos = 0;
                ssize_t codepointWidth;
                u32 prevCharacter = 0;
                u32 currCharacter = 0;
                float currFontSize;
                int xAdvance = 0, leftBearing = 0, kernAdvance = 0;
            
                // Cache default width for numeric characters if fixedWidthNumbers is true
                float numericCharWidth = fixedWidthNumbers ? defaultNumericCharWidth * fontSize : 0.0f;
            
                // Prepare the font information ahead of time if possible
                stbtt_fontinfo* currentFont = nullptr;
            
                // Use an iterator for characterWidths map lookup
                auto it = characterWidths.end();
            
                while (strPos < str.size()) {
                    codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&str[strPos]));
                    if (codepointWidth <= 0) {
                        break;
                    }
            
                    if (fixedWidthNumbers && std::isdigit(currCharacter)) {
                        totalWidth += numericCharWidth;
                    } else {
                        // Check if the character is found in the cache
                        it = characterWidths.find(static_cast<wchar_t>(currCharacter));
                        if (it != characterWidths.end()) {
                            totalWidth += it->second * fontSize;
                        } else {
                            if (!currentFont || !stbtt_FindGlyphIndex(currentFont, currCharacter)) {
                                if (stbtt_FindGlyphIndex(&this->m_extFont, currCharacter)) {
                                    currentFont = &this->m_extFont;
                                } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, currCharacter) == 0) {
                                    currentFont = &this->m_localFont;
                                } else {
                                    currentFont = &this->m_stdFont;
                                }
                            }
            
                            currFontSize = stbtt_ScaleForPixelHeight(currentFont, fontSize);
                            stbtt_GetCodepointHMetrics(currentFont, currCharacter, &xAdvance, &leftBearing);
            
                            if (prevCharacter) {
                                kernAdvance = stbtt_GetCodepointKernAdvance(currentFont, prevCharacter, currCharacter);
                                totalWidth += kernAdvance * currFontSize;
                            }
            
                            totalWidth += xAdvance * currFontSize;
                        }
                    }
            
                    strPos += codepointWidth;
                    prevCharacter = currCharacter;
                }
            
                return totalWidth;
            }






            /**
             * @brief Draws a string
             *
             * @param string String to draw
             * @param monospace Draw string in monospace font
             * @param x X pos
             * @param y Y pos
             * @param fontSize Height of the text drawn in pixels
             * @param color Text color. Use transparent color to skip drawing and only get the string's dimensions
             * @return Dimensions of drawn string
             */
            inline std::pair<u32, u32> drawString(const std::string& originalString, bool monospace, const s32 x, const s32 y, const s32 fontSize, const Color& color, const ssize_t maxWidth = 0) {
                float maxX = x;
                float currX = x;
                float currY = y;
                
                
                // Avoid copying the original string
                const std::string* stringPtr = &originalString;
                
                // Check if the string is the INPROGRESS_SYMBOL and replace with throbber symbol without copying
                if (originalString.size() == INPROGRESS_SYMBOL.size() && originalString == INPROGRESS_SYMBOL) {
                    // Static counter for the throbber symbols
                    static size_t throbberCounter = 0;

                    //size_t index = (throbberCounter / 10) % THROBBER_SYMBOLS.size();  // Change index every 10 counts
                    stringPtr = &THROBBER_SYMBOLS[(throbberCounter / 10) % THROBBER_SYMBOLS.size()];  // Point to the new string without copying
                    
                    throbberCounter++;

                    // Reset counter to prevent overflow after many cycles (ensures it never grows indefinitely)
                    if (throbberCounter >= 10 * THROBBER_SYMBOLS.size()) {
                        throbberCounter = 0;
                    }
                }
            
                // Cache the end iterator for efficiency
                auto itStrEnd = stringPtr->cend();
                auto itStr = stringPtr->cbegin();
                
                // Move variable declarations outside of the loop
                u32 currCharacter = 0;
                ssize_t codepointWidth = 0;
                u64 key = 0;
                Glyph* glyph = nullptr;
                
                float xPos = 0;
                float yPos = 0;
                u32 rowOffset = 0;
                uint8_t bmpColor = 0;
                Color tmpColor(0);
                
                // Static glyph cache
                static std::unordered_map<u64, Glyph> s_glyphCache; // may cause leak? will investigate later.
                auto it = s_glyphCache.end();

                float scaledFontSize;

                // Loop through each character in the string
                while (itStr != itStrEnd) {
                    if (maxWidth > 0 && (currX - x) >= maxWidth)
                        break;
            
                    // Decode UTF-8 codepoint
                    codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&(*itStr)));
                    if (codepointWidth <= 0)
                        break;
            
                    // Move the iterator forward by the width of the current codepoint
                    itStr += codepointWidth;
            
                    if (currCharacter == '\n') {
                        maxX = std::max(currX, maxX);
                        currX = x;
                        currY += fontSize;
                        continue;
                    }
            
                    // Calculate glyph key
                    key = (static_cast<u64>(currCharacter) << 32) | (static_cast<u64>(monospace) << 31) | (static_cast<u64>(std::bit_cast<u32>(fontSize)));
            
                    // Check cache for the glyph
                    it = s_glyphCache.find(key);
            
                    // If glyph not found, create and cache it
                    if (it == s_glyphCache.end()) {
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
            
                        s32 yAdvance = 0;
                        stbtt_GetCodepointHMetrics(glyph->currFont, monospace ? 'W' : currCharacter, &glyph->xAdvance, &yAdvance);
            
                        glyph->glyphBmp = stbtt_GetCodepointBitmap(glyph->currFont, scaledFontSize, scaledFontSize, currCharacter, &glyph->width, &glyph->height, nullptr, nullptr);
                    } else {
                        glyph = &it->second;
                    }
            
                    if (glyph->glyphBmp != nullptr && !std::iswspace(currCharacter) && fontSize > 0 && color.a != 0x0) {
                        xPos = currX + glyph->bounds[0];
                        yPos = currY + glyph->bounds[1];
            
                        // Optimized pixel processing
                        for (s32 bmpY = 0; bmpY < glyph->height; ++bmpY) {
                            rowOffset = bmpY * glyph->width;
                            for (s32 bmpX = 0; bmpX < glyph->width; ++bmpX) {
                                bmpColor = glyph->glyphBmp[rowOffset + bmpX] >> 4;
                                if (bmpColor == 0xF) {
                                    // Direct pixel manipulation
                                    this->setPixel(xPos + bmpX, yPos + bmpY, color, this->getPixelOffset(xPos + bmpX, yPos + bmpY));
                                } else if (bmpColor != 0x0) {
                                    tmpColor = color;
                                    tmpColor.a = bmpColor;
                                    this->setPixelBlendDst(xPos + bmpX, yPos + bmpY, tmpColor);
                                }
                            }
                        }
                    }
            
                    // Advance the cursor for the next glyph
                    currX += static_cast<s32>(glyph->xAdvance * glyph->currFontSize);
                }
            
                maxX = std::max(currX, maxX);
                return { static_cast<u32>(maxX - x), static_cast<u32>(currY - y) };
            }
            

            

            
            
            inline void drawStringWithColoredSections(const std::string& text, const std::vector<std::string>& specialSymbols, s32 x, const s32 y, const u32 fontSize, const Color& defaultColor, const Color& specialColor) {
                size_t startPos = 0;
                size_t textLength = text.length();
                u32 segmentWidth, segmentHeight;
            
                // Create a set for fast symbol lookup
                std::unordered_set<std::string> specialSymbolSet(specialSymbols.begin(), specialSymbols.end());
            
                // Variables initialized outside the loop
                size_t specialPos = std::string::npos;
                size_t foundLength = 0;
                std::string_view foundSymbol;
                std::string normalTextStr; // To hold the text before the special symbol
                std::string specialSymbolStr; // To hold the special symbol text
                size_t pos; // To store position of the special symbol in the text
            
                while (startPos < textLength) {
                    specialPos = std::string::npos;
                    foundLength = 0;
                    foundSymbol = std::string_view(); // Reset the foundSymbol
            
                    // Find the nearest special symbol
                    for (const auto& symbol : specialSymbolSet) {
                        pos = text.find(symbol, startPos);
                        if (pos != std::string::npos && (specialPos == std::string::npos || pos < specialPos)) {
                            specialPos = pos;
                            foundLength = symbol.length();
                            foundSymbol = symbol;
                        }
                    }
            
                    // If no special symbol is found, draw the rest of the text
                    if (specialPos == std::string::npos) {
                        drawString(text.substr(startPos), false, x, y, fontSize, defaultColor);
                        break;
                    }
            
                    // Draw the segment before the special symbol
                    if (specialPos > startPos) {
                        normalTextStr = text.substr(startPos, specialPos - startPos);
                        std::tie(segmentWidth, segmentHeight) = drawString(normalTextStr, false, x, y, fontSize, defaultColor);
                        //segmentWidth = calculateStringWidth(normalTextStr, fontSize);
                        x += segmentWidth;
                    }
            
                    // Draw the special symbol
                    specialSymbolStr = foundSymbol; // Convert std::string_view to std::string
                    std::tie(segmentWidth, segmentHeight) = drawString(specialSymbolStr, false, x, y, fontSize, specialColor);
                    //segmentWidth = calculateStringWidth(specialSymbolStr, fontSize);
                    x += segmentWidth;
            
                    // Move startPos past the special symbol
                    startPos = specialPos + foundLength;
                }
            
                // Draw any remaining text after the last special symbol
                if (startPos < textLength) {
                    drawString(text.substr(startPos), false, x, y, fontSize, defaultColor);
                }
            }

            
            /**
             * @brief Limit a string's length and end it with "…"
             *
             * @param string String to truncate
             * @param monospace Whether the font is monospace
             * @param fontSize Size of the font
             * @param maxLength Maximum length of the string in terms of width
             */
            inline std::string limitStringLength(const std::string& string, const bool monospace, const s32 fontSize, const s32 maxLength) {
                if (string.size() < 2) {
                    return string;
                }
                
                s32 currX = 0;
                ssize_t strPos = 0;
                ssize_t codepointWidth;
                u32 ellipsisCharacter = 0x2026;  // Unicode code point for '…'
                s32 ellipsisWidth;
                
                // Calculate the width of the ellipsis
                stbtt_fontinfo* ellipsisFont = &this->m_stdFont;
                if (stbtt_FindGlyphIndex(&this->m_extFont, ellipsisCharacter)) {
                    ellipsisFont = &this->m_extFont;
                } else if (this->m_hasLocalFont && stbtt_FindGlyphIndex(&this->m_stdFont, ellipsisCharacter) == 0) {
                    ellipsisFont = &this->m_localFont;
                }
                float ellipsisFontSize = stbtt_ScaleForPixelHeight(ellipsisFont, fontSize);
                int ellipsisXAdvance = 0, ellipsisYAdvance = 0;
                stbtt_GetCodepointHMetrics(ellipsisFont, ellipsisCharacter, &ellipsisXAdvance, &ellipsisYAdvance);
                ellipsisWidth = static_cast<s32>(ellipsisXAdvance * ellipsisFontSize);

                u32 currCharacter;
                std::string substr;

                while (static_cast<std::string::size_type>(strPos) < string.size() && currX + ellipsisWidth < maxLength) {
                    
                    codepointWidth = decode_utf8(&currCharacter, reinterpret_cast<const u8*>(&string[strPos]));
                    
                    if (codepointWidth <= 0) {
                        break;
                    }
                    
                    // Calculate the width of the current substring plus the ellipsis
                    substr = string.substr(0, strPos + codepointWidth);
                    currX = calculateStringWidth(substr, fontSize, monospace);
                    
                    if (currX + ellipsisWidth >= maxLength) {
                        return substr + "…";
                    }
                    
                    strPos += codepointWidth;
                }
                
                return string;
            }

            
        private:
            Renderer() {
                updateDrawFunction();
            }
            

            /**
             * @brief Gets the renderer instance
             *
             * @return Renderer
             */
            inline static Renderer& get() {
                static Renderer renderer;
                
                return renderer;
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
            
            stbtt_fontinfo m_stdFont, m_localFont, m_extFont;
            bool m_hasLocalFont = false;
            
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
            inline u32 getPixelOffset(const s32 x, const s32 y) {
                // Check for scissoring boundaries
                if (!this->m_scissoringStack.empty()) {
                    const auto& currScissorConfig = this->m_scissoringStack.top();
                    if (x < currScissorConfig.x || y < currScissorConfig.y || 
                        x >= currScissorConfig.x + currScissorConfig.w || 
                        y >= currScissorConfig.y + currScissorConfig.h) {
                        return UINT32_MAX;
                    }
                }
            
                // Calculate the base offset
                //tmpPos = (((y & 127) / 16) + ((x / 32) * 8) + ((y / 128) * (cfg::FramebufferWidth / 4)))*1024;
                return (((y & 127) / 16) + ((x / 32) * 8) + ((y / 128) * 112))*512 +
                        ((y % 16) / 8) * 256 + 
                        ((x % 32) / 16) * 128 + 
                        ((y % 8) / 2) * 32 + 
                        ((x % 16) / 8) * 16 + 
                        (y % 2) * 8 + 
                        (x % 8);
                //tmpPos *= 1024; // 16 * 16 * 4 = 1024
            
                // Calculate the fine offset and add it to the base offset
                //tmpPos += ((y % 16) / 8) * 512 + 
                //          ((x % 32) / 16) * 256 + 
                //          ((y % 8) / 2) * 64 + 
                //          ((x % 16) / 8) * 32 + 
                //          (y % 2) * 16 + 
                //          (x % 8) * 2;
            
                //return tmpPos;
            }

            

            std::pair<int, int> getUnderscanPixels() {
                if (!consoleIsDocked()) {
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
                int originalWidth = cfg::ScreenWidth;
                int originalHeight = cfg::ScreenHeight;
            
                // Adjust the width and height based on the underscan percentage
                int adjustedWidth = static_cast<int>(originalWidth * underscanPercentage);
                int adjustedHeight = static_cast<int>(originalHeight * underscanPercentage);
                
                // Calculate the underscan in pixels (left/right and top/bottom)
                int horizontalUnderscanPixels = (originalWidth - adjustedWidth) / 2;
                int verticalUnderscanPixels = (originalHeight - adjustedHeight) / 2;
                
                return {horizontalUnderscanPixels, verticalUnderscanPixels};
            }



            
            /**
             * @brief Initializes the renderer and layers
             *
             */
            void init() {
                // Get the underscan pixel values for both horizontal and vertical borders
                auto [horizontalUnderscanPixels, verticalUnderscanPixels] = getUnderscanPixels();
                //int horizontalUnderscanPixels = 0;

                useRightAlignment = (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "right_alignment") == TRUE_STR);
                //cfg::LayerPosX = 1280-32;
                if (useRightAlignment) {
                    cfg::LayerPosX = 1280-32 - horizontalUnderscanPixels;
                    layerEdge = (1280-448);
                }
                cfg::LayerPosY = 0;
                cfg::FramebufferWidth  = 448;
                cfg::FramebufferHeight = 720;
                cfg::LayerWidth  = cfg::ScreenHeight * (float(cfg::FramebufferWidth) / float(cfg::FramebufferHeight));
                cfg::LayerHeight = cfg::ScreenHeight;

                // Apply underscanning offset
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
                        if (R_SUCCEEDED(viGetZOrderCountMax(&this->m_display, &layerZ)) && layerZ > 0)
                            ASSERT_FATAL(viSetLayerZ(&this->m_layer, layerZ));
                        else ASSERT_FATAL(viSetLayerZ(&this->m_layer, 255)); // max value 255 as fallback
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
                this->waitForVSync();
                framebufferEnd(&this->m_framebuffer);
                
                this->m_currentFramebuffer = nullptr;
            }
        };
        
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
            

            std::chrono::duration<long int, std::ratio<1, 1000000000>> t;
            //double timeCounter;
            u8 saturation;
            //Color animColor = tsl::style::color::ColorClickAnimation;
            float progress;

            //Color clickColor1 = Color(0);
            //Color clickColor2 = Color(0);
            
            s32 x, y;
            s32 amplitude;
            std::chrono::steady_clock::time_point m_animationStartTime; // Start time of the animation
            
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
                    renderer->enableScissoring(0, activeHeaderHeight, tsl::cfg::FramebufferWidth, tsl::cfg::FramebufferHeight-73-activeHeaderHeight);
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
                this->m_highlightShakingStartTime = std::chrono::steady_clock::now();
            }
            
            /**
             * @brief Triggers the blue click animation to signal a element has been clicked on
             *
             */
            void inline triggerClickAnimation() {
                this->m_clickAnimationProgress = tsl::style::ListItemHighlightLength;
                this->m_animationStartTime = std::chrono::steady_clock::now();
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
                    renderer->drawRect(this->getX() + x + 4, this->getY() + y, this->getWidth() - 12, this->getHeight(), a(selectionBGColor)); // CUSTOM MODIFICATION 

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
                
                //half progress = half((std::sin(2.0 * _M_PI * fmod(std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count(), 1.0)) + 1.0) / 2.0);
                progress = (std::sin(2.0 * _M_PI * fmod(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(), 1.0)) + 1.0) / 2.0;
                
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
                    t = (std::chrono::steady_clock::now() - this->m_highlightShakingStartTime);
                    if (t >= 100ms)
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t, amplitude);
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
        
                    // Calculate time elapsed since the animation started
                    //auto now = std::chrono::steady_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->m_animationStartTime).count();
        
                    // Decrease progress based on the elapsed time (1 second = 1000ms)
                    this->m_clickAnimationProgress = tsl::style::ListItemHighlightLength * (1.0f - (elapsed / 500.0f)); // Progress decreases linearly over 1 second (1000ms)
        
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
                
                
                progress = ((std::sin(2.0 * _M_PI * fmod(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(), 1.0)) + 1.0) / 2.0);
                if (runningInterpreter.load(std::memory_order_acquire)) {
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
                    t = (std::chrono::steady_clock::now() - this->m_highlightShakingStartTime);
                    if (t >= 100ms)
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t, amplitude);
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

                    // Determine the active percentage to use
                    float activePercentage = 0.0f;
                    if (downloadPercentage > 0) {
                        activePercentage = downloadPercentage;
                    } else if (unzipPercentage > 0) {
                        activePercentage = unzipPercentage;
                    } else if (copyPercentage > 0) {
                        activePercentage = copyPercentage;
                    }
                    if (activePercentage > 0){
                        renderer->drawRect(this->getX() + x + 4, this->getY() + y, (this->getWidth()- 12 +4)*(activePercentage/100.0f), this->getHeight(), a(progressColor));
                        //if (copyPercentage == 100.0f) {
                        //    copyPercentage = -1;
                        //}
                    }

                    renderer->drawBorderedRoundedRect(this->getX() + x, this->getY() + y, this->getWidth() +4, this->getHeight(), 5, 5, a(highlightColor));
                }
                //renderer->drawRect(ELEMENT_BOUNDS(this), a(0xF000)); // This has been moved here (needs to be toggleable)
                onTrackBar = false;
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
                //static u32 layerEdge = cfg::LayerPosX == 0 ? 0 : (1280-448);
                return touchX >= this->getLeftBound() + int(layerEdge) && touchX <= this->getRightBound() + int(layerEdge) && touchY >= this->getTopBound() && touchY <= this->getBottomBound();
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

            ///**
            // * @brief Adds a child element to this element
            // *
            // * @param child Child element to add
            // */
            //void addChild(Element* child) {
            //    // Set the parent of the child to this element
            //    child->setParent(this);
            //    // Add the child to the list of children
            //    m_children.push_back(child);
            //}
            //
            ///**
            // * @brief Get the list of child elements
            // *
            // * @return Vector of child elements
            // */
            //const std::vector<Element*>& getChildren() const { return this->m_children; }


            /**
             * @brief Marks this element as focused or unfocused to draw the highlight
             *
             * @param focused Focused
             */
            virtual inline void setFocused(bool focused) {
                this->m_focused = focused;
                this->m_clickAnimationProgress = 0;
            }
            
            
            static InputMode getInputMode() { return Element::s_inputMode; }
            
            static void setInputMode(InputMode mode) { Element::s_inputMode = mode; }
            
        protected:
            constexpr static inline auto a = &gfx::Renderer::a;
            bool m_focused = false;
            u8 m_clickAnimationProgress = 0;
            
            // Highlight shake animation
            bool m_highlightShaking = false;
            std::chrono::steady_clock::time_point m_highlightShakingStartTime;
            FocusDirection m_highlightShakingDirection;
            
            static inline InputMode s_inputMode;
            
            /**
             * @brief Shake animation callculation based on a damped sine wave
             *
             * @param t Passed time
             * @param a Amplitude
             * @return Damped sine wave output
             */
            inline int shakeAnimation(std::chrono::steady_clock::duration t, float a) {
                float w = 0.2F;
                float tau = 0.05F;
                
                int t_ = t.count() / 1'000'000;
                
                return roundf(a * exp(-(tau * t_) * sin(w * t_)));
            }
            
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
            }

            virtual ~CustomDrawer() {}
            
            virtual void draw(gfx::Renderer* renderer) override {
                renderer->enableScissoring(ELEMENT_BOUNDS(this));
                this->m_renderFunc(renderer, ELEMENT_BOUNDS(this));
                renderer->disableScissoring();
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                
            }
            
        private:
            std::function<void(gfx::Renderer*, s32 x, s32 y, s32 w, s32 h)> m_renderFunc;
        };


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
            std::string m_menuMode; // CUSTOM MODIFICATION
            std::string m_colorSelection; // CUSTOM MODIFICATION
            std::string m_pageLeftName; // CUSTOM MODIFICATION
            std::string m_pageRightName; // CUSTOM MODIFICATION
            bool m_noClickableItems;

            //tsl::Color handColor = RGB888("#F7253E");
            tsl::Color titleColor = {0xF,0xF,0xF,0xF};
            const double cycleDuration = 1.5;
            float counter = 0;
            float countOffset;
            double timeInSeconds;
            float progress;
            float letterWidth;
            float x, y;
            int offset, y_offset;
            int fontSize;

            std::string menuBottomLine;
            
        OverlayFrame(const std::string& title, const std::string& subtitle, const std::string& menuMode = "", const std::string& colorSelection = "", const std::string& pageLeftName = "", const std::string& pageRightName = "", const bool& _noClickableItems=false)
            : Element(), m_title(title), m_subtitle(subtitle), m_menuMode(menuMode), m_colorSelection(colorSelection), m_pageLeftName(pageLeftName), m_pageRightName(pageRightName), m_noClickableItems(_noClickableItems) {
                activeHeaderHeight = 97;
                // Load the bitmap file into memory
                if (expandedMemory && !inPlot.load(std::memory_order_acquire) && !refreshWallpaper.load(std::memory_order_acquire)) {
                    // Lock the mutex for condition waiting
                    std::unique_lock<std::mutex> lock(wallpaperMutex);

                    // Wait for inPlot to be false before reloading the wallpaper
                    cv.wait(lock, [] { return (!inPlot.load(std::memory_order_acquire) && !refreshWallpaper.load(std::memory_order_acquire)); });

                    if (wallpaperData.empty() && isFileOrDirectory(WALLPAPER_PATH)) {
                        loadWallpaperFile(WALLPAPER_PATH);
                    }
                }

                m_isItem = false;
            }

            virtual ~OverlayFrame() {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
            }

            
            // Function to calculate FPS
            //void updateFPS(double currentTimeCount) {
            //    static double lastUpdateTime = currentTimeCount;
            //
            //    frameCount++;
            //    elapsedTime = currentTimeCount - lastUpdateTime;
            //
            //    if (elapsedTime >= 1.0) { // Update FPS every second
            //        fps = frameCount / static_cast<float>(elapsedTime);
            //        lastUpdateTime = currentTimeCount;
            //        frameCount = 0;
            //    }
            //}
            
            // CUSTOM SECTION START
            virtual void draw(gfx::Renderer *renderer) override {
                if (!themeIsInitialized) {
                    tsl::initializeThemeVars(); // Initialize variables for ultrahand themes
                    themeIsInitialized = true;
                }

                if (m_noClickableItems != noClickableItems)
                    noClickableItems = m_noClickableItems;
                renderer->fillScreen(a(defaultBackgroundColor));
                
                renderer->drawWallpaper();
                

                y = 50;
                offset = 0;
                
                #if IS_LAUNCHER_DIRECTIVE
                bool isUltrahand = (this->m_title == CAPITAL_ULTRAHAND_PROJECT_NAME && 
                                    this->m_subtitle.find("Ultrahand Package") == std::string::npos && 
                                    this->m_subtitle.find("Ultrahand Script") == std::string::npos);

                if (isUltrahand) {
                    
                    // Call the extracted widget drawing method
                    renderer->drawWidget();


                    if (touchingMenu && inMainMenu) {
                        renderer->drawRoundedRect(0.0f, 12.0f, 245.0f, 73.0f, 6.0f, a(clickColor));
                    }
                    
                    
                    x = 20;
                    fontSize = 42;
                    offset = 6;
                    
                    countOffset = 0;
                    

                    if (!disableColorfulLogo) {
                        //auto currentTime = std::chrono::steady_clock::now();
                        auto currentTimeCount = std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count();
                        float progress;

                        for (char letter : SPLIT_PROJECT_NAME_1) {
                            counter = (2 * _M_PI * (fmod(currentTimeCount, cycleDuration) + countOffset) / 1.5);
                            progress = std::sin(counter); // -1 to 1
                            
                            highlightColor = {
                                static_cast<u8>((std::get<0>(dynamicLogoRGB2) - std::get<0>(dynamicLogoRGB1)) * (progress + 1.0) / 2.0 + std::get<0>(dynamicLogoRGB1)),
                                static_cast<u8>((std::get<1>(dynamicLogoRGB2) - std::get<1>(dynamicLogoRGB1)) * (progress + 1.0) / 2.0 + std::get<1>(dynamicLogoRGB1)),
                                static_cast<u8>((std::get<2>(dynamicLogoRGB2) - std::get<2>(dynamicLogoRGB1)) * (progress + 1.0) / 2.0 + std::get<2>(dynamicLogoRGB1)),
                                15
                            };
                            
                            renderer->drawString(std::string(1, letter), false, x, y + offset, fontSize, a(highlightColor));
                            x += renderer->calculateStringWidth(std::string(1, letter), fontSize);
                            countOffset -= 0.2F;
                        }
                    } else {
                        for (char letter : SPLIT_PROJECT_NAME_1) {
                            renderer->drawString(std::string(1, letter), false, x, y + offset, fontSize, a(logoColor1));
                            x += renderer->calculateStringWidth(std::string(1, letter), fontSize);
                            countOffset -= 0.2F;
                        }
                    }
                    
                    renderer->drawString(SPLIT_PROJECT_NAME_2, false, x, y + offset, fontSize, a(logoColor2));
                    
                } else {
                    x = 20;
                    y = 52;
                    fontSize = 32;
                    if (this->m_subtitle.find("Ultrahand Package") != std::string::npos) {
                        const std::string& title = this->m_title;
                        titleColor = defaultPackageColor; // Default to green
                        
                        // Function to draw the title
                        auto drawTitle = [&](const Color& color) {
                            renderer->drawString(title, false, x, y, fontSize, a(color));
                        };
                        
                        if (this->m_colorSelection == "green") {
                            titleColor = Color(0x0, 0xF, 0x0, 0xF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "red") {
                            titleColor = RGB888("#F7253E");
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "blue") {
                            titleColor = Color(0x7, 0x7, 0xF, 0xF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "yellow") {
                            titleColor = Color(0xF, 0xF, 0x0, 0xF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "orange") {
                            titleColor = Color(0xFF, 0xA5, 0x00, 0xFF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "pink") {
                            titleColor = Color(0xFF, 0x69, 0xB4, 0xFF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "purple") {
                            titleColor = Color(0x80, 0x00, 0x80, 0xFF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "white") {
                            titleColor = Color(0xF, 0xF, 0xF, 0xF);
                            drawTitle(titleColor);
                        } else if (this->m_colorSelection == "ultra") {
                            for (char letter : title) {
                                // Calculate the progress for each letter based on the counter
                                progress = calculateAmplitude(counter - x * 0.0001F);
                                
                                // Calculate the corresponding highlight color for each letter
                                highlightColor = {
                                    static_cast<u8>((0xA - 0xF) * (3 - 1.5 * progress) + 0xF),
                                    static_cast<u8>((0xA - 0xF) * 1.5 * progress + 0xF),
                                    static_cast<u8>((0xA - 0xF) * (1.25 - progress) + 0xF),
                                    0xF
                                };
                                
                                // Draw each character with its corresponding highlight color
                                renderer->drawString(std::string(1, letter).c_str(), false, x, y, fontSize, a(highlightColor));
                                
                                // Manually calculate the width of the current letter
                                letterWidth = renderer->calculateStringWidth(std::string(1, letter), fontSize);
                                
                                // Adjust the x-coordinate for the next character's position
                                x += letterWidth;
                                
                                // Update the counter for the next character
                                counter -= 0.00004F;
                            }
                        } else if (this->m_colorSelection.size() == 7 && this->m_colorSelection[0] == '#') {
                            // Check if m_colorSelection is a valid hexadecimal color
                            if (isValidHexColor(this->m_colorSelection.substr(1))) {
                                titleColor = RGB888(this->m_colorSelection.substr(1));
                                drawTitle(titleColor);
                            } else {
                                // Invalid hexadecimal color, handle the error accordingly
                                drawTitle(titleColor); // Using the default titleColor
                            }
                        } else { // for unknown colors
                            drawTitle(titleColor); // Using the default titleColor
                        }
                    } else if (this->m_subtitle.find("Ultrahand Script") != std::string::npos) {
                        renderer->drawString(this->m_title, false, x, y, fontSize, a(Color(0xFF, 0x33, 0x3F, 0xFF)));
                    } else {
                        renderer->drawString(this->m_title, false, x, y, fontSize, a(defaultTextColor));
                    }

                }
                
                if (this->m_title == CAPITAL_ULTRAHAND_PROJECT_NAME) {
                    renderer->drawString(versionLabel, false, 20, y+25, 15, a(versionTextColor));
                } else
                    renderer->drawString(this->m_subtitle, false, 20, y+23, 15, a(versionTextColor));

                #else
                renderer->drawString(this->m_title, false, 20, 50+2, 32, a(defaultTextColor));
                renderer->drawString(this->m_subtitle, false, 20, y+23, 15, a(versionTextColor));
                #endif


                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(botttomSeparatorColor));
                
                backWidth = renderer->calculateStringWidth(BACK, 23);
                if (touchingBack) {
                    renderer->drawRoundedRect(18.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              backWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }

                selectWidth = renderer->calculateStringWidth(OK, 23);
                if (touchingSelect && !m_noClickableItems) {
                    renderer->drawRoundedRect(18.0f + backWidth+68.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              selectWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }
                
                if (!(this->m_pageLeftName).empty())
                    nextPageWidth = renderer->calculateStringWidth(this->m_pageLeftName, 23);
                else if (!(this->m_pageRightName).empty())
                    nextPageWidth = renderer->calculateStringWidth(this->m_pageRightName, 23);
                #if IS_LAUNCHER_DIRECTIVE
                else if (inMainMenu)
                    if (inOverlaysPage)
                        nextPageWidth = renderer->calculateStringWidth(PACKAGES,23);
                    else if (inPackagesPage)
                        nextPageWidth = renderer->calculateStringWidth(OVERLAYS,23);
                #endif

                if (inMainMenu || !(this->m_pageLeftName).empty() || !(this->m_pageRightName).empty()) {
                    if (touchingNextPage) {
                        renderer->drawRoundedRect(18.0f + backWidth+68.0f + ((!m_noClickableItems) ? selectWidth+68.0f : 0), static_cast<float>(cfg::FramebufferHeight - 73), 
                                                  nextPageWidth+70.0f, 73.0f, 6.0f, a(clickColor));
                    }
                }


                if (m_noClickableItems)
                    menuBottomLine = "\uE0E1"+GAP_2+BACK+GAP_1;
                else
                    menuBottomLine = "\uE0E1"+GAP_2+BACK+GAP_1+"\uE0E0"+GAP_2+OK+GAP_1;

                #if IS_LAUNCHER_DIRECTIVE
                if (this->m_menuMode == "packages") {
                    menuBottomLine += "\uE0ED"+GAP_2+OVERLAYS;
                } else if (this->m_menuMode == "overlays") {
                    menuBottomLine += "\uE0EE"+GAP_2+PACKAGES;
                }
                #endif
                
                if (!(this->m_pageLeftName).empty()) {
                    menuBottomLine += "\uE0ED"+GAP_2 + this->m_pageLeftName;
                } else if (!(this->m_pageRightName).empty()) {
                    menuBottomLine += "\uE0EE"+GAP_2 + this->m_pageRightName;
                }
                
                //renderer->drawString(menuBottomLine.c_str(), false, 30, 693, 23, a(defaultTextColor));
                // Render the text with special character handling
                renderer->drawStringWithColoredSections(menuBottomLine, {"\uE0E1","\uE0E0","\uE0ED","\uE0EE"}, 30, 693, 23, a(bottomTextColor), a(buttonColor));
                
                //if (true) {
                //    // Update FPS
                //    updateFPS(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count());
                //
                //    // Convert FPS to string
                //    std::ostringstream fpsStream;
                //    fpsStream << std::fixed << std::setprecision(2) << "FPS: " << fps;
                //    std::string fpsString = fpsStream.str();
                //    
                //    // Draw FPS string at the bottom left corner
                //    renderer->drawString(fpsString, false, 20, tsl::cfg::FramebufferHeight - 60, 20, a(tsl::Color(0xFF, 0xFF, 0xFF, 0xFF))); // Adjust position and color as needed
                //    
                //    //svcGetSystemInfo(&RAM_Used_system_u, 1, INVALID_HANDLE, 2);
                //    //svcGetSystemInfo(&RAM_Total_system_u, 0, INVALID_HANDLE, 2);
                //    //
                //    //float RAM_Total_system_f = (float)RAM_Total_system_u / 1024 / 1024;
                //    //float RAM_Used_system_f = (float)RAM_Used_system_u / 1024 / 1024;
                //    //
                //    //// Convert RAM usage to strings
                //    //std::ostringstream ramStream;
                //    //ramStream << std::fixed << std::setprecision(2)
                //    //          << RAM_Total_system_f - RAM_Used_system_f - 8.0 << " MB free (8 MB reserved)";
                //    //std::string ramString = ramStream.str();
                //    //
                //    //
                //    //renderer->drawString(ramString.c_str(), false, 130, tsl::cfg::FramebufferHeight - 60, 20, a(tsl::Color(0xFF, 0xFF, 0xFF, 0xFF))); // Adjust position and color as needed
                //}

                
                if (this->m_contentElement != nullptr)
                    this->m_contentElement->frame(renderer);

            }
            // CUSTOM SECTION END

            
            virtual inline void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(parentX, parentY, parentWidth, parentHeight);
                
                if (this->m_contentElement != nullptr) {
                    this->m_contentElement->setBoundaries(parentX + 35, parentY + 97, parentWidth - 85, parentHeight - 73 - 105); // CUSTOM MODIFICATION (125->105->102)
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
                if (!this->m_contentElement->inBounds(currX, currY) || !internalTouchReleased)
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
        
        /**
         * @brief The base frame which can contain another view with a customizable header
         *
         */
        class HeaderOverlayFrame : public Element {
        public:
            

            HeaderOverlayFrame(u16 headerHeight = 175) : Element(), m_headerHeight(headerHeight) {
                activeHeaderHeight = headerHeight;
                // Load the bitmap file into memory
                if (expandedMemory && !inPlot.load(std::memory_order_acquire) && !refreshWallpaper.load(std::memory_order_acquire)) {
                    // Lock the mutex for condition waiting
                    std::unique_lock<std::mutex> lock(wallpaperMutex);

                    // Wait for inPlot to be false before reloading the wallpaper
                    cv.wait(lock, [] { return (!inPlot.load(std::memory_order_acquire) && !refreshWallpaper.load(std::memory_order_acquire)); });

                    if (wallpaperData.empty() && isFileOrDirectory(WALLPAPER_PATH)) {
                        loadWallpaperFile(WALLPAPER_PATH);
                    }
                }
                m_isItem = false;

            }
            virtual ~HeaderOverlayFrame() {
                if (this->m_contentElement != nullptr)
                    delete this->m_contentElement;
                
                if (this->m_header != nullptr)
                    delete this->m_header;
            }
            
            virtual void draw(gfx::Renderer *renderer) override {
                if (!themeIsInitialized) {
                    tsl::initializeThemeVars(); // Initialize variables for ultrahand themes
                    themeIsInitialized = true;
                }

                renderer->fillScreen(a(defaultBackgroundColor));

                renderer->drawWallpaper();

                //renderer->fillScreen(tsl::style::color::ColorFrameBackground);
                renderer->drawRect(tsl::cfg::FramebufferWidth - 1, 0, 1, tsl::cfg::FramebufferHeight, a(0xF222));
                
                renderer->drawRect(15, tsl::cfg::FramebufferHeight - 73, tsl::cfg::FramebufferWidth - 30, 1, a(botttomSeparatorColor));
                
                //renderer->drawString(("\uE0E1  "+BACK+"     \uE0E0  "+OK), false, 30, 693, 23, a(defaultTextColor)); // CUSTOM MODIFICATION
                
                backWidth = renderer->calculateStringWidth(BACK, 23);
                if (touchingBack) {
                    renderer->drawRoundedRect(18.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              backWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }

                selectWidth = renderer->calculateStringWidth(OK, 23);
                if (touchingSelect) {
                    renderer->drawRoundedRect(18.0f + backWidth+68.0f, static_cast<float>(cfg::FramebufferHeight - 73), 
                                              selectWidth+68.0f, 73.0f, 6.0f, a(clickColor));
                }

                std::string menuBottomLine = "\uE0E1"+GAP_2+BACK+GAP_1+"\uE0E0"+GAP_2+OK+GAP_1;
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


        /**
         * @brief A List containing list items
         *
         */
        class List : public Element {

        public:
            /**
             * @brief Constructor
             *
             */
            List() : Element() {
                m_isItem = false;
            }
            virtual ~List() {
                for (auto& item : this->m_items)
                    delete item;
                this->m_items.clear();
                this->m_offset = 0;
                this->m_focusedIndex = 0;
                this->invalidate();
                this->m_clearList = false;
            }
            
            u32 scrollbarHeight;
            u32 scrollbarOffset;
            u32 offset;
            u32 prevOffset;
            s32 y;
            bool handled;
            u16 i;

            std::chrono::steady_clock::time_point lastUpdateTime;
            std::chrono::duration<float> elapsed;

            const float smoothingFactor = 0.15f;  // Lower value means faster smoothing
            const float dampingFactor = 0.3f;   // Closer to 1 means slower damping


            bool isInTable = false;  // Track if we're inside a table
            bool inScrollMode = false;  // Track if we're in scroll mode after the table
            size_t tableIndex = 0;  // Keep track of the table index
            s32 entryOffset = 0;    // Store the entry point offset when entering the table
            
            // Use a vector to track how many downward scroll steps were taken for each table
            std::vector<int> scrollStepsInsideTable;  // This will track scroll steps for each table
            
            const float TABLE_SCROLL_STEP_SIZE = 40.0f; // Fixed scroll step size

            
            virtual void draw(gfx::Renderer* renderer) override {
                // Precompute frequently used values
                const u32 rightBound = this->getRightBound();
                const s32 topBound = this->getTopBound();
                const s32 bottomBound = this->getBottomBound();
                const s32 width = this->getWidth();
                const s32 height = this->getHeight();
                
            
                if (this->m_clearList) {
                    for (auto& item : this->m_items) {
                        delete item;
                    }
                    this->m_items.clear();
                    this->m_offset = 0;
                    this->m_focusedIndex = 0;
                    this->invalidate();
                    this->m_clearList = false;
                }
            
                if (!this->m_itemsToAdd.empty()) {
                    for (const auto& [index, element] : this->m_itemsToAdd) {
                        element->invalidate();
                        if (index >= 0 && index < static_cast<int>(this->m_items.size())) {
                            this->m_items.insert(this->m_items.cbegin() + index, element);
                        } else {
                            this->m_items.push_back(element);
                        }
                    }
                    this->m_itemsToAdd.clear();
                    this->invalidate();
                    this->updateScrollOffset();
                }
                
                if (!this->m_itemsToRemove.empty()) {
                    auto it = this->m_items.cend(); // Start with end iterator to ensure it's valid
                    for (auto* element : this->m_itemsToRemove) {
                        it = std::find(this->m_items.cbegin(), this->m_items.cend(), element);
                        if (it != this->m_items.cend()) {
                            this->m_items.erase(it);
                            if (this->m_focusedIndex >= static_cast<size_t>(it - this->m_items.cbegin())) {
                                this->m_focusedIndex--;
                            }
                            delete element;
                        }
                    }
                    this->m_itemsToRemove.clear();
                    this->invalidate();
                    this->updateScrollOffset();
                }
            
                renderer->enableScissoring(this->getLeftBound(), topBound, width + 4 +4, height + 4);
            
                for (auto& entry : this->m_items) {
                    if (entry->getBottomBound() > topBound && entry->getTopBound() < bottomBound) {
                        entry->frame(renderer);
                    }
                }
                
                renderer->disableScissoring();


                if (this->m_listHeight > height) {
                    const u32 viewHeight = height - 12;
                    const u32 totalHeight = this->m_listHeight + 24;
                    const u32 maxScrollableHeight = std::max(totalHeight - viewHeight, 1u);
                    
                    u32 scrollbarHeight = (viewHeight * viewHeight) / totalHeight;
                    scrollbarHeight = std::min(scrollbarHeight, viewHeight);
                    
                    u32 scrollbarOffset = (this->m_offset / maxScrollableHeight) * (viewHeight - scrollbarHeight);
                    scrollbarOffset = std::min(scrollbarOffset, viewHeight - scrollbarHeight) + 4;
            
                    //const u32 offset = 10;
                    const u32 scrollbarX = rightBound + 20;
                    const u32 scrollbarY = this->getY() + scrollbarOffset;
                    //const u32 scrollbarWidth = 5;
            
                    renderer->drawRect(scrollbarX, scrollbarY, 5, scrollbarHeight, a(trackBarColor));
                    renderer->drawCircle(scrollbarX + 2, scrollbarY, 2, true, a(trackBarColor));
                    renderer->drawCircle(scrollbarX + 2, scrollbarY + scrollbarHeight, 2, true, a(trackBarColor));
                    

                    if (Element::getInputMode() == InputMode::Controller) {
                        //static float lastOffset = 0.0f;
                        static float velocity = 0.0f;
                    
                        // Calculate the difference between the next and current offsets
                        //float deltaOffset = this->m_nextOffset - this->m_offset;
                    
                        // Apply smoothing to the velocity
                        velocity = velocity * dampingFactor + (this->m_nextOffset - this->m_offset) * smoothingFactor;
                    

                    
                        // If the velocity is small, snap to the target offset
                        if (std::abs(velocity) < 0.2f) {
                            this->m_offset = this->m_nextOffset;
                            velocity = 0.0f;
                        } else {
                            // Update the offset with the smoothed velocity
                            this->m_offset += velocity;
                        }
                    
                        // Update the last offset for the next frame
                        //lastOffset = this->m_offset;
                    

                    } else if (Element::getInputMode() == InputMode::TouchScroll) {
                        this->m_offset += ((this->m_nextOffset) - this->m_offset);
                    }
                    
                    if (prevOffset != this->m_offset) {
                        this->invalidate();
                    }
                    prevOffset = this->m_offset;
                }
            }

            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                y = this->getY() - this->m_offset;
                
                this->m_listHeight = 0;
                for (auto &entry : this->m_items)
                    this->m_listHeight += entry->getHeight();
                
                this->m_listHeight -= 32;
                for (auto &entry : this->m_items) {
                    entry->setBoundaries(this->getX(), y, this->getWidth(), entry->getHeight());
                    entry->invalidate();
                    y += entry->getHeight();
                }
                y -= 32;
            }
                                    
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) {
                handled = false;
                
                // Discard touches out of bounds
                if (!this->inBounds(currX, currY))
                    return false;
                
                // Direct touches to all children
                for (auto &item : this->m_items)
                    handled |= item->onTouch(event, currX, currY, prevX, prevY, initialX, initialY);
                
                if (handled)
                    return true;
                
                // Handle scrolling
                if (event != TouchEvent::Release && Element::getInputMode() == InputMode::TouchScroll) {
                    if (prevX != 0 && prevY != 0)
                        this->m_nextOffset += (prevY - currY);
                    
                    if (this->m_nextOffset < 0)
                        this->m_nextOffset = 0;
                    
                    if (this->m_nextOffset > (this->m_listHeight - this->getHeight()) + 50)
                        this->m_nextOffset = (this->m_listHeight - this->getHeight() + 50);
                    
                    return true;
                }
                
                return false;
            }
            

            
            /**
             * @brief Adds a new item to the list before the next frame starts
             *
             * @param element Element to add
             * @param index Index in the list where the item should be inserted. -1 or greater list size will insert it at the end
             * @param height Height of the element. Don't set this parameter for libtesla to try and figure out the size based on the type
             */
            inline void addItem(Element* element, u16 height = 0, ssize_t index = -1) {
                if (element != nullptr) {
                    // Check if the list is empty and the first element being added is a ListItem
                    if (actualItemCount == 0 && element->m_isItem) {
                        // Add a CustomDrawer to occupy the same space as an empty CategoryHeader
                        auto* customDrawer = new tsl::elm::CustomDrawer([](gfx::Renderer* r, s32 x, s32 y, s32 w, s32 h) {
                            // Leave empty to just occupy space without rendering anything
                        });
                        customDrawer->setBoundaries(this->getX(), this->getY(), this->getWidth(), tsl::style::ListItemDefaultHeight / 2); // Set same size as CategoryHeader
                        customDrawer->setParent(this);
                        customDrawer->invalidate();
                        this->m_itemsToAdd.emplace_back(-1, customDrawer); // Insert at the beginning
                    }

                    if (height != 0) {
                        element->setBoundaries(this->getX(), this->getY(), this->getWidth(), height);
                    }

                    element->setParent(this);
                    element->invalidate();

                    this->m_itemsToAdd.emplace_back(index, element);
                }
                actualItemCount++;
            }

            

            /**
             * @brief Removes an item form the list and deletes it
             * @note Item will only be deleted if it was found in the list
             *
             * @param element Element to remove from list. Call \ref Gui::removeFocus before.
             */
            virtual void removeItem(Element *element) {
                if (element != nullptr)
                    this->m_itemsToRemove.emplace_back(element);
            }
            
            /**
             * @brief Try to remove an item from the list
             *
             * @param index Index of element in list. Call \ref Gui::removeFocus before.
             */
            virtual void removeIndex(size_t index) {
                if (index < this->m_items.size())
                    removeItem(this->m_items[index]);
            }
            
            /**
             * @brief Removes all children from the list later on
             * @warning When clearing a list, make sure none of the its children are focused. Call \ref Gui::removeFocus before.
             */
            inline void clear() {
                this->m_clearList = true;
            }
            

            
            virtual Element* requestFocus(Element* oldFocus, FocusDirection direction) override {
                //disableLogging = false;
                if (this->m_clearList || !this->m_itemsToAdd.empty()) {
                    return nullptr;
                }
            
                Element* newFocus = nullptr;
            
                // Handle initial focus
                if (direction == FocusDirection::None) {
                    size_t i = 0;
                    if (oldFocus == nullptr) {
                        s32 elementHeight = 0;
                        while (elementHeight < this->m_offset && i < this->m_items.size() - 1) {
                            i++;
                            elementHeight += this->m_items[i]->getHeight();
                        }
                    }
                    
                    // Loop backwards from the current position to the start
                    for (ssize_t j = i; j >= 0; j--) {  // Use ssize_t to allow negative indexing check
                        newFocus = this->m_items[j]->requestFocus(oldFocus, direction);
                        if (newFocus != nullptr && newFocus != oldFocus) {  // Prevent re-focusing on the same element
                            this->m_focusedIndex = j;
                            this->updateScrollOffset();
                            isInTable = false;
                            inScrollMode = false;
                            //tableIndex = 0;
                            //entryOffset = 0;
                            //scrollStepsInsideTable.clear();  // Reset the scroll steps for all tables
                            //logMessage("Focus set to a new element. Reset scrollStepsInsideTable.");
                            return newFocus;
                        }
                    }
                
                    // If no new focus found, attempt forward traversal as a fallback
                    for (size_t k = i + 1; k < this->m_items.size(); ++k) {
                        newFocus = this->m_items[k]->requestFocus(oldFocus, direction);
                        if (newFocus != nullptr && newFocus != oldFocus) {  // Prevent re-focusing on the same element
                            this->m_focusedIndex = k;
                            this->updateScrollOffset();
                            isInTable = false;
                            inScrollMode = false;
                            //tableIndex = 0;
                            //entryOffset = 0;
                            //scrollStepsInsideTable.clear();  // Reset the scroll steps for all tables
                            //logMessage("Focus set to a new element. Reset scrollStepsInsideTable.");
                            return newFocus;
                        }
                    }
                }

                
                
                // Handle scrolling down
                else if (direction == FocusDirection::Down) {
                    if (this->m_items.empty()) {
                        // If there are no items, scroll directly
                        this->m_nextOffset = std::min(this->m_nextOffset + TABLE_SCROLL_STEP_SIZE, static_cast<float>(this->m_listHeight - this->getHeight() + 50));
                        this->m_offset = this->m_nextOffset;
                        this->invalidate();  // Redraw
                        return oldFocus;
                    }
                    
                    s32 accumulatedHeight = 0;

                    for (size_t i = this->m_focusedIndex + 1; i < this->m_items.size(); ++i) {
                        newFocus = this->m_items[i]->requestFocus(oldFocus, direction);
                        if (!isInTable && newFocus != nullptr && newFocus != oldFocus) {  // Only update focus if it's a new element
                            this->m_focusedIndex = i;
                            this->updateScrollOffset();
                            isInTable = false;
                            inScrollMode = false;
                            tableIndex = 0;
                            return newFocus;
                        }
                        if (!this->m_items[i]->isItem()) {
                            // Accumulate the heights of small tables to decide if they should be skipped
                            accumulatedHeight += this->m_items[i]->getHeight();
                        }
                        
                        if (this->m_items[i]->isTable()) {
                            // Check if the table is fully visible (i.e., it fits in the viewport)
                            if (accumulatedHeight <= this->getHeight()) {
                                continue;  // Skip the table if it fits within the viewport
                            }
                            
                            isInTable = true;
                            tableIndex = i;
                            entryOffset = this->m_offset;
                            
                            // Expand scrollStepsInsideTable if necessary to track this table
                            if (scrollStepsInsideTable.size() <= tableIndex) {
                                scrollStepsInsideTable.resize(tableIndex + 1, 0);  // Ensure enough space for the current table index
                            }
                            
                            
                
                            break;
                        }
                    }
                
                    // Incremental scrolling inside the table
                    if (isInTable) {
                        // Always check if there's more to scroll
                        if (this->m_offset + TABLE_SCROLL_STEP_SIZE < (this->m_listHeight - this->getHeight() + 50)) {
                            scrollStepsInsideTable[tableIndex]++;  // Increment steps for the current table
                
                            // Move the list down by a fixed step size
                            this->m_nextOffset = std::min(this->m_nextOffset + TABLE_SCROLL_STEP_SIZE, static_cast<float>(this->m_listHeight - this->getHeight() + 50));
                            this->m_offset = this->m_nextOffset;
                            this->invalidate();  // Redraw the list to reflect the new offset
                        } else {

                            // Reached the bottom of the table
                            this->m_nextOffset = this->m_listHeight - this->getHeight() + 50;

                            if (this->m_nextOffset - this->m_offset > 0)
                                scrollStepsInsideTable[tableIndex]++;
                            this->m_offset = this->m_nextOffset;
                            this->invalidate();  // Redraw the list to reflect the full scroll
                
                            // After scrolling, try to focus on the next focusable item below the table
                            for (size_t i = tableIndex + 1; i < this->m_items.size(); ++i) {
                                if (!this->m_items[i]->isTable()) {
                                    newFocus = this->m_items[i]->requestFocus(oldFocus, direction);
                                    if (newFocus != nullptr && newFocus != oldFocus) {
                                        this->m_focusedIndex = i;
                                        this->updateScrollOffset();
                                        isInTable = false;
                                        return newFocus;
                                    }
                                }
                            }
                        }
                        return oldFocus;
                    }
                }
                
                
            
                // Handle scrolling up
                else if (direction == FocusDirection::Up) {
                    if (this->m_items.empty()) {
                        // If there are no items, scroll directly
                        //logMessage("No items to focus on, decrementing scroll directly.");
                        this->m_nextOffset = std::max(this->m_nextOffset - TABLE_SCROLL_STEP_SIZE, 0.0f);
                        this->m_offset = this->m_nextOffset;
                        this->invalidate();  // Redraw
                        return oldFocus;
                    }
                    
                    // Check if the item we're moving to is a table and we should re-enter it
                    if (!isInTable && this->m_focusedIndex > 0) {
                        // Traverse upwards to find the nearest table, skipping over non-focusable items
                        ssize_t potentialTableIndex = this->m_focusedIndex - 1;
                        int totalScrollableHeight = 0;  // To track the cumulative scrollable height
                        
                        bool _isTable = false;
                        while (potentialTableIndex >= 0) {
                            if (this->m_items[potentialTableIndex] != nullptr) { // Skip nullptr (non-focusable items)
                                if (this->m_items[potentialTableIndex]->isItem()) { // Break early for ListItems
                                    totalScrollableHeight -= this->m_offset;
                                    break;
                                } else if (this->m_items[potentialTableIndex]->isTable()) {
                                    // Check if the table fits within the viewport; if it does, skip re-entering
                                    int tableHeight = this->m_items[potentialTableIndex]->getHeight();
                                    
                                    // Set state for entering the table
                                    isInTable = true;
                                    tableIndex = potentialTableIndex;
                    
                                    // Ensure scrollStepsInsideTable has enough space for the current table index
                                    if (scrollStepsInsideTable.size() <= static_cast<size_t>(tableIndex)) {
                                        scrollStepsInsideTable.resize(static_cast<size_t>(tableIndex) + 1, 0);
                                    }
                    
                                    // Add the current table's scrollable height to the cumulative total
                                    int scrollableHeight = tableHeight;
                                    totalScrollableHeight += std::max(0, scrollableHeight); // Accumulate scrollable height
                                    _isTable = true;
                    
                                    // Update entryOffset to reflect the current offset
                                    entryOffset = this->m_offset;
                                }
                            }
                            potentialTableIndex--;  // Move to the next item above
                        }
                        if (_isTable) {
                            // Adjust scroll steps for this table
                            int requiredSteps = static_cast<int>(std::ceil(static_cast<float>(totalScrollableHeight) / TABLE_SCROLL_STEP_SIZE));
                            scrollStepsInsideTable[tableIndex] = std::max(scrollStepsInsideTable[tableIndex], requiredSteps);
                        }
                    }

                    
                    if (isInTable) {
                        //logMessage("Inside table: Offset: " + std::to_string(this->m_offset) + 
                        //           " | EntryOffset: " + std::to_string(entryOffset) + 
                        //           " | scrollStepsInsideTable: " + std::to_string(scrollStepsInsideTable[tableIndex]));
                        
                        if (scrollStepsInsideTable[tableIndex] > 0) {
                            // Decrease the offset and decrement steps one at a time
                            auto preComputedNextOffset = std::min(this->m_nextOffset - TABLE_SCROLL_STEP_SIZE, static_cast<float>(entryOffset));
                            
                            // If the offset would go beyond the top, adjust and reset scroll steps to 0
                            if (preComputedNextOffset < 0.0f) {
                                this->m_nextOffset = 0.0f;
                                scrollStepsInsideTable[tableIndex] = 0;
                                //logMessage("Scroll steps exceeded the top, resetting scrollStepsInsideTable to 0.");

                                //logMessage("Attempting to exit the table. Focus should move to the previous item.");
                        
                                for (ssize_t i = static_cast<ssize_t>(tableIndex) - 1; i >= 0; --i) {
                                    if (this->m_items[i]->isTable()) {
                                        //logMessage("Skipping table or non-focusable item at index: " + std::to_string(i));
                                        continue;
                                    }
                        
                                    newFocus = this->m_items[i]->requestFocus(oldFocus, direction);
                                    if (newFocus != nullptr && newFocus != oldFocus) {
                                        this->m_focusedIndex = static_cast<size_t>(i);
                                        this->updateScrollOffset();
                                        isInTable = false;
                                        //logMessage("Exited table. Focus is now on the previous item at index: " + std::to_string(i));
                                        return newFocus;
                                    } else {
                                        //logMessage("Failed to focus on the previous item at index: " + std::to_string(i));
                                    }
                                }
                        
                                //logMessage("All items before the table are non-focusable. Remaining in table.");
                                return oldFocus;
                            } else {
                                this->m_nextOffset = preComputedNextOffset;
                            }
                    
                            this->m_offset = this->m_nextOffset;
                            scrollStepsInsideTable[tableIndex]--;  // Decrement steps for the current table
                    
                            //logMessage("Scrolling up inside table. Current offset: " + std::to_string(this->m_offset) + 
                            //           " | Remaining scroll steps: " + std::to_string(scrollStepsInsideTable[tableIndex]));
                    
                            // Redraw the list to reflect the new position
                            this->invalidate();  // Ensure the view is updated after each movement
                    
                            return oldFocus;  // Stop here to incrementally scroll up, avoid skipping
                        }
                    
                        // Once all scroll steps are undone, attempt to exit the table and move to the previous item
                        if (scrollStepsInsideTable[tableIndex] == 0) {
                            //logMessage("Attempting to exit the table. Focus should move to the previous item.");
                    
                            for (ssize_t i = static_cast<ssize_t>(tableIndex) - 1; i >= 0; --i) {
                                if (this->m_items[i]->isTable()) {
                                    //logMessage("Skipping table or non-focusable item at index: " + std::to_string(i));
                                    continue;
                                }
                    
                                newFocus = this->m_items[i]->requestFocus(oldFocus, direction);
                                if (newFocus != nullptr && newFocus != oldFocus) {
                                    this->m_focusedIndex = static_cast<size_t>(i);
                                    this->updateScrollOffset();
                                    isInTable = false;
                                    //logMessage("Exited table. Focus is now on the previous item at index: " + std::to_string(i));
                                    return newFocus;
                                } else {
                                    //logMessage("Failed to focus on the previous item at index: " + std::to_string(i));
                                }
                            }
                    
                            //logMessage("All items before the table are non-focusable. Remaining in table.");
                            return oldFocus;
                        }
                    }

            
                    // Handle moving to the previous focusable item outside the table
                    if (!isInTable && this->m_focusedIndex > 0) {
                        for (ssize_t i = static_cast<ssize_t>(this->m_focusedIndex) - 1; i >= 0; --i) {
                            if (static_cast<size_t>(i) >= this->m_items.size() || this->m_items[i] == nullptr) {
                                //logMessage("Reached invalid or non-focusable item index.");
                                return oldFocus;
                            }
            
                            newFocus = this->m_items[i]->requestFocus(oldFocus, direction);
                            if (newFocus != nullptr && newFocus != oldFocus) {
                                this->m_focusedIndex = static_cast<size_t>(i);
                                this->updateScrollOffset();
                                isInTable = this->m_items[i]->isTable();
                                tableIndex = isInTable ? i : 0;
                                //logMessage("Focus moved to item: " + std::to_string(i));
                                return newFocus;
                            }
                        }
                    }
            
                    // Elastic scrolling at the top of the list
                    if (this->m_nextOffset > 0.0f) {
                        this->m_nextOffset = std::max(this->m_nextOffset - TABLE_SCROLL_STEP_SIZE, 0.0f);
                        this->m_offset = this->m_nextOffset;
                        this->invalidate();
                    }
            
                    return oldFocus;
                }
            
                return oldFocus;
            }
            
                        




            
            /**
             * @brief Gets the item at the index in the list
             *
             * @param index Index position in list
             * @return Element from list. nullptr for if the index is out of bounds
             */
            virtual Element* getItemAtIndex(u32 index) {
                if (this->m_items.size() <= index)
                    return nullptr;
                
                return this->m_items[index];
            }
            
            /**
             * @brief Gets the index in the list of the element passed in
             *
             * @param element Element to check
             * @return Index in list. -1 for if the element isn't a member of the list
             */
            virtual s32 getIndexInList(Element *element) {
                auto it = std::find(this->m_items.begin(), this->m_items.end(), element);
                
                if (it == this->m_items.end())
                    return -1;
                
                return it - this->m_items.begin();
            }

            /**
             * @brief Gets the index in the list of the element passed in
             *
             * @param element Element to check
             * @return Index in list. -1 for if the element isn't a member of the list
             */
            virtual s32 getLastIndex() {
                return this->m_items.size() -1;
            }

            
            virtual void setFocusedIndex(u32 index) {
                if (this->m_items.size() > index) {
                    m_focusedIndex = index;
                    this->updateScrollOffset();
                }
            }
        
        protected:
            std::vector<Element*> m_items;
            u16 m_focusedIndex = 0;
            
            float m_offset = 0, m_nextOffset = 0;
            s32 m_listHeight = 0;
            
            bool m_clearList = false;
            std::vector<Element *> m_itemsToRemove;
            std::vector<std::pair<ssize_t, Element *>> m_itemsToAdd;
            

            // Adjust these parameters to fine-tune the behavior
            //static inline constexpr float animationDuration = 3.0f;  // Duration of the animation
            std::vector<float> prefixSums;

        private:
            size_t actualItemCount = 0;

            inline void clearItems() {
                for (auto& item : this->m_items) {
                    delete item;
                }
                this->m_items.clear();
                this->m_offset = 0;
                this->m_focusedIndex = 0;
                this->invalidate();
                this->m_clearList = false;
                actualItemCount = 0;
            }
            
            inline void addPendingItems() {
                for (auto [index, element] : this->m_itemsToAdd) {
                    element->invalidate();
                    if (index >= 0 && (this->m_items.size() > static_cast<size_t>(index))) {
                        this->m_items.insert(this->m_items.cbegin() + static_cast<size_t>(index), element);
                    } else {
                        this->m_items.push_back(element);
                    }
                }
                this->m_itemsToAdd.clear();
                this->invalidate();
                this->updateScrollOffset();
            }
            
            inline void removePendingItems() {
                auto it = this->m_items.cend(); // Initialize iterator to end
                for (auto element : this->m_itemsToRemove) {
                    it = std::find(this->m_items.cbegin(), this->m_items.cend(), element);
                    if (it != this->m_items.cend()) {
                        this->m_items.erase(it);
                        if (this->m_focusedIndex >= static_cast<size_t>(it - this->m_items.cbegin())) {
                            this->m_focusedIndex--;
                        }
                        delete element;
                    }
                }
                this->m_itemsToRemove.clear();
                this->invalidate();
                this->updateScrollOffset();
            }
            
            inline void drawScrollbar(gfx::Renderer *renderer) {
                float viewHeight = static_cast<float>(this->getHeight() - 16);
                float totalHeight = static_cast<float>(this->m_listHeight + 16);
                
                scrollbarHeight = (viewHeight * viewHeight) / totalHeight;
                if (scrollbarHeight > viewHeight) {
                    scrollbarHeight = viewHeight;
                }
                
                u32 maxScrollableHeight = totalHeight - viewHeight;
                if (maxScrollableHeight < 1) maxScrollableHeight = 1;
                
                scrollbarOffset = (static_cast<double>(this->m_offset) / maxScrollableHeight) * (viewHeight - scrollbarHeight);
                if (scrollbarOffset + scrollbarHeight > viewHeight) {
                    scrollbarOffset = viewHeight - scrollbarHeight;
                }
                scrollbarOffset += 8;
                
                //int offset = 11;
                renderer->drawRect(this->getRightBound() + 21 +4, this->getY() + scrollbarOffset, 5, scrollbarHeight, a(trackBarColor));
                renderer->drawCircle(this->getRightBound() + 23 +4, this->getY() + scrollbarOffset, 2, true, a(trackBarColor));
                //renderer->drawCircle(this->getRightBound() + 12 + offset, (this->getY() + scrollbarOffset + scrollbarHeight) / 2, 2, true, a(trackBarColor));
                renderer->drawCircle(this->getRightBound() + 23 +4, this->getY() + scrollbarOffset + scrollbarHeight, 2, true, a(trackBarColor));
                
            }
            
            
            // Function to initialize prefix sums
            inline void initializePrefixSums() {
                prefixSums.clear();
                prefixSums.resize(this->m_items.size() + 1, 0.0f);
            
                for (size_t i = 1; i < prefixSums.size(); ++i) {
                    prefixSums[i] = prefixSums[i - 1] + this->m_items[i - 1]->getHeight();
                }
            }
            virtual inline void updateScrollOffset() {
                if (Element::getInputMode() != InputMode::Controller)
                    return;
                
                if (this->m_listHeight <= this->getHeight()) {
                    this->m_nextOffset = 0;
                    this->m_offset = 0;
                    return;
                }
            
                // Ensure prefixSums is up-to-date
                if (prefixSums.size() != this->m_items.size() + 1) {
                    initializePrefixSums();
                }
            
                // Use prefix sum to calculate the next offset
                this->m_nextOffset = prefixSums[this->m_focusedIndex] - (this->getHeight() / 3);
            
                // Ensure the offset is within bounds
                if (this->m_nextOffset < 0)
                    this->m_nextOffset = 0;
                
                float maxOffset = (this->m_listHeight - this->getHeight()) + 50;
                if (this->m_nextOffset > maxOffset)
                    this->m_nextOffset = maxOffset;
            }

        };
        

        /**
         * @brief A item that goes into a list
         *
         */
        class ListItem : public Element {
        public:
            //std::chrono::duration<long int, std::ratio<1, 1000000000>> t;
            u32 width, height;
            std::chrono::steady_clock::time_point m_touchStartTime;  // Track the time when touch starts


            /**
             * @brief Constructor
             *
             * @param text Initial description text
             */
            ListItem(const std::string& text, const std::string& value = "", const u32& listItemHeight = tsl::style::ListItemDefaultHeight)
                : Element(), m_text(text), m_value(value), m_listItemHeight(listItemHeight) {
                m_isItem = true;
                applyLangReplacements(this->m_text);
                applyLangReplacements(this->m_value, true);

                convertComboToUnicode(this->m_text);
                convertComboToUnicode(this->m_value);
            }
            virtual ~ListItem() {}
            
            virtual void draw(gfx::Renderer *renderer) override {
                static float lastBottomBound;
                bool useClickTextColor = false;
                if (this->m_touched && Element::getInputMode() == InputMode::Touch) {
                    if (touchInBounds) {
                        //renderer->drawRect(ELEMENT_BOUNDS(this), a(clickColor));
                        renderer->drawRect( this->getX()+4, this->getY(), this->getWidth()-8, this->getHeight(), a(clickColor));
                        useClickTextColor = true;
                    }
                    //renderer->drawRect(ELEMENT_BOUNDS(this), tsl::style::color::ColorClickAnimation);
                }

                //convertComboToUnicode(this->m_text);
                //convertComboToUnicode(this->m_value);

                // Calculate vertical offset to center the text
                s32 yOffset = (tsl::style::ListItemDefaultHeight - this->m_listItemHeight) / 2;

                if (this->m_maxWidth == 0) {
                    if (this->m_value.length() > 0) {
                        std::tie(width, height) = renderer->drawString(this->m_value, false, 0, 0, 20, a(tsl::style::color::ColorTransparent));

                        this->m_maxWidth = this->getWidth() - width - 70 +4;
                    } else {
                        this->m_maxWidth = this->getWidth() - 40 -10;
                    }
                    
                    std::tie(width, height) = renderer->drawString(this->m_text, false, 0, 0, 23, a(tsl::style::color::ColorTransparent));
                    this->m_trunctuated = width > this->m_maxWidth+20;
                    
                    if (this->m_trunctuated) {
                        this->m_scrollText = this->m_text + "        ";
                        std::tie(width, height) = renderer->drawString(this->m_scrollText, false, 0, 0, 23, a(tsl::style::color::ColorTransparent));
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
                            renderer->enableScissoring(this->getX()+6, 97, this->m_maxWidth + 40 - 6-4 , tsl::cfg::FramebufferHeight-73-97);
                        else
                            renderer->enableScissoring(this->getX()+6, 97, this->m_maxWidth + 40 - 6 , tsl::cfg::FramebufferHeight-73-97);
                        renderer->drawString(this->m_scrollText, false, this->getX() + 20-1 - this->m_scrollOffset, this->getY() + 45 - yOffset, 23, a(selectedTextColor));
                        renderer->disableScissoring();
                        //t = std::chrono::steady_clock::now() - this->timeIn;
                        if (std::chrono::steady_clock::now() - this->timeIn >= 2000ms) {
                            if (this->m_scrollOffset >= this->m_textWidth) {
                                this->m_scrollOffset = 0;
                                this->timeIn = std::chrono::steady_clock::now();
                            } else {
                                // Calculate the increment based on the desired scroll rate
                                this->m_scrollOffset = ((0.1) * std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - this->timeIn - 2000ms).count());
                            }
                        } // CUSTOM MODIFICATION END
                    } else {
                        renderer->drawString(this->m_ellipsisText, false, this->getX() + 20-1, this->getY() + 45 - yOffset, 23, a(!useClickTextColor ? defaultTextColor : clickTextColor));
                    }
                } else {
                    // Render the text with special character handling
                    renderer->drawStringWithColoredSections(this->m_text, {STAR_SYMBOL+"  "}, this->getX() + 20-1, this->getY() + 45 - yOffset, 23,
                        a(this->m_focused ? (!useClickTextColor ? selectedTextColor : clickTextColor) : (!useClickTextColor ? defaultTextColor : clickTextColor)),
                        a(this->m_focused ? starColor : selectionStarColor)
                    );
                }
                
                
                // CUSTOM SECTION START (modification for submenu footer color)
                //const std::string& value = this->m_value;
                s32 xPosition = this->getX() + this->m_maxWidth + 45 - 1 +4;
                s32 yPosition = this->getY() + 45 - yOffset;
                s32 fontSize = 20;
                //bool isFaint = ;
                //bool isFocused = this->m_focused;

                // Determine text color
                auto textColor = offTextColor;
                if (this->m_value == DROPDOWN_SYMBOL || this->m_value == OPTION_SYMBOL) {
                    textColor = this->m_focused
                        ? (!useClickTextColor ? (this->m_faint ? offTextColor : selectedTextColor) : a(clickTextColor))
                        : (!useClickTextColor ? (this->m_faint ? offTextColor : defaultTextColor) : a(clickTextColor));
                } else if (runningInterpreter.load(std::memory_order_acquire) &&
                    (this->m_value.find(DOWNLOAD_SYMBOL) != std::string::npos ||
                     this->m_value.find(UNZIP_SYMBOL) != std::string::npos ||
                     this->m_value.find(COPY_SYMBOL) != std::string::npos ||
                     this->m_value == INPROGRESS_SYMBOL)) {

                    textColor = this->m_faint ? offTextColor : a(inprogressTextColor);
                } else if (this->m_value == CROSSMARK_SYMBOL) {
                    textColor = this->m_faint ? offTextColor : a(invalidTextColor);
                } else {
                    textColor = this->m_faint ? offTextColor : a(onTextColor);
                }

                // Draw the string with the determined text color
                renderer->drawString(this->m_value, false, xPosition, yPosition, fontSize, textColor);
                // CUSTOM SECTION END 
            }
            
            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX()+2+1, this->getY(), this->getWidth()+8+1, m_listItemHeight);
            }
            
            virtual bool onClick(u64 keys) override {
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }
                if (keys & KEY_A) {
                    this->triggerClickAnimation();
                    simulatedSelectComplete = true;
                }
                else if (keys & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
                    this->m_clickAnimationProgress = 0;

                return Element::onClick(keys);
            }
            
            
            virtual bool onTouch(TouchEvent event, s32 currX, s32 currY, s32 prevX, s32 prevY, s32 initialX, s32 initialY) override {
                if (event == TouchEvent::Touch) {
                    this->m_touched = this->inBounds(currX, currY);
                    if (this->m_touched) {
                        // Start the timer when touch begins
                        m_touchStartTime = std::chrono::steady_clock::now();
                    }
                }
        
                if (event == TouchEvent::Release && this->m_touched) {
                    this->m_touched = false;
                    
                    if (Element::getInputMode() == InputMode::Touch) {
                        
                        #if IS_LAUNCHER_DIRECTIVE
                        // Calculate the touch duration
                        auto touchDuration = std::chrono::steady_clock::now() - m_touchStartTime;
                        auto touchDurationInSeconds = std::chrono::duration_cast<std::chrono::duration<float>>(touchDuration).count();
                        s64 keyToUse = (touchDurationInSeconds >= 1.0) ? STAR_KEY : ((touchDurationInSeconds >= 0.3) ? SETTINGS_KEY : KEY_A);
                        #else
                        s64 keyToUse = KEY_A;

                        #endif

                        bool handled = this->onClick(keyToUse);
                        this->m_clickAnimationProgress = 0;
                        return handled;
                    }
                }
        
                return false;
            }
            
            
            virtual void setFocused(bool state) override {
                this->m_scroll = false;
                this->m_scrollOffset = 0;
                this->timeIn = std::chrono::steady_clock::now(); // CUSTOM MODIFICATION
                Element::setFocused(state);
            }
            
            virtual inline Element* requestFocus(Element *oldFocus, FocusDirection direction) override {
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
            std::chrono::steady_clock::time_point timeIn;// = std::chrono::system_clock::now();
            std::string m_text;
            std::string m_value = "";
            std::string m_scrollText = "";
            std::string m_ellipsisText = "";
            u32 m_listItemHeight;

            bool m_scroll = false;
            bool m_trunctuated = false;
            bool m_faint = false;
            
            bool m_touched = false;
            
            u16 m_maxScroll = 0;
            //half m_scrollOffset = half(0);
            float m_scrollOffset = 0.0;
            u32 m_maxWidth = 0;
            u32 m_textWidth = 0;
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
            ToggleListItem(const std::string& text, bool initialState, const std::string& onValue = ON, const std::string& offValue = OFF)
                : ListItem(text), m_state(initialState), m_onValue(onValue), m_offValue(offValue) {
                this->setState(this->m_state);
            }
            
            virtual ~ToggleListItem() {}
            
            virtual bool onClick(u64 keys) override {
                if (simulatedSelect && !simulatedSelectComplete) {
                    keys |= KEY_A;
                    simulatedSelect = false;
                }

                if (keys & HidNpadButton_A) {
                    this->m_state = !this->m_state;
                    
                    this->setState(this->m_state);
                    this->m_stateChangedListener(this->m_state);
                    
                    simulatedSelectComplete = true;
                    return ListItem::onClick(keys);
                }
                
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
            
        protected:
            bool m_state = true;
            std::string m_onValue, m_offValue;
            
            std::function<void(bool)> m_stateChangedListener = [](bool){};
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
                applyLangReplacements(m_text);
                m_isItem = false;
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
            TrackBar(const char icon[3]) : m_icon(icon) {
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
                if (event == TouchEvent::Release) {
                    this->m_interactionLocked = false;
                    return false;
                }


                if (!this->m_interactionLocked && this->inBounds(initialX, initialY)) {
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

            virtual void draw(gfx::Renderer *renderer) override {
                renderer->drawRect(this->getX(), this->getY(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));
                renderer->drawRect(this->getX(), this->getBottomBound(), this->getWidth(), 1, a(tsl::style::color::ColorFrame));

                renderer->drawString(this->m_icon, false, this->getX() + 15, this->getY() + 50, 23, a(tsl::style::color::ColorText));

                u16 handlePos = (this->getWidth() - 95) * static_cast<float>(this->m_value) / 100;
                renderer->drawCircle(this->getX() + 60, this->getY() + 42, 2, true, a(tsl::style::color::ColorHighlight));
                renderer->drawCircle(this->getX() + 60 + this->getWidth() - 95, this->getY() + 42, 2, true, a(tsl::style::color::ColorFrame));
                renderer->drawRect(this->getX() + 60 + handlePos, this->getY() + 40, this->getWidth() - 95 - handlePos, 5, a(tsl::style::color::ColorFrame));
                renderer->drawRect(this->getX() + 60, this->getY() + 40, handlePos, 5, a(tsl::style::color::ColorHighlight));

                renderer->drawCircle(this->getX() + 62 + handlePos, this->getY() + 42, 18, true, a(tsl::style::color::ColorHandle));
                renderer->drawCircle(this->getX() + 62 + handlePos, this->getY() + 42, 18, false, a(tsl::style::color::ColorFrame));
            }

            virtual void layout(u16 parentX, u16 parentY, u16 parentWidth, u16 parentHeight) override {
                this->setBoundaries(this->getX(), this->getY(), this->getWidth(), tsl::style::TrackBarDefaultHeight);
            }

            virtual void drawFocusBackground(gfx::Renderer *renderer) {
                // No background drawn here in HOS
            }

            virtual void drawHighlight(gfx::Renderer *renderer) override {
                static float counter = 0;
                const float progress = (std::sin(counter) + 1) / 2;
                Color highlightColor = {   static_cast<u8>((0x2 - 0x8) * progress + 0x8),
                                                static_cast<u8>((0x8 - 0xF) * progress + 0xF),
                                                static_cast<u8>((0xC - 0xF) * progress + 0xF),
                                                static_cast<u8>((0x6 - 0xD) * progress + 0xD) };

                counter += 0.1F;

                u16 handlePos = (this->getWidth() - 95) * static_cast<float>(this->m_value) / 100;

                s32 x = 0;
                s32 y = 0;

                if (Element::m_highlightShaking) {
                    auto t = (std::chrono::steady_clock::now() - Element::m_highlightShakingStartTime);
                    if (t >= 100ms)
                        Element::m_highlightShaking = false;
                    else {
                        s32 amplitude = std::rand() % 5 + 5;

                        switch (Element::m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t, amplitude);
                                break;
                            default:
                                break;
                        }

                        x = std::clamp(x, -amplitude, amplitude);
                        y = std::clamp(y, -amplitude, amplitude);
                    }
                }

                for (u8 i = 16; i <= 19; i++) {
                    renderer->drawCircle(this->getX() + 62 + x + handlePos, this->getY() + 42 + y, i, false, a(highlightColor));
                }
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
            StepTrackBar(const char icon[3], size_t numSteps)
                : TrackBar(icon), m_numSteps(numSteps) { }

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
                : StepTrackBar(icon, stepDescriptions.size()), m_stepDescriptions(stepDescriptions.begin(), stepDescriptions.end()) { }

            virtual ~NamedStepTrackBar() {}

            virtual void draw(gfx::Renderer *renderer) override {

                u16 trackBarWidth = this->getWidth() - 95;
                u16 stepWidth = trackBarWidth / (this->m_numSteps - 1);

                for (u8 i = 0; i < this->m_numSteps; i++) {
                    renderer->drawRect(this->getX() + 60 + stepWidth * i, this->getY() + 50, 1, 10, a(tsl::style::color::ColorFrame));
                }

                u8 currentDescIndex = std::clamp(this->m_value / (100 / (this->m_numSteps - 1)), 0, this->m_numSteps - 1);

                auto [descWidth, descHeight] = renderer->drawString(this->m_stepDescriptions[currentDescIndex].c_str(), false, 0, 0, 15, tsl::style::color::ColorTransparent);
                renderer->drawString(this->m_stepDescriptions[currentDescIndex].c_str(), false, ((this->getX() + 60) + (this->getWidth() - 95) / 2) - (descWidth / 2), this->getY() + 20, 15, a(tsl::style::color::ColorDescription));

                StepTrackBar::draw(renderer);
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
            std::chrono::steady_clock::time_point lastUpdate;
            
            Color highlightColor = {0xf, 0xf, 0xf, 0xf};
            float progress;
            float counter = 0.0;
            s32 x, y;
            s32 amplitude;
            u32 descWidth, descHeight;
            
            
            // Ensure the order of initialization matches the order of declaration
            TrackBarV2(std::string label, std::string packagePath = "", s16 minValue = 0, s16 maxValue = 100, std::string units = "",
                     std::function<void(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
                     std::function<std::vector<std::vector<std::string>>(const std::vector<std::vector<std::string>>&, const std::string&, size_t, const std::string&)> sourceReplacementFunc = nullptr,
                     std::vector<std::vector<std::string>> cmd = {}, const std::string& selCmd = "", bool usingStepTrackbar = false, bool usingNamedStepTrackbar = false, s16 numSteps = -1, bool unlockedTrackbar = false, bool executeOnEveryTick = false)
                : m_label(label), m_packagePath(packagePath), m_minValue(minValue), m_maxValue(maxValue), m_units(units),
                  interpretAndExecuteCommands(executeCommands), getSourceReplacement(sourceReplacementFunc), commands(std::move(cmd)), selectedCommand(selCmd), m_usingStepTrackbar(usingStepTrackbar), m_usingNamedStepTrackbar(usingNamedStepTrackbar), m_numSteps(numSteps), m_unlockedTrackbar(unlockedTrackbar), m_executeOnEveryTick(executeOnEveryTick) {
                m_isItem = true;
                
                if ((!usingStepTrackbar && !usingNamedStepTrackbar) || numSteps == -1) {
                    m_numSteps = maxValue - minValue;
                }

                bool loadedValue = false;
                if (!m_packagePath.empty()) {
                    std::string initialIndex = parseValueFromIniSection(m_packagePath + "config.ini", m_label, "index");

                    if (!initialIndex.empty()) {
                        m_index = static_cast<s16>(std::stoi(initialIndex)); // convert initializedValue to s16
                    }
                    if (!m_usingNamedStepTrackbar) {
                        std::string initialValue = parseValueFromIniSection(m_packagePath + "config.ini", m_label, "value");
                        
                        if (!initialValue.empty()) {
                            m_value = static_cast<s16>(std::stoi(initialValue)); // convert initializedValue to s16
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

                lastUpdate = std::chrono::steady_clock::now();
            }
            
            virtual ~TrackBarV2() {}
            
            virtual Element* requestFocus(Element *oldFocus, FocusDirection direction) {
                return this;
            }
            
            inline void updateAndExecute(bool updateIni = true) {
                if (m_packagePath.empty()) {
                    return;
                }
            
                // Precompute values and reserve space
                const std::string indexStr = std::to_string(m_index);
                const std::string valueStr = m_usingNamedStepTrackbar ? m_selection : std::to_string(m_value);
                
                // Update INI file values if needed
                if (updateIni) {
                    const std::string configPath = m_packagePath + "config.ini";
                    setIniFileValue(configPath, m_label, "index", indexStr);
                    setIniFileValue(configPath, m_label, "value", valueStr);
                }
            
                // Process and execute commands if needed
                if (interpretAndExecuteCommands) {
                    auto modifiedCmds = getSourceReplacement(commands, valueStr, m_index, m_packagePath);
            
                    // Prepare strings for replacements
                    const std::string valuePlaceholder = "{value}";
                    const std::string indexPlaceholder = "{index}";
                    const size_t valuePlaceholderLength = valuePlaceholder.length();
                    const size_t indexPlaceholderLength = indexPlaceholder.length();
                    
                    for (auto& cmd : modifiedCmds) {
                        for (auto& arg : cmd) {
                            // Replace {value} placeholders
                            size_t pos = 0;
                            while ((pos = arg.find(valuePlaceholder, pos)) != std::string::npos) {
                                arg.replace(pos, valuePlaceholderLength, valueStr);
                                pos += valueStr.length();
                            }
            
                            // Replace {index} placeholders if needed
                            if (m_usingNamedStepTrackbar) {
                                pos = 0;
                                while ((pos = arg.find(indexPlaceholder, pos)) != std::string::npos) {
                                    arg.replace(pos, indexPlaceholderLength, indexStr);
                                    pos += indexStr.length();
                                }
                            }
                        }
                    }
            
                    // Execute commands
                    interpretAndExecuteCommands(std::move(modifiedCmds), m_packagePath, selectedCommand);
                }
            }

            
            virtual inline bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState leftJoyStick, HidAnalogStickState rightJoyStick) override {
                //static std::chrono::milliseconds initialInterval{67}; // Initial interval between value changes (67ms)
                static bool holding = false;
                static std::chrono::steady_clock::time_point holdStartTime;
                static u64 prevKeysHeld = 0;
                //static bool allowSlide = false; // Flag to allow sliding after KEY_A is pressed
                u64 keysReleased = prevKeysHeld & ~keysHeld;
                prevKeysHeld = keysHeld;
                
                auto now = std::chrono::steady_clock::now();
                auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate);
                
                if (simulatedSelect && !simulatedSelectComplete) {
                    keysDown |= KEY_A;
                    simulatedSelect = false;
                }

                // Check if KEY_A is pressed to toggle allowSlide
                if ((keysDown & KEY_A) && !m_unlockedTrackbar) {
                    allowSlide = !allowSlide;
                    holding = false; // Reset holding state when KEY_A is pressed
                    simulatedSelectComplete = true;
                    return true;
                }
                

                // Allow sliding only if KEY_A has been pressed
                if (allowSlide || m_unlockedTrackbar) {
                    if ((keysReleased & HidNpadButton_AnyLeft) || (keysReleased & HidNpadButton_AnyRight)) {
                        //if (!m_executeOnEveryTick)
                        updateAndExecute();
                        holding = false;
                        return true;
                    }
                    
                    if (keysHeld & HidNpadButton_AnyLeft && keysHeld & HidNpadButton_AnyRight)
                        return true;
                    
                    // Check if the button is being held down
                    if ((keysHeld & HidNpadButton_AnyLeft) || (keysHeld & HidNpadButton_AnyRight)) {
                        if (!holding) {
                            holding = true;
                            holdStartTime = now;
                        }
                        
                        auto holdDuration = std::chrono::duration_cast<std::chrono::milliseconds>(now - holdStartTime);

                        // Define the duration boundaries
                        //const auto initialInterval = std::chrono::milliseconds(67);  // Example initial interval
                        //const auto shortInterval = std::chrono::milliseconds(10);    // Short interval after long hold
                        //const auto transitionPoint = std::chrono::milliseconds(2000); // Point at which the shortest interval is reached
                        
                        // Calculate transition factor (t) from 0 to 1 based on how far we are from the transition point
                        float t = std::min(1.0f, static_cast<float>(holdDuration.count()) / transitionPoint.count());
                        
                        // Smooth transition between intervals
                        std::chrono::milliseconds currentInterval = interpolateDuration(initialInterval, shortInterval, t);
                        
                        if (elapsed >= currentInterval) {
                            if (keysHeld & HidNpadButton_AnyLeft) {
                                if (this->m_value > m_minValue) {
                                    this->m_index--;
                                    this->m_value--;
                                    this->m_valueChangedListener(this->m_value);
                                    if (m_executeOnEveryTick) {
                                        updateAndExecute(false);
                                    }
                                    lastUpdate = now;
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
                                    lastUpdate = now;
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
            
                if (!internalTouchReleased)
                    return false;
            
                if (event == TouchEvent::Release) {
                    updateAndExecute();
                    this->m_interactionLocked = false;
                    touchInSliderBounds = false;
                    return false;
                }
            
                if (!this->m_interactionLocked && (touchInCircle || touchInSliderBounds)) {
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
                    }
            
                    return true;
                } else {
                    this->m_interactionLocked = true;
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
                    unlockedSlide = m_unlockedTrackbar;
                    drawBar(renderer, xPos, yPos-3, handlePos, trackBarFullColor, !m_usingNamedStepTrackbar);
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 16, true, a(highlightColor));
                    renderer->drawCircle(xPos + x + handlePos, yPos +y, 12, true, a((allowSlide || m_unlockedTrackbar) ? trackBarSliderMalleableColor : trackBarSliderColor));
                }
                
                std::string labelPart = this->m_label;
                removeTag(labelPart);
                labelPart += " ";

                //std::string valuePart = m_usingNamedStepTrackbar ? this->m_selection : std::to_string(this->m_value) + (this->m_units.empty() ? "" : " ") + this->m_units;
                std::string valuePart;
                if (!m_usingNamedStepTrackbar)
                    valuePart = (this->m_units == "%" || this->m_units == "°C" || this->m_units == "°F") ? std::to_string(this->m_value) + this->m_units : std::to_string(this->m_value) + (this->m_units.empty() ? "" : " ") + this->m_units;
                else
                    valuePart = this->m_selection;
                std::string combinedString = labelPart + valuePart;
                std::tie(descWidth, descHeight) = renderer->drawString(combinedString, false, 0, 0, 16, a(tsl::style::color::ColorTransparent));
                
                size_t combinedX = (xPos + width / 2) - (descWidth / 2);
                size_t labelWidth;
                std::tie(labelWidth, descHeight) = renderer->drawString(labelPart, false, 0, 0, 16, a(tsl::style::color::ColorTransparent));
                

                renderer->drawString(labelPart, false, combinedX, this->getY() + 14 + 16, 16, (!this->m_focused ? a(defaultTextColor) : a(selectedTextColor)));
                renderer->drawString(valuePart, false, combinedX + labelWidth, this->getY() + 14 + 16, 16, a(onTextColor));
                
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
                
                progress = ((std::sin(2.0 * _M_PI * fmod(std::chrono::duration<double>(std::chrono::steady_clock::now().time_since_epoch()).count(), 1.0)) + 1.0) / 2.0);
                if (allowSlide || m_unlockedTrackbar) {
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
                
                //u16 handlePos = (this->getWidth() - 95) * (this->m_value - m_minValue) / (m_maxValue - m_minValue);
                x = 0;
                y = 0;
                
                if (this->m_highlightShaking) {
                    t = (std::chrono::steady_clock::now() - this->m_highlightShakingStartTime);
                    if (t >= 100ms)
                        this->m_highlightShaking = false;
                    else {
                        amplitude = std::rand() % 5 + 5;
                        
                        switch (this->m_highlightShakingDirection) {
                            case FocusDirection::Up:
                                y -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Down:
                                y += shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Left:
                                x -= shakeAnimation(t, amplitude);
                                break;
                            case FocusDirection::Right:
                                x += shakeAnimation(t, amplitude);
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

                onTrackBar = true;
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
            std::function<void(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> interpretAndExecuteCommands;
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
                std::function<void(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
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

                if (simulatedSelect && !simulatedSelectComplete) {
                    keysDown |= KEY_A;
                    simulatedSelect = false;
                }

                // Check if KEY_A is pressed to toggle allowSlide
                if ((keysDown & KEY_A)) {
                    if (!m_unlockedTrackbar) {
                        allowSlide = !allowSlide;
                        holding = false; // Reset holding state when KEY_A is pressed
                    //} else {
                    //    m_unlockedTrackbar = !m_unlockedTrackbar;
                    //    holding = false; // Reset holding state when KEY_A is pressed
                    }
                    simulatedSelectComplete = true;
                    return true;
                }

                if (allowSlide || m_unlockedTrackbar) {
                    if ((keysReleased & HidNpadButton_AnyLeft) || (keysReleased & HidNpadButton_AnyRight)) {
                        //if (!m_executeOnEveryTick)
                        updateAndExecute();
                        holding = false;
                        tick = 0;
                        return true;
                    }
                    
                    if (keysHeld & HidNpadButton_AnyLeft && keysHeld & HidNpadButton_AnyRight) {
                        tick = 0;
                        return true;
                    }
                    
                    if (keysHeld & (HidNpadButton_AnyLeft | HidNpadButton_AnyRight) && !(keysHeld & KEY_R)) {
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
                std::function<void(std::vector<std::vector<std::string>>&&, const std::string&, const std::string&)> executeCommands = nullptr,
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
                //u16 stepX;
                for (u8 i = 0; i < this->m_numSteps; i++) {
                    u16 stepX = baseX + std::round(i * stepSpacing);
                    
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
        Gui() { }
        
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
         * @brief Non-virtual method that ensures the base initialization always occurs
         * This should be called instead of directly calling initServices().
         */
        void initialize() {
            #ifdef OVERRIDE_APPEARANCE_PATH

            std::string APPEARANCE_PATH = OVERRIDE_APPEARANCE_PATH;
            preprocessPath(APPEARANCE_PATH);

            const std::string NEW_THEME_CONFIG_INI_PATH = APPEARANCE_PATH+"theme.ini";
            const std::string NEW_WALLPAPER_PATH = APPEARANCE_PATH+"wallpaper.rgba";


            if (isFileOrDirectory(NEW_THEME_CONFIG_INI_PATH))
                THEME_CONFIG_INI_PATH = NEW_THEME_CONFIG_INI_PATH; // Override theme path (optional)
            if (isFileOrDirectory(NEW_WALLPAPER_PATH))
                WALLPAPER_PATH = NEW_WALLPAPER_PATH; // Override wallpaper path (optional)
            #endif

            //initializeThemeVars(); // Initialize variables for ultrahand themes
            #if IS_LAUNCHER_DIRECTIVE
            #else
            initializeUltrahandSettings(); // Set up for opaque screenshots, swipe-to-open functionality, and more.
            #endif
        }

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

            isHidden.store(false);
            this->onShow();
            
            if (auto& currGui = this->getCurrentGui(); currGui != nullptr) // TESTING DISABLED (EFFECTS NEED TO BE VERIFIED)
                currGui->restoreFocus();
        }
        
        /**
         * @brief Hides the Gui
         *
         */
        void hide() {
            
            if (this->m_disableNextAnimation) {
                this->m_animationCounter = 0;
                this->m_disableNextAnimation = false;
            }
            else {
                this->m_fadeOutAnimationPlaying = true;
                this->m_animationCounter = MAX_ANIMATION_COUNTER;
            }
            isHidden.store(true);
            this->onHide();
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
            
            renderer.startFrame();
            
            this->animationLoop();
            this->getCurrentGui()->update();
            this->getCurrentGui()->draw(&renderer);
            
            renderer.endFrame();
        }
        


        /**
         * @brief Called once per frame with the latest HID inputs
         *
         * @param keysDown Buttons pressed in the last frame
         * @param keysHeld Buttons held down longer than one frame
         * @param touchInput Last touch position
         * @param leftJoyStick Left joystick position
         * @param rightJoyStick Right joystick position
         * @return Whether or not the input has been consumed
         */
        void handleInput(u64 keysDown, u64 keysHeld, bool touchDetected, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
            // Static variables to maintain state between function calls
            static HidTouchState initialTouchPos = { 0 };
            static HidTouchState oldTouchPos = { 0 };
            static bool oldTouchDetected = false;
            static elm::TouchEvent touchEvent, oldTouchEvent;
            //static elm::TouchEvent oldTouchEvent;

            //static ssize_t counter = 0;

            static std::chrono::steady_clock::time_point buttonPressTime, lastKeyEventTime;
            //static std::chrono::steady_clock::time_point lastKeyEventTime;
            static bool singlePressHandled = false;
            static const auto clickThreshold = std::chrono::milliseconds(340); // Adjust this value as needed
            static auto keyEventInterval = std::chrono::milliseconds(67); // Interval between key events
            
            auto& currentGui = this->getCurrentGui();
            

            // Return early if current GUI is not available
            if (!currentGui) return;
            if (!internalTouchReleased) return;
            
            // Retrieve current focus and top/bottom elements of the GUI
            auto currentFocus = currentGui->getFocusedElement();
            auto topElement = currentGui->getTopElement();
            auto bottomElement = currentGui->getBottomElement();
            
            if (runningInterpreter.load()) {
                if (keysDown & KEY_UP && !(keysDown & ~KEY_UP & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Up);
                else if (keysDown & KEY_DOWN && !(keysDown & ~KEY_DOWN & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Down);
                else if (keysDown & KEY_LEFT && !(keysDown & ~KEY_LEFT & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Left);
                else if (keysDown & KEY_RIGHT && !(keysDown & ~KEY_RIGHT & ALL_KEYS_MASK))
                    currentFocus->shakeHighlight(FocusDirection::Right);
                //else if (progressAnimation) {
                //    currentFocus->shakeHighlight(static_cast<FocusDirection>(counter % 4));
                //    counter = (counter + 1) % 4;
                //}
            }

            #if IS_LAUNCHER_DIRECTIVE
            #else
            if (currentFocus == nullptr) {
                if (simulatedBack) {
                    keysDown |= KEY_B;
                    simulatedBack = false;
                }
                if (keysDown & KEY_B) {
                    if (!currentGui->handleInput(KEY_B,0,{},{},{}))
                        this->goBack();
                    return;
                }
            }
            #endif
            
            if (!currentFocus && !simulatedBack && simulatedBackComplete && !stillTouching && !runningInterpreter.load(std::memory_order_acquire)) {
                if (!topElement) return;
                
                if (!currentGui->initialFocusSet() || keysDown & (HidNpadButton_AnyUp | HidNpadButton_AnyDown | HidNpadButton_AnyLeft | HidNpadButton_AnyRight)) {
                    currentGui->requestFocus(topElement, FocusDirection::None);
                    currentGui->markInitialFocusSet();
                }

            }
            static bool hasScrolled = false;


            if (!currentFocus && !touchDetected && (!oldTouchDetected || oldTouchEvent == elm::TouchEvent::Scroll)) {
                if (!simulatedBack && simulatedBackComplete && topElement) {
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
                    auto now = std::chrono::steady_clock::now();
                    buttonPressTime = now;
                    lastKeyEventTime = now;
                    hasScrolled = false;
                }
            } else {

                if (!touchDetected && !oldTouchDetected && !handled && currentFocus && !stillTouching && !runningInterpreter.load(std::memory_order_acquire)) {
                    static bool shouldShake = true;
                    bool singleArrowKeyPress = ((keysHeld & KEY_UP) != 0) + ((keysHeld & KEY_DOWN) != 0) + ((keysHeld & KEY_LEFT) != 0) + ((keysHeld & KEY_RIGHT) != 0) == 1;
                    
                    if (singleArrowKeyPress) {
                        
                        auto now = std::chrono::steady_clock::now();
                        if (keysDown) {
                            buttonPressTime = now;
                            lastKeyEventTime = now;
                            singlePressHandled = false;
                            // Immediate single press action
                            if (keysHeld & KEY_UP && !(keysHeld & ~KEY_UP & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentGui->getTopElement(), FocusDirection::Up, shouldShake); // Request focus on the top element when double-clicking up
                            else if (keysHeld & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Down, shouldShake);
                            else if (keysHeld & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Left, shouldShake);
                            else if (keysHeld & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Right, shouldShake);
                            //shouldShake = currentGui->getFocusedElement() != currentFocus;
                        }
                        if (keysHeld & ~KEY_DOWN & ~KEY_UP & ~KEY_LEFT & ~KEY_RIGHT & ALL_KEYS_MASK) // reset
                            buttonPressTime = now;
                        
                        auto durationSincePress = std::chrono::duration_cast<std::chrono::milliseconds>(now - buttonPressTime);
                        auto durationSinceLastEvent = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastKeyEventTime);
                        
                        if (!singlePressHandled && durationSincePress >= clickThreshold) {
                            singlePressHandled = true;
                        }
                        

                        // Define the duration boundaries
                        //const auto initialInterval = std::chrono::milliseconds(67);  // Example initial interval
                        //const auto shortInterval = std::chrono::milliseconds(10);    // Short interval after long hold
                        //const auto transitionPoint = std::chrono::milliseconds(2000); // Point at which the shortest interval is reached
                        
                        // Calculate transition factor (t) from 0 to 1 based on how far we are from the transition point
                        float t = std::min(1.0f, static_cast<float>(durationSincePress.count()) / transitionPoint.count());
                        
                        // Smooth transition between intervals
                        keyEventInterval = interpolateDuration(initialInterval, shortInterval, t);
                        
                        //keyEventInterval = interpolateKeyEventInterval(durationSincePress);
                        
                        if (singlePressHandled && durationSinceLastEvent >= keyEventInterval) {
                            lastKeyEventTime = now;
                            if (keysHeld & KEY_UP && !(keysHeld & ~KEY_UP & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentGui->getTopElement(), FocusDirection::Up, false);
                            else if (keysHeld & KEY_DOWN && !(keysHeld & ~KEY_DOWN & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Down, false);
                            else if (keysHeld & KEY_LEFT && !(keysHeld & ~KEY_LEFT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Left, false);
                            else if (keysHeld & KEY_RIGHT && !(keysHeld & ~KEY_RIGHT & ALL_KEYS_MASK))
                                currentGui->requestFocus(currentFocus->getParent(), FocusDirection::Right, false);
                            //shouldShake = currentGui->getFocusedElement() != currentFocus;
                        }
                    } else {
                        // Handle the rest of the input
                        if (simulatedBack) {
                            keysDown |= KEY_B;
                            simulatedBack = false;
                        }

                        if (keysDown & KEY_B)
                            this->goBack();
                        singlePressHandled = false;
                    }
                }
            }
            
            if (!touchDetected && (keysDown & KEY_L) && !(keysHeld & ~KEY_L & ALL_KEYS_MASK) && !runningInterpreter.load(std::memory_order_acquire)) {
                currentGui->requestFocus(topElement, FocusDirection::None);
                currentGui->requestFocus(topElement, FocusDirection::None);
            }
            
            if (!touchDetected && (keysDown & KEY_R) && !(keysHeld & ~KEY_R & ALL_KEYS_MASK) && !runningInterpreter.load(std::memory_order_acquire)) {
                currentGui->requestFocus(bottomElement, FocusDirection::None);
            }
            
            if (!touchDetected && oldTouchDetected && currentGui && topElement) {
                topElement->onTouch(elm::TouchEvent::Release, oldTouchPos.x, oldTouchPos.y, oldTouchPos.x, oldTouchPos.y, initialTouchPos.x, initialTouchPos.y);
            }

            //u32 layerEdge = cfg::LayerPosX == 0 ? 0 : (1280-448);

            touchingBack = (touchPos.x >= 20.0f + layerEdge && touchPos.x < backWidth+86.0f + layerEdge && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= 20.0f + layerEdge && initialTouchPos.x < backWidth+86.0f + layerEdge && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            touchingSelect = !noClickableItems && (touchPos.x >= backWidth+86.0f + layerEdge && touchPos.x < (backWidth+86.0f + selectWidth+68.0f + layerEdge) && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >=  backWidth+86.0f + layerEdge && initialTouchPos.x < (backWidth+86.0f + selectWidth+68.0f + layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            if (!noClickableItems)
                touchingNextPage = (touchPos.x >= (backWidth+86.0f + selectWidth+68.0f + layerEdge) && (touchPos.x <= backWidth+86.0f + selectWidth+68.0f +nextPageWidth+70.0f + layerEdge) && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (backWidth+86.0f + selectWidth+68.0f + layerEdge) && (initialTouchPos.x <= backWidth+86.0f + selectWidth+68.0f +nextPageWidth+70.0f + layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            else
                touchingNextPage = (touchPos.x >= (backWidth+86.0f + layerEdge) && (touchPos.x <= backWidth+86.0f +nextPageWidth+70.0f + layerEdge) && touchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (backWidth+86.0f + layerEdge) && (initialTouchPos.x <= backWidth+86.0f +nextPageWidth+70.0f + layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U);
            touchingMenu = (touchPos.x > layerEdge && touchPos.x <= 245+layerEdge && touchPos.y > 10U && touchPos.y <= 83U) && (initialTouchPos.x > layerEdge && initialTouchPos.x <= 245 + layerEdge && initialTouchPos.y > 10U && initialTouchPos.y <= 83U);
            

            if (touchDetected) {
                if (!interruptedTouch) interruptedTouch = (keysHeld & ALL_KEYS_MASK) != 0;
                
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
                    if (!runningInterpreter.load(std::memory_order_acquire)) {
                        touchInBounds = (initialTouchPos.y <= cfg::FramebufferHeight - 73U && initialTouchPos.y > 73U && initialTouchPos.x <= layerEdge + cfg::FramebufferWidth-30U && initialTouchPos.x > 40U + layerEdge);
                        if (touchInBounds) currentGui->removeFocus();
                    }
                    touchEvent = elm::TouchEvent::Touch;
                }
                
                if (currentGui && topElement && !runningInterpreter.load(std::memory_order_acquire)) {
                    topElement->onTouch(touchEvent, touchPos.x, touchPos.y, oldTouchPos.x, oldTouchPos.y, initialTouchPos.x, initialTouchPos.y);
                    if (touchPos.x > 40U + layerEdge && touchPos.x <= cfg::FramebufferWidth-30U + layerEdge && touchPos.y > 73U && touchPos.y <= cfg::FramebufferHeight - 73U) {
                        currentGui->removeFocus();
                    }
                    
                }
                
                oldTouchPos = touchPos;
                if ((touchPos.x < layerEdge || touchPos.x > cfg::FramebufferWidth + layerEdge) && tsl::elm::Element::getInputMode() == tsl::InputMode::Touch) {
                    oldTouchPos = { 0 };
                    initialTouchPos = { 0 };
                    this->hide();
                }
                stillTouching = true;
            } else {
                if (!interruptedTouch && !runningInterpreter.load(std::memory_order_acquire)) {
                    if ((oldTouchPos.x >= 20.0f + layerEdge && oldTouchPos.x < backWidth+86.0f + layerEdge && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= 20.0f + layerEdge && initialTouchPos.x < backWidth+86.0f + layerEdge && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        simulatedBackComplete = false;
                        simulatedBack = true;
                    } else if (!noClickableItems && (oldTouchPos.x >= backWidth+86.0f + layerEdge && oldTouchPos.x < (backWidth+86.0f + selectWidth+68.0f + layerEdge) && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >=  backWidth+86.0f + layerEdge && initialTouchPos.x < (backWidth+86.0f + selectWidth+68.0f + layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        simulatedSelectComplete = false;
                        simulatedSelect = true;
                    } else if (!noClickableItems && (oldTouchPos.x >= (backWidth+86.0f + selectWidth+68.0f + layerEdge) && (oldTouchPos.x <= backWidth+86.0f + selectWidth+68.0f +nextPageWidth+70.0f + layerEdge) && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (backWidth+86.0f + selectWidth+68.0f + layerEdge) && (initialTouchPos.x <= backWidth+86.0f + selectWidth+68.0f +nextPageWidth+70.0f + layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        simulatedNextPageComplete = false;
                        simulatedNextPage = true;
                    } else if (noClickableItems && (oldTouchPos.x >= (backWidth+86.0f + layerEdge) && (oldTouchPos.x <= backWidth+86.0f +nextPageWidth+70.0f + layerEdge) && oldTouchPos.y > cfg::FramebufferHeight - 73U) && (initialTouchPos.x >= (backWidth+86.0f + layerEdge) && (initialTouchPos.x <= backWidth+86.0f +nextPageWidth+70.0f + layerEdge) && initialTouchPos.y > cfg::FramebufferHeight - 73U)) {
                        simulatedNextPageComplete = false;
                        simulatedNextPage = true;
                    } else if ((oldTouchPos.x > layerEdge && oldTouchPos.x <= layerEdge + 245 && oldTouchPos.y > 10U && oldTouchPos.y <= 83U) && (initialTouchPos.x > layerEdge && initialTouchPos.x <= layerEdge + 245 && initialTouchPos.y > 10U && initialTouchPos.y <= 83U)) {
                        simulatedMenuComplete = false;
                        simulatedMenu = true;
                    }
                }
                
                elm::Element::setInputMode(InputMode::Controller);
                
                oldTouchPos = { 0 };
                initialTouchPos = { 0 };
                touchEvent = elm::TouchEvent::None;
                stillTouching = false;
                interruptedTouch = false;
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
         * @brief Data shared between the different threads
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
            
            u64 decodedKeys = hlp::comboStringToKeys(parsedConfig[ULTRAHAND_PROJECT_NAME][KEY_COMBO_STR]); // CUSTOM MODIFICATION
            if (decodedKeys)
                tsl::cfg::launchCombo = decodedKeys;
            
            datetimeFormat = parsedConfig[ULTRAHAND_PROJECT_NAME]["datetime_format"]; // read datetime_format
            removeQuotes(datetimeFormat);
            if (datetimeFormat.empty()) {
                datetimeFormat = DEFAULT_DT_FORMAT;
                removeQuotes(datetimeFormat);
            }
            std::string hideClockStr = parsedConfig[ULTRAHAND_PROJECT_NAME]["hide_clock"];
            removeQuotes(hideClockStr);
            hideClock = hideClockStr != FALSE_STR;
            
            std::string hideBatteryStr = parsedConfig[ULTRAHAND_PROJECT_NAME]["hide_battery"];
            removeQuotes(hideBatteryStr);
            hideBattery = hideBatteryStr != FALSE_STR;
            
            std::string hidePCBTempStr = parsedConfig[ULTRAHAND_PROJECT_NAME]["hide_pcb_temp"];
            removeQuotes(hidePCBTempStr);
            hidePCBTemp = hidePCBTempStr != FALSE_STR;
            
            std::string hideSOCTempStr = parsedConfig[ULTRAHAND_PROJECT_NAME]["hide_soc_temp"];
            removeQuotes(hideSOCTempStr);
            hideSOCTemp = hideSOCTempStr != FALSE_STR;
            
        }

        /**
         * @brief Update and save launch combo keys
         *
         * @param keys the new combo keys
         */
        [[maybe_unused]] static void updateCombo(u64 keys) {
            tsl::cfg::launchCombo = keys;
            hlp::ini::updateOverlaySettings({
                { TESLA_STR, { // CUSTOM MODIFICATION
                    { KEY_COMBO_STR, tsl::hlp::keysToComboString(keys) }
                }}
            }, TESLA_CONFIG_FILE);
            hlp::ini::updateOverlaySettings({
                { ULTRAHAND_PROJECT_NAME, { // CUSTOM MODIFICATION
                    { KEY_COMBO_STR, tsl::hlp::keysToComboString(keys) }
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
            hlp::ScopeGuard homeButtonEventGuard([&] { eventClose(&homeButtonPressEvent); });
            
            // To prevent focus glitchout, close the overlay immediately when the power button gets pressed
            Event powerButtonPressEvent = {};
            hidsysAcquireSleepButtonEventHandle(&powerButtonPressEvent, false);
            eventClear(&powerButtonPressEvent);
            hlp::ScopeGuard powerButtonEventGuard([&] { eventClose(&powerButtonPressEvent); });
            
            // For handling screenshots color alpha
            Event captureButtonPressEvent = {};
            hidsysAcquireCaptureButtonEventHandle(&captureButtonPressEvent, false);
            eventClear(&captureButtonPressEvent);
            hlp::ScopeGuard captureButtonEventGuard([&] { eventClose(&captureButtonPressEvent); });

            // Parse Tesla settings
            impl::parseOverlaySettings();
            
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
            
            static auto currentTouchTime = std::chrono::steady_clock::now();
            static auto lastTouchX = 0;

            // Preset touch boundaries
            static const int SWIPE_RIGHT_BOUND = 16;  // 16 + 80
            static const int SWIPE_LEFT_BOUND = (1280 - 16);
            static size_t elapsedTime = 0;
            static const size_t TOUCH_THREHELD_MS = 150; 

            s32 idx;
            Result rc;

            while (shData->running) {
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
                            internalTouchReleased = false;
                        }
                        
                        elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - currentTouchTime).count();
                        // Check if the touch is within bounds for left-to-right swipe within the time window
                        if (useSwipeToOpen && elapsedTime <= TOUCH_THREHELD_MS) {
                            if (lastTouchX != 0 && currentTouch.x != 0) {
                                if (layerEdge == 0 && currentTouch.x > SWIPE_RIGHT_BOUND + 84 && lastTouchX <= SWIPE_RIGHT_BOUND) {
                                    eventFire(&shData->comboEvent);
                                }
                                // Check if the touch is within bounds for right-to-left swipe within the time window
                                else if (layerEdge > 0 && currentTouch.x < SWIPE_LEFT_BOUND - 84 && lastTouchX >= SWIPE_LEFT_BOUND) {
                                    eventFire(&shData->comboEvent);
                                }
                            }
                        }
                    
                        // Handle touch release state
                        if (currentTouch.x == 0 && currentTouch.y == 0) {
                            internalTouchReleased = true;  // Indicate that the touch has been released
                            lastTouchX = currentTouch.x;
                        }

                        // If this is the first touch of a gesture, store lastTouchX
                        if (lastTouchX == 0 && currentTouch.x != 0) {
                            lastTouchX = currentTouch.x;
                            currentTouchTime = std::chrono::steady_clock::now();
                        }
                    
                    } else {
                        // Reset touch state if no touch is present
                        shData->touchState = { 0 };
                        internalTouchReleased = true;
                    
                        // Reset touch history to invalid state
                        lastTouchX = 0;
                    
                        // Reset time tracking
                        currentTouchTime = std::chrono::steady_clock::now();
                    }
                    

                    if ((((shData->keysHeld & tsl::cfg::launchCombo) == tsl::cfg::launchCombo) && shData->keysDown & tsl::cfg::launchCombo)) {
                        if (updateMenuCombos) {
                            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR, ULTRAHAND_COMBO_STR);
                            setIniFileValue(TESLA_CONFIG_INI_PATH, TESLA_STR, KEY_COMBO_STR, ULTRAHAND_COMBO_STR);
                            updateMenuCombos = false;
                        }
                        
                        if (shData->overlayOpen) {
                            tsl::Overlay::get()->hide();
                            shData->overlayOpen = false;
                        }
                        else {
                            eventFire(&shData->comboEvent);
                        }
                    }
                    else if (updateMenuCombos && (shData->keysHeld & tsl::cfg::launchCombo2) == tsl::cfg::launchCombo2) {
                        tsl::cfg::launchCombo = tsl::cfg::launchCombo2;
                        setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, KEY_COMBO_STR, TESLA_COMBO_STR);
                        setIniFileValue(TESLA_CONFIG_INI_PATH, TESLA_STR, KEY_COMBO_STR, TESLA_COMBO_STR);
                        eventFire(&shData->comboEvent);
                        updateMenuCombos = false;
                    }

                    
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
                            disableTransparency = true;
                            eventClear(&captureButtonPressEvent);
                            svcSleepThread(300'000'000);
                            disableTransparency = false;
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
        // std::string args = std::filesystem::path(ovlPath).filename();
        std::string args = getNameFromPath(ovlPath); // CUSTOM MODIFICATION
        args += " " + origArgs;
    
        // Check if "--skipCombo" is already in origArgs
        if (origArgs.find("--skipCombo") == std::string::npos) {
            args += " --skipCombo";
        }
    
        envSetNextLoad(ovlPath.c_str(), args.c_str());
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
        
        bool skipCombo = false;
        for (u8 arg = 0; arg < argc; arg++) {
            //if ((strcasecmp(argv[arg], "ovlmenu.ovl") == 0)) {
            //    isLauncher = true;
            //}
            if ((strcasecmp(argv[arg], "--skipCombo") == 0)) {
                skipCombo = true;
                //logMessage("Skip combo is present.");
                firstBoot = false;
                break;
            }
            //std::memset(argv[arg], 0, std::strlen(argv[arg]));
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
            overlay->initialize();
        });
        overlay->initScreen();
        overlay->changeTo(overlay->loadInitialGui());


        #if IS_LAUNCHER_DIRECTIVE
        if (firstBoot) {
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, FALSE_STR);
        }
        #endif
        
        
        bool inOverlay = (
            (parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR) != FALSE_STR)
        );
        if (inOverlay && skipCombo) {
            #if IS_LAUNCHER_DIRECTIVE
            setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, FALSE_STR);
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
            

            ASSERT_FATAL(timeInitialize()); // CUSTOM MODIFICATION
            __libnx_init_time();            // CUSTOM MODIFICATION
            timeExit(); // CUSTOM MODIFICATION

            #if USING_WIDGET_DIRECTIVE
            powerInit();
            i2cInitialize();
            #endif

            fsdevMountSdmc();
            splInitialize();
            spsmInitialize();
            //i2cInitialize();
            ASSERT_FATAL(socketInitializeDefault());
            ASSERT_FATAL(nifmInitialize(NifmServiceType_User));
        });
        ASSERT_FATAL(smInitialize()); // needed to prevent issues with powering device into sleep
    }
    
    /**
     * @brief libtesla service exiting function to override libnx's
     *
     */
    void __appExit(void) {
        
        socketExit();
        nifmExit();
        spsmExit();
        splExit();
        fsdevUnmountAll();
        
        #if USING_WIDGET_DIRECTIVE
        i2cExit();
        powerExit(); // CUSTOM MODIFICATION
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
