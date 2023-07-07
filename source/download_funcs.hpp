#include <cstdio>
#include <curl/curl.h>
#include <sys/stat.h>
#include <dirent.h>


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
            logMessage(std::string("Invalid URL: ")+ url);
            return false;
        }
    }
    
    FILE* file = fopen(destination.c_str(), "wb");
    if (!file) {
        logMessage(std::string("Error opening file: ")+ destination);
        return false;
    }

    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

        // If you have a cacert.pem file, you can set it as a trusted CA
        curl_easy_setopt(curl, CURLOPT_CAINFO, "sdmc:/config/ultrahand/cacert.pem");

        CURLcode result = curl_easy_perform(curl);
        if (result != CURLE_OK) {
            logMessage(std::string("Error downloading file: ")+ curl_easy_strerror(result));
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

bool directoryExists(const std::string& path) {
    struct stat info;
    return stat(path.c_str(), &info) == 0 && S_ISDIR(info.st_mode);
}

std::string getFileNameFromURL(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos)
        return url.substr(lastSlash + 1);
    return "";
}

std::string getDestinationPath(const std::string& destinationDir, const std::string& fileName) {
    return destinationDir + "/" + fileName;
}

std::string generateUniqueDestination(const std::string& destination) {
    std::string baseDir = destination.substr(0, destination.find_last_of('/'));
    std::string fileName = destination.substr(destination.find_last_of('/') + 1);
    std::string baseName = fileName.substr(0, fileName.find_last_of('.'));
    std::string extension = fileName.substr(fileName.find_last_of('.'));
    
    std::string uniqueDestination = destination;
    int count = 1;

    while (!directoryExists(baseDir) || fopen(uniqueDestination.c_str(), "rb")) {
        uniqueDestination = baseDir + "/" + baseName + "_" + std::to_string(count) + extension;
        count++;
    }

    return uniqueDestination;
}

bool createDirectory2(const std::string& path) {
    return mkdir(path.c_str(), 0777) == 0;
}

bool ensureDirectoryExists(const std::string& path) {
    if (directoryExists(path))
        return true;
    
    if (createDirectory2(path))
        return true;
    
    logMessage(std::string("Failed to create directory: ")+ path);
    return false;
}

bool downloadFileWithDirectory(const std::string& url, const std::string& destinationDir) {
    std::string fileName = getFileNameFromURL(url);
    std::string destinationPath = getDestinationPath(destinationDir, fileName);

    if (!ensureDirectoryExists(destinationDir))
        return false;

    std::string uniqueDestination = generateUniqueDestination(destinationPath);
    return downloadFile(url, uniqueDestination);
}

