#pragma once

#include <NosLib/HttpClient.hpp>
#include <NosLib/Logging.hpp>

#include <fstream>
#include <format>

namespace AccountToken
{
	inline std::string AccountToken;

	inline void GetAccountToken()
	{
		httplib::Client cookieGetter = NosLib::MakeClient("https://api.gofile.io", true, "NCGI");

		cookieGetter.set_keep_alive(true);

		/*
		Example Reponse from GoLink:
		{"status":"ok","data":{"id":"a11cf144-f11a-4458-a085-468a4d3fc55b","token":"euCT1LRW8n67BJjGiu0tBPcTB6v1q7XF"}}
		*/
		std::string response = cookieGetter.Post("/accounts")->body;
		std::string id = response.substr(29, 36);
		AccountToken = response.substr(76, 32);

		NosLib::Logging::CreateLog<char>(std::format("Got Token {}", AccountToken), NosLib::Logging::Severity::Info);

		cookieGetter.set_default_headers({
				{"Authorization", std::format("Bearer {}", AccountToken)},
				{"User-Agent", "NCGI (cpp-httplib)"} });
		cookieGetter.Get("/contents/ccbnSP?wt=4fd6sg89d7s6");

		NosLib::Logging::CreateLog<char>(std::format("Authorized Token {}", AccountToken), NosLib::Logging::Severity::Info);
	}
}