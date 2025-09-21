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

#if !USING_FSTREAM_DIRECTIVE
#include <stdio.h>
#else
#include <fstream>
#endif

#include <fnmatch.h>
#include <numeric>
#include <queue>
#include <mutex>
#include <condition_variable>
//#include <regex>
//#include <sys/statvfs.h>


using namespace ult;

static std::atomic<bool> abortCommand(false);
static std::atomic<bool> triggerExit(false);

std::atomic<bool> exitingUltrahand{false};
std::atomic<bool> isDownloadCommand{false};
std::atomic<bool> commandSuccess{false};
std::atomic<bool> refreshPage{false};
std::atomic<bool> refreshPackage{false};
std::atomic<bool> skipJumpReset{false};
std::atomic<bool> interpreterLogging{false};


std::atomic<bool> goBackAfter{false};

std::atomic<bool> usingErista{util::IsErista()};
std::atomic<bool> usingMariko{util::IsMariko()};

// Device info globals
static char amsVersion[12];
static char hosVersion[12];
static std::string memoryType;
static std::string memoryVendor = UNAVAILABLE_SELECTION;
static std::string memoryModel = UNAVAILABLE_SELECTION;
static std::string memorySize = UNAVAILABLE_SELECTION;
static uint32_t cpuSpeedo0, cpuSpeedo2, socSpeedo0; // CPU, GPU, SOC
static uint32_t cpuIDDQ, gpuIDDQ, socIDDQ;
static bool usingEmunand = true;



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

std::vector<std::string> getOverlayNames() {
    std::vector<std::string> names;
    const auto iniData = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
    for (const auto& [sectionName, _] : iniData) {
        names.push_back(sectionName);
    }
    return names;
}

std::vector<std::string> getPackageNames() {
    std::vector<std::string> names;
    const auto iniData = ult::getParsedDataFromIniFile(ult::PACKAGES_INI_FILEPATH);
    for (const auto& [sectionName, _] : iniData) {
        names.push_back(sectionName);
    }
    return names;
}



void removeKeyComboFromOthers(const std::string& keyCombo, const std::string& currentOverlay) {
    // Declare variables once for reuse across both scopes
    std::string existingCombo;
    std::string comboListStr;
    std::vector<std::string> comboList;
    bool modified;
    std::string newComboStr;
    
    // Process overlays first
    {
        auto overlaysIniData = ult::getParsedDataFromIniFile(ult::OVERLAYS_INI_FILEPATH);
        bool overlaysModified = false;
        
        const auto overlayNames = getOverlayNames();
        
        for (const auto& overlayName : overlayNames) {
            auto overlayIt = overlaysIniData.find(overlayName);
            if (overlayIt == overlaysIniData.end()) continue; // Skip if overlay not in INI
            
            auto& overlaySection = overlayIt->second;
            
            // 1. Remove from main key_combo field if it matches
            auto keyComboIt = overlaySection.find("key_combo");
            if (keyComboIt != overlaySection.end()) {
                existingCombo = keyComboIt->second;
                if (!existingCombo.empty() && tsl::hlp::comboStringToKeys(existingCombo) == tsl::hlp::comboStringToKeys(keyCombo)) {
                    overlaySection["key_combo"] = "";
                    overlaysModified = true;
                }
            }
            
            // 2. Remove from mode_combos list - clear ALL instances of this combo
            auto modeCombosIt = overlaySection.find("mode_combos");
            if (modeCombosIt != overlaySection.end()) {
                comboListStr = modeCombosIt->second;
            } else {
                comboListStr = "";
            }
            
            comboList = splitIniList(comboListStr);
            modified = false;
            
            for (size_t i = 0; i < comboList.size(); ++i) {
                if (!comboList[i].empty() && tsl::hlp::comboStringToKeys(comboList[i]) == tsl::hlp::comboStringToKeys(keyCombo)) {
                    comboList[i] = "";  // Clear ALL instances
                    modified = true;
                }
            }
            
            // Only update if something was actually removed
            if (modified) {
                newComboStr = "(" + joinIniList(comboList) + ")";
                overlaySection["mode_combos"] = newComboStr;
                overlaysModified = true;
            }
        }
        
        // Write back if modified, then clear memory
        if (overlaysModified) {
            ult::saveIniFileData(ult::OVERLAYS_INI_FILEPATH, overlaysIniData);
        }
        // overlaysIniData automatically cleared when scope ends
    }
    
    // Process packages second (overlays INI data is already cleared)
    {
        auto packagesIniData = ult::getParsedDataFromIniFile(ult::PACKAGES_INI_FILEPATH);
        bool packagesModified = false;
        
        auto packageNames = getPackageNames();
        
        for (const auto& packageName : packageNames) {
            auto packageIt = packagesIniData.find(packageName);
            if (packageIt == packagesIniData.end()) continue; // Skip if package not in INI
            
            auto& packageSection = packageIt->second;
            auto keyComboIt = packageSection.find("key_combo");
            if (keyComboIt != packageSection.end()) {
                existingCombo = keyComboIt->second; // Reusing the same variable
                if (!existingCombo.empty() && tsl::hlp::comboStringToKeys(existingCombo) == tsl::hlp::comboStringToKeys(keyCombo)) {
                    packageSection["key_combo"] = "";
                    packagesModified = true;
                }
            }
        }
        
        // Write back if modified, then clear memory
        if (packagesModified) {
            ult::saveIniFileData(ult::PACKAGES_INI_FILEPATH, packagesIniData);
        }
        // packagesIniData automatically cleared when scope ends
    }
}

// Define default key combos (same as UltrahandSettingsMenu)

const std::vector<std::string> defaultCombos = {
    // Primary (Default) Combos – top priority, always first
    "ZL+ZR+DDOWN",
    "ZL+ZR+DRIGHT",
    "ZL+ZR+DUP",
    "ZL+ZR+DLEFT",

    // Basic Combos – L/R with D-pad
    "L+R+DDOWN",
    "L+R+DRIGHT",
    "L+R+DUP",
    "L+R+DLEFT",
    "L+DDOWN",
    "R+DDOWN",

    // Menu Key Combos – involving PLUS/MINUS
    "ZL+ZR+PLUS",
    "L+R+PLUS",
    "ZL+ZR+MINUS",
    "L+R+MINUS",
    "ZL+MINUS",
    "ZR+MINUS",
    "ZL+PLUS",
    "ZR+PLUS",
    "MINUS+PLUS",

    // Stick Click Combos
    "LS+RS",
    "L+DDOWN+RS",
    "L+R+LS",
    "L+R+RS",

    // Advanced Combos – unlikely to conflict, good for extra features
    "ZL+ZR+LS",
    "ZL+ZR+RS",
    "ZL+ZR+L",
    "ZL+ZR+R",
    "ZL+ZR+LS+RS"
};

// Global constant map for button and arrow placeholders
const std::unordered_map<std::string, std::string> symbolPlaceholders = {
    {"{A}", ""},
    {"{B}", ""},
    {"{X}", ""},
    {"{Y}", ""},
    {"{L}", ""},
    {"{R}", ""},
    {"{ZL}", ""},
    {"{ZR}", ""},
    {"{DUP}", ""},
    {"{DDOWN}", ""},
    {"{DLEFT}", ""},
    {"{DRIGHT}", ""},
    {"{LS}", ""},
    {"{RS}", ""},
    {"{PLUS}", ""},
    {"{MINUS}", ""},
    {"{UP_ARROW}", ""},
    {"{DOWN_ARROW}", ""},
    {"{LEFT_ARROW}", ""},
    {"{RIGHT_ARROW}", ""},
    {"{RIGHT_UP_ARROW}", ""},
    {"{RIGHT_DOWN_ARROW}", ""},
    {"{LEFT_UP_ARROW}", ""},
    {"{LEFT_DOWN_ARROW}", ""},
    {"{POWER}", ""},
    {"{HOME}", ""},
    {"{CAPTURE}", ""},
    {"{REFRESH_SYMBOL}", ""},
    {"{WARNING_SYMBOL}", ""},
    {"{INFO_SYMBOL}", ""},
    {"{DIVIDER_SYMBOL}", ""}
};



bool parseVersion(const char* version, int& major, int& minor, int& patch) {
    major = 0;
    minor = 0;
    patch = 0;
    const char* ptr = version;

    // Parse major version
    while (*ptr >= '0' && *ptr <= '9') {
        major = major * 10 + (*ptr - '0');
        ++ptr;
    }
    if (*ptr != '.') return false;
    ++ptr;

    // Parse minor version
    while (*ptr >= '0' && *ptr <= '9') {
        minor = minor * 10 + (*ptr - '0');
        ++ptr;
    }
    if (*ptr != '.') return false;
    ++ptr;

    // Parse patch version
    while (*ptr >= '0' && *ptr <= '9') {
        patch = patch * 10 + (*ptr - '0');
        ++ptr;
    }
    return *ptr == '\0';
}

bool isVersionGreaterOrEqual(const char* currentVersion, const char* requiredVersion) {
    int currMajor, currMinor, currPatch;
    int reqMajor, reqMinor, reqPatch;

    if (!parseVersion(currentVersion, currMajor, currMinor, currPatch) ||
        !parseVersion(requiredVersion, reqMajor, reqMinor, reqPatch)) {
        return false; // Invalid format
    }

    // Compare each component
    if (currMajor > reqMajor) return true;
    if (currMajor < reqMajor) return false;
    if (currMinor > reqMinor) return true;
    if (currMinor < reqMinor) return false;
    return currPatch >= reqPatch;
}


//void testAudioOutput() {
//    Result res;
//    audoutInitialize();
//    
//    // Sample rate and buffer size
//    const size_t sampleRate = 48000; // 48 kHz
//    const size_t seconds = 1; // Duration of audio
//    const size_t bufferSize = sampleRate * sizeof(int16_t) * seconds;
//
//    // Allocate buffer dynamically to avoid large stack allocations
//    int16_t* buffer = (int16_t*)malloc(bufferSize);
//    if (buffer == nullptr) {
//        logMessage("Failed to allocate buffer memory!\n");
//        audoutExit();
//        return;
//    }
//    
//    // Generate a simple tone (sine wave)
//    float frequency = 400.0f; // 440 Hz tone
//    float amplitude = 0.5f;   // Volume
//
//    for (size_t i = 0; i < bufferSize / sizeof(int16_t); i++) {
//        float sample = amplitude * sinf(2.0f * M_PI * frequency * i / sampleRate);
//        buffer[i] = (int16_t)(sample * 32767.0f); // Convert to 16-bit PCM
//    }
//
//    // Prepare buffers
//    AudioOutBuffer audioBuffer;
//    audioBuffer.buffer = buffer;
//    audioBuffer.buffer_size = bufferSize;
//    audioBuffer.data_size = bufferSize;
//
//    // Start audio output
//    res = audoutStartAudioOut();
//    if (R_FAILED(res)) {
//        logMessage("Failed to start audio output!");
//        free(buffer);
//        audoutExit();
//        return;
//    }
//
//    // Play the sound
//    res = audoutAppendAudioOutBuffer(&audioBuffer);
//    if (R_FAILED(res)) {
//        logMessage("Failed to append audio buffer!");
//    }
//    
//    audoutWaitPlayFinish(NULL, NULL, 1000);
//
//    // Clean up
//    free(buffer);
//    audoutStopAudioOut();
//    audoutExit();
//}

#define FUSE_CPU_SPEEDO_0_CALIB 0x114
//#define FUSE_CPU_SPEEDO_1_CALIB 0x12C
#define FUSE_CPU_SPEEDO_2_CALIB 0x130

#define FUSE_SOC_SPEEDO_0_CALIB 0x134
//#define FUSE_SOC_SPEEDO_1_CALIB 0x138
//#define FUSE_SOC_SPEEDO_2_CALIB 0x13C

#define FUSE_CPU_IDDQ_CALIB 0x118
#define FUSE_SOC_IDDQ_CALIB 0x140
#define FUSE_GPU_IDDQ_CALIB 0x228


//bool areAllNonZero(const std::initializer_list<u32>& values) {
//    return std::all_of(values.begin(), values.end(), [](u32 value) { return value != 0; });
//}

//uint32_t readFuseValue(std::ifstream &file, std::streamoff offset) {
//    uint32_t value = 0;
//    // Seek to the specified offset
//    file.seekg(offset, std::ios::beg);
//    if (file.good()) {
//        // Read the value (assuming it's a 4-byte integer)
//        file.read(reinterpret_cast<char*>(&value), sizeof(value));
//    } else {
//        //std::cerr << "Error: Unable to seek to the specified offset." << std::endl;
//    }
//    return value;
//}

void writeFuseIni(const std::string& outputPath, const char* data = nullptr) {
#if !USING_FSTREAM_DIRECTIVE
    // Use stdio.h functions for file operations
    FILE* outFile = fopen(outputPath.c_str(), "w");
    if (outFile) {
        // Write the header in one call instead of three separate fwrite calls
        fputs("[", outFile);
        fputs(FUSE_STR.c_str(), outFile); 
        fputs("]\n", outFile);
        
        if (data) {
            // fprintf is necessary here for actual formatting - keep these
            fprintf(outFile, "cpu_speedo_0=%u\n", *reinterpret_cast<const uint32_t*>(data + FUSE_CPU_SPEEDO_0_CALIB));
            fprintf(outFile, "cpu_speedo_2=%u\n", *reinterpret_cast<const uint32_t*>(data + FUSE_CPU_SPEEDO_2_CALIB));
            fprintf(outFile, "soc_speedo_0=%u\n", *reinterpret_cast<const uint32_t*>(data + FUSE_SOC_SPEEDO_0_CALIB));
            fprintf(outFile, "cpu_iddq=%u\n", *reinterpret_cast<const uint32_t*>(data + FUSE_CPU_IDDQ_CALIB));
            fprintf(outFile, "soc_iddq=%u\n", *reinterpret_cast<const uint32_t*>(data + FUSE_SOC_IDDQ_CALIB));
            fprintf(outFile, "gpu_iddq=%u\n", *reinterpret_cast<const uint32_t*>(data + FUSE_GPU_IDDQ_CALIB));
            fputs("disable_reload=false\n", outFile);
        } else {
            // Single fputs call instead of seven separate ones
            fputs("cpu_speedo_0=\n"
                  "cpu_speedo_2=\n"
                  "soc_speedo_0=\n"
                  "cpu_iddq=\n"
                  "soc_iddq=\n"
                  "gpu_iddq=\n"
                  "disable_reload=false\n", outFile);
        }
        fclose(outFile);
    }
#else
    // Use fstream for file operations
    std::ofstream outFile(outputPath);
    if (outFile) {
        // Uncomment this line if needed to include the commented warning line
        // outFile.write("; do not adjust these values manually unless they were not dumped correctly\n", 81);

        outFile.write("[", 1);
        outFile.write(FUSE_STR.c_str(), FUSE_STR.size());
        outFile.write("]\n", 2);

        if (data) {
            outFile << "cpu_speedo_0=" << *reinterpret_cast<const uint32_t*>(data + FUSE_CPU_SPEEDO_0_CALIB) << '\n'
                    << "cpu_speedo_2=" << *reinterpret_cast<const uint32_t*>(data + FUSE_CPU_SPEEDO_2_CALIB) << '\n'
                    << "soc_speedo_0=" << *reinterpret_cast<const uint32_t*>(data + FUSE_SOC_SPEEDO_0_CALIB) << '\n'
                    << "cpu_iddq=" << *reinterpret_cast<const uint32_t*>(data + FUSE_CPU_IDDQ_CALIB) << '\n'
                    << "soc_iddq=" << *reinterpret_cast<const uint32_t*>(data + FUSE_SOC_IDDQ_CALIB) << '\n'
                    << "gpu_iddq=" << *reinterpret_cast<const uint32_t*>(data + FUSE_GPU_IDDQ_CALIB) << '\n'
                    << "disable_reload=false\n";
        } else {
            outFile << "cpu_speedo_0=\n"
                    << "cpu_speedo_2=\n"
                    << "soc_speedo_0=\n"
                    << "cpu_iddq=\n"
                    << "soc_iddq=\n"
                    << "gpu_iddq=\n"
                    << "disable_reload=false\n";
        }

        outFile.close();
    }
#endif
}


void fuseDumpToIni(const std::string& outputPath = FUSE_DATA_INI_PATH) {
    if (isFileOrDirectory(outputPath)) return;

    u64 pid = 0;
    if (R_FAILED(pmdmntGetProcessId(&pid, 0x0100000000000006))) {
        //pmdmntExit();
        writeFuseIni(outputPath);
        return;
    }
    //pmdmntExit();

    Handle debug;
    if (R_FAILED(svcDebugActiveProcess(&debug, pid))) {
        writeFuseIni(outputPath);
        return;
    }

    MemoryInfo mem_info = {0};
    u32 pageinfo = 0;
    u64 addr = 0;

    char stack[0x10] = {0};
    const char compare[0x10] = {0};
    char dump[0x400] = {0};

    while (true) {
        if (R_FAILED(svcQueryDebugProcessMemory(&mem_info, &pageinfo, debug, addr)) || mem_info.addr < addr) {
            break;
        }

        if (mem_info.type == MemType_Io && mem_info.size == 0x1000) {
            if (R_FAILED(svcReadDebugProcessMemory(stack, debug, mem_info.addr, sizeof(stack)))) {
                break;
            }

            if (std::memcmp(stack, compare, sizeof(stack)) == 0) {
                if (R_FAILED(svcReadDebugProcessMemory(dump, debug, mem_info.addr + 0x800, sizeof(dump)))) {
                    break;
                }

                writeFuseIni(outputPath, dump);
                svcCloseHandle(debug);
                return;
            }
        }

        addr = mem_info.addr + mem_info.size;
    }

    svcCloseHandle(debug);
    writeFuseIni(outputPath);
}


std::string getLocalIpAddress() {
    Result rc;
    u32 ipAddress;

    ASSERT_FATAL(nifmInitialize(NifmServiceType_User)); // for local IP

    // Get the current IP address
    rc = nifmGetCurrentIpAddress(&ipAddress);
    if (R_SUCCEEDED(rc)) {
        // Convert the IP address to a string
        char ipStr[16];
        snprintf(ipStr, sizeof(ipStr), "%u.%u.%u.%u",
                 ipAddress & 0xFF,
                 (ipAddress >> 8) & 0xFF,
                 (ipAddress >> 16) & 0xFF,
                 (ipAddress >> 24) & 0xFF);
        nifmExit();
        return std::string(ipStr);
    } else {
        // Return a default IP address if the IP could not be retrieved
        return UNAVAILABLE_SELECTION;  // Or "Unknown" if you prefer
    }
}




// Function to remove all empty command strings
void removeEmptyCommands(std::vector<std::vector<std::string>>& commands) {
    commands.erase(std::remove_if(commands.begin(), commands.end(),
        [](const std::vector<std::string>& vec) {
            return vec.empty();
        }),
        commands.end());
}




// Define the helper function
void formatVersion(uint64_t packed_version, int shift1, int shift2, int shift3, char* version_str) {
    sprintf(version_str, "%d.%d.%d",
            static_cast<uint8_t>((packed_version >> shift1) & 0xFF),
            static_cast<uint8_t>((packed_version >> shift2) & 0xFF),
            static_cast<uint8_t>((packed_version >> shift3) & 0xFF));
}


