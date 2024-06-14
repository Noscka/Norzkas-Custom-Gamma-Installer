#pragma once

#include <NosLib\DynamicArray.hpp>
#include <NosLib\String.hpp>
#include <NosLib\HostPath.hpp>
#include <NosLib\HttpClient.hpp>
#include <NosLib\FileManagement.hpp>

#include <bit7z\bit7z.hpp>
#include <bit7z\bit7zlibrary.hpp>
#include <bit7z\bitfileextractor.hpp>

#include <format>
#include <source_location>

#include "AccountToken.hpp"
#include "HTMLParsing.hpp"
#include "Validation.hpp"
#include "InstallOptions.hpp"
#include "InstallManager.hpp"
#include "File.hpp"

inline void copyIfExists(const std::wstring& from, const std::wstring& to)
{
	/* if DOESN'T exist, go to next path (this is to remove 1 layer of nesting) */
	if (!std::filesystem::exists(from))
	{
		return;
	}

	/* if it does exist, copy the directory with all the subdirectories and folders */
	std::filesystem::create_directories(to);
	std::filesystem::copy(from, to, std::filesystem::copy_options::recursive | std::filesystem::copy_options::overwrite_existing);
}
namespace ModPackMaker
{
	inline std::wstring ModDirectory = L"mods\\";
	inline std::wstring ExtractedDirectory = L"extracted\\";
	inline std::wstring DownloadedDirectory = L"downloads\\";

	/* Sub directories that are inside each mod folder */
	inline NosLib::DynamicArray<std::wstring> ModSubDirectories = NosLib::DynamicArray<std::wstring>({ L"fomod\\" }) + Validation::StalkerSubDirectories.ObjectExclude(L"bin\\");

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

		static inline int ModPrefixIndexCounter = 1;			/* a global trackers for the prefix number, GAMMA adds the mods index into the front of the file, only gets used on separators and standard mods */
		static inline int ModCounter = 0;						/* a global mod counter, counts EVERY mod, this is for the loading screen so it knows how many there are */

		static inline bit7z::Bit7zLibrary lib = bit7z::Bit7zLibrary(L"7z.dll"); /* Load 7z.dll into a class */
		static inline bit7z::BitFileExtractor extractor = bit7z::BitFileExtractor(lib); /* create extractor object */

		Type ModType;									/* the mod type */
		int ModPrefixIndex;								/* the mod index, that will be used in the folder name */
		NosLib::HostPath Link;							/* the download link, will be used to download */
		NosLib::DynamicArray<std::wstring> InsidePaths;	/* an array of the inner paths (incase there is many) */
		std::wstring CreatorName;						/* the creator name (used in folder name) */
		std::wstring OutName;							/* the main folder name (use in folder name) */
		std::wstring OriginalLink;						/* original mod link (I don't know why its there but I'll parse it anyway) */

		File* FileObject;

		std::wstring FileExtension;						/* extension of the downloaded file (Gets set at download time) */
		std::wstring OutPath;							/* This is Custom modtype only, it defines were to copy the files to */
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
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoList;		/* A list of all mods */
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modErrorList;	/* a list of all errors */

#pragma region constructors
	/// <summary>
	/// Seperator constructor, only has a name and index
	/// </summary>
	/// <param name="outName">- seperator name</param>
		inline ModInfo(const std::wstring& outName)
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
		inline ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& creatorName, const std::wstring& outName, const std::wstring& originalLink)
		{
			ModPrefixIndex = ModPrefixIndexCounter;
			ModPrefixIndexCounter++;

			Link = NosLib::HostPath(link);
			InsidePaths << insidePaths;
			CreatorName = creatorName;
			OutName = outName;
			OriginalLink = originalLink;
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
		/// <param name="customExtension">(default = L"") - custom extension</param>
		inline ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& useInstallPath = true, const std::wstring& customExtension = L"")
		{
			Link = NosLib::HostPath(link);
			InsidePaths << insidePaths;
			OutName = outName;
			OutPath = outPath;
			ModType = Type::Custom;

			UseInstallPath = useInstallPath;
			FileExtension = customExtension;

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
		/// <param name="customExtension">(default = L"") - custom extension</param>
		inline ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>&& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& useInstallPath = true, const std::wstring& customExtension = L"")
		{
			Link = NosLib::HostPath(link);
			InsidePaths << insidePaths;
			OutName = outName;
			OutPath = outPath;
			ModType = Type::Custom;

			UseInstallPath = useInstallPath;
			FileExtension = customExtension;

			InitializeModInfo();
		}
