#pragma once

#include <NosLib/HashTable.hpp>
#include <NosLib/HostPath.hpp>
#include <NosLib/String.hpp>
#include <NosLib/Logging.hpp>

#include <string>
#include <functional>
#include <filesystem>

class File
{
public:
	using Initial = std::function<void(const std::wstring&)>;
	using Progress = std::function<bool(uint64_t, uint64_t)>;
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

	static NosLib::HashTable<std::wstring, File*> fileHastTable;

	inline static std::wstring DownloadDirectory;

	NosLib::HostPath Link;
	FileStore FileName;

	bool Downloaded = false;
	bool Extracted = false;
	int UsageCount;

	Initial InitialCallback;
	Progress ProgressCallback;


	File(const NosLib::HostPath& link, const std::wstring& fileName, const Initial& initialCallback, const Progress& progressCallback, const std::wstring& fileExtensionOverwrite = L"")
	{
		Link = link;
		FileName.FileName = fileName;
		FileName.FileExtension = fileExtensionOverwrite;
		UsageCount = 0;

		InitialCallback = initialCallback;
		ProgressCallback = progressCallback;

	}
public:
	File(){}

	inline static void SetDownloadDirectory(const std::wstring& downloadDirectory)
	{
		DownloadDirectory = downloadDirectory;
		NosLib::Logging::CreateLog<wchar_t>(std::format(L"Set download path to: \"{}\"", DownloadDirectory), NosLib::Logging::Severity::Info);
	}

	inline static File* RegisterFile(const NosLib::HostPath& link, const std::wstring& fileName, const Initial& initialCallback, const Progress& progressCallback, const std::wstring& fileExtensionOverwrite = L"")
	{
		File** searchedFile = fileHastTable.Find(link.Full());
		File* returnFile;

		std::wstring logMessage;

		/* Same Link file not found */
		if (searchedFile == nullptr)
		{
			returnFile = new File(link, fileName, initialCallback, progressCallback, fileExtensionOverwrite);
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
	std::wstring GetFile()
	{
		if (!Downloaded)
		{
			/* if failed */
			if (!DownloadFile())
			{
				return L"";
			}
			
			Downloaded = true;
		}

		return DownloadDirectory + FileName.GetFullFileName();
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
		if (-1 == std::filesystem::remove_all((DownloadDirectory + FileName.GetFullFileName()), ec))
		{
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove download File", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
		}

		fileHastTable.Remove(GetKey());
		delete this;
	}

	inline std::wstring GetKey()
	{
		return Link.Full();
	}
protected:

	std::wstring GetFileExtensionFromHeader(const std::string& type);
	HostType DetermineHostType(const std::wstring& hostName);
	bool DownloadFile();
	bool ModDBDownload(httplib::Client* downloadClient, const std::wstring& pathOffsets);
	bool GithubDownload(httplib::Client* downloadClient, const std::wstring& pathOffsets);
	bool GetAndSaveFile(httplib::Client* client, const std::wstring& urlFilePath, const std::wstring& pathOffsets);
};