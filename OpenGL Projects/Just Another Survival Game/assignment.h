#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"


namespace lab
{
    class Assignment : public gfxc::SimpleScene
    {
    public:
        Assignment();
        ~Assignment();

        void Init() override;

    private:
        void CreateFramebuffer(int width, int height);

        void LoadShader(const std::string& name, const std::string& VS, const std::string& FS);

        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::vec3& position, const glm::vec3& color = glm::vec3(1));
        void RenderCharacter(const std::string& name, glm::vec3 currentPosition, float currentAngle, float jointAngle, glm::vec3& playerPosition, const glm::vec3& color = glm::vec3(1));

        void SpawnLampposts();
        void RenderEnvironment(glm::vec3 playerPosition);
        void RenderSpotLights();

        int GetRandomNumber(int min, int max);

        void SpawnEnemies();
        void RenderEnemies(glm::vec3 playerPosition, float deltaTimeSeconds);

        void ShootEnemy(glm::vec3 playerPosition, glm::vec3 playerDirection, bool playerIsShooting, float& playerHealth, glm::vec3& enemyPosition, glm::vec3& enemyColor);

        void RenderUI(Mesh* mesh, Shader* shader, const glm::vec3& color = glm::vec3(1));

        void RenderTextureScreen(Shader* shader, unsigned int textureID);
        void DrawFramebufferTextures();
        void DrawFramebufferVignette();

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
        bool isCoop;

        // Player 1
        glm::vec3 player1Position;
        glm::vec3 player1Direction;

        float player1TargetAngle;
        float player1JointAngle;

        bool player1IsMoving;
        bool player1MovingState;
        bool player1IsShooting;

        float player1Health;

        // Player2
        glm::vec3 player2Position;
        glm::vec3 player2Direction;

        float player2TargetAngle;
        float player2JointAngle;

        bool player2MovingState;
        bool player2IsMoving;
        bool player2IsShooting;

        float player2Health;

        float totalExp;
        float maxSliderValue;

        // Lampposts and lights
        std::vector<glm::vec3> lampposts;

        std::vector<glm::vec3> spotLightPositions;
        std::vector<glm::vec3> spotLightDirections;
        float spotLightAngle;

        // Enemies
        std::vector<std::pair<glm::vec3, glm::vec3>> enemies;

        float enemyJointAngle;
        bool enemyMovingState;

        // Misc
        float fov;

        glm::ivec2 resolution;

        unsigned int FBO;
        unsigned int colorTexture;

        glm::vec3 healthBarColor;
        float frequency;
        float graphSpeed;

        float vignetteSmooth = 0.0f;

        gfxc::Camera* camera;

        bool isRave = false;
    };
}   // namespace lab
