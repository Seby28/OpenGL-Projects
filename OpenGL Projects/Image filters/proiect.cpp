#include "lab_m2/proiect/proiect.h"

#include <iostream>
#include <vector>

#include "pfd/portable-file-dialogs.h"

using namespace m2;
using namespace std;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Proiect::Proiect()
{
    outputMode = 0;
    gpuProcessing = true;
    saveScreenToImage = false;
    filterType = false;
    window->SetSize(600, 600);

    brightness = 0.0f;
    contrast = 1.0f;

    shadeTint = false;
    tint = 0.0f;
    shade = 0.0f;
}


Proiect::~Proiect()
{
}


void Proiect::Init()
{
    // Load default texture fore imagine processing
    originalImage  = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "pos_x.png"), nullptr, "image", true, true);
    processedImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube", "pos_x.png"), nullptr, "newImage", true, true);

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "lab7", "shaders");

    // Create a shader program for particle system
    {
        Shader* shader = new Shader("ImageProcessing");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void Proiect::FrameStart()
{
}


void Proiect::Update(float deltaTimeSeconds)
{
    ClearScreen();

    auto shader = shaders["ImageProcessing"];
    shader->Use();

    if (saveScreenToImage)
    {
        window->SetSize(originalImage->GetWidth(), originalImage->GetHeight());
    }

    int texture_loc = shader->GetUniformLocation("textureImage");
    glUniform1i(texture_loc, 0);

    auto textureImage = (gpuProcessing == true) ? originalImage : processedImage;
    textureImage->BindToTextureUnit(GL_TEXTURE0);

    int flip_loc = shader->GetUniformLocation("flipVertical");
    glUniform1i(flip_loc, saveScreenToImage ? 0 : 1);

    int screenSize_loc = shader->GetUniformLocation("screenSize");
    glm::ivec2 resolution = window->GetResolution();
    glUniform2i(screenSize_loc, resolution.x, resolution.y);

    int outputMode_loc = shader->GetUniformLocation("outputMode");
    glUniform1i(outputMode_loc, outputMode);

    int filterType_loc = shader->GetUniformLocation("filterType");
    glUniform1i(filterType_loc, filterType);

    glUniform1f(glGetUniformLocation(shader->program, "contrastFactor"), contrast);
    glUniform1f(glGetUniformLocation(shader->program, "brightnessValue"), brightness);

    glUniform1i(glGetUniformLocation(shader->program, "shadeTint"), shadeTint);
    glUniform1f(glGetUniformLocation(shader->program, "tint"), tint);
    glUniform1f(glGetUniformLocation(shader->program, "shade"), shade);

    RenderMesh(meshes["quad"], shader, glm::mat4(1));

    if (saveScreenToImage)
    {
        saveScreenToImage = false;

        GLenum format = GL_RGB;
        if (originalImage->GetNrChannels() == 4)
        {
            format = GL_RGBA;
        }

        glReadPixels(0, 0, originalImage->GetWidth(), originalImage->GetHeight(), format, GL_UNSIGNED_BYTE, processedImage->GetImageData());
        processedImage->UploadNewData(processedImage->GetImageData());
        SaveImage("shader_processing_" + std::to_string(outputMode));

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }

    {
        if (brightness >= 0.75f)
        {
            brightness = 0.75f;
        }
        if (brightness <= -0.75f)
        {
            brightness = -0.75f;
        }
    }
    {
        if (contrast >= 2.25f)
        {
            contrast = 2.25f;
        }
        if (contrast <= 0.25f)
        {
            contrast = 0.25f;
        }
    }
    {
        if (shade >= 1.0f)
        {
            shade = 1.0f;
        }
        if (shade <= 0.0f)
        {
            shade = 0.0f;
        }
    }
    {
        if (tint >= 1.0f)
        {
            tint = 1.0f;
        }
        if (tint <= 0.0f)
        {
            tint = 0.0f;
        }
    }
}


void Proiect::FrameEnd()
{
    DrawCoordinateSystem();
}


void Proiect::OnFileSelected(const std::string& fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << endl;
        originalImage  = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        processedImage = TextureManager::LoadTexture(fileName, nullptr, "newImage", true, true);

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}


void Proiect::GrayScale()
{
    unsigned int channels  = originalImage->GetNrChannels();
    unsigned char* data    = originalImage->GetImageData();
    unsigned char* newData = processedImage->GetImageData();

    if (channels < 3)
        return;

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

    for (int i = 0; i < imageSize.y; i++)
    {
        for (int j = 0; j < imageSize.x; j++)
        {
            int offset = channels * (i * imageSize.x + j);

            // Reset save image data
            char value = static_cast<char>(data[offset + 0] * 0.2f + data[offset + 1] * 0.71f + data[offset + 2] * 0.07f);
            memset(&newData[offset], value, 3);
        }
    }

    processedImage->UploadNewData(newData);
}


void Proiect::SaveImage(const std::string& fileName)
{
    cout << "Saving image! ";
    processedImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}


void Proiect::OpenDialog()
{
    std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void Proiect::OnInputUpdate(float deltaTime, int mods)
{
    // Treat continuous update based on input
    if (window->KeyHold(GLFW_KEY_P) == true && outputMode == 3)
    {
        brightness += deltaTime * 0.25f;
    }
    else if (window->KeyHold(GLFW_KEY_O) == true && outputMode == 3)
    {
        brightness -= deltaTime * 0.25f;
    }

    if (window->KeyHold(GLFW_KEY_P) == true && outputMode == 4)
    {
        contrast += deltaTime * 0.25f;
    }
    else if (window->KeyHold(GLFW_KEY_O) == true && outputMode == 4)
    {
        contrast -= deltaTime * 0.25f;
    }

    if (window->KeyHold(GLFW_KEY_P) == true && outputMode == 5)
    {
        shade += deltaTime * 0.25f;
    }
    else if (window->KeyHold(GLFW_KEY_O) == true && outputMode == 5)
    {
        shade -= deltaTime * 0.25f;
    }

    if (window->KeyHold(GLFW_KEY_P) == true && outputMode == 5)
    {
        tint += deltaTime * 0.25f;
    }
    else if (window->KeyHold(GLFW_KEY_O) == true && outputMode == 5)
    {
        tint -= deltaTime * 0.25f;
    }
}


void Proiect::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_E)
    {
        gpuProcessing = !gpuProcessing;
        if (gpuProcessing == false)
        {
            outputMode = 0;
        }
        cout << "Processing on GPU: " << (gpuProcessing ? "true" : "false") << endl;
    }

    if (key == GLFW_KEY_X)
    {
        filterType = !filterType;
        cout << "Filter type: " << (!filterType ? "Color filter" : "Spatial filter") << endl;
    }

    if (key == GLFW_KEY_M)
    {
        shadeTint = !shadeTint;
        shade = 0.0f;
        tint = 0.0f;
    }

    if (key == GLFW_KEY_LEFT_BRACKET)
    {
        shade = 0.0f;
        tint = 0.0f;

        brightness = 0.0f;
        contrast = 1.0f;
    }

    if (key - GLFW_KEY_0 >= 0 && key <= GLFW_KEY_5)
    {
        outputMode = key - GLFW_KEY_0;

        if (gpuProcessing == false)
        {
            outputMode = 0;
            GrayScale();
        }
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        if (!gpuProcessing)
        {
            SaveImage("processCPU_" + std::to_string(outputMode));
        }
        else
        {
            saveScreenToImage = true;
        }
    }
}


void Proiect::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Proiect::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void Proiect::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void Proiect::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void Proiect::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void Proiect::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
