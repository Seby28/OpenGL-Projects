#include "lab/assignment/assignment.h"

#include <iostream>
#include <string>
#include <vector>


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


lab::Assignment::Assignment()
{
    isCoop = false;

    player1Position  = glm::vec3(0, 0, 2);
    player1Direction = glm::vec3(0, 0, 0);

    player1TargetAngle = glm::radians(180.0f);
    player1JointAngle  = 0.0f;

    player1IsMoving    = false;
    player1MovingState = true;
    player1IsShooting  = false;

    player1Health = 400.0f;

    player2Position  = player1Position + glm::vec3(4, 0, 0);
    player2Direction = glm::vec3(0, 0, 0);

    player2TargetAngle = glm::radians(180.0f);
    player2JointAngle  = 0.0f;

    player2IsMoving    = false;
    player2MovingState = true;
    player2IsShooting  = false;

    player2Health = 400.0f;

    totalExp = 0.0f;
    maxSliderValue = 400.0f;

    enemyJointAngle = glm::radians(180.0f);
    enemyMovingState = false;

    spotLightAngle = 0.0f;

    fov = 60.0f;

    healthBarColor = glm::vec3(1, 1, 0);
    frequency = 4.0f;
    graphSpeed = 1.0f;

    camera = new gfxc::Camera();
}


lab::Assignment::~Assignment()
{
}


void lab::Assignment::Init()
{
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
        meshes[mesh->GetMeshID()] = mesh;
    }

    LoadShader("Generic", "Generic", "Illumination");
    LoadShader("Curve", "Curve", "Illumination");
    LoadShader("Minimap", "Minimap", "Minimap");
    LoadShader("Vignette", "Minimap", "Vignette");
    LoadShader("Quad", "Tangent", "Parallax");
    LoadShader("Graph", "UI", "Graph");
    LoadShader("UI", "UI", "UI");

    SpawnLampposts();

    spotLightPositions.push_back(player1Position + glm::vec3(0, 2, 0) + player1Direction * 0.5f);
    spotLightPositions.push_back(player2Position + glm::vec3(0, 2, 0) + player2Direction * 0.5f);

    spotLightDirections.push_back(player1Direction);
    spotLightDirections.push_back(player2Direction);

    SpawnEnemies();

    CreateFramebuffer(2048, 2048);

    window->SetVSync(true);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


void lab::Assignment::CreateFramebuffer(int width, int height)
{
    glGenFramebuffers(1, &FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

    glGenTextures(1, &colorTexture);
    glBindTexture(GL_TEXTURE_2D, colorTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, colorTexture, 0);

    std::vector<GLenum> drawTextures;

    drawTextures.push_back(GL_COLOR_ATTACHMENT0);
    glDrawBuffers(drawTextures.size(), &drawTextures[0]);

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void lab::Assignment::LoadShader(const std::string& name, const std::string& VS, const std::string& FS)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, "src/lab", "assignment", "shaders");

    {
        Shader* shader = new Shader(name);
        shader->AddShader(PATH_JOIN(shaderPath, VS + "_VS.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, FS + "_FS.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }
}


void lab::Assignment::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, glm::vec3& playerPosition, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    glUseProgram(shader->program);

    int location;

    location = glGetUniformLocation(shader->program, "Model");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(modelMatrix));

    glm::mat4 viewMatrix = GetSceneCamera()->GetViewMatrix();
    location = glGetUniformLocation(shader->program, "View");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(viewMatrix));

    glm::mat4 projectionMatrix = GetSceneCamera()->GetProjectionMatrix();
    location = glGetUniformLocation(shader->program, "Projection");
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

    location = glGetUniformLocation(shader->program, "uPointLightPosition1");
    glUniform3fv(location, 1, glm::value_ptr(player1Position + glm::vec3(0, 2, 0)));

    location = glGetUniformLocation(shader->program, "uPointLightPosition2");
    glUniform3fv(location, 1, glm::value_ptr(player2Position + glm::vec3(0, 2, 0)));

    location = glGetUniformLocation(shader->program, "uSpotLightPositions");
    glUniform3fv(location, 52, glm::value_ptr(spotLightPositions[0]));

    location = glGetUniformLocation(shader->program, "uSpotLightDirections");
    glUniform3fv(location, 52, glm::value_ptr(spotLightDirections[0]));

    glm::vec3 eyePosition = GetSceneCamera()->m_transform->GetWorldPosition();
    location = glGetUniformLocation(shader->program, "uEyePosition");
    glUniform3fv(location, 1, glm::value_ptr(eyePosition));

    location = glGetUniformLocation(shader->program, "uColor");
    glUniform3fv(location, 1, glm::value_ptr(color));

    location = glGetUniformLocation(shader->program, "uPlayerPosition");
    glUniform3fv(location, 1, glm::value_ptr(playerPosition));

    location = glGetUniformLocation(shader->program, "uCurvatureFactor");
    glUniform1f(location, 0.005f);

    location = glGetUniformLocation(shader->program, "uResolution");
    glUniform2iv(location, 1, glm::value_ptr(resolution));

    location = glGetUniformLocation(shader->program, "uRave");
    glUniform1i(location, isRave);

    float time = (float)Engine::GetElapsedTime();
    location = glGetUniformLocation(shader->program, "uTime");
    glUniform1f(location, time);

    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


