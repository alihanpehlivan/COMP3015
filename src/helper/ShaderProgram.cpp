#include "../pch.h"
#include "ShaderProgram.h"

// Available shader extensions
static std::map<std::string, ESP::ShaderType> _fileExtensions =
{
	{".vs",			ESP::ShaderType::VERTEX				},
	{ ".vert",		ESP::ShaderType::VERTEX				},
	{ "_vert.glsl",	ESP::ShaderType::VERTEX				},
	{ ".vert.glsl",	ESP::ShaderType::VERTEX				},
	{ ".gs",		ESP::ShaderType::GEOMETRY			},
	{ ".geom",		ESP::ShaderType::GEOMETRY			},
	{ ".geom.glsl",	ESP::ShaderType::GEOMETRY			},
	{ ".tcs",		ESP::ShaderType::TESS_CONTROL		},
	{ ".tcs.glsl",	ESP::ShaderType::TESS_CONTROL		},
	{ ".tes",		ESP::ShaderType::TESS_EVALUATION	},
	{ ".tes.glsl",	ESP::ShaderType::TESS_EVALUATION	},
	{ ".fs",		ESP::ShaderType::FRAGMENT			},
	{ ".frag",		ESP::ShaderType::FRAGMENT			},
	{ "_frag.glsl",	ESP::ShaderType::FRAGMENT			},
	{ ".frag.glsl",	ESP::ShaderType::FRAGMENT			},
	{ ".cs",		ESP::ShaderType::COMPUTE			},
	{ ".cs.glsl",	ESP::ShaderType::COMPUTE			},
};

ESP::ESP()
{
}

ESP::~ESP()
{
	if (_handle == 0)
		return;

	DetachAndDeleteShaderObjects();
	glDeleteProgram(_handle);
}

GLuint ESP::Create()
{
	_handle = glCreateProgram();
	if (!_handle)
		throw ESPException("Unable to create shader program.");

	// Declare that program is separable
	glProgramParameteri(_handle, GL_PROGRAM_SEPARABLE, GL_TRUE);
	LOG_INFO("ESP: program handle {} created", _handle);
	return _handle;
}

void ESP::Use()
{
	if (!_handle)
		throw ESPException("Shader is not created");
	if (!_linked)
		throw ESPException("Shader has not been linked");
	glUseProgram(_handle);
}

void ESP::Compile(const char* fileName)
{
	// Check the filename extension to determine shader type
	auto ext = std::filesystem::path(fileName).extension().string();
	auto it = _fileExtensions.find(ext);

	if (it == _fileExtensions.end())
		throw ESPException(fmt::format("Unrecognized extension: {}", ext));

	// Type recognized, move on to next stage
	auto shaderType = it->second;
	Compile(fileName, shaderType);
}

void ESP::Compile(const char* fileName, ShaderType type)
{
	if (!std::filesystem::exists(fileName))
		throw ESPException(fmt::format("Shader: {} not found in the filesystem.", fileName));

	std::ifstream inFile(fileName, std::ios::in);

	if (!inFile)
		throw ESPException(fmt::format("Unable to open: {}", fileName));

	// Get file contents
	std::stringstream code;
	code << inFile.rdbuf();
	inFile.close();

	Compile(code.str(), type, fileName);
}

void ESP::Compile(const std::string& source, ShaderType type, const char* fileName)
{
	if (!_handle)
		throw ESPException("While compiling {}, program was not created!");

	auto newShaderHandle = glCreateShader((GLenum)type);

	const char* shaderSrc = source.c_str();
	glShaderSource(newShaderHandle, 1, &shaderSrc, 0);

	// Compile
	glCompileShader(newShaderHandle);

	// Check for errors
	int result, length;
	glGetShaderiv(newShaderHandle, GL_COMPILE_STATUS, &result);

	if (GL_FALSE == result)
	{
		// Compile failed, throw
		LOG_ERROR("Shader compilation failed in: {}", fileName ? fileName : "<no file name>");

		std::string msg;
		glGetShaderiv(newShaderHandle, GL_INFO_LOG_LENGTH, &length);

		if (length > 0)
		{
			std::string log(length, ' ');
			int written = 0;
			glGetShaderInfoLog(newShaderHandle, length, &written, &log[0]);
			LOG_ERROR(log);
			msg += log;
		}

		throw ESPException(msg);
	}
	else
	{
		// Compile succeeded, attach shader
		glAttachShader(_handle, newShaderHandle);
		LOG_INFO("ESP: new shader handle {} (from: \"{}\") created and attached to prog handle {}", newShaderHandle, fileName, _handle);
	}
}

