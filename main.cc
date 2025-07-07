#include "Tomos/core/Application.hh"
#include "Tomos/systems/camera/CameraComponent.hh"
#include "Tomos/systems/camera/CameraSystem.hh"
#include "Tomos/systems/mesh/MeshSystem.hh"
#include "Tomos/systems/script/ScriptComponent.hh"
#include "Tomos/systems/script/ScriptSystem.hh"
#include "Tomos/util/gltfLoader/GLBLoader.hh"
#include "Tomos/util/renderer/Renderer.hh"
#include "Tomos/util/renderer/Shader.hh"
#include "Tomos/util/renderer/VertexArray.hh"
#include "Tomos/util/resourceManager/ResourceManager.hh"
#include "Tomos/util/imgui/ImGuiLayer.hh"
#include "Tomos/util/renderer/passes/mesh/BasePass.hh"

using namespace Tomos;

class CameraMoveScript : public Script
{
public:
    CameraMoveScript()
    {
        Application::get()->getWindow().setCursorMode( Window::CursorMode::Disabled );
    }

    void update() override
    {
        const float moveSpeed        = 5.0f * Application::getState().time().deltaTime(); // meters per second
        const float mouseSensitivity = 0.001f;

        // Reset movement
        m_movement = glm::vec3( 0.0f );

        // Handle WASD movement
        if ( Application::getState().input().isKeyDown( GLFW_KEY_W ) ) m_movement.z += moveSpeed;
        if ( Application::getState().input().isKeyDown( GLFW_KEY_S ) ) m_movement.z -= moveSpeed;
        if ( Application::getState().input().isKeyDown( GLFW_KEY_A ) ) m_movement.x -= moveSpeed;
        if ( Application::getState().input().isKeyDown( GLFW_KEY_D ) ) m_movement.x += moveSpeed;
        if ( Application::getState().input().isKeyDown( GLFW_KEY_SPACE ) ) m_movement.y += moveSpeed;
        if ( Application::getState().input().isKeyDown( GLFW_KEY_LEFT_SHIFT ) ) m_movement.y -= moveSpeed;

        // Mouse look (always active in FPS mode)
        auto delta = Application::getState().input().getMouseDelta();
        delta.first *= -mouseSensitivity;
        delta.second *= -mouseSensitivity;

        glm::vec3 euler = glm::eulerAngles( m_node->m_transform.m_rotation );
        float     yaw   = euler.y;
        float     pitch = euler.x;

        yaw += delta.first;
        pitch += delta.second;
        pitch = glm::clamp( pitch, -1.5f, 1.5f ); // Limit pitch to avoid over-rotation

        m_node->m_transform.m_rotation = glm::quat( glm::vec3( pitch, yaw, 0.0f ) );

        // Apply movement
        glm::vec3 forward = m_node->m_transform.m_rotation * glm::vec3( 0.0f, 0.0f, -1.0f );
        glm::vec3 right   = m_node->m_transform.m_rotation * glm::vec3( 1.0f, 0.0f, 0.0f );
        glm::vec3 up      = m_node->m_transform.m_rotation * glm::vec3( 0.0f, 1.0f, 0.0f );

        m_node->m_transform.m_translation += forward * m_movement.z + right * m_movement.x + up * m_movement.y;
        m_node->m_transform.update();
    }

private:
    glm::vec3 m_movement{0.0f, 0.0f, 0.0f};
};

class MainScene : public Scene
{
public:
    MainScene( int p_layerId ) :
        Scene( p_layerId, "Main" )
    {
        // Camera setup
        m_cameraComponent->m_active             = true;
        m_cameraNode->m_transform.m_translation = {0.0f, 2.0f, 10.0f}; // Start 10m back and 2m up
        m_cameraNode->m_transform.update();
        m_cameraNode->addComponent( m_cameraComponent );
        m_cameraNode->addComponent( m_scriptComponent );
        getRoot().addChild( m_cameraNode );

        getRoot().addChild( loadResult.m_rootNode );

        auto instance1                       = GLBLoader::createInstance( loadResult.m_rootNode );
        instance1->m_transform.m_translation = {50.0f, 0.0f, 0.0f};
        instance1->m_transform.update();

        auto instance2                       = GLBLoader::createInstance( loadResult.m_rootNode );
        instance2->m_transform.m_translation = {-50.0f, 0.0f, 0.0f};
        instance2->m_transform.update();

        getRoot().addChild( instance1 );
        getRoot().addChild( instance2 );
    }

    void update() override
    {
        Scene::update();
    }

private:
    std::shared_ptr<Shader> m_shader =
            std::make_shared<Shader>( ResourceManager::getShaderPath( "generic_vertex.glsl" ), ResourceManager::getShaderPath( "generic_fragment.glsl" ) );
    GLBLoader::LoadResult loadResult = GLBLoader::loadGLB( ResourceManager::GetModelPath( "SponzaBS.glb" ), m_shader, true );


    std::shared_ptr<Node>            m_cameraNode      = std::make_shared<Node>( "Camera" );
    std::shared_ptr<CameraComponent> m_cameraComponent = std::make_shared<CameraComponent>( 45.0f, 0.1f, 10000.0f, "MainCamera" );
    std::shared_ptr<ScriptComponent> m_scriptComponent = std::make_shared<ScriptComponent>( std::make_shared<CameraMoveScript>(), "CameraMoveScript" );
};

int main()
{
    Application::init( WindowProps( "Demo App", 1280, 720, true, 16.0 / 9.0 ) );

    Application::getState().config().setSchema<BaseConfig>();

    Application::getState().ecs().registerSystem<MeshSystem>();
    Application::getState().ecs().registerSystem<CameraSystem>();
    Application::getState().ecs().registerSystem<ScriptSystem>();

    auto layer = new Layer( "main" );
    layer->addRenderPass( std::make_unique<BasePass>() );

    layer->getSceneManager() << std::make_shared<MainScene>( layer->getLayerId() );
    Application::get()->pushLayer( layer );
    // Application::get()->pushOverlay( new ImGuiLayer() );

    Application::get()->run();

    return 0;
}
