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

    void compile();

public:
    SceneBasic_Uniform();

    void initScene();
    void update( float t );
    void render();
    void resize(int, int);
};
