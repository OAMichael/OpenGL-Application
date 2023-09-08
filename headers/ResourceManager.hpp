#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <unordered_map>

#define DEBUG_MODEL

#include "Image.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"

namespace Resources {

struct ImageDesc : RenderResourceDesc {

	int width;
	int height;
	int components;
	int bits;
	int format = GL_RGBA;

	const unsigned char* p_data;
};

struct SamplerDesc : RenderResourceDesc {

	uint32_t minFilter = Sampler::Filter::NEAREST_MIPMAP_LINEAR;
	uint32_t magFilter = Sampler::Filter::LINEAR;

	uint32_t wrapS = Sampler::WrapMode::REPEAT;
	uint32_t wrapT = Sampler::WrapMode::REPEAT;
	uint32_t wrapR = Sampler::WrapMode::REPEAT;
};

struct TextureDesc : RenderResourceDesc {

	unsigned faces = 1;
	Image* p_images[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	glm::vec4 factor;
	int format = GL_RGBA;

	Sampler* p_sampler = nullptr;
};

struct MaterialDesc : RenderResourceDesc {

	Texture* p_TexArray[Material::TextureIdx::COUNT];
};

struct BufferDesc : RenderResourceDesc {
	unsigned int target;

	const unsigned char* p_data;
	size_t bytesize;
};

struct ShaderDesc : RenderResourceDesc {

	std::string vertFilename;
	std::string fragFilename;
};


class ResourceManager final {
private:
	std::unordered_map<std::string, Image*> images_;
	std::unordered_map<std::string, Sampler*> samplers_;
	std::unordered_map<std::string, Texture*> textures_;
	std::unordered_map<std::string, Material*> materials_;
	std::unordered_map<std::string, Buffer*> buffers_;
	std::unordered_map<std::string, GeneralApp::Shader*> shaders_;

	std::unordered_map<ResourceHandle, RenderResource*> allResources_;

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

	Buffer& createBuffer(const BufferDesc& bufDesc);
	void updateBuffer(const std::string& name, const unsigned char* data, const size_t bytesize, const size_t byteoffset = 0);
	void bindBufferShader(const std::string& name, const unsigned binding, const GeneralApp::Shader& shader);

	GeneralApp::Shader& createShader(const ShaderDesc& shaderDesc);

	void generateMipMaps(const std::string& texName);
	void generateMipMaps(const ResourceHandle handle);

	void deleteImage(const std::string& name);
	void deleteSampler(const std::string& name);
	void deleteTexture(const std::string& name);
	void deleteMaterial(const std::string& name);
	void deleteBuffer(const std::string& name);
	void deleteShader(const std::string& name);

	Image& getImage(const std::string& name);
	Sampler& getSampler(const std::string& name);
	Texture& getTexture(const std::string& name);
	Material& getMaterial(const std::string& name);
	Buffer& getBuffer(const std::string& name);
	GeneralApp::Shader& getShader(const std::string& name);

	RenderResource* getResource(const ResourceHandle handle);

	bool hasImage(const std::string& name);
	bool hasSampler(const std::string& name);
	bool hasTexture(const std::string& name);
	bool hasMaterial(const std::string& name);
	bool hasBuffer(const std::string& name);
	bool hasShader(const std::string& name);

	bool hasResource(const ResourceHandle handle);

	bool hasImage(const std::string& name, const std::string& uri);
	bool hasSampler(const std::string& name, const std::string& uri);
	bool hasTexture(const std::string& name, const std::string& uri);
	bool hasMaterial(const std::string& name, const std::string& uri);
	bool hasBuffer(const std::string& name, const std::string& uri);
	bool hasShader(const std::string& name, const std::string& uri);

	void cleanUp();

	~ResourceManager();
};

}
#endif
