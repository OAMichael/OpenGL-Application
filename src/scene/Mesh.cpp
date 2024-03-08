#include "Mesh.hpp"
#include "ResourceManager.hpp"
#include "SceneManager.hpp"
#include "GLTFLoader.hpp"

void Geometry::Mesh::draw(Resources::Shader& shader)
{
    if (!meshPtr_ || !modelPtr_)
        return;

    auto& modelRef = *modelPtr_;
    auto& meshRef = *meshPtr_;

    auto resourceManager = Resources::ResourceManager::getInstance();
    auto sceneManager = SceneResources::SceneManager::getInstance();

    const int materialTextures[Resources::Material::TextureIdx::IDX_COUNT] = {
        Resources::Material::BASE_COLOR,
        Resources::Material::METALLIC_ROUGHNESS,
        Resources::Material::EMISSIVE,
        Resources::Material::NORMAL,
        Resources::Material::OCCLUSION
    };

    shader.use();
    shader.setIntArray("uMaterialTextures", materialTextures, Resources::Material::TextureIdx::IDX_COUNT);
    shader.setInt("uIrradianceMap", Resources::Material::TextureIdx::IDX_COUNT);
    shader.setInt("uPrefilterMap", Resources::Material::TextureIdx::IDX_COUNT + 1);
    shader.setInt("uBrdfLUT", Resources::Material::TextureIdx::IDX_COUNT + 2);
    shader.setUint("uEnvironmentType", (uint32_t)sceneManager->getEnvironmentType());

    for (size_t i = 0; i < meshRef.primitives.size(); ++i) {
        glBindVertexArray(VAOs_[i]);

        const tinygltf::Primitive& primitive = meshRef.primitives[i];
        const tinygltf::Accessor& indexAccessor = modelRef.accessors[primitive.indices];
        const tinygltf::BufferView& bufView = modelRef.bufferViews[indexAccessor.bufferView];

        auto& primitiveMaterial = resourceManager->getMaterial(primitiveMaterial_[i]);

        glm::vec4 materialTexturesFactors[Resources::Material::TextureIdx::IDX_COUNT];
        for (int i = 0; i < Resources::Material::TextureIdx::IDX_COUNT; ++i) {
            auto tex = primitiveMaterial.textures[i];
            materialTexturesFactors[i] = tex->factor;
            resourceManager->bindTexture(tex->handle, i);
        }

        auto envType = sceneManager->getEnvironmentType();
        Resources::ResourceHandle irradianceHandle;
        Resources::ResourceHandle prefilterHandle;
        Resources::ResourceHandle brdfLUTHandle;

        if (envType == SceneResources::SceneManager::EnvironmentType::SKYBOX) {
            irradianceHandle = sceneManager->getIrradianceMapSkyboxTextureHandle();
            prefilterHandle = sceneManager->getPrefilterHDRMapSkyboxTextureHandle();
            brdfLUTHandle = sceneManager->getBRDFLUTSkyboxTextureHandle();
        }
        else if (envType == SceneResources::SceneManager::EnvironmentType::EQUIRECTANGULAR) {
            irradianceHandle = sceneManager->getIrradianceMapEquirectTextureHandle();
            prefilterHandle = sceneManager->getPrefilterHDRMapEquirectTextureHandle();
            brdfLUTHandle = sceneManager->getBRDFLUTEquirectTextureHandle();
        }

        if (irradianceHandle.isValid() && prefilterHandle.isValid() && brdfLUTHandle.isValid()) {
            resourceManager->bindTexture(irradianceHandle, Resources::Material::TextureIdx::IDX_COUNT);
            resourceManager->bindTexture(prefilterHandle, Resources::Material::TextureIdx::IDX_COUNT + 1);
            resourceManager->bindTexture(brdfLUTHandle, Resources::Material::TextureIdx::IDX_COUNT + 2);
        }
        else {
            resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK], Resources::Material::TextureIdx::IDX_COUNT);
            resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_CUBEMAP_BLACK], Resources::Material::TextureIdx::IDX_COUNT + 1);
            resourceManager->bindTexture(Resources::defaultTexturesNames[Resources::Texture::DefaultTextures::DEFAULT_TEXTURE_BLACK], Resources::Material::TextureIdx::IDX_COUNT + 2);
        }

        shader.setVec4Array("uMaterialTexturesFactors", &materialTexturesFactors[0][0], Resources::Material::TextureIdx::IDX_COUNT);
        shader.setUint("uMaterialFlags", primitiveMaterial.materialFlags);

        glBindBuffer(bufView.target, VBOs_.at(indexAccessor.bufferView));
        if (primitive.indices >= 0) {
            glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType, (void*)BUFFER_OFFSET(indexAccessor.byteOffset));
        }
        else {
            const auto accessorIdx = std::begin(primitive.attributes)->second;
            const auto& accessor = modelRef.accessors[accessorIdx];
            glDrawArrays(primitive.mode, 0, accessor.count);
        }
    }
    glBindVertexArray(0);
}

