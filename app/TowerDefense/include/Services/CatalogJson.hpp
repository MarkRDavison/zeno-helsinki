#pragma once

#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <stdexcept>
#include <string>
#include <utility>

namespace tower::catalogJson
{
	[[noreturn]] inline void fail(const std::string& message)
	{
		throw std::runtime_error(message);
	}

	inline const hl::JsonNode& field(const hl::JsonNode& row, const char* name)
	{
		try
		{
			return row[name];
		}
		catch (const std::string& error)
		{
			fail(error);
		}
	}

	inline std::string requireString(
		const hl::JsonNode& row,
		const char* name,
		const char* file)
	{
		const auto& node = field(row, name);
		if (node.type != hl::JsonNode::Type::ValueString || node.content.empty())
		{
			fail(std::string(file) + ": '" + name + "' must be a non-empty string");
		}

		return node.content;
	}

	inline int requireIntAtLeast(
		const hl::JsonNode& row,
		const char* name,
		int minimum,
		const char* file)
	{
		const auto& node = field(row, name);
		if (node.type != hl::JsonNode::Type::ValueInteger || node.integer < minimum)
		{
			fail(std::string(file) + ": '" + name + "' must be an integer >= "
				+ std::to_string(minimum));
		}

		return node.integer;
	}

	inline float requirePositive(
		const hl::JsonNode& row,
		const char* name,
		const char* file)
	{
		const auto& node = field(row, name);
		if (node.type == hl::JsonNode::Type::ValueNumber)
		{
			if (node.number <= 0.0f)
			{
				fail(std::string(file) + ": '" + name + "' must be > 0");
			}

			return node.number;
		}

		if (node.type == hl::JsonNode::Type::ValueInteger)
		{
			if (node.integer <= 0)
			{
				fail(std::string(file) + ": '" + name + "' must be > 0");
			}

			return static_cast<float>(node.integer);
		}

		fail(std::string(file) + ": '" + name + "' must be a number");
	}

	inline std::pair<int, int> requireSize(const hl::JsonNode& row, const char* file)
	{
		const auto& node = field(row, "size");
		if (node.type != hl::JsonNode::Type::Array || node.children.size() != 2)
		{
			fail(std::string(file) + ": 'size' must be [x, z] with two integers >= 1");
		}

		const auto parseDim = [&](const hl::JsonNode* child) -> int
		{
			if (child == nullptr || child->type != hl::JsonNode::Type::ValueInteger || child->integer < 1)
			{
				fail(std::string(file) + ": 'size' must be [x, z] with two integers >= 1");
			}

			return child->integer;
		};

		return { parseDim(node.children[0]), parseDim(node.children[1]) };
	}

	inline void requireArrayRoot(const hl::JsonDocument& doc, const char* file)
	{
		if (doc.m_Root == nullptr || doc.m_Root->type != hl::JsonNode::Type::Array)
		{
			fail(std::string(file) + ": root must be an array");
		}
	}
}
