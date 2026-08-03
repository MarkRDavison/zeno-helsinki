#include <Ui/UiElement.hpp>

namespace hur
{

	class UiPanel : public UiElement
	{
	public:
		glm::vec3 colour;

		void draw(hl::UiRoot& root, glm::vec2 screenSize) override;
	};

}