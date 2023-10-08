/********************************************************************************
 * File: hex_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file provides functions for working with hexadecimal data in C++.
 *   It includes functions for converting between ASCII and hexadecimal strings,
 *   finding hexadecimal data offsets in a file, and editing hexadecimal data in a file.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio> // Added for FILE and fopen
#include <cstring> // Added for std::memcmp
#include <sys/stat.h> // Added for stat


// For improving the speed of hexing consecutively with the same file and asciiPattern.
static std::unordered_map<std::string, std::string> hexSumCache;

/**
 * @brief Converts an ASCII string to a hexadecimal string.
 *
 * This function takes an ASCII string as input and converts it into a hexadecimal string.
 *
 * @param asciiStr The ASCII string to convert.
 * @return The corresponding hexadecimal string.
 */
std::string asciiToHex(const std::string& asciiStr) {
    std::string hexStr;
    hexStr.reserve(asciiStr.length() * 2); // Reserve space for the hexadecimal string
    
    for (char c : asciiStr) {
        unsigned char uc = static_cast<unsigned char>(c); // Convert char to unsigned char
        char hexChar[3]; // Buffer to store the hexadecimal representation (2 characters + null terminator)
        
        // Format the unsigned char as a hexadecimal string and append it to the result
        std::snprintf(hexChar, sizeof(hexChar), "%02X", uc);
        hexStr += hexChar;
    }
    
    if (hexStr.length() % 2 != 0) {
        hexStr = '0' + hexStr;
    }
    
    return hexStr;
}

/**
 * @brief Converts a decimal string to a hexadecimal string.
 *
 * This function takes a decimal string as input and converts it into a hexadecimal string.
 *
 * @param decimalStr The decimal string to convert.
 * @return The corresponding hexadecimal string.
 */
std::string decimalToHex(const std::string& decimalStr) {
    // Convert decimal string to integer
    int decimalValue = std::stoi(decimalStr);
    
    // Convert decimal to hexadecimal
    std::string hexadecimal;
    while (decimalValue > 0) {
        int remainder = decimalValue % 16;
        char hexChar = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
        hexadecimal += hexChar;
        decimalValue /= 16;
    }
    
    // Reverse the hexadecimal string
    std::reverse(hexadecimal.begin(), hexadecimal.end());
    
    // If the length is odd, add a trailing '0'
    if (hexadecimal.length() % 2 != 0) {
        hexadecimal = '0' + hexadecimal;
    }
    
    return hexadecimal;
}

/**
 * @brief Converts a decimal string to a reversed hexadecimal string.
 *
 * This function takes a decimal string as input, converts it into a hexadecimal
 * string, and reverses the resulting hexadecimal string in groups of order.
 *
 * @param decimalStr The decimal string to convert.
 * @param order The grouping order for reversing the hexadecimal string.
 * @return The reversed hexadecimal string.
 */
std::string decimalToReversedHex(const std::string& decimalStr, int order = 2) {
    std::string hexadecimal = decimalToHex(decimalStr);
    
    // Reverse the hexadecimal string in groups of order
    std::string reversedHex;
    for (int i = hexadecimal.length() - order; i >= 0; i -= order) {
        reversedHex += hexadecimal.substr(i, order);
    }
    
    return reversedHex;
}

/**
 * @brief Finds the offsets of hexadecimal data in a file.
 *
 * This function searches for occurrences of hexadecimal data in a binary file
 * and returns the file offsets where the data is found.
 *
 * @param filePath The path to the binary file.
 * @param hexData The hexadecimal data to search for.
 * @return A vector of strings containing the file offsets where the data is found.
 */
