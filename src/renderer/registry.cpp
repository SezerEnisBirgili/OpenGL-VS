
#include "registry.h"

int Registry::loadModel(std::string path)
{

}

int Registry::add(const std::string& key, const Mesh& mesh)
{
	if (auto it = meshes.find(key); it != meshes.end())
		return meshed[key];

	meshes[key] = mesh;

	return id;
}

bool Registry::contains(const std::string& key, const Mesh& mesh) {
	return meshes.contains(key);
}