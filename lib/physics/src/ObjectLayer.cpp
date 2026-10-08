#include "ObjectLayer.hpp"

namespace hl::physics::detail
{
	namespace
	{
		constexpr JPH::ObjectLayer cObjectNonMoving = 0;
		constexpr JPH::ObjectLayer cObjectMoving = 1;
		constexpr JPH::ObjectLayer cObjectSensor = 2;
		constexpr JPH::BroadPhaseLayer cBroadphaseNonMoving(0);
		constexpr JPH::BroadPhaseLayer cBroadphaseMoving(1);
		constexpr unsigned cBroadphaseCount = 2;
	}

	JPH::ObjectLayer toObjectLayer(Layer layer)
	{
		switch (layer)
		{
		case Layer::NonMoving:
			return cObjectNonMoving;
		case Layer::Moving:
			return cObjectMoving;
		case Layer::Sensor:
			return cObjectSensor;
		}
		return cObjectMoving;
	}

	JPH::uint BroadPhaseLayerInterfaceImpl::GetNumBroadPhaseLayers() const
	{
		return cBroadphaseCount;
	}

	JPH::BroadPhaseLayer BroadPhaseLayerInterfaceImpl::GetBroadPhaseLayer(JPH::ObjectLayer layer) const
	{
		return layer == cObjectNonMoving ? cBroadphaseNonMoving : cBroadphaseMoving;
	}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
	const char* BroadPhaseLayerInterfaceImpl::GetBroadPhaseLayerName(JPH::BroadPhaseLayer layer) const
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

	bool ObjectVsBroadPhaseLayerFilterImpl::ShouldCollide(JPH::ObjectLayer layer, JPH::BroadPhaseLayer broadphase) const
	{
		if (layer == cObjectNonMoving)
		{
			return broadphase == cBroadphaseMoving;
		}
		if (layer == cObjectSensor)
		{
			return broadphase == cBroadphaseMoving;
		}
		return true;
	}

	bool ObjectLayerPairFilterImpl::ShouldCollide(JPH::ObjectLayer a, JPH::ObjectLayer b) const
	{
		if (a == cObjectNonMoving)
		{
			return b == cObjectMoving;
		}
		if (b == cObjectNonMoving)
		{
			return a == cObjectMoving;
		}
		if (a == cObjectSensor)
		{
			return b == cObjectMoving;
		}
		if (b == cObjectSensor)
		{
			return a == cObjectMoving;
		}
		return true;
	}
}
