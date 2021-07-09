#include <math.h>
#include <vector>
#include <time.h>

#include <glutil.h>
#include <figures.h>
#include <camera.h>

#include <files.hpp>
#include <model.hpp>


i32 n = 11;

bool shoot = false;

const u32 FSIZE = sizeof(f32);
const u32 ISIZE = sizeof(u32);
const u32 SCRWIDTH = 1280;
const u32 SCRHEIGHT = 720;
const f32 ASPECT = (f32)SCRWIDTH / (f32)SCRHEIGHT;

glm::vec3 lightPos(50.0f, 20.0f, 100.0f);
glm::vec3 posNave = glm::vec3(n / 2.0, n / 2.0, 18.0f);
glm::vec3 posBola = posNave;

Cam* cam;

f32 lastx;
f32 lasty;
f32 deltaTime = 0.0f;
f32 lastFrame = 0.0f;
bool firstmouse = true;
bool wireframe = false;

/**
 * keyboard input processing
 **/
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
	//movimineto de la camara
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		cam->processKeyboard(FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		cam->processKeyboard(LEFT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		cam->processKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		cam->processKeyboard(RIGHT, deltaTime);
	}

	//movimineto de la nave
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		if (posNave.y + deltaTime * 5 <= n - 1) {
			posNave.y += deltaTime * 5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		if (posNave.y - deltaTime * 5 >= 0) {
			posNave.y -= deltaTime * 5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (posNave.x - deltaTime * 5 >= 0) {
			posNave.x -= deltaTime * 5;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (posNave.x + deltaTime * 5 < n - 1) {
			posNave.x += deltaTime * 5;
		}
	}
	//disparar
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		shoot = true;
	}
}

void mouse_callback(GLFWwindow* window, f64 xpos, f64 ypos) {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
		if (firstmouse) {
			lastx = xpos;
			lasty = ypos;
			firstmouse = false;
			return;
		}
		cam->processMouse((f32)(xpos - lastx), (f32)(lasty - ypos));
		lastx = xpos;
		lasty = ypos;
	}
	else {
		firstmouse = true;
	}
}

void scroll_callback(GLFWwindow* window, f64 xoffset, f64 yoffset) {
	cam->processScroll((f32)yoffset);
}

