#include "pch.h"
#include "scenebasic_uniform.h"
#include "helper/texture.h"
#include "helper/camera.h"
#include "helper/objmesh.h"
#include "helper/noisetex.h"

#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"

namespace Configs
{
    // Scene & render settings
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

    // Animation settings
    static bool useWaveAnim = false;
    static float waveFreq = 2.5f;
    static float waveVelocity = 2.5f;
    static float waveAmp = 0.6f;

    // Texture Settings
    static bool usePerlinTexture = true;
    static float perlinBaseFreq = 16.f;
    static float perlinPersistence = 0.5;
    static int perlinWidth = 128;
    static int perlinHeight = 128;
    static bool perlinPeriodic = false;

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
    TEX_PERLIN_NOISE,
    TEX_MAX_NUM,
};

static std::array<GLuint, TEX_MAX_NUM> textureArray;

bool SceneBasic_Uniform::initScene(Camera* camera)
{
    if (!compile())
        return false;

	glEnable(GL_DEPTH_TEST);

    this->camera = camera;
    view = glm::lookAt(glm::vec3(1.0f, 1.25f, 1.25f), glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    projection = glm::mat4(1.0f);

    textureArray[TEX_DIFFUSE_MAP] = Texture::loadTexture("media/texture/Brick_Wall_017_basecolor.jpg");
    textureArray[TEX_NORMAL_MAP] = Texture::loadTexture("media/texture/Brick_Wall_017_normal.jpg");

    textureArray[TEX_OGRE_DIFFUSE_MAP] = Texture::loadTexture("media/texture/ogre_diffuse.png");
    textureArray[TEX_OGRE_NORMAL_MAP] = Texture::loadTexture("media/texture/ogre_normalmap.png");

    textureArray[TEX_MOSS] = Texture::loadTexture("media/texture/moss.png");
    textureArray[TEX_PERLIN_NOISE] = NoiseTex::generate2DTex(
        Configs::perlinBaseFreq,
        Configs::perlinPersistence,
        Configs::perlinWidth,
        Configs::perlinHeight,
        Configs::perlinPeriodic);

    return true;
}

////////////////////////////////

namespace pipeline
{
    enum type
    {
        TEXTURE_MIXED,
        
        MAX,
    };
} // namespace pipeline

namespace program
{
    enum type
    {
        TEXTURE_MIXED_VERT_DEFAULT,
        TEXTURE_MIXED_VERT_WAVE,
        TEXTURE_MIXED_FRAG_DEFAULT,
        
        MAX,
    };
} // namespace program

// Uniform buffer objects, shared across all programs
namespace ubo
{
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
        ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT] = sm.Create();
        ProgramName[program::TEXTURE_MIXED_VERT_WAVE] = sm.Create();
        ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT] = sm.Create();

        glAttachShader(ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT], sm.Compile("shader/TextureMixed.vert")); GLERR;
        glAttachShader(ProgramName[program::TEXTURE_MIXED_VERT_WAVE], sm.Compile("shader/TextureMixedWave.vert")); GLERR;
        glAttachShader(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], sm.Compile("shader/TextureMixed.frag")); GLERR;

        sm.Link(ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT]);
        sm.Link(ProgramName[program::TEXTURE_MIXED_VERT_WAVE]);
        sm.Link(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT]);

        sm.CleanupProgram(ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT]);
        sm.CleanupProgram(ProgramName[program::TEXTURE_MIXED_VERT_WAVE]);
        sm.CleanupProgram(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT]);

        // Test pipeline program stages
        glUseProgramStages(PipelineName[pipeline::TEXTURE_MIXED], GL_VERTEX_SHADER_BIT, ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT]); GLERR;
        glUseProgramStages(PipelineName[pipeline::TEXTURE_MIXED], GL_FRAGMENT_SHADER_BIT, ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT]); GLERR;
        sm.ValidatePipeline(PipelineName[pipeline::TEXTURE_MIXED]);
	}
    catch (ShaderManagerException&e)
    {
        LOG_CRITICAL(e.what());
        return false;
	}

    // Generate matrices uniform buffer objects
    setupUBO();

    return true;
}

