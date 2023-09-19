/********************************************************************************
 * File: json_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file provides functions for working with JSON files in C++ using
 *   the `jansson` library. It includes a function to read JSON data from a file.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 ********************************************************************************/

#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <jansson.h>

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
