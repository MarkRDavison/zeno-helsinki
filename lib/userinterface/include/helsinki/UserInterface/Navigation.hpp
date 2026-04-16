#pragma once

#include <helsinki/System/Types.hpp>
#include <helsinki/UserInterface/Layout.hpp>

#include <ranges>
#include <limits>

namespace hl
{
    enum class ENavAction : std::uint8_t
    {
        None = 0,

        MoveLeft,
        MoveRight,
        MoveUp,
        MoveDown,

        Activate,  ///< Activate the currently focused item, e.g., XBox A button, Enter key, etc.
        Cancel,    ///< Cancel or go back, e.g., XBox B button, Escape key, etc.
    };

    template<std::ranges::input_range Range>
        requires std::convertible_to<std::ranges::range_value_t<Range>, const LayoutNode*>
    const LayoutNode* FindNavigatableNode(
        ENavAction a_Action,
        LayoutNode* a_Start,
        Range&& a_Nodes,
        f32 a_LateralPenaltyWeight = 2.f)
    {
        if (!a_Start)
            return nullptr; // Invalid starting node

        // Determine the preferred direction vector
        Vec2f dir{};
        switch (a_Action)
        {
        case ENavAction::MoveLeft:  dir = { -1,  0 }; break;
        case ENavAction::MoveRight: dir = { 1,  0 }; break;
        case ENavAction::MoveUp:    dir = { 0, -1 }; break;
        case ENavAction::MoveDown:  dir = { 0,  1 }; break;
        default:                    return nullptr; // Unsupported action
        }

        const Vec2f startCenter = a_Start->Layout.FinalRect.Center();

        const LayoutNode* best = nullptr;
        f32 bestScore = std::numeric_limits<f32>::max();

        for (const LayoutNode* candidate : a_Nodes)
        {
            if (candidate == a_Start) continue;

            Vec2f candidateCenter = candidate->Layout.FinalRect.Center();
            Vec2f delta = candidateCenter - startCenter;

            // Reject candidates not in the movement direction
            const f32 forward = dot(delta, dir);
            if (forward <= 0) continue;

            // Score candidates based on a combination of forward distance and lateral deviation
            f32 lateral = length(delta - dir * forward);
            f32 score = forward + lateral * a_LateralPenaltyWeight;

            if (score < bestScore)
            {
                bestScore = score;
                best = candidate;
            }
        }

        return best;
    }
}