#pragma once

#include <NosLib/HttpClient.hpp>

#include <string>
#include <fstream>

class ModDB
{
protected:
	inline static ModDB* Instance = nullptr;

	NosLib::HttpClient::ptr ModDBDownloadClient;
	NosLib::HttpClient::ptr ModDBMirrorClient;

	ModDB()
	{
		ModDBDownloadClient = NosLib::HttpClient::MakeClient("https://www.moddb.com");
		ModDBDownloadClient->set_follow_location(true);
		ModDBDownloadClient->set_keep_alive(true);

		ModDBMirrorClient = NosLib::HttpClient::MakeClient("https://www.moddb.com");
		ModDBMirrorClient->set_follow_location(false);
		ModDBMirrorClient->set_keep_alive(true);
	}

	inline static void Initialize()
	{
		if (Instance == nullptr)
		{
			Instance = new ModDB();
		}
	}
public:
	inline static NosLib::HttpClient* GetDownloadClient()
	{
		Initialize();
		return Instance->ModDBDownloadClient.get();
	}

	inline static std::wstring GetDownloadString(const std::wstring& downloadLink)
	{
		Initialize();
		return Instance->GetGivenMirror(downloadLink);
		/* Quickest Mirror Currently makes ModDB block user */
		//return Instance->GetQuickestMirror(downloadLink);
	}
protected:
	std::wstring GetGivenMirror(const std::wstring& downloadLink);

	std::wstring GetQuickestMirror(const std::wstring& downloadLink);
	std::string GetPageContent(const std::string& downloadLink);
	NosLib::DynamicArray<std::string> ExtractMirrors(const std::string& pageContent);
	uint32_t PingMirror(const std::string& mirrorHostname);
	std::string GetHostName(const std::string& downloadLink);
};