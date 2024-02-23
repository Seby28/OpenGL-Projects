#include "lab/assignment3/assignment3.h"

#include <iostream>
#include <string>
#include <vector>

#include <stb/stb_image.h>


using namespace m1;
using namespace std;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Assignment3::Assignment3()
{
    boatCount = 4;

    lightDirection = glm::vec3(20, -10, 40);

    mainColor = glm::vec3(1);

    maxSliderValue = glm::ivec3(280);
    click = glm::bvec3(false);

    resolution = window->GetResolution();
}


Assignment3::~Assignment3()
{
}


void Assignment3::Init()
{
    camera = new gfxc::Camera();

    {
        Texture2D* texture = LoadTexture("src\\lab\\assignment3\\images\\water.jpg");
        textures["water"] = texture;
    }

    {
        Texture2D* texture = LoadTexture("src\\lab\\assignment3\\images\\ground.jpg");
        textures["ground"] = texture;
    }

    {
        Texture2D* texture = LoadTexture("src\\lab\\assignment3\\images\\concrete.png");
        textures["concrete"] = texture;
    }

    {
        Texture2D* texture = LoadTexture("src\\lab\\assignment3\\images\\default.png");
        textures["default"] = texture;
    }

    {
        Texture2D* texture = LoadTexture("src\\lab\\assignment3\\images\\crate.jpg");
        textures["crate"] = texture;
    }

    {
        Texture2D* texture = LoadTexture("src\\lab\\assignment3\\images\\moon.jpg");
        textures["moon"] = texture;
    }

    {
        Mesh* mesh = new Mesh("cylinder");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "cylinder.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("sphere");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "sphere.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("cube");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("plane");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "plane50.obj");
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    LoadShader("Texture", "Texture", "Texture");
    LoadShader("Water", "Water", "Water");
    LoadShader("Emissive", "Emissive", "Emissive");
    LoadShader("UI", "UI", "UI");
    LoadShader("Mix", "UI", "MixColors");
    LoadShader("Shader", "UI", "Circle");

    SpawnBoats();
    pointLightPositions.push_back(glm::vec3(0, 3, 0));
    pointLightColors.push_back(glm::vec3(1));

    spotLightPositions.push_back(glm::vec3(0, 3, 0));
    spotLightPositions.push_back(glm::vec3(0, 3, 0));

    spotLightDirections.push_back(glm::vec3(0, -0.15f, -1));
    spotLightDirections.push_back(glm::vec3(0, -0.15f,  1));

    sliderValue = maxSliderValue;
}


void Assignment3::LoadShader(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, "src/lab", "assignment3", "shaders");

    {
        Shader* shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, vertexShader + "_VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, fragmentShader + "_FS.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


Texture2D* Assignment3::CreateTexture(unsigned int width, unsigned int height, unsigned int channels, unsigned char* data)
{
    GLuint textureID = 0;
    unsigned int size = width * height * channels;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    if (GLEW_EXT_texture_filter_anisotropic)
    {
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 4);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    CheckOpenGLError();

    int imageFormat;
    switch (channels)
    {
    case 1: imageFormat = GL_RED;
        break;
    case 2: imageFormat = GL_RG;
        break;
    case 3: imageFormat = GL_RGB;
        break;
    case 4: imageFormat = GL_RGBA;
        break;
    default:
        break;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, imageFormat, width, height, 0, imageFormat, GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);

    CheckOpenGLError();

    // Save the texture into a wrapper Texture2D class for using easier later during rendering phase
    Texture2D* texture = new Texture2D();
    texture->Init(textureID, width, height, channels);

    SAFE_FREE_ARRAY(data);
    return texture;
}


Texture2D* Assignment3::LoadTexture(const char* imagePath)
{
    int width, height, channels;
    unsigned char* imageData = stbi_load(imagePath, &width, &height, &channels, 0);

    return CreateTexture(width, height, channels, imageData);
}


void Assignment3::RenderColorMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    int location;

    location = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Send the view matrix unfirom
    glm::mat4 view = GetSceneCamera()->GetViewMatrix();
    location = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(view));

    // Send the projection matrix uniform
    glm::mat4 projection = GetSceneCamera()->GetProjectionMatrix();
    location = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));

    location = glGetUniformLocation(shader->program, "resolution");
    glUniform2iv(location, 1, glm::value_ptr(resolution));

    location = glGetUniformLocation(shader->program, "color");
    glUniform3fv(location, 1, glm::value_ptr(color));

    float time = (float)Engine::GetElapsedTime();
    location = glGetUniformLocation(shader->program, "time");
    glUniform1f(location, time);

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


