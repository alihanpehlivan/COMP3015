#include "../pch.h"
#include "shader_manager.h"

// Available shader extensions
static std::map<std::string, ShaderManager::ShaderType> _fileExtensions =
{
	{".vs",			ShaderManager::ShaderType::VERTEX				},
	{ ".vert",		ShaderManager::ShaderType::VERTEX				},
	{ "_vert.glsl",	ShaderManager::ShaderType::VERTEX				},
	{ ".vert.glsl",	ShaderManager::ShaderType::VERTEX				},
	{ ".gs",		ShaderManager::ShaderType::GEOMETRY			},
	{ ".geom",		ShaderManager::ShaderType::GEOMETRY			},
	{ ".geom.glsl",	ShaderManager::ShaderType::GEOMETRY			},
	{ ".tcs",		ShaderManager::ShaderType::TESS_CONTROL		},
	{ ".tcs.glsl",	ShaderManager::ShaderType::TESS_CONTROL		},
	{ ".tes",		ShaderManager::ShaderType::TESS_EVALUATION	},
	{ ".tes.glsl",	ShaderManager::ShaderType::TESS_EVALUATION	},
	{ ".fs",		ShaderManager::ShaderType::FRAGMENT			},
	{ ".frag",		ShaderManager::ShaderType::FRAGMENT			},
	{ "_frag.glsl",	ShaderManager::ShaderType::FRAGMENT			},
	{ ".frag.glsl",	ShaderManager::ShaderType::FRAGMENT			},
	{ ".cs",		ShaderManager::ShaderType::COMPUTE			},
	{ ".cs.glsl",	ShaderManager::ShaderType::COMPUTE			},
};

GLuint ShaderManager::Create()
{
	GLuint handle = glCreateProgram();
	if (!handle)
		throw ShaderManagerException("Unable to create shader program.");

	// Declare that program is separable
	glProgramParameteri(handle, GL_PROGRAM_SEPARABLE, GL_TRUE);
	LOG_INFO("Shader Manager: program handle {} created", handle);
	return handle;
}

GLuint ShaderManager::Compile(const char* fileName)
{
	// Check the filename extension to determine shader type
	auto ext = std::filesystem::path(fileName).extension().string();
	auto it = _fileExtensions.find(ext);

	if (it == _fileExtensions.end())
		throw ShaderManagerException(fmt::format("Unrecognized extension: {}", ext));

	// Type recognized, move on to next stage
	auto shaderType = it->second;
	return Compile(fileName, shaderType);
}

GLuint ShaderManager::Compile(const char* fileName, ShaderType type)
{
	if (!std::filesystem::exists(fileName))
		throw ShaderManagerException(fmt::format("Shader: {} not found in the filesystem.", fileName));

	std::ifstream inFile(fileName, std::ios::in);

	if (!inFile)
		throw ShaderManagerException(fmt::format("Unable to open: {}", fileName));

	// Get file contents
	std::stringstream code;
	code << inFile.rdbuf();
	inFile.close();

	return Compile(code.str(), type, fileName);
}

GLuint ShaderManager::Compile(const std::string& source, ShaderType type, const char* fileName)
{
	GLint status;
	auto shaderHandle = glCreateShader((GLenum)type);

	const char* shaderSrc = source.c_str();
	glShaderSource(shaderHandle, 1, &shaderSrc, 0);
	glCompileShader(shaderHandle);
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &status);

	if (GL_FALSE == status)
	{
		char log[5000];
		glGetShaderInfoLog(shaderHandle, 5000, 0, log);
		throw ShaderManagerException(fmt::format("Shader failed to compile:\n{}", log));
	}
	else
	{
		LOG_INFO("Shader Manager: shader from: \"{}\" compiled with handle: {}", fileName, shaderHandle);
		return shaderHandle;
	}

	return 0;
}

void ShaderManager::Link(GLuint program)
{
	GLint status;
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);

	if (GL_FALSE == status)
	{
		char log[5000];
		glGetProgramInfoLog(program, 5000, 0, log);
		throw ShaderManagerException(fmt::format("Program failed to link:\n{}", log));
	}
}

