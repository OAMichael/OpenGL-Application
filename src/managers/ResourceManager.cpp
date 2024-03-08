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

Image& ResourceManager::createImage(const char* filename, bool isHdr) {
    if (hasImage(filename))
        return getImage(filename);

    int width, height, nrChannels;
    unsigned char* data = nullptr;
    if (isHdr) {
        data = reinterpret_cast<unsigned char*>(stbi_loadf(filename, &width, &height, &nrChannels, 0));
    }
    else {
        stbi_hdr_to_ldr_gamma(1.0f);
        data = stbi_load(filename, &width, &height, &nrChannels, 0);
    }

    if (data) {
        ImageDesc imageDesc;
        imageDesc.name = filename;
        imageDesc.uri = filename;
        imageDesc.width = width;
        imageDesc.height = height;
        imageDesc.components = nrChannels;
        imageDesc.p_data = data;

        if (isHdr) {
            imageDesc.bits = 8 * sizeof(float);
        }
        else {
            imageDesc.bits = 8;                     // stb_image automatically converts
        }

        auto& im = createImage(imageDesc);

        stbi_image_free(data);
        return im;
    }
    else {
        LOG_E("Failed to load image: \'%s\'", filename);
        stbi_image_free(data);
        return getImage(Image::DefaultImages::DEFAULT_IMAGE_BLACK);
    }
}

