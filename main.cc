#include "Tomos/core/Application.hh"
#include "Tomos/systems/camera/CameraComponent.hh"
#include "Tomos/systems/camera/CameraSystem.hh"
#include "Tomos/systems/light/LightSystem.hh"
#include "Tomos/systems/mesh/MeshSystem.hh"
#include "Tomos/systems/script/ScriptComponent.hh"
#include "Tomos/systems/script/ScriptSystem.hh"
#include "Tomos/util/gltfLoader/GLBLoader.hh"
#include "Tomos/util/imgui/ImGuiLayer.hh"
#include "Tomos/util/renderer/Shader.hh"
#include "Tomos/util/renderer/VertexArray.hh"
#include "Tomos/util/renderer/passes/light/LightPass.hh"
#include "Tomos/util/renderer/passes/mesh/GBufferPass.hh"
#include "Tomos/util/resourceManager/ResourceManager.hh"

using namespace Tomos;

class CameraMoveScript : public Script
{
public:
    CameraMoveScript() {
        // Set raw input and disable cursor for a typical FPS camera experience
        Application::get()->getWindow().setRawInput( true );
        Application::get()->getWindow().setCursorMode( Window::CursorMode::Disabled );
    }

    void update() override
    {
        // Initialize pitch and yaw from the node's initial rotation on the first update call.
        // This ensures the script starts controlling the camera from its initial orientation
        // set in MainScene, rather than resetting it to (0,0,0) Euler angles.
        if (!m_initialized && m_node) {
            glm::vec3 initialEuler = glm::eulerAngles(m_node->m_transform.m_rotation);
            m_pitch = initialEuler.x; // Pitch is rotation around X-axis
            m_yaw = initialEuler.y;   // Yaw is rotation around Y-axis
            m_initialized = true;
        }

        const float moveSpeed        = 5.0f * Application::getState().time().deltaTime();  // Movement speed in meters per second
        const float mouseSensitivity = 0.001f;                                           // Sensitivity for mouse look

        // Reset movement vector for the current frame
        m_movement = glm::vec3( 0.0f );

        // Handle WASD for horizontal movement and Space/Shift for vertical movement
        // W: Move forward (along camera's negative Z-axis)
        if ( Application::getState().input().isKeyDown( GLFW_KEY_W ) ) m_movement.z += moveSpeed;
        // S: Move backward (along camera's positive Z-axis)
        if ( Application::getState().input().isKeyDown( GLFW_KEY_S ) ) m_movement.z -= moveSpeed;
        // A: Move left (along camera's negative X-axis)
        if ( Application::getState().input().isKeyDown( GLFW_KEY_A ) ) m_movement.x -= moveSpeed;
        // D: Move right (along camera's positive X-axis)
        if ( Application::getState().input().isKeyDown( GLFW_KEY_D ) ) m_movement.x += moveSpeed;
        // Space: Move up (along camera's positive Y-axis)
        if ( Application::getState().input().isKeyDown( GLFW_KEY_SPACE ) ) m_movement.y += moveSpeed;
        // Left Shift: Move down (along camera's negative Y-axis)
        if ( Application::getState().input().isKeyDown( GLFW_KEY_LEFT_SHIFT ) ) m_movement.y -= moveSpeed;

        // Handle mouse look
        auto delta = Application::getState().input().getMouseDelta();

        // Accumulate yaw (horizontal rotation)
        // Positive delta.first (mouse moves right) should increase yaw (rotate camera right)
        m_yaw   += -delta.first * mouseSensitivity;
        // Accumulate pitch (vertical rotation)
        // Positive delta.second (mouse moves down) should increase pitch (look camera down)
        m_pitch += -delta.second * mouseSensitivity;

        // Clamp pitch to prevent the camera from flipping upside down
        // Clamping to slightly less than +/- 90 degrees (PI/2 radians) is good practice
        m_pitch = glm::clamp( m_pitch, -glm::half_pi<float>() + 0.01f, glm::half_pi<float>() - 0.01f );

        // Reconstruct the camera's rotation quaternion from the accumulated pitch and yaw.
        // This assumes a YXZ Euler order (yaw around Y, then pitch around X, then roll around Z).
        // The roll component is kept at 0.0f.
        m_node->m_transform.m_rotation = glm::quat( glm::vec3( m_pitch, m_yaw, 0.0f ) );

        // Calculate the camera's local forward, right, and up vectors based on its current rotation
        glm::vec3 forward = m_node->m_transform.m_rotation * glm::vec3( 0.0f, 0.0f, -1.0f ); // Standard OpenGL/glm forward is negative Z
        glm::vec3 right   = m_node->m_transform.m_rotation * glm::vec3( 1.0f, 0.0f, 0.0f );   // Standard OpenGL/glm right is positive X
        glm::vec3 up      = m_node->m_transform.m_rotation * glm::vec3( 0.0f, 1.0f, 0.0f );   // Standard OpenGL/glm up is positive Y

        // Apply movement to the camera's translation based on its local axes
        m_node->m_transform.m_translation += forward * m_movement.z;
        m_node->m_transform.m_translation += right * m_movement.x;
        m_node->m_transform.m_translation += up * m_movement.y;

        // Update the node's transform to apply the changes
        m_node->m_transform.update();
    }

private:
    glm::vec3 m_movement{ 0.0f, 0.0f, 0.0f }; // Stores the movement vector for the current frame
    float     m_yaw   = 0.0f;                 // Accumulated yaw angle (rotation around Y-axis)
    float     m_pitch = 0.0f;                 // Accumulated pitch angle (rotation around X-axis)
    bool      m_initialized = false;          // Flag to ensure initial angles are read only once
};

