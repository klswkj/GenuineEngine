#pragma once
#include "ComponentFactory.h"
#include <cereal/types/complex.hpp>
#include <cereal/types/common.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/json.hpp>
#include <cereal/types/polymorphic.hpp>


template<typename T>
class Serializable
{
	static void RegisterType(const char* name) 
	{
		ComponentFactory::RegisterComponentContainer(
			StringID(name), 
			[&]()->IComponent* 
			{
				return (IComponent*)new Component<T>();
			}
		);

		ComponentFactory::RegisterComponentTypeID(StringID(name), typeid(T).hash_code());
	};

public:
	Serializable(const char* name) :
		Name(name)
	{
		RegisterType(name);
	}

	const char* Name;
};

//! Lets the compiler know that this struct is a in-game component
#define GameComponent(name) \
struct name : public IComponentData { \
static Serializable<name> Reflectable_ ## name; \
static const char* GetName() \
{ \
	return #name ; \
} \
virtual IComponentData* Clone() override \
{ \
	name* val = new name(); \
	*val = *this; \
	return val; \
}

#define EndComponent(name) \
}; \


#define RegisterComponent(name)\
Serializable<name> name ## ::Reflectable_ ## name = Serializable<name>(#name); 


#define Construct(type) \
static const char* GetName() \
{ \
	return Reflectable_ ## type.Name; \
}
