#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <string>

struct Triangle {
    glm::vec3 v1, v2, v3;
};

std::vector<Triangle> loadSMFModel(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<glm::vec3> vertices;
    std::vector<Triangle> triangles;

    if (!file.is_open()) {
        std::cerr << "❌ File not available " << filename << std::endl;
        return triangles;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        char type;
        iss >> type;

        if (type == 'v') {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back({x, y, z});
        } else if (type == 'f') {
            int a, b, c;
            iss >> a >> b >> c;
            triangles.push_back({vertices[a - 1], vertices[b - 1], vertices[c - 1]});
        }
    }

    std::cout << "✅ Uplouded " << triangles.size() << " triangles " << filename << std::endl;
    return triangles;
}

glm::vec3 computeNormal(const Triangle& t) {
    glm::vec3 u = t.v2 - t.v1;
    glm::vec3 v = t.v3 - t.v1;
    glm::vec3 n = glm::normalize(glm::cross(u, v));
    return glm::abs(n);
}

int main() {
    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(800, 600, "Assignment 3 - Flat Shading", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewInit();
    glEnable(GL_DEPTH_TEST);


    std::string filename = "bunny-200.smf";
    auto mesh = loadSMFModel(filename);

    if (mesh.empty()) {
        std::cerr << "⚠️ Файл не найден или пуст. Создаю тестовую модель..." << std::endl;
        for (float x = -1; x < 1; x += 0.1f)
            for (float y = -1; y < 1; y += 0.1f)
                mesh.push_back({{x, y, 0.0f}, {x + 0.1f, y, 0.0f}, {x, y + 0.1f, 0.0f}});
    }

    glm::vec3 centroid(0.0f);
    for (auto& t : mesh) centroid += (t.v1 + t.v2 + t.v3) / 3.0f;
    centroid /= (float)mesh.size();

    float angle = 0.0f, radius = 3.0f, height = 0.5f;

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) angle -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) angle += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) height += 0.02f;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) height -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) radius -= 0.02f;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) radius += 0.02f;

        glm::vec3 eye(radius * cos(angle), radius * sin(angle), height);
        glm::mat4 view = glm::lookAt(eye + centroid, centroid, glm::vec3(0, 0, 1));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        glm::mat4 vp = proj * view;

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glLoadMatrixf(glm::value_ptr(vp));

        glBegin(GL_TRIANGLES);
        for (auto& t : mesh) {
            glm::vec3 color = computeNormal(t);
            glColor3f(color.x, color.y, color.z);
            glVertex3fv(glm::value_ptr(t.v1));
            glVertex3fv(glm::value_ptr(t.v2));
            glVertex3fv(glm::value_ptr(t.v3));
        }
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
