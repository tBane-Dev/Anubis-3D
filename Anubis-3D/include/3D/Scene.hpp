#pragma once
#include "Model3D.hpp"

namespace Anubis {

	class Scene {
	public:

		sf::Vector3f _camera;
		float _cameraAngle;
		float _cameraRadius;
		sf::Vector2i _lastCameraMouse;


		std::vector<std::shared_ptr<Model3D>> _models;
		std::shared_ptr<Model3D> _hovered;
		std::shared_ptr<Model3D> _pressed;

		Scene();
		~Scene();

		void setViewport();
		void setProjection();
		void setCamera();

		void addModel(std::shared_ptr<Anubis::Model3D> model);

		void drawShapesOfModels();
		void drawModels();

		int getHoveredModelId();

		void cursorHover();
		void handleEvent(const sf::Event& event);
		void update();
		void draw();
	};

	extern std::shared_ptr<Scene> scene;
}