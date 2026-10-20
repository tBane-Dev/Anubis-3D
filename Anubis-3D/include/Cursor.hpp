#pragma once
#include "RenderWindow.hpp"

namespace Anubis {

	class Cursor {
	public:
		sf::Vector2i _lastPosition;
		sf::Vector2i _position;

		Cursor();
		~Cursor();

		void update();
		void handleEvent();
		void draw();
	};

	extern std::shared_ptr<Cursor> cursor;
}

