#include "ResourceManager.hpp"
#include "Logger.hpp"
#include "stb_image.h"

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

    LOG_I("Creating image '\%s\' with URI \'%s\'", imageDesc.name.c_str(), imageDesc.uri.c_str());

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
    if (imageDesc.p_data) {
        std::copy(imageDesc.p_data, imageDesc.p_data + bytesize, newImage->image.begin());
    }

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
        LOG_E("Failed to load image: \'%s\'", filename);
        stbi_image_free(data);
        return getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK]);
    }
}

Image& ResourceManager::createImage(const std::string& filename) {
    return createImage(filename.c_str());
}

Sampler& ResourceManager::createSampler(const SamplerDesc& samplerDesc) {
    if (hasSampler(samplerDesc.name, samplerDesc.uri))
        return getSampler(samplerDesc.name);

    LOG_I("Creating sampler \'%s\' with URI \'%s\'", samplerDesc.name.c_str(), samplerDesc.uri.c_str());

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

    LOG_I("Creating texture \'%s\' with URI \'%s\'", textureDesc.name.c_str(), textureDesc.uri.c_str());

    Texture* newTexture = new Texture();

    newTexture->name = textureDesc.name;
    newTexture->uri = textureDesc.uri;

    newTexture->type = RenderResource::ResourceType::TEXTURE;

    newTexture->factor = textureDesc.factor;
    newTexture->faces = textureDesc.faces;
    newTexture->format = textureDesc.format;

    if (!textureDesc.p_images[0]) {
        LOG_E("Image cannot be NULL");
        delete newTexture;
        return getTexture(defaultTexturesNames[Texture::DEFAULT_TEXTURE_BLACK]);
    }

    for(int i = 0; i < textureDesc.faces; ++i)
        newTexture->images[i] = textureDesc.p_images[i];

    glGenTextures(1, &newTexture->GL_id);

    if(textureDesc.faces == 1)
        glBindTexture(GL_TEXTURE_2D, newTexture->GL_id);
    else if(textureDesc.faces == 6)
        glBindTexture(GL_TEXTURE_CUBE_MAP, newTexture->GL_id);
    else {
        LOG_E("Number of faces must be 1 or 6");
        delete newTexture;
        return getTexture(defaultTexturesNames[Texture::DEFAULT_TEXTURE_BLACK]);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    newTexture->sampler = textureDesc.p_sampler;
    if (!newTexture->sampler) {
        newTexture->sampler = &getSampler(defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_REPEAT]);
    }

    if (newTexture->faces == 1) {
        // Handle depth texture
        if (textureDesc.format == GL_DEPTH_COMPONENT || textureDesc.format == GL_DEPTH_COMPONENT16 ||
            textureDesc.format == GL_DEPTH_COMPONENT24 || textureDesc.format == GL_DEPTH_COMPONENT32 ||
            textureDesc.format == GL_DEPTH_COMPONENT32F) {

            GLenum type = GL_FLOAT;
            // TODO: type

            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                textureDesc.format,
                newTexture->images[0]->width,
                newTexture->images[0]->height,
                0,
                GL_DEPTH_COMPONENT,
                type,
                newTexture->images[0]->image.data()
            );
        }
        else {
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
                LOG_W("Undefined image format");
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
        }
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
                LOG_W("Undefined image format");
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
    textures_[newTexture->name] = newTexture;

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

    LOG_I("Creating material \'%s\' with URI \'%s\'", matDesc.name.c_str(), matDesc.uri.c_str());

    for (int i = 0; i < Material::TextureIdx::COUNT; ++i) {
        newMaterial->textures[i] = matDesc.p_TexArray[i];
    }
    
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

    LOG_I("Creating buffer \'%s\' with URI \'%s\'", bufDesc.name.c_str(), bufDesc.uri.c_str());

    newBuffer->name = bufDesc.name;
    newBuffer->uri = bufDesc.uri;
    newBuffer->type = RenderResource::ResourceType::BUFFER;
    newBuffer->target = bufDesc.target;

    newBuffer->data.resize(bufDesc.bytesize);
    std::fill(newBuffer->data.begin(), newBuffer->data.end(), 0);
    if (bufDesc.p_data) {
        std::copy(bufDesc.p_data, bufDesc.p_data + bufDesc.bytesize, newBuffer->data.begin());
    }

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
    if (!hasBuffer(name)) {
        LOG_E("No buffer named \'%s\' is created", name.c_str());
        return;
    }

    auto& buffer = getBuffer(name);

    std::copy(data + byteoffset, data + byteoffset + bytesize, buffer.data.begin() + byteoffset);

    glBindBuffer(buffer.target, buffer.GL_id);
    glBufferSubData(buffer.target, byteoffset, bytesize, data);
    glBindBuffer(buffer.target, 0);
}

