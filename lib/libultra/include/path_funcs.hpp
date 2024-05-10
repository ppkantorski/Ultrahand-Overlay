/********************************************************************************
 * File: path_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains function declarations and utility functions related
 *   to file and directory path manipulation. These functions are used in the
 *   Ultrahand Overlay project to handle file operations, such as creating
 *   directories, moving files, and more.
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
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include <queue>

static std::atomic<bool> abortFileOp(false);

size_t COPY_BUFFER_SIZE = 4096*4; // Increase buffer size to 128 KB


/**
 * @brief Creates a single directory if it doesn't exist.
 *
 * This function checks if the specified directory exists, and if not, it creates the directory.
 *
 * @param directoryPath The path of the directory to be created.
 */
void createSingleDirectory(const std::string& directoryPath) {
    //if (!isDirectory(directoryPath)) {
    mkdir(directoryPath.c_str(), 0777); // Use mode 0777 to allow wide access
    //}
}


/**
 * @brief Creates a directory and its parent directories if they don't exist.
 *
 * This function creates a directory specified by `directoryPath` and also creates any parent directories
 * if they don't exist. It handles nested directory creation.
 *
 * @param directoryPath The path of the directory to be created.
 */
void createDirectory(const std::string& directoryPath) {
    std::string path = directoryPath;
    
    // Remove leading "sdmc:/" if present
    if (path.substr(0, 6) == "sdmc:/")
        path = path.substr(6);
    
    std::string parentPath = "sdmc:/";
    size_t pos = path.find('/');
    
    // Iterate through the path and create each directory level if it doesn't exist
    while (pos != std::string::npos) {
        std::string token = path.substr(0, pos);
        if (!token.empty()) {
            parentPath += token + "/";
            createSingleDirectory(parentPath); // Create the parent directory
        }
        path.erase(0, pos + 1);
        pos = path.find('/');
    }
    
    // Create the final directory level if it doesn't exist
    if (!path.empty()) {
        parentPath += path;
        createSingleDirectory(parentPath); // Create the final directory
    }
}




/**
 * @brief Creates a text file with the specified content.
 *
 * This function creates a text file specified by `filePath` and writes the given `content` to the file.
 *
 * @param filePath The path of the text file to be created.
 * @param content The content to be written to the text file.
 */
void createTextFile(const std::string& filePath, const std::string& content) {
    std::ofstream file(filePath);
    if (file.is_open()) {
        file << content;
        file.close();
    }
}




/**
 * @brief Deletes a file or directory.
 *
 * This function deletes the file or directory specified by `path`. It can delete both files and directories.
 *
 * @param path The path of the file or directory to be deleted.
 */
void deleteFileOrDirectory(const std::string& pathToDelete) {
    std::vector<std::string> stack;
    stack.push_back(pathToDelete + (pathToDelete.back() == '/' ? "" : "/")); // Normalize the path
    struct stat pathStat;
    std::string currentPath;
    bool isEmpty;

    while (!stack.empty()) {
        currentPath = stack.back();
        
        if (stat(currentPath.c_str(), &pathStat) != 0) {
            logMessage("Error accessing path: " + currentPath);
            stack.pop_back();
            continue;
        }

        if (S_ISREG(pathStat.st_mode)) { // It's a file
            stack.pop_back(); // Remove from stack before deletion
            if (remove(currentPath.c_str()) == 0) {
                //logMessage("File deleted: " + currentPath);
            } else {
                logMessage("Failed to delete file: " + currentPath);
            }
        } else if (S_ISDIR(pathStat.st_mode)) { // It's a directory
            DIR* directory = opendir(currentPath.c_str());
            if (!directory) {
                logMessage("Failed to open directory: " + currentPath);
                stack.pop_back();
                continue;
            }

            dirent* entry;
            isEmpty = true;
            while ((entry = readdir(directory)) != nullptr) {
                const std::string& fileName = entry->d_name;
                if (fileName != "." && fileName != "..") {
                    std::string filePath = currentPath + fileName;
                    stack.push_back(filePath + (filePath.back() == '/' ? "" : "/"));
                    isEmpty = false;
                }
            }
            closedir(directory);

            if (isEmpty) {
                stack.pop_back(); // Directory is now empty, safe to remove from stack
                if (rmdir(currentPath.c_str()) == 0) {
                    //logMessage("Directory deleted: " + currentPath);
                } else {
                    logMessage("Failed to delete directory: " + currentPath);
                }
            }
        } else {
            stack.pop_back(); // Unknown file type, just remove from stack
            logMessage("Unknown file type: " + currentPath);
        }
    }
}




