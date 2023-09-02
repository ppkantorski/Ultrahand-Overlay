#include <iostream>
#include <string>
#include <fstream>
#include <utility>

std::pair<std::string, int> readTextFromFile (const std::string& filePath) {
    // logMessage("Entered readTextFromFile");

    std::string lines;
    std::string currentLine;
    std::ifstream file(filePath);
    std::vector<std::string> words;
    int lineCount = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string word;
        std::string currentLine;

        while (lineStream >> word) {
            if (currentLine.empty()) {
                currentLine = word;
            } else if (currentLine.length() + 1 + word.length() <= 45) {
                currentLine += " " + word;
            } else {
                lines += currentLine + "\n";
                currentLine = word;
                lineCount++;
            }
        }

        if (!currentLine.empty()) {
            lines += currentLine + "\n";
            lineCount++;
        }
    }

    file.close();
    return std::make_pair(lines, lineCount);
}