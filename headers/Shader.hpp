#ifndef SHADER_HPP
#define SHADER_HPP

#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace GeneralApp {

class Shader {

private:
    void checkCompileErrors(const GLuint& shader, const std::string& type);
    std::string PreprocessIncludes(const std::string& source, const std::string& filename, int level = 0);

public:
    unsigned GL_id;
    std::string name;

    Shader();
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    void use() const;

    unsigned getID() const;
    
    void setBool(const std::string& name, const bool& value) const;

    void setInt(const std::string& name, const int& value) const;

    void setUint(const std::string& name, const uint32_t& value) const;

    void setFloat(const std::string& name, const float& value) const;

    void setVec2(const std::string& name, const glm::vec2& value) const;

    void setVec2(const std::string& name, const float& x, const float& y) const;

    void setVec3(const std::string& name, const glm::vec3& value) const;

    void setVec3(const std::string& name, const float& x, const float& y, const float& z) const;

    void setVec4(const std::string& name, const glm::vec4& value) const;

    void setVec4(const std::string& name, const float& x, const float& y, const float& z, const float& w) const;

    void setMat2(const std::string& name, const glm::mat2& mat) const;

    void setMat3(const std::string& name, const glm::mat3& mat) const;

    void setMat4(const std::string& name, const glm::mat4& mat) const;

};

}
#endif