#pragma once

#include "..\EXTERNAL\httplib.h"

#include <NosLib\DynamicArray.hpp>
#include <NosLib\String.hpp>

#include <format>

#include "AccountToken.hpp"
#include "HTMLParsing.hpp"
#include "GlobalVariables.hpp"

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

namespace ModPackMaker
{
	std::wstring InstallPath;

	std::wstring ModDirectory = L"mods\\";
	std::wstring ExtractedDirectory = L"extracted\\";
	std::wstring DownloadedDirectory = L"downloads\\";

	/* Sub directories that are inside each mod folder */
	const std::wstring subdirectories[] = {L"gamedata\\", L"fomod\\", L"db\\", L"tools\\", L"appdata\\"};

	struct HostPath
	{
		std::string Host;
		std::string Path;

		HostPath() {}

		HostPath(const std::wstring& host, const std::wstring& path)
		{
			Host = NosLib::String::ConvertString<char, wchar_t>(host);
			Path = NosLib::String::ConvertString<char, wchar_t>(path);
		}

		HostPath(const std::string& host, const std::string& path)
		{
			Host = host;
			Path = path;
		}

		HostPath(const std::string& link)
		{
			int slashCount = 0;

			for (int i = 0; i < link.length(); i++)
			{
				if (slashCount == 3)
				{
					Host = link.substr(0, i - 1);
					Path = link.substr(i - 1);
					break;
				}

				if (link[i] == L'/')
				{
					slashCount++;
				}
			}
		}
	};

	class ModInfo
	{
	private:
		/// <summary>
		/// describes what type of "mod" it is
		/// </summary>
		enum class Type
		{
			Seperator,	/* separators, which don't contain any mods but help with organization */
			Standard,	/* normal mod, so the struct will contain all the necessary data */
			Custom,		/* custom/manually created "mod" object */
		};

		static inline int ModPrefixIndexCounter = 1;			/* a global trackers for the prefix number, GAMMA adds the mods index into the front of the file, only gets used on seperators and standard mods */
		static inline int ModCounter = 0;						/* a global mod counter, counts EVERY mod, this is for the loading screen so it knows how many there are */

		static inline bit7z::Bit7zLibrary lib = bit7z::Bit7zLibrary(L"7z.dll"); /* Load 7z.dll into a class */
		static inline bit7z::BitFileExtractor extractor = bit7z::BitFileExtractor(lib); /* create extractor object */

		Type ModType;									/* the mod type */
		int ModPrefixIndex;								/* the mod index, that will be used in the folder name */
		HostPath Link;									/* the download link, will be used to download */
		NosLib::DynamicArray<std::string> InsidePaths;	/* an array of the inner paths (incase there is many) */
		std::string CreatorName;						/* the creator name (used in folder name) */
		std::string OutName;							/* the main folder name (use in folder name) */
		std::string OriginalLink;						/* original mod link (I don't know why its there but I'll parse it anyway) */

		std::string LeftOver;							/* any left over data */

		std::string FileExtension;						/* extension of the downloaded file (Gets set at download time) */
		std::string OutPath;							/* This is Custom modtype only, it defines were to copy the files to */
		bool UseInstallPath = true;						/* If mod should include mod path when installing (ONLY FOR CUSTOM) */

		/* LOADING SCREEN VARIABLES */
		int ModIndex;						/* mods index, used for loading screen */
		static inline bool Parsed = false;	/* if the installer has parsed the modpack file yet */

		/// <summary>
		/// Needs to be run by all the constructors, initializes for loading screen
		/// </summary>
		void InitializeModInfo()
		{
			ModCounter++;
			ModIndex = ModCounter;
		}
	public:
		static inline NosLib::LoadingScreen* LoadingScreenObjectPointer;

		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoList;		/* A list of all mods */
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modErrorList;	/* a list of all errors */

	#pragma region constructors
		/// <summary>
		/// Seperator constructor, only has a name and index
		/// </summary>
		/// <param name="outName">- seperator name</param>
		ModInfo(const std::string& outName)
		{
			ModPrefixIndex = ModPrefixIndexCounter;
			ModPrefixIndexCounter++;

			OutName = outName;
			ModType = Type::Seperator;

			InitializeModInfo();
		}