void lab::Assignment::RenderCharacter(const std::string& shaderName, glm::vec3 currentPosition, float currentAngle, float jointAngle, glm::vec3& playerPosition, const glm::vec3& color)
{
    {   // Left leg
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 0.5f, 0));
        model = glm::translate(model, currentPosition);
        model = glm::rotate(model, currentAngle, glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(0.25f, 0, 0));
        model = glm::translate(model, glm::vec3(0, 0.25f, 0));
        model = glm::rotate(model, jointAngle, glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -0.25f, 0));
        model = glm::scale(model, glm::vec3(0.5f, 1, 0.5f));
        RenderSimpleMesh(meshes["cube"], shaders[shaderName], model, playerPosition, color);
    }

    {   // Right leg
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 0.5f, 0));
        model = glm::translate(model, currentPosition);
        model = glm::rotate(model, currentAngle, glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(-0.25f, 0, 0));
        model = glm::translate(model, glm::vec3(0, 0.25f, 0));
        model = glm::rotate(model, -jointAngle, glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -0.25f, 0));
        model = glm::scale(model, glm::vec3(0.5f, 1, 0.5f));
        RenderSimpleMesh(meshes["cube"], shaders[shaderName], model, playerPosition, color);
    }

    {   // Body
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 1.55f, 0));
        model = glm::translate(model, currentPosition);
        model = glm::rotate(model, currentAngle, glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(1, 1, 0.5f));
        RenderSimpleMesh(meshes["cube"], shaders[shaderName], model, playerPosition, color);
    }

    {   // Left arm
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 1.55f, 0));
        model = glm::translate(model, currentPosition);
        model = glm::rotate(model, currentAngle, glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(0.8f, 0, 0));
        model = glm::translate(model, glm::vec3(0, 0.25f, 0));
        model = glm::rotate(model, -jointAngle, glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -0.25f, 0));
        model = glm::scale(model, glm::vec3(0.5f, 1, 0.5f));
        RenderSimpleMesh(meshes["cube"], shaders[shaderName], model, playerPosition, color);
    }

    {   // Right arm
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 1.55f, 0));
        model = glm::translate(model, currentPosition);
        model = glm::rotate(model, currentAngle, glm::vec3(0, 1, 0));
        model = glm::translate(model, glm::vec3(-0.8f, 0, 0));
        model = glm::translate(model, glm::vec3(0, 0.25f, 0));
        model = glm::rotate(model, jointAngle, glm::vec3(1, 0, 0));
        model = glm::translate(model, glm::vec3(0, -0.25f, 0));
        model = glm::scale(model, glm::vec3(0.5f, 1, 0.5f));
        RenderSimpleMesh(meshes["cube"], shaders[shaderName], model, playerPosition, color);
    }

    {   // Head
        glm::mat4 model = glm::mat4(1);
        model = glm::translate(model, glm::vec3(0, 2.35f, 0));
        model = glm::translate(model, currentPosition);
        model = glm::rotate(model, currentAngle, glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(0.5f));
        RenderSimpleMesh(meshes["cube"], shaders[shaderName], model, playerPosition, color);
    }
}


