#pragma once

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <chrono>

// For logging messages and debugging
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

// Function to download a file given a URL and destination path
void downloadFile(const std::string& fileUrl, const std::string& toDestination) {
    // Extract the hostname and path from the fileUrl
    std::string hostname;
    std::string path;
    std::size_t hostnameStart = fileUrl.find("://");
    if (hostnameStart != std::string::npos) {
        hostnameStart += 3;  // Skip the "://"
        std::size_t pathStart = fileUrl.find('/', hostnameStart);
        if (pathStart != std::string::npos) {
            hostname = fileUrl.substr(hostnameStart, pathStart - hostnameStart);
            path = fileUrl.substr(pathStart);
        }
    }

    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* serverInfo;
    int result = getaddrinfo(hostname.c_str(), "80", &hints, &serverInfo);
    int retryCount = 0;
    const int maxRetryCount = 3;
    const int retryDelayMs = 1000; // 1 second

    while (result != 0 && retryCount < maxRetryCount) {
        logMessage("Failed to get address info: " + std::string(gai_strerror(result)) + ". Retrying...");
        std::this_thread::sleep_for(std::chrono::milliseconds(retryDelayMs));
        result = getaddrinfo(hostname.c_str(), "80", &hints, &serverInfo);
        retryCount++;
    }

    if (result != 0) {
        logMessage("Failed to get address info after retries: " + std::string(gai_strerror(result)));
        return;
    }

    int sockfd = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
    if (sockfd < 0) {
        perror("Failed to create socket");
        freeaddrinfo(serverInfo);
        return;
    }

    if (connect(sockfd, serverInfo->ai_addr, serverInfo->ai_addrlen) < 0) {
        perror("Failed to connect");
        close(sockfd);
        freeaddrinfo(serverInfo);
        return;
    }

    freeaddrinfo(serverInfo);

    // Send the HTTP request
    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + hostname + "\r\n\r\n";
    if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
        perror("Failed to send request");
        close(sockfd);
        return;
    }

    // Receive and save the file data
    std::string savePath = toDestination + path.substr(path.rfind('/') + 1);
    FILE* file = fopen(savePath.c_str(), "wb");
    if (file == NULL) {
        perror("Failed to create file");
        close(sockfd);
        return;
    }

    char buffer[4096];
    ssize_t bytesRead;
    while ((bytesRead = recv(sockfd, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, sizeof(char), static_cast<size_t>(bytesRead), file);
    }

    fclose(file);
    close(sockfd);
}
