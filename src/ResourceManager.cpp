#include "ResourceManager.hpp"
#include "../3rdparty/stb_image.h"


namespace Resources {

static inline ResourceHandle createNewResourceHandle() {
    static uint64_t resource = 0;
    ++resource;
    
    return { resource };
}

ResourceManager* ResourceManager::instancePtr = nullptr;

Image& ResourceManager::createImage(const ImageDesc& imageDesc) {
    if (hasImage(imageDesc.name, imageDesc.uri))
        return getImage(imageDesc.name);

#ifdef DEBUG_MODEL
    std::cout << "Creating image \'" << imageDesc.name << "\'" << std::endl;
#endif

    Image* newImage = new Image();

    newImage->bits = imageDesc.bits;
    newImage->components = imageDesc.components;
    newImage->width = imageDesc.width;
    newImage->height = imageDesc.height;
    newImage->format = imageDesc.format;
    newImage->name = imageDesc.name;
    newImage->uri = imageDesc.uri;

    newImage->type = RenderResource::ResourceType::IMAGE;

    size_t bytesize = imageDesc.width * imageDesc.height * imageDesc.components * (imageDesc.bits / 8);
    newImage->image.resize(bytesize);
    std::copy(imageDesc.p_data, imageDesc.p_data + bytesize, newImage->image.begin());

    newImage->handle = createNewResourceHandle();

    images_[newImage->name] = newImage;
    allResources_[newImage->handle] = newImage;

    return *newImage;
}

Image& ResourceManager::createImage(const char* filename) {
    if (hasImage(filename))
        return getImage(filename);

    int width, height, nrChannels;
    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 0);
    if (data) {
        ImageDesc imageDesc;
        imageDesc.name = filename;
        imageDesc.uri = filename;
        imageDesc.width = width;
        imageDesc.height = height;
        imageDesc.components = nrChannels;
        imageDesc.bits = 8;                     // stb_image automatically converts
        imageDesc.p_data = data;

        auto& im = createImage(imageDesc);

        stbi_image_free(data);
        return im;
    }
    else {
        printf("Failed to load image: \'%s\'\n", filename);
        stbi_image_free(data);
        return getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_WHITE]);
    }
}

Image& ResourceManager::createImage(const std::string& filename) {
    return createImage(filename.c_str());
}

Sampler& ResourceManager::createSampler(const SamplerDesc& samplerDesc) {
    if (hasSampler(samplerDesc.name, samplerDesc.uri))
        return getSampler(samplerDesc.name);

#ifdef DEBUG_MODEL
    std::cout << "Creating sampler \'" << samplerDesc.name << "\'" << std::endl;
#endif

    Sampler* newSampler = new Sampler();
    
    newSampler->name = samplerDesc.name;
    newSampler->uri = samplerDesc.uri;

    newSampler->type = RenderResource::ResourceType::SAMPLER;

    newSampler->minFilter = samplerDesc.minFilter;
    newSampler->magFilter = samplerDesc.magFilter;
    newSampler->wrapS = samplerDesc.wrapS;
    newSampler->wrapT = samplerDesc.wrapT;
    newSampler->wrapR = samplerDesc.wrapR;

    glGenSamplers(1, &newSampler->GL_id);
    glSamplerParameteri(newSampler->GL_id, GL_TEXTURE_WRAP_S, samplerDesc.wrapS);
    glSamplerParameteri(newSampler->GL_id, GL_TEXTURE_WRAP_T, samplerDesc.wrapT);
    glSamplerParameteri(newSampler->GL_id, GL_TEXTURE_WRAP_R, samplerDesc.wrapR);
    glSamplerParameteri(newSampler->GL_id, GL_TEXTURE_MIN_FILTER, samplerDesc.minFilter);
    glSamplerParameteri(newSampler->GL_id, GL_TEXTURE_MAG_FILTER, samplerDesc.magFilter);

    newSampler->handle = createNewResourceHandle();

    samplers_[newSampler->name] = newSampler;
    allResources_[newSampler->handle] = newSampler;

    return *newSampler;
}