void Assignment3::RenderTextureMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    int location;

    location = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    // Send the view matrix unfirom
    glm::mat4 view = GetSceneCamera()->GetViewMatrix();
    location = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(view));

    // Send the projection matrix uniform
    glm::mat4 projection = GetSceneCamera()->GetProjectionMatrix();
    location = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projection));

    location = glGetUniformLocation(shader->program, "lightDirection");
    glUniform3fv(location, 1, glm::value_ptr(lightDirection));

    location = glGetUniformLocation(shader->program, "pointLightPositions");
    glUniform3fv(location, 5, glm::value_ptr(pointLightPositions[0]));

    location = glGetUniformLocation(shader->program, "pointLightColors");
    glUniform3fv(location, 5, glm::value_ptr(pointLightColors[0]));

    location = glGetUniformLocation(shader->program, "spotLightPositions");
    glUniform3fv(location, 2, glm::value_ptr(spotLightPositions[0]));

    location = glGetUniformLocation(shader->program, "spotLightDirections");
    glUniform3fv(location, 2, glm::value_ptr(spotLightDirections[0]));

    glm::vec3 eyePosition = GetSceneCamera()->m_transform->GetWorldPosition();
    location = glGetUniformLocation(shader->program, "eyePosition");
    glUniform3fv(location, 1, glm::value_ptr(eyePosition));

    location = glGetUniformLocation(shader->program, "resolution");
    glUniform2iv(location, 1, glm::value_ptr(resolution));

    location = glGetUniformLocation(shader->program, "mainColor");
    glUniform3fv(location, 1, glm::value_ptr(mainColor));

    float time = (float)Engine::GetElapsedTime();
    location = glGetUniformLocation(shader->program, "time");
    glUniform1f(location, time);

    if (texture)
    {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture1"), 0);
    }

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


int Assignment3::GetRandomNumber(int min, int max)
{
    int range = max - min + 1;
    int number = (rand() % 2 == 0) ? min + rand() % range : -max + rand() % range;

    return number;
}


void Assignment3::RenderEnvironment()
{
    {   // Lighthouse body
        glm::mat4 model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(1, 3, 1));
        RenderTextureMesh(meshes["cylinder"], shaders["Texture"], model, textures["default"]);
    }

    {   // Light
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 3, 0));
        model = glm::scale(model, glm::vec3(1, 0.5f, 1));
        RenderColorMesh(meshes["cylinder"], shaders["Color"], model, mainColor);
    }

    {   // Lighthouse top
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 3.5f, 0));
        model = glm::scale(model, glm::vec3(1, 0.5f, 1));
        RenderTextureMesh(meshes["cylinder"], shaders["Texture"], model, textures["default"]);
    }

    {   // Island
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 0, 0));
        model = glm::scale(model, glm::vec3(4, 1, 4));
        RenderTextureMesh(meshes["sphere"], shaders["Texture"], model, textures["ground"]);
    }

    {   // Water
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 0, 0));
        model = glm::scale(model, glm::vec3(1, 1, 1));
        RenderTextureMesh(meshes["plane"], shaders["Water"], model, textures["water"]);
    }

    {   // Moon
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(-20, 10, -40));
        model = glm::scale(model, glm::vec3(8));
        RenderTextureMesh(meshes["sphere"], shaders["Emissive"], model, textures["moon"]);
    }
}


void Assignment3::SpawnBoats()
{
    for (int i = 0; i < boatCount; i++)
    {
        int radius = GetRandomNumber(8, 10);
        glm::vec3 position = glm::vec3(0);
        position.x = radius * sin(glm::radians(float(rand() % 180)));
        position.z = radius * cos(glm::radians(float(rand() % 180)));

        float speed = (float)GetRandomNumber(10, 40);

        boatPositions.push_back(position);
        boatSpeedAngle.push_back(std::make_pair(speed, 0.0f));

        pointLightPositions.push_back(position);
        pointLightColors.push_back(glm::vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f));
    }
}


void Assignment3::RenderBoat(glm::vec3& position, float& angle, int sign)
{
    glm::mat4 model = glm::mat4(1);
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
    model = glm::translate(model, position);
    model = glm::rotate(model, glm::radians(sign * 90.0f), glm::vec3(0, 1, 0));
    RenderTextureMesh(meshes["cube"], shaders["Texture"], model, textures["crate"]);
}


