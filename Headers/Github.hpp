#pragma once

#include <NosLib/HttpClient.hpp>

class Github
{
protected:
	inline static Github* Instance = nullptr;

	httplib::Client GithubClient;

	Github() : GithubClient("https://github.com")
	{
		GithubClient.set_follow_location(true);
	}
public:
	inline static httplib::Client* GetDownloadClient()
	{
		if (Instance == nullptr)
		{
			Instance = new Github();
		}

		return &(Instance->GithubClient);
	}
};