std::vector<std::string> findHexDataOffsets(const std::string& filePath, const std::string& hexData) {
    std::vector<std::string> offsets;
    
    // Open the file for reading in binary mode
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        //std::cerr << "Failed to open the file." << std::endl;
        return offsets;
    }
    
    // Check the file size
    struct stat fileStatus;
    if (stat(filePath.c_str(), &fileStatus) != 0) {
        //std::cerr << "Failed to retrieve file size." << std::endl;
        fclose(file);
        return offsets;
    }
    //std::size_t fileSize = fileStatus.st_size;
    
    // Convert the hex data string to binary data
    std::vector<unsigned char> binaryData; // Changed to use unsigned char
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16)); // Changed to use unsigned char
        binaryData.push_back(byte);
    }
    
    // Read the file in chunks to find the offsets where the hex data is located
    const std::size_t bufferSize = 131072;
    std::vector<unsigned char> buffer(bufferSize); // Changed to use unsigned char
    std::streampos offset = 0;
    std::size_t bytesRead = 0; // Changed to std::size_t
    while ((bytesRead = fread(buffer.data(), sizeof(unsigned char), bufferSize, file)) > 0) { // Changed to use unsigned char and std::size_t
        for (std::size_t i = 0; i < bytesRead; i++) {
            if (std::memcmp(buffer.data() + i, binaryData.data(), binaryData.size()) == 0) {
                std::streampos currentOffset = static_cast<std::streampos>(offset) + static_cast<std::streamoff>(i);
                offsets.push_back(std::to_string(currentOffset));
            }
        }
        offset += bytesRead;
    }
    
    fclose(file);
    return offsets;
}

/**
 * @brief Finds the offsets of hexadecimal data in a file.
 *
 * This function searches for occurrences of hexadecimal data in a binary file
 * and returns the file offsets where the data is found.
 *
 * @param filePath The path to the binary file.
 * @param hexData The hexadecimal data to search for.
 * @return A vector of strings containing the file offsets where the data is found.
 */
std::vector<std::string> findHexDataOffsetsFile(FILE* file, const std::string& hexData) {
    std::vector<std::string> offsets;
    
    
    if (!file) {
        //std::cerr << "Failed to open the file." << std::endl;
        return offsets;
    }
    
    //std::size_t fileSize = fileStatus.st_size;
    
    // Convert the hex data string to binary data
    std::vector<unsigned char> binaryData; // Changed to use unsigned char
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16)); // Changed to use unsigned char
        binaryData.push_back(byte);
    }
    
    // Read the file in chunks to find the offsets where the hex data is located
    const std::size_t bufferSize = 131072;
    std::vector<unsigned char> buffer(bufferSize); // Changed to use unsigned char
    std::streampos offset = 0;
    std::size_t bytesRead = 0; // Changed to std::size_t
    while ((bytesRead = fread(buffer.data(), sizeof(unsigned char), bufferSize, file)) > 0) { // Changed to use unsigned char and std::size_t
        for (std::size_t i = 0; i < bytesRead; i++) {
            if (std::memcmp(buffer.data() + i, binaryData.data(), binaryData.size()) == 0) {
                std::streampos currentOffset = static_cast<std::streampos>(offset) + static_cast<std::streamoff>(i);
                offsets.push_back(std::to_string(currentOffset));
            }
        }
        offset += bytesRead;
    }
    
    return offsets;
}

/**
 * @brief Edits hexadecimal data in a file at a specified offset.
 *
 * This function opens a binary file, seeks to a specified offset, and replaces
 * the data at that offset with the provided hexadecimal data.
 *
 * @param filePath The path to the binary file.
 * @param offsetStr The offset in the file to performthe edit.
 * @param hexData The hexadecimal data to replace at the offset.
 */
void hexEditByOffset(const std::string& filePath, const std::string& offsetStr, const std::string& hexData) {
    // Convert the offset string to std::streampos
    std::streampos offset = std::stoll(offsetStr);
    
    // Open the file for reading and writing in binary mode
    FILE* file = fopen(filePath.c_str(), "rb+");
    if (!file) {
        //logMessage("Failed to open the file.");
        return;
    }
    
    // Move the file pointer to the specified offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        //logMessage("Failed to move the file pointer.");
        fclose(file);
        return;
    }
    
    // Convert the hex data string to binary data
    std::vector<unsigned char> binaryData; // Changed to use unsigned char
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16)); // Changed to use unsigned char
        binaryData.push_back(byte);
    }
    
    // Calculate the number of bytes to be replaced
    std::size_t bytesToReplace = binaryData.size();
    
    // Read the existing data from the file
    std::vector<unsigned char> existingData(bytesToReplace); // Changed to use unsigned char
    if (fread(existingData.data(), sizeof(unsigned char), bytesToReplace, file) != bytesToReplace) { // Changed to use unsigned char
        //logMessage("Failed to read existing data from the file.");
        fclose(file);
        return;
    }
    
    // Move the file pointer back to the offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        //logMessage("Failed to move the file pointer.");
        fclose(file);
        return;
    }
    
    // Write the replacement binary data to the file
    if (fwrite(binaryData.data(), sizeof(unsigned char), bytesToReplace, file) != bytesToReplace) { // Changed to use unsigned char
        //logMessage("Failed to write data to the file.");
        fclose(file);
        return;
    }
    
    fclose(file);
    //logMessage("Hex editing completed.");
}