static void ImGui_Render()
{
    // Create the frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    {
        // Beginning of the window
        ImGui::Begin("Animation and Texture Settings");

        // ----
        ImGui::Spacing();

        // ----
        ImGui::Separator();

        ImGui::Checkbox("Enable Wave Animation", &Configs::useWaveAnim);

        ImGui::BeginDisabled(!Configs::useWaveAnim);
        ImGui::DragFloat("Freq", &Configs::waveFreq, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Velocity", &Configs::waveVelocity, 0.01f, 0.0f, 10.0f);
        ImGui::DragFloat("Amp", &Configs::waveAmp, 0.01f, 0.0f, 10.0f);
        ImGui::EndDisabled();

        // ----
        ImGui::Spacing();

        // ----
        ImGui::Separator();
        ImGui::Text("Texture Settings:");
        ImGui::Checkbox("Use Perlin Noise Generated Texture", &Configs::usePerlinTexture);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Customize Perlin Noise:");

        ImGui::BeginDisabled(!Configs::usePerlinTexture);
        ImGui::DragFloat("BaseFreq", &Configs::perlinBaseFreq, 0.01f, 0.0f, 100.0f);
        ImGui::DragFloat("Persistence", &Configs::perlinPersistence, 0.01f, 0.0f, 1.0f);
        ImGui::DragInt("Width", &Configs::perlinWidth, 1, 128, 1024);
        ImGui::DragInt("Height", &Configs::perlinHeight, 1, 128, 1024);
        ImGui::Checkbox("Is Periodic?", &Configs::perlinPeriodic);

        if (ImGui::Button("Regenerate Texture"))
        {
            textureArray[TEX_PERLIN_NOISE] = NoiseTex::generate2DTex(
                Configs::perlinBaseFreq,
                Configs::perlinPersistence,
                Configs::perlinWidth,
                Configs::perlinHeight,
                Configs::perlinPeriodic);
        }
        ImGui::EndDisabled();

        // End of the frame
        ImGui::End();
    }

    {
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
        ImGui::DragFloat("Toon Fraction", &Configs::toonFraction, 0.01f, 0.0f, 10.0f);
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
        ImGui::DragFloat("Mat.Shininess", &Configs::matShininess, 0.01f, 0.0f, 1.0f);
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
    }

    // Prepare RenderDrawData
    ImGui::Render();

    // Present RenderData
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void SceneBasic_Uniform::update(float t)
{
    // Update View & Project matrices
    view = camera->GetViewMatrix();
    projection = glm::perspective(glm::radians(camera->Zoom), (float)width / (float)height, 0.1f, 100.0f);

    float deltaT = t - timePrev;

    if (timePrev == 0.0f)
        deltaT = 0.0f;

    timePrev = t;

    if (Configs::animateLight)
    {
        Configs::lightAngle += rotSpeed * deltaT;
        if (Configs::lightAngle > glm::two_pi<float>()) Configs::lightAngle -= glm::two_pi<float>();
    }

    Configs::cameraPos = camera->Position;

    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT], "LightPosition", view * glm::vec4(Configs::lightDist * cos(Configs::lightAngle), 1.0f, Configs::lightDist * sin(Configs::lightAngle), 1.0f));

    // Default mixed texture uniforms
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "UseBlinnPhong", Configs::useBlinnPhong);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "UseTextures", Configs::useTextures);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "UseTextureMix", Configs::useTextureMix);
    
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "UseToon", Configs::useToon);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "UseAdditiveToon", Configs::useAdditiveToon);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "ToonFraction", Configs::toonFraction);
    
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "Light.Ld", Configs::lightLd);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "Light.Ls", Configs::lightLs);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "Light.La", Configs::lightLa);
    
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "Material.Ka", Configs::matKa);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "Material.Ks", Configs::matKs);
    sm.SetUniform(ProgramName[program::TEXTURE_MIXED_FRAG_DEFAULT], "Material.Shininess", Configs::matShininess);

    // Animation uniforms
    if (Configs::useWaveAnim)
    {
        sm.SetUniform(ProgramName[program::TEXTURE_MIXED_VERT_WAVE], "Time", timePrev);
        sm.SetUniform(ProgramName[program::TEXTURE_MIXED_VERT_WAVE], "WaveFreq", Configs::waveFreq);
        sm.SetUniform(ProgramName[program::TEXTURE_MIXED_VERT_WAVE], "WaveVelocity", Configs::waveVelocity);
        sm.SetUniform(ProgramName[program::TEXTURE_MIXED_VERT_WAVE], "WaveAmp", Configs::waveAmp);
    }
}

void SceneBasic_Uniform::render()
{
    glUseProgram(0);
    glBindProgramPipeline(PipelineName[pipeline::TEXTURE_MIXED]); GLERR;

    glClearColor(Configs::bgColor.x, Configs::bgColor.y, Configs::bgColor.z, Configs::bgColor.w);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    std::function<void()> funcRender =
        [&]()
    {
        // Render Plane
        if (Configs::usePerlinTexture)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureArray[TEX_PERLIN_NOISE]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureArray[TEX_DIFFUSE_MAP]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureArray[TEX_NORMAL_MAP]);
        }

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureArray[TEX_MOSS]);

        model = glm::mat4(1.0f);
        setMatrices();

        // Simply, swap pipeline vertex program stages
        if (Configs::useWaveAnim)
        {
            glUseProgramStages(PipelineName[pipeline::TEXTURE_MIXED], GL_VERTEX_SHADER_BIT, ProgramName[program::TEXTURE_MIXED_VERT_WAVE]);
        }
        else
        {
            glUseProgramStages(PipelineName[pipeline::TEXTURE_MIXED], GL_VERTEX_SHADER_BIT, ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT]);
        }

        plane.render();

        // Wave animation cant be used on the ogre head
        if (Configs::useWaveAnim)
            glUseProgramStages(PipelineName[pipeline::TEXTURE_MIXED], GL_VERTEX_SHADER_BIT, ProgramName[program::TEXTURE_MIXED_VERT_DEFAULT]);

        // Render the Mesh
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureArray[TEX_OGRE_DIFFUSE_MAP]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureArray[TEX_OGRE_NORMAL_MAP]);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 2.0f, 0.0f));
        setMatrices();
        mesh->render();
    };

    //glBindProgramPipeline(PipelineName[pipeline::DIFFUSE]); GLERR;
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
