#include "DebugUI.h"

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
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::ShowDemoWindow();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void DebugUI::AddMouseEvent(const MouseButtonEvent& evt) const
	{
		auto& io = ImGui::GetIO();
		io.AddMouseButtonEvent(0, evt.GetState() == MouseButtonState::DOWN);
	}

}
