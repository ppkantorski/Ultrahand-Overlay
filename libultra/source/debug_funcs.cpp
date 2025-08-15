/********************************************************************************
 * File: debug_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file contains the implementation of debugging functions for the
 *   Ultrahand Overlay project.
 ********************************************************************************/

#include "debug_funcs.hpp"

namespace ult {
    #if USING_LOGGING_DIRECTIVE
    // Define static variables
    const std::string defaultLogFilePath = "sdmc:/switch/.packages/log.txt";

    std::string logFilePath = defaultLogFilePath; 
    bool disableLogging = true;
    std::mutex logMutex;
    
    void logMessage(const std::string& message) {
        //if (disableLogging)
        //    return;
        
        std::time_t currentTime = std::time(nullptr);
        std::tm* timeInfo = std::localtime(&currentTime);
        char timestamp[30]; // Exact size for "[YYYY-MM-DD HH:MM:SS] " + null terminator
        strftime(timestamp, sizeof(timestamp), "[%Y-%m-%d %H:%M:%S] ", timeInfo);
        
        // Depending on the directive, use either std::ofstream or stdio functions
        #if !USING_FSTREAM_DIRECTIVE
        // Use stdio functions to open, write, and close the file
        {
            std::lock_guard<std::mutex> lock(logMutex); // Locks the mutex for thread-safe access
            
            FILE* file = fopen(logFilePath.c_str(), "a"); // Open the file in append mode
            if (file != nullptr) {
                fprintf(file, "%s%s\n", timestamp, message.c_str());
                fclose(file); // Close the file after writing
            } else {
                // Handle error when file opening fails (if necessary)
                // printf("Failed to open log file: %s\n", logFilePath.c_str());
            }
        }
        #else
        // Use std::ofstream if !USING_FSTREAM_DIRECTIVE is not defined
        {
            std::lock_guard<std::mutex> lock(logMutex); // Locks the mutex for the duration of this block
            
            std::ofstream file(logFilePath.c_str(), std::ios::app);
            if (file.is_open()) {
                file << timestamp << message << "\n";
            } else {
                // Handle error when file opening fails
                // std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            }
        }
        #endif
    }
    #endif
}