#include <helsinki/Physics/World.hpp>

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/ObjectLayer.h>
#include <Jolt/Physics/Collision/BroadPhase/BroadPhaseLayer.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <stdexcept>
#include <thread>

namespace hl::physics
{
	namespace
	{
		constexpr JPH::ObjectLayer cObjectNonMoving = 0;
		constexpr JPH::ObjectLayer cObjectMoving = 1;
		constexpr JPH::BroadPhaseLayer cBroadphaseNonMoving(0);
		constexpr JPH::BroadPhaseLayer cBroadphaseMoving(1);
		constexpr unsigned cBroadphaseCount = 2;
		constexpr float c2dBoxHalfExtentZ = 0.1f;

		JPH::ObjectLayer toObjectLayer(Layer layer)
		{
			switch (layer)
			{
			case Layer::NonMoving:
				return cObjectNonMoving;
			case Layer::Moving:
			case Layer::Sensor:
				return cObjectMoving;
			}
			return cObjectMoving;
		}

		JPH::EMotionType toMotionType(MotionType motion)
		{
			switch (motion)
			{
			case MotionType::Static:
				return JPH::EMotionType::Static;
			case MotionType::Kinematic:
				return JPH::EMotionType::Kinematic;
			case MotionType::Dynamic:
				return JPH::EMotionType::Dynamic;
			}
			return JPH::EMotionType::Dynamic;
		}

		JPH::RVec3 toRVec3(glm::vec3 v)
		{
			return JPH::RVec3(v.x, v.y, v.z);
		}

		JPH::Vec3 toVec3(glm::vec3 v)
		{
			return JPH::Vec3(v.x, v.y, v.z);
		}

		JPH::Quat toQuat(glm::quat q)
		{
			return JPH::Quat(q.x, q.y, q.z, q.w);
		}

		glm::vec3 toGlm(JPH::Vec3 v)
		{
			return glm::vec3(v.GetX(), v.GetY(), v.GetZ());
		}

		glm::quat toGlm(JPH::Quat q)
		{
			return glm::quat(q.GetW(), q.GetX(), q.GetY(), q.GetZ());
		}

		BodyId toBodyId(JPH::BodyID id)
		{
			BodyId result;
			result.bits = id.GetIndexAndSequenceNumber();
			return result;
		}

		JPH::BodyID toJoltId(BodyId id)
		{
			return JPH::BodyID(id.bits);
		}

		class BroadPhaseLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
		{
		public:
			JPH::uint GetNumBroadPhaseLayers() const override
			{
				return cBroadphaseCount;
			}

			JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer layer) const override
			{
				return layer == cObjectNonMoving ? cBroadphaseNonMoving : cBroadphaseMoving;
			}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
			const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const override
			{
				switch (static_cast<JPH::BroadPhaseLayer::Type>(layer))
				{
				case static_cast<JPH::BroadPhaseLayer::Type>(cBroadphaseNonMoving):
					return "NON_MOVING";
				case static_cast<JPH::BroadPhaseLayer::Type>(cBroadphaseMoving):
					return "MOVING";
				default:
					return "INVALID";
				}
			}
#endif
		};

