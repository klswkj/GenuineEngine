#pragma once
#include "../Engine.Serialization/StringHash.h"
#include "Component.h"


typedef std::function<IComponent*()> FactoryFunction;
class ComponentFactory
{
	static std::unordered_map<HashID, FactoryFunction> factoryMap;
	static std::unordered_map<HashID, TypeID> typeMap;
public:
	static void RegisterComponentContainer(HashID componentId, FactoryFunction function);
	static void RegisterComponentTypeID(HashID componentId, TypeID typeId);
	static std::unordered_map<HashID, TypeID>& GetTypeMap() { return typeMap; }
	static IComponent* Create(HashID componentId);
	static TypeID GetTypeID(HashID componentId);
};

