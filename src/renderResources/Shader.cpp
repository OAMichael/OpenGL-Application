#include <regex>
#include <sstream>

#include "Shader.hpp"
#include "Logger.hpp"

namespace Resources {

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
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
        
        vertexCode_ = vShaderStream.str();
        fragmentCode_ = fShaderStream.str();
    }
    catch (std::ifstream::failure& e) {
        LOG_E("File not successfully read: %s", e.what());
    }

    vertexCode_ = PreprocessIncludes(vertexCode_, vertexPath);
    fragmentCode_ = PreprocessIncludes(fragmentCode_, fragmentPath);

    const char* vShaderCode = vertexCode_.c_str();
    const char* fShaderCode = fragmentCode_.c_str();
    
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
            bool vertexShader = type == "VERTEX";
            const char* strType = vertexShader ? "vertex" : "fragment";

            std::string lineNoStr = infoLog;
            size_t par1 = lineNoStr.find_first_of("(", 0);
            if (par1 < lineNoStr.length() - 1) {
                lineNoStr = lineNoStr.substr(par1 + 1);
            }
            size_t par2 = lineNoStr.find_first_of(")", 0);
            if (par2 < lineNoStr.length()) {
                lineNoStr = lineNoStr.substr(0, par2);
            }

            int lineNo = std::stoi(lineNoStr);
            std::string errorLines = vertexShader ? vertexCode_ : fragmentCode_;

            for (int i = 0; i < lineNo - 7; ++i) {
                errorLines = errorLines.substr(errorLines.find_first_of("\n") + 1);
            }

            std::string errorLinesBefore = errorLines;
            size_t pos = 0;
            for (int i = 0; i < 7; ++i) {
                if (pos < errorLinesBefore.length() - 1) {
                    pos = errorLinesBefore.find_first_of("\n", pos + 1);
                }
            }
            if (pos < errorLinesBefore.length()) {
                errorLinesBefore = errorLinesBefore.substr(0, pos);
                if (pos < errorLines.length()) {
                    errorLines = errorLines.substr(pos + 1);
                }
            }
            errorLinesBefore = errorLinesBefore + "\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

            pos = 0;
            for (int i = 0; i < 7; ++i) {
                if (pos < errorLines.length() - 1) {
                    pos = errorLines.find_first_of("\n", pos + 1);
                }
            }
            if (pos < errorLines.length()) {
                errorLines = errorLines.substr(0, pos);
            }

            errorLinesBefore = errorLinesBefore + errorLines;

            LOG_E("Compiling %s shader: %s\nShader line: %d\n%s\n", strType, infoLog, lineNo, errorLinesBefore.c_str());
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
                LOG_E("Compiling shader: %s.%d: cannot open include file \'%s\'", filename.c_str(), line_number, include_filename.c_str());
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