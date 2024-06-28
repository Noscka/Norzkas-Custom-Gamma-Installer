#pragma once

#include <NosLib/HttpClient.hpp>

class Github
{
protected:
	inline static Github* Instance = nullptr;

	NosLib::HttpClient::ptr GithubClient;
	NosLib::HttpClient::ptr GithubObjectsClient;

	Github()
	{
		GithubClient = NosLib::HttpClient::MakeClient("https://github.com");
		GithubClient->set_follow_location(true);
		GithubClient->set_keep_alive(true);

		GithubObjectsClient = NosLib::HttpClient::MakeClient("https://objects.githubusercontent.com");
		GithubObjectsClient->set_follow_location(true);
		GithubObjectsClient->set_keep_alive(true);
	}

	inline static void Initialize()
	{
		if (Instance == nullptr)
		{
			Instance = new Github();
		}
	}
public:
	inline static NosLib::HttpClient* GetDownloadClient()
	{
		Initialize();

		return Instance->GithubClient.get();
	}

	inline static NosLib::HttpClient* GetDownloadObjectsClient()
	{
		Initialize();

		return Instance->GithubObjectsClient.get();
	}
};