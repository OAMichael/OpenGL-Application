#include "../headers/Mesh.hpp"
#include "../headers/ResourceManager.hpp"


void Geometry::Mesh::draw(GeneralApp::Shader& shader)
{
    if (!meshPtr_ || !modelPtr_)
        return;

    auto& modelRef = *modelPtr_;
    auto& meshRef = *meshPtr_;

    Resources::ResourceManager* resourceManager = Resources::ResourceManager::getInstance();

    glBindVertexArray(VAO_);
    const int materialTextures[Resources::Material::TextureIdx::COUNT] = {
        Resources::Material::BASE_COLOR,
        Resources::Material::METALLIC_ROUGHNESS,
        Resources::Material::EMISSIVE,
        Resources::Material::NORMAL,
        Resources::Material::OCCLUSION
    };
    glUniform1iv(glGetUniformLocation(shader.getID(), "materialTextures"), Resources::Material::TextureIdx::COUNT, materialTextures);
    for (size_t i = 0; i < meshRef.primitives.size(); ++i) {
        const tinygltf::Primitive& primitive = meshRef.primitives[i];
        auto& primitiveMaterial = resourceManager->getMaterial(primitiveMaterial_[primitive.mode]);
        const tinygltf::Accessor& indexAccessor = modelRef.accessors[primitive.indices];

        glm::vec4 materialTexturesFactors[Resources::Material::TextureIdx::COUNT];
        for (int i = 0; i < Resources::Material::TextureIdx::COUNT; ++i) {
            auto& texRef = resourceManager->getTexture(primitiveMaterial.textures[i]->name);
            materialTexturesFactors[i] = texRef.factor;

            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, texRef.GL_id);
            glBindSampler(i, texRef.sampler->GL_id);
        }

        glUniform4fv(glGetUniformLocation(shader.getID(), "materialTexturesFactors"), Resources::Material::TextureIdx::COUNT, &materialTexturesFactors[0][0]);
        shader.setUint("materialFlags", primitiveMaterial.materialFlags);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs_.at(indexAccessor.bufferView));
        glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType, (void*)BUFFER_OFFSET(indexAccessor.byteOffset));

        for (int i = 0; i < Resources::Material::TextureIdx::COUNT; ++i) {
            glBindSampler(i, 0);
        }
        glActiveTexture(GL_TEXTURE0);
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

    glGenVertexArrays(1, &VAO_);
    glBindVertexArray(VAO_);

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
        const tinygltf::Primitive& primitive = meshRef.primitives[i];
        const tinygltf::Accessor& indexAccessor = modelRef.accessors[primitive.indices];

        for (auto& attrib : primitive.attributes) {
            tinygltf::Accessor accessor = modelRef.accessors[attrib.second];
            int byteStride = accessor.ByteStride(modelRef.bufferViews[accessor.bufferView]);
            glBindBuffer(GL_ARRAY_BUFFER, VBOs_[accessor.bufferView]);

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

        Resources::ResourceManager* resourceManager = Resources::ResourceManager::getInstance();

        Resources::Image& defaultWhiteImage = resourceManager->getImage(Resources::defaultImagesNames[Resources::Image::DEFAULT_IMAGE_WHITE]);
        Resources::Texture& defaultWhiteTexture = resourceManager->getTexture(Resources::defaultTexturesNames[Resources::Texture::DEFAULT_TEXTURE_WHITE]);
        
        Resources::MaterialDesc matDesc;
        matDesc.name = modelRef.materials[primitive.material].name;
        for (uint32_t i = 0; i < Resources::Texture::COUNT; ++i)
            matDesc.p_TexArray[i] = &defaultWhiteTexture;


        auto baseColorTexIdx = modelRef.materials[primitive.material].pbrMetallicRoughness.baseColorTexture.index;
        auto baseColorFactor = modelRef.materials[primitive.material].pbrMetallicRoughness.baseColorFactor;
        {
            glm::vec4 factor = glm::vec4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);
            Resources::TextureDesc texDesc = {
                "base_color_" + modelRef.materials[primitive.material].name,
                &defaultWhiteImage,
                factor
            };

            if (baseColorTexIdx > -1 && baseColorTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[baseColorTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        &image.image
                    };
                    auto& baseColorImage = resourceManager->createImage(imDesc);
                    texDesc.name = baseColorImage.name;
                    texDesc.p_image = &baseColorImage;
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

            Resources::TextureDesc texDesc = {
                "metallic_roughness_" + modelRef.materials[primitive.material].name,
                &defaultWhiteImage,
                factor
            };

            if (metallicRoughnessTexIdx > -1 && metallicRoughnessTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[metallicRoughnessTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        &image.image
                    };
                    auto& metallicRoughnessImage = resourceManager->createImage(imDesc);
                    texDesc.name = metallicRoughnessImage.name;
                    texDesc.p_image = &metallicRoughnessImage;
                }
            }
            auto& metallicRoughnessTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::METALLIC_ROUGHNESS] = &metallicRoughnessTexture;
        }

        auto emissiveTexIdx = modelRef.materials[primitive.material].emissiveTexture.index;
        auto emissiveFactor = modelRef.materials[primitive.material].emissiveFactor;
        {
            glm::vec4 factor = glm::vec4(emissiveFactor[0], emissiveFactor[1], emissiveFactor[2], 1.0);

            Resources::TextureDesc texDesc = {
                "emissive_" + modelRef.materials[primitive.material].name,
                &defaultWhiteImage,
                factor
            };

            if (emissiveTexIdx > -1 && emissiveTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[emissiveTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        &image.image
                    };
                    auto& emissiveImage = resourceManager->createImage(imDesc);
                    texDesc.name = emissiveImage.name;
                    texDesc.p_image = &emissiveImage;
                }
            }
            auto& emissiveTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::EMISSIVE] = &emissiveTexture;
        }

        auto normalTexIdx = modelRef.materials[primitive.material].normalTexture.index;
        {
            glm::vec4 factor = glm::vec4(1.0);

            Resources::TextureDesc texDesc = {
                "normal_" + modelRef.materials[primitive.material].name,
                &defaultWhiteImage,
                factor
            };

            if (normalTexIdx > -1 && normalTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[normalTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        &image.image
                    };
                    auto& normalImage = resourceManager->createImage(imDesc);
                    texDesc.name = normalImage.name;
                    texDesc.p_image = &normalImage;
                }
            }
            auto& normalTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::NORMAL] = &normalTexture;
        }

        auto occlusionTexIdx = modelRef.materials[primitive.material].occlusionTexture.index;
        {
            glm::vec4 factor = glm::vec4(1.0);

            Resources::TextureDesc texDesc = {
                "occlusion_" + modelRef.materials[primitive.material].name,
                &defaultWhiteImage,
                factor
            };

            if (occlusionTexIdx > -1 && occlusionTexIdx < modelRef.textures.size()) {
                const tinygltf::Texture& tex = modelRef.textures[occlusionTexIdx];
                if (tex.source > -1 && tex.source < modelRef.images.size()) {
                    auto& image = modelRef.images[tex.source];

                    Resources::ImageDesc imDesc = {
                        image.name,
                        image.width,
                        image.height,
                        image.component,
                        image.bits,
                        &image.image
                    };
                    auto& occlusionImage = resourceManager->createImage(imDesc);
                    texDesc.name = occlusionImage.name;
                    texDesc.p_image = &occlusionImage;
                }
            }
            auto& occlusionTexture = resourceManager->createTexture(texDesc);
            matDesc.p_TexArray[Resources::Material::OCCLUSION] = &occlusionTexture;
        }

        auto& newMat = resourceManager->createMaterial(matDesc);
        primitiveMaterial_[primitive.mode] = matDesc.name;

        if (normalTexIdx > -1 && normalTexIdx < modelRef.textures.size()) {
            newMat.materialFlags |= Resources::Material::MATERIAL_FLAG_NORMAL_MAP_BIT;
        }
    }
    glBindVertexArray(0);
}
