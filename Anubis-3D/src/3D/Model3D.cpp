#include "3D/Model3D.hpp"
#include "3D/Scene.hpp"
#include "Time.hpp"
#include "Cursor.hpp"

namespace Anubis {

	int Model3D::_nextID = 0;

	Model3D::Model3D(float x, float y, float z, Quaternion rotation) {

		_id = _nextID++;

		_x = x;
		_y = y;
		_z = z;

		Quaternion _rotation = rotation;

		_meshes.clear();
	}

	Model3D::~Model3D() {

	}

	void Model3D::addMesh(std::shared_ptr<Mesh3D> mesh) {
		_meshes.push_back(mesh);
	}

	void Model3D::handleEvent(const sf::Event& event) {

		static sf::Vector2f offset(0.0f, 0.0f);
		static sf::Vector2f _lastRotationMouse = sf::Vector2f(Anubis::cursor->_position);

		auto size = Anubis::render_window->getSize();
		auto mouse = Anubis::cursor->_position;

		if (size.x == 0 || size.y == 0)
			return;

		auto camera = scene->_camera;

		float sinAngle = std::sin(scene->_cameraAngle);
		float cosAngle = std::cos(scene->_cameraAngle);

		float relativeX = _x - camera.x;
		float relativeY = _y - camera.y;
		float relativeZ = _z - camera.z;

		float modelViewX = relativeX * cosAngle - relativeZ * sinAngle;
		float modelViewY = relativeY;

		float distance = -relativeX * sinAngle - relativeZ * cosAngle;
		float aspect = float(size.x) / float(size.y);

		float halfHeight = distance * std::tan(30.f * 3.14159265f / 180.f);
		float halfWidth = halfHeight * aspect;

		float mouseX = (2.f * mouse.x / size.x - 1.f) * halfWidth;
		float mouseY = (1.f - 2.f * mouse.y / size.y) * halfHeight;

		if (scene->_hovered == this->shared_from_this()) {
			if (const auto* mbp = event.getIf<sf::Event::MouseButtonPressed>(); mbp) {

				scene->_pressed = this->shared_from_this();

				if (mbp->button == sf::Mouse::Button::Left) {
					offset.x = mouseX - modelViewX;
					offset.y = mouseY - modelViewY;
				}

				if (mbp->button == sf::Mouse::Button::Right) {
					_lastRotationMouse = sf::Vector2f(mbp->position);
				}
			}
		}

		if (scene->_pressed == this->shared_from_this()) {
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) {
				float viewX = mouseX - offset.x;
				float viewY = mouseY - offset.y;

				_x = camera.x + viewX * cosAngle - distance * sinAngle;
				_y = camera.y + viewY;
				_z = camera.z - viewX * sinAngle - distance * cosAngle;
			}

			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Right))
			{
				auto delta = sf::Vector2f(Anubis::cursor->_position) - _lastRotationMouse;
				_lastRotationMouse = sf::Vector2f(Anubis::cursor->_position);

				float dx = (float)(delta.x);
				float dy = (float)(delta.y);
				float length = std::hypot(dx, dy);
				if (length > 0.f) {
					float sensitivity = 0.5f; // Stopnie na piksel.
					float toRadians = 3.14159265f / 180.f;

					float halfAngle = length * sensitivity * toRadians * 0.5f;
					float s = std::sin(halfAngle) / length;

					Quaternion deltaRotation{
						std::cos(halfAngle),
						dy * s * cosAngle,
						dx * s,
						-dy * s * sinAngle
					};

					_rotation = deltaRotation * _rotation;
					_rotation.normalize();
				}
			}
		}

		if (const auto* mbr = event.getIf<sf::Event::MouseButtonReleased>(); mbr) {
			if (scene->_pressed == this->shared_from_this())
				scene->_pressed = nullptr;
		}
	}

	void Model3D::update() {

		// rotation about X axis
		float speed = 30.f; // Stopnie na sekundę.
		float toRadians = 3.14159265f / 180.f;
		float angle = speed * Anubis::deltaTime.asSeconds() * toRadians;

		Quaternion deltaRotation{
			std::cos(angle * 0.5f), // w
			0,						// x
			std::sin(angle * 0.5f), // y
			0.f						// z      
		};

		_rotation = deltaRotation * _rotation;
		_rotation.normalize();
	}

	void Model3D::drawShape() {
		for (auto& mesh : _meshes) {
			mesh->drawShape(_x, _y, _z, _rotation);
		}
	}

	void Model3D::draw() {

		for (auto& mesh : _meshes) {
			bool highlighting = scene->_pressed == this->shared_from_this() || (scene->_pressed == nullptr && scene->_hovered == this->shared_from_this());
			mesh->draw(highlighting, _x, _y, _z, _rotation);
		}
	}
}
