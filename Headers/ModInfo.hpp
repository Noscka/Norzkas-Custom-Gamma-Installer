#pragma once

#include <NosLib\DynamicArray.hpp>
#include <NosLib\String.hpp>
#include <NosLib\HostPath.hpp>
#include <NosLib\HttpClient.hpp>
#include <NosLib\FileManagement.hpp>

#include <format>
#include <source_location>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include "Validation.hpp"
#include "File.hpp"

class ModProcessorThread;

/* Sub directories that are inside each mod folder */
inline NosLib::DynamicArray<std::wstring> ModSubDirectories = NosLib::DynamicArray<std::wstring>({ L"fomod\\" }) + Validation::StalkerSubDirectories.ObjectExclude(L"bin\\");

class ModInfo
{
public:
	enum class WorkState : uint16_t
	{
		NotStarted,
		InProgress,
		Completed
	};

protected:
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

	/* Mod Params */
	Type ModType;									/* the mod type */
	int ModPrefixIndex;								/* the mod index, that will be used in the folder name */
	NosLib::DynamicArray<std::wstring> InsidePaths;	/* an array of the inner paths (incase there is many) */
	std::wstring CreatorName;						/* the creator name (used in folder name) */
	std::wstring OutName;							/* the main folder name (use in folder name) */
	std::wstring OriginalLink;						/* original mod link (I don't know why its there but I'll parse it anyway) */

	/* Extra Mod Params */
	std::wstring OutPath;							/* This is Custom modtype only, it defines were to copy the files to */
	bool UseInstallPath = true;						/* If mod should include mod path when installing (ONLY FOR CUSTOM) */

	/* MultiThreading */
	inline static std::mutex PriorityMutex;
	inline static std::condition_variable PriorityCV;
	ModProcessorThread* ProcessingThread;
	std::mutex WorkStateMutex;
	std::atomic<WorkState> CurrentWorkState = WorkState::NotStarted;
	File* FileObject = nullptr;

	/// <summary>
	/// Needs to be run by all the constructors, initializes for loading screen
	/// </summary>
	void InitializeModInfo();

public:
	inline static NosLib::DynamicArray<ModInfo*> ModInfoList; /* A list of all mods */
	inline static NosLib::DynamicArray<ModInfo*> PriorityModList;

protected:
	/// <summary>
	/// Seperator constructor, only has a name and index
	/// </summary>
	/// <param name="outName">- seperator name</param>
	ModInfo(const std::wstring& outName);

	/// <summary>
	/// Standard Mod Constructor, has all the data
	/// </summary>
	/// <param name="link"> -the download link, will be used to download</param>
	/// <param name="insidePaths">- an array of the inner paths (incase there is many)</param>
	/// <param name="creatorName">- the creator name (used in folder name)</param>
	/// <param name="outName">- the main folder name (use in folder name)</param>
	/// <param name="originalLink">- original mod link (I don't know why its there but I'll parse it anyway)</param>
	/// <param name="leftOver">- Any left over data</param>
	ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& creatorName, const std::wstring& outName, const std::wstring& originalLink);

public:
	/// <summary>
	/// Custom Mod Constructor, can output anywhere
	/// </summary>
	/// <param name="link"> - the download link, will be used to download</param>
	/// <param name="insidePaths">- an array of the inner paths (incase there is many) I-Value</param>
	/// <param name="OutPath">- where to copy the extracted data to</param>
	/// <param name="outName">- what to name the file</param>
	/// <param name="useInstallPath">(default = true) - if it should add installPath string to the front of its paths</param>
	/// <param name="customExtension">(default = L"") - custom extension</param>
	ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& useInstallPath = true, const std::wstring& customExtension = L"");

	/// <summary>
	/// Custom Mod Constructor, can output anywhere
	/// </summary>
	/// <param name="link"> - the download link, will be used to download</param>
	/// <param name="insidePaths">- an array of the inner paths (incase there is many) R-Value</param>
	/// <param name="OutPath">- where to copy the extracted data to</param>
	/// <param name="outName">- what to name the file</param>
	/// <param name="useInstallPath">(default = true) - if it should add installPath string to the front of its paths</param>
	/// <param name="customExtension">(default = L"") - custom extension</param>
	ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>&& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& useInstallPath = true, const std::wstring& customExtension = L"");

	std::wstring GetFolderName();

	static ModInfo* AddMod(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& priorityInstall = false, const bool& useInstallPath = true, const std::wstring& customExtension = L"");
	static ModInfo* AddMod(const std::wstring& link, NosLib::DynamicArray<std::wstring>&& insidePaths, const std::wstring& outPath, const std::wstring& outName, const bool& priorityInstall = false, const bool& useInstallPath = true, const std::wstring& customExtension = L"");

	WorkState GetModWorkState();
	void WaitPriority(ModProcessorThread* processingThread);

	void ProcessMod(ModProcessorThread* processingThread);

	/// <summary>
	/// takes in a filename for a modpackMaker and parses it fully
	/// </summary>
	/// <param name="modpackMakerFileName">(default = "modpack_maker_list.txt") - path/name of modpack Maker</param>
	/// <returns>a DynamicArray of ModInfo pointers (ModInfo*)</returns>
	static void ModpackMakerFile_Parse(const std::wstring& modpackMakerFileName);

protected:
	/// <summary>
	/// Takes in a string and actually parses it, and puts it into an object
	/// </summary>
	/// <param name="line">- input line</param>
	/// <returns>pointer of ModInfo, containing parsed mod info</returns>
	static ModInfo* ParseLine(std::wstring& line);

	void UpdateLoadingScreen(const std::wstring& status);
	void UpdateLoadingScreen(const int& percentageOnCurrentMod);
	void UpdateLoadingScreen(const int& percentageOnCurrentMod, const std::wstring& status);

	void LogError(const std::wstring& errorMessage, const std::source_location& errorLocation);

	void StandardModProcess();
	void CustomModProcess();
	void SeparatorModProcess();

	void InitialResponseCallback(const std::wstring& statusString);
	bool ProgressCallback(uint64_t len, uint64_t total);
};