Image& ResourceManager::createImage(const std::string& filename, bool isHdr) {
    return createImage(filename.c_str(), isHdr);
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
        return getTexture(Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK);
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
        return getTexture(Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    newTexture->sampler = textureDesc.p_sampler;
    if (!newTexture->sampler) {
        newTexture->sampler = &getSampler(Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_REPEAT);
    }

    bool isFloat = false;
    bool isDepth = false;

    if (textureDesc.format == GL_RGB16F || textureDesc.format == GL_RGBA16F ||
        textureDesc.format == GL_RGB32F || textureDesc.format == GL_RGBA32F) {

        isFloat = true;
    }
    else if (textureDesc.format == GL_DEPTH_COMPONENT || textureDesc.format == GL_DEPTH_COMPONENT16
        || textureDesc.format == GL_DEPTH_COMPONENT24 || textureDesc.format == GL_DEPTH_COMPONENT32
        || textureDesc.format == GL_DEPTH_COMPONENT32F) {

        isDepth = true;
    }

    if (newTexture->faces == 1) {
        // Handle depth texture
        if (isDepth) {
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
            if (isFloat) {
                type = GL_FLOAT;
            } else if (newTexture->images[0]->bits == 16) {
                type = GL_UNSIGNED_SHORT;
            }

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
            if (isFloat) {
                type = GL_FLOAT;
            }
            else if (newTexture->images[0]->bits == 16) {
                type = GL_UNSIGNED_SHORT;
            }

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


void ResourceManager::bindTexture(const std::string& name, const unsigned texUnit) {
    auto& texture = getTexture(name);
    GLenum target = texture.faces == 6 ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(target, texture.GL_id);
    glBindSampler(texUnit, texture.sampler->GL_id);
}

void ResourceManager::bindTexture(const ResourceHandle handle, const unsigned texUnit) {
    auto& texture = getTexture(handle);
    GLenum target = texture.faces == 6 ? GL_TEXTURE_CUBE_MAP : GL_TEXTURE_2D;
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(target, texture.GL_id);
    glBindSampler(texUnit, texture.sampler->GL_id);
}

void ResourceManager::unbindTexture(const GLenum target, const unsigned texUnit) {
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(target, 0);
    glBindSampler(texUnit, 0);
}


Material& ResourceManager::createMaterial(const MaterialDesc& matDesc) {
    if (hasMaterial(matDesc.name, matDesc.uri))
        return getMaterial(matDesc.name);

    Material* newMaterial = new Material();

    LOG_I("Creating material \'%s\' with URI \'%s\'", matDesc.name.c_str(), matDesc.uri.c_str());

    for (int i = 0; i < Material::TextureIdx::IDX_COUNT; ++i) {
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

void ResourceManager::bindBufferShader(const std::string& name, const unsigned binding, const Shader& shader) {
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
    auto& texture = getTexture(texName);
    auto texType = texture.faces == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
    glBindTexture(texType, texture.GL_id);
    glGenerateMipmap(texType);
    glBindTexture(texType, 0);
}

void ResourceManager::generateMipMaps(const ResourceHandle handle) {
    Texture& texture = getTexture(handle);
    auto texType = texture.faces == 1 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
    glBindTexture(texType, texture.GL_id);
    glGenerateMipmap(texType);
    glBindTexture(texType, 0);
}


Shader& ResourceManager::createShader(const ShaderDesc& shaderDesc) {
    if (hasShader(shaderDesc.name, shaderDesc.uri))
        return getShader(shaderDesc.name);

    LOG_I("Creating shader \'%s\' with URI \'%s\'", shaderDesc.name.c_str(), shaderDesc.uri.c_str());

    Shader* newShader = new Shader(shaderDesc.vertFilename.c_str(), shaderDesc.fragFilename.c_str());

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
        return getFramebuffer(Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER);
    }

    unsigned commonWidth = framebufDesc.colorAttachments[0]->images[0]->width;
    unsigned commonHeigth = framebufDesc.colorAttachments[0]->images[0]->height;

    for (unsigned i = 0; i < framebufDesc.colorAttachmentsCount; ++i) {
        if (framebufDesc.colorAttachments[i]->faces != 1) {
            LOG_E("Framebuffer color attachments' view must be 2D Texture");
            delete newFramebuffer;
            return getFramebuffer(Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER);
        }

        if (framebufDesc.colorAttachments[i]->images[0]->width != commonWidth ||
            framebufDesc.colorAttachments[i]->images[0]->height != commonHeigth) {
            LOG_E("All framebuffer color attachments must have same dimensions");
            delete newFramebuffer;
            return getFramebuffer(Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER);
        }
    }

    glGenFramebuffers(1, &newFramebuffer->GL_id);
    glBindFramebuffer(GL_FRAMEBUFFER, newFramebuffer->GL_id);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, framebufDesc.depthAttachment->GL_id, 0);
    newFramebuffer->depthAttachment = framebufDesc.depthAttachment;

    std::vector<unsigned> attachments;
    for (unsigned i = 0; i < framebufDesc.colorAttachmentsCount; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, framebufDesc.colorAttachments[i]->GL_id, 0);
        newFramebuffer->colorAttachments[i] = framebufDesc.colorAttachments[i];
        attachments.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    glDrawBuffers(attachments.size(), attachments.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        LOG_E("Framebuffer \'%s\' is not complete", framebufDesc.name.c_str());
        delete newFramebuffer;
        bindFramebuffer(defaultFramebuffers_[Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER]->handle);
        return getFramebuffer(Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER);
    }
    bindFramebuffer(defaultFramebuffers_[Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER]->handle);

    newFramebuffer->handle = createNewResourceHandle();
    framebuffers_[newFramebuffer->name] = newFramebuffer;
    allResources_[newFramebuffer->handle] = newFramebuffer;

    if (!framebufDesc.dependency.empty()) {
        if (hasFramebuffer(framebufDesc.dependency)) {
            auto& dep = getFramebuffer(framebufDesc.dependency);
            dep.dependants.push_back(newFramebuffer);
        }
        else {
            LOG_W("Dependency framebuffer \'%s\' does not exist", framebufDesc.dependency.c_str());
        }
    }

    return *newFramebuffer;
}


void ResourceManager::bindFramebuffer(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::FRAMEBUFFER) {
        LOG_E("No framebuffer with handle %ld is created", handle.nativeHandle);
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, static_cast<Framebuffer*>(it->second)->GL_id);
}

void ResourceManager::bindFramebuffer(const std::string& name) {
    if (!hasFramebuffer(name)) {
        LOG_E("No framebuffer named \'%s\' is created", name.c_str());
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[name]->GL_id);
}


void ResourceManager::resizeFramebuffer(const ResourceHandle handle, unsigned width, unsigned height) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::FRAMEBUFFER) {
        LOG_E("No framebuffer with handle %ld is created", handle.nativeHandle);
        return;
    }

    auto framebuffer = static_cast<Framebuffer*>(it->second);

    // Do all this stuff only for user created framebuffers
    if (framebuffer->GL_id != 0) {
        auto depthAttachment = framebuffer->depthAttachment;
        glBindTexture(GL_TEXTURE_2D, depthAttachment->GL_id);
        glTexImage2D(GL_TEXTURE_2D, 0, depthAttachment->format, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

        for (int i = 0; i < framebuffer->colorAttachmentsCount; ++i) {
            auto colorAttachment = framebuffer->colorAttachments[i];
            glBindTexture(GL_TEXTURE_2D, colorAttachment->GL_id);

            bool isFloat = false;
            if (colorAttachment->format == GL_RGB16F || colorAttachment->format == GL_RGBA16F ||
                colorAttachment->format == GL_RGB32F || colorAttachment->format == GL_RGBA32F) {

                isFloat = true;
            }

            GLenum type = GL_UNSIGNED_BYTE;
            if (isFloat) {
                type = GL_FLOAT;
            }
            else if (colorAttachment->images[0]->bits == 16) {
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

    for (auto dep : framebuffer->dependants) {
        resizeFramebuffer(dep->handle, width, height);
    }
}

void ResourceManager::resizeFramebuffer(const std::string& name, unsigned width, unsigned height) {
    if (!hasFramebuffer(name)) {
        LOG_E("No framebuffer named \'%s\' is created", name.c_str());
        return;
    }
    resizeFramebuffer(getFramebuffer(name).handle, width, height);
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
        return getImage(Image::DefaultImages::DEFAULT_IMAGE_BLACK);
    }
    return *images_[name];
}

Sampler& ResourceManager::getSampler(const std::string& name) {
    if (!hasSampler(name)) {
        LOG_E("No sampler named \'%s\' is created", name.c_str());
        return getSampler(Sampler::DEFAULT_SAMPLER_NEAREST_REPEAT);
    }
    return *samplers_[name];
}

Texture& ResourceManager::getTexture(const std::string& name) {
    if (!hasTexture(name)) {
        LOG_E("No texture named \'%s\' is created", name.c_str());
        return getTexture(Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK);
    }
    return *textures_[name];
}

Material& ResourceManager::getMaterial(const std::string& name) {
    if (!hasMaterial(name)) {
        LOG_E("No material named \'%s\' is created", name.c_str());
        return getMaterial(Material::DefaultMaterials::DEFAULT_MATERIAL);
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

Shader& ResourceManager::getShader(const std::string& name) {
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
        return getFramebuffer(Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER);
    }
    return *framebuffers_[name];
}


Image& ResourceManager::getImage(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::IMAGE) {
        LOG_E("No image with handle %ld is created", handle.nativeHandle);
        return getImage(Image::DefaultImages::DEFAULT_IMAGE_BLACK);
    }
    return *static_cast<Image*>(it->second);
}

Sampler& ResourceManager::getSampler(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::SAMPLER) {
        LOG_E("No sampler with handle %ld is created", handle.nativeHandle);
        return getSampler(Sampler::DEFAULT_SAMPLER_NEAREST_REPEAT);
    }
    return *static_cast<Sampler*>(it->second);
}

Texture& ResourceManager::getTexture(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::TEXTURE) {
        LOG_E("No texture with handle %ld is created", handle.nativeHandle);
        return getTexture(Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK);
    }
    return *static_cast<Texture*>(it->second);
}

Material& ResourceManager::getMaterial(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::MATERIAL) {
        LOG_E("No material with handle %ld is created", handle.nativeHandle);
        return getMaterial(Material::DefaultMaterials::DEFAULT_MATERIAL);
    }
    return *static_cast<Material*>(it->second);
}

Buffer& ResourceManager::getBuffer(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::BUFFER) {
        LOG_E("No buffer with handle %ld is created", handle.nativeHandle);

        // TODO:
        std::abort();
    }
    return *static_cast<Buffer*>(it->second);
}

