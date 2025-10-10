#define _USE_MATH_DEFINES
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>

// Global variables
GLFWwindow* mainWindow = nullptr;
GLFWwindow* secondWindow = nullptr;

// Animation control
bool animationEnabled = true;

// Colors
float squareColor[3] = {1.0f, 1.0f, 1.0f}; // White
float subWindowBgColor[3] = {0.2f, 0.2f, 0.5f}; // Blue-gray
float circleTriangleColor[3] = {1.0f, 0.0f, 0.0f}; // Red

// Animation parameters
float squareRotation = 0.0f;
float triangleRotation = 0.0f;
float circleScale = 1.0f;
bool circleGrowing = true;

// Breathing circles
struct BreathingCircle {
    float x, y;
    float color[3];
    float scale;
    bool growing;
};
std::vector<BreathingCircle> breathingCircles;

const float PI = 3.14159265358979323846f;

// Subwindow position and size in normalized coordinates
float subWindowX = 0.6f;
float subWindowY = 0.6f;
float subWindowSize = 0.3f;

// Flag to force window refresh
bool needsRefresh = false;

void drawBlackWhiteSquare() {
    glBegin(GL_QUADS);
    // Black half (left) - всегда черная
    glColor3f(0.0f, 0.0f, 0.0f);
    glVertex2f(-0.5f, -0.5f);
    glVertex2f(0.0f, -0.5f);
    glVertex2f(0.0f, 0.5f);
    glVertex2f(-0.5f, 0.5f);

    // White half (right) - использует выбранный цвет
    glColor3f(squareColor[0], squareColor[1], squareColor[2]);
    glVertex2f(0.0f, -0.5f);
    glVertex2f(0.5f, -0.5f);
    glVertex2f(0.5f, 0.5f);
    glVertex2f(0.0f, 0.5f);
    glEnd();
}

void drawEllipse() {
    glColor3f(0.8f, 0.8f, 0.2f); // Yellow color
    glBegin(GL_POLYGON);
    int segments = 50;
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(0.4f * cos(angle), 0.2f * sin(angle));
    }
    glEnd();
}

void drawCircle(float x, float y) {
    glColor3f(circleTriangleColor[0], circleTriangleColor[1], circleTriangleColor[2]);
    glBegin(GL_POLYGON);
    int segments = 50;
    for (int i = 0; i < segments; i++) {
        float angle = 2.0f * PI * i / segments;
        glVertex2f(x + 0.2f * circleScale * cos(angle), y + 0.2f * circleScale * sin(angle));
    }
    glEnd();
}

void drawTriangle(float x, float y) {
    glColor3f(circleTriangleColor[0], circleTriangleColor[1], circleTriangleColor[2]);
    glBegin(GL_TRIANGLES);
    glVertex2f(x - 0.2f, y - 0.2f);
    glVertex2f(x + 0.2f, y - 0.2f);
    glVertex2f(x, y + 0.2f);
    glEnd();
}

void updateAnimations() {
    if (!animationEnabled) return;

    // Square rotation (counter-clockwise)
    squareRotation += 1.5f;
    // squareRotation -= 1.0f;
    if (squareRotation > 360.0f) squareRotation -= 360.0f;
    // if (squareRotation < -360.0f) squareRotation += 360.0f;
    // Triangle rotation (clockwise)
    triangleRotation -= 1.0f;
    // triangleRotation += 1.5f;
    if (triangleRotation <  -360.0f) triangleRotation += 360.0f;
    // if (triangleRotation > 360.0f) triangleRotation -= 360.0f;
    // Circle breathing
    if (circleGrowing) {
        circleScale += 0.01f;
        if (circleScale >= 1.5f) circleGrowing = false;
    } else {
        circleScale -= 0.01f;
        if (circleScale <= 0.5f) circleGrowing = true;
    }

    // Update breathing circles
    for (auto& circle : breathingCircles) {
        if (circle.growing) {
            circle.scale += 0.02f;
            if (circle.scale >= 2.0f) circle.growing = false;
        } else {
            circle.scale -= 0.02f;
            if (circle.scale <= 0.5f) circle.growing = true;
        }
    }
}