void ShaderManager::CleanupProgram(GLuint program)
{
	// Detach and delete the shader objects (if they are not already removed)
	GLint numShaders = 0;
	glGetProgramiv(program, GL_ATTACHED_SHADERS, &numShaders);
	std::vector<GLuint> shaderNames(numShaders);
	glGetAttachedShaders(program, numShaders, NULL, shaderNames.data());

	for (GLuint shader : shaderNames)
	{
		glDetachShader(program, shader);
		glDeleteShader(shader);
		LOG_INFO("Shader Manager: detaching and deleting shader handle {} from prog handle {}", shader, program);
	}
}

void ShaderManager::ValidateProgram(GLuint program)
{
	GLint status;
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &status);

	if (GL_FALSE == status)
	{
		char log[5000];
		glGetProgramInfoLog(program, 5000, 0, log);
		throw ShaderManagerException(fmt::format("Program failed to validate:\n{}", log));
	}
}

void ShaderManager::ValidatePipeline(GLuint pipeline)
{
	GLint status;
	glValidateProgramPipeline(pipeline);
	glGetProgramiv(pipeline, GL_VALIDATE_STATUS, &status);

	if (GL_FALSE == status)
	{
		char log[5000];
		glGetProgramPipelineInfoLog(pipeline, 5000, 0, log);
		throw ShaderManagerException(fmt::format("Pipeline failed to validate:\n{}", log));
	}
}

void ShaderManager::BindAttribLocation(GLuint program, GLuint location, const char* name,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glBindAttribLocation(program, location, name);
	GLUtils::checkForOpenGLError(srcloc.file_name(), srcloc.line());
}

void ShaderManager::BindFragDataLocation(GLuint program, GLuint location, const char* name,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glBindFragDataLocation(program, location, name);
	GLUtils::checkForOpenGLError(srcloc.file_name(), srcloc.line());
}

void ShaderManager::SetUniform(GLuint program, const char* name, float x, float y, float z,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform3f(program, GetUniformLocation(program, name, srcloc), x, y, z); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, const glm::vec3& v,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	this->SetUniform(program, name, v.x, v.y, v.z, srcloc);
}

void ShaderManager::SetUniform(GLuint program, const char* name, const glm::vec4& v,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform4f(program, GetUniformLocation(program, name, srcloc), v.x, v.y, v.z, v.w); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, const glm::vec2& v,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform2f(program, GetUniformLocation(program, name, srcloc), v.x, v.y); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, const glm::mat4& m,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniformMatrix4fv(program, GetUniformLocation(program, name, srcloc), 1, GL_FALSE, &m[0][0]); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, const glm::mat3& m,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniformMatrix3fv(program, GetUniformLocation(program, name, srcloc), 1, GL_FALSE, &m[0][0]); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, float val,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform1f(program, GetUniformLocation(program, name, srcloc), val); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, int val,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform1i(program, GetUniformLocation(program, name, srcloc), val); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, GLuint val,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform1ui(program, GetUniformLocation(program, name, srcloc), val); GLERR;
}

void ShaderManager::SetUniform(GLuint program, const char* name, bool val,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	glProgramUniform1i(program, GetUniformLocation(program, name, srcloc), val); GLERR;
}

GLint ShaderManager::GetUniformLocation(GLuint program, const char* name,
	const std::source_location srcloc/*= std::source_location::current()*/)
{
	auto& programMap = _uniformLocations[program];
	auto it = programMap.find(name);

	if (it == programMap.end())
	{
		GLint loc = glGetUniformLocation(program, name);

		// Log any errors in details
		if (loc < 0)
		{
			LOG_CRITICAL("cant find uniform location \"{}\" in program {}", name, program);
			LOG_CRITICAL("in function: \"{}\" file: \"{}\" line: \"{}\"",
				srcloc.function_name(), srcloc.file_name(), srcloc.line());
		}

		programMap[name] = loc;
		return loc;
	}

	return it->second;
}
