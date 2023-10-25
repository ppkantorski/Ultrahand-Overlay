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
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <jansson.h>
#include <regex>
#include <debug_funcs.hpp>


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
 * @brief Removes all white spaces from a string.
 *
 * This function removes all white spaces, including spaces, tabs, newlines, carriage returns, form feeds,
 * and vertical tabs from the input string.
 *
 * @param str The input string to remove white spaces from.
 * @return The string with white spaces removed.
 */
std::string removeWhiteSpaces(const std::string& str) {
    std::string result;
    for (char c : str) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            result.push_back(c);
        }
    }
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


/**
 * @brief Converts a string to an integer.
 *
 * This function attempts to convert the specified string to an integer.
 * If the conversion fails due to invalid input or out-of-range values,
 * it returns 0.
 *
 * @param input_string The string to convert to an integer.
 * @return The converted integer value or 0 on conversion failure.
 */
int stringToNumber(const std::string& input_string) {
    try {
        return std::stoi(input_string);
    } catch (const std::invalid_argument& e) {
        return 0;
    } catch (const std::out_of_range& e) {
        return 0;
    }
}


/**
 * @brief Splits a string into a vector of strings using a delimiter.
 *
 * This function splits the input string into multiple strings using the specified delimiter.
 *
 * @param str The input string to split.
 * @return A vector of strings containing the split values.
 */
std::vector<std::string> stringToList(const std::string& str) {
    std::string values, token;
    std::vector<std::string> result;
    
    // Check if the input string starts and ends with '(' and ')'
    if ((str.front() == '(' && str.back() == ')') || (str.front() == '[' && str.back() == ']')) {
        // Remove the parentheses
        values = str.substr(1, str.size() - 2);
        
        // Create a stringstream to split the string by commas
        std::istringstream ss(values);
        
        while (std::getline(ss, token, ',')) {
            // Remove any leading or trailing spaces from the token
            token = token.substr(token.find_first_not_of(" "), token.find_last_not_of(" ") + 1);
            result.push_back(token);
        }
    }
    
    return result;
}


/**
 * @brief Parses a JSON string into a json_t object.
 *
 * This function takes a JSON string as input and parses it into a json_t object using Jansson library's `json_loads` function.
 * If parsing fails, it returns an empty json_t object.
 *
 * @param input The input JSON string to parse.
 * @return A json_t object representing the parsed JSON, or an empty json_t object if parsing fails.
 */
json_t* stringToJson(const std::string& input) {
    json_t* jsonObj = nullptr;
    json_error_t error;
    
    logMessage(input.c_str());
    jsonObj = json_loads(input.c_str(), 0, &error);

    if (!jsonObj) {
        // Return an empty json_t* (you can also return nullptr)
        jsonObj = json_object();
        logMessage("ERROR LOADING JSON FROM STRING!");
    }

    return jsonObj;
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
std::string formatPriorityString(const std::string& priority, int desiredWidth=4) {
    std::string formattedString = priority;
    
    if (int(priority.length()) > desiredWidth) {
        formattedString = "9" + std::string(desiredWidth - 1, '9'); // Set to 9's if too long
    } else{
        while (int(formattedString.length()) < desiredWidth) {
            formattedString = "0"+formattedString;
        }
    }


    // Convert the stringstream to a string and return it
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
std::string removeTag(const std::string &input) {
    size_t pos = input.find('?');
    if (pos != std::string::npos) {
        return input.substr(0, pos);
    }
    return input; // Return the original string if no '?' is found.
}



// This will take a string like "v1.3.5-abasdfasdfa" and output "1.3.5". string could also look like "test-1.3.5-1" or "v1.3.5" and we will only want "1.3.5"
std::string cleanVersionLabel(const std::string &input) {
    std::regex versionRegex(R"([v-]?(\d+\.\d+\.\d+))");
    std::smatch match;

    if (std::regex_search(input, match, versionRegex)) {
        if (match.size() > 1) {
            return match[1].str();
        }
    }

    // Return an empty string if no version number is found
    return input;
}


std::string extractTitle(const std::string& input) {
    size_t spacePos = input.find(' '); // Find the position of the first space

    if (spacePos != std::string::npos) {
        // Extract the substring before the first space
        return input.substr(0, spacePos);
    } else {
        // If no space is found, return the original string
        return input;
    }
}
