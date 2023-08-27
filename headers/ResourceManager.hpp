#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <unordered_map>

#define DEBUG_MODEL

#include "../headers/Image.hpp"
#include "../headers/Sampler.hpp"
#include "../headers/Texture.hpp"
#include "../headers/Material.hpp"

namespace Resources {

struct ImageDesc {
	std::string name;

	int width;
	int height;
	int components;
	int bits;

	const unsigned char* p_data;
};

struct SamplerDesc {
	std::string name;

	uint32_t minFilter = Sampler::Filter::NEAREST_MIPMAP_LINEAR;
	uint32_t magFilter = Sampler::Filter::LINEAR;

	uint32_t wrapS = Sampler::WrapMode::REPEAT;
	uint32_t wrapT = Sampler::WrapMode::REPEAT;
	uint32_t wrapR = Sampler::WrapMode::REPEAT;
};

struct TextureDesc {
	std::string name;

	unsigned faces = 1;
	Image* p_images[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	glm::vec4 factor;

	Sampler* p_sampler = nullptr;
};

struct MaterialDesc {
	std::string name;

	Texture* p_TexArray[Material::TextureIdx::COUNT];
};


class ResourceManager final {
private:
	std::unordered_map<std::string, Image*> images_;
	std::unordered_map<std::string, Sampler*> samplers_;
	std::unordered_map<std::string, Texture*> textures_;
	std::unordered_map<std::string, Material*> materials_;

	struct UBO {
		unsigned int binding;
		std::string name;
	};
	std::vector<UBO> UBOs_;

	void createDefaultImages();
	void createDefaultSamplers();
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
	Image& createImage(const char* filename);
	Image& createImage(const std::string& filename);

	Sampler& createSampler(const SamplerDesc& samplerDesc);
	
	Texture& createTexture(const TextureDesc& textureDesc);
	Texture& createTexture(const char* filename, Sampler* sampler = nullptr);
	Texture& createTexture(const std::string& filename, Sampler* sampler = nullptr);

	Material& createMaterial(const MaterialDesc& matDesc);


	template<typename T>
	void createUBO(const unsigned binding, const T* data, const size_t bytesize, const std::string& name) {
		unsigned int ubo;
		glGenBuffers(1, &ubo);

		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, bytesize, data, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBufferRange(GL_UNIFORM_BUFFER, binding, ubo, 0, bytesize);

		UBOs_.push_back({ ubo, name });
	}

	template<typename T>
	void updateUBO(const T* data, const size_t bytesize, const size_t byteoffset, const std::string& name) {
		if (auto it = std::find_if(UBOs_.begin(), UBOs_.end(), [&name](const UBO& u) -> bool { return u.name == name; });
		it != UBOs_.end()) {
			glBindBuffer(GL_UNIFORM_BUFFER, it->binding);
			glBufferSubData(GL_UNIFORM_BUFFER, byteoffset, bytesize, data);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
	}

	void deleteImage(const std::string& name);
	void deleteSampler(const std::string& name);
	void deleteTexture(const std::string& name);
	void deleteMaterial(const std::string& name);

	Image& getImage(const std::string& name);
	Sampler& getSampler(const std::string& name);
	Texture& getTexture(const std::string& name);
	Material& getMaterial(const std::string& name);

	bool hasImage(const std::string& name);
	bool hasSampler(const std::string& name);
	bool hasTexture(const std::string& name);
	bool hasMaterial(const std::string& name);


	~ResourceManager();
};

}
#endif
