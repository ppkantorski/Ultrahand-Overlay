/********************************************************************************
 * File: string_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains function declarations and utility functions for string
 *   manipulation. These functions are used in the Ultrahand Overlay project to
 *   perform operations like trimming whitespaces, removing quotes, replacing
 *   multiple slashes, and more.
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
#include <string>
#include <iterator> 
#include <vector>
#include <jansson.h>
#include <regex>
#include <sys/stat.h>
#include <dirent.h>
#include "debug_funcs.hpp"

// Function to remove any non-alphanumeric characters from a string
//std::string replaceSingleQuote(const std::string& str) {
//    std::string result = str;
//    std::replace(result.begin(), result.end(), '\'', '_');
//    return result;
//}

// Function to remove invalid characters from file names
inline std::string cleanFileName(const std::string& fileName) {
    std::string cleanedFileName = fileName;
    cleanedFileName.erase(std::remove_if(cleanedFileName.begin(), cleanedFileName.end(), [](char c) {
        return !(std::isalnum(c) || std::isspace(c) || c == '-' || c == '_');
    }), cleanedFileName.end());
    return cleanedFileName;
}

// Function to clean directory names by removing invalid characters
inline std::string cleanDirectoryName(const std::string& name) {
    std::string cleanedName = name;
    cleanedName.erase(std::remove_if(cleanedName.begin(), cleanedName.end(), [](char c) {
        return !(std::isalnum(c) || std::isspace(c) || c == '-' || c == '_');
    }), cleanedName.end());
    return cleanedName;
}

/**
 * @brief Trims leading and trailing whitespaces from a string.
 *
 * This function removes leading and trailing whitespaces, tabs, newlines, carriage returns, form feeds,
 * and vertical tabs from the input string.
 *
 * @param str The input string to trim.
 * @return The trimmed string.
 */
inline std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r\f\v");
    if (first == std::string::npos)
        return {};  // Use {} to avoid an extra constructor call

    size_t last = str.find_last_not_of(" \t\n\r\f\v");
    return str.substr(first, last - first + 1);
}


