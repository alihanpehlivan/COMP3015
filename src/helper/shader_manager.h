#pragma once
// author: Alihan Pehlivan
// brief: Shader Manager which supports multiple shader objects and programs.

// Print any errors after performing OpenGL operations
#define GLERR GLUtils::checkForOpenGLError(__FILE__, __LINE__)

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

	void BindAttribLocation(GLuint program, GLuint location, const char* name,
		const std::source_location srcloc = std::source_location::current());
	void BindFragDataLocation(GLuint program, GLuint location, const char* name,
		const std::source_location srcloc = std::source_location::current());

	void SetUniform(GLuint program, const char* name, float x, float y, float z,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, const glm::vec2& v,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, const glm::vec3& v,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, const glm::vec4& v,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, const glm::mat4& m,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, const glm::mat3& m,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, float val,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, int val,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, bool val,
		const std::source_location srcloc = std::source_location::current());
	void SetUniform(GLuint program, const char* name, GLuint val,
		const std::source_location srcloc = std::source_location::current());

private:
	GLint GetUniformLocation(GLuint program, const char* name,
		const std::source_location srcloc = std::source_location::current());

	// <program <uniform name, uniform location>>
	std::map<GLuint, std::map<std::string, GLint>> _uniformLocations;
};
