#include "pch.h"
#include "scenebasic_uniform.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"

static ImVec4 s_ClearColor = ImVec4(25 / 255.0f, 25 / 255.0f, 25 / 255.0f, 1.00f);

//constructor for torus
SceneBasic_Uniform::SceneBasic_Uniform() :
    plane(10.0f, 10.0f, 100, 100),
    torus(0.7f, 0.3f, 50, 50)
{
    mesh = ObjMesh::load("media/pig_triangulated.obj", true);
}

//constructor for teapot
//SceneBasic_Uniform::SceneBasic_Uniform() : teapot(13, glm::translate(mat4(1.0f), vec3(0.0f, 1.5f, 0.25f))) {}

bool SceneBasic_Uniform::initScene()
{
    if (!compile())
        return false;

	glEnable(GL_DEPTH_TEST);

    view = glm::lookAt(glm::vec3(0.5f, 0.75f, 0.75f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::mat4(1.0f);

    float x, z;
    for (int i = 0; i < 3; i++)
    {
        std::stringstream name;
        name << "lights[" << i << "].Position";
        x = 2.0f * cosf((glm::two_pi<float>() / 3) * i);
        z = 2.0f * sinf((glm::two_pi<float>() / 3) * i);
        prog.setUniform(name.str().c_str(), view * glm::vec4(x, 1.2f, z +
            1.0f, 1.0f));
    }

    // Diffuse and specular light intensity
    prog.setUniform("lights[0].L", glm::vec3(0.0f, 0.0f, 1.0f));
    prog.setUniform("lights[1].L", glm::vec3(0.0f, 1.0f, 0.0f));
    prog.setUniform("lights[2].L", glm::vec3(1.0f, 0.0f, 0.0f));

    // Ambient light intensity
    prog.setUniform("lights[0].La", glm::vec3(0.3f, 0.3f, 0.3f));
    prog.setUniform("lights[1].La", glm::vec3(0.3f, 0.3f, 0.3f));
    prog.setUniform("lights[2].La", glm::vec3(0.3f, 0.3f, 0.3f));

    return true;
}

bool SceneBasic_Uniform::compile()
{
	try
    {
		prog.compileShader("shader/basic_uniform.vert");
		prog.compileShader("shader/basic_uniform.frag");
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

void SceneBasic_Uniform::update(float t)
{
    if (m_animate)
    {
        _radians = std::max<float>(90.f, _radians + 0.08f);
    }
}

void SceneBasic_Uniform::render()
{
    glClearColor(s_ClearColor.x, s_ClearColor.y, s_ClearColor.z, s_ClearColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    prog.setUniform("Material.Kd", 0.4f, 0.4f, 0.4f);
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("Material.Ka", 0.5f, 0.5f, 0.5f);
    prog.setUniform("Material.Shininess", 180.0f);
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    setMatrices();
    mesh->render();


    prog.setUniform("Material.Kd", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Ks", 0.9f, 0.9f, 0.9f);
    prog.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
    prog.setUniform("Material.Shininess", 180.0f);
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.45f, 0.0f));
    setMatrices();
    plane.render();




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

void SceneBasic_Uniform::UpdateViewMatrix(glm::mat4 viewMatrix)
{
    view = viewMatrix;
}

void SceneBasic_Uniform::UpdateProjMatrix(glm::mat4 projMatrix)
{
    projection = projMatrix;
}
