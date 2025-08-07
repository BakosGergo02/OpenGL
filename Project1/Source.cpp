#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include <array>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#define PI 3.14159265359

using namespace std;

#define numVAOs 1

GLuint VBO;
GLuint VAO;

std::vector<glm::vec3> verticesBuffer;
GLfloat centerx = 0;
GLfloat centery = 0;
GLfloat radius = 0.14;
GLdouble deltaTime = 0;
GLboolean direction = true;
void updateVertexData()
{
	/*Kirajzoljuk az egyenest.*/
	verticesBuffer.clear();
	verticesBuffer.push_back(glm::vec3(0.33, centery, 0));
	verticesBuffer.push_back(glm::vec3(-0.33, centery, 0));

	/*Kirajzoljuk a kört.*/
	for (int i = 0; i < 100; i++)
	{
		verticesBuffer.push_back(
			glm::vec3(
				centerx + radius * cos(i * (2 * PI / 100)),
				centery + radius * sin(i * (2 * PI / 100)),
				0
			));
	}
}
/*VBO frissítése*/
void updateVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesBuffer.size() * sizeof(glm::vec3), verticesBuffer.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

GLuint renderingProgram;
GLuint vao[numVAOs];

bool checkOpenGLError() {
	bool foundError = false;
	int glErr = glGetError();
	while (glErr != GL_NO_ERROR) {
		cout << "glError: " << glErr << endl;
		foundError = true;
		glErr = glGetError();
	}
	return foundError;
}

void printShaderLog(GLuint shader) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetShaderInfoLog(shader, len, &chWrittn, log);
		cout << "Shader Info Log: " << log << endl;
		free(log);
	}
}

void printProgramLog(int prog) {
	int len = 0;
	int chWrittn = 0;
	char* log;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &len);
	if (len > 0) {
		log = (char*)malloc(len);
		glGetProgramInfoLog(prog, len, &chWrittn, log);
		cout << "Program Info Log: " << log << endl;
		free(log);
	}
}
/** Függvény, a shader fájlok sorainak beolvasásához. */
string readShaderSource(const char* filePath) {
	string content;
	ifstream fileStream(filePath, ios::in);
	string line = "";

	while (!fileStream.eof()) {
		getline(fileStream, line);
		content.append(line + "\n");
	}
	fileStream.close();
	return content;
}

GLuint createShaderProgram() {

	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

	/* Beolvassuk a shader fájlok tartalmát. */
	string vertShaderStr = readShaderSource("vertexShader.glsl");
	string fragShaderStr = readShaderSource("fragmentShader.glsl");

	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);

	const char* vertShaderSrc = vertShaderStr.c_str();
	const char* fragShaderSrc = fragShaderStr.c_str();

	glShaderSource(vShader, 1, &vertShaderSrc, NULL);
	glShaderSource(fShader, 1, &fragShaderSrc, NULL);

	glCompileShader(vShader);
	checkOpenGLError();
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &vertCompiled);
	if (vertCompiled != 1) {
		cout << "vertex compilation failed" << endl;
		printShaderLog(vShader);
	}


	glCompileShader(fShader);
	checkOpenGLError();
	glGetShaderiv(fShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	// Shader program objektum létrehozása. Eltároljuk az ID értéket.
	GLuint vfProgram = glCreateProgram();
	glAttachShader(vfProgram, vShader);
	glAttachShader(vfProgram, fShader);

	glLinkProgram(vfProgram);
	checkOpenGLError();
	glGetProgramiv(vfProgram, GL_LINK_STATUS, &linked);
	if (linked != 1) {
		cout << "linking failed" << endl;
		printProgramLog(vfProgram);
	}

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	return vfProgram;
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key) {
		case GLFW_KEY_UP:
			centery += 0.1;
			break;
		case GLFW_KEY_DOWN:
			centery -= 0.1;
			break;
		}
	}
	updateVertexData();
	updateVBO();
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos)
{

}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

}