i32 main() {
	srand(time(NULL));
	//crear elementos base
	GLFWwindow* window = glutilInit(3, 3, SCRWIDTH, SCRHEIGHT, "Space Invaders");
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	Shader* lightingShader = new Shader("shader.vert", "shader.frag");
	Shader* lightCubeShader = new Shader("shader2.vert", "shader2.frag");

	Cube* cubex = new Cube();
	cam = new Cam();
	glm::vec3 lightColor = glm::vec3(1.0f);

	Files* files = new Files("resources/shaders", "resources/textures", "resources/objects");
	ShaderModel* shader = new ShaderModel(files, "shader_modelo.vert", "shader_modelo.frag");
	Model* obj1 = new Model(files, "Spaceship/Spaceship.obj");
	Model* obj2 = new Model(files, "Allien/Allien.obj");


	int count = 0;
	std::vector<glm::vec3> posAl(n * n * 5);
	std::vector<bool> est(n * n * 5);
	for (u32 i = 0; i < n; ++i) {
		for (u32 j = 0; j < 5; ++j) {
			for (u32 k = 0; k < n; ++k) {
				f32 x = i + 0.05 * i;
				f32 z = j + 0.05 * j;
				f32 y = k + 0.05 * k;
				posAl[count] = glm::vec3(x, y, z);
				est[count] = true;
				count++;
			}
		}
	}

	//definir velocidad bala
	glm::vec3 veloc = glm::vec3(0, 0, -0.2);

	//configuraciones
	u32 cubeVao, lightCubeVao, vbo, ebo;
	glGenVertexArrays(1, &cubeVao);
	glGenVertexArrays(1, &lightCubeVao);
	glGenBuffers(1, &vbo);
	glGenBuffers(1, &ebo);

	glBindVertexArray(cubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glBufferData(GL_ARRAY_BUFFER, cubex->getVSize() * FSIZE,
		cubex->getVertices(), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubex->getISize() * ISIZE,
		cubex->getIndices(), GL_STATIC_DRAW);

	// posiciones
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);
	// normales: ojo que es el 3er comp, por eso offset es 6
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(6));
	glEnableVertexAttribArray(1);
	// textures
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(9));
	glEnableVertexAttribArray(2);

	glBindVertexArray(lightCubeVao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, cubex->len(), cubex->skip(0));
	glEnableVertexAttribArray(0);

	glEnable(GL_DEPTH_TEST);

	unsigned int texture1 = lightingShader->loadTexture("texture1.jpg");

	//definir variables del movimiento y posicion de los asteroides
	f32 invXos = 0.0;
	f32 invX = 0.06;
	f32 invZ = 0.003;
	f32 invmaxZ = -1.0;
	while (!glfwWindowShouldClose(window)) {
		f32 currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		processInput(window);
		glClearColor(0.04f, 0.04f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//iluminacion
		lightPos.x = sin(currentFrame / 16) * 30;
		lightPos.z = cos(currentFrame / 16) * 30;

		glm::mat4 proj = glm::perspective(cam->getZoom(), ASPECT, 0.1f, 100.0f);
		lightingShader->useProgram();
		glm::mat4 model = glm::mat4(1.0f);
		glBindVertexArray(cubeVao);

	
		i32 sEst = 0;



		//movimiento aliens en X
		if (abs(invXos) >= 2.0) invX *= -1;
		invXos += invX;
		for (u32 i = 0; i < posAl.size(); ++i) {
			sEst += int(est[i]);

			//si esta muerto(estado=falso) no se dibuja ni se detecta colision
			if (!est[i]) continue;

			//maxima posicion en Z para detectar derrota
			if (invmaxZ < posAl[i].z) invmaxZ = posAl[i].z;

			//movimiento aliens en Z
			posAl[i].z += invZ;
			//movimiento aliens en X
			glm::vec3 posicionaux = posAl[i];
			posicionaux.x += invXos;

			if (int(posAl[i].x) % 3 == 1) {
				glBindTexture(GL_TEXTURE_2D, texture1);
			}

			lightingShader->setVec3("xyzMat.specular", 0.5f, 0.5f, 0.5f);
			lightingShader->setF32("xyzMat.shininess", 64.0f);

			lightingShader->setVec3("xyzLht.position", lightPos);
			lightingShader->setVec3("xyz", cam->getPos());

			lightingShader->setVec3("xyzLht.ambient", 0.2f, 0.2f, 0.2f);
			lightingShader->setVec3("xyzLht.diffuse", 0.5f, 0.5f, 0.5f);
			lightingShader->setVec3("xyzLht.specular", 1.0f, 1.0f, 1.0f);

			lightingShader->setMat4("proj", proj);
			lightingShader->setMat4("view", cam->getViewM4());

			//dibujar aliens
			model = glm::mat4(1.0f);
			model = glm::translate(model, posicionaux);
			lightingShader->setMat4("model", model);
			glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);

							
			bool colx = (posicionaux.x + 0.5 > posBola.x - 0.05 && posicionaux.x - 0.5 < posBola.x + 0.05);
			bool coly = (posicionaux.y + 0.5 > posBola.y - 0.05 && posicionaux.y - 0.5 < posBola.y + 0.05);
			bool colz = (posicionaux.z + 0.5 > posBola.z - 0.05 && posicionaux.z - 0.5 < posBola.z + 0.05);

			//respuesta de colision									
			if (colx && coly && colz) {
				est[i] = false;
				shoot = false;
			}
		}



		//dibujar nave	
		shader->use();
		shader->setVec3("xyz", lightPos);
		shader->setVec3("xyzColor", lightColor);
		shader->setVec3("xyzView", cam->getPos());

		shader->setMat4("proj", proj);
		shader->setMat4("view", cam->getViewM4());

		model = glm::mat4(1.0f);
		model = glm::translate(model, posNave);
		model = glm::scale(model, glm::vec3(1.0 / 2));
		model = glm::rotate(model, 3.14f, glm::vec3(0.0, 1.0, 0.0));
		shader->setMat4("model", model);
		obj2->Draw(shader);

		//dibujar bala
		if (!shoot) {
			//si no esta disparada: esta con la Nave
			posBola = posNave;
		}
		else {
			//si esta disparada: se mueve en Z
			posBola.z += veloc.z;
		}

		//si se pasa del espacio de juego: deja de estar disparada
		if (posBola.z <= -1) shoot = false;

		shader->use();
		shader->setVec3("xyz", lightPos);
		shader->setVec3("xyzColor", lightColor);
		shader->setVec3("xyzView", cam->getPos());

		shader->setMat4("proj", proj);
		shader->setMat4("view", cam->getViewM4());

		model = glm::mat4(1.0f);
		model = glm::translate(model, posBola);
		model = glm::scale(model, glm::vec3(1.0 / 20));
		shader->setMat4("model", model);
		obj1->Draw(shader);

		//luz
		lightCubeShader->useProgram();
		lightCubeShader->setMat4("proj", proj);
		lightCubeShader->setMat4("view", cam->getViewM4());
		model = glm::mat4(1.0f);
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.05));
		lightCubeShader->setMat4("model", model);

		glBindVertexArray(lightCubeVao);
		glDrawElements(GL_TRIANGLES, cubex->getISize(), GL_UNSIGNED_INT, 0);


		//si no hay aliens: ganas y se cierra la ventana
		//si los aliens llegan a tu ubicacion: pierdes y se cierra la ventan
		if (sEst == 0 || invmaxZ >= posNave.z - 2.0) {
			glfwSetWindowShouldClose(window, true);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	};

	glDeleteVertexArrays(1, &cubeVao);
	glDeleteVertexArrays(1, &lightCubeVao);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &ebo);

	delete lightingShader;
	delete lightCubeShader;
	delete cubex;
	delete cam;
	delete shader;
	delete obj1;
	delete obj2;

	return 0;
}