class MainScene : public Scene
{
public:
    MainScene( const std::string& p_layerId ) : Scene( p_layerId, "Main" )
    {
        // Camera setup
        m_cameraComponent->m_active             = true;
        m_cameraNode->m_transform.m_translation = { 5.0f, 5.0f, 1.0f };  // Start 10m back and 2m up
        m_cameraNode->m_transform.m_rotation    = glm::quat( glm::vec3( -0.1f, 0.7f, 0.0f ) );  // Look slightly down
        m_cameraNode->m_transform.update();
        m_cameraNode->addComponent( m_cameraComponent );
        m_cameraNode->addComponent( m_scriptComponent );
        getRoot().addChild( m_cameraNode );

        getRoot().addChild( loadResult.m_rootNode );

        auto ln                       = std::make_shared<Node>( "LightNode" );
        ln->m_transform.m_translation = { 0.0f, 3.0f, 0.0f };  // Position the light above the scene
        ln->m_transform.update();
        ln->addComponent( std::make_shared<LightComponent>( std::make_shared<PointLightInfo>( glm::vec3( 1.0f, 1.0f, 1.0f ),  // Color
                                                                                             200.0f, true,
                                                                                             1500.0f ) ) );
        getRoot().addChild( ln );

        auto instance1                       = GLBLoader::createInstance( loadResult.m_rootNode );
        instance1->m_transform.m_translation = { 50.0f, 0.0f, 0.0f };
        instance1->m_transform.update();

        auto instance2                       = GLBLoader::createInstance( loadResult.m_rootNode );
        instance2->m_transform.m_translation = { -50.0f, 0.0f, 0.0f };
        instance2->m_transform.update();

        getRoot().addChild( instance1 );
        getRoot().addChild( instance2 );
    }

    void update() override { Scene::update(); }

private:
    std::shared_ptr<Shader> m_shader =
            std::make_shared<Shader>( ResourceManager::getShaderPath( "geometry_vertex.glsl" ), ResourceManager::getShaderPath( "geometry_fragment.glsl" ) );
    GLBLoader::LoadResult loadResult = GLBLoader::loadGLB( ResourceManager::GetModelPath( "SponzaBS.glb" ), m_shader, true );


    std::shared_ptr<Node>            m_cameraNode      = std::make_shared<Node>( "Camera" );
    std::shared_ptr<CameraComponent> m_cameraComponent = std::make_shared<CameraComponent>( 45.0f, 0.1f, 10000.0f, "MainCamera" );
    std::shared_ptr<ScriptComponent> m_scriptComponent = std::make_shared<ScriptComponent>( std::make_shared<CameraMoveScript>(), "CameraMoveScript" );
};

int main()
{
    Application::init( WindowProps( "Demo App", 1280, 720, false, 16.0 / 9.0 ) );

    Application::getState().config().setSchema<BaseConfig>();

    Application::getState().ecs().registerSystem<CameraSystem>();
    Application::getState().ecs().registerSystem<MeshSystem>();
    Application::getState().ecs().registerSystem<LightSystem>();
    Application::getState().ecs().registerSystem<ScriptSystem>();

    auto layer = std::make_shared<Layer>( "main" );

    float quadVertices[] = { // positions, texcoords
                             -1.0f, 1.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f,

                             -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  -1.0f, 1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 1.0f };

    auto vbo = std::make_shared<VertexBuffer>( quadVertices, sizeof( quadVertices ) );
    vbo->setLayout( { { ShaderDataType::Float2, "aPosition" }, { ShaderDataType::Float2, "aTexCoord" } } );

    auto layerQuad = std::make_shared<VertexArray>();
    layerQuad->addVertexBuffer( vbo );
    layer->setQuad( layerQuad );

    std::shared_ptr<GBufferPass> gbuf = std::make_shared<GBufferPass>( 1280, 720, layer->getLayerId() );
    layer->addRenderPass( gbuf );
    layer->addRenderPass( std::make_shared<LightPass>( gbuf->getFrameBuffer(), layer ) );

    layer->getSceneManager() << std::make_shared<MainScene>( layer->getLayerId() );
    Application::get()->pushLayer( layer.get() );

    Application::get()->run();

    return 0;
}