/**
 * @brief Deletes files or directories that match a specified pattern.
 *
 * This function deletes files or directories specified by `pathPattern` by matching against a pattern.
 * It identifies files or directories that match the pattern and deletes them.
 *
 * @param pathPattern The pattern used to match and delete files or directories.
 */
void deleteFileOrDirectoryByPattern(const std::string& pathPattern) {
    //logMessage("pathPattern: "+pathPattern);
    std::vector<std::string> fileList = getFilesListByWildcards(pathPattern);
    
    for (const auto& path : fileList) {
        logMessage("path: "+path);
        deleteFileOrDirectory(path);
    }
}



void moveDirectory(const std::string& sourcePath, const std::string& destinationPath) {
    // Check if source directory exists
    struct stat sourceInfo;
    if (stat(sourcePath.c_str(), &sourceInfo) != 0) {
        logMessage("Source directory doesn't exist: " + sourcePath);
        return;
    }

    // Ensure destination directory exists
    if (mkdir(destinationPath.c_str(), 0777) != 0 && errno != EEXIST) {
        logMessage("Failed to create destination directory: " + destinationPath);
        return;
    }

    DIR* dir = opendir(sourcePath.c_str());
    if (!dir) {
        logMessage("Failed to open source directory: " + sourcePath);
        return;
    }

    //struct stat fileInfo;

    //std::unique_ptr<DIR, DirCloser> dirHolder(dir);
    dirent* entry;

    std::string name, fullPathSrc, fullPathDst;

    while ((entry = readdir(dir)) != nullptr) {
        name = entry->d_name;
        if (name == "." || name == "..") continue;

        fullPathSrc = sourcePath + '/' + name;
        fullPathDst = destinationPath + '/' + name;

        // Check if file already exists in destination
        //f (stat(fullPathDst.c_str(), &fileInfo) == 0) {
        //   logMessage("Overwriting destination: " + fullPathDst);
        //   remove(fullPathDst.c_str());
        //

        remove(fullPathDst.c_str()); // remove file if exists

        // Move file or directory
        if (rename(fullPathSrc.c_str(), fullPathDst.c_str()) != 0) {
            logMessage("Failed to move: " + fullPathSrc);
        }
    }
    closedir(dir);

    // Delete the source directory
    if (rmdir(sourcePath.c_str()) != 0) {
        logMessage("Failed to delete source directory: " + sourcePath);
    }
}




void moveFile(const std::string& sourcePath, const std::string& destinationPath) {
    if (!isFileOrDirectory(sourcePath)) {
        logMessage("Source file doesn't exist or is not a regular file: " + sourcePath);
        return;
    }

    if (destinationPath.back() == '/') {
        if (!isDirectory(destinationPath))
            createDirectory(destinationPath);
        // Destination is a directory, construct full destination path
        std::string destFile = destinationPath + getFileName(sourcePath);
        remove(destFile.c_str());
        if (rename(sourcePath.c_str(), destFile.c_str()) != 0) {
            logMessage("Failed to move file to directory: " + sourcePath);
        }
    } else {
        // Destination is a file path, directly rename the file
        remove(destinationPath.c_str());
        if (rename(sourcePath.c_str(), destinationPath.c_str()) != 0) {
            logMessage("Failed to move file: " + sourcePath);
        }
    }
}



/**
 * @brief Moves a file or directory to a new destination.
 *
 * This function moves a file or directory from the `sourcePath` to the `destinationPath`. It can handle both
 * files and directories and ensures that the destination directory exists before moving.
 *
 * @param sourcePath The path of the source file or directory.
 * @param destinationPath The path of the destination where the file or directory will be moved.
 */
