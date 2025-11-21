/********************************************************************************
 * File: debug_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file contains the implementation of debugging functions for the
 *   Ultrahand Overlay project.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *   Note: Please be aware that this notice cannot be altered or removed. It is a part
 *   of the project's documentation and must remain intact.
 * 
 *  Licensed under both GPLv2 and CC-BY-4.0
 *  Copyright (c) 2023-2025 ppkantorski
 ********************************************************************************/

#include "debug_funcs.hpp"

namespace ult {
    #if USING_LOGGING_DIRECTIVE
    // Define static variables
    const std::string defaultLogFilePath = "sdmc:/switch/.packages/log.txt";
    std::string logFilePath = defaultLogFilePath; 
    bool disableLogging = true;
    std::mutex logMutex;
    
    void logMessage(const char* message) {
        std::time_t currentTime = std::time(nullptr);
        std::tm* timeInfo = std::localtime(&currentTime);
        char timestamp[30];
        strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeInfo);
        
        #if !USING_FSTREAM_DIRECTIVE
        {
            std::lock_guard<std::mutex> lock(logMutex);
            
            FILE* file = fopen(logFilePath.c_str(), "a");
            if (file != nullptr) {
                fputs(timestamp, file);
                fputs(message, file);
                fputc('\n', file);
                fclose(file);
            }
        }
        #else
        {
            std::lock_guard<std::mutex> lock(logMutex);
            
            std::ofstream file(logFilePath.c_str(), std::ios::app);
            if (file.is_open()) {
                file << timestamp << message << "\n";
            }
        }
        #endif
    }
    
    // Overload for std::string
    void logMessage(const std::string& message) {
        logMessage(message.c_str());
    }
    #endif
}