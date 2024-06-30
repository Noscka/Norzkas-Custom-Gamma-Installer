#pragma once

#include <NosLib/HttpClient.hpp>

class Github
{
protected:
	Github()
	{
		
	}

	inline static void Initialize()
	{
		#if 0
		if (Instance == nullptr)
		{
			Instance = new Github();
		}
		#endif // 0
	}
public:
	inline static NosLib::HttpClient::ptr CreateDownloadClient()
	{
		Initialize();

		NosLib::HttpClient::ptr githubClient;
		githubClient = NosLib::HttpClient::MakeClient("https://github.com");
		githubClient->set_follow_location(true);
		githubClient->set_keep_alive(true);

		return githubClient;
	}

	inline static NosLib::HttpClient::ptr CreateDownloadObjectsClient()
	{
		Initialize();

		NosLib::HttpClient::ptr githubObjectsClient;
		githubObjectsClient = NosLib::HttpClient::MakeClient("https://objects.githubusercontent.com");
		githubObjectsClient->set_follow_location(true);
		githubObjectsClient->set_keep_alive(true);

		return githubObjectsClient;
	}
};