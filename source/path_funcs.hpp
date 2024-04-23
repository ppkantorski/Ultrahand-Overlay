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
#include <sys/stat.h>
#include <dirent.h>

static std::atomic<bool> abortFileOp(false);

/**
 * @brief Creates a single directory if it doesn't exist.
 *
 * This function checks if the specified directory exists, and if not, it creates the directory.
 *
 * @param directoryPath The path of the directory to be created.
 */
void createSingleDirectory(const std::string& directoryPath) {
    struct stat st;
    if (stat(directoryPath.c_str(), &st) != 0)
        mkdir(directoryPath.c_str(), 0777);
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
    
    size_t pos = 0;
    std::string token;
    std::string parentPath = "sdmc:/";
    
    // Iterate through the path and create each directory level if it doesn't exist
    while ((pos = path.find('/')) != std::string::npos) {
        token = path.substr(0, pos);
        if (token.empty()) {
            // Skip empty tokens (e.g., consecutive slashes)
            path.erase(0, pos + 1);
            continue;
        }
        
        parentPath += token + "/";
        createSingleDirectory(parentPath); // Create the parent directory
        path.erase(0, pos + 1);
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
    FILE* file = std::fopen(filePath.c_str(), "w");
    if (file != nullptr) {
        std::fwrite(content.c_str(), 1, content.length(), file);
        std::fclose(file);
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
    struct stat pathStat;
    if (stat(pathToDelete.c_str(), &pathStat) == 0) {
        if (S_ISREG(pathStat.st_mode)) {
            if (std::remove(pathToDelete.c_str()) == 0) {
                // Deletion successful
            }
        } else if (S_ISDIR(pathStat.st_mode)) {
            // Delete all files in the directory
            DIR* directory = opendir(pathToDelete.c_str());
            if (directory != nullptr) {
                dirent* entry;
                while ((entry = readdir(directory)) != nullptr) {
                    const std::string& fileName = entry->d_name;
                    if (fileName != "." && fileName != "..") {
                        std::string filePath = pathToDelete + "/" + fileName;
                        deleteFileOrDirectory(filePath);
                    }
                }
                closedir(directory);
            }
            
            // Remove the directory itself
            if (rmdir(pathToDelete.c_str()) == 0) {
                // Deletion successful
            }
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
        //logMessage("path: "+path);
        deleteFileOrDirectory(path);
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
    struct stat sourceInfo, destinationInfo;
    
    //logMessage("sourcePath: "+sourcePath);
    //logMessage("destinationPath: "+destinationPath);
    
    if (stat(sourcePath.c_str(), &sourceInfo) == 0) {
        // Source file or directory exists
        
        // Check if the destination path exists
        bool destinationExists = (stat(getParentDirFromPath(destinationPath).c_str(), &destinationInfo) == 0);
        if (!destinationExists)
            createDirectory(getParentDirFromPath(destinationPath).c_str()); // Create the destination directory
        
        if (S_ISDIR(sourceInfo.st_mode)) {
            // Source path is a directory
            DIR* dir = opendir(sourcePath.c_str());
            if (!dir) {
                //logMessage("Failed to open source directory: "+sourcePath);
                //printf("Failed to open source directory: %s\n", sourcePath.c_str());
                return;
            }
            
            struct dirent* entry;
            
            std::string sourceFilePath, destinationFilePath;
            
            while ((entry = readdir(dir)) != NULL) {
                const std::string& fileOrFolderName = entry->d_name;
                
                if (fileOrFolderName != "." && fileOrFolderName != "..") {
                    sourceFilePath = sourcePath + fileOrFolderName;
                    destinationFilePath = destinationPath + fileOrFolderName;
                    
                    if (entry->d_type == DT_DIR) {
                        // Append trailing slash to destination path for folders
                        destinationFilePath += "/";
                        sourceFilePath += "/";
                    }
                    
                    moveFileOrDirectory(sourceFilePath, destinationFilePath);
                }
            }
            
            closedir(dir);
            
            // Delete the source directory
            deleteFileOrDirectory(sourcePath);
            
            return;
        } else {
            // Source path is a regular file
            std::string filename = getNameFromPath(sourcePath.c_str());
            
            std::string destinationFilePath = destinationPath;
            
            if (destinationPath[destinationPath.length() - 1] == '/') {
                destinationFilePath += filename;
            }
            
            
            //logMessage("sourcePath: "+sourcePath);
            //logMessage("destinationFilePath: "+destinationFilePath);
            
            deleteFileOrDirectory(destinationFilePath); // delete destiantion file for overwriting
            if (rename(sourcePath.c_str(), destinationFilePath.c_str()) == -1) {
                //printf("Failed to move file: %s\n", sourcePath.c_str());
                //logMessage("Failed to move file: "+sourcePath);
                return;
            }
            
            return;
        }
    }
    
    // Move unsuccessful or source file/directory doesn't exist
    return;
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
    
    std::string fileListAsString;
    for (const std::string& filePath : fileList)
        fileListAsString += filePath + "\n";
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


/**
 * @brief Copies a single file from the source path to the destination path.
 *
 * This function copies a single file specified by `fromFile` to the location specified by `toFile`.
 *
 * @param fromFile The path of the source file to be copied.
 * @param toFile The path of the destination where the file will be copied.
 */
void copySingleFile(const std::string& fromFile, const std::string& toFile) {
    FILE* srcFile = fopen(fromFile.c_str(), "rb");
    FILE* destFile = fopen(toFile.c_str(), "wb");
    if (srcFile && destFile) {
        const size_t bufferSize = 4096*2; // Increase buffer size to 128 KB
        char buffer[bufferSize];
        size_t bytesRead;
        
        while ((bytesRead = fread(buffer, 1, bufferSize, srcFile)) > 0) {
            fwrite(buffer, 1, bytesRead, destFile);
            if (abortFileOp.load(std::memory_order_acquire)) {
                break;
            }
        }

        
        fclose(srcFile);
        fclose(destFile);
        if (abortFileOp.load(std::memory_order_acquire)) {
            deleteFileOrDirectory(toFile);
            abortFileOp.store(false, std::memory_order_release);
        }

    } else {
        // Error opening files or performing copy action.
        // Handle the error accordingly.
    }
}

/**
 * @brief Copies a file or directory from the source path to the destination path.
 *
 * This function copies a file or directory specified by `fromFileOrDirectory` to the location specified by `toFileOrDirectory`.
 * If the source is a regular file, it copies the file to the destination. If the source is a directory, it recursively copies
 * the entire directory and its contents to the destination.
 *
 * @param fromFileOrDirectory The path of the source file or directory to be copied.
 * @param toFileOrDirectory The path of the destination where the file or directory will be copied.
 */
void copyFileOrDirectory(const std::string& fromFileOrDirectory, const std::string& toFileOrDirectory) {
    struct stat fromFileOrDirectoryInfo;
    if (stat(fromFileOrDirectory.c_str(), &fromFileOrDirectoryInfo) == 0) {
        if (S_ISREG(fromFileOrDirectoryInfo.st_mode)) {
            // Source is a regular file
            std::string fromFile = fromFileOrDirectory;
            
            struct stat toFileOrDirectoryInfo;
            
            if (stat(toFileOrDirectory.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISDIR(toFileOrDirectoryInfo.st_mode)) {
                // Destination is a directory
                std::string toDirectory = toFileOrDirectory;
                std::string fileName = fromFile.substr(fromFile.find_last_of('/') + 1);
                std::string toFilePath = toDirectory + fileName;
                
                // Create the destination directory if it doesn't exist
                createDirectory(toDirectory);
                
                // Check if the destination file exists and remove it
                if (stat(toFilePath.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISREG(toFileOrDirectoryInfo.st_mode))
                    std::remove(toFilePath.c_str());
                
                copySingleFile(fromFile, toFilePath);
            } else {
                std::string toFile = toFileOrDirectory;
                // Destination is a file or doesn't exist
                std::string toDirectory = toFile.substr(0, toFile.find_last_of('/'));
                
                // Create the destination directory if it doesn't exist
                createDirectory(toDirectory);
                
                // Destination is a file or doesn't exist
                // Check if the destination file exists and remove it
                if (stat(toFile.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISREG(toFileOrDirectoryInfo.st_mode))
                    std::remove(toFile.c_str());
                
                copySingleFile(fromFile, toFile);
            }
        } else if (S_ISDIR(fromFileOrDirectoryInfo.st_mode)) {
            // Source is a directory
            std::string fromDirectory = fromFileOrDirectory;
            //logMessage("fromDirectory: "+fromDirectory);
            
            struct stat toFileOrDirectoryInfo;
            if (stat(toFileOrDirectory.c_str(), &toFileOrDirectoryInfo) == 0 && S_ISDIR(toFileOrDirectoryInfo.st_mode)) {
                // Destination is a directory
                std::string toDirectory = toFileOrDirectory;
                std::string dirName = getNameFromPath(fromDirectory);
                if (dirName != "") {
                    std::string toDirPath = toDirectory + dirName +"/";
                    //logMessage("toDirectory: "+toDirectory);
                    //logMessage("dirName: "+dirName);
                    //logMessage("toDirPath: "+toDirPath);
                    
                    // Create the destination directory
                    createDirectory(toDirPath);
                    //mkdir(toDirPath.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
                    
                    // Open the source directory
                    DIR* dir = opendir(fromDirectory.c_str());
                    if (dir != nullptr) {
                        dirent* entry;
                        std::string fromFilePath, subFolderPath;
                        
                        while ((entry = readdir(dir)) != nullptr) {
                            const std::string& fileOrFolderName = entry->d_name;
                            
                            // handle cade for files
                            if (fileOrFolderName != "." && fileOrFolderName != "..") {
                                fromFilePath = fromDirectory + fileOrFolderName;
                                copyFileOrDirectory(fromFilePath, toDirPath);
                            }
                            // handle case for subfolders within the from file path
                            if (entry->d_type == DT_DIR && fileOrFolderName != "." && fileOrFolderName != "..") {
                                subFolderPath = fromDirectory + fileOrFolderName + "/";
                                copyFileOrDirectory(subFolderPath, toDirPath);
                            }
                            
                        }
                        closedir(dir);
                    }
                }
            }
        }
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
    
    for (const std::string& sourcePath : fileList) {
        //logMessage("sourcePath: "+sourcePath);
        //logMessage("toDirectory: "+toDirectory);
        if (sourcePath != toDirectory)
            copyFileOrDirectory(sourcePath, toDirectory);
    }
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
