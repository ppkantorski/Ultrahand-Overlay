#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include <cstdio> // Added for FILE and fopen
#include <cstring> // Added for std::memcmp
#include <sys/stat.h> // Added for stat

// Hex-editing commands
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

std::string decimalToReversedHex(const std::string& decimalStr, int order = 2) {
    std::string hexadecimal = decimalToHex(decimalStr);

    // Reverse the hexadecimal string in groups of order
    std::string reversedHex;
    for (int i = hexadecimal.length() - order; i >= 0; i -= order) {
        reversedHex += hexadecimal.substr(i, order);
    }

    return reversedHex;
}

std::vector<std::string> findHexDataOffsets(const std::string& filePath, const std::string& hexData) {
    std::vector<std::string> offsets;

    // Open the file for reading in binary mode
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        logMessage("Failed to open the file.");
        return offsets;
    }

    // Check the file size
    struct stat fileStatus;
    if (stat(filePath.c_str(), &fileStatus) != 0) {
        logMessage("Failed to retrieve file size.");
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
    const std::size_t bufferSize = 1024;
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

std::string readHexDataAtOffset(const std::string& filePath, const std::string& hexData, const std::string& offsetStr, size_t length) {
    // logMessage("Entered readHexDataAtOffset");

    std::vector<std::string> offsets = findHexDataOffsets(filePath, hexData);
    std::stringstream hexStream;
    char lowerToUpper;
    std::string result = "";
    char hexBuffer[length];
    int sum = 0;

    // Open the file for reading in binary mode
    FILE* file = fopen(filePath.c_str(), "rb");
    if (!file) {
        logMessage("Failed to open the file.");
        return "";
    }

    if (!offsets.empty()) {
        sum = std::stoi(offsetStr) + std::stoi(offsets[0]); // count from "C" letter
    }
    else {
        logMessage("CUST not found.");
    }

    if (fseek(file, sum, SEEK_SET) != 0) {
        logMessage("Error seeking to offset.");
        fclose(file);
        return "";
    }

    if (fread(hexBuffer, 1, length, file) == length) {
        for (size_t i = 0; i < length; ++i) {
            hexStream << std::setfill('0') << std::setw(2) << std::hex << static_cast<int>(hexBuffer[i]);
        }
    } else {
        if (feof(file)) {
            logMessage("End of file reached.");
        } else if (ferror(file)) {
            logMessage("Error reading data from file: ");
            logMessage(std::to_string(errno)); // Print the error description
        }
    }

    while (hexStream.get(lowerToUpper)) {
        result += std::toupper(lowerToUpper);
    }

    // logMessage("Hex data at offset:" + result);
    
    fclose(file);
    return result;
}

bool hexEditByOffset(const std::string& filePath, const std::string& offsetStr, const std::string& hexData) {
    // Convert the offset string to std::streampos
    std::streampos offset = std::stoll(offsetStr);

    // Open the file for reading and writing in binary mode
    FILE* file = fopen(filePath.c_str(), "rb+");
    if (!file) {
        logMessage("Failed to open the file.");
        return false;
    }

    // Move the file pointer to the specified offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        logMessage("Failed to move the file pointer.");
        fclose(file);
        return false;
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
        logMessage("Failed to read existing data from the file.");
        fclose(file);
        return false;
    }

    // Move the file pointer back to the offset
    if (fseek(file, offset, SEEK_SET) != 0) {
        logMessage("Failed to move the file pointer.");
        fclose(file);
        return false;
    }

    // Write the replacement binary data to the file
    if (fwrite(binaryData.data(), sizeof(unsigned char), bytesToReplace, file) != bytesToReplace) { // Changed to use unsigned char
        logMessage("Failed to write data to the file.");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
    //logMessage("Hex editing completed.");
}

bool hexEditFindReplace(const std::string& filePath, const std::string& hexDataToReplace, const std::string& hexDataReplacement, const std::string& occurrence = "0") {
    std::vector<std::string> offsetStrs = findHexDataOffsets(filePath, hexDataToReplace);
    if (!offsetStrs.empty()) {
        if (occurrence == "0") {
            // Replace all occurrences
            for (const std::string& offsetStr : offsetStrs) {
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            }
        }
        else {
            // Convert the occurrence string to an integer
            std::size_t index = std::stoul(occurrence); // Changed to use std::size_t
            if (index > 0 && index <= offsetStrs.size()) {
                // Replace the specified occurrence/index
                std::string offsetStr = offsetStrs[index - 1];
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            }
            else {
                return false;
                // Invalid occurrence/index specified
                logMessage("Invalid occurrence/index specified.");
            }
        }
        return true;
        //std::cout << "Hex data replaced successfully." << std::endl;
    }
    else {
        return false;
        logMessage("Hex data to replace not found.");
    }
}

bool hexEditCustOffset(const std::string& filePath, const std::string& offsetStr, const std::string& hexDataReplacement) {
    std::vector<std::string> offsetStrs = findHexDataOffsets(filePath, "43555354"); // 43555354 is a CUST
    if (!offsetStrs.empty()) {
        int sum = std::stoi(offsetStr) + std::stoi(offsetStrs[0]); // count from "C" letter
        hexEditByOffset(filePath, std::to_string(sum), hexDataReplacement);
    }
    else {
        return false;
        logMessage("CUST not found." );
    }
    return true;
}
