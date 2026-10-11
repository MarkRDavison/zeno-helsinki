#pragma once

#include <helsinki/Ui/Export.hpp>

#include <helsinki/Ui/Widget.hpp>

#include <optional>

namespace hl::ui
{
	class HELSINKI_UI_EXPORT ProgressBar : public Widget
	{
	public:
		explicit ProgressBar(Node& node);

		void prepare() override;
		void paint(IPaint& paint) const override;

		void setValue(float value);
		float value() const { return _value; }

		std::optional<glm::vec3> trackColor;
		std::optional<glm::vec3> fillColor;

		glm::vec3 resolvedTrackColor() const { return resolve(trackColor, theme().well); }
		glm::vec3 resolvedFillColor() const { return resolve(fillColor, theme().accent); }

	private:
		float _value = 0.0f;
	};
}
