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
    Camera* camera;
    ShaderManager sm;

    Plane plane;
    Cube cube;
    std::unique_ptr<ObjMesh> mesh;

    float timePrev = 0.0f, rotSpeed = 0.5f;

    void setMatrices();
    bool compile();

public:
    SceneBasic_Uniform();

    void setupUBO();

    bool initScene(Camera* camera) override;
    void update(float t) override;
    void render() override;
    void resize(int, int) override;
};
