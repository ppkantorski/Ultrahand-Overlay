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
 *  Copyright (c) 2023 ppkantorski
 *  All rights reserved.
 ********************************************************************************/

#pragma once
#include <sys/stat.h>
#include <dirent.h>
#include <fnmatch.h>
#include <jansson.h>
#include <string_funcs.hpp>
#include "debug_funcs.hpp"

// Constants for overlay module
constexpr int OverlayLoaderModuleId = 348;
constexpr Result ResultSuccess = MAKERESULT(0, 0);
constexpr Result ResultParseError = MAKERESULT(OverlayLoaderModuleId, 1);

/**
 * @brief Retrieves overlay module information from a given file.
 *
 * @param filePath The path to the overlay module file.
 * @return A tuple containing the result code, module name, and display version.
 */
std::tuple<Result, std::string, std::string> getOverlayInfo(std::string filePath) {
    FILE* file = fopen(filePath.c_str(), "r");

    NroHeader nroHeader;
    NroAssetHeader assetHeader;
    NacpStruct nacp;

    // Read NRO header
    fseek(file, sizeof(NroStart), SEEK_SET);
    if (fread(&nroHeader, sizeof(NroHeader), 1, file) != 1) {
        fclose(file);
        return { ResultParseError, "", "" };
    }

    // Read asset header
    fseek(file, nroHeader.size, SEEK_SET);
    if (fread(&assetHeader, sizeof(NroAssetHeader), 1, file) != 1) {
        fclose(file);
        return { ResultParseError, "", "" };
    }

    // Read NACP struct
    fseek(file, nroHeader.size + assetHeader.nacp.offset, SEEK_SET);
    if (fread(&nacp, sizeof(NacpStruct), 1, file) != 1) {
        fclose(file);
        return { ResultParseError, "", "" };
    }
    
    fclose(file);

    // Return overlay information
    return {
        ResultSuccess,
        std::string(nacp.lang[0].name, std::strlen(nacp.lang[0].name)),
        std::string(nacp.display_version, std::strlen(nacp.display_version))
    };
}


/**
 * @brief Reads the contents of a file and returns it as a string.
 *
 * @param filePath The path to the file to be read.
 * @return The content of the file as a string.
 */
std::string getFileContents(const std::string& filePath) {
    std::string content;
    FILE* file = fopen(filePath.c_str(), "rb");
    if (file) {
        struct stat fileInfo;
        if (stat(filePath.c_str(), &fileInfo) == 0 && fileInfo.st_size > 0) {
            content.resize(fileInfo.st_size);
            fread(&content[0], 1, fileInfo.st_size, file);
        }
        fclose(file);

        // Normalize line endings to '\n'
        content.erase(std::remove(content.begin(), content.end(), '\r'), content.end());
    }
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
    std::size_t equalsPos = line.find('=');
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
 * @brief Extracts the file name from a URL.
 *
 * @param url The URL from which to extract the file name.
 * @return The extracted file name.
 */
std::string getFileNameFromURL(const std::string& url) {
    size_t lastSlash = url.find_last_of('/');
    if (lastSlash != std::string::npos)
        return url.substr(lastSlash + 1);
    return "";
}



/**
 * @brief Extracts the name of the parent directory from a given file path.
 *
 * @param path The file path from which to extract the parent directory name.
 * @return The parent directory name.
 */
std::string getParentDirNameFromPath(const std::string& path) {
    // Find the position of the last occurrence of the directory separator '/'
    std::size_t lastSlashPos = removeEndingSlash(path).rfind('/');

    // Check if the slash is found and not at the beginning of the path
    if (lastSlashPos != std::string::npos && lastSlashPos != 0) {
        // Find the position of the second last occurrence of the directory separator '/'
        std::size_t secondLastSlashPos = path.rfind('/', lastSlashPos - 1);

        // Check if the second last slash is found
        if (secondLastSlashPos != std::string::npos) {
            // Extract the substring between the second last and last slashes
            std::string subPath = path.substr(secondLastSlashPos + 1, lastSlashPos - secondLastSlashPos - 1);

            // Check if the substring contains spaces or special characters
            if (subPath.find_first_of(" \t\n\r\f\v") != std::string::npos) {
                // If it does, return the substring within quotes
                return "\"" + subPath + "\"";
            }

            // If it doesn't, return the substring as is
            return subPath;
        }
    }

    // If the path format is not as expected or the parent directory is not found,
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

    DIR* dir = opendir(directoryPath.c_str());
    if (dir != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string entryName = entry->d_name;

            // Exclude current directory (.) and parent directory (..)
            if (entryName != "." && entryName != "..") {
                struct stat entryStat;
                std::string fullPath = directoryPath + "/" + entryName;

                if (stat(fullPath.c_str(), &entryStat) == 0 && S_ISDIR(entryStat.st_mode)) {
                    subdirectories.push_back(entryName);
                }
            }
        }

        closedir(dir);
    }

    return subdirectories;
}