#pragma endregion

		inline std::wstring GetFullFileName(const bool& withExtension)
		{
			switch (ModType)
			{
			case Type::Seperator:
				return std::format(L"{}- {}_separator", ModPrefixIndex, OutName);

			case Type::Standard:
				return std::vformat(std::wstring(withExtension ? L"{}- {} {}{}" : L"{}- {} {}"), std::make_wformat_args(ModPrefixIndex, OutName, CreatorName, FileExtension));

			case Type::Custom:
				return std::vformat(std::wstring(withExtension ? L"{}{}" : L"{}"), std::make_wformat_args(OutName, FileExtension));

			default:
				return L"Unknown Mod Type";
			}
		}

		inline void ProcessMod()
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
				LogError(L"Undefined Mod Type tried to be processed", std::source_location::current());
				return;
			}
		}

#pragma region Parsing
	/// <summary>
	/// takes in a filename for a modpackMaker and parses it fully
	/// </summary>
	/// <param name="modpackMakerFileName">(default = "modpack_maker_list.txt") - path/name of modpack Maker</param>
	/// <returns>a DynamicArray of ModInfo pointers (ModInfo*)</returns>
		static inline NosLib::DynamicArray<ModInfo*>* ModpackMakerFile_Parse(const std::wstring& modpackMakerFileName)
		{
			/* open binary file stream of modpack maker list */
			std::wifstream modMakerFile(modpackMakerFileName, std::ios::binary);

			/* got through all lines in the file. each line is a new mod */
			std::wstring line;
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
		static inline ModInfo* ParseLine(std::wstring& line)
		{
			/* create array, the file uses \t to separate info */
			NosLib::DynamicArray<std::wstring> wordArray(6, 2);

			/* split the line into the previous array */
			NosLib::String::Split<wchar_t>(&wordArray, line, '\t');

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
			NosLib::DynamicArray<std::wstring> pathArray(5, 5);
			NosLib::String::Split<wchar_t>(&pathArray, wordArray[1], ':');

			bool hasRoot = false;

			/* go through all path strings in the array, and if any are equal to 0, that means it is root */
			for (int i = 0; i <= pathArray.GetLastArrayIndex(); i++)
			{
				if (pathArray[i] == L"0" || pathArray[i] == L"\\")
				{
					pathArray[i] = L"\\";
					hasRoot = true;
				}
				else if (pathArray[i][0] != L'\\')
				{
					pathArray[i].insert(0, L"\\");
				}

				if (pathArray[i].back() != L'\\')
				{
					pathArray[i].append(L"\\");
				}
			}

			if (!hasRoot)
			{
				pathArray.Insert(L"\\", 0);
			}

			/* finally, if it has gotten here, it means the current line is a normal mod, pass in all the info to the constructor */
			return new ModInfo(wordArray[0], pathArray, wordArray[2], wordArray[3], wordArray[4]);
		}
#pragma endregion

		inline void UpdateLoadingScreen(std::wstring status)
		{
			InstallManager* instance = InstallManager::GetInstallManager();
			status.insert(0, (Parsed ?
							  std::format(L"Mod {} out of {}\n", ModIndex, ModCounter) :
							  L"Initial Setup\n"));
			instance->UpdateStatus(status);
		}

		inline void UpdateLoadingScreen(const int& percentageOnCurrentMod)
		{
			InstallManager* instance = InstallManager::GetInstallManager();

			instance->UpdateTotalProgress((ModIndex * 100) / ModCounter);
			instance->UpdateSingularProgress(percentageOnCurrentMod);
		}

		inline void UpdateLoadingScreen(const int& percentageOnCurrentMod, const std::wstring& status)
		{
			InstallManager* instance = InstallManager::GetInstallManager();

			UpdateLoadingScreen(percentageOnCurrentMod);
			UpdateLoadingScreen(status);
		}


#pragma region Mod Processing
		inline void DownloadMod(const std::wstring& downloadDirectory)
		{
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"Downloading {} To \"{}\"", GetFullFileName(true), downloadDirectory), NosLib::Logging::Severity::Info);

			/* create client for the host */
			httplib::Client downloadClient = NosLib::MakeClient(NosLib::String::ToString(Link.Host), true, "NCGI");

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
				LogError(L"Undefined Mod Type tried to be processed", std::source_location::current());
				return;
			}

			NosLib::Logging::CreateLog<wchar_t>(std::format(L"Downloaded {} To \"{}\"", GetFullFileName(true), downloadDirectory), NosLib::Logging::Severity::Info);
		}

		inline void ExtractMod(const std::wstring& downloadDirectory, const std::wstring& extractDirectory)
		{
			std::wstring filePath = downloadDirectory + GetFullFileName(true);

			/* create directories in order to prevent any errors */
			std::filesystem::create_directories(extractDirectory);

			/* extract into said directory */
			UpdateLoadingScreen(std::format(L"extracting \"{}\"", GetFullFileName(true)));
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"Extracting \"{}\" To \"{}\"", filePath, extractDirectory), NosLib::Logging::Severity::Info);
			try
			{
				extractor.extract(filePath, extractDirectory);
			}
			catch (const bit7z::BitException& ex)
			{
				std::wstring errorMessage;
				for (std::pair<std::wstring, std::error_code> entry : ex.failedFiles())
				{
					errorMessage += std::format(L"{} : {}\n", entry.first, NosLib::String::ToWstring(entry.second.message()));
				}

				errorMessage += NosLib::String::ToWstring(std::format("{}\n", ex.what()));
				NosLib::Logging::CreateLog<wchar_t>(errorMessage, NosLib::Logging::Severity::Error);
				return;
			}
			NosLib::Logging::CreateLog<wchar_t>(std::format(L"Extracted \"{}\" To \"{}\"", filePath, extractDirectory), NosLib::Logging::Severity::Info);
			UpdateLoadingScreen(std::format(L"extracted \"{}\"", GetFullFileName(true)));
		}

		inline void LogError(const std::wstring& errorMessage, const std::source_location& errorLocation)
		{
			std::wstring logMessage = std::format(L"{} :: {} :: {} | mod: \"{}\" -> {}\n",
												  NosLib::String::ToWstring(errorLocation.file_name()),
												  NosLib::String::ToWstring(errorLocation.function_name()),
												  errorLocation.line(),
												  GetFullFileName(true),
												  errorMessage);

			NosLib::Logging::CreateLog<wchar_t>(logMessage, NosLib::Logging::Severity::Error);

			ModPackMaker::ModInfo::modErrorList.Append(this); /* add this mod to "failed" list */
		}

		inline void StandardModProcess()
		{
			std::wstring DownloadsOutDirectory = InstallOptions::GammaInstallPath + DownloadedDirectory;
			DownloadMod(DownloadsOutDirectory);

			/* create path to extract into */
			std::wstring extractedOutDirectory = InstallOptions::GammaInstallPath + ExtractedDirectory + GetFullFileName(false);
			ExtractMod(DownloadsOutDirectory, extractedOutDirectory);

			UpdateLoadingScreen(L"...Copying files\n");
			/* for every "inner" path, go through and find the needed files */
			for (std::wstring path : InsidePaths)
			{
				/* root inner path, everything revolves around this */
				std::wstring rootFrom = (extractedOutDirectory + path);
				std::wstring rootTo = (InstallOptions::GammaInstallPath + ModDirectory + GetFullFileName(false) + L"\\");


				/* create directories to prevent errors */
				std::filesystem::create_directories(rootTo);

				try
				{
					/* copy all files from root (any readme/extra info files) */
					std::filesystem::copy(rootFrom, rootTo, std::filesystem::copy_options::overwrite_existing);

					for (std::wstring subdirectory : ModSubDirectories)
					{
						std::wstring subRootFrom = rootFrom + subdirectory;
						std::wstring subRootTo = rootTo + subdirectory;

						NosLib::Logging::CreateLog<wchar_t>(std::format(L"Copying \"{}\" To \"{}\"", subRootFrom, subRootTo), NosLib::Logging::Severity::Info);

						copyIfExists(subRootFrom, subRootTo);

						NosLib::Logging::CreateLog<wchar_t>(std::format(L"Copied \"{}\" To \"{}\"", subRootFrom, subRootTo), NosLib::Logging::Severity::Info);
					}
				}
				catch (const std::exception& ex)
				{
					LogError(NosLib::String::ToWstring(ex.what()), std::source_location::current());
				}
			}
			UpdateLoadingScreen(L"Finished Copying");

			UpdateLoadingScreen(L"...Cleaning up files");
			std::error_code ec;
			if (-1 == std::filesystem::remove_all(DownloadsOutDirectory + GetFullFileName(true), ec))
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove download File", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
			}

			if (-1 == std::filesystem::remove_all(extractedOutDirectory, ec))
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove extract directory", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
			}
			UpdateLoadingScreen(L"Finished Clean up");
		}

		inline void CustomModProcess()
		{
			std::wstring DownloadsOutDirectory = InstallOptions::GammaInstallPath + DownloadedDirectory;
			DownloadMod(DownloadsOutDirectory);

			/* create path to extract into */
			std::wstring extractedOutDirectory = InstallOptions::GammaInstallPath + ExtractedDirectory + GetFullFileName(false);
			ExtractMod(DownloadsOutDirectory, extractedOutDirectory);

			UpdateLoadingScreen(L"...Copying files\n");
			/* for every "inner" path, go through and find the needed files */
			for (std::wstring path : InsidePaths)
			{
				/* root inner path, everything revolves around this */
				std::wstring rootFrom = (extractedOutDirectory + path);
				std::wstring rootTo = (UseInstallPath ? InstallOptions::GammaInstallPath : L"") + OutPath;

				NosLib::Logging::CreateLog<wchar_t>(std::format(L"Copying \"{}\" To \"{}\"", rootFrom, rootTo), NosLib::Logging::Severity::Info);

				/* create directories to prevent errors */
				std::filesystem::create_directories(rootTo);

				try
				{
					/* copy all files from root (any readme/extra info files) */
					std::filesystem::copy(rootFrom, rootTo, std::filesystem::copy_options::overwrite_existing);

					copyIfExists(rootFrom, rootTo);
					NosLib::Logging::CreateLog<wchar_t>(std::format(L"Copied \"{}\" To \"{}\"", rootFrom, rootTo), NosLib::Logging::Severity::Info);
				}
				catch (const std::exception& ex)
				{
					LogError(NosLib::String::ToWstring(ex.what()), std::source_location::current());
				}
			}
			UpdateLoadingScreen(L"Finished Copying");

			UpdateLoadingScreen(L"...Cleaning up files");
			std::error_code ec;
			if (-1 == std::filesystem::remove_all(DownloadsOutDirectory + GetFullFileName(true), ec))
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove download File", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
			}

			if (-1 == std::filesystem::remove_all(extractedOutDirectory, ec))
			{
				NosLib::Logging::CreateLog<wchar_t>(std::format(L"error: \"{}\" When trying to remove extract directory", NosLib::String::ToWstring(ec.message())), NosLib::Logging::Severity::Error);
			}
			UpdateLoadingScreen(L"Finished Clean up");
		}

		inline void SeparatorModProcess()
		{
			std::filesystem::create_directories(InstallOptions::GammaInstallPath + ModDirectory + GetFullFileName(false));
		}
