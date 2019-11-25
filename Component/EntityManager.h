#pragma once
#include "stdafx.h"
#include <unordered_map>
#include "../Engine.Serialization/StringHash.h"
#include "Scene.h"
#include "SceneCommon.h"
#include "ComponentFactory.h"
#include "Component.h"
#include <cereal/types/polymorphic.hpp>
#include "SystemContext.h"
#include <parallel_hashmap/phmap.h>

class ResourceManager;


class EntityManager;

struct Vector3
{
	XMFLOAT3 Value;

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(
			cereal::make_nvp("X", Value.x),
			cereal::make_nvp("Y", Value.y),
			cereal::make_nvp("Z", Value.z)
		);
	}
};

struct Entity
{
	const EntityID	EntityID;
	const NodeID	Node;
	HashID			Mesh;
	HashID			Material;
	XMFLOAT4X4		WorldTransform;
};

class EntityManager
{
	static EntityManager* Instance;

	Scene* scene;
	phmap::flat_hash_map<NodeID, EntityID> nodeMap;
	std::unordered_map<std::string, EntityID> entityNameIndexMap;
	std::unordered_map<TypeID, IComponent*> components;

	std::vector<NodeID>		entities; // Entity List. Index of this vector will act as EntityID
	std::vector<HashID>		meshes;
	std::vector<HashID>		materials;
	std::vector<EntityID>	parents;
	std::vector<byte>		active;
	std::vector<EntityID>	freeEntityIds;

public:
	EntityManager(Scene* scene);
	static EntityManager*	GetInstance() { return Instance; }
	EntityID				CreateEntity(std::string name, const Transform& transform = DefaultTransform);
	EntityID				CreateEntity(std::string name, HashID mesh = 0u, HashID material = 0u, const Transform& transform = DefaultTransform);
	EntityID				CreateEntity(EntityID parentId, std::string name, HashID mesh = 0u, HashID material = 0u, const Transform& transform = DefaultTransform);
	void					Remove(EntityID entity);
	//void			ExecutePurge(); //Perform all remove operations at once.

	template<typename T>
	void			RegisterComponent();

	void			RegisterComponent(const char* componentName);

	template<typename T>
	void			RegisterEntity(EntityID entity, const T& componentData = T());

	template<typename T>
	void			AddComponent(EntityID entity, const T& componentData = T());

	void			AddComponent(EntityID entity, const char* componentName, IComponentData* data = nullptr);

	template<typename T>
	void			GetComponentEntities(std::vector<EntityID> &outEntities);
	void			GetComponentEntities(TypeID componentId, std::vector<EntityID> &outEntities);

	template<typename T, typename FuncType>
	void			GetComponentEntitiesWithCB(FuncType callback);

	template<typename... Args>
	void			GetMultiComponentEntities(std::vector<EntityID> &outEntities, Args*& ...args);

	template<typename T>
	T&				GetComponent(EntityID entity);

	IComponentData* GetComponent(const char* componentName, EntityID entity);

	IComponent*		GetComponentContainer(const char* componentName);

	template<typename T>
	T*				GetComponents(size_t& outCount);

	//Setters
	void			SetMesh(EntityID entity, HashID mesh);
	void			SetMaterial(EntityID entity, HashID material);
	void			SetPosition(EntityID entity, const XMFLOAT3& position);
	void			SetRotation(EntityID entity, const XMFLOAT3& rotation);
	void			SetScale(EntityID entity, const XMFLOAT3& scale);
	void			SetTransform(EntityID entity, const Transform& transform);
	void			SetActive(EntityID entity, bool enable);
	void			SaveToFile(const char* filename, ResourceManager* rm);
	void			Load(const char* filename, SystemContext context);

	//Getters
	const XMFLOAT3&		GetPosition(EntityID entity);
	const XMFLOAT3&		GetRotation(EntityID entity);
	XMFLOAT3			GetRotationInDegrees(EntityID entity);
	const XMFLOAT3&		GetScale(EntityID entity);
	const XMFLOAT4X4&	GetTransformMatrix(EntityID entity);
	EntityID			GetEntityID(std::string entityName);
	const bool			IsActive(EntityID entity);

	Entity				GetEntity(EntityID entity);
	void				GetEntities(std::vector<Entity>& outEntityList);
	void				UpdateEntity(const Entity& entity);

	inline size_t		Count() const { return entities.size(); };
	~EntityManager();
};

template<typename T>
inline void EntityManager::RegisterComponent()
{
	auto typeHash = typeid(T).hash_code();
	if (components.find(typeHash) == components.end())
	{
		auto component = new Component<T>();
		components.insert(std::pair<TypeID, IComponent*>(typeHash, (IComponent*)component));
	}
}

template<typename T>
inline void EntityManager::RegisterEntity(EntityID entity, const T & componentData)
{
	auto typeHash = typeid(T).hash_code();
	Component<T>* component = (Component<T>*)components[typeHash];
	component->AddEntity(entity, componentData);
}

template<typename T>
inline void EntityManager::AddComponent(EntityID entity, const T & componentData)
{
	std::shared_ptr<T> ptr = std::shared_ptr<T>(&componentData);
	auto typeHash = typeid(T).hash_code();
	if (components.find(typeHash) == components.end())
	{
		RegisterComponent<T>();
	}
	RegisterEntity(entity, componentData);
}

template<typename T>
inline void EntityManager::GetComponentEntities(std::vector<EntityID>& outEntities)
{
	auto typeHash = typeid(T).hash_code();
	Component<T>* component = (Component<T>*)components[typeHash];
	for (auto e : component->Entities)
	{
		if (active[e])
			outEntities.push_back(e);
	}
}

template<typename T, typename FuncType>
inline void EntityManager::GetComponentEntitiesWithCB(FuncType callback)
{
	auto typeHash = typeid(T).hash_code();
	Component<T>* component = (Component<T>*)components[typeHash];
	callback(component->Entities);
}

template<typename ...Args>
inline void EntityManager::GetMultiComponentEntities(std::vector<EntityID>& outEntities, Args*& ...args)
{
	std::vector<size_t> compSizes;
	std::vector< std::vector<EntityID> > entityList;
	auto cb = [&](std::vector<EntityID> inEntities) {
		std::sort(inEntities.begin(), inEntities.end());
		if (outEntities.size() == 0)
			outEntities = inEntities;
		else
			std::set_intersection(outEntities.begin(), outEntities.end(), inEntities.begin(), inEntities.end(), outEntities.begin());
	};

	auto list = { (components.find(typeid(Args).hash_code()))... };
	auto c = { 0, (GetComponentEntitiesWithCB<Args>(cb), 0) ... };
	(void)c;
}

template<typename T>
inline T & EntityManager::GetComponent(EntityID entity)
{
	auto typeHash = typeid(T).hash_code();
	Component<T>* component = (Component<T>*)components[typeHash];
	return component->GetData(entity);
}

template<typename T>
inline T * EntityManager::GetComponents(size_t & outCount)
{
	auto typeHash = typeid(T).hash_code();
	Component<T>* component = (Component<T>*)components[typeHash];
	return component->Components.data();
}