		/// <summary>
		/// Standard Mod Constructor, has all the data
		/// </summary>
		/// <param name="link"> -the download link, will be used to download</param>
		/// <param name="insidePaths">- an array of the inner paths (incase there is many)</param>
		/// <param name="creatorName">- the creator name (used in folder name)</param>
		/// <param name="outName">- the main folder name (use in folder name)</param>
		/// <param name="originalLink">- original mod link (I don't know why its there but I'll parse it anyway)</param>
		/// <param name="leftOver">- Any left over data</param>
		ModInfo(const std::string& link, NosLib::DynamicArray<std::string>& insidePaths, const std::string& creatorName, const std::string& outName, const std::string& originalLink, const std::string& leftOver)
		{
			ModPrefixIndex = ModPrefixIndexCounter;
			ModPrefixIndexCounter++;

			Link = HostPath(link);
			InsidePaths << insidePaths;
			CreatorName = creatorName;
			OutName = outName;
			OriginalLink = originalLink;
			LeftOver = leftOver;
			ModType = Type::Standard;

			InitializeModInfo();
		}

		/// <summary>
		/// Custom Mod Constructor, can output anywhere
		/// </summary>
		/// <param name="link"> - the download link, will be used to download</param>
		/// <param name="insidePaths">- an array of the inner paths (incase there is many) I-Value</param>
		/// <param name="OutPath">- where to copy the extracted data to</param>
		/// <param name="outName">- what to name the file</param>
		/// <param name="useInstallPath">(default = true) - if it should add installPath string to the front of its paths</param>
		ModInfo(const std::string& link, NosLib::DynamicArray<std::string>& insidePaths, const std::string& outPath, const std::string& outName, const bool& useInstallPath = true)
		{
			Link = HostPath(link);
			InsidePaths << insidePaths;
			OutName = outName;
			OutPath = outPath;
			ModType = Type::Custom;

			UseInstallPath = useInstallPath;

			InitializeModInfo();
		}

		/// <summary>
		/// Custom Mod Constructor, can output anywhere
		/// </summary>
		/// <param name="link"> - the download link, will be used to download</param>
		/// <param name="insidePaths">- an array of the inner paths (incase there is many) R-Value</param>
		/// <param name="OutPath">- where to copy the extracted data to</param>
		/// <param name="outName">- what to name the file</param>
		/// <param name="useInstallPath">(default = true) - if it should add installPath string to the front of its paths</param>
		ModInfo(const std::string& link, NosLib::DynamicArray<std::string>&& insidePaths, const std::string& outPath, const std::string& outName, const bool& useInstallPath = true)
		{
			Link = HostPath(link);
			InsidePaths << insidePaths;
			OutName = outName;
			OutPath = outPath;
			ModType = Type::Custom;

			UseInstallPath = useInstallPath;

			InitializeModInfo();
		}
	#pragma endregion

		std::string GetFullFileName(const bool& withExtension)
		{
			switch (ModType)
			{
			case Type::Seperator:
				return std::format("{}- {}_separator", ModPrefixIndex, OutName);

			case Type::Standard:
				return std::vformat((withExtension ? "{}- {} {}{}" : "{}- {} {}"), std::make_format_args(ModPrefixIndex, OutName, CreatorName, FileExtension));

			case Type::Custom:
				return std::vformat((withExtension ? "{}{}" : "{}"), std::make_format_args(OutName, FileExtension));

			default:
				return "Unknown Mod Type";
			}
		}

		void ProcessMod()
		{
			/* do different things depending on the mod type */
			switch (ModType)
			{
			case ModPackMaker::ModInfo::Type::Seperator:
				SeparatorModProcess(); /* if separator, just create folder with special name */
				break;

			case ModPackMaker::ModInfo::Type::Standard:
				StandardModProcess(); /* if mod, then download, extract and the construct (copy all the inner paths to end file) the mod */
				break;

			case ModPackMaker::ModInfo::Type::Custom:
				CustomModProcess(); /* if custom, then download, extract and copy the files to the specified directory */
				break;

			default: /* default meaning it is some other type which hasn't been defined yet */
				LogError("Undefined Mod Type tried to be processed", __FUNCTION__);
				return;
			}
		}

