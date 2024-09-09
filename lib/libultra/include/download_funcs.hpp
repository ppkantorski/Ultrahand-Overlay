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
//#include <minizip/unzip.h>
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include "path_funcs.hpp"
#include "debug_funcs.hpp"
//#include "json_funcs.hpp"

size_t DOWNLOAD_BUFFER_SIZE = 4096*4;
size_t UNZIP_BUFFER_SIZE = 4096*4;

// Path to the CA certificate
const std::string cacertPath = "sdmc:/config/ultrahand/cacert.pem";
const std::string cacertURL = "https://curl.se/ca/cacert.pem";

// Shared atomic flag to indicate whether to abort the download operation
static std::atomic<bool> abortDownload(false);
// Define an atomic bool for interpreter completion
static std::atomic<bool> abortUnzip(false);
static std::atomic<int> downloadPercentage(-1);
static std::atomic<int> unzipPercentage(-1);

static const std::string userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36";


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
        //int newProgress = static_cast<int>(float(nowDownloaded) / float(totalToDownload) *100);
        percentage->store(static_cast<int>(float(nowDownloaded) / float(totalToDownload) *100), std::memory_order_release);
    }

    if (abortDownload.load(std::memory_order_acquire)) {
        percentage->store(-1, std::memory_order_release);
        return 1;  // Abort the download
    }

    return 0;  // Continue the download
}


// Global initialization function
void initializeCurl() {
    CURLcode res = curl_global_init(CURL_GLOBAL_DEFAULT);
    if (res != CURLE_OK) {
        logMessage("curl_global_init() failed: " + std::string(curl_easy_strerror(res)));
        // Handle error appropriately, possibly exit the program
    }
}

// Global cleanup function
void cleanupCurl() {
    curl_global_cleanup();
}

/**
 * @brief Downloads a file from a URL to a specified destination.
 *
 * @param url The URL of the file to download.
 * @param toDestination The destination path where the file should be saved.
 * @return True if the download was successful, false otherwise.
 */
// Your downloadFile function
bool downloadFile(const std::string& url, const std::string& toDestination) {
    abortDownload.store(false);

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

    if (!isDirectory(DOWNLOADS_PATH)) {
        createDirectory(DOWNLOADS_PATH);
    }
    
    std::string tempFilePath = DOWNLOADS_PATH + getFileName(destination) + ".tmp";

    std::ofstream file(tempFilePath, std::ios::binary);
    if (!file.is_open()) {
        logMessage("Error opening file: " + tempFilePath);
        return false;
    }

    std::unique_ptr<CURL, CurlDeleter> curl(curl_easy_init());
    if (!curl) {
        logMessage("Error initializing curl.");
        file.close();
        return false;
    }

    downloadPercentage.store(0, std::memory_order_release);

    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl.get(), CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFOFUNCTION, progressCallback);
    curl_easy_setopt(curl.get(), CURLOPT_XFERINFODATA, &downloadPercentage);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, userAgent.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS); // Enable HTTP/2
    curl_easy_setopt(curl.get(), CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2); // Force TLS 1.2

    // Disable SSL verification for testing purposes
    //curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 0L);
    //curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 0L);

    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_BUFFERSIZE, DOWNLOAD_BUFFER_SIZE); // Increase buffer size

    CURLcode result = curl_easy_perform(curl.get());
    file.close();

    if (result != CURLE_OK) {
        logMessage("Error downloading file: " + std::string(curl_easy_strerror(result)));
        deleteFileOrDirectory(tempFilePath);
        downloadPercentage.store(-1, std::memory_order_release);
        return false;
    }

    std::ifstream checkFile(tempFilePath);
    if (!checkFile || checkFile.peek() == std::ifstream::traits_type::eof()) {
        logMessage("Error downloading file: Empty file");
        deleteFileOrDirectory(tempFilePath);
        downloadPercentage.store(-1, std::memory_order_release);
        checkFile.close();
        return false;
    }
    checkFile.close();

    downloadPercentage.store(100, std::memory_order_release);
    moveFile(tempFilePath, destination);

    //logMessage("Download Complete!");
    return true;
}


// Define a custom deleter for the unique_ptr to properly close the ZZIP_DIR handle
struct ZzipDirDeleter {
    void operator()(ZZIP_DIR* dir) const {
        if (dir) {
            zzip_dir_close(dir);
        }
    }
};

