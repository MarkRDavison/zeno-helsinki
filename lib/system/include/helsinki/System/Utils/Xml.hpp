#pragma once

#include <helsinki/System/Export.hpp>

#include <functional>
#include <unordered_map>
#include <string>

namespace hl
{

	using AttributeMap = std::unordered_map<std::string, std::string>;

	enum class Type
	{
		OpenElement,
		CloseElement,
		SelfCloseElement,
		Content
	};

	struct ItemDiscovered
	{
		std::string name;
		std::string content;
		Type type;
		AttributeMap attributes;
	};

	class HELSINKI_SYSTEM_EXPORT XmlNode
	{
	public:
		~XmlNode();

		std::string name;
		std::string content;
		AttributeMap attributes;
		std::vector<XmlNode*> children;
		XmlNode* parent{ nullptr };
	};

	class HELSINKI_SYSTEM_EXPORT XmlDocument
	{
	public:
		XmlDocument(); // TODO: Limit what is exposed on this class
		XmlDocument(const XmlDocument& _doc); // TODO: Copy/move, assign/move operator
		~XmlDocument();

		const XmlNode* select(const std::string& _selection) const;
		const std::vector<XmlNode*> selectMany(const std::string& _selection) const;
		const XmlNode* select(XmlNode* _start, const std::string& _selection) const;
		const std::vector<XmlNode*> selectMany(XmlNode* _start, const std::string& _selection) const;
		std::function<void(ItemDiscovered)> itemDiscovered;

		const std::string& getInputRepresentation() const;
		bool readFromFile(const std::string& _file);
		bool readFromText(const std::string& _text);

		bool parse(const std::string& _text);

		std::string dump(int _indentation = 4);


		static std::size_t getNextTagStartIndex(const std::string& _text, std::size_t _current);
		static std::size_t getNextTagCloseIndex(const std::string& _text, std::size_t _current);
		static std::size_t getNextOccurenceIndex(const std::string& _text, std::size_t _current, char _char);
		static std::string extractElementName(const std::string& _elementTag);

		XmlNode* m_Node{ nullptr };

	protected:
		std::pair<std::size_t, std::size_t> getNextTagIndexes(const std::string& _text, std::size_t _current);
		static std::unordered_map<std::string, std::string> extractAttributePairs(const std::string& _text);
		static std::string trim(const std::string& _text);

	private:
		std::string m_Text;
	};

	namespace Xml
	{
		HELSINKI_SYSTEM_EXPORT XmlDocument parseFromFile(const std::string& _file);
		HELSINKI_SYSTEM_EXPORT XmlDocument parseFromText(const std::string& _text);
		HELSINKI_SYSTEM_EXPORT bool validate(const std::string& _text);
		HELSINKI_SYSTEM_EXPORT bool createTreeFromStreamingDocument(XmlDocument& _document);

		HELSINKI_SYSTEM_EXPORT bool hasAttribute(const XmlNode* _node, const std::string& _name);

		template <typename T>
		static T getAttribute(const XmlNode*, const std::string&)
		{
			throw std::exception();
		}

		template <>
		inline std::string getAttribute(const XmlNode* _node, const std::string& _name)
		{
			if (_node->attributes.count(_name) > 0)
			{
				return _node->attributes.at(_name);
			}
			return "";
		}

		template <>
		inline bool getAttribute(const XmlNode* _node, const std::string& _name)
		{
			if (_node->attributes.count(_name) > 0)
			{
				return _node->attributes.at(_name) == "true";
			}
			return false;
		}

		template <>
		inline unsigned getAttribute(const XmlNode* _node, const std::string& _name)
		{
			if (_node->attributes.count(_name) > 0)
			{
				return (unsigned)std::atol(_node->attributes.at(_name).c_str());
			}
			return 0;
		}

		template <>
		inline int getAttribute(const XmlNode* _node, const std::string& _name)
		{
			if (_node->attributes.count(_name) > 0)
			{
				return std::atoi(_node->attributes.at(_name).c_str());
			}
			return 0;
		}

		template <>
		inline float getAttribute(const XmlNode* _node, const std::string& _name)
		{
			if (_node->attributes.count(_name) > 0)
			{
				return (float)std::atof(_node->attributes.at(_name).c_str());
			}
			return 0.0f;
		}
	}
}