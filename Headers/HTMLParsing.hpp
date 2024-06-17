#pragma once

#include <string>
#include <fstream>

namespace ModDBParsing
{
	struct HTMLParseReturn
	{
		std::wstring Link;
		bool Found;
	};

	const inline std::string linkIdentifier = "<a href=\""; /* substring which is used to identify the start of the link */

	/// <summary>
	/// takes in a line of html and parses to find the link
	/// </summary>
	/// <param name="line">- a html line</param>
	/// <returns>HTMLParseReturn struct</returns>
	inline HTMLParseReturn ParseHtmlLine(const std::string& line)
	{
		/* find the start offset using the above string */
		size_t startPosition = line.find(linkIdentifier);

		/* if it returns "npos", means it wasn't found and therefore return */
		if (startPosition == std::string::npos)
		{
			return {L"Not Found", false};
		}

		/* add the lenght of the substring used since that is true start */
		startPosition += linkIdentifier.length();

		/* find a second " to find the end position */
		size_t endPosition = line.find("\"", startPosition);

		/* if return "npos", return again since it means something isn't right */
		if (endPosition == std::string::npos)
		{
			return {L"Not Found", false};
		}

		std::string link = line.substr(startPosition, endPosition - (startPosition));

		/* if got here, that means it has all the needed info, create substring and put it into struct */
		return HTMLParseReturn{NosLib::String::ToWstring(link), true};
	}

	/// <summary>
	/// The function that takes a file and iterates through it
	/// </summary>
	/// <param name="fileName">- file path/name</param>
	/// <returns>a link if found</returns>
	inline std::wstring ParseHtmlForLink(const std::string& html)
	{
		std::istringstream htmlStream(html);

		std::string line;
		while (std::getline(htmlStream, line))
		{
			HTMLParseReturn parseResult = ParseHtmlLine(line);

			if (parseResult.Found == true)
			{
				return parseResult.Link;
			}
		}

		return L"No Link Found";
	}
}