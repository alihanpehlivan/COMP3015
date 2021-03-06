#pragma once

class Camera;

class Scene
{
protected:
	glm::mat4 model{}, view{}, projection{};

public:
    int width;
    int height;

	Scene() : width(800), height(600) { }
	virtual ~Scene() {}

    void setDimensions(int w, int h)
    {
        width = w;
        height = h;
    }
   
    // Load textures, initialize shaders, etc.
    virtual bool initScene(Camera* camera) = 0;

    // This is called prior to every frame.  Use this to update your animation.
    virtual void update(float t) = 0;

    // Draw your scene.
    virtual void render() = 0;
    
    // Called when screen is resized
    virtual void resize(int, int) = 0;
};
