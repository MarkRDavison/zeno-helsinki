#pragma once

#include <helsinki/System/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

namespace hl::physics
{
	enum class Dim
	{
		D2,
		D3
	};

	enum class MotionType
	{
		Static,
		Kinematic,
		Dynamic
	};

	enum class Layer
	{
		NonMoving,
		Moving,
		Sensor
	};

	struct BodyId
	{
		std::uint32_t bits = 0xffffffffu;

		static BodyId invalid()
		{
			return BodyId{};
		}

		bool valid() const
		{
			return bits != 0xffffffffu;
		}

		friend bool operator==(BodyId lhs, BodyId rhs)
		{
			return lhs.bits == rhs.bits;
		}
	};

	struct CharacterId
	{
		std::uint32_t bits = 0xffffffffu;

		static CharacterId invalid()
		{
			return CharacterId{};
		}

		bool valid() const
		{
			return bits != 0xffffffffu;
		}

		friend bool operator==(CharacterId lhs, CharacterId rhs)
		{
			return lhs.bits == rhs.bits;
		}
	};

	struct Pose
	{
		glm::vec3 position{0.f};
		glm::quat rotation{1.f, 0.f, 0.f, 0.f};
	};

	struct Shape
	{
		enum class Kind
		{
			Sphere,
			Box,
			Capsule,
			Cylinder
		};

		Kind kind = Kind::Box;
		glm::vec3 halfExtents{0.5f};
		float radius = 0.5f;
		float height = 1.f;
	};

	struct BodyDesc
	{
		Shape shape;
		MotionType motion = MotionType::Dynamic;
		Layer layer = Layer::Moving;
		Pose pose;
		float mass = 1.f;
		float friction = 0.2f;
		float restitution = 0.f;
		bool sensor = false;
		bool oneWay = false;
		std::uint64_t userData = 0;
	};

	struct WorldSettings
	{
		Dim dim = Dim::D3;
		glm::vec3 gravity{0.f, -9.81f, 0.f};
		std::uint32_t maxBodies = 1024;
	};

	struct Contact
	{
		BodyId bodyA;
		BodyId bodyB;
		glm::vec3 point{0.f};
		glm::vec3 normal{0.f};
	};

	struct RayHit
	{
		BodyId body;
		float fraction = 0.f;
		glm::vec3 point{0.f};
		glm::vec3 normal{0.f};
	};

	struct CharacterDesc
	{
		Dim dim = Dim::D3;
		Pose pose;
		float capsuleRadius = 0.4f;
		float capsuleHeight = 1.2f;
		float maxSlopeAngleDeg = 45.f;
		float jumpSpeed = 6.f;
	};
}
