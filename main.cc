#include "Tomos/core/Application.hh"
#include "Tomos/systems/camera/CameraComponent.hh"
#include "Tomos/systems/camera/CameraSystem.hh"
#include "Tomos/util/renderer/Renderer.hh"
#include "Tomos/util/renderer/Shader.hh"
#include "Tomos/util/renderer/VertexArray.hh"

using namespace Tomos;

class MainScene : public Scene
{
public:
    MainScene() :
        Scene( "Main" )
    {
        m_vertex->setLayout( m_l );
        m_vertexA->addVertexBuffer( m_vertex );
        m_vertexA->setIndexBuffer( m_index );

        m_cameraComponent->m_active             = true;
        m_cameraNode->m_transform.m_translation = {0, 0, 5};
        m_cameraNode->m_transform.update();
        m_cameraNode->addComponent( m_cameraComponent );
        getRoot().addChild( m_cameraNode );
    }

    void update() override
    {
        Scene::update();

        Renderer::setClearedColor( {0.6f, 0.2f, 0.6f, 1} );
        Renderer::clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        Renderer::beginScene();

        auto p = Application::get()->getState().m_ecs.getSystem<CameraSystem>().getViewProjectionMat();
        Renderer::draw(
                m_shader,
                m_vertexA,
                glm::rotate( glm::mat4( 1.0f ), glm::radians( 360 * Application::get()->getState().m_time.gameTime() ), glm::vec3( 1.0f, 1.0f, 1.0f ) ),
                p
                );


        Renderer::endScene();
    }

private:
    float m_verticies[3 * 7] = {
            -0.5f, -0.5, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            0.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f
    };

    std::shared_ptr<VertexArray>  m_vertexA = std::make_shared<VertexArray>();
    std::shared_ptr<VertexBuffer> m_vertex  = std::make_shared<VertexBuffer>( m_verticies, sizeof( m_verticies ) );
    BufferLayout                  m_l       = {
            {ShaderDataType::Float3, "aPosition"},
            {ShaderDataType::Float4, "aColor"},
    };

    unsigned int                 m_indicies[3] = {0, 1, 2};
    std::shared_ptr<IndexBuffer> m_index       = std::make_shared<IndexBuffer>( m_indicies, sizeof( m_indicies ) );

    std::shared_ptr<Shader> m_shader = std::make_shared<Shader>( "resource/shaders/generic.vertex.glsl",
                                                                 "resource/shaders/generic.fragment.glsl" );

    std::shared_ptr<Node>            m_cameraNode      = std::make_shared<Node>( "Camera" );
    std::shared_ptr<CameraComponent> m_cameraComponent = std::make_shared<CameraComponent>( 90.0f, 0.01, 1000, "meow" );
};


int main()
{
    Application::get()->getState().m_ecs.registerSystem<CameraSystem>();

    auto layer = new Layer( "main" );

    layer->getSceneManager() << std::make_shared<MainScene>();

    Application::get()->getState().m_layerStack.pushLayer( layer );

    Application::get()->run();

    return 0;
}
