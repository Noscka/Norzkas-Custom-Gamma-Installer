#pragma once

#include <NosLib/HostPath.hpp>
#include <NosLib/String.hpp>

class File
{
private:
	static inline NosLib::DynamicArray<File*> fileArray;

	NosLib::HostPath Link;
	std::wstring FileName;

	File(const NosLib::HostPath& link)
	{
		Link = link;
	}

	/// <summary>
	/// Function used to compare the file objects
	/// </summary>
	/// <returns>if the objects both are about the same file</returns>
	static bool Compare(File* left, File* right)
	{
		return (left->Link == right->Link);
	}
public:
	void UpdateFileName(const std::wstring& fileName)
	{
		FileName = fileName;
	}

	/// <summary>
	/// Registers the file, if the file already exists, returns that object
	/// </summary>
	/// <param name="name">- name of the file</param>
	/// <returns>file pointer that will be used</returns>
	static File* RegisterFile(const NosLib::HostPath& link)
	{
		File* newFile = new File(link);

		for (File* entry : fileArray)
		{
			if (Compare(newFile, entry))
			{
				delete newFile;
				return entry;
			}
		}

		fileArray.Append(newFile);

		return newFile;
	}
};