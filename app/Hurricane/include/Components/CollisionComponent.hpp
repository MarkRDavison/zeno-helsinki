#pragma once

#include <string>
#include <helsinki/Engine/ECS/Component.hpp>
#include <helsinki/System/glm.hpp>

namespace hur
{
    enum class CollisionLayer : uint32_t
    {
        None = 0,
        Player = 1 << 0,
        Enemy = 1 << 1,
        PlayerBullet = 1 << 2,
        EnemyBullet = 1 << 3
    };

	class CollisionComponent : public hl::Component
	{
	public:
        // TODO: Store AABB here?
        CollisionLayer layer;
        CollisionLayer mask;
	};

    static bool ShouldTest(const CollisionComponent& a, const CollisionComponent& b)
    {
        return 
            ((uint32_t)a.mask & (uint32_t)b.layer) &&
            ((uint32_t)b.mask & (uint32_t)a.layer);
    }
}