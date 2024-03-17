#ifndef RESOURCE_MANAGER_HPP
#define RESOURCE_MANAGER_HPP

#include <unordered_map>
#include <array>

#include "Image.hpp"
#include "Sampler.hpp"
#include "Texture.hpp"
#include "Material.hpp"
#include "Buffer.hpp"
#include "Shader.hpp"
#include "Framebuffer.hpp"

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
	Image* p_images[6] = {};
	glm::vec4 factor = { 1, 1, 1, 1 };
	int format = GL_RGBA;

	Sampler* p_sampler = nullptr;
};

struct MaterialDesc : RenderResourceDesc {

	Texture* p_TexArray[Material::TextureIdx::IDX_COUNT];
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

struct FramebufferDesc : RenderResourceDesc {
	unsigned colorAttachmentsCount = 1;
	Texture* colorAttachments[Framebuffer::MAXIMUM_COLOR_ATTACHMENTS_COUNT] = {};
	Texture* depthAttachment = nullptr;
	std::string dependency = "";
};


class ResourceManager final {
private:
	std::unordered_map<std::string, Image*> images_;
	std::unordered_map<std::string, Sampler*> samplers_;
	std::unordered_map<std::string, Texture*> textures_;
	std::unordered_map<std::string, Material*> materials_;
	std::unordered_map<std::string, Buffer*> buffers_;
	std::unordered_map<std::string, Shader*> shaders_;
	std::unordered_map<std::string, Framebuffer*> framebuffers_;

	std::unordered_map<ResourceHandle, RenderResource*> allResources_;

	std::array<Image*, Image::DefaultImages::COUNT> defaultImages_;
	std::array<Sampler*, Sampler::DefaultSamplers::COUNT> defaultSamplers_;
	std::array<Texture*, Texture::DefaultTextures::COUNT> defaultTextures_;
	std::array<Material*, Material::DefaultMaterials::COUNT> defaultMaterials_;
	std::array<Framebuffer*, Framebuffer::DefaultFramebuffers::COUNT> defaultFramebuffers_;

	void createDefaultImages();
	void createDefaultSamplers();
	void createDefaultTextures();
	void createDefaultMaterials();
	void createDefaultFramebuffer();

	static ResourceManager* instancePtr;
	ResourceManager();

public:

	ResourceManager(const ResourceManager& obj) = delete;

	static ResourceManager* getInstance() {
		if (!instancePtr)
			instancePtr = new ResourceManager();

		return instancePtr;
	}

	void Init();

	Image& createImage(const ImageDesc& imageDesc);
	Image& createImage(const char* filename, bool isHdr = false);
	Image& createImage(const std::string& filename, bool isHdr = false);

	Sampler& createSampler(const SamplerDesc& samplerDesc);
	
	Texture& createTexture(const TextureDesc& textureDesc);
	Texture& createTexture(const char* filename, bool isHdr = false, Sampler* sampler = nullptr);
	Texture& createTexture(const std::string& filename, bool isHdr = false, Sampler* sampler = nullptr);
	void bindTexture(const std::string& name, const unsigned texUnit = 0);
	void bindTexture(const ResourceHandle handle, const unsigned texUnit = 0);
	void unbindTexture(const GLenum target = GL_TEXTURE_2D, const unsigned texUnit = 0);

	Material& createMaterial(const MaterialDesc& matDesc);

	Buffer& createBuffer(const BufferDesc& bufDesc);
	void updateBuffer(const std::string& name, const unsigned char* data, const size_t bytesize, const size_t byteoffset = 0);
	void bindBufferShader(const std::string& name, const unsigned binding, const Shader& shader);

	Framebuffer& createFramebuffer(const FramebufferDesc& framebufDesc);
	void bindFramebuffer(const std::string& name);
	void bindFramebuffer(const ResourceHandle handle);
	void resizeFramebuffer(const std::string& name, unsigned width, unsigned height);
	void resizeFramebuffer(const ResourceHandle handle, unsigned width, unsigned height);

	Shader& createShader(const ShaderDesc& shaderDesc);

	void generateMipMaps(const std::string& texName);
	void generateMipMaps(const ResourceHandle handle);

	void deleteImage(const std::string& name);
	void deleteSampler(const std::string& name);
	void deleteTexture(const std::string& name);
	void deleteMaterial(const std::string& name);
	void deleteBuffer(const std::string& name);
	void deleteShader(const std::string& name);
	void deleteFramebuffer(const std::string& name);

	Image& getImage(const std::string& name);
	Sampler& getSampler(const std::string& name);
	Texture& getTexture(const std::string& name);
	Material& getMaterial(const std::string& name);
	Buffer& getBuffer(const std::string& name);
	Shader& getShader(const std::string& name);
	Framebuffer& getFramebuffer(const std::string& name);

	Image& getImage(const ResourceHandle handle);
	Sampler& getSampler(const ResourceHandle handle);
	Texture& getTexture(const ResourceHandle handle);
	Material& getMaterial(const ResourceHandle handle);
	Buffer& getBuffer(const ResourceHandle handle);
	Shader& getShader(const ResourceHandle handle);
	Framebuffer& getFramebuffer(const ResourceHandle handle);

	inline Image& getImage(const Image::DefaultImages defaultImage) { return *defaultImages_[defaultImage]; }
	inline Sampler& getSampler(const Sampler::DefaultSamplers defaultSampler) { return *defaultSamplers_[defaultSampler]; };
	inline Texture& getTexture(const Texture::DefaultTextures defaultTexture) { return *defaultTextures_[defaultTexture]; };
	inline Material& getMaterial(const Material::DefaultMaterials defaultMaterial) { return *defaultMaterials_[defaultMaterial]; };
	inline Framebuffer& getFramebuffer(const Framebuffer::DefaultFramebuffers defaultFramebuffer) { return *defaultFramebuffers_[defaultFramebuffer]; };
	// TODO: Buffer, Shader

	bool hasImage(const std::string& name);
	bool hasSampler(const std::string& name);
	bool hasTexture(const std::string& name);
	bool hasMaterial(const std::string& name);
	bool hasBuffer(const std::string& name);
	bool hasShader(const std::string& name);
	bool hasFramebuffer(const std::string& name);

	bool hasResource(const ResourceHandle handle);

	bool hasImage(const std::string& name, const std::string& uri);
	bool hasSampler(const std::string& name, const std::string& uri);
	bool hasTexture(const std::string& name, const std::string& uri);
	bool hasMaterial(const std::string& name, const std::string& uri);
	bool hasBuffer(const std::string& name, const std::string& uri);
	bool hasShader(const std::string& name, const std::string& uri);
	bool hasFramebuffer(const std::string& name, const std::string& uri);

	unsigned chooseDefaultInternalFormat(const int components, bool isFloat = false) const;

	void cleanUp();

	~ResourceManager() { cleanUp(); }
};

}
#endif
