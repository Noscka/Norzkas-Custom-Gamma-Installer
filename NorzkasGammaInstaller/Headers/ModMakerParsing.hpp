#pragma once

#include <NosLib/DynamicArray.hpp>
#include "String.h"

namespace ModPackMaker
{
	struct ModInfo
	{
		enum class Type
		{
			Seperator,
			Standard,
		};

		static inline int CurrentModIndex = 1;

		Type ModType;
		int ModIndex;
		std::wstring Link;
		NosLib::DynamicArray<std::wstring> InsidePaths;
		std::wstring CreatorName;
		std::wstring OutName;
		std::wstring OriginalLink;

		std::wstring LeftOver;

		ModInfo(const std::wstring& outName)
		{
			ModIndex = CurrentModIndex;
			CurrentModIndex++;

			OutName = outName;
			ModType = Type::Seperator;
		}

		ModInfo(const std::wstring& link, NosLib::DynamicArray<std::wstring>& insidePaths, const std::wstring& creatorName, const std::wstring& outName, const std::wstring& originalLink, const std::wstring& leftOver)
		{
			ModIndex = CurrentModIndex;
			CurrentModIndex++;

			Link = link;
			InsidePaths << insidePaths;
			CreatorName = creatorName;
			OutName = outName;
			OriginalLink = originalLink;
			LeftOver = leftOver;
			ModType = Type::Standard;
		}
	};

	ModInfo* ParseLine(std::wstring& line)
	{
		NosLib::DynamicArray<std::wstring> wordArray(6, 2);

		NosLib::String::Split<wchar_t>(&wordArray, line, L'\t');

		for (int i = 0; i <= wordArray.GetLastArrayIndex(); i++)
		{
			wordArray[i] = CustomStrings::reduce(wordArray[i]);
		}

		if (wordArray.GetLastArrayIndex() == 0)
		{
			return new ModInfo(wordArray[0]);
		}

		NosLib::DynamicArray<std::wstring> pathArray(5, 5);
		NosLib::String::Split<wchar_t>(&pathArray, wordArray[1], L':');

		return new ModInfo(wordArray[0], pathArray, wordArray[2], wordArray[3], wordArray[4], wordArray[5]);
	}

	void PrintModInfo(ModInfo& modInfo)
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

		wprintf(std::format(
			L"===================================================\n\
Type:\t{}\n\n\
Download Link:\t|{}\n\
Number:\t{}\n\
Inside Path:\t|{}\n\
Creator Name:\t|{}\n\
Out Name:\t|{}\n\
Original Link:\t|{}\n\
Left Over:\t|{}\n\
===================================================\n", TypeString, modInfo.Link, modInfo.ModIndex, pathList, modInfo.CreatorName, modInfo.OutName, modInfo.OriginalLink, modInfo.LeftOver).c_str());
	}

	NosLib::DynamicArray<ModInfo*> ModpackMakerFile_Parse()
	{
		NosLib::DynamicArray<ModInfo*> outputArray;

		std::wifstream modMakerFile("modpack_maker_list.txt", std::ios::binary);

		std::wstring line;
		while (std::getline(modMakerFile, line))
		{
			outputArray.Append(ParseLine(line));
		}

		return outputArray;
	}
}
