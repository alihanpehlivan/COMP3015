#pragma once

#include "helper/scene.h"
#include "helper/plane.h"
#include "helper/cube.h"

class GLSLProgram;

class Plane;
class Torus;
class Cube;
class ObjMesh;
class Camera;

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    
    Plane plane;
    Cube cube;
    std::unique_ptr<ObjMesh> mesh;

    float tPrev = 0.0f, rotSpeed = 0.5f;

    void setMatrices();
    bool compile();

public:
    SceneBasic_Uniform();

    bool initScene() override;
    void update(Camera* camera, float t) override;
    void render() override;
    void resize(int, int) override;

    // Keyboard events from scenerunner
    void processKey(int key, int scancode, int action, int mods);
};
