#include <helsinki/Physics/World.hpp>

#include "ObjectLayer.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ShapeFilter.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>

#include <tracy/Tracy.hpp>

#include <algorithm>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <vector>

namespace hl::physics
{
	namespace
	{
		constexpr float c2dBoxHalfExtentZ = 0.1f;

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

		constexpr std::uint32_t cCharacterIndexMask = 0xffffu;
		constexpr std::uint32_t cCharacterGenerationShift = 16;

		CharacterId makeCharacterId(std::uint32_t index, std::uint32_t generation)
		{
			CharacterId id;
			id.bits = (index & cCharacterIndexMask) | (generation << cCharacterGenerationShift);
			if (!id.valid())
			{
				id.bits = (index & cCharacterIndexMask) | ((generation + 1u) << cCharacterGenerationShift);
			}
			return id;
		}

		struct CharacterRecord
		{
			std::uint32_t generation = 0;
			bool occupied = false;
			JPH::Ref<JPH::CharacterVirtual> character;
			glm::vec3 wish{0.f};
			bool climbing = false;
			bool pendingJump = false;
			float jumpSpeed = 6.f;
		};

		struct ContactQueue
		{
			std::mutex mutex;
			std::vector<Contact> contacts;

			void push(const Contact& contact)
			{
				std::lock_guard<std::mutex> lock(mutex);
				contacts.push_back(contact);
			}

			std::vector<Contact> take()
			{
				std::lock_guard<std::mutex> lock(mutex);
				std::vector<Contact> pending;
				pending.swap(contacts);
				return pending;
			}
		};

		Contact makeContact(const JPH::Body& bodyA, const JPH::Body& bodyB, const JPH::ContactManifold& manifold)
		{
			Contact contact;
			contact.bodyA = toBodyId(bodyA.GetID());
			contact.bodyB = toBodyId(bodyB.GetID());
			if (manifold.mRelativeContactPointsOn1.size() > 0)
			{
				contact.point = toGlm(manifold.GetWorldSpaceContactPointOn1(0));
			}
			contact.normal = toGlm(manifold.mWorldSpaceNormal);
			return contact;
		}

		class WorldContactListener final : public JPH::ContactListener
		{
		public:
			explicit WorldContactListener(ContactQueue& queue)
				: _queue(queue)
			{
			}

			JPH::ValidateResult OnContactValidate(
				const JPH::Body&,
				const JPH::Body&,
				JPH::RVec3Arg,
				const JPH::CollideShapeResult&) override
			{
				return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
			}

			void OnContactAdded(
				const JPH::Body& bodyA,
				const JPH::Body& bodyB,
				const JPH::ContactManifold& manifold,
				JPH::ContactSettings&) override
			{
				_queue.push(makeContact(bodyA, bodyB, manifold));
			}

			void OnContactPersisted(
				const JPH::Body& bodyA,
				const JPH::Body& bodyB,
				const JPH::ContactManifold& manifold,
				JPH::ContactSettings&) override
			{
				_queue.push(makeContact(bodyA, bodyB, manifold));
			}

		private:
			ContactQueue& _queue;
		};
	}

	struct World::Impl
	{
		Context* context = nullptr;
		Dim dim = Dim::D3;
		detail::BroadPhaseLayerInterfaceImpl broadPhaseLayers;
		detail::ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhase;
		detail::ObjectLayerPairFilterImpl objectLayerPairs;
		JPH::TempAllocatorImpl tempAllocator;
		JPH::JobSystemThreadPool jobSystem;
		ContactQueue contactQueue;
		WorldContactListener contactListener;
		JPH::PhysicsSystem physics;
		std::function<void(const Contact&)> contactCallback;
		std::vector<CharacterRecord> characters;

		CharacterRecord* characterAt(CharacterId id)
		{
			if (!id.valid())
			{
				return nullptr;
			}
			const std::uint32_t index = id.bits & cCharacterIndexMask;
			const std::uint32_t generation = id.bits >> cCharacterGenerationShift;
			if (index >= characters.size())
			{
				return nullptr;
			}
			CharacterRecord& record = characters[index];
			if (!record.occupied || record.generation != generation || record.character == nullptr)
			{
				return nullptr;
			}
			return &record;
		}

		const CharacterRecord* characterAt(CharacterId id) const
		{
			return const_cast<Impl*>(this)->characterAt(id);
		}

