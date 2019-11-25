#pragma once

#include <fstream>
#include <cereal/archives/json.hpp>
#include "Component.h"


class ComponentSerDe
{
public:
	static void Save(std::unordered_map<TypeID, IComponent*>& components, std::string filename);
	static void Load(std::unordered_map<TypeID, IComponent*>& components, std::string filename);
	ComponentSerDe();
	~ComponentSerDe();
};



