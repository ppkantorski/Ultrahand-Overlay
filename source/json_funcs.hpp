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
#include <fstream>
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
    // Open the file
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        //logMessage("Failed to open file: " + filePath);
        return nullptr;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    if (fileSize == 0) {
        //logMessage("File is empty: " + filePath);
        file.close();
        return nullptr;
    }

    // Allocate memory based on file size
    std::vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);

    if (file.gcount() != fileSize) {
        //logMessage("Failed to read the entire file: " + filePath);
        file.close();
        return nullptr;
    }

    // Null-terminate the buffer to make it a valid C-string
    buffer.push_back('\0');

    // Parse the JSON content
    json_error_t error;
    json_t* root = json_loads(buffer.data(), 0, &error);
    if (!root) {
        //logMessage("JSON parsing error at line " + std::to_string(error.line) + ": " + error.text);
    } else {
        //logMessage("JSON file successfully parsed.");
    }

    // Clean up
    file.close();

    return root;
}


/**
 * @brief Parses a JSON string into a json_t object.
 *
 * This function takes a JSON string as input and parses it into a json_t object using Jansson library's `json_loads` function.
 * If parsing fails, it logs the error and returns nullptr.
 *
 * @param input The input JSON string to parse.
 * @return A json_t object representing the parsed JSON, or nullptr if parsing fails.
 */
json_t* stringToJson(const std::string& input) {
    json_error_t error;
    json_t* jsonObj = json_loads(input.c_str(), 0, &error);

    if (!jsonObj) {
        logMessage("Failed to parse JSON: " + std::string(error.text) + " at line " + std::to_string(error.line));
        return nullptr; // Return nullptr to indicate failure clearly
    }

    return jsonObj;
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




