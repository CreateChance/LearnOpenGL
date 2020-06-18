#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

/**
 * 通过封装一个宏来自动化处理 GL 错误信息
 *
 */

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError() {
	// get all error now.
	while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, const int line) {
	while (GLenum error = glGetError()) {
		std::cout << "[OpenGL Error] (" << error << "): " << function << " " << file << ":" << line << std::endl;
		return false;
	}
	return true;
}

static unsigned int compileShader(unsigned int type, const std::string& source) {
	// create a new shader object.
	unsigned int id = glCreateShader(type);
	// get c style char array.
	const char* src = source.c_str();
	// specify shader's source code.
	glShaderSource(id, 1, &src, nullptr);
	// just compille it.
	glCompileShader(id);

	// maybe we get errors, so check it.
	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		char* message = (char*)alloca(length * sizeof(char));
		glGetShaderInfoLog(id, length, &length, message);
		std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader" << std::endl;
		std::cout << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static unsigned int createProgram(const std::string& vertexShader, const std::string& fragmentShader) {
	// create prog
	unsigned int program = glCreateProgram();
	unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
	unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	glValidateProgram(program);

	// delete shader object cause we don't need it any more.
	glDeleteShader(vs);
	glDeleteShader(fs);

	return program;
}

int main(void) {
	GLFWwindow* window;

	if (!glfwInit()) {
		return -1;
	}

	window = glfwCreateWindow(640, 480, "SampleWindow", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GLEW_OK) {
		std::cout << "Error!" << std::endl;
		return -1;
	}

	std::cout << glGetString(GL_VERSION) << std::endl;

	// define a buffer.
	// 注意观察，这里的数据我们已经去掉了重复顶点数据，避免内存浪费
	float postions[8] = {
		-0.5f, -0.5f,
		 0.5f, -0.5f,
		 0.5f,  0.5f,
		-0.5f,  0.5f
	};

	// 通过定义顶点绘制顺序，来重复利用上面的顶点数据，避免浪费显存
	// TODO: 貌似浪费了很多的显存？这里的数据也是需要上传到 GPU 存储的啊？
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	unsigned int buffer;
	// only one buffer we neeed.
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), postions, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, 0);

	// index buffer.
	// 通过指定 index buffer object 来指定我们要绘制的三角形顶点顺序
	unsigned int ibo;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);


	std::string vertexShader =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) in vec4 position;"
		"\n"
		"void main()\n"
		"{\n"
		"	gl_Position = position;\n"
		"}\n";

	std::string fragmentShader =
		"#version 330 core\n"
		"\n"
		"layout(location = 0) out vec4 color;"
		"\n"
		"void main()\n"
		"{\n"
		"	color = vec4(0.0, 1.0, 0.0, 0.0);\n"
		"}\n";
	unsigned int program = createProgram(vertexShader, fragmentShader);
	glUseProgram(program);

	while (!glfwWindowShouldClose(window)) {
		glClear(GL_COLOR_BUFFER_BIT);

		// 这里我们需要通过这个函数来绘制，不再是 glDrawArrays 了，这里并不需要指定 indices 指针，因为我们上面已经 bind 过 element buffer array 了
		GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}