#include <helsinki/System/Utils/Json.hpp>
#include <cassert>

#define TOKEN_START_OBJECT '{'
#define TOKEN_CLOSE_OBJECT '}'
#define TOKEN_START_ARRAY '['
#define TOKEN_CLOSE_ARRAY ']'
#define TOKEN_VALUE_SEP ':'
#define TOKEN_COMMA ','
#define TOKEN_QUOTE '"'
#define WHITESPACE " \t\n\r"
#define VALUE_AFTER_VALUE " \t\n\r,]}"
#define NUMBER_SEPERATOR '.'

namespace hl
{

	JsonDocument::JsonDocument()
	{
	}
	JsonDocument::JsonDocument(const JsonDocument& _document)
	{
		this->m_Root = _document.m_Root;
		this->m_Text = std::string(_document.m_Text);
	}
	JsonDocument::~JsonDocument()
	{
		if (m_Root != nullptr)
		{
			delete m_Root;
		}
	}

	void JsonDocument::readFromText(const std::string& _text)
	{
		m_Text = _text;
	}
	void JsonDocument::parse(const std::string& _text)
	{
		m_Text = _text;
		std::string text(_text);

		while (!text.empty())
		{
			std::size_t nonWhitespaceIndex = text.find_first_not_of(WHITESPACE);
			if (nonWhitespaceIndex == std::string::npos) { break; }

			char c = text[nonWhitespaceIndex];
			JsonToken token;
			switch (c)
			{
			case TOKEN_START_OBJECT:
			{
				text.erase(0, nonWhitespaceIndex + 1);
				token.type = JsonToken::Type::ObjectStart;
				tokenFound(token);
				break;
			}
			case TOKEN_CLOSE_OBJECT:
			{
				text.erase(0, nonWhitespaceIndex + 1);
				token.type = JsonToken::Type::ObjectEnd;
				tokenFound(token);
				break;
			}
			case TOKEN_START_ARRAY:
			{
				text.erase(0, nonWhitespaceIndex + 1);
				token.type = JsonToken::Type::ArrayStart;
				tokenFound(token);
				break;
			}
			case TOKEN_CLOSE_ARRAY:
			{
				text.erase(0, nonWhitespaceIndex + 1);
				token.type = JsonToken::Type::ArrayEnd;
				tokenFound(token);
				break;
			}
			case TOKEN_QUOTE:
			{
				std::size_t quoteStart = nonWhitespaceIndex;
				std::size_t quoteEnd = text.find(TOKEN_QUOTE, quoteStart + 1);

				assert(quoteEnd != std::string::npos);
				assert(quoteStart < quoteEnd);

				token.type = JsonToken::Type::ValueString;
				token.content = text.substr(quoteStart + 1, quoteEnd - quoteStart - 1);
				text.erase(0, quoteEnd + 1);
				tokenFound(token);

				break;
			}
			case TOKEN_VALUE_SEP:
			{
				text.erase(0, nonWhitespaceIndex + 1);
				token.type = JsonToken::Type::ValueSeperator;
				tokenFound(token);
				break;
			}
			case TOKEN_COMMA:
			{
				text.erase(0, nonWhitespaceIndex + 1);
				token.type = JsonToken::Type::Comma;
				tokenFound(token);
				break;
			}
			default:
			{
				std::size_t valueEnd = text.find_first_of(VALUE_AFTER_VALUE, nonWhitespaceIndex + 1);
				assert(valueEnd != std::string::npos);
				token.content = text.substr(nonWhitespaceIndex, valueEnd - nonWhitespaceIndex);
				if (token.content.find(NUMBER_SEPERATOR) != std::string::npos)
				{
					token.type = JsonToken::Type::ValueNumber;
				}
				else if (
					token.content == "true" ||
					token.content == "True" ||
					token.content == "TRUE" ||
					token.content == "false" ||
					token.content == "False" ||
					token.content == "FALSE")
				{
					token.type = JsonToken::Type::ValueBoolean;
				}
				else
				{
					token.type = JsonToken::Type::ValueInteger;
				}
				tokenFound(token);
				text.erase(0, valueEnd);
				break;
			}
			}
		}
	}
	const std::string& JsonDocument::getInputRepresentation() const
	{
		return m_Text;
	}
	std::string recursiveDump(int _indentation, int _level, const JsonNode& _node, std::string _input)
	{

		std::string output;

		switch (_node.type)
		{
		case JsonNode::Type::Object:
		{
			output += _input + std::string(_level * _indentation, ' ');

			if (!_node.name.empty())
			{
				output += "\"" + _node.name + "\": ";
			}

			output += "{\n";

			for (auto child : _node.children)
			{
				output += recursiveDump(_indentation, _level + 1, *child, "");
			}

			output = output + std::string(_level * _indentation, ' ') + "},\n";
		}
		break;
		case JsonNode::Type::ValueString:
		case JsonNode::Type::ValueInteger:
		case JsonNode::Type::ValueNumber:
		case JsonNode::Type::ValueBoolean:
		{
			output += _input + std::string(_level * _indentation, ' ');

			auto surround = _node.type == JsonNode::Type::ValueString ? "\"" : "";

			output += "\"" + _node.name + "\": " + surround + _node.content + surround + ",\n";

		}
		break;
		case JsonNode::Type::Array:
		{
			output += _input + std::string(_level * _indentation, ' ');

			if (!_node.name.empty())
			{
				output += "\"" + _node.name + "\": ";
			}

			output += "[\n";

			for (auto child : _node.children)
			{
				output += recursiveDump(_indentation, _level + 1, *child, "");
			}

			output = output + std::string(_level * _indentation, ' ') + "]\n";
		}
		break;
		default:
			break;
		}

		return output;
	}
	const std::string JsonDocument::dump(int _indentation /*= 4*/) const
	{
		if (m_Root == nullptr)
		{
			return "NOT PARSED";
		}
		JsonNode& root = *m_Root;

		return recursiveDump(_indentation, 0, root, "");
	}
	JsonDocument Json::parseFromText(const std::string& _text)
	{
		JsonDocument doc;
		doc.readFromText(_text);
		createTreeFromStreamingDocument(doc);

		return doc;
	}
	void Json::createTreeFromStreamingDocument(JsonDocument& _document)
	{
		JsonNode* root{ nullptr };
		JsonNode* current{ nullptr };
		JsonNode* next{ nullptr };

		_document.tokenFound = [&](JsonToken _token) -> void
			{

				switch (_token.type)
				{
				case JsonToken::Type::ObjectStart:
				{
					if (root == nullptr)
					{
						current = new JsonNode();
						current->type = JsonNode::Type::Object;
						root = current;
					}
					else if (next != nullptr)
					{
						// We are creating a new object, and it has a name from previous tokens
						next->type = JsonNode::Type::Object;
						current = next;
						next = nullptr;
					}
					else if (current->type == JsonNode::Type::Array)
					{
						assert(next == nullptr);
						// We are creating a new object in an array, so it has no name
						next = new JsonNode();
						next->type = JsonNode::Type::Object;
						next->parent = current;
						current->children.push_back(next);
						current = next;
						next = nullptr;
					}
					else
					{
						assert(false);
					}
					break;
				}
				case JsonToken::Type::ObjectEnd:
				{
					assert(current != nullptr && current->type == JsonNode::Type::Object);
					current = current->parent;
					break;
				}
				case JsonToken::Type::ArrayStart:
				{
					if (root == nullptr)
					{
						current = new JsonNode();
						current->type = JsonNode::Type::Array;
						root = current;
					}
					else if (next != nullptr)
					{
						// We are creating a new array, and it has a name from previous tokens
						next->type = JsonNode::Type::Array;
						current = next;
						next = nullptr;
					}
					else if (current->type == JsonNode::Type::Array)
					{
						// Nested array
						next = new JsonNode();
						next->type = JsonNode::Type::Array;
						next->parent = current;
						current->children.push_back(next);
						current = next;
						next = nullptr;
					}
					else
					{
						assert(false);
					}
					break;
				}
				case JsonToken::Type::ArrayEnd:
				{
					assert(current != nullptr && current->type == JsonNode::Type::Array);
					current = current->parent;
					break;
				}
				case JsonToken::Type::ValueSeperator:
				{
					assert(next != nullptr);
					break;
				}
				case JsonToken::Type::Comma:
				{
					assert(next == nullptr);
					break;
				}
				case JsonToken::Type::ValueString:
				{
					if (next != nullptr && next->type == JsonNode::Type::None)
					{
						next->type = JsonNode::Type::ValueString;
						next->content = _token.content;
						next = nullptr;
					}
					else if (current->type == JsonNode::Type::Object)
					{
						next = new JsonNode();
						next->parent = current;
						next->name = _token.content;
						current->children.push_back(next);
					}
					else if (current->type == JsonNode::Type::Array)
					{
						next = new JsonNode();
						next->type = JsonNode::Type::ValueString;
						next->content = _token.content;
						current->children.push_back(next);
						next = nullptr;
					}
					else
					{
						assert(false);
					}
					break;
				}
				case JsonToken::Type::ValueNumber:
				{
					const float value = stof(_token.content);
					if (next != nullptr && next->type == JsonNode::Type::None)
					{
						next->type = JsonNode::Type::ValueNumber;
						next->content = _token.content;
						next->number = value;
						next = nullptr;
					}
					else if (current->type == JsonNode::Type::Object)
					{
						assert(false);
					}
					else if (current->type == JsonNode::Type::Array)
					{
						next = new JsonNode();
						next->type = JsonNode::Type::ValueNumber;
						next->content = _token.content;
						next->number = value;
						current->children.push_back(next);
						next = nullptr;
					}
					else
					{
						assert(false);
					}
					break;
				}
				case JsonToken::Type::ValueInteger:
				{
					const int value = stoi(_token.content);
					if (next != nullptr && next->type == JsonNode::Type::None)
					{
						next->type = JsonNode::Type::ValueInteger;
						next->content = _token.content;
						next->integer = value;
						next = nullptr;
					}
					else if (current->type == JsonNode::Type::Object)
					{
						assert(false);
					}
					else if (current->type == JsonNode::Type::Array)
					{
						next = new JsonNode();
						next->type = JsonNode::Type::ValueInteger;
						next->content = _token.content;
						next->integer = value;
						current->children.push_back(next);
						next = nullptr;
					}
					else
					{
						assert(false);
					}
					break;
				}
				case JsonToken::Type::ValueBoolean:
				{
					const bool value = "true" == _token.content;
					if (next != nullptr && next->type == JsonNode::Type::None)
					{
						next->type = JsonNode::Type::ValueBoolean;
						next->content = _token.content;
						next->boolean = value;
						next = nullptr;
					}
					else if (current->type == JsonNode::Type::Object)
					{
						assert(false);
					}
					else if (current->type == JsonNode::Type::Array)
					{
						next = new JsonNode();
						next->type = JsonNode::Type::ValueBoolean;
						next->content = _token.content;
						next->boolean = value;
						current->children.push_back(next);
						next = nullptr;
					}
					else
					{
						assert(false);
					}
					break;
				}
				default:
					assert(false);
				}
			};

		_document.parse(_document.getInputRepresentation());

		_document.m_Root = root;
	}

	JsonNode::~JsonNode()
	{
		for (auto c : children)
		{
			delete c;
		}
	}
	JsonNode& JsonNode::operator[](const std::string& _name) const
	{
		for (auto c : children)
		{
			if (c->name == _name)
			{
				return *c;
			}
		}

		throw std::string("Invalid key for json object: ") + _name;
	}
	JsonNode& JsonNode::operator[](std::size_t _index) const
	{
		return *children.at(_index);
	}
}