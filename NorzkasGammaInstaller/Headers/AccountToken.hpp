#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../EXTERNAL/httplib.h"
#include "../EXTERNAL/json.hpp"

#include <NosLib/Console.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

#include "Headers/HTTPLogging.hpp"

namespace AccountToken
{
	std::string AccountToken;

	void GetAccountToken()
	{
		httplib::Client cookieGetter("https://api.gofile.io");

		cookieGetter.set_follow_location(false);
		cookieGetter.set_logger(&LoggingFunction);
		cookieGetter.set_keep_alive(true);
		cookieGetter.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

		AccountToken = nlohmann::json::parse(cookieGetter.Get("/createAccount")->body)["data"]["token"];

		cookieGetter.Get(std::format("/getContent?contentId=WlndL7&token={}&websiteToken=7fd94ds12fds4", AccountToken));
	}
}