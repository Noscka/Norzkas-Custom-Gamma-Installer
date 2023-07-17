#define CPPHTTPLIB_OPENSSL_SUPPORT
#define BIT7Z_AUTO_FORMAT
#define BIT7Z_REGEX_MATCHING

#include "EXTERNAL\httplib.h"
#include "EXTERNAL\json.hpp"

#include <NosLib/Console.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include <windows.h>
#include <conio.h>
#include <fstream>
#include <format>
#include <filesystem>

#include "Headers\HTTPLogging.hpp"
#include "Headers\AccountToken.hpp"
#include "Headers\ModMakerParsing.hpp"
#include "Headers\HTMLParsing.h"
#include "Headers\HTTPOperations.hpp"

const std::wstring ModDirectory = L"mods\\";
const std::wstring ExtractedDirectory = L"extracted\\";
const std::wstring DownloadedDirectory = L"downloads\\";

 /* Sub directories that are inside each mod folder */
const std::wstring subdirectories[] = {L"gamedata\\", L"fomod\\", L"db\\", L"tools\\", L"appdata\\"};

void copyIfExists(const std::wstring& from, const std::wstring& to)
{
	/* if DOESN'T exist, go to next path (this is to remove 1 layer of nesting */
	if (!std::filesystem::exists(from))
	{
		return;
	}
	
	/* repeat the previous step but this time with "fomod" sub directory */
	std::filesystem::create_directories(to);
	std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
}

void StandardModProcess(ModPackMaker::ModInfo* mod, const bit7z::BitFileExtractor& extractor)
{
	/* create client for the host */
	httplib::Client downloadClient(mod->Link.Host);

	/* set properties */
	downloadClient.set_follow_location(false);
	downloadClient.set_keep_alive(false);
	downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	/* Decide the host type, there are different download steps for different websites */
	switch (DetermineHostType(mod->Link.Host))
	{
	case HostType::ModDB:
		ModDBDownload(&downloadClient, mod, NosLib::String::ToString(DownloadedDirectory));
		break;

	case HostType::Github:
		GithubDownload(&downloadClient, mod, NosLib::String::ToString(DownloadedDirectory));
		break;

	default:
		wprintf(L"Unknown host type\ncontinuing to next\n");
		return;
	}

	/* create path to extract into */
	std::wstring extractedOutDirectory = ExtractedDirectory + NosLib::String::ToWstring(mod->GetFullFileName(false));

	/* create directories in order to prevent any errors */
	std::filesystem::create_directories(extractedOutDirectory);

	/* extract into said directory */
	extractor.extract(NosLib::String::ToString(DownloadedDirectory)+ mod->GetFullFileName(true), NosLib::String::ToString(extractedOutDirectory));

	/* for every "inner" path, go through and find the needed files */
	for (std::string path : mod->InsidePaths)
	{
		/* root inner path, everything revolves around this */
		std::wstring rootFrom = (extractedOutDirectory + NosLib::String::ToWstring(path));
		std::wstring rootTo = (ModDirectory + NosLib::String::ToWstring(mod->GetFullFileName(false)) + L"\\");

		/* create directories to prevent errors */
		std::filesystem::create_directories(rootTo);

		try
		{
			/* copy all files from root (any readme/extra info files) */
			std::filesystem::copy(rootFrom, rootTo, std::filesystem::copy_options::overwrite_existing);

			for (std::wstring subdirectory : subdirectories)
			{
				copyIfExists(rootFrom + subdirectory, rootTo + subdirectory);
			}
		}
		catch (const std::exception& ex)
		{
			std::ofstream outLog("log.txt", std::ios::binary | std::ios::app);
			std::string logMessage = std::format("error in file \"{}\" at line \"{}\" with mod \"{}\" -> {}\n",__FILE__, __LINE__,mod->GetFullFileName(true), ex.what());
			outLog.write(logMessage.c_str(), logMessage.size());
			outLog.close();

			std::cerr << logMessage << std::endl;

			ModPackMaker::ModInfo::modFailedList.Append(mod); /* add this mod to "failed" list */

			/* create "failed mod list" file if there was any failed/unfinished mods */
			std::ofstream failedMostListOutput(L"failed mod list.txt", std::ios::binary | std::ios::app);

			std::string pathList;
			for (int i = 0; i <= mod->InsidePaths.GetLastArrayIndex(); i++)
			{
				pathList += mod->InsidePaths[i];
				if (i != mod->InsidePaths.GetLastArrayIndex())
				{
					pathList += ":";
				}
			}

			std::string line = std::format("{}\t{}\t{}\t{}\t{}\t{}\t----\t{}", mod->Link.Host + mod->Link.Path, pathList, mod->CreatorName, mod->OutName, mod->OriginalLink, mod->LeftOver, mod->GetFullFileName(true));

			failedMostListOutput.write(line.c_str(), line.size());

			failedMostListOutput.close();
		}
	}
}

void SeparatorModProcess(ModPackMaker::ModInfo* mod)
{
	std::filesystem::create_directories(ModDirectory + NosLib::String::ToWstring(mod->GetFullFileName(false)));
}