void ResourceManager::bindBufferShader(const std::string& name, const unsigned binding, const GeneralApp::Shader& shader) {
    if (!hasBuffer(name)) {
        LOG_E("No buffer named \'%s\' is created", name.c_str());
        return;
    }

    auto& buffer = getBuffer(name);

    glBindBufferRange(buffer.target, binding, buffer.GL_id, 0, buffer.data.size());

    unsigned int uboIndex = glGetUniformBlockIndex(shader.GL_id, name.c_str());
    glUniformBlockBinding(shader.GL_id, uboIndex, binding);
}


void ResourceManager::generateMipMaps(const std::string& texName) {
    if (!hasTexture(texName)) {
        LOG_E("No texture named \'%s\' is created", texName.c_str());
        return;
    }

    auto& texture = getTexture(texName);
    
    auto texType = texture.faces == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
    glBindTexture(texType, texture.GL_id);
    glGenerateMipmap(texType);
    glBindTexture(texType, 0);
}

void ResourceManager::generateMipMaps(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end()) {
        LOG_E("No resource with handle %d is created", handle.nativeHandle);
        return;
    }

    auto resource = it->second;
    if (resource->type != RenderResource::ResourceType::TEXTURE) {
        LOG_E("Can create mip maps for textures only");
        return;
    }

    Texture* texture = static_cast<Texture*>(resource);
    auto texType = texture->faces == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
    glBindTexture(texType, texture->GL_id);
    glGenerateMipmap(texType);
    glBindTexture(texType, 0);
}


GeneralApp::Shader& ResourceManager::createShader(const ShaderDesc& shaderDesc) {
    if (hasShader(shaderDesc.name, shaderDesc.uri))
        return getShader(shaderDesc.name);

    LOG_I("Creating shader \'%s\' with URI \'%s\'", shaderDesc.name.c_str(), shaderDesc.uri.c_str());

    GeneralApp::Shader* newShader = new GeneralApp::Shader(shaderDesc.vertFilename.c_str(), shaderDesc.fragFilename.c_str());

    newShader->name = shaderDesc.name;
    newShader->uri = shaderDesc.uri;
    newShader->type = RenderResource::ResourceType::SHADER;

    newShader->handle = createNewResourceHandle();
    shaders_[newShader->name] = newShader;
    allResources_[newShader->handle] = newShader;

    return *newShader;
}


