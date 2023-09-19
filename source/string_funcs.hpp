/********************************************************************************
 * File: string_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains function declarations and utility functions for string
 *   manipulation. These functions are used in the Ultrahand-Overlay project to
 *   perform operations like trimming whitespaces, removing quotes, replacing
 *   multiple slashes, and more.
 *
 *   For the latest updates and contributions, visit the project's GitHub repository.
 *   (GitHub Repository: https://github.com/ppkantorski/Ultrahand-Overlay)
 *
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#pragma once
#include <string>

/**
 * @brief Trims leading and trailing whitespaces from a string.
 *
 * This function removes leading and trailing whitespaces, tabs, newlines, carriage returns, form feeds,
 * and vertical tabs from the input string.
 *
 * @param str The input string to trim.
 * @return The trimmed string.
 */
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    if (first == std::string::npos || last == std::string::npos)
        return "";
    return str.substr(first, last - first + 1);
}

/**
 * @brief Removes quotes from a string.
 *
 * This function removes single and double quotes from the beginning and end of the input string.
 *
 * @param str The input string to remove quotes from.
 * @return The string with quotes removed.
 */
std::string removeQuotes(const std::string& str) {
    std::size_t firstQuote = str.find_first_of("'\"");
    std::size_t lastQuote = str.find_last_of("'\"");
    if (firstQuote != std::string::npos && lastQuote != std::string::npos && firstQuote < lastQuote) {
        return str.substr(firstQuote + 1, lastQuote - firstQuote - 1);
    }
    return str;
}

/**
 * @brief Replaces multiple consecutive slashes with a single slash in a string.
 *
 * This function replaces sequences of two or more consecutive slashes with a single slash in the input string.
 *
 * @param input The input string to process.
 * @return The string with multiple slashes replaced.
 */
std::string replaceMultipleSlashes(const std::string& input) {
    std::string output;
    bool previousSlash = false;

    for (char c : input) {
        if (c == '/') {
            if (!previousSlash) {
                output.push_back(c);
            }
            previousSlash = true;
        } else {
            output.push_back(c);
            previousSlash = false;
        }
    }

    return output;
}

/**
 * @brief Removes the leading slash from a string if present.
 *
 * This function removes the leading slash ('/') from the input string if it exists.
 *
 * @param pathPattern The input string representing a path pattern.
 * @return The string with the leading slash removed.
 */
std::string removeLeadingSlash(const std::string& pathPattern) {
    if (!pathPattern.empty() && pathPattern[0] == '/') {
        return pathPattern.substr(1);
    }
    return pathPattern;
}

/**
 * @brief Removes the trailing slash from a string if present.
 *
 * This function removes the trailing slash ('/') from the input string if it exists.
 *
 * @param pathPattern The input string representing a path pattern.
 * @return The string with the trailing slash removed.
 */
std::string removeEndingSlash(const std::string& pathPattern) {
    if (!pathPattern.empty() && pathPattern.back() == '/') {
        return pathPattern.substr(0, pathPattern.length() - 1);
    }
    return pathPattern;
}

/**
 * @brief Preprocesses a path string by replacing multiple slashes and adding "sdmc:" prefix.
 *
 * This function preprocesses a path string by removing multiple consecutive slashes,
 * adding the "sdmc:" prefix if not present, and returning the resulting string.
 *
 * @param path The input path string to preprocess.
 * @return The preprocessed path string.
 */
std::string preprocessPath(const std::string& path) {
    std::string formattedPath = replaceMultipleSlashes(removeQuotes(path));
    if (formattedPath.compare(0, 5, "sdmc:") != 0) {
        return std::string("sdmc:") + formattedPath;
    } else {
        return formattedPath;
    }
}

/**
 * @brief Preprocesses a URL string by adding "https://" prefix.
 *
 * This function preprocesses a URL string by adding the "https://" prefix if not already present,
 * and returns the resulting URL string.
 *
 * @param path The input URL string to preprocess.
 * @return The preprocessed URL string.
 */
std::string preprocessUrl(const std::string& path) {
    std::string formattedPath = removeQuotes(path);
    if ((formattedPath.compare(0, 7, "http://") == 0) || (formattedPath.compare(0, 8, "https://") == 0)) {
        return formattedPath;
    } else {
        return std::string("https://") + formattedPath;
    }
}

/**
 * @brief Drops the file extension from a filename.
 *
 * This function removes the file extension (characters after the last dot) from the input filename string.
 *
 * @param filename The input filename from which to drop the extension.
 * @return The filename without the extension.
 */
std::string dropExtension(const std::string& filename) {
    size_t lastDotPos = filename.find_last_of(".");
    if (lastDotPos != std::string::npos) {
        return filename.substr(0, lastDotPos);
    }
    return filename;
}

/**
 * @brief Checks if a string starts with a given prefix.
 *
 * This function checks if the input string starts with the specified prefix.
 *
 * @param str The input string to check.
 * @param prefix The prefix to check for.
 * @return True if the string starts with the prefix, false otherwise.
 */
bool startsWith(const std::string& str, const std::string& prefix) {
    return str.compare(0, prefix.length(), prefix) == 0;
}

/**
 * @brief Checks if a path points to a directory.
 *
 * This function checks if the specified path points to a directory.
 *
 * @param path The path to check.
 * @return True if the path is a directory, false otherwise.
 */
bool isDirectory(const std::string& path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);
    }
    return false;
}

/**
 * @brief Checks if a path points to a file or directory.
 *
 * This function checks if the specified path points to either a file or a directory.
 *
 * @param path The path to check.
 * @return True if the path points to a file or directory, false otherwise.
 */
bool isFileOrDirectory(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}
