#pragma once

#include <helsinki/System/Utils/Json.hpp>
#include <helsinki/System/Utils/String.hpp>
#include <helsinki/System/glm.hpp>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tower::catalogJson
{
	[[noreturn]] inline void fail(const std::string& message)
	{
		throw std::runtime_error(message);
	}

	inline const hl::JsonNode* findChild(const hl::JsonNode& row, const char* name)
	{
		for (const auto* child : row.children)
		{
			if (child != nullptr && child->name == name)
			{
				return child;
			}
		}

		return nullptr;
	}

	inline float optionalPositive(
		const hl::JsonNode& row,
		const char* name,
		float fallback,
		const char* file)
	{
		const auto* node = findChild(row, name);
		if (node == nullptr)
		{
			return fallback;
		}

		if (node->type == hl::JsonNode::Type::ValueNumber)
		{
			if (node->number <= 0.0f)
			{
				fail(std::string(file) + ": '" + name + "' must be > 0");
			}

			return node->number;
		}

		if (node->type == hl::JsonNode::Type::ValueInteger)
		{
			if (node->integer <= 0)
			{
				fail(std::string(file) + ": '" + name + "' must be > 0");
			}

			return static_cast<float>(node->integer);
		}

		fail(std::string(file) + ": '" + name + "' must be a number");
	}

	inline float optionalNonNegative(
		const hl::JsonNode& row,
		const char* name,
		float fallback,
		const char* file)
	{
		const auto* node = findChild(row, name);
		if (node == nullptr)
		{
			return fallback;
		}

		if (node->type == hl::JsonNode::Type::ValueNumber)
		{
			if (node->number < 0.0f)
			{
				fail(std::string(file) + ": '" + name + "' must be >= 0");
			}

			return node->number;
		}

		if (node->type == hl::JsonNode::Type::ValueInteger)
		{
			if (node->integer < 0)
			{
				fail(std::string(file) + ": '" + name + "' must be >= 0");
			}

			return static_cast<float>(node->integer);
		}

		fail(std::string(file) + ": '" + name + "' must be a number");
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

	inline int optionalIntAtLeast(
		const hl::JsonNode& row,
		const char* name,
		int minimum,
		int fallback,
		const char* file)
	{
		const auto* node = findChild(row, name);
		if (node == nullptr)
		{
			return fallback;
		}

		if (node->type != hl::JsonNode::Type::ValueInteger || node->integer < minimum)
		{
			fail(std::string(file) + ": '" + name + "' must be an integer >= "
				+ std::to_string(minimum));
		}

		return node->integer;
	}

	inline int optionalInt(
		const hl::JsonNode& row,
		const char* name,
		int fallback,
		const char* file)
	{
		const auto* node = findChild(row, name);
		if (node == nullptr)
		{
			return fallback;
		}

		if (node->type != hl::JsonNode::Type::ValueInteger)
		{
			fail(std::string(file) + ": '" + name + "' must be an integer");
		}

		return node->integer;
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

	inline std::pair<int, int> requireBoardSize(const hl::JsonNode& row, const char* file)
	{
		const auto& node = field(row, "boardSize");
		if (node.type != hl::JsonNode::Type::Array || node.children.size() != 2)
		{
			fail(std::string(file) + ": 'boardSize' must be [width, depth] with two integers >= 2");
		}

		const auto parseDim = [&](const hl::JsonNode* child) -> int
		{
			if (child == nullptr || child->type != hl::JsonNode::Type::ValueInteger || child->integer < 2)
			{
				fail(std::string(file) + ": 'boardSize' must be [width, depth] with two integers >= 2");
			}

			return child->integer;
		};

		return { parseDim(node.children[0]), parseDim(node.children[1]) };
	}

	inline float requireNumber(const hl::JsonNode& node, const char* file, const char* what)
	{
		if (node.type == hl::JsonNode::Type::ValueNumber)
		{
			return node.number;
		}

		if (node.type == hl::JsonNode::Type::ValueInteger)
		{
			return static_cast<float>(node.integer);
		}

		fail(std::string(file) + ": '" + what + "' must be a number");
	}

	inline float requireAnyNumber(const hl::JsonNode& node, const char* file, const char* what)
	{
		return requireNumber(node, file, what);
	}

	inline const hl::JsonNode* optionalObject(
		const hl::JsonNode& row,
		const char* name,
		const char* file)
	{
		const auto* node = findChild(row, name);
		if (node == nullptr)
		{
			return nullptr;
		}

		if (node->type != hl::JsonNode::Type::Object)
		{
			fail(std::string(file) + ": '" + name + "' must be an object");
		}

		return node;
	}

	inline glm::vec3 requireVec3(
		const hl::JsonNode& row,
		const char* name,
		const char* file,
		bool componentsPositive)
	{
		const auto& node = field(row, name);
		if (node.type != hl::JsonNode::Type::Array || node.children.size() != 3)
		{
			fail(std::string(file) + ": '" + name + "' must be [x, y, z]");
		}

		glm::vec3 value{ 0.0f };
		for (int i = 0; i < 3; ++i)
		{
			const auto* child = node.children[static_cast<std::size_t>(i)];
			if (child == nullptr)
			{
				fail(std::string(file) + ": '" + name + "' must be [x, y, z]");
			}

			value[i] = requireNumber(*child, file, name);
			if (componentsPositive && value[i] <= 0.0f)
			{
				fail(std::string(file) + ": '" + name + "' components must be > 0");
			}
		}

		return value;
	}

	inline glm::vec3 requireColor01(const hl::JsonNode& row, const char* name, const char* file)
	{
		const glm::vec3 value = requireVec3(row, name, file, false);
		for (int i = 0; i < 3; ++i)
		{
			if (value[i] < 0.0f || value[i] > 1.0f)
			{
				fail(std::string(file) + ": '" + name + "' components must be 0..1");
			}
		}

		return value;
	}

	inline std::vector<std::string> optionalStringArray(
		const hl::JsonNode& row,
		const char* name,
		const char* file)
	{
		const auto* node = findChild(row, name);
		if (node == nullptr)
		{
			return {};
		}

		if (node->type != hl::JsonNode::Type::Array)
		{
			fail(std::string(file) + ": '" + name + "' must be an array of strings");
		}

		std::vector<std::string> values;
		for (const auto* child : node->children)
		{
			if (child == nullptr
				|| child->type != hl::JsonNode::Type::ValueString
				|| child->content.empty())
			{
				fail(std::string(file) + ": '" + name + "' must be an array of strings");
			}

			values.push_back(child->content);
		}

		return values;
	}

	inline void requireArrayRoot(const hl::JsonDocument& doc, const char* file)
	{
		if (doc.m_Root == nullptr || doc.m_Root->type != hl::JsonNode::Type::Array)
		{
			fail(std::string(file) + ": root must be an array");
		}
	}

	inline void requireObjectRoot(const hl::JsonDocument& doc, const char* file)
	{
		if (doc.m_Root == nullptr || doc.m_Root->type != hl::JsonNode::Type::Object)
		{
			fail(std::string(file) + ": root must be an object");
		}
	}
}
