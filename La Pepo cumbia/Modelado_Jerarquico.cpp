#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Shaders
#include "Shader.h"

void Inputs(GLFWwindow* window);

const GLint WIDTH = 1200, HEIGHT = 800;

// Variables de Camara
float movX = 0.0f;
float movY = 0.0f;
float movZ = -5.0f;
float rot = 0.0f;

// Variables de Articulaciones
float hombro = 0.0f;
float codo = 0.0f;
float mano = 0.0f;

// Variables para cada falange por separado (6 variables)
float pulgar1 = 0.0f, pulgar2 = 0.0f;
float indice1 = 0.0f, indice2 = 0.0f;
float medio1 = 0.0f, medio2 = 0.0f;

int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Modelado Jerarquico - 3 Dedos Articulados", nullptr, nullptr);

	int screenWidth, screenHeight;
	glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

	if (nullptr == window) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;

	if (GLEW_OK != glewInit()) {
		std::cout << "Failed to initialise GLEW" << std::endl;
		return EXIT_FAILURE;
	}

	glViewport(0, 0, screenWidth, screenHeight);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader ourShader("Shader/core.vs", "Shader/core.frag");

	float vertices[] = {
		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
	};

	GLuint VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);
	glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);

	while (!glfwWindowShouldClose(window)) {
		Inputs(window);
		glfwPollEvents();

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.Use();
		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 view = glm::mat4(1.0f);
		glm::mat4 modelTemp = glm::mat4(1.0f);
		glm::mat4 modelTemp2 = glm::mat4(1.0f);

		view = glm::translate(view, glm::vec3(movX, movY, movZ));
		view = glm::rotate(view, glm::radians(rot), glm::vec3(0.0f, 1.0f, 0.0f));

		GLint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLint projecLoc = glGetUniformLocation(ourShader.Program, "projection");
		GLint uniformColor = ourShader.uniformColor;

		glUniformMatrix4fv(projecLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		glBindVertexArray(VAO);

		// 1. BRAZO (Hombro)
		model = glm::rotate(model, glm::radians(hombro), glm::vec3(0.0f, 0.0, 1.0f));
		modelTemp = model = glm::translate(model, glm::vec3(1.5f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f, 1.0f, 1.0f));
		color = glm::vec3(0.0f, 1.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// 2. ANTEBRAZO (Codo)
		model = glm::translate(modelTemp, glm::vec3(1.5f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(codo), glm::vec3(0.0f, 1.0, 0.0f));
		modelTemp = model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(2.0f, 1.0f, 1.0f));
		color = glm::vec3(1.0f, 0.0f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// 3. PALMA DE LA MANO
		model = glm::translate(modelTemp, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(mano), glm::vec3(0.0f, 1.0f, 0.0f));
		modelTemp2 = modelTemp = model = glm::translate(model, glm::vec3(0.25f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f, 1.0f, 1.0f));
		color = glm::vec3(0.0f, 0.0f, 1.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
				// 4. DEDO PULGAR 
				// Articulación Base (Falange 1)
		model = glm::translate(modelTemp2, glm::vec3(-0.05f, 0.0f, -0.4f));
		model = glm::rotate(model, glm::radians(pulgar1), glm::vec3(0.0f, 0.0f, 1.0f));

		// Guardamos la articulación intermedia para la falange 2 (en su extremo, a 0.3f)
		modelTemp = glm::translate(model, glm::vec3(0.3f, 0.0f, 0.0f));

		// Dibujamos Falange 1
		model = glm::translate(model, glm::vec3(0.15f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.15f, 0.15f));
		color = glm::vec3(1.0f, 1.0f, 0.0f); // Amarillo
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Falange 2 (Punta) conectada a la articulación intermedia
		model = glm::rotate(modelTemp, glm::radians(pulgar2), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.12f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.24f, 0.13f, 0.13f));
		color = glm::vec3(0.85f, 0.85f, 0.0f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 5. DEDO INDICE
		// Articulación Base (Falange 1)
		model = glm::translate(modelTemp2, glm::vec3(0.25f, 0.0f, -0.2f));
		model = glm::rotate(model, glm::radians(indice1), glm::vec3(0.0f, 0.0f, 1.0f));

		// Guardamos la articulación intermedia para la falange 2 (en su extremo, a 0.3f)
		modelTemp = glm::translate(model, glm::vec3(0.3f, 0.0f, 0.0f));

		// Dibujamos Falange 1
		model = glm::translate(model, glm::vec3(0.15f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.3f, 0.15f, 0.15f));
		color = glm::vec3(0.0f, 1.0f, 1.0f); // Cian
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Falange 2 (Punta) conectada a la articulación intermedia
		model = glm::rotate(modelTemp, glm::radians(indice2), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.12f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.24f, 0.13f, 0.13f));
		color = glm::vec3(0.0f, 0.8f, 0.8f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		// 6. DEDO MEDIO
		// Articulación Base (Falange 1)
		model = glm::translate(modelTemp2, glm::vec3(0.25f, 0.0f, 0.2f));
		model = glm::rotate(model, glm::radians(medio1), glm::vec3(0.0f, 0.0f, 1.0f));

		// Guardamos la articulación intermedia para la falange 2 (en su extremo, a 0.36f)
		modelTemp = glm::translate(model, glm::vec3(0.36f, 0.0f, 0.0f));

		// Dibujamos Falange 1
		model = glm::translate(model, glm::vec3(0.18f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.36f, 0.15f, 0.15f));
		color = glm::vec3(1.0f, 0.0f, 1.0f); // Magenta
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Falange 2 (Punta) conectada a la articulación intermedia
		model = glm::rotate(modelTemp, glm::radians(medio2), glm::vec3(0.0f, 0.0f, 1.0f));
		model = glm::translate(model, glm::vec3(0.14f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.28f, 0.13f, 0.13f));
		color = glm::vec3(0.8f, 0.0f, 0.8f);
		glUniform3fv(uniformColor, 1, glm::value_ptr(color));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glBindVertexArray(0);
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return EXIT_SUCCESS;
}

void Inputs(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// Camara
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) movX += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) movX -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) movY += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) movY -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) movZ -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) movZ += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) rot += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) rot -= 0.008f;

	// Hombro, Codo y Muneca
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) hombro += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) hombro -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) codo += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) codo -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) mano += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) mano -= 0.008f;

	// PULGAR (U/J para falange base, M/N para punta)
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) pulgar1 += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) pulgar1 -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) pulgar2 += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) pulgar2 -= 0.008f;

	// INDICE (I/K para falange base, 8/9 para punta)
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) indice1 += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) indice1 -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_8) == GLFW_PRESS) indice2 += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) indice2 -= 0.008f;

	// MEDIO (O/L para falange base, 0/P para punta)
	if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) medio1 += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) medio1 -= 0.008f;
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) medio2 += 0.008f;
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) medio2 -= 0.008f;
}