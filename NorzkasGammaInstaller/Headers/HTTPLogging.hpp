#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "EXTERNAL/httplib.h"

#include <Windows.h>
#include <format>

void PrintServerResponse(const httplib::Result& serverResult)
{
	if (!serverResult)
	{
		wprintf(std::format(L"error code: {}\n", (int)serverResult.error()).c_str());
	}
}

void LoggingFunction(const httplib::Request& req, const httplib::Response& res)
{
	wprintf(L"====================================================================================================================\nRequest\n");
	wprintf(std::format(L":METHOD: {}\n", NosLib::String::ConvertString<wchar_t, char>(req.method)).c_str());
	wprintf(std::format(L":PATH:   {}\n", NosLib::String::ConvertString<wchar_t, char>(req.path)).c_str());
	wprintf(std::format(L":BODY:   {}\n", NosLib::String::ConvertString<wchar_t, char>(req.body)).c_str());

	wprintf(L"======HEADERS======\n");

	for (auto itr = req.headers.begin(); itr != req.headers.end(); itr++)
	{
		wprintf(NosLib::String::ConvertString<wchar_t, char>(std::format("{} : {}\n", itr->first, itr->second)).c_str());
	}
	wprintf(L"====================================================================================================================\nResponse\n");

	wprintf(std::format(L":STATUS: {}\n", res.status).c_str());
	wprintf(std::format(L":REASON: {}\n", NosLib::String::ConvertString<wchar_t, char>(res.reason)).c_str());
	wprintf(std::format(L":BODY:   {}\n", NosLib::String::ConvertString<wchar_t, char>(res.body)).c_str());
	wprintf(std::format(L":LOCATION:   {}\n", NosLib::String::ConvertString<wchar_t, char>(res.location)).c_str());

	wprintf(L"======HEADERS======\n");

	for (auto itr = res.headers.begin(); itr != res.headers.end(); itr++)
	{
		wprintf(NosLib::String::ConvertString<wchar_t, char>(std::format("{} : {}\n", itr->first, itr->second)).c_str());
	}
	wprintf(L"====================================================================================================================\n");
}