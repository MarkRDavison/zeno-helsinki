#include <helsinki/System/Utils/Xml.hpp>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>

#define INVALID_INDEX 0xffffffff
#define OPEN_TAG '<'
#define CLOSE_TAG '>'
#define F_SLASH '/'
#define B_SLASH '\\'
#define TAG_NAME_CLOSE_CHARS "/\\> "
#define ATTRIBUTE_QUOTE_SINGLE '\''
#define ATTRIBUTE_QUOTE_DOUBLE '\"'
#define ATTRIBUTE_QUOTES "'\""
#define ATTRIBUTE_EQUALS '='
#define WHITESPACE " \t\n\r"

namespace hl
{

	std::vector<std::string> split(const std::string& _str, char _delim)
	{
		std::stringstream ss(_str);
		std::string item;
		std::vector<std::string> elems;
		while (std::getline(ss, item, _delim))
		{
			elems.push_back(std::move(item));
		}
		return elems;
	}

	XmlNode::~XmlNode()
	{
		for (auto c : children)
		{
			delete c;
		}
	}
	XmlDocument::XmlDocument()
	{
	}
	XmlDocument::XmlDocument(const XmlDocument& _doc)
	{
		this->m_Node = _doc.m_Node;
		this->m_Text = std::string(_doc.m_Text);
	}
	XmlDocument::~XmlDocument()
	{
		if (m_Node != nullptr)
		{
			delete m_Node;
			m_Node = nullptr;
		}
	}

