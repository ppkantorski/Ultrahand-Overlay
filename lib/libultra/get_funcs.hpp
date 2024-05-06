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
//#include <sys/stat.h>
#include <fstream>
#include <dirent.h>
#include <fnmatch.h>
#include <jansson.h>
#include "debug_funcs.hpp"
#include "string_funcs.hpp"


// Constants for overlay module
constexpr int OverlayLoaderModuleId = 348;
constexpr Result ResultSuccess = MAKERESULT(0, 0);
constexpr Result ResultParseError = MAKERESULT(OverlayLoaderModuleId, 1);



struct DirCloser {
    void operator()(DIR* dir) const {
        if (dir) closedir(dir);
    }
};



/**
 * @brief Retrieves overlay module information from a given file.
 *
 * @param filePath The path to the overlay module file.
 * @return A tuple containing the result code, module name, and display version.
 */
std::tuple<Result, std::string, std::string> getOverlayInfo(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        return {ResultParseError, "", ""};
    }

    NroHeader nroHeader;
    NroAssetHeader assetHeader;
    NacpStruct nacp;

    // Read NRO header
    file.seekg(sizeof(NroStart), std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&nroHeader), sizeof(NroHeader))) {
        return {ResultParseError, "", ""};
    }

    // Read asset header
    file.seekg(nroHeader.size, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&assetHeader), sizeof(NroAssetHeader))) {
        return {ResultParseError, "", ""};
    }

    // Read NACP struct
    file.seekg(nroHeader.size + assetHeader.nacp.offset, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(&nacp), sizeof(NacpStruct))) {
        return {ResultParseError, "", ""};
    }

    // Assuming nacp.lang[0].name and nacp.display_version are null-terminated
    return {
        ResultSuccess,
        std::string(nacp.lang[0].name),
        std::string(nacp.display_version)
    };
}



/**
 * @brief Reads the contents of a file and returns it as a string, normalizing line endings.
 *
 * @param filePath The path to the file to be read.
 * @return The content of the file as a string with line endings normalized to '\n'.
 */
std::string getFileContents(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary | std::ios::ate);
    if (!file) {
        logMessage("Failed to open file: " + filePath);
        return "";
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        return "";
    }

    file.seekg(0, std::ios::beg);
    std::string content(size, '\0');
    if (!file.read(&content[0], size)) {
        logMessage("Failed to read file: " + filePath);
    }

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
    size_t equalsPos = line.find('=');
    if (equalsPos != std::string::npos) {
        std::string value = line.substr(equalsPos + 1);
        return trim(value);
    }
    return "";
}

/**
 * @brief Extracts the name from a file path, including handling directories.
 *
 * @param path The file path from which to extract the name.
 * @return The extracted name as a string. If the path indicates a directory, it extracts the last directory name.
 * If the path is empty or no name is found, an empty string is returned.
 */
std::string getNameFromPath(const std::string& path) {
    size_t lastSlash = path.find_last_of('/');
    if (lastSlash != std::string::npos) {
        std::string name = path.substr(lastSlash + 1);
        if (name.empty()) {
            // The path ends with a slash, indicating a directory
            std::string strippedPath = path.substr(0, lastSlash);
            lastSlash = strippedPath.find_last_of('/');
            if (lastSlash != std::string::npos) {
                name = strippedPath.substr(lastSlash + 1);
            }
        }
        return name;
    }
    return path;
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
    size_t pos = path.find_last_of("/\\");
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
std::string getParentDirNameFromPath(const std::string& path, size_t level = 0) {
    // Split the path into individual directories
    std::vector<std::string> directories;
    size_t pos = 0;
    while (pos != std::string::npos) {
        size_t nextPos = path.find('/', pos + 1);
        directories.push_back(path.substr(pos + 1, nextPos - pos - 1));
        pos = nextPos;
    }

    // Calculate the index of the desired directory
    size_t targetIndex = directories.size() - 2 - level; // Adjusted to get parent directory

    // Check if the target index is valid
    if (targetIndex < directories.size() - 1) {
        // Extract the directory name at the target index
        std::string targetDir = directories[targetIndex];

        // Check if the directory name contains spaces or special characters
        if (targetDir.find_first_of(" \t\n\r\f\v") != std::string::npos) {
            // If it does, return the directory name within quotes
            return "\"" + targetDir + "\"";
        }

        // If it doesn't, return the directory name as is
        return targetDir;
    }

    // If the path format is not as expected or the target directory is not found,
    // return an empty string or handle the case accordingly
    return "";
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
        std::string parentDir = path.substr(0, lastSlash + 1);
        return parentDir;
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
        dirent* entry;
        while ((entry = readdir(dir.get())) != nullptr) {
            std::string entryName = entry->d_name;
            
            // Exclude current directory (.) and parent directory (..)
            if (entryName != "." && entryName != "..") {
                std::string fullPath = directoryPath + "/" + entryName;
                struct stat entryStat;
                
                if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
                    subdirectories.push_back(entryName);
                }
            }
        }
    }
    
    return subdirectories;
}