void Geometry::Mesh::init()
{
    if (!meshPtr_ || !modelPtr_)
        return;

    auto& modelRef = *modelPtr_;
    auto& meshRef = *meshPtr_;

    name = meshRef.name;

    for (size_t i = 0; i < modelRef.bufferViews.size(); ++i) {
        const tinygltf::BufferView& bufferView = modelRef.bufferViews[i];
        if (bufferView.target == 0)
            continue;

        const tinygltf::Buffer& buffer = modelRef.buffers[bufferView.buffer];

        unsigned int vbo;
        glGenBuffers(1, &vbo);
        VBOs_[i] = vbo;

        glBindBuffer(bufferView.target, vbo);
        glBufferData(bufferView.target, bufferView.byteLength, &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
    }


    for (size_t i = 0; i < meshRef.primitives.size(); ++i) {
        unsigned int vao;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        VAOs_[i] = vao;

        const tinygltf::Primitive& primitive = meshRef.primitives[i];
        const tinygltf::Accessor& indexAccessor = modelRef.accessors[primitive.indices];

        for (auto& attrib : primitive.attributes) {
            tinygltf::Accessor accessor = modelRef.accessors[attrib.second];
            const tinygltf::BufferView& bufView = modelRef.bufferViews[accessor.bufferView];
            int byteStride = accessor.ByteStride(bufView);
            glBindBuffer(bufView.target, VBOs_[accessor.bufferView]);

            int size = 1;
            if (accessor.type != TINYGLTF_TYPE_SCALAR)
                size = accessor.type;


            int vaa = -1;
            if (!attrib.first.compare("POSITION"))
                vaa = 0;

            if (!attrib.first.compare("NORMAL"))
                vaa = 1;

            if (!attrib.first.compare("TEXCOORD_0"))
                vaa = 2;

            if (vaa > -1) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType,
                    accessor.normalized ? GL_TRUE : GL_FALSE,
                    byteStride, (void*)BUFFER_OFFSET(accessor.byteOffset));
            }
        }

        auto resourceManager = Resources::ResourceManager::getInstance();
        auto gltfLoader = GLTF::GLTFLoader::getInstance();


        std::string baseDir = "/";
        for (auto it : gltfLoader->loadedModels) {
            if (modelPtr_ == &(it.second->getModelRef()))
                baseDir = it.second->getFilename();
        }

        Resources::Image& defaultWhiteImage = resourceManager->getImage(Resources::Image::DEFAULT_IMAGE_WHITE);
        Resources::Texture& defaultWhiteTexture = resourceManager->getTexture(Resources::Texture::DEFAULT_TEXTURE_WHITE);
        
        Resources::MaterialDesc matDesc;
        matDesc.name = modelRef.materials[primitive.material].name;
        for (uint32_t i = 0; i < Resources::Texture::COUNT; ++i)
            matDesc.p_TexArray[i] = &defaultWhiteTexture;


        auto baseColorTexIdx = modelRef.materials[primitive.material].pbrMetallicRoughness.baseColorTexture.index;
        auto baseColorFactor = modelRef.materials[primitive.material].pbrMetallicRoughness.baseColorFactor;
        {
            glm::vec4 factor = glm::vec4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);
            
            Resources::TextureDesc texDesc;
            texDesc.name = "basecolor_" + modelRef.materials[primitive.material].name;
            texDesc.uri = "";
            texDesc.faces = 1;
            texDesc.p_images[0] = &defaultWhiteImage;
            texDesc.factor = factor;
            texDesc.format = GL_SRGB8;
            texDesc.p_sampler = nullptr;

            if (baseColorTexIdx > -1 && baseColorTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[baseColorTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name, 
                        image.uri,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        GL_SRGB8,
                        image.image.data()
                    };
                    auto& baseColorImage = resourceManager->createImage(imDesc);
                    texDesc.name = baseColorImage.name;
                    texDesc.uri = baseDir + "/textures/" + std::to_string(baseColorTexIdx);
                    texDesc.p_images[0] = &baseColorImage;
                }
            }
            auto& baseColorTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::BASE_COLOR] = &baseColorTexture;
        }

        auto metallicRoughnessTexIdx = modelRef.materials[primitive.material].pbrMetallicRoughness.metallicRoughnessTexture.index;
        double metallicFactor = modelRef.materials[primitive.material].pbrMetallicRoughness.metallicFactor;
        double roughnessFactor = modelRef.materials[primitive.material].pbrMetallicRoughness.roughnessFactor;
        {
            glm::vec4 factor = glm::vec4(1.0, roughnessFactor, metallicFactor, 1.0);

            Resources::TextureDesc texDesc;
            texDesc.name = "metallicRoughness_" + modelRef.materials[primitive.material].name;
            texDesc.uri = "";
            texDesc.faces = 1;
            texDesc.p_images[0] = &defaultWhiteImage;
            texDesc.factor = factor;
            texDesc.p_sampler = nullptr;

            if (metallicRoughnessTexIdx > -1 && metallicRoughnessTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[metallicRoughnessTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.uri,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        GL_RGBA,
                        image.image.data()
                    };
                    auto& metallicRoughnessImage = resourceManager->createImage(imDesc);
                    texDesc.name = metallicRoughnessImage.name;
                    texDesc.uri = baseDir + "/textures/" + std::to_string(metallicRoughnessTexIdx);
                    texDesc.p_images[0] = &metallicRoughnessImage;
                }
            }
            auto& metallicRoughnessTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::METALLIC_ROUGHNESS] = &metallicRoughnessTexture;
        }

        auto emissiveTexIdx = modelRef.materials[primitive.material].emissiveTexture.index;
        auto emissiveFactor = modelRef.materials[primitive.material].emissiveFactor;
        {
            glm::vec4 factor = glm::vec4(emissiveFactor[0], emissiveFactor[1], emissiveFactor[2], 1.0);

            Resources::TextureDesc texDesc;
            texDesc.name = "emissive_" + modelRef.materials[primitive.material].name;
            texDesc.uri = "";
            texDesc.faces = 1;
            texDesc.p_images[0] = &defaultWhiteImage;
            texDesc.factor = factor;
            texDesc.p_sampler = nullptr;

            if (emissiveTexIdx > -1 && emissiveTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[emissiveTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.uri,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        GL_RGBA,
                        image.image.data()
                    };
                    auto& emissiveImage = resourceManager->createImage(imDesc);
                    texDesc.name = emissiveImage.name;
                    texDesc.uri = baseDir + "/textures/" + std::to_string(emissiveTexIdx);
                    texDesc.p_images[0] = &emissiveImage;
                }
            }
            auto& emissiveTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::EMISSIVE] = &emissiveTexture;
        }

        auto normalTexIdx = modelRef.materials[primitive.material].normalTexture.index;
        {
            glm::vec4 factor = glm::vec4(1.0);

            Resources::TextureDesc texDesc;
            texDesc.name = "normal_" + modelRef.materials[primitive.material].name;
            texDesc.uri = "";
            texDesc.faces = 1;
            texDesc.p_images[0] = &defaultWhiteImage;
            texDesc.factor = factor;
            texDesc.p_sampler = nullptr;

            if (normalTexIdx > -1 && normalTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[normalTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.uri,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        GL_RGBA,
                        image.image.data()
                    };
                    auto& normalImage = resourceManager->createImage(imDesc);
                    texDesc.name = normalImage.name;
                    texDesc.uri = baseDir + "/textures/" + std::to_string(normalTexIdx);
                    texDesc.p_images[0] = &normalImage;
                }
            }
            auto& normalTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::NORMAL] = &normalTexture;
        }

        auto occlusionTexIdx = modelRef.materials[primitive.material].occlusionTexture.index;
        {
            glm::vec4 factor = glm::vec4(1.0);

            Resources::TextureDesc texDesc;
            texDesc.name = "occlusion_" + modelRef.materials[primitive.material].name;
            texDesc.uri = "";
            texDesc.faces = 1;
            texDesc.p_images[0] = &defaultWhiteImage;
            texDesc.factor = factor;
            texDesc.p_sampler = nullptr;

            if (occlusionTexIdx > -1 && occlusionTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[occlusionTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.uri,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        GL_RGBA,
                        image.image.data()
                    };
                    auto& occlusionImage = resourceManager->createImage(imDesc);
                    texDesc.name = occlusionImage.name;
                    texDesc.uri = baseDir + "/textures/" + std::to_string(occlusionTexIdx);
                    texDesc.p_images[0] = &occlusionImage;
                }
            }
            auto& occlusionTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::OCCLUSION] = &occlusionTexture;
        }

        matDesc.uri = baseDir + "/materials/" + std::to_string(primitive.material);
        auto& newMat = resourceManager->createMaterial(matDesc);
        primitiveMaterial_[i] = newMat.handle;

        if (normalTexIdx > -1 && normalTexIdx < modelRef.textures.size()) {
            newMat.materialFlags |= Resources::Material::MATERIAL_FLAG_NORMAL_MAP_BIT;
        }
    }
    glBindVertexArray(0);
}
