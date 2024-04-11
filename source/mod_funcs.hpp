/********************************************************************************
 * File: mod_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains function declarations and utility functions for IPS
 *   binary generations. These functions are used in the Ultrahand Overlay project
 *   to convert `.pchtxt` mods into `.ips` binaries.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/

#pragma once
#include <vector>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <sys/stat.h>
#include <debug_funcs.hpp>

const std::string IPS32_HEAD_MAGIC = "IPS32";
const std::string IPS32_FOOT_MAGIC = "EEOF";

/**
 * @brief Converts a .pchtxt file to an IPS file.
 *
 * This function reads the contents of a .pchtxt file, extracts the address-value pairs,
 * and generates an IPS file with the provided output path.
 *
 * @param pchtxt_path The file path to the .pchtxt file.
 * @param output_ips_path The file path for the output IPS file.
 * @return True if the convsersion was successful, false otherwise.
 */
bool pchtxt2ips(const std::string& pchtxtPath, const std::string& outputIpsPath) {
    bool success = true;

    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> patches;

    FILE* pchtxtFile = fopen(pchtxtPath.c_str(), "r");
    if (!pchtxtFile) {
        logMessage("Error: Unable to open file " + pchtxtPath);
        success = false;
        exit(1);
    }

    char line[1024]; // Assuming maximum line length is 1024 characters
    
    uint32_t lineNum = 0;
    while (fgets(line, sizeof(line), pchtxtFile) != NULL) {
        ++lineNum;
        std::string lineStr(line);
        if (lineStr.empty() || lineStr.find('@') == 0) continue;  // Skip empty lines and lines starting with '@'

        std::string addressStr, valueStr;
        char* token = strtok(line, " ");
        if (!token || !(addressStr = std::string(token)).size() || !(token = strtok(NULL, " ")) || !(valueStr = std::string(token)).size()) {
            //logMessage("Warning: Line " + std::to_string(lineNum) + " does not contain address and value");
            continue;
        }

        char* endPtr;
        uint32_t address = std::strtoul(addressStr.c_str(), &endPtr, 16);
        if (*endPtr != '\0') {
            //logMessage("Warning: Line " + std::to_string(lineNum) + " contains an invalid address");
            continue;
        }

        std::vector<uint8_t> valueBytes;
        for (size_t i = 0; i < valueStr.length(); i += 2) {
            uint8_t byte = std::stoi(valueStr.substr(i, 2), nullptr, 16);
            valueBytes.push_back(byte);
        }

        patches.push_back(std::make_pair(address, valueBytes));
    }

    fclose(pchtxtFile);

    // Write IPS file
    FILE* ipsFile = fopen(outputIpsPath.c_str(), "wb");
    if (!ipsFile) {
        logMessage("Error: Unable to create IPS file " + outputIpsPath);
        success = false;
        exit(1);
    }

    fwrite(IPS32_HEAD_MAGIC.c_str(), 1, IPS32_HEAD_MAGIC.size(), ipsFile);

    for (const auto& patch : patches) {
        uint32_t address = patch.first;
        const std::vector<uint8_t>& value = patch.second;
        fwrite(&address, sizeof(address), 1, ipsFile);  // Write address
        uint16_t valueLength = value.size();
        fwrite(&valueLength, sizeof(valueLength), 1, ipsFile);  // Write length of value
        fwrite(value.data(), 1, value.size(), ipsFile);  // Write value
    }

    fwrite(IPS32_FOOT_MAGIC.c_str(), 1, IPS32_FOOT_MAGIC.size(), ipsFile);

    fclose(ipsFile);

    return success;
}
