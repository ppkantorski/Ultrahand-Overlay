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
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2024 ppkantorski
 ********************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
//#include <cstdio> // Added for FILE and fopen
#include <fstream>
#include <cstring> // Added for std::memcmp


size_t HEX_BUFFER_SIZE = 4096;


// For improving the speed of hexing consecutively with the same file and asciiPattern.
static std::unordered_map<std::string, std::string> hexSumCache; // MOVED TO main.cpp

/**
 * @brief Converts an ASCII string to a hexadecimal string.
 *
 * This function takes an ASCII string as input and converts it into a hexadecimal string.
 *
 * @param asciiStr The ASCII string to convert.
 * @return The corresponding hexadecimal string.
 */
std::string asciiToHex(const std::string& asciiStr) {
    static const char hexDigits[] = "0123456789ABCDEF";
    std::string hexStr;
    hexStr.reserve(asciiStr.length() * 2);  // Reserve space for the hexadecimal string

    for (unsigned char c : asciiStr) {
        hexStr += hexDigits[c >> 4];   // Append the high nibble
        hexStr += hexDigits[c & 0xF];  // Append the low nibble
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
    int decimalValue = std::stoi(decimalStr);

    // Edge case for zero
    if (decimalValue == 0) {
        return "00";
    }

    // Preallocate the maximum possible size for a 32-bit integer
    char hexBuffer[8];  // 8 characters are enough for a full 32-bit integer
    int index = 0;

    while (decimalValue != 0) {
        int remainder = decimalValue % 16;
        // Fill the buffer from the end to the start
        hexBuffer[7 - index] = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
        index++;
        decimalValue /= 16;
    }

    // Construct string from the filled part of the buffer
    return std::string(hexBuffer + 8 - index, index);
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
    
    std::string reversedHex(hexadecimal.length(), '0');  // Preallocate string with the required length
    int numGroups = hexadecimal.length() / order;
    
    for (int group = 0; group < numGroups; ++group) {
        for (int charInGroup = 0; charInGroup < order; ++charInGroup) {
            reversedHex[group * order + charInGroup] = hexadecimal[(numGroups - 1 - group) * order + charInGroup];
        }
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
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return offsets; // Return empty vector if file cannot be opened
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Convert the hex data string to binary data
    std::vector<unsigned char> binaryData;
    if (hexData.length() % 2 != 0) {
        file.close();
        return offsets; // Ensure hexData has an even length
    }
    for (size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        binaryData.push_back(byte);
    }

    // Read the file in chunks to find the offsets where the hex data is located
    constexpr size_t HEX_BUFFER_SIZE = 4096*4; // Arbitrary buffer size, can be adjusted
    std::vector<unsigned char> buffer(HEX_BUFFER_SIZE);
    size_t bytesRead = 0;
    size_t offset = 0;

    while (file.read(reinterpret_cast<char*>(buffer.data()), HEX_BUFFER_SIZE)) {
        bytesRead = file.gcount();
        for (size_t i = 0; i < bytesRead; ++i) {
            if (offset + i + binaryData.size() <= fileSize && std::memcmp(buffer.data() + i, binaryData.data(), binaryData.size()) == 0) {
                offsets.push_back(std::to_string(offset + i));
            }
        }
        offset += bytesRead;
    }

    file.close();
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
//std::vector<std::string> findHexDataOffsetsF(FILE* file, const std::string& hexData) {
//    std::vector<std::string> offsets;
//    
//    
//    if (!file) {
//        //std::cerr << "Failed to open the file." << std::endl;
//        return offsets;
//    }
//    
//    //size_t fileSize = fileStatus.st_size;
//    
//    // Convert the hex data string to binary data
//    std::vector<unsigned char> binaryData; // Changed to use unsigned char
//    for (size_t i = 0; i < hexData.length(); i += 2) {
//        std::string byteString = hexData.substr(i, 2);
//        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16)); // Changed to use unsigned char
//        binaryData.push_back(byte);
//    }
//    
//    // Read the file in chunks to find the offsets where the hex data is located
//    //const size_t bufferSize = 131072;
//    std::vector<unsigned char> buffer(HEX_BUFFER_SIZE); // Changed to use unsigned char
//    std::streampos offset = 0;
//    size_t bytesRead = 0; // Changed to size_t
//    std::streampos currentOffset;
//    while ((bytesRead = fread(buffer.data(), sizeof(unsigned char), HEX_BUFFER_SIZE, file)) > 0) { // Changed to use unsigned char and size_t
//        for (size_t i = 0; i < bytesRead; i++) {
//            if (std::memcmp(buffer.data() + i, binaryData.data(), binaryData.size()) == 0) {
//                currentOffset = static_cast<std::streampos>(offset) + static_cast<std::streamoff>(i);
//                offsets.push_back(std::to_string(currentOffset));
//            }
//        }
//        offset += bytesRead;
//    }
//    
//    return offsets;
//}


// Function to convert a hex string to binary data
std::vector<unsigned char> hexToBinary(const std::string& hexData) {
    std::vector<unsigned char> binaryData;
    for (size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        unsigned char byte = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
        binaryData.push_back(byte);
    }
    return binaryData;
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
    std::streampos offset = std::stoll(offsetStr);

    // Open the file for both reading and writing in binary mode
    std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::out);
    if (!file.is_open()) {
        logMessage("Failed to open the file.");
        return;
    }

    // Retrieve the file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (offset >= fileSize) {
        logMessage("Offset exceeds file size.");
        return;
    }

    // Convert the hex string to binary data
    std::vector<unsigned char> binaryData(hexData.length() / 2);
    for (size_t i = 0, j = 0; i < hexData.length(); i += 2, ++j) {
        std::string byteString = hexData.substr(i, 2);
        binaryData[j] = static_cast<unsigned char>(std::stoi(byteString, nullptr, 16));
    }

    // Move to the specified offset
    file.seekg(offset);

    // Write the binary data directly to the file at the offset
    file.seekp(offset);
    file.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
    if (!file) {
        logMessage("Failed to write data to the file.");
        return;
    }
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
        std::string customHexPattern;
        if (customAsciiPattern[0] == '#') {
            // remove #
            customHexPattern = customAsciiPattern.substr(1);
        } else {
            // Convert custom ASCII pattern to a custom hex pattern
            customHexPattern = asciiToHex(customAsciiPattern);
        }
        
        
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

    if (offsetStrs.empty()) {
        //std::cout << "Hex data to replace not found." << std::endl;
        return;
    }

    // Open the file once for all operations
    std::fstream file(filePath, std::ios::in | std::ios::out | std::ios::binary);
    if (!file.is_open()) {
        //std::cout << "Failed to open the file for editing." << std::endl;
        return;
    }

    if (occurrence == 0) {
        // Replace all occurrences
        for (const std::string& offsetStr : offsetStrs) {
            hexEditByOffset(filePath, offsetStr, hexDataReplacement);
        }
    } else if (occurrence > 0 && occurrence <= offsetStrs.size()) {
        // Replace the specified occurrence/index
        std::string offsetStr = offsetStrs[occurrence - 1];
        hexEditByOffset(filePath, offsetStr, hexDataReplacement);
    } else {
        //std::cout << "Invalid occurrence/index specified." << std::endl;
    }

    file.close(); // Close the file after all operations
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
    std::string cacheKey = filePath + '?' + customAsciiPattern + '?' + std::to_string(occurrence);
    int hexSum = -1;

    auto cachedResult = hexSumCache.find(cacheKey);
    if (cachedResult != hexSumCache.end()) {
        hexSum = std::stoi(cachedResult->second);
    } else {
        std::string customHexPattern = asciiToHex(customAsciiPattern); // Function should cache its results if expensive
        std::vector<std::string> offsets = findHexDataOffsets(filePath, customHexPattern); // Consider optimizing this search
        
        if (!offsets.empty() && offsets.size() > occurrence) {
            hexSum = std::stoi(offsets[occurrence]);
            hexSumCache[cacheKey] = std::to_string(hexSum);
        } else {
            logMessage("Offset not found.");
            return "";
        }
    }
    
    std::streampos totalOffset = hexSum + std::stoll(offsetStr);
    std::vector<char> hexBuffer(length);
    std::vector<char> hexStream(length * 2);

    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        logMessage("Failed to open the file.");
        return "";
    }
    
    file.seekg(totalOffset);
    if (!file) {
        logMessage("Error seeking to offset.");
        return "";
    }

    file.read(hexBuffer.data(), length);
    if (file.gcount() == static_cast<std::streamsize>(length)) {
        const char hexDigits[] = "0123456789ABCDEF";
        for (size_t i = 0; i < length; ++i) {
            hexStream[i * 2] = hexDigits[(hexBuffer[i] >> 4) & 0xF];
            hexStream[i * 2 + 1] = hexDigits[hexBuffer[i] & 0xF];
        }
    } else {
        logMessage("Error reading data from file or end of file reached.");
        return "";
    }

    file.close();
    std::string result(hexStream.begin(), hexStream.end());
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
//std::string parseHexDataAtCustomOffsetF(FILE*& file, const std::string& filePath, const std::string& customAsciiPattern, const std::string& offsetStr, size_t length, size_t occurrence = 0) {
//    
//    // Create a cache key based on filePath and customAsciiPattern
//    std::string cacheKey = filePath + '?' + customAsciiPattern + '?' + std::to_string(occurrence);
//    
//    int hexSum = -1;
//    
//    // Check if the result is already cached
//    auto cachedResult = hexSumCache.find(cacheKey);
//    if (cachedResult != hexSumCache.end()) {
//        hexSum = std::stoi(cachedResult->second); // load sum from cache
//    }
//    
//    if (hexSum == -1) {
//        // Convert custom ASCII pattern to a custom hex pattern
//        std::string customHexPattern = asciiToHex(customAsciiPattern);
//        
//        // Find hex data offsets in the file
//        std::vector<std::string> offsets = findHexDataOffsetsF(file, customHexPattern);
//        
//        if (!offsets.empty()) {
//            hexSum = std::stoi(offsets[occurrence]);
//            
//            // Convert 'hexSum' to a string and add it to the cache
//            hexSumCache[cacheKey] = std::to_string(hexSum);
//        } else {
//            logMessage("Offset not found.");
//            return "";
//        }
//    }
//    
//    // Calculate the total offset to seek in the file
//    int sum = hexSum + std::stoi(offsetStr);
//    
//    
//    // Open the file for reading in binary mode
//    //FILE* file = fopen(filePath.c_str(), "rb");
//    if (!file) {
//        logMessage("Failed to open the file.");
//        return "";
//    }
//    
//    // Seek to the specified offset
//    if (fseek(file, sum, SEEK_SET) != 0) {
//        logMessage("Error seeking to offset.");
//        //fclose(file);
//        return "";
//    }
//    
//    char hexBuffer[length];
//    char hexDigits[] = "0123456789ABCDEF";
//    char* hexStream = new char[length * 2];  // Allocate memory for the result
//    
//    size_t bytesRead = fread(hexBuffer, 1, length, file);
//    if (bytesRead == length) {
//        for (size_t i = 0; i < length; ++i) {
//            hexStream[i * 2] = hexDigits[(hexBuffer[i] >> 4) & 0xF];
//            hexStream[i * 2 + 1] = hexDigits[hexBuffer[i] & 0xF];
//        }
//    } else if (feof(file)) {
//        logMessage("End of file reached.");
//    } else if (ferror(file)) {
//        logMessage("Error reading data from file: " + std::to_string(errno));
//    }
//    
//    
//    
//    // Convert lowercase hex to uppercase and return the result
//    std::string result(hexStream, length * 2);
//    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
//    
//    delete[] hexStream;
//    
//    return result;
//}


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
    const std::string searchString = "{hex_file(";
    std::string replacement = arg;

    size_t startPos = replacement.find(searchString);
    if (startPos == std::string::npos) return replacement;

    size_t endPos = replacement.find(")}", startPos + searchString.length());
    if (endPos == std::string::npos) return replacement;

    std::string placeholderContent = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
    
    size_t firstComma = placeholderContent.find(',');
    size_t secondComma = placeholderContent.rfind(',');
    if (firstComma == std::string::npos || secondComma == std::string::npos || firstComma == secondComma) {
        return replacement;  // Not enough parts
    }

    std::string customAsciiPattern = trim(placeholderContent.substr(0, firstComma));
    std::string offsetStr = trim(placeholderContent.substr(firstComma + 1, secondComma - firstComma - 1));
    size_t length = std::stoul(trim(placeholderContent.substr(secondComma + 1)));

    std::string parsedResult = parseHexDataAtCustomOffset(hexPath, customAsciiPattern, offsetStr, length);
    if (!parsedResult.empty()) {
        replacement.replace(startPos, endPos - startPos + searchString.length() + 2, parsedResult);
    }

    return replacement;
}
