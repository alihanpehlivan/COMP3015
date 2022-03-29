#pragma once
// author: Alihan Pehlivan
// brief: Shader Manager which supports multiple shader objects and programs.

class ShaderManagerException : public std::runtime_error
{
public:
	ShaderManagerException(const std::string& msg) :
		std::runtime_error(msg) {}
};

class ShaderManager
{
public:
	enum class ShaderType : uint32_t
	{
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
		COMPUTE = GL_COMPUTE_SHADER,
	};

	ShaderManager() = default;
	~ShaderManager() = default;

	// Non-copyable
	ShaderManager(const ShaderManager&) = delete;
	ShaderManager& operator=(const ShaderManager&) = delete;

	GLuint Create();

	GLuint Compile(const char* fileName);
	GLuint Compile(const char* fileName, ShaderType type);
	GLuint Compile(const std::string& source, ShaderType type, const char* fileName);

	void Link(GLuint program);
	void CleanupProgram(GLuint program);
	void ValidateProgram(GLuint program);
	void ValidatePipeline(GLuint pipeline);

	void BindAttribLocation(GLuint progHandle, GLuint location, const char* name);
	void BindFragDataLocation(GLuint progHandle, GLuint location, const char* name);

	void SetUniform(GLuint progHandle, const char* name, float x, float y, float z);
	void SetUniform(GLuint progHandle, const char* name, const glm::vec2& v);
	void SetUniform(GLuint progHandle, const char* name, const glm::vec3& v);
	void SetUniform(GLuint progHandle, const char* name, const glm::vec4& v);
	void SetUniform(GLuint progHandle, const char* name, const glm::mat4& m);
	void SetUniform(GLuint progHandle, const char* name, const glm::mat3& m);
	void SetUniform(GLuint progHandle, const char* name, float val);
	void SetUniform(GLuint progHandle, const char* name, int val);
	void SetUniform(GLuint progHandle, const char* name, bool val);
	void SetUniform(GLuint progHandle, const char* name, GLuint val);

private:
	GLint GetUniformLocation(GLuint progHandle, const char* name);
};
