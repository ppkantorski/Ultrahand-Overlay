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
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <debug_funcs.hpp>

const std::string IPS32_HEAD_MAGIC = "IPS32";
const std::string IPS32_FOOT_MAGIC = "EEOF";

/**
 * @brief Converts a .pchtxt file to an IPS file using fstream.
 *
 * This function reads the contents of a .pchtxt file, extracts the address-value pairs,
 * and generates an IPS file with the provided output folder.
 *
 * @param pchtxtPath The file path to the .pchtxt file.
 * @param outputFolder The folder path for the output IPS file.
 * @return True if the conversion was successful, false otherwise.
 */
bool pchtxt2ips(const std::string& pchtxtPath, const std::string& outputFolder) {
    std::ifstream pchtxtFile(pchtxtPath);
    if (!pchtxtFile) {
        logMessage("Error: Unable to open file " + pchtxtPath);
        return false;
    }

    std::vector<std::pair<uint32_t, std::vector<uint8_t>>> patches;
    std::string line;
    uint32_t lineNum = 0;
    std::string nsobid;

    while (std::getline(pchtxtFile, line)) {
        ++lineNum;
        if (line.empty() || line.front() == '@') {
            if (line.find("@nsobid-") == 0) {
                nsobid = line.substr(8);
            }
            continue;  // Skip empty lines and lines starting with '@'
        }

        std::istringstream iss(line);
        std::string addressStr, valueStr;
        if (!(iss >> addressStr >> valueStr)) {
            continue;
        }

        char* endPtr;
        uint32_t address = std::strtoul(addressStr.c_str(), &endPtr, 16);
        if (*endPtr != '\0') {
            continue;
        }

        std::vector<uint8_t> valueBytes;
        for (size_t i = 0; i < valueStr.length(); i += 2) {
            uint8_t byte = std::stoi(valueStr.substr(i, 2), nullptr, 16);
            valueBytes.push_back(byte);
        }
        
        if (valueBytes.empty()) {
            continue;
        }

        patches.push_back(std::make_pair(address, valueBytes));
    }

    if (nsobid.empty()) {
        nsobid = pchtxtPath.substr(pchtxtPath.find_last_of("/\\") + 1);
        nsobid = nsobid.substr(0, nsobid.find_last_of("."));
    }

    pchtxtFile.close();

    std::string ipsFileName = nsobid + ".ips";
    std::string ipsFilePath = outputFolder + '/' + ipsFileName;

    std::ofstream ipsFile(ipsFilePath, std::ios::binary);
    if (!ipsFile) {
        logMessage("Error: Unable to create IPS file " + ipsFilePath);
        return false;
    }

    ipsFile.write(IPS32_HEAD_MAGIC.c_str(), IPS32_HEAD_MAGIC.size());

    for (const auto& patch : patches) {
        uint32_t address = patch.first;
        ipsFile.write(reinterpret_cast<const char*>(&address), sizeof(address));  // Write address
        uint16_t valueLength = patch.second.size();
        ipsFile.write(reinterpret_cast<const char*>(&valueLength), sizeof(valueLength));  // Write length of value
        ipsFile.write(reinterpret_cast<const char*>(patch.second.data()), patch.second.size());  // Write value
    }

    ipsFile.write(IPS32_FOOT_MAGIC.c_str(), IPS32_FOOT_MAGIC.size());
    ipsFile.close();

    return true;
}