Shader& ResourceManager::getShader(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::SHADER) {
        LOG_E("No shader with handle %ld is created", handle.nativeHandle);

        // TODO:
        std::abort();
    }
    return *static_cast<Shader*>(it->second);
}

Framebuffer& ResourceManager::getFramebuffer(const ResourceHandle handle) {
    auto it = allResources_.find(handle);
    if (it == allResources_.end() || it->second->type != RenderResource::ResourceType::FRAMEBUFFER) {
        LOG_E("No framebuffer with handle %ld is created", handle.nativeHandle);
        return getFramebuffer(Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER);
    }
    return *static_cast<Framebuffer*>(it->second);
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
    defaultImages_[Image::DefaultImages::DEFAULT_IMAGE_WHITE] = &createImage(defaultDesc);

    defaultDesc.name = defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK];
    defaultValue[0] = 0;
    defaultValue[1] = 0;
    defaultValue[2] = 0;
    defaultValue[3] = 0;
    defaultImages_[Image::DefaultImages::DEFAULT_IMAGE_BLACK] = &createImage(defaultDesc);
}

void ResourceManager::createDefaultSamplers() {
    SamplerDesc defaultDesc;

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_CLAMP];
    defaultDesc.minFilter = Sampler::NEAREST;
    defaultDesc.magFilter = Sampler::NEAREST;
    defaultDesc.wrapS = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapT = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapR = Sampler::CLAMP_TO_EDGE;
    defaultSamplers_[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_CLAMP] = &createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_REPEAT];
    defaultDesc.minFilter = Sampler::NEAREST;
    defaultDesc.magFilter = Sampler::NEAREST;
    defaultDesc.wrapS = Sampler::REPEAT;
    defaultDesc.wrapT = Sampler::REPEAT;
    defaultDesc.wrapR = Sampler::REPEAT;
    defaultSamplers_[Sampler::DefaultSamplers::DEFAULT_SAMPLER_NEAREST_REPEAT] = &createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_REPEAT];
    defaultDesc.minFilter = Sampler::LINEAR;
    defaultDesc.magFilter = Sampler::LINEAR;
    defaultDesc.wrapS = Sampler::REPEAT;
    defaultDesc.wrapT = Sampler::REPEAT;
    defaultDesc.wrapR = Sampler::REPEAT;
    defaultSamplers_[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_REPEAT] = &createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_CLAMP];
    defaultDesc.minFilter = Sampler::LINEAR;
    defaultDesc.magFilter = Sampler::LINEAR;
    defaultDesc.wrapS = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapT = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapR = Sampler::CLAMP_TO_EDGE;
    defaultSamplers_[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_CLAMP] = &createSampler(defaultDesc);

    defaultDesc.name = defaultSamplersNames[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_MIPMAP_LINEAR_CLAMP];
    defaultDesc.minFilter = Sampler::LINEAR_MIPMAP_LINEAR;
    defaultDesc.magFilter = Sampler::LINEAR;
    defaultDesc.wrapS = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapT = Sampler::CLAMP_TO_EDGE;
    defaultDesc.wrapR = Sampler::CLAMP_TO_EDGE;
    defaultSamplers_[Sampler::DefaultSamplers::DEFAULT_SAMPLER_LINEAR_MIPMAP_LINEAR_CLAMP] = &createSampler(defaultDesc);
}