/**
 * @brief Recursively retrieves a list of files from a directory.
 *
 * @param directoryPath The path of the directory to search.
 * @return A vector of strings containing the paths of the files.
 */
std::vector<std::string> getFilesListFromDirectory(const std::string& directoryPath) {
    std::vector<std::string> fileList;

    DIR* dir = opendir(directoryPath.c_str());
    if (dir != nullptr) {
        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string entryName = entry->d_name;
            std::string entryPath = directoryPath;
            if (entryPath.back() != '/')
                entryPath += '/';
            entryPath += entryName;

            // Skip directories "." and ".."
            if (entryName != "." && entryName != "..") {
                if (isDirectory(entryPath)) {
                    // Recursively retrieve files from subdirectories
                    std::vector<std::string> subDirFiles = getFilesListFromDirectory(entryPath);
                    fileList.insert(fileList.end(), subDirFiles.begin(), subDirFiles.end());
                } else {
                    fileList.push_back(entryPath);
                }
            }
        }
        closedir(dir);
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
    std::string dirPath = "";
    std::string wildcard = "";

    std::size_t wildcardPos = pathPattern.find('*');
    if (wildcardPos != std::string::npos) {
        std::size_t slashPos = pathPattern.rfind('/', wildcardPos);

        if (slashPos != std::string::npos) {
            dirPath = pathPattern.substr(0, slashPos + 1);
            wildcard = pathPattern.substr(slashPos + 1);
        } else {
            dirPath = "";
            wildcard = pathPattern;
        }
    } else {
        dirPath = pathPattern + "/";
    }

    //logMessage("dirPath: " + dirPath);
    //logMessage("wildcard: " + wildcard);

    std::vector<std::string> fileList;

    bool isFolderWildcard = wildcard.back() == '/';
    if (isFolderWildcard) {
        wildcard = wildcard.substr(0, wildcard.size() - 1);  // Remove the trailing slash
    }

    //logMessage("isFolderWildcard: " + std::to_string(isFolderWildcard));

    DIR* dir = opendir(dirPath.c_str());
    if (dir != nullptr) {
        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string entryName = entry->d_name;
            std::string entryPath = dirPath + entryName;

            bool isEntryDirectory = isDirectory(entryPath);

            //logMessage("entryName: " + entryName);
            //logMessage("entryPath: " + entryPath);
            //logMessage("isFolderWildcard: " + std::to_string(isFolderWildcard));
            //logMessage("isEntryDirectory: " + std::to_string(isEntryDirectory));

            if (isFolderWildcard && isEntryDirectory && fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0) {
                if (entryName != "." && entryName != "..") {
                    fileList.push_back(entryPath+"/");
                }
            } else if (!isFolderWildcard && !isEntryDirectory) {
                std::size_t wildcardPos = wildcard.find('*');
                if (wildcardPos != std::string::npos) {
                    std::string prefix = wildcard.substr(0, wildcardPos);
                    if (entryName.find(prefix) == 0) {
                        std::string suffix = wildcard.substr(wildcardPos + 1);
                        if (entryName.size() >= suffix.size() && entryName.compare(entryName.size() - suffix.size(), suffix.size(), suffix) == 0) {
                            fileList.push_back(entryPath);
                        }
                    }
                } else if (fnmatch(wildcard.c_str(), entryName.c_str(), FNM_NOESCAPE) == 0) {
                    fileList.push_back(entryPath);
                }
            }
        }
        closedir(dir);
    }

    //std::string fileListAsString;
    //for (const std::string& filePath : fileList) {
    //    fileListAsString += filePath + "\n";
    //}
    //logMessage("File List:\n" + fileListAsString);

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

    // Check if the pattern contains multiple wildcards
    std::size_t wildcardPos = pathPattern.find('*');
    if (wildcardPos != std::string::npos && pathPattern.find('*', wildcardPos + 1) != std::string::npos) {
        std::string dirPath = "";
        std::string wildcard = "";

        // Extract the directory path and the first wildcard
        std::size_t slashPos = pathPattern.rfind('/', wildcardPos);
        if (slashPos != std::string::npos) {
            dirPath = pathPattern.substr(0, slashPos + 1);
            wildcard = pathPattern.substr(slashPos + 1, wildcardPos - slashPos - 1);
        } else {
            dirPath = "";
            wildcard = pathPattern.substr(0, wildcardPos);
        }

        // Get the list of directories matching the first wildcard
        std::vector<std::string> subDirs = getFilesListByWildcard(dirPath + wildcard + "*/");

        // Process each subdirectory recursively
        for (const std::string& subDir : subDirs) {
            std::string subPattern = subDir + removeLeadingSlash(pathPattern.substr(wildcardPos + 1));
            std::vector<std::string> subFileList = getFilesListByWildcards(subPattern);
            fileList.insert(fileList.end(), subFileList.begin(), subFileList.end());
        }
    } else {
        // Only one wildcard present, use getFilesListByWildcard directly
        fileList = getFilesListByWildcard(pathPattern);
    }

    return fileList;
}