void ESP::Link()
{
	if (!_handle)
		throw ESPException("There are no programs to link.");

	if (_linked)
		return;

	int status = 0;
	std::string errString;
	glLinkProgram(_handle);
	glGetProgramiv(_handle, GL_LINK_STATUS, &status);

	if (GL_FALSE == status)
	{
		int length = 0;
		glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &length);
		errString += "Program link failed:\n";
		if (length > 0) {
			std::string log(length, ' ');
			int written = 0;
			glGetProgramInfoLog(_handle, length, &written, &log[0]);
			errString += log;
		}
		LOG_ERROR(errString);
	}
	else
	{
		_linked = true;
	}

	ExtractUniformLocations();
	DetachAndDeleteShaderObjects();

	if (GL_FALSE == status)
		throw ESPException(errString);
}

void ESP::Validate()
{
	if (!_linked)
		throw ESPException("Program is not linked");

	GLint status;
	glValidateProgram(_handle);
	glGetProgramiv(_handle, GL_VALIDATE_STATUS, &status);

	if (GL_FALSE == status)
	{
		int length = 0;
		std::string logString;

		glGetProgramiv(_handle, GL_INFO_LOG_LENGTH, &length);

		if (length > 0)
		{
			char* c_log = new char[length];
			int written = 0;
			glGetProgramInfoLog(_handle, length, &written, c_log);
			logString = c_log;
			delete[] c_log;
		}

		throw ESPException(fmt::format("Program failed to validate:\n{}", logString));
	}
}

void ESP::DetachAndDeleteShaderObjects()
{
	if (!_handle)
		throw ESPException("Trying to detach and delete an empty shader program handle.");

	// Detach and delete the shader objects (if they are not already removed)
	GLint numShaders = 0;
	glGetProgramiv(_handle, GL_ATTACHED_SHADERS, &numShaders);
	std::vector<GLuint> shaderNames(numShaders);
	glGetAttachedShaders(_handle, numShaders, NULL, shaderNames.data());

	for (GLuint shader : shaderNames)
	{
		glDetachShader(_handle, shader);
		glDeleteShader(shader);
		LOG_INFO("ESP: detaching shader handle {} from prog handle {}", shader, _handle);
	}
}

void ESP::BindAttribLocation(GLuint location, const char* name) {
	glBindAttribLocation(_handle, location, name);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::BindFragDataLocation(GLuint location, const char* name) {
	glBindFragDataLocation(_handle, location, name);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, float x, float y, float z) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("xyz set uniform error: {} loc: {} ", name, loc);
	glUniform3f(loc, x, y, z);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, const glm::vec3& v) {
	this->SetUniform(name, v.x, v.y, v.z);
}

void ESP::SetUniform(const char* name, const glm::vec4& v) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("vec4 set uniform error: {} loc: {} ", name, loc);
	glUniform4f(loc, v.x, v.y, v.z, v.w);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, const glm::vec2& v) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("vec2 set uniform error: {} loc: {} ", name, loc);
	glUniform2f(loc, v.x, v.y);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, const glm::mat4& m) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("mat4 set uniform error: {} loc: {} ", name, loc);
	glUniformMatrix4fv(loc, 1, GL_FALSE, &m[0][0]);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, const glm::mat3& m) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("mat3 set uniform error: {} loc: {} ", name, loc);
	glUniformMatrix3fv(loc, 1, GL_FALSE, &m[0][0]);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, float val) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("float set uniform error: {} : {} loc: {} ", name, val, loc);
	glUniform1f(loc, val);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, int val) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("int set uniform error: {} : {} loc: {} ", name, val, loc);
	glUniform1i(loc, val);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, GLuint val) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("uint set uniform error: {} : {} loc: {} ", name, val, loc);
	glUniform1ui(loc, val);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::SetUniform(const char* name, bool val) {
	GLint loc = GetUniformLocation(name);
	if (loc < 0)
		LOG_CRITICAL("bool set uniform error: {} : {} loc: {} ", name, val, loc);
	glUniform1i(loc, val);
	GLUtils::checkForOpenGLError(__FILE__, __LINE__);
}

void ESP::ExtractUniformLocations()
{
	LOG_INFO("ESP: extracting uniform locations of prog handle: {}", _handle);

	GLint numUniforms = 0;

	glGetProgramInterfaceiv(_handle, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

	GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

	for (GLint i = 0; i < numUniforms; ++i)
	{
		GLint results[4];
		glGetProgramResourceiv(_handle, GL_UNIFORM, i, 4, properties, 4, NULL, results);

		if (results[3] != -1)
			continue;  // Skip uniforms in blocks

		GLint nameBufSize = results[0] + 1;
		auto name = std::make_unique<char[]>(nameBufSize);
		glGetProgramResourceName(_handle, GL_UNIFORM, i, nameBufSize, NULL, name.get());
		_uniformMap[name.get()] = results[2];
		LOG_INFO("ESP: extracting uniform location id {} ({}) from prog handle {}", results[2], name.get(), _handle);
	}
}

GLint ESP::GetUniformLocation(const char* name)
{
	auto pos = _uniformMap.find(name);

	if (pos == _uniformMap.end())
	{
		auto loc = glGetUniformLocation(_handle, name);
		_uniformMap[name] = loc;
		return loc;
	}

	return pos->second;
}


