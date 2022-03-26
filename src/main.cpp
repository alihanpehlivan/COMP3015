#include "pch.h"
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
	runner.init("Alihan's Custom Renderer w/ Shaders");

	auto scene = std::make_unique<SceneBasic_Uniform>();
	auto res = runner.run(scene.get());

	LOG_INFO("program terminated with: {}", res ? "FAILURE" : "SUCCESS");
	Log::Destroy();
	return res;
}