/**
 * @brief Replaces a placeholder with a replacement string in the input.
 *
 * This function replaces all occurrences of a specified placeholder with the
 * provided replacement string in the input string.
 *
 * @param input The input string.
 * @param placeholder The placeholder to replace.
 * @param replacement The string to replace the placeholder with.
 * @return The input string with placeholders replaced by the replacement string.
 */
std::string replacePlaceholder(const std::string& input, const std::string& placeholder, const std::string& replacement) {
    std::string result = input;
    std::size_t pos = result.find(placeholder);
    if (pos != std::string::npos) {
        result.replace(pos, placeholder.length(), replacement);
    }
    return result;
}

/**
 * @brief Replaces a JSON source placeholder with the actual JSON source.
 *
 * This function replaces a JSON source placeholder with the actual JSON source
 * based on the provided JSON path.
 *
 * @param placeholder The JSON source placeholder.
 * @param jsonSource The JSON information source. (will differ depending on source param)
 * @param source A boolean flag indicating whether to include the source.
 * @return The updated JSON source with the placeholder replaced.
 */
std::string replaceJsonSourcePlaceholder(const std::string& placeholder, const std::string& jsonSource, const std::string type = "") {
    // Load JSON data from the provided file
    json_t* root = nullptr; // Initialize root to nullptr
    json_error_t error;
    

    std::string replacement = placeholder;
    std::string searchString = "{json_file(";
    if (type == "file") {
        searchString = "{json_file_source(";
        root = json_load_file(jsonSource.c_str(), 0, &error);
        if (!root) {
            // Handle JSON parsing error
            // printf("JSON parsing error: %s\n", error.text);
            return placeholder;  // Return the original placeholder if JSON parsing fails
        }
    } else if (type == "variable") {
        searchString = "{json_source(";
        
        root = stringToJson(removeQuotes(jsonSource.c_str()));
        if (!root) {
            // Handle JSON parsing error
            // printf("JSON parsing error: %s\n", error.text);
            return placeholder;  // Return the original placeholder if JSON parsing fails
        }
    }
    
    std::size_t startPos = replacement.find(searchString);
    std::size_t endPos = replacement.find(")}");
    if (startPos != std::string::npos && endPos != std::string::npos && endPos > startPos) {
        std::string jsonSourceArgs = replacement.substr(startPos + searchString.length(), endPos - startPos - searchString.length());
        std::vector<std::string> keys;
        std::string key;
        std::istringstream keyStream(jsonSourceArgs);
        while (std::getline(keyStream, key, ',')) {
            keys.push_back(trim(key));
        }

        // Traverse the JSON structure based on the keys
        json_t* current = root;
        for (const auto& key : keys) {
            if (json_is_object(current)) {
                current = json_object_get(current, key.c_str());
            } else if (json_is_array(current)) {
                if (key == "[]") {
                    size_t index = 0;
                    while (json_array_size(current) > index) {
                        json_t* arrayItem = json_array_get(current, index);
                        if (json_is_object(arrayItem)) {
                            current = arrayItem;
                            break;
                        }
                        ++index;
                    }
                } else {
                    size_t index = std::stoul(key);
                    if (index < json_array_size(current)) {
                        current = json_array_get(current, index);
                    } else {
                        // Handle invalid JSON array index
                        // printf("Invalid JSON array index: %s\n", key.c_str());
                        json_decref(root);
                        return placeholder;  // Return the original placeholder if JSON array index is invalid
                    }
                }
            } else {
                // Handle invalid JSON structure or key
                // printf("Invalid JSON structure or key: %s\n", key.c_str());
                json_decref(root);
                return placeholder;  // Return the original placeholder if JSON structure or key is invalid
            }
        }

        if (json_is_string(current)) {
            std::string url = json_string_value(current);
            // Replace the entire placeholder with the URL
            replacement.replace(startPos, endPos - startPos + searchString.length() + 2, url);
        }
    }

    json_decref(root);
    return replacement;
}


