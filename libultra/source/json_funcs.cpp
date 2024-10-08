/********************************************************************************
 * File: json_funcs.cpp
 * Author: ppkantorski
 * Description:
 * This source file provides functions for working with JSON files in C++ using
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

#include "json_funcs.hpp"


namespace ult {

    /**
     * @brief Reads JSON data from a file and returns it as a `json_t` object.
     *
     * @param filePath The path to the JSON file.
     * @return A `json_t` object representing the parsed JSON data. Returns `nullptr` on error.
     */
    json_t* readJsonFromFile(const std::string& filePath) {
    #if NO_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "rb");  // Open the file in binary mode
        if (!file) {
            // Optionally log: Failed to open file
            return nullptr;
        }
    
        // Move to the end of the file to determine its size
        fseek(file, 0, SEEK_END);
        size_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);  // Move back to the start of the file
    
        std::string content;
        content.resize(fileSize);  // Reserve space in the string to avoid multiple allocations
    
        // Read the file into the string's buffer
        size_t bytesRead = fread(&content[0], 1, fileSize, file);
        if (bytesRead != fileSize) {
            // Optionally log: Failed to read file
            fclose(file);  // Close the file before returning
            return nullptr;
        }
    
        fclose(file);  // Close the file after reading
    #else
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
    #endif
    
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
    
    
    
    
    
    // Function to get a string from a JSON object
    std::string getStringFromJson(const json_t* root, const char* key) {
        const json_t* value = json_object_get(root, key);
        if (value && json_is_string(value)) {
            return json_string_value(value);
        } else {
            return ""; // Key not found or not a string, return empty string/char*
        }
    }
    
    
    /**
     * @brief Loads a JSON file from the specified path and retrieves a string value for a given key.
     *
     * This function combines the functionality of loading a JSON file and retrieving a string value associated
     * with a given key in a single step.
     *
     * @param filePath The path to the JSON file.
     * @param key The key whose associated string value is to be retrieved.
     * @return A string containing the value associated with the given key, or an empty string if the key is not found.
     */
    std::string getStringFromJsonFile(const std::string& filePath, const std::string& key) {
        // Load JSON from file using a smart pointer
        std::unique_ptr<json_t, JsonDeleter> root(readJsonFromFile(filePath), JsonDeleter());
        if (!root) {
            logMessage("Failed to load JSON file from path: " + filePath);
            return "";
        }
    
        // Retrieve the string value associated with the key
        json_t* jsonKey = json_object_get(root.get(), key.c_str());
        const char* value = json_is_string(jsonKey) ? json_string_value(jsonKey) : nullptr;
    
        // Check if the value was found and return it
        if (value) {
            return std::string(value);
        } else {
            logMessage("Key not found or not a string in JSON: " + key);
            return "";
        }
    }
}
