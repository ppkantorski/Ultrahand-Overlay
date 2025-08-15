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

#pragma once

#ifndef GET_FUNCS_HPP
#define GET_FUNCS_HPP


#include <cstring>
#include <dirent.h>
#include <fnmatch.h>
#include "debug_funcs.hpp"
#include "string_funcs.hpp"


namespace ult {
    struct DirCloser {
        void operator()(DIR* dir) const {
            if (dir) closedir(dir);
        }
    };
    
    
    
    /**
     * @brief Reads the contents of a file and returns it as a string, normalizing line endings.
     *
     * @param filePath The path to the file to be read.
     * @return The content of the file as a string with line endings normalized to '\n'.
     */
    //std::string getFileContents(const std::string& filePath);
    
    
    /**
     * @brief Concatenates the provided directory and file names to form a destination path.
     *
     * @param destinationDir The directory where the file should be placed.
     * @param fileName The name of the file.
     * @return The destination path as a string.
     */
    std::string getDestinationPath(const std::string& destinationDir, const std::string& fileName);
    
    /**
     * @brief Extracts the value part from a string line containing a key-value pair.
     *
     * @param line The string line containing a key-value pair (e.g., "key=value").
     * @return The extracted value as a string. If no value is found, an empty string is returned.
     */
    std::string getValueFromLine(const std::string& line);
    
    
    
    /**
     * @brief Extracts the name from a file path, including handling directories.
     *
     * @param path The file path from which to extract the name.
     * @return The extracted name as a string. If the path indicates a directory, it extracts the last directory name.
     * If the path is empty or no name is found, an empty string is returned.
     */
    std::string getNameFromPath(const std::string& path);
    
    
    
    /**
     * @brief Extracts the file name from a full file path.
     *
     * This function takes a filesystem path and returns only the file name,
     * stripping away any directory paths that precede it.
     *
     * @param path The full path to the file.
     * @return The file name extracted from the full path.
     */
    std::string getFileName(const std::string& path);
    
    
    
    /**
     * @brief Extracts the name of the parent directory from a given file path at a specified level.
     *
     * @param path The file path from which to extract the parent directory name.
     * @param level The level of the parent directory to extract (0 for immediate parent, 1 for grandparent, and so on).
     * @return The name of the parent directory at the specified level.
     */
    std::string getParentDirNameFromPath(const std::string& path, size_t level = 0);
    
    
    
    /**
     * @brief Extracts the parent directory path from a given file path.
     *
     * @param path The file path from which to extract the parent directory path.
     * @return The parent directory path.
     */
    std::string getParentDirFromPath(const std::string& path);
    
    
    
    /**
     * @brief Gets a list of subdirectories in a directory.
     *
     * @param directoryPath The path of the directory to search.
     * @return A vector of strings containing the names of subdirectories.
     */
    std::vector<std::string> getSubdirectories(const std::string& directoryPath);
    
    // Cache to store directory status
    // Assuming a very simple cache implementation
    //extern std::vector<std::pair<std::string, bool>> directoryCache;
    
    bool isDirectory(struct dirent* entry, const std::string& path);
    
    /**
     * @brief Recursively retrieves a list of files from a directory.
     *
     * @param directoryPath The path of the directory to search.
     * @return A vector of strings containing the paths of the files.
     */
    std::vector<std::string> getFilesListFromDirectory(const std::string& directoryPath);
    
    // Helper function to check if a path is a directory
    //bool isDirectoryCached(const struct dirent* entry, const std::string& fullPath) {
    //    struct stat st;
    //    if (stat(fullPath.c_str(), &st) != 0) return false;
    //    return S_ISDIR(st.st_mode);
    //}
    
    

    void handleDirectory(const std::string& basePath, const std::vector<std::string>& parts, size_t partIndex, std::vector<std::string>& results, bool directoryOnly, size_t maxLines=0);
    /**
     * @brief Gets a list of files and folders based on a wildcard pattern.
     *
     * This function searches for files and folders in a directory that match the
     * specified wildcard pattern.
     *
     * @param pathPattern The wildcard pattern to match files and folders.
     * @return A vector of strings containing the paths of matching files and folders.
     */
    std::vector<std::string> getFilesListByWildcards(const std::string& pathPattern, size_t maxLines=0);
    
}

#endif