void ResourceManager::createDefaultTextures() {
    TextureDesc defaultDesc;
    defaultDesc.factor = glm::vec4(1.0);

    auto& defaultImageWhite = getImage(Image::DefaultImages::DEFAULT_IMAGE_WHITE);
    defaultDesc.p_images[0] = &defaultImageWhite;
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE];
    defaultTextures_[Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE] = &createTexture(defaultDesc);

    auto& defaultImageBlack = getImage(Image::DefaultImages::DEFAULT_IMAGE_BLACK);
    defaultDesc.p_images[0] = &defaultImageBlack;
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK];
    defaultTextures_[Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK] = &createTexture(defaultDesc);

    defaultDesc.faces = 6;
    defaultDesc.factor = glm::vec4(1.0);
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK];

    for (unsigned i = 0; i < 6; ++i) {
        auto& blackImage = getImage(Image::DefaultImages::DEFAULT_IMAGE_BLACK);
        defaultDesc.p_images[i] = &blackImage;
    }
    defaultTextures_[Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK] = &createTexture(defaultDesc);
}

void ResourceManager::createDefaultMaterials() {
    MaterialDesc defaultDesc;
    defaultDesc.name = defaultMaterialNames[Material::DefaultMaterials::DEFAULT_MATERIAL];
    
    auto& defaultTextureWhite = getTexture(Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE);

    for (int i = 0; i < Texture::DefaultTextures::COUNT; ++i) {
        defaultDesc.p_TexArray[i] = &defaultTextureWhite;
    }
    defaultMaterials_[Material::DefaultMaterials::DEFAULT_MATERIAL] = &createMaterial(defaultDesc);
}

void ResourceManager::createDefaultFramebuffer() {
    Framebuffer* defaultFramebuffer = new Framebuffer();

    LOG_I("Creating framebuffer \'%s\' with URI \'\'", defaultFramebufferNames[Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER].c_str());

    defaultFramebuffer->GL_id = 0;
    defaultFramebuffer->name = defaultFramebufferNames[Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER];
    defaultFramebuffer->uri = "";
    defaultFramebuffer->type = RenderResource::ResourceType::FRAMEBUFFER;

    defaultFramebuffer->handle = createNewResourceHandle();
    framebuffers_[defaultFramebuffer->name] = defaultFramebuffer;
    allResources_[defaultFramebuffer->handle] = defaultFramebuffer;
    defaultFramebuffers_[Framebuffer::DefaultFramebuffers::DEFAULT_FRAMEBUFFER] = defaultFramebuffer;
}

}
