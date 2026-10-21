#include "3D/OpenGLBuffers.hpp"
#include "DebugLog.hpp"

namespace Anubis::GL {
	bool initialize() {
		genBuffers = reinterpret_cast<GenBuffersFunction>(sf::Context::getFunction("glGenBuffers"));
		bindBuffer = reinterpret_cast<BindBufferFunction>(sf::Context::getFunction("glBindBuffer"));
		bufferData = reinterpret_cast<BufferDataFunction>(sf::Context::getFunction("glBufferData"));
		deleteBuffers = reinterpret_cast<DeleteBuffersFunction>(sf::Context::getFunction("glDeleteBuffers"));

		if (!genBuffers || !bindBuffer ||
			!bufferData || !deleteBuffers) {
			DebugError(L"OpenGL buffer functions are unavailable");
			return false;
		}

		DebugStat(L"OpenGL buffer functions loaded");

		genFramebuffers = reinterpret_cast<GenFramebuffersFunction>(sf::Context::getFunction("glGenFramebuffers"));
		bindFramebuffer = reinterpret_cast<BindFramebufferFunction>(sf::Context::getFunction("glBindFramebuffer"));
		framebufferTexture2D = reinterpret_cast<FramebufferTexture2DFunction>(sf::Context::getFunction("glFramebufferTexture2D"));
		checkFramebufferStatus = reinterpret_cast<CheckFramebufferStatusFunction>(sf::Context::getFunction("glCheckFramebufferStatus"));
		deleteFramebuffers = reinterpret_cast<DeleteFramebuffersFunction>(sf::Context::getFunction("glDeleteFramebuffers"));

		if (!genFramebuffers || !bindFramebuffer ||
			!framebufferTexture2D || !checkFramebufferStatus ||
			!deleteFramebuffers) {
			DebugError(L"OpenGL framebuffer functions are unavailable");
			return false;
		}

		DebugStat(L"OpenGL framebuffer functions loaded");

		genQueries = reinterpret_cast<GenQueriesFunction>(sf::Context::getFunction("glGenQueries"));
		deleteQueries = reinterpret_cast<DeleteQueriesFunction>(sf::Context::getFunction("glDeleteQueries"));
		beginQuery = reinterpret_cast<BeginQueryFunction>(sf::Context::getFunction("glBeginQuery"));
		endQuery = reinterpret_cast<EndQueryFunction>(sf::Context::getFunction("glEndQuery"));
		getQueryObjectuiv = reinterpret_cast<GetQueryObjectuivFunction>(sf::Context::getFunction("glGetQueryObjectuiv"));

		if (!genQueries || !deleteQueries ||
			!beginQuery || !endQuery || !getQueryObjectuiv) {
			DebugError(L"OpenGL query functions are unavailable");
			return false;
		}

		DebugStat(L"OpenGL query functions loaded");

		return true;
	}
}