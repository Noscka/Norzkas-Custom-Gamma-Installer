#pragma once
#include <NosLib/Http/HttpClient.hpp>
#include <NosLib/Http/URL.hpp>
#include <NosLib/ErrorHandling.hpp>

#include <string>

class ModDB
{
protected:
	NosLib::HttpClient::Ptr ModDBMirrorClient;

	ModDB()
	{
		ModDBMirrorClient = NosLib::HttpClient::MakeClient("https://www.moddb.com");
		ModDBMirrorClient->set_follow_location(false);
		ModDBMirrorClient->set_keep_alive(true);
	}

	inline static ModDB& get_instance()
	{
		static ModDB* instance{ nullptr };
		if (instance == nullptr)
		{
			instance = new ModDB();
		}
		return *instance;
	}
public:
	inline static NosLib::HttpClient::Ptr CreateDownloadClient()
	{
		NosLib::HttpClient::Ptr modDBDownloadClient;
		modDBDownloadClient = NosLib::HttpClient::MakeClient("https://www.moddb.com");
		modDBDownloadClient->set_follow_location(true);
		modDBDownloadClient->set_keep_alive(true);
		return modDBDownloadClient;
	}

	static NosLib::Result<NosLib::URL> get_download_url(const NosLib::URL& download_page);
protected:
	NosLib::Result<NosLib::URL> get_mirror_url(const NosLib::URL& download_page);
	NosLib::Result<std::string> get_page_content(const NosLib::URL& download_page);
};