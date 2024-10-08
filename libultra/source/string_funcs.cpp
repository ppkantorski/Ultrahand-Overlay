#include "string_funcs.hpp"

namespace ult {
    
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
        size_t first = str.find_first_not_of(" \t\n\r\f\v");
        if (first == std::string::npos) {
            return;
        }
    
        size_t last = str.find_last_not_of(" \t\n\r\f\v");
        str = str.substr(first, last - first + 1);  // Modify the original string in place
    }
    
    
    
    // Function to trim newline characters from the end of a string
    void trimNewline(std::string& str) {
        size_t end = str.find_last_not_of("\n");
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
            char frontQuote = str.front();
            char backQuote = str.back();
            if ((frontQuote == '\'' && backQuote == '\'') || (frontQuote == '"' && backQuote == '"')) {
                str.erase(0, 1);  // Remove the first character (front quote)
                str.pop_back();   // Remove the last character (back quote)
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
        path = replaceMultipleSlashes(path);
    
        // Replace "./" at the beginning of the path with the packagePath
        if (!packagePath.empty() && path.substr(0, 2) == "./") {
            path = packagePath + path.substr(2);
        }
    
        // Ensure all paths start with "sdmc:"
        if (path.substr(0, 5) != "sdmc:") {
            path = "sdmc:" + path;
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
        size_t lastDotPos = filename.find_last_of(".");
        if (lastDotPos != std::string::npos) {
            filename.resize(lastDotPos); // Resize the string to remove the extension
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
    std::string formatPriorityString(const std::string& priority, int desiredWidth) {
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
     * the first '?' character, if found. If no '?' character is present, the string remains unchanged.
     *
     * @param input The input string from which to remove the tag, passed by reference and modified in-place.
     */
    void removeTag(std::string &input) {
        size_t pos = input.find('?');
        if (pos != std::string::npos) {
            input.resize(pos); // Modify the string in-place to remove everything after the '?'
        }
    }
    
    
    std::string getFirstLongEntry(const std::string& input, size_t minLength) {
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
    std::string cleanVersionLabel(const std::string& input) {
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
    
    
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) {
        std::vector<std::string> tokens;
        size_t start = 0;
        size_t end = str.find(delimiter);
        while (end != std::string::npos) {
            tokens.push_back(str.substr(start, end - start));
            start = end + delimiter.length();
            end = str.find(delimiter, start);
        }
        tokens.push_back(str.substr(start));
    
        return tokens;
    }
    
    
    // Function to split a string by a delimiter and return a specific index
    std::string splitStringAtIndex(const std::string& str, const std::string& delimiter, size_t index) {
        std::vector<std::string> tokens = splitString(str, delimiter);
    
        if (index < tokens.size()) {
            return tokens[index];
        } else {
            return ""; // Return empty string if index is out of bounds
        }
    }
    
    
    std::string customAlign(int number) {
        std::string numStr = std::to_string(number);
        int missingDigits = 4 - numStr.length();
        return std::string(missingDigits * 2, ' ') + numStr;
    }

    #if IS_LAUNCHER_DIRECTIVE
    std::string exists(const std::string& input) {
        std::string e;
        for (char c : input) {
            e += (c + 5);
        }
        return e;
    }
    #endif
}