Texture& ResourceManager::createTexture(const TextureDesc& textureDesc) {
    if (hasTexture(textureDesc.name, textureDesc.uri))
        return getTexture(textureDesc.name);

#ifdef DEBUG_MODEL
    std::cout << "Creating texture \'" << textureDesc.name << "\'" << std::endl;
#endif

    Texture* newTexture = new Texture();

    newTexture->name = textureDesc.name;
    newTexture->uri = textureDesc.uri;

    newTexture->type = RenderResource::ResourceType::TEXTURE;

    newTexture->factor = textureDesc.factor;
    newTexture->faces = textureDesc.faces;
    newTexture->format = textureDesc.format;

    textures_[newTexture->name] = newTexture;

    if (!textureDesc.p_images[0]) {
        std::cout << "Image cannot be NULL !!!" << std::endl;
        return *newTexture;
    }

    for(int i = 0; i < textureDesc.faces; ++i)
        newTexture->images[i] = textureDesc.p_images[i];

    glGenTextures(1, &newTexture->GL_id);

    if(textureDesc.faces == 1)
        glBindTexture(GL_TEXTURE_2D, newTexture->GL_id);
    else if(textureDesc.faces == 6)
        glBindTexture(GL_TEXTURE_CUBE_MAP, newTexture->GL_id);
    else {
        std::cout << "Number of faces must be 1 or 6 !!!" << std::endl;
        return *newTexture;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    newTexture->sampler = textureDesc.p_sampler;
    if (!newTexture->sampler) {
        newTexture->sampler = &getSampler(defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_REPEAT]);
    }

    if (newTexture->faces == 1) {
        GLenum format = GL_RGBA;
        switch (newTexture->images[0]->components) {
        case 1:
            format = GL_RED;
            break;
        case 2:
            format = GL_RG;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            std::cout << "Undefined image format" << std::endl;
        }

        GLenum type = GL_UNSIGNED_BYTE;
        if (newTexture->images[0]->bits == 16)
            type = GL_UNSIGNED_SHORT;

        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            textureDesc.format,
            newTexture->images[0]->width,
            newTexture->images[0]->height,
            0,
            format,
            type,
            newTexture->images[0]->image.data()
        );
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else {
        for (int i = 0; i < newTexture->faces; ++i) {
            GLenum format = GL_RGBA;
            switch (newTexture->images[i]->components) {
            case 1:
                format = GL_RED;
                break;
            case 2:
                format = GL_RG;
                break;
            case 3:
                format = GL_RGB;
                break;
            case 4:
                format = GL_RGBA;
                break;
            default:
                std::cout << "Undefined image format" << std::endl;
            }

            GLenum type = GL_UNSIGNED_BYTE;
            if (newTexture->images[i]->bits == 16)
                type = GL_UNSIGNED_SHORT;

            glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0,
                textureDesc.format,
                newTexture->images[i]->width,
                newTexture->images[i]->height,
                0,
                format,
                type,
                newTexture->images[i]->image.data()
            );
        }
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
    newTexture->handle = createNewResourceHandle();
    allResources_[newTexture->handle] = newTexture;

    return *newTexture;
}

Texture& ResourceManager::createTexture(const char* filename, Sampler* sampler) {
    if (hasTexture(filename))
        return getTexture(filename);

    auto& image = createImage(filename);

    TextureDesc texDesc;
    texDesc.factor = glm::vec4(1.0f);
    texDesc.name = image.name;
    texDesc.uri = filename;
    texDesc.p_images[0] = &image;
    texDesc.p_sampler = sampler;

    return createTexture(texDesc);
}

Texture& ResourceManager::createTexture(const std::string& filename, Sampler* sampler) {
    return createTexture(filename.c_str(), sampler);
}


Material& ResourceManager::createMaterial(const MaterialDesc& matDesc) {
    if (hasMaterial(matDesc.name, matDesc.uri))
        return getMaterial(matDesc.name);

    Material* newMaterial = new Material();

#ifdef DEBUG_MODEL
    std::cout << "Creating material \'" << matDesc.name << "\'" << std::endl;
#endif

    for(int i = 0; i < Material::TextureIdx::COUNT; ++i)
        newMaterial->textures[i] = matDesc.p_TexArray[i];
    
    newMaterial->name = matDesc.name;
    newMaterial->uri = matDesc.uri;
    newMaterial->type = RenderResource::ResourceType::MATERIAL;
    newMaterial->handle = createNewResourceHandle();

    materials_[newMaterial->name] = newMaterial;
    allResources_[newMaterial->handle] = newMaterial;

    return *newMaterial;
}

