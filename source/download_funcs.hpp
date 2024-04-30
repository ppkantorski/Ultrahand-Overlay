/********************************************************************************
 * File: download_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains functions for downloading and extracting files
 *   using libcurl and zlib. It includes functions for downloading files from URLs,
 *   writing received data to a file, and extracting files from ZIP archives.
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
#include <fstream>
#include <curl/curl.h>
#include <zlib.h>
#include <zzip/zzip.h>
//#include <queue>
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include "path_funcs.hpp"
#include "debug_funcs.hpp"
//#include "json_funcs.hpp"

//const size_t downloadBufferSize = 4096*2;
//const zzip_ssize_t unzipBufferSize = 512;


// Shared atomic flag to indicate whether to abort the download operation
static std::atomic<bool> abortDownload(false);
// Define an atomic bool for interpreter completion
static std::atomic<bool> abortUnzip(false);
static std::atomic<int> downloadPercentage(-1);
static std::atomic<int> unzipPercentage(-1);


// Define a custom deleter for the unique_ptr to properly clean up the CURL handle
struct CurlDeleter {
    void operator()(CURL* curl) const {
        curl_easy_cleanup(curl);
    }
};


// Callback function to write received data to a file.
size_t writeCallback(void* ptr, size_t size, size_t nmemb, std::ostream* stream) {
    auto& file = *static_cast<std::ofstream*>(stream);
    size_t totalBytes = size * nmemb;
    file.write(static_cast<const char*>(ptr), totalBytes);
    return totalBytes;
}



// Your C function
extern "C" int progressCallback(void *ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    auto percentage = static_cast<std::atomic<int>*>(ptr);

    if (totalToDownload > 0) {
        int newProgress = static_cast<int>(double(nowDownloaded) / double(totalToDownload) *100);
        percentage->store(newProgress, std::memory_order_release);
    }

    if (abortDownload.load(std::memory_order_acquire)) {
        percentage->store(-1, std::memory_order_release);
        return 1;  // Abort the download
    }

    return 0;  // Continue the download
}




/**
 * @brief Downloads a file from a URL to a specified destination.
 *
 * @param url The URL of the file to download.
 * @param toDestination The destination path where the file should be saved.
 * @return True if the download was successful, false otherwise.
 */
bool downloadFile(const std::string& url, const std::string& toDestination) {
    abortDownload.store(false);
    downloadPercentage.store(0, std::memory_order_release);

    if (url.find_first_of("{}") != std::string::npos) {
        logMessage("Invalid URL: " + url);
        return false;
    }

    std::string destination = toDestination;
    if (destination.back() == '/') {
        createDirectory(destination);
        size_t lastSlash = url.find_last_of('/');
        if (lastSlash != std::string::npos) {
            destination += url.substr(lastSlash + 1);
        } else {
            logMessage("Invalid URL: " + url);
            return false;
        }
    } else {
        createDirectory(destination.substr(0, destination.find_last_of('/')));
    }

    std::ofstream file(destination, std::ios::binary);
    if (!file.is_open()) {
        logMessage("Error opening file: " + destination);
        return false;
    }

    std::unique_ptr<CURL, CurlDeleter> curl(curl_easy_init());
    if (!curl) {
        logMessage("Error initializing curl.");
        return false;
    }

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &downloadPercentage);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    //curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 5); // Limit the number of redirects


    CURLcode result = curl_easy_perform(curl.get());
    file.close();

    if (result != CURLE_OK) {
        logMessage("Error downloading file: " + std::string(curl_easy_strerror(result)));
        deleteFileOrDirectory(destination);
        return false;
    }

    std::ifstream checkFile(destination);
    if (!checkFile || checkFile.peek() == std::ifstream::traits_type::eof()) {
        logMessage("Error downloading file: Empty file");
        deleteFileOrDirectory(destination);
        return false;
    }
    checkFile.close();

    if (downloadPercentage.load(std::memory_order_acquire) == -1 || downloadPercentage.load(std::memory_order_acquire) == 0)
        downloadPercentage.store(100, std::memory_order_release);
    logMessage("Download Complete!");
    return true;
}


// Define a custom deleter for the unique_ptr to properly close the ZZIP_DIR handle
struct ZzipDirDeleter {
    void operator()(ZZIP_DIR* dir) const {
        zzip_dir_close(dir);
    }
};


/**
 * @brief Extracts files from a ZIP archive to a specified destination.
 *
 * @param zipFilePath The path to the ZIP archive file.
 * @param toDestination The destination directory where files should be extracted.
 * @return True if the extraction was successful, false otherwise.
 */
bool unzipFile(const std::string& zipFilePath, const std::string& toDestination) {
    abortUnzip.store(false, std::memory_order_release); // Reset abort flag

    // Open the ZIP archive using a unique_ptr with custom deleter
    std::unique_ptr<ZZIP_DIR, ZzipDirDeleter> dir(zzip_dir_open(zipFilePath.c_str(), nullptr));
    if (!dir) {
        logMessage(std::string("Error opening zip file: ") + zipFilePath);
        return false;
    }

    bool success = true;
    ZZIP_DIRENT entry;
    while (zzip_dir_read(dir.get(), &entry)) {
        if (abortUnzip.load(std::memory_order_acquire)) {
            abortUnzip.store(false, std::memory_order_release); // Reset abort flag
            break;
        }

        // Skip empty entries, "..." files, and files starting with "."
        if (entry.d_name[0] == '\0') {
            continue;
        }

        std::string fileName = entry.d_name;
        std::string extractedFilePath = toDestination + fileName;

        // Skip extractedFilePath ends with "..."
        if (extractedFilePath.size() >= 3 && extractedFilePath.substr(extractedFilePath.size() - 3) == "...")
            continue;

        // Replace ":" characters except in "sdmc:/"
        size_t firstColonPos = extractedFilePath.find(':');
        if (firstColonPos != std::string::npos) {
            size_t colonPos = extractedFilePath.find(':', firstColonPos + 1);
            while (colonPos != std::string::npos) {
                extractedFilePath[colonPos] = ' ';
                colonPos = extractedFilePath.find(':', colonPos + 1);
            }
        }

        // Replace double spaces with single space
        size_t pos = extractedFilePath.find("  ");
        while (pos != std::string::npos) {
            extractedFilePath.replace(pos, 2, " ");
            pos = extractedFilePath.find("  ", pos + 1);
        }


        // Skip over present directory entries when extracting files from a zip archive
        if (!extractedFilePath.empty() && extractedFilePath.back() == '/') {
            continue;
        }

        // Extract the directory path from the extracted file path
        std::string directoryPath;
        if (extractedFilePath.back() != '/') {
            directoryPath = extractedFilePath.substr(0, extractedFilePath.find_last_of('/')) + "/";
        } else {
            directoryPath = extractedFilePath;
        }

        createDirectory(directoryPath);

        ZZIP_FILE* file = zzip_file_open(dir.get(), entry.d_name, 0);
        if (file) {
            std::ofstream outputFile(extractedFilePath, std::ios::binary);
            if (outputFile.is_open()) {
                zzip_ssize_t bytesRead;
                const zzip_ssize_t bufferSize = 4096;
                char buffer[bufferSize];
            
                while ((bytesRead = zzip_file_read(file, buffer, bufferSize)) > 0) {
                    outputFile.write(buffer, bytesRead);
                }
            
                outputFile.close();
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

    return success;
}
