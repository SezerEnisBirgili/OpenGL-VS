#include "world.h"
#include "entities/GameObject.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <set>
#include <glm/gtc/matrix_transform.hpp>

World::World(Registry* registry, int x, int y, int z) : reg(registry), boundx(x), boundy(y), boundz(z) {
    blocks.resize(x * y * z, 0);
}

int World::getIndex(const glm::vec3& v) const {
    return (int)v.x * boundy * boundz + (int)v.y * boundz + (int)v.z;
}

bool World::isBlockSolid(const glm::vec3& v) const {
    if (!isWithinBounds(v)) return false;

    int e = blocks[getIndex(v)];
    if (e == NULL_ENTITY) return false;

    auto it = reg->materials.find(e);
    if (it == reg->materials.end()) {
        std::cerr << "[World::isBlockSolid] Error: Entity " << e << " at block position (" << v.x << ", " << v.y << ", " << v.z << ") has no MaterialComponent!\n";
        return false;
    }

    return !it->second.isAir;
}

int World::getBlock(const glm::vec3& v) const {
    if (!isWithinBounds(v)) return NULL_ENTITY;
    return blocks[getIndex(v)];
}

void World::setBlock(int e, const glm::vec3& v) {
    if (isWithinBounds(v))  {
        blocks[getIndex(v)] = e;
    }
}

void World::removeBlock(const glm::vec3& v) {
    if (isWithinBounds(v)) {
        setBlock(NULL_ENTITY, v);
    }
}

void World::createPlatform(int e, int sizeX, int sizeZ) {
    glm::vec3 v(0.0f);
    for (int x = 0; x < sizeX; x++) {
        for (int z = 0; z < sizeZ; z++) {
            v.x = (float)x;
            v.z = (float)z;
            setBlock(e, v);
        }
    }
}

bool World::exportWorldToPath(Registry& reg, const std::string& destinationPath) const {

    std::ofstream f(destinationPath);
    if (!f) {
        std::cerr << "[world] cannot open " << destinationPath << " for writing\n";
        return false;
    }

    std::unordered_map<int, int> idToIndex;
    std::set<int> paletteIds; 
    std::vector<std::pair<int, int>> runs; 

    for (int x = 0; x < boundx; x++)
        for (int y = 0; y < boundy; y++)
            for (int z = 0; z < boundz; z++) 
            {
                int idx = getBlock(glm::vec3(x, y, z));
                if (!runs.empty() && runs.back().first == idx) 
                {
                    runs.back().second++;
                }
                else 
                {
                    if (!paletteIds.count(idx)) { paletteIds.insert(idx); }
                    runs.push_back({ idx, 1 });
                }
            }

    f << "size " << boundx << " " << boundy << " " << boundz << "\n";
    f << "palette " << paletteIds.size() << "\n";

    int i = 0;
    for (int id : paletteIds)
    {
        idToIndex[id] = i;
        std::string name = (id == NULL_ENTITY) ? "air" : reg.names.at(id);
        f << i << " " << name << "\n";
        i++;
    }

    f << "data\n";
    for (auto& [id, count] : runs)
        f << idToIndex.at(id) << " x " << count << "\n";
    
    f.close();
    return !f.fail();
}

