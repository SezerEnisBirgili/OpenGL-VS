class Registry {
public:
    int loadModel(std::string path);
    Mesh& mesh(int id);
    Texture& texture(int id);

    int add(const std::string& key, const Mesh& mesh);
    bool contains(const std::string& key, const Mesh& mesh);
private:
    std::vector<Model>   models;
    std::vector<Mesh>    meshes;
    std::vector<Texture> textures;

    std::unordered_map<std::string, Model> models;
    std::unordered_map<std::string, Mesh> meshes;
    std::unordered_map<std::string, int> textureByPath;
};