void moveFileOrDirectory(const std::string& sourcePath, const std::string& destinationPath) {
    if (sourcePath.back() == '/' && destinationPath.back() == '/') {
        moveDirectory(sourcePath, destinationPath);
    } else {
        moveFile(sourcePath, destinationPath);
    }
}



/**
 * @brief Moves files or directories matching a specified pattern to a destination directory.
 *
 * This function identifies files or directories that match the `sourcePathPattern` and moves them to the `destinationPath`.
 * It processes each matching entry in the source directory pattern and moves them to the specified destination.
 *
 * @param sourcePathPattern The pattern used to match files or directories to be moved.
 * @param destinationPath The destination directory where matching files or directories will be moved.
 */
void moveFilesOrDirectoriesByPattern(const std::string& sourcePathPattern, const std::string& destinationPath) {
    std::vector<std::string> fileList = getFilesListByWildcards(sourcePathPattern);
    
    //std::string fileListAsString;
    //for (const std::string& filePath : fileList)
    //    fileListAsString += filePath + "\n";
    //logMessage("File List:\n" + fileListAsString);
    
    //logMessage("pre loop");
    std::string folderName, fixedDestinationPath;
    
    // Iterate through the file list
    for (const std::string& sourceFileOrDirectory : fileList) {
        //logMessage("sourceFileOrDirectory: "+sourceFileOrDirectory);
        // if sourceFile is a file (Needs condition handling)
        if (!isDirectory(sourceFileOrDirectory)) {
            //logMessage("destinationPath: "+destinationPath);
            moveFileOrDirectory(sourceFileOrDirectory.c_str(), destinationPath.c_str());
        } else if (isDirectory(sourceFileOrDirectory)) {
            // if sourceFile is a directory (needs conditoin handling)
            folderName = getNameFromPath(sourceFileOrDirectory);
            fixedDestinationPath = destinationPath + folderName + "/";
            
            //logMessage("fixedDestinationPath: "+fixedDestinationPath);
            
            moveFileOrDirectory(sourceFileOrDirectory.c_str(), fixedDestinationPath.c_str());
        }
        
    }
    //logMessage("post loop");
}

static std::atomic<int> copyPercentage(-1);

/**
 * @brief Copies a single file from the source path to the destination path.
 *
 * This function copies a single file specified by `fromFile` to the location specified by `toFile`.
 *
 * @param fromFile The path of the source file to be copied.
 * @param toFile The path of the destination where the file will be copied.
 */
void copySingleFile(const std::string& fromFile, const std::string& toFile, long long& totalBytesCopied, const long long totalSize) {
    std::ifstream srcFile(fromFile, std::ios::binary);
    std::ofstream destFile(toFile, std::ios::binary);
    //const size_t COPY_BUFFER_SIZE = 4096;
    char buffer[COPY_BUFFER_SIZE];
    
    if (!srcFile || !destFile) {
        logMessage("Error opening files for copying.");
        return;
    }

    while (srcFile.read(buffer, COPY_BUFFER_SIZE)) {
        if (abortFileOp.load(std::memory_order_acquire)) {
            destFile.close();
            srcFile.close();
            remove(toFile.c_str());
            copyPercentage.store(-1, std::memory_order_release);
            return;
        }
        destFile.write(buffer, srcFile.gcount());
        totalBytesCopied += srcFile.gcount();
        copyPercentage.store(static_cast<int>(100 * totalBytesCopied / totalSize), std::memory_order_release);
    }

    // Write the remaining bytes
    if (srcFile.gcount() > 0) {
        destFile.write(buffer, srcFile.gcount());
        totalBytesCopied += srcFile.gcount();
        copyPercentage.store(static_cast<int>(100 * totalBytesCopied / totalSize), std::memory_order_release);
    }
}


/**
 * Recursively calculates the total size of the given file or directory.
 * @param path The path to the file or directory.
 * @return The total size in bytes of all files within the directory or the size of a file.
 */
