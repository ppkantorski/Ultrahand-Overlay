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
    // Check if the file exists
    struct stat fileStat;
    if (stat(filePath.c_str(), &fileStat) != 0) {
        //fprintf(stderr, "Error opening file: %s\n", filePath.c_str());
        return nullptr;
    }
    
    // Open the file
    FILE* file = fopen(filePath.c_str(), "r");
    if (!file) {
        //fprintf(stderr, "Error opening file: %s\n", filePath.c_str());
        return nullptr;
    }
    
    // Get the file size
    size_t fileSize = fileStat.st_size;
    
    // Use a fixed-size buffer for reading file content
    constexpr size_t bufferSize = 4096; // Choose an appropriate buffer size
    char buffer[bufferSize];
    std::string jsonContent;
    
    // Read the file content into the buffer
    while (!feof(file)) {
        size_t bytesRead = fread(buffer, 1, bufferSize, file);
        jsonContent.append(buffer, bytesRead);
    }
    
    // Close the file
    fclose(file);
    
    // Parse the JSON data
    json_error_t error;
    json_t* root = json_loads(jsonContent.c_str(), JSON_DECODE_ANY, &error);
    if (!root) {
        //fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        return nullptr;
    }
    
    return root;
}



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
    json_error_t error;
    
    if (commandName == "json" || commandName == "json_source") {
        jsonDict = stringToJson(jsonPathOrString);
    } else if (commandName == "json_file" || commandName == "json_file_source") {
        jsonDict = json_load_file(jsonPathOrString.c_str(), 0, &error);
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



