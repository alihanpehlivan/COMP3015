#include "pch.h"

#include "helper/scene.h"
#include "helper/scenerunner.h"
#include "scenebasic_uniform.h"

// Windows Entry Point
int WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR lpCmdLine,
	_In_ int nShowCmd
)
{
	SceneRunner runner("Alihan's Custom Renderer w/ Shaders");

	auto scene = std::make_unique<SceneBasic_Uniform>();
	return runner.run(*scene);
}
