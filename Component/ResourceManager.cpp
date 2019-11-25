#include "stdafx.h"
#include "ResourceManager.h"
#include "ModelLoader.h"

ResourceManager* ResourceManager::Instance = nullptr;

ResourceManager::ResourceManager(ID3D12Device* device)
{
	this->device = device;
	Instance = this;
}

ResourceManager * ResourceManager::CreateInstance(ID3D12Device * device)
{
	ResourceManager* rm = nullptr;
	if (Instance == nullptr)
	{
		rm = new ResourceManager(device);
	}

	return rm;
}

ResourceManager * ResourceManager::GetInstance()
{
	return Instance;
}

void ResourceManager::Initialize(AnimationManager* animationManager, EntityManager* entityMgr)
{
	animManager = animationManager;
	entityManager = entityMgr;
}

void ResourceManager::LoadTexture(
	ID3D12CommandQueue * commandQueue,
	DeferredRenderer * renderer,
	HashID textureID,
	std::wstring filepath,
	TextureFileType texFileType,
	bool isCubeMap,
	TextureViewType viewType
)
{
	auto texture = new Texture(renderer, device);
	texture->CreateTexture(filepath, texFileType, commandQueue, isCubeMap);
	textures.insert(std::pair<HashID, Texture*>(textureID, texture));
}

void ResourceManager::LoadTextures(ID3D12CommandQueue* commandQueue, DeferredRenderer* renderer, std::vector<TextureLoadData> textureLoadData)
{
	ResourceUploadBatch uploadBatch(device);
	uploadBatch.Begin();
	for (auto& t : textureLoadData)
	{
		auto texture = new Texture(renderer, device);
		texture->CreateTexture(t.FilePath, t.FileType, commandQueue, uploadBatch, t.IsCubeMap);
		textures.insert(std::pair<HashID, Texture*>(t.TextureID, texture));
	}
	auto uploadOperation = uploadBatch.End(commandQueue);
	uploadOperation.wait();
}

void ResourceManager::LoadMaterial(ID3D12CommandQueue* commandQueue, DeferredRenderer* renderer, MaterialLoadData loadData)
{
	auto hash = StringID(loadData.MaterialID.c_str());
	strings.insert(std::pair<HashID, std::string>(hash, loadData.MaterialID));
	auto material = new Material(
		renderer,
		{
			loadData.AlbedoFile,
			loadData.NormalFile,
			loadData.RoughnessFile,
			loadData.MetalnessFile
		},
		device,
		commandQueue
	);

	materials.insert(std::pair<HashID, Material*>(hash, material));
}

void ResourceManager::LoadMaterials(ID3D12CommandQueue * commandQueue, DeferredRenderer * renderer, std::vector<MaterialLoadData> materials)
{
	for (auto &loadData : materials)
	{
		LoadMaterial(commandQueue, renderer, loadData);
	}
}

void ResourceManager::LoadMeshes(ID3D12GraphicsCommandList* commandList, std::vector<std::string> meshList)
{
	for (auto& m : meshList)
	{
		LoadMesh(commandList, m);
	}
}

void ResourceManager::LoadMesh(ID3D12GraphicsCommandList * commandList, std::string filePath)
{
	auto filename = GetFileNameWithoutExtension(filePath);
	LoadMesh(commandList, filename.c_str(), filePath);
}

void ResourceManager::LoadMesh(ID3D12GraphicsCommandList * commandList, const char* hashId, std::string filePath)
{
	auto hash = StringID(hashId);
	strings.insert(std::pair<HashID, std::string>(hash, hashId));
	auto mesh = ModelLoader::LoadFile(filePath, commandList);
	meshes.insert(std::pair<HashID, Mesh*>(hash, mesh));
	if (mesh->IsAnimated())
	{
		animManager->RegisterMeshAnimations(hash, &mesh->Animations);
	}
}

Mesh * ResourceManager::GetMesh(HashID hashId)
{
	return meshes[hashId];
}

Material * ResourceManager::GetMaterial(HashID materialID)
{
	return materials[materialID];
}

Texture * ResourceManager::GetTexture(HashID textureID)
{
	return textures[textureID];
}

const char * ResourceManager::GetString(HashID hashId)
{
	return strings[hashId].c_str();
}

Scene ResourceManager::LoadScene(std::string filename, std::vector<Entity*> &outEntities)
{
	auto scene = SceneSerDe::LoadScene(filename);
	int idx = 0;
	for (auto e : scene.Entities)
	{
		auto meshID = StringID(e.MeshID);
		XMFLOAT3 rotation(e.Rotation.Vector.x * XM_PIDIV2, e.Rotation.Vector.y * XM_PIDIV2, e.Rotation.Vector.z * XM_PIDIV2); //Converts degrees to radians
		auto transform = Transform::Create(e.Position, rotation, e.Scale);
		auto entityID = entityManager->CreateEntity(std::to_string(idx), StringID(e.MeshID), StringID(e.MaterialID), transform);
		if (GetMesh(meshID)->IsAnimated())
		{
			animManager->RegisterEntity(entityID, meshID);
		}
		idx++;
	}

	return scene;
}

void ResourceManager::LoadResources(std::string filename, ID3D12CommandQueue* cqueue, ID3D12GraphicsCommandList* clist, DeferredRenderer* renderer)
{
	auto rc = ResourcePackSerDe::LoadResources(filename);
	for (auto m : rc.Materials)
	{
		LoadMaterial(
			cqueue,
			renderer,
			{
				m.MaterialID,
				ToWideString(m.AlbedoPath),
				ToWideString(m.NormalPath),
				ToWideString(m.RoughnessPath),
				ToWideString(m.MetalnessPath)
			}
		);
	}

	for (auto m : rc.Meshes)
	{
		LoadMesh(clist, m.MeshID.c_str(), m.MeshPath);
	}

	for (auto t : rc.Textures)
	{
		TextureFileType type;
		if (t.TexType == WIC) type = TexFileTypeWIC;
		else type = TexFileTypeDDS;
		LoadTexture(cqueue, renderer, StringID(t.TextureID), ToWideString(t.TexturePath), type, t.IsCubeMap);
	}
}


ResourceManager::~ResourceManager()
{
	for (auto& m : meshes)
	{
		delete m.second;
	}

	for (auto& m : materials)
	{
		delete m.second;
	}

	for (auto& m : textures)
	{
		delete m.second;
	}
}
