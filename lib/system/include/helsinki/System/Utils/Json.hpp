#pragma once 

#include <vector>
#include <string>
#include <functional>

namespace hl
{

	class JsonToken
	{
	public:
		enum class Type
		{
			None = 0,
			ObjectStart,
			ObjectEnd,
			ArrayStart,
			ArrayEnd,
			ValueSeperator,
			Comma,
			ValueString,
			ValueNumber,
			ValueInteger,
			ValueBoolean
		} type;

		std::string content;
	};

	class JsonNode
	{
	public:
		~JsonNode();
		enum class Type
		{
			None = 0,
			Object,
			Array,
			ValueString,
			ValueNumber,
			ValueInteger,
			ValueBoolean
		} type;
		std::string name;
		JsonNode* parent{ nullptr };
		std::vector<JsonNode*> children;
		std::string content;
		float number;
		int integer;
		bool boolean;

		JsonNode& operator[](const std::string& _name) const;
		JsonNode& operator[](std::size_t _index) const;
	};

	class JsonDocument
	{
	public:
		JsonDocument();
		JsonDocument(const JsonDocument& _document);
		~JsonDocument();

		void readFromText(const std::string& _text);
		void parse(const std::string& _text);
		const std::string& getInputRepresentation() const;
		const std::string dump(int _indentation = 4) const;

		std::function<void(JsonToken)> tokenFound;
		JsonNode* m_Root{ nullptr };

	private:
		std::string m_Text;
	};

	class Json
	{
		Json() = delete;
		~Json() = delete;
	public:
		static JsonDocument parseFromText(const std::string& _text);
		static void createTreeFromStreamingDocument(JsonDocument& _document);
	};
}