#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Transformation variables
float scaleX = 1.0f, scaleY = 1.0f, scaleZ = 1.0f;
float rotateX = 0.0f, rotateY = 0.0f, rotateZ = 0.0f;
float translateX = 0.0f, translateY = 0.0f, translateZ = 0.0f;

// Delta values for each transformation type
float scaleDelta = 0.1f;
float rotateDelta = 5.0f; // degrees
float translateDelta = 0.1f;

// Current transformation mode
enum TransformMode { SCALE, ROTATE, TRANSLATE };
TransformMode currentMode = SCALE;

// Cube vertices and indices
float vertices[] = {
    // positions          // colors
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  // red
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  // green
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  // blue
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  // yellow
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  // magenta
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,  // cyan
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  // white
    -0.5f,  0.5f,  0.5f,  0.5f, 0.5f, 0.5f   // gray
};

unsigned int indices[] = {
    // back face
    0, 1, 2, 2, 3, 0,
    // front face
    4, 5, 6, 6, 7, 4,
    // left face
    4, 0, 3, 3, 7, 4,
    // right face
    1, 5, 6, 6, 2, 1,
    // bottom face
    4, 5, 1, 1, 0, 4,
    // top face
    3, 2, 6, 6, 7, 3
};

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
uniform mat4 transform;
void main() {
    gl_Position = transform * vec4(aPos, 1.0);
    ourColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 ourColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(ourColor, 1.0);
}
)";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void printMenu() {
    std::cout << "\n=== CUBE TRANSFORMATION MENU ===" << std::endl;
    std::cout << "Current Mode: ";
    switch(currentMode) {
        case SCALE: std::cout << "SCALE"; break;
        case ROTATE: std::cout << "ROTATE"; break;
        case TRANSLATE: std::cout << "TRANSLATE"; break;
    }
    std::cout << std::endl;

    std::cout << "\nTransformation Values:" << std::endl;
    std::cout << "Scale:     X=" << scaleX << " Y=" << scaleY << " Z=" << scaleZ << std::endl;
    std::cout << "Rotation:  X=" << rotateX << "° Y=" << rotateY << "° Z=" << rotateZ << "°" << std::endl;
    std::cout << "Translate: X=" << translateX << " Y=" << translateY << " Z=" << translateZ << std::endl;

    std::cout << "\nDelta Values:" << std::endl;
    std::cout << "Scale Delta: " << scaleDelta << std::endl;
    std::cout << "Rotate Delta: " << rotateDelta << "°" << std::endl;
    std::cout << "Translate Delta: " << translateDelta << std::endl;

    std::cout << "\nCONTROLS:" << std::endl;
    std::cout << "Mode Selection:" << std::endl;
    std::cout << "  1 - SCALE mode" << std::endl;
    std::cout << "  2 - ROTATE mode" << std::endl;
    std::cout << "  3 - TRANSLATE mode" << std::endl;

    std::cout << "\nAxis Controls:" << std::endl;
    std::cout << "  X/x - Increase/Decrease X component" << std::endl;
    std::cout << "  Y/y - Increase/Decrease Y component" << std::endl;
    std::cout << "  Z/z - Increase/Decrease Z component" << std::endl;

    std::cout << "\nDelta Controls:" << std::endl;
    std::cout << "  + - Increase delta for current transformation" << std::endl;
    std::cout << "  - - Decrease delta for current transformation" << std::endl;

    std::cout << "\nOther Controls:" << std::endl;
    std::cout << "  R - Reset all transformations" << std::endl;
    std::cout << "  M - Show this menu" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "=================================" << std::endl;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Mode selection
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        currentMode = SCALE;
        std::cout << "Mode changed to: SCALE" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        currentMode = ROTATE;
        std::cout << "Mode changed to: ROTATE" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        currentMode = TRANSLATE;
        std::cout << "Mode changed to: TRANSLATE" << std::endl;
    }

    // Axis controls (with shift for decrease)
    bool shiftPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                        glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    // X axis
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        if (currentMode == SCALE) {
            if (shiftPressed) scaleX = std::max(0.1f, scaleX - scaleDelta);
            else scaleX += scaleDelta;
            std::cout << "Scale X: " << scaleX << std::endl;
        }
        else if (currentMode == ROTATE) {
            if (shiftPressed) rotateX -= rotateDelta;
            else rotateX += rotateDelta;
            // Keep rotation within 0-360 degrees
            if (rotateX >= 360.0f) rotateX -= 360.0f;
            if (rotateX < 0.0f) rotateX += 360.0f;
            std::cout << "Rotate X: " << rotateX << "°" << std::endl;
        }
        else if (currentMode == TRANSLATE) {
            if (shiftPressed) translateX -= translateDelta;
            else translateX += translateDelta;
            std::cout << "Translate X: " << translateX << std::endl;
        }
    }

    // Y axis
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        if (currentMode == SCALE) {
            if (shiftPressed) scaleY = std::max(0.1f, scaleY - scaleDelta);
            else scaleY += scaleDelta;
            std::cout << "Scale Y: " << scaleY << std::endl;
        }
        else if (currentMode == ROTATE) {
            if (shiftPressed) rotateY -= rotateDelta;
            else rotateY += rotateDelta;
            if (rotateY >= 360.0f) rotateY -= 360.0f;
            if (rotateY < 0.0f) rotateY += 360.0f;
            std::cout << "Rotate Y: " << rotateY << "°" << std::endl;
        }
        else if (currentMode == TRANSLATE) {
            if (shiftPressed) translateY -= translateDelta;
            else translateY += translateDelta;
            std::cout << "Translate Y: " << translateY << std::endl;
        }
    }

    // Z axis
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        if (currentMode == SCALE) {
            if (shiftPressed) scaleZ = std::max(0.1f, scaleZ - scaleDelta);
            else scaleZ += scaleDelta;
            std::cout << "Scale Z: " << scaleZ << std::endl;
        }
        else if (currentMode == ROTATE) {
            if (shiftPressed) rotateZ -= rotateDelta;
            else rotateZ += rotateDelta;
            if (rotateZ >= 360.0f) rotateZ -= 360.0f;
            if (rotateZ < 0.0f) rotateZ += 360.0f;
            std::cout << "Rotate Z: " << rotateZ << "°" << std::endl;
        }
        else if (currentMode == TRANSLATE) {
            if (shiftPressed) translateZ -= translateDelta;
            else translateZ += translateDelta;
            std::cout << "Translate Z: " << translateZ << std::endl;
        }
    }

    // Delta controls
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        if (currentMode == SCALE) {
            scaleDelta += 0.01f;
            std::cout << "Scale Delta increased to: " << scaleDelta << std::endl;
        }
        else if (currentMode == ROTATE) {
            rotateDelta += 1.0f;
            std::cout << "Rotate Delta increased to: " << rotateDelta << "°" << std::endl;
        }
        else if (currentMode == TRANSLATE) {
            translateDelta += 0.01f;
            std::cout << "Translate Delta increased to: " << translateDelta << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS ||
        glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        if (currentMode == SCALE) {
            scaleDelta = std::max(0.01f, scaleDelta - 0.01f);
            std::cout << "Scale Delta decreased to: " << scaleDelta << std::endl;
        }
        else if (currentMode == ROTATE) {
            rotateDelta = std::max(1.0f, rotateDelta - 1.0f);
            std::cout << "Rotate Delta decreased to: " << rotateDelta << "°" << std::endl;
        }
        else if (currentMode == TRANSLATE) {
            translateDelta = std::max(0.01f, translateDelta - 0.01f);
            std::cout << "Translate Delta decreased to: " << translateDelta << std::endl;
        }
    }

    // Reset all transformations
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        scaleX = scaleY = scaleZ = 1.0f;
        rotateX = rotateY = rotateZ = 0.0f;
        translateX = translateY = translateZ = 0.0f;
        std::cout << "All transformations reset" << std::endl;
    }

    // Show menu
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        printMenu();
    }
}