long long getTotalSize(const std::string& path) {
    struct stat statbuf;
    if (lstat(path.c_str(), &statbuf) != 0) {
        return 0; // Cannot stat file
    }

    if (S_ISREG(statbuf.st_mode)) {
        return statbuf.st_size;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        long long totalSize = 0;
        std::queue<std::string> directories;
        directories.push(path);
        std::string currentPath, newPath;

        while (!directories.empty()) {
            currentPath = directories.front();
            directories.pop();

            DIR* dir = opendir(currentPath.c_str());
            if (!dir) {
                continue; // Cannot open directory, skip it
            }

            dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                    continue; // Skip "." and ".."
                }
                newPath = currentPath + "/" + entry->d_name;

                if (lstat(newPath.c_str(), &statbuf) != 0) {
                    continue; // Cannot stat file, skip it
                }

                if (S_ISREG(statbuf.st_mode)) {
                    totalSize += statbuf.st_size;
                } else if (S_ISDIR(statbuf.st_mode)) {
                    directories.push(newPath); // Push subdirectory onto queue for processing
                }
            }
            closedir(dir);
        }

        return totalSize;
    }

    return 0; // Non-file/directory entries
}


/**
 * @brief Copies a file or directory from the source path to the destination path.
 *
 * This function copies a file or directory specified by `fromFileOrDirectory` to the location specified by `toFileOrDirectory`.
 * If the source is a regular file, it copies the file to the destination. If the source is a directory, it recursively copies
 * the entire directory and its contents to the destination.
 *
 * @param fromPath The path of the source file or directory to be copied.
 * @param toPath The path of the destination where the file or directory will be copied.
 */
void copyFileOrDirectory(const std::string& fromPath, const std::string& toPath, long long* totalBytesCopied = nullptr, long long totalSize = 0) {
    bool isTopLevelCall = totalBytesCopied == nullptr;
    long long tempBytesCopied;

    if (isTopLevelCall) {
        totalSize = getTotalSize(fromPath);
        if (totalBytesCopied)
            *totalBytesCopied = 0;
    }

    if (toPath.back() != '/') {
        // If toPath is a file, create its parent directory and copy the file
        createDirectory(getParentDirFromPath(toPath));
        tempBytesCopied = 0;
        copySingleFile(fromPath, toPath, tempBytesCopied, totalSize);
        return;
    }

    // Ensure the toPath directory exists
    createDirectory(toPath);

    std::vector<std::pair<std::string, std::string>> directories;
    directories.push_back({fromPath, toPath}); // Add initial paths to the vector

    size_t currentDirectoryIndex = 0;
    std::string filename, toFilePath, toDirPath, currentFromPath, currentToPath;
    

    struct stat fromStat;

    while (currentDirectoryIndex < directories.size()) {
        std::tie(currentFromPath, currentToPath) = directories[currentDirectoryIndex++]; // Get paths from the vector

        if (stat(currentFromPath.c_str(), &fromStat) != 0) {
            logMessage("Failed to get stat of " + currentFromPath);
            continue;
        }

        if (S_ISREG(fromStat.st_mode)) {
            // If it's a regular file, copy it to the toPath directory
            filename = getNameFromPath(currentFromPath);
            toFilePath = currentToPath + "/" + filename;
            createDirectory(getParentDirFromPath(toFilePath)); // Ensure the parent directory exists
            tempBytesCopied = totalBytesCopied ? *totalBytesCopied : 0;
            copySingleFile(currentFromPath, toFilePath, tempBytesCopied, totalSize);

            if (totalBytesCopied) {
                *totalBytesCopied = tempBytesCopied; // Update total bytes copied
                if (totalSize > 0) {
                    copyPercentage.store(static_cast<int>((tempBytesCopied * 100) / totalSize), std::memory_order_release); // Update progress
                }
            }
        } else if (S_ISDIR(fromStat.st_mode)) {
            // If it's a directory, iterate over its contents and add them to the vector for processing
            DIR* dir = opendir(currentFromPath.c_str());
            if (!dir) {
                logMessage("Failed to open directory: " + currentFromPath);
                continue;
            }

            dirent* entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                std::string subFromPath = currentFromPath + "/" + entry->d_name;
                std::string subToPath = currentToPath + "/" + entry->d_name;
                directories.push_back({subFromPath, subToPath}); // Add subdirectory to the vector for processing
            }
            closedir(dir);
        }
    }

    if (isTopLevelCall && totalBytesCopied) {
        copyPercentage.store(100, std::memory_order_release); // Set progress to 100% on completion of top-level call
    }
}




