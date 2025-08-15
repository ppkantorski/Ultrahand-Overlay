/********************************************************************************
 * File: path_funcs.cpp
 * Author: ppkantorski
 * Description:
 *   This source file implements the functions declared in path_funcs.hpp.
 *   These utility functions are focused on file and directory path manipulation
 *   for the Ultrahand Overlay project. Functionality includes creating directories,
 *   checking existence of files or paths, moving or copying files, and normalizing
 *   file system paths for cross-platform compatibility.
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

#include "path_funcs.hpp"


namespace ult {
    std::atomic<bool> abortFileOp(false);
    
    size_t COPY_BUFFER_SIZE = 65536/8; // Back to non-const as requested

    std::atomic<int> copyPercentage(-1);
    
    std::mutex logMutex2; // Mutex for thread-safe logging (defined here, declared as extern in header)

    // RAII wrapper for FILE* to ensure proper cleanup
    class FileGuard {
    private:
        FILE* file;
    public:
        FileGuard(FILE* f) : file(f) {}
        ~FileGuard() { if (file) fclose(file); }
        FILE* get() { return file; }
        FILE* release() { FILE* f = file; file = nullptr; return f; }
    };

    /**
     * @brief Checks if a path points to a directory.
     *
     * This function checks if the specified path points to a directory.
     *
     * @param path The path to check.
     * @return True if the path is a directory, false otherwise.
     */
    bool isDirectory(const std::string& path) {
        struct stat pathStat;
        return (stat(path.c_str(), &pathStat) == 0 && S_ISDIR(pathStat.st_mode));
    }
    
    /**
     * @brief Checks if a path points to a file.
     *
     * This function checks if the specified path points to a file.
     *
     * @param path The path to check.
     * @return True if the path is a file, false otherwise.
     */
    bool isFile(const std::string& path) {
        struct stat pathStat;
        return (stat(path.c_str(), &pathStat) == 0 && S_ISREG(pathStat.st_mode));
    }
    
    /**
     * @brief Checks if a path points to a file or directory.
     *
     * This function checks if the specified path points to either a file or a directory.
     *
     * @param path The path to check.
     * @return True if the path points to a file or directory, false otherwise.
     */
    bool isFileOrDirectory(const std::string& path) {
        struct stat pathStat;
        return (stat(path.c_str(), &pathStat) == 0);
    }
    
    // Helper function to check if directory is empty
    bool isDirectoryEmpty(const std::string& dirPath) {
        DIR* dir = opendir(dirPath.c_str());
        if (!dir) return false; // Can't open, assume not empty
        
        dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                closedir(dir);
                return false; // Found a file/directory
            }
        }
        closedir(dir);
        return true; // Empty directory
    }


    /**
     * @brief Creates a single directory if it doesn't exist.
     *
     * This function checks if the specified directory exists, and if not, it creates the directory.
     *
     * @param directoryPath The path of the directory to be created.
     */
    void createSingleDirectory(const std::string& directoryPath) {
        // mkdir is generally thread-safe, returns EEXIST if already exists
        if (mkdir(directoryPath.c_str(), 0777) != 0) {
            // Only log error if it's not EEXIST
            if (errno != EEXIST) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Failed to create directory: " + directoryPath + " - " + std::string(strerror(errno)));
                #endif
            }
        }
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
        const std::string volume = ROOT_PATH;
        std::string path = directoryPath;
    
        // Remove leading "sdmc:/" if present
        if (path.compare(0, volume.size(), volume) == 0) {
            path = path.substr(volume.size());
        }
    
        std::string parentPath = volume;
        size_t pos = 0, nextPos;
    
        // Iterate through the path and create each directory level if it doesn't exist
        while ((nextPos = path.find('/', pos)) != std::string::npos) {
            if (nextPos != pos) {
                parentPath.append(path, pos, nextPos - pos);
                parentPath += '/';
                createSingleDirectory(parentPath); // Create the parent directory
            }
            pos = nextPos + 1;
        }
    
        // Create the final directory level if it doesn't exist
        if (pos < path.size()) {
            parentPath += path.substr(pos);
            createSingleDirectory(parentPath); // Create the final directory
        }
    }
    
    #if !USING_FSTREAM_DIRECTIVE
    void writeLog(FILE* logFile, const std::string& line) {
        if (logFile) {
            std::lock_guard<std::mutex> lock(logMutex2);
            fprintf(logFile, "%s\n", line.c_str());
            fflush(logFile); // Ensure data is written immediately
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to write to log file.");
            #endif
        }
    }
    #else
    void writeLog(std::ofstream& logFile, const std::string& line) {
        if (logFile.is_open()) {
            std::lock_guard<std::mutex> lock(logMutex2);
            logFile << line << std::endl;
            logFile.flush(); // Ensure data is written immediately
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to write to log file.");
            #endif
        }
    }
    #endif
    
    /**
     * @brief Creates a text file with the specified content.
     *
     * This function creates a text file specified by `filePath` and writes the given `content` to the file.
     *
     * @param filePath The path of the text file to be created.
     * @param content The content to be written to the text file.
     */
    void createTextFile(const std::string& filePath, const std::string& content) {
        // Create parent directory first
        createDirectory(getParentDirFromPath(filePath));
        
    #if !USING_FSTREAM_DIRECTIVE
        FileGuard file(fopen(filePath.c_str(), "w"));
        if (file.get()) {
            fputs(content.c_str(), file.get());
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error: Unable to create file " + filePath);
            #endif
        }
    #else
        std::ofstream file(filePath);
        if (file.is_open()) {
            file << content;
            file.close();
        } else {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error: Unable to create file " + filePath);
            #endif
        }
    #endif
    }
    

    /**
     * @brief Deletes a file or directory.
     *
     * This function deletes the file or directory specified by `path`. It can delete both files and directories.
     *
     * @param path The path of the file or directory to be deleted.
     */
    void deleteFileOrDirectory(const std::string& pathToDelete, const std::string& logSource) {
        std::vector<std::string> stack;
        
        // Batch logging optimization - collect successful deletions instead of logging immediately
        std::vector<std::string> successfulDeletions;
        const bool needsLogging = !logSource.empty();
    
        const bool pathIsFile = pathToDelete.back() != '/';
    
        if (pathIsFile) {
            if (isFile(pathToDelete)) {
                if (remove(pathToDelete.c_str()) == 0) {
                    if (needsLogging) {
                        successfulDeletions.push_back(pathToDelete);
                    }
                } else {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Failed to delete file: " + pathToDelete);
                    #endif
                }
            }
            
            // Write log for single file deletion if needed
            if (needsLogging && !successfulDeletions.empty()) {
    #if !USING_FSTREAM_DIRECTIVE
                createDirectory(getParentDirFromPath(logSource));
                if (FILE* logFile = fopen(logSource.c_str(), "a")) {
                    writeLog(logFile, pathToDelete);
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
    #else
                createDirectory(getParentDirFromPath(logSource));
                std::ofstream logSourceFile(logSource, std::ios::app);
                if (logSourceFile.is_open()) {
                    writeLog(logSourceFile, pathToDelete);
                    logSourceFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
    #endif
            }
            return;
        }
    
        stack.push_back(pathToDelete);
        struct stat pathStat;
        std::string currentPath, filePath;
        bool isEmpty;
    
        while (!stack.empty()) {
            currentPath = stack.back();
    
            if (stat(currentPath.c_str(), &pathStat) != 0) {
                stack.pop_back();
                continue;
            }
    
            if (S_ISREG(pathStat.st_mode)) { // It's a file
                stack.pop_back(); // Remove from stack before deletion
                if (remove(currentPath.c_str()) == 0) {
                    // Batch logging - store successful deletion instead of writing immediately
                    if (needsLogging) {
                        successfulDeletions.push_back(currentPath);
                    }
                } else {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Failed to delete file: " + currentPath);
                    #endif
                }
            } else if (S_ISDIR(pathStat.st_mode)) { // It's a directory
                DIR* directory = opendir(currentPath.c_str());
                if (!directory) {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Failed to open directory: " + currentPath);
                    #endif
                    stack.pop_back();
                    continue;
                }
    
                dirent* entry;
                isEmpty = true;
                while ((entry = readdir(directory)) != nullptr) {
                    const std::string& fileName = entry->d_name;
                    if (fileName != "." && fileName != "..") {
                        filePath = currentPath + fileName;
                        stack.push_back(filePath + (filePath.back() == '/' ? "" : "/"));
                        isEmpty = false;
                    }
                }
                closedir(directory);
    
                if (isEmpty) {
                    stack.pop_back(); // Directory is now empty, safe to remove from stack
                    if (rmdir(currentPath.c_str()) == 0) {
                        // Note: Typically we don't log directory deletions, only files
                        // If you want to log directory deletions too, uncomment the lines below:
                        // if (needsLogging) {
                        //     successfulDeletions.push_back(currentPath);
                        // }
                    } else {
                        #if USING_LOGGING_DIRECTIVE
                        if (!disableLogging)
                            logMessage("Failed to delete directory: " + currentPath);
                        #endif
                    }
                }
            } else {
                stack.pop_back(); // Unknown file type, just remove from stack
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Unknown file type: " + currentPath);
                #endif
            }
        }
    
        // KEY OPTIMIZATION: Batch write all successful deletions to log file at the end
        // This eliminates the overhead of logging inside the hot loop
        if (needsLogging && !successfulDeletions.empty()) {
    #if !USING_FSTREAM_DIRECTIVE
            createDirectory(getParentDirFromPath(logSource));
            if (FILE* logFile = fopen(logSource.c_str(), "a")) {
                for (const auto& deletedPath : successfulDeletions) {
                    writeLog(logFile, deletedPath);
                }
                fclose(logFile);
            }
            #if USING_LOGGING_DIRECTIVE
            else {
                if (!disableLogging)
                    logMessage("Failed to open source log file: " + logSource);
            }
            #endif
    #else
            createDirectory(getParentDirFromPath(logSource));
            std::ofstream logSourceFile(logSource, std::ios::app);
            if (logSourceFile.is_open()) {
                for (const auto& deletedPath : successfulDeletions) {
                    writeLog(logSourceFile, deletedPath);
                }
                logSourceFile.close();
            }
            #if USING_LOGGING_DIRECTIVE
            else {
                if (!disableLogging)
                    logMessage("Failed to open source log file: " + logSource);
            }
            #endif
    #endif
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
    void deleteFileOrDirectoryByPattern(const std::string& pathPattern, const std::string& logSource) {
        //logMessage("pathPattern: "+pathPattern);
        std::vector<std::string> fileList = getFilesListByWildcards(pathPattern);
        
        for (auto& path : fileList) {
            //logMessage("path: "+path);
            deleteFileOrDirectory(path, logSource);
            path = "";
        }
    }
    
    void moveDirectory(const std::string& sourcePath, const std::string& destinationPath,
                       const std::string& logSource, const std::string& logDestination) {
        
        struct stat sourceInfo;
        if (stat(sourcePath.c_str(), &sourceInfo) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Source directory doesn't exist: " + sourcePath);
            #endif
            return;
        }
    
        if (mkdir(destinationPath.c_str(), 0777) != 0 && errno != EEXIST) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to create destination directory: " + destinationPath);
            #endif
            return;
        }
    
        // Batch logging optimization - collect successful operations instead of logging immediately
        std::vector<std::string> successfulSources, successfulDestinations;
        bool needsLogging = !logSource.empty() || !logDestination.empty();
    
        std::vector<std::pair<std::string, std::string>> stack;
        std::vector<std::string> directoriesToRemove;
        
        stack.push_back({sourcePath, destinationPath});
    
        // Variables moved outside the loop to avoid repeated allocation
        std::string name, fullPathSrc, fullPathDst;
        dirent* entry;
        DIR* dir;
        
        std::string currentSource, currentDestination;
        while (!stack.empty()) {
            std::tie(currentSource, currentDestination) = stack.back();
            stack.pop_back();
    
            dir = opendir(currentSource.c_str());
            if (!dir) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Failed to open source directory: " + currentSource);
                #endif
                continue;
            }
    
            while ((entry = readdir(dir)) != nullptr) {
                name = entry->d_name;
                if (name == "." || name == "..") continue;
    
                // Optimize string concatenation - avoid repeated substr operations
                fullPathSrc = currentSource;
                if (!fullPathSrc.empty() && fullPathSrc.back() == '/') {
                    fullPathSrc.pop_back();
                }
                fullPathSrc += "/" + name;
                
                fullPathDst = currentDestination;
                if (!fullPathDst.empty() && fullPathDst.back() == '/') {
                    fullPathDst.pop_back();
                }
                fullPathDst += "/" + name;
    
                if (entry->d_type == DT_DIR) {
                    if (mkdir(fullPathDst.c_str(), 0777) != 0 && errno != EEXIST) {
                        #if USING_LOGGING_DIRECTIVE
                        if (!disableLogging)
                            logMessage("Failed to create destination directory: " + fullPathDst);
                        #endif
                        continue;
                    }
                    stack.push_back({fullPathSrc, fullPathDst});
                    directoriesToRemove.push_back(fullPathSrc);

                    // Log each subdirectory that gets successfully created
                    if (needsLogging) {
                        successfulSources.push_back(fullPathSrc + "/");
                        successfulDestinations.push_back(fullPathDst + "/");
                    }
                } else {
                    remove(fullPathDst.c_str());
                    if (rename(fullPathSrc.c_str(), fullPathDst.c_str()) == 0) {
                        // Batch logging - store successful operations instead of writing immediately
                        if (needsLogging) {
                            successfulSources.push_back(fullPathSrc);
                            successfulDestinations.push_back(fullPathDst);
                        }
                    } else {
                        #if USING_LOGGING_DIRECTIVE
                        if (!disableLogging)
                            logMessage("Failed to move: " + fullPathSrc);
                        #endif
                    }
                }
            }
            closedir(dir);
        }
    
        // Clean up source directories in reverse order
        for (auto it = directoriesToRemove.rbegin(); it != directoriesToRemove.rend(); ++it) {
            if (rmdir(it->c_str()) != 0) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Failed to delete source directory: " + *it);
                #endif
            }
        }
    
        if (rmdir(sourcePath.c_str()) != 0) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Failed to delete source directory: " + sourcePath);
            #endif
        }
    
        // KEY OPTIMIZATION: Batch write all successful operations to log files at the end
        // This eliminates the overhead of logging inside the hot loop
        if (needsLogging && !successfulSources.empty()) {
    #if !USING_FSTREAM_DIRECTIVE
            if (!logSource.empty()) {
                createDirectory(getParentDirFromPath(logSource));
                if (FILE* logFile = fopen(logSource.c_str(), "a")) {
                    for (auto it = successfulSources.rbegin(); it != successfulSources.rend(); ++it) {
                        writeLog(logFile, *it);
                    }
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
            }
    
            if (!logDestination.empty()) {
                createDirectory(getParentDirFromPath(logDestination));
                if (FILE* logFile = fopen(logDestination.c_str(), "a")) {
                    for (auto it = successfulDestinations.rbegin(); it != successfulDestinations.rend(); ++it) {
                        writeLog(logFile, *it);
                    }
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open destination log file: " + logDestination);
                }
                #endif
            }
    #else
            if (!logSource.empty()) {
                createDirectory(getParentDirFromPath(logSource));
                std::ofstream logSourceFile(logSource, std::ios::app);
                if (logSourceFile.is_open()) {
                    for (auto it = successfulSources.rbegin(); it != successfulSources.rend(); ++it) {
                        writeLog(logSourceFile, *it);
                    }
                    logSourceFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
            }
    
            if (!logDestination.empty()) {
                createDirectory(getParentDirFromPath(logDestination));
                std::ofstream logDestFile(logDestination, std::ios::app);
                if (logDestFile.is_open()) {
                    for (auto it = successfulDestinations.rbegin(); it != successfulDestinations.rend(); ++it) {
                        writeLog(logDestFile, *it);
                    }
                    logDestFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open destination log file: " + logDestination);
                }
                #endif
            }
    #endif
        }
    }
    
    bool moveFile(const std::string& sourcePath,
                  const std::string& destinationPath,
                  const std::string& logSource,
                  const std::string& logDestination) {
        if (!isFileOrDirectory(sourcePath)) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Source file doesn't exist or is not a regular file: " + sourcePath);
            #endif
            return false;
        }
    
        std::string finalDestPath;
        bool moveSuccess = false;
    
        if (destinationPath.back() == '/') {
            // Destination is a directory - construct full destination path
            if (!isDirectory(destinationPath)) {
                createDirectory(destinationPath);
            }
            
            finalDestPath = destinationPath + getFileName(sourcePath);
            remove(finalDestPath.c_str());
            
            if (rename(sourcePath.c_str(), finalDestPath.c_str()) == 0) {
                moveSuccess = true;
            } else {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Failed to move file to directory: " + sourcePath);
                #endif
            }
        } else {
            // Destination is a file path - directly rename the file
            finalDestPath = destinationPath;
            remove(finalDestPath.c_str());
            createDirectory(getParentDirFromPath(finalDestPath));
            
            if (rename(sourcePath.c_str(), finalDestPath.c_str()) == 0) {
                moveSuccess = true;
            } else {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging) {
                    logMessage("Failed to move file: " + sourcePath + " -> " + finalDestPath);
                    logMessage("Error: " + std::string(strerror(errno)));
                }
                #endif
            }
        }
    
        // Only write to log files if the move was successful
        // This is the key optimization - logs are only opened when actually needed!
        if (moveSuccess) {
    #if !USING_FSTREAM_DIRECTIVE
            if (!logSource.empty()) {
                createDirectory(getParentDirFromPath(logSource));
                if (FILE* logFile = fopen(logSource.c_str(), "a")) {
                    writeLog(logFile, sourcePath);
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
            }
    
            if (!logDestination.empty()) {
                createDirectory(getParentDirFromPath(logDestination));
                if (FILE* logFile = fopen(logDestination.c_str(), "a")) {
                    writeLog(logFile, finalDestPath);
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open destination log file: " + logDestination);
                }
                #endif
            }
    #else
            if (!logSource.empty()) {
                createDirectory(getParentDirFromPath(logSource));
                std::ofstream logSourceFile(logSource, std::ios::app);
                if (logSourceFile.is_open()) {
                    writeLog(logSourceFile, sourcePath);
                    logSourceFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
            }
    
            if (!logDestination.empty()) {
                createDirectory(getParentDirFromPath(logDestination));
                std::ofstream logDestFile(logDestination, std::ios::app);
                if (logDestFile.is_open()) {
                    writeLog(logDestFile, finalDestPath);
                    logDestFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open destination log file: " + logDestination);
                }
                #endif
            }
    #endif
        }
    
        return moveSuccess;
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
    void moveFileOrDirectory(const std::string& sourcePath, const std::string& destinationPath,
        const std::string& logSource, const std::string& logDestination) {
        if (sourcePath.back() == '/' && destinationPath.back() == '/') {
            moveDirectory(sourcePath, destinationPath, logSource, logDestination);
        } else {
            moveFile(sourcePath, destinationPath, logSource, logDestination);
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
    void moveFilesOrDirectoriesByPattern(const std::string& sourcePathPattern, const std::string& destinationPath,
        const std::string& logSource, const std::string& logDestination) {
        
        std::vector<std::string> fileList = getFilesListByWildcards(sourcePathPattern);
        
        //std::string fileListAsString;
        //for (const std::string& filePath : fileList)
        //    fileListAsString += filePath + "\n";
        //logMessage("File List:\n" + fileListAsString);
        
        //logMessage("pre loop");
        std::string folderName, fixedDestinationPath;
        
        // Iterate through the file list
        for (std::string& sourceFileOrDirectory : fileList) {
            //logMessage("sourceFileOrDirectory: "+sourceFileOrDirectory);
            // if sourceFile is a file (Needs condition handling)
            if (!isDirectory(sourceFileOrDirectory)) {
                //logMessage("destinationPath: "+destinationPath);
                moveFileOrDirectory(sourceFileOrDirectory.c_str(), destinationPath.c_str(), logSource, logDestination);
            } else if (isDirectory(sourceFileOrDirectory)) {
                // if sourceFile is a directory (needs conditoin handling)
                folderName = getNameFromPath(sourceFileOrDirectory);
                fixedDestinationPath = destinationPath + folderName + "/";
                
                //logMessage("fixedDestinationPath: "+fixedDestinationPath);
                
                moveFileOrDirectory(sourceFileOrDirectory.c_str(), fixedDestinationPath.c_str(), logSource, logDestination);
            }
            sourceFileOrDirectory = "";
        }
        //logMessage("post loop");
    }
    
    /**
     * @brief Copies a single file from the source path to the destination path.
     *
     * This function copies a single file specified by `fromFile` to the location specified by `toFile`.
     *
     * @param fromFile The path of the source file to be copied.
     * @param toFile The path of the destination where the file will be copied.
     */
    void copySingleFile(const std::string& fromFile, const std::string& toFile, long long& totalBytesCopied, 
                        const long long totalSize, const std::string& logSource, const std::string& logDestination) {
        static constexpr size_t maxRetries = 10;
        const size_t bufferSize = COPY_BUFFER_SIZE;
        
        // Create destination directory once
        createDirectory(getParentDirFromPath(toFile));
        
        // Use heap allocation for the buffer to avoid stack overflow with large buffer sizes
        std::unique_ptr<char[]> buffer(new char[bufferSize]);
    
    #if !USING_FSTREAM_DIRECTIVE
        FILE* srcFile = nullptr;
        FILE* destFile = nullptr;
        
        // Retry loop for file opening
        for (size_t retryCount = 0; retryCount <= maxRetries; ++retryCount) {
            srcFile = fopen(fromFile.c_str(), "rb");
            if (!srcFile) {
                if (retryCount == maxRetries) {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Error: Failed to open source file after " + std::to_string(maxRetries) + " retries");
                    #endif
                    return;
                }
                continue;
            }
            
            destFile = fopen(toFile.c_str(), "wb");
            if (!destFile) {
                fclose(srcFile);
                if (retryCount == maxRetries) {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Error: Failed to open destination file after " + std::to_string(maxRetries) + " retries");
                    #endif
                    return;
                }
                continue;
            }
            
            break; // Both files opened successfully
        }
        
        // RAII cleanup
        FileGuard srcGuard(srcFile);
        FileGuard destGuard(destFile);
        
        // Main copy loop - optimized for performance
        size_t bytesRead;
        char* bufferPtr = buffer.get();
        size_t remainingBytes, written;
        while ((bytesRead = fread(bufferPtr, 1, bufferSize, srcFile)) > 0) {
            if (abortFileOp.load(std::memory_order_acquire)) {
                remove(toFile.c_str());
                copyPercentage.store(-1, std::memory_order_release);
                return;
            }
            
            // Write all bytes - handle partial writes
            char* writePtr = bufferPtr;
            remainingBytes = bytesRead;
            
            while (remainingBytes > 0) {
                written = fwrite(writePtr, 1, remainingBytes, destFile);
                if (written == 0) {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Error writing to destination file");
                    #endif
                    remove(toFile.c_str());
                    copyPercentage.store(-1, std::memory_order_release);
                    return;
                }
                writePtr += written;
                remainingBytes -= written;
            }
            
            totalBytesCopied += bytesRead;
            if (totalSize > 0) {
                copyPercentage.store(static_cast<int>(100 * totalBytesCopied / totalSize), std::memory_order_release);
            }
        }
        
        // Check for read errors
        if (ferror(srcFile)) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Error reading from source file");
            #endif
            remove(toFile.c_str());
            copyPercentage.store(-1, std::memory_order_release);
            return;
        }
    
    #else
        std::ifstream srcFile;
        std::ofstream destFile;
        
        // Retry loop for file opening
        for (size_t retryCount = 0; retryCount <= maxRetries; ++retryCount) {
            srcFile.open(fromFile, std::ios::binary);
            destFile.open(toFile, std::ios::binary);
            
            if (srcFile.is_open() && destFile.is_open()) {
                break;
            }
            
            srcFile.close();
            destFile.close();
            
            if (retryCount == maxRetries) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Error: Failed to open files after " + std::to_string(maxRetries) + " retries");
                #endif
                return;
            }
        }
        
        // Main copy loop
        char* bufferPtr = buffer.get();
        while (srcFile.read(bufferPtr, bufferSize) || srcFile.gcount() > 0) {
            if (abortFileOp.load(std::memory_order_acquire)) {
                srcFile.close();
                destFile.close();
                remove(toFile.c_str());
                copyPercentage.store(-1, std::memory_order_release);
                return;
            }
            
            std::streamsize bytesToWrite = srcFile.gcount();
            destFile.write(bufferPtr, bytesToWrite);
            
            if (!destFile.good()) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Error writing to destination file");
                #endif
                srcFile.close();
                destFile.close();
                remove(toFile.c_str());
                copyPercentage.store(-1, std::memory_order_release);
                return;
            }
            
            totalBytesCopied += bytesToWrite;
            if (totalSize > 0) {
                copyPercentage.store(static_cast<int>(100 * totalBytesCopied / totalSize), std::memory_order_release);
            }
        }
        
        srcFile.close();
        destFile.close();
    #endif
        
        // Only open and write to log files if they're needed - this is the key optimization!
        if (!logSource.empty()) {
            createDirectory(getParentDirFromPath(logSource));
            if (FILE* logFile = fopen(logSource.c_str(), "a")) {
                writeLog(logFile, fromFile);
                fclose(logFile);
            }
            #if USING_LOGGING_DIRECTIVE
            else {
                if (!disableLogging)
                    logMessage("Failed to open source log file: " + logSource);
            }
            #endif
        }
        
        if (!logDestination.empty()) {
            createDirectory(getParentDirFromPath(logDestination));
            if (FILE* logFile = fopen(logDestination.c_str(), "a")) {
                writeLog(logFile, toFile);
                fclose(logFile);
            }
            #if USING_LOGGING_DIRECTIVE
            else {
                if (!disableLogging)
                    logMessage("Failed to open destination log file: " + logDestination);
            }
            #endif
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
    void copyFileOrDirectory(const std::string& fromPath, const std::string& toPath, long long* totalBytesCopied, long long totalSize,
        const std::string& logSource, const std::string& logDestination) {
        bool isTopLevelCall = totalBytesCopied == nullptr;
        long long tempBytesCopied = 0;
    
        // Batch logging optimization - collect successful operations instead of logging immediately
        std::vector<std::string> successfulSources, successfulDestinations;
        bool needsLogging = !logSource.empty() || !logDestination.empty();
        bool filesCopied = false;
    
        if (isTopLevelCall) {
            totalSize = getTotalSize(fromPath);
            totalBytesCopied = &tempBytesCopied;
        }
    
        if (toPath.back() != '/') {
            // If toPath is a file, create its parent directory and copy the file
            createDirectory(getParentDirFromPath(toPath));
            copySingleFile(fromPath, toPath, *totalBytesCopied, totalSize, logSource, logDestination);
            return;
        }
    
        // Ensure the toPath directory exists
        createDirectory(toPath);
    
        std::vector<std::pair<std::string, std::string>> directories;
        directories.emplace_back(fromPath, toPath); // Use emplace_back for efficiency
    
        size_t currentDirectoryIndex = 0;
        
        // Pre-declare strings to avoid repeated allocations
        std::string filename, toFilePath, toDirPath, currentFromPath, currentToPath;
        std::string subFromPath, subToPath;
    
        struct stat fromStat;
    
        while (currentDirectoryIndex < directories.size()) {
            if (abortFileOp.load(std::memory_order_acquire)) {
                copyPercentage.store(-1, std::memory_order_release);
                return;
            }
            
            // Use structured binding more efficiently
            const auto& [currentFrom, currentTo] = directories[currentDirectoryIndex++];
            currentFromPath = currentFrom;
            currentToPath = currentTo;
    
            if (stat(currentFromPath.c_str(), &fromStat) != 0) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Failed to get stat of " + currentFromPath);
                #endif
                continue;
            }
    
            if (S_ISREG(fromStat.st_mode)) {
                // If it's a regular file, copy it to the toPath directory
                filename = getNameFromPath(currentFromPath);
                
                // More efficient path building
                toFilePath.assign(getParentDirFromPath(currentToPath));
                toFilePath += '/';
                toFilePath += filename;
                
                createDirectory(getParentDirFromPath(toFilePath)); // Ensure the parent directory exists
                copySingleFile(currentFromPath, toFilePath, *totalBytesCopied, totalSize, logSource, logDestination);
                
                // Mark that files were copied
                filesCopied = true;
    
                if (totalSize > 0) {
                    copyPercentage.store(static_cast<int>((*totalBytesCopied * 100) / totalSize), std::memory_order_release); // Update progress
                }
            } else if (S_ISDIR(fromStat.st_mode)) {
                // If it's a directory, iterate over its contents and add them to the vector for processing
                DIR* dir = opendir(currentFromPath.c_str());
                if (!dir) {
                    #if USING_LOGGING_DIRECTIVE
                    if (!disableLogging)
                        logMessage("Failed to open directory: " + currentFromPath);
                    #endif
                    continue;
                }
    
                bool hasContent = false;
                // Cache current path lengths for efficient string building
                dirent* entry;
                while ((entry = readdir(dir)) != nullptr) {
                    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
                    
                    hasContent = true;
                    
                    // More efficient path building
                    subFromPath.clear();
                    subFromPath.assign(currentFromPath);
                    subFromPath += '/';
                    subFromPath += entry->d_name;
                    
                    subToPath.clear();
                    subToPath.assign(currentToPath);
                    subToPath += '/';
                    subToPath += entry->d_name;
                    
                    directories.emplace_back(std::move(subFromPath), std::move(subToPath)); // Use move semantics
                }
                closedir(dir);
                
                // Log this directory if it has content and we need logging
                if (hasContent && needsLogging) {
                    successfulSources.push_back(currentFromPath + "/");
                    successfulDestinations.push_back(currentToPath + "/");
                }
            }
        }
    
        // Batch write all successful operations to log files at the end
        if (isTopLevelCall && needsLogging && filesCopied && !successfulSources.empty()) {
    #if !USING_FSTREAM_DIRECTIVE
            if (!logSource.empty()) {
                createDirectory(getParentDirFromPath(logSource));
                if (FILE* logFile = fopen(logSource.c_str(), "a")) {
                    for (auto it = successfulSources.rbegin(); it != successfulSources.rend(); ++it) {
                        writeLog(logFile, *it);
                    }
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
            }
    
            if (!logDestination.empty()) {
                createDirectory(getParentDirFromPath(logDestination));
                if (FILE* logFile = fopen(logDestination.c_str(), "a")) {
                    for (auto it = successfulDestinations.rbegin(); it != successfulDestinations.rend(); ++it) {
                        writeLog(logFile, *it);
                    }
                    fclose(logFile);
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open destination log file: " + logDestination);
                }
                #endif
            }
    #else
            if (!logSource.empty()) {
                createDirectory(getParentDirFromPath(logSource));
                std::ofstream logSourceFile(logSource, std::ios::app);
                if (logSourceFile.is_open()) {
                    for (auto it = successfulSources.rbegin(); it != successfulSources.rend(); ++it) {
                        writeLog(logSourceFile, *it);
                    }
                    logSourceFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open source log file: " + logSource);
                }
                #endif
            }
    
            if (!logDestination.empty()) {
                createDirectory(getParentDirFromPath(logDestination));
                std::ofstream logDestFile(logDestination, std::ios::app);
                if (logDestFile.is_open()) {
                    for (auto it = successfulDestinations.rbegin(); it != successfulDestinations.rend(); ++it) {
                        writeLog(logDestFile, *it);
                    }
                    logDestFile.close();
                }
                #if USING_LOGGING_DIRECTIVE
                else {
                    if (!disableLogging)
                        logMessage("Failed to open destination log file: " + logDestination);
                }
                #endif
            }
    #endif
        }
    
        if (isTopLevelCall) {
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
    void copyFileOrDirectoryByPattern(const std::string& sourcePathPattern, const std::string& toDirectory,
        const std::string& logSource, const std::string& logDestination) {
        std::vector<std::string> fileList = getFilesListByWildcards(sourcePathPattern);
        long long totalSize = 0;
        for (const std::string& path : fileList) {
            totalSize += getTotalSize(path);
        }
    
        long long totalBytesCopied = 0;
        for (std::string& sourcePath : fileList) {
            copyFileOrDirectory(sourcePath, toDirectory, &totalBytesCopied, totalSize, logSource, logDestination);
            sourcePath = "";
        }
        //copyPercentage.store(-1, std::memory_order_release);  // Reset after operation
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
        long long totalSize = 0;
        long long totalBytesCopied = 0;
        
        if (mode == "copy") {
            // Calculate total size for progress tracking
            for (const auto& path : fileList) {
                if (path != targetPath + path.substr(sourcePath.size())) {
                    totalSize += getTotalSize(path);
                }
            }
        }
        
        for (auto& path : fileList) {
            // Generate the corresponding path in the target directory by replacing the source path
            updatedPath = targetPath + path.substr(sourcePath.size());
            //logMessage("mirror-delete: "+path+" "+updatedPath);
            if (mode == "delete")
                deleteFileOrDirectory(updatedPath);
            else if (mode == "copy") {
                if (path != updatedPath)
                    copyFileOrDirectory(path, updatedPath, &totalBytesCopied, totalSize);
            }
            path = "";
        }
        //fileList.clear();
    }
    
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
    void createFlagFiles(const std::string& wildcardPattern, const std::string& outputDir) {
        // 1) Gather all matches from the wildcard pattern
        std::vector<std::string> allMatches = ult::getFilesListByWildcards(wildcardPattern);
        if (allMatches.empty()) {
            return; // No matches, nothing to do
        }
    
        // 2) Ensure the output directory exists
        createDirectory(outputDir);
    
        // 3) Generate empty .txt files for each matched path
        std::string outputPrefix = outputDir;
        if (!outputPrefix.empty() && outputPrefix.back() != '/')
            outputPrefix.push_back('/');
        
        std::string baseName, outFile;
        for (auto& fullPath : allMatches) {
            baseName = ult::getNameFromPath(fullPath);
            if (baseName.empty()) {
                fullPath = "";
                continue;
            }
    
            outFile = outputPrefix + baseName;
    
        #if !USING_FSTREAM_DIRECTIVE
            FileGuard fp(std::fopen(outFile.c_str(), "wb"));
            // File automatically closed by FileGuard destructor
        #else
            std::ofstream ofs(outFile, std::ios::binary | std::ios::trunc);
            ofs.close();
        #endif
            fullPath = "";
        }
    }
    
    /**
     * @brief Removes all files starting with "._" from a directory and its subdirectories.
     *
     * This function recursively scans the specified directory and removes all files
     * whose names start with "._" (commonly macOS metadata files). It processes
     * all subdirectories recursively.
     *
     * @param sourcePath The path of the directory to clean.
     */
    void dotCleanDirectory(const std::string& sourcePath) {
        if (!isDirectory(sourcePath)) {
            #if USING_LOGGING_DIRECTIVE
            if (!disableLogging)
                logMessage("Path is not a directory: " + sourcePath);
            #endif
            return;
        }
    
        std::vector<std::string> stack;
        stack.push_back(sourcePath);
        
        std::string currentPath, filePath;
        std::string fileName;
        struct stat pathStat;
        DIR* directory;
        dirent* entry;
    
        while (!stack.empty()) {
            currentPath = stack.back();
            stack.pop_back();
    
            directory = opendir(currentPath.c_str());
            if (!directory) {
                #if USING_LOGGING_DIRECTIVE
                if (!disableLogging)
                    logMessage("Failed to open directory: " + currentPath);
                #endif
                continue;
            }
    
            while ((entry = readdir(directory)) != nullptr) {
                fileName = entry->d_name;
                if (fileName == "." || fileName == "..") continue;
    
                // Build full file path
                filePath = currentPath;
                if (!filePath.empty() && filePath.back() != '/') {
                    filePath += '/';
                }
                filePath += fileName;
    
                if (stat(filePath.c_str(), &pathStat) != 0) {
                    continue; // Skip if can't stat
                }
    
                if (S_ISDIR(pathStat.st_mode)) {
                    // Add subdirectory to stack for processing
                    stack.push_back(filePath);
                } else if (S_ISREG(pathStat.st_mode)) {
                    // Check if filename starts with "._"
                    if (fileName.length() >= 2 && fileName.substr(0, 2) == "._") {
                        if (remove(filePath.c_str()) == 0) {
                            #if USING_LOGGING_DIRECTIVE
                            if (!disableLogging)
                                logMessage("Removed dot-underscore file: " + filePath);
                            #endif
                        } else {
                            #if USING_LOGGING_DIRECTIVE
                            if (!disableLogging)
                                logMessage("Failed to remove dot-underscore file: " + filePath);
                            #endif
                        }
                    }
                }
            }
            closedir(directory);
        }
    }
}