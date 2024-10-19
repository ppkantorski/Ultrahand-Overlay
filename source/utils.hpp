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
//#include <sys/statvfs.h>


using namespace ult;

static std::atomic<bool> abortCommand(false);
static std::atomic<bool> triggerExit(false);

static bool exitingUltrahand = false;

bool isDownloadCommand = false;
bool commandSuccess = false;
bool refreshPage = false;
bool refreshPackage = false;
bool interpreterLogging = false;

bool usingErista = util::IsErista();
bool usingMariko = util::IsMariko();

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
    {"{INFO_SYMBOL}", ""}
};




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
    std::ofstream outFile(outputPath);
    if (outFile) {
        //outFile.write("; do not adjust these values manually unless they were not dumped correctly\n", 81);
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
        default: return "";
    }
}



const char* getStorageInfo(const std::string& storageType) {
    s64 freeSpace = 0;
    s64 totalSpace = 0;
    char* buffer = (char*)malloc(30);

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

    // Calculate the used space
    s64 usedSpace = totalSpace - freeSpace;

    // Convert the used, free, and total space from bytes to GB
    float usedSpaceGB = usedSpace / (1024.0f * 1024.0f * 1024.0f);
    float totalSpaceGB = totalSpace / (1024.0f * 1024.0f * 1024.0f);

    // Format the used and total space as a string
    snprintf(buffer, 64, "%.2f GB / %.2f GB", usedSpaceGB, totalSpaceGB);

    // Close the file system
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
        const std::pair<const char*, u32*> keys[] = {
            {"cpu_speedo_0", &cpuSpeedo0},
            {"cpu_speedo_2", &cpuSpeedo2},
            {"soc_speedo_0", &socSpeedo0},
            {"cpu_iddq", &cpuIDDQ},
            {"soc_iddq", &socIDDQ},
            {"gpu_iddq", &gpuIDDQ}
        };
        std::string value;
        for (const auto& key : keys) {
            value = parseValueFromIniSection(FUSE_DATA_INI_PATH, FUSE_STR, key.first);
            *key.second = value.empty() ? 0 : std::stoi(value);
        }
    }

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
        commandSuccess = false;
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
        commandSuccess = false;
        //LogLine("Error btmGetDeviceCondition: %u - %X\n", rc, rc);
    }
    
    if (R_SUCCEEDED(rc)) {
        //LogLine("Disconnecting controllers. Count: %u\n", g_connected_count);
        for (int i = 0; i != g_connected_count; ++i) {
            rc = btmHidDisconnect(g_addresses[i]);
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




void initializeTheme(const std::string& themeIniPath = THEME_CONFIG_INI_PATH) {
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

    file.close();

    // Assuming nacp.lang[0].name and nacp.display_version are null-terminated
    return {
        ResultSuccess,
        std::string(nacp.lang[0].name),
        std::string(nacp.display_version)
    };
}



void addHeader(auto& list, const std::string& headerText) {
    list->addItem(new tsl::elm::CategoryHeader(headerText));
}

void addBasicListItem(auto& list, const std::string& itemText) {
    list->addItem(new tsl::elm::ListItem(itemText));
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


std::vector<std::string> wrapText(const std::string& text, float maxWidth, const std::string& wrappingMode, bool useIndent, const std::string& indent, float indentWidth, size_t fontSize) {
    if (wrappingMode == "none" || (wrappingMode != "char" && wrappingMode != "word")) {
        return std::vector<std::string>{text};  // Return the entire text as a single line
    }

    std::vector<std::string> wrappedLines;
    std::string currentLine;
    std::string currentWord;
    bool firstLine = true;
    float currentMaxWidth = maxWidth;

    if (wrappingMode == "char") {
        size_t i = 0;
        while (i < text.size()) {
            currentLine += text[i];
            currentMaxWidth = firstLine ? maxWidth : maxWidth - indentWidth;  // Subtract indent width for subsequent lines

            if (tsl::gfx::calculateStringWidth(currentLine, fontSize, false) > currentMaxWidth) {
                wrappedLines.push_back(((firstLine && useIndent) || !useIndent) ? currentLine : indent + currentLine);  // Indent if not the first line
                currentLine.clear();  // Start a new line
                firstLine = false;
            }
            i++;
        }

        if (!currentLine.empty()) {
            wrappedLines.push_back(((firstLine && useIndent) || !useIndent) ? currentLine : indent + currentLine);  // Add the last line
        }
    } else if (wrappingMode == "word") {
        StringStream stream(text);
        while (stream >> currentWord) {
            if (tsl::gfx::calculateStringWidth(currentLine + currentWord, fontSize, false) > currentMaxWidth) {
                wrappedLines.push_back(((firstLine && useIndent) || !useIndent) ? currentLine : indent + currentLine);  // Add the current line
                currentLine.clear();  // Start a new line with the current word
                firstLine = false;
            }

            if (!currentLine.empty()) {
                currentLine += " ";  // Add a space between words
            }
            currentLine += currentWord;
        }

        if (!currentLine.empty()) {
            wrappedLines.push_back(((firstLine && useIndent) || !useIndent) ? currentLine : indent + currentLine);  // Add the last line
        }
    }

    return wrappedLines;
}



void drawTable(std::unique_ptr<tsl::elm::List>& list, std::vector<std::string>& sectionLines, std::vector<std::string>& infoLines,
               size_t columnOffset = 163, size_t startGap = 19, size_t endGap = 12, size_t newlineGap = 0,
               const std::string& tableSectionTextColor = DEFAULT_STR, const std::string& tableInfoTextColor = DEFAULT_STR, 
               const std::string& alignment = LEFT_STR, bool hideTableBackground = false, bool useHeaderIndent = false, 
               bool isScrollable = false, const std::string& wrappingMode = "none", bool useWrappedTextIndent = false) {

    const size_t lineHeight = 16;
    const size_t fontSize = 16;
    const size_t xMax = tsl::cfg::FramebufferWidth - 95;
    //const std::string indent = "    ";  // 4 spaces for indentation
    const std::string indent = "└ ";
    float indentWidth = tsl::gfx::calculateStringWidth(indent, fontSize, false);  // Calculate width of the indent

    auto getTextColor = [](const std::string& colorStr, auto defaultColor) {
        if (colorStr == "warning") return tsl::warningTextColor;
        if (colorStr == "text") return tsl::defaultTextColor;
        if (colorStr == "on_value") return tsl::onTextColor;
        if (colorStr == "off_value") return tsl::offTextColor;
        if (colorStr == "header") return tsl::headerTextColor;
        if (colorStr == "info") return tsl::infoTextColor;
        if (colorStr == "section") return tsl::sectionTextColor;
        return (colorStr == DEFAULT_STR) ? defaultColor : tsl::RGB888(colorStr);
    };

    auto alternateSectionTextColor = getTextColor(tableSectionTextColor, tsl::sectionTextColor);
    auto alternateInfoTextColor = getTextColor(tableInfoTextColor, tsl::infoTextColor);


    // Vectors for recalculating offsets and expanding lines
    std::vector<std::string> expandedSectionLines;
    std::vector<std::string> expandedInfoLines;
    std::vector<s32> yOffsets;
    std::vector<int> infoXOffsets;

    // Preprocess and wrap section lines, ensuring infoLines align correctly
    size_t currentY = startGap;

    std::vector<std::string> wrappedLines;
    float infoTextWidth;

    for (size_t i = 0; i < sectionLines.size(); ++i) {
        // Wrap the section lines before passing them to the drawer, and indent if required
        wrappedLines = wrapText(sectionLines[i], xMax - 12 - 4, wrappingMode, useWrappedTextIndent, indent, indentWidth, fontSize);
        for (const auto& wrappedLine : wrappedLines) {
            expandedSectionLines.push_back(wrappedLine);
            expandedInfoLines.push_back(i < infoLines.size() ? infoLines[i] : "");  // Replicate the info line for the first wrapped line

            // Calculate X offsets for the info line based on alignment
            if (i < infoLines.size()) {
                const std::string& infoText = expandedInfoLines.back();
                infoTextWidth = tsl::gfx::calculateStringWidth(infoText, fontSize, false);

                if (alignment == LEFT_STR) {
                    infoXOffsets.push_back(static_cast<int>(columnOffset));
                } else if (alignment == RIGHT_STR) {
                    infoXOffsets.push_back(static_cast<int>(xMax - infoTextWidth + (columnOffset - 160 + 1)));
                } else { // CENTER_STR
                    infoXOffsets.push_back(static_cast<int>(columnOffset + (xMax - infoTextWidth) / 2));
                }
            }

            yOffsets.push_back(currentY);  // Set the offset for each line
            currentY += lineHeight + newlineGap;  // Increment Y position for the next line
        }
    }

    // Compute total height based on the number of expanded lines
    size_t totalHeight = lineHeight * expandedSectionLines.size() + newlineGap * (expandedSectionLines.size() - 1) + endGap;

    // Add the TableDrawer with modified sectionLines and infoLines (now they match in size)
    list->addItem(new tsl::elm::TableDrawer([=](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) mutable {
        if (useHeaderIndent) {
            renderer->drawRect(x - 2, y + 2, 4, 22, renderer->a(tsl::headerSeparatorColor));
        }
        std::string infoText;

        // Draw each section and info line
        for (size_t i = 0; i < expandedSectionLines.size(); ++i) {
            // Draw the section line
            renderer->drawString(expandedSectionLines[i], false, x + 12, y + yOffsets[i], fontSize, renderer->a(alternateSectionTextColor));

            // Draw the corresponding info line
            if (i < expandedInfoLines.size()) {
                infoText = (expandedInfoLines[i].find(NULL_STR) != std::string::npos) ? UNAVAILABLE_SELECTION : expandedInfoLines[i];
                renderer->drawString(infoText, false, x + infoXOffsets[i], y + yOffsets[i], fontSize, renderer->a(alternateInfoTextColor));
            }
        }
    }, hideTableBackground, endGap, isScrollable), totalHeight);
}





void applyPlaceholderReplacements(std::vector<std::string>& cmd, const std::string& hexPath, const std::string& iniPath, const std::string& listString, const std::string& listPath, const std::string& jsonString, const std::string& jsonPath);

std::string getFirstSectionText(const std::vector<std::vector<std::string>>& tableData, const std::string& packagePath) {
    std::string message;
    std::string listFileSourcePath;
    std::string hexPath, iniPath, listString, listPath, jsonString, jsonPath;
    
    bool inEristaSection = false;
    bool inMarikoSection = false;
    
    for (const auto& commands : tableData) {
        auto cmd = commands;  // Make a copy if you need to modify it

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
                std::vector<std::string> lines = readListFromFile(listFileSourcePath);

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




void addTable(std::unique_ptr<tsl::elm::List>& list, std::vector<std::vector<std::string>>& tableData,
    const std::string& packagePath, const size_t& columnOffset=163, const size_t& tableStartGap=19, const size_t& tableEndGap=12, const size_t& tableSpacing=0,
    const std::string& tableSectionTextColor=DEFAULT_STR, const std::string& tableInfoTextColor=DEFAULT_STR, const std::string& tableAlignment=RIGHT_STR,
    const bool& hideTableBackground = false, const bool& useHeaderIndent = false, const bool& isScrollable = false, const std::string& wrappingMode="none", const bool& useWrappedTextIndent = false) {

    std::string message;

    //std::string sectionString, infoString;
    std::vector<std::string> sectionLines, infoLines;

    std::string listFileSourcePath;

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
            disableLogging = true;
            logFilePath = defaultLogFilePath;
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

            applyPlaceholderReplacements(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

            if (interpreterLogging) {
                disableLogging = false;
                message = "Reading line:";
                for (const std::string& token : cmd)
                    message += " " + token;
                logMessage(message);
            }

            const size_t cmdSize = cmd.size();

            if (commandName == "list_file_source" && listFileSourcePath.empty()) {
                listFileSourcePath = cmd[1];
                preprocessPath(listFileSourcePath, packagePath);
                
                // Read lines from the file
                std::vector<std::string> lines = readListFromFile(listFileSourcePath);
            
                // Append lines to sectionLines and add empty lines to infoLines
                for (const auto& line : lines) {
                    sectionLines.push_back(line);
                    infoLines.push_back(""); // Add an empty string for each section line
                }
            }


            else if (commandName == LIST_STR) {
                if (cmdSize >= 2) {
                    listString = cmd[1];
                    removeQuotes(listString);
                }
            } else if (commandName == LIST_FILE_STR) {
                if (cmdSize >= 2) {
                    listPath = cmd[1];
                    preprocessPath(listPath, packagePath);
                }
            } else if (commandName == JSON_STR) {
                if (cmdSize >= 2) {
                    jsonString = cmd[1];
                }
            } else if (commandName == JSON_FILE_STR) {
                if (cmdSize >= 2) {
                    jsonPath = cmd[1];
                    preprocessPath(jsonPath, packagePath);
                }
            } else if (commandName == INI_FILE_STR) {
                if (cmdSize >= 2) {
                    iniPath = cmd[1];
                    preprocessPath(iniPath, packagePath);
                }
            } else if (commandName == HEX_FILE_STR) {
                if (cmdSize >= 2) {
                    hexPath = cmd[1];
                    preprocessPath(hexPath, packagePath);
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

    drawTable(list, sectionLines, infoLines, columnOffset, tableStartGap, tableEndGap, tableSpacing, tableSectionTextColor, tableInfoTextColor, tableAlignment, hideTableBackground, useHeaderIndent, isScrollable, wrappingMode, useWrappedTextIndent);
}


void addHelpInfo(std::unique_ptr<tsl::elm::List>& list) {
    // Add a section break with small text to indicate the "Commands" section
    addHeader(list, USER_GUIDE);

    // Adjust the horizontal offset as needed
    int xOffset = std::stoi(USERGUIDE_OFFSET);

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

    // Draw the table with the defined lines
    drawTable(list, sectionLines, infoLines, xOffset, 19, 12, 4);
    //drawTable(list, sectionLines, infoLines, xOffset, 19, 12, 4, DEFAULT_STR, DEFAULT_STR, LEFT_STR, false, false, true, "none", false);
}



void addPackageInfo(std::unique_ptr<tsl::elm::List>& list, auto& packageHeader, std::string type = PACKAGE_STR) {
    // Add a section break with small text to indicate the "Commands" section
    addHeader(list, (type == PACKAGE_STR ? PACKAGE_INFO : OVERLAY_INFO));

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

    // Drawing the table with section lines and info lines
    //drawTable(list, sectionLines, infoLines, xOffset, 20, 12, 3);
    drawTable(list, sectionLines, infoLines, xOffset, 19, 12, 3, DEFAULT_STR, DEFAULT_STR, LEFT_STR, false, false, true);
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
    static const std::vector<std::pair<const char*, size_t>> protectedFolders = {
        {"sdmc:/Nintendo/", 14},
        {"sdmc:/emuMMC/", 13},
        {"sdmc:/emuMMC/RAW1/", 17},
        {"sdmc:/atmosphere/", 15},
        {"sdmc:/bootloader/", 15},
        {"sdmc:/switch/", 12},
        {"sdmc:/config/", 12},
        {"sdmc:/", 6}
    };

    static const std::vector<const char*> ultraProtectedFolders = {
        "sdmc:/Nintendo/Contents/",
        "sdmc:/Nintendo/save/",
        "sdmc:/emuMMC/RAW1/Nintendo/Contents/",
        "sdmc:/emuMMC/RAW1/Nintendo/save/"
    };

    static const std::vector<const char*> dangerousPatterns = {
        "..",    // Attempts to traverse to parent directories
        "~",     // Represents user's home directory, can be dangerous if misused
        "*",     // Wildcard in general
        "*/"     // Wildcard in general
    };

    // Check ultra-protected folders first
    for (const auto& folder : ultraProtectedFolders) {
        if (patternPath.compare(0, std::strlen(folder), folder) == 0) {
            return true; // Path is an ultra-protected folder
        }
    }

    // Check protected folders
    for (const auto& [folder, folderLen] : protectedFolders) {
        if (patternPath.compare(0, folderLen, folder) == 0) {
            std::string relativePath = patternPath.substr(folderLen);

            // Check for dangerous patterns in the relative path
            for (const auto& pattern : dangerousPatterns) {
                if (relativePath.find(pattern) != std::string::npos) {
                    return true; // Path contains a dangerous pattern
                }
            }

            // Check for wildcard patterns that could affect ultra-protected folders
            for (const auto& ultraFolder : ultraProtectedFolders) {
                if (relativePath.find(ultraFolder + folderLen) == 0) {
                    return true; // Path with wildcard could affect an ultra-protected folder
                }
            }

            break; // If it matches a protected folder, no need to check others
        }
    }

    // Check dangerous patterns in general
    for (const auto& pattern : dangerousPatterns) {
        if (patternPath.find(pattern) != std::string::npos) {
            return true; // Path contains a dangerous pattern
        }
    }

    // Check wildcard at root level
    size_t rootPos = patternPath.find(":/");
    if (rootPos != std::string::npos && patternPath.find('*', rootPos) != std::string::npos) {
        return true; // Root path contains a wildcard
    }

    return false; // No dangerous combinations found
}






// Function to populate selectedItemsListOff from a JSON array based on a key
void populateSelectedItemsList(const std::string& sourceType, const std::string& jsonStringOrPath, const std::string& jsonKey, std::vector<std::string>& selectedItemsList) {
    // Check for empty JSON source strings
    if (jsonStringOrPath.empty()) {
        return;
    }

    // Use a unique_ptr to manage JSON object with appropriate deleter
    std::unique_ptr<json_t, void(*)(json_t*)> jsonData(nullptr, json_decref);

    // Convert JSON string or read from file based on the source type
    if (sourceType == JSON_STR) {
        jsonData.reset(stringToJson(jsonStringOrPath));
    } else if (sourceType == JSON_FILE_STR) {
        jsonData.reset(readJsonFromFile(jsonStringOrPath));
    }

    // Early return if jsonData is null or not an array
    if (!jsonData || !json_is_array(jsonData.get())) {
        return;
    }

    // Prepare for efficient insertion
    json_t* jsonArray = jsonData.get();
    const size_t arraySize = json_array_size(jsonArray);
    selectedItemsList.reserve(arraySize);

    // Store the key as a const char* to avoid repeated c_str() calls
    const char* jsonKeyCStr = jsonKey.c_str();

    // Iterate over the JSON array
    for (size_t i = 0; i < arraySize; ++i) {
        auto* item = json_array_get(jsonArray, i);
        if (json_is_object(item)) {
            auto* keyValue = json_object_get(item, jsonKeyCStr);
            if (json_is_string(keyValue)) {
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
inline void applyPlaceholderReplacement(std::string& input, const std::string& placeholder, const std::string& replacement) {
    size_t pos = input.find(placeholder);
    if (pos == std::string::npos) {
        return;  // Returns original string directly if no placeholder is found
    }
    //std::string result = input;
    input.replace(pos, placeholder.length(), replacement);
    //return result;
}




void applyReplaceIniPlaceholder(std::string& arg, const std::string& commandName, const std::string& iniPath) {

    const std::string searchString = "{" + commandName + "(";
    size_t startPos = arg.find(searchString);
    if (startPos == std::string::npos) {
        return;
    }

    size_t endPos = arg.find(")}", startPos);
    if (endPos == std::string::npos || endPos <= startPos) {
        return;
    }

    //std::string replacement = arg;  // Copy arg because we need to modify it

    std::string placeholderContent = arg.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
    trim(placeholderContent);

    size_t commaPos = placeholderContent.find(',');
    if (commaPos != std::string::npos) {
        std::string iniSection = placeholderContent.substr(0, commaPos);
        trim(iniSection);
        removeQuotes(iniSection);
        std::string iniKey = placeholderContent.substr(commaPos + 1);
        trim(iniKey);
        removeQuotes(iniKey);

        std::string parsedResult = parseValueFromIniSection(iniPath, iniSection, iniKey);
        // Replace the placeholder with the parsed result and keep the remaining string intact
        arg = arg.substr(0, startPos) + parsedResult + arg.substr(endPos + 2);
    } else {
        // Check if the content is an integer
        if (std::all_of(placeholderContent.begin(), placeholderContent.end(), ::isdigit)) {
            size_t entryIndex = std::stoi(placeholderContent);

            // Return list of section names and use entryIndex to get the specific entry
            std::vector<std::string> sectionNames = parseSectionsFromIni(iniPath);
            if (entryIndex < sectionNames.size()) {
                std::string sectionName = sectionNames[entryIndex];
                arg = arg.substr(0, startPos) + sectionName + arg.substr(endPos + 2);
            } else {
                // Handle the case where entryIndex is out of range
                arg = arg.substr(0, startPos) + NULL_STR + arg.substr(endPos + 2);
            }
        } else {
            // Handle the case where the placeholder content is not a valid index
            arg = arg.substr(0, startPos) + NULL_STR + arg.substr(endPos + 2);
        }
    }



    //return replacement;
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
void replacePlaceholdersInArg(std::string& source, const std::unordered_map<std::string, std::string>& replacements) {
    for (const auto& [placeholder, replacement] : replacements) {
        size_t pos = 0;
        while ((pos = source.find(placeholder, pos)) != std::string::npos) {
            source.replace(pos, placeholder.length(), replacement);
            pos += replacement.length();  // Move past the replacement to avoid infinite loop
        }
    }
}


// Optimized getSourceReplacement function
std::vector<std::vector<std::string>> getSourceReplacement(const std::vector<std::vector<std::string>>& commands,
    const std::string& entry, size_t entryIndex, const std::string& packagePath = "") {

    //std::string memoryVendor = splitStringAtIndex(memoryType, "_", 0);
    //const std::string memoryModel = splitStringAtIndex(memoryType, "_", 1);

    bool inEristaSection = false;
    bool inMarikoSection = false;
    
    std::vector<std::vector<std::string>> modifiedCommands;
    std::string listString, listPath, jsonString, jsonPath, iniPath;
    bool usingFileSource = false;

    std::string fileName = getNameFromPath(entry);
    if (!isDirectory(entry))
        dropExtension(fileName);

    std::vector<std::string> modifiedCmd;
    std::string commandName;
    std::string modifiedArg;
    size_t startPos, endPos;
    std::string replacement;

    std::string path;

    for (const auto& cmd : commands) {
        if (cmd.empty())
            continue;
        
        modifiedCmd.clear();
        modifiedCmd.reserve(cmd.size());
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
        
        if ((inEristaSection && usingErista) || (inMarikoSection && usingMariko) || (!inEristaSection && !inMarikoSection)) {
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
                
                replaceAllPlaceholders(modifiedArg, "{file_source}", entry);
                replaceAllPlaceholders(modifiedArg, "{file_name}", fileName);
                path = getParentDirNameFromPath(entry);
                removeQuotes(path);
                replaceAllPlaceholders(modifiedArg, "{folder_name}", path);

                if (modifiedArg.find("{list_source(") != std::string::npos) {
                    //modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    applyPlaceholderReplacement(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{list_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = stringToList(listString)[entryIndex];
                        replacement = replacement.empty() ? NULL_STR : replacement;
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                if (modifiedArg.find("{list_file_source(") != std::string::npos) {
                    //modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    applyPlaceholderReplacement(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{list_file_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = getEntryFromListFile(listPath, entryIndex);
                        replacement = replacement.empty() ? NULL_STR : replacement;
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                if (modifiedArg.find("{ini_file_source(") != std::string::npos) {
                    //modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    applyPlaceholderReplacement(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{ini_file_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        //replacement = applyReplaceIniPlaceholder(modifiedArg, "ini_file_source", iniPath);
                        applyReplaceIniPlaceholder(modifiedArg, "ini_file_source", iniPath);
                        modifiedArg = modifiedArg.empty() ? NULL_STR : modifiedArg;
                        modifiedArg.replace(startPos, endPos - startPos + 2, modifiedArg);
                    }
                }

                if (modifiedArg.find("{json_source(") != std::string::npos) {
                    //modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    applyPlaceholderReplacement(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{json_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = replaceJsonPlaceholder(modifiedArg.substr(startPos, endPos - startPos + 2), "json_source", jsonString);
                        replacement = replacement.empty() ? NULL_STR : replacement;
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }

                if (modifiedArg.find("{json_file_source(") != std::string::npos) {
                    //modifiedArg = replacePlaceholder(modifiedArg, "*", std::to_string(entryIndex));
                    applyPlaceholderReplacement(modifiedArg, "*", std::to_string(entryIndex));
                    startPos = modifiedArg.find("{json_file_source(");
                    endPos = modifiedArg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = replaceJsonPlaceholder(modifiedArg.substr(startPos, endPos - startPos + 2), "json_file_source", jsonPath);
                        replacement = replacement.empty() ? NULL_STR : replacement;
                        modifiedArg.replace(startPos, endPos - startPos + 2, replacement);
                    }
                }
                
                modifiedCmd.push_back(std::move(modifiedArg));
            }

            modifiedCommands.emplace_back(std::move(modifiedCmd));
        }
    }

    if (usingFileSource) {
        modifiedCommands.insert(modifiedCommands.begin(), {"file_name", fileName});
    }

    return modifiedCommands;
}


std::string getCurrentTimestamp(const std::string& format) {
    auto now_time_t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    char buffer[30]; // Adjust size based on expected max format length
    if (std::strftime(buffer, sizeof(buffer), format.c_str(), std::localtime(&now_time_t))) {
        return std::string(buffer);
    } else {
        return ""; // or handle the error as needed
    }
}


// Helper function to skip spaces
void skipSpaces(const std::string& expression, size_t& pos) {
    while (pos < expression.length() && std::isspace(expression[pos])) {
        ++pos;
    }
}

// Helper function to parse a number or a nested expression in parentheses
float parseExpression(const std::string& expression, size_t& pos, bool& valid);

float parseNumber(const std::string& expression, size_t& pos, bool& valid) {
    skipSpaces(expression, pos);

    // Check if the expression starts with a '(' indicating a nested expression
    if (expression[pos] == '(') {
        ++pos;  // Skip the '('
        float result = parseExpression(expression, pos, valid);
        if (expression[pos] == ')') {
            ++pos;  // Skip the ')'
        } else {
            valid = false;  // Unmatched parentheses
        }
        return result;
    }

    // Parse a number
    float result = 0.0f;
    bool hasDecimal = false;
    float decimalPlace = 0.1f;
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
float parseExpression(const std::string& expression, size_t& pos, bool& valid) {
    skipSpaces(expression, pos);

    float result = parseNumber(expression, pos, valid);
    if (!valid) return 0;

    while (pos < expression.length()) {
        skipSpaces(expression, pos);
        if (pos >= expression.length()) break;

        char op = expression[pos++];
        skipSpaces(expression, pos);

        float operand = parseNumber(expression, pos, valid);
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
                if (std::fmod(result, 1.0f) != 0.0f || std::fmod(operand, 1.0f) != 0.0f) {
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
float evaluateExpression(const std::string& expression, bool& valid) {
    size_t pos = 0;
    return parseExpression(expression, pos, valid);
}

// Handle Math Placeholder with Parentheses, Modulus, and Optional Integer Support
std::string handleMath(const std::string& placeholder) {
    size_t startPos = placeholder.find('(') + 1;
    size_t endPos = placeholder.find(')');

    // Return NULL_STR if format is invalid
    if (startPos == std::string::npos || endPos == std::string::npos || startPos >= endPos) {
        return NULL_STR;
    }

    std::string mathExpression = placeholder.substr(startPos, endPos - startPos);

    // Check for an optional second parameter, like 'true' to force integer output
    size_t commaPos = mathExpression.find(',');
    bool forceInteger = false;

    if (commaPos != std::string::npos) {
        std::string secondParam = mathExpression.substr(commaPos + 1);
        trim(secondParam);  // Optionally, remove spaces from the second param
        forceInteger = (secondParam == TRUE_STR);
        mathExpression = mathExpression.substr(0, commaPos);  // Remove the second param for evaluation
    }

    // Remove unnecessary spaces in the expression
    mathExpression.erase(remove_if(mathExpression.begin(), mathExpression.end(), ::isspace), mathExpression.end());

    // Evaluate the math expression
    bool valid = false;
    float result = evaluateExpression(mathExpression, valid);

    // Return NULL_STR if the expression was invalid
    if (!valid) {
        return NULL_STR;
    }

    // If forceInteger is true, or the result is already an integer, return it as an integer
    if (forceInteger || std::fmod(result, 1.0f) == 0.0f) {
        return std::to_string(static_cast<int>(result));
    }

    // Otherwise, return the result as a floating-point string
    return std::to_string(result);
}

std::string handleLength(const std::string& placeholder) {
    size_t startPos = placeholder.find('(') + 1;
    size_t endPos = placeholder.find(')');
    
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        std::string str = placeholder.substr(startPos, endPos - startPos);
        trim(str);  // Remove any extra spaces
        removeQuotes(str);  // If your strings are wrapped in quotes, remove them
        
        // Return the length of the string as a string
        return std::to_string(str.length());
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

void replacePlaceholdersRecursively(std::string& arg, const std::vector<std::pair<std::string, std::function<std::string(const std::string&)>>>& placeholders) {
    std::string lastArg;
    
    // Continue replacing until no more placeholders are found
    bool placeholdersRemaining = true;
    while (placeholdersRemaining) {
        placeholdersRemaining = false; // Reset the flag at the beginning of each loop
        
        for (const auto& [placeholder, replacer] : placeholders) {
            size_t startPos, endPos;
            size_t nestedStartPos, nextStartPos, nextEndPos;

            while ((startPos = arg.find(placeholder)) != std::string::npos) {
                nestedStartPos = startPos;
                while (true) {
                    nextStartPos = arg.find(placeholder, nestedStartPos + 1);
                    nextEndPos = arg.find(")}", nestedStartPos);
                    if (nextStartPos != std::string::npos && nextStartPos < nextEndPos) {
                        nestedStartPos = nextStartPos;
                    } else {
                        endPos = nextEndPos;
                        break;
                    }
                }

                if (endPos == std::string::npos || endPos <= startPos) break;

                std::string placeholderContent = arg.substr(startPos, endPos - startPos + 2);
                std::string replacement = replacer(placeholderContent);

                if (replacement.empty()) {
                    replacement = NULL_STR;
                }

                arg.replace(startPos, endPos - startPos + 2, replacement);
                placeholdersRemaining = true; // Since we replaced something, we continue processing

                // To prevent infinite loops, if no change is made, break
                if (arg == lastArg) {
                    arg.replace(startPos, endPos - startPos + 2, NULL_STR);
                    break;
                }

                lastArg = arg;
            }
        }
    }
}




void applyPlaceholderReplacements(std::vector<std::string>& cmd, const std::string& hexPath, const std::string& iniPath, const std::string& listString, const std::string& listPath, const std::string& jsonString, const std::string& jsonPath) {
    std::vector<std::pair<std::string, std::function<std::string(const std::string&)>>> placeholders = {
        {"{hex_file(", [&](const std::string& placeholder) { return replaceHexPlaceholder(placeholder, hexPath); }},
        {"{ini_file(", [&](const std::string& placeholder) { 
            std::string result = placeholder;
            applyReplaceIniPlaceholder(result, INI_FILE_STR, iniPath); 
            return result; 
        }},
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
            removeQuotes(format);
            return getCurrentTimestamp(format);
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
        {"{random(", [&](const std::string& placeholder) {
            // Ensure the random seed is initialized
            std::srand(std::time(0));
            
            size_t startPos = placeholder.find('(') + 1;
            size_t endPos = placeholder.find(')');
            std::string parameters = placeholder.substr(startPos, endPos - startPos);
            size_t commaPos = parameters.find(',');
            
            if (commaPos != std::string::npos) {
                int lowValue = std::stoi(parameters.substr(0, commaPos));
                int highValue = std::stoi(parameters.substr(commaPos + 1));
                
                // Generate a random number in the range [lowValue, highValue]
                int randomValue = lowValue + rand() % (highValue - lowValue + 1);
                
                return std::to_string(randomValue);  // Return the random value as a string
            }
            return placeholder;
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
        }},
        {"{split(", [&](const std::string& placeholder) {
            size_t startPos = placeholder.find('(') + 1;
            size_t endPos = placeholder.find(')');
            std::string parameters = placeholder.substr(startPos, endPos - startPos);
            
            size_t firstCommaPos = parameters.find(',');
            size_t lastCommaPos = parameters.find_last_of(',');
        
            if (firstCommaPos != std::string::npos && lastCommaPos != std::string::npos && firstCommaPos != lastCommaPos) {
                std::string str = parameters.substr(0, firstCommaPos);
                std::string delimiter = parameters.substr(firstCommaPos + 1, lastCommaPos - firstCommaPos - 1);
                size_t index = std::stoi(parameters.substr(lastCommaPos + 1));
                trim(str);
                removeQuotes(str);
                trim(delimiter);
                removeQuotes(delimiter);

                std::string result = splitStringAtIndex(str, delimiter, index);
                if (result.empty()) {
                    return str;
                } else {
                    return result;
                }
            }
            return placeholder;
        }},
        {"{math(", [&](const std::string& placeholder) { return handleMath(placeholder); }},
        {"{length(", [&](const std::string& placeholder) { return handleLength(placeholder); }},
    };

    // Create a map with all non-button/arrow placeholders and their replacements
    std::unordered_map<std::string, std::string> generalPlaceholders = {
        {"{ram_vendor}", memoryVendor},
        {"{ram_model}", memoryModel},
        {"{ams_version}", amsVersion},
        {"{hos_version}", hosVersion},
        {"{cpu_speedo}", std::to_string(cpuSpeedo0)},
        {"{cpu_iddq}", std::to_string(cpuIDDQ)},
        {"{gpu_speedo}", std::to_string(cpuSpeedo2)},
        {"{gpu_iddq}", std::to_string(gpuIDDQ)},
        {"{soc_speedo}", std::to_string(socSpeedo0)},
        {"{soc_iddq}", std::to_string(socIDDQ)},
        {"{title_id}", getTitleIdAsString()}
    };

    // Iterate through each command and replace placeholders in one pass
    for (auto& arg : cmd) {
        // Replace general placeholders
        replacePlaceholdersInArg(arg, generalPlaceholders);

        // Replace button/arrow placeholders from the global map
        replacePlaceholdersInArg(arg, symbolPlaceholders);

        // Additionally replace placeholders from your custom map
        //for (const auto& [placeholder, replacer] : placeholders) {
        //    replacePlaceholders(arg, placeholder, replacer);
        //}

        // Resolve nested placeholders
        replacePlaceholdersRecursively(arg, placeholders);
    }
}



// forward declarartion
void processCommand(const std::vector<std::string>& cmd, const std::string& packagePath, const std::string& selectedCommand);



/**
 * @brief Apply placeholder replacements to a list of commands and handle control flow commands.
 *
 * This function iterates over a list of commands, where each command is a vector of strings,
 * and applies the `applyPlaceholderReplacements` function to handle the placeholders.
 * It also handles control flow commands like "erista:" and "mariko:".
 *
 * @param commands A list of commands, where each command is represented as a vector of strings.
 * @param packagePath The path to the package (optional).
 * @param selectedCommand A specific command to execute (optional).
 * @return Returns true if the placeholders were successfully applied to the commands, false otherwise.
 */
bool applyPlaceholderReplacementsToCommands(std::vector<std::vector<std::string>>& commands, const std::string& packagePath = "", const std::string& selectedCommand = "") {

    std::string listString, listPath, jsonString, jsonPath, hexPath, iniPath;

    bool inEristaSection = false;
    bool inMarikoSection = false;
    bool eraseAtEnd ;

    // Process commands with control flow and apply replacements
    for (auto it = commands.begin(); it != commands.end();) {
        
        auto& cmd = *it; // Get the current command

        if (cmd.empty()) {
            it = commands.erase(it); // Remove empty command
            continue;
        }

        const std::string& commandName = cmd[0];

        // Handle control flow commands like "erista:" and "mariko:"
        if (commandName == "erista:") {
            inEristaSection = true;
            inMarikoSection = false;
            it = commands.erase(it); // Remove processed command
            continue;
        } else if (commandName == "mariko:") {
            inEristaSection = false;
            inMarikoSection = true;
            it = commands.erase(it); // Remove processed command
            continue;
        } else if (commandName.front() == ';' ||
            (commandName.size() >= 7 && commandName.substr(commandName.size() - 7) == "_source") ||
            commandName == "logging") { // remove stuff that isn't executable
            it = commands.erase(it); // Remove processed command
            continue;
        }

        // If we're in a section not relevant to the current hardware, skip the command
        if ((inEristaSection && !usingErista) || (inMarikoSection && !usingMariko)) {
            ++it; // Skip this command
            continue;
        }

        // Apply placeholder replacements in the current command
        applyPlaceholderReplacements(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

        // Handle special commands like LIST_STR, LIST_FILE_STR, etc.
        size_t cmdSize = cmd.size();

        if (commandName == LIST_STR && cmdSize >= 2) {
            listString = std::string(cmd[1]);  // Make a copy of cmd[1] into listString
            removeQuotes(listString);
            eraseAtEnd = true;
        } else if (commandName == LIST_FILE_STR && cmdSize >= 2) {
            listPath = std::string(cmd[1]);  // Make a copy of cmd[1] into listPath
            preprocessPath(listPath, packagePath);
            eraseAtEnd = true;
        } else if (commandName == JSON_STR && cmdSize >= 2) {
            jsonString = std::string(cmd[1]);  // Make a copy of cmd[1] into jsonString
            eraseAtEnd = true;
        } else if (commandName == JSON_FILE_STR && cmdSize >= 2) {
            jsonPath = std::string(cmd[1]);  // Make a copy of cmd[1] into jsonPath
            preprocessPath(jsonPath, packagePath);
            eraseAtEnd = true;
        } else if (commandName == INI_FILE_STR && cmdSize >= 2) {
            iniPath = std::string(cmd[1]);  // Make a copy of cmd[1] into iniPath
            preprocessPath(iniPath, packagePath);
            eraseAtEnd = true;
        } else if (commandName == HEX_FILE_STR && cmdSize >= 2) {
            hexPath = std::string(cmd[1]);  // Make a copy of cmd[1] into hexPath
            preprocessPath(hexPath, packagePath);
            eraseAtEnd = true;
        } else if (commandName == "filter" || commandName == "file_name") {
            eraseAtEnd = true;
        } else {
            eraseAtEnd = false;
        }

        // Now, handle adding quotes only if needed
        for (size_t i = 1; i < cmd.size(); ++i) {
            std::string& argument = cmd[i];

            // If the argument is exactly '', skip processing (preserve the empty quotes)
            if (argument == "") {
                argument = "''";
                //continue;  // Do nothing, preserve as is
            }

            // If the argument already has quotes, skip it
            if ((argument.front() == '"' && argument.back() == '"') ||
                (argument.front() == '\'' && argument.back() == '\'')) {
                continue; // Already quoted, skip
            }

            // If the argument contains no spaces, do not add quotes
            if (argument.find(' ') == std::string::npos) {
                continue;  // No spaces, so no need for quotes
            }

            // If the argument contains a single quote, wrap it in double quotes
            if (argument.find('\'') != std::string::npos) {
                argument = "\"" + argument + "\"";  // Wrap in double quotes
            }
            // If the argument contains a double quote, wrap it in single quotes
            else if (argument.find('"') != std::string::npos) {
                argument = "\'" + argument + "\'";  // Wrap in single quotes
            }
            // If the argument contains spaces but no quotes, wrap it in single quotes by default
            else {
                argument = "\'" + argument + "\'";
            }
        }

        // If eraseAtEnd is true, erase the current command
        if (eraseAtEnd) {
            it = commands.erase(it);  // Erase and update iterator
        } else {
            ++it;  // Move to the next command
        }
    }

    return true;  // Indicating that placeholder replacements have been applied successfully
}




/**
 * @brief Interpret and execute a list of commands.
 *
 * This function interprets and executes a list of commands based on their names and arguments.
 *
 * @param commands A list of commands, where each command is represented as a vector of strings.
 */
bool interpretAndExecuteCommands(std::vector<std::vector<std::string>>&& commands, const std::string& packagePath="", const std::string& selectedCommand="") {
    
    if (!packagePath.empty()) {
        disableLogging = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_LOGGING_STR) == TRUE_STR);
        logFilePath = packagePath + "log.txt";
    }

    // Load key-value pairs from the "BUFFERS" section of the INI file
    auto bufferSection = getKeyValuePairsFromSection(ULTRAHAND_CONFIG_INI_PATH, BUFFERS);
    
    if (!bufferSection.empty()) {
        // Directly update buffer sizes without a map
        std::string section;
    
        section = "copy_buffer_size";
        if (bufferSection.count(section) > 0) {
            COPY_BUFFER_SIZE = std::stoi(bufferSection[section]);
        }
    
        section = "unzip_buffer_size";
        if (bufferSection.count(section) > 0) {
            UNZIP_BUFFER_SIZE = std::stoi(bufferSection[section]);
        }
    
        section = "download_buffer_size";
        if (bufferSection.count(section) > 0) {
            DOWNLOAD_BUFFER_SIZE = std::stoi(bufferSection[section]);
        }
    
        section = "hex_buffer_size";
        if (bufferSection.count(section) > 0) {
            HEX_BUFFER_SIZE = std::stoi(bufferSection[section]);
        }
    }

    std::string message;

    bool inEristaSection = false;
    bool inMarikoSection = false;
    bool inTrySection = false;
    std::string listString, listPath, jsonString, jsonPath, hexPath, iniPath, lastArg;

    //size_t startPos, endPos, listIndex;
    std::string replacement;

    // Overwrite globals
    commandSuccess = true;
    refreshPage = false;
    refreshPackage = false;
    interpreterLogging = false;

    size_t cmdSize;

    while (!commands.empty()) {

        auto& cmd = commands.front(); // Get the first command for processing

        if (abortCommand.load(std::memory_order_acquire)) {
            abortCommand.store(false, std::memory_order_release);
            commandSuccess = false;
            disableLogging = true;
            logFilePath = defaultLogFilePath;
            return commandSuccess;
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

                applyPlaceholderReplacements(cmd, hexPath, iniPath, listString, listPath, jsonString, jsonPath);

                if (interpreterLogging) {
                    disableLogging = false;
                    message = "Executing command: ";
                    for (const std::string& token : cmd)
                        message += token + " ";
                    logMessage(message);
                }

                cmdSize = cmd.size();

                if (commandName == LIST_STR) {
                    if (cmdSize >= 2) {
                        listString = cmd[1];
                        removeQuotes(listString);
                    }
                } else if (commandName == LIST_FILE_STR) {
                    if (cmdSize >= 2) {
                        listPath = cmd[1];
                        preprocessPath(listPath, packagePath);
                    }
                } else if (commandName == JSON_STR) {
                    if (cmdSize >= 2) {
                        jsonString = cmd[1];
                    }
                } else if (commandName == JSON_FILE_STR) {
                    if (cmdSize >= 2) {
                        jsonPath = cmd[1];
                        preprocessPath(jsonPath, packagePath);
                    }
                } else if (commandName == INI_FILE_STR) {
                    if (cmdSize >= 2) {
                        iniPath = cmd[1];
                        preprocessPath(iniPath, packagePath);
                    }
                } else if (commandName == HEX_FILE_STR) {
                    if (cmdSize >= 2) {
                        hexPath = cmd[1];
                        preprocessPath(hexPath, packagePath);
                    }
                } else {
                    processCommand(cmd, packagePath, selectedCommand);
                }
            }
        }

        commands.erase(commands.begin()); // Remove processed command
    }
    disableLogging = true;
    logFilePath = defaultLogFilePath;

    return commandSuccess;
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
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath);
    long long totalBytesCopied, totalSize;

    if (!sourceListPath.empty() && !destinationListPath.empty()) {
        std::vector<std::string> sourceFilesList = readListFromFile(sourceListPath);
        std::vector<std::string> destinationFilesList = readListFromFile(destinationListPath);

        std::unordered_set<std::string> filterSet;
        if (!filterListPath.empty())
            filterSet = readSetFromFile(filterListPath);

        
        for (size_t i = 0; i < sourceFilesList.size(); ++i) {
            sourcePath = sourceFilesList[i];
            preprocessPath(sourcePath, packagePath);
            destinationPath = destinationFilesList[i];
            preprocessPath(destinationPath, packagePath);
            if (filterListPath.empty() || (!filterListPath.empty() && filterSet.find(sourcePath) == filterSet.end())) {
                totalBytesCopied = 0;
                totalSize = getTotalSize(sourcePath);  // Ensure this is calculated if needed.
                copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
            }
        }
    } else {
        // Ensure source and destination paths are set
        if (sourcePath.empty() || destinationPath.empty()) {
            logMessage("Source and destination paths must be specified.");
        } else {
            // Perform the copy operation
            if (!isFileOrDirectory(sourcePath)) {
                logMessage("Source file or directory doesn't exist: " + sourcePath);
            } else {
                if (sourcePath.find('*') != std::string::npos) {
                    copyFileOrDirectoryByPattern(sourcePath, destinationPath, logSource, logDestination); // Copy files by pattern
                } else {
                    totalBytesCopied = 0;
                    totalSize = getTotalSize(sourcePath);  // Ensure this is calculated if needed.
                    copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize, logSource, logDestination);
                }
            }
        }
    }
}

void handleDeleteCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath);

    if (!sourceListPath.empty()) {
        std::vector<std::string> sourceFilesList = readListFromFile(sourceListPath);
        std::unordered_set<std::string> filterSet;
        if (!filterListPath.empty())
            filterSet = readSetFromFile(filterListPath);

        for (size_t i = 0; i < sourceFilesList.size(); ++i) {
            sourcePath = sourceFilesList[i];
            preprocessPath(sourcePath, packagePath);
            if (filterListPath.empty() || (!filterListPath.empty() && filterSet.find(sourcePath) == filterSet.end()))
                deleteFileOrDirectory(sourcePath);
        }
    } else {

        // Ensure source path is set
        if (sourcePath.empty()) {
            logMessage("Source path must be specified.");
        } else {
            // Perform the delete operation
            if (!isDangerousCombination(sourcePath)) {
                if (sourcePath.find('*') != std::string::npos)
                    deleteFileOrDirectoryByPattern(sourcePath, logSource); // Delete files by pattern
                else
                    deleteFileOrDirectory(sourcePath, logSource); // Delete single file or directory
            }
        }
    }
}


void handleMirrorCommand(const std::vector<std::string>& cmd, const std::string& packagePath) {
    if (cmd.size() >= 2) {
        std::string sourcePath = cmd[1];
        preprocessPath(sourcePath, packagePath);
        std::string destinationPath;
        if (cmd.size() >= 3) {
            destinationPath = cmd[2];   // Extract the path first
            preprocessPath(destinationPath, packagePath);  // Preprocess it
        } else {
            destinationPath = ROOT_PATH;  // Default value if cmd.size() < 3
        }
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
    std::string sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath;
    parseCommandArguments(cmd, packagePath, sourceListPath, destinationListPath, logSource, logDestination, sourcePath, destinationPath, copyFilterListPath, filterListPath);

    long long totalBytesCopied, totalSize;

    if (!sourceListPath.empty() && !destinationListPath.empty()) {
        std::vector<std::string> sourceFilesList = readListFromFile(sourceListPath);
        std::vector<std::string> destinationFilesList = readListFromFile(destinationListPath);
        if (sourceFilesList.size() != destinationFilesList.size()) {
            logMessage("Source and destination lists must have the same number of entries.");
        } else {
            std::unordered_set<std::string> copyFilterSet;
            if (!copyFilterListPath.empty())
                copyFilterSet = readSetFromFile(copyFilterListPath);

            std::unordered_set<std::string> filterSet;
            if (!filterListPath.empty())
                filterSet = readSetFromFile(filterListPath);

            for (size_t i = 0; i < sourceFilesList.size(); ++i) {
                sourcePath = sourceFilesList[i];
                preprocessPath(sourcePath, packagePath);
                destinationPath = destinationFilesList[i];
                preprocessPath(destinationPath, packagePath);
                if (filterListPath.empty() || (!filterListPath.empty() && filterSet.find(sourcePath) == filterSet.end())) {
                    if (!copyFilterListPath.empty() && copyFilterSet.find(sourcePath) != copyFilterSet.end()) {
                        totalBytesCopied = 0;
                        totalSize = getTotalSize(sourcePath);  // Ensure this is calculated if needed.
                        copyFileOrDirectory(sourcePath, destinationPath, &totalBytesCopied, totalSize);
                    } else {
                        moveFileOrDirectory(sourcePath, destinationPath, "", "");
                    }
                }
            }
        }
    } else {
        // Ensure source and destination paths are set
        if (sourcePath.empty() || destinationPath.empty()) {
            logMessage("Source and destination paths must be specified.");
        } else {
            // Perform the move operation
            if (!isDangerousCombination(sourcePath)) {
                if (sourcePath.find('*') != std::string::npos)
                    moveFilesOrDirectoriesByPattern(sourcePath, destinationPath, logSource, logDestination); // Move files by pattern
                else
                    moveFileOrDirectory(sourcePath, destinationPath, logSource, logDestination); // Move single file or directory
            }
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

void handleHexEdit(const std::string& sourcePath, const std::string& secondArg, const std::string& thirdArg, const std::string& commandName, const std::vector<std::string>& cmd) {
    if (commandName == "hex-by-offset") {
        hexEditByOffset(sourcePath, secondArg, thirdArg);
    } else if (commandName == "hex-by-swap") {
        if (cmd.size() >= 5) {
            std::string selectedStr = cmd[4];
            removeQuotes(selectedStr);
            size_t occurrence = std::stoul(selectedStr);
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
            size_t occurrence = std::stoul(selectedStr);
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    } else if (commandName == "hex-by-decimal") {
        std::string hexDataToReplace = decimalToHex(secondArg);
        std::string hexDataReplacement = decimalToHex(thirdArg);
        if (cmd.size() >= 5) {
            std::string selectedStr = cmd[4];
            removeQuotes(selectedStr);
            size_t occurrence = std::stoul(selectedStr);
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement, occurrence);
        } else {
            hexEditFindReplace(sourcePath, hexDataToReplace, hexDataReplacement);
        }
    } else if (commandName == "hex-by-rdecimal") {
        std::string hexDataToReplace = decimalToReversedHex(secondArg);
        std::string hexDataReplacement = decimalToReversedHex(thirdArg);
        if (cmd.size() >= 5) {
            std::string selectedStr = cmd[4];
            removeQuotes(selectedStr);
            size_t occurrence = std::stoul(selectedStr);
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
        hexEditByCustomOffset(sourcePath, customPattern, offset, hexDataReplacement);
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
            std::string desiredValue = cmd[1];
            removeQuotes(desiredValue);
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
    } else if (commandName.substr(0, 7) == "hex-by-") {
        if (cmd.size() >= 4) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            std::string secondArg = cmd[2];
            removeQuotes(secondArg);
            std::string thirdArg = cmd[3];
            removeQuotes(thirdArg);

            if (commandName == "hex-by-custom-offset" || commandName == "hex-by-custom-decimal-offset" || commandName == "hex-by-custom-rdecimal-offset") {
                if (cmd.size() >= 5) {
                    std::string customPattern = cmd[2];
                    removeQuotes(customPattern);
                    std::string offset = cmd[3];
                    removeQuotes(offset);
                    std::string hexDataReplacement = cmd[4];
                    removeQuotes(hexDataReplacement);
                    handleHexByCustom(sourcePath, customPattern, offset, hexDataReplacement, commandName);
                }
            } else {
                handleHexEdit(sourcePath, secondArg, thirdArg, commandName, cmd);
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
            commandSuccess = downloadSuccess && commandSuccess;
        }
    } else if (commandName == "unzip") {
        if (cmd.size() >= 3) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            std::string destinationPath = cmd[2];
            preprocessPath(destinationPath, packagePath);
            commandSuccess = unzipFile(sourcePath, destinationPath) && commandSuccess;
        }
    } else if (commandName == "pchtxt2ips") {
        if (cmd.size() >= 3) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            std::string destinationPath = cmd[2];
            preprocessPath(destinationPath, packagePath);
            commandSuccess = pchtxt2ips(sourcePath, destinationPath) && commandSuccess;
        }
    } else if (commandName == "pchtxt2cheat") {
        if (cmd.size() >= 2) {
            std::string sourcePath = cmd[1];
            preprocessPath(sourcePath, packagePath);
            commandSuccess = pchtxt2cheat(sourcePath) && commandSuccess;
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
                    if (!commandSuccess) resetCommandSuccess = true;
            
                    interpretAndExecuteCommands(std::move(bootCommands), packagePath, bootCommandName);
                    resetPercentages();
                    if (resetCommandSuccess) {
                        commandSuccess = false;
                    }
                }
            }

        }
    } else if (commandName == "reboot") { // credits to Studious Pancake for the Payload and utils methods
        //spsmInitialize();
        //i2cInitialize();
        if (util::IsErista() || util::SupportsMarikoRebootToConfig()) {
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
        exitingUltrahand = true;
        //setIniFileValue(ULTRAHAND_CONFIG_INI_PATH, ULTRAHAND_PROJECT_NAME, IN_OVERLAY_STR, TRUE_STR); // this is handled within tesla.hpp
        tsl::setNextOverlay(OVERLAY_PATH+"ovlmenu.ovl");
        tsl::Overlay::get()->close();
        return;
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
                lblSetCurrentBrightnessSetting(std::stof(togglePattern) / 100.0f);
            }
            lblExit();
        }
    //} else if (commandName == "volume") {
    //    disableLogging = false;
    //    if (cmd.size() >= 2) {
    //        std::string volumeInput = cmd[1];
    //        logMessage("Received volume command: " + volumeInput);  // Log the input
    //        removeQuotes(volumeInput);  // Sanitize input by removing quotes
    //        
    //        if (isValidNumber(volumeInput)) {
    //            logMessage("Volume input is a valid number: " + volumeInput);  // Log valid number
    //            
    //            // Convert input string to a float for percentage (0-100)
    //            float volumePercentage = std::stof(volumeInput);
    //            logMessage("Converted volume to percentage: " + std::to_string(volumePercentage));  // Log the percentage
    //            
    //            // Ensure the volume is within valid range 0 to 100
    //            if (volumePercentage < 0.0f || volumePercentage > 100.0f) {
    //                logMessage("Volume percentage out of bounds: " + std::to_string(volumePercentage));
    //                return;  // Exit if invalid percentage
    //            }
    //            
    //            // Initialize the settings service
    //            logMessage("Initializing settings service...");
    //            Result rc = setsysInitialize();
    //            if (R_SUCCEEDED(rc)) {
    //                logMessage("Settings service initialized successfully.");
    //                
    //                SetSysAudioVolume audio_volume;
    //                audio_volume.volume = static_cast<uint8_t>((volumePercentage / 100.0f) * 15.0f);  // Scale to 0-15
    //                logMessage("Volume scaled to 0-15: " + std::to_string(audio_volume.volume));  // Log scaled volume
    //                
    //                // Set the volume for the Console
    //                logMessage("Setting audio volume...");
    //                rc = setsysSetAudioVolume(SetSysAudioDevice_Console, &audio_volume);
    //                
    //                // Check if setting the volume was successful
    //                if (R_FAILED(rc)) {
    //                    logMessage("Failed to set audio volume. Error code: " + std::to_string(rc));
    //                    setsysExit();
    //                    return;
    //                }
    //                
    //                logMessage("Audio volume set successfully.");
    //                setsysExit();
    //            } else {
    //                logMessage("Failed to initialize settings service. Error code: " + std::to_string(rc));
    //            }
    //        } else {
    //            logMessage("Invalid volume input: " + volumeInput);
    //        }
    //    } else {
    //        logMessage("Volume command missing required argument.");
    //    }
    //} else if (commandName == "wifi") {
    //    disableLogging = false;
    //    if (cmd.size() >= 2) {
    //        std::string togglePattern = cmd[1];
    //        removeQuotes(togglePattern);
    //
    //        Result rc;
    //
    //        if (togglePattern == ON_STR) {
    //            logMessage("Turning Wi-Fi ON...");
    //            rc = nifmSetWirelessCommunicationEnabled(true);  // Turn Wi-Fi on
    //            if (R_SUCCEEDED(rc)) {
    //                logMessage("Wi-Fi enabled successfully.");
    //            } else {
    //                logMessage("Failed to enable Wi-Fi. Error code: " + std::to_string(rc));
    //            }
    //        } else if (togglePattern == OFF_STR) {
    //            logMessage("Turning Wi-Fi OFF...");
    //            rc = nifmSetWirelessCommunicationEnabled(false);  // Turn Wi-Fi off
    //            if (R_SUCCEEDED(rc)) {
    //                logMessage("Wi-Fi disabled successfully.");
    //            } else {
    //                logMessage("Failed to disable Wi-Fi. Error code: " + std::to_string(rc));
    //            }
    //        } else {
    //            logMessage("Invalid Wi-Fi toggle command: " + togglePattern);
    //        }
    //    } else {
    //        logMessage("Wi-Fi command missing required argument.");
    //    }
    //
    } else if (commandName == "refresh") {
        if (cmd.size() == 1)
            refreshPage = true;
        else if (cmd.size() > 1) {
            std::string refreshPattern = cmd[1];
            removeQuotes(refreshPattern);
            if (refreshPattern == "theme")
                tsl::initializeThemeVars();
            else if (refreshPattern == "package")
                refreshPackage = true;
            else if (refreshPattern == "wallpaper") {
                reloadWallpaper();
            }
        }
    } else if (commandName == "logging") {
        interpreterLogging = true;
    } else if (commandName == "clear") {
        if (cmd.size() >= 2) {
            std::string clearOption = cmd[1];
            removeQuotes(clearOption);
            if (clearOption == "log") deleteFileOrDirectory(defaultLogFilePath);
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



void backgroundInterpreter(void*) {
    std::tuple<std::vector<std::vector<std::string>>, std::string, std::string> args;
    while (!interpreterThreadExit.load(std::memory_order_acquire)) {
        
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            queueCondition.wait(lock, [] { return !interpreterQueue.empty() || interpreterThreadExit.load(std::memory_order_acquire); });
            if (interpreterThreadExit.load(std::memory_order_acquire)) {
                //logMessage("Exiting Thread...");
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



void startInterpreterThread(const std::string& packagePath = "") {
    int stackSize = 0x8000;

    if (!packagePath.empty()) {
        disableLogging = !(parseValueFromIniSection(PACKAGES_INI_FILEPATH, getNameFromPath(packagePath), USE_LOGGING_STR) == TRUE_STR);
        logFilePath = packagePath + "log.txt";
    }

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
        logFilePath = defaultLogFilePath;
        disableLogging = true;
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
