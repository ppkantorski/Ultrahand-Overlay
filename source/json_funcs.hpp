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
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <jansson.h>
#include <get_funcs.hpp>

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

    // Read the file content into a buffer
    char* buffer = static_cast<char*>(malloc(fileSize + 1));
    if (!buffer) {
        //fprintf(stderr, "Memory allocation error.\n");
        fclose(file);
        return nullptr;
    }

    size_t bytesRead = fread(buffer, 1, fileSize, file);
    buffer[bytesRead] = '\0';

    // Close the file
    fclose(file);

    // Parse the JSON data
    json_error_t error;
    json_t* root = json_loads(buffer, JSON_DECODE_ANY, &error);
    if (!root) {
        //fprintf(stderr, "Error parsing JSON: %s\n", error.text);
        free(buffer);
        return nullptr;
    }

    // Clean up
    free(buffer);

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
std::string replaceJsonPlaceholder(const std::string& arg, const std::string& commandName, const json_t* jsonDict) {
    
    //logMessage("arg: "+arg);
    //logMessage("commandName: "+commandName);
    
    std::string replacement = arg;
    std::string searchString = "{"+commandName+"(";
    
    
    std::size_t startPos = replacement.find(searchString);
    std::size_t endPos = replacement.find(")}");
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        std::string jsonSourcePathArgs = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
        std::vector<std::string> keys;
        std::string key;
        std::istringstream keyStream(jsonSourcePathArgs);
        while (std::getline(keyStream, key, ',')) {
            keys.push_back(trim(key));
        }
        
        // Traverse the JSON structure based on the keys
        auto current = jsonDict;
        for (const auto& key : keys) {
            if (json_is_object(current)) {
                current = json_object_get(current, key.c_str());
            } else if (json_is_array(current)) {
                if (key == "[]") {
                    size_t index = 0;
                    while (json_array_size(current) > index) {
                        json_t* arrayItem = json_array_get(current, index);
                        if (json_is_object(arrayItem)) {
                            current = arrayItem;
                            break;
                        }
                        ++index;
                    }
                } else {
                    size_t index = std::stoul(key);
                    if (index < json_array_size(current)) {
                        current = json_array_get(current, index);
                    } else {
                        // Handle invalid JSON array index
                        // printf("Invalid JSON array index: %s\n", key.c_str());
                        logMessage("Invalid JSON array index: "+key);
                        //json_decref(jsonDict);
                        return arg;  // Return the original placeholder if JSON array index is invalid
                    }
                }
            } else {
                // Handle invalid JSON structure or key
                // printf("Invalid JSON structure or key: %s\n", key.c_str());
                logMessage("Invalid JSON structure or key: "+key);
                //json_decref(jsonDict);
                return arg;  // Return the original placeholder if JSON structure or key is invalid
            }
        }
        
        if (json_is_string(current)) {
            std::string url = json_string_value(current);
            // Replace the entire placeholder with the URL
            replacement.replace(startPos, endPos - startPos + searchString.length() + 2, url);
        }
    }
    
    //json_decref(jsonDict);
    return replacement;
}