const char* getMemoryType(uint64_t packed_version) {
    switch (packed_version) {
        case 0: return "Samsung_K4F6E304HB-MGCH_4 GB LPDDR4 3200 Mbps";
        case 1: return "Hynix_H9HCNNNBPUMLHR-NLE_4 GB LPDDR4 3200 Mbps";
        case 2: return "Micron_MT53B512M32D2NP-062 WT:C_4 GB LPDDR4 3200 Mbps";
        case 3: return "Hynix_H9HCNNNBKMMLXR-NEE_4 GB LPDDR4X 4266 Mbps";
        case 4: return "Samsung_K4FHE3D4HM-MGCH_6GB LPDDR4 3200 Mbps";
        case 5: return "Hynix_H9HCNNNBKMMLXR-NEE_4 GB LPDDR4X 4266 Mbps";
        case 6: return "Hynix_H9HCNNNBKMMLXR-NEE_4 GB LPDDR4X 4266 Mbps";
        case 7: return "Samsung_K4FBE3D4HM-MGXX_8 GB LPDDR4 3200 Mbps";
        case 8: return "Samsung_K4U6E3S4AM-MGCJ_4 GB LPDDR4X 3733 Mbps";
        case 9: return "Samsung_K4UBE3D4AM-MGCJ_8 GB LPDDR4X 3733 Mbps";
        case 10: return "Hynix_H9HCNNNBKMMLHR-NME_4 GB LPDDR4X 3733 Mbps";
        case 11: return "Micron_MT53E512M32D2NP-046 WT:E_4 GB LPDDR4X 4266 Mbps";
        case 12: return "Samsung_K4U6E3S4AM-MGCJ_4 GB LPDDR4X 3733 Mbps";
        case 13: return "Samsung_K4UBE3D4AM-MGCJ_8 GB LPDDR4X 3733 Mbps";
        case 14: return "Hynix_H9HCNNNBKMMLHR-NME_4 GB LPDDR4X 3733 Mbps";
        case 15: return "Micron_MT53E512M32D2NP-046 WT:E_4 GB LPDDR4X 4266 Mbps";
        case 17: return "Samsung_K4U6E3S4AA-MGCL_4 GB LPDDR4X 4266 Mbps";
        case 18: return "Samsung_K4UBE3D4AA-MGCL_8 GB LPDDR4X 4266 Mbps";
        case 19: return "Samsung_K4U6E3S4AA-MGCL_4 GB LPDDR4X 4266 Mbps";
        case 20: return "Samsung_K4U6E3S4AB-MGCL_4 GB LPDDR4X 4266 Mbps";
        case 21: return "Samsung_K4U6E3S4AB-MGCL_4 GB LPDDR4X 4266 Mbps";
        case 22: return "Samsung_K4U6E3S4AB-MGCL_4 GB LPDDR4X 4266 Mbps";
        case 23: return "Samsung_K4UBE3D4AA-MGCL_8 GB LPDDR4X 4266 Mbps";
        case 24: return "Samsung_K4U6E3S4AA-MGCL_4 GB LPDDR4X 4266 Mbps";
        case 25: return "Micron_MT53E512M32D2NP-046 WT:F_4 GB LPDDR4X 4266 Mbps";
        case 26: return "Micron_MT53E512M32D2NP-046 WT:F_4 GB LPDDR4X 4266 Mbps";
        case 27: return "Micron_MT53E512M32D2NP-046 WT:F_4 GB LPDDR4X 4266 Mbps";
        case 28: return "Samsung_K4UBE3D4AA-MGCL_8 GB LPDDR4X 4266 Mbps";
        case 29: return "Hynix_H54G46CYRBX267_4 GB LPDDR4X 4266 Mbps";
        case 30: return "Hynix_H54G46CYRBX267_4 GB LPDDR4X 4266 Mbps";
        case 31: return "Hynix_H54G46CYRBX267_4 GB LPDDR4X 4266 Mbps";
        case 32: return "Micron_MT53E512M32D1NP-046 WT:B_4 GB LPDDR4X 4266 Mbps";
        case 33: return "Micron_MT53E512M32D1NP-046 WT:B_4 GB LPDDR4X 4266 Mbps";
        case 34: return "Micron_MT53E512M32D1NP-046 WT:B_4 GB LPDDR4X 4266 Mbps";

        case 16: return "Hynix_NMEM-DIE (Copper)_null";
        case 35: return "null_H54G46CYRBX267_null";
        case 36: return "Magnesium_MT53E512M32D1NP-046 WT:B_null";
        case 37: return "Samsung_K4FBE3D4HM-GHCL_null";
        case 38: return "Samsung_K4FBE3D4HM-GFCL_null";
        case 39: return "Samsung_K4FBE3D4HM-TFCL_null";
        case 40: return "Samsung_K4FBE3D4HM-MGCJ_null";
        case 41: return "Samsung_K4FBE3D4HM-THCL_null";
        case 42: return "Samsung_K4FBE3D4HM-SGCL_null";
        default: return "";
    }
}



const char* getStorageInfo(const std::string& storageType) {
    s64 freeSpace = 0;
    s64 totalSpace = 0;
    char* buffer = (char*)malloc(64);  // Increased size for percentage info

    FsFileSystem fs;
    Result result;

    // Open the appropriate file system based on storage type
    if (storageType == "emmc")
        result = fsOpenBisFileSystem(&fs, FsBisPartitionId_User, "");
    else if (storageType == "sdmc")
        result = fsOpenContentStorageFileSystem(&fs, FsContentStorageId_SdCard);
    else
        return buffer;

    if (R_FAILED(result)) {
        return buffer;
    }

    // Get free space on the storage
    result = fsFsGetFreeSpace(&fs, "/", &freeSpace);
    if (R_FAILED(result)) {
        fsFsClose(&fs);
        return buffer;
    }

    // Get total space on the storage
    result = fsFsGetTotalSpace(&fs, "/", &totalSpace);
    if (R_FAILED(result)) {
        fsFsClose(&fs);
        return buffer;
    }

    // Calculate the used space and usage percentage
    const s64 usedSpace = totalSpace - freeSpace;
    const float usedSpaceGB = usedSpace / (1024.0f * 1024.0f * 1024.0f);
    const float totalSpaceGB = totalSpace / (1024.0f * 1024.0f * 1024.0f);
    const int percentUsed = (totalSpace > 0) ? static_cast<int>((usedSpace * 100) / totalSpace) : 0;

    // Format the string with percentage
    snprintf(buffer, 64, "%.1f GB / %.1f GB (%d%%)", usedSpaceGB, totalSpaceGB, percentUsed);

    fsFsClose(&fs);
    return buffer;
}



void unpackDeviceInfo() {
    u64 packed_version;
    splGetConfig((SplConfigItem)2, &packed_version);
    memoryType = getMemoryType(packed_version);
    //memoryVendor = UNAVAILABLE_SELECTION;
    //memoryModel = UNAVAILABLE_SELECTION;
    //memorySize = UNAVAILABLE_SELECTION;
    
    if (!memoryType.empty()) {
        std::vector<std::string> memoryData = splitString(memoryType, "_");
        if (memoryData.size() > 0) memoryVendor = memoryData[0];
        if (memoryData.size() > 1) memoryModel = memoryData[1];
        if (memoryData.size() > 2) memorySize = memoryData[2];
    }
    splGetConfig((SplConfigItem)65000, &packed_version);
    
    // Format AMS version
    formatVersion(packed_version, 56, 48, 40, amsVersion);
    
    // Format HOS version
    formatVersion(packed_version, 24, 16, 8, hosVersion);
    splGetConfig((SplConfigItem)65007, &packed_version);
    usingEmunand = (packed_version != 0);
    fuseDumpToIni();
    
    if (isFileOrDirectory(FUSE_DATA_INI_PATH)) {
        // Load INI data once instead of 6 separate file reads
        const auto fuseSection = getKeyValuePairsFromSection(FUSE_DATA_INI_PATH, FUSE_STR);
        
        const std::pair<const char*, u32*> keys[] = {
            {"cpu_speedo_0", &cpuSpeedo0},
            {"cpu_speedo_2", &cpuSpeedo2},
            {"soc_speedo_0", &socSpeedo0},
            {"cpu_iddq", &cpuIDDQ},
            {"soc_iddq", &socIDDQ},
            {"gpu_iddq", &gpuIDDQ}
        };
        
        // Helper lambda to safely get u32 values
        auto getU32Value = [&](const std::string& key) -> u32 {
            auto it = fuseSection.find(key);
            return (it != fuseSection.end() && !it->second.empty()) ? ult::stoi(it->second) : 0;
        };
        
        for (const auto& key : keys) {
            *key.second = getU32Value(key.first);
        }
    }
}


// Escapes regex special chars except '*'
static std::string resolveWildcardFromKnownPath(
    const std::string& oldPattern,
    const std::string& resolvedPath,
    const std::string& newPattern
) {
    // Helper function to check if paths match and extract wildcard values
    auto matchAndExtract = [](const std::string& pattern, const std::string& path, std::vector<std::string>& captures) -> bool {
        size_t patternPos = 0;
        size_t pathPos = 0;
        size_t nextPatternPos;
        char nextChar;
        size_t foundPos;
        bool found;
        char patternChar;
        char pathChar;
        bool match;

        while (patternPos < pattern.size() && pathPos < path.size()) {
            if (pattern[patternPos] == '*') {
                // Find the next non-wildcard character in pattern
                nextPatternPos = patternPos + 1;
                while (nextPatternPos < pattern.size() && pattern[nextPatternPos] == '*') {
                    nextPatternPos++; // Skip consecutive wildcards
                }
                
                if (nextPatternPos >= pattern.size()) {
                    // Wildcard is at the end, capture everything remaining
                    captures.push_back(path.substr(pathPos));
                    return true;
                }
                
                // Find the next literal character after the wildcard
                nextChar = pattern[nextPatternPos];
                
                // Find where this character appears in the path
                foundPos = pathPos;
                found = false;
                
                // Look for the next literal sequence starting from current path position
                while (foundPos < path.size()) {
                    if ((nextChar == '/' || nextChar == '\\') && 
                        (path[foundPos] == '/' || path[foundPos] == '\\')) {
                        found = true;
                        break;
                    } else if (nextChar != '/' && nextChar != '\\' && 
                               std::tolower(path[foundPos]) == std::tolower(nextChar)) {
                        found = true;
                        break;
                    }
                    foundPos++;
                }
                
                if (!found) {
                    return false;
                }
                
                // Capture everything between current position and found position
                captures.push_back(path.substr(pathPos, foundPos - pathPos));
                pathPos = foundPos;
                patternPos = nextPatternPos;
            } else {
                // Literal character matching (case-insensitive, handle path separators)
                patternChar = pattern[patternPos];
                pathChar = path[pathPos];
                
                match = false;
                if ((patternChar == '/' || patternChar == '\\') && 
                    (pathChar == '/' || pathChar == '\\')) {
                    match = true;
                } else if (std::tolower(patternChar) == std::tolower(pathChar)) {
                    match = true;
                }
                
                if (!match) {
                    return false;
                }
                
                patternPos++;
                pathPos++;
            }
        }
        
        // Handle remaining wildcards at the end of pattern
        while (patternPos < pattern.size() && pattern[patternPos] == '*') {
            captures.push_back("");
            patternPos++;
        }
        
        // Pattern should be fully consumed, path can have remaining characters
        return patternPos >= pattern.size();
    };
    
    // Try to match the pattern at different positions in the resolved path (simulating .* anchors)
    std::vector<std::string> captures;
    bool matched = false;
    std::string subPath;

    for (size_t startPos = 0; startPos <= resolvedPath.size(); ++startPos) {
        captures.clear();
        //captures.shrink_to_fit();
        subPath = resolvedPath.substr(startPos);
        
        if (matchAndExtract(oldPattern, subPath, captures)) {
            matched = true;
            break;
        }
    }
    
    if (!matched) {
        return newPattern;
    }
    
    // Replace wildcards in newPattern with captured values
    std::string result = newPattern;
    size_t pos;
    for (size_t i = 0; i < captures.size(); ++i) {
        pos = result.find('*');
        if (pos == std::string::npos) break;
        result.replace(pos, 1, captures[i]);
    }
    
    return result;
}


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
        commandSuccess.store(false, std::memory_order_release);
        //LogLine("Error btmInitialize: %u - %X\n", rc, rc);
        return;
    }
    
    BtmConnectedDeviceV13 connected_devices[8];
    rc = btmGetDeviceCondition(BtmProfile_None, connected_devices, 8, &g_connected_count);
    if (R_SUCCEEDED(rc)) {
        for (s32 i = 0; i != g_connected_count; ++i) {
            g_addresses[i] = connected_devices[i].address;
        }
    } else {
        commandSuccess.store(false, std::memory_order_release);
        //LogLine("Error btmGetDeviceCondition: %u - %X\n", rc, rc);
    }
    
    if (R_SUCCEEDED(rc)) {
        //LogLine("Disconnecting controllers. Count: %u\n", g_connected_count);
        for (int i = 0; i != g_connected_count; ++i) {
            rc = btmHidDisconnect(g_addresses[i]);
            if (R_FAILED(rc)) {
                commandSuccess.store(false, std::memory_order_release);
                //LogLine("Error btmHidDisconnect: %u - %X\n", rc, rc);
            } else {
                //LogLine("Disconnected Address: %u - %X\n", g_addresses[i], g_addresses[i]);
            }
        }
        //LogLine("All controllers disconnected.\n");
    } else {
        commandSuccess.store(false, std::memory_order_release);
    }
    
    // Exit Bluetooth manager
    btmExit();
}




void initializeTheme(const std::string& themeIniPath = THEME_CONFIG_INI_PATH) {
    // Load INI data once
    tsl::hlp::ini::IniData themeData = getParsedDataFromIniFile(themeIniPath);
    bool needsUpdate = false;
    
    // Check if file exists and has theme section
    //const bool fileExists = isFileOrDirectory(themeIniPath);
    const bool hasThemeSection = isFileOrDirectory(themeIniPath) && (themeData.count(THEME_STR) > 0);
    
    if (hasThemeSection) {
        // File exists with theme section - check for missing keys
        auto& themeSection = themeData[THEME_STR];
        for (const auto& [key, value] : defaultThemeSettingsMap) {
            if (themeSection.count(key) == 0) {
                themeSection[key] = value;
                needsUpdate = true;
            }
        }
    } else {
        // File doesn't exist or theme section is missing - initialize all defaults
        auto& themeSection = themeData[THEME_STR];
        for (const auto& [key, value] : defaultThemeSettingsMap) {
            themeSection[key] = value;
        }
        needsUpdate = true;
    }
    
    // Write back only if changes were made
    if (needsUpdate) {
        saveIniFileData(themeIniPath, themeData);
    }
    
    // Ensure themes directory exists
    if (!isFileOrDirectory(THEMES_PATH)) {
        createDirectory(THEMES_PATH);
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
    
    const bool teslaConfigExists = isFileOrDirectory(TESLA_CONFIG_INI_PATH);
    const bool ultrahandConfigExists = isFileOrDirectory(ULTRAHAND_CONFIG_INI_PATH);

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
constexpr uint32_t ULTR_SIGNATURE = 0x52544C55;

/**
 * @brief Retrieves overlay module information from a given file.
 *
 * @param filePath The path to the overlay module file.
 * @return A tuple containing the result code, module name, and display version.
 */
std::tuple<Result, std::string, std::string, bool> getOverlayInfo(const std::string& filePath) {
    
#if !USING_FSTREAM_DIRECTIVE
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        return {ResultParseError, "", "", false};
    }
    
    // Get file size once for bounds checking
    fseek(file, 0, SEEK_END);
    const long fileSize = ftell(file);
    if (static_cast<size_t>(fileSize) < sizeof(NroStart) + sizeof(NroHeader)) {
        fclose(file);
        return {ResultParseError, "", "", false};
    }
    
    // Read NRO header
    fseek(file, sizeof(NroStart), SEEK_SET);
    NroHeader nroHeader;
    if (fread(&nroHeader, sizeof(NroHeader), 1, file) != 1) {
        fclose(file);
        return {ResultParseError, "", "", false};
    }
    
    // Early validation of header size
    if (nroHeader.size >= fileSize) {
        fclose(file);
        return {ResultParseError, "", "", false};
    }
    
    // Check signature and read asset header in one operation
    bool usingLibUltrahand = false;
    uint32_t signature;
    if (fileSize >= 4 && fseek(file, -4, SEEK_END) == 0 && 
        fread(&signature, 4, 1, file) == 1 && signature == ULTR_SIGNATURE) {
        usingLibUltrahand = true;
    }
    
    // Read asset header
    fseek(file, nroHeader.size, SEEK_SET);
    NroAssetHeader assetHeader;
    if (fread(&assetHeader, sizeof(NroAssetHeader), 1, file) != 1) {
        fclose(file);
        return {ResultParseError, "", "", false};
    }
    
    // Validate NACP offset before seeking
    const size_t nacpPos = nroHeader.size + assetHeader.nacp.offset;
    if (nacpPos + sizeof(NacpStruct) > static_cast<size_t>(fileSize)) {
        fclose(file);
        return {ResultParseError, "", "", false};
    }
    
    // Read NACP struct
    fseek(file, nacpPos, SEEK_SET);
    NacpStruct nacp;
    if (fread(&nacp, sizeof(NacpStruct), 1, file) != 1) {
        fclose(file);
        return {ResultParseError, "", "", false};
    }
    
    fclose(file);
    
#else
    // Optimized std::ifstream version
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return {ResultParseError, "", "", false};
    }
    
    // Get file size for validation
    file.seekg(0, std::ios::end);
    const auto fileSize = file.tellg();
    if (fileSize < sizeof(NroStart) + sizeof(NroHeader)) {
        return {ResultParseError, "", "", false};
    }
    
    // Read NRO header
    file.seekg(sizeof(NroStart), std::ios::beg);
    NroHeader nroHeader;
    if (!file.read(reinterpret_cast<char*>(&nroHeader), sizeof(NroHeader))) {
        return {ResultParseError, "", "", false};
    }
    
    // Early validation
    if (nroHeader.size >= fileSize) {
        return {ResultParseError, "", "", false};
    }
    
    // Check signature
    bool usingLibUltrahand = false;
    if (fileSize >= 4) {
        file.seekg(-4, std::ios::end);
        uint32_t signature;
        if (file.read(reinterpret_cast<char*>(&signature), 4) && signature == ULTR_SIGNATURE) {
            usingLibUltrahand = true;
        }
    }
    
    // Read asset header
    file.seekg(nroHeader.size, std::ios::beg);
    NroAssetHeader assetHeader;
    if (!file.read(reinterpret_cast<char*>(&assetHeader), sizeof(NroAssetHeader))) {
        return {ResultParseError, "", "", false};
    }
    
    // Validate NACP position
    const auto nacpPos = nroHeader.size + assetHeader.nacp.offset;
    if (nacpPos + sizeof(NacpStruct) > fileSize) {
        return {ResultParseError, "", "", false};
    }
    
    // Read NACP struct
    file.seekg(nacpPos, std::ios::beg);
    NacpStruct nacp;
    if (!file.read(reinterpret_cast<char*>(&nacp), sizeof(NacpStruct))) {
        return {ResultParseError, "", "", false};
    }
#endif
    
    // Optimized string construction using string_view-like approach
    // Find string ends using pointer arithmetic (faster than indexing)
    const char* nameStart = nacp.lang[0].name;
    const char* nameEnd = nameStart;
    const char* nameLimit = nameStart + sizeof(nacp.lang[0].name);
    while (nameEnd < nameLimit && *nameEnd != '\0') ++nameEnd;
    
    const char* versionStart = nacp.display_version;
    const char* versionEnd = versionStart;
    const char* versionLimit = versionStart + sizeof(nacp.display_version);
    while (versionEnd < versionLimit && *versionEnd != '\0') ++versionEnd;
    
    return {
        ResultSuccess,
        std::string(nameStart, nameEnd - nameStart),
        std::string(versionStart, versionEnd - versionStart),
        usingLibUltrahand
    };
}

void addHeader(auto& list, const std::string& headerText) {
    list->addItem(new tsl::elm::CategoryHeader(headerText));
}

void addBasicListItem(auto& list, const std::string& itemText, bool isMini = false) {
    list->addItem(new tsl::elm::ListItem(itemText, "", isMini));
}

void addDummyListItem(auto& list, s32 index = -1) {
    list->addItem(new tsl::elm::DummyListItem(), 0, index);
}

// Helper function to wrap text into multiple lines based on a maximum width (character count)
// Subsequent lines are indented by 4 spaces
//std::vector<std::string> wrapText(const std::string& text, size_t maxWidth) {
//    std::vector<std::string> wrappedLines;
//    //std::string indent = "    ";  // 4 spaces for indentation
//    std::string indent = "└ ";
//    size_t currentPos = 0;
//
//    // First line (no indentation)
//    wrappedLines.push_back(text.substr(currentPos, maxWidth));
//    currentPos += maxWidth;
//
//    // Subsequent lines (indented by 4 spaces)
//    while (currentPos < text.size()) {
//        wrappedLines.push_back(indent + text.substr(currentPos, maxWidth));
//        currentPos += maxWidth;
//    }
//
//    return wrappedLines;
//}


