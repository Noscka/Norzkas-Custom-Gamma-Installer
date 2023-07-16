#pragma once

#include <NosLib/DynamicArray.hpp>
#include "String.h"

namespace ModPackMaker
{
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

	HostPath GetHostPath(const std::wstring& link)
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

	struct ModInfo
	{
		/// <summary>
		/// describes what type of "mod" it is
		/// </summary>
		enum class Type
		{
			Seperator, /* separators, which don't contain any mods but help with organization */
			Standard, /* normal mod, so the struct will contain all the necessary data */
		};

		static inline int CurrentModIndex = 1; /* a global counter, the normal GAMMA installer numbers all the mods depending on where they are in the install file. keep a global track */

		Type ModType; /* the mod type */
		int ModIndex; /* the mod index, that will be used in the folder name */
		HostPath Link; /* the download link, will be used to download */
		NosLib::DynamicArray<std::wstring> InsidePaths; /* an array of the inner paths (incase there is many) */
		std::wstring CreatorName; /* the creator name (used in folder name) */
		std::wstring OutName; /* the main folder name (use in folder name) */
		std::wstring OriginalLink; /* original mod link (I don't know why its there but I'll parse it anyway) */

		std::wstring LeftOver; /* any left over data */

		/// <summary>
		/// Seperator constructor, only has a name and index
		/// </summary>
		/// <param name="outName">- seperator name</param>
		ModInfo(const std::wstring& outName)
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
		ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& creatorName, const std::wstring& outName, const std::wstring& originalLink, const std::wstring& leftOver)
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
	};

	/// <summary>
	/// Takes in a string and actually parses it, and puts it into an object
	/// </summary>
	/// <param name="line">- input line</param>
	/// <returns>pointer of ModInfo, containing parsed mod info</returns>
	ModInfo* ParseLine(std::wstring& line)
	{
		/* create array, the file uses \t to separate info */
		NosLib::DynamicArray<std::wstring> wordArray(6, 2);

		/* split the line into the previous array */
		NosLib::String::Split<wchar_t>(&wordArray, line, L'\t');

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
		NosLib::DynamicArray<std::wstring> pathArray(5, 5);
		NosLib::String::Split<wchar_t>(&pathArray, wordArray[1], L':');

		/* finally, if it has gotten here, it means the current line is a normal mod, pass in all the info to the contrustor */
		return new ModInfo(wordArray[0], pathArray, wordArray[2], wordArray[3], wordArray[4], wordArray[5]);
	}

	std::wstring StringModInfo(ModInfo& modInfo)
	{
		std::wstring TypeString;

		switch (modInfo.ModType)
		{
		case ModInfo::Type::Seperator:
			TypeString = L"Seperator";
			break;
		case ModInfo::Type::Standard:
			TypeString = L"Standard";
			break;
		default:
			TypeString = L"None";
			break;
		}

		std::wstring pathList;
		for (int i = 0; i <= modInfo.InsidePaths.GetLastArrayIndex(); i++)
		{
			pathList += modInfo.InsidePaths[i];
			if (i != modInfo.InsidePaths.GetLastArrayIndex())
			{
				pathList += L" | ";
			}
		}

		return std::format(
			L"===================================================\n\
Type:\t{}\n\n\
Download Link:\t|{}\n\
Number:\t{}\n\
Inside Path:\t|{}\n\
Creator Name:\t|{}\n\
Out Name:\t|{}\n\
Original Link:\t|{}\n\
Left Over:\t|{}\n\
===================================================\n", TypeString, NosLib::String::ConvertString<wchar_t, char>(modInfo.Link.Host + modInfo.Link.Path), modInfo.ModIndex, pathList, modInfo.CreatorName, modInfo.OutName, modInfo.OriginalLink, modInfo.LeftOver);
	}

	/// <summary>
	/// takes in a filename for a modpackMaker and parses it fully
	/// </summary>
	/// <param name="modpackMakerFileName">- path/name of modpack Maker</param>
	/// <returns>a DynamicArray of ModInfo pointers (ModInfo*)</returns>
	NosLib::DynamicArray<ModInfo*> ModpackMakerFile_Parse(const std::wstring& modpackMakerFileName)
	{
		/* create output array */
		NosLib::DynamicArray<ModInfo*> outputArray;

		/* open binary file stream of modpack maker list */
		std::wifstream modMakerFile(modpackMakerFileName, std::ios::binary);

		/* got through all lines in the file. each line is a new mod */
		std::wstring line;
		while (std::getline(modMakerFile, line))
		{
			/* append to array */
			outputArray.Append(ParseLine(line));
		}

		return outputArray;
	}
}