Buffer& ResourceManager::createBuffer(const BufferDesc& bufDesc) {
    if (hasBuffer(bufDesc.name, bufDesc.uri))
        return getBuffer(bufDesc.name);

    Buffer* newBuffer = new Buffer();

#ifdef DEBUG_MODEL
    std::cout << "Creating buffer \'" << bufDesc.name << "\'" << std::endl;
#endif

    newBuffer->name = bufDesc.name;
    newBuffer->uri = bufDesc.uri;
    newBuffer->type = RenderResource::ResourceType::BUFFER;
    newBuffer->target = bufDesc.target;

    newBuffer->data.resize(bufDesc.bytesize);
    std::fill(newBuffer->data.begin(), newBuffer->data.end(), 0);
    if(bufDesc.p_data)
        std::copy(bufDesc.p_data, bufDesc.p_data + bufDesc.bytesize, newBuffer->data.begin());

    glGenBuffers(1, &(newBuffer->GL_id));

    glBindBuffer(newBuffer->target, newBuffer->GL_id);
    glBufferData(newBuffer->target, bufDesc.bytesize, newBuffer->data.data(), GL_STATIC_DRAW);
    glBindBuffer(newBuffer->target, 0);

    newBuffer->handle = createNewResourceHandle();

    buffers_[newBuffer->name] = newBuffer;
    allResources_[newBuffer->handle] = newBuffer;

    return *newBuffer;
}

void ResourceManager::updateBuffer(const std::string& name, const unsigned char* data, const size_t bytesize, const size_t byteoffset) {
    auto it = buffers_.find(name);
    if (it == buffers_.end())
        return;

    std::copy(data + byteoffset, data + byteoffset + bytesize, it->second->data.begin() + byteoffset);

    glBindBuffer(it->second->target, it->second->GL_id);
    glBufferSubData(it->second->target, byteoffset, bytesize, data);
    glBindBuffer(it->second->target, 0);
}

void ResourceManager::bindBufferShader(const std::string& name, const unsigned binding, const GeneralApp::Shader& shader) {
    auto it = buffers_.find(name);
    if (it == buffers_.end())
        return;

    glBindBufferRange(it->second->target, binding, it->second->GL_id, 0, it->second->data.size());

    unsigned int uboIndex = glGetUniformBlockIndex(shader.GL_id, name.c_str());
    glUniformBlockBinding(shader.GL_id, uboIndex, binding);
}


void ResourceManager::generateMipMaps(const std::string& texName) {
    if (!hasTexture(texName))
        return;

    auto& texture = getTexture(texName);
    
    auto texType = texture.faces == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
    glBindTexture(texType, texture.GL_id);
    glGenerateMipmap(texType);
    glBindTexture(texType, 0);
}

GeneralApp::Shader& ResourceManager::createShader(const ShaderDesc& shaderDesc) {
    if (hasShader(shaderDesc.name, shaderDesc.uri))
        return getShader(shaderDesc.name);

#ifdef DEBUG_MODEL
    std::cout << "Creating shader \'" << shaderDesc.name << "\'" << std::endl;
#endif

    GeneralApp::Shader* newShader = new GeneralApp::Shader(shaderDesc.vertFilename.c_str(), shaderDesc.fragFilename.c_str());
    newShader->name = shaderDesc.name;
    newShader->uri = shaderDesc.uri;
    newShader->type = RenderResource::ResourceType::SHADER;

    newShader->handle = createNewResourceHandle();

    shaders_[newShader->name] = newShader;
    allResources_[newShader->handle] = newShader;

    return *newShader;
}


void ResourceManager::deleteImage(const std::string& name) {
    if (auto it = images_.find(name); it != images_.end()) {
        delete images_[name];
        images_.erase(it);
    }
}

void ResourceManager::deleteSampler(const std::string& name) {
    if (auto it = samplers_.find(name); it != samplers_.end()) {
        glDeleteSamplers(1, &(it->second->GL_id));
        delete samplers_[name];
        samplers_.erase(it);
    }
}

void ResourceManager::deleteTexture(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end()) {
        glDeleteTextures(1, &(it->second->GL_id));
        delete textures_[name];
        textures_.erase(it);
    }
}

void ResourceManager::deleteMaterial(const std::string& name) {
    if (auto it = materials_.find(name); it != materials_.end()) {
        delete materials_[name];
        materials_.erase(it);
    }
}

void ResourceManager::deleteBuffer(const std::string& name) {
    if (auto it = buffers_.find(name); it != buffers_.end()) {
        glDeleteBuffers(1, &(it->second->GL_id));
        delete buffers_[name];
        buffers_.erase(it);
    }
}