	#pragma region Parsing
		/// <summary>
		/// takes in a filename for a modpackMaker and parses it fully
		/// </summary>
		/// <param name="modpackMakerFileName">(default = "modpack_maker_list.txt") - path/name of modpack Maker</param>
		/// <returns>a DynamicArray of ModInfo pointers (ModInfo*)</returns>
		static NosLib::DynamicArray<ModInfo*>* ModpackMakerFile_Parse(const std::string& modpackMakerFileName = "modpack_maker_list.txt")
		{
			/* open binary file stream of modpack maker list */
			std::ifstream modMakerFile(modpackMakerFileName, std::ios::binary);

			/* got through all lines in the file. each line is a new mod */
			std::string line;
			while (std::getline(modMakerFile, line))
			{
				/* append to array */
				modInfoList.Append(ParseLine(line));
			}

			Parsed = true;

			return &modInfoList;
		}

	private:
		/// <summary>
		/// Takes in a string and actually parses it, and puts it into an object
		/// </summary>
		/// <param name="line">- input line</param>
		/// <returns>pointer of ModInfo, containing parsed mod info</returns>
		static ModInfo* ParseLine(std::string& line)
		{
			/* create array, the file uses \t to separate info */
			NosLib::DynamicArray<std::string> wordArray(6, 2);

			/* split the line into the previous array */
			NosLib::String::Split<char>(&wordArray, line, '\t');

			/* go through all strings in the array and "reduce" them (take out spaces in front, behind and any duplicate spaces inbetween) */
			for (int i = 0; i <= wordArray.GetLastArrayIndex(); i++)
			{
				wordArray[i] = NosLib::String::Reduce(wordArray[i]);
			}

			/* if there is only 1 object (so last index is 0), that means its a separator, use a different constructor */
			if (wordArray.GetLastArrayIndex() == 0)
			{
				return new ModInfo(wordArray[0]);
			}

			/* some mods have multiple inner paths that get combined, separate them into an array for easier processing */
			NosLib::DynamicArray<std::string> pathArray(5, 5);
			NosLib::String::Split<char>(&pathArray, wordArray[1], ':');

			bool hasRoot = false;

			/* go through all path strings in the array, and if any are equal to 0, that means it is root */
			for (int i = 0; i <= pathArray.GetLastArrayIndex(); i++)
			{
				if (pathArray[i] == "0" || pathArray[i] == "\\")
				{
					pathArray[i] = "\\";
					hasRoot = true;
				}
				else if (pathArray[i][0] != '\\')
				{
					pathArray[i].insert(0, "\\");
				}

				if (pathArray[i].back() != '\\')
				{
					pathArray[i].append("\\");
				}
			}

			if (!hasRoot)
			{
				pathArray.Insert("\\", 0);
			}

			/* finally, if it has gotten here, it means the current line is a normal mod, pass in all the info to the constructor */
			return new ModInfo(wordArray[0], pathArray, wordArray[2], wordArray[3], wordArray[4], wordArray[5]);
		}
	#pragma endregion

		float LastPercentageOnCurrentMod = 0.0f; /* tracks the last percentage on the file, incase overload without the percentage gets called */

		void UpdateLoadingScreen(const float& percentageOnCurrentMod, std::wstring status /* get copy so it can be modified */)
		{
			LastPercentageOnCurrentMod = percentageOnCurrentMod;

			float SpaceAmountDone = percentageOnCurrentMod * max(NosLib::Console::GetConsoleSize().Columns - 60, 20);

			/* if first character isn't new line, insert \n to front */
			if (status[0] != L'\n')
			{
				status.insert(0, L"\n");
			}
			/* if last character isn't new line, append \n to back */
			if (status.back() != L'\n')
			{
				status += L"\n";
			}

			LoadingScreenObjectPointer->UpdateKnownProgressBar((Parsed ? NosLib::Cast<float>(NosLib::Cast<float>(ModIndex)/ NosLib::Cast<float>(ModCounter)) : 0.0f), NosLib::String::Shorten(NosLib::LoadingScreen::GenerateProgressBar(percentageOnCurrentMod) + status + (Parsed ? std::format(L"mod {} out of {}", ModIndex, ModCounter) : L"Set Up Files, No Mod Count")));
		}