bool applyPlaceholderReplacements(std::vector<std::string>& cmd, const std::string& hexPath, const std::string& iniPath, const std::string& listString, const std::string& listPath, const std::string& jsonString, const std::string& jsonPath);

std::string getFirstSectionText(const std::vector<std::vector<std::string>>& tableData, const std::string& packagePath) {
    std::string message;
    std::string listFileSourcePath;
    std::string hexPath, iniPath, listString, listPath, jsonString, jsonPath;
    
    bool inEristaSection = false;
    bool inMarikoSection = false;
    std::vector<std::string> lines;
    
    std::vector<std::string> cmd;
    for (const auto& commands : tableData) {
        cmd = commands;  // Make a copy if you need to modify it

        if (cmd.empty()) {
            continue;
        }

        const std::string& commandName = cmd[0];

        if (commandName == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            continue;
        } else if (commandName == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            continue;
        }

        if ((inEristaSection && !inMarikoSection && usingErista) ||
            (!inEristaSection && inMarikoSection && usingMariko) ||
            (!inEristaSection && !inMarikoSection)) {

            // Apply placeholder replacements if necessary
            applyPlaceholderReplacements(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

            const size_t cmdSize = cmd.size();

            if (commandName == "list_file_source" && listFileSourcePath.empty()) {
                listFileSourcePath = cmd[1];
                preprocessPath(listFileSourcePath, packagePath);

                // Read lines from the file
                lines = readListFromFile(listFileSourcePath);

                // Return the first line if available
                if (!lines.empty()) {
                    return lines[0];
                }
            } else if (commandName == LIST_STR) {
                if (cmdSize >= 2) {
                    listString = cmd[1];
                    removeQuotes(listString);
                    // Process listString if needed
                }
            } else if (commandName == LIST_FILE_STR) {
                if (cmdSize >= 2) {
                    listPath = cmd[1];
                    preprocessPath(listPath, packagePath);
                    // Read from listPath if needed
                }
            } else if (commandName == JSON_STR) {
                if (cmdSize >= 2) {
                    jsonString = cmd[1];
                    // Process jsonString if needed
                }
            } else if (commandName == JSON_FILE_STR) {
                if (cmdSize >= 2) {
                    jsonPath = cmd[1];
                    preprocessPath(jsonPath, packagePath);
                    // Read from jsonPath if needed
                }
            } else if (commandName == INI_FILE_STR) {
                if (cmdSize >= 2) {
                    iniPath = cmd[1];
                    preprocessPath(iniPath, packagePath);
                    // Read from iniPath if needed
                }
            } else if (commandName == HEX_FILE_STR) {
                if (cmdSize >= 2) {
                    hexPath = cmd[1];
                    preprocessPath(hexPath, packagePath);
                    // Read from hexPath if needed
                }
            } else {
                // Default case: This is where sectionLines are populated in addTable
                // Return cmd[0] as the first section text
                return cmd[0];
            }
        }
    }

    // If no section text is found, return an empty string or a default value
    return "";  // Or return a default message if appropriate
}


void addGap(tsl::elm::List* list, s32 gapHeight) {
    list->addItem(new tsl::elm::CustomDrawer(
        [](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
            // Empty drawer - just creates space
        }
    ), gapHeight);
}

std::vector<std::string> wrapText(
    const std::string& text,
    float maxWidth,
    const std::string& wrappingMode,
    bool useIndent,
    const std::string& indent,
    float indentWidth,
    size_t fontSize
) {
    if (wrappingMode == "none" || (wrappingMode != "char" && wrappingMode != "word")) {
        return { text };
    }

    std::vector<std::string> wrappedLines;

    bool firstLine = true;
    std::string currentLine;

    if (wrappingMode == "char") {
        for (char c : text) {
            const float currentMaxWidth = firstLine ? maxWidth : maxWidth - indentWidth;

            currentLine.push_back(c);
            if (tsl::gfx::calculateStringWidth(currentLine, fontSize, false) > currentMaxWidth) {
                // Remove last character and push line
                const char lastChar = currentLine.back();
                currentLine.pop_back();
                if (!currentLine.empty()) {
                    if (useIndent && !firstLine)
                        wrappedLines.push_back(indent + currentLine);
                    else
                        wrappedLines.push_back(currentLine);
                }
                currentLine = lastChar;
                firstLine = false;
            }
        }

        if (!currentLine.empty()) {
            if (useIndent && !firstLine)
                wrappedLines.push_back(indent + currentLine);
            else
                wrappedLines.push_back(currentLine);
        }
    } 
    else {
        // Word wrapping
        StringStream stream(text);
        std::string currentWord;

        std::string testLine;

        while (stream >> currentWord) {
            const float currentMaxWidth = firstLine ? maxWidth : maxWidth - indentWidth;

            testLine = currentLine;
            if (!testLine.empty()) testLine.push_back(' ');
            testLine += currentWord;

            if (tsl::gfx::calculateStringWidth(testLine, fontSize, false) > currentMaxWidth) {
                if (!currentLine.empty()) {
                    if (useIndent && !firstLine)
                        wrappedLines.push_back(indent + currentLine);
                    else
                        wrappedLines.push_back(currentLine);
                }
                currentLine = std::move(currentWord);
                firstLine = false;
            } else {
                currentLine.swap(testLine);
            }
        }

        if (!currentLine.empty()) {
            if (useIndent && !firstLine)
                wrappedLines.push_back(indent + currentLine);
            else
                wrappedLines.push_back(currentLine);
        }
    }

    return wrappedLines;
}

// ─── Helper: flatten + placeholder + wrap & expand ─────────────────────────────
static bool buildTableDrawerLines(
    const std::vector<std::vector<std::string>>& tableData,
    std::vector<std::string>&                    sectionLines,
    std::vector<std::string>&                    infoLines,
    const std::string&                           packagePath,
    size_t                                       columnOffset,
    size_t                                       startGap,
    size_t                                       newlineGap,
    const std::string&                           wrappingMode,
    const std::string&                           alignment,
    bool                                         useWrappedTextIndent,
    std::vector<std::string>&                    outSection,
    std::vector<std::string>&                    outInfo,
    std::vector<s32>&                            outY,
    std::vector<int>&                            outX
) {
    static constexpr size_t lineHeight = 16;
    static constexpr size_t fontSize = 16;
    const size_t xMax = tsl::cfg::FramebufferWidth - 95;
    const std::string indent = "└ ";
    const float indentWidth = tsl::gfx::calculateStringWidth(indent, fontSize, false);

    outSection.clear();
    //outSection.shrink_to_fit();
    outInfo.clear();
    //outInfo.shrink_to_fit();
    outY.clear();
    //outY.shrink_to_fit();
    outX.clear();
    //outX.shrink_to_fit();

    size_t curY = startGap;
    bool anyReplacementsMade = false;

    // A small lambda to wrap and push lines with proper x,y and info alignment
    auto processLines = [&](const std::vector<std::string>& lines, const std::vector<std::string>& infos) {
        std::string infoText;
        int xPos;
        float infoWidth;
        std::vector<std::string> wrappedLines;
        for (size_t i = 0; i < lines.size(); ++i) {
            const std::string& baseText = lines[i];
            const std::string& infoTextRaw = (i < infos.size()) ? infos[i] : "";
            infoText = (infoTextRaw.find(NULL_STR) != std::string::npos) ? UNAVAILABLE_SELECTION : infoTextRaw;

            // Wrap the base text according to wrappingMode and indent params
            wrappedLines = wrapText(
                baseText,
                xMax - 8,
                wrappingMode,
                useWrappedTextIndent,
                indent, indentWidth,
                fontSize
            );

            // Cache width of info text only once per base line, not per wrapped line
            infoWidth = tsl::gfx::calculateStringWidth(infoText, fontSize, false);

            for (auto& line : wrappedLines) {
                outSection.push_back(std::move(line));
                line.shrink_to_fit();
                outInfo.push_back(infoText);

                xPos = 0;
                if (alignment == LEFT_STR) {
                    xPos = static_cast<int>(columnOffset);
                } else if (alignment == RIGHT_STR) {
                    xPos = static_cast<int>(xMax - infoWidth + (columnOffset - 160 + 1));
                } else { // CENTER_STR
                    xPos = static_cast<int>(columnOffset + (xMax - infoWidth) / 2);
                }

                outX.push_back(xPos);
                outY.push_back(static_cast<s32>(curY));
                curY += lineHeight + newlineGap;
            }
        }
    };

    if (!tableData.empty()) {
        std::vector<std::string> baseSection;
        std::vector<std::string> baseInfo;
        std::vector<std::string> lines;

        std::string listFileSourcePath;
        std::string hexPath, iniPath, listString, listPath, jsonString, jsonPath;
        bool inErista = false, inMariko = false;

        for (const auto& cmds : tableData) {
            if (cmds.empty()) continue;
            const auto& name = cmds[0];

            if (name == "erista:") {
                inErista = true;
                inMariko = false;
                continue;
            }
            if (name == "mariko:") {
                inErista = false;
                inMariko = true;
                continue;
            }

            if ((inErista && usingErista) || (inMariko && usingMariko) || (!inErista && !inMariko)) {
                auto cmd = cmds;  // Copy for placeholder replacements

                // Track if any placeholder replacements were made
                if (applyPlaceholderReplacements(
                    cmd, hexPath, iniPath,
                    listString, listPath,
                    jsonString, jsonPath
                )) {
                    anyReplacementsMade = true;
                }

                if (cmd[0] == "list_file_source" && cmd.size() >= 2 && listFileSourcePath.empty()) {
                    listFileSourcePath = cmd[1];
                    preprocessPath(listFileSourcePath, packagePath);
                    lines = readListFromFile(listFileSourcePath);
                    for (const auto& line : lines) {
                        baseSection.push_back(line);
                        baseInfo.push_back("");
                    }
                }
                else if (cmd[0] == LIST_STR && cmd.size() >= 2) {
                    listString = cmd[1];
                    removeQuotes(listString);
                }
                else if (cmd[0] == LIST_FILE_STR && cmd.size() >= 2) {
                    listPath = cmd[1];
                    preprocessPath(listPath, packagePath);
                }
                else if (cmd[0] == JSON_STR && cmd.size() >= 2) {
                    jsonString = cmd[1];
                }
                else if (cmd[0] == JSON_FILE_STR && cmd.size() >= 2) {
                    jsonPath = cmd[1];
                    preprocessPath(jsonPath, packagePath);
                }
                else if (cmd[0] == INI_FILE_STR && cmd.size() >= 2) {
                    iniPath = cmd[1];
                    preprocessPath(iniPath, packagePath);
                }
                else if (cmd[0] == HEX_FILE_STR && cmd.size() >= 2) {
                    hexPath = cmd[1];
                    preprocessPath(hexPath, packagePath);
                }
                else {
                    baseSection.push_back(cmd[0]);
                    baseInfo.push_back(cmd.size() > 2 ? cmd[2] : "");
                }
            }
        }
        processLines(baseSection, baseInfo);
    } else {
        processLines(sectionLines, infoLines);
    }
    
    // Return true if any placeholder replacements were made
    return anyReplacementsMade;
}


// Helper function moved outside - no lambda creation overhead
static tsl::Color getRawColor(const std::string& c, tsl::Color defaultColor) {
    // Check most common/special case first
    if (c == DEFAULT_STR) return defaultColor;
    
    // Quick length check to eliminate impossible matches
    const size_t len = c.length();
    
    // Group by length to reduce comparisons
    switch (len) {
        case 4:
            if (c == "text") return tsl::defaultTextColor;
            if (c == "info") return tsl::infoTextColor;
            break;
            
        case 6:
            if (c == "header") return tsl::headerTextColor;
            break;
            
        case 7:
            if (c[0] == 'w' && c == "warning") return tsl::warningTextColor;
            if (c[0] == 's' && c == "section") return tsl::sectionTextColor;
            if (c[0] == 'b' && c == "bad_ram") return tsl::badRamTextColor;
            break;
            
        case 8:
            if (c == "on_value") return tsl::onTextColor;
            break;
            
        case 9:
            if (c == "off_value") return tsl::offTextColor;
            break;
            
        case 11:
            if (c[0] == 'h' && c == "healthy_ram") return tsl::healthyRamTextColor;
            if (c[0] == 'n' && c == "neutral_ram") return tsl::neutralRamTextColor;
            break;
    }
    
    return tsl::RGB888(c);
}

void drawTable(
    tsl::elm::List*      list,
    const std::vector<std::vector<std::string>>& tableData,
    std::vector<std::string>&             sectionLines,
    std::vector<std::string>&             infoLines,
    size_t columnOffset             = 164,
    size_t startGap                 = 20,
    size_t endGap                   = 9,
    size_t newlineGap               = 4,
    const std::string& tableSectionTextColor       = DEFAULT_STR,
    const std::string& tableInfoTextColor          = DEFAULT_STR,
    const std::string& tableInfoTextHighlightColor = DEFAULT_STR,
    const std::string& alignment                   = LEFT_STR,
    bool hideTableBackground        = false,
    bool useHeaderIndent            = false,
    bool isPolling                  = false,
    bool isScrollable               = true,
    const std::string& wrappingMode               = "none",
    bool useWrappedTextIndent        = false,
    std::string packagePath          = ""
) {
    // Prebuild initial buffers
    std::vector<std::string> cacheExpSec, cacheExpInfo;
    std::vector<s32>         cacheYOff;
    std::vector<s32>         cacheXOff;

    const bool usingPlaceholders = buildTableDrawerLines(
        tableData, sectionLines, infoLines, packagePath,
        columnOffset, startGap, newlineGap,
        wrappingMode, alignment, useWrappedTextIndent,
        cacheExpSec, cacheExpInfo, cacheYOff, cacheXOff
    ) && isPolling;

    // Resolve colors once outside the render loop
    const auto secRaw = getRawColor(tableSectionTextColor, tsl::sectionTextColor);
    const auto infoRaw = getRawColor(tableInfoTextColor, tsl::infoTextColor);
    const auto hiliteRaw = getRawColor(tableInfoTextHighlightColor, tsl::infoTextColor);
    
    // Pre-calculate color comparison
    const bool sameCol = (tableInfoTextColor == tableInfoTextHighlightColor);

    // Use nanoseconds for high-performance timing
    auto lastUpdateNS = std::make_shared<u64>(armTicksToNs(armGetSystemTick()));
    static constexpr u64 ONE_SECOND_NS = 1000000000ULL;

    static const std::vector<std::string> specialCharacters =  {""};
    
    list->addItem(new tsl::elm::TableDrawer(
        [=](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) mutable {

            if (usingPlaceholders) {
                const u64 currentNS = armTicksToNs(armGetSystemTick());
                
                if ((currentNS - *lastUpdateNS) >= ONE_SECOND_NS) {
                    buildTableDrawerLines(
                        tableData, sectionLines, infoLines, packagePath,
                        columnOffset, startGap, newlineGap,
                        wrappingMode, alignment, useWrappedTextIndent,
                        cacheExpSec, cacheExpInfo, cacheYOff, cacheXOff
                    );
                    *lastUpdateNS = currentNS;
                }
            }

            // Minimal branching, maximum cache efficiency
            if (useHeaderIndent) {
                renderer->drawRect(x-2, y, 4, 22, (tsl::headerSeparatorColor));
            }

            // Convert to renderer colors once per frame
            const auto secColor = (secRaw);
            const auto infoColor = (infoRaw);
            const auto dividerColor = (tsl::textSeparatorColor);
            
            const size_t count = cacheExpSec.size();
            const s32 baseX = x + 12;
            
            if (sameCol) {
                // Fastest path: same colors, minimal function calls
                for (size_t i = 0; i < count; ++i) {
                    const s32 yPos = y + cacheYOff[i];
                    renderer->drawStringWithColoredSections(cacheExpSec[i], false, specialCharacters, baseX, yPos, 16, secColor, dividerColor);
                    renderer->drawStringWithColoredSections(cacheExpInfo[i], false, specialCharacters, x + cacheXOff[i], yPos, 16, infoColor, dividerColor);
                }
            } else {
                // Different colors path
                const auto hiliteColor = hiliteRaw;
                
                for (size_t i = 0; i < count; ++i) {
                    const s32 yPos = y + cacheYOff[i];
                    renderer->drawStringWithColoredSections(cacheExpSec[i], false, specialCharacters, baseX, yPos, 16, secColor, dividerColor);
                    renderer->drawStringWithHighlight(
                        cacheExpInfo[i], false, x + cacheXOff[i], yPos, 16,
                        infoColor, hiliteColor
                    );
                }
            }
        },
        hideTableBackground,
        endGap,
        isScrollable
    ),
    static_cast<u32>(
        16 * cacheExpSec.size()
        + newlineGap * (cacheExpSec.empty() ? 0 : cacheExpSec.size() - 1)
        + endGap
    ));
}

// ─── addTable simply forwards through ───────────────────────────────────────────
void addTable(
    tsl::elm::List*       list,
    const std::vector<std::vector<std::string>>& tableData,
    const std::string&                     packagePath,
    const size_t&                          columnOffset                = 164,
    const size_t&                          tableStartGap               = 20,
    const size_t&                          tableEndGap                 = 9,
    const size_t&                          tableSpacing                = 0,
    const std::string&                     tableSectionTextColor       = DEFAULT_STR,
    const std::string&                     tableInfoTextColor          = DEFAULT_STR,
    const std::string&                     tableInfoTextHighlightColor = DEFAULT_STR,
    const std::string&                     tableAlignment              = RIGHT_STR,
    const bool&                            hideTableBackground         = false,
    const bool&                            useHeaderIndent             = false,
    const bool&                            isPolling                   = false,
    const bool&                            isScrollable                = true,
    const std::string&                     wrappingMode                = "none",
    const bool&                            useWrappedTextIndent        = false
) {
    std::vector<std::string> sectionLines, infoLines;
    drawTable(
        list, tableData,
        sectionLines, infoLines,
        columnOffset, tableStartGap, tableEndGap, tableSpacing,
        tableSectionTextColor, tableInfoTextColor, tableInfoTextHighlightColor,
        tableAlignment, hideTableBackground, useHeaderIndent,
        isPolling, isScrollable, wrappingMode, useWrappedTextIndent,
        packagePath
    );
}


void addHelpInfo(tsl::elm::List* list) {
    // Add a section break with small text to indicate the "Commands" section
    addHeader(list, USER_GUIDE);

    // Adjust the horizontal offset as needed
    const int xOffset = ult::stoi(USERGUIDE_OFFSET);

    // Define the section lines and info lines directly
    std::vector<std::string> sectionLines = {
        SETTINGS_MENU,
        SCRIPT_OVERLAY,
        STAR_FAVORITE,
        APP_SETTINGS
    };

    std::vector<std::string> infoLines = {
        "\uE0B5 (" + ON_MAIN_MENU + ")",
        "\uE0B6 (" + ON_A_COMMAND + ")",
        "\uE0E2 (" + ON_OVERLAY_PACKAGE + ")",
        "\uE0E3 (" + ON_OVERLAY_PACKAGE + ")"
    };

    std::vector<std::vector<std::string>> dummyTableData;

    // Draw the table with the defined lines
    drawTable(list, dummyTableData, sectionLines, infoLines, xOffset, 20, 9, 4);
    //drawTable(list, sectionLines, infoLines, xOffset, 19, 12, 4, DEFAULT_STR, DEFAULT_STR, LEFT_STR, false, false, true, "none", false);
}



void addPackageInfo(tsl::elm::List* list, auto& packageHeader, std::string type = PACKAGE_STR) {
    // Add a section break with small text to indicate the "Commands" section
    addHeader(list, (type == PACKAGE_STR ? PACKAGE_INFO : OVERLAY_INFO));

    const int maxLineLength = 28;  // Adjust the maximum line length as needed
    const int xOffset = 120;    // Adjust the horizontal offset as needed
    //int numEntries = 0;   // Count of the number of entries

    std::vector<std::string> sectionLines;
    std::vector<std::string> infoLines;

    // Helper function to add text with wrapping
    auto addWrappedText = [&](const std::string& header, const std::string& text) {
        sectionLines.push_back(header);
        const std::string::size_type aboutHeaderLength = header.length();
        
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
        sectionLines.push_back(_TITLE);
        infoLines.push_back(packageHeader.title);
        //numEntries++;
    }

    if (!packageHeader.version.empty()) {
        sectionLines.push_back(_VERSION);
        infoLines.push_back(packageHeader.version);
        //numEntries++;
    }

    if (!packageHeader.creator.empty()) {
        //sectionLines.push_back(CREATOR);
        //infoLines.push_back(packageHeader.creator);
        //numEntries++;
        addWrappedText(_CREATOR, packageHeader.creator);
    }

    if (!packageHeader.about.empty()) {
        addWrappedText(_ABOUT, packageHeader.about);
    }

    if (!packageHeader.credits.empty()) {
        addWrappedText(_CREDITS, packageHeader.credits);
    }

    std::vector<std::vector<std::string>> dummyTableData;

    // Drawing the table with section lines and info lines
    //drawTable(list, sectionLines, infoLines, xOffset, 20, 12, 3);
    drawTable(list, dummyTableData, sectionLines, infoLines, xOffset, 20, 9, 3, DEFAULT_STR, DEFAULT_STR, DEFAULT_STR, LEFT_STR, false, false, true);
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

bool isDangerousCombination(const std::string& originalPath) {
    // Early exit: Check for double wildcards first (cheapest check)
    if (originalPath.find("**") != std::string::npos) {
        return true;
    }
    
    // Early exit: Check if path contains wildcards at all
    const bool hasWildcards = originalPath.find('*') != std::string::npos;
    
    // 1) Normalize repeated wildcards only if wildcards exist
    std::string patternPath;
    if (hasWildcards) {
        //patternPath.reserve(originalPath.length()); // Avoid reallocations
        bool lastWasStar = false;
        for (char c : originalPath) {
            if (c == '*') {
                if (!lastWasStar) {
                    patternPath += c;
                    lastWasStar = true;
                }
                // skip extra '*'
            } else {
                patternPath += c;
                lastWasStar = false;
            }
        }
    } else {
        patternPath = originalPath; // No wildcards, use as-is
    }
    
    // 2) Define folder sets with precomputed lengths for faster comparisons
    struct FolderInfo {
        const char* path;
        size_t length;
    };
    
    static constexpr FolderInfo albumFolders[] = {
        {"sdmc:/Nintendo/Album/", 20},
        {"sdmc:/emuMMC/RAW1/Nintendo/Album/", 33}
    };
    
    static constexpr FolderInfo ultraProtectedFolders[] = {
        {"sdmc:/Nintendo/Contents/", 23},
        {"sdmc:/Nintendo/save/", 18},
        {"sdmc:/emuMMC/RAW1/Nintendo/Contents/", 36},
        {"sdmc:/emuMMC/RAW1/Nintendo/save/", 31}
    };
    
    static constexpr FolderInfo restrictedWildcardFolders[] = {
        {"sdmc:/config/", 13},
        {"sdmc:/bootloader/", 18},
        {"sdmc:/atmosphere/", 18},
        {"sdmc:/switch/", 14}
    };
    
    static constexpr FolderInfo protectedFolders[] = {
        {"sdmc:/Nintendo/", 16},
        {"sdmc:/emuMMC/", 13},
        {"sdmc:/emuMMC/RAW1/", 18}
    };
    
    // 3) Check for directory traversal patterns (before other dangerous patterns)
    //if (patternPath.find("/../") != std::string::npos || 
    //    patternPath.find("../") == 0 ||
    //    patternPath.find("/..") == patternPath.length() - 3) {
    //    return true;
    //}
    
    // 4) Check other always dangerous patterns (cheap string searches)
    static const std::vector<const char*> alwaysDangerousPatterns = {
        "~",
        "null*",
        "*null"
    };
    
    for (const auto& pat : alwaysDangerousPatterns) {
        if (patternPath.find(pat) != std::string::npos) {
            return true;
        }
    }
    
    // 5) Root folder wildcard check (only if wildcards exist)
    if (hasWildcards) {
        static constexpr const char rootPrefix[] = "sdmc:/";
        static constexpr size_t rootLen = 6; // Length of "sdmc:/"
        
        if (patternPath.length() >= rootLen && 
            patternPath.compare(0, rootLen, rootPrefix) == 0) {
            size_t nextSlash = patternPath.find('/', rootLen);
            if (nextSlash == std::string::npos) nextSlash = patternPath.size();
            const size_t wildcardPos = patternPath.find('*', rootLen);
            if (wildcardPos != std::string::npos && wildcardPos < nextSlash) {
                return true; // wildcard in top-level directory disallowed
            }
        } else {
            // Disallow wildcards outside sdmc:/
            return true;
        }
    }
    
    // 6) Check ultra-protected folders (highest priority)
    for (const auto& folder : ultraProtectedFolders) {
        if (patternPath.length() >= folder.length &&
            patternPath.compare(0, folder.length, folder.path) == 0) {
            return true;
        }
    }
    
    // 7) Check restricted wildcard folders (only if wildcards exist)
    std::string relative;
    if (hasWildcards) {
        for (const auto& folder : restrictedWildcardFolders) {
            if (patternPath.length() >= folder.length &&
                patternPath.compare(0, folder.length, folder.path) == 0) {
                
                // OPTIMIZED: Create relative string only when processing restricted folders
                relative = patternPath.substr(folder.length);
                
                // If relative is empty or just '*', it means "the whole folder" or "all files"
                if (relative.empty() || relative == "*" || relative == "*/") {
                    return true; // block broad delete or targeting folder itself
                }
                
                // Otherwise allow wildcards deeper inside folder
                return false;
            }
        }
    }
    
    // 8) Check protected folders
    bool isAlbum;
    
    for (const auto& folder : protectedFolders) {
        if (patternPath.length() >= folder.length &&
            patternPath.compare(0, folder.length, folder.path) == 0) {
            
            // Check if this is an album folder (exception to protection)
            isAlbum = false;
            for (const auto& albumFolder : albumFolders) {
                if (patternPath.length() >= albumFolder.length &&
                    patternPath.compare(0, albumFolder.length, albumFolder.path) == 0) {
                    isAlbum = true;
                    break;
                }
            }
            
            if (isAlbum) {
                return false; // wildcards allowed in album folders
            }
            
            // Check for wildcards in protected folder (only if wildcards exist)
            if (hasWildcards) {
                // OPTIMIZED: Create relative string only when processing protected folders
                relative = patternPath.substr(folder.length);
                if (relative.find('*') != std::string::npos) {
                    return true; // wildcard in protected folder disallowed
                }
            }
            return false;
        }
    }
    
    // 9) Otherwise, no dangerous combination detected
    return false;
}


// Function to populate selectedItemsListOff from a JSON array based on a key
void populateSelectedItemsListFromJson(const std::string& sourceType, const std::string& jsonStringOrPath, const std::string& jsonKey, std::vector<std::string>& selectedItemsList) {
    selectedItemsList.clear();

    // Check for empty JSON source strings
    if (jsonStringOrPath.empty()) {
        return;
    }
    // Use a unique_ptr to manage JSON object with appropriate deleter
    std::unique_ptr<json_t, JsonDeleter> jsonData(nullptr, JsonDeleter());
    // Convert JSON string or read from file based on the source type
    if (sourceType == JSON_STR) {
        jsonData.reset(stringToJson(jsonStringOrPath));
    } else if (sourceType == JSON_FILE_STR) {
        jsonData.reset(readJsonFromFile(jsonStringOrPath));
    }
    // Early return if jsonData is null or not an array
    if (!jsonData) {
        return;
    }
    
    cJSON* jsonArray = reinterpret_cast<cJSON*>(jsonData.get());
    if (!cJSON_IsArray(jsonArray)) {
        return;
    }
    
    // Prepare for efficient insertion
    const int arraySize = cJSON_GetArraySize(jsonArray);
    //selectedItemsList.reserve(arraySize);
    
    // Store the key as a const char* to avoid repeated c_str() calls
    const char* jsonKeyCStr = jsonKey.c_str();
    
    // Iterate over the JSON array
    for (int i = 0; i < arraySize; ++i) {
        cJSON* item = cJSON_GetArrayItem(jsonArray, i);
        if (cJSON_IsObject(item)) {
            cJSON* keyValue = cJSON_GetObjectItemCaseSensitive(item, jsonKeyCStr);
            if (cJSON_IsString(keyValue) && keyValue->valuestring) {
                selectedItemsList.emplace_back(keyValue->valuestring);
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
inline void applyPlaceholderReplacement(std::string& input, const std::string& placeholder, const std::string& replacement) {
    const size_t pos = input.find(placeholder);
    if (pos == std::string::npos) {
        return;  // Returns original string directly if no placeholder is found
    }
    //std::string result = input;
    input.replace(pos, placeholder.length(), replacement);
    //return result;
}




void applyReplaceIniPlaceholder(std::string& arg, const std::string& commandName, const std::string& iniPath) {
    const std::string searchString = "{" + commandName + "(";
    
    // Early exit: Check if placeholder pattern exists before doing any INI work
    if (arg.find(searchString) == std::string::npos) {
        return; // No placeholders found, nothing to do
    }
    
    // Pre-declare variables outside the loop
    size_t startPos = 0;
    size_t endPos;
    size_t commaPos;
    size_t entryIndex;
    std::string placeholderContent;
    std::string replacement;
    std::string iniSection;
    std::string iniKey;
    
    // Cache section names to avoid re-parsing INI file multiple times
    std::vector<std::string> sectionNames;
    bool sectionsLoaded = false;
    
    // Build result incrementally to avoid expensive string replacement operations
    std::string result;
    size_t lastPos = 0;
    const size_t searchStringLen = searchString.length();
    
    // Process all occurrences of the placeholder
    while ((startPos = arg.find(searchString, lastPos)) != std::string::npos) {
        endPos = arg.find(")}", startPos);
        if (endPos == std::string::npos || endPos <= startPos) {
            // Invalid placeholder, append text up to this point and continue searching
            result.append(arg, lastPos, startPos + searchStringLen - lastPos);
            lastPos = startPos + searchStringLen;
            continue;
        }
        
        // Append text before placeholder
        result.append(arg, lastPos, startPos - lastPos);
        
        placeholderContent = arg.substr(startPos + searchStringLen, 
                                       endPos - startPos - searchStringLen);
        trim(placeholderContent);
        
        commaPos = placeholderContent.find(',');
        
        if (commaPos != std::string::npos) {
            // Handle section,key format
            iniSection = placeholderContent.substr(0, commaPos);
            trim(iniSection);
            removeQuotes(iniSection);
            
            iniKey = placeholderContent.substr(commaPos + 1);
            trim(iniKey);
            removeQuotes(iniKey);
            
            replacement = parseValueFromIniSection(iniPath, iniSection, iniKey);
        } else {
            // Check if the content is an integer
            if (std::all_of(placeholderContent.begin(), placeholderContent.end(), ::isdigit)) {
                entryIndex = ult::stoi(placeholderContent);
                
                // Load section names only once when needed
                if (!sectionsLoaded) {
                    sectionNames = parseSectionsFromIni(iniPath);
                    sectionsLoaded = true;
                }
                
                if (entryIndex < sectionNames.size()) {
                    replacement = sectionNames[entryIndex];
                } else {
                    replacement = NULL_STR;
                }
            } else {
                replacement = NULL_STR;
            }
        }
        
        // Append the replacement
        result.append(replacement);
        
        // Update lastPos to continue after this placeholder
        lastPos = endPos + 2;
    }
    
    // Append remaining text after last placeholder
    result.append(arg, lastPos);
    
    // Replace original string with result
    arg = std::move(result);
}


/**
 * @brief Replaces a JSON source placeholder with the actual JSON source.
 *
 * Optimized version with variables moved to usage scope to avoid repeated allocations.
 *
 * @param arg The input string containing the placeholder.
 * @param commandName The name of the JSON command (e.g., "json", "json_file").
 * @param jsonPathOrString The path to the JSON file or the JSON string itself.
 * @return std::string The input string with the placeholder replaced by the actual JSON source,
 *                   or the original input string if replacement failed or jsonDict is nullptr.
 */
/**
 * @brief Replaces a JSON source placeholder with the actual JSON source.
 *
 * Optimized version with variables moved outside loops to avoid repeated allocations.
 *
 * @param arg The input string containing the placeholder.
 * @param commandName The name of the JSON command (e.g., "json", "json_file").
 * @param jsonPathOrString The path to the JSON file or the JSON string itself.
 * @return std::string The input string with the placeholder replaced by the actual JSON source,
 *                   or the original input string if replacement failed or jsonDict is nullptr.
 */
std::string replaceJsonPlaceholder(const std::string& arg, const std::string& commandName, const std::string& jsonPathOrString) {
    // Early exit: Check if placeholder pattern exists before doing any JSON work
    const std::string searchString = "{" + commandName + "(";
    if (arg.find(searchString) == std::string::npos) {
        return arg; // No placeholders found, return original
    }
    
    // Load JSON data only if we have placeholders to process
    std::unique_ptr<json_t, JsonDeleter> jsonDict;
    if (commandName == "json" || commandName == "json_source") {
        jsonDict.reset(stringToJson(jsonPathOrString));
    } else if (commandName == "json_file" || commandName == "json_file_source") {
        jsonDict.reset(readJsonFromFile(jsonPathOrString));
    }
    if (!jsonDict) {
        return arg; // Return original string if JSON data couldn't be loaded
    }
    
    // Build result incrementally to avoid expensive string replacement operations
    std::string result;
    
    size_t lastPos = 0;
    size_t startPos = arg.find(searchString);
    
    const size_t searchStringLen = searchString.length();
    
    // Pre-declare variables outside loops to avoid repeated allocations
    size_t endPos, nextPos, commaPos, index;
    std::string key;
    bool validValue;
    
    while (startPos != std::string::npos) {
        endPos = arg.find(")}", startPos);
        if (endPos == std::string::npos) {
            break; // Break if no closing tag is found
        }
        
        // Append text before placeholder
        result.append(arg, lastPos, startPos - lastPos);
        
        nextPos = startPos + searchStringLen;
        cJSON* value = reinterpret_cast<cJSON*>(jsonDict.get()); // Get the JSON root object
        validValue = true;
        
        while (nextPos < endPos && validValue) {
            commaPos = arg.find(',', nextPos);
            if (commaPos == std::string::npos || commaPos > endPos) {
                commaPos = endPos; // Set to endPos if no comma is found or it's beyond endPos
            }
            
            // Reuse string capacity for key
            key.assign(arg, nextPos, commaPos - nextPos);
            
            if (cJSON_IsObject(value)) {
                value = cJSON_GetObjectItemCaseSensitive(value, key.c_str()); // Navigate through object
            } else if (cJSON_IsArray(value)) {
                index = std::stoul(key); // Convert key to index for arrays
                value = cJSON_GetArrayItem(value, index);
            } else {
                validValue = false; // Set validValue to false if value is neither object nor array
            }
            nextPos = commaPos + 1; // Move next position past the comma
        }
        
        if (validValue && value && cJSON_IsString(value) && value->valuestring) {
            result.append(value->valuestring); // Append replacement value
        } else {
            // If replacement failed, keep the original placeholder
            //result.append(arg, startPos, endPos + 2 - startPos);
            result.append(NULL_STR);
        }
        
        lastPos = endPos + 2;
        startPos = arg.find(searchString, lastPos); // Find next occurrence from last position
    }
    
    // Append remaining text after last placeholder
    result.append(arg, lastPos);
    
    return result; // Return the modified string
}

// Helper function to replace placeholders
void replaceAllPlaceholders(std::string& source, const std::string& placeholder, const std::string& replacement) {
    //std::string modifiedArg = source;
    std::string lastArg;
    while (source.find(placeholder) != std::string::npos) {
        //modifiedArg = replacePlaceholder(modifiedArg, placeholder, replacement);
        applyPlaceholderReplacement(source, placeholder, replacement);
        if (source == lastArg)
            break;
        lastArg = source;
    }
    return;
}



// Helper function to replace all placeholders in a single pass
bool replacePlaceholdersInArg(std::string& source, const std::unordered_map<std::string, std::string>& replacements) {
    bool replaced = false;
    size_t pos;
    for (const auto& [placeholder, replacement] : replacements) {
        pos = 0;
        while ((pos = source.find(placeholder, pos)) != std::string::npos) {
            source.replace(pos, placeholder.length(), replacement);
            pos += replacement.length();  // Move past the replacement to avoid infinite loop
            replaced = true;
        }
    }
    return replaced;
}


// Optimized getSourceReplacement function
std::vector<std::vector<std::string>> getSourceReplacement(const std::vector<std::vector<std::string>>& commands,
    const std::string& entry, size_t entryIndex, const std::string& packagePath = "") {

    bool inEristaSection = false;
    bool inMarikoSection = false;

    std::vector<std::vector<std::string>> modifiedCommands;
    std::string listString, listPath, jsonString, jsonPath, iniPath;
    bool usingFileSource = false;

    std::string fileName = getNameFromPath(entry);
    if (!isDirectory(entry)) {
        dropExtension(fileName);
    }

    std::vector<std::string> modifiedCmd;
    std::string commandName;
    std::string modifiedArg;
    size_t startPos, endPos;
    std::string replacement;

    std::string path;
    std::string raw;

    for (const auto& cmd : commands) {
        if (cmd.empty()) {
            continue;
        }

        modifiedCmd.clear();
        //modifiedCmd.shrink_to_fit();
        //modifiedCmd.reserve(cmd.size());
        commandName = cmd[0];

        if (commandName == "download") {
            isDownloadCommand.store(true, std::memory_order_release);
        }

        if (stringToLowercase(commandName) == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            continue;
        } else if (stringToLowercase(commandName) == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            continue;
        }

        if ((inEristaSection && usingErista) ||
            (inMarikoSection && usingMariko) ||
            (!inEristaSection && !inMarikoSection))
        {
            // Apply placeholder replacements if necessary
            for (const auto& arg : cmd) {
                modifiedArg = arg;

                if (commandName == "file_source") {
                    usingFileSource = true;
                }
                else if (commandName == "list_source" && listString.empty()) {
                    listString = cmd[1];
                    removeQuotes(listString);
                }
                else if (commandName == "list_file_source" && listPath.empty()) {
                    listPath = cmd[1];
                    preprocessPath(listPath, packagePath);
                }
                else if (commandName == "ini_file_source" && iniPath.empty()) {
                    iniPath = cmd[1];
                    preprocessPath(iniPath, packagePath);
                }
                else if (commandName == "json_source" && jsonString.empty()) {
                    jsonString = cmd[1];
                }
                else if (commandName == "json_file_source" && jsonPath.empty()) {
                    jsonPath = cmd[1];
                    preprocessPath(jsonPath, packagePath);
                }

                // These three always apply
                replaceAllPlaceholders(modifiedArg, "{file_source}", entry);
                replaceAllPlaceholders(modifiedArg, "{file_name}", fileName);
                path = getParentDirNameFromPath(entry);
                removeQuotes(path);
                replaceAllPlaceholders(modifiedArg, "{folder_name}", path);

                // {list_source(...)} block
                if (modifiedArg.find("{list_source(") != std::string::npos) {
                    applyPlaceholderReplacement(modifiedArg, "*", ult::to_string(entryIndex));
                    startPos = modifiedArg.find("{list_source(");
                    endPos   = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        // Get the raw value (may be empty)
                        raw = stringToList(listString)[entryIndex];
                        // Use returnOrNull to turn empty → NULL_STR
                        replacement = returnOrNull(raw);
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                // {list_file_source(...)} block
                if (modifiedArg.find("{list_file_source(") != std::string::npos) {
                    applyPlaceholderReplacement(modifiedArg, "*", ult::to_string(entryIndex));
                    startPos = modifiedArg.find("{list_file_source(");
                    endPos   = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        raw = getEntryFromListFile(listPath, entryIndex);
                        replacement = returnOrNull(raw);
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                // {ini_file_source(...)} block - FIXED
                if (modifiedArg.find("{ini_file_source(") != std::string::npos) {
                    applyPlaceholderReplacement(modifiedArg, "*", ult::to_string(entryIndex));
                    // applyReplaceIniPlaceholder modifies modifiedArg in place, so we just call it
                    applyReplaceIniPlaceholder(modifiedArg, "ini_file_source", iniPath);
                    // No additional replacement needed!
                }

                // {json_source(...)} block
                if (modifiedArg.find("{json_source(") != std::string::npos) {
                    applyPlaceholderReplacement(modifiedArg, "*", ult::to_string(entryIndex));
                    startPos = modifiedArg.find("{json_source(");
                    endPos   = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        raw = replaceJsonPlaceholder(
                            modifiedArg.substr(startPos, endPos - startPos + 2),
                            "json_source",
                            jsonString
                        );
                        replacement = returnOrNull(raw);
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                // {json_file_source(...)} block
                if (modifiedArg.find("{json_file_source(") != std::string::npos) {
                    applyPlaceholderReplacement(modifiedArg, "*", ult::to_string(entryIndex));
                    startPos = modifiedArg.find("{json_file_source(");
                    endPos   = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        raw = replaceJsonPlaceholder(
                            modifiedArg.substr(startPos, endPos - startPos + 2),
                            "json_file_source",
                            jsonPath
                        );
                        replacement = returnOrNull(raw);
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                modifiedCmd.push_back(std::move(modifiedArg));
            }

            modifiedCommands.emplace_back(std::move(modifiedCmd));
        }
    }

    if (usingFileSource) {
        modifiedCommands.insert(modifiedCommands.begin(), { "file_name", fileName });
        modifiedCommands.insert(modifiedCommands.begin(), { "folder_name", path });
        modifiedCommands.insert(modifiedCommands.begin(), { "sourced_path", entry });
    }

    return modifiedCommands;
}


std::string getCurrentTimestamp(const std::string& format) {
    // Try using standard POSIX time() function
    time_t seconds = time(nullptr);
    const u32 milliseconds = (armTicksToNs(armGetSystemTick()) / 1000000ULL) % 1000; // We lose millisecond precision with this method
    
    // If you need milliseconds, try to get them from system tick
    //u64 tick_ns = armTicksToNs(armGetSystemTick());
    //milliseconds = (armTicksToNs(armGetSystemTick()) / 1000000ULL) % 1000;
    
    std::string modifiedFormat = format;
    bool hasMilliseconds = false;
    
    const size_t pos = modifiedFormat.find("%f");
    if (pos != std::string::npos) {
        modifiedFormat.erase(pos, 2);
        hasMilliseconds = true;
    }
    
    char buffer[30];
    if (!std::strftime(buffer, sizeof(buffer), modifiedFormat.c_str(), std::localtime(&seconds))) {
        return "";
    }
    
    std::string formattedTime(buffer);
    StringStream oss;
    
    if (formattedTime.find("%s") != std::string::npos) {
        oss << static_cast<long long>(seconds); 
        formattedTime.replace(formattedTime.find("%s"), 2, oss.str());
        oss.clear();
    }
    
    if (hasMilliseconds) {
        std::string millisecondsStr = ult::to_string(milliseconds);
        if (millisecondsStr.length() < 3) {
            millisecondsStr.insert(0, 3 - millisecondsStr.length(), '0');
        }
        if (pos > 0 && format[pos - 1] == '.') {
            formattedTime += millisecondsStr;
        } else {
            formattedTime += "." + millisecondsStr;
        }
    }
    
    return formattedTime;
}





// Helper function to skip spaces
void skipSpaces(const std::string& expression, size_t& pos) {
    while (pos < expression.length() && std::isspace(expression[pos])) {
        ++pos;
    }
}

// Helper function to parse a number or a nested expression in parentheses
double parseExpression(const std::string& expression, size_t& pos, bool& valid);

double parseNumber(const std::string& expression, size_t& pos, bool& valid) {
    skipSpaces(expression, pos);

    // Check if the expression starts with a '(' indicating a nested expression
    if (expression[pos] == '(') {
        ++pos;  // Skip the '('
        const double result = parseExpression(expression, pos, valid);
        if (expression[pos] == ')') {
            ++pos;  // Skip the ')'
        } else {
            valid = false;  // Unmatched parentheses
        }
        return result;
    }

    // Parse a number
    double result = 0.0f;
    bool hasDecimal = false;
    double decimalPlace = 0.1f;
    bool isNegative = false;

    if (expression[pos] == '-') {
        isNegative = true;
        ++pos;
    }

    while (pos < expression.length() && (std::isdigit(expression[pos]) || expression[pos] == '.')) {
        if (expression[pos] == '.') {
            hasDecimal = true;
            ++pos;
            continue;
        }

        if (hasDecimal) {
            result += (expression[pos] - '0') * decimalPlace;
            decimalPlace *= 0.1f;
        } else {
            result = result * 10.0f + (expression[pos] - '0');
        }
        ++pos;
    }

    if (isNegative) {
        result = -result;
    }

    valid = true;
    return result;
}

// Function to evaluate an expression, which may include parentheses
double parseExpression(const std::string& expression, size_t& pos, bool& valid) {
    skipSpaces(expression, pos);

    double result = parseNumber(expression, pos, valid);
    if (!valid) return 0;

    char op;
    double operand;
    while (pos < expression.length()) {
        skipSpaces(expression, pos);
        if (pos >= expression.length()) break;

        op = expression[pos++];
        skipSpaces(expression, pos);

        operand = parseNumber(expression, pos, valid);
        if (!valid) return 0;

        switch (op) {
            case '+':
                result += operand;
                break;
            case '-':
                result -= operand;
                break;
            case '*':
                result *= operand;
                break;
            case '/':
                if (operand == 0) {
                    valid = false;  // Division by zero
                    return 0;
                }
                result /= operand;
                break;
            case '%':
                if (STBTT_fmod(result, 1.0f) != 0.0f || STBTT_fmod(operand, 1.0f) != 0.0f) {
                    valid = false;  // Modulus only valid for integers
                    return 0;
                }
                result = static_cast<int>(result) % static_cast<int>(operand);
                break;
            default:
                valid = false;
                return 0;
        }
    }

    valid = true;
    return result;
}

// Function to evaluate a complete expression (this will call parseExpression)
double evaluateExpression(const std::string& expression, bool& valid) {
    size_t pos = 0;
    return parseExpression(expression, pos, valid);
}

// Handle Math Placeholder with Parentheses, Modulus, and Optional Integer Support
std::string handleMath(const std::string& placeholder) {
    const size_t startPos = placeholder.find('(') + 1;
    const size_t endPos = placeholder.find(')');

    if (startPos == std::string::npos || endPos == std::string::npos || startPos >= endPos) {
        return NULL_STR;
    }

    std::string mathExpression = placeholder.substr(startPos, endPos - startPos);
    removeQuotes(mathExpression);

    const size_t commaPos = mathExpression.find(',');
    bool forceInteger = false;

    if (commaPos != std::string::npos) {
        std::string secondParam = mathExpression.substr(commaPos + 1);
        trim(secondParam);
        forceInteger = (secondParam == TRUE_STR);
        mathExpression = mathExpression.substr(0, commaPos);
    }

    size_t pos;
    // Add spaces around operators to ensure proper parsing
    for (const char op : {'+', '-', '*', '/'}) {
        pos = 0;
        while ((pos = mathExpression.find(op, pos)) != std::string::npos) {
            if (pos > 0 && mathExpression[pos - 1] != ' ') {
                mathExpression.insert(pos, " ");
                pos++;
            }
            if (pos + 1 < mathExpression.size() && mathExpression[pos + 1] != ' ') {
                mathExpression.insert(pos + 1, " ");
            }
            pos += 2; // Move past the operator and space
        }
    }

    bool valid = false;
    const double result = evaluateExpression(mathExpression, valid);

    if (!valid) {
        return NULL_STR;
    }

    // Format the result with StringStream
    StringStream oss;
    if (forceInteger || STBTT_fmod(result, 1.0) == 0.0) {
        oss << static_cast<int>(result);  // Integer output if required
    } else {
        // Manually format to two decimal places for double output
        const int intPart = static_cast<int>(result);
        const int decimalPart = static_cast<int>((result - intPart) * 100);  // Get two decimal places

        oss << intPart << ".";

        // Handle cases where decimal part has only one digit (e.g., 3.1 should be 3.10)
        if (decimalPart < 10) {
            oss << "0";
        }
        oss << decimalPart;
    }

    return oss.str();
}


std::string handleLength(const std::string& placeholder) {
    const size_t startPos = placeholder.find('(') + 1;
    const size_t endPos = placeholder.find(')');
    
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        std::string str = placeholder.substr(startPos, endPos - startPos);
        trim(str);  // Remove any extra spaces
        removeQuotes(str);  // If your strings are wrapped in quotes, remove them
        
        // Return the length of the string as a string
        return ult::to_string(str.length());
    }
    
    return placeholder;  // If invalid, return the original placeholder
}



// Define the replacePlaceholders function outside of applyPlaceholderReplacements
//auto replacePlaceholders = [](std::string& arg, const std::string& placeholder, const std::function<std::string(const std::string&)>& replacer) {
//    size_t startPos, endPos;
//    std::string lastArg, replacement;
//
//    size_t nestedStartPos, nextStartPos, nextEndPos;
//
//    while ((startPos = arg.find(placeholder)) != std::string::npos) {
//        nestedStartPos = startPos;
//        while (true) {
//            nextStartPos = arg.find(placeholder, nestedStartPos + 1);
//            nextEndPos = arg.find(")}", nestedStartPos);
//            if (nextStartPos != std::string::npos && nextStartPos < nextEndPos) {
//                nestedStartPos = nextStartPos;
//            } else {
//                endPos = nextEndPos;
//                break;
//            }
//        }
//
//        if (endPos == std::string::npos || endPos <= startPos) break;
//
//        replacement = replacer(arg.substr(startPos, endPos - startPos + 2));
//        if (replacement.empty()) {
//            replacement = NULL_STR;
//        }
//        arg.replace(startPos, endPos - startPos + 2, replacement);
//        if (arg == lastArg) {
//            if (interpreterLogging) {
//                disableLogging = false;
//                logMessage("failed replacement arg: " + arg);
//            }
//            replacement = NULL_STR;
//            arg.replace(startPos, endPos - startPos + 2, replacement);
//            break;
//        }
//        lastArg = arg;
//    }
//};

// Finds the next occurrence of ANY function-style placeholder opener from `starts`,
// beginning at `from`. On tie (same index), it prefers the LONGEST token.
// Returns npos if none found, and sets matchedLen accordingly.
static size_t findNextOpenToken(const std::string& s,
                                size_t from,
                                const std::vector<std::string>& starts,
                                size_t& matchedLen)
{
    size_t bestPos = std::string::npos;
    matchedLen = 0;

    for (const auto& tok : starts) {
        size_t pos = s.find(tok, from);
        if (pos == std::string::npos) continue;

        if (bestPos == std::string::npos || pos < bestPos ||
            (pos == bestPos && tok.size() > matchedLen)) {
            bestPos = pos;
            matchedLen = tok.size();
        }
    }
    return bestPos;
}

// Starting at `startPos` which points to the FIRST character of the opener of the
// CURRENT placeholder (e.g., the '{' in "{slice("), find the matching closing
// pair for that placeholder. We count nested openers of ANY placeholder type
// and match on ")}" closes.
//
// Return value: index of the ')' in the matching closing ")}" (so you can substr
// with +2 to include ")}"). Returns npos if unmatched.
static size_t findMatchingClose(const std::string& s,
                                size_t startPos,
                                const std::vector<std::string>& starts,
                                size_t outerOpenLen)
{
    // We have already seen one opener at startPos
    int depth = 1;
    size_t scan = startPos + outerOpenLen;

    size_t openLen;
    bool openerFirst;

    while (scan < s.size()) {
        // Find next opener (any) and next closer ")}"
        const size_t nextCloser = s.find(")}", scan);

        openLen = 0;
        const size_t nextOpener = findNextOpenToken(s, scan, starts, openLen);

        // Decide which token comes first
        if (nextCloser == std::string::npos && nextOpener == std::string::npos) {
            return std::string::npos; // no more tokens
        }

        openerFirst = false;
        if (nextOpener != std::string::npos) {
            if (nextCloser == std::string::npos) openerFirst = true;
            else openerFirst = (nextOpener < nextCloser);
        }

        if (openerFirst) {
            // Found a nested opener
            ++depth;
            scan = nextOpener + openLen; // continue after this opener
        } else {
            // Found a closer
            --depth;
            if (depth == 0) {
                return nextCloser; // return index of ')'
            }
            scan = nextCloser + 2; // continue after ")}"
        }
    }

    return std::string::npos; // unmatched
}

bool replacePlaceholdersRecursively(
    std::string& arg,
    const std::vector<std::pair<std::string, std::function<std::string(const std::string&)>>>& placeholders)
{
    bool anyReplacementsMade = false;

    // Precompute all opener tokens ("{slice(", "{math(", ...).
    std::vector<std::string> starts;
    starts.reserve(placeholders.size());
    for (const auto& pr : placeholders) {
        starts.push_back(pr.first);
    }

    bool replacedThisPass;
    size_t searchPos;
    std::string replacement;
    std::string inner;

    // Keep sweeping until no replacements occur.
    for (;;) {
        replacedThisPass = false;

        // Try each placeholder type.
        for (size_t t = 0; t < placeholders.size(); ++t) {
            const auto& opener = placeholders[t].first;      // e.g., "{slice("
            const auto& replacer = placeholders[t].second;

            searchPos = 0;

            while (true) {
                // Find the next occurrence of THIS opener
                const size_t startPos = arg.find(opener, searchPos);
                if (startPos == std::string::npos) break;

                // Find its matching ")}" by counting nested ANY opener
                const size_t closePos = findMatchingClose(arg, startPos, starts, opener.size());
                if (closePos == std::string::npos) {
                    // Unbalanced; skip this and move on to avoid infinite loop
                    searchPos = startPos + opener.size();
                    continue;
                }

                // Full placeholder text including wrapper: "{name(...)}"
                const size_t fullLen = (closePos - startPos) + 2; // include the ")}"
                const std::string placeholderText = arg.substr(startPos, fullLen);

                // Resolve INNER content first (exclude the outer wrapper)
                const size_t innerStart = opener.size();
                const size_t innerLen   = placeholderText.size() - innerStart - 2; // minus ")}"
                inner = placeholderText.substr(innerStart, innerLen);

                // Recurse on inner only (so we don't re-match the outer opener)
                replacePlaceholdersRecursively(inner, placeholders);

                // Rebuild the outer placeholder with resolved args
                const std::string resolvedPlaceholder = opener + inner + ")}";

                // Call the replacer on the resolved placeholder
                replacement = replacer(resolvedPlaceholder);
                if (replacement.empty()) {
                    replacement = NULL_STR; // preserve your NULL_STR convention
                }

                // Perform replacement if it changes anything
                const std::string before = arg;
                arg.replace(startPos, fullLen, replacement);

                if (arg != before) {
                    anyReplacementsMade = true;
                    replacedThisPass = true;
                    // Continue scanning after the replacement
                    searchPos = startPos + replacement.size();
                } else {
                    // If nothing changed, advance to avoid re-hitting the same spot
                    searchPos = startPos + opener.size();
                }
            }
        }

        if (!replacedThisPass) break;
    }

    return anyReplacementsMade;
}



std::unordered_map<std::string, std::string> generalPlaceholders;
void updateGeneralPlaceholders() {
    generalPlaceholders = {
        {"{ram_vendor}", memoryVendor},
        {"{ram_model}", memoryModel},
        {"{ams_version}", amsVersion},
        {"{hos_version}", hosVersion},
        {"{cpu_speedo}", ult::to_string(cpuSpeedo0)},
        {"{cpu_iddq}", ult::to_string(cpuIDDQ)},
        {"{gpu_speedo}", ult::to_string(cpuSpeedo2)},
        {"{gpu_iddq}", ult::to_string(gpuIDDQ)},
        {"{soc_speedo}", ult::to_string(socSpeedo0)},
        {"{soc_iddq}", ult::to_string(socIDDQ)},
        {"{title_id}", getTitleIdAsString()},
        {"{local_ip}", getLocalIpAddress()}
    };
}

bool applyPlaceholderReplacements(std::vector<std::string>& cmd, const std::string& hexPath, const std::string& iniPath, const std::string& listString, const std::string& listPath, const std::string& jsonString, const std::string& jsonPath) {
    bool replacementsMade = false;
    
    std::vector<std::pair<std::string, std::function<std::string(const std::string&)>>> placeholders = {
        {"{hex_file(", [&](const std::string& placeholder) { return returnOrNull(replaceHexPlaceholder(placeholder, hexPath)); }},
        {"{ini_file(", [&](const std::string& placeholder) { 
            std::string result = placeholder;
            applyReplaceIniPlaceholder(result, INI_FILE_STR, iniPath); 
            return returnOrNull(result); 
        }},
        {"{list(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find('(') + 1;
            //size_t endPos = placeholder.find(')');
            //size_t listIndex = ult::stoi(placeholder.substr(startPos, placeholder.find(')') - startPos));
            return returnOrNull(stringToList(listString)[ult::stoi(placeholder.substr(startPos, placeholder.find(')') - startPos))]);
        }},
        {"{list_file(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find('(') + 1;
            //size_t endPos = placeholder.find(')');
            //size_t listIndex = ult::stoi(placeholder.substr(startPos, placeholder.find(')') - startPos));
            return returnOrNull(getEntryFromListFile(listPath, ult::stoi(placeholder.substr(startPos, placeholder.find(')') - startPos))));
        }},
        {"{json(", [&](const std::string& placeholder) { return replaceJsonPlaceholder(placeholder, JSON_STR, jsonString); }},
        {"{json_file(", [&](const std::string& placeholder) { return replaceJsonPlaceholder(placeholder, JSON_FILE_STR, jsonPath); }},
        {"{timestamp(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find("(") + 1;
            const size_t endPos = placeholder.find(")");
            std::string format = (endPos != std::string::npos) ? placeholder.substr(startPos, endPos - startPos) : "%Y-%m-%d %H:%M:%S";
            removeQuotes(format);
            return returnOrNull(getCurrentTimestamp(format));
        }},
        {"{decimal_to_hex(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find("(") + 1;
            //size_t endPos = placeholder.find(")");
            const std::string params = placeholder.substr(startPos, placeholder.find(")") - startPos);
        
            const size_t commaPos = params.find(",");
            std::string decimalValue;
            std::string order;
        
            if (commaPos != std::string::npos) {
                decimalValue = params.substr(0, commaPos);
                order = params.substr(commaPos + 1);
                order.erase(0, order.find_first_not_of(" \t\n\r"));
                order.erase(order.find_last_not_of(" \t\n\r") + 1);
            } else {
                decimalValue = params;
                order = "";
            }
        
            if (order.empty()) {
                return returnOrNull(decimalToHex(decimalValue));
            } else {
                return returnOrNull(decimalToHex(decimalValue, ult::stoi(order)));
            }
        }},
        {"{ascii_to_hex(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find("(") + 1;
            //size_t endPos = placeholder.find(")");
            //std::string asciiValue = placeholder.substr(startPos, placeholder.find(")") - startPos);
            return returnOrNull(asciiToHex(placeholder.substr(startPos, placeholder.find(")") - startPos)));
        }},
        {"{hex_to_rhex(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find("(") + 1;
            //size_t endPos = placeholder.find(")");
            //std::string hexValue = placeholder.substr(startPos, placeholder.find(")") - startPos);
            return returnOrNull(hexToReversedHex(placeholder.substr(startPos, placeholder.find(")") - startPos)));
        }},
        {"{hex_to_decimal(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find("(") + 1;
            //size_t endPos = placeholder.find(")");
            //std::string hexValue = placeholder.substr(startPos, placeholder.find(")") - startPos);
            return returnOrNull(hexToDecimal(placeholder.substr(startPos, placeholder.find(")") - startPos)));
        }},
        {"{random(", [&](const std::string& placeholder) {
            std::srand(std::time(0));
            
            const size_t startPos = placeholder.find('(') + 1;
            const size_t endPos = placeholder.find(')');
            const std::string parameters = placeholder.substr(startPos, endPos - startPos);
            const size_t commaPos = parameters.find(',');
            
            if (commaPos != std::string::npos) {
                const int lowValue = ult::stoi(parameters.substr(0, commaPos));
                //int highValue = ult::stoi(parameters.substr(commaPos + 1));
                //int randomValue = lowValue + rand() % (ult::stoi(parameters.substr(commaPos + 1)) - lowValue + 1);
                return returnOrNull(ult::to_string(lowValue + rand() % (ult::stoi(parameters.substr(commaPos + 1)) - lowValue + 1)));
            }
            return returnOrNull(placeholder);
        }},
        {"{slice(", [&](const std::string& placeholder) {
            const size_t startPos = placeholder.find('(');
            const size_t endPos = placeholder.rfind(')');
            if (startPos == std::string::npos || endPos == std::string::npos || endPos <= startPos + 1) {
                return returnOrNull(placeholder);
            }
        
            const std::string parameters = placeholder.substr(startPos + 1, endPos - startPos - 1);
            const size_t firstComma = parameters.find(',');
            const size_t secondComma = (firstComma == std::string::npos) ? std::string::npos : parameters.find(',', firstComma + 1);
            if (firstComma == std::string::npos || secondComma == std::string::npos) {
                return returnOrNull(placeholder);
            }
        
            std::string strPart    = parameters.substr(0, firstComma);
            std::string startIndex = parameters.substr(firstComma + 1, secondComma - firstComma - 1);
            std::string endIndex   = parameters.substr(secondComma + 1);
        
            trim(strPart);
            removeQuotes(strPart);
            trim(startIndex);
            removeQuotes(startIndex);
            trim(endIndex);
            removeQuotes(endIndex);
        
            if (startIndex.empty() || endIndex.empty() ||
                !std::all_of(startIndex.begin(), startIndex.end(), ::isdigit) ||
                !std::all_of(endIndex.begin(), endIndex.end(), ::isdigit)) {
                return returnOrNull(placeholder);
            }
        
            const size_t sliceStart = static_cast<size_t>(ult::stoi(startIndex));
            const size_t sliceEnd   = static_cast<size_t>(ult::stoi(endIndex));
        
            if (sliceEnd <= sliceStart || sliceStart >= strPart.length()) {
                return returnOrNull(placeholder);
            }
        
            //std::string result = sliceString(strPart, sliceStart, sliceEnd);
            return returnOrNull(sliceString(strPart, sliceStart, sliceEnd));
        }},
        {"{split(", [&](const std::string& placeholder) {
            const size_t openParen = placeholder.find('(');
            const size_t closeParen = placeholder.find(')');
        
            if (openParen == std::string::npos || closeParen == std::string::npos || closeParen <= openParen) {
                return NULL_STR;
            }
        
            const std::string parameters = placeholder.substr(openParen + 1, closeParen - openParen - 1);
        
            const size_t firstCommaPos = parameters.find(',');
            const size_t lastCommaPos  = parameters.find_last_of(',');
        
            if (firstCommaPos == std::string::npos
             || lastCommaPos  == std::string::npos
             || firstCommaPos == lastCommaPos) {
                return NULL_STR;
            }
        
            std::string str = parameters.substr(0, firstCommaPos);
            std::string delimiter = parameters.substr(firstCommaPos + 1, lastCommaPos - firstCommaPos - 1);
            std::string indexStr = parameters.substr(lastCommaPos + 1);
        
            trim(str);
            removeQuotes(str);
            trim(delimiter);
            removeQuotes(delimiter);
            trim(indexStr);
        
            if (indexStr.empty() || !std::all_of(indexStr.begin(), indexStr.end(), ::isdigit)) {
                return NULL_STR;
            }
        
            //size_t index = ult::stoi(indexStr);
            std::string result = splitStringAtIndex(str, delimiter, ult::stoi(indexStr));
        
            return result.empty() ? NULL_STR : result;
        }},
        {"{math(", [&](const std::string& placeholder) { return returnOrNull(handleMath(placeholder)); }},
        {"{length(", [&](const std::string& placeholder) { return returnOrNull(handleLength(placeholder)); }},
    };

    //generalPlaceholders = {
    //    {"{ram_vendor}", memoryVendor},
    //    {"{ram_model}", memoryModel},
    //    {"{ams_version}", amsVersion},
    //    {"{hos_version}", hosVersion},
    //    {"{cpu_speedo}", ult::to_string(cpuSpeedo0)},
    //    {"{cpu_iddq}", ult::to_string(cpuIDDQ)},
    //    {"{gpu_speedo}", ult::to_string(cpuSpeedo2)},
    //    {"{gpu_iddq}", ult::to_string(gpuIDDQ)},
    //    {"{soc_speedo}", ult::to_string(socSpeedo0)},
    //    {"{soc_iddq}", ult::to_string(socIDDQ)},
    //    {"{title_id}", getTitleIdAsString()}
    //};
    updateGeneralPlaceholders();

    // Iterate through each command and replace placeholders
    for (auto& arg : cmd) {
        //std::string originalArg = arg; // Store original to compare later
        
        // Replace general placeholders - modify these functions to return bool
        if (replacePlaceholdersInArg(arg, generalPlaceholders)) {
            replacementsMade = true;
        }

        // Replace button/arrow placeholders from the global map
        if (replacePlaceholdersInArg(arg, symbolPlaceholders)) {
            replacementsMade = true;
        }

        // Resolve nested placeholders - modify this function to return bool
        if (replacePlaceholdersRecursively(arg, placeholders)) {
            replacementsMade = true;
        }
        
        // Alternative: Simple string comparison if you can't modify the helper functions
        // if (arg != originalArg) {
        //     replacementsMade = true;
        // }
    }
    
    return replacementsMade;
}



// forward declarartion
void processCommand(const std::vector<std::string>& cmd, const std::string& packagePath, const std::string& selectedCommand);


/**
 * @brief Apply placeholder replacements to a list of commands and handle control flow commands.
 *
 * This function iterates over a list of commands, where each command is a vector of strings,
 * and applies the `applyPlaceholderReplacements` function to handle the placeholders.
 * It also handles control flow commands like "erista:" and "mariko:".
 * Optimized for minimal memory usage with in-place compaction.
 *
 * @param commands A list of commands, where each command is represented as a vector of strings.
 * @param packagePath The path to the package (optional).
 * @param selectedCommand A specific command to execute (optional).
 * @return Returns true if the placeholders were successfully applied to the commands, false otherwise.
 */
bool applyPlaceholderReplacementsToCommands(std::vector<std::vector<std::string>>& commands, 
                                           const std::string& packagePath = "", 
                                           const std::string& selectedCommand = "") {
    
    std::string listString, listPath, jsonString, jsonPath, hexPath, iniPath;
    bool inEristaSection = false;
    bool inMarikoSection = false;
    
    // Two-pointer technique: readIndex reads all commands, writeIndex writes kept commands
    size_t writeIndex = 0;
    
    for (size_t readIndex = 0; readIndex < commands.size(); ++readIndex) {
        auto& cmd = commands[readIndex];
        
        if (cmd.empty()) {
            // Skip empty commands, don't increment writeIndex
            cmd.shrink_to_fit();
            continue;
        }

        const std::string_view commandName = cmd[0];

        // Handle control flow commands
        if (commandName == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue; // Don't keep this command
        } else if (commandName == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue; // Don't keep this command
        } else if ((!commandName.empty() && commandName.front() == ';') ||
                   (commandName.size() >= 7 && commandName.substr(commandName.size() - 7) == "_source") ||
                   commandName == "logging") {
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue; // Don't keep this command
        }

        // Skip commands not relevant to current hardware
        if ((inEristaSection && !usingErista) || (inMarikoSection && !usingMariko)) {
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue; // Don't keep this command
        }

        // Apply placeholder replacements
        applyPlaceholderReplacements(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

        // Handle special commands
        const size_t cmdSize = cmd.size();
        bool shouldKeep = true;

        if (commandName == LIST_STR && cmdSize >= 2) {
            listString = cmd[1];
            removeQuotes(listString);
            shouldKeep = false;
        } else if (commandName == LIST_FILE_STR && cmdSize >= 2) {
            listPath = cmd[1];
            preprocessPath(listPath, packagePath);
            shouldKeep = false;
        } else if (commandName == JSON_STR && cmdSize >= 2) {
            jsonString = cmd[1];
            shouldKeep = false;
        } else if (commandName == JSON_FILE_STR && cmdSize >= 2) {
            jsonPath = cmd[1];
            preprocessPath(jsonPath, packagePath);
            shouldKeep = false;
        } else if (commandName == INI_FILE_STR && cmdSize >= 2) {
            iniPath = cmd[1];
            preprocessPath(iniPath, packagePath);
            shouldKeep = false;
        } else if (commandName == HEX_FILE_STR && cmdSize >= 2) {
            hexPath = cmd[1];
            preprocessPath(hexPath, packagePath);
            shouldKeep = false;
        } else if (commandName == "filter" || commandName == "file_name" || 
                   commandName == "folder_name" || commandName == "sourced_path") {
            shouldKeep = false;
        }

        if (!shouldKeep) {
            //cmd.clear();
            //cmd.shrink_to_fit();
            cmd = {};
            continue; // Don't keep this command
        }

        // Apply quoting logic to arguments for commands we're keeping
        for (size_t i = 1; i < cmd.size(); ++i) {
            std::string& argument = cmd[i];

            if (argument.empty()) {
                argument = "''";
                continue;
            }

            if (argument.size() >= 2 &&
                ((argument.front() == '"' && argument.back() == '"') ||
                 (argument.front() == '\'' && argument.back() == '\''))) {
                continue;
            }

            if (argument.find(' ') == std::string::npos) {
                continue;
            }

            // Optimized quoting - fewer string operations
            const bool hasSingleQuote = argument.find('\'') != std::string::npos;
            const bool hasDoubleQuote = argument.find('"') != std::string::npos;
            
            if (hasSingleQuote && !hasDoubleQuote) {
                argument.insert(0, 1, '"');
                argument.push_back('"');
            } else {
                argument.insert(0, 1, '\'');
                argument.push_back('\'');
            }
        }

        // Keep this command - move it to the write position
        if (writeIndex != readIndex) {
            commands[writeIndex] = std::move(cmd);
            cmd.shrink_to_fit();
        }
        ++writeIndex;
    }
    
    // Resize to keep only the commands we want
    commands.resize(writeIndex);
    commands.shrink_to_fit();  // Always - maximize memory efficiency

    // Shrink capacity if we removed a lot of commands
    //if (commands.capacity() > commands.size() * 2) {
    //    commands.shrink_to_fit();
    //}

    return true;
}



/**
 * @brief Interpret and execute a list of commands.
 *
 * This function interprets and executes a list of commands based on their names and arguments.
 * Optimized for minimal memory usage by clearing processed commands immediately.
 *
 * @param commands A list of commands, where each command is represented as a vector of strings.
 */
bool interpretAndExecuteCommands(std::vector<std::vector<std::string>>&& commands, 
                                const std::string& packagePath = "", 
                                const std::string& selectedCommand = "") {
    
    #if USING_LOGGING_DIRECTIVE
    if (!packagePath.empty()) {
        disableLogging = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_LOGGING_STR) == TRUE_STR);
        logFilePath = packagePath + "log.txt";
    }
    #endif

    // Initialize buffer sizes based on expanded memory setting
    //if (expandedMemory) {
    //    COPY_BUFFER_SIZE = 262144;
    //    HEX_BUFFER_SIZE = 8192;
    //    UNZIP_READ_BUFFER = 262144;
    //    UNZIP_WRITE_BUFFER = 131072;
    //    DOWNLOAD_READ_BUFFER = 262144;
    //    DOWNLOAD_WRITE_BUFFER = 131072;
    //}

    // Load and apply buffer configuration from INI file
    const auto bufferSection = getKeyValuePairsFromSection(ULTRAHAND_CONFIG_INI_PATH, MEMORY_STR);
    
    if (!bufferSection.empty()) {
        struct BufferConfig {
            const char* key;
            size_t* target;
        };
        
        const BufferConfig configs[] = {
            {"copy_buffer_size", &COPY_BUFFER_SIZE},
            {"unzip_read_buffer", &UNZIP_READ_BUFFER},
            {"unzip_write_buffer", &UNZIP_WRITE_BUFFER},
            {"download_read_buffer", &DOWNLOAD_READ_BUFFER},
            {"download_write_buffer", &DOWNLOAD_WRITE_BUFFER},
            {"hex_buffer_size", &HEX_BUFFER_SIZE}
        };
        
        for (const auto& config : configs) {
            const auto it = bufferSection.find(config.key);
            if (it != bufferSection.end()) {
                *config.target = ult::stoi(it->second);
            }
        }
    }

    // Initialize state variables
    bool inEristaSection = false;
    bool inMarikoSection = false;
    bool inTrySection = false;
    
    // String buffers for command processing
    std::string listString, listPath, jsonString, jsonPath, hexPath, iniPath;
    
    #if USING_LOGGING_DIRECTIVE
    std::string messageBuffer;
    #endif

    // Reset global state
    commandSuccess.store(true, std::memory_order_release);
    refreshPage.store(false, std::memory_order_release);
    refreshPackage.store(false, std::memory_order_release);
    interpreterLogging.store(false, std::memory_order_release);

    // Process commands one by one, clearing each after processing
    for (size_t i = 0; i < commands.size(); ++i) {
        // Check for abort signal
        if (abortCommand.load(std::memory_order_acquire)) {
            abortCommand.store(false, std::memory_order_release);
            commandSuccess.store(false, std::memory_order_release);
            // Clear all remaining commands
            //for (size_t j = i; j < commands.size(); ++j) {
            //    commands[j].clear();
            //    commands[j].shrink_to_fit();
            //}
            commands = {};
            //commands.clear();
            //commands.shrink_to_fit();
            #if USING_LOGGING_DIRECTIVE
            disableLogging = true;
            logFilePath = defaultLogFilePath;
            #endif
            return commandSuccess;
        }

        auto& cmd = commands[i];
        
        // Skip empty commands
        if (cmd.empty()) {
            continue;
        }

        // Use string_view to avoid copying command name
        const std::string_view commandName = cmd[0];

        // Handle control flow commands
        if (commandName == "try:") {
            if (inTrySection && commandSuccess.load(std::memory_order_acquire)) {
                // Clear remaining commands and exit
                //for (size_t j = i; j < commands.size(); ++j) {
                //    commands[j].clear();
                //    commands[j].shrink_to_fit();
                //}
                //commands.clear();
                //commands.shrink_to_fit();
                commands = {};
                #if USING_LOGGING_DIRECTIVE
                disableLogging = true;
                logFilePath = defaultLogFilePath;
                #endif
                return true;
            }
            commandSuccess.store(true, std::memory_order_release);
            inTrySection = true;
            // Clear and continue
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue;
        }
        
        if (commandName == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            // Clear and continue
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue;
        }
        
        if (commandName == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            // Clear and continue
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue;
        }

        // Skip commands in try section if previous command failed
        if (!commandSuccess.load(std::memory_order_acquire) && inTrySection) {
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue;
        }

        // Determine if command should execute based on platform sections
        const bool shouldExecute = 
            (inEristaSection && !inMarikoSection && usingErista) ||
            (!inEristaSection && inMarikoSection && usingMariko) ||
            (!inEristaSection && !inMarikoSection);

        if (!shouldExecute) {
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue;
        }

        // Only execute if not in try section or if we're succeeding in try section
        if (inTrySection && !commandSuccess.load(std::memory_order_acquire)) {
            cmd = {};
            //cmd.clear();
            //cmd.shrink_to_fit();
            continue;
        }

        // Apply placeholder replacements only if needed
        bool hasPlaceholders = false;
        for (const auto& arg : cmd) {
            if (arg.find('{') != std::string::npos) {
                hasPlaceholders = true;
                break;
            }
        }
        
        if (hasPlaceholders) {
            applyPlaceholderReplacements(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);
        }

        #if USING_LOGGING_DIRECTIVE
        if (interpreterLogging.load(std::memory_order_acquire)) {
            disableLogging = false;
        }

        if (!disableLogging) {
            // Build log message efficiently
            messageBuffer.clear();
            messageBuffer += "Executing command: ";
            for (const auto& token : cmd) {
                messageBuffer += token;
                messageBuffer += ' ';
            }
            
            logMessage(messageBuffer);
        }
        #endif

        const size_t cmdSize = cmd.size();

        // Process different command types with direct assignment to reuse string buffers
        if (commandName == LIST_STR) {
            if (cmdSize >= 2) {
                listString = cmd[1]; // Reuses existing string buffer
                removeQuotes(listString);
            }
        } 
        else if (commandName == LIST_FILE_STR) {
            if (cmdSize >= 2) {
                listPath = cmd[1];
                preprocessPath(listPath, packagePath);
            }
        } 
        else if (commandName == JSON_STR) {
            if (cmdSize >= 2) {
                jsonString = cmd[1];
            }
        } 
        else if (commandName == JSON_FILE_STR) {
            if (cmdSize >= 2) {
                jsonPath = cmd[1];
                preprocessPath(jsonPath, packagePath);
            }
        } 
        else if (commandName == INI_FILE_STR) {
            if (cmdSize >= 2) {
                iniPath = cmd[1];
                preprocessPath(iniPath, packagePath);
            }
        } 
        else if (commandName == HEX_FILE_STR) {
            if (cmdSize >= 2) {
                hexPath = cmd[1];
                preprocessPath(hexPath, packagePath);
            }
        } 
        else {
            // Process all other commands
            processCommand(cmd, packagePath, selectedCommand);
        }
        
        // Clear the processed command immediately to free its memory
        cmd = {};
        //cmd.clear();
        //cmd.shrink_to_fit();
    }

    // Final cleanup
    commands = {};
    //commands.clear();
    //commands.shrink_to_fit();

    #if USING_LOGGING_DIRECTIVE
    disableLogging = true;
    logFilePath = defaultLogFilePath;
    #endif

    return commandSuccess.load(std::memory_order_acquire);
}


// Helper function to parse command arguments
void parseCommandArguments(const std::vector<std::string>& cmd, const std::string& packagePath, std::string& sourceListPath, std::string& destinationListPath, std::string& logSource, std::string& logDestination, std::string& sourcePath, std::string& destinationPath, std::string& copyFilterListPath, std::string& filterListPath) {
    for (size_t i = 1; i < cmd.size(); ++i) {
        if (cmd[i] == "-src" && i + 1 < cmd.size()) {
            sourceListPath = cmd[++i];
            preprocessPath(sourceListPath, packagePath);
        } else if (cmd[i] == "-dest" && i + 1 < cmd.size()) {
            destinationListPath = cmd[++i];
            preprocessPath(destinationListPath, packagePath);
        } else if (cmd[i] == "-log_src" && i + 1 < cmd.size()) {
            logSource = cmd[++i];
            preprocessPath(logSource, packagePath);
        } else if (cmd[i] == "-log_dest" && i + 1 < cmd.size()) {
            logDestination = cmd[++i];
            preprocessPath(logDestination, packagePath);
        } else if ((cmd[i] == "-copy_filter" || cmd[i] == "-cp_filter") && i + 1 < cmd.size()) {
            copyFilterListPath = cmd[++i];
            preprocessPath(copyFilterListPath, packagePath);
        } else if (cmd[i] == "-filter" && i + 1 < cmd.size()) {
            filterListPath = cmd[++i];
            preprocessPath(filterListPath, packagePath);
        } else if (sourcePath.empty()) {
            sourcePath = cmd[i];
            preprocessPath(sourcePath, packagePath);
        } else if (destinationPath.empty()) {
            destinationPath = cmd[i];
            preprocessPath(destinationPath, packagePath);
        }
    }
}


void handleMakeDirCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd.size() >= 2) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        createDirectory(sourcePath);
    }
}

void handleCopyCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    // Declare only the strings we always need
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath);
    
    if (!sourceListPath.empty() && !destinationListPath.empty()) {
        // Process list-based copying
        auto sourceFilesList = readListFromFile(sourceListPath);
        auto destinationFilesList = readListFromFile(destinationListPath);
        
        // Only create filterSet if filter file exists
        std::unique_ptr<std::unordered_set<std::string>> filterSet;
        if (!filterListPath.empty()) {
            filterSet = std::make_unique<std::unordered_set<std::string>>(readSetFromFile(filterListPath));
        }
        
        const size_t listSize = std::min(sourceFilesList.size(), destinationFilesList.size());
        for (size_t i = 0; i < listSize; ++i) {
            // Reuse existing sourcePath and destinationPath strings
            sourcePath = std::move(sourceFilesList[i]);
            sourceFilesList[i].shrink_to_fit();     // Free the capacity
            preprocessPath(sourcePath, packagePath);
            
            destinationPath = std::move(destinationFilesList[i]);
            destinationFilesList[i].shrink_to_fit(); // Free the capacity
            preprocessPath(destinationPath, packagePath);
            
            // Only check filter if it exists
            const bool shouldCopy = !filterSet || filterSet->find(sourcePath) == filterSet->end();
            
            if (shouldCopy) {
                const long long totalSize = getTotalSize(sourcePath);
                long long totalBytesCopied = 0;
                copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
            }
        }
        
    } else {
        // Single file/directory copying - early returns to avoid unnecessary work
        if (sourcePath.empty() || destinationPath.empty()) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Source and destination paths must be specified.");
            #endif
            return;
        }
        
        if (!isFileOrDirectory(sourcePath)) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Source file or directory doesn't exist: " + sourcePath);
            #endif
            return;
        }
        
        if (sourcePath.find('*') != std::string::npos) {
            copyFileOrDirectoryByPattern(sourcePath, destinationPath, logSource, logDestination);
        } else {
            const long long totalSize = getTotalSize(sourcePath);
            long long totalBytesCopied = 0;
            copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize, logSource, logDestination);
        }
    }
}

void handleDeleteCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    // Declare only the strings we need
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath);
    
    if (!sourceListPath.empty()) {
        // Process list-based deletion
        auto sourceFilesList = readListFromFile(sourceListPath);
        
        // Only create filterSet if filter file exists
        std::unique_ptr<std::unordered_set<std::string>> filterSet;
        if (!filterListPath.empty()) {
            filterSet = std::make_unique<std::unordered_set<std::string>>(readSetFromFile(filterListPath));
        }
        
        for (size_t i = 0; i < sourceFilesList.size(); ++i) {
            // Move string to avoid copy
            sourcePath = std::move(sourceFilesList[i]);
            sourceFilesList[i].shrink_to_fit();     // Free the capacity
            preprocessPath(sourcePath, packagePath);
            
            // Only check filter if it exists
            const bool shouldDelete = !filterSet || filterSet->find(sourcePath) == filterSet->end();
            
            if (shouldDelete) {
                deleteFileOrDirectory(sourcePath);
            }
            
            // Clear the vector element immediately to free memory
            //sourceFilesList[i].clear();
        }
        
    } else {
        // Single file/directory deletion - early returns for error conditions
        if (sourcePath.empty()) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Source path must be specified.");
            #endif
            return;
        }
        
        if (isDangerousCombination(sourcePath)) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Dangerous combination detected.");
            #endif
            return;
        }
        
        // Perform the delete operation
        if (sourcePath.find('*') != std::string::npos) {
            deleteFileOrDirectoryByPattern(sourcePath, logSource);
        } else {
            deleteFileOrDirectory(sourcePath, logSource);
        }
    }
}


void handleMirrorCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    // Early validation
    if (cmd.size() < 2) {
        #if USING_LOGGING_DIRECTIVE
        if (!disableLogging)
            logMessage("Mirror command requires at least a source path.");
        #endif
        return;
    }
    
    // Extract and preprocess source path
    std::string sourcePath = cmd[1];
    preprocessPath(sourcePath, packagePath);
    
    // Extract destination path or use default
    std::string destinationPath;
    if (cmd.size() >= 3) {
        destinationPath = cmd[2];
        preprocessPath(destinationPath, packagePath);
    } else {
        destinationPath = ROOT_PATH;
    }
    
    // Determine operation type using string_view to avoid string creation
    const std::string_view commandName = cmd[0];
    const std::string operation = (commandName == "mirror_copy" || commandName == "mirror_cp") ? "copy" : "delete";
    
    if (sourcePath.find('*') == std::string::npos) {
        // Single directory mirror
        mirrorFiles(sourcePath, destinationPath, operation);
    } else {
        // Wildcard mirror - get file list and process with immediate cleanup
        auto fileList = getFilesListByWildcards(sourcePath);
        
        // Process files one by one, freeing memory as we go
        for (size_t i = 0; i < fileList.size(); ++i) {
            // Move the string to avoid copy
            const auto sourceDirectory = std::move(fileList[i]);
            fileList[i].shrink_to_fit();     // Free the capacity
            mirrorFiles(sourceDirectory, destinationPath, operation);
            
            // Clear the vector element immediately to free memory
            //fileList[i].clear();
        }
    }
}

void handleMoveCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    // Declare only the strings we need
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath);
    
    if (!sourceListPath.empty() && !destinationListPath.empty()) {
        // Load filter sets (these are typically small)
        std::unique_ptr<std::unordered_set<std::string>> copyFilterSet;
        if (!copyFilterListPath.empty()) {
            copyFilterSet = std::make_unique<std::unordered_set<std::string>>(readSetFromFile(copyFilterListPath));
        }
        
        std::unique_ptr<std::unordered_set<std::string>> filterSet;
        if (!filterListPath.empty()) {
            filterSet = std::make_unique<std::unordered_set<std::string>>(readSetFromFile(filterListPath));
        }
    
        // Pre-allocate strings to avoid reallocations in loop
        //sourcePath.reserve(1024);
        //destinationPath.reserve(1024);
    
        // Stream process both files line by line
    #if !USING_FSTREAM_DIRECTIVE
        FILE* sourceFile = fopen(sourceListPath.c_str(), "r");
        FILE* destFile = fopen(destinationListPath.c_str(), "r");
        
        if (!sourceFile || !destFile) {
            if (sourceFile) fclose(sourceFile);
            if (destFile) fclose(destFile);
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open source or destination list files");
            #endif
            return;
        }
        
        // Set larger buffers for better I/O performance
        static constexpr size_t FILE_BUFFER_SIZE = 8192;
        setvbuf(sourceFile, nullptr, _IOFBF, FILE_BUFFER_SIZE);
        setvbuf(destFile, nullptr, _IOFBF, FILE_BUFFER_SIZE);
        
        static constexpr size_t BUFFER_SIZE = 8192;
        char sourceBuffer[BUFFER_SIZE];
        char destBuffer[BUFFER_SIZE];
        
        // Static string for repeated log messages to reduce allocations
        #if USING_LOGGING_DIRECTIVE
        static const std::string skipDirMsg = "Skipping non-empty directory: ";
        #endif
        
        // Process files line by line simultaneously
        while (fgets(sourceBuffer, BUFFER_SIZE, sourceFile) && 
               fgets(destBuffer, BUFFER_SIZE, destFile)) {
            
            // Optimized newline removal - scan once instead of using strlen
            char* srcEnd = sourceBuffer;
            while (*srcEnd && *srcEnd != '\n') ++srcEnd;
            const size_t sourceLen = srcEnd - sourceBuffer;
            *srcEnd = '\0';
            
            char* destEnd = destBuffer;
            while (*destEnd && *destEnd != '\n') ++destEnd;
            const size_t destLen = destEnd - destBuffer;
            *destEnd = '\0';
            
            // Clear and append to reuse existing string capacity
            sourcePath.clear();
            sourcePath.append(sourceBuffer, sourceLen);
            
            destinationPath.clear();
            destinationPath.append(destBuffer, destLen);
            
            preprocessPath(sourcePath, packagePath);
            preprocessPath(destinationPath, packagePath);
            
            // Cache filter lookup result
            const bool shouldProcess = !filterSet || filterSet->find(sourcePath) == filterSet->end();
            
            if (shouldProcess) {
                // Check if it's a directory using the buffer directly (avoid string access)
                const bool isDirectory = sourceLen > 0 && sourcePath[sourceLen - 1] == '/';
                
                if (!isDirectory) {
                    // Check copy filter once and cache result
                    const bool shouldCopy = copyFilterSet && copyFilterSet->find(sourcePath) != copyFilterSet->end();
                    
                    if (shouldCopy) {
                        const long long totalSize = getTotalSize(sourcePath);
                        long long totalBytesCopied = 0;
                        copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
                    } else {
                        moveFileOrDirectory(sourcePath, destinationPath, logSource, logDestination);
                    }
                } else {
                    if (isDirectoryEmpty(sourcePath)) {
                        moveFileOrDirectory(sourcePath, destinationPath, logSource, logDestination);
                    }
                    #if USING_LOGGING_DIRECTIVE
                    else if (!disableLogging) {
                        logMessage(skipDirMsg + sourcePath);
                    }
                    #endif
                }
            }
            
            // Periodically shrink strings if they've grown too large
            // This prevents unbounded memory growth for very long paths
            if (sourcePath.capacity() > 8192) {
                sourcePath.shrink_to_fit();
            }
            if (destinationPath.capacity() > 8192) {
                destinationPath.shrink_to_fit();
            }
        }
        
        fclose(sourceFile);
        fclose(destFile);
        
    #else
        std::ifstream sourceFile(sourceListPath);
        std::ifstream destFile(destinationListPath);
        
        if (!sourceFile.is_open() || !destFile.is_open()) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open source or destination list files");
            #endif
            return;
        }
        
        // Set larger buffers for better I/O performance
        static char sourceFileBuffer[8192], destFileBuffer[8192];
        sourceFile.rdbuf()->pubsetbuf(sourceFileBuffer, sizeof(sourceFileBuffer));
        destFile.rdbuf()->pubsetbuf(destFileBuffer, sizeof(destFileBuffer));
        
        // Process files line by line simultaneously
        while (std::getline(sourceFile, sourcePath) && std::getline(destFile, destinationPath)) {
            preprocessPath(sourcePath, packagePath);
            preprocessPath(destinationPath, packagePath);
            
            // Cache filter lookup result  
            const bool shouldProcess = !filterSet || filterSet->find(sourcePath) == filterSet->end();
            
            if (shouldProcess) {
                // Check if it's a directory (ends with /)
                const bool isDirectory = !sourcePath.empty() && sourcePath.back() == '/';
                
                if (!isDirectory) {
                    // Check copy filter once and cache result
                    const bool shouldCopy = copyFilterSet && copyFilterSet->find(sourcePath) != copyFilterSet->end();
                    
                    if (shouldCopy) {
                        const long long totalSize = getTotalSize(sourcePath);
                        long long totalBytesCopied = 0;
                        copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
                    } else {
                        moveFileOrDirectory(sourcePath, destinationPath, logSource, logDestination);
                    }
                } else {
                    if (isDirectoryEmpty(sourcePath)) {
                        moveFileOrDirectory(sourcePath, destinationPath, logSource, logDestination);
                    }
                    #if USING_LOGGING_DIRECTIVE
                    else if (!disableLogging) {
                        logMessage("Skipping non-empty directory: " + sourcePath);
                    }
                    #endif
                }
            }
        }
    #endif
    
        
    } else {
        // Single file/directory moving - early returns for error conditions
        if (sourcePath.empty() || destinationPath.empty()) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Source and destination paths must be specified.");
            #endif
            return;
        }
        
        if (isDangerousCombination(sourcePath)) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Dangerous combination detected.");
            #endif
            return;
        }
        
        // Perform the move operation
        if (sourcePath.find('*') != std::string::npos) {
            moveFilesOrDirectoriesByPattern(sourcePath, destinationPath, logSource, logDestination);
        } else {
            moveFileOrDirectory(sourcePath, destinationPath, logSource, logDestination);
        }
    }
}

