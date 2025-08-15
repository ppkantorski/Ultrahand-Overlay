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

#ifndef STRING_FUNCS_HPP
#define STRING_FUNCS_HPP

#include <cstring>
#include <string>
#include <iterator> 
#include <vector>
//#include <jansson.h>
//#include <regex>
#include <algorithm>
#include <sys/stat.h>
#include <dirent.h>
#include "global_vars.hpp"
#include "debug_funcs.hpp"

namespace ult {
    
    extern std::string to_string(int value);
    extern int stoi(const std::string& str, std::size_t* pos = nullptr, int base = 10);
    extern float stof(const std::string& str);


    /**
     * @brief A lightweight string stream class that mimics basic functionality of std::istringstream.
     */
    class StringStream {
    public:
        StringStream() : position(0), hexMode(false), validState(true) {}
    
    
        // Add this constructor to accept a string
        StringStream(const std::string& input) : data(input), position(0), hexMode(false) {}
    
        // Set hex mode
        StringStream& hex() {
            hexMode = true;
            return *this;
        }
    
        // Reset hex mode
        StringStream& resetHex() {
            hexMode = false;
            return *this;
        }
    
        // Mimics std::getline() with a delimiter
        bool getline(std::string& output, char delimiter);
    
        // Mimics operator >> to split by whitespace
        StringStream& operator>>(std::string& output);
    
        // Overload the << operator to insert strings and integers
        StringStream& operator<<(const std::string& input);
        StringStream& operator<<(const char* input);
        StringStream& operator<<(char input);
        StringStream& operator<<(int input);  // Handles int insertion with hex support
        StringStream& operator<<(long long input); // for long long
    
        // Conversion to bool for checking stream state (success/failure)
        explicit operator bool() const {
            return validState;
        }
    
        std::string str() const;
        void clear() { data.clear(); position = 0; } // Add clear function
    
    private:
        std::string data;
        size_t position;
        bool hexMode;
        bool validState;  // Track if the stream is in a valid state
    };
    


    /**
     * @brief Trims leading and trailing whitespaces from a string.
     *
     * This function removes leading and trailing whitespaces, tabs, newlines, carriage returns, form feeds,
     * and vertical tabs from the input string.
     *
     * @param str The input string to trim.
     * @return The trimmed string.
     */
    void trim(std::string& str);
    
    
    
    // Function to trim newline characters from the end of a string
    void trimNewline(std::string& str);
    
    
    /**
     * @brief Removes all white spaces from a string.
     *
     * This function removes all white spaces, including spaces, tabs, newlines, carriage returns, form feeds,
     * and vertical tabs from the input string.
     *
     * @param str The input string to remove white spaces from.
     * @return The string with white spaces removed.
     */
    std::string removeWhiteSpaces(const std::string& str);
    
    
    
    /**
     * @brief Removes quotes from a string.
     *
     * This function removes single and double quotes from the beginning and end of the input string.
     *
     * @param str The input string to remove quotes from.
     * @return The string with quotes removed.
     */
    void removeQuotes(std::string& str);
    
    
    /**
     * @brief Replaces multiple consecutive slashes with a single slash in a string.
     *
     * This function replaces sequences of two or more consecutive slashes with a single slash in the input string.
     *
     * @param input The input string to process.
     * @return The string with multiple slashes replaced.
     */
    std::string replaceMultipleSlashes(const std::string& input);
    
    
    void resolveDirectoryTraversal(std::string& path);
    
    /**
     * @brief Preprocesses a path string by replacing multiple slashes and adding "sdmc:" prefix.
     *
     * This function preprocesses a path string by removing multiple consecutive slashes,
     * adding the "sdmc:" prefix if not present, and modifying the input string in place.
     *
     * @param path The input path string to preprocess, passed by reference.
     */
    void preprocessPath(std::string& path, const std::string& packagePath = "");
    
    /**
     * @brief Preprocesses a URL string by adding "https://" prefix.
     *
     * This function preprocesses a URL string by adding the "https://" prefix if not already present.
     *
     * @param path The input URL string to preprocess, passed by reference and modified in-place.
     */
    void preprocessUrl(std::string& path);
    
    /**
     * @brief Drops the file extension from a filename.
     *
     * This function removes the file extension (characters after the last dot) from the input filename string.
     *
     * @param filename The input filename from which to drop the extension, passed by reference and modified in-place.
     */
    void dropExtension(std::string& filename);
    
    /**
     * @brief Checks if a string starts with a given prefix.
     *
     * This function checks if the input string starts with the specified prefix.
     *
     * @param str The input string to check.
     * @param prefix The prefix to check for.
     * @return True if the string starts with the prefix, false otherwise.
     */
    bool startsWith(const std::string& str, const std::string& prefix);
    
    
    // Helper function to check if a string is a valid integer
    bool isValidNumber(const std::string& str);
    
    // For properly handling placeholder replacements
    std::string returnOrNull(const std::string& value);

    
    // Function to slice a string from start to end index
    std::string sliceString(const std::string& str, size_t start, size_t end);
    
    
    /**
     * @brief Converts a string to lowercase.
     *
     * This function takes a string as input and returns a lowercase version of that string.
     *
     * @param str The input string to convert to lowercase.
     * @return The lowercase version of the input string.
     */
    
    std::string stringToLowercase(const std::string& str);
    

    /**
     * @brief Converts a string to uppercase.
     *
     * This function takes a string as input and returns an uppercase version of that string.
     *
     * @param str The input string to convert to uppercase.
     * @return The uppercase version of the input string.
     */
    
    std::string stringToUppercase(const std::string& str);
    
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
    std::string formatPriorityString(const std::string& priority, int desiredWidth = 4);
    
    
    
    /**
     * @brief Removes the part of the string after the first occurrence of '?' character.
     *
     * This function takes a string and removes the portion of the string that appears after
     * the first '?' character, if found. If no '?' character is present, the string remains unchanged.
     *
     * @param input The input string from which to remove the tag, passed by reference and modified in-place.
     */
    void removeTag(std::string &input);
    
    std::string getFirstLongEntry(const std::string& input, size_t minLength = 8);
    
    
    // This will take a string like "v1.3.5-abasdfasdfa" and output "1.3.5". string could also look like "test-1.3.5-1" or "v1.3.5" and we will only want "1.3.5"
    std::string cleanVersionLabel(const std::string& input);
    
    
    std::string extractTitle(const std::string& input);
    
    
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter);
    
    
    // Function to split a string by a delimiter and return a specific index
    std::string splitStringAtIndex(const std::string& str, const std::string& delimiter, size_t index);
    
    
    std::string customAlign(int number);
    
    #if IS_LAUNCHER_DIRECTIVE
    std::string inputExists(const std::string& input);
    #endif
}

#endif