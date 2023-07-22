#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../EXTERNAL/httplib.h"

#include <NosLib/Console.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

#include "Headers/HTTPLogging.hpp"
#include "GlobalVariables.hpp"

namespace AccountToken
{
	std::string AccountToken;

	void GetAccountToken()
	{
		httplib::Client cookieGetter("https://api.gofile.io");

		if constexpr (Global::verbose)
		{
			cookieGetter.set_logger(&LoggingFunction);
		}
		cookieGetter.set_follow_location(false);
		cookieGetter.set_keep_alive(true);
		cookieGetter.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

		/*
		Using 32 and 32 for the substr to get the token because this is an example reponse from server:
		{"status":"ok","data":{"token":"TQVqYtD4oadFe3RLFWs0Kgwha43dqIKY"}}
		*/
		wprintf(L"Getting GoLink Token\n");
		AccountToken = cookieGetter.Get("/createAccount")->body.substr(32, 32);

		cookieGetter.Get(std::format("/getContent?contentId=WlndL7&token={}&websiteToken=7fd94ds12fds4", AccountToken));
		wprintf(std::format(L"Got Token \"{}\" and got authorized\n", NosLib::String::ToWstring(AccountToken)).c_str());
	}
}