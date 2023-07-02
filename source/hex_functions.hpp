#pragma once
#include <string>
#include <vector>
#include <algorithm>

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

std::string decimalToReversedHex(const std::string& decimalStr, int order=2) {
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
    std::size_t fileSize = fileStatus.st_size;

    // Convert the hex data string to binary data
    std::vector<char> binaryData;
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        binaryData.push_back(byte);
    }

    // Read the file in chunks to find the offsets where the hex data is located
    const std::size_t bufferSize = 1024;
    std::vector<char> buffer(bufferSize);
    std::streampos offset = 0;
    std::streampos bytesRead = 0;
    while ((bytesRead = fread(buffer.data(), sizeof(char), bufferSize, file)) > 0) {
        for (std::size_t i = 0; i < bytesRead; i++) {
            if (std::memcmp(buffer.data() + i, binaryData.data(), binaryData.size()) == 0) {
                std::streampos currentOffset = offset + i;
                offsets.push_back(std::to_string(currentOffset));
            }
        }
        offset += bytesRead;
    }

    fclose(file);
    return offsets;
}

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
    std::vector<char> binaryData;
    for (std::size_t i = 0; i < hexData.length(); i += 2) {
        std::string byteString = hexData.substr(i, 2);
        char byte = static_cast<char>(std::stoi(byteString, nullptr, 16));
        binaryData.push_back(byte);
    }

    // Calculate the number of bytes to be replaced
    std::size_t bytesToReplace = binaryData.size();

    // Read the existing data from the file
    std::vector<char> existingData(bytesToReplace);
    if (fread(existingData.data(), sizeof(char), bytesToReplace, file) != bytesToReplace) {
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
    if (fwrite(binaryData.data(), sizeof(char), bytesToReplace, file) != bytesToReplace) {
        //logMessage("Failed to write data to the file.");
        fclose(file);
        return;
    }

    fclose(file);
    //logMessage("Hex editing completed.");
}



void hexEditFindReplace(const std::string& filePath, const std::string& hexDataToReplace, const std::string& hexDataReplacement, const std::string& occurrence="0") {
    std::vector<std::string> offsetStrs = findHexDataOffsets(filePath, hexDataToReplace);
    if (!offsetStrs.empty()) {
        if (occurrence == "0") {
            // Replace all occurrences
            for (const std::string& offsetStr : offsetStrs) {
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            }
        } else {
            // Convert the occurrence string to an integer
            int index = std::stoi(occurrence);
            if (index > 0 && index <= offsetStrs.size()) {
                // Replace the specified occurrence/index
                std::string offsetStr = offsetStrs[index - 1];
                //logMessage("offsetStr: "+offsetStr);
                //logMessage("hexDataReplacement: "+hexDataReplacement);
                hexEditByOffset(filePath, offsetStr, hexDataReplacement);
            } else {
                // Invalid occurrence/index specified
                //std::cout << "Invalid occurrence/index specified." << std::endl;
            }
        }
        //std::cout << "Hex data replaced successfully." << std::endl;
    } else {
        //std::cout << "Hex data to replace not found." << std::endl;
    }
}
