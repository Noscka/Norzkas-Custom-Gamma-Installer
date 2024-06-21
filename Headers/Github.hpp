#pragma once

#include <NosLib/HttpClient.hpp>

class Github
{
protected:
	inline static Github* Instance = nullptr;

	httplib::Client GithubClient;
	httplib::Client GithubObjectsClient;

	Github() : 
		GithubClient("https://github.com"),
		GithubObjectsClient("https://objects.githubusercontent.com")
	{
		GithubClient.set_follow_location(true);
		GithubObjectsClient.set_follow_location(true);
	}

	inline static void Initialize()
	{
		if (Instance == nullptr)
		{
			Instance = new Github();
		}
	}
public:
	inline static httplib::Client* GetDownloadClient()
	{
		Initialize();

		return &(Instance->GithubClient);
	}

	inline static httplib::Client* GetDownloadObjectsClient()
	{
		Initialize();

		return &(Instance->GithubObjectsClient);
	}
};