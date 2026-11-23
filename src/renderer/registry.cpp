#include "registry.h"
#include <algorithm>


int Registry::loadModel(std::string path)
{
	// TODO: run ModelLoader here
	return -1;
}

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

	int id = (int)meshes.size();
	paths.emplace(key, id);
	return id;
}

Mesh* Registry::get(const std::string& key)
{
	auto it = paths.find(key);
	if (it == paths.end())
		return nullptr;

	return &meshes.at(it->second);
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

Mesh& Registry::mesh(int id)
{
	return meshes.at(id);
}