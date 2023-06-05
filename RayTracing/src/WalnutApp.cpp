#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h"

#include <glm/gtc/type_ptr.hpp>

class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		:m_Camera(45.0f, 0.1f, 100.0f)
	{

		{
			Sphere sphere;
			sphere.Position = { 0.0f,0.0f,0.0f };
			sphere.Radius = 0.5f;
			sphere.Albedo = { 1.0f, 0.0f, 1.0f };
			m_scene.Spheres.push_back(sphere);
			m_scene.Spheres.push_back(sphere);
		}

		{
			Sphere sphere;
			sphere.Position = { 2.0f,0.0f,0.0f };
			sphere.Radius = 1.0f;
			sphere.Albedo = { 0.0f, 1.0f, 1.0f };
			m_scene.Spheres.push_back(sphere);
		}
	}
public:

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
		auto image = renderer.GetFinalImage();
		if (image)
		{
			Render();
		}
	}

	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		//create button set click event 
		if (ImGui::Button("Render"))
		{
			Render();
		}
		ImGui::End();
		

		ImGui::Begin("Scene");
		ImGui::DragFloat3("Position", glm::value_ptr(m_scene.Spheres[0].Position), 0.05f);
		ImGui::DragFloat("Radius", &m_scene.Spheres[0].Radius, 0.05f);
		ImGui::DragFloat3("Albedo", glm::value_ptr(m_scene.Spheres[0].Albedo), 0.05f);



		ImGui::End();





		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
		ImGui::Begin("Viewport");

		m_Viewportwidth = static_cast<uint32_t>(ImGui::GetContentRegionAvail().x);
		m_ViewportHeight= static_cast<uint32_t>(ImGui::GetContentRegionAvail().y);
	
		//render
		auto image = renderer.GetFinalImage();
		if (image)
		{
			//ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() });

			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));//flip uv
		}


		ImGui::End();
		ImGui::PopStyleVar();// ? ?
	
	}


	void Render()
	{
		Walnut::Timer timer;

		//renderer resize
		renderer.OnResize(m_Viewportwidth, m_ViewportHeight);
		m_Camera.OnResize(m_Viewportwidth, m_ViewportHeight);
		//renderer render
		renderer.Render(m_scene, m_Camera);
		
		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer renderer;
	Camera m_Camera;
	uint32_t m_Viewportwidth, m_ViewportHeight;
	Scene m_scene;
	float m_LastRenderTime;

};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "RayTracing Example";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}