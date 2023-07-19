#pragma once

#include "..\EXTERNAL\httplib.h"

#include <NosLib/DynamicArray.hpp>
#include <format>
#include "String.h"
#include "HTMLParsing.h"

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
	const std::wstring ModDirectory = L"mods\\";
	const std::wstring ExtractedDirectory = L"extracted\\";
	const std::wstring DownloadedDirectory = L"downloads\\";

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
	};

	HostPath GetHostPath(const std::string& link)
	{
		int slashCount = 0;

		HostPath output;

		for (int i = 0; i < link.length(); i++)
		{
			if (slashCount == 3)
			{
				output = HostPath(link.substr(0, i - 1), link.substr(i - 1));
				break;
			}

			if (link[i] == L'/')
			{
				slashCount++;
			}
		}

		return output;
	}

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

		static inline int CurrentModIndex = 1;			/* a global counter, the normal GAMMA installer numbers all the mods depending on where they are in the install file. keep a global track */

		Type ModType;									/* the mod type */
		int ModIndex;									/* the mod index, that will be used in the folder name */
		HostPath Link;									/* the download link, will be used to download */
		NosLib::DynamicArray<std::string> InsidePaths;	/* an array of the inner paths (incase there is many) */
		std::string CreatorName;						/* the creator name (used in folder name) */
		std::string OutName;							/* the main folder name (use in folder name) */
		std::string OriginalLink;						/* original mod link (I don't know why its there but I'll parse it anyway) */

		std::string LeftOver;							/* any left over data */

		std::string FileExtension;						/* extension of the downloaded file (Gets set at download time) */
		std::string OutPath;							/* This is Custom modtype only, it defines were to copy the files to */

	public:
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoList;		/* A list of all mods */
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modFailedList;	/* a list of all failed mods (so even errors) */

	#pragma region constructors
		/// <summary>
		/// Seperator constructor, only has a name and index
		/// </summary>
		/// <param name="outName">- seperator name</param>
		ModInfo(const std::string& outName)
		{
			ModIndex = CurrentModIndex;
			CurrentModIndex++;

			OutName = outName;
			ModType = Type::Seperator;
		}

		/// <summary>
		/// Standard Mod Constructor, has all the data
		/// </summary>
		/// <param name="link"> -the download link, will be used to download</param>
		/// <param name="insidePaths">- an array of the inner paths (incase there is many)</param>
		/// <param name="creatorName">- the creator name (used in folder name)</param>
		/// <param name="outName">- the main folder name (use in folder name)</param>
		/// <param name="originalLink">- </param>
		/// <param name="leftOver">- original mod link (I don't know why its there but I'll parse it anyway)</param>
		ModInfo(const std::string& link, NosLib::DynamicArray<std::string>& insidePaths, const std::string& creatorName, const std::string& outName, const std::string& originalLink, const std::string& leftOver)
		{
			ModIndex = CurrentModIndex;
			CurrentModIndex++;

			Link = GetHostPath(link);
			InsidePaths << insidePaths;
			CreatorName = creatorName;
			OutName = outName;
			OriginalLink = originalLink;
			LeftOver = leftOver;
			ModType = Type::Standard;
		}

		ModInfo(const std::string& link, NosLib::DynamicArray<std::string>& insidePaths, const std::string& outPath, const std::string& outName)
		{
			Link = GetHostPath(link);
			InsidePaths << insidePaths;
			OutName = outName;
			OutPath = outPath;
			ModType = Type::Custom;
		}
	#pragma endregion

		std::string GetFullFileName(const bool& withExtension)
		{
			switch (ModType)
			{
			case Type::Seperator:
				return std::format("{}- {}_separator", ModIndex, OutName);

			case Type::Standard:
				if (withExtension)
				{
					return std::format("{}- {} {}{}", ModIndex, OutName, CreatorName, FileExtension);
				}
				/* ELSE */
				return std::format("{}- {} {}", ModIndex, OutName, CreatorName);

			case Type::Custom:
				if (withExtension)
				{
					return std::format("{}{}", OutName, FileExtension);
				}
				/* ELSE */
				return std::format("{}", OutName);

			default:
				return "Unknown Mod Type";
			}
		}

		void ProcessMod()
		{
			/* create extractor object */
			bit7z::BitFileExtractor extractor(bit7z::Bit7zLibrary("7z.dll"));

			/* do different things depending on the mod type */
			switch (ModType)
			{
			case ModPackMaker::ModInfo::Type::Seperator:
				SeparatorModProcess(); /* if separator, just create folder with special name */
				break;

			case ModPackMaker::ModInfo::Type::Standard:
				StandardModProcess(extractor); /* if mod, then download, extract and the construct (copy all the inner paths to end file) the mod */
				break;

			case ModPackMaker::ModInfo::Type::Custom:
				CustomModProcess(); /* if custom, then download, extract and copy the files to the specified directory */
				break;

			default: /* default meaning it is some other type which hasn't been defined yet */
				wprintf(L"Mod type not yet define\ncontinuing to next\n");
				return;
			}
		}

	#pragma region Parsing
		/// <summary>
		/// takes in a filename for a modpackMaker and parses it fully
		/// </summary>
		/// <param name="modpackMakerFileName">- path/name of modpack Maker</param>
		/// <returns>a DynamicArray of ModInfo pointers (ModInfo*)</returns>
		static NosLib::DynamicArray<ModInfo*>* ModpackMakerFile_Parse(const std::string& modpackMakerFileName)
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
				wordArray[i] = CustomStrings::reduce(wordArray[i]);
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

	#pragma region Mod Processing
		void StandardModProcess(const bit7z::BitFileExtractor& extractor)
		{
			/* create client for the host */
			httplib::Client downloadClient(Link.Host);

			/* set properties */
			downloadClient.set_follow_location(false);
			downloadClient.set_keep_alive(false);
			downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

			/* Decide the host type, there are different download steps for different websites */
			switch (DetermineHostType(Link.Host))
			{
			case HostType::ModDB:
				ModDBDownload(&downloadClient, this, NosLib::String::ToString(DownloadedDirectory));
				break;

			case HostType::Github:
				GithubDownload(&downloadClient, this, NosLib::String::ToString(DownloadedDirectory));
				break;

			default:
				wprintf(L"Unknown host type\ncontinuing to next\n");
				return;
			}

			/* create path to extract into */
			std::wstring extractedOutDirectory = ExtractedDirectory + NosLib::String::ToWstring(GetFullFileName(false));

			/* create directories in order to prevent any errors */
			std::filesystem::create_directories(extractedOutDirectory);

			/* extract into said directory */
			//bit7z::BitFileExtractor extractor(bit7z::Bit7zLibrary("7z.dll")); /* Need a custom object, for some reason it crashes otherwise */
			extractor.extract(NosLib::String::ToString(DownloadedDirectory) + GetFullFileName(true), NosLib::String::ToString(extractedOutDirectory));
			wprintf(std::format(L"extracted: {} into: {}\n", NosLib::String::ToWstring(GetFullFileName(true)), extractedOutDirectory).c_str());

			/* for every "inner" path, go through and find the needed files */
			for (std::string path : InsidePaths)
			{
				/* root inner path, everything revolves around this */
				std::wstring rootFrom = (extractedOutDirectory + NosLib::String::ToWstring(path));
				std::wstring rootTo = (ModDirectory + NosLib::String::ToWstring(GetFullFileName(false)) + L"\\");

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
					std::string logMessage = std::format("error in file \"{}\" at line \"{}\" with mod \"{}\" -> {}\n", __FILE__, __LINE__, GetFullFileName(true), ex.what());
					outLog.write(logMessage.c_str(), logMessage.size());
					outLog.close();

					std::cerr << logMessage << std::endl;

					ModPackMaker::ModInfo::modFailedList.Append(this); /* add this mod to "failed" list */

					/* create "failed mod list" file if there was any failed/unfinished mods */
					std::ofstream failedMostListOutput(L"failed mod list.txt", std::ios::binary | std::ios::app);

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

					failedMostListOutput.write(line.c_str(), line.size());

					failedMostListOutput.close();
				}
			}
		}

		void CustomModProcess()
		{
			/* create client for the host */
			httplib::Client downloadClient(Link.Host);

			/* set properties */
			downloadClient.set_follow_location(false);
			downloadClient.set_keep_alive(true);
			downloadClient.set_default_headers({{"User-Agent", "Norzka-Gamma-Installer (cpp-httplib)"}});

			/* Decide the host type, there are different download steps for different websites */
			switch (DetermineHostType(Link.Host))
			{
			case HostType::ModDB:
				ModDBDownload(&downloadClient, this, NosLib::String::ToString(DownloadedDirectory));
				break;

			case HostType::Github:
				GithubDownload(&downloadClient, this, NosLib::String::ToString(DownloadedDirectory));
				break;

			default:
				wprintf(L"Unknown host type\ncontinuing to next\n");
				return;
			}

			/* create path to extract into */
			std::wstring extractedOutDirectory = ExtractedDirectory + NosLib::String::ToWstring(GetFullFileName(false));

			/* create directories in order to prevent any errors */
			std::filesystem::create_directories(extractedOutDirectory);

			/* extract into said directory */
			bit7z::BitFileExtractor extractor(bit7z::Bit7zLibrary("7z.dll")); /* Need a custom object, for some reason it crashes otherwise */
			extractor.extract(NosLib::String::ToString(DownloadedDirectory) + GetFullFileName(true), NosLib::String::ToString(extractedOutDirectory));
			wprintf(std::format(L"extracted: {} into: {}\n", NosLib::String::ToWstring(GetFullFileName(true)), extractedOutDirectory).c_str());

			/* for every "inner" path, go through and find the needed files */
			for (std::string path : InsidePaths)
			{
				/* root inner path, everything revolves around this */
				std::wstring rootFrom = (extractedOutDirectory + NosLib::String::ToWstring(path));
				std::wstring rootTo = NosLib::String::ToWstring(OutPath);

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
					std::string logMessage = std::format("error in file \"{}\" at line \"{}\" with mod \"{}\" -> {}\n", __FILE__, __LINE__, GetFullFileName(true), ex.what());
					outLog.write(logMessage.c_str(), logMessage.size());
					outLog.close();

					std::cerr << logMessage << std::endl;

					ModPackMaker::ModInfo::modFailedList.Append(this); /* add this mod to "failed" list */

					/* create "failed mod list" file if there was any failed/unfinished mods */
					std::ofstream failedMostListOutput(L"failed mod list.txt", std::ios::binary | std::ios::app);

					std::string pathList;
					for (int i = 0; i <= InsidePaths.GetLastArrayIndex(); i++)
					{
						pathList += InsidePaths[i];
						if (i != InsidePaths.GetLastArrayIndex())
						{
							pathList += ":";
						}
					}

					std::string line = std::format("{}\t{}\t{}\t{}\t{}\t{}\t----\t{}\n", Link.Host + Link.Path, pathList, CreatorName, OutName, OriginalLink, LeftOver, GetFullFileName(true));

					failedMostListOutput.write(line.c_str(), line.size());

					failedMostListOutput.close();
				}
			}
		}

		void SeparatorModProcess()
		{
			std::filesystem::create_directories(ModDirectory + NosLib::String::ToWstring(GetFullFileName(false)));
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

			httplib::Result res = client->Get(filepath,
				[&](const httplib::Response& response)
				{
					mod->FileExtension = GetFileExtensionFromHeader(response.headers.find("Content-Type")->second);
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
				[](uint64_t len, uint64_t total)
				{
					/* TEMP: Just so it doesn't slow down downloads just because it is spamming the console */
					if (len % 431 != 0)
					{
						return true;
					}

					wprintf(L"%lld / %lld bytes => %d%% complete\n", len, total, (int)(len * 100 / total));
					return true; // return 'false' if you want to cancel the request.
				});

			PrintServerResponse(res);

			DownloadFile.close();

			return;
		}

		enum class HostType
		{
			ModDB,
			Github,
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

			return HostType::Unknown;
		}

		void ModDBDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
		{
			ModDBParsing::HTMLParseReturn LinkOutput = ModDBParsing::ParseHtmlForLink(downloadClient->Get(mod->Link.Path)->body);

			if (LinkOutput.Link == "No Link Found")
			{
				return;
			}

			downloadClient->set_follow_location(true);
			//downloadClient->set_logger(&LoggingFunction);
			GetAndSaveFile(downloadClient, mod, LinkOutput.Link, pathOffsets);
		}

		void GithubDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::string& pathOffsets = "")
		{
			downloadClient->set_follow_location(true);
			//downloadClient->set_logger(&LoggingFunction);
			GetAndSaveFile(downloadClient, mod, mod->Link.Path, pathOffsets);
		}
	#pragma endregion
	};
}