// Menu callbacks
void mainMenuCallback(int option) {
    switch (option) {
        case 0: // Stop Animation
            animationEnabled = false;
            std::cout << "Animation Stopped" << std::endl;
            break;
        case 1: // Start Animation
            animationEnabled = true;
            std::cout << "Animation Started" << std::endl;
            break;
        case 2: // White
            squareColor[0] = squareColor[1] = squareColor[2] = 1.0f;
            std::cout << "Square Color: White" << std::endl;
            break;
        case 3: // Red
            squareColor[0] = 1.0f; squareColor[1] = 0.0f; squareColor[2] = 0.0f;
            std::cout << "Square Color: Red" << std::endl;
            break;
        case 4: // Green
            squareColor[0] = 0.0f; squareColor[1] = 1.0f; squareColor[2] = 0.0f;
            std::cout << "Square Color: Green" << std::endl;
            break;
    }
    needsRefresh = true;
}

void subWindowMenuCallback(int option) {
    switch (option) {
        case 0: // Red
            subWindowBgColor[0] = 1.0f; subWindowBgColor[1] = 0.0f; subWindowBgColor[2] = 0.0f;
            std::cout << "SubWindow Background: Red" << std::endl;
            break;
        case 1: // Green
            subWindowBgColor[0] = 0.0f; subWindowBgColor[1] = 1.0f; subWindowBgColor[2] = 0.0f;
            std::cout << "SubWindow Background: Green" << std::endl;
            break;
        case 2: // Blue
            subWindowBgColor[0] = 0.0f; subWindowBgColor[1] = 0.0f; subWindowBgColor[2] = 1.0f;
            std::cout << "SubWindow Background: Blue" << std::endl;
            break;
        case 3: // Yellow
            subWindowBgColor[0] = 1.0f; subWindowBgColor[1] = 1.0f; subWindowBgColor[2] = 0.0f;
            std::cout << "SubWindow Background: Yellow" << std::endl;
            break;
    }
    needsRefresh = true;
}

// Check if mouse click is in subwindow area
bool isInSubWindow(double x, double y, int windowWidth, int windowHeight) {
    // Convert to normalized coordinates
    float normX = (float)((x / windowWidth) * 2.0 - 1.0);
    float normY = (float)(1.0 - (y / windowHeight) * 2.0);

    // Subwindow bounds in normalized coordinates
    float subWindowLeft = subWindowX - subWindowSize;
    float subWindowRight = subWindowX + subWindowSize;
    float subWindowBottom = subWindowY - subWindowSize;
    float subWindowTop = subWindowY + subWindowSize;

    return (normX >= subWindowLeft && normX <= subWindowRight &&
            normY >= subWindowBottom && normY <= subWindowTop);
}

