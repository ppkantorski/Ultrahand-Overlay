/********************************************************************************
 * File: list_funcs.hpp
 * Author: ppkantorski
 * Description:
 *   This header file contains function declarations and utility functions related
 *   to working with lists and vectors of strings. These functions are used in the
 *   Ultrahand Overlay project to perform various operations on lists, such as
 *   removing entries, filtering, and more.
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

/**
 * @brief Removes entries from a vector of strings that match a specified entry.
 *
 * This function removes entries from the `itemsList` vector of strings that match the `entry`.
 *
 * @param entry The entry to be compared against the elements in `itemsList`.
 * @param itemsList The vector of strings from which matching entries will be removed.
 */
void removeEntryFromList(const std::string& entry, std::vector<std::string>& itemsList) {
    itemsList.erase(std::remove_if(itemsList.begin(), itemsList.end(), [&](const std::string& item) {
        return item.compare(0, entry.length(), entry) == 0;
    }), itemsList.end());
}

/**
 * @brief Filters a list of strings based on a specified filter list.
 *
 * This function filters a list of strings (`itemsList`) by removing entries that match any
 * of the criteria specified in the `filterList`. It uses the `removeEntryFromList` function
 * to perform the removal.
 *
 * @param filterList The list of entries to filter by. Entries in `itemsList` matching any entry in this list will be removed.
 * @param itemsList The list of stringsto be filtered.
 */
void filterItemsList(const std::vector<std::string>& filterList, std::vector<std::string>& itemsList) {
    for (const auto& entry : filterList) {
        removeEntryFromList(entry, itemsList);
    }
}
