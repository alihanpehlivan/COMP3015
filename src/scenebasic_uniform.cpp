#include "pch.h"
#include "scenebasic_uniform.h"
#include "helper/texture.h"
#include "helper/camera.h"
#include "helper/objmesh.h"

#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"

namespace Configs
{
    // Scene & shader settings
    static ImVec4 bgColor = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.00f);
    static bool useBlinnPhong = true;
    static bool useTextures = true;
    static bool useTextureMix = true;

    static bool useToon = false;
    static float toonFraction = 1.0f;

    static bool animateLight = true;
    static glm::vec4 lightPos = { 0.f, 0.f, 0.f, 0.f }; // Light position
    static glm::vec3 lightLd = { 0.9f, 0.9f, 0.9f }; // Diffuse light intensity
    static glm::vec3 lightLs = { 0.9f, 0.9f, 0.9f }; // Specular light intensity
    static glm::vec3 lightLa = { 0.9f, 0.9f, 0.9f }; // Ambient light intensity

    static glm::vec3 matKa = { 0.1f, 0.1f, 0.1f }; // Ambient reflectivity
    static glm::vec3 matKs = { 0.2f, 0.2f, 0.2f }; // Specular reflectivity
    static float matShininess = 1.0f; // Specular shininess factor
};

SceneBasic_Uniform::SceneBasic_Uniform() :
    plane(30.0f, 30.0f, 100, 100, 5, 5)
{
    mesh = ObjMesh::load("media/bs_ears.obj", false, true);
}

// TODO: Make a texture holder singleton class instance for easy access.
enum ETextures
{
    TEX_DIFFUSE_MAP,  // For plane
    TEX_NORMAL_MAP,
    TEX_OGRE_DIFFUSE_MAP, // For ogre heade
    TEX_OGRE_NORMAL_MAP,
    TEX_MOSS,  // For texture mix test
    TEX_MAX_NUM,
};

static std::array<GLuint, TEX_MAX_NUM> textureArray;