		Impl()
			: tempAllocator(10 * 1024 * 1024)
			, contactListener(contactQueue)
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
		_impl->physics.SetContactListener(&_impl->contactListener);
	}

	World::~World()
	{
		if (!_impl)
		{
			return;
		}

		_impl->physics.SetContactListener(nullptr);

		for (CharacterRecord& record : _impl->characters)
		{
			record.character = nullptr;
			record.occupied = false;
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

		const JPH::Vec3 gravity = _impl->physics.GetGravity();
		const bool dim2 = _impl->dim == Dim::D2;
		for (CharacterRecord& record : _impl->characters)
		{
			if (!record.occupied || record.character == nullptr)
			{
				continue;
			}

			JPH::CharacterVirtual& character = *record.character;
			const bool grounded =
				character.GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
			JPH::Vec3 wish = toVec3(record.wish);
			if (dim2)
			{
				wish.SetZ(0.f);
			}

			JPH::Vec3 velocity;
			if (record.climbing)
			{
				velocity = JPH::Vec3(wish.GetX(), wish.GetY(), wish.GetZ());
			}
			else
			{
				const JPH::Vec3 horizontal(wish.GetX(), 0.f, wish.GetZ());
				if (grounded)
				{
					velocity = character.GetGroundVelocity() + horizontal;
					if (record.pendingJump)
					{
						velocity += JPH::Vec3(0.f, record.jumpSpeed, 0.f);
					}
					else
					{
						velocity += gravity * delta;
					}
				}
				else
				{
					velocity = JPH::Vec3(0.f, character.GetLinearVelocity().GetY(), 0.f)
						+ horizontal
						+ gravity * delta;
				}
			}
			record.pendingJump = false;
			velocity = character.CancelVelocityTowardsSteepSlopes(velocity);
			character.SetLinearVelocity(velocity);
		}

		_impl->physics.Update(delta, 1, &_impl->tempAllocator, &_impl->jobSystem);

		const JPH::ObjectLayer movingLayer = detail::toObjectLayer(Layer::Moving);
		const JPH::CharacterVirtual::ExtendedUpdateSettings updateSettings;
		JPH::ShapeFilter shapeFilter;
		for (CharacterRecord& record : _impl->characters)
		{
			if (!record.occupied || record.character == nullptr)
			{
				continue;
			}

			JPH::CharacterVirtual& character = *record.character;
			const JPH::Vec3 updateGravity = record.climbing ? JPH::Vec3::sZero() : gravity;
			JPH::IgnoreSingleBodyFilter bodyFilter(character.GetInnerBodyID());
			character.ExtendedUpdate(
				delta,
				updateGravity,
				updateSettings,
				_impl->physics.GetDefaultBroadPhaseLayerFilter(movingLayer),
				_impl->physics.GetDefaultLayerFilter(movingLayer),
				bodyFilter,
				shapeFilter,
				_impl->tempAllocator);

			if (dim2)
			{
				JPH::RVec3 position = character.GetPosition();
				position.SetZ(0.f);
				character.SetPosition(position);
				JPH::Vec3 velocity = character.GetLinearVelocity();
				velocity.SetZ(0.f);
				character.SetLinearVelocity(velocity);
			}
		}

		const std::vector<Contact> pending = _impl->contactQueue.take();
		if (_impl->contactCallback)
		{
			for (const Contact& contact : pending)
			{
				_impl->contactCallback(contact);
			}
		}
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
			detail::toObjectLayer(desc.layer));
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

	void World::setContactCallback(std::function<void(const Contact&)> callback)
	{
		_impl->contactCallback = std::move(callback);
	}

	std::optional<RayHit> World::castRay(glm::vec3 origin, glm::vec3 endOffset, float maxFraction) const
	{
		const JPH::RRayCast ray(toRVec3(origin), toVec3(endOffset) * maxFraction);
		JPH::RayCastResult hit;
		if (!_impl->physics.GetNarrowPhaseQuery().CastRay(ray, hit))
		{
			return std::nullopt;
		}

		RayHit result;
		result.body = toBodyId(hit.mBodyID);
		result.fraction = hit.mFraction;
		result.point = toGlm(ray.GetPointOnRay(hit.mFraction));

		JPH::BodyLockRead lock(_impl->physics.GetBodyLockInterface(), hit.mBodyID);
		if (lock.Succeeded())
		{
			result.normal = toGlm(lock.GetBody().GetWorldSpaceSurfaceNormal(
				hit.mSubShapeID2,
				ray.GetPointOnRay(hit.mFraction)));
		}
		return result;
	}

	CharacterId World::createCharacter(const CharacterDesc& desc)
	{
		if (desc.capsuleRadius <= 0.f || desc.capsuleHeight <= 0.f)
		{
			throw std::runtime_error("hl::physics::World::createCharacter: capsuleRadius and capsuleHeight must be positive");
		}

		const float halfHeight = desc.capsuleHeight * 0.5f;
		JPH::RefConst<JPH::Shape> capsule = new JPH::CapsuleShape(halfHeight, desc.capsuleRadius);
		const JPH::Vec3 shapeOffset(0.f, halfHeight + desc.capsuleRadius, 0.f);
		JPH::RefConst<JPH::Shape> shape = new JPH::RotatedTranslatedShape(
			shapeOffset,
			JPH::Quat::sIdentity(),
			capsule);

		JPH::CharacterVirtualSettings settings;
		settings.mShape = shape;
		settings.mInnerBodyShape = shape;
		settings.mInnerBodyLayer = detail::toObjectLayer(Layer::Moving);
		settings.mMaxSlopeAngle = JPH::DegreesToRadians(desc.maxSlopeAngleDeg);
		settings.mSupportingVolume = JPH::Plane(JPH::Vec3::sAxisY(), -desc.capsuleRadius);
		settings.mEnhancedInternalEdgeRemoval = true;

		glm::vec3 position = desc.pose.position;
		if (_impl->dim == Dim::D2)
		{
			position.z = 0.f;
		}

		JPH::Ref<JPH::CharacterVirtual> character = new JPH::CharacterVirtual(
			&settings,
			toRVec3(position),
			toQuat(desc.pose.rotation),
			&_impl->physics);

		std::uint32_t index = static_cast<std::uint32_t>(_impl->characters.size());
		for (std::uint32_t i = 0; i < _impl->characters.size(); ++i)
		{
			if (!_impl->characters[i].occupied)
			{
				index = i;
				break;
			}
		}
		if (index == _impl->characters.size())
		{
			_impl->characters.emplace_back();
		}

		CharacterRecord& record = _impl->characters[index];
		record.generation += 1;
		if (record.generation == 0)
		{
			record.generation = 1;
		}
		record.occupied = true;
		record.character = character;
		record.wish = glm::vec3{0.f};
		record.climbing = false;
		record.pendingJump = false;
		record.jumpSpeed = desc.jumpSpeed;
		return makeCharacterId(index, record.generation);
	}

	void World::destroyCharacter(CharacterId id)
	{
		CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr)
		{
			return;
		}
		record->character = nullptr;
		record->occupied = false;
		record->wish = glm::vec3{0.f};
		record->climbing = false;
		record->pendingJump = false;
	}

