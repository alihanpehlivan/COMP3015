#include "../pch.h"

namespace GLUtils {

void APIENTRY debugCallback( GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length, const GLchar * msg, const void * param ) {

	std::string sourceStr;
	switch(source) {
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		sourceStr = "WindowSys";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		sourceStr = "App";
		break;
	case GL_DEBUG_SOURCE_API:
		sourceStr = "OpenGL";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		sourceStr = "ShaderCompiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		sourceStr = "3rdParty";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		sourceStr = "Other";
		break;
	default:
		sourceStr = "Unknown";
	}
	
	std::string typeStr;
	switch(type) {
	case GL_DEBUG_TYPE_ERROR:
		typeStr = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		typeStr = "Deprecated";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		typeStr = "Undefined";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		typeStr = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		typeStr = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		typeStr = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		typeStr = "PushGrp";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		typeStr = "PopGrp";
		break;
	case GL_DEBUG_TYPE_OTHER:
		typeStr = "Other";
		break;
	default:
		typeStr = "Unknown";
	}
	
	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		LOG_CRITICAL("{} : {} ({}): {}", sourceStr, typeStr, id, msg);
		break;
	case GL_DEBUG_SEVERITY_MEDIUM:
		LOG_ERROR("{} : {} ({}): {}", sourceStr, typeStr, id, msg);
		break;
	case GL_DEBUG_SEVERITY_LOW:
		LOG_INFO("{} : {} ({}): {}", sourceStr, typeStr, id, msg);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		LOG_INFO("{} : {} ({}): {}", sourceStr, typeStr, id, msg);
		break;
	default:
		break;
	}
}

int checkForOpenGLError(const char * file, int line) {
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
		const char * message = "";
		switch( glErr )
		{
		case GL_INVALID_ENUM:
			message = "Invalid enum";
			break;
		case GL_INVALID_VALUE:
			message = "Invalid value";
			break;
		case GL_INVALID_OPERATION:
			message = "Invalid operation";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			message = "Invalid framebuffer operation";
			break;
		case GL_OUT_OF_MEMORY:
			message = "Out of memory";
			break;
		default:
			message = "Unknown error";
		}

        printf("glError in file %s @ line %d: %s\n", file, line, message);
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}

void dumpGLInfo(bool dumpExtensions)
{
	auto renderer = glGetString(GL_RENDERER);
	auto vendor = glGetString(GL_VENDOR);
	auto version = glGetString(GL_VERSION);
	auto glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);

	GLint major, minor, samples, sampleBuffers;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	glGetIntegerv(GL_SAMPLES, &samples);
	glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
	
	LOG_INFO("-------------------------------------------------------------");
	LOG_INFO("GL Vendor     : {}", vendor);
	LOG_INFO("GL Renderer   : {}", renderer);
	LOG_INFO("GL Version    : {}", version);
	LOG_INFO("GL Version    : {}.{}", major, minor);
	LOG_INFO("GLSL Version  : {}", glslVersion);
	LOG_INFO("MSAA samples  : {}", samples);
	LOG_INFO("MSAA buffers  : {}", sampleBuffers);
	LOG_INFO("-------------------------------------------------------------");

	if (dumpExtensions)
	{
		GLint nExtensions;
		glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);
		for (int i = 0; i < nExtensions; i++)
			LOG_INFO("{}", glGetStringi(GL_EXTENSIONS, i));
	}
}

} // namespace GLUtils
