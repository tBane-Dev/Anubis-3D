#include "3D/Scene.hpp"
#include <SFML/OpenGL.hpp>
#include <GL/glu.h>
#include "DebugLog.hpp"
#include "RenderWindow.hpp"
#include "Cursor.hpp"

namespace Anubis {

    Scene::Scene() {

        _cameraAngle = 0.f; // radians
        _cameraRadius = 5.f;
        _lastCameraMouse = -sf::Vector2i(0, 0);

        _camera.x = std::sin(_cameraAngle) * _cameraRadius;
        _camera.y = 0.f;
        _camera.z = std::cos(_cameraAngle) * _cameraRadius;

    }

    Scene::~Scene() {
    }

    void Scene::setViewport()
    {
        glViewport(0, 0, Anubis::render_window->getSize().x, Anubis::render_window->getSize().y);
    }

    void Scene::setProjection()
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        gluPerspective(
            60.0,
            double(Anubis::render_window->getSize().x) / double(Anubis::render_window->getSize().y),
            0.1,
            100.0
        );
    }

    void Scene::setCamera()
    {
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        float toDegrees = 180.f / 3.14159265f;

        glRotatef(-_cameraAngle * toDegrees, 0.f, 1.f, 0.f);
        glTranslatef(-_camera.x, -_camera.y, -_camera.z);
    }

    void Scene::addModel(std::shared_ptr<Model3D> model) {
        _models.push_back(model);
    }

    void Scene::drawShapesOfModels() {
        for (auto& model : _models) {
            model->drawShape();
        }
    }

    void Scene::drawModels() {
        for (auto& model : _models) {
            model->draw();
        }
    }

    int Scene::getHoveredModelId() {
        constexpr int BUFFER_SIZE = 64;
        GLuint selectBuffer[BUFFER_SIZE];

        GLint viewport[4];

        setViewport();
        glGetIntegerv(GL_VIEWPORT, viewport);

        glSelectBuffer(BUFFER_SIZE, selectBuffer);
        glRenderMode(GL_SELECT);

        glInitNames();
        glPushName(0);

        glClear(GL_DEPTH_BUFFER_BIT);

        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();

        // obszar kliknięcia: 1x1 pikseli
        gluPickMatrix(
            Anubis::cursor->_position.x,
            viewport[3] - Anubis::cursor->_position.y - 1,
            1.0,
            1.0,
            viewport
        );

        gluPerspective(
            60.0,
            double(Anubis::render_window->getSize().x) / double(Anubis::render_window->getSize().y),
            0.1,
            100.0
        );

        setCamera();

        for (auto& model : _models) {
            glLoadName(model->_id);
            model->drawShape();
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();

        glFlush();

        GLint hits = glRenderMode(GL_RENDER);

        if (hits <= 0) {
            return -1;
        }

        GLuint* ptr = selectBuffer;

        GLuint nearestDepth = UINT_MAX;
        int nearestId = -1;

        for (int i = 0; i < hits; i++) {

            GLuint namesCount = ptr[0];
            GLuint minDepth = ptr[1];
            //GLUint maxDepth = ptr[2];

            ptr += 3;

            if (namesCount > 0 && minDepth < nearestDepth) {
                nearestDepth = minDepth;
                nearestId = ptr[0];
            }

            ptr += namesCount;
        }

        return nearestId;
    }

    void Scene::cursorHover() {

        _hovered = nullptr;

        int id = getHoveredModelId();

        if (id >= 0) {
            auto it = std::find_if(_models.begin(), _models.end(),
                [id](const auto& model) {
                    return model->_id == id;
                });

            if (it != _models.end())
                _hovered = *it;
        }
    }

    void Scene::handleEvent(const sf::Event& event) {

        if (const auto* scroll = event.getIf<sf::Event::MouseWheelScrolled>(); scroll) {
            _cameraRadius -= scroll->delta * 0.5f;

            _camera.x = std::sin(_cameraAngle) * _cameraRadius;
            _camera.y = 0.f;
            _camera.z = std::cos(_cameraAngle) * _cameraRadius;
            return;
        }

        if (const auto* pressed = event.getIf<sf::Event::MouseButtonPressed>(); pressed && pressed->button == sf::Mouse::Button::Middle)
        {
            _lastCameraMouse = pressed->position;
            return;
        }

        if (const auto* moved = event.getIf<sf::Event::MouseMoved>(); moved && sf::Mouse::isButtonPressed(sf::Mouse::Button::Middle))
        {
            sf::Vector2i delta = moved->position - _lastCameraMouse;
            _lastCameraMouse = moved->position;

            float sensitivity = 0.005f; // Radiany na piksel.
            _cameraAngle -= float(delta.x) * sensitivity;

            _camera.x = std::sin(_cameraAngle) * _cameraRadius;
            _camera.y = 0.f;
            _camera.z = std::cos(_cameraAngle) * _cameraRadius;
            return;
        }
        for (auto& model : _models) {
            model->handleEvent(event);
        }

    }

    void Scene::update() {
        for (auto& model : _models) {
            model->update();
        }
    }

    void Scene::draw() {

        setViewport();
        setProjection();
        setCamera();

        drawModels();
    }

    std::shared_ptr<Scene> scene;
}

