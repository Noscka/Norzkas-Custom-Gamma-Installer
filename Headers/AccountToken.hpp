#pragma once

#include <NosLib/Console.hpp>
#include <NosLib/HttpClient.hpp>

#include <fstream>
#include <format>

#include "GlobalVariables.hpp"

namespace AccountToken
{
	std::string AccountToken;

	void GetAccountToken()
	{
		httplib::Client cookieGetter = NosLib::MakeClient("https://api.gofile.io", true, "NCGI");

		cookieGetter.set_keep_alive(true);

		/*
		Using 32 and 32 for the substr to get the token because this is an example reponse from server:
		{"status":"ok","data":{"token":"TQVqYtD4oadFe3RLFWs0Kgwha43dqIKY"}}
		*/
		//wprintf(L"Getting GoLink Token\n");
		AccountToken = cookieGetter.Get("/createAccount")->body.substr(32, 32);

		cookieGetter.Get(std::format("/getContent?contentId=WlndL7&token={}&websiteToken=7fd94ds12fds4", AccountToken));

		if constexpr (Global::verbose)
		{
			printf("Got Token %s and got authorized\n", AccountToken.c_str());
		}
	}
}