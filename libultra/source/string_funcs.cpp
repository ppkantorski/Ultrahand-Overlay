/********************************************************************************
 * File: string_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file provides implementations for the string manipulation
 *   functions declared in string_funcs.hpp. These utility functions support
 *   operations such as trimming whitespace, removing quotes, normalizing
 *   slashes, and performing other string cleanup tasks used throughout
 *   the Ultrahand Overlay project.
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

#include "string_funcs.hpp"

namespace ult {


    // Custom string conversion methods in place of std::
    std::string to_string(int value) {
        char buffer[12]; // Sufficient for 32-bit int
        snprintf(buffer, sizeof(buffer), "%d", value);
        return std::string(buffer);
    }
    
    int stoi(const std::string& str, std::size_t* pos, int base) {
        char* end;
        const long result = std::strtol(str.c_str(), &end, base);
    
        if (pos) {
            *pos = end - str.c_str();  // Set the position to the last character processed
        }
    
        // Handle out-of-range or conversion issues here if needed
        return static_cast<int>(result);
    }

    
    float stof(const std::string& str) {
        return strtof(str.c_str(), nullptr);
    }


    // Mimics std::getline() with a delimiter
    bool StringStream::getline(std::string& output, char delimiter) {
        if (position >= data.size()) {
            return false;
        }
        
        const size_t nextPos = data.find(delimiter, position);
        
        if (nextPos != std::string::npos) {
            output.assign(data, position, nextPos - position);  // No temporary string creation
            position = nextPos + 1;
        } else {
            output.assign(data, position, data.size() - position);  // No temporary string creation
            position = data.size();
        }
    
        return true;
    }
    
    // Mimics operator >> to split by whitespace
    StringStream& StringStream::operator>>(std::string& output) {
        // Skip leading whitespace
        while (position < data.size() && std::isspace(data[position])) {
            ++position;
        }
    
        if (position >= data.size()) {
            output.clear();
            validState = false;
            return *this;
        }
    
        size_t nextPos = position;
        while (nextPos < data.size() && !std::isspace(data[nextPos])) {
            ++nextPos;
        }
    
        output.assign(data, position, nextPos - position);  // Replace substr() with assign()
        position = nextPos;
    
        validState = true;
        return *this;
    }
    
    // Overload << operator for std::string
    StringStream& StringStream::operator<<(const std::string& input) {
        data += input;
        return *this;
    }
    
    // Overload << operator for const char*
    StringStream& StringStream::operator<<(const char* input) {
        data += input;
        return *this;
    }
    
    // Overload << operator for char
    StringStream& StringStream::operator<<(char input) {
        data += input;
        return *this;
    }
    
    // Overload << operator for int (handles hex mode)
    StringStream& StringStream::operator<<(int input) {
        if (hexMode) {
            char buffer[20];  // Buffer large enough for hex conversion
            sprintf(buffer, "%x", input);  // Convert integer to hex string
            data += buffer;
        } else {
            data += ult::to_string(input);
        }
        return *this;
    }

    // Define the new overload for long long
    StringStream& StringStream::operator<<(long long input) {
        data += std::to_string(input);
        return *this;
    }
    
    // Return the current buffer content
    std::string StringStream::str() const {
        return data;
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
    void trim(std::string& str) {
        const size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) {
            str.clear(); // Fix: clear all-whitespace strings
            return;
        }
    
        const size_t last = str.find_last_not_of(" \t\n\r\f\v");
        
        // True in-place modification - no temporary string creation
        if (last + 1 < str.length()) {
            str.erase(last + 1);  // Remove trailing whitespace
        }
        if (first > 0) {
            str.erase(0, first);  // Remove leading whitespace  
        }
    }
    
    
    // Function to trim newline characters from the end of a string
    void trimNewline(std::string& str) {
        const size_t end = str.find_last_not_of("\n");
        if (end == std::string::npos) {
            str.clear();  // If the string consists entirely of newlines, clear it
        } else {
            str.erase(end + 1);  // Remove all characters after the last non-newline character
        }
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
    void removeQuotes(std::string& str) {
        if (str.size() >= 2) {
            const char front = str[0];
            const char back = str[str.size() - 1];
            if ((front == '\'' && back == '\'') || (front == '"' && back == '"')) {
                str.erase(0, 1);
                str.pop_back();
            }
        }
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
     * @brief Preprocesses a path string by replacing multiple slashes and adding "sdmc:" prefix.
     *
     * This function preprocesses a path string by removing multiple consecutive slashes,
     * adding the "sdmc:" prefix if not present, and modifying the input string in place.
     *
     * @param path The input path string to preprocess, passed by reference.
     */
    void preprocessPath(std::string& path, const std::string& packagePath) {
        removeQuotes(path);
        
        // In-place multiple slash removal - no temporary string creation
        if (!path.empty()) {
            size_t writePos = 0;
            bool previousSlash = false;
            
            for (size_t i = 0; i < path.length(); ++i) {
                if (path[i] == '/') {
                    if (!previousSlash) {
                        path[writePos++] = path[i];
                    }
                    previousSlash = true;
                } else {
                    path[writePos++] = path[i];
                    previousSlash = false;
                }
            }
            path.resize(writePos);
        }
    
        // Direct character comparison instead of substr()
        if (!packagePath.empty() && path.length() >= 2 && path[0] == '.' && path[1] == '/') {
            path.replace(0, 2, packagePath);
        }
    
        // Direct character comparison instead of substr()
        if (path.length() < 5 || 
            path[0] != 's' || path[1] != 'd' || path[2] != 'm' || path[3] != 'c' || path[4] != ':') {
            path.insert(0, "sdmc:");
        }
    }
    
    /**
     * @brief Preprocesses a URL string by adding "https://" prefix.
     *
     * This function preprocesses a URL string by adding the "https://" prefix if not already present.
     *
     * @param path The input URL string to preprocess, passed by reference and modified in-place.
     */
    void preprocessUrl(std::string& path) {
        removeQuotes(path);
        if ((path.compare(0, 7, "http://") == 0) || (path.compare(0, 8, "https://") == 0)) {
            return; // No need to modify the string if it already has a prefix
        } else {
            path = "https://" + path; // Prepend "https://"
        }
    }
    
    /**
     * @brief Drops the file extension from a filename.
     *
     * This function removes the file extension (characters after the last dot) from the input filename string.
     *
     * @param filename The input filename from which to drop the extension, passed by reference and modified in-place.
     */
    void dropExtension(std::string& filename) {
        const size_t lastDotPos = filename.find_last_of('.');  // Single char instead of string
        if (lastDotPos != std::string::npos) {
            filename.resize(lastDotPos);
        }
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
    
    
    
    
    // Helper function to check if a string is a valid integer
    bool isValidNumber(const std::string& str) {
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
    
    std::string returnOrNull(const std::string& value) {
        return value.empty() ? NULL_STR : value;
    }

    
    // Function to slice a string from start to end index
    std::string sliceString(const std::string& str, size_t start, size_t end) {
        if (start < 0) start = 0;
        if (end > static_cast<size_t>(str.length())) end = str.length();
        if (start > end) start = end;
        return str.substr(start, end - start);
    }
    
    
    
    //std::string addQuotesIfNeeded(const std::string& str) {
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
    
    std::string stringToLowercase(const std::string& str) {
        std::string result = str;
        //std::transform(result.begin(), result.end(), result.begin(),
        //               [](unsigned char c) { return std::tolower(c); });
        //return result;
        for (char& c : result) {
            if (c >= 'A' && c <= 'Z') {
                c += 32;
            }
        }
        return result;
    }
    
    /**
     * @brief Converts a string to uppercase.
     *
     * This function takes a string as input and returns an uppercase version of that string.
     *
     * @param str The input string to convert to uppercase.
     * @return The uppercase version of the input string.
     */
    
    std::string stringToUppercase(const std::string& str) {
        std::string result = str;
        for (char& c : result) {
            if (c >= 'a' && c <= 'z') {
                c -= 32;
            }
        }
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
    std::string formatPriorityString(const std::string& priority, int desiredWidth) {
        const int priorityLength = priority.length();
        
        if (priorityLength > desiredWidth) {
            // FASTEST: Single allocation with direct fill
            return std::string(desiredWidth, '9');
        } else {
            // FASTEST: Single allocation + direct memory copy
            std::string result(desiredWidth, '0');  // Pre-fill with zeros
            memcpy(&result[desiredWidth - priorityLength], priority.data(), priorityLength);
            return result;
        }
    }
    
    
    
    /**
     * @brief Removes the part of the string after the first occurrence of '?' character.
     *
     * This function takes a string and removes the portion of the string that appears after
     * the first '?' character, if found. If no '?' character is present, the string remains unchanged.
     *
     * @param input The input string from which to remove the tag, passed by reference and modified in-place.
     */
    void removeTag(std::string &input) {
        const size_t pos = input.find('?');
        if (pos != std::string::npos) {
            input.resize(pos); // Modify the string in-place to remove everything after the '?'
        }
    }
    
    
    std::string getFirstLongEntry(const std::string& input, size_t minLength) {
        StringStream iss(input);  // Use custom StringStream
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
    std::string cleanVersionLabel(const std::string& input) {
        std::string result;
        result.reserve(input.size());
        
        size_t start = 0;
        
        // Find the start of the version number (first digit)
        while (start < input.size() && !std::isdigit(input[start])) {
            start++;
        }
        
        if (start == input.size()) {
            return ""; // No digits found
        }
        
        
        // Extract version number with dots and plus signs
        for (size_t i = start; i < input.size(); ++i) {
            const char c = input[i];
            if (std::isdigit(c) || c == '.' || c == '+') {
                result += c;
            } else {
                break; // Stop at first character that's not digit, dot, or plus
            }
        }
        
        return result;
    }
    
    
    std::string extractTitle(const std::string& input) {
        const size_t spacePos = input.find(' '); // Find the position of the first space
        
        if (spacePos != std::string::npos) {
            // Extract the substring before the first space
            return input.substr(0, spacePos);
        } else {
            // If no space is found, return the original string
            return input;
        }
    }
    
    
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> tokens;
        
        // OPTIMIZATION: Pre-allocate space to avoid reallocations
        tokens.reserve(str.length() / (delimiter.length() + 1) + 1);
        
        size_t start = 0;
        size_t end = str.find(delimiter);
        
        while (end != std::string::npos) {
            // OPTIMIZATION: Direct construction instead of substr() - no temporary string
            tokens.emplace_back(str, start, end - start);
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        
        // OPTIMIZATION: Direct construction for last token
        tokens.emplace_back(str, start);
        
        return tokens;
    }
    
    
    // Function to split a string by a delimiter and return a specific index
    std::string splitStringAtIndex(const std::string& str, const std::string& delimiter, size_t index) {
        const std::vector<std::string> tokens = splitString(str, delimiter);
    
        if (index < tokens.size()) {
            return tokens[index];
        } else {
            return ""; // Return empty string if index is out of bounds
        }
    }
    
    
    std::string customAlign(int number) {
        const std::string numStr = ult::to_string(number);
        const int paddingSpaces = (4 - numStr.length()) * 2;
        
        // FASTEST: Single allocation + direct memory operations
        std::string result(paddingSpaces + numStr.length(), ' ');
        memcpy(&result[paddingSpaces], numStr.data(), numStr.length());
        
        return result;
    }

    //#if IS_LAUNCHER_DIRECTIVE
    //std::string inputExists(const std::string& input) {
    //    std::string e;
    //    for (char c : input) {
    //        e += (c + 5);
    //    }
    //    return e;
    //}
    //#endif
}
