#include "DebugUI.h"

#include "Math/Ray.h"
#include "Math/Matrix4.h"
#include "Math/Vector4.h"
#include "Utils/Log.h"

#include <string>
#include <format>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <ImGuizmo.cpp>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_win32.h>

namespace {

	Game::Ray ScreenRay(const Game::MouseButtonEvent& evt, const Game::Window& window, const Game::Camera& camera)
	{
		const auto x = 2.0f * evt.GetX() / window.GetRenderWidth() - 1.0f;
		const auto y = 1.0f - 2.0f * evt.GetY() / window.GetRenderHeight();
		const auto rayClip = Game::vec4{ x, y, -1.0f, 1.0f };

		const auto invProj = Game::mat4::Invert(camera.GetData().projection);
		auto rayEye = invProj * rayClip;
		rayEye.z = -1.0f;
		rayEye.w = 0.0f;
		//rayEye = Game::vec4{ rayEye.x, rayEye.y, -1.0f, 0.0f };

		const auto invView = Game::mat4::Invert(camera.GetData().view);
		const auto dirWS = Game::vec3::Normalize(invView * rayEye);
		const auto originWS = Game::vec3{ invView[12], invView[13], invView[14] };

		return { originWS, dirWS };
	}

}

namespace Game {

	DebugUI::DebugUI(const Window& window)
		: m_Window{ window }
		, m_Click{}
		, m_SelectedEntity{}
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
			}

			if (&entity == m_SelectedEntity)
			{
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
			const auto pickRay = ScreenRay(*m_Click, m_Window, scene.camera);
			const auto intersection = scene.IntersectRay(pickRay);
			m_SelectedEntity = intersection.transform([](const auto& e) { return e.entity; }).value_or(nullptr);

			m_Click.reset();
		}
	}

	void DebugUI::AddMouseEvent(const MouseButtonEvent& evt)
	{
		auto& io = ImGui::GetIO();
		io.AddMouseButtonEvent(0, evt.GetState() == MouseButtonState::DOWN);

		if (!io.WantCaptureMouse)
		{
			m_Click = evt;
		}
	}

}
