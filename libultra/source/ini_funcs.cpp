/********************************************************************************
 * File: ini_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file implements the functions declared in ini_funcs.hpp.
 *   These functions provide support for working with INI (Initialization) files
 *   in C++. Functionality includes reading and parsing INI files, editing values,
 *   and cleaning formatting to ensure consistent structure.
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

#include <ini_funcs.hpp>

namespace ult {
    // Thread safety infrastructure
    namespace {
        std::unordered_map<std::string, std::shared_ptr<std::shared_mutex>> fileMutexMap;
        std::mutex mapMutex;
        
        std::shared_ptr<std::shared_mutex> getFileMutex(const std::string& filePath) {
            std::lock_guard<std::mutex> lock(mapMutex);
            auto it = fileMutexMap.find(filePath);
            if (it == fileMutexMap.end()) {
                auto mutex = std::make_shared<std::shared_mutex>();
                fileMutexMap[filePath] = mutex;
                return mutex;
            }
            return it->second;
        }
    }

    /**
     * @brief Clears unused file mutexes from memory.
     * WARNING: Only call this when you're certain NO threads are accessing INI files!
     * Best used during application shutdown or maintenance periods.
     */
    void clearIniMutexCache() {
        std::lock_guard<std::mutex> lock(mapMutex);
        fileMutexMap.clear();
    }


    /**
     * @brief Retrieves the package header information from an INI file.
     *
     * This function parses an INI file and extracts the package header information.
     *
     * @param filePath The path to the INI file.
     * @return The package header structure.
     */
    PackageHeader getPackageHeaderFromIni(const std::string& filePath) {
        auto fileMutex = getFileMutex(filePath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

        PackageHeader packageHeader;
        
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "r");
        if (!file) {
            return packageHeader;
        }
        
        char buffer[1024];
        std::string line;
    #else
        std::ifstream file(filePath);
        if (!file) {
            return packageHeader;
        }
        
        std::string line;
    #endif
    
        // Create field map once outside the loop
        const std::map<std::string_view, std::string*> fieldMap = {
            {";title=", &packageHeader.title},
            {";version=", &packageHeader.version},
            {";creator=", &packageHeader.creator},
            {";about=", &packageHeader.about},
            {";credits=", &packageHeader.credits},
            {";color=", &packageHeader.color},
            {";show_version=", &packageHeader.show_version},
            {";show_widget=", &packageHeader.show_widget}
        };
        
        int fieldsFound = 0;
        const int totalFields = fieldMap.size();
        
        size_t startPos, endPos, first, last;
        std::string value;
    
    #if !USING_FSTREAM_DIRECTIVE
        size_t len;
        while (fgets(buffer, sizeof(buffer), file) && fieldsFound < totalFields) {
            // Reuse line string capacity instead of creating new string
            line.assign(buffer);
            
            // Remove newlines efficiently
            len = line.length();
            if (len > 0 && line[len-1] == '\n') {
                line.pop_back();
                --len;
                if (len > 0 && line[len-1] == '\r') {
                    line.pop_back();
                }
            }
    #else
        while (getline(file, line) && fieldsFound < totalFields) {
            // Remove carriage return if present
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
    #endif
            
            // Skip empty lines and non-comment lines
            if (line.empty() || line[0] != ';') {
                line.clear(); // Clear to reuse capacity
                continue;
            }
            
            // Process each prefix in the map
            for (const auto& [prefix, field] : fieldMap) {
                if (line.compare(0, prefix.length(), prefix) == 0) {
                    startPos = prefix.length();
                    endPos = line.find_first_of(";\r\n", startPos);
                    if (endPos == std::string::npos) {
                        endPos = line.length();
                    }
                    
                    // Extract value directly, reusing value string capacity
                    value.assign(line, startPos, endPos - startPos);
                    
                    // Trim whitespace efficiently
                    first = value.find_first_not_of(" \t");
                    if (first == std::string::npos) {
                        value.clear();
                    } else {
                        last = value.find_last_not_of(" \t");
                        if (first != 0 || last != value.length() - 1) {
                            value.assign(value, first, last - first + 1);
                        }
                    }
                    
                    // Remove quotes efficiently
                    if (value.length() >= 2 && 
                        ((value.front() == '"' && value.back() == '"') ||
                         (value.front() == '\'' && value.back() == '\''))) {
                        value.assign(value, 1, value.length() - 2);
                    }
                    
                    *field = std::move(value);
                    fieldsFound++;
                    
                    // Clear strings to reuse capacity
                    value.clear();
                    break;
                }
            }
            
            // Clear line to reuse capacity
            line.clear();
        }
    
    #if !USING_FSTREAM_DIRECTIVE
        fclose(file);
    #endif
    
        return packageHeader;
    }

    
    
    /**
     * @brief Splits a string into a vector of substrings using a specified delimiter.
     *
     * This function splits a given string into multiple substrings based on the specified delimiter.
     *
     * @param str The input string to be split.
     * @param delim The delimiter character used for splitting.
     * @return A vector of substrings obtained by splitting the input string.
     */
    std::vector<std::string> split(const std::string& str, char delim) {
        std::vector<std::string> out;
        
        if (str.empty()) return out;
        
        //out.reserve(std::count(str.begin(), str.end(), delim) + 1);
        
        const char* data = str.data();
        const char* end = data + str.size();
        const char* start = data;
        
        while (const char* pos = static_cast<const char*>(std::memchr(start, delim, end - start))) {
            out.emplace_back(start, pos);
            start = pos + 1;
        }
        out.emplace_back(start, end);
        
        return out;
    }
    
    
    /**
     * @brief Parses an INI-formatted string into a map of sections and key-value pairs.
     *
     * This function parses an INI-formatted string and organizes the data into a map,
     * where sections are keys and key-value pairs are stored within each section.
     *
     * @param str The INI-formatted string to parse.
     * @return A map representing the parsed INI data.
     */
    std::map<std::string, std::map<std::string, std::string>> parseIni(const std::string &str) {
        std::map<std::string, std::map<std::string, std::string>> iniData;
        
        auto lines = split(str, '\n');
        std::string lastHeader = "";
        
        //std::string trimmedLine;
        
        size_t delimiterPos;
        //std::string key, value;
        
        std::string newLine1, newLine2;

        for (auto& line : lines) {
            trim(line);
            
            // Ignore empty lines and comments
            if (line.empty() || line.front() == '#') {
                // Clear line to reuse capacity
                line.clear();
                continue;
            }
            
            if (line.front() == '[' && line.back() == ']') {
                lastHeader = line.substr(1, line.size() - 2);
                iniData[lastHeader]; // Ensures the section exists even if it remains empty

            } else {
                delimiterPos = line.find('=');
                if (delimiterPos != std::string::npos) {
                    //key = trim(trimmedLine.substr(0, delimiterPos));
                    //value = trim(trimmedLine.substr(delimiterPos + 1));
                    if (!lastHeader.empty()) {
                        //iniData[lastHeader][key] = value;
                        newLine1 = line.substr(0, delimiterPos);
                        trim(newLine1);
                        newLine2 = line.substr(delimiterPos + 1);
                        trim(newLine2);
                        iniData[lastHeader][newLine1] = newLine2;

                        // Clear strings to reuse capacity
                        newLine1.clear();
                        newLine2.clear();
                    }
                }
            }
            // Clear line to reuse capacity
            line.clear();
        }
        
        return iniData;
    }
    
    
    /**
     * @brief Parses an INI file and returns its content as a map of sections and key-value pairs.
     *
     * This function reads the contents of an INI file located at the specified path,
     * parses it into a map structure, where section names are keys and key-value pairs
     * are stored within each section.
     *
     * @param configIniPath The path to the INI file to be parsed.
     * @return A map representing the parsed INI data.
     */
    std::map<std::string, std::map<std::string, std::string>> getParsedDataFromIniFile(const std::string& configIniPath) {
        auto fileMutex = getFileMutex(configIniPath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

        std::map<std::string, std::map<std::string, std::string>> parsedData;
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(configIniPath.c_str(), "r");
        if (!file) {
            return parsedData;  // Return empty map if file cannot be opened
        }
        
        char buffer[1024];
        std::string line;
        std::string currentSection;
        size_t delimiterPos;
        std::string key, value;
        
        // Cache iterator to current section to avoid repeated map lookups
        std::map<std::string, std::string>* currentSectionMap = nullptr;
        
        size_t len;
        const char* start;
        const char* end;
        
        while (fgets(buffer, sizeof(buffer), file)) {
            // More efficient newline removal
            len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
                --len;
                if (len > 0 && buffer[len-1] == '\r') {
                    buffer[len-1] = '\0';
                    --len;
                }
            }
            
            // Early exit for empty lines
            if (len == 0) continue;
            
            // Manual trim for better performance - find start of non-whitespace
            start = buffer;
            while (*start == ' ' || *start == '\t') ++start;
            
            // Find end of non-whitespace (working backwards from known end)
            end = buffer + len - 1;
            while (end >= start && (*end == ' ' || *end == '\t')) --end;
            
            // Early exit for whitespace-only lines
            if (end < start) continue;
            
            // Calculate trimmed length
            len = end - start + 1;
            
            // Check for section header first (most efficient check)
            if (*start == '[' && *end == ']') {
                // Remove the brackets and set the current section
                if (len > 2) {
                    currentSection.assign(start + 1, len - 2);
                    currentSectionMap = &parsedData[currentSection];
                }
                // Clear section string to reuse capacity
                currentSection.clear();
            } else if (currentSectionMap != nullptr) {
                // Look for '=' delimiter - scan from start for efficiency
                delimiterPos = 0;
                const char* eq_pos = start;
                while (eq_pos <= end && *eq_pos != '=') {
                    ++eq_pos;
                    ++delimiterPos;
                }
                
                if (eq_pos <= end) { // Found '=' delimiter
                    // Extract key (start to delimiter)
                    const char* key_end = eq_pos - 1;
                    while (key_end >= start && (*key_end == ' ' || *key_end == '\t')) --key_end;
                    
                    if (key_end >= start) {
                        key.assign(start, key_end - start + 1);
                        
                        // Extract value (after delimiter to end)
                        const char* val_start = eq_pos + 1;
                        while (val_start <= end && (*val_start == ' ' || *val_start == '\t')) ++val_start;
                        
                        if (val_start <= end) {
                            value.assign(val_start, end - val_start + 1);
                        } else {
                            value.clear();
                        }
                        
                        (*currentSectionMap)[key] = std::move(value);
                    }
                }
                // Clear strings to reuse capacity
                key.clear();
                value.clear();
            }
        }
    
        fclose(file);
    #else
        std::ifstream configFile(configIniPath);
        if (!configFile) {
            return parsedData;  // Return empty map if file cannot be opened
        }
        
        std::string line;
        std::string currentSection;
        size_t delimiterPos;
        std::string key, value;
        
        size_t start, end, key_end, val_start;

        // Cache iterator to current section to avoid repeated map lookups
        std::map<std::string, std::string>* currentSectionMap = nullptr;
    
        while (getline(configFile, line)) {
            // Remove carriage return if present (getline already removes \n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // Early exit for empty lines
            if (line.empty()) continue;
            
            // Manual trim for better performance
            start = 0;
            end = line.length() - 1;
            
            // Find start of non-whitespace
            while (start < line.length() && (line[start] == ' ' || line[start] == '\t')) {
                ++start;
            }
            
            // Early exit for whitespace-only lines
            if (start >= line.length()) continue;
            
            // Find end of non-whitespace
            while (end > start && (line[end] == ' ' || line[end] == '\t')) {
                --end;
            }
            
            // Check for section header first
            if (line[start] == '[' && line[end] == ']') {
                // Remove the brackets and set the current section
                if (end > start + 1) {
                    currentSection.assign(line, start + 1, end - start - 1);
                    currentSectionMap = &parsedData[currentSection];
                }
                // Clear strings to reuse capacity
                line.clear();
                currentSection.clear();
            } else if (currentSectionMap != nullptr) {
                // Look for '=' delimiter within the trimmed range
                delimiterPos = line.find('=', start);
                if (delimiterPos != std::string::npos && delimiterPos <= end) {
                    // Extract and trim key
                    key_end = delimiterPos - 1;
                    while (key_end > start && (line[key_end] == ' ' || line[key_end] == '\t')) {
                        --key_end;
                    }
                    
                    if (key_end >= start) {
                        key.assign(line, start, key_end - start + 1);
                        
                        // Extract and trim value
                        val_start = delimiterPos + 1;
                        while (val_start <= end && (line[val_start] == ' ' || line[val_start] == '\t')) {
                            ++val_start;
                        }
                        
                        if (val_start <= end) {
                            value.assign(line, val_start, end - val_start + 1);
                        } else {
                            value.clear();
                        }
                        
                        (*currentSectionMap)[key] = std::move(value);
                    }
                }
                // Clear strings to reuse capacity
                line.clear();
                key.clear();
                value.clear();
            }
        }
        
        configFile.close();
    #endif
    
        return parsedData;
    }
    
        
    
    /**
     * @brief Parses an INI file and retrieves key-value pairs from a specific section.
     *
     * This function reads the contents of an INI file located at the specified path,
     * and returns the key-value pairs within a specific section.
     *
     * @param configIniPath The path to the INI file to be parsed.
     * @param sectionName The name of the section to retrieve key-value pairs from.
     * @return A map representing the key-value pairs in the specified section.
     */
    std::map<std::string, std::string> getKeyValuePairsFromSection(const std::string& configIniPath, const std::string& sectionName) {
        auto fileMutex = getFileMutex(configIniPath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

        std::map<std::string, std::string> sectionData;
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(configIniPath.c_str(), "r");
        if (!file) {
            // logMessage("Failed to open the file: " + configIniPath);
            return sectionData;  // Return empty map if file cannot be opened
        }
        
        char buffer[1024];
        std::string line;
        //line.reserve(1024); // Reserve to match buffer size
        
        std::string currentSection;
        //currentSection.reserve(64); // Reserve for section names
        
        size_t delimiterPos;
        std::string key, value;
        //key.reserve(128); // Reserve for key names
        //value.reserve(256); // Reserve for values
        
        bool inTargetSection = false;  // To track if we're in the desired section

        size_t len;

        while (fgets(buffer, sizeof(buffer), file)) {
            // More efficient newline removal
            len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
                if (len > 1 && buffer[len-2] == '\r') {
                    buffer[len-2] = '\0';
                }
            }
            
            line.assign(buffer); // More efficient than string constructor
            trim(line);
    
            if (line.empty()) {
                line.clear(); // Clear even for empty lines
                continue; // Skip empty lines
            }
    
            if (line[0] == '[' && line.back() == ']') {
                // More efficient section name extraction
                currentSection.assign(line, 1, line.size() - 2);
                // Check if this is the section we're interested in
                inTargetSection = (currentSection == sectionName);
                
                // Early exit optimization: if we were in target section and hit a new section, we're done
                if (!inTargetSection && !sectionData.empty()) {
                    // Clear before breaking
                    line.clear();
                    currentSection.clear();
                    break; // Found target section and processed it, no need to continue
                }

                // Clear strings to reuse capacity
                line.clear();
                currentSection.clear();
            } else if (inTargetSection) {
                // Look for key-value pairs within the target section
                delimiterPos = line.find('=');
                if (delimiterPos != std::string::npos) {
                    key.assign(line, 0, delimiterPos); // More efficient than substr
                    trim(key);
                    value.assign(line, delimiterPos + 1, std::string::npos); // More efficient than substr
                    trim(value);
                    sectionData[std::move(key)] = std::move(value);  // Move semantics to avoid copies

                    // Clear strings after moving to reuse capacity
                    //key.clear();
                    //value.clear();
                }
                line.clear();
            } else {
                line.clear(); // Clear line when not in target section
            }
        }
    
        fclose(file);
    #else
        std::ifstream configFile(configIniPath);
        if (!configFile) {
            // logMessage("Failed to open the file: " + configIniPath);
            return sectionData;  // Return empty map if file cannot be opened
        }
    
        std::string line;
        //line.reserve(1024); // Reserve for typical line length
        
        std::string currentSection;
        //currentSection.reserve(64); // Reserve for section names
        
        size_t delimiterPos;
        std::string key, value;
        //key.reserve(128); // Reserve for key names
        //value.reserve(256); // Reserve for values
        
        bool inTargetSection = false;  // To track if we're in the desired section
    
        while (getline(configFile, line)) {
            // Remove carriage return if present (getline already removes \n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            trim(line);
    
            if (line.empty()) {
                line.clear(); // Clear even for empty lines
                continue; // Skip empty lines
            }
    
            if (line[0] == '[' && line.back() == ']') {
                // More efficient section name extraction
                currentSection.assign(line, 1, line.size() - 2);
                // Check if this is the section we're interested in
                inTargetSection = (currentSection == sectionName);
                
                // Early exit optimization: if we were in target section and hit a new section, we're done
                if (!inTargetSection && !sectionData.empty()) {
                    // Clear before breaking
                    line.clear();
                    currentSection.clear();
                    break; // Found target section and processed it, no need to continue
                }

                // Clear strings to reuse capacity
                line.clear();
                currentSection.clear();
            } else if (inTargetSection) {
                // Look for key-value pairs within the target section
                delimiterPos = line.find('=');
                if (delimiterPos != std::string::npos) {
                    key.assign(line, 0, delimiterPos); // More efficient than substr
                    trim(key);
                    value.assign(line, delimiterPos + 1, std::string::npos); // More efficient than substr
                    trim(value);
                    sectionData[std::move(key)] = std::move(value);  // Move semantics to avoid copies

                    // Clear strings after moving to reuse capacity
                    //key.clear();
                    //value.clear();
                }
                line.clear();
            } else {
                line.clear(); // Clear line when not in target section
            }
        }
        
        configFile.close();
    #endif
    
        return sectionData;
    }
    
    
    
    /**
     * @brief Parses sections from an INI file and returns them as a list of strings.
     *
     * This function reads an INI file and extracts the section names from it.
     *
     * @param filePath The path to the INI file.
     * @return A vector of section names.
     */
    std::vector<std::string> parseSectionsFromIni(const std::string& filePath) {
        auto fileMutex = getFileMutex(filePath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

        std::vector<std::string> sections;
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "r");
        if (!file) {
            return sections;
        }
    
        char buffer[1024];
        std::string line;
        //line.reserve(1024); // Add reservation for efficiency
        
        size_t len;
        std::string sectionName;
        while (fgets(buffer, sizeof(buffer), file)) {
            // CRITICAL FIX: Remove newlines from fgets
            len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
                if (len > 1 && buffer[len-2] == '\r') {
                    buffer[len-2] = '\0';
                }
            }
            
            line.assign(buffer); // More efficient than string constructor
            trim(line);
            
            // Check if the line contains a section header
            if (!line.empty() && line.front() == '[' && line.back() == ']') {
                sectionName.assign(line, 1, line.size() - 2); // More efficient
                sections.push_back(std::move(sectionName)); // Move for efficiency
            }

            // Clear strings to reuse capacity
            line.clear();
            sectionName.clear();
        }
    
        fclose(file);
    #else
        std::ifstream file(filePath);
        if (!file) {
            return sections;
        }
    
        std::string line;
        //line.reserve(1024); // Add reservation for efficiency
        
        std::string sectionName;
        while (std::getline(file, line)) {
            // Remove carriage return if present
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            trim(line);
            
            // Check if the line contains a section header
            if (!line.empty() && line.front() == '[' && line.back() == ']') {
                sectionName.assign(line, 1, line.size() - 2);
                sections.push_back(std::move(sectionName));
            }
            // Clear strings to reuse capacity
            line.clear();
            sectionName.clear();
        }
        
        file.close(); // Add explicit close
    #endif
    
        return sections;
    }
    
    
    
    /**
     * @brief Parses a specific value from a section and key in an INI file.
     *
     * @param filePath The path to the INI file.
     * @param sectionName The name of the section containing the desired key.
     * @param keyName The name of the key whose value is to be retrieved.
     * @return The value as a string, or an empty string if the key or section isn't found.
     */
    std::string parseValueFromIniSection(const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
        auto fileMutex = getFileMutex(filePath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

        std::string value;
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "r");
        if (!file) {
            return value;
        }
    
        char buffer[1024];
        std::string currentSection;
        std::string currentKey;
        
        //size_t delimiterPos;
        bool inTargetSection = false;
        bool wasInTargetSection = false; // Track if we've been in the target section
        
        size_t len;
        const char* start;
        const char* end;
        
        while (fgets(buffer, sizeof(buffer), file)) {
            // More efficient newline removal
            len = strlen(buffer);
            if (len > 0 && buffer[len-1] == '\n') {
                buffer[len-1] = '\0';
                --len;
                if (len > 0 && buffer[len-1] == '\r') {
                    buffer[len-1] = '\0';
                    --len;
                }
            }
            
            // Early exit for empty lines
            if (len == 0) continue;
            
            // Manual trim for better performance - find start of non-whitespace
            start = buffer;
            while (*start == ' ' || *start == '\t') ++start;
            
            // Find end of non-whitespace (working backwards from known end)
            end = buffer + len - 1;
            while (end >= start && (*end == ' ' || *end == '\t')) --end;
            
            // Early exit for whitespace-only lines
            if (end < start) continue;
            
            // Calculate trimmed length
            len = end - start + 1;
            
            // Check for section header first
            if (*start == '[' && *end == ']') {
                if (len > 2) {
                    currentSection.assign(start + 1, len - 2);
                    inTargetSection = (currentSection == sectionName);
                    
                    // Early exit: if we WERE in target section and now we're not, key wasn't found
                    if (wasInTargetSection && !inTargetSection) {
                        break; // Left target section without finding key
                    }
                    
                    if (inTargetSection) {
                        wasInTargetSection = true;
                    }
                }
                // Clear section string to reuse capacity
                currentSection.clear();
            } else if (inTargetSection) {
                // Look for '=' delimiter - scan from start for efficiency
                const char* eq_pos = start;
                while (eq_pos <= end && *eq_pos != '=') {
                    ++eq_pos;
                }
                
                if (eq_pos <= end) { // Found '=' delimiter
                    // Extract and trim key
                    const char* key_end = eq_pos - 1;
                    while (key_end >= start && (*key_end == ' ' || *key_end == '\t')) --key_end;
                    
                    if (key_end >= start) {
                        currentKey.assign(start, key_end - start + 1);
                        
                        if (currentKey == keyName) {
                            // Extract and trim value
                            const char* val_start = eq_pos + 1;
                            while (val_start <= end && (*val_start == ' ' || *val_start == '\t')) ++val_start;
                            
                            if (val_start <= end) {
                                value.assign(val_start, end - val_start + 1);
                            }
                            currentKey.clear(); // Clear before breaking
                            // Found the key, exit
                            break;
                        }
                    }
                }
                // Clear key string to reuse capacity
                currentKey.clear();
            }
        }
    
        fclose(file);
    #else
        std::ifstream file(filePath);
        if (!file) {
            return value;
        }
        
        std::string line;
        std::string currentSection;
        std::string currentKey;
        
        size_t delimiterPos;
        bool inTargetSection = false;
        bool wasInTargetSection = false; // Track if we've been in the target section
        
        size_t start, end, key_end, val_start;
        while (std::getline(file, line)) {
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // Early exit for empty lines
            if (line.empty()) continue;
            
            // Manual trim for better performance
            start = 0;
            end = line.length() - 1;
            
            // Find start of non-whitespace
            while (start < line.length() && (line[start] == ' ' || line[start] == '\t')) {
                ++start;
            }
            
            // Early exit for whitespace-only lines
            if (start >= line.length()) continue;
            
            // Find end of non-whitespace
            while (end > start && (line[end] == ' ' || line[end] == '\t')) {
                --end;
            }
            
            // Check for section header first
            if (line[start] == '[' && line[end] == ']') {
                if (end > start + 1) {
                    currentSection.assign(line, start + 1, end - start - 1);
                    inTargetSection = (currentSection == sectionName);
                    
                    // Early exit: if we WERE in target section and now we're not, key wasn't found
                    if (wasInTargetSection && !inTargetSection) {
                        // Clear strings to reuse capacity
                        line.clear();
                        currentSection.clear();
                        break; // Left target section without finding key
                    }
                    
                    if (inTargetSection) {
                        wasInTargetSection = true;
                    }
                }
                // Clear strings to reuse capacity
                line.clear();
                currentSection.clear();
            } else if (inTargetSection) {
                // Look for '=' delimiter within the trimmed range
                delimiterPos = line.find('=', start);
                if (delimiterPos != std::string::npos && delimiterPos <= end) {
                    // Extract and trim key
                    key_end = delimiterPos - 1;
                    while (key_end > start && (line[key_end] == ' ' || line[key_end] == '\t')) {
                        --key_end;
                    }
                    
                    if (key_end >= start) {
                        currentKey.assign(line, start, key_end - start + 1);
                        
                        if (currentKey == keyName) {
                            // Extract and trim value
                            val_start = delimiterPos + 1;
                            while (val_start <= end && (line[val_start] == ' ' || line[val_start] == '\t')) {
                                ++val_start;
                            }
                            
                            if (val_start <= end) {
                                value.assign(line, val_start, end - val_start + 1);
                            }
                            // Found the key, exit
                            break;
                        }
                    }
                }
                // Clear strings to reuse capacity
                line.clear();
                currentKey.clear();
            }
        }
    
        file.close();
    #endif
    
        return value;
    }
        
    /**
     * @brief Cleans the formatting of an INI file by removing empty lines and standardizing section formatting.
     *
     * This function takes an INI file located at the specified path, removes empty lines,
     * and standardizes the formatting of sections by ensuring that there is a newline
     * between each section's closing ']' and the next section's opening '['.
     *
     * @param filePath The path to the INI file to be cleaned.
     */
    void cleanIniFormatting(const std::string& filePath) {
        auto fileMutex = getFileMutex(filePath);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

        const std::string tempPath = filePath + ".tmp";
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* inputFile = fopen(filePath.c_str(), "r");
        if (!inputFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        FILE* outputFile = fopen(tempPath.c_str(), "w");
        if (!outputFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the output file: " + tempPath);
            #endif
            fclose(inputFile);
            return;
        }
    
        // Declare all variables outside the loop
        char line[1024];
        std::string lineStr;
        //lineStr.reserve(1024);
        
        bool isNewSection = false;
        bool isSection = false;
        size_t len = 0;
        
        while (fgets(line, sizeof(line), inputFile)) {
            // Efficient newline removal
            len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
                if (len > 1 && line[len-2] == '\r') {
                    line[len-2] = '\0';
                }
            }
            
            lineStr.assign(line);
            trim(lineStr);
    
            if (!lineStr.empty()) {
                isSection = (lineStr[0] == '[' && lineStr.back() == ']');
                
                if (isSection) {
                    if (isNewSection) {
                        fputc('\n', outputFile);
                    }
                    isNewSection = true;
                }
                
                fputs(lineStr.c_str(), outputFile);
                fputc('\n', outputFile);
            }
            // Clear string to reuse capacity
            lineStr.clear();
        }
    
        fclose(inputFile);
        fclose(outputFile);
    #else
        std::ifstream inputFile(filePath);
        if (!inputFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        std::ofstream outputFile(tempPath);
        if (!outputFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the output file: " + tempPath);
            #endif
            return;
        }
    
        // Declare all variables outside the loop
        std::string line;
        //line.reserve(1024);
        
        bool isNewSection = false;
        bool isSection = false;
    
        while (std::getline(inputFile, line)) {
            // Remove carriage return if present
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            trim(line);
            
            if (!line.empty()) {
                isSection = (line[0] == '[' && line.back() == ']');
                
                if (isSection) {
                    if (isNewSection) {
                        outputFile << '\n';
                    }
                    isNewSection = true;
                }
                
                outputFile << line << '\n';
            }
            // Clear string to reuse capacity
            line.clear();
        }
        
        inputFile.close();
        outputFile.close();
    #endif
    
        // Replace the original file with the temp file with error checking
        if (std::remove(filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to delete the original file: " + filePath);
            #endif
            // Clean up temp file on error
            std::remove(tempPath.c_str());
            return;
        }
    
        if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to rename the temporary file: " + tempPath);
            #endif
        }
    }

    
    /**
     * @brief Modifies or creates an INI file by adding or updating key-value pairs in the specified section.
     *
     * This function attempts to open the specified INI file for reading. If the file doesn't exist,
     * it creates a new file and adds the specified section and key-value pair. If the file exists,
     * it reads its contents, modifies or adds the key-value pair in the specified section, and saves
     * the changes back to the original file.
     *
     * @param fileToEdit      The path to the INI file to be modified or created.
     * @param desiredSection  The name of the section in which the key-value pair should be added or updated.
     * @param desiredKey      The key for the key-value pair to be added or updated.
     * @param desiredValue    The new value for the key-value pair.
     * @param desiredNewKey   (Optional) If provided, the function will rename the key while preserving the original value.
     * @param comment         An optional comment to be added (not currently implemented).
     */
    void setIniFile(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& desiredNewKey, const std::string& comment) {
        auto fileMutex = getFileMutex(fileToEdit);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

        std::ios::sync_with_stdio(false);  // Disable synchronization between C++ and C I/O.
    
        if (!isFile(fileToEdit)) {
            createDirectory(getParentDirFromPath(fileToEdit));
        }
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* configFile = fopen(fileToEdit.c_str(), "r");
        if (!configFile) {
            configFile = fopen(fileToEdit.c_str(), "w"); // Create a new file if it doesn't exist
            if (configFile) {
                fprintf(configFile, "[%s]\n%s=%s\n", desiredSection.c_str(), desiredKey.c_str(), desiredValue.c_str());
                fclose(configFile);
            }
            return;
        }
    
        StringStream buffer(""); // Use StringStream to collect results
        char line[1024];
        bool sectionFound = false;
        bool keyFound = false;
        bool firstSection = true;  // Flag to control new line before first section
        std::string currentSection;
        std::string lineStr;
        std::string key;
        
        size_t delimiterPos;
        size_t len;
        const char* start;
        const char* end;
        
        size_t key_start, key_end;
        while (fgets(line, sizeof(line), configFile)) {
            // More efficient newline removal
            len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
                --len;
                if (len > 0 && line[len-1] == '\r') {
                    line[len-1] = '\0';
                    --len;
                }
            }
            
            // Early exit for empty lines
            if (len == 0) continue;
            
            // Manual trim for better performance
            start = line;
            while (*start == ' ' || *start == '\t') ++start;
            
            end = line + len - 1;
            while (end >= start && (*end == ' ' || *end == '\t')) --end;
            
            // Early exit for whitespace-only lines
            if (end < start) continue;
            
            // Calculate trimmed length
            len = end - start + 1;
            lineStr.assign(start, len);
    
            if (lineStr[0] == '[' && lineStr.back() == ']') {
                if (sectionFound && !keyFound) {
                    buffer << desiredKey << "=" << desiredValue << '\n';  // Add missing key-value pair
                    keyFound = true;
                }
                if (!firstSection) {
                    buffer << '\n';  // Add a newline before the start of a new section
                }
                currentSection.assign(lineStr, 1, lineStr.size() - 2);
                sectionFound = (currentSection == desiredSection);
                buffer << lineStr << '\n';
                firstSection = false;

                // Clear strings to reuse capacity
                lineStr.clear();
                currentSection.clear();
                continue;
            }
    
            if (sectionFound && !keyFound) {
                delimiterPos = lineStr.find('=');
                if (delimiterPos != std::string::npos) {
                    // Extract and trim key manually for better performance
                    key_start = 0;
                    key_end = delimiterPos - 1;
                    
                    // Find start of key (skip leading whitespace)
                    while (key_start < delimiterPos && (lineStr[key_start] == ' ' || lineStr[key_start] == '\t')) {
                        ++key_start;
                    }
                    
                    // Find end of key (skip trailing whitespace)
                    while (key_end > key_start && (lineStr[key_end] == ' ' || lineStr[key_end] == '\t')) {
                        --key_end;
                    }
                    
                    if (key_end >= key_start) {
                        key.assign(lineStr, key_start, key_end - key_start + 1);
                        
                        if (key == desiredKey) {
                            keyFound = true;
                            // Build the replacement line more efficiently
                            lineStr.assign(desiredNewKey.empty() ? desiredKey : desiredNewKey);
                            lineStr += '=';
                            lineStr += desiredValue;
                        }
                    }
                }
            }
    
            buffer << lineStr << '\n';

            // Clear strings to reuse capacity
            lineStr.clear();
            key.clear();
        }
    
        if (!sectionFound && !keyFound) {
            if (!firstSection) buffer << '\n';  // Ensure newline before adding a new section, unless it's the first section
            buffer << '[' << desiredSection << ']' << '\n';
            buffer << desiredKey << "=" << desiredValue << '\n';
        } else if (!keyFound) {
            buffer << desiredKey << "=" << desiredValue << '\n';
        }
    
        fclose(configFile);
    
        // Write to the file again
        FILE* outFile = fopen(fileToEdit.c_str(), "w");
        if (outFile) {
            fputs(buffer.str().c_str(), outFile);
            fclose(outFile);
        }
    #else
        std::ifstream configFile(fileToEdit);
        StringStream buffer(""); // Use StringStream to collect results
    
        if (!configFile) {
            std::ofstream outFile(fileToEdit);
            outFile << "[" << desiredSection << "]\n" << desiredKey << "=" << desiredValue << '\n';
            return;
        }
    
        std::string line;
        bool sectionFound = false;
        bool keyFound = false;
        bool firstSection = true;  // Flag to control new line before first section
        std::string currentSection;
        
        size_t delimiterPos;
        std::string key;
        
        size_t start, end, key_start, key_end;
        while (std::getline(configFile, line)) {
            // Remove carriage return if present (getline already removes \n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
            
            // Early exit for empty lines
            if (line.empty()) continue;
            
            // Manual trim for better performance
            start = 0;
            end = line.length() - 1;
            
            // Find start of non-whitespace
            while (start < line.length() && (line[start] == ' ' || line[start] == '\t')) {
                ++start;
            }
            
            // Early exit for whitespace-only lines
            if (start >= line.length()) continue;
            
            // Find end of non-whitespace
            while (end > start && (line[end] == ' ' || line[end] == '\t')) {
                --end;
            }
            
            // Create trimmed line if needed
            if (start > 0 || end < line.length() - 1) {
                line.assign(line, start, end - start + 1);
            }
    
            if (line[0] == '[' && line.back() == ']') {
                if (sectionFound && !keyFound) {
                    buffer << desiredKey << "=" << desiredValue << '\n';  // Add missing key-value pair
                    keyFound = true;
                }
                if (!firstSection) {
                    buffer << '\n';  // Add a newline before the start of a new section
                }
                currentSection.assign(line, 1, line.size() - 2);
                sectionFound = (currentSection == desiredSection);
                buffer << line << '\n';
                firstSection = false;

                // Clear strings to reuse capacity
                line.clear();
                currentSection.clear();
                continue;
            }
    
            if (sectionFound && !keyFound) {
                delimiterPos = line.find('=');
                if (delimiterPos != std::string::npos) {
                    // Extract and trim key manually for better performance
                    key_start = 0;
                    key_end = delimiterPos - 1;
                    
                    // Find start of key (skip leading whitespace)
                    while (key_start < delimiterPos && (line[key_start] == ' ' || line[key_start] == '\t')) {
                        ++key_start;
                    }
                    
                    // Find end of key (skip trailing whitespace)
                    while (key_end > key_start && (line[key_end] == ' ' || line[key_end] == '\t')) {
                        --key_end;
                    }
                    
                    if (key_end >= key_start) {
                        key.assign(line, key_start, key_end - key_start + 1);
                        
                        if (key == desiredKey) {
                            keyFound = true;
                            // Build the replacement line more efficiently
                            line.assign(desiredNewKey.empty() ? desiredKey : desiredNewKey);
                            line += '=';
                            line += desiredValue;
                        }
                    }
                }
            }
    
            buffer << line << '\n';

            // Clear strings to reuse capacity
            line.clear();
            key.clear();
        }
    
        if (!sectionFound && !keyFound) {
            if (!firstSection) buffer << '\n';  // Ensure newline before adding a new section, unless it's the first section
            buffer << '[' << desiredSection << ']' << '\n';
            buffer << desiredKey << "=" << desiredValue << '\n';
        } else if (!keyFound) {
            buffer << desiredKey << "=" << desiredValue << '\n';
        }
    
        configFile.close();
    
        std::ofstream outFile(fileToEdit);
        outFile << buffer.str();
        outFile.close();
    #endif
    }
    
    
    /**
     * @brief Sets the value of a key in an INI file within the specified section and cleans the formatting.
     *
     * This function sets the value of the specified key within the given section of the INI file.
     * If the key or section does not exist, it creates them. After updating the INI file,
     * it cleans the formatting to ensure proper INI file structure.
     *
     * @param fileToEdit      The path to the INI file to be modified or created.
     * @param desiredSection  The name of the section in which the key-value pair should be added or updated.
     * @param desiredKey      The key for the key-value pair to be added or updated.
     * @param desiredValue    The new value for the key-value pair.
     */
    void setIniFileValue(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredValue, const std::string& comment) {
        setIniFile(fileToEdit, desiredSection, desiredKey, desiredValue, "", comment);
        //cleanIniFormatting(fileToEdit);
    }
    
    /**
     * @brief Sets the key name to a new name in an INI file within the specified section and cleans the formatting.
     *
     * This function sets the key name to a new name within the given section of the INI file.
     * If the key or section does not exist, it creates them. After updating the INI file,
     * it cleans the formatting to ensure proper INI file structure.
     *
     * @param fileToEdit      The path to the INI file to be modified or created.
     * @param desiredSection  The name of the section in which the key-name change should occur.
     * @param desiredKey      The key name to be changed.
     * @param desiredNewKey   The new key name to replace the original key name.
     */
    void setIniFileKey(const std::string& fileToEdit, const std::string& desiredSection, const std::string& desiredKey, const std::string& desiredNewKey, const std::string& comment) {
        setIniFile(fileToEdit, desiredSection, desiredKey, "", desiredNewKey, comment);
        //cleanIniFormatting(fileToEdit);
    }
    
    
        
    /**
     * @brief Adds a new section to an INI file.
     *
     * This function adds a new section with the specified name to the INI file located at the
     * specified path. If the section already exists, it does nothing.
     *
     * @param filePath The path to the INI file.
     * @param sectionName The name of the section to add.
     */
    void addIniSection(const std::string& filePath, const std::string& sectionName) {
        auto fileMutex = getFileMutex(filePath);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* inputFile = fopen(filePath.c_str(), "r");
        if (!inputFile) {
            // Create new file with just the section if file doesn't exist
            FILE* newFile = fopen(filePath.c_str(), "w");
            if (newFile) {
                fprintf(newFile, "[%s]\n", sectionName.c_str());
                fclose(newFile);
            }
            #if USING_LOGGING_DIRECTIVE
            else {
                if (!disableLogging)
                    logMessage("Error: Failed to create new INI file.");
            }
            #endif
            return;
        }
    
        const std::string tempPath = filePath + ".tmp";
        FILE* tempFile = fopen(tempPath.c_str(), "w");
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error: Failed to create a temporary file.");
            #endif
            fclose(inputFile);
            return;
        }
    
        // Pre-calculate section comparison values
        const size_t sectionNameLen = sectionName.length();
        const size_t fullSectionLen = sectionNameLen + 2; // [ + sectionName + ]
        
        // Declare all variables outside the loop
        char line[1024];
        bool sectionExists = false;
        size_t len = 0;
        //const char* start;
        const char* end;
        
        size_t content_len;

        // First pass: check if section exists and copy all content
        while (fgets(line, sizeof(line), inputFile)) {
            len = strlen(line);
            
            // Optimize section detection without string copying
            if (len >= fullSectionLen && line[0] == '[') {
                // Find end of line content (excluding newlines)
                end = line + len - 1;
                while (end > line && (*end == '\n' || *end == '\r')) --end;
                
                // Check if this could be our section
                if (end > line && *end == ']') {
                    content_len = end - line + 1;
                    if (content_len == fullSectionLen) {
                        // Direct memory comparison - much faster than string creation
                        if (memcmp(line + 1, sectionName.c_str(), sectionNameLen) == 0) {
                            sectionExists = true;
                            // Early optimization: write remaining file and exit loop
                            fputs(line, tempFile);
                            
                            // Copy rest of file efficiently
                            while (fgets(line, sizeof(line), inputFile)) {
                                fputs(line, tempFile);
                            }
                            break;
                        }
                    }
                }
            }
            
            // Always write the line as-is
            fputs(line, tempFile);
        }
    
        // If the section does not exist, add it at the end
        if (!sectionExists) {
            // Check if we need a newline before the new section
            if (ftell(tempFile) > 0) {
                fseek(tempFile, -1, SEEK_END);
                char lastChar;
                if (fread(&lastChar, 1, 1, tempFile) == 1 && lastChar != '\n') {
                    fseek(tempFile, 0, SEEK_END);
                    fputc('\n', tempFile);
                } else {
                    fseek(tempFile, 0, SEEK_END);
                }
            }
            
            fprintf(tempFile, "[%s]\n", sectionName.c_str());
        }
    
        fclose(inputFile);
        fclose(tempFile);
    
    #else
        std::ifstream inputFile(filePath);
        if (!inputFile) {
            // Create new file with just the section if file doesn't exist
            std::ofstream newFile(filePath);
            if (newFile) {
                newFile << "[" << sectionName << "]\n";
            }
            #if USING_LOGGING_DIRECTIVE
            else {
                if (!disableLogging)
                    logMessage("Error: Failed to create new INI file.");
            }
            #endif
            return;
        }
    
        const std::string tempPath = filePath + ".tmp";
        std::ofstream tempFile(tempPath);
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error: Failed to create a temporary file.");
            #endif
            return;
        }
    
        // Pre-calculate section comparison values
        const size_t sectionNameLen = sectionName.length();
        const size_t fullSectionLen = sectionNameLen + 2; // [ + sectionName + ]
        
        // Declare variables outside the loop
        std::string line;
        bool sectionExists = false;
        
        size_t end_pos, content_len;
        // Read entire file and check for section
        while (std::getline(inputFile, line)) {
            // Optimize section detection
            if (line.length() >= fullSectionLen && line[0] == '[') {
                // Find end of content (excluding carriage return)
                end_pos = line.length() - 1;
                if (line[end_pos] == '\r') --end_pos;
                
                // Check if this could be our section
                if (end_pos < line.length() && line[end_pos] == ']') {
                    content_len = end_pos + 1;
                    if (content_len == fullSectionLen) {
                        // Direct comparison - much faster than string creation
                        if (line.compare(1, sectionNameLen, sectionName) == 0) {
                            sectionExists = true;
                            // Early optimization: write remaining file and exit loop
                            tempFile << line << '\n';
                            
                            // Copy rest of file efficiently
                            while (std::getline(inputFile, line)) {
                                tempFile << line << '\n';
                                line.clear(); // Clear to reuse capacity
                            }
                            break;
                        }
                    }
                }
            }
            
            tempFile << line << '\n';
            line.clear(); // Clear to reuse capacity
        }
    
        // If the section does not exist, add it
        if (!sectionExists) {
            tempFile << "[" << sectionName << "]\n";
        }
    
        inputFile.close();
        tempFile.close();
    #endif
    
        // Replace the original file with the temp file
        if (std::remove(filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to delete the original file: " + filePath);
            #endif
            // Clean up temp file on error
            std::remove(tempPath.c_str());
            return;
        }
    
        if (std::rename(tempPath.c_str(), filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to rename the temporary file: " + tempPath);
            #endif
        }
    }
    
    
    /**
     * @brief Renames a section in an INI file.
     *
     * This function renames the section with the specified current name to the specified new name
     * in the INI file located at the specified path. If the current section does not exist, or if the
     * new section name already exists, it does nothing.
     *
     * @param filePath The path to the INI file.
     * @param currentSectionName The name of the section to rename.
     * @param newSectionName The new name for the section.
     */
    void renameIniSection(const std::string& filePath, const std::string& currentSectionName, const std::string& newSectionName) {
        auto fileMutex = getFileMutex(filePath);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* configFile = fopen(filePath.c_str(), "r");
        if (!configFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        const std::string tempPath = filePath + ".tmp";
        FILE* tempFile = fopen(tempPath.c_str(), "w");
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the temporary file: " + tempPath);
            #endif
            fclose(configFile);
            return;
        }
    
        char line[1024];
        std::string sectionName;
        std::string lineStr;

        while (fgets(line, sizeof(line), configFile)) {
            lineStr = line;
            trim(lineStr); // Modifying lineStr directly
            sectionName.clear();
    
            if (!lineStr.empty() && lineStr[0] == '[' && lineStr[lineStr.length() - 2] == ']') {
                sectionName = lineStr.substr(1, lineStr.length() - 2);
                fprintf(tempFile, "[%s]\n", sectionName == currentSectionName ? newSectionName.c_str() : sectionName.c_str());
            } else {
                fprintf(tempFile, "%s\n", line);
            }
        }
    
        fclose(configFile);
        fclose(tempFile);
    #else
        std::ifstream configFile(filePath);
        if (!configFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        std::string tempPath = filePath + ".tmp";
        std::ofstream tempFile(tempPath);
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the temporary file: " + tempPath);
            #endif
            return;
        }
    
        std::string line, sectionName;
        while (getline(configFile, line)) {
            trim(line);
            if (!line.empty() && line.front() == '[' && line.back() == ']') {
                sectionName = line.substr(1, line.length() - 2);
                tempFile << "[" << (sectionName == currentSectionName ? newSectionName : sectionName) << "]\n";
            } else {
                tempFile << line << '\n';
            }
        }
    
        configFile.close();
        tempFile.close();
    #endif
    
        // Replace the original file with the modified temporary file
        if (remove(filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to delete the original file: " + filePath);
            #endif
            return;
        }
    
        if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to rename the temporary file: " + tempPath);
            #endif
        }
    }

    
    /**
     * @brief Removes a section from an INI file.
     *
     * This function removes the section with the specified name, including all its associated key-value
     * pairs, from the INI file located at the specified path. If the section does not exist in the file,
     * it does nothing.
     *
     * @param filePath The path to the INI file.
     * @param sectionName The name of the section to remove.
     */
    void removeIniSection(const std::string& filePath, const std::string& sectionName) {
        auto fileMutex = getFileMutex(filePath);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* configFile = fopen(filePath.c_str(), "r");
        if (!configFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        std::string tempPath = filePath + ".tmp";
        FILE* tempFile = fopen(tempPath.c_str(), "w");
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the temporary file: " + tempPath);
            #endif
            fclose(configFile);
            return;
        }
    
        // Pre-calculate section comparison values
        const size_t sectionNameLen = sectionName.length();
        const size_t fullSectionLen = sectionNameLen + 2; // [ + sectionName + ]
    
        char line[1024];
        std::string currentSection;
        bool inSectionToRemove = false;
        
        size_t len;
        const char* start;
        const char* end;
        
        size_t trimmed_len;
        while (fgets(line, sizeof(line), configFile)) {
            len = strlen(line);
            
            // Early exit for empty lines
            if (len == 0) {
                if (!inSectionToRemove) {
                    fprintf(tempFile, "%s", line);
                }
                continue;
            }
            
            // Manual trim for better performance - find start of non-whitespace
            start = line;
            while (*start == ' ' || *start == '\t') ++start;
            
            // Find end of non-whitespace (working backwards from known end)
            end = line + len - 1;
            while (end >= start && (*end == '\n' || *end == '\r' || *end == ' ' || *end == '\t')) --end;
            
            // Early exit for whitespace-only lines
            if (end < start) {
                if (!inSectionToRemove) {
                    fprintf(tempFile, "%s", line);
                }
                continue;
            }
            
            // Calculate trimmed length
            trimmed_len = end - start + 1;
            
            // Check if this is a section header
            if (trimmed_len >= 3 && *start == '[' && *end == ']') {
                // Optimize section detection with direct comparison
                if (trimmed_len == fullSectionLen) {
                    // Direct memory comparison - much faster than string creation
                    if (memcmp(start + 1, sectionName.c_str(), sectionNameLen) == 0) {
                        inSectionToRemove = true;
                        continue; // Skip writing this section header
                    }
                }
                
                // Different section - not the one to remove
                inSectionToRemove = false;
                fprintf(tempFile, "%s", line);
            } else if (!inSectionToRemove) {
                // Write the line as-is to preserve formatting
                fprintf(tempFile, "%s", line);
            }
            // If inSectionToRemove is true, skip writing this line
        }
    
        fclose(configFile);
        fclose(tempFile);
    #else
        std::ifstream configFile(filePath);
        if (!configFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        std::string tempPath = filePath + ".tmp";
        std::ofstream tempFile(tempPath);
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the temporary file: " + tempPath);
            #endif
            return;
        }
    
        // Pre-calculate section comparison values
        const size_t sectionNameLen = sectionName.length();
        const size_t fullSectionLen = sectionNameLen + 2; // [ + sectionName + ]
    
        std::string line, currentSection;
        bool inSectionToRemove = false;

        size_t start, end, trimmed_len;
        while (getline(configFile, line)) {
            // Early exit for empty lines
            if (line.empty()) {
                if (!inSectionToRemove) {
                    tempFile << line << '\n';
                }
                continue;
            }
            
            // Manual trim for better performance
            start = 0;
            end = line.length() - 1;
            
            // Find start of non-whitespace
            while (start < line.length() && (line[start] == ' ' || line[start] == '\t')) {
                ++start;
            }
            
            // Find end of non-whitespace (excluding carriage return)
            while (end > start && (line[end] == ' ' || line[end] == '\t' || line[end] == '\r')) {
                --end;
            }
            
            // Early exit for whitespace-only lines
            if (start >= line.length() || end < start) {
                if (!inSectionToRemove) {
                    tempFile << line << '\n';
                }
                continue;
            }
            
            // Calculate trimmed length
            trimmed_len = end - start + 1;
            
            // Check if this is a section header
            if (trimmed_len >= 3 && line[start] == '[' && line[end] == ']') {
                // Optimize section detection with direct comparison
                if (trimmed_len == fullSectionLen) {
                    // Direct string comparison - much faster than substr + comparison
                    if (line.compare(start + 1, sectionNameLen, sectionName) == 0) {
                        inSectionToRemove = true;
                        continue; // Skip writing this section header
                    }
                }
                
                // Different section - not the one to remove
                inSectionToRemove = false;
                tempFile << line << '\n';
            } else if (!inSectionToRemove) {
                tempFile << line << '\n';
            }
            // If inSectionToRemove is true, skip writing this line
        }
    
        configFile.close();
        tempFile.close();
    #endif
    
        // Replace the original file with the temp file
        if (remove(filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to delete the original file: " + filePath);
            #endif
            return;
        }
    
        if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to rename the temporary file: " + tempPath);
            #endif
        }
    }
    

    /**
     * @brief Removes a key-value pair from an INI file.
     */
    void removeIniKey(const std::string& filePath, const std::string& sectionName, const std::string& keyName) {
        auto fileMutex = getFileMutex(filePath);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* configFile = fopen(filePath.c_str(), "r");
        if (!configFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        std::string tempPath = filePath + ".tmp";
        FILE* tempFile = fopen(tempPath.c_str(), "w");
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the temporary file: " + tempPath);
            #endif
            fclose(configFile);
            return;
        }
    
        // Declare all variables outside the loop for efficiency
        char line[1024];
        std::string currentSection;
        //currentSection.reserve(64);
        
        std::string lineStr;
        //lineStr.reserve(1024);
        
        std::string trimmedLine;
        //trimmedLine.reserve(1024);
        
        std::string lineKey;
        //lineKey.reserve(128);
        
        bool inTargetSection = false;
        size_t eqPos;
        
        while (fgets(line, sizeof(line), configFile)) {
            lineStr.assign(line);
            
            // Create a trimmed copy for parsing
            trimmedLine = lineStr;
            if (!trimmedLine.empty() && trimmedLine.back() == '\n') {
                trimmedLine.pop_back();
                if (!trimmedLine.empty() && trimmedLine.back() == '\r') {
                    trimmedLine.pop_back();
                }
            }
            trim(trimmedLine);
    
            if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
                currentSection.assign(trimmedLine, 1, trimmedLine.length() - 2);
                inTargetSection = (currentSection == sectionName);
                fprintf(tempFile, "%s", line); // Write original line
            } else if (inTargetSection) {
                // Check if line starts with the key (handle spaces around =)
                eqPos = trimmedLine.find('=');
                if (eqPos != std::string::npos) {
                    lineKey.assign(trimmedLine, 0, eqPos);
                    trim(lineKey);
                    if (lineKey == keyName) {
                        continue; // Skip this line
                    }
                }
                fprintf(tempFile, "%s", line);
            } else {
                fprintf(tempFile, "%s", line);
            }
        }
    
        fclose(configFile);
        fclose(tempFile);
    #else
        std::ifstream configFile(filePath);
        if (!configFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to open the input file: " + filePath);
            #endif
            return;
        }
    
        std::string tempPath = filePath + ".tmp";
        std::ofstream tempFile(tempPath);
        if (!tempFile) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create the temporary file: " + tempPath);
            #endif
            return;
        }
    
        // Declare all variables outside the loop for efficiency
        std::string line;
        //line.reserve(1024);
        
        std::string currentSection;
        //currentSection.reserve(64);
        
        std::string trimmedLine;
        //trimmedLine.reserve(1024);
        
        std::string lineKey;
        //lineKey.reserve(128);
        
        bool inTargetSection = false;
        size_t eqPos;
    
        while (getline(configFile, line)) {
            trimmedLine = line;
            // Remove carriage return if present
            if (!trimmedLine.empty() && trimmedLine.back() == '\r') {
                trimmedLine.pop_back();
            }
            trim(trimmedLine);
    
            if (!trimmedLine.empty() && trimmedLine.front() == '[' && trimmedLine.back() == ']') {
                currentSection.assign(trimmedLine, 1, trimmedLine.length() - 2);
                inTargetSection = (currentSection == sectionName);
                tempFile << line << '\n';
            } else if (inTargetSection) {
                // Better key matching that handles spaces
                eqPos = trimmedLine.find('=');
                if (eqPos != std::string::npos) {
                    lineKey.assign(trimmedLine, 0, eqPos);
                    trim(lineKey);
                    if (lineKey == keyName) {
                        continue; // Skip this line
                    }
                }
                tempFile << line << '\n';
            } else {
                tempFile << line << '\n';
            }
        }
    
        configFile.close();
        tempFile.close();
    #endif
    
        // Replace the original file with the temp file
        if (remove(filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to delete the original file: " + filePath);
            #endif
            return;
        }
    
        if (rename(tempPath.c_str(), filePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to rename the temporary file: " + tempPath);
            #endif
        }
    }
    
    //void saveIniFileData(const std::string& filePath, const std::map<std::string, std::map<std::string, std::string>>& data) {
    //    std::ofstream file(filePath);
    //    if (!file.is_open()) {
    //        // Handle error: could not open file
    //        return;
    //    }
    //
    //    for (const auto& section : data) {
    //        file << "[" << section.first << "]\n";
    //        for (const auto& kv : section.second) {
    //            file << kv.first << "=" << kv.second << "\n";
    //        }
    //        file << "\n"; // Separate sections with a newline
    //    }
    //
    //    file.close();
    //}
    
    
    void syncIniValue(std::map<std::string, std::map<std::string, std::string>>& packageConfigData,
                       const std::string& packageConfigIniPath,
                       const std::string& optionName,
                       const std::string& key,
                       std::string& value) {
        auto optionIt = packageConfigData.find(optionName);
        if (optionIt != packageConfigData.end()) {
            auto it = optionIt->second.find(key);
            if (it != optionIt->second.end()) {
                value = it->second;  // Update value only if the key exists
            //} else {
            //    setIniFileValue(packageConfigIniPath, optionName, key, value); // Set INI file value if key not found
            //}
            } else {
                // Key not found - add it to in-memory data and save entire structure
                packageConfigData[optionName][key] = value;
                saveIniFileData(packageConfigIniPath, packageConfigData);
            }
        }
    }
    
    
    /**
     * @brief Parses a command line into individual parts, handling quoted strings.
     *
     * @param line The command line to parse.
     * @return A vector of strings containing the parsed command parts.
     */
    std::vector<std::string> parseCommandLine(const std::string& line) {
        std::vector<std::string> commandParts;
        bool inQuotes = false;
        std::string part;
        //part.reserve(64);  // Reserve space to avoid reallocations
    
        StringStream iss(line);
        
        std::string arg;
        //arg.reserve(32);   // Reserve space for args too
        
        while (iss.getline(part, '\'')) {
            if (inQuotes) {
                commandParts.push_back(std::move(part));  // Use move instead of copy
            } else {
                StringStream argIss(part);
                while (argIss >> arg) {
                    commandParts.push_back(std::move(arg));  // Use move instead of copy
                }
            }
            inQuotes = !inQuotes;
        }
    
        return commandParts;
    }
    
    
    /**
     * @brief Loads and parses options from an INI file.
     *
     * This function reads and parses options from an INI file, organizing them by section.
     *
     * @param packageIniPath The path to the INI file.
     * @return A vector containing pairs of section names and their associated key-value pairs.
     */
    std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> loadOptionsFromIni(const std::string& packageIniPath) {
        auto fileMutex = getFileMutex(packageIniPath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* packageFile = fopen(packageIniPath.c_str(), "r");
        if (!packageFile) return {}; // Return empty vector if file can't be opened
        
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;
        
        char line[1024];
        std::string currentSection;
        std::vector<std::vector<std::string>> sectionCommands;
        std::string strLine;
        
        size_t len;
        //const char* start;
        //const char* end;
    
        while (fgets(line, sizeof(line), packageFile)) {
            // More efficient newline removal
            len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
                --len;
                if (len > 0 && line[len-1] == '\r') {
                    line[len-1] = '\0';
                    --len;
                }
            }
            
            // Early exit for empty lines
            if (len == 0) continue;
            
            // Check for comments early (most efficient check)
            if (line[0] == '#') continue;
            
            // Assign the processed line
            strLine.assign(line, len);
    
            if (strLine[0] == '[' && strLine.back() == ']') { // Section headers
                if (!currentSection.empty()) {
                    options.emplace_back(std::move(currentSection), std::move(sectionCommands));
                    //sectionCommands.clear();
                    //sectionCommands.shrink_to_fit(); // Free capacity after move
                }
                currentSection.assign(strLine, 1, strLine.size() - 2);
            } else if (!currentSection.empty()) { // Command lines within sections
                sectionCommands.push_back(parseCommandLine(strLine));
            }

            // Clear strLine content to free string memory
            strLine.clear();
        }
    
        if (!currentSection.empty()) {
            options.emplace_back(std::move(currentSection), std::move(sectionCommands));
        }
        
        fclose(packageFile);
    #else
        std::ifstream packageFile(packageIniPath);
        if (!packageFile) return {}; // Return empty vector if file can't be opened
    
        std::vector<std::pair<std::string, std::vector<std::vector<std::string>>>> options;
        
        std::string line, currentSection;
        std::vector<std::vector<std::string>> sectionCommands;
    
        while (std::getline(packageFile, line)) {
            // Remove carriage return if present (getline already removes \n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
    
            // Early exits for empty lines and comments
            if (line.empty() || line[0] == '#') continue;
    
            if (line[0] == '[' && line.back() == ']') { // Section headers
                if (!currentSection.empty()) {
                    options.emplace_back(std::move(currentSection), std::move(sectionCommands));
                    //sectionCommands.clear();
                    //sectionCommands.shrink_to_fit(); // Free capacity after move
                }
                currentSection.assign(line, 1, line.size() - 2);
            } else if (!currentSection.empty()) { // Command lines within sections
                sectionCommands.push_back(parseCommandLine(line));
            }
            // Clear line content to reuse capacity
            line.clear();
        }
    
        if (!currentSection.empty()) {
            options.emplace_back(std::move(currentSection), std::move(sectionCommands));
        }
        
        packageFile.close();
    #endif
    
        return options;
    }

    /**
     * @brief Loads a specific section from an INI file.
     *
     * This function reads and parses a specific section from an INI file.
     *
     * @param packageIniPath The path to the INI file.
     * @param sectionName The name of the section to load.
     * @return A vector of commands within the specified section.
     */
    std::vector<std::vector<std::string>> loadSpecificSectionFromIni(const std::string& packageIniPath, const std::string& sectionName) {
        auto fileMutex = getFileMutex(packageIniPath);
        std::shared_lock<std::shared_mutex> lock(*fileMutex);

    #if !USING_FSTREAM_DIRECTIVE
        FILE* packageFile = fopen(packageIniPath.c_str(), "r");
        
        if (!packageFile) return {}; // Return empty vector if file can't be opened
        
        std::vector<std::vector<std::string>> sectionCommands;
        
        char line[1024];
        std::string currentSection;
        bool inTargetSection = false;
        std::string strLine;
        
        size_t len;
        
        while (fgets(line, sizeof(line), packageFile)) {
            // More efficient newline removal
            len = strlen(line);
            if (len > 0 && line[len-1] == '\n') {
                line[len-1] = '\0';
                --len;
                if (len > 0 && line[len-1] == '\r') {
                    line[len-1] = '\0';
                    --len;
                }
            }
            
            // Early exit for empty lines
            if (len == 0) continue;
            
            // Check for comments early (most efficient check)
            if (line[0] == '#') continue;
            
            // Assign the processed line
            strLine.assign(line, len);
    
            if (strLine[0] == '[' && strLine.back() == ']') { // Section headers
                currentSection.assign(strLine, 1, strLine.size() - 2);
                inTargetSection = (currentSection == sectionName); // Check if this is the target section
                
                // Early exit optimization: if we were in target section and hit a new section, we're done
                if (!inTargetSection && !sectionCommands.empty()) {
                    break; // Found target section and processed it, no need to continue
                }
            } else if (inTargetSection) { // Only parse commands within the target section
                sectionCommands.push_back(parseCommandLine(strLine));
            }

            // Clear strings to reuse capacity
            strLine.clear();
            currentSection.clear();
        }
    
        fclose(packageFile);
    #else
        std::ifstream packageFile(packageIniPath);
        
        if (!packageFile) return {}; // Return empty vector if file can't be opened
        
        std::string line, currentSection;
        std::vector<std::vector<std::string>> sectionCommands;
        bool inTargetSection = false;
    
        while (std::getline(packageFile, line)) {
            // Remove carriage return if present (getline already removes \n)
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }
    
            // Early exits for empty lines and comments
            if (line.empty() || line[0] == '#') continue;
    
            if (line[0] == '[' && line.back() == ']') { // Section headers
                currentSection.assign(line, 1, line.size() - 2);
                inTargetSection = (currentSection == sectionName); // Check if this is the target section
                
                // Early exit optimization: if we were in target section and hit a new section, we're done
                if (!inTargetSection && !sectionCommands.empty()) {
                    break; // Found target section and processed it, no need to continue
                }
            } else if (inTargetSection) { // Only parse commands within the target section
                sectionCommands.push_back(parseCommandLine(line));
            }

            // Clear strings to reuse capacity
            strLine.clear();
            currentSection.clear();
        }
    
        packageFile.close();
    #endif
    
        return sectionCommands; // Return only the commands from the target section
    }
    

    /**
     * @brief Saves INI data structure to a file.
     *
     * This function writes a complete INI data structure to the specified file path.
     * The data structure should be organized as sections containing key-value pairs.
     *
     * @param filePath The path to the INI file to write.
     * @param data The complete INI data structure to save.
     */
    void saveIniFileData(const std::string& filePath, const std::map<std::string, std::map<std::string, std::string>>& data) {
        auto fileMutex = getFileMutex(filePath);
        std::unique_lock<std::shared_mutex> lock(*fileMutex);

        #if !USING_FSTREAM_DIRECTIVE
        FILE* file = fopen(filePath.c_str(), "w");
        if (!file) {
            // Handle error: could not open file
            return;
        }
    
        for (const auto& section : data) {
            fprintf(file, "[%s]\n", section.first.c_str());
            for (const auto& kv : section.second) {
                fprintf(file, "%s=%s\n", kv.first.c_str(), kv.second.c_str());
            }
            fprintf(file, "\n"); // Separate sections with a newline
        }
    
        fclose(file);
        #else
        std::ofstream file(filePath);
        if (!file.is_open()) {
            // Handle error: could not open file
            return;
        }
    
        for (const auto& section : data) {
            file << "[" << section.first << "]\n";
            for (const auto& kv : section.second) {
                file << kv.first << "=" << kv.second << "\n";
            }
            file << "\n"; // Separate sections with a newline
        }
    
        file.close();
        #endif
    }
}