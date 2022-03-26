#pragma once

#include "helper/scene.h"
#include "helper/glslprogram.h"
#include "helper/torus.h"
#include "helper/teapot.h"

class SceneBasic_Uniform : public Scene
{
private:
    GLSLProgram prog;
    
    Torus torus;
    //Teapot teapot;

    void setMatrices();

    bool compile();

public:
    SceneBasic_Uniform();

    bool initScene();
    void update( float t );
    void render();
    void resize(int, int);
};