void handleIniCommands(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd[0] == "add-ini-section" && cmd.size() >= 2) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string desiredSection = cmd[2];
        removeQuotes(desiredSection);
        addIniSection(sourcePath, desiredSection);
    } else if (cmd[0] == "rename-ini-section" && cmd.size() >= 3) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string desiredSection = cmd[2];
        removeQuotes(desiredSection);
        std::string desiredNewSection = cmd[3];
        removeQuotes(desiredNewSection);
        renameIniSection(sourcePath, desiredSection, desiredNewSection);
    } else if (cmd[0] == "remove-ini-section" && cmd.size() >= 2) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string desiredSection = cmd[2];
        removeQuotes(desiredSection);
        removeIniSection(sourcePath, desiredSection);
    } else if (cmd[0] == "remove-ini-key" && cmd.size() >= 3) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string desiredSection = cmd[2];
        removeQuotes(desiredSection);
        std::string desiredKey = cmd[3];
        removeQuotes(desiredKey);
        removeIniKey(sourcePath, desiredSection, desiredKey);
    } else if ((cmd[0] == "set-ini-val" || cmd[0] == "set-ini-value") && cmd.size() >= 5) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string desiredSection = cmd[2];
        removeQuotes(desiredSection);
        std::string desiredKey = cmd[3];
        removeQuotes(desiredKey);
        std::string desiredValue = std::accumulate(cmd.begin() + 4, cmd.end(), std::string(""), [](const std::string& a, const std::string& b) -> std::string {
            std::string returnStr = (a.empty() ? b : a + " " + b);
            removeQuotes(returnStr);
            return returnStr;
        });
        setIniFileValue(sourcePath, desiredSection, desiredKey, desiredValue);
    } else if (cmd[0] == "set-ini-key" && cmd.size() >= 5) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string desiredSection = cmd[2];
        removeQuotes(desiredSection);
        std::string desiredKey = cmd[3];
        removeQuotes(desiredKey);
        std::string desiredNewKey = std::accumulate(cmd.begin() + 4, cmd.end(), std::string(""), [](const std::string& a, const std::string& b) -> std::string {
            std::string returnStr = (a.empty() ? b : a + " " + b);
            removeQuotes(returnStr);
            return returnStr;
        });
        setIniFileKey(sourcePath, desiredSection, desiredKey, desiredNewKey);
    }
}

void handleJsonCommands(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd[0] == "set-json-key" && cmd.size() >= 4) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        
        std::string oldKey = cmd[2];
        removeQuotes(oldKey);
        
        std::string newKey = std::accumulate(cmd.begin() + 3, cmd.end(), std::string(""), [](const std::string& a, const std::string& b) -> std::string {
            std::string returnStr = (a.empty() ? b : a + " " + b);
            removeQuotes(returnStr);
            return returnStr;
        });
        
        ult::renameJsonKey(sourcePath, oldKey, newKey);
    }
    else if ((cmd[0] == "set-json-val" || cmd[0] == "set-json-value") && cmd.size() >= 4) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        
        std::string desiredKey = cmd[2];
        removeQuotes(desiredKey);
        
        std::string desiredValue = std::accumulate(cmd.begin() + 3, cmd.end(), std::string(""), [](const std::string& a, const std::string& b) -> std::string {
            std::string returnStr = (a.empty() ? b : a + " " + b);
            removeQuotes(returnStr);
            return returnStr;
        });
        
        // set-json-val and set-json-value create file if it doesn't exist
        bool createIfNotExists = true;
        
        ult::setJsonValue(sourcePath, desiredKey, desiredValue, createIfNotExists);
    }
}

