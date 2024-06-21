#pragma once

#include <NosLib/HashTable.hpp>
#include <NosLib/HostPath.hpp>
#include <NosLib/String.hpp>
#include <NosLib/Logging.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include "ModDB.hpp"

#include <string>
#include <functional>
#include <filesystem>

class ModInfo;

class File
{
public:
	using Status = void(ModInfo::*)(const std::wstring&);
	using Progress = bool(ModInfo::*)(uint64_t, uint64_t);
protected:
	struct FileStore
	{
		std::wstring FileName; /* Just the name: example ABC */
		std::wstring FileExtension; /* Just the Extension: example .rar */

		std::wstring GetFileName()
		{
			return FileName;
		}

		std::wstring GetFileExtension()
		{
			return FileExtension;
		}

		std::wstring GetFullFileName()
		{
			return FileName + FileExtension;
		}
	};

	enum class HostType
	{
		ModDB,
		Github,
		Unknown,
	};

	static inline bit7z::Bit7zLibrary lib = bit7z::Bit7zLibrary(L"7z.dll"); /* Load 7z.dll into a class */
	static inline bit7z::BitFileExtractor extractor = bit7z::BitFileExtractor(lib); /* create extractor object */

	static NosLib::HashTable<std::wstring, File*> fileHastTable;

	inline static std::wstring DownloadDirectory;
	inline static std::wstring ExtractDirectory;

	NosLib::HostPath Link;
	FileStore FileName;

	bool Extracted = false;
	int UsageCount;

	ModInfo* CallerPointer = nullptr;
	Status StatusCallback;
	Progress ProgressCallback;


	File(const NosLib::HostPath& link, const std::wstring& fileName, const std::wstring& fileExtensionOverwrite = L"")
	{
		Link = link;
		FileName.FileName = fileName;
		FileName.FileExtension = fileExtensionOverwrite;
		UsageCount = 0;
	}

public:
	File(){}

	inline static void SetDirectories(const std::wstring& downloadDirectory, const std::wstring& extractDirectory)
	{
		DownloadDirectory = downloadDirectory;
		ExtractDirectory = extractDirectory;
		NosLib::Logging::CreateLog<wchar_t>(std::format(L"Set download directory to: \"{}\"", DownloadDirectory), NosLib::Logging::Severity::Info);
		NosLib::Logging::CreateLog<wchar_t>(std::format(L"Set extract directory to: \"{}\"", ExtractDirectory), NosLib::Logging::Severity::Info);
	}


	inline static File* RegisterFile(const NosLib::HostPath& link, const std::wstring& fileName, const std::wstring& fileExtensionOverwrite = L"")
	{
		File** searchedFile = fileHastTable.Find(link.Full());
		File* returnFile;

		std::wstring logMessage;

		/* Same Link file not found */
		if (searchedFile == nullptr)
		{
			returnFile = new File(link, fileName, fileExtensionOverwrite);
			fileHastTable.Insert(returnFile);
			logMessage = std::format(L"File \"{}\" For \"{}\" Not Found, Creating new", returnFile->FileName.GetFullFileName(), returnFile->Link.Full());
		}
		else
		{
			returnFile = *searchedFile;
			logMessage = std::format(L"File \"{}\" For \"{}\" Found, Has {} uses", returnFile->FileName.GetFullFileName(), returnFile->Link.Full(), returnFile->UsageCount+1);
		}

		NosLib::Logging::CreateLog<wchar_t>(logMessage, NosLib::Logging::Severity::Debug);
		returnFile->UsageCount++;

		return returnFile;
	}

	/* Returns File Path */
	std::wstring GetFile(ModInfo* callerPointer, const Status& statusCallback, const Progress& progressCallback)
	{
		/* Update Callbacks */
		CallerPointer = callerPointer;
		StatusCallback = statusCallback;
		ProgressCallback = progressCallback;

		if (!Extracted)
		{
			if (!DownloadFile())
			{
				return L"";
			}

			/* if failed */
			if (!ExtractFile())
			{
				return L"";
			}

			Extracted = true;
		}

		return GetExtractPath();
	}

	/* Finished using file */
	inline void Finished()
	{
		UsageCount--;

		/* More file objects are using */
		if (UsageCount > 0)
		{
			return;
		}

		std::error_code ec;
		if (-1 == std::filesystem::remove_all(GetDownloadPath(), ec))
		{
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove download File", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
		}
		if (-1 == std::filesystem::remove_all(GetExtractPath(), ec))
		{
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove extract directory", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
		}

		fileHastTable.Remove(GetKey());
		delete this;
	}

	inline std::wstring GetKey()
	{
		return Link.Full();
	}
protected:
	std::wstring GetDownloadPath()
	{
		return DownloadDirectory + FileName.GetFullFileName();
	}

	std::wstring GetExtractPath()
	{
		return ExtractDirectory + FileName.GetFileName();
	}

	std::wstring GetFileExtensionFromHeader(const std::string& type);
	HostType DetermineHostType(const std::wstring& hostName);
	bool DownloadFile();
	bool ModDBDownload(httplib::Client* downloadClient, const std::wstring& pathOffsets);
	bool GithubDownload(httplib::Client* downloadClient, const std::wstring& pathOffsets);
	bool GetAndSaveFile(httplib::Client* client, const std::wstring& urlFilePath, const std::wstring& pathOffsets);

	bool ExtractFile();
};