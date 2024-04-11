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
 * and generates an IPS file with the provided output folder.
 *
 * @param pchtxtPath The file path to the .pchtxt file.
 * @param outputFolder The folder path for the output IPS file.
 * @return True if the conversion was successful, false otherwise.
 */
bool pchtxt2ips(const std::string& pchtxtPath, const std::string& outputFolder) {
    bool success = true;

    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> patches;

    FILE* pchtxtFile = fopen(pchtxtPath.c_str(), "r");
    if (!pchtxtFile) {
        logMessage("Error: Unable to open file " + pchtxtPath);
        success = false;
        return success;
    }

    char line[2048]; // Assuming maximum line length is 512 characters
    

    uint32_t lineNum = 0;
    std::string nsobid;
    while (fgets(line, sizeof(line), pchtxtFile) != NULL) {

        ++lineNum;
        // Check for newline character to handle lines longer than the buffer
        if (line[strlen(line) - 1] != '\n') {
            logMessage("Error: Line " + std::to_string(lineNum) + " exceeds maximum line length");
            break;
        }

        std::string lineStr(line);
        if (lineStr.empty() || lineStr.find('@') == 0) continue;  // Skip empty lines and lines starting with '@'

        if (lineStr.find("@nsobid-") == 0) {
            nsobid = lineStr.substr(8); // Extract the nsobid value
            continue;
        }

        std::string addressStr, valueStr;

        // Tokenize the line using std::istringstream
        std::istringstream iss(line);
        if (!(iss >> addressStr >> valueStr)) {
            // Log only if the line doesn't contain address and value
            continue;
        }

        char* endPtr;
        uint32_t address = std::strtoul(addressStr.c_str(), &endPtr, 16);
        if (*endPtr != '\0') {
            // Log only if the address is invalid
            continue;
        }

        std::vector<uint8_t> valueBytes;
        for (size_t i = 0; i < valueStr.length(); i += 2) {
            uint8_t byte = std::stoi(valueStr.substr(i, 2), nullptr, 16);
            valueBytes.push_back(byte);
        }
        
        // Check if valueBytes is empty, if so, log a warning and continue
        if (valueBytes.empty()) {
            // Log only if the value is invalid
            continue;
        }
        
        patches.push_back(std::make_pair(address, valueBytes));
    }

    // If nsobid is empty, use the base name of the pchtxt file
    if (nsobid.empty()) {
        std::string nsobidPrefix = "@nsobid-";
        std::string lineStr;
        fseek(pchtxtFile, 0, SEEK_SET); // Move file pointer to the beginning
        while (fgets(line, sizeof(line), pchtxtFile) != NULL) {
            lineStr = line;
            size_t pos = lineStr.find(nsobidPrefix);
            if (pos != std::string::npos) {
                nsobid = lineStr.substr(pos + nsobidPrefix.size());
                // Remove trailing newline character
                nsobid.erase(nsobid.find_last_not_of("\r\n") + 1); // Remove all trailing whitespace, including newlines
                break;
            }
        }
        if (nsobid.empty()) {
            nsobid = pchtxtPath.substr(pchtxtPath.find_last_of("/\\") + 1);
            nsobid = nsobid.substr(0, nsobid.find_last_of("."));
        }
    }
    
    //logMessage("nsobid: "+nsobid);

    fclose(pchtxtFile);

    std::string ipsFileName = nsobid + ".ips";

    // Construct IPS file path
    std::string ipsFilePath = outputFolder + ipsFileName;

    //logMessage("ipsFilePath: "+ipsFilePath);

    // Write IPS file
    FILE* ipsFile = fopen(ipsFilePath.c_str(), "wb");
    if (!ipsFile) {
        logMessage("Error: Unable to create IPS file " + ipsFilePath);
        success = false;
        return success;
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