bool World::importWorldFromPath(Registry& reg, const std::string& sourcePath) {

    std::ifstream f(sourcePath);
    if (!f) 
    {
        std::cerr << "[world] cannot open " << sourcePath << " for reading\n";
        return false;
    }

    // --- size ---
    std::string word;
    int sx = 0, sy = 0, sz = 0;
    if (!(f >> word >> sx >> sy >> sz) || word != "size") 
    {
        std::cerr << "[world] bad or missing size line\n";
        return false;
    }
    if (sx != boundx || sy != boundy || sz != boundz) 
    {
        std::cerr << "[world] size mismatch: file " << sx << "x" << sy << "x" << sz << ", world " << boundx << "x" << boundy << "x" << boundz << "\n";
        return false;
    }

    // --- name -> block entity, from the live registry ---
    std::unordered_map<std::string, int> nameToId;
    for (int e : reg.blockPalette) nameToId[reg.names.at(e)] = e;

    // --- palette ---
    int paletteCount = 0;
    if (!(f >> word >> paletteCount) || word != "palette" || paletteCount <= 0) 
    {
        std::cerr << "[world] bad or missing palette line\n";
        return false;
    }

    std::vector<int> palette(paletteCount, NULL_ENTITY);
    for (int n = 0; n < paletteCount; n++) {
        int idx = -1;
        if (!(f >> idx) || idx < 0 || idx >= paletteCount) {
            std::cerr << "[world] bad palette index\n";
            return false;
        }

        std::string name;
        std::getline(f, name);                               // rest of line, may contain spaces
        name.erase(0, name.find_first_not_of(' '));          // trim leading space
        while (!name.empty() && (name.back() == '\r' || name.back() == ' '))
            name.pop_back();                                 // trim trailing \r / spaces

        if (name == "air") 
        {
            palette[idx] = NULL_ENTITY;
        } 
        else if (auto it = nameToId.find(name); it != nameToId.end()) 
        {
            palette[idx] = it->second;
        } 
        else 
        {
            std::cerr << "[world] unknown block '" << name << "', using air\n";
            palette[idx] = NULL_ENTITY;
        }
    }

    // --- data: parse into a temporary buffer first ---
    if (!(f >> word) || word != "data") 
    {
        std::cerr << "[world] missing data section\n";
        return false;
    }

    const int total = boundx * boundy * boundz;
    std::vector<int> cells;
    cells.reserve(total);

    int idx = 0, count = 0;
    char sep = 0;

    while (f >> idx >> sep >> count) 
    {
        if (sep != 'x' || idx < 0 || idx >= paletteCount || count < 0 || cells.size() + count > total) {
            std::cerr << "[world] corrupt data run\n";
            return false;
        }
        cells.insert(cells.end(), count, palette[idx]);
    }

    if (cells.size() != total) 
    {
        std::cerr << "[world] file truncated: got " << cells.size() << " blocks, expected " << total << "\n";
        return false;
    }

    // --- apply: same x, y, z order as the export ---
    int pos = 0;
    for (int x = 0; x < boundx; x++)
        for (int y = 0; y < boundy; y++)
            for (int z = 0; z < boundz; z++)
                setBlock(cells[pos++], glm::vec3(x, y, z));

    return true;
}


// Opaque only
void World::collectInstancedRenderItems(std::vector<InstancedRenderItem>& out, const glm::mat4& parentTransform) const {
    std::unordered_map<int, std::vector<glm::vec3>> opaqueGrouped;

    for (int x = 0; x < boundx; ++x) {
        for (int y = 0; y < boundy; ++y) {
            for (int z = 0; z < boundz; ++z) {
                glm::vec3 pos(x, y, z);
                int e = blocks[getIndex(pos)];

                if (e == NULL_ENTITY) continue;

                if (reg && reg->materials.count(e)) {
                    if (!reg->getMaterial(e).isTransparent) {
                        opaqueGrouped[e].push_back(pos);
                    }
                }
            }
        }
    }

    for (auto& [e, positions] : opaqueGrouped) {
        std::vector<glm::vec3> transformedPositions;
        transformedPositions.reserve(positions.size());

        for (const auto& pos : positions) {
            glm::vec3 worldPos = glm::vec3(parentTransform * glm::vec4(blockCenter(pos), 1.0f));
            transformedPositions.push_back(worldPos);
        }

        out.push_back({ e, std::move(transformedPositions) });
    }
}

// Transparent only
void World::collectRenderItems(std::vector<RenderItem>& out, const glm::mat4& parentTransform) const {
    for (int x = 0; x < boundx; ++x) {
        for (int y = 0; y < boundy; ++y) {
            for (int z = 0; z < boundz; ++z) {
                glm::vec3 pos(x, y, z);
                int e = blocks[getIndex(pos)];

                if (e == NULL_ENTITY) continue;

                if (reg && reg->materials.count(e)) {
                    if (reg->getMaterial(e).isTransparent) {
                        glm::vec3 worldPos = glm::vec3(parentTransform * glm::vec4(blockCenter(pos), 1.0f));
                        glm::mat4 model(parentTransform);
                        model[3] = glm::vec4(worldPos, 1.0f);
                        out.push_back({ e, model });
                    }
                }
            }
        }
    }
}