void ResourceManager::deleteShader(const std::string& name) {
    if (auto it = shaders_.find(name); it != shaders_.end()) {
        glDeleteProgram(it->second->GL_id);
        delete shaders_[name];
        shaders_.erase(it);
    }
}


Image& ResourceManager::getImage(const std::string& name) {
    if (hasImage(name))
        return *images_[name];

    auto emptyImage = new Image();
    images_[name] = emptyImage;
    return *emptyImage;
}

Sampler& ResourceManager::getSampler(const std::string& name) {
    if (hasSampler(name))
        return *samplers_[name];

    auto emptySampler = new Sampler();
    samplers_[name] = emptySampler;
    return *emptySampler;
}

Texture& ResourceManager::getTexture(const std::string& name) {
    if (hasTexture(name))
        return *textures_[name];

    auto emptyTexture = new Texture();
    textures_[name] = emptyTexture;
    return *emptyTexture;
}

Material& ResourceManager::getMaterial(const std::string& name) {
    if (hasMaterial(name))
        return *materials_[name];

    auto emptyMaterial = new Material();
    materials_[name] = emptyMaterial;
    return *emptyMaterial;
}

Buffer& ResourceManager::getBuffer(const std::string& name) {
    if (hasBuffer(name))
        return *buffers_[name];

    auto emptyBuffer = new Buffer();
    buffers_[name] = emptyBuffer;
    return *emptyBuffer;
}

GeneralApp::Shader& ResourceManager::getShader(const std::string& name) {
    if (hasShader(name))
        return *shaders_[name];

    auto emptyShader = new GeneralApp::Shader();
    shaders_[name] = emptyShader;
    return *emptyShader;
}

RenderResource* ResourceManager::getResource(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it != allResources_.end())
        return it->second;

    return nullptr;
}


bool ResourceManager::hasImage(const std::string& name) {
    auto it = images_.find(name);
    return it != images_.end();
}

bool ResourceManager::hasSampler(const std::string& name) {
    auto it = samplers_.find(name);
    return it != samplers_.end();
}

bool ResourceManager::hasTexture(const std::string& name) {
    auto it = textures_.find(name);
    return it != textures_.end();
}

bool ResourceManager::hasMaterial(const std::string& name) {
    auto it = materials_.find(name);
    return it != materials_.end();
}

bool ResourceManager::hasBuffer(const std::string& name) {
    auto it = buffers_.find(name);
    return it != buffers_.end();
}

bool ResourceManager::hasShader(const std::string& name) {
    auto it = shaders_.find(name);
    return it != shaders_.end();
}


bool ResourceManager::hasResource(const ResourceHandle handle) {
    return allResources_.find(handle) != allResources_.end();
}


bool ResourceManager::hasImage(const std::string& name, const std::string& uri) {
    auto nameIt = images_.find(name);
    if (nameIt == images_.end())
        return false;

    for (auto im : images_) {
        if (im.first != name)
            continue;

        if (im.second->uri == uri)
            return true;
    }

    return false;
}

bool ResourceManager::hasSampler(const std::string& name, const std::string& uri) {
    auto nameIt = samplers_.find(name);
    if (nameIt == samplers_.end())
        return false;

    for (auto samp : samplers_) {
        if (samp.first != name)
            continue;

        if (samp.second->uri == uri)
            return true;
    }

    return false;
}

bool ResourceManager::hasTexture(const std::string& name, const std::string& uri) {
    auto nameIt = textures_.find(name);
    if (nameIt == textures_.end())
        return false;

    for (auto tex : textures_) {
        if (tex.first != name)
            continue;

        if (tex.second->uri == uri)
            return true;
    }

    return false;
}

bool ResourceManager::hasMaterial(const std::string& name, const std::string& uri) {
    auto nameIt = materials_.find(name);
    if (nameIt == materials_.end())
        return false;

    for (auto mat : materials_) {
        if (mat.first != name)
            continue;

        if (mat.second->uri == uri)
            return true;
    }

    return false;
}

bool ResourceManager::hasBuffer(const std::string& name, const std::string& uri) {
    auto nameIt = buffers_.find(name);
    if (nameIt == buffers_.end())
        return false;

    for (auto buf : buffers_) {
        if (buf.first != name)
            continue;

        if (buf.second->uri == uri)
            return true;
    }

    return false;
}

