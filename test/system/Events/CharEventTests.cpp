#include <catch2/catch_test_macros.hpp>
#include <helsinki/System/Events/CharEvent.hpp>
#include <helsinki/System/Events/KeyEvents.hpp>
#include <memory>
#include <string>

TEST_CASE("CharEvent stores codepoint and clones", "[System][Events]")
{
	hl::CharEvent typed(static_cast<uint32_t>('A'));
	CHECK(typed.codepoint() == static_cast<uint32_t>('A'));
	CHECK(std::string(typed.GetType()) == "CharEvent");
	CHECK(std::string(hl::CharEvent::GetStaticType()) == "CharEvent");

	std::unique_ptr<hl::Event> clone(typed.Clone());
	auto* asChar = dynamic_cast<hl::CharEvent*>(clone.get());
	REQUIRE(asChar != nullptr);
	CHECK(asChar->codepoint() == static_cast<uint32_t>('A'));

	hl::CharEvent zed(static_cast<uint32_t>('z'));
	CHECK(zed.codepoint() == static_cast<uint32_t>('z'));
}

TEST_CASE("KeyRepeatEvent is distinct from KeyPressEvent", "[System][Events]")
{
	hl::KeyRepeatEvent repeat(65);
	hl::KeyPressEvent press(65);

	CHECK(std::string(repeat.GetType()) == "KeyRepeatEvent");
	CHECK(std::string(press.GetType()) == "KeyPressEvent");
	CHECK(repeat.GetKeyCode() == 65);

	std::unique_ptr<hl::Event> clone(repeat.Clone());
	auto* asRepeat = dynamic_cast<hl::KeyRepeatEvent*>(clone.get());
	REQUIRE(asRepeat != nullptr);
	CHECK(asRepeat->GetKeyCode() == 65);
	CHECK(dynamic_cast<hl::KeyPressEvent*>(clone.get()) == nullptr);
}
