#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

class ExampleLayer : public Walnut::Layer
{

public:
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

	virtual void OnUpdate(float ts) override
	{
		auto image = renderer.GetFinalImage();
		if (image)
		{
			Render();
		}
	}

	void Render()
	{
		Walnut::Timer timer;

		//renderer resize
		renderer.OnResize(m_Viewportwidth, m_ViewportHeight);
		//renderer render
		renderer.Render();
		
		m_LastRenderTime = timer.ElapsedMillis();
	}

private:
	Renderer renderer;
	uint32_t m_Viewportwidth, m_ViewportHeight;
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