void lab::Assignment::SpawnLampposts()
{
    for (int i = 0; i < 25; i++)
    {
        glm::vec3 offset = glm::vec3((i % 5) * 20, 0, (i / 5) * 20);
        lampposts.push_back(glm::vec3(-40, 0, -40) + offset);

        spotLightPositions.push_back(lampposts[i] + glm::vec3(0, 4, 0));
        spotLightPositions.push_back(lampposts[i] + glm::vec3(0, 4, 0));

        spotLightDirections.push_back(glm::vec3(0, -1, 0));
        spotLightDirections.push_back(glm::vec3(0, -1, 0));
    }
}


void lab::Assignment::RenderEnvironment(glm::vec3 playerPosition)
{
    for (int i = 0; i < lampposts.size(); i++)
    {
        {
            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0, 4, 0));
            model = glm::translate(model, lampposts[i]);
            model = glm::scale(model, glm::vec3(0.25f, 8, 0.25f));
            RenderSimpleMesh(meshes["cube"], shaders["Curve"], model, playerPosition, (isRave) ? glm::vec3(0.5f, 0.5f, 0) : glm::vec3(0.5f, 0.5f, 1));
        }

        {
            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0, 8, 0));
            model = glm::translate(model, lampposts[i]);
            model = glm::scale(model, glm::vec3(0.25f, 0.25f, 4));
            RenderSimpleMesh(meshes["cube"], shaders["Curve"], model, playerPosition, (isRave) ? glm::vec3(0.5f, 0.5f, 0) : glm::vec3(0.5f, 0.5f, 1));
        }
    }

    {
        glm::mat4 model = glm::mat4(1);
        model = glm::scale(model, glm::vec3(4, 1, 4));
        RenderSimpleMesh(meshes["plane"], shaders["Curve"], model, playerPosition, (isRave) ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 1));
    }
}


void lab::Assignment::RenderSpotLights()
{
    for (int i = 0, j = 0; i < lampposts.size(); i++, j += 2)
    {
        spotLightPositions[j].x = lampposts[i].x + 4.0f * sin((spotLightAngle));
        spotLightPositions[j].z = lampposts[i].z + 4.0f * cos((spotLightAngle));

        spotLightPositions[j + 1].x = lampposts[i].x - 4.0f * sin((spotLightAngle));
        spotLightPositions[j + 1].z = lampposts[i].z - 4.0f * cos((spotLightAngle));
    }
}


int lab::Assignment::GetRandomNumber(int min, int max)
{
    int range = max - min + 1;
    int number = (rand() % 2 == 0) ? min + rand() % range : -max + rand() % range;

    return number;
}


void lab::Assignment::SpawnEnemies()
{
    for (int i = 0; i < 20; i++)
    {
        glm::vec3 position = glm::vec3(0);
        position.x = (float)GetRandomNumber(player1Position.x + 5, player1Position.x + 40);
        position.z = (float)GetRandomNumber(player1Position.z + 5, player1Position.z + 40);

        glm::vec3 color = glm::vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f);

        enemies.push_back(std::make_pair(position, color));
    }
}


void lab::Assignment::RenderEnemies(glm::vec3 playerPosition, float deltaTimeSeconds)
{
    float speed = 2.0f;

    for (auto& enemy : enemies)
    {
        glm::vec3 direction = glm::vec3(0);

        if (!isCoop)
        {
            direction = player1Position - enemy.first;
        }
        else
        {
            if (glm::length(player1Position - enemy.first) < glm::length(player2Position - enemy.first))
            {
                direction = player1Position - enemy.first;
            }
            else
            {
                direction = player2Position - enemy.first;
            }
        }
        direction = glm::normalize(direction);

        float angle = atan2(direction.x, direction.z);
        enemy.first += direction * speed * deltaTimeSeconds;
        RenderCharacter("Curve", enemy.first, angle, enemyJointAngle, playerPosition, enemy.second);
    }
}