// Function to trim newline characters from the end of a string
inline std::string trimNewline(const std::string &str) {
    size_t end = str.find_last_not_of("\n");
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

/**
 * @brief Removes all white spaces from a string.
 *
 * This function removes all white spaces, including spaces, tabs, newlines, carriage returns, form feeds,
 * and vertical tabs from the input string.
 *
 * @param str The input string to remove white spaces from.
 * @return The string with white spaces removed.
 */
inline std::string removeWhiteSpaces(const std::string& str) {
    std::string result;
    result.reserve(str.size()); // Reserve space for the result to avoid reallocations
    
    std::remove_copy_if(str.begin(), str.end(), std::back_inserter(result), [](unsigned char c) {
        return std::isspace(c);
    });
    
    return result;
}



/**
 * @brief Removes quotes from a string.
 *
 * This function removes single and double quotes from the beginning and end of the input string.
 *
 * @param str The input string to remove quotes from.
 * @return The string with quotes removed.
 */
inline std::string removeQuotes(const std::string& str) {
    if (str.size() >= 2) {
        char frontQuote = str.front();
        char backQuote = str.back();
        if ((frontQuote == '\'' && backQuote == '\'') || (frontQuote == '"' && backQuote == '"')) {
            return str.substr(1, str.size() - 2);
        }
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
inline std::string replaceMultipleSlashes(const std::string& input) {
    std::string output;
    output.reserve(input.size()); // Reserve space for the output string
    
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
inline std::string_view removeLeadingSlash(const std::string_view& pathPattern) {
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
inline std::string removeEndingSlash(const std::string& pathPattern) {
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
inline std::string preprocessPath(const std::string& path, const std::string& packagePath = "") {
    std::string formattedPath = replaceMultipleSlashes(removeQuotes(path));

    // Replace "./" at the beginning of the path with the packagePath
    if (!packagePath.empty() && formattedPath.substr(0, 2) == "./") {
        formattedPath = packagePath + formattedPath.substr(2);
    }

    // Ensure all paths start with "sdmc:"
    if (formattedPath.substr(0, 5) != "sdmc:") {
        formattedPath = "sdmc:" + formattedPath;
    }

    return formattedPath;
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
inline std::string preprocessUrl(const std::string& path) {
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
inline std::string dropExtension(std::string filename) {
    size_t lastDotPos = filename.find_last_of(".");
    if (lastDotPos != std::string::npos) {
        filename.resize(lastDotPos); // Resize the string to remove the extension
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
inline bool startsWith(const std::string& str, const std::string& prefix) {
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
inline bool isDirectory(const std::string& path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0) {
        return S_ISDIR(pathStat.st_mode);
    }
    return false;
}



/**
 * @brief Checks if a path points to a file.
 *
 * This function checks if the specified path points to a file.
 *
 * @param path The path to check.
 * @return True if the path is a file, false otherwise.
 */
inline bool isFile(const std::string& path) {
    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) == 0) {
        return S_ISREG(pathStat.st_mode);
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
inline bool isFileOrDirectory(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}



// Helper function to check if a string is a valid integer
inline bool isValidNumber(const std::string& str) {
    if (str.empty() || ((str[0] != '-') && !std::isdigit(str[0])) || (str[0] == '-' && str.size() == 1)) {
        return false;
    }
    for (size_t i = 1; i < str.size(); ++i) {
        if (!std::isdigit(str[i])) {
            return false;
        }
    }
    return true;
}


// Function to slice a string from start to end index
inline std::string sliceString(const std::string& str, size_t start, size_t end) {
    if (start < 0) start = 0;
    if (end > static_cast<size_t>(str.length())) end = str.length();
    if (start > end) start = end;
    return str.substr(start, end - start);
}



//inline std::string addQuotesIfNeeded(const std::string& str) {
//    if (str.find(' ') != std::string::npos) {
//        return "\"" + str + "\"";
//    }
//    return str;
//}


/**
 * @brief Converts a string to lowercase.
 *
 * This function takes a string as input and returns a lowercase version of that string.
 *
 * @param str The input string to convert to lowercase.
 * @return The lowercase version of the input string.
 */

inline std::string stringToLowercase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}


/**
 * @brief Formats a priority string to a desired width.
 *
 * This function takes a priority string and formats it to a specified desired width by padding with '0's if it's shorter
 * or truncating with '9's if it's longer.
 *
 * @param priority The input priority string to format.
 * @param desiredWidth The desired width of the formatted string (default is 4).
 * @return A formatted priority string.
 */
inline std::string formatPriorityString(const std::string& priority, int desiredWidth = 4) {
    std::string formattedString;
    int priorityLength = priority.length();
    
    if (priorityLength > desiredWidth) {
        formattedString = std::string(desiredWidth, '9'); // Set to 9's if too long
    } else {
        formattedString = std::string(desiredWidth - priorityLength, '0') + priority;
    }
    
    return formattedString;
}



/**
 * @brief Removes the part of the string after the first occurrence of '?' character.
 *
 * This function takes a string and removes the portion of the string that appears after
 * the first '?' character, if found. If no '?' character is present, the original string
 * is returned unchanged.
 *
 * @param input The input string from which to remove the tag.
 * @return The input string with everything after the first '?' character removed.
 */
inline std::string removeTag(const std::string &input) {
    size_t pos = input.find('?');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return input; // Return the original string if no '?' is found.
}


inline std::string getFirstLongEntry(const std::string& input, size_t minLength = 8) {
    std::istringstream iss(input);
    std::string word;

    // Split the input string based on spaces and get the first word
    if (iss >> word) {
        // Check if the first word's length is greater than the specified length
        if (word.length() > minLength) {
            return word;
        }
    }
    
    // Return an empty string if the first word is not longer than minLength
    return input;
}


// This will take a string like "v1.3.5-abasdfasdfa" and output "1.3.5". string could also look like "test-1.3.5-1" or "v1.3.5" and we will only want "1.3.5"
inline std::string cleanVersionLabel(const std::string& input) {
    std::string versionLabel;
    std::string prefix; // To store the preceding characters
    versionLabel.reserve(input.size()); // Reserve space for the output string

    bool foundDigit = false;
    for (char c : input) {
        if (std::isdigit(c) || c == '.') {
            if (!foundDigit) {
                // Include the prefix in the version label before adding digits
                if (!prefix.empty() && prefix.back() != 'v') {
                    versionLabel += prefix;
                }
                prefix.clear();
            }
            versionLabel += c;
            foundDigit = true;
        } else {
            if (!foundDigit) {
                prefix += c; // Add to prefix until a digit is found
            } else {
                // Stop at the first non-digit character after encountering digits
                break;
            }
        }
    }
    
    return versionLabel;
}


inline std::string extractTitle(const std::string& input) {
    size_t spacePos = input.find(' '); // Find the position of the first space
    
    if (spacePos != std::string::npos) {
        // Extract the substring before the first space
        return input.substr(0, spacePos);
    } else {
        // If no space is found, return the original string
        return input;
    }
}


inline std::string removeFilename(const std::string& path) {
    size_t found = path.find_last_of('/');
    if (found != std::string::npos) {
        return path.substr(0, found + 1);
    }
    return path; // If no directory separator is found, return the original path
}


// Function to split a string by a delimiter and return a specific index
inline std::string splitString(const std::string& str, const std::string& delimiter, size_t index) {
    std::vector<std::string> tokens;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        tokens.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    tokens.push_back(str.substr(start));

    if (index < tokens.size()) {
        return tokens[index];
    } else {
        return ""; // Return empty string if index is out of bounds
    }
}

std::string createHiddenFilePath(const std::string& originalPath) {
    // Find the position of the last '/'
    size_t lastSlash = originalPath.find_last_of('/');

    // Extract the directory and filename
    std::string directory = (lastSlash == std::string::npos) ? "" : originalPath.substr(0, lastSlash);
    std::string filename = (lastSlash == std::string::npos) ? originalPath : originalPath.substr(lastSlash + 1);

    // Add '.' to the start of the filename to make it hidden
    std::string hiddenFilename = "." + filename;

    // Combine the directory with the hidden filename
    std::string hiddenFilePath = directory.empty() ? hiddenFilename : directory + "/" + hiddenFilename;

    return hiddenFilePath;
}


//std::string padToEqualLength(const std::string& str, size_t len) {
//    if (str.length() < len)
//        return str + std::string(len - str.length(), '\0');
//    return str.substr(0, len);
//}
