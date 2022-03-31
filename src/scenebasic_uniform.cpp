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
    static glm::vec3 cameraPos;

    static bool useBlinnPhong = true;
    static bool useTextures = true;
    static bool useTextureMix = true;

    static bool useToon = false;
    static bool useAdditiveToon = false;
    static float toonFraction = 1.0f;

    static bool animateLight = true;
    static float lightAngle = 0.0f; // Light angle
    static float lightDist = 10.0f; // Light distance from center
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

////////////////////////////////

namespace pipeline
{
    enum type
    {
        DIFFUSE,
        PHONG,
        
        MAX,
    };
} // namespace pipeline

namespace program
{
    enum type
    {
        DIFFUSE_VERT,
        DIFFUSE_FRAG,
        PHONG_VERT,
        PHONG_FRAG,
        
        MAX,
    };
} // namespace program

namespace ubo
{
    // Uniform buffer objects, shared across all programs
    enum type
    {
        MATRICES,
        MAX,
    };

    // Bound indexes set in the vertex shader
    GLuint MATRICES_INDEX = 0;
}

std::array<GLuint, pipeline::MAX> PipelineName;
std::array<GLuint, program::MAX> ProgramName;
std::array<GLuint, ubo::MAX> UBOName;

////////////////////////////////

GLuint fbo;

void SceneBasic_Uniform::setupFBO()
{
    // The depth buffer
    GLuint depthBuf;
    glGenRenderbuffers(1, &depthBuf);
    glBindRenderbuffer(GL_RENDERBUFFER, depthBuf);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);

    // The diffuse+specular component
    glActiveTexture(GL_TEXTURE0);
    GLuint diffSpecTex;
    glGenTextures(1, &diffSpecTex);
    glBindTexture(GL_TEXTURE_2D, diffSpecTex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, width, height);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuf);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, diffSpecTex, 0);

    GLenum DrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, DrawBuffers);

    GLenum result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("no framebuf.");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void SceneBasic_Uniform::setupUBO()
{
    // NOTE NOTE NOTE: When uploading UBO treat mat3 as mat4
    // mat4:MVP, mat4:ModelViewMatrix, mat3:NormalMatrix
    // (READ std140 specifications) https://www.khronos.org/opengl/wiki/Talk:Uniform_Buffer_Object
    // If the member is a three-component vector with components consuming N basic machine units, the base alignment is 4N.
    GLsizeiptr uboSize = 3 * sizeof(glm::mat4);

    glGenBuffers(1, &UBOName[ubo::MATRICES]);

    glBindBuffer(GL_UNIFORM_BUFFER, UBOName[ubo::MATRICES]);
    glBufferData(GL_UNIFORM_BUFFER, uboSize, 0, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    // Define the range of the buffer that links to a uniform binding point
    glBindBufferRange(GL_UNIFORM_BUFFER, ubo::MATRICES_INDEX, UBOName[ubo::MATRICES], 0, uboSize);
}

bool SceneBasic_Uniform::compile()
{
    glGenProgramPipelines(pipeline::MAX, &PipelineName[0]); GLERR;

	try
    {
        ProgramName[program::DIFFUSE_VERT] = sm.Create();
        ProgramName[program::DIFFUSE_FRAG] = sm.Create();

        glAttachShader(ProgramName[program::DIFFUSE_VERT], sm.Compile("shader/DiffuseSingleTexture.vert")); GLERR;
        glAttachShader(ProgramName[program::DIFFUSE_FRAG], sm.Compile("shader/DiffuseSingleTexture.frag")); GLERR;

        sm.Link(ProgramName[program::DIFFUSE_VERT]);
        sm.Link(ProgramName[program::DIFFUSE_FRAG]);

        sm.CleanupProgram(ProgramName[program::DIFFUSE_VERT]);
        sm.CleanupProgram(ProgramName[program::DIFFUSE_FRAG]);

        // Test pipeline program stages
        glUseProgramStages(PipelineName[pipeline::DIFFUSE], GL_VERTEX_SHADER_BIT, ProgramName[program::DIFFUSE_VERT]); GLERR;
        glUseProgramStages(PipelineName[pipeline::DIFFUSE], GL_FRAGMENT_SHADER_BIT, ProgramName[program::DIFFUSE_FRAG]); GLERR;
        sm.ValidatePipeline(PipelineName[pipeline::DIFFUSE]);
	}
    catch (ShaderManagerException&e)
    {
        LOG_CRITICAL(e.what());
        return false;
	}

    try
    {
        ProgramName[program::PHONG_VERT] = sm.Create();
        ProgramName[program::PHONG_FRAG] = sm.Create();

        glAttachShader(ProgramName[program::PHONG_VERT], sm.Compile("shader/BlinnPhongPerFragSingleLight.vert")); GLERR;
        glAttachShader(ProgramName[program::PHONG_FRAG], sm.Compile("shader/BlinnPhongPerFragSingleLight.frag")); GLERR;

        sm.Link(ProgramName[program::PHONG_VERT]);
        sm.Link(ProgramName[program::PHONG_FRAG]);

        sm.CleanupProgram(ProgramName[program::PHONG_VERT]);
        sm.CleanupProgram(ProgramName[program::PHONG_FRAG]);

        // Test pipeline program stages
        glUseProgramStages(PipelineName[pipeline::PHONG], GL_VERTEX_SHADER_BIT, ProgramName[program::PHONG_VERT]); GLERR;
        glUseProgramStages(PipelineName[pipeline::PHONG], GL_FRAGMENT_SHADER_BIT, ProgramName[program::PHONG_FRAG]); GLERR;
        sm.ValidatePipeline(PipelineName[pipeline::PHONG]);
    }
    catch (ShaderManagerException& e)
    {
        LOG_CRITICAL(e.what());
        return false;
    }

    // Generate matrices uniform buffer objects
    setupUBO();

    // Generate frame buffer object
    setupFBO();

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
    ImGui::Checkbox("Use Additive Toon", &Configs::useAdditiveToon);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("If enabled will use Phong + Toon mixed otherwise pure toon effect will be applied");
        ImGui::EndTooltip();
    }
    ImGui::DragFloat("Toon Fraction", &Configs::toonFraction, 0.01, 0.0, 10.0);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Toon intensity");
        ImGui::EndTooltip();
    }
    // ----
    ImGui::Separator();
    ImGui::Text("Light & Mat Settings:");
    ImGui::Checkbox("Animate Light", &Configs::animateLight);

    if (ImGui::DragFloat("Light Angle", &Configs::lightAngle, 0.03333f, -100.f, 100.f))
        Configs::animateLight = false;
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Light angle around the center of the scene");
        ImGui::EndTooltip();
    }
    ImGui::DragFloat("Light Dist", &Configs::lightDist, 0.01f, -100.f, 100.f);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Light distance from center of the scene");
        ImGui::EndTooltip();
    }
    ImGui::Spacing();
    ImGui::ColorEdit3("Light.Ld", (float*)&Configs::lightLd);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Diffuse light intensity");
        ImGui::EndTooltip();
    }
    ImGui::ColorEdit3("Light.Ls", (float*)&Configs::lightLs);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Specular light intensity");
        ImGui::EndTooltip();
    }
    ImGui::ColorEdit3("Light.La", (float*)&Configs::lightLa);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Ambient light intensity");
        ImGui::EndTooltip();
    }
    ImGui::Spacing();
    ImGui::ColorEdit3("Mat.Ka", (float*)&Configs::matKa);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Ambient reflectivity");
        ImGui::EndTooltip();
    }
    ImGui::ColorEdit3("Mat.Ks", (float*)&Configs::matKs);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Specular reflectivity");
        ImGui::EndTooltip();
    }
    ImGui::DragFloat("Mat.Shininess", &Configs::matShininess, 0.01, 0.0, 1.0);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Specular shininess factor");
        ImGui::EndTooltip();
    }


    // ----
    ImGui::Separator();
    ImGui::Text("Camera Pos: %.3f, %.3f, %.3f", Configs::cameraPos.x, Configs::cameraPos.y, Configs::cameraPos.z);

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
        Configs::lightAngle += rotSpeed * deltaT;
        if (Configs::lightAngle > glm::two_pi<float>()) Configs::lightAngle -= glm::two_pi<float>();
    }

    Configs::cameraPos = camera->Position;

    sm.SetUniform(ProgramName[program::PHONG_VERT], "LightPosition", view * glm::vec4(Configs::lightDist * cos(Configs::lightAngle), 1.0f, Configs::lightDist * sin(Configs::lightAngle), 1.0f));

    sm.SetUniform(ProgramName[program::PHONG_FRAG], "Light.Ld", Configs::lightLd);
    sm.SetUniform(ProgramName[program::PHONG_FRAG], "Light.Ls", Configs::lightLs);
    sm.SetUniform(ProgramName[program::PHONG_FRAG], "Light.La", Configs::lightLa);
    
    sm.SetUniform(ProgramName[program::PHONG_FRAG], "Material.Ka", Configs::matKa);
    sm.SetUniform(ProgramName[program::PHONG_FRAG], "Material.Ks", Configs::matKs);
    sm.SetUniform(ProgramName[program::PHONG_FRAG], "Material.Shininess", Configs::matShininess);
}

