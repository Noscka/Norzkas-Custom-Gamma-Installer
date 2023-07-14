#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../EXTERNAL/httplib.h"
#include "../EXTERNAL/json.hpp"

#include <NosLib/Console.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>

#include "Headers/Global.hpp"

namespace AccountToken
{
	std::string AccountToken;

	void GetAccountToken()
	{
		httplib::Client cookieGetter("https://api.gofile.io");

		cookieGetter.set_follow_location(false);
		cookieGetter.set_logger(&LoggingFunction);
		cookieGetter.set_keep_alive(false);
		cookieGetter.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

		httplib::Result res = cookieGetter.Get("/createAccount");

		//std::ofstream DownloadFile("cookie.json", std::ios::binary | std::ios::trunc);
		//DownloadFile.write(res->body.c_str(), res->body.size());

		nlohmann::json data = nlohmann::json::parse(res->body);
		//std::wcout << NosLib::String::ConvertString<wchar_t, char>(data["data"]["token"]) << std::endl;
		AccountToken = data["data"]["token"];

		res = cookieGetter.Get(std::format("/getContent?contentId=WlndL7&token={}&websiteToken=7fd94ds12fds4", AccountToken));
	}
}