void lab::Assignment::ShootEnemy(glm::vec3 playerPosition, glm::vec3 playerDirection, bool playerIsShooting, float& playerHealth, glm::vec3& enemyPosition, glm::vec3& enemyColor)
{
    glm::vec3 center = playerPosition + 2.0f * playerDirection;
    if (playerIsShooting && glm::length(glm::vec2(enemyPosition.x, enemyPosition.z) - glm::vec2(center.x, center.z)) <= 2.0f)
    {
        enemyPosition.x = (float)GetRandomNumber(player1Position.x + 5, player1Position.x + 40);
        enemyPosition.z = (float)GetRandomNumber(player1Position.z + 5, player1Position.z + 40);

        enemyColor = glm::vec3(rand() % 256 / 255.0f, rand() % 256 / 255.0f, rand() % 256 / 255.0f);

        totalExp += 10;
        if (totalExp >= maxSliderValue)
        {
            totalExp = maxSliderValue;
        }
    }

    if (glm::length(glm::vec2(enemyPosition.x, enemyPosition.z) - glm::vec2(playerPosition.x, playerPosition.z)) <= 0.05f)
    {
        playerHealth -= 0.25f;
        if (playerHealth <= 0.0f)
        {
            playerHealth = 0.0f;
        }
    }
}


