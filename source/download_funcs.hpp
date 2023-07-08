#pragma once
#include <cstdio>
#include <curl/curl.h>
#include <zlib.h>
#include <zzip/zzip.h>
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include "path_funcs.hpp"


#include <time.h>
void logMessage(const std::string& message) {
    std::time_t currentTime = std::time(nullptr);
    std::string logEntry = std::asctime(std::localtime(&currentTime));
    std::size_t lastNonNewline = logEntry.find_last_not_of("\r\n");
    if (lastNonNewline != std::string::npos) {
        logEntry.erase(lastNonNewline + 1);
    }
    logEntry = "[" + logEntry + "] " + message + "\n";

    FILE* file = fopen("sdmc:/config/ultrahand/log.txt", "a");
    if (file != nullptr) {
        fputs(logEntry.c_str(), file);
        fclose(file);
    }
}

size_t writeCallback(void* contents, size_t size, size_t nmemb, FILE* file) {
    // Callback function to write received data to a file
    size_t written = fwrite(contents, size, nmemb, file);
    return written;
}


bool downloadFile(const std::string& url, const std::string& toDestination) {
    std::string destination = toDestination.c_str();
    // Check if the destination ends with "/"
    if (destination.back() == '/') {
        // Extract the filename from the URL
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash != std::string::npos) {
            std::string filename = url.substr(lastSlash + 1);
            destination += filename;
        } else {
            logMessage(std::string("Invalid URL: ") + url);
            return false;
        }
    }

    const int MAX_RETRIES = 3;
    int retryCount = 0;
    CURL* curl = nullptr;

    while (retryCount < MAX_RETRIES) {
        curl = curl_easy_init();
        if (curl) {
            // Successful initialization, break out of the loop
            break;
        } else {
            // Failed initialization, increment retry count and try again
            retryCount++;
            logMessage("Error initializing curl. Retrying...");
        }
    }
    if (!curl) {
        // Failed to initialize curl after multiple attempts
        logMessage("Error initializing curl after multiple retries.");
        return false;
    }
    
    FILE* file = fopen(destination.c_str(), "wb");
    if (!file) {
        logMessage(std::string("Error opening file: ") + destination);
        return false;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

    // Set a user agent
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

    // Enable following redirects
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    // If you have a cacert.pem file, you can set it as a trusted CA
    // curl_easy_setopt(curl, CURLOPT_CAINFO, "sdmc:/config/ultrahand/cacert.pem");

    CURLcode result = curl_easy_perform(curl);
    if (result != CURLE_OK) {
        logMessage(std::string("Error downloading file: ") + curl_easy_strerror(result));
        curl_easy_cleanup(curl);
        fclose(file);
        // Delete the file if nothing was written to it
        std::remove(destination.c_str());
        return false;
    }

    curl_easy_cleanup(curl);
    fclose(file);;
    // Check if the file is empty
    long fileSize = ftell(file);
    if (fileSize == 0) {
        logMessage(std::string("Error downloading file: Empty file"));
        std::remove(destination.c_str());
        return false;
    }

    return true;
}


bool unzipFile(const std::string& zipFilePath, const std::string& toDestination) {
    ZZIP_DIR* dir = zzip_dir_open(zipFilePath.c_str(), nullptr);
    if (!dir) {
        logMessage(std::string("Error opening zip file: ") + zipFilePath);
        return false;
    }

    bool success = true;
    ZZIP_DIRENT entry;
    while (zzip_dir_read(dir, &entry)) {
        if (entry.d_name[0] == '\0') continue;  // Skip empty entries

        std::string fileName = entry.d_name;
        std::string extractedFilePath = toDestination + fileName;
        
        // Extract the directory path from the extracted file path
        std::string directoryPath;
        if (extractedFilePath.back() != '/') {
            directoryPath = extractedFilePath.substr(0, extractedFilePath.find_last_of('/'))+"/";
        } else {
            directoryPath = extractedFilePath;
        }
        
        createDirectory(directoryPath);
        
        if (isDirectory(directoryPath)) {
            logMessage("directoryPath: success");
        } else {
            logMessage("directoryPath: failure");
        }
        
        logMessage(std::string("directoryPath: ") + directoryPath);

        ZZIP_FILE* file = zzip_file_open(dir, entry.d_name, 0);
        if (file) {
            FILE* outputFile = fopen(extractedFilePath.c_str(), "wb");
            if (outputFile) {
                zzip_ssize_t bytesRead;
                const zzip_ssize_t bufferSize = 8192;
                char buffer[bufferSize];

                while ((bytesRead = zzip_file_read(file, buffer, bufferSize)) > 0) {
                    fwrite(buffer, 1, bytesRead, outputFile);
                }

                fclose(outputFile);
            } else {
                logMessage(std::string("Error opening output file: ") + extractedFilePath);
                success = false;
            }

            zzip_file_close(file);
        } else {
            logMessage(std::string("Error opening file in zip: ") + fileName);
            success = false;
        }
    }

    zzip_dir_close(dir);
    return success;
}
