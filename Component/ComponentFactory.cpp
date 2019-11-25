#include "stdafx.h"
#include "ComponentFactory.h"
#include "AnimationComponent.h"


ComponentFactory cf;
std::unordered_map<HashID, FactoryFunction> ComponentFactory::factoryMap;
std::unordered_map<HashID, TypeID> ComponentFactory::typeMap;

void ComponentFactory::RegisterComponentContainer(HashID componentId, FactoryFunction function)
{
	if (factoryMap.find(componentId) == factoryMap.end())
		factoryMap.insert(std::pair<HashID, FactoryFunction>(componentId, function));
}

void ComponentFactory::RegisterComponentTypeID(HashID componentId, TypeID typeId)
{
	if (typeMap.find(componentId) == typeMap.end())
	{
		typeMap.insert(std::pair<HashID, TypeID>(componentId, typeId));
	}
}

IComponent * ComponentFactory::Create(HashID componentId)
{
	return factoryMap[componentId]();
}

TypeID ComponentFactory::GetTypeID(HashID componentId)
{
	return typeMap[componentId];
}

#include "../Engine.Components/Components.inl"
RegisterComponent(AnimationComponent)
RegisterComponent(AnimationBufferComponent)