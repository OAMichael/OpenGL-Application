#include <regex>
#include <sstream>

#include "Shader.hpp"
#include "Logger.hpp"

namespace Resources {

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;

    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);

        std::stringstream vShaderStream, fShaderStream;
        
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();       
        
        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();         
    }
    catch (std::ifstream::failure& e) {
        LOG_E("File not successfully read: %s", e.what());
    }

    vertexCode = PreprocessIncludes(vertexCode, vertexPath);
    fragmentCode = PreprocessIncludes(fragmentCode, fragmentPath);

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    unsigned int vertex, fragment;

    vertex = glCreateShader(GL_VERTEX_SHADER);
    fragment = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    
    glCompileShader(vertex);
    glCompileShader(fragment);

    checkCompileErrors(vertex, "VERTEX");
    checkCompileErrors(fragment, "FRAGMENT");

    GL_id = glCreateProgram();

    glAttachShader(GL_id, vertex);
    glAttachShader(GL_id, fragment);

    glLinkProgram(GL_id);

    checkCompileErrors(GL_id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
}


void Shader::checkCompileErrors(const GLuint& shader, const std::string& type) {
    GLint success;
    GLchar infoLog[1024];
    if(type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            LOG_E("Compiling shader: %s", infoLog);
        }
    }
    else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            LOG_E("Linking shader: %s", infoLog);
        }
    }
}

std::string Shader::PreprocessIncludes(const std::string& source, const std::string& filename, int level)
{
    if (level > 32) {
        LOG_W("Header inclusion depth limit reached, might be caused by cyclic header inclusion");
    }

    static const std::regex re("^[ ]*#[ ]*include[ ]+[\"<](.*)[\">].*");

    std::stringstream input;
    std::stringstream output;
    input << source;

    size_t line_number = 1;
    std::smatch matches;

    std::string line;
    while (std::getline(input, line)) {
        if (std::regex_search(line, matches, re)) {
            std::string include_filename = matches[1];
            include_filename = Utils::fileBaseDir(filename) + include_filename;
            std::string include_string;

            std::ifstream include_file;
            std::stringstream include_lines;
            include_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
            try {
                include_file.open(include_filename);
                include_lines << include_file.rdbuf();
                include_file.close();
                include_string = include_lines.str();
            }
            catch (std::ifstream::failure& e) {
                LOG_E("Compiling shader: %s.%d: cannot open include file \'%s\'", filename.c_str(), line_number, include_filename);
            }
            output << PreprocessIncludes(include_string, include_filename, level + 1) << std::endl;
        }
        else {
            output << line << std::endl;
        }
        ++line_number;
    }

    return output.str();
}

}