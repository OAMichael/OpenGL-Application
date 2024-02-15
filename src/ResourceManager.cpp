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
    std::cout << "Creating image \'" << imageDesc.name << "\' with URI \'" << imageDesc.uri << "\'" << std::endl;
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
        std::cout << "Failed to load image: \'" << filename << "\'" << std::endl;
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

#ifdef DEBUG_MODEL
    std::cout << "Creating sampler \'" << samplerDesc.name << "\' with URI \'" << samplerDesc.uri << "\'" << std::endl;
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
    std::cout << "Creating texture \'" << textureDesc.name << "\' with URI \'" << textureDesc.uri << "\'" << std::endl;
#endif

    Texture* newTexture = new Texture();

    newTexture->name = textureDesc.name;
    newTexture->uri = textureDesc.uri;

    newTexture->type = RenderResource::ResourceType::TEXTURE;

    newTexture->factor = textureDesc.factor;
    newTexture->faces = textureDesc.faces;
    newTexture->format = textureDesc.format;

    if (!textureDesc.p_images[0]) {
        std::cout << "Image cannot be NULL !!!" << std::endl;
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
        std::cout << "Number of faces must be 1 or 6 !!!" << std::endl;
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

#ifdef DEBUG_MODEL
    std::cout << "Creating material \'" << matDesc.name << "\' with URI \'" << matDesc.uri << "\'" << std::endl;
#endif

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

#ifdef DEBUG_MODEL
    std::cout << "Creating buffer \'" << bufDesc.name << "\' with URI \'" << bufDesc.uri << "\'" << std::endl;
#endif

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
        std::cout << "No buffer named \'" << name << "\' is created" << std::endl;
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
        std::cout << "No buffer named \'" << name << "\' is created" << std::endl;
        return;
    }

    auto& buffer = getBuffer(name);

    glBindBufferRange(buffer.target, binding, buffer.GL_id, 0, buffer.data.size());

    unsigned int uboIndex = glGetUniformBlockIndex(shader.GL_id, name.c_str());
    glUniformBlockBinding(shader.GL_id, uboIndex, binding);
}


void ResourceManager::generateMipMaps(const std::string& texName) {
    if (!hasTexture(texName)) {
        std::cout << "No texture named \'" << texName << "\' is created" << std::endl;
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
        std::cout << "No resource with handle" << handle.nativeHandle << " is created" << std::endl;
        return;
    }

    auto resource = it->second;
    if (resource->type != RenderResource::ResourceType::TEXTURE) {
        std::cout << "Can create mip maps for textures only" << std::endl;
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

#ifdef DEBUG_MODEL
    std::cout << "Creating shader \'" << shaderDesc.name << "\' with URI \'" << shaderDesc.uri << "\'" << std::endl;
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


Framebuffer& ResourceManager::createFramebuffer(const FramebufferDesc& framebufDesc) {
    if (hasFramebuffer(framebufDesc.name))
        return getFramebuffer(framebufDesc.name);

#ifdef DEBUG_MODEL
    std::cout << "Creating framebuffer \'" << framebufDesc.name << "\' with URI \'" << framebufDesc.uri << "\'" << std::endl;
#endif

    Framebuffer* newFramebuffer = new Framebuffer();

    newFramebuffer->name = framebufDesc.name;
    newFramebuffer->uri = framebufDesc.uri;
    newFramebuffer->type = RenderResource::ResourceType::FRAMEBUFFER;

    if (!framebufDesc.depthAttachment || framebufDesc.colorAttachmentsCount < 1) {
        std::cout << "Framebuffer must have depth attachment and at least 1 color attachment" << std::endl;
        delete newFramebuffer;
        return getFramebuffer(defaultFramebufferName);
    }

    unsigned commonWidth = framebufDesc.colorAttachments[0]->images[0]->width;
    unsigned commonHeigth = framebufDesc.colorAttachments[0]->images[0]->height;

    for (unsigned i = 0; i < framebufDesc.colorAttachmentsCount; ++i) {
        if (framebufDesc.colorAttachments[i]->faces != 1) {
            std::cout << "Framebuffer color attachments' view must be 2D Texture" << std::endl;
            delete newFramebuffer;
            return getFramebuffer(defaultFramebufferName);
        }

        if (framebufDesc.colorAttachments[i]->images[0]->width != commonWidth ||
            framebufDesc.colorAttachments[i]->images[0]->height != commonHeigth) {
            std::cout << "All framebuffer color attachments must have same dimensions" << std::endl;
            delete newFramebuffer;
            return getFramebuffer(defaultFramebufferName);
        }
    }

    glGenFramebuffers(1, &newFramebuffer->GL_id);
    glBindFramebuffer(GL_FRAMEBUFFER, newFramebuffer->GL_id);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, framebufDesc.depthAttachment->GL_id, 0);
    for (unsigned i = 0; i < framebufDesc.colorAttachmentsCount; ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, framebufDesc.colorAttachments[i]->GL_id, 0);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Framebuffer \'" << framebufDesc.name << "\' is not complete" << std::endl;
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
        std::cout << "No framebuffer named \'" << name << "\' is created" << std::endl;
        return;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffers_[name]->GL_id);
}


