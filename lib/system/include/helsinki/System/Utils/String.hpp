#pragma once

#include <string>
#include <vector>
#include <optional>
#include <sstream>
#include <cstdint>

namespace hl
{
	template <typename T, typename Enable = void>
	struct is_optional : std::false_type {};

	template <typename T>
	struct is_optional<std::optional<T>> : std::true_type {};

	class String
	{
	public:
		String() = delete;

		static std::vector<std::string> Split(const std::string& str, char sep);
		static std::vector<std::string> Split(const std::string& str, const std::string& sep);

		static bool StartsWith(std::string_view str, std::string_view token);

		static bool Contains(std::string_view str, std::string_view token) noexcept;

		static bool IsWhitespace(char c) noexcept;

		static bool IsNumber(std::string_view str) noexcept;

		static int32_t FindCharPos(std::string_view str, char c) noexcept;

		static std::string_view Trim(std::string_view str, std::string_view whitespace = " \t\n\r");

		static std::string RemoveAll(std::string str, char token);

		static std::string RemoveLast(std::string str, char token);

		static std::string ReplaceAll(std::string str, std::string_view token, std::string_view to);

		static std::string ReplaceFirst(std::string str, std::string_view token, std::string_view to);

		static std::string FixEscapedChars(std::string str);

		static std::string UnfixEscapedChars(std::string str);

		static std::string Lowercase(std::string str);

		static std::string Uppercase(std::string str);
		static std::string readFile(const std::string& filename);

		template<typename T>
		static std::string To(T val)
		{
			if constexpr (std::is_same_v<std::string, T> || std::is_same_v<const char*, T>)
			{
				return val;
			}
			else if constexpr (std::is_enum_v<T>)
			{
				typedef typename std::underlying_type<T>::type safe_type;
				return std::to_string(static_cast<safe_type>(val));
			}
			else if constexpr (std::is_same_v<bool, T>)
			{
				return val ? "true" : "false";
			}
			else if constexpr (std::is_same_v<std::nullptr_t, T>)
			{
				return "null";
			}
			else if constexpr (hl::is_optional<T>::value)
			{
				if (!val.has_value())
					return "null";
				return To(*val);
			}
			else if constexpr (std::is_same_v<char, T>)
			{
				return std::string(1, val);
			}
			else
			{
				return std::to_string(val);
			}
		}

		template<typename T>
		static T From(const std::string& str)
		{
			if constexpr (std::is_same_v<std::string, T>)
			{
				return str;
			}
			else if constexpr (std::is_enum_v<T>)
			{
				typedef typename std::underlying_type<T>::type safe_type;
				return static_cast<T>(From<safe_type>(str));
			}
			else if constexpr (std::is_same_v<bool, T>)
			{
				return str == "true" || From<std::optional<int32_t>>(str) == 1;
			}
			else if constexpr (hl::is_optional<T>::value)
			{
				typedef typename T::value_type base_type;
				base_type temp;
				std::istringstream iss(str);

				if ((iss >> temp).fail())
					return std::nullopt;
				return temp;
			}
			else
			{
				long double temp;
				std::istringstream iss(str);
				iss >> temp;
				return static_cast<T>(temp);
			}
		}

		// fnv1a 32 and 64 bit hash functions
		// key is the data to hash, len is the size of the data (or how much of it to hash against)
		// code license: public domain or equivalent
		// post: https://notes.underscorediscovery.com/constexpr-fnv1a/

		constexpr static uint32_t fnv1a_32(std::string_view str, const uint32_t value = 0x811c9dc5) noexcept
		{
			return str.size() == 0 ? value : fnv1a_32(std::string_view(str.data() + 1, str.size() - 1),
				(value ^ uint32_t(str[0])) * 0x1000193);
		}

		constexpr static uint64_t fnv1a_64(std::string_view str, const uint64_t value = 0xcbf29ce484222325) noexcept
		{
			return str.size() == 0 ? value : fnv1a_64(std::string_view(str.data() + 1, str.size() - 1),
				(value ^ uint64_t(str[0])) * 0x100000001b3);
		}
	};

	constexpr std::uint32_t operator""_hash(const char* s, std::size_t count)
	{
		return String::fnv1a_32(std::string_view(s, count));
	}
}