		class ObjectVsBroadPhaseLayerFilterImpl final : public JPH::ObjectVsBroadPhaseLayerFilter
		{
		public:
			bool ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadphase) const override
			{
				if (layer == cObjectNonMoving)
				{
					return broadphase == cBroadphaseMoving;
				}
				return true;
			}
		};

		class ObjectLayerPairFilterImpl final : public JPH::ObjectLayerPairFilter
		{
		public:
			bool ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const override
			{
				if (a == cObjectNonMoving)
				{
					return b == cObjectMoving;
				}
				if (b == cObjectNonMoving)
				{
					return a == cObjectMoving;
				}
				return true;
			}
		};
	}

	struct World::Impl
	{
		Context* context = nullptr;
		Dim dim = Dim::D3;
		BroadPhaseLayerInterfaceImpl broadPhaseLayers;
		ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhase;
		ObjectLayerPairFilterImpl objectLayerPairs;
		JPH::TempAllocatorImpl tempAllocator;
		JPH::JobSystemThreadPool jobSystem;
		JPH::PhysicsSystem physics;

		Impl()
			: tempAllocator(10 * 1024 * 1024)
		{
		}
	};

	World::World(Context& context, const WorldSettings& settings)
		: _impl(std::make_unique<Impl>())
	{
		if (!context.ok())
		{
			throw std::runtime_error("hl::physics::World requires a live Context");
		}

		_impl->context = &context;
		_impl->dim = settings.dim;

		const unsigned hardware = std::thread::hardware_concurrency();
		const int workers = static_cast<int>(hardware > 1 ? hardware - 1 : 1);
		_impl->jobSystem.Init(
			static_cast<JPH::uint>(JPH::cMaxPhysicsJobs),
			static_cast<JPH::uint>(JPH::cMaxPhysicsBarriers),
			workers);

		const JPH::uint maxBodies = std::max<JPH::uint>(1, settings.maxBodies);
		_impl->physics.Init(
			maxBodies,
			0,
			maxBodies,
			maxBodies,
			_impl->broadPhaseLayers,
			_impl->objectVsBroadPhase,
			_impl->objectLayerPairs);
		_impl->physics.SetGravity(toVec3(settings.gravity));
	}

	World::~World()
	{
		if (!_impl)
		{
			return;
		}

		JPH::BodyIDVector ids;
		_impl->physics.GetBodies(ids);
		JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		for (const JPH::BodyID id : ids)
		{
			if (bodies.IsAdded(id))
			{
				bodies.RemoveBody(id);
			}
			bodies.DestroyBody(id);
		}
	}

	void World::step(float delta)
	{
		if (!_impl->context || !_impl->context->ok())
		{
			throw std::runtime_error("hl::physics::World::step requires a live Context");
		}

		ZoneScopedN("PhysicsStep");
		_impl->physics.Update(delta, 1, &_impl->tempAllocator, &_impl->jobSystem);
	}

	BodyId World::createBody(const BodyDesc& desc)
	{
		const bool dim2 = _impl->dim == Dim::D2;
		glm::vec3 halfExtents = desc.shape.halfExtents;
		if (dim2 && desc.shape.kind == Shape::Kind::Box)
		{
			halfExtents.z = c2dBoxHalfExtentZ;
		}

		JPH::RefConst<JPH::Shape> shape;
		switch (desc.shape.kind)
		{
		case Shape::Kind::Box:
			shape = new JPH::BoxShape(toVec3(halfExtents));
			break;
		case Shape::Kind::Sphere:
			shape = new JPH::SphereShape(desc.shape.radius);
			break;
		case Shape::Kind::Capsule:
		case Shape::Kind::Cylinder:
			throw std::runtime_error("hl::physics::World::createBody: shape kind not implemented");
		}

		glm::vec3 position = desc.pose.position;
		if (dim2)
		{
			position.z = 0.f;
		}

		JPH::BodyCreationSettings settings(
			shape,
			toRVec3(position),
			toQuat(desc.pose.rotation),
			toMotionType(desc.motion),
			toObjectLayer(desc.layer));
		settings.mFriction = desc.friction;
		settings.mRestitution = desc.restitution;
		settings.mIsSensor = desc.sensor;
		settings.mUserData = desc.userData;
		if (dim2 && desc.motion != MotionType::Static)
		{
			settings.mAllowedDOFs = JPH::EAllowedDOFs::Plane2D;
		}
		if (desc.motion != MotionType::Static && desc.mass > 0.f)
		{
			settings.mOverrideMassProperties = JPH::EOverrideMassProperties::CalculateInertia;
			settings.mMassPropertiesOverride.mMass = desc.mass;
		}

		const JPH::EActivation activation = desc.motion == MotionType::Static
			? JPH::EActivation::DontActivate
			: JPH::EActivation::Activate;
		const JPH::BodyID id = _impl->physics.GetBodyInterface().CreateAndAddBody(settings, activation);
		if (id.IsInvalid())
		{
			return BodyId::invalid();
		}
		return toBodyId(id);
	}

	void World::destroyBody(BodyId id)
	{
		if (!id.valid())
		{
			return;
		}

		const JPH::BodyID joltId = toJoltId(id);
		JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return;
		}
		bodies.RemoveBody(joltId);
		bodies.DestroyBody(joltId);
	}

	Pose World::getPose(BodyId id) const
	{
		if (!id.valid())
		{
			return Pose{};
		}

		const JPH::BodyID joltId = toJoltId(id);
		const JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return Pose{};
		}

		JPH::RVec3 position;
		JPH::Quat rotation;
		bodies.GetPositionAndRotation(joltId, position, rotation);
		return Pose{ .position = toGlm(position), .rotation = toGlm(rotation) };
	}

	void World::setPose(BodyId id, const Pose& pose)
	{
		if (!id.valid())
		{
			return;
		}

		const JPH::BodyID joltId = toJoltId(id);
		JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return;
		}
		glm::vec3 position = pose.position;
		if (_impl->dim == Dim::D2)
		{
			position.z = 0.f;
		}
		bodies.SetPositionAndRotation(joltId, toRVec3(position), toQuat(pose.rotation), JPH::EActivation::Activate);
	}

	glm::vec3 World::getLinearVelocity(BodyId id) const
	{
		if (!id.valid())
		{
			return glm::vec3{0.f};
		}

		const JPH::BodyID joltId = toJoltId(id);
		const JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return glm::vec3{0.f};
		}
		return toGlm(bodies.GetLinearVelocity(joltId));
	}

	void World::setLinearVelocity(BodyId id, glm::vec3 velocity)
	{
		if (!id.valid())
		{
			return;
		}

		const JPH::BodyID joltId = toJoltId(id);
		JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return;
		}
		bodies.SetLinearVelocity(joltId, toVec3(velocity));
	}

	void World::addImpulse(BodyId id, glm::vec3 impulse)
	{
		if (!id.valid())
		{
			return;
		}

		const JPH::BodyID joltId = toJoltId(id);
		JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return;
		}
		bodies.AddImpulse(joltId, toVec3(impulse));
	}

	void World::addForce(BodyId id, glm::vec3 force)
	{
		if (!id.valid())
		{
			return;
		}

		const JPH::BodyID joltId = toJoltId(id);
		JPH::BodyInterface& bodies = _impl->physics.GetBodyInterface();
		if (!bodies.IsAdded(joltId))
		{
			return;
		}
		bodies.AddForce(joltId, toVec3(force));
	}
}
