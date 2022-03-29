// Precompiled Header
#pragma once

#define WIN32_LEAN_AND_MEAN // Don't include everything
#include <Windows.h>

#include <assert.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <list>
#include <algorithm>
#include <map>
#include <vector>
#include <string>
#include <tuple>
#include <array>
#include <filesystem>

// GL Loader-Generator
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GL Math
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Utilities
#include "helper/glutils.h"
#include "helper/shader_manager.h"

// Custom logger
#include "log.h"
