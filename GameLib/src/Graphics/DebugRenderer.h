#pragma once

#include "Core/Entity.h"
#include "Core/Scene.h"
#include "Events/MouseButtonEvent.h"
#include "Resources\ResourceLoader.h"
#include "MeshManager.h"
#include "TextureManager.h"
#include "Renderer.h"
#include "Window.h"

#include <optional>

namespace Game {

	class DebugRenderer : public Renderer
	{
	public:
		DebugRenderer(const Window& window, ResourceLoader& resourceLoader, TextureManager& textureManager, MeshManager& meshManager);
		~DebugRenderer();

		void AddMouseEvent(const MouseButtonEvent& evt);
		void SetEnabled(bool enabled);

	protected:
		void PostRender(Scene& scene) override;

	private:
		bool m_Enabled;
		std::optional<MouseButtonEvent> m_Click;
		const Entity* m_SelectedEntity;
	};

}
