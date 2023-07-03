#pragma once
#include <curl/curl.h>
#include <sys/stat.h>


// For loggging messages and debugging
#include <ctime>
void logMessage(const std::string& message) {
    std::time_t currentTime = std::time(nullptr);
    std::string logEntry = std::asctime(std::localtime(&currentTime));
    // Find the last non-newline character
    std::size_t lastNonNewline = logEntry.find_last_not_of("\r\n");

    // Remove everything after the last non-newline character
    if (lastNonNewline != std::string::npos) {
        logEntry.erase(lastNonNewline + 1);
    }
    logEntry = "["+logEntry+"] ";
    logEntry += message+"\n";

    FILE* file = fopen("sdmc:/config/ultrahand/log.txt", "a");
    if (file != nullptr) {
        fputs(logEntry.c_str(), file);
        fclose(file);
    }
}



// Callback function to write downloaded data into a file
static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
    FILE* file = static_cast<FILE*>(userp);
    fwrite(contents, size, nmemb, file);
    return size * nmemb;
}

// Function to download a file given a URL and destination path
void downloadFile(const std::string& fileUrl, const std::string& toDestination) //
{
    auto curl = curl_easy_init();
    if (curl)
    {
        logMessage("Initialized curl.");
        // Open the destination file
        FILE* outputFile = fopen(toDestination.c_str(), "wb");
        if (!outputFile)
        {
            logMessage(std::string("Failed to open destination file: ") +toDestination);
            return;
        }
        
        logMessage("Setting url to download from.");
        // Set the URL to download from
        curl_easy_setopt(curl, CURLOPT_URL, fileUrl.c_str());

        // Set the write callback function
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, outputFile);
        logMessage("Set the write callback complete.");
    
    
        logMessage("Performing the download: "+fileUrl);
        // Perform the download
        CURLcode res = curl_easy_perform(curl); // ISSUE IS HERE. IT KEEPS FREEZING...
        if (res != CURLE_OK)
        {
            logMessage(std::string("Failed to download file: ") + curl_easy_strerror(res));
            fclose(outputFile);
            return;
        }
        logMessage("Download complete.");
        
        // Cleanup
        logMessage("Cleanup.");
        curl_easy_cleanup(curl);
        logMessage("Cleanup complete.");
        fclose(outputFile);
        
        logMessage(std::string("File downloaded successfully to: ") + toDestination);
        return;
    }
    logMessage("Failed to initialize curl");
}