// Cache to store directory status
std::unordered_map<std::string, bool> directoryCache;

bool isDirectory2(struct dirent* entry, const std::string& path) {
    if (entry->d_type == DT_DIR) {
        return true;
    } else if (entry->d_type == DT_UNKNOWN) {
        // Check if directory status is already cached
        auto it = directoryCache.find(path);
        if (it != directoryCache.end()) {
            return it->second;
        } else {
            struct stat path_stat;
            stat(path.c_str(), &path_stat);
            bool isDir = S_ISDIR(path_stat.st_mode);
            directoryCache[path] = isDir; // Cache directory status
            return isDir;
        }
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
    
    if (!dir) return fileList;  // Return empty list if directory cannot be opened

    struct dirent* entry;
    while ((entry = readdir(dir.get())) != nullptr) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;  // Skip the current and parent directory entries
        }

        std::string entryPath = directoryPath;
        if (entryPath.back() != '/') {
            entryPath += '/';
        }
        entryPath += entry->d_name;

        if (isDirectory2(entry, entryPath)) {  // Check if the path is a directory
            // Recursively retrieve files from subdirectories
            std::vector<std::string> subDirFiles = getFilesListFromDirectory(entryPath);
            fileList.insert(fileList.end(), subDirFiles.begin(), subDirFiles.end());
        } else {
            fileList.push_back(entryPath);  // It's a file, add to the list
        }
    }

    return fileList;
}


/**
 * @brief Gets a list of files and folders based on a wildcard pattern.
 *
 * @param pathPattern The wildcard pattern to match files and folders.
 * @return A vector of strings containing the paths of matching files and folders.
 */
std::vector<std::string> getFilesListByWildcard(const std::string& pathPattern) {
    std::string dirPath;
    std::string wildcard;
    size_t wildcardPos = pathPattern.find('*');

    if (wildcardPos != std::string::npos) {
        size_t slashPos = pathPattern.rfind('/', wildcardPos);
        if (slashPos != std::string::npos) {
            dirPath = pathPattern.substr(0, slashPos + 1);
            wildcard = pathPattern.substr(slashPos + 1);
        } else {
            dirPath = "./";  // Assume current directory if no slash is found
            wildcard = pathPattern;
        }
    } else {
        dirPath = pathPattern + "/";
    }

    bool isFolderWildcard = !wildcard.empty() && wildcard.back() == '/';
    if (isFolderWildcard) {
        wildcard.pop_back();  // Prepare wildcard for directory matching
    }

    std::vector<std::string> fileList;
    std::unique_ptr<DIR, DirCloser> dir(opendir(dirPath.c_str()));
    if (!dir) return fileList;  // Early exit if the directory cannot be opened

    dirent* entry;
    while ((entry = readdir(dir.get())) != nullptr) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..") continue;

        std::string entryPath = dirPath + entryName;
        bool isEntryDirectory = isDirectory2(entry, entryPath);

        if (isFolderWildcard && isEntryDirectory) {
            if (fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0) {
                fileList.push_back(entryPath + "/");
            }
        } else if (!isFolderWildcard && !isEntryDirectory) {
            if (fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0) {
                fileList.push_back(entryPath);
            }
        }
    }

    return fileList;
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
    std::vector<std::string> fileList;

    // Find the position of the first wildcard
    size_t firstWildcardPos = pathPattern.find('*');
    if (firstWildcardPos != std::string::npos) {
        // Check if there's another wildcard following the first
        size_t secondWildcardPos = pathPattern.find('*', firstWildcardPos + 1);
        size_t slashPos = pathPattern.rfind('/', firstWildcardPos);

        // Extract directory path up to the first wildcard
        std::string dirPath = (slashPos != std::string::npos) ? pathPattern.substr(0, slashPos + 1) : "";
        std::string firstWildcard = (slashPos != std::string::npos) ? pathPattern.substr(slashPos + 1, firstWildcardPos - slashPos - 1) : pathPattern.substr(0, firstWildcardPos);

        if (secondWildcardPos != std::string::npos) {
            // Get the list of directories matching the first wildcard
            std::vector<std::string> subDirs = getFilesListByWildcards(dirPath + firstWildcard + "*/");

            // Process each directory recursively
            for (const std::string& subDir : subDirs) {
                // Append the rest of the path pattern after the first wildcard to each subdirectory
                std::string subPattern = subDir + pathPattern.substr(secondWildcardPos);
                std::vector<std::string> subFileList = getFilesListByWildcards(subPattern);
                fileList.insert(fileList.end(), subFileList.begin(), subFileList.end());
            }
        } else {
            // If there's only one wildcard, use getFilesListByWildcard directly
            fileList = getFilesListByWildcard(pathPattern);
        }
    }

    return fileList;
}