Framebuffer& ResourceManager::createFramebuffer(const FramebufferDesc& framebufDesc) {
    if (hasFramebuffer(framebufDesc.name))
        return getFramebuffer(framebufDesc.name);

    LOG_I("Creating framebuffer \'%s\' with URI \'%s\'", framebufDesc.name.c_str(), framebufDesc.uri.c_str());

    Framebuffer* newFramebuffer = new Framebuffer();

    newFramebuffer->name = framebufDesc.name;
    newFramebuffer->uri = framebufDesc.uri;
    newFramebuffer->colorAttachmentsCount = framebufDesc.colorAttachmentsCount;
    newFramebuffer->type = RenderResource::ResourceType::FRAMEBUFFER;

    if (!framebufDesc.depthAttachment || framebufDesc.colorAttachmentsCount < 1) {
        LOG_E("Framebuffer must have depth attachment and at least 1 color attachment");
        delete newFramebuffer;
        return getFramebuffer(defaultFramebufferName);
    }

    unsigned commonWidth = framebufDesc.colorAttachments[0]->images[0]->width;
    unsigned commonHeigth = framebufDesc.colorAttachments[0]->images[0]->height;

    for (unsigned i = 0; i < framebufDesc.colorAttachmentsCount; ++i) {
        if (framebufDesc.colorAttachments[i]->faces != 1) {
            LOG_E("Framebuffer color attachments' view must be 2D Texture");
            delete newFramebuffer;
            return getFramebuffer(defaultFramebufferName);
        }

        if (framebufDesc.colorAttachments[i]->images[0]->width != commonWidth ||
            framebufDesc.colorAttachments[i]->images[0]->height != commonHeigth) {
            LOG_E("All framebuffer color attachments must have same dimensions");
            delete newFramebuffer;
            return getFramebuffer(defaultFramebufferName);
        }
    }

    glGenFramebuffers(1, &newFramebuffer->GL_id);
    glBindFramebuffer(GL_FRAMEBUFFER, newFramebuffer->GL_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufDesc.depthAttachment->GL_id, 0);
    newFramebuffer->depthAttachment = framebufDesc.depthAttachment;

    for (unsigned i = 0; i < framebufDesc.colorAttachmentsCount; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, framebufDesc.colorAttachments[i]->GL_id, 0);
        newFramebuffer->colorAttachments[i] = framebufDesc.colorAttachments[i];
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_E("Framebuffer \'%s\' is not complete", framebufDesc.name.c_str());
        delete newFramebuffer;
        bindFramebuffer(defaultFramebufferName);
        return getFramebuffer(defaultFramebufferName);
    }
    bindFramebuffer(defaultFramebufferName);

    newFramebuffer->handle = createNewResourceHandle();
    framebuffers_[newFramebuffer->name] = newFramebuffer;
    allResources_[newFramebuffer->handle] = newFramebuffer;

    return *newFramebuffer;
}


void ResourceManager::bindFramebuffer(const std::string& name) {
    if (!hasFramebuffer(name)) {
        LOG_E("No framebuffer named \'%s\' is created", name.c_str());
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[name]->GL_id);
}


