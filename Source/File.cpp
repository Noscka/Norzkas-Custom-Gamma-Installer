#include "../Headers/File.hpp"
#include "../Headers/HTMLParsing.hpp"

#include <NosLib/HttpClient.hpp>

#include <fstream>
#include <filesystem>

NosLib::HashTable<std::wstring, File*> File::fileHastTable(&File::GetKey, 400);


std::wstring File::GetFileExtensionFromHeader(const std::string& type)
{
	if (type.find("application/zip") != std::string::npos)
	{
		return L".zip";
	}
	
	if (type.find("application/x-7z-compressed") != std::string::npos)
	{
		return L".7z";
	}
	
	if (type.find("application/x-rar-compressed") != std::string::npos || type.find("application/vnd.rar") != std::string::npos)
	{
		return L".rar";
	}

	return L".ERROR";
}

File::HostType File::DetermineHostType(const std::wstring& hostName)
{
	if (hostName.find(L"moddb") != std::wstring::npos)
	{
		return HostType::ModDB;
	}
	
	if (hostName.find(L"github") != std::wstring::npos)
	{
		return HostType::Github;
	}

	return HostType::Unknown;
}

bool File::DownloadFile()
{
	/* create client for the host */
	httplib::Client downloadClient = NosLib::MakeClient(NosLib::String::ToString(Link.Host), true);
	downloadClient.set_follow_location(true);

	/* create directories in order to prevent any errors */
	std::filesystem::create_directories(DownloadDirectory);

	/* Decide the host type, there are different download steps for different websites */
	switch (DetermineHostType(Link.Host))
	{
	case HostType::ModDB:
		return ModDBDownload(&downloadClient, DownloadDirectory);

	case HostType::Github:
		return GithubDownload(&downloadClient, DownloadDirectory);

	default:
		NosLib::Logging::CreateLog<wchar_t>(std::format(L"Mod uses unknown provider: \"{}\"", Link.Full()), NosLib::Logging::Severity::Error);
		return false;
	}

	return false;
}

bool File::ModDBDownload(httplib::Client* downloadClient, const std::wstring& pathOffsets)
{
	std::wstring linkOutput = ModDBParsing::ParseHtmlForLink(downloadClient->Get(NosLib::String::ToString(Link.Path))->body);

	if (linkOutput.empty())
	{
		NosLib::Logging::CreateLog<wchar_t>(std::format(L"Couldn't find link for \"{}\" using link \"{}\"", FileName.GetFullFileName(), Link.Full()), NosLib::Logging::Severity::Error);
		return false;
	}

	return GetAndSaveFile(downloadClient, linkOutput, pathOffsets);
}

bool File::GithubDownload(httplib::Client* downloadClient, const std::wstring& pathOffsets)
{
	return GetAndSaveFile(downloadClient, Link.Path, pathOffsets);
}

bool File::GetAndSaveFile(httplib::Client* client, const std::wstring& urlFilePath, const std::wstring& pathOffsets)
{
	std::ofstream downloadFile;

	httplib::Result res = client->Get(NosLib::String::ToString(urlFilePath),
									  [&](const httplib::Response& response)
	{
		if (FileName.FileExtension.empty())
		{
			FileName.FileExtension = GetFileExtensionFromHeader(response.headers.find("Content-Type")->second);
		}

		std::wstring statusText = std::format(L"Downloading \"{}\"", FileName.GetFullFileName());

		httplib::Headers::const_iterator itr = response.headers.find("Transfer-Encoding");
		if ((itr != response.headers.end() ? itr->second : "HEADER DOESN'T EXIST") == "chunked")
		{
			statusText += L" - Won't Show Progress Due to \"chunked\" Transfer-Encoding";
		}

		InitialCallback(statusText);

		/* before start download, get "Content-Type" header tag to see the extensions, then open with the name+extension */
		downloadFile.open(pathOffsets + FileName.GetFullFileName(), std::ios::binary | std::ios::trunc);
		return true;
	},
									  [&](const char* data, size_t data_length)
	{
		/* write to file while downloading, this makes sure that it doesn't download to memory and then write */
		downloadFile.write(data, data_length);
		return true;
	},
									  ProgressCallback);

	if (!res)
	{
		NosLib::Logging::CreateLog<wchar_t>(std::format(L"connection error code: {}\n", NosLib::String::ToWstring(httplib::to_string(res.error()))), NosLib::Logging::Severity::Error);
		return false;
	}

	downloadFile.close();
	return true;
}

bool File::ExtractFile()
{
	/* create directories in order to prevent any errors */
	std::filesystem::create_directories(GetExtractPath());

	NosLib::Logging::CreateLog<wchar_t>(std::format(L"Extracting \"{}\" To \"{}\"", GetDownloadPath(), GetExtractPath()), NosLib::Logging::Severity::Info);

	uint64_t totalSize = 0;

	extractor.setTotalCallback([&](uint64_t total_size)
	{
		totalSize = total_size;
	});

	extractor.setProgressCallback([&](uint64_t processed_size)
	{
		return ProgressCallback(processed_size, totalSize);
	});

	try
	{
		InitialCallback(std::format(L"Extracting \"{}\"", FileName.GetFullFileName()));
		extractor.extract(GetDownloadPath(), GetExtractPath());
	}
	catch (const bit7z::BitException& ex)
	{
		std::wstring errorMessage;
		for (std::pair<std::wstring, std::error_code> entry : ex.failedFiles())
		{
			errorMessage += std::format(L"{} : {}\n", entry.first, NosLib::String::ToWstring(entry.second.message()));
		}

		errorMessage += NosLib::String::ToWstring(std::format("{}\n", ex.what()));
		NosLib::Logging::CreateLog<wchar_t>(errorMessage, NosLib::Logging::Severity::Error);
		return false;
	}
	NosLib::Logging::CreateLog<wchar_t>(std::format(L"Extracted \"{}\" To \"{}\"", GetDownloadPath(), GetExtractPath()), NosLib::Logging::Severity::Info);

	return true;
}