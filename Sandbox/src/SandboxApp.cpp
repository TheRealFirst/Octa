#include <Pistachio.h>

#include "Platform/OpenGL/OpenGLShader.h"

#include "imgui/imgui.h"

#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

class ExampleLayer : public Pistachio::Layer
{
public:
	ExampleLayer() : Layer("Example"), m_Camera(-1.6f, 1.6f, -0.9f, 0.9f), m_CameraPosition(0.0f)
	{
		m_SquareVA.reset(Pistachio::VertexArray::Create());

		float squareVertices[5 * 4] = {
			-0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
			 0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
			-0.5f,  0.5f, 0.0f, 0.0f, 1.0f,
		};

		Pistachio::Ref<Pistachio::VertexBuffer> squareVB;
		squareVB.reset(Pistachio::VertexBuffer::Create(squareVertices, sizeof(squareVertices)));
		squareVB->SetLayout({
			{ Pistachio::ShaderDataType::Float3, "a_Position" },
			{ Pistachio::ShaderDataType::Float2, "a_TexCoord" },
		});
		m_SquareVA->AddVertexBuffer(squareVB);

		uint32_t squareIndices[6] = { 0, 1, 2, 2, 3, 0 };
		Pistachio::Ref<Pistachio::IndexBuffer> squareIB;
		squareIB.reset(Pistachio::IndexBuffer::Create(squareIndices, sizeof(squareIndices) / sizeof(uint32_t)));
		m_SquareVA->SetIndexBuffer(squareIB);

		std::string flatColorShaderVertexSrc = R"(
			#version 330 core
			
			layout(location = 0) in vec3 a_Position;

			uniform mat4 u_ViewProjection;
			uniform mat4 u_Transform;

			out vec3 v_Position;

			void main()
			{
				v_Position = a_Position;
				gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
			}
		)";

		std::string flatColorShaderFragmentSrc = R"(
			#version 330 core
			
			layout(location = 0) out vec4 color;

			in vec3 v_Position;

			uniform vec3 u_Color;

			void main()
			{
				color = vec4(u_Color, 1.0f);
			}
		)";

		m_FlatColorShader = Pistachio::Shader::Create("FlatColor", flatColorShaderVertexSrc, flatColorShaderFragmentSrc);
		
		auto textureShader = m_ShaderLibrary.Load("assets/shaders/Texture.glsl");

		m_Texture = Pistachio::Texture2D::Create("assets/textures/Checkerboard.png");
		m_TransparentTexture = Pistachio::Texture2D::Create("assets/textures/ChernoLogo.png");

		std::dynamic_pointer_cast<Pistachio::OpenGLShader>(textureShader)->Bind();
		std::dynamic_pointer_cast<Pistachio::OpenGLShader>(textureShader)->UploadUniformInt("u_Texture", 0);
	}

	void OnUpdate(Pistachio::Timestep ts) override
	{
		if(Pistachio::Input::IsKeyPressed(PA_KEY_LEFT))
			m_CameraPosition.x -= m_CameraMoveSpeed * ts;
		else if(Pistachio::Input::IsKeyPressed(PA_KEY_RIGHT))
			m_CameraPosition.x += m_CameraMoveSpeed * ts;
		
		if(Pistachio::Input::IsKeyPressed(PA_KEY_UP))
			m_CameraPosition.y += m_CameraMoveSpeed * ts;
		else if(Pistachio::Input::IsKeyPressed(PA_KEY_DOWN))
			m_CameraPosition.y -= m_CameraMoveSpeed * ts;

		if(Pistachio::Input::IsKeyPressed(PA_KEY_A))
			m_CameraRotation += m_CameraRotationSpeed * ts;
		else if(Pistachio::Input::IsKeyPressed(PA_KEY_D))
			m_CameraRotation -= m_CameraRotationSpeed * ts;

		
		Pistachio::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		Pistachio::RenderCommand::Clear();

		m_Camera.SetPosition(m_CameraPosition);
		m_Camera.SetRotation(m_CameraRotation);

		Pistachio::Renderer::BeginScene(m_Camera);

		static glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));

		std::dynamic_pointer_cast<Pistachio::OpenGLShader>(m_FlatColorShader)->Bind();
		std::dynamic_pointer_cast<Pistachio::OpenGLShader>(m_FlatColorShader)->UploadUniformFloat3("u_Color", m_SquareColor);
				
		for (int y = 0; y < 20; y++)
		{
			for (int x = 0; x < 20; x++)
			{
				glm::vec3 pos(x * 0.11f, y * 0.11f, 0.0f);
				glm::mat4 transform = glm::translate(glm::mat4(1.0f), pos) * scale;
				Pistachio::Renderer::Submit(m_FlatColorShader, m_SquareVA, transform);
			}
		}

		auto textureShader = m_ShaderLibrary.Get("Texture");

		m_Texture->Bind();
		Pistachio::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));
		m_TransparentTexture->Bind();
		Pistachio::Renderer::Submit(textureShader, m_SquareVA, glm::scale(glm::mat4(1.0f), glm::vec3(1.5f)));

		Pistachio::Renderer::EndScene();
	}

	virtual void OnImGuiRender() override
	{
		ImGui::Begin("Settings");
		ImGui::ColorEdit3("Square Color", glm::value_ptr(m_SquareColor));
		ImGui::End();
	}

	void OnEvent(Pistachio::Event& event) override
	{
	}

	
private:
	Pistachio::ShaderLibrary m_ShaderLibrary;
	Pistachio::Ref<Pistachio::Shader> m_FlatColorShader;
	Pistachio::Ref<Pistachio::VertexArray> m_SquareVA;

	Pistachio::Ref<Pistachio::Texture2D> m_Texture;
	Pistachio::Ref<Pistachio::Texture2D> m_TransparentTexture;

	Pistachio::OrthographicCamera m_Camera;
	glm::vec3 m_CameraPosition;
	float m_CameraMoveSpeed = 5.0f;

	float m_CameraRotation = 0.0f;
	float m_CameraRotationSpeed = 180.0f;

	glm::vec3 m_SquareColor = {0.2f, 0.3f, 0.8f}; 
};

class Sandbox : public Pistachio::Application {
public:
	Sandbox()
	{
		PushLayer(new ExampleLayer());
	}
	~Sandbox() {

	}
};

Pistachio::Application* Pistachio::CreateApplication() {
	return new Sandbox();
}
