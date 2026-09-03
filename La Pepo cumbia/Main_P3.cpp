#include <iostream>

// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shaders
#include "Shader.h"

const GLint WIDTH = 800, HEIGHT = 600;

void drawCube(GLint modelLoc, glm::vec3 position, float angleInDegrees, glm::vec3 rotationAxis, glm::vec3 scale) {
    glm::mat4 model = glm::mat4(1.0f);

    // 1. Traslación
    model = glm::translate(model, position);

    // 2. Rotación
    if (angleInDegrees != 0.0f) {
        model = glm::rotate(model, glm::radians(angleInDegrees), rotationAxis);
    }

    // 3. Escala
    model = glm::scale(model, scale);

    // Enviar matriz model al shader y dibujar
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

int main() {
    glfwInit();

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Proyecciones y transformaciones basicas", nullptr, nullptr);

    int screenWidth, screenHeight;
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    // Verificación de errores de creación de ventana
    if (nullptr == window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;

    // Verificación de errores de inicialización de GLEW
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialise GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Define las dimensiones del viewport
    glViewport(0, 0, screenWidth, screenHeight);

    // Opciones de OpenGL
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Compilación del shader program
    Shader ourShader("Shader/core.vs", "Shader/core.frag");

    // Vértices del cubo (posición y color RGB)
    float vertices[] = {
        // Front
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,

        // Back
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,

        // Right
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,

         // Left
         -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,
         -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,
         -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 0.0f,

         // Bottom
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
          0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
          0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 1.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 1.0f,

         // Top
         -0.5f,  0.5f, -0.5f,  1.0f, 0.2f, 0.5f,
          0.5f,  0.5f, -0.5f,  1.0f, 0.2f, 0.5f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.2f, 0.5f,
          0.5f,  0.5f,  0.5f,  1.0f, 0.2f, 0.5f,
         -0.5f,  0.5f,  0.5f,  1.0f, 0.2f, 0.5f,
         -0.5f,  0.5f, -0.5f,  1.0f, 0.2f, 0.5f
    };

    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    // Enlazar VAO y VBO
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Atributo 0: Posición
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Matriz de proyección en perspectiva
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Limpiar buffers
        glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.Use();

        // Matriz de vista (cámara en Z = -10 mirando al origen)
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0.0f, 0.0f, -10.0f));

        // Obtener locations de los uniforms
        GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
        GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
        GLint projecLoc = glGetUniformLocation(ourShader.Program, "projection");

        // Enviar matrices globales de proyección y vista
        glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(VAO);

        drawCube(modelLoc,
            glm::vec3(-2.5f, 1.8f, 0.0f),
            45.0f, glm::vec3(0.0f, 1.0f, 0.0f),
            glm::vec3(0.8f, 0.8f, 0.8f));

        drawCube(modelLoc,
            glm::vec3(2.5f, 1.8f, -1.0f),
            30.0f, glm::vec3(1.0f, 0.0f, 1.0f),
            glm::vec3(0.6f, 1.6f, 0.6f));

        drawCube(modelLoc,
            glm::vec3(-2.2f, -1.8f, 0.5f),
            60.0f, glm::vec3(1.0f, 1.0f, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f));

        drawCube(modelLoc,
            glm::vec3(2.2f, -1.8f, 1.0f),
            -100.0f, glm::vec3(0.0f, 0.0f, 1.0f),
            glm::vec3(1.8f, 0.4f, 3.2f));

        glBindVertexArray(0);

        glfwSwapBuffers(window);
    }

    // Liberar recursos
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return EXIT_SUCCESS;
}