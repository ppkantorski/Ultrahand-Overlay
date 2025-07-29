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
#ifndef DEBUG_FUNCS_HPP
#define DEBUG_FUNCS_HPP

#if !USING_FSTREAM_DIRECTIVE // For not using fstream (needs implementing)
#include <stdio.h>
#else
#include <fstream>
#endif
#include <mutex>
#include <string>
#include <ctime>

namespace ult {
    #if USING_LOGGING_DIRECTIVE

    // Specify the log file path
    extern const std::string defaultLogFilePath;
    
    extern std::string logFilePath;  // Declare logFilePath as extern
    extern bool disableLogging;        // Declare disableLogging as extern
    
    // Global mutex for thread-safe logging
    extern std::mutex logMutex;        // Declare logMutex as extern

    /**
     * @brief Logs a message with a timestamp to a log file in a thread-safe manner.
     *
     * @param message The message to be logged.
     */
    void logMessage(const std::string& message);
    #endif
}

#endif // DEBUG_FUNCS_HPP