void CustomModProcess(ModPackMaker::ModInfo* mod)
{
	/* create client for the host */
	httplib::Client downloadClient(mod->Link.Host);

	/* set properties */
	downloadClient.set_follow_location(false);
	downloadClient.set_keep_alive(false);
	downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	/* Decide the host type, there are different download steps for different websites */
	switch (DetermineHostType(mod->Link.Host))
	{
	case HostType::ModDB:
		ModDBDownload(&downloadClient, mod, NosLib::String::ToString(DownloadedDirectory));
		break;

	case HostType::Github:
		GithubDownload(&downloadClient, mod, NosLib::String::ToString(DownloadedDirectory));
		break;

	default:
		wprintf(L"Unknown host type\ncontinuing to next\n");
		return;
	}

	/* create path to extract into */
	std::wstring extractedOutDirectory = ExtractedDirectory + NosLib::String::ToWstring(mod->GetFullFileName(false));

	/* create directories in order to prevent any errors */
	std::filesystem::create_directories(extractedOutDirectory);

	/* extract into said directory */
	bit7z::BitFileExtractor extractor(bit7z::Bit7zLibrary("7z.dll")); /* Need a custom object, for some reason it crashes otherwise */
	extractor.extract(NosLib::String::ToString(DownloadedDirectory) + mod->GetFullFileName(true), NosLib::String::ToString(extractedOutDirectory));

	/* for every "inner" path, go through and find the needed files */
	for (std::string path : mod->InsidePaths)
	{
		/* root inner path, everything revolves around this */
		std::wstring rootFrom = (extractedOutDirectory + NosLib::String::ToWstring(path));
		std::wstring rootTo = NosLib::String::ToWstring(mod->OutPath);

		/* create directories to prevent errors */
		std::filesystem::create_directories(rootTo);

		try
		{
			/* copy all files from root (any readme/extra info files) */
			std::filesystem::copy(rootFrom, rootTo, std::filesystem::copy_options::overwrite_existing);

			copyIfExists(rootFrom, rootTo);
		}
		catch (const std::exception& ex)
		{
			std::ofstream outLog("log.txt", std::ios::binary | std::ios::app);
			std::string logMessage = std::format("error in file \"{}\" at line \"{}\" with mod \"{}\" -> {}\n", __FILE__, __LINE__, mod->GetFullFileName(true), ex.what());
			outLog.write(logMessage.c_str(), logMessage.size());
			outLog.close();

			std::cerr << logMessage << std::endl;

			ModPackMaker::ModInfo::modFailedList.Append(mod); /* add this mod to "failed" list */

			/* create "failed mod list" file if there was any failed/unfinished mods */
			std::ofstream failedMostListOutput(L"failed mod list.txt", std::ios::binary | std::ios::app);

			std::string pathList;
			for (int i = 0; i <= mod->InsidePaths.GetLastArrayIndex(); i++)
			{
				pathList += mod->InsidePaths[i];
				if (i != mod->InsidePaths.GetLastArrayIndex())
				{
					pathList += ":";
				}
			}

			std::string line = std::format("{}\t{}\t{}\t{}\t{}\t{}\t----\t{}\n", mod->Link.Host + mod->Link.Path, pathList, mod->CreatorName, mod->OutName, mod->OriginalLink, mod->LeftOver, mod->GetFullFileName(true));

			failedMostListOutput.write(line.c_str(), line.size());

			failedMostListOutput.close();
		}
	}
}

int main()
{
	NosLib::Console::InitializeModifiers::EnableUnicode();
	NosLib::Console::InitializeModifiers::EnableANSI();
	NosLib::Console::InitializeModifiers::BeatifyConsole<wchar_t>(L"Norzka's Gamma Installer");
	NosLib::Console::InitializeModifiers::InitializeEventHandler();


	/* parse modpack maker file, put it into global static array */
	ModPackMaker::ModpackMakerFile_Parse("modpack_maker_list.txt");

	/* create extractor object */
	bit7z::BitFileExtractor extractor(bit7z::Bit7zLibrary("7z.dll"));

	/* create directory for downloaded files */
	std::filesystem::create_directories(DownloadedDirectory);
	std::filesystem::create_directories(ModDirectory);

	/* go through all mods in global static array */
	for (ModPackMaker::ModInfo* mod : ModPackMaker::ModInfo::modInfoList)
	{
		/* do different things depending on the mod type */
		switch (mod->ModType)
		{
		case ModPackMaker::ModInfo::Type::Seperator:
			SeparatorModProcess(mod); /* if separator, just create folder with special name */
			break;

		case ModPackMaker::ModInfo::Type::Standard:
			StandardModProcess(mod, extractor); /* if mod, then download, extract and the construct (copy all the inner paths to end file) the mod */
			break;

		default: /* default meaning it is some other type which hasn't been defined yet */
			wprintf(L"Mod type not yet define\ncontinuing to next\n");
			continue;
		}
	}

	NosLib::DynamicArray<std::string> innerDefinitionPaths;
	innerDefinitionPaths.Append("\\Stalker_GAMMA-main\\G.A.M.M.A\\modpack_addons");

	ModPackMaker::ModInfo definitionFile(std::string("https://github.com") + "/Grokitach/Stalker_GAMMA/archive/refs/heads/main.zip", innerDefinitionPaths, NosLib::String::ToString(ModDirectory), "G.A.M.M.A. modpack definition");

	CustomModProcess(&definitionFile);

	wprintf(L"Press any button to continue"); _getch();
	return 0;

	/* LATER SETUP */

	AccountToken::GetAccountToken();

	httplib::Client downloader("https://file120.gofile.io");

	downloader.set_follow_location(false);
	downloader.set_logger(&LoggingFunction);
	downloader.set_keep_alive(false);
	downloader.set_default_headers({
		{"Cookie", std::format("accountToken={}", AccountToken::AccountToken)},
		{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

	//GetAndSaveFile(&downloader, "/download/86137f5a-201e-41ff-86b7-47fa42ba2d11/GAMMA%20RC3.7z", "GAMMA RC3.7z");

	wprintf(L"Press any button to continue"); _getch();
    return 0;
}