void ResourceManager::deleteImage(const std::string& name) {
    if (auto it = images_.find(name); it != images_.end()) {
        allResources_.erase(it->second->handle);
        delete images_[name];
        images_.erase(it);
    }
}

void ResourceManager::deleteSampler(const std::string& name) {
    if (auto it = samplers_.find(name); it != samplers_.end()) {
        glDeleteSamplers(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete samplers_[name];
        samplers_.erase(it);
    }
}

void ResourceManager::deleteTexture(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end()) {
        glDeleteTextures(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete textures_[name];
        textures_.erase(it);
    }
}

void ResourceManager::deleteMaterial(const std::string& name) {
    if (auto it = materials_.find(name); it != materials_.end()) {
        allResources_.erase(it->second->handle);
        delete materials_[name];
        materials_.erase(it);
    }
}

void ResourceManager::deleteBuffer(const std::string& name) {
    if (auto it = buffers_.find(name); it != buffers_.end()) {
        glDeleteBuffers(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete buffers_[name];
        buffers_.erase(it);
    }
}

void ResourceManager::deleteShader(const std::string& name) {
    if (auto it = shaders_.find(name); it != shaders_.end()) {
        glDeleteProgram(it->second->GL_id);
        allResources_.erase(it->second->handle);
        delete shaders_[name];
        shaders_.erase(it);
    }
}

void ResourceManager::deleteFramebuffer(const std::string& name) {
    if (auto it = framebuffers_.find(name); it != framebuffers_.end()) {
        glDeleteFramebuffers(1, &(it->second->GL_id));
        allResources_.erase(it->second->handle);
        delete framebuffers_[name];
        framebuffers_.erase(it);
    }
}


Image& ResourceManager::getImage(const std::string& name) {
    if (!hasImage(name)) {
        std::cout << "No image named \'" << name << "\' is created" << std::endl;
        return *images_[defaultImagesNames[Image::DEFAULT_IMAGE_BLACK]];
    }
    return *images_[name];
}

Sampler& ResourceManager::getSampler(const std::string& name) {
    if (!hasSampler(name)) {
        std::cout << "No sampler named \'" << name << "\' is created" << std::endl;
        return *samplers_[defaultSamplersNames[Sampler::DEFAULT_SAMPLER_NEAREST_REPEAT]];
    }
    return *samplers_[name];
}

Texture& ResourceManager::getTexture(const std::string& name) {
    if (!hasTexture(name)) {
        std::cout << "No texture named \'" << name << "\' is created" << std::endl;
        return *textures_[defaultTexturesNames[Texture::DEFAULT_TEXTURE_BLACK]];
    }
    return *textures_[name];
}

Material& ResourceManager::getMaterial(const std::string& name) {
    if (!hasMaterial(name)) {
        std::cout << "No material named \'" << name << "\' is created" << std::endl;
        return *materials_[defaultMaterialName];
    }
    return *materials_[name];
}

Buffer& ResourceManager::getBuffer(const std::string& name) {
    if (!hasBuffer(name)) {
        std::cout << "No buffer named \'" << name << "\' is created" << std::endl;
        // TODO:
        std::abort();
    }
    return *buffers_[name];
}

GeneralApp::Shader& ResourceManager::getShader(const std::string& name) {
    if (!hasShader(name)) {
        std::cout << "No shader named \'" << name << "\' is created" << std::endl;
        // TODO:
        std::abort();
    }
    return *shaders_[name];
}

Framebuffer& ResourceManager::getFramebuffer(const std::string& name) {
    if (!hasFramebuffer(name)) {
        std::cout << "No framebuffer named \'" << name << "\' is created" << std::endl;
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
    for (auto it = images_.begin(); it != images_.end(); ++it) {
        deleteImage(it->first);
    }

    for (auto it = samplers_.begin(); it != samplers_.end(); ++it) {
        deleteSampler(it->first);
    }

    for (auto it = textures_.begin(); it != textures_.end(); ++it) {
        deleteTexture(it->first);
    }

    for (auto it = materials_.begin(); it != materials_.end(); ++it) {
        deleteMaterial(it->first);
    }

    for (auto it = buffers_.begin(); it != buffers_.end(); ++it) {
        deleteBuffer(it->first);
    }

    for (auto it = shaders_.begin(); it != shaders_.end(); ++it) {
        deleteShader(it->first);
    }

    for (auto it = framebuffers_.begin(); it != framebuffers_.end(); ++it) {
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

#ifdef DEBUG_MODEL
    std::cout << "Creating framebuffer \'" << defaultFramebufferName << "\' with URI \'" << "" << "\'" << std::endl;
#endif

    defaultFramebuffer->GL_id = 0;
    defaultFramebuffer->name = defaultFramebufferName;
    defaultFramebuffer->uri = "";
    defaultFramebuffer->type = RenderResource::ResourceType::FRAMEBUFFER;

    defaultFramebuffer->handle = createNewResourceHandle();
    framebuffers_[defaultFramebuffer->name] = defaultFramebuffer;
    allResources_[defaultFramebuffer->handle] = defaultFramebuffer;
}

}
