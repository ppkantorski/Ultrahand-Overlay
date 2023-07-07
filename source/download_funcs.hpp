#pragma once

#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/select.h>
#include <errno.h>


#include <cstdio>
#include <cstring>
#include <string>
#include <ctime>
#include <chrono>
#include <thread>

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


#include <cmath>


// NTP client code
#include <errno.h>
#include <exception>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>



// Function to download a file given a URL and destination path
void downloadFile(const std::string& fileUrl, const std::string& toDestination) {
    std::string hostname;
    std::string path;
    std::size_t hostnameStart = fileUrl.find("://");
    if (hostnameStart != std::string::npos) {
        hostnameStart += 3;
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
    const int retryDelayMs = 1000;

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

    std::string request = "GET " + path + " HTTP/1.1\r\nHost: " + hostname + "\r\n\r\n";
    if (send(sockfd, request.c_str(), request.length(), 0) < 0) {
        perror("Failed to send request");
        close(sockfd);
        return;
    }

    std::string savePath = toDestination + path.substr(path.rfind('/') + 1);
    FILE* outputFile = fopen(savePath.c_str(), "wb");
    if (outputFile == nullptr) {
        logMessage("Failed to create file: " + savePath);
        close(sockfd);
        return;
    }

    const size_t bufferSize = 131072;
    char buffer[bufferSize];
    ssize_t bytesRead;
    bool headerReceived = false;

    fd_set readSet;

    struct timeval timeout;
    timeout.tv_sec = 3; // Initial timeout value in seconds
    timeout.tv_usec = 0;

    while (true) {
        FD_ZERO(&readSet);
        FD_SET(sockfd, &readSet);

        int selectResult = select(sockfd + 1, &readSet, nullptr, nullptr, &timeout);

        if (selectResult == -1) {
            perror("Error in select");
            break;
        } else if (selectResult == 0) {
            // Timeout occurred
            logMessage("Receive timeout occurred");
            break;
        }

        bytesRead = recv(sockfd, buffer, bufferSize, 0);

        if (bytesRead < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                // No data available, continue waiting
                continue;
            } else {
                // Other receive error occurred
                perror("Failed to receive data");
                break;
            }
        } else if (bytesRead == 0) {
            // Connection closed
            break;
        }

        if (!headerReceived) {
            std::string response(buffer, bytesRead);
            std::size_t headerEnd = response.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                bytesRead -= (headerEnd + 4);
                std::memcpy(buffer, buffer + headerEnd + 4, bytesRead);
                headerReceived = true;

                // Reset the timeout to its initial value
                timeout.tv_sec = 3;
                timeout.tv_usec = 0;
            } else {
                continue;
            }
        }

        fwrite(buffer, sizeof(char), bytesRead, outputFile);
    }

    if (bytesRead < 0) {
        perror("Failed to receive data");
    }

    fclose(outputFile);
    close(sockfd);
}