void init() {
	renderingProgram = createShaderProgram();

	/* Létrehozzuk a szükséges Vertex buffer és vertex array objektumot. */
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	/* Típus meghatározása: a GL_ARRAY_BUFFER nevesített csatolóponthoz kapcsoljuk a buffert (ide kerülnek a vertex adatok). */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/* Másoljuk az adatokat a pufferbe! Megadjuk az aktuálisan csatolt puffert,  azt hogy hány bájt adatot másolunk,
	a másolandó adatot, majd a feldolgozás módját is meghatározzuk: most az adat nem változik a feltöltés után */
	//glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);
	glBufferData(GL_ARRAY_BUFFER, verticesBuffer.size() * sizeof(glm::vec3), verticesBuffer.data(), GL_STATIC_DRAW);

	/* A puffer kész, lecsatoljuk, már nem szeretnénk módosítani. */
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Csatoljuk a vertex array objektumunkat a konfiguráláshoz. */
	glBindVertexArray(VAO);

	/* Vertex buffer objektum újracsatolása. */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/* Ezen adatok szolgálják a 0 indexû vertex attribútumot (itt: pozíció).
	Elsõként megadjuk ezt az azonosítószámot.
	Utána az attribútum méretét (vec3, láttuk a shaderben).
	Harmadik az adat típusa.
	Negyedik az adat normalizálása, ez maradhat FALSE jelen példában.
	Az attribútum értékek hogyan következnek egymás után? Milyen lépésköz után találom a következõ vertex adatait?
	Végül megadom azt, hogy honnan kezdõdnek az értékek a pufferben. Most rögtön, a legelejétõl veszem õket.*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	/* Engedélyezzük az imént definiált 0 indexû attribútumot. */
	glEnableVertexAttribArray(0);

	/* Leválasztjuk a vertex array objektumot és a puffert is.*/
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}
/** A jelenetünk utáni takarítás. */
void cleanUpScene()
{
	/** Töröljük a vertex puffer és vertex array objektumokat. */
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	/** Töröljük a shader programot. */
	glDeleteProgram(renderingProgram);
}

void display(GLFWwindow* window, double currentTime) {


	/* Töröljük le a kiválasztott buffereket! Fontos lehet minden egyes alkalommal törölni! */
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1, 1, 0, 1.0);
	/*Aktiváljuk a shader - program objektumunkat.*/
	glUseProgram(renderingProgram);

	/*Csatoljuk a vertex array objektumunkat. */
	glBindVertexArray(VAO);
	unsigned int isLine = glGetUniformLocation(renderingProgram, "isLine");
	unsigned int center = glGetUniformLocation(renderingProgram, "center");

	float centerValues[] = { centerx * 300, centery * 300, 0 };
	glUniform3fv(center, 1, centerValues);

	glPointSize(25.0);
	glLineWidth(3.0);
	int offset = 0;
	glUniform1i(isLine, true);
	glDrawArrays(GL_LINE_LOOP, offset, 2);
	offset += 2;
	glUniform1i(isLine, false);
	glLineWidth(1.0);
	glDrawArrays(GL_TRIANGLE_FAN, offset, 100);
	offset += 100;

	if (currentTime - 0.05f > deltaTime) {
		deltaTime = currentTime;
		if (centerx + radius > 1 || centerx - radius < -1) {
			direction = !direction;
		}
		if (direction) {
			centerx += 0.05f;
		}
		else
		{
			centerx -= 0.05f;
		}
		updateVertexData();
		updateVBO();
	}
	/* Leválasztjuk, nehogy bármilyen érték felülíródjon.*/
	glBindVertexArray(0);

}

int main(void) {

	updateVertexData();
	/* Próbáljuk meg inicializálni a GLFW-t! */
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	/* A kívánt OpenGL verzió (4.3) */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	/* Próbáljuk meg létrehozni az ablakunkat.*/
	GLFWwindow* window = glfwCreateWindow(600, 600, "beadando1", NULL, NULL);

	/* Válasszuk ki az ablakunk OpenGL kontextusát, hogy használhassuk. */
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);


	/* Incializáljuk a GLEW-t, hogy elérhetõvé váljanak az OpenGL függvények.*/
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	/* Az alkalmazáshoz kapcsolódó elõkészítõ lépések, pl. hozd létre a shader objektumokat.*/
	init();


	while (!glfwWindowShouldClose(window)) {
		/* a kód, amellyel rajzolni tudunk a GLFWwindow ojektumunkba.*/
		display(window, glfwGetTime());
		/* double buffered */
		glfwSwapBuffers(window);
		/* események kezelése az ablakunkkal kapcsolatban, pl. gombnyomás*/
		glfwPollEvents();
	}

	/** Felesleges objektumok törlése. */
	cleanUpScene();
	/** Töröljük a GLFW ablakot. */
	glfwDestroyWindow(window);
	/** Leállítjuk a GLFW-t */
	glfwTerminate();

	return EXIT_SUCCESS;
}
