/********************************************************************************
 * File: debug_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file contains the implementation of debugging functions for the
 *   Ultrahand Overlay project.
 ********************************************************************************/

#include "debug_funcs.hpp"

namespace ult {
    // Define static variables
    std::string logFilePath = defaultLogFilePath; 
    bool disableLogging = true;
    std::mutex logMutex;

    void logMessage(const std::string& message) {
        #if IS_LAUNCHER_DIRECTIVE
        if (disableLogging)
            return;

        std::time_t currentTime = std::time(nullptr);
        std::tm* timeInfo = std::localtime(&currentTime);
        char buffer[30];
        strftime(buffer, sizeof(buffer), "[%Y-%m-%d %H:%M:%S] ", timeInfo);
        std::string timestamp(buffer);

        // Open the file with std::ofstream in append mode inside the lock
        {
            std::lock_guard<std::mutex> lock(logMutex); // Locks the mutex for the duration of this block

            std::ofstream file(logFilePath.c_str(), std::ios::app);
            if (file.is_open()) {
                file << timestamp + message + "\n";
            } else {
                // Handle error when file opening fails
                // std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            }
        } // file closes automatically upon leaving this block
        #endif
    }
}
