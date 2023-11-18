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
 *  Copyright (c) 2023 ppkantorski
 *  Licensed under CC BY-NC-SA 4.0
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
std::string replaceJsonPlaceholder(const std::string& arg, const std::string& commandName, const std::string& jsonPathOrString) {
    json_t* jsonDict = nullptr;
    json_error_t error;

    if (commandName == "json" || commandName == "json_source") {
        jsonDict = stringToJson(jsonPathOrString);
    } else if (commandName == "json_file" || commandName == "json_file_source") {
        jsonDict = json_load_file(jsonPathOrString.c_str(), 0, &error);
    }

    std::string replacement = arg;
    std::string searchString = "{" + commandName + "(";
    std::size_t startPos = replacement.find(searchString);

    while (startPos != std::string::npos) {
        std::size_t endPos = replacement.find(")}", startPos);
        if (endPos == std::string::npos) {
            break;  // Missing closing brace, exit the loop
        }

        std::string placeholder = replacement.substr(startPos, endPos - startPos + 2);

        // Extract keys and indexes from the placeholder
        std::vector<std::string> keysAndIndexes;
        size_t nextPos = startPos + searchString.length();

        while (nextPos < endPos) {
            size_t commaPos = replacement.find(',', nextPos);
            size_t len = (commaPos != std::string::npos) ? (commaPos - nextPos) : (endPos - nextPos);
            keysAndIndexes.push_back(replacement.substr(nextPos, len));
            nextPos += len + 1;
        }

        json_t* value = jsonDict;
        for (const std::string& keyIndex : keysAndIndexes) {
            if (json_is_object(value)) {
                value = json_object_get(value, keyIndex.c_str());
            } else if (json_is_array(value)) {
                size_t index = std::stoul(keyIndex);
                value = json_array_get(value, index);
            }

            if (value == nullptr) {
                // Key or index not found, stop further processing
                break;
            }
        }

        if (value != nullptr && json_is_string(value)) {
            // Replace the placeholder with the JSON value
            replacement.replace(startPos, endPos - startPos + 2, json_string_value(value));
        }

        // Move to the next placeholder
        startPos = replacement.find(searchString, endPos);
    }

    // Free JSON data
    if (jsonDict != nullptr) {
        json_decref(jsonDict);
    }

    return replacement;
}


