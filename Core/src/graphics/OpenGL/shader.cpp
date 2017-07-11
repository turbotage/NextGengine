#include "shader.h"
#include "shaders\vertex.h"
#include "shaders\fragment.h"

GLuint ng::graphics::Shader::getVertexShader(int shaderNumber)
{
	GLuint shader = glCreateShader(GL_VERTEX_SHADER);
	std::string code;
	switch (shaderNumber) {
	case 0:
		code = vertex0;
		break;
	}
	char const * vertexSourcePointer = code.c_str();
	glShaderSource(shader, 1, &vertexSourcePointer, NULL);
	glCompileShader(shader);

	GLint result = GL_FALSE;
	int infoLogLength;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(shader, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
		throw std::runtime_error(&vertexShaderErrorMessage[0]);
	}
	return shader;
}

GLuint ng::graphics::Shader::getFragmentShader(int shaderNumber)
{
	GLuint shader = glCreateShader(GL_FRAGMENT_SHADER);
	std::string code;
	switch (shaderNumber)
	{
	case 0:
		code = fragment0;
		break;
	}
	char const* fragmentSourcePointer = code.c_str();
	glShaderSource(shader, 1, &fragmentSourcePointer, NULL);
	glCompileShader(shader);
	GLint result = GL_FALSE;
	int infoLogLength;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
		glGetShaderInfoLog(shader, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
		throw std::runtime_error(&fragmentShaderErrorMessage[0]);
	}
	return shader;
}

GLuint ng::graphics::Shader::createProgram(std::vector<GLuint>& shaders)
{
	GLuint programID = glCreateProgram();
	for (auto& shader : shaders) {
		glAttachShader(programID, shader);
	}
	glLinkProgram(programID);
	GLint result = GL_FALSE;
	int infoLogLength;
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> programErrorMessage(infoLogLength + 1);
		glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
		throw std::runtime_error(&programErrorMessage[0]);
	}
	for (auto& shader : shaders) {
		glDetachShader(programID, shader);
	}
	for (auto& shader : shaders) {
		glDeleteShader(shader);
	}
	return programID;
}

ng::graphics::Shader::Shader()
{

}