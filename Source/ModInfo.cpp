#include "../Headers/ModInfo.hpp"

#include "../Headers/InstallOptions.hpp"
#include "../Headers/InstallManager.hpp"
#include "../Headers/ModProcessorThread.hpp"

void copyIfExists(const std::wstring& from, const std::wstring& to)
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
#pragma region constructors
/// <summary>
/// Seperator constructor, only has a name and index
/// </summary>
/// <param name="outName">- seperator name</param>
	ModInfo::ModInfo(const std::wstring& outName)
{
	ModPrefixIndex = ModPrefixIndexCounter;
	ModPrefixIndexCounter++;

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
/// <param name="originalLink">- original mod link (I don't know why its there but I'll parse it anyway)</param>
/// <param name="leftOver">- Any left over data</param>
ModInfo::ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& creatorName, const std::wstring& outName, const std::wstring& originalLink)
{
	ModPrefixIndex = ModPrefixIndexCounter;
	ModPrefixIndexCounter++;

	InsidePaths << insidePaths;
	CreatorName = creatorName;
	OutName = outName;
	OriginalLink = originalLink;
	ModType = Type::Standard;

	FileObject = File::RegisterFile(link, outName);
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
ModInfo::ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& useInstallPath, const std::wstring& customExtension)
{
	InsidePaths << insidePaths;
	OutName = outName;
	OutPath = outPath;
	ModType = Type::Custom;

	FileObject = File::RegisterFile(link, outName, customExtension);

	UseInstallPath = useInstallPath;
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
ModInfo::ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>&& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& useInstallPath, const std::wstring& customExtension)
{
	InsidePaths << insidePaths;
	OutName = outName;
	OutPath = outPath;
	ModType = Type::Custom;

	FileObject = File::RegisterFile(link, outName, customExtension);

	UseInstallPath = useInstallPath;
}
#pragma endregion

std::wstring ModInfo::GetFolderName()
{
	switch (ModType)
	{
	case Type::Seperator:
		return std::format(L"{}- {}_separator", ModPrefixIndex, OutName);

	case Type::Standard:
		return std::format(L"{}- {} {}", ModPrefixIndex, OutName, CreatorName);

	case Type::Custom:
		return std::format(L"{}", OutName);

	default:
		return L"Unknown Mod Type";
	}
}

ModInfo::WorkState ModInfo::GetModWorkState()
{
	std::lock_guard<std::mutex> lk(WorkStateMutex);

	/* If Started, return as inProgress too */
	if (FileObject != nullptr)
	{
		if (FileObject->CheckIfStarted())
		{
			WorkState::InProgress;
		}
	}

	return CurrentWorkState.load();
}

void ModInfo::ProcessMod(ModProcessorThread* processingThread)
{
	CurrentWorkState = WorkState::InProgress;
	ProcessingThread = processingThread;

	/* do different things depending on the mod type */
	switch (ModType)
	{
	case Type::Seperator:
		SeparatorModProcess(); /* if separator, just create folder with special name */
		break;

	case Type::Standard:
		StandardModProcess(); /* if mod, then download, extract and the construct (copy all the inner paths to end file) the mod */
		break;

	case Type::Custom:
		CustomModProcess(); /* if custom, then download, extract and copy the files to the specified directory */
		break;

	default: /* default meaning it is some other type which hasn't been defined yet */
		LogError(L"Undefined Mod Type tried to be processed", std::source_location::current());
		return;
	}

	if (FileObject != nullptr)
	{
		FileObject->Finished();
		FileObject = nullptr;
	}

	CurrentWorkState = WorkState::Completed;
	processingThread = nullptr;
}

#pragma region Parsing
/// <summary>
/// takes in a filename for a modpackMaker and parses it fully
/// </summary>
/// <param name="modpackMakerFileName">(default = "modpack_maker_list.txt") - path/name of modpack Maker</param>
/// <returns>a DynamicArray of ModInfo pointers (ModInfo*)</returns>
void ModInfo::ModpackMakerFile_Parse(const std::wstring& modpackMakerFileName)
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

	//Parsed = true;
}

/// <summary>
/// Takes in a string and actually parses it, and puts it into an object
/// </summary>
/// <param name="line">- input line</param>
/// <returns>pointer of ModInfo, containing parsed mod info</returns>
ModInfo* ModInfo::ParseLine(std::wstring& line)
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

void ModInfo::UpdateLoadingScreen(const std::wstring& status)
{
	if (ProcessingThread == nullptr)
	{
		return;
	}

	ProcessingThread->UpdateModStatus(status);
}

void ModInfo::UpdateLoadingScreen(const int& percentageOnCurrentMod)
{
	if (ProcessingThread == nullptr)
	{
		return;
	}

	ProcessingThread->UpdateModProgress(percentageOnCurrentMod);
}

void ModInfo::UpdateLoadingScreen(const int& percentageOnCurrentMod, const std::wstring& status)
{
	UpdateLoadingScreen(percentageOnCurrentMod);
	UpdateLoadingScreen(status);
}


#pragma region Mod Processing

void ModInfo::LogError(const std::wstring& errorMessage, const std::source_location& errorLocation)
{
	std::wstring logMessage = std::format(L"{} :: {} :: {} | mod: \"{}\" -> {}\n",
											NosLib::String::ToWstring(errorLocation.file_name()),
											NosLib::String::ToWstring(errorLocation.function_name()),
											errorLocation.line(),
											OutName,
											errorMessage);

	NosLib::Logging::CreateLog<wchar_t>(logMessage, NosLib::Logging::Severity::Error);
}

void ModInfo::StandardModProcess()
{
	UpdateLoadingScreen(L"Requesting File...");
	std::wstring extractPath = FileObject->GetFile(this, &ModInfo::InitialResponseCallback, &ModInfo::ProgressCallback);
	UpdateLoadingScreen(L"Received File");

	if (extractPath.empty())
	{
		LogError(L"Failed to Get Mod File", std::source_location::current());
	}

	UpdateLoadingScreen(L"Copying files...");
	/* for every "inner" path, go through and find the needed files */
	for (std::wstring path : InsidePaths)
	{
		/* root inner path, everything revolves around this */
		std::wstring rootFrom = (extractPath + path);
		std::wstring rootTo = (InstallOptions::GammaInstallPath + InstallInfo::ModDirectory + GetFolderName() + L"\\");


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
}

void ModInfo::CustomModProcess()
{
	UpdateLoadingScreen(L"Requesting File...");
	std::wstring extractPath = FileObject->GetFile(this, &ModInfo::InitialResponseCallback, &ModInfo::ProgressCallback);
	UpdateLoadingScreen(L"Received File");

	if (extractPath.empty())
	{
		LogError(L"Failed to Get Mod File", std::source_location::current());
	}

	UpdateLoadingScreen(L"Copying files...");
	/* for every "inner" path, go through and find the needed files */
	for (std::wstring path : InsidePaths)
	{
		/* root inner path, everything revolves around this */
		std::wstring rootFrom = (extractPath + path);
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
}

void ModInfo::SeparatorModProcess()
{
	std::filesystem::create_directories(InstallOptions::GammaInstallPath + InstallInfo::ModDirectory + GetFolderName());
}
#pragma endregion