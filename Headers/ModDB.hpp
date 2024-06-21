#pragma once

#include <NosLib/HttpClient.hpp>

#include <string>
#include <fstream>

class ModDB
{
protected:
	inline static ModDB* Instance = nullptr;

	httplib::Client ModDBDownloadClient;
	httplib::Client ModDBMirrorClient;

	ModDB() : 
		ModDBDownloadClient("https://www.moddb.com"),
		ModDBMirrorClient("https://www.moddb.com")
	{
		ModDBDownloadClient.set_follow_location(true);
		ModDBMirrorClient.set_follow_location(false);
	}

	inline static void Initialize()
	{
		if (Instance == nullptr)
		{
			Instance = new ModDB();
		}
	}
public:
	inline static httplib::Client* GetDownloadClient()
	{
		Initialize();
		return &(Instance->ModDBDownloadClient);
	}

	inline static std::wstring GetDownloadString(const std::wstring& downloadLink)
	{
		Initialize();
		return Instance->GetQuickestMirror(downloadLink);
	}
protected:
	std::wstring GetQuickestMirror(const std::wstring& downloadLink);
	NosLib::DynamicArray<std::string> ExtractMirrors(const std::string& pageContent);
	uint32_t PingMirror(const std::string& mirrorHostname);
	std::string GetHostName(const std::string& downloadLink);
};