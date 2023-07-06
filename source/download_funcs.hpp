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


// NTP client code
#include <errno.h>
#include <exception>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>

#define UNIX_OFFSET 2208988800L

#define NTP_DEFAULT_PORT "123"
#define DEFAULT_TIMEOUT 3

// Flags 00|100|011 for li=0, vn=4, mode=3
#define NTP_FLAGS 0x23

typedef struct {
    uint8_t flags;
    uint8_t stratum;
    uint8_t poll;
    uint8_t precision;
    uint32_t root_delay;
    uint32_t root_dispersion;
    uint8_t referenceID[4];
    uint32_t ref_ts_secs;
    uint32_t ref_ts_frac;
    uint32_t origin_ts_secs;
    uint32_t origin_ts_frac;
    uint32_t recv_ts_secs;
    uint32_t recv_ts_fracs;
    uint32_t transmit_ts_secs;
    uint32_t transmit_ts_frac;

} ntp_packet;

struct NtpException : public std::exception {
protected:
    int m_code;
    std::string m_message;

public:
    NtpException(int code, std::string message) {
        m_code = code;
        m_message = message;
    }

    const char* what() const noexcept {
        return m_message.c_str();
    }
};

class NTPClient {
private:
    int m_timeout;
    const char* m_port;
    const char* m_server;

public:
    NTPClient(const char* server, const char* port, int timeout) {
        m_server = server;
        m_port = port;
        m_timeout = timeout;
    }

    NTPClient(const char* server, const char* port) : NTPClient(server, port, DEFAULT_TIMEOUT) {}
    NTPClient(const char* server) : NTPClient(server, NTP_DEFAULT_PORT, DEFAULT_TIMEOUT) {}
    NTPClient() : NTPClient("pool.ntp.org", NTP_DEFAULT_PORT, DEFAULT_TIMEOUT) {}

    void setTimeout(int timeout) {
        m_timeout = timeout;
    }

    time_t getTime() noexcept(false) {
        int server_sock, status;
        struct addrinfo hints, *servinfo, *ap;
        socklen_t addrlen = sizeof(struct sockaddr_storage);
        ntp_packet packet = {.flags = NTP_FLAGS};

        hints = (struct addrinfo){.ai_family = AF_INET, .ai_socktype = SOCK_DGRAM};

        if ((status = getaddrinfo(m_server, m_port, &hints, &servinfo)) != 0) {
            throw NtpException(1, "Unable to get address info (" + std::string(gai_strerror(status)) + ")");
        }

        for (ap = servinfo; ap != NULL; ap = ap->ai_next) {
            server_sock = socket(ap->ai_family, ap->ai_socktype, ap->ai_protocol);
            if (server_sock != -1)
                break;
        }

        if (ap == NULL) {
            throw NtpException(2, "Unable to create the socket");
        }

        struct timeval timeout = {.tv_sec = m_timeout, .tv_usec = 0};

        if (setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
            throw NtpException(3, "Unable to set RCV timeout");
        }

        if (setsockopt(server_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
            throw NtpException(4, "Unable to set SND timeout");
        }

        if ((status = sendto(server_sock, &packet, sizeof(packet), 0, ap->ai_addr, ap->ai_addrlen)) == -1) {
            throw NtpException(5, "Unable to send packet");
        }

        if ((status = recvfrom(server_sock, &packet, sizeof(packet), 0, ap->ai_addr, &addrlen)) == -1) {
            if (errno == 11 || errno == 35) { // NX: 11, OTH: 35
                throw NtpException(6, "Connection timeout, retry. (" + std::to_string(m_timeout) + "s)");
            } else {
                throw NtpException(7, "Unable to receive packet (" + std::to_string(errno) + ")");
            }
        }

        freeaddrinfo(servinfo);
        close(server_sock);

        packet.recv_ts_secs = ntohl(packet.recv_ts_secs);

        return packet.recv_ts_secs - UNIX_OFFSET;
    }

    long getTimeOffset(time_t currentTime) noexcept(false) {
        time_t ntpTime = getTime();
        return currentTime - ntpTime;
    }
};

// Function to download a file given a URL and destination path
void downloadFile(const std::string& fileUrl, const std::string& toDestination) {
    // NTP client code here
    // ...

    // Create an NTP client instance
    NTPClient ntpClient;

    // Get the current time using NTP
    time_t currentTime;
    try {
        currentTime = ntpClient.getTime();
    } catch (const NtpException& e) {
        logMessage("Failed to retrieve current time from NTP: " + std::string(e.what()));
        return;
    }

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
