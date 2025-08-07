#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "glm/glm.hpp"

#include <numbers>
#include <array>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <math.h>

#define PI 3.14159265358979323846

using namespace std;

static int WIN_WIDTH = 600;
static int WIN_HEIGHT = 600;

bool isDragging = false;
double dragStartX, dragStartY;


std::vector<glm::vec3> pointsToDraw;
typedef struct Circle {
	GLint VertexCount = 0;
	std::vector<glm::vec3> Points = {};
	float color[3] = { 0.0, 1.0, 0.0 };
	GLfloat centerX;
	GLfloat centerY;
	GLfloat radius;
	Circle(GLfloat posX, GLfloat posY, GLfloat radius, GLint vertexCount) {
		centerX = posX;
		centerY = posY;
		this->radius = radius;
		VertexCount = vertexCount;
	}
	void drawCircle()
	{
		for (int i = 0; i < VertexCount; i++)
		{
			pointsToDraw.push_back(
				glm::vec3(
					centerX + radius * cos(i * (2 * PI / VertexCount)),
					centerY + radius * sin(i * (2 * PI / VertexCount)),
					0
				));
		}
	}
}Circle;

typedef struct BezierCurve {
	GLint VertexCount = 0;
	std::vector<glm::vec3> Points = {};
	std::vector<Circle> circlePoints = {};
	GLint BlendingVertexCount = 0.0f;
	float color[3] = { 0.0, 1.0, 0.0 };
	BezierCurve(GLint vertexCount) {
		VertexCount = vertexCount;
	}
	float blending(GLfloat i, GLfloat t, GLfloat count)
	{
		return ((i == 0 || i == count - 1) ? 1 : count - 1) * powf(t, i) * powf((1 - t), count - 1 - i);
	}

	glm::vec3 GetPoint(GLint i) {
		if (i >= Points.size()) {
			Points.push_back(glm::vec3(0, 0, 0));
		}
		return Points.at(i);
	}
	void drawBezierCurve()
	{
		glm::vec3 nextPoint;
		GLfloat t = 0.0f;
		GLfloat increment = 1.0f / 100.0f; /* h�ny darab szakaszb�l rakjuk �ssze a g�rb�nket? */
		BlendingVertexCount = 0;
		while (t <= 1.0f)
		{
			nextPoint = glm::vec3(0.0f, 0.0f, 0.0f);

			for (int j = 0; j < VertexCount; j++)
			{
				nextPoint.x = nextPoint.x + (blending((GLfloat)j, t, (GLfloat)VertexCount) * GetPoint(j).x);
				nextPoint.y = nextPoint.y + (blending((GLfloat)j, t, (GLfloat)VertexCount) * GetPoint(j).y);
				nextPoint.z = nextPoint.z + (blending((GLfloat)j, t, (GLfloat)VertexCount) * GetPoint(j).z);
			}
			pointsToDraw.push_back(glm::vec3(nextPoint.x, nextPoint.y, nextPoint.z));
			BlendingVertexCount++;
			t += increment;
		}
		for (GLint j = 0; j < VertexCount; j++)
		{
			pointsToDraw.push_back(GetPoint(j));
		}
		for (GLint j = 0; j < VertexCount; j++)
		{
			glm::vec3 controlPoint = GetPoint(j);
			Circle point = Circle(controlPoint.x, controlPoint.y, 0.03, 12);
			point.drawCircle();
			circlePoints.push_back(point);
		}
	}
}BezierCurve;

std::vector<BezierCurve> beziers;

float pointColor[3] = { 1.0, 0.0, 0.0 };
float lineColor[3] = { 0.0, 0.0, 1.0 };
float activePoint[3] = { 0.5, 0.5, 0.5 };
/* Vertex buffer objektum �s vertex array objektum az adatt�rol�shoz.*/
GLuint VBO;
GLuint VAO;

GLuint renderingProgram;
GLint loc;

void UpdateVertexData() {
	pointsToDraw.clear();
	for (int i = 0; i < beziers.size(); i++)
	{
		beziers.at(i).drawBezierCurve();
	}
}
void UpdateVBO() {
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, pointsToDraw.size() * sizeof(glm::vec3), pointsToDraw.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

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

GLuint createShaderProgram(const char* fragmentFile) {

	GLint vertCompiled;
	GLint fragCompiled;
	GLint linked;

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
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &fragCompiled);
	if (fragCompiled != 1) {
		cout << "fragment compilation failed" << endl;
		printShaderLog(fShader);
	}

	// Shader program objektum l�trehoz�sa. Elt�roljuk az ID �rt�ket.
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
			beziers.at(0).VertexCount++;
			break;
		case GLFW_KEY_DOWN:
			beziers.at(0).VertexCount--;
			break;
		}
	}

	UpdateVertexData();
	UpdateVBO();
}