// Main window display (with black & white square)
void mainWindowDisplay() {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw single black & white square with rotation
    glPushMatrix();
    glRotatef(squareRotation, 0.0f, 0.0f, 1.0f);
    drawBlackWhiteSquare();
    glPopMatrix();

    // Draw subwindow area (fixed position in main window)
    glPushMatrix();
    glTranslatef(subWindowX, subWindowY, 0.0f);
    glScalef(subWindowSize, subWindowSize, 1.0f);

    // Subwindow background
    glBegin(GL_QUADS);
    glColor3f(subWindowBgColor[0], subWindowBgColor[1], subWindowBgColor[2]);
    glVertex2f(-1.0f, -1.0f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(1.0f, 1.0f);
    glVertex2f(-1.0f, 1.0f);
    glEnd();

    // Ellipse in subwindow
    drawEllipse();
    glPopMatrix();

    // Breathing circles
    for (const auto& circle : breathingCircles) {
        glPushMatrix();
        glTranslatef(circle.x, circle.y, 0.0f);
        glScalef(circle.scale, circle.scale, 1.0f);
        glColor3f(circle.color[0], circle.color[1], circle.color[2]);
        glBegin(GL_POLYGON);
        int segments = 50;
        for (int i = 0; i < segments; i++) {
            float angle = 2.0f * PI * i / segments;
            glVertex2f(0.1f * cos(angle), 0.1f * sin(angle));
        }
        glEnd();
        glPopMatrix();
    }

    glfwSwapBuffers(mainWindow);
}

// Second window display (circle and triangle)
void secondWindowDisplay() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1, 1, -1, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // Draw circle on the left side
    drawCircle(-0.5f, 0.0f);

    // Draw triangle on the right side with rotation
    glPushMatrix();
    glTranslatef(0.5f, 0.0f, 0.0f);
    glRotatef(triangleRotation, 0.0f, 0.0f, 1.0f);
    drawTriangle(0.0f, 0.0f);
    glPopMatrix();

    glfwSwapBuffers(secondWindow);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        // Color changes for circle and triangle (only in second window)
        if (window == secondWindow) {
            switch (key) {
                case GLFW_KEY_R:
                    circleTriangleColor[0] = 1.0f; circleTriangleColor[1] = 0.0f; circleTriangleColor[2] = 0.0f;
                    std::cout << "Circle/Triangle Color: Red" << std::endl;
                    needsRefresh = true;
                    break;
                case GLFW_KEY_G:
                    circleTriangleColor[0] = 0.0f; circleTriangleColor[1] = 1.0f; circleTriangleColor[2] = 0.0f;
                    std::cout << "Circle/Triangle Color: Green" << std::endl;
                    needsRefresh = true;
                    break;
                case GLFW_KEY_B:
                    circleTriangleColor[0] = 0.0f; circleTriangleColor[1] = 0.0f; circleTriangleColor[2] = 1.0f;
                    std::cout << "Circle/Triangle Color: Blue" << std::endl;
                    needsRefresh = true;
                    break;
                case GLFW_KEY_Y:
                    circleTriangleColor[0] = 1.0f; circleTriangleColor[1] = 1.0f; circleTriangleColor[2] = 0.0f;
                    std::cout << "Circle/Triangle Color: Yellow" << std::endl;
                    needsRefresh = true;
                    break;
                case GLFW_KEY_O:
                    circleTriangleColor[0] = 1.0f; circleTriangleColor[1] = 0.5f; circleTriangleColor[2] = 0.0f;
                    std::cout << "Circle/Triangle Color: Orange" << std::endl;
                    needsRefresh = true;
                    break;
                case GLFW_KEY_P:
                    circleTriangleColor[0] = 1.0f; circleTriangleColor[1] = 0.0f; circleTriangleColor[2] = 1.0f;
                    std::cout << "Circle/Triangle Color: Purple" << std::endl;
                    needsRefresh = true;
                    break;
                case GLFW_KEY_W:
                    circleTriangleColor[0] = 1.0f; circleTriangleColor[1] = 1.0f; circleTriangleColor[2] = 1.0f;
                    std::cout << "Circle/Triangle Color: White" << std::endl;
                    needsRefresh = true;
                    break;
            }
        }
    }
}

void mouseCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        if (window == mainWindow) {
            // Add breathing circle at mouse position
            double x, y;
            glfwGetCursorPos(window, &x, &y);

            int width, height;
            glfwGetWindowSize(window, &width, &height);

            // Check if click is in subwindow area (avoid creating circles in subwindow)
            if (isInSubWindow(x, y, width, height)) {
                return; // Click in subwindow area, don't create circle
            }

            // Convert to normalized coordinates
            float normX = (float)((x / width) * 2.0 - 1.0);
            float normY = (float)(1.0 - (y / height) * 2.0);

            BreathingCircle circle;
            circle.x = normX;
            circle.y = normY;
            circle.scale = 0.5f;
            circle.growing = true;

            // Random color
            static std::random_device rd;
            static std::mt19937 gen(rd());
            std::uniform_real_distribution<float> dis(0.0f, 1.0f);

            circle.color[0] = dis(gen);
            circle.color[1] = dis(gen);
            circle.color[2] = dis(gen);

            breathingCircles.push_back(circle);
            std::cout << "Added breathing circle at (" << normX << ", " << normY << ")" << std::endl;
            needsRefresh = true;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        if (window == mainWindow) {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            int width, height;
            glfwGetWindowSize(window, &width, &height);

            if (isInSubWindow(x, y, width, height)) {
                // Right click in subwindow - show subwindow menu
                std::cout << "\n=== SubWindow Menu ===" << std::endl;
                std::cout << "1. Red Background" << std::endl;
                std::cout << "2. Green Background" << std::endl;
                std::cout << "3. Blue Background" << std::endl;
                std::cout << "4. Yellow Background" << std::endl;
                std::cout << "Choose an option (1-4): ";

                int option;
                std::cin >> option;
                if (option >= 1 && option <= 4) {
                    subWindowMenuCallback(option - 1);
                    needsRefresh = true; // Добавьте эту строку
                }
            } else {
                // Right click in main window - show main menu
                std::cout << "\n=== Main Window Menu ===" << std::endl;
                std::cout << "1. Stop Animation" << std::endl;
                std::cout << "2. Start Animation" << std::endl;
                std::cout << "3. Square Color: White" << std::endl;
                std::cout << "4. Square Color: Red" << std::endl;
                std::cout << "5. Square Color: Green" << std::endl;
                std::cout << "Choose an option (1-5): ";

                int option;
                std::cin >> option;
                if (option >= 1 && option <= 5) {
                    mainMenuCallback(option - 1);
                    needsRefresh = true; // И эту строку тоже
                }
            }
        }
    }
}

void printInstructions() {
    std::cout << "=== Assignment 2 Instructions ===" << std::endl;
    std::cout << "Main Window:" << std::endl;
    std::cout << "  - Single black & white rotating square (left-black, right-white)" << std::endl;
    std::cout << "  - Subwindow with ellipse (top-right)" << std::endl;
    std::cout << "  - Right click in subwindow: Change background color" << std::endl;
    std::cout << "  - Right click outside subwindow: Main menu" << std::endl;
    std::cout << "  - Left click: Add breathing circle" << std::endl;
    std::cout << std::endl;
    std::cout << "Second Window (Circle & Triangle):" << std::endl;
    std::cout << "  R - Red, G - Green, B - Blue" << std::endl;
    std::cout << "  Y - Yellow, O - Orange, P - Purple, W - White" << std::endl;
    std::cout << "=================================" << std::endl;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Enable double buffering
    glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

    // Create main window
    mainWindow = glfwCreateWindow(800, 600, "Main Window - Black & White Square + SubWindow", nullptr, nullptr);
    if (!mainWindow) {
        std::cerr << "Failed to create main window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Create second window for circle and triangle
    secondWindow = glfwCreateWindow(400, 400, "Window 2 - Circle & Triangle", nullptr, nullptr);
    if (!secondWindow) {
        std::cerr << "Failed to create second window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Position windows
    glfwSetWindowPos(mainWindow, 100, 100);
    glfwSetWindowPos(secondWindow, 950, 100);

    // Set callbacks
    glfwSetKeyCallback(secondWindow, keyboardCallback);
    glfwSetMouseButtonCallback(mainWindow, mouseCallback);

    // Print instructions
    printInstructions();

    // Main loop
    while (!glfwWindowShouldClose(mainWindow) && !glfwWindowShouldClose(secondWindow)) {
        updateAnimations();

        // Force refresh if needed
        if (needsRefresh) {
            needsRefresh = false;
        }

        // Render main window
        glfwMakeContextCurrent(mainWindow);
        mainWindowDisplay();

        // Render second window
        glfwMakeContextCurrent(secondWindow);
        secondWindowDisplay();

        glfwPollEvents();
    }

    glfwDestroyWindow(mainWindow);
    glfwDestroyWindow(secondWindow);
    glfwTerminate();

    return 0;
}