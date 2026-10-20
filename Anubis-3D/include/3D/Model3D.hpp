#pragma once
#include <vector>
#include "3D/Quaternion.hpp"
#include "3D/Vertex3D.hpp"
#include "3D/Mesh3D.hpp"
#include "TexturesManager.hpp"

namespace Anubis {
	class Model3D : public std::enable_shared_from_this<Model3D> {
	public:
		static int _nextID;
		int _id;
		float _x, _y, _z;
		Quaternion _rotation;

		std::vector<std::shared_ptr<Mesh3D>> _meshes;

		Model3D(float x = 0, float y = 0, float z = 0, Quaternion rotation = Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
		~Model3D();

		void addMesh(std::shared_ptr<Anubis::Mesh3D> mesh);

		void handleEvent(const sf::Event& event);
		void update();
		void drawShape();
		void draw();
	};
}