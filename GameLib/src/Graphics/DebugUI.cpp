#include "DebugUI.h"

#include <string>
#include <format>

#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>
#include <imgui.h>

namespace Game {

	DebugUI::DebugUI(const Window& window)
		: m_Window{ window }
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		auto& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		ShowCursor(true);
		io.MouseDrawCursor |= io.WantCaptureMouse;

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_InitForOpenGL(m_Window.GetNativeHandle());
		ImGui_ImplOpenGL3_Init();
	}

	DebugUI::~DebugUI()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	void DebugUI::Render(Scene& scene) const
	{
		auto& io = ImGui::GetIO();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::LabelText("FPS", "%0.1f", io.Framerate);

		for (const auto& entity : scene.entities)
		{
			auto& material = scene.materialManager[entity.materialKey];

			if (ImGui::CollapsingHeader(entity.name.c_str()))
			{
				float color[3]{};
				std::memcpy(color, &material.color, sizeof(color));

				const auto label = std::format("{} color", entity.name);

				if (ImGui::ColorPicker3(label.c_str(), color))
				{
					std::memcpy(&material.color, color, sizeof(color));
				}
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void DebugUI::AddMouseEvent(const MouseButtonEvent& evt) const
	{
		auto& io = ImGui::GetIO();
		io.AddMouseButtonEvent(0, evt.GetState() == MouseButtonState::DOWN);
	}

}