bool ResourceManager::hasShader(const std::string& name, const std::string& uri) {
    auto nameIt = shaders_.find(name);
    if (nameIt == shaders_.end())
        return false;

    for (auto shdr : shaders_) {
        if (shdr.first != name)
            continue;

        if (shdr.second->uri == uri)
            return true;
    }

    return false;
}


ResourceManager::ResourceManager() {
    createDefaultImages();
    createDefaultSamplers();
    createDefaultTextures();
    createDefaultMaterials();
}

ResourceManager::~ResourceManager() {
    cleanUp();
}


void ResourceManager::cleanUp() {
    for (auto& im : images_) {
        delete im.second;
        images_.erase(im.first);
    }

    for (auto& samp : samplers_) {
        glDeleteSamplers(1, &(samp.second->GL_id));
        delete samp.second;
        samplers_.erase(samp.first);
    }

    for (auto& tex : textures_) {
        glDeleteTextures(1, &(tex.second->GL_id));
        delete tex.second;
        textures_.erase(tex.first);
    }

    for (auto& mat : materials_) {
        delete mat.second;
        materials_.erase(mat.first);
    }

    for (auto& buf : buffers_) {
        glDeleteBuffers(1, &(buf.second->GL_id));
        delete buf.second;
        buffers_.erase(buf.first);
    }

    for (auto& shader : shaders_) {
        glDeleteProgram(shader.second->GL_id);
        delete shader.second;
        shaders_.erase(shader.first);
    }
}


void ResourceManager::createDefaultImages() {
    // Default white and black images share most of quantities
    ImageDesc defaultDesc;
    defaultDesc.width = 1;
    defaultDesc.height = 1;
    defaultDesc.components = 4;
    defaultDesc.bits = 8;

    std::vector<unsigned char> defaultValue;
    defaultValue.resize(4);
    defaultDesc.p_data = defaultValue.data();

    defaultDesc.name = defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_WHITE];
    defaultValue[0] = 255;
    defaultValue[1] = 255;
    defaultValue[2] = 255;
    defaultValue[3] = 255;
    createImage(defaultDesc);

    defaultDesc.name = defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK];
    defaultValue[0] = 0;
    defaultValue[1] = 0;
    defaultValue[2] = 0;
    defaultValue[3] = 0;
    createImage(defaultDesc);
}

void ResourceManager::createDefaultSamplers() {
    SamplerDesc defaultDesc;

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_CLAMP];
    defaultDesc.minFilter = Sampler::NEAREST;
    defaultDesc.magFilter = Sampler::NEAREST;
    defaultDesc.wrapS = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapT = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapR = Sampler::CLAMP_TO_EDGE;
    createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_REPEAT];
    defaultDesc.minFilter = Sampler::NEAREST;
    defaultDesc.magFilter = Sampler::NEAREST;
    defaultDesc.wrapS = Sampler::REPEAT;
    defaultDesc.wrapT = Sampler::REPEAT;
    defaultDesc.wrapR = Sampler::REPEAT;
    createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_REPEAT];
    defaultDesc.minFilter = Sampler::LINEAR;
    defaultDesc.magFilter = Sampler::LINEAR;
    defaultDesc.wrapS = Sampler::REPEAT;
    defaultDesc.wrapT = Sampler::REPEAT;
    defaultDesc.wrapR = Sampler::REPEAT;
    createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_CLAMP];
    defaultDesc.minFilter = Sampler::LINEAR;
    defaultDesc.magFilter = Sampler::LINEAR;
    defaultDesc.wrapS = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapT = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapR = Sampler::CLAMP_TO_EDGE;
    createSampler(defaultDesc);
}

void ResourceManager::createDefaultTextures() {
    TextureDesc defaultDesc;
    defaultDesc.factor = glm::vec4(1.0);

    auto& defaultImageWhite = getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_WHITE]);
    defaultDesc.p_images[0] = &defaultImageWhite;
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE];
    createTexture(defaultDesc);

    auto& defaultImageBlack = getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK]);
    defaultDesc.p_images[0] = &defaultImageBlack;
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK];
    createTexture(defaultDesc);
}

void ResourceManager::createDefaultMaterials() {
    MaterialDesc defaultDesc;
    defaultDesc.name = defaultMaterialName;
    
    auto& defaultTextureWhite = getTexture(defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE]);

    for (int i = 0; i < Texture::DefaultTextures::COUNT; ++i)
        defaultDesc.p_TexArray[i] = &defaultTextureWhite;

    createMaterial(defaultDesc);
}

}
