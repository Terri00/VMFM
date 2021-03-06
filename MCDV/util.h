#pragma once
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

#include <regex>

std::vector<std::string> split(std::string string, char delim)
{
	std::stringstream cStream(string);
	std::string seg;
	std::vector<std::string> sgts;

	while (std::getline(cStream, seg, delim))
		sgts.push_back(seg);

	return sgts;
}

std::vector<std::string> split(std::string s, std::string delimiter)
{
	std::vector<std::string> sgts;
	size_t pos = 0;
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		sgts.push_back(token);
		s.erase(0, pos + delimiter.length());
	}
	sgts.push_back(s);
	return sgts;
}

namespace sutil
{
	std::string trimspace(std::string const& str)
	{
		if (str.empty())
			return str;

		std::size_t firstScan = str.find_first_not_of(' ');
		std::size_t first = firstScan == std::string::npos ? str.length() : firstScan;
		std::size_t last = str.find_last_not_of(' ');
		return str.substr(first, last - first + 1);
	}

	std::string trimbt(std::string const& str)
	{
		if (str.empty())
			return str;

		std::size_t firstScan = str.find_first_not_of('\t');
		std::size_t first = firstScan == std::string::npos ? str.length() : firstScan;
		std::size_t last = str.find_last_not_of('\t');
		return str.substr(first, last - first + 1);
	}

	std::string trim(std::string str)
	{
		return trimspace(trimbt(str));
	}

	std::string removeChar(std::string str, char ch)
	{
		str.erase(std::remove(str.begin(), str.end(), ch), str.end());
		return str;
	}

	std::vector<std::string> regexmulti(std::string src, std::string pattern)
	{
		const std::regex r(pattern);

		std::smatch res;

		std::vector<std::string> matches;
		while (std::regex_search(src, res, r)) {
			matches.push_back(res[0]);
			src = res.suffix();
		}

		return matches;
	}

	std::vector<std::string> regexmulti(std::string src, const std::regex pattern)
	{
		std::smatch res;

		std::vector<std::string> matches;
		while (std::regex_search(src, res, pattern)) {
			matches.push_back(res[0]);
			src = res.suffix();
		}

		return matches;
	}
}