/**
 * @brief Copies files or directories matching a specified pattern to a destination directory.
 *
 * This function identifies files or directories that match the `sourcePathPattern` and copies them to the `toDirectory`.
 * It processes each matching entry in the source directory pattern and copies them to the specified destination.
 *
 * @param sourcePathPattern The pattern used to match files or directories to be copied.
 * @param toDirectory The destination directory where matching files or directories will be copied.
 */
void copyFileOrDirectoryByPattern(const std::string& sourcePathPattern, const std::string& toDirectory) {
    std::vector<std::string> fileList = getFilesListByWildcards(sourcePathPattern);
    long long totalSize = 0;
    for (const std::string& path : fileList) {
        totalSize += getTotalSize(path);
    }

    long long totalBytesCopied = 0;
    for (const std::string& sourcePath : fileList) {
        copyFileOrDirectory(sourcePath, toDirectory, &totalBytesCopied, totalSize);
    }
    copyPercentage.store(-1, std::memory_order_release);  // Reset after operation
}





/**
 * @brief Mirrors the deletion of files from a source directory to a target directory.
 *
 * This function mirrors the deletion of files from a `sourcePath` directory to a `targetPath` directory.
 * It deletes corresponding files in the `targetPath` that match the source directory structure.
 *
 * @param sourcePath The path of the source directory.
 * @param targetPath The path of the target directory where files will be mirrored and deleted.
 *                   Default is "sdmc:/". You can specify a different target path if needed.
 */
void mirrorFiles(const std::string& sourcePath, const std::string targetPath, const std::string mode) {
    std::vector<std::string> fileList = getFilesListFromDirectory(sourcePath);
    std::string updatedPath;
    for (const auto& path : fileList) {
        // Generate the corresponding path in the target directory by replacing the source path
        updatedPath = targetPath + path.substr(sourcePath.size());
        //logMessage("mirror-delete: "+path+" "+updatedPath);
        if (mode == "delete")
            deleteFileOrDirectory(updatedPath);
        else if (mode == "copy") {
            if (path != updatedPath)
                copyFileOrDirectory(path, updatedPath);
        }
    }
    //fileList.clear();
}

/**
 * @brief Recursively copies files and directories from the source directory to a target directory, mirroring the structure.
 *
 * This function recursively copies files and directories from the `sourcePath` to the `targetPath`, preserving the directory structure.
 * It identifies the files and directories in the source directory and creates equivalent paths in the target directory.
 *
 * @param sourcePath The source directory from which files and directories will be copied.
 * @param targetPath The target directory where the mirrored structure will be created (default is "sdmc:/").
 */
//void mirrorCopyFiles(const std::string& sourcePath, const std::string& targetPath="sdmc:/") {
//    std::vector<std::string> fileList = getFilesListFromDirectory(sourcePath);
//    
//    for (const auto& path : fileList) {
//        // Generate the corresponding path in the target directory by replacing the source path
//        std::string updatedPath = targetPath + path.substr(sourcePath.size());
//        if (path != updatedPath){
//            //logMessage("mirror-copy: "+path+" "+updatedPath);
//            copyFileOrDirectory(path, updatedPath);
//        }
//    }
//}

/**
 * @brief Ensures that a directory exists by creating it if it doesn't.
 *
 * This function checks if the specified directory path exists. If the directory does not exist, it creates it.
 *
 * @param path The path of the directory to ensure its existence.
 * @return True if the directory exists or was successfully created, false otherwise.
 */
bool ensureDirectoryExists(const std::string& path) {
    if (isDirectory(path))
        return true;
    else {
        createDirectory(path);
        if (isDirectory(path))
            return true;
    }
    
    //logMessage(std::string("Failed to create directory: ") + path);
    return false;
}
