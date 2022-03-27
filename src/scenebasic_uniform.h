#pragma once

#include "helper/scene.h"
#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/teapot.h"
#include "helper/plane.h"
#include "helper/objmesh.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    
    std::unique_ptr<ObjMesh> mesh;
    Plane plane;
    Torus torus;
    //Teapot teapot;

    void setMatrices();
    bool compile();

public:
    SceneBasic_Uniform();

    bool initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int, int) override;

    void UpdateViewMatrix(glm::mat4 viewMatrix) override;
    void UpdateProjMatrix(glm::mat4 projMatrix) override;
    void ToggleBlinnPhong();
};
