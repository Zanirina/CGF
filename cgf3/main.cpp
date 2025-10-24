#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <array>
#include <cmath>

#include "shader_utils.h"
#include <imgui.h>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

struct Triangle {
    glm::vec3 v1, v2, v3;
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

struct LightProps {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

std::vector<Triangle> loadSMFModel(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<glm::vec3> vertices;
    std::vector<Triangle> triangles;
    if (!file.is_open()) {
        std::cerr << "Error: couldn't open file " << filename << std::endl;
        return triangles;
    }
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        char t;
        if (!(iss >> t)) continue;
        if (t == 'v') {
            float x,y,z; iss >> x >> y >> z;
            vertices.emplace_back(x,y,z);
        } else if (t == 'f') {
            int a,b,c; iss >> a >> b >> c;
            if (a<=0 || b<=0 || c<=0 || a>(int)vertices.size() || b>(int)vertices.size() || c>(int)vertices.size()) continue;
            triangles.push_back({vertices[a-1], vertices[b-1], vertices[c-1]});
        }
    }
    std::cout << "Loaded " << triangles.size() << " triangles from " << filename << std::endl;
    return triangles;
}

// ----- per-triangle normal -----
glm::vec3 computeNormal(const Triangle& t) {
    glm::vec3 u = t.v2 - t.v1;
    glm::vec3 v = t.v3 - t.v1;
    glm::vec3 n = glm::normalize(glm::cross(u, v));
    return n;
}

// ----- build vertex array (positions + normals per-vertex) -----
// For Gouraud and Phong we want vertex normals averaged: compute per-vertex normals by accumulating
std::vector<Vertex> buildVertexDataWithAveragedNormals(const std::vector<Triangle>& tris) {
    // Map positions to indices: since many identical positions may occur, we'll build arrays
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normalsAccum; // accumulative normal
    std::vector<int> idxMap; // not needed if we simply duplicate vertices but then compute averaged normals per index
    // Simpler approach: create per-vertex (unique) map by position string -> index
    std::map<std::tuple<int,int,int>, int> dummy; // not used
    // EASIER: We'll unique by floating coords using exact match (ok for SMF)
    std::vector<Vertex> verts;
    // First gather unique positions and track adjacency
    std::vector<glm::vec3> uniqPos;
    std::vector<glm::vec3> uniqNormalSum;
    std::vector<std::vector<int>> facesForVertex;

    auto findOrAdd = [&](const glm::vec3 &p) -> int {
        for (int i=0;i<(int)uniqPos.size();++i) {
            if (uniqPos[i]==p) return i;
        }
        uniqPos.push_back(p);
        uniqNormalSum.emplace_back(0.0f);
        facesForVertex.emplace_back();
        return (int)uniqPos.size()-1;
    };

    // store faces indices referencing uniq positions
    std::vector<std::array<int,3>> facesIdx;
    for (auto &t: tris) {
        int ia = findOrAdd(t.v1), ib = findOrAdd(t.v2), ic = findOrAdd(t.v3);
        facesIdx.push_back({ia,ib,ic});
    }
    // accumulate normals
    for (auto &fidx : facesIdx) {
        Triangle t{uniqPos[fidx[0]], uniqPos[fidx[1]], uniqPos[fidx[2]]};
        glm::vec3 n = computeNormal(t);
        uniqNormalSum[fidx[0]] += n;
        uniqNormalSum[fidx[1]] += n;
        uniqNormalSum[fidx[2]] += n;
    }
    // normalize and build final vertex array (triangle list)
    for (auto &fidx: facesIdx) {
        for (int k=0;k<3;++k) {
            int vid = fidx[k];
            glm::vec3 n = glm::normalize(uniqNormalSum[vid]);
            verts.push_back({uniqPos[vid], n});
        }
    }
    return verts;
}

// ----- simple helper to draw an overlay menu (3 rectangles) -----
void drawMaterialMenuOverlay(int winW, int winH, int mouseX, int mouseY, const std::vector<Material>& materials, bool highlight, int hoveredIndex) {
    // Set orthographic projection for overlay and draw simple colored quads
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, winW, winH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    int menuW = 180;
    int menuH = 120;
    int x = winW - menuW - 20;
    int y = 20;

    for (int i=0;i<(int)materials.size();++i) {
        int itemH = menuH / materials.size();
        int ix = x;
        int iy = y + i*itemH;
        // border
        glBegin(GL_QUADS);
        if (i==hoveredIndex) {
            glColor3f(0.9f,0.9f,0.9f);
            glVertex2f(ix-2, iy-2); glVertex2f(ix+menuW+2, iy-2);
            glVertex2f(ix+menuW+2, iy+itemH+2); glVertex2f(ix-2, iy+itemH+2);
        }
        // background
        glColor3f(materials[i].diffuse.r, materials[i].diffuse.g, materials[i].diffuse.b);
        glVertex2f(ix, iy); glVertex2f(ix+menuW, iy);
        glVertex2f(ix+menuW, iy+itemH); glVertex2f(ix, iy+itemH);
        glEnd();
    }

    // restore matrices
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

// ----- main -----
int main() {
    if (!glfwInit()) { std::cerr<<"GLFW init failed\n"; return -1; }
    GLFWwindow* window = glfwCreateWindow(1024, 768, "Assignment3 - Lights & Shading", nullptr, nullptr);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    if (glewInit()!=GLEW_OK) { std::cerr<<"GLEW init failed\n"; return -1; }
    glEnable(GL_DEPTH_TEST);

    // ---- load model ----
    std::string modelPath = "models/bunny-200.smf";
    auto tris = loadSMFModel(modelPath);
    if (tris.empty()) {
        std::cerr<<"Error: model empty\n";
        return -1;
    }

    // compute centroid & bounding radius
    glm::vec3 centroid(0.0f);
    float countVerts = 0.0f;
    for (auto &t : tris) {
        centroid += (t.v1 + t.v2 + t.v3) / 3.0f;
        countVerts += 1.0f;
    }
    centroid /= countVerts;
    float maxDist = 0.0f;
    for (auto &t : tris) {
        for (auto p : {t.v1, t.v2, t.v3}) {
            float d = glm::length(p - centroid);
            if (d > maxDist) maxDist = d;
        }
    }

    // Build vertex array (averaged normals for vertex shading)
    auto verts = buildVertexDataWithAveragedNormals(tris);

    GLuint VAO, VBO;
    glGenVertexArrays(1,&VAO); glGenBuffers(1,&VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verts.size()*sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*)offsetof(Vertex,normal));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // ---- shader programs ----
    GLuint gouraudProg = createShaderProgram("shaders/gouraud.vert", "shaders/gouraud.frag");
    GLuint phongProg   = createShaderProgram("shaders/phong.vert",   "shaders/phong.frag");
    if (!gouraudProg || !phongProg) {
        std::cerr<<"Error: shader creation failed\n";
        return -1;
    }
    GLuint program = gouraudProg;

    // ---- lights and materials ----
    LightProps lightProps;
    lightProps.ambient = glm::vec3(0.2f);
    lightProps.diffuse = glm::vec3(0.6f);
    lightProps.specular = glm::vec3(1.0f);

    // material presets (3)
    std::vector<Material> materials;
    // material 0: reddish bright specular (matches required properties)
    materials.push_back({ glm::vec3(0.6f,0.2f,0.2f), glm::vec3(0.9f,0.1f,0.1f), glm::vec3(0.8f,0.8f,0.8f), 80.0f });
    // material 1: dull green (low specular)
    materials.push_back({ glm::vec3(0.1f,0.3f,0.1f), glm::vec3(0.2f,0.7f,0.2f), glm::vec3(0.2f,0.2f,0.2f), 8.0f });
    // material 2: shiny blue plastic
    materials.push_back({ glm::vec3(0.05f,0.05f,0.2f), glm::vec3(0.1f,0.3f,0.8f), glm::vec3(0.9f,0.9f,1.0f), 120.0f });

    int currentMaterial = 0;

    // two lights:
    // light1 - object space, on a cylinder around object (user controls)
    float light1_angle = 0.0f, light1_radius = maxDist * 1.5f, light1_height = 0.0f;
    // light2 - camera space (near eye)
    glm::vec3 light2_offset(0.0f, 0.0f, 0.0f); // we will place near eye

    // camera params (smooth)
    float camAngle = 0.0f;
    float camRadius = maxDist * 3.0f;
    float camHeight = 0.0f;
    float camSpeedAngle = 1.5f; // rad/sec when key pressed
    float camSpeedRadius = 2.0f; // units/sec
    float camSpeedHeight = 1.0f;

    double lastTime = glfwGetTime();

    // UI menu state
    bool showMenu = false;
    int hoveredMenuIndex = -1;

    // mouse state for clicking
    double mouseX=0, mouseY=0;
    int winW=1024, winH=768;

    // callbacks for mouse pos
    glfwSetCursorPosCallback(window, [](GLFWwindow*, double xpos, double ypos){
        // we query mouse pos when needed
        (void)xpos; (void)ypos;
    });

    // scroll to zoom (radius)
    glfwSetScrollCallback(window, [](GLFWwindow* w, double xoff, double yoff){
        // store in window user ptr? easier: adjust via globals isn't nice — we'll poll scroll in main loop instead
        (void)w; (void)xoff; (void)yoff;
    });

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = float(now - lastTime);
        lastTime = now;

        glfwGetFramebufferSize(window, &winW, &winH);
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // Input: smooth camera using dt
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) camAngle -= camSpeedAngle * dt;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) camAngle += camSpeedAngle * dt;
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) camHeight += camSpeedHeight * dt;
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) camHeight -= camSpeedHeight * dt;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camRadius -= camSpeedRadius * dt;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camRadius += camSpeedRadius * dt;

        // light1 controls (object-space cylinder)
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) light1_angle -= 1.5f * dt;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) light1_angle += 1.5f * dt;
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) light1_radius -= 1.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) light1_radius += 1.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) light1_height -= 1.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) light1_height += 1.0f * dt;

        // shading mode
        static bool gPressed = false, pPressed = false;
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed) { program = gouraudProg; gPressed = true; pPressed = false; std::cout<<"Mode: Gouraud\n"; }
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) gPressed = false;
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pPressed) { program = phongProg; pPressed = true; gPressed = false; std::cout<<"Mode: Phong\n"; }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) pPressed = false;

        // toggle menu on right mouse button press
        static bool rightWas = false;
        int rightState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT);
        if (rightState == GLFW_PRESS && !rightWas) {
            showMenu = !showMenu;
        }
        rightWas = (rightState==GLFW_PRESS);

        // if menu visible, detect hover and left click selection
        hoveredMenuIndex = -1;
        if (showMenu) {
            int menuW = 180;
            int menuH = 120;
            int mx = winW - menuW - 20;
            int my = 20;
            int itemH = menuH / (int)materials.size();
            for (int i=0;i<(int)materials.size();++i) {
                int ix=mx, iy=my + i*itemH;
                if (mouseX >= ix && mouseX <= ix+menuW && mouseY >= iy && mouseY <= iy+itemH) hoveredMenuIndex = i;
            }
            // left click to select
            static bool leftWas = false;
            int leftState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if (leftState == GLFW_PRESS && !leftWas) {
                if (hoveredMenuIndex >=0) {
                    currentMaterial = hoveredMenuIndex;
                    std::cout<<"Material selected: "<<currentMaterial<<"\n";
                    showMenu = false;
                }
            }
            leftWas = (leftState==GLFW_PRESS);
        }

        // compute camera eye and view/projection
        glm::vec3 eye(camRadius * cos(camAngle), camRadius * sin(camAngle), camHeight);
        glm::mat4 view = glm::lookAt(eye + centroid, centroid, glm::vec3(0,0,1));
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)winW/(float)winH, 0.01f, 1000.0f);
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 mvp = proj * view * model;

        // compute light1 pos in object coords (around centroid)
        glm::vec3 light1pos = centroid + glm::vec3(light1_radius * cos(light1_angle), light1_radius * sin(light1_angle), light1_height);

        // light2 near camera (camera coordinate system) -> place a bit in front of eye
        glm::vec3 cameraDir = glm::normalize(centroid - (eye + centroid)); // direction from eye to centroid (since eye already offset by centroid)
        glm::vec3 light2pos = (eye + centroid) + glm::normalize(centroid - (eye + centroid)) * 0.5f * maxDist; // a bit towards object

        glViewport(0,0,winW,winH);
        glClearColor(0.12f,0.12f,0.12f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use program, set uniforms for both lights and material
        glUseProgram(program);
        glBindVertexArray(VAO);
        // common uniforms
        GLint loc;
        loc = glGetUniformLocation(program, "uMVP"); if (loc>=0) glUniformMatrix4fv(loc,1,GL_FALSE, glm::value_ptr(mvp));
        loc = glGetUniformLocation(program, "uModel"); if (loc>=0) glUniformMatrix4fv(loc,1,GL_FALSE, glm::value_ptr(model));
        // lights (shared lightProps values)
        loc = glGetUniformLocation(program, "uLight1Pos"); if (loc>=0) glUniform3f(loc, light1pos.x, light1pos.y, light1pos.z);
        loc = glGetUniformLocation(program, "uLight2Pos"); if (loc>=0) glUniform3f(loc, light2pos.x, light2pos.y, light2pos.z);
        loc = glGetUniformLocation(program, "uLightAmbient"); if (loc>=0) glUniform3f(loc, lightProps.ambient.r, lightProps.ambient.g, lightProps.ambient.b);
        loc = glGetUniformLocation(program, "uLightDiffuse"); if (loc>=0) glUniform3f(loc, lightProps.diffuse.r, lightProps.diffuse.g, lightProps.diffuse.b);
        loc = glGetUniformLocation(program, "uLightSpecular"); if (loc>=0) glUniform3f(loc, lightProps.specular.r, lightProps.specular.g, lightProps.specular.b);
        // view pos
        loc = glGetUniformLocation(program, "uViewPos"); if (loc>=0) glUniform3f(loc, eye.x+centroid.x, eye.y+centroid.y, eye.z+centroid.z);
        // material
        Material m = materials[currentMaterial];
        loc = glGetUniformLocation(program, "uMatAmbient"); if (loc>=0) glUniform3f(loc, m.ambient.r, m.ambient.g, m.ambient.b);
        loc = glGetUniformLocation(program, "uMatDiffuse"); if (loc>=0) glUniform3f(loc, m.diffuse.r, m.diffuse.g, m.diffuse.b);
        loc = glGetUniformLocation(program, "uMatSpecular"); if (loc>=0) glUniform3f(loc, m.specular.r, m.specular.g, m.specular.b);
        loc = glGetUniformLocation(program, "uMatShininess"); if (loc>=0) glUniform1f(loc, m.shininess);

        glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verts.size());

        // draw overlay menu if visible
        if (showMenu) {
            // convert hoveredMenuIndex computed earlier
            drawMaterialMenuOverlay(winW, winH, (int)mouseX, (int)mouseY, materials, true, hoveredMenuIndex);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
