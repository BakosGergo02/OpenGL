//Bakos Gergő P2RHJN

#include "common.cpp"
#include <vector>

GLchar windowTitle[] = "Bakos_Gergo";
bool lightOn = true;
bool prevL = false;
bool useTexture = false;

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        keyboard[key] = GL_TRUE;
    }
    else if (action == GLFW_RELEASE) {
        keyboard[key] = GL_FALSE;
    }
}

void cursorPosCallback(GLFWwindow* window, double xPos, double yPos) {}
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {}

void generateSphere(std::vector<float>& vertices, std::vector<unsigned int>& indices, int sectorCount = 36, int stackCount = 18) {
    const float PI = 3.14159265359f;
    float x, y, z, xy, nx, ny, nz, s, t;
    float sectorStep = 2 * PI / sectorCount;
    float stackStep = PI / stackCount;

    for (int i = 0; i <= stackCount; ++i) {
        float stackAngle = PI / 2 - i * stackStep;
        xy = cosf(stackAngle);
        z = sinf(stackAngle);
        for (int j = 0; j <= sectorCount; ++j) {
            float sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            nx = x; ny = y; nz = z;
            s = (float)j / sectorCount;
            t = (float)i / stackCount;
            vertices.insert(vertices.end(), { x, y, z, nx, ny, nz, s, t });
        }
    }

    for (int i = 0; i < stackCount; ++i) {
        int k1 = i * (sectorCount + 1);
        int k2 = k1 + sectorCount + 1;
        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            if (i != 0) indices.insert(indices.end(), { static_cast<unsigned int>(k1), static_cast<unsigned int>(k2), static_cast<unsigned int>(k1 + 1) });
            if (i != (stackCount - 1)) indices.insert(indices.end(), { static_cast<unsigned int>(k1 + 1), static_cast<unsigned int>(k2), static_cast<unsigned int>(k2 + 1) });
        }
    }
}

float radius = 9.0f;
float camAngle = 0.0f;
float camHeight = 0.0f;

void processInput() {
    if (keyboard[GLFW_KEY_LEFT])  camAngle -= 0.01f;
    if (keyboard[GLFW_KEY_RIGHT]) camAngle += 0.01f;
    if (keyboard[GLFW_KEY_UP])    camHeight += 0.05f;
    if (keyboard[GLFW_KEY_DOWN])  camHeight -= 0.05f;

    if (keyboard[GLFW_KEY_L] && !prevL) {
        lightOn = !lightOn;
        std::cout << (lightOn ? "Light On" : "Light Off") << std::endl;
            prevL = true;
    }
    else if (!keyboard[GLFW_KEY_L]) {
        prevL = false;
    }
}


