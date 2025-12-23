#pragma once

#include "Events/MouseButtonEvent.h"
#include "Core/Scene.h"
#include "Window.h"

#include <optional>

namespace Game {

	class DebugUI
	{
	public:
		DebugUI(const Window& window);
		~DebugUI();

		void Render(Scene& scene);

		void AddMouseEvent(const MouseButtonEvent& evt);

	private:
		const Window& m_Window;
		std::optional<MouseButtonEvent> m_Click;
		const Entity* m_SelectedEntity;
	};

}