void ResourceManager::resizeFramebuffer(const std::string& name, unsigned width, unsigned height) {
    if (!hasFramebuffer(name)) {
        LOG_E("No framebuffer named \'%s\' is created", name.c_str());
        return;
    }

    auto framebuffer = framebuffers_[name];

    auto depthAttachment = framebuffer->depthAttachment;
    glBindTexture(GL_TEXTURE_2D, depthAttachment->GL_id);
    glTexImage2D(GL_TEXTURE_2D, 0, depthAttachment->format, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    for (int i = 0; i < framebuffer->colorAttachmentsCount; ++i) {
        auto colorAttachment = framebuffer->colorAttachments[i];
        glBindTexture(GL_TEXTURE_2D, colorAttachment->GL_id);

        GLenum type = GL_UNSIGNED_BYTE;
        if (colorAttachment->images[0]->bits == 16) {
            type = GL_UNSIGNED_SHORT;
        }

        GLenum format = GL_RGBA;
        switch (colorAttachment->images[0]->components) {
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
            break;
        }

        glTexImage2D(GL_TEXTURE_2D, 0, colorAttachment->format, width, height, 0, format, type, NULL);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
}


void ResourceManager::deleteImage(const std::string& name) {
    if (auto it = images_.find(name); it != images_.end()) {
        LOG_I("Deleting image \'%s\'", name.c_str());
        allResources_.erase(it->second->handle);
        delete images_[name];
        images_.erase(it);
    }
}

void ResourceManager::deleteSampler(const std::string& name) {
    if (auto it = samplers_.find(name); it != samplers_.end()) {
        LOG_I("Deleting sampler \'%s\'", name.c_str());
        glDeleteSamplers(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete samplers_[name];
        samplers_.erase(it);
    }
}

void ResourceManager::deleteTexture(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end()) {
        LOG_I("Deleting texture \'%s\'", name.c_str());
        glDeleteTextures(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete textures_[name];
        textures_.erase(it);
    }
}

void ResourceManager::deleteMaterial(const std::string& name) {
    if (auto it = materials_.find(name); it != materials_.end()) {
        LOG_I("Deleting material \'%s\'", name.c_str());
        allResources_.erase(it->second->handle);
        delete materials_[name];
        materials_.erase(it);
    }
}

void ResourceManager::deleteBuffer(const std::string& name) {
    if (auto it = buffers_.find(name); it != buffers_.end()) {
        LOG_I("Deleting buffer \'%s\'", name.c_str());
        glDeleteBuffers(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete buffers_[name];
        buffers_.erase(it);
    }
}

void ResourceManager::deleteShader(const std::string& name) {
    if (auto it = shaders_.find(name); it != shaders_.end()) {
        LOG_I("Deleting shader \'%s\'", name.c_str());
        glDeleteProgram(it->second->GL_id);
        allResources_.erase(it->second->handle);
        delete shaders_[name];
        shaders_.erase(it);
    }
}

void ResourceManager::deleteFramebuffer(const std::string& name) {
    if (auto it = framebuffers_.find(name); it != framebuffers_.end()) {
        LOG_I("Deleting framebuffer \'%s\'", name.c_str());
        glDeleteFramebuffers(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete framebuffers_[name];
        framebuffers_.erase(it);
    }
}


Image& ResourceManager::getImage(const std::string& name) {
    if (!hasImage(name)) {
        LOG_E("No image named \'%s\' is created", name.c_str());
        return *images_[defaultImagesNames[Image::DEFAULT_IMAGE_BLACK]];
    }
    return *images_[name];
}

Sampler& ResourceManager::getSampler(const std::string& name) {
    if (!hasSampler(name)) {
        LOG_E("No sampler named \'%s\' is created", name.c_str());
        return *samplers_[defaultSamplersNames[Sampler::DEFAULT_SAMPLER_NEAREST_REPEAT]];
    }
    return *samplers_[name];
}

Texture& ResourceManager::getTexture(const std::string& name) {
    if (!hasTexture(name)) {
        LOG_E("No texture named \'%s\' is created", name.c_str());
        return *textures_[defaultTexturesNames[Texture::DEFAULT_TEXTURE_BLACK]];
    }
    return *textures_[name];
}

Material& ResourceManager::getMaterial(const std::string& name) {
    if (!hasMaterial(name)) {
        LOG_E("No material named \'%s\' is created", name.c_str());
        return *materials_[defaultMaterialName];
    }
    return *materials_[name];
}

Buffer& ResourceManager::getBuffer(const std::string& name) {
    if (!hasBuffer(name)) {
        LOG_E("No buffer named \'%s\' is created", name.c_str());

        // TODO:
        std::abort();
    }
    return *buffers_[name];
}

GeneralApp::Shader& ResourceManager::getShader(const std::string& name) {
    if (!hasShader(name)) {
        LOG_E("No shader named \'%s\' is created", name.c_str());

        // TODO:
        std::abort();
    }
    return *shaders_[name];
}

Framebuffer& ResourceManager::getFramebuffer(const std::string& name) {
    if (!hasFramebuffer(name)) {
        LOG_E("No framebuffer named \'%s\' is created", name.c_str());
        return *framebuffers_[defaultFramebufferName];
    }
    return *framebuffers_[name];
}

RenderResource* ResourceManager::getResource(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it != allResources_.end()) {
        return it->second;
    }
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

bool ResourceManager::hasFramebuffer(const std::string& name) {
    auto it = framebuffers_.find(name);
    return it != framebuffers_.end();
}

bool ResourceManager::hasResource(const ResourceHandle handle) {
    return allResources_.find(handle) != allResources_.end();
}


bool ResourceManager::hasImage(const std::string& name, const std::string& uri) {
    auto nameIt = images_.find(name);
    if (nameIt == images_.end()) {
        return false;
    }

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
    if (nameIt == samplers_.end()) {
        return false;
    }

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
    if (nameIt == textures_.end()) {
        return false;
    }

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
    if (nameIt == materials_.end()) {
        return false;
    }

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
    if (nameIt == buffers_.end()) {
        return false;
    }

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
    if (nameIt == shaders_.end()) {
        return false;
    }

    for (auto shdr : shaders_) {
        if (shdr.first != name)
            continue;

        if (shdr.second->uri == uri)
            return true;
    }
    return false;
}

bool ResourceManager::hasFramebuffer(const std::string& name, const std::string& uri) {
    auto nameIt = framebuffers_.find(name);
    if (nameIt == framebuffers_.end()) {
        return false;
    }

    for (auto fb : framebuffers_) {
        if (fb.first != name)
            continue;

        if (fb.second->uri == uri)
            return true;
    }
    return false;
}


ResourceManager::ResourceManager() {
    createDefaultImages();
    createDefaultSamplers();
    createDefaultTextures();
    createDefaultMaterials();
    createDefaultFramebuffer();
}

ResourceManager::~ResourceManager() {
    cleanUp();
}


void ResourceManager::cleanUp() {
    while (!images_.empty()) {
        auto it = images_.begin();
        deleteImage(it->first);
    }

    while (!samplers_.empty()) {
        auto it = samplers_.begin();
        deleteSampler(it->first);
    }

    while (!textures_.empty()) {
        auto it = textures_.begin();
        deleteTexture(it->first);
    }

    while (!materials_.empty()) {
        auto it = materials_.begin();
        deleteMaterial(it->first);
    }

    while (!buffers_.empty()) {
        auto it = buffers_.begin();
        deleteBuffer(it->first);
    }

    while (!shaders_.empty()) {
        auto it = shaders_.begin();
        deleteShader(it->first);
    }

    while (!framebuffers_.empty()) {
        auto it = framebuffers_.begin();
        deleteFramebuffer(it->first);
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

    defaultDesc.faces = 6;
    defaultDesc.factor = glm::vec4(1.0);
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK];

    for (unsigned i = 0; i < 6; ++i) {
        auto& blackImage = getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK]);
        defaultDesc.p_images[i] = &blackImage;
    }
    createTexture(defaultDesc);
}

void ResourceManager::createDefaultMaterials() {
    MaterialDesc defaultDesc;
    defaultDesc.name = defaultMaterialName;
    
    auto& defaultTextureWhite = getTexture(defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE]);

    for (int i = 0; i < Texture::DefaultTextures::COUNT; ++i) {
        defaultDesc.p_TexArray[i] = &defaultTextureWhite;
    }
    createMaterial(defaultDesc);
}

void ResourceManager::createDefaultFramebuffer() {
    Framebuffer* defaultFramebuffer = new Framebuffer();

    LOG_I("Creating framebuffer \'%s\' with URI \'\'", defaultFramebufferName.c_str());

    defaultFramebuffer->GL_id = 0;
    defaultFramebuffer->name = defaultFramebufferName;
    defaultFramebuffer->uri = "";
    defaultFramebuffer->type = RenderResource::ResourceType::FRAMEBUFFER;

    defaultFramebuffer->handle = createNewResourceHandle();
    framebuffers_[defaultFramebuffer->name] = defaultFramebuffer;
    allResources_[defaultFramebuffer->handle] = defaultFramebuffer;
}

}
