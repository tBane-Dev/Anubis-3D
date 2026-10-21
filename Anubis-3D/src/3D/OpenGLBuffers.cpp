#include "3D/OpenGLBuffers.hpp"
#include "DebugLog.hpp"

namespace Anubis::GL {
	bool initialize() {
		genBuffers = reinterpret_cast<GenBuffersFunction>(
			sf::Context::getFunction("glGenBuffers")
			);

		bindBuffer = reinterpret_cast<BindBufferFunction>(
			sf::Context::getFunction("glBindBuffer")
			);

		bufferData = reinterpret_cast<BufferDataFunction>(
			sf::Context::getFunction("glBufferData")
			);

		deleteBuffers = reinterpret_cast<DeleteBuffersFunction>(
			sf::Context::getFunction("glDeleteBuffers")
			);

		if (!genBuffers || !bindBuffer ||
			!bufferData || !deleteBuffers) {
			DebugError(L"OpenGL buffer functions are unavailable");
			return false;
		}

		DebugStat(L"OpenGL buffer functions loaded");
		return true;
	}
}