bool SceneBasic_Uniform::initScene()
{
    if (!compile())
        return false;

	glEnable(GL_DEPTH_TEST);

    view = glm::lookAt(glm::vec3(1.0f, 1.25f, 1.25f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::mat4(1.0f);

    textureArray[TEX_DIFFUSE_MAP] = Texture::loadTexture("media/texture/Brick_Wall_017_basecolor.jpg");
    textureArray[TEX_NORMAL_MAP] = Texture::loadTexture("media/texture/Brick_Wall_017_normal.jpg");

    textureArray[TEX_OGRE_DIFFUSE_MAP] = Texture::loadTexture("media/texture/ogre_diffuse.png");
    textureArray[TEX_OGRE_NORMAL_MAP] = Texture::loadTexture("media/texture/ogre_normalmap.png");

    textureArray[TEX_MOSS] = Texture::loadTexture("media/texture/moss.png");

    return true;
}

bool SceneBasic_Uniform::compile()
{
	try
    {
		prog.compileShader("shader/02_Textured.vert");
		prog.compileShader("shader/02_Textured.frag");
		//prog.compileShader("shader/02_Textured.geom");
		prog.link();
		prog.use();
	}
    catch (GLSLProgramException &e)
    {
        LOG_CRITICAL(e.what());
        return false;
	}

    return true;
}

static void ImGui_Render()
{
    // Create the frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Beginning of the window
    ImGui::Begin("Render Settings");

    // ----
    ImGui::Spacing();

    // ----
    ImGui::Separator();
    ImGui::Text("Env Settings:");
    ImGui::ColorEdit3("BG Color", (float*)&Configs::bgColor); // Edit 3 floats representing a color

    // ----
    ImGui::Separator();
    ImGui::Text("Shader Settings:");
    ImGui::Checkbox("Use Blinn Phong", &Configs::useBlinnPhong);
    ImGui::Checkbox("Enable Textures", &Configs::useTextures);

    ImGui::BeginDisabled(!Configs::useTextures);

    if (!Configs::useTextures)
        Configs::useTextureMix = false;

    ImGui::Checkbox("Enable Texture Mix", &Configs::useTextureMix);
    ImGui::EndDisabled();

    // ----
    ImGui::Separator();
    ImGui::Text("Toon Settings:");
    ImGui::Checkbox("Enable Toon Shading", &Configs::useToon);
    ImGui::DragFloat("Toon Fraction", &Configs::toonFraction, 0.01, 0.0, 1.0);

    // ----
    ImGui::Separator();
    ImGui::Text("Light & Mat Settings:");
    ImGui::Checkbox("Animate Light", &Configs::animateLight);

    if (ImGui::DragFloat4("Light.Position", (float*)&Configs::lightPos, 0.03333f, -100.f, 100.f))
        Configs::animateLight = false;

    ImGui::ColorEdit3("Light.Ld", (float*)&Configs::lightLd);
    ImGui::ColorEdit3("Light.Ls", (float*)&Configs::lightLs);
    ImGui::ColorEdit3("Light.La", (float*)&Configs::lightLa);
    ImGui::Spacing();
    ImGui::ColorEdit3("Mat.Ka", (float*)&Configs::matKa);
    ImGui::ColorEdit3("Mat.Ks", (float*)&Configs::matKs);
    ImGui::DragFloat("Mat.Shininess", &Configs::matShininess, 0.01, 0.0, 1.0);

    // ----
    ImGui::Separator();
    ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

    // End of the frame
    ImGui::End();

    // Prepare RenderDrawData
    ImGui::Render();

    // Present RenderData
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneBasic_Uniform::update(Camera* camera, float t)
{
    // Update View & Project matrices
    view = camera->GetViewMatrix();
    projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);

    float deltaT = t - tPrev;

    if (tPrev == 0.0f)
        deltaT = 0.0f;

    tPrev = t;

    if (Configs::animateLight)
    {
        angle += rotSpeed * deltaT;
        if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();

        Configs::lightPos = view * glm::vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f);
    }

    // Update settings in case if they are changed
    prog.setUniform("UseBlinnPhong", Configs::useBlinnPhong);
    prog.setUniform("UseTextures", Configs::useTextures);
    prog.setUniform("UseTextureMix", Configs::useTextureMix);

    prog.setUniform("UseToon", Configs::useToon);
    prog.setUniform("ToonFraction", Configs::toonFraction);

    prog.setUniform("Light.Position", Configs::lightPos);

    prog.setUniform("Light.Ld", Configs::lightLd);
    prog.setUniform("Light.Ls", Configs::lightLs);
    prog.setUniform("Light.La", Configs::lightLa);

    prog.setUniform("Material.Ka", Configs::matKa);
    prog.setUniform("Material.Ks", Configs::matKs);
    prog.setUniform("Material.Shininess", Configs::matShininess);
}

void SceneBasic_Uniform::render()
{
    glClearColor(Configs::bgColor.x, Configs::bgColor.y, Configs::bgColor.z, Configs::bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render Plane
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEX_DIFFUSE_MAP]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEX_NORMAL_MAP]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEX_MOSS]);

    model = glm::mat4(1.0f);
    setMatrices();
    plane.render();

    // Render the Mesh
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEX_OGRE_DIFFUSE_MAP]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEX_OGRE_NORMAL_MAP]);

    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
    setMatrices();
    mesh->render();

    // ImGui renders on top of everything
    ImGui_Render();
}

void SceneBasic_Uniform::setMatrices()
{
    glm::mat4 mv = view * model; //we create a model view matrix
    
    prog.setUniform("ModelViewMatrix", mv);
    prog.setUniform("NormalMatrix", glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2])));
    prog.setUniform("MVP", projection * mv);
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::processKey(int key, int scancode, int action, int mods)
{
    switch (key)
    {
    default:
        break;
    }
}
