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

#ifndef HEX_FUNCS_HPP
#define HEX_FUNCS_HPP

#if !USING_FSTREAM_DIRECTIVE // For not using fstream (needs implementing)
#include <stdio.h>
#else
#include <fstream>
#endif

#include <string>
#include <vector>
#include <algorithm>
#include <functional>
//#include <cstdio> // Added for FILE and fopen
#include <cstring> // Added for std::memcmp
#include <mutex>
#include <shared_mutex>

#include <global_vars.hpp>
#include <debug_funcs.hpp>
#include <string_funcs.hpp>


namespace ult {
    extern size_t HEX_BUFFER_SIZE;
    
    
    // For improving the speed of hexing consecutively with the same file and asciiPattern.
    extern std::unordered_map<std::string, std::string> hexSumCache; // MOVED TO main.cpp
    
    // Lookup table for hex characters
    inline constexpr char hexLookup[] = "0123456789ABCDEF";
    

    // ULTRA-FAST hex conversion with lookup table
    inline constexpr unsigned char hexTable[256] = {
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,
        0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,10,11,12,13,14,15,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
    };


    extern void clearHexSumCache();
    extern size_t getHexSumCacheSize();

    /**
     * @brief Converts an ASCII string to a hexadecimal string.
     *
     * This function takes an ASCII string as input and converts it into a hexadecimal string.
     *
     * @param asciiStr The ASCII string to convert.
     * @return The corresponding hexadecimal string.
     */
    
    
    // Function to convert ASCII string to Hex string
    std::string asciiToHex(const std::string& asciiStr);
    
    /**
     * @brief Converts a decimal string to a hexadecimal string.
     *
     * This function takes a decimal string as input and converts it into a hexadecimal string.
     *
     * @param decimalStr The decimal string to convert.
     * @return The corresponding hexadecimal string.
     */
    std::string decimalToHex(const std::string& decimalStr, int byteGroupSize = 2);
    
    
    /**
     * @brief Converts a hexadecimal string to a decimal string.
     *
     * This function takes a hexadecimal string as input and converts it into a decimal string.
     *
     * @param hexStr The hexadecimal string to convert.
     * @return The corresponding decimal string.
     */
    std::string hexToDecimal(const std::string& hexStr);
    
    
    
    std::string hexToReversedHex(const std::string& hexadecimal, int order = 2);
    
    /**
     * @brief Converts a decimal string to a reversed hexadecimal string.
     *
     * This function takes a decimal string as input, converts it into a hexadecimal
     * string, and reverses the resulting hexadecimal string in groups of byteGroupSize.
     *
     * @param decimalStr The decimal string to convert.
     * @param byteGroupSize The grouping byteGroupSize for reversing the hexadecimal string.
     * @return The reversed hexadecimal string.
     */
    std::string decimalToReversedHex(const std::string& decimalStr, int byteGroupSize = 2);
    
    
    
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
    std::vector<std::string> findHexDataOffsets(const std::string& filePath, const std::string& hexData);
    
    
    
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
    void hexEditByOffset(const std::string& filePath, const std::string& offsetStr, const std::string& hexData);
    
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
    void hexEditByCustomOffset(const std::string& filePath, const std::string& customAsciiPattern, const std::string& offsetStr, const std::string& hexDataReplacement, size_t occurrence = 0);
    
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
    void hexEditFindReplace(const std::string& filePath, const std::string& hexDataToReplace, const std::string& hexDataReplacement, size_t occurrence = 0);

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
    std::string parseHexDataAtCustomOffset(const std::string& filePath, const std::string& customAsciiPattern, const std::string& offsetStr, size_t length, size_t occurrence = 0);
    
    
    
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
    
    std::string replaceHexPlaceholder(const std::string& arg, const std::string& hexPath);
    
    
    
    std::string extractVersionFromBinary(const std::string &filePath);
}

#endif
