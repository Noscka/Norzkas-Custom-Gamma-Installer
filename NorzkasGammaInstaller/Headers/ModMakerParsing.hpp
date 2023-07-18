#pragma once

#include <NosLib/DynamicArray.hpp>
#include <format>
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
	public:
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modInfoList;		/* A list of all mods */
		static inline NosLib::DynamicArray<ModPackMaker::ModInfo*> modFailedList;	/* a list of all failed mods (so even errors) */

		/// <summary>
		/// describes what type of "mod" it is
		/// </summary>
		enum class Type
		{
			Seperator, /* separators, which don't contain any mods but help with organization */
			Standard, /* normal mod, so the struct will contain all the necessary data */
			Custom, /* custom/manually created "mod" object */
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

		std::string ToString()
		{
			std::string TypeString;

			switch (ModType)
			{
			case ModInfo::Type::Seperator:
				TypeString = "Seperator";
				break;
			case ModInfo::Type::Standard:
				TypeString = "Standard";
				break;
			default:
				TypeString = "None";
				break;
			}

			std::string pathList;
			for (int i = 0; i <= InsidePaths.GetLastArrayIndex(); i++)
			{
				pathList += InsidePaths[i];
				if (i != InsidePaths.GetLastArrayIndex())
				{
					pathList += " | ";
				}
			}

			return std::format("===================================================\nType:\t{}\n\nDownload Link:\t|{}\nNumber:\t{}\nInside Path:\t|{}\nCreator Name:\t|{}\nOut Name:\t|{}\nOriginal Link:\t|{}\nLeft Over:\t|{}\n===================================================\n", TypeString, (Link.Host + Link.Path), ModIndex, pathList, CreatorName, OutName, OriginalLink, LeftOver);
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
	};
}
