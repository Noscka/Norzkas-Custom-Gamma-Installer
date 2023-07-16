#pragma once

#include <string>
#include <fstream>

namespace ModDBParsing
{
	struct HTMLParseReturn
	{
		std::string Link;
		bool Found;
	};

	const std::string linkIdentifier = "<a href=\""; /* substring which is used to identify the start of the link */

	/// <summary>
	/// takes in a line of html and parses to find the link
	/// </summary>
	/// <param name="line">- a html line</param>
	/// <returns>HTMLParseReturn struct</returns>
	HTMLParseReturn ParseHtmlLine(const std::string& line)
	{
		/* find the start offset using the above string */
		size_t startPosition = line.find(linkIdentifier);

		/* if it returns "npos", means it wasn't found and therefore return */
		if (startPosition == std::string::npos)
		{
			return {"Not Found", false};
		}

		/* add the lenght of the substring used since that is true start */
		startPosition += linkIdentifier.length();

		/* find a second " to find the end position */
		size_t endPosition = line.find("\"", startPosition);

		/* if return "npos", return again since it means something isn't right */
		if (endPosition == std::string::npos)
		{
			return {"Not Found", false};
		}

		/* if got here, that means it has all the needed info, create substring and put it into struct */
		return HTMLParseReturn{line.substr(startPosition, endPosition - (startPosition)), true};
	}

	/// <summary>
	/// The function that takes a file and iterates through it
	/// </summary>
	/// <param name="fileName">- file path/name</param>
	/// <returns>a link if found</returns>
	HTMLParseReturn ParseHtmlForLink(const std::string& html)
	{
		HTMLParseReturn output{"No Link Found", false};

		std::istringstream htmlStream(html);

		std::string line;
		while (std::getline(htmlStream, line))
		{
			HTMLParseReturn parseResult = ParseHtmlLine(line);

			if (parseResult.Found == true)
			{
				output = HTMLParseReturn{parseResult.Link, true};
				break;
			}
		}

		return output;
	}
}