#pragma once
#include <vector>
#include "3D/Quaternion.hpp"
#include "3D/Vertex3D.hpp"
#include "TexturesManager.hpp"

namespace Anubis {
	class Mesh3D {
	public:
		std::vector<Vertex3D> _vertices;
		std::shared_ptr<Texture> _texture;

		Mesh3D();
		~Mesh3D();

		void setTexture(std::shared_ptr<Texture> texture);

		void drawShape(float x = 0, float y = 0, float z = 0, Anubis::Quaternion rotation = Anubis::Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
		void draw(bool isHighlighted, float x = 0, float y = 0, float z = 0, Anubis::Quaternion rotation = Anubis::Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
	};

	std::shared_ptr<Mesh3D> generateCube(float size);
}