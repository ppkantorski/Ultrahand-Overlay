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


#include <string>
#include <fstream>
#include <jansson.h>
#include "string_funcs.hpp"



// Define a custom deleter for json_t*
struct JsonDeleter {
    void operator()(json_t* json) const {
        if (json) {
            json_decref(json);
        }
    }
};


/**
 * @brief Reads JSON data from a file and returns it as a `json_t` object.
 *
 * @param filePath The path to the JSON file.
 * @return A `json_t` object representing the parsed JSON data. Returns `nullptr` on error.
 */
json_t* readJsonFromFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        // Optionally log: Failed to open file
        return nullptr;
    }

    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    std::string content;
    content.resize(fileSize);  // Reserve space in the string to avoid multiple allocations

    file.read(&content[0], fileSize);  // Read directly into the string's buffer
    if (file.fail() && !file.eof()) {
        // Optionally log: Failed to read file
        return nullptr;
    }

    file.close();  // Close the file after reading

    // Parse the JSON content
    json_error_t error;
    json_t* root = json_loads(content.c_str(), 0, &error);
    if (!root) {
        // Optionally log: JSON parsing error at line `error.line`: `error.text`
        return nullptr;
    }

    // Optionally log: JSON file successfully parsed
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
 * @param jsonPathOrString The path to the JSON file or the JSON string itself.
 * @return std::string The input string with the placeholder replaced by the actual JSON source,
 *                   or the original input string if replacement failed or jsonDict is nullptr.
 */
// Replace JSON placeholders in the string
std::string replaceJsonPlaceholder(const std::string& arg, const std::string& commandName, const std::string& jsonPathOrString) {
    std::unique_ptr<json_t, JsonDeleter> jsonDict;
    if (commandName == "json" || commandName == "json_source") {
        jsonDict.reset(stringToJson(jsonPathOrString));
    } else if (commandName == "json_file" || commandName == "json_file_source") {
        jsonDict.reset(readJsonFromFile(jsonPathOrString));
    }

    if (!jsonDict) {
        return arg; // Return original string if JSON data couldn't be loaded
    }

    std::string replacement = arg;
    const std::string searchString = "{" + commandName + "(";
    size_t startPos = replacement.find(searchString);

    // Declare variables outside the loop to avoid reinitialization
    size_t endPos = 0;
    size_t nextPos = 0;
    size_t commaPos = 0;
    size_t index ;
    std::string key;
    bool validValue = false;
    
    while (startPos != std::string::npos) {
        endPos = replacement.find(")}", startPos);
        if (endPos == std::string::npos) {
            break; // Break if no closing tag is found
        }

        nextPos = startPos + searchString.length();
        json_t* value = jsonDict.get(); // Get the JSON root object
        validValue = true;

        while (nextPos < endPos && validValue) {
            commaPos = replacement.find(',', nextPos);
            if (commaPos == std::string::npos || commaPos > endPos) {
                commaPos = endPos; // Set to endPos if no comma is found or it's beyond endPos
            }

            key = replacement.substr(nextPos, commaPos - nextPos); // Extract the key
            if (json_is_object(value)) {
                value = json_object_get(value, key.c_str()); // Navigate through object
            } else if (json_is_array(value)) {
                index = std::stoul(key); // Convert key to index for arrays
                value = json_array_get(value, index);
            } else {
                validValue = false; // Set validValue to false if value is neither object nor array
            }
            nextPos = commaPos + 1; // Move next position past the comma
        }

        if (validValue && value && json_is_string(value)) {
            replacement.replace(startPos, endPos + 2 - startPos, json_string_value(value)); // Replace text
        }

        startPos = replacement.find(searchString, endPos + 2); // Find next occurrence
    }

    return replacement; // Return the modified string
}



// Function to get a string from a JSON object
const char* getStringFromJson(const json_t* root, const char* key) {
    const json_t* value = json_object_get(root, key);
    if (value && json_is_string(value)) {
        return json_string_value(value);
    } else {
        return ""; // Key not found or not a string, return empty string/char*
    }
}

