#include "stdafx.h"
#include "SystemManager.h"


SystemManager::SystemManager(EntityManager * entityMgr, SystemContext* context) :
	entityManager(entityMgr),
	context(context)
{
}

SystemManager::~SystemManager()
{
	for (auto system : internalSystems)
	{
		delete system;
	}
}

void SystemManager::RegisterSystems(std::vector<ISystem*>&& systems)
{
	this->systems = std::move(systems);
	for (auto system : this->systems)
	{
		system->SetEntityManager(entityManager);
		system->SetContext(*context);
	}
}

void SystemManager::RegisterSystems()
{
	for (auto system : systems)
	{
		system->SetEntityManager(entityManager);
		system->SetContext(*context);
		system->Init();
	}
}

std::vector<ISystem*>& SystemManager::GetSystems()
{
	return systems;
}

void SystemManager::Init()
{
	for (auto system : systems)
	{
		system->Init();
	}

	for (auto system : internalSystems)
	{
		system->Init();
	}
}

void SystemManager::Update(float deltaTime)
{
	for (auto system : systems)
	{
		system->PreUpdate();
		system->Update(deltaTime);
		system->PostUpdate();
	}

	for (auto system : internalSystems)
	{
		system->PreUpdate();
		system->Update(deltaTime);
		system->PostUpdate();
	}
}

void SystemManager::Shutdown()
{
	for (auto system : systems)
	{
		system->Shutdown();
	}

	for (auto system : internalSystems)
	{
		system->Shutdown();
	}
}