void lab::Assignment::RenderUI(Mesh* mesh, Shader* shader, const glm::vec3& color)
{
    if (!mesh || !shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    int location;

    location = glGetUniformLocation(shader->program, "uColor");
    glUniform3fv(location, 1, glm::value_ptr(color));

    location = glGetUniformLocation(shader->program, "uHealthBarColor");
    glUniform3fv(location, 1, glm::value_ptr(healthBarColor));

    float time = (float)Engine::GetElapsedTime();
    location = glGetUniformLocation(shader->program, "uTime");
    glUniform1f(location, time);

    location = glGetUniformLocation(shader->program, "uFrequency");
    glUniform1f(location, frequency);

    location = glGetUniformLocation(shader->program, "uSpeed");
    glUniform1f(location, graphSpeed);

    location = glGetUniformLocation(shader->program, "uResolution");
    glUniform2iv(location, 1, glm::value_ptr(resolution));

    location = glGetUniformLocation(shader->program, "uRave");
    glUniform1i(location, isRave);

    // Draw the object
    glBindVertexArray(mesh->GetBuffers()->m_VAO);
    glDrawElements(mesh->GetDrawMode(), static_cast<int>(mesh->indices.size()), GL_UNSIGNED_INT, 0);
}


void lab::Assignment::RenderTextureScreen(Shader* shader, unsigned int textureID)
{
    if (!shader || !shader->GetProgramID())
        return;

    // Render an object using the specified shader and the specified position
    glUseProgram(shader->program);

    // Set texture uniform
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(glGetUniformLocation(shader->program, "uTexture"), 0);

    int location = glGetUniformLocation(shader->program, "uRave");
    glUniform1i(location, isRave);

    // Draw the object
    glBindVertexArray(meshes["quad"]->GetBuffers()->m_VAO);
    glDrawElements(meshes["quad"]->GetDrawMode(), static_cast<int>(meshes["quad"]->indices.size()), GL_UNSIGNED_INT, 0);
}


void lab::Assignment::DrawFramebufferTextures()
{
    int screenPosX = window->GetResolution().x - 300;
    int screenPosY = window->GetResolution().y - 300;

    int width  = 200;
    int height = 200;

    if (colorTexture)
    {
        // Render the color texture on the screen
        glViewport(screenPosX, screenPosY, width, height);

        RenderTextureScreen(shaders["Minimap"], colorTexture);
    }
}


void lab::Assignment::DrawFramebufferVignette()
{
    int width  = resolution.x;
    int height = resolution.y;

    if (colorTexture)
    {
        // Render the color texture on the screen
        glViewport(0, 0, width, height);

        RenderTextureScreen(shaders["Minimap"], colorTexture);
    }
}


void lab::Assignment::FrameStart()
{
    (isRave) ? glClearColor(0, 0, 0, 1) : glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    resolution = window->GetResolution();
    // glViewport(0, 0, resolution.x, resolution.y);
    isRave = !isRave;

}


void lab::Assignment::Update(float deltaTimeSeconds)
{

    spotLightAngle += deltaTimeSeconds;

    if (player1IsMoving)
    {
        if (player1MovingState == true)
        {
            player1JointAngle += 4.0f * deltaTimeSeconds;
            if (player1JointAngle >= glm::radians(45.0f))
            {
                player1MovingState = !player1MovingState;
            }
        }
        else
        {
            player1JointAngle -= 4.0f * deltaTimeSeconds;
            if (player1JointAngle <= glm::radians(-45.0f))
            {
                player1MovingState = !player1MovingState;
            }
        }
    }
    else
    {
        player1JointAngle = lerp(player1JointAngle, 0.0f, 0.1f);
        player1MovingState = true;
    }

    {
        spotLightPositions[spotLightPositions.size() - 2] = player1Position;
        spotLightPositions[spotLightPositions.size() - 1] = player2Position;

        spotLightDirections[spotLightDirections.size() - 2] = player1Direction;
        spotLightDirections[spotLightDirections.size() - 1] = player2Direction;
    }

    if (isCoop && player2IsMoving)
    {
        if (player2MovingState == true)
        {
            player2JointAngle += 4.0f * deltaTimeSeconds;
            if (player2JointAngle >= glm::radians(45.0f))
            {
                player2MovingState = !player2MovingState;
            }
        }
        else
        {
            player2JointAngle -= 4.0f * deltaTimeSeconds;
            if (player2JointAngle <= glm::radians(-45.0f))
            {
                player2MovingState = !player2MovingState;
            }
        }
    }
    else
    {
        player2JointAngle = lerp(player2JointAngle, 0.0f, 0.1f);
        player2MovingState = true;
    }

    if (enemyMovingState == true)
    {
        enemyJointAngle += 2.0f * deltaTimeSeconds;
        if (enemyJointAngle >= glm::radians(45.0f))
        {
            enemyMovingState = !enemyMovingState;
        }
    }
    else
    {
        enemyJointAngle -= 2.0f * deltaTimeSeconds;
        if (enemyJointAngle <= glm::radians(-45.0f))
        {
            enemyMovingState = !enemyMovingState;
        }
    }

    if (!isCoop)
    {
        camera = GetSceneCamera();

        {
            // glm::vec3 relativeCameraPosition = glm::vec3(0, 10.5f, 0);
            // camera->SetOrthographic(28.0f, 28.0f, 0.1f, 200.0f);
            // camera->SetPositionAndRotation(player1Position + relativeCameraPosition, glm::quatLookAt(-glm::normalize(relativeCameraPosition), glm::vec3(0, 0, -1)));
        
            glBindFramebuffer(GL_FRAMEBUFFER, FBO);
        
            glClearColor(0.05, 0.05, 0.05, 0.5);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
            glViewport(0, 0, 2048, 2048);

            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
            {
                glm::mat4 model = glm::mat4(1);
                model = glm::scale(model, glm::vec3(1, 1, 1));
                RenderSimpleMesh(meshes["plane"], shaders["Color"], model, player1Position, glm::vec3(0, 1, 0));
            }

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            {
                glm::mat4 model = glm::mat4(1);
                model = glm::translate(model, player1Position);
                RenderSimpleMesh(meshes["cube"], shaders["Color"], model, player1Position, glm::vec3(1, 0, 0));
            }

            for (auto& enemy : enemies)
            {
                glm::mat4 model = glm::mat4(1);
                model = glm::translate(model, enemy.first);
                RenderSimpleMesh(meshes["cube"], shaders["Color"], model, player1Position, enemy.second);
            }
        }

        {
            glm::vec3 relativeCameraPosition = glm::vec3(0, 10.5f, 12.5f);
            camera->SetPerspective(fov, 16.0f / 9.0f, 0.01f, 200.0f);
            // camera->SetPositionAndRotation(player1Position + relativeCameraPosition, glm::quatLookAt(-glm::normalize(relativeCameraPosition), glm::vec3(0, 1, 0)));
            camera = GetSceneCamera();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        glViewport(0, 0, resolution.x, resolution.y);

        // RenderCharacter("Generic", player1Position, player1TargetAngle, player1JointAngle, player1Position, glm::vec3(1, 0, 0));

        {
            glm::mat4 model = glm::mat4(1);
            model = glm::translate(model, glm::vec3(0, 4, 4));
            // model = glm::rotate(model, (spotLightAngle), glm::vec3(1, 0, 0));
            // model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1, 0, 0));
            model = glm::scale(model, glm::vec3(4));
            RenderSimpleMesh(meshes["quad"], shaders["Quad"], model, player1Position, glm::vec3(1));
        }

        // if (totalExp < maxSliderValue)
        // {
        //     RenderEnemies(player1Position, deltaTimeSeconds);
        // }

        for (auto& enemy : enemies)
        {
            ShootEnemy(player1Position, player1Direction, player1IsShooting, player1Health, enemy.first, enemy.second);
        }

        RenderEnvironment(player1Position);
        RenderSpotLights();

        if (player1Health >= 200.0f && player1Health <= 400.0f)
        {
            healthBarColor = glm::vec3(0, 1, 0);
            frequency = 4.0f;
            graphSpeed = 1.0f;
        }
        else if (player1Health >= 80.0f && player1Health < 200.0f)
        {
            healthBarColor = glm::vec3(1, 0.5f, 0);
            frequency = 8.0f;
            graphSpeed = 1.2f;
        }
        else if (player1Health >= 0.0f && player1Health < 80.0f)
        {
            healthBarColor = glm::vec3(1, 0, 0);
            frequency = 12.0f;
            graphSpeed = 1.8f;
        }

        glViewport(10, 80, (int)player1Health, 20);
        RenderUI(meshes["quad"], shaders["UI"], (isRave) ? healthBarColor : glm::vec3(1.0f) - healthBarColor);
        glViewport(10, 80, (int)maxSliderValue, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0.25f));

        glViewport(10, 40, totalExp, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0, 1, 1));
        glViewport(10, 40, (int)maxSliderValue, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0.25f));

        DrawFramebufferTextures();

        // glViewport(window->GetResolution().x - 305, window->GetResolution().y - 305, 210, 210);
        // RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0, 1, 0));

        glViewport(window->GetResolution().x - 305, window->GetResolution().y - 365, 210, 50);
        RenderUI(meshes["quad"], shaders["Graph"], glm::vec3(0, 1, 0));
    }
    else
    {
        auto camera = GetSceneCamera();

        {
            glm::vec3 relativeCameraPosition = glm::vec3(0, 10.5f, 12.5f);
            camera->SetPerspective(fov, 8.0f / 9.0f, 0.01f, 200.0f);
            camera->SetPositionAndRotation(player1Position + relativeCameraPosition, glm::quatLookAt(-glm::normalize(relativeCameraPosition), glm::vec3(0, 1, 0)));
        }

        glViewport(0, 0, resolution.x * 0.5f, resolution.y);

        RenderCharacter("Generic", player1Position, player1TargetAngle, player1JointAngle, player1Position, glm::vec3(1, 0, 0));
        RenderCharacter("Curve", player2Position, player2TargetAngle, player2JointAngle, player1Position, glm::vec3(1, 1, 0));

        if (totalExp < maxSliderValue)
        {
            RenderEnemies(player1Position, deltaTimeSeconds);
        }

        for (auto& enemy : enemies)
        {
            ShootEnemy(player1Position, player1Direction, player1IsShooting, player1Health, enemy.first, enemy.second);
            ShootEnemy(player2Position, player2Direction, player2IsShooting, player2Health, enemy.first, enemy.second);
        }

        RenderEnvironment(player1Position);
        RenderSpotLights();

        glViewport(10, 80, (int)player1Health, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(1, 0, 0));
        glViewport(10, 80, (int)maxSliderValue, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0.25f));

        glViewport(10, 40, totalExp, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0, 1, 1));
        glViewport(10, 40, (int)maxSliderValue, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0.25f));

        {
            glm::vec3 relativeCameraPosition = glm::vec3(0, 10.5f, 12.5f);
            camera->SetPerspective(fov, 8.0f / 9.0f, 0.01f, 200.0f);
            camera->SetPositionAndRotation(player2Position + relativeCameraPosition, glm::quatLookAt(-glm::normalize(relativeCameraPosition), glm::vec3(0, 1, 0)));
        }

        glViewport(resolution.x * 0.5f, 0, resolution.x * 0.5f, resolution.y);

        RenderCharacter("Curve", player1Position, player1TargetAngle, player1JointAngle, player2Position, glm::vec3(1, 0, 0));
        RenderCharacter("Generic", player2Position, player2TargetAngle, player2JointAngle, player2Position, glm::vec3(1, 1, 0));

        if (totalExp < maxSliderValue)
        {
            RenderEnemies(player2Position, deltaTimeSeconds);
        }

        RenderEnvironment(player2Position);
        RenderSpotLights();

        glViewport(resolution.x * 0.5f + 10, 80, (int)player2Health, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(1, 1, 0));
        glViewport(resolution.x * 0.5f + 10, 80, (int)maxSliderValue, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0.25f));

        glViewport(resolution.x * 0.5f + 10, 40, totalExp, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0, 1, 1));
        glViewport(resolution.x * 0.5f + 10, 40, (int)maxSliderValue, 20);
        RenderUI(meshes["quad"], shaders["UI"], glm::vec3(0.25f));
    }
}


