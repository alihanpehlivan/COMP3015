#pragma once
// Alihan Pehlivan - ESP
// Shader loader with more verbose and more debugging possibility

// Easy shader program exception
class ESPException : public std::runtime_error {
public:
	ESPException(const std::string& msg) :
		std::runtime_error(msg) {}
};

// Easy shader program
class ESP
{
public:
	using UniformMap_t = std::map<std::string, int>;

	enum class ShaderType : uint32_t
	{
		VERTEX = GL_VERTEX_SHADER,
		FRAGMENT = GL_FRAGMENT_SHADER,
		GEOMETRY = GL_GEOMETRY_SHADER,
		TESS_CONTROL = GL_TESS_CONTROL_SHADER,
		TESS_EVALUATION = GL_TESS_EVALUATION_SHADER,
		COMPUTE = GL_COMPUTE_SHADER,
	};

	ESP();
	~ESP();

	// Non-copyable
	ESP(const ESP&) = delete;
	ESP& operator=(const ESP&) = delete;

	// Create a shader program and set it as active one and returns prog handle
	GLuint Create();
	void Use();
	void Compile(const char* fileName);
	void Compile(const char* fileName, ShaderType type);
	void Compile(const std::string& source, ShaderType type, const char* fileName);
	void Link();
	void Validate();
	void DetachAndDeleteShaderObjects();

	void BindAttribLocation(GLuint location, const char* name);
	void BindFragDataLocation(GLuint location, const char* name);

	void SetUniform(const char* name, float x, float y, float z);
	void SetUniform(const char* name, const glm::vec2& v);
	void SetUniform(const char* name, const glm::vec3& v);
	void SetUniform(const char* name, const glm::vec4& v);
	void SetUniform(const char* name, const glm::mat4& m);
	void SetUniform(const char* name, const glm::mat3& m);
	void SetUniform(const char* name, float val);
	void SetUniform(const char* name, int val);
	void SetUniform(const char* name, bool val);
	void SetUniform(const char* name, GLuint val);

private:
	void ExtractUniformLocations();
	GLint GetUniformLocation(const char* name);

private:
	GLuint _handle = 0;
	bool _linked = false;
	std::map<std::string, int> _uniformMap;
};
