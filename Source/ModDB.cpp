#include <NosLib/Logging.hpp>
#include <html.hpp>

#include "../Headers/ModDB.hpp"

#include <future>
#include <vector>

#include <IPExport.h>
#include <icmpapi.h>
#include <WinSock2.h>
#include <Ws2tcpip.h>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

std::wstring ModDB::GetGivenMirror(const std::wstring& downloadLink)
{
	std::string pageContent = GetPageContent(NosLib::String::ToString(downloadLink));

	if (pageContent.empty())
	{
		return L"";
	}

	html::parser p;
	html::node_ptr node = p.parse(pageContent);

	std::vector<html::node*> mirrorContainers = node->select("a[href]");

	if (mirrorContainers.size() < 1)
	{
		return L"";
	}

	return NosLib::String::ToWstring(mirrorContainers[0]->get_attr("href"));
}

std::wstring ModDB::GetQuickestMirror(const std::wstring& downloadLink)
{
	std::string pageContent = GetPageContent(NosLib::String::ToString(downloadLink) + "/all");
	
	if (pageContent.empty())
	{
		return L"";
	}

	NosLib::DynamicArray<std::string> mirrors = ExtractMirrors(pageContent);
	std::vector<std::future<uint32_t>> futures;

	for (const std::string& mirror : mirrors)
	{
		futures.emplace_back(std::async(std::launch::async,
								  [this](const std::string& mirrorHostname) { return PingMirror(mirrorHostname); },
								  std::cref(mirror)));
	}

	int quickestIndex = 0;
	uint32_t quickestTime = -1; /* underflow to max */
	for (int i = 0; i < futures.size(); i++)
	{
		uint32_t currentTime = futures[i].get();

		if (currentTime < quickestTime)
		{
			quickestTime = currentTime;
			quickestIndex = i;
		}
	}

	NosLib::Logging::CreateLog<char>(std::format("Quickest Mirror was: {} | ping time: {}ms", quickestIndex, quickestTime), NosLib::Logging::Severity::Debug);

	return NosLib::String::ToWstring(mirrors[quickestIndex]);
}

std::string ModDB::GetPageContent(const std::string& downloadLink)
{
	httplib::Result modDBResult = ModDBMirrorClient->Get(downloadLink );

	if (modDBResult->status == 503)
	{
		NosLib::Logging::CreateLog<char>("ModDB currently Unavailable, most likely too many requests", NosLib::Logging::Severity::Error);
		return "";
	}

	if (modDBResult->status != 200)
	{
		NosLib::Logging::CreateLog<char>(std::format("File not found. Status: {} | Reason: \"{}\"", modDBResult->status, modDBResult->reason), NosLib::Logging::Severity::Error);
		return "";
	}

	return modDBResult->body;
}

NosLib::DynamicArray<std::string> ModDB::ExtractMirrors(const std::string& pageContent)
{
	NosLib::DynamicArray<std::string> out;

	html::parser p;
	html::node_ptr node = p.parse(pageContent);

	std::vector<html::node*> mirrorContainers = node->select("a#downloadon");

	for (html::node* entry : mirrorContainers)
	{
		out.Append(entry->get_attr("href"));
	}

	return out;
}

uint32_t ModDB::PingMirror(const std::string& mirrorHostname)
{
	httplib::Result res = ModDBMirrorClient->Get(mirrorHostname);

	httplib::Headers::const_iterator itr = res->headers.find("location");
	std::string hostnameString = GetHostName(itr->second);

	// Create the ICMP context.
	HANDLE icmp_handle = IcmpCreateFile();
	if (icmp_handle == INVALID_HANDLE_VALUE)
	{
		throw;
	}

	addrinfo* addr = nullptr;

	/* Get IP Address */
	int errorStatus = getaddrinfo(hostnameString.c_str(), NULL, NULL, &addr);
	if (errorStatus != 0)
	{
		throw;
	}

	sockaddr_in* destAddr_ipv4 = reinterpret_cast<sockaddr_in*>(addr->ai_addr);

	// Payload to send.
	constexpr WORD payload_size = 1;
	unsigned char payload[payload_size]{ 42 };

	// Reply buffer for exactly 1 echo reply, payload data, and 8 bytes for ICMP error message.
	constexpr DWORD reply_buf_size = sizeof(ICMP_ECHO_REPLY) + payload_size + 8;
	unsigned char reply_buf[reply_buf_size]{};

	IcmpSendEcho(icmp_handle,
				 destAddr_ipv4->sin_addr.S_un.S_addr,
				 payload,
				 payload_size,
				 NULL,
				 reply_buf,
				 reply_buf_size,
				 1000);

	const ICMP_ECHO_REPLY* reply = (const ICMP_ECHO_REPLY*)reply_buf;

	if (reply->Status != IP_SUCCESS)
	{
		NosLib::Logging::CreateLog<char>(std::format("Mirror \"{}\" failed ping", hostnameString, reply->RoundTripTime), NosLib::Logging::Severity::Debug);
		return -1;
	}

	NosLib::Logging::CreateLog<char>(std::format("Mirror \"{}\" ping time: {}ms", hostnameString, reply->RoundTripTime), NosLib::Logging::Severity::Debug);

	IcmpCloseHandle(icmp_handle);
	return reply->RoundTripTime;
}

std::string ModDB::GetHostName(const std::string& downloadLink)
{
	size_t position = downloadLink.find("//");

	std::string out = downloadLink.substr(position +2);

	position = out.find('/');

	out = out.substr(0, position);

	NosLib::Logging::CreateLog<char>(std::format("mirror hostname: {}", out), NosLib::Logging::Severity::Debug);

	return out;
}