#pragma once

#include <windows.h>
#include <stdexcept>
#include <string>

/// <summary>Creates all directories down to the specified path</summary>
/// <param name="directory">Directory that will be created recursively</param>
/// <remarks>
///   The provided directory must not be terminated with a path separator.
/// </remarks>
void createDirectoryRecursively(const std::wstring& directory) {
    static const std::wstring separators(L"\\/");

    // If the specified directory name doesn't exist, do our thing
    DWORD fileAttributes = ::GetFileAttributesW(directory.c_str());
    if (fileAttributes == INVALID_FILE_ATTRIBUTES) {

        // Recursively do it all again for the parent directory, if any
        std::size_t slashIndex = directory.find_last_of(separators);
        if (slashIndex != std::wstring::npos) {
            createDirectoryRecursively(directory.substr(0, slashIndex));
        }

        // Create the last directory on the path (the recursive calls will have taken
        // care of the parent directories by now)
        BOOL result = ::CreateDirectoryW(directory.c_str(), nullptr);
		if (result == FALSE) {
			throw std::runtime_error("Could not create directory");
		}

    }
    else { // Specified directory name already exists as a file or directory

        bool isDirectoryOrJunction =
            ((fileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            ((fileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) != 0);

        if (!isDirectoryOrJunction) {
            throw std::runtime_error(
                "Could not create directory because a file with the same name exists"
            );
        }

    }
}