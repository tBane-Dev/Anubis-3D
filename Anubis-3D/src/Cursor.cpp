#include "Cursor.hpp"

namespace Anubis {
	Cursor::Cursor() {

		_lastPosition = sf::Mouse::getPosition(*render_window);
		_position = sf::Mouse::getPosition(*render_window);
	}

	Cursor::~Cursor() {

	}

	void Cursor::update() {
		_lastPosition = _position;
		_position = sf::Mouse::getPosition(*Anubis::render_window);
	}

	void Cursor::handleEvent() {


	}

	void Cursor::draw() {

	}

	std::shared_ptr<Cursor> cursor;
}