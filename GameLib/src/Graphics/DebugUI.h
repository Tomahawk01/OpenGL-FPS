#pragma once

#include "Events/MouseButtonEvent.h"
#include "Scene.h"
#include "Window.h"

namespace Game {

	class DebugUI
	{
	public:
		DebugUI(const Window& window);
		~DebugUI();

		void Render(Scene& scene) const;

		void AddMouseEvent(const MouseButtonEvent& evt) const;

	private:
		const Window& m_Window;
	};

}