// Matrix transformation functions
void scaleMatrix(float* matrix, float sx, float sy, float sz) {
    float temp[] = {
        sx, 0.0f, 0.0f, 0.0f,
        0.0f, sy, 0.0f, 0.0f,
        0.0f, 0.0f, sz, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    for (int i = 0; i < 16; i++) matrix[i] = temp[i];
}

void rotateXMatrix(float* matrix, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float temp[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cosA, -sinA, 0.0f,
        0.0f, sinA, cosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    for (int i = 0; i < 16; i++) matrix[i] = temp[i];
}

void rotateYMatrix(float* matrix, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float temp[] = {
        cosA, 0.0f, sinA, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        -sinA, 0.0f, cosA, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    for (int i = 0; i < 16; i++) matrix[i] = temp[i];
}

void rotateZMatrix(float* matrix, float angle) {
    float cosA = cos(angle);
    float sinA = sin(angle);
    float temp[] = {
        cosA, -sinA, 0.0f, 0.0f,
        sinA, cosA, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    for (int i = 0; i < 16; i++) matrix[i] = temp[i];
}

void translateMatrix(float* matrix, float tx, float ty, float tz) {
    float temp[] = {
        1.0f, 0.0f, 0.0f, tx,
        0.0f, 1.0f, 0.0f, ty,
        0.0f, 0.0f, 1.0f, tz,
        0.0f, 0.0f, 0.0f, 1.0f
    };
    for (int i = 0; i < 16; i++) matrix[i] = temp[i];
}

void multiplyMatrix(float* result, const float* a, const float* b) {
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            result[i * 4 + j] = 0;
            for (int k = 0; k < 4; k++)
                result[i * 4 + j] += a[i * 4 + k] * b[k * 4 + j];
        }
}

void identityMatrix(float* matrix) {
    for (int i = 0; i < 16; i++) matrix[i] = 0.0f;
    for (int i = 0; i < 4; i++) matrix[i * 4 + i] = 1.0f;
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "3D Cube Transformations", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Print initial instructions
    printMenu();

    // Compile shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Setup buffers
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        // Create transformation matrix in order: scale -> rotation -> translation
        float transform[16];
        float scaleMat[16], rotateXMat[16], rotateYMat[16], rotateZMat[16], translateMat[16];
        float temp1[16], temp2[16], temp3[16];

        // Start with identity
        identityMatrix(transform);

        // 1. Scale
        scaleMatrix(scaleMat, scaleX, scaleY, scaleZ);
        multiplyMatrix(temp1, scaleMat, transform);

        // 2. Rotation (X -> Y -> Z)
        rotateXMatrix(rotateXMat, rotateX * 3.14159f / 180.0f);
        multiplyMatrix(temp2, rotateXMat, temp1);

        rotateYMatrix(rotateYMat, rotateY * 3.14159f / 180.0f);
        multiplyMatrix(temp3, rotateYMat, temp2);

        rotateZMatrix(rotateZMat, rotateZ * 3.14159f / 180.0f);
        multiplyMatrix(temp1, rotateZMat, temp3);

        // 3. Translation
        translateMatrix(translateMat, translateX, translateY, translateZ);
        multiplyMatrix(transform, translateMat, temp1);

        // Pass transformation to shader
        int transformLoc = glGetUniformLocation(shaderProgram, "transform");
        glUniformMatrix4fv(transformLoc, 1, GL_FALSE, transform);

        // Draw cube
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}