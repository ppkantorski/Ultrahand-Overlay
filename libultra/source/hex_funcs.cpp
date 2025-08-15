/********************************************************************************
 * File: hex_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file implements the functions declared in hex_funcs.hpp.
 *   These functions provide support for manipulating hexadecimal data,
 *   including conversions between ASCII and hexadecimal strings,
 *   locating specific hex patterns within files, and editing file contents
 *   at hex offsets.
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

#include "hex_funcs.hpp"

namespace ult {
    size_t HEX_BUFFER_SIZE = 4096;//65536/4;
    
    // Thread-safe cache and file operation mutexes
    std::shared_mutex cacheMutex;  // Allows multiple readers, single writer
    std::mutex fileWriteMutex;     // Protects file write operations
    
    // For improving the speed of hexing consecutively with the same file and asciiPattern.
    std::unordered_map<std::string, std::string> hexSumCache;
    
    
    /**
     * @brief Thread-safe cache management functions
     */
    void clearHexSumCache() {
        std::lock_guard<std::shared_mutex> writeLock(cacheMutex);
        //hexSumCache.clear();
        hexSumCache = {};
    }

    size_t getHexSumCacheSize() {
        std::shared_lock<std::shared_mutex> readLock(cacheMutex);
        return hexSumCache.size();
    }

    /**
     * @brief Converts an ASCII string to a hexadecimal string.
     *
     * This function takes an ASCII string as input and converts it into a hexadecimal string.
     *
     * @param asciiStr The ASCII string to convert.
     * @return The corresponding hexadecimal string.
     */
    
    
    // Function to convert ASCII string to Hex string
    std::string asciiToHex(const std::string& asciiStr) {
        std::string hexStr;
        //hexStr.reserve(asciiStr.length() * 2); // Reserve space for the hexadecimal string
    
        for (unsigned char c : asciiStr) {
            hexStr.push_back(hexLookup[c >> 4]); // High nibble
            hexStr.push_back(hexLookup[c & 0x0F]); // Low nibble
        }
    
        return hexStr;
    }
    
    /**
     * @brief Converts a decimal string to a fixed-width hexadecimal string.
     *
     * @param decimalStr The decimal string to convert.
     * @param byteGroupSize The number of hex digits to output (must be even for byte alignment).
     * @return Hex string of exactly 'byteGroupSize' digits, or empty string if value doesn't fit.
     */
    std::string decimalToHex(const std::string& decimalStr, int byteGroupSize) {
        const int decimalValue = ult::stoi(decimalStr);
        if (decimalValue < 0 || byteGroupSize <= 0 || (byteGroupSize % 2) != 0) {
            // Invalid input: negative number, or byteGroupSize <= 0, or odd byteGroupSize
            return "";
        }
    
        // Special case: zero
        if (decimalValue == 0) {
            return std::string(byteGroupSize, '0');
        }
    
        // Convert decimalValue to hex (uppercase, minimal length)
        std::string hex;
        int tempValue = decimalValue;
        int remainder;
        char hexChar;
        while (tempValue > 0) {
            remainder = tempValue % 16;
            hexChar = (remainder < 10) ? ('0' + remainder) : ('A' + remainder - 10);
            hex.insert(hex.begin(), hexChar);
            tempValue /= 16;
        }
    
        // Ensure hex length is even by adding leading zero if needed
        if (hex.length() % 2 != 0) {
            hex.insert(hex.begin(), '0');
        }
    
        // Minimum size needed to fit hex string
        const size_t hexLen = hex.length();
    
        // Adjust minimum byteGroupSize to be at least hexLen
        size_t minByteGroupSize = std::max(static_cast<size_t>(byteGroupSize), hexLen);
    
        // If byteGroupSize was too small, adjust to hex length (must be even)
        if (minByteGroupSize % 2 != 0) {
            minByteGroupSize++;
        }
    
        // If minByteGroupSize is less than hex length, number doesn't fit
        if (minByteGroupSize < hexLen) {
            return ""; // can't fit
        }
    
        // Pad with leading zeros to match minByteGroupSize
        if (hexLen < minByteGroupSize) {
            hex.insert(hex.begin(), minByteGroupSize - hexLen, '0');
        }
    
        return hex;
    }
    
    
    /**
     * @brief Converts a hexadecimal string to a decimal string.
     *
     * This function takes a hexadecimal string as input and converts it into a decimal string.
     *
     * @param hexStr The hexadecimal string to convert.
     * @return The corresponding decimal string.
     */
    std::string hexToDecimal(const std::string& hexStr) {
        // Convert hexadecimal string to integer
        int decimalValue = 0;
        const size_t len = hexStr.length();
        
        char hexChar;
        int value;

        // Iterate over each character in the hexadecimal string
        for (size_t i = 0; i < len; ++i) {
            hexChar = hexStr[i];
            //int value;
    
            // Convert hex character to its decimal value
            if (hexChar >= '0' && hexChar <= '9') {
                value = hexChar - '0';
            } else if (hexChar >= 'A' && hexChar <= 'F') {
                value = 10 + (hexChar - 'A');
            } else if (hexChar >= 'a' && hexChar <= 'f') {
                value = 10 + (hexChar - 'a');
            } else {
                break;
                //throw std::invalid_argument("Invalid hexadecimal character");
            }
    
            // Update the decimal value
            decimalValue = decimalValue * 16 + value;
        }
    
        // Convert the decimal value to a string
        return ult::to_string(decimalValue);
    }
    
    
    
    std::string hexToReversedHex(const std::string& hexadecimal, int order) {
        // Reverse the hexadecimal string in groups of order
        std::string reversedHex;
        for (int i = hexadecimal.length() - order; i >= 0; i -= order) {
            reversedHex += hexadecimal.substr(i, order);
        }
        
        return reversedHex;
    }
    
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
    std::string decimalToReversedHex(const std::string& decimalStr, int byteGroupSize) {
        //std::string hexadecimal = decimalToHex(decimalStr, byteGroupSize);
        
        // Reverse the hexadecimal string in groups of byteGroupSize
        //std::string reversedHex;
        //for (int i = hexadecimal.length() - byteGroupSize; i >= 0; i -= byteGroupSize) {
        //    reversedHex += hexadecimal.substr(i, byteGroupSize);
        //}
        
        return hexToReversedHex(decimalToHex(decimalStr, byteGroupSize));
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
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "rb");
        if (!file) {
            return offsets;
        }
    
        fseek(file, 0, SEEK_END);
        const size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
    
        std::vector<unsigned char> binaryData;
        if (hexData.length() % 2 != 0) {
            fclose(file);
            return offsets;
        }
        
        
        const size_t hexLen = hexData.length();
        binaryData.resize(hexLen / 2);
        const unsigned char* hexPtr = reinterpret_cast<const unsigned char*>(hexData.c_str());
        
        // Unrolled hex conversion loop
        size_t i = 0;
        for (; i + 4 <= hexLen; i += 4) {
            binaryData[i/2] = (hexTable[hexPtr[i]] << 4) | hexTable[hexPtr[i + 1]];
            binaryData[i/2 + 1] = (hexTable[hexPtr[i + 2]] << 4) | hexTable[hexPtr[i + 3]];
        }
        // Handle remaining bytes
        for (; i < hexLen; i += 2) {
            binaryData[i/2] = (hexTable[hexPtr[i]] << 4) | hexTable[hexPtr[i + 1]];
        }
    
        // Optimized search variables
        const unsigned char* patternPtr = binaryData.data();
        const size_t patternLen = binaryData.size();
        const unsigned char firstByte = patternPtr[0];
        
        std::vector<unsigned char> buffer(HEX_BUFFER_SIZE);
        size_t bytesRead = 0;
        size_t offset = 0;
        

        while ((bytesRead = fread(buffer.data(), 1, HEX_BUFFER_SIZE, file)) > 0) {
            const unsigned char* bufPtr = buffer.data();
            
            // Optimized search with first-byte filtering and loop unrolling
            i = 0;
            const size_t searchEnd = bytesRead;
            
            // Process 4 bytes at a time for better cache usage
            for (; i + 4 <= searchEnd; i += 4) {
                // Check 4 positions at once
                if (bufPtr[i] == firstByte) {
                    if (offset + i + patternLen <= fileSize && 
                        memcmp(bufPtr + i, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i));
                    }
                }
                if (bufPtr[i + 1] == firstByte) {
                    if (offset + i + 1 + patternLen <= fileSize && 
                        memcmp(bufPtr + i + 1, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i + 1));
                    }
                }
                if (bufPtr[i + 2] == firstByte) {
                    if (offset + i + 2 + patternLen <= fileSize && 
                        memcmp(bufPtr + i + 2, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i + 2));
                    }
                }
                if (bufPtr[i + 3] == firstByte) {
                    if (offset + i + 3 + patternLen <= fileSize && 
                        memcmp(bufPtr + i + 3, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i + 3));
                    }
                }
            }
            
            // Handle remaining bytes
            for (; i < searchEnd; ++i) {
                if (bufPtr[i] == firstByte) {
                    if (offset + i + patternLen <= fileSize && 
                        memcmp(bufPtr + i, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i));
                    }
                }
            }
            
            offset += bytesRead;
        }
    
        fclose(file);
        
    #else
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            return offsets;
        }
    
        file.seekg(0, std::ios::end);
        const size_t fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
    
        std::vector<unsigned char> binaryData;
        if (hexData.length() % 2 != 0) {
            file.close();
            return offsets;
        }
        
        
        const size_t hexLen = hexData.length();
        binaryData.resize(hexLen / 2);
        const unsigned char* hexPtr = reinterpret_cast<const unsigned char*>(hexData.c_str());
        
        size_t i = 0;
        for (; i + 4 <= hexLen; i += 4) {
            binaryData[i/2] = (hexTable[hexPtr[i]] << 4) | hexTable[hexPtr[i + 1]];
            binaryData[i/2 + 1] = (hexTable[hexPtr[i + 2]] << 4) | hexTable[hexPtr[i + 3]];
        }
        for (; i < hexLen; i += 2) {
            binaryData[i/2] = (hexTable[hexPtr[i]] << 4) | hexTable[hexPtr[i + 1]];
        }
    
        const unsigned char* patternPtr = binaryData.data();
        const size_t patternLen = binaryData.size();
        const unsigned char firstByte = patternPtr[0];
        
        std::vector<unsigned char> buffer(HEX_BUFFER_SIZE);
        size_t bytesRead = 0;
        size_t offset = 0;
    
        while (file.read(reinterpret_cast<char*>(buffer.data()), HEX_BUFFER_SIZE) || file.gcount() > 0) {
            bytesRead = file.gcount();
            const unsigned char* bufPtr = buffer.data();
            
            // Same optimized search as FILE* version
            i = 0;
            const size_t searchEnd = bytesRead;
            
            for (; i + 4 <= searchEnd; i += 4) {
                if (bufPtr[i] == firstByte) {
                    if (offset + i + patternLen <= fileSize && 
                        memcmp(bufPtr + i, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i));
                    }
                }
                if (bufPtr[i + 1] == firstByte) {
                    if (offset + i + 1 + patternLen <= fileSize && 
                        memcmp(bufPtr + i + 1, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i + 1));
                    }
                }
                if (bufPtr[i + 2] == firstByte) {
                    if (offset + i + 2 + patternLen <= fileSize && 
                        memcmp(bufPtr + i + 2, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i + 2));
                    }
                }
                if (bufPtr[i + 3] == firstByte) {
                    if (offset + i + 3 + patternLen <= fileSize && 
                        memcmp(bufPtr + i + 3, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i + 3));
                    }
                }
            }
            
            for (; i < searchEnd; ++i) {
                if (bufPtr[i] == firstByte) {
                    if (offset + i + patternLen <= fileSize && 
                        memcmp(bufPtr + i, patternPtr, patternLen) == 0) {
                        offsets.emplace_back(ult::to_string(offset + i));
                    }
                }
            }
            
            offset += bytesRead;
        }
    
        file.close();
    #endif
    
        return offsets;
    }

    
    
    /**
     * @brief Edits hexadecimal data in a file at a specified offset.
     *
     * This function opens a binary file, seeks to a specified offset, and replaces
     * the data at that offset with the provided hexadecimal data.
     *
     * @param filePath The path to the binary file.
     * @param offsetStr The offset in the file to perform the edit.
     * @param hexData The hexadecimal data to replace at the offset.
     */
    void hexEditByOffset(const std::string& filePath, const std::string& offsetStr, const std::string& hexData) {
        // Lock file writes to prevent concurrent modifications to the same file
        std::lock_guard<std::mutex> fileWriteLock(fileWriteMutex);

        const std::streampos offset = std::stoll(offsetStr);
    
    #if !USING_FSTREAM_DIRECTIVE
        // Open the file for both reading and writing in binary mode
        FILE* file = fopen(filePath.c_str(), "rb+");
        if (!file) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the file.");
            #endif
            return;
        }
    
        // Retrieve the file size
        fseek(file, 0, SEEK_END);
        const std::streampos fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
    
        if (offset >= fileSize) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Offset exceeds file size.");
            #endif
            fclose(file);
            return;
        }
    
        // Convert the hex string to binary data
        std::vector<unsigned char> binaryData(hexData.length() / 2);
        std::string byteString;
        for (size_t i = 0, j = 0; i < hexData.length(); i += 2, ++j) {
            byteString = hexData.substr(i, 2);
            binaryData[j] = static_cast<unsigned char>(ult::stoi(byteString, nullptr, 16));
        }
    
        // Move to the specified offset and write the binary data directly to the file
        fseek(file, offset, SEEK_SET);
        const size_t bytesWritten = fwrite(binaryData.data(), sizeof(unsigned char), binaryData.size(), file);
        if (bytesWritten != binaryData.size()) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to write data to the file.");
            #endif
            fclose(file);
            return;
        }
    
        fclose(file);
    #else
        // Open the file for both reading and writing in binary mode
        std::fstream file(filePath, std::ios::binary | std::ios::in | std::ios::out);
        if (!file.is_open()) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the file.");
            #endif
            return;
        }
    
        // Retrieve the file size
        file.seekg(0, std::ios::end);
        const std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
    
        if (offset >= fileSize) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Offset exceeds file size.");
            #endif
            return;
        }
    
        // Convert the hex string to binary data
        std::vector<unsigned char> binaryData(hexData.length() / 2);
        std::string byteString;
        for (size_t i = 0, j = 0; i < hexData.length(); i += 2, ++j) {
            byteString = hexData.substr(i, 2);
            binaryData[j] = static_cast<unsigned char>(ult::stoi(byteString, nullptr, 16));
        }
    
        // Move to the specified offset and write the binary data directly to the file
        file.seekp(offset);
        file.write(reinterpret_cast<const char*>(binaryData.data()), binaryData.size());
        if (!file) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to write data to the file.");
            #endif
            return;
        }
    
        file.close();
    #endif
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
    void hexEditByCustomOffset(const std::string& filePath, const std::string& customAsciiPattern, const std::string& offsetStr, const std::string& hexDataReplacement, size_t occurrence) {
        
        // Create a cache key based on filePath and customAsciiPattern
        const std::string cacheKey = filePath + '?' + customAsciiPattern + '?' + ult::to_string(occurrence);
        
        int hexSum = -1;
        
        // Thread-safe cache access
        {
            std::shared_lock<std::shared_mutex> readLock(cacheMutex);
            const auto cachedResult = hexSumCache.find(cacheKey);
            if (cachedResult != hexSumCache.end()) {
                hexSum = ult::stoi(cachedResult->second);
            }
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
            const std::vector<std::string> offsets = findHexDataOffsets(filePath, customHexPattern);
            
            if (!offsets.empty()) {
                hexSum = ult::stoi(offsets[occurrence]);
                
                // Thread-safe cache write
                {
                    std::lock_guard<std::shared_mutex> writeLock(cacheMutex);
                    hexSumCache[cacheKey] = ult::to_string(hexSum);
                }
            } else {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Offset not found.");
                #endif
                return;
            }
        }
        
        
        if (hexSum != -1) {
            // Calculate the total offset to seek in the file
            //int sum = hexSum + ult::stoi(offsetStr);
            hexEditByOffset(filePath, ult::to_string(hexSum + ult::stoi(offsetStr)), hexDataReplacement);
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to find " + customAsciiPattern + ".");
            #endif
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
    void hexEditFindReplace(const std::string& filePath, const std::string& hexDataToReplace, const std::string& hexDataReplacement, size_t occurrence) {
        const std::vector<std::string> offsetStrs = findHexDataOffsets(filePath, hexDataToReplace);
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
                    //std::string offsetStr = offsetStrs[occurrence - 1];
                    //logMessage("offsetStr: "+offsetStr);
                    //logMessage("hexDataReplacement: "+hexDataReplacement);
                    hexEditByOffset(filePath, offsetStrs[occurrence - 1], hexDataReplacement);
                } else {
                    // Invalid occurrence/index specified
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Invalid hex occurrence/index specified.");
                    #endif
                }
            }
            //std::cout << "Hex data replaced successfully." << std::endl;
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
    std::string parseHexDataAtCustomOffset(const std::string& filePath, const std::string& customAsciiPattern, 
                                         const std::string& offsetStr, size_t length, size_t occurrence) {
        const std::string cacheKey = filePath + '?' + customAsciiPattern + '?' + ult::to_string(occurrence);
        int hexSum = -1;

        // Thread-safe cache read
        {
            std::shared_lock<std::shared_mutex> readLock(cacheMutex);
            const auto cachedResult = hexSumCache.find(cacheKey);
            if (cachedResult != hexSumCache.end()) {
                hexSum = ult::stoi(cachedResult->second);
            }
        }

        if (hexSum == -1) {
            const std::string customHexPattern = asciiToHex(customAsciiPattern);
            const std::vector<std::string> offsets = findHexDataOffsets(filePath, customHexPattern);

            if (!offsets.empty() && offsets.size() > occurrence) {
                hexSum = ult::stoi(offsets[occurrence]);
                
                // Thread-safe cache write
                {
                    std::lock_guard<std::shared_mutex> writeLock(cacheMutex);
                    hexSumCache[cacheKey] = ult::to_string(hexSum);
                }
            } else {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Offset not found.");
                #endif
                return "";
            }
        }

        const std::streampos totalOffset = hexSum + std::stoll(offsetStr);
        std::vector<char> hexBuffer(length);
        std::vector<char> hexStream(length * 2);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "rb");
        if (!file) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the file.");
            #endif
            return "";
        }

        if (fseek(file, totalOffset, SEEK_SET) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error seeking to offset.");
            #endif
            fclose(file);
            return "";
        }

        const size_t bytesRead = fread(hexBuffer.data(), sizeof(char), length, file);
        if (bytesRead == length) {
            static constexpr char hexDigits[] = "0123456789ABCDEF";
            for (size_t i = 0; i < length; ++i) {
                hexStream[i * 2] = hexDigits[(hexBuffer[i] >> 4) & 0xF];
                hexStream[i * 2 + 1] = hexDigits[hexBuffer[i] & 0xF];
            }
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error reading data from file or end of file reached.");
            #endif
            fclose(file);
            return "";
        }

        fclose(file);
    #else
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the file.");
            #endif
            return "";
        }

        file.seekg(totalOffset);
        if (!file) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error seeking to offset.");
            #endif
            return "";
        }

        file.read(hexBuffer.data(), length);
        if (file.gcount() == static_cast<std::streamsize>(length)) {
            static constexpr char hexDigits[] = "0123456789ABCDEF";
            for (size_t i = 0; i < length; ++i) {
                hexStream[i * 2] = hexDigits[(hexBuffer[i] >> 4) & 0xF];
                hexStream[i * 2 + 1] = hexDigits[hexBuffer[i] & 0xF];
            }
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error reading data from file or end of file reached.");
            #endif
            return "";
        }

        file.close();
    #endif

        std::string result(hexStream.begin(), hexStream.end());
        result = stringToUppercase(result);

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
        const std::string searchString = "{hex_file(";
        
        const size_t startPos = replacement.find(searchString);
        const size_t endPos = replacement.find(")}");
        
        if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
            const std::string placeholderContent = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
            
            // Split the placeholder content into its components (customAsciiPattern, offsetStr, length)
            std::vector<std::string> components;
            
            // Use StringStream instead of std::istringstream
            StringStream componentStream(placeholderContent);
            std::string component;
            
            while (componentStream.getline(component, ',')) {
                trim(component);
                components.push_back(component);
            }
            
            if (components.size() == 3) {
                // Extract individual components
                const std::string customAsciiPattern = components[0];
                const std::string offsetStr = components[1];
                const size_t length = std::stoul(components[2]);
                
                // Call the parsing function and replace the placeholder
                const std::string parsedResult = parseHexDataAtCustomOffset(hexPath, customAsciiPattern, offsetStr, length);
                
                // Only replace if parsedResult returns a non-empty string
                if (!parsedResult.empty()) {
                    // Replace the entire placeholder with the parsed result
                    replacement.replace(startPos, endPos - startPos + searchString.length() + 2, parsedResult);
                }
            }
        }
        
        return replacement;
    }

    
    
        
    /**
     * @brief Extracts the version string from a binary file.
     *
     * This function reads a binary file and searches for a version pattern
     * in the format "v#.#.#" (e.g., "v1.2.3").
     *
     * @param filePath The path to the binary file.
     * @return The version string if found; otherwise, an empty string.
     */
    std::string extractVersionFromBinary(const std::string &filePath) {
    #if !USING_FSTREAM_DIRECTIVE
        // Step 1: Open the binary file
        FILE* file = fopen(filePath.c_str(), "rb");
        if (!file) {
            return ""; // Return empty string if file cannot be opened
        }
    
        // Get the file size
        fseek(file, 0, SEEK_END);
        const std::streamsize size = ftell(file);
        fseek(file, 0, SEEK_SET);
    
        // Read the entire file into a buffer
        std::vector<uint8_t> buffer(size);
        const size_t bytesRead = fread(buffer.data(), sizeof(uint8_t), size, file);
        fclose(file); // Close the file after reading
        
        if (bytesRead != static_cast<size_t>(size)) {
            return ""; // Return empty string if reading fails
        }
    #else
        // Step 1: Read the entire binary file into a vector
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return ""; // Return empty string if file cannot be opened
        }
    
        const std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
    
        std::vector<uint8_t> buffer(size);
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            return ""; // Return empty string if reading fails
        }
    #endif
    
        // Step 2: Search for the pattern "v#.#.#"
        const char* data = reinterpret_cast<const char*>(buffer.data());
        for (std::streamsize i = 0; i < size; ++i) {
            if (data[i] == 'v' && i + 5 < size && 
                std::isdigit(data[i + 1]) && data[i + 2] == '.' && 
                std::isdigit(data[i + 3]) && data[i + 4] == '.' && 
                std::isdigit(data[i + 5])) {
    
                // Extract the version string
                return std::string(data + i, 6); // Return the version string
            }
        }
    
        return "";  // Return empty string if no match is found
    }
}
