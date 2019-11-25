#include "stdafx.h"
#include "System.h"

ISystem::ISystem(EntityManager * entityManager):
	entity(entityManager)
{
}

void ISystem::GetEntities(std::vector<EntityID>& outEntities)
{
	std::vector<EntityID> entities;
	for (auto component : components)
	{
		entity->GetComponentEntities(component, entities);
		std::sort(entities.begin(), entities.end());
		if (outEntities.size() == 0)
			outEntities = entities;
		else //Get only intersecting entity list
			std::set_intersection(outEntities.begin(), outEntities.end(), entities.begin(), entities.end(), outEntities.begin()); 
	}
}

//void SampleSystem::PreUpdate()
//{
//	GetEntities(entities);
//	GetComponents(aData, entities);
//	GetComponents(bData, entities);
//}
//
//void SampleSystem::Update(float deltaTime)
//{
//	totalTime += deltaTime;
//	auto idx = 0;
//	for (auto e : entities)
//	{
//		auto position = entity->GetPosition(e);
//		//aData[idx]->speed = 10.f;
//		position.y = aData[idx]->speed * sin(totalTime);
//		entity->SetPosition(e, position);
//		idx++;
//	}
//}
//
//void SampleSystem::PostUpdate()
//{
//	entities.clear();
//	aData.clear();
//	bData.clear();
//}