GLint dragged = -1;
GLint draggedCurve = -1;
GLint hover = -1;
GLint hoverCurve = -1;
GLfloat dist2(glm::vec3 P1, glm::vec3 P2)
{
	GLfloat t1 = P1.x - P2.x;
	GLfloat t2 = P1.y - P2.y;

	return t1 * t1 + t2 * t2;
}

GLint getActivePoint(vector<glm::vec3> p, GLint size, GLfloat sens, GLfloat x, GLfloat y)
{

	GLint i;
	GLfloat s = sens * sens;

	GLfloat xNorm = x / (WIN_WIDTH / 2) - 1.0f;
	GLfloat yNorm = y / (WIN_HEIGHT / 2) - 1.0f;
	glm::vec3 P = glm::vec3(xNorm, yNorm, 0.0f);

	for (i = 0; i < size; i++) {
		if (dist2(p[i], P) < s) {
			return i;
		}
	}
	return -1;

}
void cursorPosCallback(GLFWwindow* window, double x, double y)
{
	if (dragged != -1 && draggedCurve != -1) {
		isDragging = true;

		float xNorm = x / (WIN_WIDTH / 2) - 1.0f;
		float yNorm = (WIN_HEIGHT - y) / (WIN_HEIGHT / 2) - 1.0f;

		beziers[draggedCurve].Points[dragged] = glm::vec3(xNorm, yNorm, 0.0f);
		UpdateVertexData();
		UpdateVBO();
	}
}


void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	double x, y;
	glfwGetCursorPos(window, &x, &y);

	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		if (action == GLFW_PRESS) {
			dragStartX = x;
			dragStartY = y;
			isDragging = false;  // még nem tudjuk, hogy húzás lesz-e

			// Megpróbáljuk elkezdeni a drag-et
			for (int c = 0; c < beziers.size(); c++) {
				for (int i = 0; i < beziers[c].Points.size(); i++) {
					glm::vec3 p = beziers[c].Points[i];
					float sx = (p.x + 1.0f) * WIN_WIDTH / 2.0f;
					float sy = (1.0f - p.y) * WIN_HEIGHT / 2.0f;

					if (abs(x - sx) < 10 && abs(y - sy) < 10) {
						dragged = i;
						draggedCurve = c;
						return;
					}
				}
			}
		}
		else if (action == GLFW_RELEASE) {
			double dx = x - dragStartX;
			double dy = y - dragStartY;
			float dist = sqrt(dx * dx + dy * dy);

			// Ha nem történt jelentős elmozdulás → ez kattintás
			if (dist < 5.0) {
				float xNorm = x / (WIN_WIDTH / 2) - 1.0f;
				float yNorm = (WIN_HEIGHT - y) / (WIN_HEIGHT / 2) - 1.0f;

				beziers.at(0).Points.push_back(glm::vec3(xNorm, yNorm, 0.0f));
				beziers.at(0).VertexCount++;
				UpdateVertexData();
				UpdateVBO();
			}

			// Reset drag állapot
			dragged = -1;
			draggedCurve = -1;
			isDragging = false;
		}
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		float xNorm = x / (WIN_WIDTH / 2) - 1.0f;
		float yNorm = (WIN_HEIGHT - y) / (WIN_HEIGHT / 2) - 1.0f;

		int closestIndex = -1;
		float minDist = 1000.0f;

		for (int i = 0; i < beziers.at(0).Points.size(); i++) {
			float d = dist2(beziers.at(0).Points[i], glm::vec3(xNorm, yNorm, 0.0f));
			if (d < minDist) {
				minDist = d;
				closestIndex = i;
			}
		}

		if (closestIndex != -1 && beziers.at(0).Points.size() > 2) {
			beziers.at(0).Points.erase(beziers.at(0).Points.begin() + closestIndex);
			beziers.at(0).VertexCount--;
			UpdateVertexData();
			UpdateVBO();
		}
	}
}