/**
 * @brief Edits a specific offset in a file with custom hexadecimal data.
 *
 * This function searches for a custom pattern in the file and calculates a new offset
 * based on user-provided offsetStr and the found pattern. It then replaces the data
 * at the calculated offset with the provided hexadecimal data.
 *
 * @param filePath The path to the binary file.
 * @param offsetStr The user-provided offset for the edit.
 * @param customPattern The custom pattern to search for in the file.
 * @param hexDataReplacement The hexadecimal data to replace at the calculated offset.
 * @param occurrence The occurrence/index of the data to replace (default is "0" to replace all occurrences).
 */
void hexEditByCustomOffset(const std::string& filePath, const std::string& customAsciiPattern, const std::string& offsetStr, const std::string& hexDataReplacement, size_t occurrence = 0) {
    
    // Create a cache key based on filePath and customAsciiPattern
    std::string cacheKey = filePath + '?' + customAsciiPattern + '?' + std::to_string(occurrence);
    
    int hexSum = -1;
    
    // Check if the result is already cached
    auto cachedResult = hexSumCache.find(cacheKey);
    if (cachedResult != hexSumCache.end()) {
        hexSum = std::stoi(cachedResult->second); // load sum from cache
    }
    
    if (hexSum == -1) {
        // Convert custom ASCII pattern to a custom hex pattern
        std::string customHexPattern = asciiToHex(customAsciiPattern);
        
        // Find hex data offsets in the file
        std::vector<std::string> offsets = findHexDataOffsets(filePath, customHexPattern);
        
        if (!offsets.empty()) {
            hexSum = std::stoi(offsets[occurrence]);
            
            // Convert 'hexSum' to a string and add it to the cache
            hexSumCache[cacheKey] = std::to_string(hexSum);
        } else {
            logMessage("Offset not found.");
            return;
        }
    }
    
    
    if (hexSum != -1) {
        // Calculate the total offset to seek in the file
        int sum = hexSum + std::stoi(offsetStr);
        hexEditByOffset(filePath, std::to_string(sum), hexDataReplacement);
    } else {
        logMessage("Failed to find " + customAsciiPattern + ".");
    }
}

/**
 * @brief Finds and replaces hexadecimal data in a file.
 *
 * This function searches for occurrences of hexadecimal data in a binary file
 * and replaces them with a specified hexadecimal replacement data.
 *
 * @param filePath The path to the binary file.
 * @param hexDataToReplace The hexadecimal data to search for and replace.
 * @param hexDataReplacement The hexadecimal data to replace with.
 * @param occurrence The occurrence/index of the data to replace (default is "0" to replace all occurrences).
 */
void hexEditFindReplace(const std::string& filePath, const std::string& hexDataToReplace, const std::string& hexDataReplacement, size_t occurrence = 0) {
    std::vector<std::string> offsetStrs = findHexDataOffsets(filePath, hexDataToReplace);
    if (!offsetStrs.empty()) {
        if (occurrence == 0) {
            // Replace all occurrences
            for (const std::string& offsetStr : offsetStrs) {
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            }
        } else {
            // Convert the occurrence string to an integer
            if (occurrence > 0 && occurrence <= offsetStrs.size()) {
                // Replace the specified occurrence/index
                std::string offsetStr = offsetStrs[occurrence - 1];
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            } else {
                // Invalid occurrence/index specified
                //std::cout << "Invalid occurrence/index specified." << std::endl;
            }
        }
        //std::cout << "Hex data replaced successfully." << std::endl;
    }
    else {
        //std::cout << "Hex data to replace not found." << std::endl;
    }
}

/**
 * @brief Finds and replaces hexadecimal data in a file.
 *
 * This function searches for occurrences of hexadecimal data in a binary file
 * and replaces them with a specified hexadecimal replacement data.
 *
 * @param filePath The path to the binary file.
 * @param hexDataToReplace The hexadecimal data to search for and replace.
 * @param hexDataReplacement The hexadecimal data to replace with.
 * @param occurrence The occurrence/index of the data to replace (default is "0" to replace all occurrences).
 */
