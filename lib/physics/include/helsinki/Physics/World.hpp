#pragma once

#include <helsinki/Physics/Context.hpp>
#include <helsinki/Physics/Types.hpp>
#include <helsinki/System/Utils/NonCopyable.hpp>
#include <memory>

namespace hl::physics
{
	class World : public hl::NonCopyable
	{
	public:
		World(Context& context, const WorldSettings& settings);
		~World() override;

		void step(float delta);

		BodyId createBody(const BodyDesc& desc);
		void destroyBody(BodyId id);

		Pose getPose(BodyId id) const;
		void setPose(BodyId id, const Pose& pose);
		glm::vec3 getLinearVelocity(BodyId id) const;
		void setLinearVelocity(BodyId id, glm::vec3 velocity);
		void addImpulse(BodyId id, glm::vec3 impulse);
		void addForce(BodyId id, glm::vec3 force);

	private:
		struct Impl;
		std::unique_ptr<Impl> _impl;
	};
}