		void UpdateLoadingScreen(const std::wstring& status)
		{
			return UpdateLoadingScreen(LastPercentageOnCurrentMod, status);
		}

	#pragma region Mod Processing
		void DownloadMod(const std::string& downloadDirectory)
		{
			/* create client for the host */
			httplib::Client downloadClient(Link.Host);

			/* set properties */
			downloadClient.set_follow_location(false);
			downloadClient.set_keep_alive(false);
			downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

			/* create directories in order to prevent any errors */
			std::filesystem::create_directories(downloadDirectory);

			/* Decide the host type, there are different download steps for different websites */
			switch (DetermineHostType(Link.Host))
			{
			case HostType::ModDB:
				ModDBDownload(&downloadClient, this, downloadDirectory);
				break;

			case HostType::Github:
				GithubDownload(&downloadClient, this, downloadDirectory);
				break;

			case HostType::GoFile:
				GoFileDownload(&downloadClient, this, downloadDirectory);
				break;

			default:
				LogError("Undefined Mod Type tried to be processed", __FUNCTION__);
				return;
			}
		}

		void ExtractMod(const std::wstring& downloadDirectory, const std::wstring& extractDirectory)
		{
			/* create directories in order to prevent any errors */
			std::filesystem::create_directories(extractDirectory);

			/* extract into said directory */
			UpdateLoadingScreen(std::format(L"...extracting: {} into: {}", downloadDirectory + NosLib::String::ToWstring(GetFullFileName(true)), extractDirectory));
			extractor.extract(downloadDirectory + NosLib::String::ToWstring(GetFullFileName(true)), extractDirectory);
			UpdateLoadingScreen(std::format(L"extracted: {} into: {}", downloadDirectory + NosLib::String::ToWstring(GetFullFileName(true)), extractDirectory));
		}

		void LogError(const std::string& exceptionMessage, const std::string& functioName)
		{
			std::ofstream outLog("log.txt", std::ios::binary | std::ios::app);
			std::string logMessage = std::format("error in file \"{}\" in function \"{}\" with mod \"{}\" -> {}\n", __FILE__, functioName, GetFullFileName(true), exceptionMessage);
			outLog.write(logMessage.c_str(), logMessage.size());
			outLog.close();

			ModPackMaker::ModInfo::modErrorList.Append(this); /* add this mod to "failed" list */

			/* create "error mod list" file if there was any failed/unfinished mods */
			std::ofstream errorModListOutput(L"error mod list.txt", std::ios::binary | std::ios::app);

			std::string pathList;
			for (int i = 0; i <= InsidePaths.GetLastArrayIndex(); i++)
			{
				pathList += InsidePaths[i];
				if (i != InsidePaths.GetLastArrayIndex())
				{
					pathList += ":";
				}
			}

			std::string line = std::format("{}\t----\t{}\t{}\t{}\t{}\t{}\t{}\n", GetFullFileName(true), Link.Host + Link.Path, pathList, CreatorName, OutName, OriginalLink, LeftOver);

			errorModListOutput.write(line.c_str(), line.size());

			errorModListOutput.close();
		}