	void World::setMove(CharacterId id, glm::vec3 wish)
	{
		CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr)
		{
			return;
		}
		if (_impl->dim == Dim::D2)
		{
			wish.z = 0.f;
		}
		record->wish = wish;
	}

	void World::jump(CharacterId id)
	{
		CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr || record->climbing)
		{
			return;
		}
		if (record->character->GetGroundState() != JPH::CharacterBase::EGroundState::OnGround)
		{
			return;
		}
		record->pendingJump = true;
	}

	void World::setClimbing(CharacterId id, bool climbing)
	{
		CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr)
		{
			return;
		}
		record->climbing = climbing;
		if (climbing)
		{
			record->pendingJump = false;
		}
	}

	Pose World::getPose(CharacterId id) const
	{
		const CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr)
		{
			return Pose{};
		}
		return Pose{
			.position = toGlm(record->character->GetPosition()),
			.rotation = toGlm(record->character->GetRotation())
		};
	}

	bool World::isGrounded(CharacterId id) const
	{
		const CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr)
		{
			return false;
		}
		return record->character->GetGroundState() == JPH::CharacterBase::EGroundState::OnGround;
	}

	bool World::isClimbing(CharacterId id) const
	{
		const CharacterRecord* record = _impl->characterAt(id);
		if (record == nullptr)
		{
			return false;
		}
		return record->climbing;
	}
}
