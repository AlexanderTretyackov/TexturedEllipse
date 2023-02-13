#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
#define _USE_MATH_DEFINES
#include <cmath>
#include "stb_image.h"

using namespace std;

GLFWwindow* g_window;

GLuint g_shaderProgram;

class Model
{
public:
    GLuint vbo;
    GLuint ibo;
    GLuint vao;
    GLsizei indexCount;
};

Model g_model;

class Texture
{
public: const GLchar* fileImageName;		// Имя файла с картинкой
      const GLchar* uniformMapName;		// Имя юниформа
      GLint mapLocation;					// Идентификатор юниформа
      GLint texUnit;

      Texture(const GLchar* fileImageName, const GLchar* uniformMapName, GLint texUnit)
      {
          this->fileImageName = fileImageName;
          this->uniformMapName = uniformMapName;
          this->texUnit = texUnit;
      }
};
GLuint texturesId[1];

Texture texture1 = Texture("1.jpg", "u_map1", 0);

void createTexture(const GLchar* nameFile, GLint texID)
{
    GLint texW, texH, numComponents;

    // Берем указатель на данные изображения в памяти
    GLvoid* imageData = stbi_load(nameFile, &texW, &texH, &numComponents, 4);

    // Привязка текстуры к цели текстурирования
    glBindTexture(GL_TEXTURE_2D, texID);

    // Задаем изображение текстуры
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);

    stbi_image_free(imageData);

    // Задаем параметры текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void initTextures()
{
    // Генерируем id для текстур
    glGenTextures(1, texturesId);

    createTexture(texture1.fileImageName, texturesId[0]);
    texture1.mapLocation = glGetUniformLocation(g_shaderProgram, texture1.uniformMapName);
}

const double PI = 3.141592653589793;

GLuint createShader(const GLchar* code, GLenum type)
{
    GLuint result = glCreateShader(type);

    glShaderSource(result, 1, &code, NULL);
    glCompileShader(result);

    GLint compiled;
    glGetShaderiv(result, GL_COMPILE_STATUS, &compiled);

    if (!compiled)
    {
        GLint infoLen = 0;
        glGetShaderiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = (char*)alloca(infoLen);
            glGetShaderInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader compilation error" << endl << infoLog << endl;
        }
        glDeleteShader(result);
        return 0;
    }

    return result;
}

GLuint createProgram(GLuint vsh, GLuint fsh)
{
    GLuint result = glCreateProgram();

    glAttachShader(result, vsh);
    glAttachShader(result, fsh);

    glLinkProgram(result);

    GLint linked;
    glGetProgramiv(result, GL_LINK_STATUS, &linked);

    if (!linked)
    {
        GLint infoLen = 0;
        glGetProgramiv(result, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            char* infoLog = (char*)alloca(infoLen);
            glGetProgramInfoLog(result, infoLen, NULL, infoLog);
            cout << "Shader program linking error" << endl << infoLog << endl;
        }
        glDeleteProgram(result);
        return 0;
    }

    return result;
}