/**
 * @brief Modifies a list of commands based on specified conditions.
 *
 * This function modifies a list of commands according to specified conditions.
 *
 * @param commands The list of commands to modify.
 * @param entry The entry to apply modifications to.
 * @param toggle A boolean flag indicating whether totoggle modifications.
 * @param on A boolean flag indicating whether modifications are turned on.
 * @param usingJsonSource A boolean flag indicating JSON source usage.
 * @return The modified list of commands.
 */
std::vector<std::vector<std::string>> getModifyCommands(const std::vector<std::vector<std::string>>& commands, const std::string& entry, bool toggle = false, bool on = true, bool usingJsonSource = false) {
    std::vector<std::vector<std::string>> modifiedCommands;
    std::string jsonSource, replacement;
    
    bool addCommands = false;
    for (const auto& cmd : commands) {
        if (cmd.size() > 1) {
            if (toggle) {
                if (cmd[0] == "file_source_on") {
                    addCommands = true;
                    if (!on) {
                        addCommands = !addCommands;
                    }
                } else if (cmd[0] == "file_source_off") {
                    addCommands = false;
                    if (!on) {
                        addCommands = !addCommands;
                    }
                }
            }
            if (cmd[0] == "json_file") {
                jsonSource = preprocessPath(cmd[1]);
            } 
            if ((usingJsonSource) && (cmd[0] == "json_file_source")) {
                jsonSource = preprocessPath(cmd[1]);
            } 
            if ((usingJsonSource) && (cmd[0] == "json_source")) {
                jsonSource = removeQuotes(cmd[1]);
            }
        }
        if (!toggle or addCommands) {
            std::vector<std::string> modifiedCmd = cmd;
            for (auto& arg : modifiedCmd) {
                if (!toggle && (arg.find("{list_source}") != std::string::npos)) {
                    arg = replacePlaceholder(arg, "{list_source}", entry);
                } else if (!toggle && (arg.find("{file_source}") != std::string::npos)) {
                    arg = replacePlaceholder(arg, "{file_source}", entry);
                } else if (on && (arg.find("{file_source_on}") != std::string::npos)) {
                    arg = replacePlaceholder(arg, "{file_source_on}", entry);
                } else if (!on && (arg.find("{file_source_off}") != std::string::npos)) {
                    arg = replacePlaceholder(arg, "{file_source_off}", entry);
                } else if (arg.find("{file_name}") != std::string::npos) {
                    arg = replacePlaceholder(arg, "{file_name}", getNameFromPath(entry));
                } else if (arg.find("{folder_name}") != std::string::npos) {
                    arg = replacePlaceholder(arg, "{folder_name}", getParentDirNameFromPath(entry));
                } else if (arg.find("{json_file(") != std::string::npos) {
                    arg = replaceJsonSourcePlaceholder(arg, jsonSource);
                } else if (usingJsonSource && (arg.find("{json_source(") != std::string::npos)) {
                    std::string countStr = entry;
                    
                    //logMessage(std::string("count: ")+countStr);
                    //logMessage(std::string("pre arg: ") + arg);
                    arg = replacePlaceholder(arg, "*", entry);
                    //logMessage(std::string("post arg: ") + arg);

                    
                    size_t startPos = arg.find("{json_source(");
                    size_t endPos = arg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = replaceJsonSourcePlaceholder(arg.substr(startPos, endPos - startPos + 2), jsonSource, "variable");
                        //logMessage2("replacement: "+replacement);
                        //logMessage2("pre-arg: "+arg);
                        arg.replace(startPos, endPos - startPos + 2, replacement);
                        //logMessage2("post-arg: "+arg);
                    }
                } else if (usingJsonSource && (arg.find("{json_file_source(") != std::string::npos)) {
                    std::string countStr = entry;
                    
                    //logMessage(std::string("count: ")+countStr);
                    //logMessage(std::string("pre arg: ") + arg);
                    arg = replacePlaceholder(arg, "*", entry);
                    //logMessage(std::string("post arg: ") + arg);

                    
                    size_t startPos = arg.find("{json_file_source(");
                    size_t endPos = arg.find(")}");
                    if (endPos != std::string::npos && endPos > startPos) {
                        replacement = replaceJsonSourcePlaceholder(arg.substr(startPos, endPos - startPos + 2), jsonSource, "file");
                        //logMessage2("replacement: "+replacement);
                        //logMessage2("pre-arg: "+arg);
                        arg.replace(startPos, endPos - startPos + 2, replacement);
                        //logMessage2("post-arg: "+arg);
                    }
                }
            }
            modifiedCommands.emplace_back(modifiedCmd);
        }
    }
    return modifiedCommands;
}
