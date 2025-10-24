#pragma once
#include <GL/glew.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

inline std::string loadShaderSource(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: couldn't open shader " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

inline GLuint compileShader(GLenum type, const std::string& source) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetShaderInfoLog(shader, 1024, nullptr, info);
        std::cerr << "Shader compile error:\n" << info << std::endl;
    }
    return shader;
}

inline GLuint createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vSrc = loadShaderSource(vertexPath);
    std::string fSrc = loadShaderSource(fragmentPath);

    if (vSrc.empty() || fSrc.empty()) {
        std::cerr << "Error: shader source empty for " << vertexPath << " or " << fragmentPath << std::endl;
        return 0;
    }

    GLuint vShader = compileShader(GL_VERTEX_SHADER, vSrc);
    GLuint fShader = compileShader(GL_FRAGMENT_SHADER, fSrc);

    GLuint program = glCreateProgram();
    glAttachShader(program, vShader);
    glAttachShader(program, fShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info[1024];
        glGetProgramInfoLog(program, 1024, nullptr, info);
        std::cerr << "Program link error:\n" << info << std::endl;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);

    return program;
}
