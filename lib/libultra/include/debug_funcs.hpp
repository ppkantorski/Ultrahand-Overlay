/********************************************************************************
 * File: debug_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains debugging functions for the Ultrahand Overlay project.
 *   These functions allow logging messages with timestamps to a log file.
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
#include <mutex>

// Specify the log file path
const std::string defaultLogFilePath = "sdmc:/switch/.packages/log.txt";

static std::string logFilePath = defaultLogFilePath; 
static bool disableLogging = true;

// Global mutex for thread-safe logging
std::mutex logMutex;

/**
 * @brief Logs a message with a timestamp to a log file in a thread-safe manner.
 *
 * @param message The message to be logged.
 */
void logMessage(const std::string& message) {
    if (disableLogging)
        return;
    std::time_t currentTime = std::time(nullptr);
    std::tm* timeInfo = std::localtime(&currentTime);
    char buffer[30];
    strftime(buffer, 30, "[%Y-%m-%d %H:%M:%S] ", timeInfo);
    std::string timestamp(buffer);

    //std::string logEntry = timestamp + message + "\n";

    // Open the file with std::ofstream in append mode inside the lock
    {
        std::lock_guard<std::mutex> lock(logMutex); // Locks the mutex for the duration of this block

        std::ofstream file(logFilePath.c_str(), std::ios::app);
        if (file.is_open()) {
            file << timestamp + message + "\n";
        } else {
            // Handle error when file opening fails, such as logging to an alternative output or retrying
            //std::cerr << "Failed to open log file: " << logFilePath << std::endl;
        }
        file.close();
    } // file closes automatically upon leaving this block
}