struct ZzipFileDeleter {
    void operator()(ZZIP_FILE* file) const {
        if (file) {
            zzip_file_close(file);
        }
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

    std::unique_ptr<ZZIP_DIR, ZzipDirDeleter> dir(zzip_dir_open(zipFilePath.c_str(), nullptr));
    if (!dir) {
        logMessage("Error opening zip file: " + zipFilePath);
        return false;
    }

    ZZIP_DIRENT entry;
    zzip_ssize_t totalUncompressedSize = 0;

    // First pass: Calculate the total size of all files
    while (zzip_dir_read(dir.get(), &entry)) {
        if (entry.d_name[0] != '\0' && entry.st_size > 0) {
            totalUncompressedSize += entry.st_size;
        }
    }

    // Close and reopen the directory for extraction
    dir.reset(zzip_dir_open(zipFilePath.c_str(), nullptr));
    if (!dir) {
        logMessage("Failed to reopen zip file: " + zipFilePath);
        return false;
    }

    bool success = true;
    zzip_ssize_t currentUncompressedSize = 0;

    std::string fileName, extractedFilePath;
    std::string directoryPath;
    char buffer[UNZIP_BUFFER_SIZE];
    zzip_ssize_t bytesRead;

    unzipPercentage.store(0, std::memory_order_release); // Initialize percentage

    std::unique_ptr<ZZIP_FILE, ZzipFileDeleter> file;
    std::ofstream outputFile;

    auto it = extractedFilePath.end();
    // Second pass: Extract files and update progress
    while (zzip_dir_read(dir.get(), &entry)) {
        if (entry.d_name[0] == '\0') continue; // Skip empty entries

        fileName = entry.d_name;

        // Remove invalid characters
        extractedFilePath = toDestination + fileName;
        it = extractedFilePath.begin() + std::min(extractedFilePath.find(ROOT_PATH) + 5, extractedFilePath.size());
        extractedFilePath.erase(std::remove_if(it, extractedFilePath.end(), [](char c) {
            return c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|';
        }), extractedFilePath.end());

        if (!extractedFilePath.empty() && extractedFilePath.back() == '/') continue; // Skip directories

        directoryPath = extractedFilePath.substr(0, extractedFilePath.find_last_of('/') + 1);
        createDirectory(directoryPath);

        // Reset the unique_ptr before opening a new file
        file.reset(zzip_file_open(dir.get(), entry.d_name, 0));
        if (!file) {
            logMessage("Error opening file in zip: " + fileName);
            success = false;
            continue;
        }

        outputFile.open(extractedFilePath, std::ios::binary);
        if (!outputFile.is_open()) {
            logMessage("Error opening output file: " + extractedFilePath);
            success = false;
            continue;
        }

        while ((bytesRead = zzip_file_read(file.get(), buffer, UNZIP_BUFFER_SIZE)) > 0) {
            if (abortUnzip.load(std::memory_order_acquire)) {
                logMessage("Aborting unzip operation during file extraction.");
                outputFile.close();
                deleteFileOrDirectory(extractedFilePath); // Cleanup partial file
                success = false;
                break;
            }
            outputFile.write(buffer, bytesRead);
            logMessage("Extracted: " + extractedFilePath);

            currentUncompressedSize += bytesRead;

            unzipPercentage.store(totalUncompressedSize != 0 ?
                static_cast<int>(100.0 * std::min(1.0, static_cast<double>(currentUncompressedSize) / static_cast<double>(totalUncompressedSize))) : 0,
                std::memory_order_release);
        }
        outputFile.close();

        if (!success) break; // Exit the outer loop if interrupted during extraction
    }

    if (success) {
        unzipPercentage.store(100, std::memory_order_release); // Ensure it's set to 100% on successful extraction
        //logMessage("Extraction Complete!");
    } else {
        unzipPercentage.store(-1, std::memory_order_release);
    }

    return success;
}


//bool unzipFile(const std::string& zipFilePath, const std::string& toDestination) {
//    abortUnzip.store(false, std::memory_order_release); // Reset abort flag
//
//    unzFile zipfile = unzOpen(zipFilePath.c_str());
//    if (!zipfile) {
//        logMessage("Error opening zip file: " + zipFilePath);
//        return false;
//    }
//
//    unz_global_info globalInfo;
//    if (unzGetGlobalInfo(zipfile, &globalInfo) != UNZ_OK) {
//        logMessage("Error getting global info from zip file: " + zipFilePath);
//        unzClose(zipfile);
//        return false;
//    }
//
//    z_off_t totalUncompressedSize = 0;
//
//    // First pass: Calculate the total size of all files
//    if (unzGoToFirstFile(zipfile) != UNZ_OK) {
//        logMessage("Error going to first file in zip: " + zipFilePath);
//        unzClose(zipfile);
//        return false;
//    }
//    do {
//        unz_file_info fileInfo;
//        char fileName[256];
//        if (unzGetCurrentFileInfo(zipfile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) != UNZ_OK) {
//            logMessage("Error getting file info from zip: " + zipFilePath);
//            unzClose(zipfile);
//            return false;
//        }
//        if (fileName[0] != '\0' && fileInfo.uncompressed_size > 0) {
//            totalUncompressedSize += fileInfo.uncompressed_size;
//        }
//    } while (unzGoToNextFile(zipfile) == UNZ_OK);
//
//    // Second pass: Extract files and update progress
//    if (unzGoToFirstFile(zipfile) != UNZ_OK) {
//        logMessage("Error going to first file in zip: " + zipFilePath);
//        unzClose(zipfile);
//        return false;
//    }
//
//    bool success = true;
//    z_off_t currentUncompressedSize = 0;
//    char buffer[UNZIP_BUFFER_SIZE];
//
//    unzipPercentage.store(0, std::memory_order_release); // Initialize percentage
//
//    do {
//        unz_file_info fileInfo;
//        char fileName[256];
//        if (unzGetCurrentFileInfo(zipfile, &fileInfo, fileName, sizeof(fileName), nullptr, 0, nullptr, 0) != UNZ_OK) {
//            logMessage("Error getting file info from zip: " + zipFilePath);
//            success = false;
//            break;
//        }
//
//        if (fileName[0] == '\0') continue; // Skip empty entries
//
//        std::string extractedFilePath = toDestination + fileName;
//
//        // Remove invalid characters
//        auto it = extractedFilePath.begin() + std::min(extractedFilePath.find("sdmc:/") + 5, extractedFilePath.size());
//        extractedFilePath.erase(std::remove_if(it, extractedFilePath.end(), [](char c) {
//            return c == ':' || c == '*' || c == '?' || c == '\"' || c == '<' || c == '>' || c == '|';
//        }), extractedFilePath.end());
//
//        if (!extractedFilePath.empty() && extractedFilePath.back() == '/') continue; // Skip directories
//
//        std::string directoryPath = extractedFilePath.substr(0, extractedFilePath.find_last_of('/') + 1);
//        createDirectory(directoryPath);
//
//        if (unzOpenCurrentFile(zipfile) != UNZ_OK) {
//            logMessage("Error opening file in zip: " + std::string(fileName));
//            success = false;
//            continue;
//        }
//
//        std::ofstream outputFile(extractedFilePath, std::ios::binary);
//        if (!outputFile.is_open()) {
//            logMessage("Error opening output file: " + extractedFilePath);
//            unzCloseCurrentFile(zipfile);
//            success = false;
//            continue;
//        }
//
//        int bytesRead;
//        while ((bytesRead = unzReadCurrentFile(zipfile, buffer, UNZIP_BUFFER_SIZE)) > 0) {
//            if (abortUnzip.load(std::memory_order_acquire)) {
//                logMessage("Aborting unzip operation during file extraction.");
//                outputFile.close();
//                deleteFileOrDirectory(extractedFilePath); // Cleanup partial file
//                success = false;
//                break;
//            }
//            outputFile.write(buffer, bytesRead);
//            logMessage("Extracted: " + extractedFilePath);
//
//            currentUncompressedSize += bytesRead;
//
//            unzipPercentage.store(totalUncompressedSize != 0 ?
//                static_cast<int>(100.0 * std::min(1.0, static_cast<double>(currentUncompressedSize) / static_cast<double>(totalUncompressedSize))) : 0,
//                std::memory_order_release);
//        }
//        outputFile.close();
//        unzCloseCurrentFile(zipfile);
//
//        if (!success) break; // Exit the outer loop if interrupted during extraction
//    } while (unzGoToNextFile(zipfile) == UNZ_OK);
//
//    if (success) {
//        unzipPercentage.store(100, std::memory_order_release); // Ensure it's set to 100% on successful extraction
//        logMessage("Extraction Complete!");
//    } else {
//        unzipPercentage.store(-1, std::memory_order_release);
//    }
//
//    unzClose(zipfile);
//    return success;
//}
