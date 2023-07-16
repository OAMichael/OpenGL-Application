#include "../headers/Mesh.hpp"


void Geometry::Mesh::draw(GeneralApp::Shader& shader)
{
    if (!meshPtr_ || !modelPtr_)
        return;

    auto& modelRef = *modelPtr_;
    auto& meshRef = *meshPtr_;

    Resources::ResourceManager* resourceManager = Resources::ResourceManager::getInstance();

    glBindVertexArray(VAO_);
    for (size_t i = 0; i < meshRef.primitives.size(); ++i) {
        const tinygltf::Primitive& primitive = meshRef.primitives[i];
        if (resourceManager->hasTexture(primitiveMaterial_[primitive.mode])) {
            auto& texRef = resourceManager->getTexture(primitiveMaterial_[primitive.mode]);
            const tinygltf::Accessor& indexAccessor = modelRef.accessors[primitive.indices];

            glBindTexture(GL_TEXTURE_2D, texRef.GL_id);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBOs_.at(indexAccessor.bufferView));
            glDrawElements(primitive.mode, indexAccessor.count, indexAccessor.componentType, (void*)BUFFER_OFFSET(indexAccessor.byteOffset));
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

        auto baseColorTexIdx = modelRef.materials[primitive.material].pbrMetallicRoughness.baseColorTexture.index;
        auto baseColorFactor = modelRef.materials[primitive.material].pbrMetallicRoughness.baseColorFactor;

        primitiveMaterial_[primitive.mode] = Resources::defaultTexturesNames[Resources::Texture::DEFAULT_TEXTURE_WHITE];

        if (baseColorTexIdx > -1 || baseColorFactor.size() == 4) {
            auto& baseColorImage = resourceManager->getImage(Resources::defaultImagesNames[Resources::Image::DEFAULT_IMAGE_WHITE]);
            glm::vec4 factor = glm::vec4(1.0);

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
                    baseColorImage = resourceManager->createImage(imDesc);
                }
            }
            if (baseColorFactor.size() == 4)
                factor = glm::vec4(baseColorFactor[0], baseColorFactor[1], baseColorFactor[2], baseColorFactor[3]);

            Resources::TextureDesc texDesc = {
                baseColorImage.name,
                &baseColorImage,
                factor
            };

            auto& createdTexture = resourceManager->createTexture(texDesc);
            primitiveMaterial_[primitive.mode] = createdTexture.name;
        }
    }
    glBindVertexArray(0);
}
