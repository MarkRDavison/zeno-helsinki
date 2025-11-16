#include <helsinki/System/Utils/String.hpp>

#include <codecvt>
#include <locale>
#include <fstream>
#include <algorithm>

namespace hl
{
	std::vector<std::string> String::Split(const std::string& str, char sep)
	{
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(str);

		while (std::getline(tokenStream, token, sep))
		{
			tokens.emplace_back(token);
		}

		return tokens;
	}

	std::vector<std::string> String::Split(const std::string& str, const std::string& sep)
	{
		std::vector<std::string> result;
		std::stringstream stringStream(str);
		std::string line;
		while (std::getline(stringStream, line))
		{
			std::size_t prev = 0, pos;
			while ((pos = line.find_first_of(sep, prev)) != std::string::npos)
			{
				if (pos > prev)
				{
					result.push_back(line.substr(prev, pos - prev));
				}
				prev = pos + 1;
			}
			if (prev < line.length())
			{
				result.push_back(line.substr(prev, std::string::npos));
			}
		}

		return result;
	}

	bool String::StartsWith(std::string_view str, std::string_view token)
	{
		if (str.length() < token.length())
		{
			return false;
		}
		return str.compare(0, token.length(), token) == 0;
	}

	bool String::Contains(std::string_view str, std::string_view token) noexcept
	{
		return str.find(token) != std::string::npos;
	}

	bool String::IsWhitespace(char c) noexcept
	{
		return c == ' ' || c == '\n' || c == '\r' || c == '\t';
	}

	bool String::IsNumber(std::string_view str) noexcept
	{
		return std::all_of(str.cbegin(), str.cend(), [](auto c)
			{
				return (c >= '0' && c <= '9') || c == '.' || c == '-';
			});
	}

	int32_t String::FindCharPos(std::string_view str, char c) noexcept
	{
		auto res = str.find(c);
		return res == std::string::npos ? -1 : static_cast<int32_t>(res);
	}

	std::string_view String::Trim(std::string_view str, std::string_view whitespace)
	{
		auto strBegin = str.find_first_not_of(whitespace);
		if (strBegin == std::string::npos)
		{
			return "";
		}

		auto strEnd = str.find_last_not_of(whitespace);
		auto strRange = strEnd - strBegin + 1;
		return str.substr(strBegin, strRange);
	}

	std::string String::RemoveAll(std::string str, char token)
	{
		str.erase(std::remove(str.begin(), str.end(), token), str.end());
		return str;
	}

	std::string String::RemoveLast(std::string str, char token)
	{
		for (auto it = str.end(); it != str.begin(); --it)
		{
			if (*it == token)
			{
				str.erase(it);
				return str;
			}
		}

		return str;
	}

	std::string String::ReplaceAll(std::string str, std::string_view token, std::string_view to)
	{
		auto pos = str.find(token);
		while (pos != std::string::npos)
		{
			str.replace(pos, token.size(), to);
			pos = str.find(token, pos + token.size());
		}

		return str;
	}

	std::string String::ReplaceFirst(std::string str, std::string_view token, std::string_view to)
	{
		const auto startPos = str.find(token);
		if (startPos == std::string::npos)
		{
			return str;
		}

		str.replace(startPos, token.length(), to);
		return str;
	}

	std::string String::FixEscapedChars(std::string str)
	{
		static const std::vector<std::pair<char, std::string_view>> replaces = { {'\\', "\\\\"}, {'\n', "\\n"}, {'\r', "\\r"}, {'\t', "\\t"}, {'\"', "\\\""} };

		for (const auto& [from, to] : replaces)
		{
			auto pos = str.find(from);
			while (pos != std::string::npos)
			{
				str.replace(pos, 1, to);
				pos = str.find(from, pos + 2);
			}
		}

		return str;
	}

	std::string String::UnfixEscapedChars(std::string str)
	{
		static const std::vector<std::pair<std::string_view, char>> replaces = { {"\\n", '\n'}, {"\\r", '\r'}, {"\\t", '\t'}, {"\\\"", '\"'}, {"\\\\", '\\'} };

		for (const auto& [from, to] : replaces)
		{
			auto pos = str.find(from);
			while (pos != std::string::npos)
			{
				if (pos != 0 && str[pos - 1] == '\\')
				{
					str.erase(str.begin() + --pos);
				}
				else
				{
					str.replace(pos, from.size(), 1, to);
				}
				pos = str.find(from, pos + 1);
			}
		}

		return str;
	}

	std::string String::Lowercase(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](const char c) -> char { return (char)::tolower(c); });
		return str;
	}

	std::string String::Uppercase(std::string str)
	{
		std::transform(str.begin(), str.end(), str.begin(), [](const char c) -> char { return (char)::toupper(c); });
		return str;
	}
	std::string String::readFile(const std::string& filename)
	{
		std::ifstream file(filename, std::ios::in | std::ios::binary);
		if (!file)
		{
			throw std::runtime_error("Failed to open file: " + filename);
		}

		std::ostringstream contents;
		contents << file.rdbuf();
		return contents.str();
	}
}