		void StandardModProcess()
		{
			std::string DownloadsOutDirectory = NosLib::String::ToString(InstallPath + DownloadedDirectory);
			DownloadMod(DownloadsOutDirectory);

			/* create path to extract into */
			std::wstring extractedOutDirectory = InstallPath + ExtractedDirectory + NosLib::String::ToWstring(GetFullFileName(false));
			ExtractMod(NosLib::String::ToWstring(DownloadsOutDirectory), extractedOutDirectory);

			UpdateLoadingScreen(L"...Copying files\n");
			/* for every "inner" path, go through and find the needed files */
			for (std::string path : InsidePaths)
			{
				/* root inner path, everything revolves around this */
				std::wstring rootFrom = (extractedOutDirectory + NosLib::String::ToWstring(path));
				std::wstring rootTo = (InstallPath + ModDirectory + NosLib::String::ToWstring(GetFullFileName(false)) + L"\\");

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
					LogError(ex.what(), __FUNCTION__);
				}
			}
			UpdateLoadingScreen(L"Finished Copying");

			UpdateLoadingScreen(L"...Cleaning up files");
			std::filesystem::remove_all(DownloadsOutDirectory + GetFullFileName(true));
			std::filesystem::remove_all(extractedOutDirectory);
			UpdateLoadingScreen(L"Finished Clean up");
		}

		void CustomModProcess()
		{
			std::string DownloadsOutDirectory = NosLib::String::ToString(InstallPath + DownloadedDirectory);
			DownloadMod(DownloadsOutDirectory);

			/* create path to extract into */
			std::wstring extractedOutDirectory = InstallPath + ExtractedDirectory + NosLib::String::ToWstring(GetFullFileName(false));
			ExtractMod(NosLib::String::ToWstring(DownloadsOutDirectory), extractedOutDirectory);

			UpdateLoadingScreen(L"...Copying files\n");
			/* for every "inner" path, go through and find the needed files */
			for (std::string path : InsidePaths)
			{
				/* root inner path, everything revolves around this */
				std::wstring rootFrom = (extractedOutDirectory + NosLib::String::ToWstring(path));
				std::wstring rootTo = ((UseInstallPath ? InstallPath : L"") + NosLib::String::ToWstring(OutPath));

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
					LogError(ex.what(), __FUNCTION__);
				}
			}
			UpdateLoadingScreen(L"Finished Copying");