void lab::Assignment::FrameEnd()
{
    // DrawCoordinateSystem();
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */


void lab::Assignment::OnInputUpdate(float deltaTime, int mods)
{
    glm::vec3 right = glm::vec3(1, 0, 0);
    glm::vec3 forward = glm::vec3(0, 0, 1);

    if (!window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        const float speed = 6.0f;
    
        if (window->KeyHold(GLFW_KEY_W))
        {
            player1IsMoving = true;
            player1Direction.z -= 1.0f;
        }
        if (window->KeyHold(GLFW_KEY_S))
        {
            player1IsMoving = true;
            player1Direction.z += 1.0f;
        }
        if (window->KeyHold(GLFW_KEY_A))
        {
            player1IsMoving = true;
            player1Direction.x -= 1.0f;
        }
        if (window->KeyHold(GLFW_KEY_D))
        {
            player1IsMoving = true;
            player1Direction.x += 1.0f;
        }
    
        if (player1Direction != glm::vec3(0))
        {
            player1Direction = glm::normalize(player1Direction);
            player1Position += player1Direction * speed * deltaTime;
            player1TargetAngle = atan2(player1Direction.x, player1Direction.z);
        }
    }

    if (isCoop && !window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT))
    {
        const float speed = 6.0f;

        if (window->KeyHold(GLFW_KEY_UP))
        {
            player2IsMoving = true;
            player2Direction.z -= 1.0f;
        }
        if (window->KeyHold(GLFW_KEY_DOWN))
        {
            player2IsMoving = true;
            player2Direction.z += 1.0f;
        }
        if (window->KeyHold(GLFW_KEY_LEFT))
        {
            player2IsMoving = true;
            player2Direction.x -= 1.0f;
        }
        if (window->KeyHold(GLFW_KEY_RIGHT))
        {
            player2IsMoving = true;
            player2Direction.x += 1.0f;
        }

        if (player2Direction != glm::vec3(0))
        {
            player2Direction = glm::normalize(player2Direction);
            player2Position += player2Direction * speed * deltaTime;
            player2TargetAngle = atan2(player2Direction.x, player2Direction.z);
        }
    }
}


void lab::Assignment::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_SPACE)
    {
        player1IsShooting = true;
    }

    if (key == GLFW_KEY_KP_ENTER)
    {
        player2IsShooting = true;
    }

    if (key == GLFW_KEY_KP_0)
    {
        isCoop = !isCoop;
    }
}


void lab::Assignment::OnKeyRelease(int key, int mods)
{
    if (key == GLFW_KEY_SPACE)
    {
        player1IsShooting = false;
    }

    if (key == GLFW_KEY_KP_ENTER)
    {
        player2IsShooting = false;
    }

    if (key == GLFW_KEY_W || key == GLFW_KEY_S || key == GLFW_KEY_A || key == GLFW_KEY_D)
    {
        player1IsMoving = false;
        player1Direction = glm::vec3(0);
    }

    if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN || key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
    {
        player2IsMoving = false;
        player2Direction = glm::vec3(0);
    }
}


void lab::Assignment::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
    // Add mouse move event
}


void lab::Assignment::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button press event
}


void lab::Assignment::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
    // Add mouse button release event
}


void lab::Assignment::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
    // Treat mouse scroll event
}


void lab::Assignment::OnWindowResize(int width, int height)
{
    // Treat window resize event
}
