/********************************************************************************
 * File: get_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains functions for retrieving information and data
 *   from various sources, including file system and JSON files. It includes
 *   functions for obtaining overlay module information, reading file contents,
 *   and parsing JSON data.
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

#include "get_funcs.hpp"


namespace ult {
    
    /**
     * @brief Reads the contents of a file and returns it as a string, normalizing line endings.
     *
     * @param filePath The path to the file to be read.
     * @return The content of the file as a string with line endings normalized to '\n'.
     */
    std::string getFileContents(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file) {
            logMessage("Failed to open file: " + filePath);
            return "";
        }
    
        // Determine the file size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        if (size <= 0) {
            return "";
        }
        file.seekg(0, std::ios::beg);
    
        // Read the entire file into a string
        std::string content(size, '\0');
        if (!file.read(&content[0], size)) {
            logMessage("Failed to read file: " + filePath);
            return "";
        }
    
        // Erase any carriage return characters
        content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());
        return content;
    }
    
    
    
    
    /**
     * @brief Concatenates the provided directory and file names to form a destination path.
     *
     * @param destinationDir The directory where the file should be placed.
     * @param fileName The name of the file.
     * @return The destination path as a string.
     */
    std::string getDestinationPath(const std::string& destinationDir, const std::string& fileName) {
        return destinationDir + "/" + fileName;
    }
    
    /**
     * @brief Extracts the value part from a string line containing a key-value pair.
     *
     * @param line The string line containing a key-value pair (e.g., "key=value").
     * @return The extracted value as a string. If no value is found, an empty string is returned.
     */
    std::string getValueFromLine(const std::string& line) {
        // Find the position of '=' character from the end of the string
        size_t equalsPos = line.rfind('=');
        if (equalsPos != std::string::npos && equalsPos + 1 < line.size()) {
            // Directly return the trimmed substring after '='
            std::string newLine = line.substr(equalsPos + 1);
            trim(newLine);
            return newLine;
        }
        return ""; // Return an empty string if '=' is not found or no content after '='
    }
    
    
    
    /**
     * @brief Extracts the name from a file path, including handling directories.
     *
     * @param path The file path from which to extract the name.
     * @return The extracted name as a string. If the path indicates a directory, it extracts the last directory name.
     * If the path is empty or no name is found, an empty string is returned.
     */
    std::string getNameFromPath(const std::string& path) {
        size_t lastNonSlash = path.find_last_not_of('/');
        if (lastNonSlash == std::string::npos) {
            return "";  // All slashes, or empty string effectively
        }
    
        size_t lastSlash = path.find_last_of('/', lastNonSlash);
        if (lastSlash == std::string::npos) {
            return path.substr(0, lastNonSlash + 1);  // No slashes, the entire path is a filename
        }
    
        return path.substr(lastSlash + 1, lastNonSlash - lastSlash);  // Standard case, efficiently handled
    }
    
    
    
    /**
     * @brief Extracts the file name from a full file path.
     *
     * This function takes a filesystem path and returns only the file name,
     * stripping away any directory paths that precede it.
     *
     * @param path The full path to the file.
     * @return The file name extracted from the full path.
     */
    std::string getFileName(const std::string& path) {
        // Find the last slash in the path
        size_t pos = path.find_last_of("/");
        if (pos != std::string::npos) {
            // Return the substring after the last slash
            return path.substr(pos + 1);
        }
        return "";
    }
    
    
    
    /**
     * @brief Extracts the name of the parent directory from a given file path at a specified level.
     *
     * @param path The file path from which to extract the parent directory name.
     * @param level The level of the parent directory to extract (0 for immediate parent, 1 for grandparent, and so on).
     * @return The name of the parent directory at the specified level.
     */
    std::string getParentDirNameFromPath(const std::string& path, size_t level) {
        if (path.empty()) return "";
    
        // Start from the end of the string and move backwards to find the slashes
        size_t endPos = path.find_last_not_of('/');
        if (endPos == std::string::npos) return ""; // All slashes or empty path
    
        size_t pos = path.rfind('/', endPos);
        if (pos == std::string::npos || pos == 0) return ""; // No parent directory or single slash
    
        while (level-- > 0 && pos != std::string::npos) {
            endPos = pos - 1;
            pos = path.rfind('/', endPos);
            if (pos == std::string::npos || pos == 0) return ""; // No more directories to go up
        }
    
        size_t start = path.rfind('/', pos - 1);
        if (start == std::string::npos) start = 0;
        else start += 1; // Move past the slash
    
        std::string parentDir = path.substr(start, pos - start);
    
        // Check for spaces or special characters
        if (parentDir.find_first_of(" \t\n\r\f\v") != std::string::npos) {
            // If it does, return the directory name within quotes
            return "\"" + parentDir + "\"";
        }
    
        return parentDir;
    }
    
    
    
    /**
     * @brief Extracts the parent directory path from a given file path.
     *
     * @param path The file path from which to extract the parent directory path.
     * @return The parent directory path.
     */
    std::string getParentDirFromPath(const std::string& path) {
        size_t lastSlash = path.find_last_of('/');
        if (lastSlash != std::string::npos) {
            return  path.substr(0, lastSlash + 1);
        }
        return path;
    }
    
    
    
    /**
     * @brief Gets a list of subdirectories in a directory.
     *
     * @param directoryPath The path of the directory to search.
     * @return A vector of strings containing the names of subdirectories.
     */
    std::vector<std::string> getSubdirectories(const std::string& directoryPath) {
        std::vector<std::string> subdirectories;
        std::unique_ptr<DIR, DirCloser> dir(opendir(directoryPath.c_str()));
        
        if (dir) {
            std::string entryName, fullPath;
            dirent* entry;
            struct stat entryStat;
            while ((entry = readdir(dir.get())) != nullptr) {
                entryName = entry->d_name;
                
                // Exclude current directory (.) and parent directory (..)
                if (entryName != "." && entryName != "..") {
                    fullPath = directoryPath + "/" + entryName;
                    
                    if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
                        subdirectories.push_back(entryName);
                    }
                }
            }
        }
        
        return subdirectories;
    }
    
    
    // Cache to store directory status
    // Assuming a very simple cache implementation
    std::vector<std::pair<std::string, bool>> directoryCache;
    
    bool isDirectoryCached(struct dirent* entry, const std::string& path) {
        if (entry->d_type == DT_DIR) {
            return true;
        } else if (entry->d_type == DT_UNKNOWN) {
            for (const auto& item : directoryCache) {
                if (item.first == path) {
                    return item.second;
                }
            }
    
            struct stat path_stat;
            stat(path.c_str(), &path_stat);
            bool isDir = S_ISDIR(path_stat.st_mode);
            directoryCache.push_back({path, isDir});
            return isDir;
        }
        return false;
    }
    
    /**
     * @brief Recursively retrieves a list of files from a directory.
     *
     * @param directoryPath The path of the directory to search.
     * @return A vector of strings containing the paths of the files.
     */
    std::vector<std::string> getFilesListFromDirectory(const std::string& directoryPath) {
        std::vector<std::string> fileList;
        std::unique_ptr<DIR, DirCloser> dir(opendir(directoryPath.c_str()));
        std::vector<std::string> subDirFiles;  // Moved vector definition here
    
        if (!dir) return fileList;  // Return empty list if directory cannot be opened
    
        struct dirent* entry;
        while ((entry = readdir(dir.get())) != nullptr) {
            if (entry->d_type == DT_REG) {  // Check if it's a regular file
                fileList.push_back(directoryPath + "/" + entry->d_name);
            } else if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                // Recursively retrieve files from subdirectories
                subDirFiles = getFilesListFromDirectory(directoryPath + "/" + entry->d_name);
                fileList.insert(fileList.end(), subDirFiles.begin(), subDirFiles.end());
            }
        }
    
        return fileList;
    }
    
    // Helper function to check if a path is a directory
    //bool isDirectoryCached(const struct dirent* entry, const std::string& fullPath) {
    //    struct stat st;
    //    if (stat(fullPath.c_str(), &st) != 0) return false;
    //    return S_ISDIR(st.st_mode);
    //}
    
    // Recursive function to handle wildcard directories and file patterns
    void handleDirectory(const std::string& basePath, const std::vector<std::string>& parts, size_t partIndex, std::vector<std::string>& results, bool directoryOnly) {
        if (partIndex >= parts.size()) return;
    
        // Open the directory safely using a smart pointer
        std::unique_ptr<DIR, DirCloser> dir(opendir(basePath.c_str()));
        if (!dir) return;
    
        struct dirent* entry;
        std::string entryName, fullPath;
        bool isCurrentDir, match;
    
        while ((entry = readdir(dir.get())) != nullptr) {
            entryName = entry->d_name;
            if (entryName == "." || entryName == "..") continue;
            
            fullPath = basePath + (basePath.back() == '/' ? "" : "/") + entryName;
            isCurrentDir = isDirectoryCached(entry, fullPath);
            
            match = (fnmatch(parts[partIndex].c_str(), entryName.c_str(), FNM_NOESCAPE) == 0);
            if (!match) continue; // Skip non-matching entries
            
            // Recurse into directories if there are more parts to process
            if (isCurrentDir && partIndex < parts.size() - 1) {
                handleDirectory(fullPath, parts, partIndex + 1, results, directoryOnly);
            }
            
            // Add matching directories/files to results
            if (match && partIndex == parts.size() - 1 && (directoryOnly ? isCurrentDir : true)) {
                results.push_back(fullPath + (isCurrentDir ? "/" : ""));
            }
        }
    }
    
    /**
     * @brief Gets a list of files and folders based on a wildcard pattern.
     *
     * This function searches for files and folders in a directory that match the
     * specified wildcard pattern.
     *
     * @param pathPattern The wildcard pattern to match files and folders.
     * @return A vector of strings containing the paths of matching files and folders.
     */
    std::vector<std::string> getFilesListByWildcards(const std::string& pathPattern) {
        std::vector<std::string> results;
        bool directoryOnly = pathPattern.back() == '/';
        size_t prefixEnd = pathPattern.find(":/") + 2;
        std::string basePath = pathPattern.substr(0, prefixEnd);
        std::vector<std::string> parts;
    
        size_t start = prefixEnd, pos;
        while ((pos = pathPattern.find('/', start)) != std::string::npos) {
            parts.push_back(pathPattern.substr(start, pos - start));
            start = pos + 1;
        }
        if (!directoryOnly) {
            parts.push_back(pathPattern.substr(start)); // Include the last segment if not directoryOnly
        }
    
        handleDirectory(basePath, parts, 0, results, directoryOnly);
    
        return results;
    }
    
}