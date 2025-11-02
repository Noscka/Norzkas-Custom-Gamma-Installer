#pragma once

#include <NosLib/Http/URL.hpp>
#include <NosLib/Http/HttpClient.hpp>
#include <NosLib/Logging.hpp>
#include <NosLib/ErrorHandling.hpp>
#include <NCGI/ErrorCodes.hpp>

#include "ModInfo.hpp"

namespace MO
{
	namespace /* Private */
	{
		NosLib::Result<std::string> get_latest_mo_version(const NosLib::HttpClient::Ptr& client)
		{
			NosLib::URL mo_release_url("https://github.com/ModOrganizer2/modorganizer/releases/latest");

			httplib::Result res = client->Get(mo_release_url.GetSubdir());

			if (!res)
			{
				return { NCGIError::HttpError, httplib::to_string(res.error()) };
			}

			httplib::Headers::const_iterator itr = res->headers.find("location");

			if (itr == res->headers.end())
			{
				return { NCGIError::HttpError, "Unable to find location header" };
			}


			/* Link will look like this: https://github.com/ModOrganizer2/modorganizer/releases/tag/v2.5.0 */
			std::string latestReleaseLink = itr->second;
			uint16_t offset = 2; /* Offset for "/v"  at the start of the substr */

			std::string latestVersion = latestReleaseLink.substr(latestReleaseLink.find_last_of(L'/') + offset);
			return latestVersion;
		}

		NosLib::Result<std::string> get_latest_download_link(const NosLib::HttpClient::Ptr& client, const std::string& version)
		{
			httplib::Result res = client->Get(std::format("/ModOrganizer2/modorganizer/releases/download/v{}/Mod.Organizer-{}.7z", version, version));

			if (!res)
			{
				return { NCGIError::HttpError, httplib::to_string(res.error()) };
			}

			httplib::Headers::const_iterator itr = res->headers.find("location");

			if (itr == res->headers.end())
			{
				return { NCGIError::HttpError, "Unable to find location header" };
			}

			std::string downloadLink = itr->second;

			return downloadLink;
		}
	}

	ModInfo GetModOrganizerModObject()
	{
		auto client = NosLib::HttpClient::MakeClient("https://github.com");
		client->set_keep_alive(true);

		/* TODO: ADD ERROR HANDLING */
		std::string moVersion = *get_latest_mo_version(client);
		std::string moDownloadLink = *get_latest_download_link(client, moVersion);

		std::string fileName = std::format("Mod.Organizer-{}", moVersion);

		return ModInfo(moDownloadLink,
					   std::vector<std::filesystem::path>({ "\\" }),
					   ""/* Root */,
					   fileName,
					   true,
					   ".7z");
	}

	void WriteConfigFile(const std::filesystem::path& modOrganizerRoot, std::filesystem::path& stalkerAnomalyPath)
	{
		std::string configContent = std::format(
			R"([General]
gameName=STALKER Anomaly
selected_profile=@ByteArray(Default)
gamePath=@ByteArray({})
version=2.5
first_start=false

[Settings]
style=vs15 Dark-Red.qss
profile_local_inis=true
profile_local_saves=true
profile_archive_invalidation=true)", stalkerAnomalyPath.string());

		std::ofstream moConfigWrite(modOrganizerRoot / "ModOrganizer.ini", std::ios::binary | std::ios::trunc);

		moConfigWrite.write(configContent.c_str(), configContent.size());

		moConfigWrite.close();
	}
}