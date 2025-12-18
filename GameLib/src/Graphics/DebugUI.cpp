#include "DebugUI.h"

#include "Math/Matrix4.h"
#include "Utils/Log.h"

#include <string>
#include <format>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <ImGuizmo.cpp>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>

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

	void DebugUI::Render(Scene& scene)
	{
		auto& io = ImGui::GetIO();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGuizmo::SetOrthographic(false);
		ImGuizmo::BeginFrame();
		ImGuizmo::Enable(true);
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGui::LabelText("FPS", "%0.1f", io.Framerate);

		for (auto& entity : scene.entities)
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

				auto transform = mat4{ entity.transform };
				const auto& cameraData = scene.camera.GetData();

				ImGuizmo::Manipulate(
					cameraData.view.Data().data(),
					cameraData.projection.Data().data(),
					ImGuizmo::TRANSLATE | ImGuizmo::SCALE | ImGuizmo::BOUNDS | ImGuizmo::ROTATE,
					ImGuizmo::WORLD,
					const_cast<float*>(transform.Data().data()),
					nullptr,
					nullptr,
					nullptr,
					nullptr);

				entity.transform = transform;
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (m_Click)
		{
			uint8_t buffer[4]{};
			glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glReadBuffer(GL_BACK);
			glReadPixels(static_cast<GLint>(m_Click->GetX()), static_cast<GLint>(m_Click->GetY()), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
			Log::Trace("r:{:x}, g:{:x}, b:{:x}", buffer[0], buffer[1], buffer[2]);
			m_Click.reset();
		}
	}

	void DebugUI::AddMouseEvent(const MouseButtonEvent& evt)
	{
		auto& io = ImGui::GetIO();
		io.AddMouseButtonEvent(0, evt.GetState() == MouseButtonState::DOWN);

		m_Click = evt;
	}

}
