#include <cstdio>
#include <string>
#include <sys/stat.h>
#include <jansson.h>

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
