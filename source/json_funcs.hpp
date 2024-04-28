/********************************************************************************
 * File: json_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file provides functions for working with JSON files in C++ using
 *   the `jansson` library. It includes a function to read JSON data from a file.
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

#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <jansson.h>
#include <get_funcs.hpp>

//constexpr size_t jsonBufferSize = 1024; // Choose an appropriate buffer size

/**
 * @brief Reads JSON data from a file and returns it as a `json_t` object.
 *
 * @param filePath The path to the JSON file.
 * @return A `json_t` object representing the parsed JSON data. Returns `nullptr` on error.
 */
json_t* readJsonFromFile(const std::string& filePath) {
    // Check file existence and size
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0 || fileStat.st_size == 0) {
        //logMessage("File does not exist or is empty: " + filePath);
        return nullptr;
    }

    // Open the file
    FILE* file = fopen(filePath.c_str(), "rb"); // Open in binary mode to ensure no character translation
    if (!file) {
        //logMessage("Failed to open file: " + filePath);
        return nullptr;
    }

    // Allocate memory based on file size
    char* buffer = new (std::nothrow) char[fileStat.st_size + 1];
    if (!buffer) {
        fclose(file);
        //logMessage("Memory allocation failed for reading file: " + filePath);
        return nullptr;
    }

    // Read the entire file into the buffer
    size_t bytesRead = fread(buffer, 1, fileStat.st_size, file);
    if (bytesRead < static_cast<size_t>(fileStat.st_size)) {
        fclose(file);
        delete[] buffer;
        logMessage("Failed to read the entire file: " + filePath);
        return nullptr;
    }

    // Null-terminate the buffer to make it a valid C-string
    buffer[bytesRead] = '\0';

    // Parse the JSON content
    json_error_t error;
    json_t* root = json_loads(buffer, 0, &error);
    if (!root) {
        //logMessage("JSON parsing error at line " + std::to_string(error.line) + ": " + error.text);
    } else {
        //logMessage("JSON file successfully parsed.");
    }

    // Clean up
    fclose(file);
    delete[] buffer;

    return root;
}


// Global mutex for thread-safe JSON operations
std::mutex jsonMutex;

/**
 * @brief Replaces a JSON source placeholder with the actual JSON source.
 *
 * @param arg The input string containing the placeholder.
 * @param commandName The name of the JSON command (e.g., "json", "json_file").
 * @param jsonDict A pointer to the JSON object from which to extract the source.
 *                If not provided (default nullptr), no JSON replacement will occur.
 * @return std::string The input string with the placeholder replaced by the actual JSON source,
 *                   or the original input string if replacement failed or jsonDict is nullptr.
 */
std::string replaceJsonPlaceholder(const std::string& arg, const std::string& commandName, const std::string& jsonPathOrString) {
    json_t* jsonDict = nullptr;
    
    // Use std::lock_guard to ensure thread safety when accessing JSON data
    std::lock_guard<std::mutex> lock(jsonMutex);
    
    if (commandName == "json" || commandName == "json_source") {
        jsonDict = stringToJson(jsonPathOrString);
    } else if (commandName == "json_file" || commandName == "json_file_source") {
        jsonDict = readJsonFromFile(jsonPathOrString); // Using the function to read JSON from a file
    }
    
    if (!jsonDict) {
        return arg; // Return the original string if JSON parsing failed or jsonDict is nullptr
    }
    
    std::string replacement = arg;
    std::string searchString = "{" + commandName + "(";
    size_t startPos = replacement.find(searchString);
    size_t endPos, nextPos, commaPos, len, _index;
    bool validValue;
    std::vector<std::string> keysAndIndexes;
    keysAndIndexes.reserve(5); // Reserve capacity for keysAndIndexes vector
    
    while (startPos != std::string::npos) {
        keysAndIndexes.clear(); // Clear the vector for reuse
        endPos = replacement.find(")}", startPos);
        if (endPos == std::string::npos) {
            break;  // Missing closing brace, exit the loop
        }
        
        std::string placeholder = replacement.substr(startPos, endPos - startPos + 2);
        
        // Extract keys and indexes from the placeholder
        nextPos = startPos + searchString.length();
        
        while (nextPos < endPos) {
            commaPos = replacement.find(',', nextPos);
            len = (commaPos != std::string::npos) ? (commaPos - nextPos) : (endPos - nextPos);
            keysAndIndexes.emplace_back(replacement.substr(nextPos, len));
            nextPos += len + 1;
        }
        
        json_t* value = jsonDict;
        validValue = true;
        for (const std::string& keyIndex : keysAndIndexes) {
            if (json_is_object(value)) {
                value = json_object_get(value, keyIndex.c_str());
            } else if (json_is_array(value)) {
                _index = std::stoul(keyIndex);
                value = json_array_get(value, _index);
            } else {
                validValue = false;
                break; // Invalid JSON structure, exit the loop
            }
        }
        
        if (validValue && value != nullptr && json_is_string(value)) {
            // Replace the placeholder with the JSON value
            replacement.replace(startPos, endPos - startPos + 2, json_string_value(value));
        }
        
        // Move to the next placeholder
        startPos = replacement.find(searchString, endPos);
    }
    
    // Free JSON data if it's not already freed
    if (jsonDict != nullptr) {
        json_decref(jsonDict);
    }
    
    return replacement;
}



