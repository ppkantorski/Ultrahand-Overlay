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
#pragma once

#ifndef JSON_FUNCS_HPP
#define JSON_FUNCS_HPP

#if NO_FSTREAM_DIRECTIVE // For not using fstream (needs implementing)
#include <stdio.h>
#else
#include <fstream>
#endif

#include <string>
#include <jansson.h>
#include "string_funcs.hpp"


namespace ult {
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
    json_t* readJsonFromFile(const std::string& filePath);
    
    
    /**
     * @brief Parses a JSON string into a json_t object.
     *
     * This function takes a JSON string as input and parses it into a json_t object using Jansson library's `json_loads` function.
     * If parsing fails, it logs the error and returns nullptr.
     *
     * @param input The input JSON string to parse.
     * @return A json_t object representing the parsed JSON, or nullptr if parsing fails.
     */
    json_t* stringToJson(const std::string& input);
    
    
    // Function to get a string from a JSON object
    std::string getStringFromJson(const json_t* root, const char* key);
    
    
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
    std::string getStringFromJsonFile(const std::string& filePath, const std::string& key);
}

#endif