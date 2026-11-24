#include "registry.h"
#include "renderer/modelLoader.h"
#include <algorithm>


int Registry::loadModel(std::string path)
{
	std::string key = makeKey(ResourceType::Model, path);

	if (auto it = paths.find(key); it != paths.end())
		return it->second;

	ModelLoader loader(*this, path);
	Model* model_ptr = loader.loadModel();

	return paths.at(key);
}

Model* Registry::add(const std::string& key, const Model& model)
{
	if (auto it = paths.find(key); it != paths.end())
		return &models.at(it->second);

	int id = pathToId(key);

	return &models.try_emplace(id, model).first->second;
}

Model* Registry::model(int id) { return &models.at(id); }

Mesh* Registry::add(const std::string& key, const Mesh& mesh)
{
	if (auto it = paths.find(key); it != paths.end())
		return &meshes.at(it->second);

	int id = pathToId(key);

	return &meshes.try_emplace(id, mesh).first->second;
}

int Registry::pathToId(const std::string& key)
{
	if (auto it = paths.find(key); it != paths.end())
		return it->second;

	int id = nextId++;
	paths.emplace(key, id);
	return id;
}

Mesh* Registry::getMesh(const std::string& key)
{
	auto it = paths.find(key);
	if (it == paths.end())
		return nullptr;

	return &meshes.at(it->second);
}

Model* Registry::getModel(const std::string& key)
{
	auto it = paths.find(key);
	if (it == paths.end())
		return nullptr;

	return &models.at(it->second);
}

std::string Registry::makeKey(ResourceType type, std::string path, unsigned index)
{
	std::replace(path.begin(), path.end(), '\\', '/');

	switch (type)
	{
	case ResourceType::Model:    return path;
	case ResourceType::Mesh:     return path + "#" + std::to_string(index);
	case ResourceType::Material: return path + "#mat" + std::to_string(index);
	}

	return path;
}

bool Registry::contains(const std::string& key, const Mesh&)
{
	return paths.contains(key);
}