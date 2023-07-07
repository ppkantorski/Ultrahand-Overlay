#include <cstdio>
#include <curl/curl.h>
#include <sys/stat.h>
#include <dirent.h>
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

    FILE* file = fopen(destination.c_str(), "wb");
    if (!file) {
        logMessage(std::string("Error opening file: ") + destination);
        return false;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        // Set a user agent
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/91.0.4472.124 Safari/537.36");

        // If you have a cacert.pem file, you can set it as a trusted CA
        //curl_easy_setopt(curl, CURLOPT_CAINFO, "sdmc:/config/ultrahand/cacert.pem");

        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            logMessage(std::string("Error downloading file: ") + curl_easy_strerror(result));
            fclose(file);
            curl_easy_cleanup(curl);
            return false;
        }

        curl_easy_cleanup(curl);
    } else {
        logMessage("Error initializing curl.");
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}
