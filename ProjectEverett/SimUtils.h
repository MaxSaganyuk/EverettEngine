#pragma once

#include "CameraSim.h"
#include "SolidSim.h"
#include "LightSim.h"
#include "SoundSim.h"
#include "ColliderSim.h"

#include "external/IEverettEngine.h"

template<typename Type>
constexpr static std::optional<IEverettEngine::ObjectTypes> GetTypeEnumByType()
{
	return {};
}

#define DefineTypeEnumByType(Type)                                                         \
template<>                                                                                 \
constexpr static std::optional<IEverettEngine::ObjectTypes> GetTypeEnumByType<Type##Sim>() \
{                                                                                          \
	return IEverettEngine::ObjectTypes::Type;                                              \
}

DefineTypeEnumByType(Camera)
DefineTypeEnumByType(Solid)
DefineTypeEnumByType(Light)
DefineTypeEnumByType(Sound)
DefineTypeEnumByType(Collider)

class SimIndexer
{
	std::array<AutoReindexer, std::to_underlying(IEverettEngine::ObjectTypes::_SIZE)> indexerCollection;
public:
	template<SimObj Type>
	void Add(Type& sim)
	{
		constexpr static auto typeEnum = GetTypeEnumByType<Type>();

		if (typeEnum)
		{
			auto typeIndex = std::to_underlying(*typeEnum);

			indexerCollection[typeIndex].PushIncrementBy(1);
			sim.SetIndex(indexerCollection[typeIndex].GetBackIndex());
		}
	}

	template<SimObj Type>
	void Remove(Type& sim)
	{
		constexpr static auto typeEnum = GetTypeEnumByType<Type>();

		if (typeEnum)
		{
			auto typeIndex = std::to_underlying(*typeEnum);

			if (!indexerCollection[typeIndex].IsEmpty())
			{
				indexerCollection[typeIndex].RemoveIndex(sim.GetIndex(), 1);
			}
		}
	}

	void CleanAll() noexcept
	{
		constexpr static auto cameraTypeIndex = std::to_underlying(*GetTypeEnumByType<CameraSim>());

		for (size_t i = 0; i < indexerCollection.size(); ++i)
		{
			if (i != cameraTypeIndex)
			{
				indexerCollection[i].Clean();
			}
		}
	}
};