bool createShaderProgram()
{
    g_shaderProgram = 0;

    const GLchar vsh[] =
        "#version 330\n"
        ""
        "layout(location = 0) in vec2 a_position;"
		"out vec2 v_textureCoord;"
        ""
        "void main()"
        "{"
        "    gl_Position = vec4(a_position, 0.0, 1.0);"
        "	 v_textureCoord = vec2(a_position[0], a_position[1]);"
        "}"
        ;

    const GLchar fsh[] =
        "#version 330\n"
        ""
        ""
        "in vec2 v_textureCoord;"
        "layout(location = 0) out vec4 o_color;"
        ""
        "uniform sampler2D u_map1;"
        "void main()"
        "{"
        "	vec4 texel = texture(u_map1, v_textureCoord);"
        "   o_color = vec4(texel.xyz, 1.0);"
        "}"
        ;

    GLuint vertexShader, fragmentShader;

    vertexShader = createShader(vsh, GL_VERTEX_SHADER);
    fragmentShader = createShader(fsh, GL_FRAGMENT_SHADER);

    g_shaderProgram = createProgram(vertexShader, fragmentShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    initTextures();

    return g_shaderProgram != 0;
}

bool createModel()
{
    int countVerticiesOnEllipse = 50;

    int lengthVertexArray = countVerticiesOnEllipse * 2 + 2;
    float* vert = new float[lengthVertexArray];
    
    float step = 2 * PI / countVerticiesOnEllipse;
    float angle = 0;
    float a, b;
    a = 0.7;
    b = 0.5;

    for (int i = 0, k = 0; i < countVerticiesOnEllipse; i++, angle += step)
    {
        vert[k++] = a * cos(angle);
        vert[k++] = b * sin(angle); 
    }

    vert[lengthVertexArray - 1] = 0.0;
    vert[lengthVertexArray - 2] = 0.0;

    int k = 0;

    int lengthIndiciesArray = countVerticiesOnEllipse * 3;

    unsigned int* indicies = new unsigned int[lengthIndiciesArray];

    k = 0;

    for (int i = 0; i < countVerticiesOnEllipse; i++)
    {
        indicies[k++] = i;
        indicies[k++] = i + 1;
        indicies[k++] = countVerticiesOnEllipse;
    }

	indicies[lengthIndiciesArray - 2] = 0;

    glGenVertexArrays(1, &g_model.vao);//vertex array object
    glBindVertexArray(g_model.vao);

    glGenBuffers(1, &g_model.vbo);//vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, g_model.vbo);
    glBufferData(GL_ARRAY_BUFFER, lengthVertexArray * sizeof(float), vert, GL_STATIC_DRAW);

    glGenBuffers(1, &g_model.ibo);//index buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_model.ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, lengthIndiciesArray * sizeof(unsigned int), indicies, GL_STATIC_DRAW);
    g_model.indexCount = lengthIndiciesArray;

    //определяем memory layout
    glEnableVertexAttribArray(0);//атрибут номер 0 для координат
    glVertexAttribPointer(0//номер атрибута
        , 2//сколько чисел в атрибуте
        , GL_FLOAT//тип чисел
        , GL_FALSE//брать числа как есть, иначе нормировка была бы
        , 2 * sizeof(GLfloat)//с каким шагом из памяти выдергивать атрибуты из памяти
        , (const GLvoid*)0);//указатель смещения относительно начала массива, в нашем случае смещение - 0
    return g_model.vbo != 0 && g_model.ibo != 0 && g_model.vao != 0;
}

bool init()
{
    // Set initial color of color buffer to white.
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // Разрешает наложение текстуры
    glEnable(GL_TEXTURE_2D);

    return createShaderProgram() && createModel();
}

void reshape(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void draw()
{
    // Clear color buffer.
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_shaderProgram);
    glBindVertexArray(g_model.vao);

    glActiveTexture(GL_TEXTURE0 + texture1.texUnit);
    glBindTexture(GL_TEXTURE_2D, texturesId[0]);
    glUniform1i(texture1.mapLocation, texture1.texUnit);

    glDrawElements(GL_TRIANGLES, g_model.indexCount, GL_UNSIGNED_INT, NULL);
}

void cleanup()
{
    if (g_shaderProgram != 0)
        glDeleteProgram(g_shaderProgram);
    if (g_model.vbo != 0)
        glDeleteBuffers(1, &g_model.vbo);
    if (g_model.ibo != 0)
        glDeleteBuffers(1, &g_model.ibo);
    if (g_model.vao != 0)
        glDeleteVertexArrays(1, &g_model.vao);
}

bool initOpenGL()
{
    // Initialize GLFW functions.
    if (!glfwInit())
    {
        cout << "Failed to initialize GLFW" << endl;
        return false;
    }

    // Request OpenGL 3.3 without obsoleted functions.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window.
    g_window = glfwCreateWindow(800, 600, "OpenGL Test", NULL, NULL);
    if (g_window == NULL)
    {
        cout << "Failed to open GLFW window" << endl;
        glfwTerminate();
        return false;
    }

    // Initialize OpenGL context with.
    glfwMakeContextCurrent(g_window);

    // Set internal GLEW variable to activate OpenGL core profile.
    glewExperimental = true;

    // Initialize GLEW functions.
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return false;
    }

    // Ensure we can capture the escape key being pressed.
    glfwSetInputMode(g_window, GLFW_STICKY_KEYS, GL_TRUE);

    // Set callback for framebuffer resizing event.
    glfwSetFramebufferSizeCallback(g_window, reshape);

    return true;
}

void tearDownOpenGL()
{
    // Terminate GLFW.
    glfwTerminate();
}

int main()
{
    // Initialize OpenGL
    if (!initOpenGL())
        return -1;

    // Initialize graphical resources.
    bool isOk = init();

    if (isOk)
    {
        // Main loop until window closed or escape pressed.
        while (glfwGetKey(g_window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(g_window) == 0)
        {
            // Draw scene.
            draw();

            // Swap buffers.
            glfwSwapBuffers(g_window);
            // Poll window events.
            glfwPollEvents();
        }
    }

    // Cleanup graphical resources.
    cleanup();

    // Tear down OpenGL.
    tearDownOpenGL();

    return isOk ? 0 : -1;
}