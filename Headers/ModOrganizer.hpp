#pragma once

#include <NosLib/HostPath.hpp>
#include <NosLib/Logging.hpp>

#include "ModInfo.hpp"

namespace MO
{
	namespace /* Private */
	{
		std::wstring GetLatestMOVersion(httplib::Client* client)
		{
			NosLib::HostPath hostPath("https://github.com/ModOrganizer2/modorganizer/releases/latest");
			
			httplib::Result res = client->Get(NosLib::String::ToString(hostPath.Path));

			if (!res)
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"connection error code: {}\n", NosLib::String::ToWstring(httplib::to_string(res.error()))), NosLib::Logging::Severity::Error);
				return L"";
			}

			httplib::Headers::const_iterator itr = res->headers.find("location");

			if (itr == res->headers.end())
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"Could not find location header"), NosLib::Logging::Severity::Error);
				return L"";
			}


			/* Link will look like this: https://github.com/ModOrganizer2/modorganizer/releases/tag/v2.5.0 */
			std::string latestReleaseLink = itr->second;
			uint16_t offset = 2; /* Offset for "/v"  at the start of the substr */

			std::string latestVersion = latestReleaseLink.substr(latestReleaseLink.find_last_of(L'/') + offset);
			return NosLib::String::ToWstring(latestVersion);
		}

		std::wstring GetLatestDownloadLink(httplib::Client* client, const std::wstring& version)
		{
			std::string nVersion = NosLib::String::ToString(version);

			httplib::Result res = client->Get(std::format("/ModOrganizer2/modorganizer/releases/download/v{}/Mod.Organizer-{}.7z", nVersion, nVersion));

			if (!res)
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"connection error code: {}\n", NosLib::String::ToWstring(httplib::to_string(res.error()))), NosLib::Logging::Severity::Error);
				return L"";
			}

			httplib::Headers::const_iterator itr = res->headers.find("location");

			if (itr == res->headers.end())
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"Could not find location header"), NosLib::Logging::Severity::Error);
				return L"";
			}

			std::string downloadLink = itr->second;

			return NosLib::String::ToWstring(downloadLink);
		}
	}

	ModInfo GetModOrganizerModObject()
	{
		httplib::Client client = NosLib::HttpUtilities::MakeClient("https://github.com");
		client.set_keep_alive(true);

		std::wstring moVersion = GetLatestMOVersion(&client);
		std::wstring moDownloadLink = GetLatestDownloadLink(&client, moVersion);

		std::wstring fileName = std::format(L"Mod.Organizer-{}", moVersion);

		return ModInfo(moDownloadLink,
									 NosLib::DynamicArray<std::wstring>({ L"\\" }),
									 L""/* Root */,
									 fileName,
									 true,
									 L".7z");
	}

	void WriteConfigFile(const std::wstring& modOrganizerRoot, std::wstring stalkerAnomalyPath)
	{
		/* Need to double the \ */
		for (int i = 0; i < stalkerAnomalyPath.size(); i++)
		{
			if (stalkerAnomalyPath[i] == L'\\')
			{
				stalkerAnomalyPath.insert(i, L"\\");
				i++;
			}
		}

		std::wstring configContent = std::format(
LR"([General]
gameName=STALKER Anomaly
selected_profile=@ByteArray(Default)
gamePath=@ByteArray({})
version=2.5
first_start=false

[Settings]
style=vs15 Dark-Red.qss
profile_local_inis=true
profile_local_saves=true
profile_archive_invalidation=true)", stalkerAnomalyPath);

		std::wofstream moConfigWrite(modOrganizerRoot+L"ModOrganizer.ini", std::ios::binary | std::ios::trunc);

		moConfigWrite.write(configContent.c_str(), configContent.size());

		moConfigWrite.close();
	}
}