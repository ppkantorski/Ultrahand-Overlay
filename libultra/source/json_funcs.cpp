/********************************************************************************
 * File: json_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file provides functions for working with JSON files in C++ using
 *   the `cJSON` library. It includes a function to read JSON data from a file.
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

    static std::mutex json_access_mutex;

    /**
     * @brief Reads JSON data from a file and returns it as a `json_t` object.
     *
     * @param filePath The path to the JSON file.
     * @return A `json_t` object representing the parsed JSON data. Returns `nullptr` on error.
     */
    json_t* readJsonFromFile(const std::string& filePath) {
        std::lock_guard<std::mutex> lock(json_access_mutex);
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "rb");
        if (!file) {
            return nullptr;
        }
    
        // Get file size
        fseek(file, 0, SEEK_END);
        const long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        // Check for reasonable file size
        if (fileSize <= 0 || fileSize > 6 * 1024 * 1024) { // 6MB limit
            fclose(file);
            return nullptr;
        }
    
        // Use vector<char> for better performance and explicit null termination
        std::vector<char> buffer;
        buffer.resize(static_cast<size_t>(fileSize) + 1); // +1 for null terminator
    
        // Read the file in one operation
        const size_t bytesRead = fread(buffer.data(), 1, static_cast<size_t>(fileSize), file);
        fclose(file);
        
        if (bytesRead != static_cast<size_t>(fileSize)) {
            return nullptr;
        }
        
        // Ensure null termination for cJSON
        buffer[bytesRead] = '\0';
        
    #else
        std::ifstream file(filePath, std::ios::binary | std::ios::ate);
        if (!file.is_open()) {
            return nullptr;
        }
        
        // Get file size from current position (end)
        const std::streampos fileSize = file.tellg();
        file.seekg(0, std::ios::beg);
        
        // Check for reasonable file size
        if (fileSize <= 0 || fileSize > 6 * 1024 * 1024) {
            return nullptr;
        }
        
        // Use vector<char> for better performance and explicit null termination
        std::vector<char> buffer;
        buffer.resize(static_cast<size_t>(fileSize) + 1); // +1 for null terminator
        
        file.read(buffer.data(), static_cast<std::streamsize>(fileSize));
        
        // Check how much was actually read
        std::streamsize actualRead = file.gcount();
        if (actualRead != fileSize) {
            return nullptr;
        }
        
        // Ensure null termination for cJSON
        buffer[actualRead] = '\0';
        file.close();
    #endif
        
        // Parse the JSON content - pass buffer directly to avoid string copy
        cJSON* root = cJSON_Parse(buffer.data());
        if (!root) {
            #if USING_LOGGING_DIRECTIVE
            const char* error_ptr = cJSON_GetErrorPtr();
            if (error_ptr) {
                if (!disableLogging)
                    logMessage("JSON parsing error: " + std::string(error_ptr));
            }
            #endif
            return nullptr;
        }
        
        return reinterpret_cast<json_t*>(root);
    }
    
    
    /**
     * @brief Parses a JSON string into a json_t object.
     *
     * This function takes a JSON string as input and parses it into a json_t object using cJSON library's `cJSON_Parse` function.
     * If parsing fails, it logs the error and returns nullptr.
     *
     * @param input The input JSON string to parse.
     * @return A json_t object representing the parsed JSON, or nullptr if parsing fails.
     */
    json_t* stringToJson(const std::string& input) {
        cJSON* jsonObj = cJSON_Parse(input.c_str());
    
        if (!jsonObj) {
            #if USING_LOGGING_DIRECTIVE
            const char* error_ptr = cJSON_GetErrorPtr();
            if (error_ptr != nullptr) {
                if (!disableLogging)
                    logMessage("Failed to parse JSON: " + std::string(error_ptr));
            }
            #endif
            return nullptr; // Return nullptr to indicate failure clearly
        }
    
        return reinterpret_cast<json_t*>(jsonObj);
    }
    
    
    
    
    
    // Function to get a string from a JSON object
    std::string getStringFromJson(const json_t* root, const char* key) {
        const cJSON* croot = reinterpret_cast<const cJSON*>(root);
        const cJSON* value = cJSON_GetObjectItemCaseSensitive(croot, key);
        if (value && cJSON_IsString(value) && value->valuestring) {
            return value->valuestring;
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
            #if USING_LOGGING_DIRECTIVE
            logMessage("Failed to load JSON file from path: " + filePath);
            #endif
            return "";
        }
    
        // Retrieve the string value associated with the key
        cJSON* croot = reinterpret_cast<cJSON*>(root.get());
        cJSON* jsonKey = cJSON_GetObjectItemCaseSensitive(croot, key.c_str());
        const char* value = (cJSON_IsString(jsonKey) && jsonKey->valuestring) ? jsonKey->valuestring : nullptr;
    
        // Check if the value was found and return it
        if (value) {
            return std::string(value);
        } else {
            #if USING_LOGGING_DIRECTIVE
            logMessage("Key not found or not a string in JSON: " + key);
            #endif
            return "";
        }
    }


    /**
     * @brief Sets a value in a JSON file, creating the file if it doesn't exist.
     *
     * @param filePath The path to the JSON file.
     * @param key The key to set.
     * @param value The value to set (auto-detected type).
     * @param createIfNotExists Whether to create the file if it doesn't exist.
     * @return true if successful, false otherwise.
     */
    bool setJsonValue(const std::string& filePath, const std::string& key, const std::string& value, bool createIfNotExists) {
        // Try to load existing file
        std::unique_ptr<json_t, JsonDeleter> root(readJsonFromFile(filePath), JsonDeleter());
        
        cJSON* croot = nullptr;
        
        // If file doesn't exist, create new JSON object if allowed
        if (!root) {
            if (!createIfNotExists) {
                return false;
            }
            croot = cJSON_CreateObject();
            if (!croot) {
                return false;
            }
            root.reset(reinterpret_cast<json_t*>(croot));
        } else {
            croot = reinterpret_cast<cJSON*>(root.get());
        }
    
        // FIXED: Better value type detection
        cJSON* jsonValue = nullptr;
        
        // Trim whitespace first
        std::string trimmedValue = value;
        // Remove leading whitespace
        trimmedValue.erase(0, trimmedValue.find_first_not_of(" \t\n\r"));
        // Remove trailing whitespace  
        trimmedValue.erase(trimmedValue.find_last_not_of(" \t\n\r") + 1);
        
        if (trimmedValue.empty()) {
            jsonValue = cJSON_CreateString("");
        } else if (trimmedValue == "true") {
            jsonValue = cJSON_CreateBool(1);
        } else if (trimmedValue == "false") {
            jsonValue = cJSON_CreateBool(0);
        } else if (trimmedValue == "null") {
            jsonValue = cJSON_CreateNull();
        } else {
            // Try parsing as number (integer or float)
            char* endPtr = nullptr;
            errno = 0;
            
            // Try as integer first
            const long longValue = std::strtol(trimmedValue.c_str(), &endPtr, 10);
            if (endPtr == trimmedValue.c_str() + trimmedValue.length() && errno == 0) {
                // Successfully parsed as integer
                jsonValue = cJSON_CreateNumber(static_cast<double>(longValue));
            } else {
                // Try as float
                endPtr = nullptr;
                errno = 0;
                const double doubleValue = std::strtod(trimmedValue.c_str(), &endPtr);
                if (endPtr == trimmedValue.c_str() + trimmedValue.length() && errno == 0) {
                    // Successfully parsed as float
                    jsonValue = cJSON_CreateNumber(doubleValue);
                } else {
                    // Treat as string
                    jsonValue = cJSON_CreateString(trimmedValue.c_str());
                }
            }
        }
    
        if (!jsonValue) {
            return false;
        }
    
        // Delete existing item if it exists
        cJSON_DeleteItemFromObject(croot, key.c_str());
        
        // Add the new value
        cJSON_AddItemToObject(croot, key.c_str(), jsonValue);
    
        // Save to file with formatted output for better readability
        char* jsonString = cJSON_Print(croot); // Use formatted output instead of PrintUnformatted
        if (!jsonString) {
            return false;
        }
        
        bool success = false;
        {
            std::lock_guard<std::mutex> lock(json_access_mutex);
        #if !USING_FSTREAM_DIRECTIVE
            FILE* file = fopen(filePath.c_str(), "w"); // Use text mode for JSON
            if (file) {
                const size_t jsonLength = std::strlen(jsonString);
                const size_t bytesWritten = fwrite(jsonString, 1, jsonLength, file);
                success = (bytesWritten == jsonLength);
                fclose(file);
            }
        #else
            std::ofstream file(filePath); // Use text mode for JSON
            if (file.is_open()) {
                file << jsonString;
                success = !file.fail();
                file.close();
            }
        #endif
        }
    
        cJSON_free(jsonString);
        return success;
    }

    /**
     * @brief Renames a key in a JSON file.
     *
     * @param filePath The path to the JSON file.
     * @param oldKey The current key name.
     * @param newKey The new key name.
     * @return true if successful, false otherwise.
     */
    bool renameJsonKey(const std::string& filePath, const std::string& oldKey, const std::string& newKey) {
        // Try to load existing file
        std::unique_ptr<json_t, JsonDeleter> root(readJsonFromFile(filePath), JsonDeleter());
        
        if (!root) {
            return false;
        }
        
        cJSON* croot = reinterpret_cast<cJSON*>(root.get());
    
        // Check if old key exists
        cJSON* value = cJSON_GetObjectItemCaseSensitive(croot, oldKey.c_str());
        if (!value) {
            return false;
        }
    
        // Detach the value from the object
        cJSON_DetachItemFromObject(croot, oldKey.c_str());
        
        // Add it back with the new key
        cJSON_AddItemToObject(croot, newKey.c_str(), value);
    
        // Save to file
        char* jsonString = cJSON_Print(croot); // Use formatted output
        if (!jsonString) {
            return false;
        }
    
        bool success = false;

        {
            std::lock_guard<std::mutex> lock(json_access_mutex);
        #if !USING_FSTREAM_DIRECTIVE
            FILE* file = fopen(filePath.c_str(), "w"); // Use text mode
            if (file) {
                const size_t jsonLength = std::strlen(jsonString);
                const size_t bytesWritten = fwrite(jsonString, 1, jsonLength, file);
                success = (bytesWritten == jsonLength);
                fclose(file);
            }
        #else
            std::ofstream file(filePath); // Use text mode
            if (file.is_open()) {
                file << jsonString;
                success = !file.fail();
                file.close();
            }
        #endif
        }
    
        cJSON_free(jsonString);
        return success;
    }
}