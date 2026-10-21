#include "3D/OpenGLBuffers.hpp"
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
		if (_vertexBuffer != 0)
			GL::deleteBuffers(1, &_vertexBuffer);

		if (_indexBuffer != 0)
			GL::deleteBuffers(1, &_indexBuffer);
	}

	void Mesh3D::setTexture(std::shared_ptr<Texture> texture) {
		_texture = texture;
	}

	void Mesh3D::uploadToGPU() {
		if (!GL::genBuffers || !GL::bindBuffer || !GL::bufferData)
			throw std::runtime_error("OpenGL buffer functions are not initialized");

		if (_indices.size() >
			static_cast<std::size_t>(std::numeric_limits<GLsizei>::max()))
			throw std::runtime_error("Too many indices");

		if (_vertexBuffer == 0)
			GL::genBuffers(1, &_vertexBuffer);

		if (_indexBuffer == 0)
			GL::genBuffers(1, &_indexBuffer);

		GL::bindBuffer(GL::ArrayBuffer, _vertexBuffer);

		GL::bufferData(
			GL::ArrayBuffer,
			static_cast<std::ptrdiff_t>(_vertices.size() * sizeof(Vertex3D)),
			_vertices.data(),
			GL::StaticDraw
		);

		GL::bindBuffer(GL::ElementArrayBuffer, _indexBuffer);

		GL::bufferData(
			GL::ElementArrayBuffer,
			static_cast<std::ptrdiff_t>(
				_indices.size() * sizeof(_indices[0])
				),
			_indices.data(),
			GL::StaticDraw
		);

		GL::bindBuffer(GL::ArrayBuffer, 0);
		GL::bindBuffer(GL::ElementArrayBuffer, 0);

		_gpuIndexCount = static_cast<GLsizei>(_indices.size());
	}

	void Mesh3D::drawBuffers(bool textured) {
		static_assert(std::is_standard_layout_v<Vertex3D>);

		if (_vertexBuffer == 0 || _indexBuffer == 0 ||
			_gpuIndexCount == 0)
			return;

		glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

		GL::bindBuffer(GL::ArrayBuffer, _vertexBuffer);
		GL::bindBuffer(GL::ElementArrayBuffer, _indexBuffer);

		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);

		glVertexPointer(
			3,
			GL_FLOAT,
			sizeof(Vertex3D),
			reinterpret_cast<const void*>(offsetof(Vertex3D, _x))
		);

		glNormalPointer(
			GL_FLOAT,
			sizeof(Vertex3D),
			reinterpret_cast<const void*>(offsetof(Vertex3D, _nx))
		);

		if (textured) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);

			glTexCoordPointer(
				2,
				GL_FLOAT,
				sizeof(Vertex3D),
				reinterpret_cast<const void*>(offsetof(Vertex3D, _u))
			);
		}
		else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		glDrawElements(
			GL_TRIANGLES,
			_gpuIndexCount,
			GL_UNSIGNED_INT,
			nullptr
		);

		glPopClientAttrib();
	}


	void Mesh3D::drawShape(float x, float y, float z, Anubis::Quaternion rotation, float scale) {
		glPushMatrix();

		glTranslatef(x, y, z);
		auto matrix = rotation.matrix();
		glMultMatrixf(matrix.data());

		glScalef(scale, scale, scale);

		glDisable(GL_TEXTURE_2D);

		drawBuffers(false);

		glPopMatrix();
	}

	void Mesh3D::draw(bool isHighlighted, float x, float y, float z, Anubis::Quaternion rotation, float scale) {
		glPushMatrix();

		glTranslatef(x, y, z);
		auto matrix = rotation.matrix();
		glMultMatrixf(matrix.data());

		glScalef(scale, scale, scale);

		glEnable(GL_TEXTURE_2D);
		(isHighlighted) ? glColor3f(1.0f, 1.0f, 1.f) : glColor3f(0.75f, 0.75f, 0.75f);
		glBindTexture(GL_TEXTURE_2D, _texture ? _texture->_texture->getNativeHandle() : 0);

		drawBuffers(true);

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

		cube->_indices = {
			0, 1, 2, 3, 4, 5,        // Front
			6, 7, 8, 9, 10, 11,      // Back
			12, 13, 14, 15, 16, 17,  // Left
			18, 19, 20, 21, 22, 23,  // Right
			24, 25, 26, 27, 28, 29,  // Top
			30, 31, 32, 33, 34, 35   // Bottom
		};

		return cube;
	}
}