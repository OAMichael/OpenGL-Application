#ifndef SHADER_HPP
#define SHADER_HPP

#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <iostream>

#include <RenderResource.hpp>

namespace Resources {

class Shader : public RenderResource {

private:
    std::string vertexCode_;
    std::string fragmentCode_;

    void checkCompileErrors(const GLuint& shader, const std::string& type);
    std::string PreprocessIncludes(const std::string& source, const std::string& filename, int level = 0);

public:
    unsigned GL_id;

    Shader(const char* vertexPath, const char* fragmentPath);
    Shader() {};
    ~Shader() {};

    inline void use() const { glUseProgram(GL_id); }
    inline unsigned getID() const { return GL_id; }

    inline void setBool(const std::string& name, const bool value) const { glUniform1i(glGetUniformLocation(GL_id, name.c_str()), (int)value); }
    inline void setInt(const std::string& name, const int value) const { glUniform1i(glGetUniformLocation(GL_id, name.c_str()), value); }
    inline void setUint(const std::string& name, const uint32_t value) const { glUniform1ui(glGetUniformLocation(GL_id, name.c_str()), value); }
    inline void setFloat(const std::string& name, const float value) const { glUniform1f(glGetUniformLocation(GL_id, name.c_str()), value); }
    inline void setVec2(const std::string& name, const glm::vec2 value) const { glUniform2fv(glGetUniformLocation(GL_id, name.c_str()), 1, &value[0]); }
    inline void setVec2(const std::string& name, const float x, const float y) const { glUniform2f(glGetUniformLocation(GL_id, name.c_str()), x, y); }
    inline void setVec3(const std::string& name, const glm::vec3 value) const { glUniform3fv(glGetUniformLocation(GL_id, name.c_str()), 1, &value[0]); }
    inline void setVec3(const std::string& name, const float x, const float y, const float z) const { glUniform3f(glGetUniformLocation(GL_id, name.c_str()), x, y, z); }
    inline void setVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(GL_id, name.c_str()), 1, &value[0]); }
    inline void setVec4(const std::string& name, const float x, const float y, const float z, const float w) const { glUniform4f(glGetUniformLocation(GL_id, name.c_str()), x, y, z, w); }
    inline void setMat2(const std::string& name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(GL_id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
    inline void setMat3(const std::string& name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(GL_id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
    inline void setMat4(const std::string& name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(GL_id, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
    inline void setIntArray(const std::string& name, const int* data, const size_t size) const { glUniform1iv(glGetUniformLocation(GL_id, name.c_str()), size, data); }
    inline void setUintArray(const std::string& name, const uint32_t* data, const size_t size) const { glUniform1uiv(glGetUniformLocation(GL_id, name.c_str()), size, data); }
    inline void setFloatArray(const std::string& name, const float* data, const size_t size) const { glUniform1fv(glGetUniformLocation(GL_id, name.c_str()), size, data); }
    inline void setVec2Array(const std::string& name, const float* data, const size_t size) const { glUniform2fv(glGetUniformLocation(GL_id, name.c_str()), size, data); }
    inline void setVec3Array(const std::string& name, const float* data, const size_t size) const { glUniform3fv(glGetUniformLocation(GL_id, name.c_str()), size, data); }
    inline void setVec4Array(const std::string& name, const float* data, const size_t size) const { glUniform4fv(glGetUniformLocation(GL_id, name.c_str()), size, data); }
};

}
#endif