void SceneBasic_Uniform::render()
{
    glUseProgram(0);
    glClearColor(Configs::bgColor.x, Configs::bgColor.y, Configs::bgColor.z, Configs::bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::function<void()> funcRender =
        [&]()
    {
        //glBindProgramPipeline(PipelineName[pipeline::PHONG]); GLERR; // TEST
        // Render Plane
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, textureArray[TEX_DIFFUSE_MAP]);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, textureArray[TEX_NORMAL_MAP]);
        //glActiveTexture(GL_TEXTURE2);
        //glBindTexture(GL_TEXTURE_2D, textureArray[TEX_MOSS]);

        model = glm::mat4(1.0f);
        setMatrices();
        plane.render();

        //glBindProgramPipeline(PipelineName[pipeline::DIFFUSE]); GLERR; // TEST
        // Render the Mesh
        //glActiveTexture(GL_TEXTURE0);
        //glBindTexture(GL_TEXTURE_2D, textureArray[TEX_OGRE_DIFFUSE_MAP]);
        //glActiveTexture(GL_TEXTURE1);
        //glBindTexture(GL_TEXTURE_2D, textureArray[TEX_OGRE_NORMAL_MAP]);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        setMatrices();
        mesh->render();
    };

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureArray[TEX_DIFFUSE_MAP]);

    glBindProgramPipeline(PipelineName[pipeline::DIFFUSE]);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    funcRender();

    ///////////////////////////////////

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindProgramPipeline(PipelineName[pipeline::PHONG]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    funcRender();

    // ImGui renders on top of everything
    ImGui_Render();
}

void SceneBasic_Uniform::setMatrices()
{
    glm::mat4 mv = view * model; //we create a model view matrix

    glBindBuffer(GL_UNIFORM_BUFFER, UBOName[ubo::MATRICES]);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection * mv));
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(mv));
    glBufferSubData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(mv));
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
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
