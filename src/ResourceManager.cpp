#include "../headers/ResourceManager.hpp"


namespace Resources {

ResourceManager* ResourceManager::instancePtr = nullptr;

Image& ResourceManager::createImage(const ImageDesc& imageDesc) {
    if (auto it = images_.find(imageDesc.name); it != images_.end())
        return *images_[imageDesc.name];

    std::cout << "Creating image \'" << imageDesc.name << "\'" << std::endl;

    Image* newImage = new Image();

    newImage->bits = imageDesc.bits;
    newImage->components = imageDesc.components;
    newImage->width = imageDesc.width;
    newImage->height = imageDesc.height;
    newImage->name = imageDesc.name;

    newImage->image.resize(imageDesc.p_data->size());
    std::copy(imageDesc.p_data->begin(), imageDesc.p_data->end(), newImage->image.begin());

    images_[newImage->name] = newImage;

    return *newImage;
}


Texture& ResourceManager::createTexture(const TextureDesc& textureDesc) {
    if (auto it = textures_.find(textureDesc.name); it != textures_.end())
        return *textures_[textureDesc.name];

    std::cout << "Creating texture \'" << textureDesc.name << "\'" << std::endl;

    Texture* newTexture = new Texture();

    newTexture->name = textureDesc.name;
    newTexture->factor = textureDesc.factor;

    textures_[newTexture->name] = newTexture;

    if (!textureDesc.p_image) {
        std::cout << "Image cannot be NULL !!!" << std::endl;
        return *newTexture;
    }

    newTexture->image = textureDesc.p_image;

    glGenTextures(1, &newTexture->GL_id);

    glBindTexture(GL_TEXTURE_2D, newTexture->GL_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLenum format = GL_RGBA;

    switch (newTexture->image->components) {
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
    if (newTexture->image->bits == 16)
        type = GL_UNSIGNED_SHORT;

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, newTexture->image->width, newTexture->image->height, 0, format, type, newTexture->image->image.data());

    return *newTexture;
}


Material& ResourceManager::createMaterial(const MaterialDesc& matDesc) {
    if (auto it = materials_.find(matDesc.name); it != materials_.end())
        return *materials_[matDesc.name];

    Material* newMaterial = new Material();

    std::cout << "Creating material \'" << matDesc.name << "\'" << std::endl;

    for(int i = 0; i < Material::TextureIdx::COUNT; ++i)
        newMaterial->textures[i] = matDesc.p_TexArray[i];
    
    newMaterial->name = matDesc.name;
    materials_[newMaterial->name] = newMaterial;

    return *newMaterial;
}


void ResourceManager::deleteImage(const std::string& name) {
    if (auto it = images_.find(name); it != images_.end()) {
        delete images_[name];
        images_.erase(it);
    }
}

void ResourceManager::deleteTexture(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end()) {
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


Image& ResourceManager::getImage(const std::string& name) {
    if (auto it = images_.find(name); it != images_.end())
        return *images_[name];

    auto emptyImage = new Image();
    images_[name] = emptyImage;
    return *emptyImage;
}

Texture& ResourceManager::getTexture(const std::string& name) {
    if (auto it = textures_.find(name); it != textures_.end())
        return *textures_[name];

    auto emptyTexture = new Texture();
    textures_[name] = emptyTexture;
    return *emptyTexture;
}

Material& ResourceManager::getMaterial(const std::string& name) {
    if (auto it = materials_.find(name); it != materials_.end())
        return *materials_[name];

    auto emptyMaterial = new Material();
    materials_[name] = emptyMaterial;
    return *emptyMaterial;
}


bool ResourceManager::hasImage(const std::string& name) {
    auto it = images_.find(name);
    return it != images_.end();
}

bool ResourceManager::hasTexture(const std::string& name) {
    auto it = textures_.find(name);
    return it != textures_.end();
}

bool ResourceManager::hasMaterial(const std::string& name) {
    auto it = materials_.find(name);
    return it != materials_.end();
}


ResourceManager::ResourceManager() {
    createDefaultImages();
    createDefaultTextures();
    createDefaultMaterials();
}

ResourceManager::~ResourceManager() {
    for (auto& im : images_) {
        delete im.second;
        images_.erase(im.first);
    }

    for (auto& tex : textures_) {
        delete tex.second;
        textures_.erase(tex.first);
    }

    for (auto& mat : materials_) {
        delete mat.second;
        materials_.erase(mat.first);
    }
}


void ResourceManager::createDefaultImages() {
    // Default white and black images share most of quantities
    ImageDesc defaultDesc;
    defaultDesc.width = 1;
    defaultDesc.height = 1;
    defaultDesc.components = 1;
    defaultDesc.bits = 8;

    std::vector<unsigned char> defaultValue;
    defaultValue.resize(1);
    defaultDesc.p_data = &defaultValue;

    defaultDesc.name = defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_WHITE];
    defaultValue[0] = 255;
    createImage(defaultDesc);

    defaultDesc.name = defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK];
    defaultValue[0] = 0;
    createImage(defaultDesc);
}

void ResourceManager::createDefaultTextures() {
    TextureDesc defaultDesc;
    defaultDesc.factor = glm::vec4(1.0);

    auto& defaultImageWhite = getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_WHITE]);
    defaultDesc.p_image = &defaultImageWhite;
    defaultDesc.name = defaultTexturesNames[Texture::DefaultTextures::DEFAULT_TEXTURE_WHITE];
    createTexture(defaultDesc);

    auto& defaultImageBlack = getImage(defaultImagesNames[Image::DefaultImages::DEFAULT_IMAGE_BLACK]);
    defaultDesc.p_image = &defaultImageBlack;
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