#pragma endregion

#pragma region Downloading
		inline std::wstring GetFileExtensionFromHeader(const std::string& type)
		{
			if (type.find("application/zip") != std::string::npos)
			{
				return L".zip";
			}
			else if (type.find("application/x-7z-compressed") != std::string::npos)
			{
				return L".7z";
			}
			else if (type.find("application/x-rar-compressed") != std::string::npos || type.find("application/vnd.rar") != std::string::npos)
			{
				return L".rar";
			}

			return L".ERROR";
		}

		inline void GetAndSaveFile(httplib::Client* client, ModPackMaker::ModInfo* mod, const std::wstring& filepath, const std::wstring& pathOffsets = L"")
		{
			std::ofstream DownloadFile;

			UpdateLoadingScreen(L"Starting Downloading...");

			httplib::Result res = client->Get(NosLib::String::ToString(filepath),
			[&](const httplib::Response& response)
			{
				if (mod->FileExtension.empty())
				{
					mod->FileExtension = GetFileExtensionFromHeader(response.headers.find("Content-Type")->second);
				}

				std::wstring statusText = std::format(L"Downloading \"{}\"", GetFullFileName(true));

				httplib::Headers::const_iterator itr = response.headers.find("Transfer-Encoding");
				if ((itr != response.headers.end() ? itr->second : "HEADER DOESN'T EXIST") == "chunked")
				{
					statusText += L" - Won't Show Progress Due to \"chunked\" Transfer-Encoding";
				}

				UpdateLoadingScreen(statusText);

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
				UpdateLoadingScreen((len * 100) / total);

				return true; // return 'false' if you want to cancel the request.
			});

			if (!res)
			{
				LogError(std::format(L"connection error code: {}\n", NosLib::String::ToWstring(httplib::to_string(res.error()))), std::source_location::current());
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

		inline HostType DetermineHostType(const std::wstring& hostName)
		{
			if (hostName.find(L"moddb") != std::wstring::npos)
			{
				return HostType::ModDB;
			}
			else if (hostName.find(L"github") != std::wstring::npos)
			{
				return HostType::Github;
			}
			else if (hostName.find(L"gofile") != std::wstring::npos)
			{
				return HostType::GoFile;
			}

			return HostType::Unknown;
		}

		inline void ModDBDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::wstring& pathOffsets = L"")
		{
			ModDBParsing::HTMLParseReturn LinkOutput = ModDBParsing::ParseHtmlForLink(downloadClient->Get(NosLib::String::ToString(mod->Link.Path))->body);

			if (LinkOutput.Link == "No Link Found")
			{
				return;
			}

			downloadClient->set_follow_location(true);
			GetAndSaveFile(downloadClient, mod, NosLib::String::ToWstring(LinkOutput.Link), pathOffsets);
		}

		inline void GithubDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::wstring& pathOffsets = L"")
		{
			downloadClient->set_follow_location(true);
			GetAndSaveFile(downloadClient, mod, mod->Link.Path, pathOffsets);
		}

		inline void GoFileDownload(httplib::Client* downloadClient, ModPackMaker::ModInfo* mod, const std::wstring& pathOffsets = L"")
		{
			if (AccountToken::AccountToken.empty())
			{
				UpdateLoadingScreen(L"Getting GoLink Token");
				AccountToken::GetAccountToken();
				UpdateLoadingScreen(std::format(L"Got Token \"{}\" and got authorized", NosLib::String::ToWstring(AccountToken::AccountToken)));
			}

			downloadClient->set_follow_location(false);
			downloadClient->set_keep_alive(true);
			downloadClient->set_default_headers({
				{"Cookie", std::format("accountToken={}", AccountToken::AccountToken)},
				{"Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,*/*;q=0.8"},
				{"Accept-Encoding", "gzip, deflate, br, zstd"},
				{"Referer", "https://gofile.io/"},
				{"User-Agent", "NCGI (cpp-httplib)"} });
			GetAndSaveFile(downloadClient, mod, mod->Link.Path, pathOffsets);
		}
#pragma endregion
	};
}