	bool XmlDocument::readFromFile(const std::string& _file)
	{
		std::ifstream file;
		file.open(_file);

		if (!file.good())
		{
			return false;
		}

		std::string str;

		file.seekg(0, std::ios::end);
		str.reserve(static_cast<unsigned int>(file.tellg()));
		file.seekg(0, std::ios::beg);

		str.insert(str.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		file.close();

		return readFromText(str);
	}
	bool XmlDocument::readFromText(const std::string& _text)
	{
		m_Text = _text;
		return Xml::validate(m_Text);
	}

	bool XmlDocument::parse(const std::string& _text)
	{
		if (!Xml::validate(_text))
		{
			return false;
		}

		std::string text(_text);
		while (!text.empty())
		{
			std::size_t index = 0;
			auto nextTag = getNextTagIndexes(text, index);
			if (nextTag.first == INVALID_INDEX ||
				nextTag.second == INVALID_INDEX)
			{
				return true;
			}

			if (nextTag.first > 0)
			{
				std::string potentialContent = trim(text.substr(0, nextTag.first));
				if (!potentialContent.empty())
				{
					ItemDiscovered item;
					item.type = Type::Content;
					item.content = potentialContent;
					itemDiscovered(item);
				}
			}

			std::string tag = text.substr(nextTag.first, nextTag.second - nextTag.first + 1);
			if (tag.empty())
			{
				return false;
			}

			bool closingTag = tag[1] == F_SLASH;
			if (closingTag)
			{
				tag.erase(1, 1);
			}
			bool selfClosing = tag[tag.size() - 2] == F_SLASH;

			std::string elementName = extractElementName(tag);

			ItemDiscovered item;
			item.name = elementName;
			if (closingTag)
			{
				item.type = Type::CloseElement;
			}
			else
			{
				item.type = selfClosing ? Type::SelfCloseElement : Type::OpenElement;
				std::string attributes = text.substr(
					nextTag.first + elementName.size() + 1,
					nextTag.second - nextTag.first - elementName.size() - (selfClosing ? 1 : 0));

				item.attributes = extractAttributePairs(attributes);
			}
			itemDiscovered(item);
			text.erase(0, nextTag.second + 1);
		}

		return true;
	}

	std::string recursiveDump(int _indentation, int _level, const XmlNode* _node, std::string _input)
	{
		std::string output;

		const bool selfClosing = _node->children.empty() && _node->content.empty();

		if (!_node->content.empty())
		{
			return _input + std::string(_level * _indentation, ' ') + _node->content + '\n';
		}

		std::string attributes;

		for (const auto& kvp : _node->attributes)
		{
			attributes += " " + kvp.first + "='" + kvp.second + "'";
		}

		output += _input + std::string(_level * _indentation, ' ') + "<" + _node->name + attributes + (selfClosing ? " /" : "") + ">\n";

		for (const auto n : _node->children)
		{
			output += recursiveDump(_indentation, _level + 1, n, "");
		}

		if (!selfClosing)
		{
			output += _input + std::string(_level * _indentation, ' ') + "</" + _node->name + ">\n";
		}

		return output;
	}
	std::string XmlDocument::dump(int _indentation /*= 4*/)
	{
		if (m_Node == nullptr)
		{
			return "NOT PARSED";
		}

		return recursiveDump(_indentation, 0, m_Node, "");
	}

	const XmlNode* XmlDocument::select(const std::string& _selection) const
	{
		return select(m_Node, _selection);
	}
	const std::vector<XmlNode*> XmlDocument::selectMany(const std::string& _selection) const
	{
		return selectMany(m_Node, _selection);
	}
	const XmlNode* XmlDocument::select(XmlNode* _start, const std::string& _selection) const
	{
		const auto& many = selectMany(_start, _selection);

		if (many.empty()) { return nullptr; }

		return many.front();
	}
	const std::vector<XmlNode*> XmlDocument::selectMany(XmlNode* _start, const std::string& _selection) const
	{
		auto parts = split(_selection, '/');

		XmlNode* node = _start;
		if (parts.empty() || node->name != parts[0])
		{
			return {};
		}

		parts.erase(parts.begin());
		for (const auto& p : parts)
		{
			for (auto n : node->children)
			{
				if (n->name == p)
				{
					node = n;
					break;
				}
			}

		}
		std::vector<XmlNode*> nodes;
		if (node->parent != nullptr)
		{
			for (auto n : node->parent->children)
			{
				if (n->name == parts.back())
				{
					nodes.push_back(n);
				}
			}
			return nodes;
		}

		return { node };
	}

	const std::string& XmlDocument::getInputRepresentation() const
	{
		return m_Text;
	}

	std::size_t XmlDocument::getNextTagStartIndex(const std::string& _text, std::size_t _current)
	{
		return XmlDocument::getNextOccurenceIndex(_text, _current, OPEN_TAG);
	}
	std::size_t XmlDocument::getNextTagCloseIndex(const std::string& _text, std::size_t _current)
	{
		return XmlDocument::getNextOccurenceIndex(_text, _current, CLOSE_TAG);
	}
	std::size_t XmlDocument::getNextOccurenceIndex(const std::string& _text, std::size_t _current, char _char)
	{
		for (std::size_t i = _current; i < _text.size(); ++i)
		{
			if (_text[i] == _char)
			{
				return i;
			}
		}
		return INVALID_INDEX;
	}
	std::string XmlDocument::extractElementName(const std::string& _elementTag)
	{
		std::size_t elementNameEnd = _elementTag.find_first_of(TAG_NAME_CLOSE_CHARS, 1);
		if (elementNameEnd == std::string::npos)
		{
			return "";
		}
		return trim(_elementTag.substr(1, elementNameEnd - 1));
	}

	std::pair<std::size_t, std::size_t> XmlDocument::getNextTagIndexes(const std::string& _text, std::size_t _current)
	{
		std::size_t startIndex = getNextTagStartIndex(_text, _current);

		if (startIndex == INVALID_INDEX)
		{
			return { INVALID_INDEX, INVALID_INDEX };
		}

		std::size_t closeIndex = getNextTagCloseIndex(_text, startIndex);

		return { startIndex, closeIndex };
	}
	std::unordered_map<std::string, std::string> XmlDocument::extractAttributePairs(const std::string& _text)
	{
		std::unordered_map<std::string, std::string> pairs;

		std::string text(_text);

		while (!text.empty())
		{
			auto equalsLoc = text.find(ATTRIBUTE_EQUALS);
			if (equalsLoc == std::string::npos)
			{
				return pairs;
			}

			auto firstNonWhitespace = text.find_first_not_of(WHITESPACE, 0);
			std::string attributeName = text.substr(firstNonWhitespace, equalsLoc - firstNonWhitespace);

			auto quoteLoc = text.find_first_of(ATTRIBUTE_QUOTES, equalsLoc);
			if (quoteLoc == std::string::npos)
			{
				return pairs;
			}
			char quoteType = text[quoteLoc];
			auto quoteEnd = text.find(quoteType, quoteLoc + 1);
			if (quoteLoc == std::string::npos)
			{
				return pairs;
			}
			std::string attributeValue = text.substr(quoteLoc + 1, quoteEnd - quoteLoc - 1);

			text.erase(0, quoteEnd + 1);

			pairs[attributeName] = attributeValue;
		}

		return pairs;
	}
	std::string XmlDocument::trim(const std::string& _text)
	{
		std::string text(_text);

		std::size_t firstNonWhitespaceIndex = text.find_first_not_of(WHITESPACE);
		std::size_t lastNonWhitespaceIndex = text.find_last_not_of(WHITESPACE);

		if (firstNonWhitespaceIndex == std::string::npos &&
			lastNonWhitespaceIndex == std::string::npos)
		{
			return std::string();
		}

		if (firstNonWhitespaceIndex == std::string::npos)
		{
			firstNonWhitespaceIndex = 0;
		}

		if (lastNonWhitespaceIndex == std::string::npos)
		{
			lastNonWhitespaceIndex = text.size() - 1;
		}

		return text.substr(firstNonWhitespaceIndex, lastNonWhitespaceIndex - firstNonWhitespaceIndex + 1);
	}

	XmlDocument Xml::parseFromFile(const std::string& _file)
	{
		std::ifstream file;
		file.open(_file);

		if (!file.good())
		{
			throw std::exception();
		}

		std::string str;

		file.seekg(0, std::ios::end);
		str.reserve(static_cast<unsigned int>(file.tellg()));
		file.seekg(0, std::ios::beg);

		str.insert(str.begin(), std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());

		file.close();

		return parseFromText(str);
	}
	XmlDocument Xml::parseFromText(const std::string& _text)
	{
		XmlDocument doc;
		doc.readFromText(_text);
		Xml::createTreeFromStreamingDocument(doc);
		return doc;
	}
	bool Xml::validate(const std::string& _text)
	{
		unsigned openTags = 0;
		unsigned closeTags = 0;

		for (std::size_t i = 0; i < _text.size(); ++i)
		{
			switch (_text[i])
			{
			case OPEN_TAG:
				openTags++;
				break;
			case CLOSE_TAG:
				closeTags++;
				break;
			}
		}
		return openTags == closeTags;
	}
	bool Xml::createTreeFromStreamingDocument(XmlDocument& _document)
	{
		XmlNode* root{ nullptr };
		XmlNode* current{ root };

		_document.itemDiscovered = [&](ItemDiscovered _item) -> void
			{
				if (_item.type == Type::OpenElement)
				{
					if (root == nullptr)
					{
						root = new XmlNode();
						//root->type = Type::OpenElement;
						root->name = _item.name;
						for (const auto& kv : _item.attributes)
						{
							root->attributes.insert(kv);
						}
						current = root;
					}
					else if (current != nullptr)
					{
						XmlNode* newNode = new XmlNode();
						//newNode->type = Type::OpenElement;
						newNode->name = _item.name;
						newNode->parent = current;
						current->children.push_back(newNode);
						for (const auto& kv : _item.attributes)
						{
							newNode->attributes.insert(kv);
						}
						current = newNode;
					}
					else
					{
						std::cout << "WTF?" << std::endl;
					}
				}
				else if (_item.type == Type::SelfCloseElement)
				{
					XmlNode* newNode = new XmlNode();
					//newNode->type = Type::OpenElement;
					newNode->name = _item.name;
					newNode->parent = current;
					if (root == nullptr)
					{
						root = newNode;
						current = root;
					}
					else
					{
						current->children.push_back(newNode);
					}
					for (const auto& kv : _item.attributes)
					{
						newNode->attributes.insert(kv);
					}
				}
				else if (_item.type == Type::CloseElement)
				{
					if (current != nullptr && current->parent != nullptr)
					{
						current = current->parent;
					}
				}
				else if (_item.type == Type::Content)
				{
					XmlNode* newNode = new XmlNode();
					//newNode->type = Type::Content;
					newNode->content = _item.content;
					newNode->parent = current;
					current->children.push_back(newNode);
					for (const auto& kv : _item.attributes)
					{
						newNode->attributes.insert(kv);
					}
				}
			};

		bool result = _document.parse(_document.getInputRepresentation());

		_document.m_Node = root;

		return result;
	}
	bool Xml::hasAttribute(const XmlNode* _node, const std::string& _name)
	{
		return _node->attributes.count(_name) > 0;
	}
}