#ifndef RESOURCEMANAGER_HPP
#define RESOURCEMANAGER_HPP

#include <unordered_map>


#include "../headers/Image.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Material.hpp"

namespace Resources {

struct ImageDesc {
	std::string name;

	int width;
	int height;
	int components;
	int bits;

	const std::vector<unsigned char>* p_data;
};

struct TextureDesc {
	std::string name;

	Image* p_image;
	glm::vec4 factor;
};

struct MaterialDesc {
	std::string name;

	Texture* p_TexArray[Material::TextureIdx::COUNT];
};



class ResourceManager final {
private:
	std::unordered_map<std::string, Image*> images_;
	std::unordered_map<std::string, Texture*> textures_;
	std::unordered_map<std::string, Material*> materials_;

	void createDefaultImages();
	void createDefaultTextures();
	void createDefaultMaterials();

	static ResourceManager* instancePtr;
	ResourceManager();

public:

	ResourceManager(const ResourceManager& obj) = delete;

	static ResourceManager* getInstance() {
		if (!instancePtr)
			instancePtr = new ResourceManager();

		return instancePtr;
	}

	Image& createImage(const ImageDesc& imageDesc);
	Texture& createTexture(const TextureDesc& textureDesc);
	Material& createMaterial(const MaterialDesc& matDesc);

	void deleteImage(const std::string& name);
	void deleteTexture(const std::string& name);
	void deleteMaterial(const std::string& name);

	Image& getImage(const std::string& name);
	Texture& getTexture(const std::string& name);
	Material& getMaterial(const std::string& name);

	bool hasImage(const std::string& name);
	bool hasTexture(const std::string& name);
	bool hasMaterial(const std::string& name);

	~ResourceManager();
};

}
#endif