void init(GLFWwindow* window) {
	renderingProgram = createShaderProgram("fragmentShader.glsl");
	glUseProgram(renderingProgram);
	loc = glGetUniformLocation(renderingProgram, "my_color");

	BezierCurve bezier1 = BezierCurve(4);
	bezier1.Points = {
		glm::vec3(0.75f, 0.1f, 0.0f),
		glm::vec3(0.6f, -0.2f, 0.0f),
		glm::vec3(0.2f, -0.25f, 0.0f),
		glm::vec3(0.0f,  0.1f, 0.0f),
	};
	beziers.push_back(bezier1);
	UpdateVertexData();

	/* L�trehozzuk a sz�ks�ges Vertex buffer �s vertex array objektumot. */
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	/* T�pus meghat�roz�sa: a GL_ARRAY_BUFFER neves�tett csatol�ponthoz kapcsoljuk a buffert (ide ker�lnek a vertex adatok). */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/* M�soljuk az adatokat a pufferbe! Megadjuk az aktu�lisan csatolt puffert,  azt hogy h�ny b�jt adatot m�solunk,
	a m�soland� adatot, majd a feldolgoz�s m�dj�t is meghat�rozzuk: most az adat nem v�ltozik a felt�lt�s ut�n */
	glBufferData(GL_ARRAY_BUFFER, pointsToDraw.size() * sizeof(glm::vec3), pointsToDraw.data(), GL_STATIC_DRAW);
	/* A puffer k�sz, lecsatoljuk, m�r nem szeretn�nk m�dos�tani. */
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	/* Csatoljuk a vertex array objektumunkat a konfigur�l�shoz. */
	glBindVertexArray(VAO);

	/* Vertex buffer objektum �jracsatol�sa. */
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	/* Ezen adatok szolg�lj�k a 0 index� vertex attrib�tumot (itt: poz�ci�).
	Els�k�nt megadjuk ezt az azonos�t�sz�mot.
	Ut�na az attrib�tum m�ret�t (vec3, l�ttuk a shaderben).
	Harmadik az adat t�pusa.
	Negyedik az adat normaliz�l�sa, ez maradhat FALSE jelen p�ld�ban.
	Az attrib�tum �rt�kek hogyan k�vetkeznek egym�s ut�n? Milyen l�p�sk�z ut�n tal�lom a k�vetkez� vertex adatait?
	V�g�l megadom azt, hogy honnan kezd�dnek az �rt�kek a pufferben. Most r�gt�n, a legelej�t�l veszem �ket.*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

	/* Enged�lyezz�k az im�nt defini�lt 0 index� attrib�tumot. */
	glEnableVertexAttribArray(0);

	/* Lev�lasztjuk a vertex array objektumot �s a puufert is.*/
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

}

/** A jelenet�nk ut�ni takar�t�s. */
void cleanUpScene()
{
	/** T�r�lj�k a vertex puffer �s vertex array objektumokat. */
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	/** T�r�lj�k a shader programot. */
	glDeleteProgram(renderingProgram);
}

void display(GLFWwindow* window, double currentTime) {

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT); // fontos lehet minden egyes alkalommal törölni!

	/*Csatoljuk a vertex array objektumunkat. */
	glBindVertexArray(VAO);

	GLint counter = 0;
	glLineWidth(2.0f);
	glPointSize(10.0f);
	for (int i = 0; i < beziers.size(); i++)
	{
		if (loc != -1)
		{
			glUniform3fv(loc, 1, beziers.at(i).color);
		}
		glDrawArrays(GL_LINE_STRIP, counter, beziers.at(i).BlendingVertexCount);
		counter += beziers.at(i).BlendingVertexCount;
		if (loc != -1)
		{
			glUniform3fv(loc, 1, lineColor);
		}
		glDrawArrays(GL_LINE_STRIP, counter, beziers.at(i).VertexCount);
		counter += beziers.at(i).VertexCount;
		if (loc != -1)
		{
			glUniform3fv(loc, 1, pointColor);
		}
		for (int j = 0; j < beziers.at(i).circlePoints.size(); j++)
		{
			if (i == hoverCurve && j == hover)
			{
				if (loc != -1)
				{
					glUniform3fv(loc, 1, activePoint);
				}
			}
			else
			{
				if (loc != -1)
				{
					glUniform3fv(loc, 1, pointColor);
				}
			}
			glDrawArrays(GL_TRIANGLE_FAN, counter, beziers.at(i).circlePoints.at(j).VertexCount);
			counter += beziers.at(i).circlePoints.at(j).VertexCount;
		}


	}


	glBindVertexArray(0);
}

int main(void) {

	/* Pr�b�ljuk meg inicializ�lni a GLFW-t! */
	if (!glfwInit()) { exit(EXIT_FAILURE); }

	/* A k�v�nt OpenGL verzi� (4.3) */
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

	/* Pr�b�ljuk meg l�trehozni az ablakunkat. */
	GLFWwindow* window = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "Beadando2", NULL, NULL);

	/* V�lasszuk ki az ablakunk OpenGL kontextus�t, hogy haszn�lhassuk. */
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetCursorPosCallback(window, cursorPosCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);

	/* Incializ�ljuk a GLEW-t, hogy el�rhet�v� v�ljanak az OpenGL f�ggv�nyek. */
	if (glewInit() != GLEW_OK) { exit(EXIT_FAILURE); }
	glfwSwapInterval(1);

	/* Az alkalmaz�shoz kapcsol�d� el�k�sz�t� l�p�sek, pl. hozd l�tre a shader objektumokat. */
	init(window);

	while (!glfwWindowShouldClose(window)) {
		/* a k�d, amellyel rajzolni tudunk a GLFWwindow ojektumunkba. */
		display(window, glfwGetTime());
		/* double buffered */
		glfwSwapBuffers(window);
		/* esem�nyek kezel�se az ablakunkkal kapcsolatban, pl. gombnyom�s */
		glfwPollEvents();
	}

	/* t�r�lj�k a GLFW ablakot. */
	glfwDestroyWindow(window);
	/* Le�ll�tjuk a GLFW-t */

	cleanUpScene();

	glfwTerminate();
	exit(EXIT_SUCCESS);
}