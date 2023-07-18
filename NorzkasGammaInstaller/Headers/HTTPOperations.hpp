#pragma once

#include "EXTERNAL\httplib.h"
#include <string>

#include "Headers\ModMakerParsing.hpp"

std::string GetFileExtensionFromHeader(const std::string& type)
{
	if (type.find("application/zip") != std::string::npos)
	{
		return ".zip";
	}
	else if (type.find("application/x-7z-compressed") != std::string::npos)
	{
		return ".7z";
	}
	else if (type.find("application/x-rar-compressed") != std::string::npos || type.find("application/vnd.rar") != std::string::npos)
	{
		return ".rar";
	}

	return ".ERROR";
}

void GetAndSaveFile(httplib::Client* client, ModPackMaker::ModInfo* mod, const std::string& filepath, const std::string& pathOffsets = "")
{
	std::ofstream DownloadFile;

	httplib::Result res = client->Get(filepath,
		[&](const httplib::Response& response)
		{
			mod->FileExtension = GetFileExtensionFromHeader(response.headers.find("Content-Type")->second);
			/* before start download, get "Content-Type" header tag to see the extensions, then open with the name+extension */
			DownloadFile.open(pathOffsets + mod->GetFullFileName(true), std::ios::binary | std::ios::trunc);
			return true; // return 'false' if you want to cancel the request.
		},
		[&](const char* data, size_t data_length)
		{
			/* write to file while downloading, this makes sure that it doesn't download to memory and then write */
			DownloadFile.write(data, data_length);
			return true;
		},
		[](uint64_t len, uint64_t total)
		{
			/* TEMP: Just so it doesn't slow down downloads just because it is spamming the console */
			if (len % 431 != 0)
			{
				return true;
			}

			wprintf(L"%lld / %lld bytes => %d%% complete\n", len, total, (int)(len * 100 / total));
			return true; // return 'false' if you want to cancel the request.
		});

	PrintServerResponse(res);

	DownloadFile.close();

	return;
}

enum class HostType
{
	ModDB,
	Github,
	Unknown,
};

HostType DetermineHostType(const std::string& hostName)
{
	if (hostName.find("moddb") != std::string::npos)
	{
		return HostType::ModDB;
	}
	else if (hostName.find("github") != std::string::npos)
	{
		return HostType::Github;
	}

	return HostType::Unknown;
}

void ModDBDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
{
	ModDBParsing::HTMLParseReturn LinkOutput = ModDBParsing::ParseHtmlForLink(downloadClient->Get(mod->Link.Path)->body);

	if (LinkOutput.Link == "No Link Found")
	{
		return;
	}

	downloadClient->set_follow_location(true);
	//downloadClient->set_logger(&LoggingFunction);
	GetAndSaveFile(downloadClient, mod, LinkOutput.Link, pathOffsets);
}

void GithubDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
{
	downloadClient->set_follow_location(true);
	//downloadClient->set_logger(&LoggingFunction);
	GetAndSaveFile(downloadClient, mod, mod->Link.Path, pathOffsets);
}