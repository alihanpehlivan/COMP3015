#include "pch.h"
#include "helper/camera.h"
#include "scenerunner.h"
#include "scenebasic_uniform.h"

// Linking External Libraries
#ifdef _DEBUG
#pragma comment(lib, "fmt_Debug_x64.lib")
#pragma comment(lib, "spdlog_Debug_x64.lib")
#else
#pragma comment(lib, "fmt_Release_x64.lib")
#pragma comment(lib, "spdlog_Release_x64.lib")
#endif

// Windows Entry Point
int WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	// Setup my custom logger
    if (!Log::Init("renderer"))
        return EXIT_FAILURE;

	SceneRunner runner;
	runner.init("Alihan's Custom OpenGL Renderer");

	auto scene = std::make_unique<SceneBasic_Uniform>();
	auto camera = std::make_unique<Camera>(glm::vec3(0.0f, 5.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	auto res = runner.run(camera.get(), scene.get());

	LOG_INFO("program terminated with: {}", res ? "FAILURE" : "SUCCESS");
	Log::Destroy();
	return res;
}
