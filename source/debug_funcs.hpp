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
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#pragma once
#include <cstdio>
#include <ctime>
#include <iostream>
#include <deque>

const std::string logFilePath = "sdmc:/config/ultrahand/log.txt";
const int maxLines = 3000;

std::deque<std::string> logQueue;

/**
 * @brief Logs a message with a timestamp to a log file.
 *
 * @param message The message to be logged.
 */
void logMessage(const std::string& message) {
    std::time_t currentTime = std::time(nullptr);
    std::string logEntry = std::asctime(std::localtime(&currentTime));
    logEntry = "[" + logEntry.substr(0, logEntry.length() - 1) + "] " + message;
    
    logQueue.push_back(logEntry);
    
    if (logQueue.size() > maxLines) {
        logQueue.pop_front();
    }
    
    FILE* file = fopen(logFilePath.c_str(), "a");
    if (file != nullptr) {
        fprintf(file, "%s\n", logEntry.c_str());
        fclose(file);
    }
}