std::string parseHexDataAtCustomOffset(const std::string& filePath, const std::string& customAsciiPattern, const std::string& offsetStr, size_t length, size_t occurrence = 0) {
    // Open the file for reading in binary mode
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        logMessage("Failed to open the file.");
        return "";
    }
    
    // Create a cache key based on filePath and customAsciiPattern
    std::string cacheKey = filePath + '?' + customAsciiPattern + '?' + std::to_string(occurrence);
    
    int hexSum = -1;
    
    // Check if the result is already cached
    auto cachedResult = hexSumCache.find(cacheKey);
    if (cachedResult != hexSumCache.end()) {
        hexSum = std::stoi(cachedResult->second); // load sum from cache
    }
    
    if (hexSum == -1) {
        // Convert custom ASCII pattern to a custom hex pattern
        std::string customHexPattern = asciiToHex(customAsciiPattern);
        
        // Find hex data offsets in the file
        std::vector<std::string> offsets = findHexDataOffsets(filePath, customHexPattern);
        
        if (!offsets.empty()) {
            hexSum = std::stoi(offsets[occurrence]);
            
            // Convert 'hexSum' to a string and add it to the cache
            hexSumCache[cacheKey] = std::to_string(hexSum);
        } else {
            logMessage("Offset not found.");
            fclose(file);
            return "";
        }
    }
    
    // Calculate the total offset to seek in the file
    int sum = hexSum + std::stoi(offsetStr);
    
    // Seek to the specified offset
    if (fseek(file, sum, SEEK_SET) != 0) {
        logMessage("Error seeking to offset.");
        fclose(file);
        return "";
    }
    
    std::stringstream hexStream;
    char hexBuffer[length];
    
    // Read data from the file and convert to hex
    if (fread(hexBuffer, 1, length, file) == length) {
        for (size_t i = 0; i < length; ++i) {
            hexStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(hexBuffer[i]);
        }
    } else {
        if (feof(file)) {
            logMessage("End of file reached.");
        } else if (ferror(file)) {
            logMessage("Error reading data from file: " + std::to_string(errno));
        }
    }
    
    // Close the file
    fclose(file);
    
    // Convert lowercase hex to uppercase and return the result
    std::string result = hexStream.str();
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    
    return result;
}

/**
 * @brief Finds and replaces hexadecimal data in a file.
 *
 * This function searches for occurrences of hexadecimal data in a binary file
 * and replaces them with a specified hexadecimal replacement data.
 *
 * @param filePath The path to the binary file.
 * @param hexDataToReplace The hexadecimal data to search for and replace.
 * @param hexDataReplacement The hexadecimal data to replace with.
 * @param occurrence The occurrence/index of the data to replace (default is "0" to replace all occurrences).
 */

std::string replaceHexPlaceholder(const std::string& arg, const std::string& hexPath) {
    std::string replacement = arg;
    std::string searchString = "{hex_file(";
    
    std::size_t startPos = replacement.find(searchString);
    std::size_t endPos = replacement.find(")}");
    
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        std::string placeholderContent = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
        
        // Split the placeholder content into its components (customAsciiPattern, offsetStr, length)
        std::vector<std::string> components;
        std::istringstream componentStream(placeholderContent);
        std::string component;
        
        while (std::getline(componentStream, component, ',')) {
            components.push_back(trim(component));
        }
        
        if (components.size() == 3) {
            // Extract individual components
            std::string customAsciiPattern = components[0];
            std::string offsetStr = components[1];
            size_t length = std::stoul(components[2]);
            
            // Call the parsing function and replace the placeholder
            std::string parsedResult = parseHexDataAtCustomOffset(hexPath, customAsciiPattern, offsetStr, length);
            
            //std::string parsedResult = customAsciiPattern+offsetStr;
            
            // Only replace if parsedResult returns a non-empty string
            if (parsedResult != "") {
                // Replace the entire placeholder with the parsed result
                replacement.replace(startPos, endPos - startPos + searchString.length() + 2, parsedResult);
            }
        }
    }
    
    return replacement;
}
