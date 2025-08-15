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

#ifndef PATH_FUNCS_HPP
#define PATH_FUNCS_HPP

#if !USING_FSTREAM_DIRECTIVE // For not using fstream (needs implementing)
#include <stdio.h>
#else
#include <fstream>
#endif

#include <memory>
#include <dirent.h>
#include <sys/stat.h>
#include "global_vars.hpp"
#include "string_funcs.hpp"
#include "get_funcs.hpp"
#include <queue>
#include <mutex>


namespace ult {
    extern std::atomic<bool> abortFileOp;
    
    extern size_t COPY_BUFFER_SIZE; // Made const for thread safety
    extern std::atomic<int> copyPercentage;
    
    // Mutex for thread-safe logging operations
    extern std::mutex logMutex;
    
    /**
     * @brief Checks if a path points to a directory.
     *
     * This function checks if the specified path points to a directory.
     *
     * @param path The path to check.
     * @return True if the path is a directory, false otherwise.
     */
    bool isDirectory(const std::string& path);
    
    
    
    /**
     * @brief Checks if a path points to a file.
     *
     * This function checks if the specified path points to a file.
     *
     * @param path The path to check.
     * @return True if the path is a file, false otherwise.
     */
    bool isFile(const std::string& path);
    
    
    /**
     * @brief Checks if a path points to a file or directory.
     *
     * This function checks if the specified path points to either a file or a directory.
     *
     * @param path The path to check.
     * @return True if the path points to a file or directory, false otherwise.
     */
    bool isFileOrDirectory(const std::string& path);
    
    
    bool isDirectoryEmpty(const std::string& dirPath);
    
    /**
     * @brief Creates a single directory if it doesn't exist.
     *
     * This function checks if the specified directory exists, and if not, it creates the directory.
     *
     * @param directoryPath The path of the directory to be created.
     */
    void createSingleDirectory(const std::string& directoryPath);
    
    
    /**
     * @brief Creates a directory and its parent directories if they don't exist.
     *
     * This function creates a directory specified by `directoryPath` and also creates any parent directories
     * if they don't exist. It handles nested directory creation.
     *
     * @param directoryPath The path of the directory to be created.
     */
    void createDirectory(const std::string& directoryPath);
    
    
    #if !USING_FSTREAM_DIRECTIVE
    void writeLog(FILE* logFile, const std::string& line);
    #else
    void writeLog(std::ofstream& logFile, const std::string& line);
    #endif
    
    /**
     * @brief Creates a text file with the specified content.
     *
     * This function creates a text file specified by `filePath` and writes the given `content` to the file.
     *
     * @param filePath The path of the text file to be created.
     * @param content The content to be written to the text file.
     */
    void createTextFile(const std::string& filePath, const std::string& content);
    
    
    
    
    /**
     * @brief Deletes a file or directory.
     *
     * This function deletes the file or directory specified by `path`. It can delete both files and directories.
     *
     * @param path The path of the file or directory to be deleted.
     */
    void deleteFileOrDirectory(const std::string& pathToDelete, const std::string& logSource = "");
    
    
    
    
    /**
     * @brief Deletes files or directories that match a specified pattern.
     *
     * This function deletes files or directories specified by `pathPattern` by matching against a pattern.
     * It identifies files or directories that match the pattern and deletes them.
     *
     * @param pathPattern The pattern used to match and delete files or directories.
     */
    void deleteFileOrDirectoryByPattern(const std::string& pathPattern, const std::string& logSource = "");
    
    
    void moveDirectory(const std::string& sourcePath, const std::string& destinationPath,
                       const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    bool moveFile(const std::string& sourcePath, const std::string& destinationPath,
                  const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    
    /**
     * @brief Moves a file or directory to a new destination.
     *
     * This function moves a file or directory from the `sourcePath` to the `destinationPath`. It can handle both
     * files and directories and ensures that the destination directory exists before moving.
     *
     * @param sourcePath The path of the source file or directory.
     * @param destinationPath The path of the destination where the file or directory will be moved.
     */
    void moveFileOrDirectory(const std::string& sourcePath, const std::string& destinationPath,
        const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    /**
     * @brief Moves files or directories matching a specified pattern to a destination directory.
     *
     * This function identifies files or directories that match the `sourcePathPattern` and moves them to the `destinationPath`.
     * It processes each matching entry in the source directory pattern and moves them to the specified destination.
     *
     * @param sourcePathPattern The pattern used to match files or directories to be moved.
     * @param destinationPath The destination directory where matching files or directories will be moved.
     */
    void moveFilesOrDirectoriesByPattern(const std::string& sourcePathPattern, const std::string& destinationPath,
        const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    /**
     * @brief Copies a single file from the source path to the destination path.
     *
     * This function copies a single file specified by `fromFile` to the location specified by `toFile`.
     *
     * @param fromFile The path of the source file to be copied.
     * @param toFile The path of the destination where the file will be copied.
     */
    void copySingleFile(const std::string& fromFile, const std::string& toFile, long long& totalBytesCopied, const long long totalSize,
                        const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    /**
     * Recursively calculates the total size of the given file or directory.
     * @param path The path to the file or directory.
     * @return The total size in bytes of all files within the directory or the size of a file.
     */
    long long getTotalSize(const std::string& path);
    
    
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
    void copyFileOrDirectory(const std::string& fromPath, const std::string& toPath, long long* totalBytesCopied = nullptr, long long totalSize = 0,
        const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    
    /**
     * @brief Copies files or directories matching a specified pattern to a destination directory.
     *
     * This function identifies files or directories that match the `sourcePathPattern` and copies them to the `toDirectory`.
     * It processes each matching entry in the source directory pattern and copies them to the specified destination.
     *
     * @param sourcePathPattern The pattern used to match files or directories to be copied.
     * @param toDirectory The destination directory where matching files or directories will be copied.
     */
    void copyFileOrDirectoryByPattern(const std::string& sourcePathPattern, const std::string& toDirectory,
        const std::string& logSource = "", const std::string& logDestination = "");
    
    
    
    
    
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
    void mirrorFiles(const std::string& sourcePath, const std::string targetPath, const std::string mode);
    

    /**
     * @brief For each match of the wildcard pattern, creates an empty text file
     *        named basename.txt inside the output directory.
     *        Uses FILE* if !USING_FSTREAM_DIRECTIVE is defined, otherwise uses std::ofstream.
     *
     * @param wildcardPattern A path with a wildcard, such as /some/path/[*].
     *                        Each match results in a file named after the basename.
     * @param outputDir       Directory where the output files will be written.
     *                        Created if it doesn't already exist.
     */
    void createFlagFiles(const std::string& wildcardPattern, const std::string& outputDir);


    /**
     * @brief Removes all files starting with "._" from a directory and its subdirectories.
     *
     * This function recursively scans the specified directory and removes all files
     * whose names start with "._" (commonly macOS metadata files). It processes
     * all subdirectories recursively.
     *
     * @param sourcePath The path of the directory to clean.
     */
    void dotCleanDirectory(const std::string& sourcePath);
}

#endif