void handleHexEdit(const std::string& sourcePath, const std::string& secondArg, const std::string& thirdArg, const std::string& fourthArg, const std::string& fifthArg, const std::string& commandName, const std::vector<std::string>& cmd) {
    if (commandName == "hex-by-offset") {
        hexEditByOffset(sourcePath, secondArg, thirdArg);
    } else if (commandName == "hex-by-swap") {
        if (cmd.size() >= 5) {
            const size_t occurrence = std::stoul(fourthArg);
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
            std::string selectedStr = cmd[4];
            removeQuotes(selectedStr);
            const size_t occurrence = std::stoul(selectedStr);
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    } else if (commandName == "hex-by-decimal") {

        std::string hexDataToReplace;
        std::string hexDataReplacement;
    
        if (fourthArg.empty()) {
            hexDataToReplace = decimalToHex(secondArg);
            hexDataReplacement = decimalToHex(thirdArg);
        } else {
            hexDataToReplace = decimalToHex(secondArg, ult::stoi(fourthArg));
            hexDataReplacement = decimalToHex(thirdArg, ult::stoi(fourthArg));
        }
    
        if (cmd.size() >= 6) {
            const size_t occurrence = std::stoul(fifthArg);
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    } else if (commandName == "hex-by-rdecimal") {
        std::string hexDataToReplace;
        std::string hexDataReplacement;
    
        if (fourthArg.empty()) {
            hexDataToReplace = decimalToReversedHex(secondArg);
            hexDataReplacement = decimalToReversedHex(thirdArg);
        } else {
            hexDataToReplace = decimalToReversedHex(secondArg, ult::stoi(fourthArg));
            hexDataReplacement = decimalToReversedHex(thirdArg, ult::stoi(fourthArg));
        }
    
        if (cmd.size() >= 6) {
            const size_t occurrence = std::stoul(fifthArg);
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    }
}

void handleHexByCustom(const std::string& sourcePath, const std::string& customPattern, const std::string& offset, std::string hexDataReplacement, const std::string& commandName, std::string byteGroupSize) {
    if (hexDataReplacement != NULL_STR) {
        if (commandName == "hex-by-custom-decimal-offset") {
            if (!byteGroupSize.empty())
                hexDataReplacement = decimalToHex(hexDataReplacement, ult::stoi(byteGroupSize));
            else
                hexDataReplacement = decimalToHex(hexDataReplacement);
        } else if (commandName == "hex-by-custom-rdecimal-offset") {
            if (!byteGroupSize.empty())
                hexDataReplacement = decimalToReversedHex(hexDataReplacement, ult::stoi(byteGroupSize));
            else
                hexDataReplacement = decimalToReversedHex(hexDataReplacement);
        }
        hexEditByCustomOffset(sourcePath, customPattern, offset, hexDataReplacement);
    }
}


void rebootToHekateConfig(Payload::HekateConfigList& configList, const std::string& option, bool isIni) {
    int rebootIndex = -1;  // Initialize rebootIndex to -1, indicating no match found
    auto configIterator = configList.begin();

    if (std::all_of(option.begin(), option.end(), ::isdigit)) {
        rebootIndex = ult::stoi(option);
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
    } else if (commandName == "set-json-val" || commandName == "set-json-value" || commandName == "set-json-key") {
        handleJsonCommands(cmd, packagePath);
    } else if (commandName == "set-footer") {
        if (cmd.size() >= 2) {
            std::string desiredValue = cmd[1];
            removeQuotes(desiredValue);
            if (desiredValue.find(NULL_STR) != std::string::npos)
                commandSuccess.store(false, std::memory_order_release);
            else
                setIniFileValue((packagePath + CONFIG_FILENAME), selectedCommand, FOOTER_STR, desiredValue);
        }
    } else if (commandName == "compare") {
        if (cmd.size() >= 4) {
            std::string path1 = cmd[1];
            preprocessPath(path1, packagePath);
            std::string path2 = cmd[2];
            preprocessPath(path2, packagePath);
            std::string outputPath = cmd[3];
            preprocessPath(outputPath, packagePath);
            if (path1.find('*') != std::string::npos)
                compareWildcardFilesLists(path1, path2, outputPath);
            else
                compareFilesLists(path1, path2, outputPath);
        }
    } else if (commandName == "flag") {
        if (cmd.size() >= 3) {
            std::string wildcardPattern = cmd[1];
            preprocessPath(wildcardPattern, packagePath);
            std::string outputDir = cmd[2];
            preprocessPath(outputDir, packagePath);
            createFlagFiles(wildcardPattern, outputDir);
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Usage: flag <wildcardPattern> <outputDir>");
            #endif
        }
    } else if (commandName == "dot-clean") {
        if (cmd.size() >= 2) {
            std::string path = cmd[1];
            preprocessPath(path, packagePath);
            dotCleanDirectory(path);
        }
    } else if (commandName.substr(0, 7) == "hex-by-") {
        if (cmd.size() >= 4) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
    
            std::string secondArg = cmd[2];
            removeQuotes(secondArg);
            std::string thirdArg = cmd[3];
            removeQuotes(thirdArg);
            
            std::string fourthArg;  // optional paramter, default empty
            if (cmd.size() >= 5) {
                fourthArg = cmd[4];
                removeQuotes(fourthArg);
            }

            std::string fifthArg;  // optional paramter, default empty
            if (cmd.size() >= 6) {
                fifthArg = cmd[5];
                removeQuotes(fifthArg);
            }
    
            if (commandName == "hex-by-custom-offset" || commandName == "hex-by-custom-decimal-offset" || commandName == "hex-by-custom-rdecimal-offset") {
                if (cmd.size() >= 5) {

                    std::string customPattern = cmd[2];
                    std::string offset = cmd[3];
                    std::string hexDataReplacement = cmd[4];
            
                    removeQuotes(customPattern);
                    removeQuotes(offset);
                    removeQuotes(hexDataReplacement);
            
                    std::string byteGroupSize;  // optional
                    if (cmd.size() >= 6) {
                        byteGroupSize = cmd[5];
                        removeQuotes(byteGroupSize);
                    }
            
                    handleHexByCustom(sourcePath, customPattern, offset, hexDataReplacement, commandName, byteGroupSize);
                }
            } else {
                handleHexEdit(sourcePath, secondArg, thirdArg, fourthArg, fifthArg, commandName, cmd);
            }
        }
    } else if (commandName == "download") {
        if (cmd.size() >= 3) {
            std::string fileUrl = cmd[1];
            preprocessUrl(fileUrl);
            std::string destinationPath = cmd[2];
            preprocessPath(destinationPath, packagePath);
            bool downloadSuccess = false;
            for (size_t i = 0; i < 3; ++i) {
                downloadSuccess = downloadFile(fileUrl, destinationPath);
                if (abortDownload.load(std::memory_order_acquire)) {
                    downloadSuccess = false;
                    break;
                }
                if (downloadSuccess) break;
            }
            commandSuccess.store(
                downloadSuccess &&
                commandSuccess.load(std::memory_order_acquire),
                std::memory_order_release
            );
        }
    } else if (commandName == "unzip") {
        if (cmd.size() >= 3) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            std::string destinationPath = cmd[2];
            preprocessPath(destinationPath, packagePath);
            commandSuccess.store(
                unzipFile(sourcePath, destinationPath) &&
                commandSuccess.load(std::memory_order_acquire),
                std::memory_order_release
            );
        }
    } else if (commandName == "pchtxt2ips") {
        if (cmd.size() >= 3) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            std::string destinationPath = cmd[2];
            preprocessPath(destinationPath, packagePath);
            commandSuccess.store(
                pchtxt2ips(sourcePath, destinationPath) &&
                commandSuccess.load(std::memory_order_acquire),
                std::memory_order_release
            );
        }
    } else if (commandName == "pchtxt2cheat") {
        if (cmd.size() >= 2) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            commandSuccess.store(
                pchtxt2cheat(sourcePath) &&
                commandSuccess.load(std::memory_order_acquire),
                std::memory_order_release
            );
        }
    } else if (commandName == "exec") {
        if (cmd.size() >= 2) {
            std::string bootCommandName = cmd[1];
            removeQuotes(bootCommandName);
            if (isFileOrDirectory(packagePath + BOOT_PACKAGE_FILENAME)) {
                // Load only the commands from the specific section (bootCommandName)
                auto bootCommands = loadSpecificSectionFromIni(packagePath + BOOT_PACKAGE_FILENAME, bootCommandName);
            
                if (!bootCommands.empty()) {
                    bool resetCommandSuccess = false;
                    if (!commandSuccess.load(std::memory_order_acquire)) resetCommandSuccess = true;
            
                    interpretAndExecuteCommands(std::move(bootCommands), packagePath, bootCommandName);
                    resetPercentages();
                    if (resetCommandSuccess) {
                        commandSuccess.store(false, std::memory_order_release);
                    }
                }
            }

        }
    } else if (commandName == "reboot") { // credits to Studious Pancake for the Payload and utils methods
        //spsmInitialize();
        //i2cInitialize();
        bool launchUpdaterPayload = false;
        // Check each protected file for a corresponding `.ultra` file
        for (const std::string& file : PROTECTED_FILES) {
            if (isFile(file + ".ultra")) {
                // If found, download the updater payload and mark for launch
                launchUpdaterPayload = true;
                break;
            }
        }
    
        if (launchUpdaterPayload) {
            const std::string rebootOption = PAYLOADS_PATH + "ultrahand_updater.bin";
            if (!isFile(rebootOption)) {
                downloadFile(UPDATER_PAYLOAD_URL, PAYLOADS_PATH, true);
                downloadPercentage.store(-1, std::memory_order_release);
            }
            if (isFile(rebootOption)) {
                const std::string fileName = getNameFromPath(rebootOption);
    
                if (util::IsErista()) {
                    Payload::PayloadConfig reboot_payload = { fileName, rebootOption };
                    Payload::RebootToPayload(reboot_payload);
                } else {
                    // Strip ROOT_PATH from rebootOption before using in ini
                    std::string strippedRebootOption = rebootOption;
                    if (strippedRebootOption.find(ROOT_PATH) == 0) {
                        strippedRebootOption = strippedRebootOption.substr(ROOT_PATH.length());
                    }
                    
                    deleteFileOrDirectory("/bootloader/ini/" + fileName + ".ini");
                    setIniFileValue("/bootloader/ini/" + fileName + ".ini", fileName, "payload", strippedRebootOption);
                    setIniFileValue("/bootloader/ini/" + fileName + ".ini", fileName, "bootwait", "0");
                    Payload::HekateConfigList iniConfigList = Payload::LoadIniConfigList();
                    rebootToHekateConfig(iniConfigList, fileName, true);
                }
            } else {
                launchUpdaterPayload = false; // failed to find payload
            }
        }
        if (!launchUpdaterPayload && (util::IsErista() || util::SupportsMarikoRebootToConfig())) {
            std::string rebootOption;
            if (cmd.size() >= 2) {
                rebootOption = cmd[1];
                removeQuotes(rebootOption);
                if (cmd.size() >= 3) {
                    std::string option = cmd[2];
                    removeQuotes(option);
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
                } else if (isFile(rebootOption)) {
                    const std::string fileName = getNameFromPath(rebootOption);
                    if (util::IsErista()) {
                        Payload::PayloadConfig reboot_payload = {fileName, rebootOption};
                        Payload::RebootToPayload(reboot_payload);
                    } else {
                        // Strip ROOT_PATH from rebootOption before using in ini
                        std::string strippedRebootOption = rebootOption;
                        if (strippedRebootOption.find(ROOT_PATH) == 0) {
                            strippedRebootOption = strippedRebootOption.substr(ROOT_PATH.length());
                        }
                        
                        setIniFileValue("/bootloader/ini/" + fileName + ".ini", fileName, "payload", strippedRebootOption);
                        setIniFileValue("/bootloader/ini/" + fileName + ".ini", fileName, "bootwait", "0");
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
        spsmExit();
    } else if (commandName == "shutdown") {
        if (cmd.size() >= 2) {
            std::string selection = cmd[1];
            removeQuotes(selection);
            if (selection == "controllers") {
                powerOffAllControllers();
            }
        } else {
            //spsmInitialize();
            splExit();
            fsdevUnmountAll();
            spsmShutdown(SpsmShutdownMode_Normal);
            spsmExit();
        }
        //if (cmd.size() >= 1) {
        //    splExit();
        //    fsdevUnmountAll();
        //    spsmShutdown(SpsmShutdownMode_Normal);
        //}
    } else if (commandName == "exit") {
        //triggerExit.store(true, std::memory_order_release);
        if (cmd.size() >= 2) {
            std::string selection = cmd[1];
            removeQuotes(selection);
            if (selection == "overlays") {
                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
            } else if (selection == "packages") {
                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, "to_packages", TRUE_STR); // this is handled within tesla.hpp
                setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
            }
        }
        exitingUltrahand.store(true, std::memory_order_release);
        ult::launchingOverlay.store(true, std::memory_order_release);
        //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
        tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
        tsl::Overlay::get()->close(true);
        return;
    } else if (commandName == "back") {
        goBackAfter.store(true, std::memory_order_release);
        
    } else if (commandName == "backlight") {
        if (cmd.size() >= 2) {
            std::string togglePattern = cmd[1];
            removeQuotes(togglePattern);
            lblInitialize();
            if (togglePattern == "auto") {
                if (cmd.size() >= 3) {
                    togglePattern = cmd[2];
                    if (togglePattern == ON_STR)
                        lblEnableAutoBrightnessControl();
                    else if (togglePattern == OFF_STR)
                        lblDisableAutoBrightnessControl();
                }
            }
            else if (togglePattern == ON_STR)
                lblSwitchBacklightOn(0);
            else if (togglePattern == OFF_STR)
                lblSwitchBacklightOff(0);
            else if (isValidNumber(togglePattern)) {
                lblSetCurrentBrightnessSetting(ult::stof(togglePattern) / 100.0f);
            }
            lblExit();
        }
    } else if (commandName == "volume") {
        if (cmd.size() >= 2) {
            std::string volumeInput = cmd[1];
            removeQuotes(volumeInput);  // Sanitize input by removing quotes
            
            if (isValidNumber(volumeInput)) {
                //logMessage("Volume input is a valid number: " + volumeInput);  // Log valid number
                
                // Convert input string to a float for percentage (0-100)
                const float volumePercentage = ult::stof(volumeInput);
                //logMessage("Converted volume to percentage: " + ult::to_string(volumePercentage));  // Log the percentage
                
                // Ensure the volume is within valid range 0 to 100
                if (volumePercentage < 0.0f || volumePercentage > 150.0f) {
                    //logMessage("Volume percentage out of bounds: " + ult::to_string(volumePercentage));
                    return;  // Exit if invalid percentage
                }
                
                // Convert percentage (0-100) to volume scale (0-1)
                const float masterVolume = volumePercentage / 100.0f;
                

                //logMessage("Initializing settings service...");
                audctlInitialize();

                // Set the master volume
                audctlSetSystemOutputMasterVolume(masterVolume);

                audctlExit();
            }
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Volume command missing required argument.");
            #endif
        }
    } else if (commandName == "open") {
        if (cmd.size() >= 2) {
            std::string overlayPath = cmd[1];
            removeQuotes(overlayPath);
            preprocessPath(overlayPath, packagePath);
            
            // Verify the overlay file exists
            if (!isFileOrDirectory(overlayPath)) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Overlay file not found: " + overlayPath);
                #endif
                commandSuccess.store(false, std::memory_order_release);
                return;
            }
            
            // Build launch arguments from remaining command arguments
            std::string launchArgs, arg;
            if (cmd.size() > 2) {
                // Join all arguments after the overlay path
                for (size_t i = 2; i < cmd.size(); ++i) {
                    if (i > 2) launchArgs += " ";
                    arg = cmd[i];
                    removeQuotes(arg);
                    launchArgs += arg;
                }
            }
            
            // Always add --direct flag if not already present
            //if (launchArgs.empty()) {
            //    launchArgs = "--direct";
            //} else if (launchArgs.find("--direct") == std::string::npos) {
            //    launchArgs += " --direct";
            //}
            
            const std::string overlayFileName = ult::getNameFromPath(overlayPath);
            
            // Check if overlay is hidden (if hideHidden is enabled)
            //if (hideHidden) {
            //    const auto hideStatus = ult::parseValueFromIniSection(
            //        ult::OVERLAYS_INI_FILEPATH, overlayFileName, ult::HIDE_STR);
            //    if (hideStatus == ult::TRUE_STR) {
            //        #if USING_LOGGING_DIRECTIVE
            //        logMessage("Cannot open hidden overlay: " + overlayFileName);
            //        #endif
            //        commandSuccess.store(false, std::memory_order_release);
            //        return;
            //    }
            //}
            
            // Request overlay launch through background event poller
            {
                std::lock_guard<std::mutex> lock(ult::overlayLaunchMutex);
                ult::requestedOverlayPath = overlayPath;
                ult::requestedOverlayArgs = launchArgs;
                ult::overlayLaunchRequested.store(true, std::memory_order_release);
            }
            
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Requesting overlay launch: " + overlayPath + " with args: " + launchArgs);
            #endif

            return;
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Usage: open <overlay_path> [launch_arguments...]");
            #endif
            commandSuccess.store(false, std::memory_order_release);
        }
    
    } else if (commandName == "refresh") {
        if (cmd.size() == 1) {
            refreshPage.store(true, std::memory_order_release);
        } else if (cmd.size() > 1) {
            std::string refreshPattern = cmd[1];
            removeQuotes(refreshPattern);
            if (refreshPattern == "theme")
                tsl::initializeThemeVars();
            else if (refreshPattern == "package")
                refreshPackage.store(true, std::memory_order_release);
            else if (refreshPattern == "wallpaper") {
                reloadWallpaper();
            //} else {
            //    std::string refreshPattern2 = "";
            //    if (cmd.size() > 2) {
            //        refreshPattern2 = cmd[2];
            //    }
            //    jumpItemName = refreshPattern;
            //    jumpItemValue = refreshPattern2;
            //    refreshPage.store(true, std::memory_order_release);
            }
        }
    } else if (commandName == "refresh-to") {
        if (cmd.size() > 1) {
            std::string refreshPattern = cmd[1];
            removeQuotes(refreshPattern);
            std::string refreshPattern2 = "";
            std::string refreshPattern3 = "";
            
            if (cmd.size() > 2) {
                refreshPattern2 = cmd[2];
                removeQuotes(refreshPattern2);
            }
            
            if (cmd.size() > 3) {
                refreshPattern3 = cmd[3];
                removeQuotes(refreshPattern3);
            }
            
            jumpItemName = refreshPattern;
            jumpItemValue = refreshPattern2;
            jumpItemExactMatch = !(refreshPattern3 == FALSE_STR);
            skipJumpReset.store(true, std::memory_order_release);
            refreshPage.store(true, std::memory_order_release);
        }
    } else if (commandName == "logging") {
        interpreterLogging.store(true, std::memory_order_release);
    } else if (commandName == "notify" || commandName == "notification") {
        if (cmd.size() > 1) {
            std::string text = cmd[1];
            removeQuotes(text);
            size_t fontSize = 28;
            if (cmd.size() > 2) {
                std::string fontSizeStr = cmd[2];
                removeQuotes(fontSizeStr);
                fontSize = std::stoi(fontSizeStr);

                // Clamp font size to [1, 34]
                if (fontSize < 1) fontSize = 1;
                else if (fontSize > 34) fontSize = 34;

            }
            if (tsl::notification)
                tsl::notification->show(text, fontSize);
        }
        //if (cmd.size() > 1) {
        //    std::string text = cmd[1];
        //    removeQuotes(text);
        //    
        //    size_t fontSize = 28;
        //    if (cmd.size() > 2) {
        //        std::string fontSizeStr = cmd[2];
        //        removeQuotes(fontSizeStr);
        //        fontSize = std::stoi(fontSizeStr);
        //        fontSize = std::clamp(fontSize, size_t(1), size_t(34));
        //    }
        //    
        //    // Push as cJSON
        //    pushNotificationJson(text, fontSize);
        //}
    } else if (commandName == "clear") {
        if (cmd.size() >= 2) {
            std::string clearOption = cmd[1];
            removeQuotes(clearOption);
            if (clearOption == "log") {
                #if USING_LOGGING_DIRECTIVE
                deleteFileOrDirectory(defaultLogFilePath);
                #endif
            }
            else if (clearOption == "hex_sum_cache") hexSumCache.clear();
        }
    }
}

void executeCommands(std::vector<std::vector<std::string>> commands) {
    interpretAndExecuteCommands(std::move(commands), "", "");
    resetPercentages();
}

void executeIniCommands(const std::string &iniPath, const std::string &section) {
    if (isFileOrDirectory(iniPath)) {
        auto commands = loadSpecificSectionFromIni(iniPath, section);
        if (!commands.empty()) {
            interpretAndExecuteCommands(std::move(commands), PACKAGE_PATH, section);
            resetPercentages();
        }
    }
}



// Thread information structure
Thread interpreterThread;
std::atomic<bool> interpreterThreadExit{false};

// Cache for stack size to avoid repeated INI parsing
static int cachedStackSize = 0;

// Work data structure to pass to thread
struct InterpreterWorkData {
    std::vector<std::vector<std::string>> commands;
    std::string packagePath;
    std::string selectedCommand;
    
    InterpreterWorkData(std::vector<std::vector<std::string>>&& cmds, 
                       const std::string& path, 
                       const std::string& selected)
        : commands(std::move(cmds)), packagePath(path), selectedCommand(selected) {}
};

inline void clearInterpreterFlags(bool state = false) {
    // Use relaxed ordering for simple flag clearing - these are just state flags
    // and don't need acquire-release synchronization
    abortDownload.store(state, std::memory_order_relaxed);
    abortUnzip.store(state, std::memory_order_relaxed);
    abortFileOp.store(state, std::memory_order_relaxed);
    abortCommand.store(state, std::memory_order_relaxed);
}

void backgroundInterpreter(void* workPtr) {
    // Get work data directly - no queue needed
    auto workData = static_cast<InterpreterWorkData*>(workPtr);
    
    // Check for exit signal before starting work
    if (interpreterThreadExit.load(std::memory_order_acquire)) {
        delete workData;
        return;
    }
    
    // Process the work if we have commands
    if (!workData->commands.empty()) {
        // Clear flags and setup for execution
        clearInterpreterFlags();
        resetPercentages();
        threadFailure.store(false, std::memory_order_release);
        
        runningInterpreter.store(true, std::memory_order_release);
        
        // Execute the commands
        interpretAndExecuteCommands(std::move(workData->commands), 
                                   std::move(workData->packagePath), 
                                   std::move(workData->selectedCommand));

        // Brief sleep before cleanup
        svcSleepThread(200'000'000);

        // Cleanup
        runningInterpreter.store(false, std::memory_order_release);
        clearInterpreterFlags();
        resetPercentages();
    }
    
    // Clean up work data
    delete workData;
    
    // Thread naturally exits here
}

void closeInterpreterThread() {
    // Signal any running thread to exit
    interpreterThreadExit.store(true, std::memory_order_release);
    
    // Wait for thread to finish and clean up
    threadWaitForExit(&interpreterThread);
    threadClose(&interpreterThread);
    
    // Reset state
    clearInterpreterFlags();
    interpreterThreadExit.store(false, std::memory_order_release);
}

int getInterpreterStackSize(const std::string& packagePath = "") {
    #if USING_LOGGING_DIRECTIVE
    if (!packagePath.empty()) {
        disableLogging = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_LOGGING_STR) == TRUE_STR);
        logFilePath = packagePath + "log.txt";
    }
    #endif

    // Cache stack size parsing to avoid repeated INI file access
    if (cachedStackSize == 0) {
        const std::string interpreterHeap = parseValueFromIniSection(ULTRAHAND_CONFIG_INI_PATH, MEMORY_STR, "interpreter_heap");
        if (!interpreterHeap.empty()) {
            cachedStackSize = ult::stoi(interpreterHeap, nullptr, 16);  // Convert from base 16
        } else {
            cachedStackSize = 0x8000;  // Default value
        }
    }
    return cachedStackSize;
}



// Combined function - creates thread with work data directly
void executeInterpreterCommands(std::vector<std::vector<std::string>>&& commands, 
                               const std::string& packagePath = "", 
                               const std::string& selectedCommand = "") {
    
    // Wait for the existing thread to finish
    threadWaitForExit(&interpreterThread);
    threadClose(&interpreterThread);

    // Early exit if no commands
    if (commands.empty()) {
        return;
    }
    
    // Get stack size and setup logging
    const int stackSize = getInterpreterStackSize(packagePath);
    
    // Ensure exit flag is clear before starting
    interpreterThreadExit.store(false, std::memory_order_release);
    
    // Create work data (will be cleaned up by the thread)
    auto workData = new InterpreterWorkData(std::move(commands), packagePath, selectedCommand);
    
    // Create and start thread directly with work data
    const int result = threadCreate(&interpreterThread, backgroundInterpreter, workData, nullptr, stackSize, 0x2B, -2);
    if (result != 0) {
        // Handle thread creation failure
        commandSuccess.store(false, std::memory_order_release);
        clearInterpreterFlags();
        runningInterpreter.store(false, std::memory_order_release);
        
        // Clean up work data since thread won't
        delete workData;
        
        #if USING_LOGGING_DIRECTIVE
        if (!disableLogging)
            logMessage("Failed to create interpreter thread.");
        logFilePath = defaultLogFilePath;
        disableLogging = true;
        #endif
        return;
    }
    threadStart(&interpreterThread);
}