int main() {
    init(3, 3, GLFW_OPENGL_CORE_PROFILE);

    ShaderInfo shaders[] = {
        { GL_VERTEX_SHADER, "vertexShader.glsl" },
        { GL_FRAGMENT_SHADER, "fragmentShader.glsl" },
        { GL_NONE, nullptr }
    };
    program[0] = LoadShaders(shaders);
    glUseProgram(program[0]);

    // -- ÚJ: kockákhoz normál vektorral bővített attribútumok --
    float cubeVertices[] = {
        // positions         // normals
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 0.0f,-1.0f,
        -0.5f,-0.5f,-0.5f,  0.0f, 0.0f,-1.0f,

        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 0.0f, 1.0f,
        -0.5f,-0.5f, 0.5f,  0.0f, 0.0f, 1.0f,

        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f,-0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f,-0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,

         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  1.0f, 0.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  1.0f, 0.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  1.0f, 0.0f, 0.0f,

        -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
         0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
         0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f, 0.5f,  0.0f,-1.0f, 0.0f,
        -0.5f,-0.5f,-0.5f,  0.0f,-1.0f, 0.0f,

        -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
         0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f,  0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f,-0.5f,  0.0f, 1.0f, 0.0f
    };

    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, BO[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);


    std::vector<float> sunVertices;
    std::vector<unsigned int> sunIndices;
    generateSphere(sunVertices, sunIndices);

    GLuint sunVAO, sunVBO, sunEBO;
    glGenVertexArrays(1, &sunVAO);
    glGenBuffers(1, &sunVBO);
    glGenBuffers(1, &sunEBO);

    glBindVertexArray(sunVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
    glBufferData(GL_ARRAY_BUFFER, sunVertices.size() * sizeof(float), sunVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sunEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sunIndices.size() * sizeof(unsigned int), sunIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    texture[Texture_Default] = SOIL_load_OGL_texture("sun.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
    if (texture[Texture_Default] == 0) {
        std::cerr << "HIBA: Nem sikerült betölteni a sun.jpg textúrát!" << std::endl;
        cleanUpScene(EXIT_FAILURE);
    }


    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    locationMatModel = glGetUniformLocation(program[0], "model");
    locationMatView = glGetUniformLocation(program[0], "view");
    locationMatProjection = glGetUniformLocation(program[0], "projection");

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processInput();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        matProjection = perspective(radians(55.0f), float(windowWidth) / windowHeight, 0.1f, 100.0f);
        vec3 cameraPos = vec3(radius * cos(camAngle), radius * sin(camAngle), camHeight);
        matView = lookAt(cameraPos, vec3(0.0f), vec3(0.0f, 0.0f, 1.0f));

        glUniformMatrix4fv(locationMatView, 1, GL_FALSE, value_ptr(matView));
        glUniformMatrix4fv(locationMatProjection, 1, GL_FALSE, value_ptr(matProjection));

        float sunAngle = glfwGetTime();
        vec3 sunPos = vec3(2.0f * radius * cos(sunAngle), 2.0f * radius * sin(sunAngle), 0.0f);
        glUniform3fv(glGetUniformLocation(program[0], "lightPos"), 1, value_ptr(sunPos));
        glUniform3fv(glGetUniformLocation(program[0], "lightColor"), 1, value_ptr(vec3(1.0f, 1.0f, 0.8f)));

        //  Kockák – világítás KIKAPCSOLVA, színes anyag
        glUniform1i(glGetUniformLocation(program[0], "isSun"), false);
        glUniform1i(glGetUniformLocation(program[0], "lightEnabled"), lightOn);
        glUniform1i(glGetUniformLocation(program[0], "useTexture"), false);
        glUniform3fv(glGetUniformLocation(program[0], "objectColor"), 1, value_ptr(vec3(1.0f, 1.0f, 1.0f)));


        float zPositions[] = { -2.0f, 0.0f, 2.0f };
        glBindTexture(GL_TEXTURE_2D, 0); // ne legyen textúra a kockákon
        glBindVertexArray(VAO[0]);
        for (float z : zPositions) {
            matModel = translate(mat4(1.0f), vec3(0.0f, 0.0f, z));
            glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        // NAP
        glUniform1i(glGetUniformLocation(program[0], "isSun"), true);
        glUniform1i(glGetUniformLocation(program[0], "lightEnabled"), lightOn);
        glUniform1i(glGetUniformLocation(program[0], "useTexture"), true);
        glUniform3fv(glGetUniformLocation(program[0], "objectColor"), 1, value_ptr(vec3(1.0f)));
        matModel = translate(mat4(1.0f), sunPos) * scale(mat4(1.0f), vec3(0.5f));
        glUniformMatrix4fv(locationMatModel, 1, GL_FALSE, value_ptr(matModel));
        glBindTexture(GL_TEXTURE_2D, texture[Texture_Default]);
        glBindVertexArray(sunVAO);
        glDrawElements(GL_TRIANGLES, sunIndices.size(), GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
    }


    cleanUpScene(EXIT_SUCCESS);
    return 0;
}