			UpdateLoadingScreen(L"...Cleaning up files");
			std::filesystem::remove_all(DownloadsOutDirectory + GetFullFileName(true));
			std::filesystem::remove_all(extractedOutDirectory);
			UpdateLoadingScreen(L"Finished Clean up");
		}

		void SeparatorModProcess()
		{
			std::filesystem::create_directories(InstallPath + ModDirectory + NosLib::String::ToWstring(GetFullFileName(false)));
		}
	#pragma endregion

	#pragma region Downloading
		std::string GetFileExtensionFromHeader(const std::string& type)
		{
			if (type.find("application/zip") != std::string::npos)
			{
				return ".zip";
			}
			else if (type.find("application/x-7z-compressed") != std::string::npos)
			{
				return ".7z";
			}
			else if (type.find("application/x-rar-compressed") != std::string::npos || type.find("application/vnd.rar") != std::string::npos)
			{
				return ".rar";
			}

			return ".ERROR";
		}

		void GetAndSaveFile(httplib::Client* client, ModPackMaker::ModInfo* mod, const std::string& filepath, const std::string& pathOffsets = "")
		{
			std::ofstream DownloadFile;

			UpdateLoadingScreen(L"Starting Downloading...");

			httplib::Result res = client->Get(filepath,
				[&](const httplib::Response& response)
				{
					mod->FileExtension = GetFileExtensionFromHeader(response.headers.find("Content-Type")->second);

					if (response.headers.find("Transfer-Encoding")->second == "chunked")
					{
						UpdateLoadingScreen(L"Website is stupid af and is using \"chunked\" for \"Transfer-Encoding\"\nFILE IS DOWNLOADING BUT WON'T SHOW PROGRESS");
					}

					/* before start download, get "Content-Type" header tag to see the extensions, then open with the name+extension */
					DownloadFile.open(pathOffsets + mod->GetFullFileName(true), std::ios::binary | std::ios::trunc);
					return true; // return 'false' if you want to cancel the request.
				},
				[&](const char* data, size_t data_length)
				{
					/* write to file while downloading, this makes sure that it doesn't download to memory and then write */
					DownloadFile.write(data, data_length);
					return true;
				},
				[&](uint64_t len, uint64_t total)
				{
					UpdateLoadingScreen(((float)len / (float)total), std::format(L"Downloading {}", NosLib::String::ToWstring(GetFullFileName(true))));

					return true; // return 'false' if you want to cancel the request.
				});

			if (!res)
			{
				std::string errorMessage;
				switch (res.error())
				{
				case httplib::Error::Success:
					errorMessage = "Success";
					break;
				case httplib::Error::Unknown:
					errorMessage = "Unknown";
					break;
				case httplib::Error::Connection:
					errorMessage = "Connection";
					break;
				case httplib::Error::BindIPAddress:
					errorMessage = "Bind IP Address";
					break;
				case httplib::Error::Read:
					errorMessage = "Read ";
					break;
				case httplib::Error::Write:
					errorMessage = "Write";
					break;
				case httplib::Error::ExceedRedirectCount:
					errorMessage = "Exceed Redirect Count";
					break;
				case httplib::Error::Canceled:
					errorMessage = "Cancelled";
					break;
				case httplib::Error::SSLConnection:
					errorMessage = "SSL Connection";
					break;
				case httplib::Error::SSLLoadingCerts:
					errorMessage = "SSL Loading Certs";
					break;
				case httplib::Error::SSLServerVerification:
					errorMessage = "SSL Server Verification";
					break;
				case httplib::Error::UnsupportedMultipartBoundaryChars:
					errorMessage = "Unsupported Multipart Boundary Chars";
					break;
				case httplib::Error::Compression:
					errorMessage = "Compression";
					break;
				case httplib::Error::ConnectionTimeout:
					errorMessage = "Connection Timeout";
					break;
				}

				LogError(std::format("error code: {}\n", errorMessage), __FUNCTION__);
			}

			DownloadFile.close();

			return;
		}

		enum class HostType
		{
			ModDB,
			Github,
			GoFile,
			Unknown,
		};

		HostType DetermineHostType(const std::string& hostName)
		{
			if (hostName.find("moddb") != std::string::npos)
			{
				return HostType::ModDB;
			}
			else if (hostName.find("github") != std::string::npos)
			{
				return HostType::Github;
			}
			else if (hostName.find("gofile") != std::string::npos)
			{
				return HostType::GoFile;
			}

			return HostType::Unknown;
		}

		void ModDBDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
		{
			ModDBParsing::HTMLParseReturn LinkOutput = ModDBParsing::ParseHtmlForLink(downloadClient->Get(mod->Link.Path)->body);

			if (LinkOutput.Link == "No Link Found")
			{
				return;
			}

			if constexpr (Global::verbose)
			{
				downloadClient->set_logger(&LoggingFunction);
			}
			downloadClient->set_follow_location(true);
			GetAndSaveFile(downloadClient, mod, LinkOutput.Link, pathOffsets);
		}

		void GithubDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
		{
			if constexpr (Global::verbose)
			{
				downloadClient->set_logger(&LoggingFunction);
			}
			downloadClient->set_follow_location(true);
			GetAndSaveFile(downloadClient, mod, mod->Link.Path, pathOffsets);
		}

		void GoFileDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
		{
			if (AccountToken::AccountToken.empty())
			{

				UpdateLoadingScreen(L"Getting GoLink Token");
				AccountToken::GetAccountToken();
				UpdateLoadingScreen(std::format(L"Got Token \"{}\" and got authorized", NosLib::String::ToWstring(AccountToken::AccountToken)));
			}

			if constexpr (Global::verbose)
			{
				downloadClient->set_logger(&LoggingFunction);
			}
			downloadClient->set_follow_location(false);
			downloadClient->set_keep_alive(false);
			downloadClient->set_default_headers({
				{"Cookie", std::format("accountToken={}", AccountToken::AccountToken)},
				{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});
			GetAndSaveFile(downloadClient, mod, mod->Link.Path, pathOffsets);
		}
	#pragma endregion
	};
}
