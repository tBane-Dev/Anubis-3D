#include "3D/Mesh3D.hpp"
#include "3D/Scene.hpp"
#include <SFML/OpenGL.hpp>
#include "RenderWindow.hpp"
#include <iostream>
#include "Cursor.hpp"

namespace Anubis {
	Mesh3D::Mesh3D() {

	}

	Mesh3D::~Mesh3D() {

	}

	void Mesh3D::setTexture(std::shared_ptr<Texture> texture) {
		_texture = texture;
	}

	void Mesh3D::drawShape(float x, float y, float z, Anubis::Quaternion rotation) {
		glPushMatrix();

		glTranslatef(x, y, z);
		auto matrix = rotation.matrix();
		glMultMatrixf(matrix.data());

		glDisable(GL_TEXTURE_2D);
		glColor3f(0.75f, 0.75f, 0.75f);

		glBegin(GL_TRIANGLES);

		for (auto& vertex : _vertices) {

			glNormal3f(vertex._nx, vertex._ny, vertex._nz);
			glVertex3f(vertex._x, vertex._y, vertex._z);
		}

		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);

		glPopMatrix();
	}

	void Mesh3D::draw(bool isHighlighted, float x, float y, float z, Anubis::Quaternion rotation) {
		glPushMatrix();

		glTranslatef(x, y, z);
		auto matrix = rotation.matrix();
		glMultMatrixf(matrix.data());

		glEnable(GL_TEXTURE_2D);
		(isHighlighted) ? glColor3f(1.0f, 1.0f, 1.f) : glColor3f(0.75f, 0.75f, 0.75f);
		glBindTexture(GL_TEXTURE_2D, _texture ? _texture->_texture->getNativeHandle() : 0);

		glBegin(GL_TRIANGLES);

		for (auto& vertex : _vertices) {

			glNormal3f(vertex._nx, vertex._ny, vertex._nz);
			glTexCoord2f(vertex._u, 1.0f - vertex._v);
			glVertex3f(vertex._x, vertex._y, vertex._z);

		}

		glEnd();

		glBindTexture(GL_TEXTURE_2D, 0);

		glPopMatrix();
	}

	std::shared_ptr<Mesh3D> generateCube(float size) {
		std::shared_ptr<Mesh3D> cube = std::make_shared<Mesh3D>();
		// Front face
		cube->_vertices.emplace_back(-size, -size, size, 0.f, 0.f, 1.f, 0.f, 0.f);
		cube->_vertices.emplace_back(size, -size, size, 0.f, 0.f, 1.f, 1.f, 0.f);
		cube->_vertices.emplace_back(size, size, size, 0.f, 0.f, 1.f, 1.f, 1.f);
		cube->_vertices.emplace_back(-size, -size, size, 0.f, 0.f, 1.f, 0.f, 0.f);
		cube->_vertices.emplace_back(size, size, size, 0.f, 0.f, 1.f, 1.f, 1.f);
		cube->_vertices.emplace_back(-size, size, size, 0.f, 0.f, 1.f, 0.f, 1.f);
		// Back face
		cube->_vertices.emplace_back(-size, -size, -size, 0.f, 0.f, -1.f, 1.f, 0.f);
		cube->_vertices.emplace_back(size, -size, -size, 0.f, 0.f, -1.f, 0.f, 0.f);
		cube->_vertices.emplace_back(size, size, -size, 0.f, 0.f, -1.f, 0.f, 1.f);
		cube->_vertices.emplace_back(-size, -size, -size, 0.f, 0.f, -1.f, 1.f, 0.f);
		cube->_vertices.emplace_back(size, size, -size, 0.f, 0.f, -1.f, 0.f, 1.f);
		cube->_vertices.emplace_back(-size, size, -size, 0.f, 0.f, -1.f, 1.f, 1.f);
		// Left face
		cube->_vertices.emplace_back(-size, -size, -size, -1., 0., 0., 0., 0.);
		cube->_vertices.emplace_back(-size, -size, size, -1., 0., 0., 1., 0.);
		cube->_vertices.emplace_back(-size, size, size, -1., 0., 0., 1., 1.);
		cube->_vertices.emplace_back(-size, -size, -size, -1., 0., 0., 0., 0.);
		cube->_vertices.emplace_back(-size, size, size, -1., 0., 0., 1., 1.);
		cube->_vertices.emplace_back(-size, size, -size, -1., 0., 0., 0., 1.);
		// Right face
		cube->_vertices.emplace_back(size, -size, -size, 1., 0., 0., 1., 0.);
		cube->_vertices.emplace_back(size, -size, size, 1., 0., 0., 0., 0.);
		cube->_vertices.emplace_back(size, size, size, 1., 0., 0., 0., 1.);
		cube->_vertices.emplace_back(size, -size, -size, 1., 0., 0., 1., 0.);
		cube->_vertices.emplace_back(size, size, size, 1., 0., 0., 0., 1.);
		cube->_vertices.emplace_back(size, size, -size, 1., 0., 0., 1., 1.);
		// Top face
		cube->_vertices.emplace_back(-size, size, -size, 0., 1., 0., 0., 1.);
		cube->_vertices.emplace_back(-size, size, size, 0., 1., 0., 0., 0.);
		cube->_vertices.emplace_back(size, size, size, 0., 1., 0., 1., 0.);
		cube->_vertices.emplace_back(-size, size, -size, 0., 1., 0., 0., 1.);
		cube->_vertices.emplace_back(size, size, size, 0., 1., 0., 1., 0.);
		cube->_vertices.emplace_back(size, size, -size, 0., 1., 0., 1., 1.);
		// Bottom face
		cube->_vertices.emplace_back(-size, -size, -size, 0., -1., 0., 1., 1.);
		cube->_vertices.emplace_back(-size, -size, size, 0., -1., 0., 1., 0.);
		cube->_vertices.emplace_back(size, -size, size, 0., -1., 0., 0., 0.);
		cube->_vertices.emplace_back(-size, -size, -size, 0., -1., 0., 1., 1.);
		cube->_vertices.emplace_back(size, -size, size, 0., -1., 0., 0., 0.);
		cube->_vertices.emplace_back(size, -size, -size, 0., -1., 0., 0., 1.);

		return cube;
	}
}