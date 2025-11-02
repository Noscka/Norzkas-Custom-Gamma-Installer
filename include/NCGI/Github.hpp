#pragma once
#include <NosLib/Http/HttpClient.hpp>

class Github
{
public:
	inline static NosLib::HttpClient::Ptr CreateDownloadClient()
	{
		NosLib::HttpClient::Ptr githubClient;
		githubClient = NosLib::HttpClient::MakeClient("https://github.com");
		githubClient->set_follow_location(true);
		githubClient->set_keep_alive(true);

		return githubClient;
	}

	inline static NosLib::HttpClient::Ptr CreateDownloadObjectsClient()
	{
		NosLib::HttpClient::Ptr githubObjectsClient;
		githubObjectsClient = NosLib::HttpClient::MakeClient("https://objects.githubusercontent.com");
		githubObjectsClient->set_follow_location(true);
		githubObjectsClient->set_keep_alive(true);

		return githubObjectsClient;
	}
};