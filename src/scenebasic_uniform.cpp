#include "pch.h"
#include "scenebasic_uniform.h"
#include "helper/texture.h"
#include "helper/camera.h"
#include "helper/objmesh.h"

#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"

static ImVec4 s_ClearColor = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.00f);

//constructor for torus
SceneBasic_Uniform::SceneBasic_Uniform() :
    plane(30.0f, 30.0f, 100, 100, 5, 5)
{
    mesh = ObjMesh::load("media/bs_ears.obj", false, true);
}

static GLuint texDiffuseMap;
static GLuint texNormalMap;

static GLuint texOgreDiffuseMap;
static GLuint texOgreNormalMap;

static GLuint texMoss;

bool SceneBasic_Uniform::initScene()
{
    if (!compile())
        return false;

	glEnable(GL_DEPTH_TEST);

    view = glm::lookAt(glm::vec3(1.0f, 1.25f, 1.25f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::mat4(1.0f);

    // Diffuse light intensity
    prog.setUniform("Light.Ld", glm::vec3(0.9f, 0.9f, 0.9f));
    // Specular light intensity
    prog.setUniform("Light.Ls", glm::vec3(0.9f, 0.9f, 0.9f));
    // Ambient light intensity
    prog.setUniform("Light.La", glm::vec3(0.9f, 0.9f, 0.9f));

    texDiffuseMap = Texture::loadTexture("media/texture/Brick_Wall_017_basecolor.jpg");
    texNormalMap = Texture::loadTexture("media/texture/Brick_Wall_017_normal.jpg");
    
    texOgreDiffuseMap = Texture::loadTexture("media/texture/ogre_diffuse.png");
    texOgreNormalMap = Texture::loadTexture("media/texture/ogre_normalmap.png");
    
    texMoss = Texture::loadTexture("media/texture/moss.png");

    return true;
}

bool SceneBasic_Uniform::compile()
{
	try
    {
		prog.compileShader("shader/02_Textured.vert");
		prog.compileShader("shader/02_Textured.frag");
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
    ImGui::Text("Background Settings:");
    ImGui::ColorEdit3("Color", (float*)&s_ClearColor); // Edit 3 floats representing a color

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

    //if (this->m_animate)
    {
        angle += rotSpeed * deltaT;
        if (angle > glm::two_pi<float>()) angle -= glm::two_pi<float>();
    }
}

void SceneBasic_Uniform::render()
{
    glClearColor(s_ClearColor.x, s_ClearColor.y, s_ClearColor.z, s_ClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render Plane
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texDiffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texNormalMap);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, texMoss);

    prog.setUniform("Light.Position", view * glm::vec4(10.0f * cos(angle), 1.0f, 10.0f * sin(angle), 1.0f));
    prog.setUniform("Material.Ks", 0.2f, 0.2f, 0.2f);
    prog.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Shininess", 1.0f);

    model = glm::mat4(1.0f);
    setMatrices();
    plane.render();

    // Render the Mesh
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texOgreDiffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texOgreNormalMap);

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
    
    prog.setUniform("ModelViewMatrix", mv); //set the uniform for the model view matrix
    
    prog.setUniform("NormalMatrix", glm::mat3(glm::vec3(mv[0]), glm::vec3(mv[1]), glm::vec3(mv[2]))); //we set the uniform for normal matrix
    
    prog.setUniform("MVP", projection * mv); //we set the model view matrix by multiplying the mv with the projection matrix
}

void SceneBasic_Uniform::resize(int w, int h)
{
    glViewport(0, 0, w, h);
    width = w;
    height = h;
    projection = glm::perspective(glm::radians(70.0f), (float)w / h, 0.3f, 100.0f);
}

void SceneBasic_Uniform::ToggleBlinnPhong()
{
    isBlinnPhong = !isBlinnPhong;
    LOG_INFO("Use Blinn Phong: {}", isBlinnPhong);
    prog.setUniform("UseBlinnPhong", isBlinnPhong);
}
