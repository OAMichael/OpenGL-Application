#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <iostream>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Image.hpp"
#include "Sampler.hpp"


#include <RenderResource.hpp>

namespace Resources {

struct Texture : RenderResource {
    unsigned int GL_id;

    unsigned faces = 1;
    Image* images[6] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
    glm::vec4 factor = glm::vec4(1.0f);

    int format = -1;

    Sampler* sampler;

    Texture() {};

    enum DefaultTextures : uint32_t {
        DEFAULT_TEXTURE_WHITE = 0,
        DEFAULT_TEXTURE_BLACK = 1,

        COUNT
    };
};


extern std::string defaultTexturesNames[Texture::DefaultTextures::COUNT];

}
#endif
