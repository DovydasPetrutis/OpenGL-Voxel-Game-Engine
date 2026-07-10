#include <fstream>
#include <sstream>
#include "Shader.h"

Shader::Shader()
{
}

void Shader::use()
{
	glUseProgram(id);
}

void Shader::init(const std::string& vertexPath, const std::string& fragmentPath, std::string name)
{
	this->name = name;
	std::ifstream vertexfile(vertexPath);
	if (!vertexfile.is_open())
	{
		std::cout << "Failed to open vertex file!" << std::endl;
	}
	std::stringstream vertexBuffer;
	vertexBuffer << vertexfile.rdbuf();
	vertexfile.close();
	std::string vertexShaderSource = vertexBuffer.str();
	const char* vertexPtr = vertexShaderSource.c_str();

	std::ifstream fragmentfile(fragmentPath);
	if (!fragmentfile.is_open())
	{
		std::cout << "Failed to open vertex file!" << std::endl;
	}
	std::stringstream fragmentBuffer;
	fragmentBuffer << fragmentfile.rdbuf();
	fragmentfile.close();
	std::string fragmentShaderSource = fragmentBuffer.str();
	const char* fragmentPtr = fragmentShaderSource.c_str();

	GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShaderID, 1, &vertexPtr, NULL);
	glCompileShader(vertexShaderID);
	int success = 0;
	glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(vertexShaderID, 512, NULL, infoLog);
		std::cout << "Couldn't compile vertex shader: " << infoLog << std::endl;
	}

	GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShaderID, 1, &fragmentPtr, NULL);
	glCompileShader(fragmentShaderID);
	success = 0;
	glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(fragmentShaderID, 512, NULL, infoLog);
		std::cout << "Couldn't compile fragment shader: " << infoLog << std::endl;
	}

	id = glCreateProgram();
	glAttachShader(id, vertexShaderID);
	glAttachShader(id, fragmentShaderID);
	glLinkProgram(id);
	success = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		std::cout << "Couldn't link shaders: " << infoLog << std::endl;
	}

	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glUseProgram(id);
}

int Shader::getUniformLocation(const std::string &name)
{
	// Check if found
	if (uniformLocationCache.find(name) != uniformLocationCache.end())
	{
		return uniformLocationCache[name];
	}

	// if not found, find it
	int location = glGetUniformLocation(id, name.c_str());

	if (location == -1)
	{
		std::cout << "Warning: Uniform " << name << " does not exist!" << "Shader: " << this->name << std::endl;
	}

	uniformLocationCache[name] = location;
	return location;
}

void Shader::setTextureBindless(std::string uniformName, GLuint64 textureHandle)
{
	if (glIsTextureHandleResidentARB(textureHandle))
	{
		glProgramUniformHandleui64vARB(id, Shader::getUniformLocation(uniformName), 1, &textureHandle);
	}
	else
	{
		std::cout << "The texture is not resident for uniform: " << uniformName << " !" << std::endl;
	}
}

void Shader::uniform4f(const std::string& name, float x, float y, float z, float w)
{
	glUniform4f(getUniformLocation(name), x, y, z,w);
}

void Shader::uniform3f(const std::string& name,float x,float y,float z)
{
	glUniform3f(getUniformLocation(name), x, y, z);
}

void Shader::uniform3f(const std::string& name, glm::vec3 vec)
{
	glUniform3f(getUniformLocation(name), vec.x, vec.y, vec.z);
}

void Shader::uniform2f(const std::string& name, float x, float y)
{
	glUniform2f(getUniformLocation(name), x, y);
}

void Shader::uniform2f(const std::string& name, glm::vec2 vector)
{
	glUniform2f(getUniformLocation(name), vector.x, vector.y);
}

void Shader::uniform1i(const std::string& name, int x)
{
	glUniform1i(getUniformLocation(name), x);
}

void Shader::uniform1f(const std::string& name, float x)
{
	glUniform1f(getUniformLocation(name), x);
}

void Shader::uniformMatrix4fv(const std::string& name, glm::mat4 matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void ComputeShader::use()
{
	glUseProgram(id);
}

ComputeShader::ComputeShader()
{
}

void ComputeShader::init(const std::string& path)
{
	std::ifstream computeFile(path);
	if (!computeFile.is_open())
	{
		std::cout << "Failed to open compute shader file!" << std::endl;
	}
	std::stringstream computeBuffer;
	computeBuffer << computeFile.rdbuf();
	computeFile.close();
	std::string vertexShaderSource = computeBuffer.str();
	const char* vertexPtr = vertexShaderSource.c_str();

	GLuint computeID = glCreateShader(GL_COMPUTE_SHADER);
	glShaderSource(computeID, 1, &vertexPtr, NULL);
	glCompileShader(computeID);
	int success = 0;
	glGetShaderiv(computeID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetShaderInfoLog(computeID, 512, NULL, infoLog);
		std::cout << "Couldn't compile compute shader: " << infoLog << std::endl;
	}
	id = glCreateProgram();
	glAttachShader(id, computeID);
	glLinkProgram(id);
	success = 0;
	glGetProgramiv(id, GL_LINK_STATUS, &success);
	if (!success)
	{
		char infoLog[512];
		glGetProgramInfoLog(id, 512, NULL, infoLog);
		std::cout << "Couldn't compile compute program: " << infoLog << std::endl;
	}
	glDeleteShader(computeID);
}

int ComputeShader::getUniformLocation(const std::string& name)
{
	// Check if found
	if (uniformLocationCache.find(name) != uniformLocationCache.end())
	{
		return uniformLocationCache[name];
	}

	// if not found, find it
	int location = glGetUniformLocation(id, name.c_str());

	if (location == -1)
	{
		std::cout << "Warning: Uniform " << name << " does not exist!" << std::endl;
	}

	uniformLocationCache[name] = location;
	return location;
}

void ComputeShader::uniform1f(const std::string& name, float x)
{
	glUniform1f(getUniformLocation(name), x);
}

void ComputeShader::uniformMatrix4fv(const std::string& name, glm::mat4 matrix)
{
	glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}