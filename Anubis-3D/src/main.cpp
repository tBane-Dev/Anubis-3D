#include <iostream>

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include "gl/glu.h"

#include "Intro.hpp"

#include "3D/Vertex3D.hpp"
#include "3D/Mesh3D.hpp" 
#include "3D/Model3D.hpp"
#include "3D/Scene.hpp"

#include "RenderWindow.hpp"
#include "DebugLog.hpp"
#include "Time.hpp"
#include "Cursor.hpp"
#include "TexturesManager.hpp"
#include "GUIManager.hpp"

sf::Font font;

int main()
{
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    
    Anubis::render_window = std::make_unique<sf::RenderWindow>(sf::VideoMode(sf::Vector2u(1920, 1080), 32u), "Anubis-3D", sf::State::Windowed, settings);
    Anubis::render_window->setVerticalSyncEnabled(false);
    Anubis::render_window->setFramerateLimit(0);

    auto* version = glGetString(GL_VERSION);
    auto* renderer = glGetString(GL_RENDERER);

    if (version)
        std::cout << "OpenGL: " << version << '\n';

    if (renderer)
        std::cout << "Renderer: " << renderer << '\n';

    Anubis::render_window->clear(sf::Color::Black);
    Anubis::render_window->display();

    while (const std::optional event = Anubis::render_window->waitEvent()) {

        if (event->is<sf::Event::Closed>()) {
            Anubis::render_window->close();
            exit(0);
        }

        if (const auto* kp = event->getIf<sf::Event::KeyPressed>(); kp && kp->code == sf::Keyboard::Key::Space) {
            break;
        }
    }

    Anubis::renderIntro();

    Anubis::render_window->resetGLStates();

    if (!font.openFromFile("C:/Windows/Fonts/consola.ttf")) {
        DebugError(L"Failed to load font");
        exit(0);
    }

    Anubis::textures_manager = std::make_shared<Anubis::TexturesManager>();
    Anubis::cursor = std::make_shared<Anubis::Cursor>();
    
    Anubis::scene = std::make_shared<Anubis::Scene>();
    
    {
        // add center cube
        std::shared_ptr<Anubis::Model3D> cube = std::make_shared<Anubis::Model3D>();
        std::shared_ptr<Anubis::Mesh3D> cube_mesh = Anubis::generateCube(0.5f);
        cube_mesh->setTexture(Anubis::textures_manager->getTexture(L"assets\\tex\\wall1.png"));
        cube->addMesh(cube_mesh);
        Anubis::scene->addModel(cube);
    }
    
    {
        // add left cube
        std::shared_ptr<Anubis::Model3D> cube = std::make_shared<Anubis::Model3D>();
        std::shared_ptr<Anubis::Mesh3D> cube_mesh = Anubis::generateCube(0.5f);
        cube_mesh->setTexture(Anubis::textures_manager->getTexture(L"assets\\tex\\wall2.png"));
        cube->addMesh(cube_mesh);
        cube->_x = -2.0f;
        Anubis::scene->addModel(cube);
    }
    
    {
        // add right cube
        std::shared_ptr<Anubis::Model3D> cube = std::make_shared<Anubis::Model3D>();
        std::shared_ptr<Anubis::Mesh3D> cube_mesh = Anubis::generateCube(0.5f);
        cube_mesh->setTexture(Anubis::textures_manager->getTexture(L"assets\\tex\\wall3.png"));
        cube->addMesh(cube_mesh);
        cube->_x = 2.0f;
        Anubis::scene->addModel(cube);
    }

    
    // OpenGL init
    glClearDepth(1.f);
    glClearColor(0.25f, 0.25f, 0.25f, 1.f);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LESS);

    // init FPS clock
    sf::Clock FPSClock;
    int frames = 0;
    float elapsed = 0.0f;
    float FPS = 0;
    float ms = 0;

    while (Anubis::render_window->isOpen())
    {
        Anubis::prevTime = Anubis::currentTime;
        Anubis::currentTime = Anubis::mainClock.getElapsedTime();
        Anubis::deltaTime = Anubis::currentTime - Anubis::prevTime;
    
        // FPS TIMER
        float dtFPS = FPSClock.restart().asSeconds();
        frames++;
        elapsed += dtFPS;
    
        if (elapsed >= 0.5f) {
            FPS = frames / elapsed;
            ms = 1000.0f / FPS;
    
            std::ostringstream ss;
            ss << "Anubis-3D - " << std::fixed << std::setprecision(2) << FPS << " FPS / " << std::setprecision(3) << ms << " ms";
            Anubis::render_window->setTitle(ss.str());
            frames = 0;
            elapsed = 0.0f;
        }
    
    
    
        // Cursor Hover
        Anubis::cursor->update();
        Anubis::scene->cursorHover();
    
        // Handle Events
        while (const std::optional event = Anubis::render_window->pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                Anubis::render_window->close();
                exit(0);
            }
    
    
            Anubis::scene->handleEvent(*event);
    
        }
    
        // Update
        Anubis::scene->update();
    
        // Render
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
        Anubis::scene->draw();
    
        
	    // draw SFML 3.1
        Anubis::render_window->pushGLStates();
    
        float border = 4.0f;
        sf::RectangleShape rect(sf::Vector2f(448, 64));
        rect.setFillColor(sf::Color(47, 47, 47));
        rect.setOutlineColor(sf::Color::Black);
        rect.setOutlineThickness(border);
        rect.setPosition(sf::Vector2f(border, border));
        Anubis::render_window->draw(rect);
    
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(2) << FPS << " FPS / " << std::setprecision(3) << ms << " ms";
        sf::Text fps(font, ss.str(), 32);
        fps.setPosition(sf::Vector2f(16.0f, 16.0f));
        fps.setFillColor(sf::Color::White);
        Anubis::render_window->draw(fps);
    
        Anubis::render_window->popGLStates();
    
        // render submit
        Anubis::render_window->display();
    }

    return 0;
}