void Assignment3::FrameStart()
{
    glClearColor(0.01f, 0.05f, 0.12f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}


void Assignment3::Update(float deltaTimeSeconds)
{
    float time = (float)Engine::GetElapsedTime();

    RenderEnvironment();

    for (int i = 0; i < boatPositions.size(); i++)
    {
        boatSpeedAngle[i].second += boatSpeedAngle[i].first * deltaTimeSeconds;
        int sign = (boatSpeedAngle[i].first > 0) ? 1 : -1;

        RenderBoat(boatPositions[i], boatSpeedAngle[i].second, sign);
        pointLightPositions[i] = boatPositions[i] + glm::vec3(0, 1, 0);
        
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0), glm::radians(boatSpeedAngle[i].second), glm::vec3(0, 1, 0));
        pointLightPositions[i] = glm::vec3(glm::mat3(rotation) * pointLightPositions[i]);
    }

    {
        glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(0.5f), glm::vec3(0, 1, 0));
        spotLightDirections[0] = glm::mat3(rotation) * spotLightDirections[0];
    }
    
    {
        glm::mat4 rotation = glm::rotate(glm::mat4(1), glm::radians(0.5f), glm::vec3(0, 1, 0));
        spotLightDirections[1] = glm::mat3(rotation) * spotLightDirections[1];
    }

    // Red slider
    glViewport(10, 110, sliderValue.x, 40);
    RenderTextureMesh(meshes["quad"], shaders["Mix"], glm::mat4(1), textures["water"]);
    glViewport(10, 110, maxSliderValue.x, 40);
    RenderColorMesh(meshes["quad"], shaders["UI"], glm::mat4(1), glm::vec3(0.25f));

    // Green slider
    glViewport(10, 60, sliderValue.y, 40);
    RenderTextureMesh(meshes["quad"], shaders["Mix"], glm::mat4(1), textures["crate"]);
    glViewport(10, 60, maxSliderValue.y, 40);
    RenderColorMesh(meshes["quad"], shaders["UI"], glm::mat4(1), glm::vec3(0.25f));

    // Blue slider
    glViewport(10, 10, sliderValue.z, 40);
    RenderTextureMesh(meshes["quad"], shaders["Mix"], glm::mat4(1), textures["ground"]);
    glViewport(10, 10, maxSliderValue.z, 40);
    RenderColorMesh(meshes["quad"], shaders["UI"], glm::mat4(1), glm::vec3(0.25f));

    glViewport(50, 500, 200, 200);
    RenderColorMesh(meshes["quad"], shaders["Shader"], glm::mat4(1), glm::vec3(0, 0, 1));
}


void Assignment3::FrameEnd()
{
    // DrawCoordinateSystem();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Assignment3::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
}


void Assignment3::OnKeyPress(int key, int mods)
{
    // Add key press event
}


void Assignment3::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Assignment3::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event

    if (click.x == true)
    {
        sliderValue.x = mouseX;
        if (sliderValue.x < 0)
        {
            sliderValue.x = 0;
        }
        if (sliderValue.x > maxSliderValue.x)
        {
            sliderValue.x = maxSliderValue.x;
        }
        mainColor.x = (float)sliderValue.x / maxSliderValue.x;
    }
    if (click.y == true)
    {
        sliderValue.y = mouseX;
        if (sliderValue.y < 0)
        {
            sliderValue.y = 0;
        }
        if (sliderValue.y > maxSliderValue.y)
        {
            sliderValue.y = maxSliderValue.y;
        }
        mainColor.y = (float)sliderValue.y / maxSliderValue.y;
    }
    if (click.z == true)
    {
        sliderValue.z = mouseX;
        if (sliderValue.z < 0)
        {
            sliderValue.z = 0;
        }
        if (sliderValue.z > maxSliderValue.z)
        {
            sliderValue.z = maxSliderValue.z;
        }
        mainColor.z = (float)sliderValue.z / maxSliderValue.z;
    }
}


void Assignment3::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event

    if (mouseY < window->GetResolution().y - 110 && mouseY > window->GetResolution().y - 150)
    {
        click.x = true;
    }
    if (mouseY < window->GetResolution().y - 60 && mouseY > window->GetResolution().y - 100)
    {
        click.y = true;
    }
    if (mouseY < window->GetResolution().y - 10 && mouseY > window->GetResolution().y - 50)
    {
        click.z = true;
    }
}


void Assignment3::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event

    click = glm::bvec3(false);
}


void Assignment3::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Assignment3::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
