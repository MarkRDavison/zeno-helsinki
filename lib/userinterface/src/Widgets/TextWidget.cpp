#include <helsinki/UserInterface/Widgets/TextWidget.hpp>
#include <helsinki/UserInterface/UiScene.hpp>
#include <helsinki/System/Resource/ResourceManager.hpp>
#include <helsinki/Renderer/Resource/FontResource.hpp>
#include <iostream>
#include <algorithm>

namespace hl
{

	void TextWidget::OnPaint(UiScene& a_Scene, DrawList& a_DrawList)
	{        
        const LayoutNode* node = a_Scene.Layouts.Get(GetLayoutID());
        if (!node)
        {
            return;
        }

        const Rectf& rect = node->Layout.FinalRect;

        Vec2f offset = rect.TopLeft();

        auto offsetHalf = rect.Size / 2.0f - _generatedBounds.Size / 2.0f;

        a_DrawList.AddRect(Vec4f(0.0f, 1.0f, 1.0f, 1.0f), rect);

        auto textColor = Vec4f(0.92f, 0.92f, 0.92f, 1.0f);
        // TODO: Change text color based on parent button.... maybe button changes child color somehow...
        a_DrawList.AddText(textColor, offset + offsetHalf, _generatedVertices, 0);
	}

    void TextWidget::setText(UiScene& a_Scene, const std::string& text, const std::string& font)
    {
        _text = text;
        _font = font;

        // calculate text stuff...

        LayoutNode* layoutNode = a_Scene.Layouts.Get(a_Scene.GetWidget(this->GetID())->GetLayoutID());

        // Somehow need to know the parent size and if that doesnt allow the full text then truncate...?

        if (_autoTruncate && (
            layoutNode->Parent->Style.WidthMode != ESizingMode::Fixed ||
            layoutNode->Parent->Style.HeightMode != ESizingMode::Fixed))
        {
            std::cout << "Text must be within fixed size parent" << std::endl;
        }

        const auto& rm = a_Scene.getResourceManager();

        auto fontRes = rm.GetResource<FontResource>(font);

        if (fontRes == nullptr)
        {
            return;
        }

        // TODO: Calculate min/max/bounds as generated
        auto vertexes = fontRes->generateTextVertexes(text, 48);

        Vec2f min{};
        Vec2f max{};

        for (const auto& vertex : vertexes)
        {
            min.data[0] = std::min(min.x(), vertex.pos.x);
            min.data[1] = std::min(min.y(), vertex.pos.y);

            max.data[0] = std::max(max.x(), vertex.pos.x);
            max.data[1] = std::max(max.y(), vertex.pos.y);
        }

        _generatedBounds = Rectf::FromMinMax(min, max);

        _generatedVertices.clear();
        for (const auto& v : vertexes)
        {
            _generatedVertices.push_back(Vec4f(v.pos.x, v.pos.y, v.texCoord.x, v.texCoord.y));
        }
    }

}