#include "../headers/Cube.hpp"

#include "../external/stb_image.h"


namespace Geometry {

Cube::Cube() {

};


Cube::~Cube() {

};


Cube::Cube(const glm::vec3& pos) : Position_{pos} {

};


void Cube::generateTextures(const std::vector<std::string>& textureNames) {
	glGenTextures(1, &cubemapTexture_);
	setTextures(textureNames);
}


void Cube::setTextures(const std::vector<std::string>& textureNames) {
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture_);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    int width, height, nrChannels;
    for(unsigned int i = 0; i < textureNames.size(); i++) {
        unsigned char *data = stbi_load(textureNames[i].c_str(), &width, &height, &nrChannels, 0);
        if(data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap tex failed to load at path: " << textureNames[i] << std::endl;
            stbi_image_free(data);
        }
    }
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

}


unsigned Cube::getCubemapTextureHandle() const {
	return cubemapTexture_;
}


const std::vector<float>& Cube::getVertices() const {
	return vertices_;
};


glm::vec3 Cube::getPosition() {
	return Position_;
}


void Cube::setVisibility(bool vis) {
    isVisible_ = vis;
}


bool Cube::getVisibility() {
    return isVisible_;
}

}