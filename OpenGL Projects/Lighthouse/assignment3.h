#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"


namespace m1
{
    class Assignment3 : public gfxc::SimpleScene
    {
    public:
        Assignment3();
        ~Assignment3();

        void Init() override;

    private:
        void LoadShader(const std::string& name, const std::string& vertexShader, const std::string& fragmentShader);

        Texture2D* CreateTexture(unsigned int width, unsigned int height, unsigned int channels, unsigned char* data);
        Texture2D* LoadTexture(const char* imagePath);

        void RenderColorMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, const glm::vec3& color = glm::vec3(1));
        void RenderTextureMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, Texture2D* texture = NULL);

        int GetRandomNumber(int min, int max);

        void RenderEnvironment();

        void SpawnBoats();
        void RenderBoat(glm::vec3& position, float& angle, int sign);

        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

    private:
        std::unordered_map<std::string, Texture2D*> textures;

        int boatCount;
        std::vector<glm::vec3> boatPositions;
        std::vector<std::pair<float, float>> boatSpeedAngle;

        glm::vec3 lightDirection;
        std::vector<glm::vec3> pointLightPositions;
        std::vector<glm::vec3> pointLightColors;
        std::vector<glm::vec3> spotLightPositions;
        std::vector<glm::vec3> spotLightDirections;

        glm::vec3 mainColor;

        glm::ivec3 maxSliderValue;
        glm::ivec3 sliderValue;
        glm::bvec3 click;

        gfxc::Camera* camera;

        glm::ivec2 resolution;
    };
}   // namespace lab
